// Copyright Epic Games, Inc. All Rights Reserved.

#include "VRSpaceshipPawn.h"
#include "PlayerOriginManager.h"
#include "Spaceship.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "MotionControllerComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "HeadMountedDisplay.h"
#include "IHeadMountedDisplay.h"
#include "Kismet/GameplayStatics.h"

AVRSpaceshipPawn::AVRSpaceshipPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Enable replication
	bReplicates = true;
	SetReplicateMovement(false); // We handle movement through OriginManager

	// Setup VR component hierarchy
	SetupVRComponents();

	// Default values
	PossessedSpaceship = nullptr;
	bAutoPossessSpaceship = true;
	SpaceshipClass = ASpaceship::StaticClass();

	bEnableVRInput = true;
	InputSensitivity = 1.0f;

	bShowDebugInfo = false;
	bIsVRTrackingValid = true;
	LastTrackingValidationTime = 0.0f;

	LeftHandGrabbedActor = nullptr;
	RightHandGrabbedActor = nullptr;

	CurrentThrustInput = FVector::ZeroVector;
	CurrentRotationInput = FVector::ZeroVector;

	bLeftHandTracking = false;
	bRightHandTracking = false;
}

void AVRSpaceshipPawn::SetupVRComponents()
{
	// VR Origin - ALWAYS at (0,0,0)
	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	RootComponent = VROrigin;
	VROrigin->SetMobility(EComponentMobility::Movable);

	// VR Camera
	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->SetupAttachment(VROrigin);
	VRCamera->bLockToHmd = true; // Lock to HMD tracking

	// Left Hand Motion Controller
	LeftHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftHand"));
	LeftHand->SetupAttachment(VROrigin);
	LeftHand->MotionSource = FName("Left");
	// LeftHand->SetShowDeviceModel(true); // UE 5.6: SetShowDeviceModel removed

	// Right Hand Motion Controller
	RightHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightHand"));
	RightHand->SetupAttachment(VROrigin);
	RightHand->MotionSource = FName("Right");
	// RightHand->SetShowDeviceModel(true); // UE 5.6: SetShowDeviceModel removed

	// Player Origin Manager
	OriginManager = CreateDefaultSubobject<UPlayerOriginManager>(TEXT("OriginManager"));

	// Cockpit Mesh
	CockpitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CockpitMesh"));
	CockpitMesh->SetupAttachment(VROrigin);
	CockpitMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CockpitMesh->SetCastShadow(false);
}

void AVRSpaceshipPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate spaceship reference
	DOREPLIFETIME(AVRSpaceshipPawn, PossessedSpaceship);
}

void AVRSpaceshipPawn::BeginPlay()
{
	Super::BeginPlay();

	// Find or spawn spaceship if needed
	if (bAutoPossessSpaceship && !PossessedSpaceship)
	{
		FindOrSpawnSpaceship();
	}

	// Validate VR tracking
	ValidateVRTracking();

	if (bShowDebugInfo)
	{
		UE_LOG(LogTemp, Log, TEXT("VRSpaceshipPawn: Initialized - VR Enabled: %s"),
			IsVREnabled() ? TEXT("Yes") : TEXT("No"));
	}
}

void AVRSpaceshipPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update hand tracking
	UpdateHandTracking();

	// Update spaceship controls
	if (PossessedSpaceship && bEnableVRInput)
	{
		UpdateSpaceshipControls(DeltaTime);
	}

	// Validate VR tracking periodically
	LastTrackingValidationTime += DeltaTime;
	if (LastTrackingValidationTime > 1.0f)
	{
		ValidateVRTracking();
		LastTrackingValidationTime = 0.0f;
	}

	// Update VR origin transform (for universe recentering)
	UpdateVROriginTransform();

	// Debug visualization
	if (bShowDebugInfo)
	{
		DrawDebugVisualization();
	}
}

void AVRSpaceshipPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!PlayerInputComponent || !bEnableVRInput)
	{
		return;
	}

	// Bind movement axes
	PlayerInputComponent->BindAxis("MoveForward", this, &AVRSpaceshipPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVRSpaceshipPawn::MoveRight);
	PlayerInputComponent->BindAxis("MoveUp", this, &AVRSpaceshipPawn::MoveUp);

	// Bind rotation axes
	PlayerInputComponent->BindAxis("Pitch", this, &AVRSpaceshipPawn::PitchInput);
	PlayerInputComponent->BindAxis("Yaw", this, &AVRSpaceshipPawn::YawInput);
	PlayerInputComponent->BindAxis("Roll", this, &AVRSpaceshipPawn::RollInput);

	// Bind VR controller actions
	PlayerInputComponent->BindAction("LeftTrigger", IE_Pressed, this, &AVRSpaceshipPawn::OnLeftTriggerPressed);
	PlayerInputComponent->BindAction("LeftTrigger", IE_Released, this, &AVRSpaceshipPawn::OnLeftTriggerReleased);
	PlayerInputComponent->BindAction("RightTrigger", IE_Pressed, this, &AVRSpaceshipPawn::OnRightTriggerPressed);
	PlayerInputComponent->BindAction("RightTrigger", IE_Released, this, &AVRSpaceshipPawn::OnRightTriggerReleased);

	PlayerInputComponent->BindAction("LeftGrip", IE_Pressed, this, &AVRSpaceshipPawn::OnLeftGripPressed);
	PlayerInputComponent->BindAction("LeftGrip", IE_Released, this, &AVRSpaceshipPawn::OnLeftGripReleased);
	PlayerInputComponent->BindAction("RightGrip", IE_Pressed, this, &AVRSpaceshipPawn::OnRightGripPressed);
	PlayerInputComponent->BindAction("RightGrip", IE_Released, this, &AVRSpaceshipPawn::OnRightGripReleased);
}

void AVRSpaceshipPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Auto-possess spaceship when player controller possesses this pawn
	if (bAutoPossessSpaceship && !PossessedSpaceship)
	{
		FindOrSpawnSpaceship();
	}
}

void AVRSpaceshipPawn::UnPossessed()
{
	Super::UnPossessed();

	// Optionally release spaceship when unpossessed
	// UnpossessSpaceship();
}

// ============================================================================
// SPACESHIP INTEGRATION
// ============================================================================

void AVRSpaceshipPawn::PossessSpaceship(ASpaceship* Spaceship)
{
	if (!Spaceship)
	{
		return;
	}

	// Release previous spaceship
	if (PossessedSpaceship)
	{
		UnpossessSpaceship();
	}

	// Set new spaceship
	PossessedSpaceship = Spaceship;

	// Position VR origin at spaceship cockpit
	if (VROrigin)
	{
		FTransform CockpitTransform = Spaceship->GetCockpitTransform();
		SetActorLocation(CockpitTransform.GetLocation(), false, nullptr, ETeleportType::TeleportPhysics);
		SetActorRotation(CockpitTransform.GetRotation().Rotator(), ETeleportType::TeleportPhysics);
	}

	if (bShowDebugInfo)
	{
		UE_LOG(LogTemp, Log, TEXT("VRSpaceshipPawn: Possessed spaceship %s"), *Spaceship->GetName());
	}
}

void AVRSpaceshipPawn::UnpossessSpaceship()
{
	if (PossessedSpaceship)
	{
		if (bShowDebugInfo)
		{
			UE_LOG(LogTemp, Log, TEXT("VRSpaceshipPawn: Released spaceship %s"), *PossessedSpaceship->GetName());
		}

		PossessedSpaceship = nullptr;
	}
}

// ============================================================================
// VR INTERACTION
// ============================================================================

void AVRSpaceshipPawn::GrabWithLeftHand(AActor* Actor)
{
	if (!Actor || !LeftHand)
	{
		return;
	}

	// Release previous grab
	ReleaseLeftHand();

	// Grab new actor
	LeftHandGrabbedActor = Actor;

	// TODO: Attach actor to hand or implement physics grab
	// This is a placeholder for more sophisticated grab mechanics

	if (bShowDebugInfo)
	{
		UE_LOG(LogTemp, Log, TEXT("VRSpaceshipPawn: Left hand grabbed %s"), *Actor->GetName());
	}
}

void AVRSpaceshipPawn::ReleaseLeftHand()
{
	if (LeftHandGrabbedActor)
	{
		if (bShowDebugInfo)
		{
			UE_LOG(LogTemp, Log, TEXT("VRSpaceshipPawn: Left hand released %s"), *LeftHandGrabbedActor->GetName());
		}

		LeftHandGrabbedActor = nullptr;
	}
}

void AVRSpaceshipPawn::GrabWithRightHand(AActor* Actor)
{
	if (!Actor || !RightHand)
	{
		return;
	}

	// Release previous grab
	ReleaseRightHand();

	// Grab new actor
	RightHandGrabbedActor = Actor;

	// TODO: Attach actor to hand or implement physics grab

	if (bShowDebugInfo)
	{
		UE_LOG(LogTemp, Log, TEXT("VRSpaceshipPawn: Right hand grabbed %s"), *Actor->GetName());
	}
}

void AVRSpaceshipPawn::ReleaseRightHand()
{
	if (RightHandGrabbedActor)
	{
		if (bShowDebugInfo)
		{
			UE_LOG(LogTemp, Log, TEXT("VRSpaceshipPawn: Right hand released %s"), *RightHandGrabbedActor->GetName());
		}

		RightHandGrabbedActor = nullptr;
	}
}

// ============================================================================
// VR TRACKING & VALIDATION
// ============================================================================

void AVRSpaceshipPawn::ValidateVRTracking()
{
	// Check if VR is enabled
	bIsVRTrackingValid = IsVREnabled();

	if (bIsVRTrackingValid && VROrigin)
	{
		// Ensure VR origin is at (0,0,0) locally
		FVector LocalPosition = VROrigin->GetRelativeLocation();
		if (!LocalPosition.IsNearlyZero(1.0f))
		{
			UE_LOG(LogTemp, Warning, TEXT("VRSpaceshipPawn: VR Origin not at (0,0,0)! Resetting..."));
			VROrigin->SetRelativeLocation(FVector::ZeroVector);
		}
	}

	// Check hand tracking
	if (LeftHand)
	{
		bLeftHandTracking = LeftHand->IsTracked();
	}

	if (RightHand)
	{
		bRightHandTracking = RightHand->IsTracked();
	}
}

FVector AVRSpaceshipPawn::GetHMDPosition() const
{
	if (VRCamera)
	{
		return VRCamera->GetRelativeLocation();
	}
	return FVector::ZeroVector;
}

FRotator AVRSpaceshipPawn::GetHMDRotation() const
{
	if (VRCamera)
	{
		return VRCamera->GetRelativeRotation();
	}
	return FRotator::ZeroRotator;
}

void AVRSpaceshipPawn::ResetVROrigin()
{
	// TODO: Implement VR origin reset using UE 5.6 VR API
	// The HeadMountedDisplayFunctionLibrary was moved/changed in UE 5.6

	if (bShowDebugInfo)
	{
		UE_LOG(LogTemp, Log, TEXT("VRSpaceshipPawn: VR origin reset"));
	}
}

// ============================================================================
// INPUT HANDLING
// ============================================================================

void AVRSpaceshipPawn::MoveForward(float Value)
{
	CurrentThrustInput.X = Value * InputSensitivity;
}

void AVRSpaceshipPawn::MoveRight(float Value)
{
	CurrentThrustInput.Y = Value * InputSensitivity;
}

void AVRSpaceshipPawn::MoveUp(float Value)
{
	CurrentThrustInput.Z = Value * InputSensitivity;
}

void AVRSpaceshipPawn::PitchInput(float Value)
{
	CurrentRotationInput.Y = Value * InputSensitivity;
}

void AVRSpaceshipPawn::YawInput(float Value)
{
	CurrentRotationInput.Z = Value * InputSensitivity;
}

void AVRSpaceshipPawn::RollInput(float Value)
{
	CurrentRotationInput.X = Value * InputSensitivity;
}

void AVRSpaceshipPawn::OnLeftTriggerPressed()
{
	// TODO: Implement left trigger action
	if (bShowDebugInfo)
	{
		UE_LOG(LogTemp, Log, TEXT("VRSpaceshipPawn: Left trigger pressed"));
	}
}

void AVRSpaceshipPawn::OnLeftTriggerReleased()
{
	// TODO: Implement left trigger release
}

void AVRSpaceshipPawn::OnRightTriggerPressed()
{
	// TODO: Implement right trigger action
	if (bShowDebugInfo)
	{
		UE_LOG(LogTemp, Log, TEXT("VRSpaceshipPawn: Right trigger pressed"));
	}
}

void AVRSpaceshipPawn::OnRightTriggerReleased()
{
	// TODO: Implement right trigger release
}

void AVRSpaceshipPawn::OnLeftGripPressed()
{
	// TODO: Implement left grip action
	if (bShowDebugInfo)
	{
		UE_LOG(LogTemp, Log, TEXT("VRSpaceshipPawn: Left grip pressed"));
	}
}

void AVRSpaceshipPawn::OnLeftGripReleased()
{
	ReleaseLeftHand();
}

void AVRSpaceshipPawn::OnRightGripPressed()
{
	// TODO: Implement right grip action
	if (bShowDebugInfo)
	{
		UE_LOG(LogTemp, Log, TEXT("VRSpaceshipPawn: Right grip pressed"));
	}
}

void AVRSpaceshipPawn::OnRightGripReleased()
{
	ReleaseRightHand();
}

// ============================================================================
// UPDATE FUNCTIONS
// ============================================================================

void AVRSpaceshipPawn::UpdateHandTracking()
{
	// Hand tracking is automatically updated by motion controllers
	// This function can be extended for custom hand visualization
}

void AVRSpaceshipPawn::UpdateSpaceshipControls(float DeltaTime)
{
	if (!PossessedSpaceship)
	{
		return;
	}

	// Apply thrust input to spaceship
	if (!CurrentThrustInput.IsNearlyZero())
	{
		PossessedSpaceship->ApplyThrust(CurrentThrustInput);
	}

	// Apply rotation input to spaceship
	if (!CurrentRotationInput.IsNearlyZero())
	{
		PossessedSpaceship->ApplyRotation(CurrentRotationInput);
	}

	// Sync VR pawn position with spaceship
	// (The spaceship is the "real" physics object, VR pawn is just the view)
	if (OriginManager)
	{
		// Movement is handled through OriginManager for precision
		// Spaceship physics is applied, then we update VR pawn to match
	}
}

void AVRSpaceshipPawn::UpdateVROriginTransform()
{
	// Update VR pawn position to match spaceship
	if (PossessedSpaceship)
	{
		FTransform CockpitTransform = PossessedSpaceship->GetCockpitTransform();
		SetActorLocation(CockpitTransform.GetLocation(), false, nullptr, ETeleportType::None);
		SetActorRotation(CockpitTransform.GetRotation().Rotator(), ETeleportType::None);
	}
}

// ============================================================================
// DEBUG & UTILITIES
// ============================================================================

void AVRSpaceshipPawn::DrawDebugVisualization()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector PawnLocation = GetActorLocation();

	// Draw VR origin
	DrawDebugSphere(World, PawnLocation, 50.0f, 8, FColor::Green, false, -1.0f, 0, 2.0f);

	// Draw HMD position
	if (VRCamera)
	{
		FVector HMDWorldPos = VRCamera->GetComponentLocation();
		DrawDebugSphere(World, HMDWorldPos, 20.0f, 8, FColor::Blue, false, -1.0f, 0, 2.0f);
		DrawDebugLine(World, PawnLocation, HMDWorldPos, FColor::Blue, false, -1.0f, 0, 2.0f);
	}

	// Draw hand positions
	if (LeftHand && bLeftHandTracking)
	{
		FVector LeftHandPos = LeftHand->GetComponentLocation();
		DrawDebugSphere(World, LeftHandPos, 15.0f, 8, FColor::Red, false, -1.0f, 0, 2.0f);
	}

	if (RightHand && bRightHandTracking)
	{
		FVector RightHandPos = RightHand->GetComponentLocation();
		DrawDebugSphere(World, RightHandPos, 15.0f, 8, FColor::Yellow, false, -1.0f, 0, 2.0f);
	}

	// Draw debug text
	FString DebugText = GetVRStatusInfo();
	DrawDebugString(World, PawnLocation + FVector(0, 0, 150.0f), DebugText, nullptr, FColor::White, 0.0f, true);
}

FString AVRSpaceshipPawn::GetVRStatusInfo() const
{
	return FString::Printf(
		TEXT("VRSpaceshipPawn\nVR Tracking: %s\nLeft Hand: %s\nRight Hand: %s\nSpaceship: %s\nThrust: (%.2f, %.2f, %.2f)"),
		bIsVRTrackingValid ? TEXT("Valid") : TEXT("Invalid"),
		bLeftHandTracking ? TEXT("Tracked") : TEXT("Lost"),
		bRightHandTracking ? TEXT("Tracked") : TEXT("Lost"),
		PossessedSpaceship ? *PossessedSpaceship->GetName() : TEXT("None"),
		CurrentThrustInput.X, CurrentThrustInput.Y, CurrentThrustInput.Z
	);
}

// ============================================================================
// INTERNAL FUNCTIONS
// ============================================================================

void AVRSpaceshipPawn::FindOrSpawnSpaceship()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Try to find existing spaceship
	TArray<AActor*> FoundSpaceships;
	UGameplayStatics::GetAllActorsOfClass(World, ASpaceship::StaticClass(), FoundSpaceships);

	if (FoundSpaceships.Num() > 0)
	{
		// Possess first found spaceship
		ASpaceship* Spaceship = Cast<ASpaceship>(FoundSpaceships[0]);
		if (Spaceship)
		{
			PossessSpaceship(Spaceship);
			return;
		}
	}

	// No spaceship found, spawn one if we have authority
	if (HasAuthority() && SpaceshipClass)
	{
		FVector SpawnLocation = GetActorLocation();
		FRotator SpawnRotation = GetActorRotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		ASpaceship* NewSpaceship = World->SpawnActor<ASpaceship>(SpaceshipClass, SpawnLocation, SpawnRotation, SpawnParams);
		if (NewSpaceship)
		{
			PossessSpaceship(NewSpaceship);
		}
	}
}

bool AVRSpaceshipPawn::IsVREnabled() const
{
	// TODO: Implement VR check using UE 5.6 VR API
	// The HeadMountedDisplayFunctionLibrary was moved/changed in UE 5.6
	return false; // Stub for now
}
