// Copyright Epic Games, Inc. All Rights Reserved.

#include "AdvancedLocomotionSystem.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/TimelineComponent.h"         // For UTimelineComponent
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Vector.h"
#include "Math/Rotator.h"
#include "Math/Transform.h"
#include "PhysicsEngine/BodyInstance.h"

AAdvancedLocomotionSystem::AAdvancedLocomotionSystem()
{
	// Create camera components
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom);
	Camera->bUsePawnControlRotation = false;

	// Create timeline components
	MantleTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("MantleTimeline"));
	SlideTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("SlideTimeline"));

	// Initialize movement state
	CurrentMovementState = EMovementState::Idle;
	CurrentStance = EStance::Standing;
	CurrentRotationMode = ERotationMode::VelocityDirection;
	CurrentGait = EGait::Walking;

	// Initialize movement properties
	WalkSpeed = 165.0f;
	RunSpeed = 375.0f;
	SprintSpeed = 650.0f;
	CrouchSpeed = 150.0f;
	SlideSpeed = 800.0f;
	JumpHeight = 220.0f;
	AirControl = 0.05f;
	GroundFriction = 8.0f;
	GravityScale = 1.0f;

	// Initialize advanced movement properties
	WallRunSpeed = 600.0f;
	WallRunDuration = 2.0f;
	ClimbSpeed = 200.0f;
	MantleHeight = 120.0f;
	MantleDuration = 0.5f;

	// Initialize camera properties
	BaseFOV = 90.0f;
	AimFOV = 60.0f;
	AimFOVInterpSpeed = 10.0f;
	CameraDistance = 400.0f;
	bEnableCameraShake = true;

	// Initialize VR properties
	bVREnabled = false;
	VRMovementType = EVRMovementType::Teleport;

	// Initialize space properties
	bZeroGMovementEnabled = false;
	JetpackThrust = 1000.0f;
	JetpackFuel = 100.0f;
	JetpackFuelConsumption = 10.0f;
	bJetpackEnabled = false;

	// Initialize internal state
	bIsMoving = false;
	bIsSprinting = false;
	bIsCrouching = false;
	bIsAiming = false;
	bIsSliding = false;
	bIsWallRunning = false;
	bIsClimbing = false;
	bIsMantling = false;
	bIsJumping = false;
	bIsFalling = false;
	CurrentThrust = 0.0f;
	CurrentRotationThrust = FRotator::ZeroRotator;
	CurrentJetpackFuel = JetpackFuel;
	LastMovementInput = FVector::ZeroVector;
	LastVelocity = FVector::ZeroVector;
	LastMovementTime = 0.0f;

	// Configure character movement
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
		GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
		GetCharacterMovement()->JumpZVelocity = FMath::Sqrt(2.0f * GetWorld()->GetGravityZ() * JumpHeight);
		GetCharacterMovement()->AirControl = AirControl;
		GetCharacterMovement()->GroundFriction = GroundFriction;
		GetCharacterMovement()->GravityScale = GravityScale;
	}

	// Configure timeline callbacks
	if (MantleTimeline)
	{
		MantleTimeline->SetLooping(false);
		MantleTimeline->SetTimelineLength(MantleDuration);
	}

	if (SlideTimeline)
	{
		SlideTimeline->SetLooping(false);
		SlideTimeline->SetTimelineLength(1.0f);
	}

	// Set default movement speed
	ApplyMovementSettings();
}

void AAdvancedLocomotionSystem::BeginPlay()
{
	Super::BeginPlay();

	// Setup timeline callbacks
	if (MantleTimeline)
	{
		// Note: UCurveFloat would need to be provided for timeline interpolation
		// For now, we create delegates without actual curves
		FOnTimelineFloat MantleTimelineUpdate;
		MantleTimelineUpdate.BindUFunction(this, TEXT("UpdateMantleTimeline"));
		// AddInterpFloat requires a UCurveFloat* and the delegate
		// Since we don't have curves set up, this would need to be configured in Blueprints or with actual curve assets
		// MantleTimeline->AddInterpFloat(nullptr, MantleTimelineUpdate);

		FOnTimelineEvent MantleTimelineFinished;
		MantleTimelineFinished.BindUFunction(this, TEXT("OnMantleTimelineFinished"));
		MantleTimeline->SetTimelineFinishedFunc(MantleTimelineFinished);
	}

	if (SlideTimeline)
	{
		FOnTimelineFloat SlideTimelineUpdate;
		SlideTimelineUpdate.BindUFunction(this, TEXT("UpdateSlideTimeline"));
		// AddInterpFloat requires a UCurveFloat* and the delegate
		// SlideTimeline->AddInterpFloat(nullptr, SlideTimelineUpdate);

		FOnTimelineEvent SlideTimelineFinished;
		SlideTimelineFinished.BindUFunction(this, TEXT("OnSlideTimelineFinished"));
		SlideTimeline->SetTimelineFinishedFunc(SlideTimelineFinished);
	}

	// Initialize camera
	CameraBoom->TargetArmLength = CameraDistance;
	Camera->FieldOfView = BaseFOV;

	UE_LOG(LogTemp, Warning, TEXT("Advanced Locomotion System initialized"));
}

void AAdvancedLocomotionSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update all systems
	UpdateMovementState(DeltaTime);
	UpdateStance();
	UpdateGait();
	UpdateRotation(DeltaTime);
	UpdateCamera(DeltaTime);
	UpdateAdvancedMovement(DeltaTime);
	UpdateSpaceMovement(DeltaTime);
	UpdateVRMovement(DeltaTime);

	// Handle input
	HandleMovementInput(DeltaTime);
	HandleRotationInput(DeltaTime);

	// Update last values
	LastVelocity = GetVelocity();
	LastMovementTime = GetWorld()->GetTimeSeconds();
}

void AAdvancedLocomotionSystem::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Movement input
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AAdvancedLocomotionSystem::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AAdvancedLocomotionSystem::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("MoveUp"), this, &AAdvancedLocomotionSystem::MoveUp);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AAdvancedLocomotionSystem::LookUp);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AAdvancedLocomotionSystem::Turn);

	// Action input
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &AAdvancedLocomotionSystem::JumpAction);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Pressed, this, &AAdvancedLocomotionSystem::SprintStart);
	PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Released, this, &AAdvancedLocomotionSystem::SprintEnd);
	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &AAdvancedLocomotionSystem::CrouchStart);
	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Released, this, &AAdvancedLocomotionSystem::CrouchEnd);
	PlayerInputComponent->BindAction(TEXT("Slide"), IE_Pressed, this, &AAdvancedLocomotionSystem::SlideStart);
	PlayerInputComponent->BindAction(TEXT("Aim"), IE_Pressed, this, &AAdvancedLocomotionSystem::AimStart);
	PlayerInputComponent->BindAction(TEXT("Aim"), IE_Released, this, &AAdvancedLocomotionSystem::AimEnd);
	PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &AAdvancedLocomotionSystem::Interact);
	PlayerInputComponent->BindAction(TEXT("Vault"), IE_Pressed, this, &AAdvancedLocomotionSystem::Vault);
	PlayerInputComponent->BindAction(TEXT("Mantle"), IE_Pressed, this, &AAdvancedLocomotionSystem::Mantle);
}

void AAdvancedLocomotionSystem::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// Handle landing
	if (bIsFalling)
	{
		SetMovementState(EMovementState::Landing);
		
		// Play landing animation
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			// This would play landing montage
		}

		// Apply landing camera shake
		if (bEnableCameraShake)
		{
			PlayCameraShake(nullptr, 1.0f);
		}

		// Reset falling state
		bIsFalling = false;
		bIsJumping = false;

		// Return to appropriate movement state
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
		{
			if (GetVelocity().Size() > 0.0f)
			{
				SetMovementState(EMovementState::Walking);
			}
			else
			{
				SetMovementState(EMovementState::Idle);
			}
		}, 0.2f, false);
	}
}

void AAdvancedLocomotionSystem::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	// Handle movement mode changes
	switch (GetCharacterMovement()->MovementMode)
	{
		case MOVE_Walking:
			if (GetVelocity().Size() > 0.0f)
			{
				SetMovementState(EMovementState::Walking);
			}
			else
			{
				SetMovementState(EMovementState::Idle);
			}
			break;

		case MOVE_Falling:
			SetMovementState(EMovementState::Falling);
			bIsFalling = true;
			break;

		case MOVE_Swimming:
			SetMovementState(EMovementState::Swimming);
			break;

		case MOVE_Flying:
			SetMovementState(EMovementState::Flying);
			break;

		case MOVE_Custom:
			// Handle custom movement modes
			break;

		default:
			break;
	}
}

void AAdvancedLocomotionSystem::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// Add to movement input
		LastMovementInput.X = Value;
		
		// Apply movement
		const FRotator Rotation = GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AAdvancedLocomotionSystem::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// Add to movement input
		LastMovementInput.Y = Value;
		
		// Apply movement
		const FRotator Rotation = GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void AAdvancedLocomotionSystem::MoveUp(float Value)
{
	if (Value != 0.0f)
	{
		// Add to movement input
		LastMovementInput.Z = Value;
		
		// Apply movement (for swimming/flying)
		AddMovementInput(FVector::UpVector, Value);
	}
}

void AAdvancedLocomotionSystem::LookUp(float Value)
{
	if (Value != 0.0f)
	{
		// Apply camera rotation
		AddControllerPitchInput(Value);
	}
}

void AAdvancedLocomotionSystem::Turn(float Value)
{
	if (Value != 0.0f)
	{
		// Apply camera rotation
		AddControllerYawInput(Value);
	}
}

void AAdvancedLocomotionSystem::JumpAction()
{
	if (CanJump())
	{
		Jump();
		bIsJumping = true;
		SetMovementState(EMovementState::Jumping);
	}
}

void AAdvancedLocomotionSystem::SprintStart()
{
	if (!bIsSprinting && !bIsCrouching && !bIsSliding)
	{
		bIsSprinting = true;
		SetGait(EGait::Sprinting);
		ApplyMovementSettings();
	}
}

void AAdvancedLocomotionSystem::SprintEnd()
{
	if (bIsSprinting)
	{
		bIsSprinting = false;
		SetGait(EGait::Running);
		ApplyMovementSettings();
	}
}

void AAdvancedLocomotionSystem::CrouchStart()
{
	if (!bIsCrouching && !bIsSliding)
	{
		bIsCrouching = true;
		SetStance(EStance::Crouching);
		ApplyMovementSettings();
	}
}

void AAdvancedLocomotionSystem::CrouchEnd()
{
	if (bIsCrouching)
	{
		bIsCrouching = false;
		SetStance(EStance::Standing);
		ApplyMovementSettings();
	}
}

void AAdvancedLocomotionSystem::SlideStart()
{
	if (!bIsSliding && bIsSprinting && !bIsCrouching)
	{
		bIsSliding = true;
		SetStance(EStance::Sliding);
		SetMovementState(EMovementState::Sliding);
		ApplyMovementSettings();

		// Start slide timeline
		if (SlideTimeline)
		{
			SlideTimeline->PlayFromStart();
		}
	}
}

void AAdvancedLocomotionSystem::AimStart()
{
	if (!bIsAiming)
	{
		bIsAiming = true;
		SetRotationMode(ERotationMode::Aiming);
		ApplyMovementSettings();
	}
}

void AAdvancedLocomotionSystem::AimEnd()
{
	if (bIsAiming)
	{
		bIsAiming = false;
		SetRotationMode(ERotationMode::VelocityDirection);
		ApplyMovementSettings();
	}
}

void AAdvancedLocomotionSystem::Interact()
{
	// Perform interaction trace
	PerformLineTrace();
}

void AAdvancedLocomotionSystem::Vault()
{
	// Check for vault opportunity
	if (CheckForMantle())
	{
		StartMantle();
	}
}

void AAdvancedLocomotionSystem::Mantle()
{
	// Check for mantle opportunity
	if (CheckForMantle())
	{
		StartMantle();
	}
}

EMovementState AAdvancedLocomotionSystem::GetMovementState() const
{
	return CurrentMovementState;
}

EStance AAdvancedLocomotionSystem::GetStance() const
{
	return CurrentStance;
}

ERotationMode AAdvancedLocomotionSystem::GetRotationMode() const
{
	return CurrentRotationMode;
}

EGait AAdvancedLocomotionSystem::GetGait() const
{
	return CurrentGait;
}

void AAdvancedLocomotionSystem::SetMovementState(EMovementState NewState)
{
	if (CurrentMovementState != NewState)
	{
		EMovementState OldState = CurrentMovementState;
		CurrentMovementState = NewState;
		
		UE_LOG(LogTemp, Warning, TEXT("Movement state changed: %s -> %s"), 
			*UEnum::GetValueAsString(OldState), *UEnum::GetValueAsString(NewState));
	}
}

void AAdvancedLocomotionSystem::SetStance(EStance NewStance)
{
	if (CurrentStance != NewStance)
	{
		CurrentStance = NewStance;
		ApplyMovementSettings();
	}
}

void AAdvancedLocomotionSystem::SetRotationMode(ERotationMode NewMode)
{
	if (CurrentRotationMode != NewMode)
	{
		CurrentRotationMode = NewMode;
	}
}

void AAdvancedLocomotionSystem::SetGait(EGait NewGait)
{
	if (CurrentGait != NewGait)
	{
		CurrentGait = NewGait;
		ApplyMovementSettings();
	}
}

float AAdvancedLocomotionSystem::GetMovementSpeed() const
{
	return GetVelocity().Size();
}

FVector AAdvancedLocomotionSystem::GetMovementDirection() const
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;
	return Velocity.GetSafeNormal();
}

float AAdvancedLocomotionSystem::GetMovementInputAmount() const
{
	return LastMovementInput.Size();
}

bool AAdvancedLocomotionSystem::IsMoving() const
{
	return bIsMoving;
}

bool AAdvancedLocomotionSystem::IsInAir() const
{
	return bIsFalling || bIsJumping;
}

bool AAdvancedLocomotionSystem::IsSprinting() const
{
	return bIsSprinting;
}

bool AAdvancedLocomotionSystem::IsCrouching() const
{
	return bIsCrouching;
}

bool AAdvancedLocomotionSystem::IsAiming() const
{
	return bIsAiming;
}

bool AAdvancedLocomotionSystem::IsSliding() const
{
	return bIsSliding;
}

void AAdvancedLocomotionSystem::StartWallRun()
{
	if (CanWallRun())
	{
		bIsWallRunning = true;
		SetMovementState(EMovementState::WallRunning);
		ApplyMovementSettings();

		// Apply wall run physics
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Custom);
		}
	}
}

void AAdvancedLocomotionSystem::EndWallRun()
{
	if (bIsWallRunning)
	{
		bIsWallRunning = false;
		SetMovementState(EMovementState::Falling);
		
		// Restore normal movement
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		}
	}
}

bool AAdvancedLocomotionSystem::CanWallRun() const
{
	return !bIsWallRunning && bIsSprinting && !bIsCrouching && !bIsSliding && IsInAir();
}

void AAdvancedLocomotionSystem::StartClimb()
{
	if (CanClimb())
	{
		bIsClimbing = true;
		SetMovementState(EMovementState::Climbing);
		ApplyMovementSettings();

		// Apply climbing physics
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Custom);
		}
	}
}

void AAdvancedLocomotionSystem::EndClimb()
{
	if (bIsClimbing)
	{
		bIsClimbing = false;
		SetMovementState(EMovementState::Falling);
		
		// Restore normal movement
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		}
	}
}

bool AAdvancedLocomotionSystem::CanClimb() const
{
	return !bIsClimbing && !bIsWallRunning && !bIsSliding;
}

void AAdvancedLocomotionSystem::StartMantle()
{
	if (CanMantle())
	{
		bIsMantling = true;
		SetMovementState(EMovementState::Mantling);
		
		// Start mantle timeline
		if (MantleTimeline)
		{
			MantleTimeline->PlayFromStart();
		}

		// Disable movement during mantle
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->SetMovementMode(MOVE_None);
		}
	}
}

void AAdvancedLocomotionSystem::EndMantle()
{
	if (bIsMantling)
	{
		bIsMantling = false;
		
		// Restore movement
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}

		// Return to appropriate state
		if (GetVelocity().Size() > 0.0f)
		{
			SetMovementState(EMovementState::Walking);
		}
		else
		{
			SetMovementState(EMovementState::Idle);
		}
	}
}

bool AAdvancedLocomotionSystem::CanMantle() const
{
	return !bIsMantling && !bIsWallRunning && !bIsSliding && !bIsClimbing;
}

void AAdvancedLocomotionSystem::SetGroundFriction(float Friction)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->GroundFriction = Friction;
	}
}

void AAdvancedLocomotionSystem::SetAirControl(float Control)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->AirControl = Control;
	}
}

void AAdvancedLocomotionSystem::SetGravityScale(float Scale)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->GravityScale = Scale;
	}
}

void AAdvancedLocomotionSystem::ApplyForce(const FVector& Force)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->AddForce(Force);
	}
}

void AAdvancedLocomotionSystem::ApplyImpulse(const FVector& Impulse)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->AddImpulse(Impulse);
	}
}

void AAdvancedLocomotionSystem::SetCameraFOV(float FOV)
{
	if (Camera)
	{
		Camera->FieldOfView = FMath::Clamp(FOV, 60.0f, 120.0f);
	}
}

float AAdvancedLocomotionSystem::GetCameraFOV() const
{
	return Camera ? Camera->FieldOfView : BaseFOV;
}

void AAdvancedLocomotionSystem::SetCameraDistance(float Distance)
{
	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = FMath::Clamp(Distance, 100.0f, 1000.0f);
	}
}

float AAdvancedLocomotionSystem::GetCameraDistance() const
{
	return CameraBoom ? CameraBoom->TargetArmLength : CameraDistance;
}

void AAdvancedLocomotionSystem::EnableCameraShake(bool bEnabled)
{
	bEnableCameraShake = bEnabled;
}

void AAdvancedLocomotionSystem::PlayCameraShake(TSubclassOf<class UCameraShakeBase> ShakeClass, float Scale)
{
	if (bEnableCameraShake && GetController())
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		if (PC && ShakeClass)
		{
			PC->ClientStartCameraShake(ShakeClass, Scale);
		}
	}
}

void AAdvancedLocomotionSystem::PlayMontage(UAnimMontage* Montage, float PlayRate)
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (Montage && !AnimInstance->Montage_IsPlaying(Montage))
		{
			AnimInstance->Montage_Play(Montage, PlayRate);
		}
	}
}

void AAdvancedLocomotionSystem::StopMontage(UAnimMontage* Montage)
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (Montage && AnimInstance->Montage_IsPlaying(Montage))
		{
			AnimInstance->Montage_Stop(0.25f, Montage);
		}
	}
}

bool AAdvancedLocomotionSystem::IsMontagePlaying(UAnimMontage* Montage) const
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		return AnimInstance->Montage_IsPlaying(Montage);
	}
	return false;
}

void AAdvancedLocomotionSystem::SetAnimationInstance(UAnimInstance* AnimInstance)
{
	if (GetMesh() && AnimInstance)
	{
		// SetAnimInstance doesn't exist in UE5.6
		// Use SetAnimationMode to switch to use AnimBlueprint or AnimInstance
		// To set a specific AnimInstance class, use SetAnimClass instead
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		// Note: To actually set a specific AnimInstance, you would need to use:
		// GetMesh()->SetAnimClass(AnimInstance->GetClass());
	}
}

UAnimInstance* AAdvancedLocomotionSystem::GetAnimationInstance() const
{
	return GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
}

void AAdvancedLocomotionSystem::EnableVRSupport(bool bEnabled)
{
	bVREnabled = bEnabled;
	
	if (bVREnabled)
	{
		// Enable VR-specific features
		UE_LOG(LogTemp, Warning, TEXT("VR support enabled"));
	}
	else
	{
		// Disable VR-specific features
		UE_LOG(LogTemp, Warning, TEXT("VR support disabled"));
	}
}

bool AAdvancedLocomotionSystem::IsVREnabled() const
{
	return bVREnabled;
}

void AAdvancedLocomotionSystem::SetVRMovementType(EVRMovementType MovementType)
{
	VRMovementType = MovementType;
}

void AAdvancedLocomotionSystem::UpdateVRMotionControllers()
{
	if (bVREnabled)
	{
		// Update VR motion controllers
		// This would interface with VR system
	}
}

void AAdvancedLocomotionSystem::EnableZeroGMovement(bool bEnabled)
{
	bZeroGMovementEnabled = bEnabled;
	
	if (bZeroGMovementEnabled)
	{
		// Enable zero-G movement
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->GravityScale = 0.0f;
		}
		SetMovementState(EMovementState::ZeroG);
	}
	else
	{
		// Disable zero-G movement
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->GravityScale = GravityScale;
		}
		SetMovementState(EMovementState::Walking);
	}
}

bool AAdvancedLocomotionSystem::IsZeroGMovementEnabled() const
{
	return bZeroGMovementEnabled;
}

void AAdvancedLocomotionSystem::SetThrust(float ThrustAmount)
{
	CurrentThrust = FMath::Clamp(ThrustAmount, -1.0f, 1.0f);
}

void AAdvancedLocomotionSystem::SetRotationThrust(const FRotator& Rotation)
{
	CurrentRotationThrust = Rotation;
}

void AAdvancedLocomotionSystem::EnableJetpack(bool bEnabled)
{
	bJetpackEnabled = bEnabled;
	
	if (bEnabled)
	{
		CurrentJetpackFuel = JetpackFuel;
	}
}

bool AAdvancedLocomotionSystem::IsJetpackEnabled() const
{
	return bJetpackEnabled;
}

void AAdvancedLocomotionSystem::UpdateMovementState(float DeltaTime)
{
	// Update movement state based on current conditions
	if (bIsMantling)
	{
		SetMovementState(EMovementState::Mantling);
	}
	else if (bIsWallRunning)
	{
		SetMovementState(EMovementState::WallRunning);
	}
	else if (bIsClimbing)
	{
		SetMovementState(EMovementState::Climbing);
	}
	else if (bIsSliding)
	{
		SetMovementState(EMovementState::Sliding);
	}
	else if (bIsJumping)
	{
		SetMovementState(EMovementState::Jumping);
	}
	else if (bIsFalling)
	{
		SetMovementState(EMovementState::Falling);
	}
	else if (bZeroGMovementEnabled)
	{
		SetMovementState(EMovementState::ZeroG);
	}
	else if (GetVelocity().Size() > 0.0f)
	{
		SetMovementState(EMovementState::Walking);
	}
	else
	{
		SetMovementState(EMovementState::Idle);
	}

	// Update moving state
	bIsMoving = GetVelocity().Size() > 1.0f;
}

void AAdvancedLocomotionSystem::UpdateStance()
{
	// Update stance based on current conditions
	if (bIsSliding)
	{
		SetStance(EStance::Sliding);
	}
	else if (bIsCrouching)
	{
		SetStance(EStance::Crouching);
	}
	else
	{
		SetStance(EStance::Standing);
	}
}

void AAdvancedLocomotionSystem::UpdateGait()
{
	// Update gait based on movement speed and conditions
	if (bIsSprinting && !bIsCrouching && !bIsSliding)
	{
		SetGait(EGait::Sprinting);
	}
	else if (bIsMoving && !bIsCrouching)
	{
		SetGait(EGait::Running);
	}
	else
	{
		SetGait(EGait::Walking);
	}
}

void AAdvancedLocomotionSystem::UpdateRotation(float DeltaTime)
{
	// Update rotation based on rotation mode
	switch (CurrentRotationMode)
	{
		case ERotationMode::VelocityDirection:
			if (bIsMoving && !bIsAiming)
			{
				FRotator DesiredRotation = GetMovementDirection().Rotation();
				SmoothRotation(DeltaTime);
			}
			break;

		case ERotationMode::LookingDirection:
			// Character faces camera direction
			break;

		case ERotationMode::Aiming:
			// Character aims towards camera direction
			break;
	}
}

void AAdvancedLocomotionSystem::UpdateCamera(float DeltaTime)
{
	// Update camera FOV based on aiming
	float TargetFOV = bIsAiming ? AimFOV : BaseFOV;
	float CurrentFOV = GetCameraFOV();
	float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, AimFOVInterpSpeed);
	SetCameraFOV(NewFOV);

	// Update camera distance based on stance
	float TargetDistance = CameraDistance;
	if (bIsCrouching)
	{
		TargetDistance *= 0.8f;
	}
	else if (bIsSliding)
	{
		TargetDistance *= 0.6f;
	}

	float CurrentDistance = GetCameraDistance();
	float NewDistance = FMath::FInterpTo(CurrentDistance, TargetDistance, DeltaTime, 5.0f);
	SetCameraDistance(NewDistance);
}

void AAdvancedLocomotionSystem::UpdateAdvancedMovement(float DeltaTime)
{
	// Update wall running
	if (bIsWallRunning)
	{
		// Apply wall run physics
		FVector WallRunDirection = GetMovementDirection();
		ApplyForce(WallRunDirection * WallRunSpeed * 10.0f);

		// Check wall run duration
		static float WallRunTimer = 0.0f;
		WallRunTimer += DeltaTime;
		if (WallRunTimer >= WallRunDuration)
		{
			EndWallRun();
			WallRunTimer = 0.0f;
		}
	}

	// Update climbing
	if (bIsClimbing)
	{
		// Apply climbing physics
		FVector ClimbDirection = FVector::UpVector * LastMovementInput.X;
		ApplyForce(ClimbDirection * ClimbSpeed * 10.0f);
	}

	// Update sliding
	if (bIsSliding)
	{
		// Apply sliding physics
		FVector SlideDirection = GetMovementDirection();
		ApplyForce(SlideDirection * SlideSpeed * 10.0f);
	}

	// Check for advanced movement opportunities
	if (!bIsMantling && !bIsWallRunning && !bIsSliding)
	{
		CheckForMantle();
		CheckForWallRun();
		CheckForClimb();
	}
}

void AAdvancedLocomotionSystem::UpdateSpaceMovement(float DeltaTime)
{
	if (bZeroGMovementEnabled)
	{
		// Apply zero-G movement
		if (bJetpackEnabled && CurrentJetpackFuel > 0.0f)
		{
			// Apply jetpack thrust
			FVector ThrustForce = GetActorForwardVector() * CurrentThrust * JetpackThrust;
			ApplyForce(ThrustForce);

			// Consume fuel
			CurrentJetpackFuel -= JetpackFuelConsumption * DeltaTime;
			CurrentJetpackFuel = FMath::Max(0.0f, CurrentJetpackFuel);
		}

		// Apply rotation thrust
		if (!CurrentRotationThrust.IsNearlyZero())
		{
			FRotator NewRotation = GetActorRotation() + CurrentRotationThrust * DeltaTime * 90.0f;
			SetActorRotation(NewRotation);
		}
	}
}

void AAdvancedLocomotionSystem::UpdateVRMovement(float DeltaTime)
{
	if (bVREnabled)
	{
		UpdateVRMotionControllers();
	}
}

void AAdvancedLocomotionSystem::HandleMovementInput(float DeltaTime)
{
	// Movement input is handled in MoveForward/MoveRight/MoveUp
}

void AAdvancedLocomotionSystem::HandleRotationInput(float DeltaTime)
{
	// Rotation input is handled in LookUp/Turn
}

void AAdvancedLocomotionSystem::CalculateMovementDirection()
{
	// Calculate movement direction based on input
}

void AAdvancedLocomotionSystem::PerformLineTrace()
{
	// Perform interaction line trace
	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Camera->GetForwardVector() * 500.0f;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
	{
		// Handle interaction with hit object
		UE_LOG(LogTemp, Warning, TEXT("Hit object: %s"), *HitResult.GetActor()->GetName());
	}
}

void AAdvancedLocomotionSystem::PerformSphereTrace()
{
	// Perform sphere trace for area interactions
}

bool AAdvancedLocomotionSystem::CheckForMantle()
{
	// Check for mantle opportunity
	FVector Start = GetActorLocation();
	FVector End = Start + GetActorForwardVector() * 200.0f;
	FVector UpEnd = Start + FVector::UpVector * MantleHeight;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	// Check for obstacle in front
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
	{
		// Check if we can mantle over it
		FVector MantleCheck = HitResult.Location + FVector::UpVector * (MantleHeight + 50.0f);
		FHitResult MantleHit;
		
		if (!GetWorld()->LineTraceSingleByChannel(MantleHit, MantleCheck, MantleCheck + GetActorForwardVector() * 100.0f, ECC_Visibility, QueryParams))
		{
			return true;
		}
	}

	return false;
}

bool AAdvancedLocomotionSystem::CheckForWallRun()
{
	// Check for wall run opportunity
	FVector Start = GetActorLocation();
	FVector Right = GetActorRightVector();
	FVector Left = -Right;

	FHitResult RightHit, LeftHit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bRightWall = GetWorld()->LineTraceSingleByChannel(RightHit, Start, Start + Right * 100.0f, ECC_Visibility, QueryParams);
	bool bLeftWall = GetWorld()->LineTraceSingleByChannel(LeftHit, Start, Start + Left * 100.0f, ECC_Visibility, QueryParams);

	return (bRightWall || bLeftWall) && IsInAir() && bIsSprinting;
}

bool AAdvancedLocomotionSystem::CheckForClimb()
{
	// Check for climb opportunity
	FVector Start = GetActorLocation();
	FVector End = Start + GetActorForwardVector() * 100.0f;
	FVector UpEnd = Start + FVector::UpVector * 200.0f;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	// Check for climbable surface
	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
	{
		// Check if surface is climbable
		if (HitResult.Normal.Z > 0.7f) // Surface is mostly vertical
		{
			return true;
		}
	}

	return false;
}

void AAdvancedLocomotionSystem::StartMantleAnimation()
{
	// Start mantle animation
	PlayMontage(nullptr); // Would play actual mantle montage
}

void AAdvancedLocomotionSystem::StartSlideAnimation()
{
	// Start slide animation
	PlayMontage(nullptr); // Would play actual slide montage
}

void AAdvancedLocomotionSystem::UpdateMantleTimeline(float Value)
{
	// Update mantle position based on timeline
	FVector StartLocation = GetActorLocation();
	FVector TargetLocation = StartLocation + GetActorForwardVector() * 200.0f + FVector::UpVector * MantleHeight;
	FVector NewLocation = FMath::Lerp(StartLocation, TargetLocation, Value);
	SetActorLocation(NewLocation);
}

void AAdvancedLocomotionSystem::UpdateSlideTimeline(float Value)
{
	// Update slide physics based on timeline
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->Velocity = GetMovementDirection() * SlideSpeed;
	}
}

void AAdvancedLocomotionSystem::OnMantleTimelineFinished()
{
	EndMantle();
}

void AAdvancedLocomotionSystem::OnSlideTimelineFinished()
{
	bIsSliding = false;
	SetStance(EStance::Standing);
	ApplyMovementSettings();
}

FVector AAdvancedLocomotionSystem::GetMovementInput() const
{
	return LastMovementInput;
}

FRotator AAdvancedLocomotionSystem::GetDesiredRotation() const
{
	switch (CurrentRotationMode)
	{
		case ERotationMode::VelocityDirection:
			return GetMovementDirection().Rotation();
		case ERotationMode::LookingDirection:
			return GetControlRotation();
		case ERotationMode::Aiming:
			return GetControlRotation();
		default:
			return GetActorRotation();
	}
}

void AAdvancedLocomotionSystem::SetDesiredRotation(const FRotator& Rotation)
{
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	SetActorRotation(Rotation);
}

void AAdvancedLocomotionSystem::SmoothRotation(float DeltaTime)
{
	FRotator CurrentRotation = GetActorRotation();
	FRotator DesiredRotation = GetDesiredRotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, DesiredRotation, DeltaTime, 10.0f);
	SetActorRotation(NewRotation);
}

void AAdvancedLocomotionSystem::ApplyMovementSettings()
{
	if (!GetCharacterMovement())
	{
		return;
	}

	// Apply movement speed based on gait and stance
	float Speed = WalkSpeed;
	
	switch (CurrentGait)
	{
		case EGait::Walking:
			Speed = WalkSpeed;
			break;
		case EGait::Running:
			Speed = RunSpeed;
			break;
		case EGait::Sprinting:
			Speed = SprintSpeed;
			break;
	}

	// Apply stance modifiers
	switch (CurrentStance)
	{
		case EStance::Crouching:
			Speed *= 0.6f;
			break;
		case EStance::Sliding:
			Speed = SlideSpeed;
			break;
		default:
			break;
	}

	// Apply movement mode modifiers
	switch (CurrentMovementState)
	{
		case EMovementState::WallRunning:
			Speed = WallRunSpeed;
			break;
		case EMovementState::Climbing:
			Speed = ClimbSpeed;
			break;
		default:
			break;
	}

	GetCharacterMovement()->MaxWalkSpeed = Speed;
}

void AAdvancedLocomotionSystem::ResetMovementState()
{
	bIsMoving = false;
	bIsSprinting = false;
	bIsCrouching = false;
	bIsAiming = false;
	bIsSliding = false;
	bIsWallRunning = false;
	bIsClimbing = false;
	bIsMantling = false;
	bIsJumping = false;
	bIsFalling = false;
	CurrentThrust = 0.0f;
	CurrentRotationThrust = FRotator::ZeroRotator;
	LastMovementInput = FVector::ZeroVector;
}