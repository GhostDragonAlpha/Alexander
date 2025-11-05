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

// Forward declarations for components that will be implemented
#include "FlightController.h"
#include "ShipSystemsManager.h"

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

	// Initialize components (will be created in BeginPlay)
	CockpitComponent = nullptr;
	EngineComponent = nullptr;
	FlightController = nullptr;
	SystemsManager = nullptr;
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
}

void ASpaceship::BeginPlay()
{
	Super::BeginPlay();

	// Create components if they don't exist
	if (!CockpitComponent)
	{
		CockpitComponent = CreateDefaultSubobject<UCockpitComponent>(TEXT("CockpitComponent"));
		CockpitComponent->SetupAttachment(ShipRoot);
	}

	if (!EngineComponent)
	{
		EngineComponent = CreateDefaultSubobject<UEngineComponent>(TEXT("EngineComponent"));
	}

	if (!FlightController)
	{
		FlightController = CreateDefaultSubobject<UFlightController>(TEXT("FlightController"));
	}

	if (!SystemsManager)
	{
		SystemsManager = CreateDefaultSubobject<UShipSystemsManager>(TEXT("SystemsManager"));
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

	UE_LOG(LogTemp, Log, TEXT("Spaceship '%s' initialized - VR: %s, Flight Mode: %s"), 
		*GetName(), 
		bIsVREnabled ? TEXT("Enabled") : TEXT("Disabled"),
		*UEnum::GetValueAsString(FlightMode));
}

void ASpaceship::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update flight physics
	UpdateFlightPhysics(DeltaTime);

	// Update ship systems
	UpdateShipSystems(DeltaTime);

	// Process input
	ProcessFlightInput(DeltaTime);

	// Apply flight mode behavior
	ApplyFlightModeBehavior(DeltaTime);

	// Update damage state
	UpdateDamageState();

	// Draw debug info
	if (bShowFlightDebug)
	{
		DrawDebugInfo();
	}

	// Performance tracking
	LastPhysicsUpdateTime = DeltaTime;
	PhysicsUpdateCount++;
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

void ASpaceship::TakeDamage(float DamageAmount, FVector HitLocation, FVector HitNormal)
{
	HullIntegrity = FMath::Max(0.0f, HullIntegrity - DamageAmount);
	
	// Apply damage to engine if hit
	if (EngineComponent)
	{
		EngineComponent->RepairEngine(-DamageAmount * 0.1f); // Negative repair = damage
	}

	// Apply damage to cockpit systems
	if (CockpitComponent)
	{
		CockpitComponent->ApplyDamageEffects(DamageAmount);
	}

	UE_LOG(LogTemp, Warning, TEXT("Spaceship '%s' took %.1f damage, hull integrity: %.1f%%"), 
		*GetName(), DamageAmount, HullIntegrity);
}

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