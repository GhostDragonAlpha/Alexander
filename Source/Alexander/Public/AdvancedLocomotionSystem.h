// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "AdvancedLocomotionSystem.generated.h"

// Forward declarations
class UCameraComponent;
class USpringArmComponent;
class UAnimMontage;
class UAnimInstance;

/**
 * Movement states
 */
UENUM(BlueprintType)
enum class EMovementState : uint8
{
	None UMETA(DisplayName = "None"),
	Idle UMETA(DisplayName = "Idle"),
	Walking UMETA(DisplayName = "Walking"),
	Running UMETA(DisplayName = "Running"),
	Sprinting UMETA(DisplayName = "Sprinting"),
	Jogging UMETA(DisplayName = "Jogging"),
	Crouching UMETA(DisplayName = "Crouching"),
	Sliding UMETA(DisplayName = "Sliding"),
	Climbing UMETA(DisplayName = "Climbing"),
	Jumping UMETA(DisplayName = "Jumping"),
	Falling UMETA(DisplayName = "Falling"),
	Landing UMETA(DisplayName = "Landing"),
	Mantling UMETA(DisplayName = "Mantling"),
	WallRunning UMETA(DisplayName = "Wall Running"),
	Swimming UMETA(DisplayName = "Swimming"),
	Flying UMETA(DisplayName = "Flying"),
	Driving UMETA(DisplayName = "Driving"),
	ZeroG UMETA(DisplayName = "Zero Gravity")
};

/**
 * Stance states
 */
UENUM(BlueprintType)
enum class EStance : uint8
{
	Standing UMETA(DisplayName = "Standing"),
	Crouching UMETA(DisplayName = "Crouching"),
	Prone UMETA(DisplayName = "Prone"),
	Sliding UMETA(DisplayName = "Sliding")
};

/**
 * VR Movement types
 */
UENUM(BlueprintType)
enum class EVRMovementType : uint8
{
	Teleport UMETA(DisplayName = "Teleport"),
	SmoothLocomotion UMETA(DisplayName = "Smooth Locomotion"),
	SnapTurn UMETA(DisplayName = "Snap Turn"),
	SmoothTurn UMETA(DisplayName = "Smooth Turn"),
	RoomScale UMETA(DisplayName = "Room Scale")
};

/**
 * Rotation modes
 */
UENUM(BlueprintType)
enum class ERotationMode : uint8
{
	VelocityDirection UMETA(DisplayName = "Velocity Direction"),
	LookingDirection UMETA(DisplayName = "Looking Direction"),
	Aiming UMETA(DisplayName = "Aiming")
};

/**
 * Gait states
 */
UENUM(BlueprintType)
enum class EGait : uint8
{
	Walking UMETA(DisplayName = "Walking"),
	Running UMETA(DisplayName = "Running"),
	Sprinting UMETA(DisplayName = "Sprinting")
};

/**
 * Advanced Locomotion System - ALS-inspired character movement
 * Provides realistic character movement with advanced physics and animations
 */
UCLASS(BlueprintType, Blueprintable)
class ALEXANDER_API AAdvancedLocomotionSystem : public ACharacter
{
	GENERATED_BODY()

public:
	AAdvancedLocomotionSystem();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

public:
	// Movement input
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void MoveForward(float Value);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void MoveRight(float Value);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void MoveUp(float Value);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void LookUp(float Value);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Turn(float Value);

	// Action inputs
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void JumpAction();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SprintStart();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SprintEnd();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void CrouchStart();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void CrouchEnd();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SlideStart();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void AimStart();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void AimEnd();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Interact();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Vault();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Mantle();

	// Movement state management
	UFUNCTION(BlueprintCallable, Category = "Movement")
	EMovementState GetMovementState() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	EStance GetStance() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	ERotationMode GetRotationMode() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	EGait GetGait() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetMovementState(EMovementState NewState);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetStance(EStance NewStance);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetRotationMode(ERotationMode NewMode);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetGait(EGait NewGait);

	// Movement properties
	UFUNCTION(BlueprintCallable, Category = "Movement")
	float GetMovementSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	FVector GetMovementDirection() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	float GetMovementInputAmount() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsMoving() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsInAir() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsSprinting() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsCrouching() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsAiming() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsSliding() const;

	// Advanced movement
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartWallRun();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void EndWallRun();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool CanWallRun() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartClimb();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void EndClimb();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool CanClimb() const;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartMantle();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void EndMantle();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool CanMantle() const;

	// Physics and collision
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetGroundFriction(float Friction);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetAirControl(float Control);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetGravityScale(float Scale);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ApplyForce(const FVector& Force);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ApplyImpulse(const FVector& Impulse);

	// Camera system
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraFOV(float FOV);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	float GetCameraFOV() const;

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraDistance(float Distance);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	float GetCameraDistance() const;

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void EnableCameraShake(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void PlayCameraShake(TSubclassOf<class UCameraShakeBase> ShakeClass, float Scale = 1.0f);

	// Animation system
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayMontage(UAnimMontage* Montage, float PlayRate = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void StopMontage(UAnimMontage* Montage);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	bool IsMontagePlaying(UAnimMontage* Montage) const;

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SetAnimationInstance(UAnimInstance* AnimInstance);

	UFUNCTION(BlueprintCallable, Category = "Animation")
	UAnimInstance* GetAnimationInstance() const;

	// VR support
	UFUNCTION(BlueprintCallable, Category = "VR")
	void EnableVRSupport(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "VR")
	bool IsVREnabled() const;

	UFUNCTION(BlueprintCallable, Category = "VR")
	void SetVRMovementType(EVRMovementType MovementType);

	UFUNCTION(BlueprintCallable, Category = "VR")
	void UpdateVRMotionControllers();

	// Space-specific movement
	UFUNCTION(BlueprintCallable, Category = "Space")
	void EnableZeroGMovement(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Space")
	bool IsZeroGMovementEnabled() const;

	UFUNCTION(BlueprintCallable, Category = "Space")
	void SetThrust(float ThrustAmount);

	UFUNCTION(BlueprintCallable, Category = "Space")
	void SetRotationThrust(const FRotator& Rotation);

	UFUNCTION(BlueprintCallable, Category = "Space")
	void EnableJetpack(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Space")
	bool IsJetpackEnabled() const;

protected:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UTimelineComponent* MantleTimeline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UTimelineComponent* SlideTimeline;

	// Movement state
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	EMovementState CurrentMovementState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	EStance CurrentStance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	ERotationMode CurrentRotationMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	EGait CurrentGait;

	// Movement properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RunSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float CrouchSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SlideSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float JumpHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AirControl;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GroundFriction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GravityScale;

	// Advanced movement properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WallRunSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WallRunDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float ClimbSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MantleHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MantleDuration;

	// Camera properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float BaseFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float AimFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float AimFOVInterpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bEnableCameraShake;

	// VR properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR")
	bool bVREnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR")
	EVRMovementType VRMovementType;

	// Space properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Space")
	bool bZeroGMovementEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Space")
	float JetpackThrust;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Space")
	float JetpackFuel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Space")
	float JetpackFuelConsumption;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Space")
	bool bJetpackEnabled;

	// Internal state
	UPROPERTY()
	bool bIsMoving;

	UPROPERTY()
	bool bIsSprinting;

	UPROPERTY()
	bool bIsAiming;

	UPROPERTY()
	bool bIsSliding;

	UPROPERTY()
	bool bIsWallRunning;

	UPROPERTY()
	bool bIsClimbing;

	UPROPERTY()
	bool bIsMantling;

	UPROPERTY()
	bool bIsJumping;

	UPROPERTY()
	bool bIsFalling;

	UPROPERTY()
	float CurrentThrust;

	UPROPERTY()
	FRotator CurrentRotationThrust;

	UPROPERTY()
	float CurrentJetpackFuel;

	UPROPERTY()
	FVector LastMovementInput;

	UPROPERTY()
	FVector LastVelocity;

	UPROPERTY()
	float LastMovementTime;

private:
	void UpdateMovementState(float DeltaTime);
	void UpdateStance();
	void UpdateGait();
	void UpdateRotation(float DeltaTime);
	void UpdateCamera(float DeltaTime);
	void UpdateAdvancedMovement(float DeltaTime);
	void UpdateSpaceMovement(float DeltaTime);
	void UpdateVRMovement(float DeltaTime);
	void HandleMovementInput(float DeltaTime);
	void HandleRotationInput(float DeltaTime);
	void CalculateMovementDirection();
	void PerformLineTrace();
	void PerformSphereTrace();
	bool CheckForMantle();
	bool CheckForWallRun();
	bool CheckForClimb();
	void StartMantleAnimation();
	void StartSlideAnimation();
	void UpdateMantleTimeline(float Value);
	void UpdateSlideTimeline(float Value);
	void OnMantleTimelineFinished();
	void OnSlideTimelineFinished();
	FVector GetMovementInput() const;
	float GetMovementInputAmount() const;
	FRotator GetDesiredRotation() const;
	void SetDesiredRotation(const FRotator& Rotation);
	void SmoothRotation(float DeltaTime);
	void ApplyMovementSettings();
	void ResetMovementState();
};