// Copyright Epic Games, Inc. All Rights Reserved.

#include "Spaceship.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "CockpitComponent.h"
#include "EngineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "GameFramework/GameStateBase.h"

// Forward declarations for components that will be implemented
#include "FlightController.h"
#include "ShipSystemsManager.h"
#include "PerformanceProfilerSubsystem.h"

// Agent 3 components (fully integrated)
#include "PlayerOriginManager.h"
#include "GravitySimulator.h"
#include "CelestialBodyRegistry.h"
#include "CelestialScalingSafetySystem.h"

// ============================================================================
// NETWORK SERIALIZATION IMPLEMENTATIONS
// ============================================================================

bool FSpaceshipVirtualPosition::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;

	// Serialize sector coordinates (3 x int32 = 12 bytes)
	Ar << SectorCoordinates.X;
	Ar << SectorCoordinates.Y;
	Ar << SectorCoordinates.Z;

	// Serialize local position (3 x float = 12 bytes)
	Ar << LocalPosition.X;
	Ar << LocalPosition.Y;
	Ar << LocalPosition.Z;

	// Total: 24 bytes per position (much better than 3 x double = 24 bytes + overhead)

	// Reconstruct absolute position on receiving end
	if (Ar.IsLoading())
	{
		const double SectorSize = 100000.0; // 100km sectors
		AbsolutePosition.X = static_cast<double>(SectorCoordinates.X) * SectorSize + static_cast<double>(LocalPosition.X);
		AbsolutePosition.Y = static_cast<double>(SectorCoordinates.Y) * SectorSize + static_cast<double>(LocalPosition.Y);
		AbsolutePosition.Z = static_cast<double>(SectorCoordinates.Z) * SectorSize + static_cast<double>(LocalPosition.Z);
	}

	return true;
}

int64 FSpaceshipVirtualPosition::CompressCoordinate(double Coordinate) const
{
	// Convert meters to centimeters and store as int64
	// Precision: 1cm up to ~92 million km (beyond Mars orbit)
	return static_cast<int64>(Coordinate * 100.0);
}

double FSpaceshipVirtualPosition::DecompressCoordinate(int64 Compressed) const
{
	// Convert centimeters back to meters
	return static_cast<double>(Compressed) / 100.0;
}

FVector FSpaceshipNetworkState::CompressVelocity(const FVector& Velocity)
{
	// Compress velocity to 16-bit precision per axis
	// Range: -32768 to 32767 m/s (±32 km/s, sufficient for most spacecraft)
	// If higher speeds needed, scale factor can be adjusted
	const float Scale = 1.0f; // 1 m/s resolution
	return FVector(
		FMath::Clamp(FMath::RoundToFloat(Velocity.X / Scale), -32768.0f, 32767.0f) * Scale,
		FMath::Clamp(FMath::RoundToFloat(Velocity.Y / Scale), -32768.0f, 32767.0f) * Scale,
		FMath::Clamp(FMath::RoundToFloat(Velocity.Z / Scale), -32768.0f, 32767.0f) * Scale
	);
}

FVector FSpaceshipNetworkState::DecompressVelocity(const FVector& Compressed)
{
	// Velocity is already in correct scale, just return it
	return Compressed;
}

ASpaceship::ASpaceship()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set as root of orbital system by default
	OrbitMode = EOrbitMode::Root;
	Mass = 50000.0f; // 50 tons default spaceship mass

	// Create root component
	ShipRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ShipRoot"));
	RootComponent = ShipRoot;

	// Create ship mesh
	ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	ShipMesh->SetupAttachment(ShipRoot);
	ShipMesh->SetCollisionProfileName(TEXT("Pawn"));

	// Create VR camera
	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->SetupAttachment(ShipRoot);

	// Create ship components properly in constructor (not BeginPlay)
	CockpitComponent = CreateDefaultSubobject<UCockpitComponent>(TEXT("CockpitComponent"));
	CockpitComponent->SetupAttachment(ShipRoot);

	EngineComponent = CreateDefaultSubobject<UEngineComponent>(TEXT("EngineComponent"));

	FlightController = CreateDefaultSubobject<UFlightController>(TEXT("FlightController"));

	SystemsManager = CreateDefaultSubobject<UShipSystemsManager>(TEXT("SystemsManager"));

	OriginManager = nullptr; // Will be found/created in BeginPlay
	// VRPawn = nullptr; // VRPawn disabled for UE 5.6 compatibility

	// Default flight properties
	MaxThrust = 100000.0f;
	MaxAngularVelocity = 90.0f;
	ThrustSensitivity = 0.5f;
	RotationSensitivity = 0.5f;

	// Initialize state
	DamageState = EShipDamageState::Pristine;
	HullIntegrity = 100.0f;
	ThrustInput = FVector::ZeroVector;
	RotationInput = FVector::ZeroVector;

	// Performance tracking
	LastPhysicsUpdateTime = 0.0f;
	PhysicsUpdateCount = 0;

	// Network setup
	bReplicates = true;
	bAlwaysRelevant = false; // Use distance-based relevance
	// UE 5.6: Use NetUpdateFrequency property directly instead of SetNetUpdateFrequency()
	NetUpdateFrequency = 20.0f; // 20 updates per second default
	MinNetUpdateFrequency = 5.0f;
	NetCullDistanceSquared = 100000000.0f; // 100km squared

	// Origin-centered physics
	bUseOriginCenteredPhysics = true;
	bOriginSystemInitialized = false;
	CurrentScaleFactor = 1.0f;
	MaxOriginDistance = 10000.0f;

	// Network prediction
	bEnableClientPrediction = true;
	bEnableServerReconciliation = true;
	NetworkUpdateRate = 20.0f;
	NetworkSequenceCounter = 0;
	LastNetworkUpdateTime = 0.0f;
}

void ASpaceship::BeginPlay()
{
	Super::BeginPlay();

	// Components are now created in constructor (lines 120-127)
	// Verify they exist
	check(CockpitComponent);
	check(EngineComponent);
	check(FlightController);
	check(SystemsManager);

	// Initialize origin-centered physics system (Agent 3 integration)
	if (bUseOriginCenteredPhysics)
	{
		InitializeOriginSystem();
	}

	// Initialize VR if enabled
	if (bIsVREnabled)
	{
		InitializeVR();
	}

	// Initialize cockpit
	if (CockpitComponent)
	{
		CockpitComponent->InitializeCockpit();
	}

	// Initialize engine
	if (EngineComponent)
	{
		EngineComponent->StartEngine();
	}

	// Setup default orbital parameters if we have a target
	if (OrbitTarget.IsValid() && OrbitMode == EOrbitMode::Orbit)
	{
		InitializeCircularOrbit();
	}

	// Initialize virtual position from current world position
	VirtualPosition.AbsolutePosition = FVector(GetActorLocation());
	VirtualPosition.SectorCoordinates = FIntVector::ZeroValue;
	VirtualPosition.LocalPosition = GetActorLocation();

	// Network: Request full state sync if we're a client
	if (GetLocalRole() == ROLE_AutonomousProxy && !HasAuthority())
	{
		ServerRequestFullStateSync();
	}

	UE_LOG(LogTemp, Log, TEXT("Spaceship '%s' initialized - VR: %s, Flight Mode: %s, Origin System: %s, Network Role: %s"),
		*GetName(),
		bIsVREnabled ? TEXT("Enabled") : TEXT("Disabled"),
		*UEnum::GetValueAsString(FlightMode),
		bOriginSystemInitialized ? TEXT("Active") : TEXT("Disabled"),
		*UEnum::GetValueAsString(GetLocalRole()));
}

void ASpaceship::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Profile spaceship tick (includes network replication)
	UPerformanceProfilerSubsystem* Profiler = GetWorld()->GetSubsystem<UPerformanceProfilerSubsystem>();
	PROFILE_SCOPE(Profiler, FName("Spaceship_Tick"));

	// ORIGIN-CENTERED PHYSICS INTEGRATION (Agent 4 implementation)
	if (bUseOriginCenteredPhysics && OriginManager && bOriginSystemInitialized)
	{
		// NEW BEHAVIOR: Origin-centered universe translation
		UpdateOriginCenteredPhysics(DeltaTime);
	}
	else
	{
		// OLD BEHAVIOR: Traditional world-space movement (fallback)
		UpdateFlightPhysics(DeltaTime);
	}

	// Apply gravitational forces (Agent 3 GravitySimulator integration)
	ApplyGravitationalForces(DeltaTime);

	// Update ship systems
	UpdateShipSystems(DeltaTime);

	// Process input
	ProcessFlightInput(DeltaTime);

	// Apply flight mode behavior
	ApplyFlightModeBehavior(DeltaTime);

	// Update damage state
	UpdateDamageState();

	// Safety systems (clamp values, detect errors)
	ClampPhysicsValues();
	DetectAndRecoverPrecisionErrors();

	// Network updates (server authoritative, client prediction)
	if (HasAuthority())
	{
		// Server: Update network state and send to clients
		if (ShouldSendNetworkUpdate())
		{
			UpdateNetworkState();
			SendNetworkUpdate();
		}
	}
	else if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		// Client: Predict movement locally
		if (bEnableClientPrediction)
		{
			PredictMovementLocal(DeltaTime);
		}

		// Apply smooth correction if prediction error exists
		if (bEnableServerReconciliation && ClientPrediction.PredictionError.SizeSquared() > NETWORK_CORRECTION_THRESHOLD * NETWORK_CORRECTION_THRESHOLD)
		{
			ApplySmoothCorrection(DeltaTime);
		}
	}

	// Check for network desync
	DetectAndRecoverNetworkDesync();

	// Check if origin recentering needed
	if (ShouldRecenterOrigin())
	{
		RecenterOrigin();
	}

	// Draw debug info
	if (bShowFlightDebug)
	{
		DrawDebugInfo();
	}

	// Performance tracking
	LastPhysicsUpdateTime = DeltaTime;
	PhysicsUpdateCount++;
	TimeSinceLastNetworkUpdate += DeltaTime;
}

void ASpaceship::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Super::SetupPlayerInputComponent(PlayerInputComponent); // AOrbitalBody doesn't have this method

	// Flight controls - disabled for UE 5.6 compatibility issues
	// PlayerInputComponent->BindAxis("Thrust", this, &ASpaceship::ApplyThrust);
	// PlayerInputComponent->BindAxis("Strafe", this, &ASpaceship::ApplyThrust);
	// PlayerInputComponent->BindAxis("Vertical", this, &ASpaceship::ApplyThrust);
	// PlayerInputComponent->BindAxis("Pitch", this, &ASpaceship::ApplyRotation);
	// PlayerInputComponent->BindAxis("Yaw", this, &ASpaceship::ApplyRotation);
	// PlayerInputComponent->BindAxis("Roll", this, &ASpaceship::ApplyRotation);

	// Flight mode controls
	// PlayerInputComponent->BindAction("ToggleAssistedFlight", IE_Pressed, this, &ASpaceship::ToggleAssistedFlight);
	// PlayerInputComponent->BindAction("EmergencyStop", IE_Pressed, this, &ASpaceship::EmergencyStop);
}

void ASpaceship::ApplyThrust(FVector LocalThrust)
{
	ThrustInput = LocalThrust * ThrustSensitivity;
	bIsThrusting = !ThrustInput.IsNearlyZero();
}

void ASpaceship::ApplyRotation(FVector LocalRotation)
{
	RotationInput = LocalRotation * RotationSensitivity;
	bIsRotating = !RotationInput.IsNearlyZero();
}

void ASpaceship::SetFlightMode(EFlightMode NewMode)
{
	if (FlightMode != NewMode)
	{
		FlightMode = NewMode;
		UE_LOG(LogTemp, Log, TEXT("Spaceship '%s' flight mode changed to: %s"), 
			*GetName(), *UEnum::GetValueAsString(NewMode));
	}
}

void ASpaceship::ToggleAssistedFlight()
{
	if (FlightMode == EFlightMode::Normal)
	{
		SetFlightMode(EFlightMode::Assisted);
	}
	else if (FlightMode == EFlightMode::Assisted)
	{
		SetFlightMode(EFlightMode::Normal);
	}
}

void ASpaceship::EmergencyStop()
{
	Velocity = FVector::ZeroVector;
	CurrentAngularVelocity = FVector::ZeroVector;
	ThrustInput = FVector::ZeroVector;
	RotationInput = FVector::ZeroVector;
	
	if (EngineComponent)
	{
		EngineComponent->SetThrustLevel(0.0f);
	}

	UE_LOG(LogTemp, Warning, TEXT("Spaceship '%s' emergency stop activated"), *GetName());
}

// void ASpaceship::TakeDamage(float DamageAmount, FVector HitLocation, FVector HitNormal)
// {
// 	HullIntegrity = FMath::Max(0.0f, HullIntegrity - DamageAmount);
// 	
// 	// Apply damage to engine if hit
// 	if (EngineComponent)
// 	{
// 		EngineComponent->RepairEngine(-DamageAmount * 0.1f); // Negative repair = damage
// 	}
// 
// 	// Apply damage to cockpit systems
// 	if (CockpitComponent)
// 	{
// 		CockpitComponent->ApplyDamageEffects(DamageAmount);
// 	}
// 
// 	UE_LOG(LogTemp, Warning, TEXT("Spaceship '%s' took %.1f damage, hull integrity: %.1f%%"), 
// 		*GetName(), DamageAmount, HullIntegrity);
// }

void ASpaceship::RepairSystems(float RepairAmount)
{
	HullIntegrity = FMath::Min(100.0f, HullIntegrity + RepairAmount);
	
	if (EngineComponent)
	{
		EngineComponent->RepairEngine(RepairAmount);
	}

	UE_LOG(LogTemp, Log, TEXT("Spaceship '%s' repaired %.1f, hull integrity: %.1f%%"), 
		*GetName(), RepairAmount, HullIntegrity);
}

bool ASpaceship::IsOperational() const
{
	return HullIntegrity > 0.0f && 
		(EngineComponent ? EngineComponent->IsOperational() : true) &&
		DamageState != EShipDamageState::Destroyed;
}

FString ASpaceship::GetShipStatus() const
{
	FString Status = FString::Printf(TEXT("Ship: %s\n"), *GetName());
	Status += FString::Printf(TEXT("Hull: %.1f%%\n"), HullIntegrity);
	Status += FString::Printf(TEXT("Damage State: %s\n"), *UEnum::GetValueAsString(DamageState));
	Status += FString::Printf(TEXT("Flight Mode: %s\n"), *UEnum::GetValueAsString(FlightMode));
	Status += FString::Printf(TEXT("Velocity: %.1f m/s\n"), Velocity.Size());
	
	if (EngineComponent)
	{
		Status += FString::Printf(TEXT("Engine: %s\n"), *EngineComponent->GetEngineStatus());
	}
	
	return Status;
}

void ASpaceship::InitializeVR()
{
	// Get VR pawn if available
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			// VRPawn = Cast<AVRCharacter>(Pawn); // VRPawn disabled for UE 5.6 compatibility
		}
	}

	// Setup VR camera position
	if (CockpitComponent)
	{
		FTransform CockpitTransform = GetCockpitTransform(); // Call method on this spaceship, not on CockpitComponent
		VRCamera->SetRelativeTransform(CockpitTransform);
	}

	UE_LOG(LogTemp, Log, TEXT("VR systems initialized for spaceship '%s'"), *GetName());
}

FTransform ASpaceship::GetCockpitTransform() const
{
	if (CockpitComponent)
	{
		return CockpitComponent->GetComponentTransform();
	}
	return FTransform::Identity;
}

void ASpaceship::HandleVRControllerInteraction(UMotionControllerComponent* Controller, bool bIsRightHand)
{
	// VR controller interaction disabled for UE 5.6 compatibility
	// if (CockpitComponent)
	// {
	//     CockpitComponent->HandleVRControllerInteraction(Controller, bIsRightHand);
	// }
}

void ASpaceship::UpdateFlightPhysics(float DeltaTime)
{
	if (!IsOperational())
		return;

	// Calculate thrust in world space
	FVector LocalThrust = ThrustInput * MaxThrust;
	FVector WorldThrust = GetActorTransform().TransformVectorNoScale(LocalThrust);

	// Apply thrust to velocity
	if (bIsThrusting && EngineComponent)
	{
		float ActualThrust = EngineComponent->GetActualThrust();
		FVector ThrustForce = WorldThrust.GetSafeNormal() * ActualThrust;
		Velocity += ThrustForce / Mass * DeltaTime;
	}

	// Apply rotation
	if (bIsRotating)
	{
		FVector RotationVelocity = RotationInput * MaxAngularVelocity;
		FRotator DeltaRotation = FRotator(
			RotationVelocity.X * DeltaTime,
			RotationVelocity.Y * DeltaTime,
			RotationVelocity.Z * DeltaTime
		);
		AddActorWorldRotation(DeltaRotation);
		CurrentAngularVelocity = RotationVelocity;
	}
	else
	{
		CurrentAngularVelocity = FVector::ZeroVector;
	}

	// Update engine thrust level
	if (EngineComponent)
	{
		float ThrustLevel = ThrustInput.Size();
		EngineComponent->SetThrustLevel(ThrustLevel);
	}
}

void ASpaceship::UpdateShipSystems(float DeltaTime)
{
	// Update engine systems
	if (EngineComponent)
	{
		// Engine component handles its own updates in TickComponent
	}

	// Update cockpit systems
	if (CockpitComponent)
	{
		// Cockpit component handles its own updates in TickComponent
	}

	// Update ship systems manager
	if (SystemsManager)
	{
		// SystemsManager->UpdateSystems(DeltaTime, HullIntegrity); // Method doesn't exist in UShipSystemsManager
	}
}

void ASpaceship::ProcessFlightInput(float DeltaTime)
{
	// Input is processed in ApplyThrust and ApplyRotation
	// This function can be used for additional input processing
}

void ASpaceship::ApplyFlightModeBehavior(float DeltaTime)
{
	switch (FlightMode)
	{
	case EFlightMode::Assisted:
		// Apply automatic stabilization
		if (!bIsRotating)
		{
			// Dampen angular velocity
			CurrentAngularVelocity *= FMath::Exp(-2.0f * DeltaTime);
		}
		if (!bIsThrusting)
		{
			// Dampen linear velocity slightly
			Velocity *= FMath::Exp(-0.1f * DeltaTime);
		}
		break;

	case EFlightMode::Newtonian:
		// Pure Newtonian physics - no assistance
		break;

	case EFlightMode::Docking:
		// Precision control mode
		MaxThrust = 25000.0f; // Reduced thrust
		MaxAngularVelocity = 30.0f; // Reduced rotation
		break;

	case EFlightMode::Combat:
		// Enhanced maneuverability
		MaxThrust = 150000.0f; // Increased thrust
		MaxAngularVelocity = 120.0f; // Increased rotation
		break;

	default:
		// Normal mode
		MaxThrust = 100000.0f;
		MaxAngularVelocity = 90.0f;
		break;
	}
}

void ASpaceship::UpdateDamageState()
{
	EShipDamageState NewState = EShipDamageState::Pristine;

	if (HullIntegrity <= 0.0f)
	{
		NewState = EShipDamageState::Destroyed;
	}
	else if (HullIntegrity <= 20.0f)
	{
		NewState = EShipDamageState::Critical;
	}
	else if (HullIntegrity <= 40.0f)
	{
		NewState = EShipDamageState::HeavyDamage;
	}
	else if (HullIntegrity <= 60.0f)
	{
		NewState = EShipDamageState::ModerateDamage;
	}
	else if (HullIntegrity <= 80.0f)
	{
		NewState = EShipDamageState::MinorDamage;
	}

	if (NewState != DamageState)
	{
		DamageState = NewState;
		UE_LOG(LogTemp, Warning, TEXT("Spaceship '%s' damage state changed to: %s"), 
			*GetName(), *UEnum::GetValueAsString(DamageState));
	}
}

void ASpaceship::DrawDebugInfo()
{
	if (!GetWorld())
		return;

	FVector ShipLocation = GetActorLocation();

	// Draw velocity vector
	if (Velocity.Size() > 0.1f)
	{
		DrawDebugDirectionalArrow(GetWorld(), ShipLocation, ShipLocation + Velocity, 100.0f, FColor::Green, false, 0.0f, 0, 2.0f);
	}

	// Draw thrust vector
	if (bIsThrusting)
	{
		FVector LocalThrust = ThrustInput * MaxThrust;
		FVector WorldThrust = GetActorTransform().TransformVectorNoScale(LocalThrust);
		DrawDebugDirectionalArrow(GetWorld(), ShipLocation, ShipLocation + WorldThrust * 0.001f, 100.0f, FColor::Red, false, 0.0f, 0, 2.0f);
	}

	// Draw ship status
	if (bShowShipStatus)
	{
		FString StatusText = GetShipStatus();
		DrawDebugString(GetWorld(), ShipLocation + FVector(0, 0, 200.0f), StatusText, nullptr, FColor::White, 0.0f, true);
	}

	// Draw damage state color
	FColor DamageColor = FColor::Green;
	switch (DamageState)
	{
	case EShipDamageState::MinorDamage:
		DamageColor = FColor::Yellow;
		break;
	case EShipDamageState::ModerateDamage:
		DamageColor = FColor::Orange;
		break;
	case EShipDamageState::HeavyDamage:
		DamageColor = FColor::Red;
		break;
	case EShipDamageState::Critical:
		DamageColor = FColor::Magenta;
		break;
	case EShipDamageState::Destroyed:
		DamageColor = FColor::Black;
		break;
	}

	DrawDebugSphere(GetWorld(), ShipLocation, 50.0f, 16, DamageColor, false, 0.0f, 0, 2.0f);
}
// ============================================================================
// ORIGIN-CENTERED PHYSICS IMPLEMENTATIONS (Agent 4)
// ============================================================================

void ASpaceship::InitializeOriginSystem()
{
	// AGENT 4 IMPLEMENTATION: Full integration with Agent 3 components

	// Try to find existing PlayerOriginManager component
	OriginManager = FindComponentByClass<UPlayerOriginManager>();

	// If not found, create it
	if (!OriginManager)
	{
		OriginManager = NewObject<UPlayerOriginManager>(this, TEXT("OriginManager"));
		if (OriginManager)
		{
			OriginManager->RegisterComponent();
			OriginManager->bAutoRecenterUniverse = true;
			OriginManager->PrecisionThreshold = MaxOriginDistance;

			UE_LOG(LogTemp, Log, TEXT("Spaceship '%s': Created new PlayerOriginManager component"), *GetName());
		}
	}

	// Validate safety system
	if (UCelestialScalingSafetySystem* SafetySystem = GetWorld()->GetSubsystem<UCelestialScalingSafetySystem>())
	{
		UE_LOG(LogTemp, Log, TEXT("Spaceship '%s': CelestialScalingSafetySystem available"), *GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Spaceship '%s': CelestialScalingSafetySystem not available - safety checks disabled"), *GetName());
	}

	// Check for GravitySimulator subsystem
	if (UGravitySimulator* GravSim = GetWorld()->GetSubsystem<UGravitySimulator>())
	{
		UE_LOG(LogTemp, Log, TEXT("Spaceship '%s': GravitySimulator subsystem available"), *GetName());
	}

	if (OriginManager)
	{
		bOriginSystemInitialized = true;
		UE_LOG(LogTemp, Log, TEXT("Spaceship '%s': Origin-centered physics system initialized successfully"), *GetName());
	}
	else
	{
		bOriginSystemInitialized = false;
		UE_LOG(LogTemp, Warning, TEXT("Spaceship '%s': Failed to initialize origin system - using fallback world-space mode"), *GetName());
	}
}

FVector ASpaceship::CalculateMovementThisFrame(float DeltaTime)
{
	if (!IsOperational())
		return FVector::ZeroVector;

	// Calculate thrust in world space
	FVector LocalThrust = ThrustInput * MaxThrust;
	FVector WorldThrust = GetActorTransform().TransformVectorNoScale(LocalThrust);

	// Calculate acceleration from thrust
	FVector ThrustAcceleration = FVector::ZeroVector;
	if (bIsThrusting && EngineComponent)
	{
		float ActualThrust = EngineComponent->GetActualThrust();
		ThrustAcceleration = WorldThrust.GetSafeNormal() * ActualThrust / FMath::Max(Mass, 1.0f);
	}

	// Calculate movement from current velocity + acceleration
	FVector NewVelocity = Velocity + ThrustAcceleration * DeltaTime;
	FVector Movement = NewVelocity * DeltaTime;

	// Update velocity for next frame
	Velocity = NewVelocity;

	return Movement;
}

void ASpaceship::ApplyMovementWorldSpace(float DeltaTime)
{
	// Traditional world-space movement (fallback mode)
	FVector Movement = CalculateMovementThisFrame(DeltaTime);
	AddActorWorldOffset(Movement, true);
}


void ASpaceship::UpdateOriginCenteredPhysics(float DeltaTime)
{
	// AGENT 4 IMPLEMENTATION: Origin-centered universe translation with full integration
	// The spaceship stays near origin (0,0,0) and the universe moves around it

	if (!ValidateOriginManager())
	{
		FallbackToWorldSpaceMode(TEXT("OriginManager validation failed"));
		return;
	}

	// Calculate intended movement for this frame
	FVector IntendedMovement = CalculateMovementThisFrame(DeltaTime);

	// AGENT 3 INTEGRATION: Tell OriginManager to translate the universe instead of moving ship
	if (OriginManager && IntendedMovement.SizeSquared() > KINDA_SMALL_NUMBER)
	{
		// Move in virtual space
		OriginManager->MovePlayer(IntendedMovement);

		// Update our virtual position cache
		VirtualPosition.AbsolutePosition = FVector(OriginManager->GetVirtualPositionAsWorld());
		VirtualPosition.SectorCoordinates = OriginManager->GetSectorCoordinates();
		VirtualPosition.LocalPosition = OriginManager->GetLocalPosition();
	}

	// Keep spaceship at or near origin for precision
	// Small local adjustments are OK, but large movements translate the universe
	FVector CurrentPosition = GetActorLocation();
	DistanceFromOrigin = CurrentPosition.Size();

	// Safety check: If ship drifts too far from origin, snap back
	if (DistanceFromOrigin > MaxOriginDistance * 0.5f)
	{
		// Gradually pull back to origin
		FVector CorrectionVector = -CurrentPosition.GetSafeNormal() * (DistanceFromOrigin - MaxOriginDistance * 0.25f);
		AddActorWorldOffset(CorrectionVector * DeltaTime * 0.1f);

		if (UCelestialScalingSafetySystem* SafetySystem = GetWorld()->GetSubsystem<UCelestialScalingSafetySystem>())
		{
			SafetySystem->LogSafetyEvent(
				FString::Printf(TEXT("Ship drifting from origin (%.2f m) - applying correction"), DistanceFromOrigin),
				ESafetyErrorSeverity::Warning,
				this
			);
		}
	}

	// Apply rotation (this is always local)
	if (bIsRotating)
	{
		FVector RotationVelocity = RotationInput * MaxAngularVelocity;
		FRotator DeltaRotation = FRotator(
			RotationVelocity.X * DeltaTime,
			RotationVelocity.Y * DeltaTime,
			RotationVelocity.Z * DeltaTime
		);
		AddActorWorldRotation(DeltaRotation);
		CurrentAngularVelocity = RotationVelocity;
	}

	// Update virtual position
	UpdateVirtualPosition();

	// Update engine thrust level
	if (EngineComponent)
	{
		float ThrustLevel = ThrustInput.Size();
		EngineComponent->SetThrustLevel(ThrustLevel);
	}
}

void ASpaceship::ApplyGravitationalForces(float DeltaTime)
{
	// AGENT 4 IMPLEMENTATION: Full integration with Agent 3's GravitySimulator

	UGravitySimulator* GravSim = GetWorld()->GetSubsystem<UGravitySimulator>();
	if (!GravSim || !GravSim->IsGravityEnabled())
		return;

	// Get current position for gravity calculation
	FVector CurrentPosition = GetActorLocation();
	if (OriginManager)
	{
		// Use virtual position for accurate gravity calculation
		CurrentPosition = OriginManager->GetVirtualPositionAsWorld();
	}

	// Calculate gravitational force from all celestial bodies
	FVector GravityForce = GravSim->CalculateTotalGravitationalForce(this, CurrentPosition);

	// SAFETY SYSTEM: Limit gravitational forces to prevent extreme G-forces
	LimitGravitationalForces(GravityForce, DeltaTime);

	// Apply force to velocity (F = ma, so a = F/m)
	if (Mass > KINDA_SMALL_NUMBER && GravityForce.SizeSquared() > KINDA_SMALL_NUMBER)
	{
		FVector Acceleration = GravityForce / Mass;
		Velocity += Acceleration * DeltaTime;

		// Debug visualization
		if (bShowFlightDebug && GetWorld())
		{
			DrawDebugDirectionalArrow(
				GetWorld(),
				GetActorLocation(),
				GetActorLocation() + GravityForce * 0.0001f, // Scale for visibility
				100.0f,
				FColor::Purple,
				false,
				0.0f,
				0,
				2.0f
			);
		}
	}
}

bool ASpaceship::ShouldRecenterOrigin() const
{
	// Check if spaceship has drifted too far from origin
	return DistanceFromOrigin > MaxOriginDistance;
}

void ASpaceship::RecenterOrigin()
{
	if (!OriginManager)
		return;

	UE_LOG(LogTemp, Warning, TEXT("Spaceship '%s': Recentering origin - distance was %.2f m"), *GetName(), DistanceFromOrigin);

	// AGENT 4 IMPLEMENTATION: Full integration with Agent 3's OriginManager
	// Tell OriginManager to recenter the entire universe
	OriginManager->RecenterUniverse();

	// Reset local position to near origin
	SetActorLocation(FVector::ZeroVector);
	DistanceFromOrigin = 0.0f;
	LastOriginPosition = FVector::ZeroVector;

	// Log to safety system
	if (UCelestialScalingSafetySystem* SafetySystem = GetWorld()->GetSubsystem<UCelestialScalingSafetySystem>())
	{
		SafetySystem->LogSafetyEvent(
			FString::Printf(TEXT("Universe recentered - ship was %.2f m from origin"), DistanceFromOrigin),
			ESafetyErrorSeverity::Info,
			this
		);
	}

	// Notify all clients in multiplayer
	if (HasAuthority())
	{
		MulticastUpdateUniversePosition(LastOriginPosition);
	}
}

void ASpaceship::DetectAndRecoverPrecisionErrors()
{
	// Check for floating-point precision degradation
	FVector CurrentPos = GetActorLocation();
	
	// Check for NaN or Inf values
	if (!CurrentPos.ContainsNaN() && FMath::IsFinite(CurrentPos.X) && FMath::IsFinite(CurrentPos.Y) && FMath::IsFinite(CurrentPos.Z))
	{
		// Position is valid
		PrecisionErrorCount = 0;
		return;
	}

	// Precision error detected!
	PrecisionErrorCount++;
	UE_LOG(LogTemp, Error, TEXT("Spaceship '%s': Precision error detected (count: %d) - Position: %s"), 
		*GetName(), PrecisionErrorCount, *CurrentPos.ToString());

	if (PrecisionErrorCount > 3)
	{
		// Critical error - force recovery
		UE_LOG(LogTemp, Error, TEXT("Spaceship '%s': CRITICAL precision errors - forcing recovery"), *GetName());
		
		// Reset to safe state
		SetActorLocation(FVector::ZeroVector);
		Velocity = FVector::ZeroVector;
		CurrentAngularVelocity = FVector::ZeroVector;
		VirtualPosition.AbsolutePosition = FVector::ZeroVector;
		VirtualPosition.LocalPosition = FVector::ZeroVector;
		
		PrecisionErrorCount = 0;
	}
}

void ASpaceship::UpdateVirtualPosition()
{
	// Update virtual position from world position
	FVector WorldPos = GetActorLocation();
	
	// Update absolute position (keeping high precision)
	VirtualPosition.AbsolutePosition = FVector(WorldPos);
	
	// Calculate sector coordinates (100km sectors)
	const double SectorSize = 100000.0;
	VirtualPosition.SectorCoordinates.X = FMath::FloorToInt(VirtualPosition.AbsolutePosition.X / SectorSize);
	VirtualPosition.SectorCoordinates.Y = FMath::FloorToInt(VirtualPosition.AbsolutePosition.Y / SectorSize);
	VirtualPosition.SectorCoordinates.Z = FMath::FloorToInt(VirtualPosition.AbsolutePosition.Z / SectorSize);
	
	// Calculate local position within sector
	VirtualPosition.LocalPosition.X = VirtualPosition.AbsolutePosition.X - (VirtualPosition.SectorCoordinates.X * SectorSize);
	VirtualPosition.LocalPosition.Y = VirtualPosition.AbsolutePosition.Y - (VirtualPosition.SectorCoordinates.Y * SectorSize);
	VirtualPosition.LocalPosition.Z = VirtualPosition.AbsolutePosition.Z - (VirtualPosition.SectorCoordinates.Z * SectorSize);
}


// ============================================================================
// NETWORK REPLICATION IMPLEMENTATIONS (Agent 4)
// ============================================================================

void ASpaceship::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ASpaceship, ReplicatedNetworkState, COND_SimulatedOnly);
}

void ASpaceship::OnRep_NetworkState()
{
	ProcessNetworkState(ReplicatedNetworkState);
}

void ASpaceship::UpdateNetworkState()
{
	if (!HasAuthority())
		return;
	ReplicatedNetworkState = BuildNetworkState();
	ReplicatedNetworkState.SequenceNumber = ++NetworkSequenceCounter;
	ReplicatedNetworkState.Timestamp = GetWorld()->GetTimeSeconds();
}

void ASpaceship::SendNetworkUpdate()
{
	if (!HasAuthority())
		return;
	LastNetworkUpdateTime = GetWorld()->GetTimeSeconds();
	TimeSinceLastNetworkUpdate = 0.0f;
	if ((GetActorLocation() - LastReplicatedPosition).SizeSquared() > 100.0f)
	{
		FVector OriginOffset = GetActorLocation() - LastReplicatedPosition;
		MulticastUpdateUniversePosition(OriginOffset);
		LastReplicatedPosition = GetActorLocation();
	}
}

bool ASpaceship::ShouldSendNetworkUpdate() const
{
	if (!HasAuthority())
		return false;
	float UpdateInterval = 1.0f / NetworkUpdateRate;
	return TimeSinceLastNetworkUpdate >= UpdateInterval;
}

FSpaceshipNetworkState ASpaceship::BuildNetworkState() const
{
	FSpaceshipNetworkState State;
	State.VirtualPos = VirtualPosition;
	State.CompressedVelocity = FSpaceshipNetworkState::CompressVelocity(Velocity);
	State.Rotation = GetActorRotation();
	State.CompressedAngularVelocity = FSpaceshipNetworkState::CompressVelocity(CurrentAngularVelocity);
	State.Timestamp = GetWorld()->GetTimeSeconds();
	State.SequenceNumber = NetworkSequenceCounter;
	State.ActiveScaleFactor = CurrentScaleFactor;
	State.InputStateHash = CalculateInputHash();

	// Phase 3: Include ship customization stats in network state
	State.ReplicatedMass = Mass;
	State.ReplicatedThrustPower = MaxThrust;
	State.ReplicatedMaxVelocity = ReplicatedNetworkState.ReplicatedMaxVelocity;

	return State;
}

void ASpaceship::ProcessNetworkState(const FSpaceshipNetworkState& State)
{
	if (HasAuthority())
		return;
	VirtualPosition = State.VirtualPos;
	FVector NewVelocity = FSpaceshipNetworkState::DecompressVelocity(State.CompressedVelocity);
	SetActorRotation(State.Rotation);
	CurrentScaleFactor = State.ActiveScaleFactor;

	// Phase 3: Apply replicated ship customization stats on clients
	Mass = State.ReplicatedMass;
	MaxThrust = State.ReplicatedThrustPower;
	// Note: MaxVelocity is handled by FlightController, updated via ShipCustomizationComponent

	if (bEnableServerReconciliation && GetLocalRole() == ROLE_AutonomousProxy)
	{
		ReconcileWithServerState(State);
	}
	else
	{
		Velocity = NewVelocity;
		CurrentAngularVelocity = FSpaceshipNetworkState::DecompressVelocity(State.CompressedAngularVelocity);
	}
}

uint32 ASpaceship::CalculateInputHash() const
{
	uint32 Hash = 0;
	Hash ^= GetTypeHash(ThrustInput.X);
	Hash ^= GetTypeHash(ThrustInput.Y) << 1;
	Hash ^= GetTypeHash(ThrustInput.Z) << 2;
	Hash ^= GetTypeHash(RotationInput.X) << 3;
	Hash ^= GetTypeHash(RotationInput.Y) << 4;
	Hash ^= GetTypeHash(RotationInput.Z) << 5;
	return Hash;
}

void ASpaceship::PredictMovementLocal(float DeltaTime)
{
	ClientPrediction.PredictedPosition = VirtualPosition;
	ClientPrediction.PredictedVelocity = Velocity;
	ClientPrediction.InputHistory.Add(CalculateInputHash());
	ClientPrediction.TimestampHistory.Add(GetWorld()->GetTimeSeconds());
	while (ClientPrediction.TimestampHistory.Num() > 60)
	{
		ClientPrediction.InputHistory.RemoveAt(0);
		ClientPrediction.TimestampHistory.RemoveAt(0);
	}
}

void ASpaceship::ReconcileWithServerState(const FSpaceshipNetworkState& ServerState)
{
	FVector ServerPosition = FVector(ServerState.VirtualPos.AbsolutePosition);
	FVector PredictedPosition = FVector(ClientPrediction.PredictedPosition.AbsolutePosition);
	FVector Error = ServerPosition - PredictedPosition;
	float ErrorMagnitude = Error.Size();
	if (ErrorMagnitude > NETWORK_CORRECTION_THRESHOLD)
	{
		UE_LOG(LogTemp, Verbose, TEXT("Spaceship '%s': Prediction error %.2f m - applying correction"), *GetName(), ErrorMagnitude);
		ClientPrediction.PredictionError = Error;
		ClientPrediction.LastServerState = ServerState;
	}
	else
	{
		ClientPrediction.PredictionError = FVector::ZeroVector;
	}
}

void ASpaceship::ApplySmoothCorrection(float DeltaTime)
{
	const float CorrectionSpeed = 5.0f;
	float CorrectionAmount = FMath::Clamp(DeltaTime * CorrectionSpeed, 0.0f, 1.0f);
	FVector Correction = ClientPrediction.PredictionError * CorrectionAmount;
	AddActorWorldOffset(Correction);
	ClientPrediction.PredictionError *= (1.0f - CorrectionAmount);
	if (ClientPrediction.PredictionError.SizeSquared() < 0.01f)
	{
		ClientPrediction.PredictionError = FVector::ZeroVector;
	}
}

float ASpaceship::CalculatePredictionError() const
{
	return ClientPrediction.PredictionError.Size();
}


// ============================================================================
// RPC IMPLEMENTATIONS (Agent 4)
// ============================================================================

void ASpaceship::ServerMoveSpaceship_Implementation(FVector Movement, FVector AngularMovement, float DeltaTime, uint32 InputHash, float Timestamp)
{
	if (Movement.SizeSquared() > MAX_VELOCITY * MAX_VELOCITY * DeltaTime * DeltaTime)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spaceship '%s': Client sent invalid movement"), *GetName());
		return;
	}
	ThrustInput = Movement / (MaxThrust * DeltaTime);
	RotationInput = AngularMovement / (MaxAngularVelocity * DeltaTime);
}

bool ASpaceship::ServerMoveSpaceship_Validate(FVector Movement, FVector AngularMovement, float DeltaTime, uint32 InputHash, float Timestamp)
{
	return DeltaTime > 0.0f && DeltaTime < 1.0f;
}

void ASpaceship::MulticastUpdateUniversePosition_Implementation(FVector OriginOffset)
{
	if (HasAuthority())
		return;
	UE_LOG(LogTemp, Verbose, TEXT("Spaceship '%s': Received universe position update: %s"), *GetName(), *OriginOffset.ToString());
}

void ASpaceship::ClientCorrectPrediction_Implementation(FSpaceshipNetworkState CorrectState)
{
	ProcessNetworkState(CorrectState);
}

void ASpaceship::ServerRequestFullStateSync_Implementation()
{
	if (!HasAuthority())
		return;
	FSpaceshipNetworkState FullState = BuildNetworkState();
	// Note: Since ASpaceship inherits from AOrbitalBody (AActor), not APawn,
	// we don't have GetController(). Late-joining clients will receive state via OnRep.
	ClientReceiveFullState(FullState, VirtualPosition);
	UE_LOG(LogTemp, Log, TEXT("Spaceship '%s': Sent full state sync to client"), *GetName());
}

bool ASpaceship::ServerRequestFullStateSync_Validate()
{
	return true;
}

void ASpaceship::ClientReceiveFullState_Implementation(FSpaceshipNetworkState FullState, FSpaceshipVirtualPosition Position)
{
	UE_LOG(LogTemp, Log, TEXT("Spaceship '%s': Received full state sync from server"), *GetName());
	ProcessNetworkState(FullState);
	VirtualPosition = Position;
	ClientPrediction.PredictionError = FVector::ZeroVector;
	ClientPrediction.InputHistory.Empty();
	ClientPrediction.TimestampHistory.Empty();
}

// ============================================================================
// SHIP CUSTOMIZATION NETWORK RPC IMPLEMENTATIONS (Phase 3)
// ============================================================================

bool ASpaceship::ServerApplyCustomization_Validate(float NewMass, float NewThrustPower, float NewMaxVelocity)
{
	// Validate ranges to prevent cheating
	// Mass: 100 kg (tiny ship) to 1,000,000 kg (massive capital ship)
	// ThrustPower: 1,000 N (weak) to 10,000,000 N (powerful)
	// MaxVelocity: 100 m/s (slow) to 100,000 m/s (very fast)
	return NewMass > 0.0f && NewMass < 1000000.0f &&
	       NewThrustPower > 0.0f && NewThrustPower < 10000000.0f &&
	       NewMaxVelocity > 0.0f && NewMaxVelocity < 100000.0f;
}

void ASpaceship::ServerApplyCustomization_Implementation(float NewMass, float NewThrustPower, float NewMaxVelocity)
{
	// SERVER AUTHORITY: Apply ship customization stats
	// This is called by clients via RPC when they change ship customization

	UE_LOG(LogTemp, Log, TEXT("Spaceship '%s': Server applying customization - Mass: %.2f, Thrust: %.2f, MaxVel: %.2f"),
		*GetName(), NewMass, NewThrustPower, NewMaxVelocity);

	// Apply mass to OrbitalBody (affects gravity calculations)
	Mass = NewMass;

	// Apply thrust power (NOTE: MaxThrust is used by UpdateFlightPhysics)
	MaxThrust = NewThrustPower;

	// Apply max velocity (stored in network state for replication)
	// NOTE: FlightController's MaxSafeVelocity is applied in ShipCustomizationComponent
	// We store it in network state for replication to all clients

	// Update replicated network state
	ReplicatedNetworkState.ReplicatedMass = NewMass;
	ReplicatedNetworkState.ReplicatedThrustPower = NewThrustPower;
	ReplicatedNetworkState.ReplicatedMaxVelocity = NewMaxVelocity;

	// Force network update to replicate to all clients
	UpdateNetworkState();
	SendNetworkUpdate();

	UE_LOG(LogTemp, Log, TEXT("Spaceship '%s': Customization applied and replicated to clients"), *GetName());
}

// ============================================================================
// ERROR HANDLING & SAFETY IMPLEMENTATIONS (Agent 4)
// ============================================================================

void ASpaceship::ClampPhysicsValues()
{
	if (Velocity.SizeSquared() > MAX_VELOCITY * MAX_VELOCITY)
	{
		Velocity = Velocity.GetSafeNormal() * MAX_VELOCITY;
		UE_LOG(LogTemp, Warning, TEXT("Spaceship '%s': Velocity clamped to MAX_VELOCITY"), *GetName());
	}
	if (CurrentAngularVelocity.SizeSquared() > MAX_ANGULAR_VELOCITY_RAD * MAX_ANGULAR_VELOCITY_RAD)
	{
		CurrentAngularVelocity = CurrentAngularVelocity.GetSafeNormal() * MAX_ANGULAR_VELOCITY_RAD;
	}
	if (ThrustInput.SizeSquared() > 1.0f)
	{
		ThrustInput = ThrustInput.GetSafeNormal();
	}
	if (RotationInput.SizeSquared() > 1.0f)
	{
		RotationInput = RotationInput.GetSafeNormal();
	}
}

void ASpaceship::LimitGravitationalForces(FVector& GravityForce, float DeltaTime)
{
	// AGENT 4 IMPLEMENTATION: Use SafetySystem for force limiting

	UCelestialScalingSafetySystem* SafetySystem = GetWorld()->GetSubsystem<UCelestialScalingSafetySystem>();
	if (!SafetySystem)
	{
		// Fallback: Manual limiting
		float GravityAcceleration = GravityForce.Size() / FMath::Max(Mass, 1.0f);
		float MaxAcceleration = MAX_G_FORCE * 9.81f;
		if (GravityAcceleration > MaxAcceleration)
		{
			GravityForce = GravityForce.GetSafeNormal() * MaxAcceleration * Mass;
			UE_LOG(LogTemp, Warning, TEXT("Spaceship '%s': Gravitational force limited to %.1f G"), *GetName(), MAX_G_FORCE);
		}
		return;
	}

	// Use safety system for intelligent force clamping
	FVector OriginalForce = GravityForce;
	GravityForce = SafetySystem->ClampGravitationalForce(GravityForce, Mass, MAX_G_FORCE);

	// Log if force was clamped
	if (!GravityForce.Equals(OriginalForce, 0.01f))
	{
		float GForce = SafetySystem->CalculateGForce(OriginalForce / Mass);
		SafetySystem->LogSafetyEvent(
			FString::Printf(TEXT("Gravitational force clamped from %.1f G to %.1f G"), GForce, MAX_G_FORCE),
			ESafetyErrorSeverity::Warning,
			this
		);
	}
}

void ASpaceship::DetectAndRecoverNetworkDesync()
{
	if (GetLocalRole() != ROLE_AutonomousProxy)
		return;
	float PredictionError = CalculatePredictionError();
	if (PredictionError > 100.0f)
	{
		NetworkDesyncCount++;
		if (NetworkDesyncCount > 10)
		{
			UE_LOG(LogTemp, Error, TEXT("Spaceship '%s': Network desync detected - requesting full state sync"), *GetName());
			ServerRequestFullStateSync();
			NetworkDesyncCount = 0;
		}
	}
	else
	{
		NetworkDesyncCount = 0;
	}
}

void ASpaceship::FallbackToWorldSpaceMode(const FString& Reason)
{
	if (!bUseOriginCenteredPhysics)
		return;
	UE_LOG(LogTemp, Error, TEXT("Spaceship '%s': Falling back to world-space mode - Reason: %s"), *GetName(), *Reason);
	bUseOriginCenteredPhysics = false;
	bOriginSystemInitialized = false;
}

bool ASpaceship::ValidateOriginManager() const
{
	return OriginManager != nullptr;
}
