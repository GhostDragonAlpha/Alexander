// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnhancedInput/Public/InputActionValue.h"
#include "SurfacePlayerCharacter.generated.h"

// Forward declarations
class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;
class UMotionControllerComponent;
class USurfaceExplorationComponent;
class ASpaceship;

/**
 * Movement Mode for surface exploration
 */
UENUM(BlueprintType)
enum class ESurfaceMovementMode : uint8
{
    Walking UMETA(DisplayName = "Walking"),
    Running UMETA(DisplayName = "Running"),
    Crouching UMETA(DisplayName = "Crouching"),
    Jumping UMETA(DisplayName = "Jumping"),
    Flying UMETA(DisplayName = "Flying (Jetpack)"),
    Climbing UMETA(DisplayName = "Climbing")
};

/**
 * Surface Player Character
 * Character class for walking on planet/moon surfaces
 * Supports both VR and non-VR gameplay
 */
UCLASS(Blueprintable, BlueprintType)
class ALEXANDER_API ASurfacePlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ASurfacePlayerCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
    // ============================================================================
    // COMPONENTS
    // ============================================================================

    // Camera boom (spring arm) for third person view
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    USpringArmComponent* CameraBoom;

    // Follow camera
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* FollowCamera;

    // VR camera (first person)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
    UCameraComponent* VRCamera;

    // VR Left hand controller
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
    UMotionControllerComponent* LeftController;

    // VR Right hand controller
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
    UMotionControllerComponent* RightController;

    // Surface exploration component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Exploration")
    USurfaceExplorationComponent* ExplorationComponent;

    // ============================================================================
    // MOVEMENT PROPERTIES
    // ============================================================================

    // Current movement mode
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    ESurfaceMovementMode MovementMode;

    // Base walk speed (cm/s)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed;

    // Base run speed (cm/s)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed;

    // Crouch speed (cm/s)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float CrouchSpeed;

    // Jump velocity (cm/s)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float JumpVelocity;

    // Is player currently running?
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    bool bIsRunning;

    // Stamina for running/jumping
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Movement")
    float Stamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxStamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float StaminaRegenRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float StaminaDrainRate;

    // ============================================================================
    // VR PROPERTIES
    // ============================================================================

    // Is this character in VR mode?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR")
    bool bIsVRMode;

    // VR locomotion mode
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR")
    bool bUseVRTeleport;

    // VR movement speed multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR")
    float VRMovementSpeedMultiplier;

    // ============================================================================
    // INTERACTION
    // ============================================================================

    // Interaction range (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRange;

    // Currently targeted interactive object
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
    AActor* TargetedActor;

    // Nearby spaceship (for entry)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
    ASpaceship* NearbySpaceship;

    // ============================================================================
    // MOVEMENT FUNCTIONS
    // ============================================================================

    // Movement input
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveForward(float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveRight(float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void LookUp(float Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void Turn(float Value);

    // Enhanced input actions
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void Move(const FInputActionValue& Value);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void Look(const FInputActionValue& Value);

    // Sprint
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartSprinting();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopSprinting();

    // Crouch
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartCrouching();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopCrouching();

    // Jump
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartJumping();

    void StopJumping();

    // Set movement mode
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetMovementMode(ESurfaceMovementMode NewMode);

    // ============================================================================
    // INTERACTION FUNCTIONS
    // ============================================================================

    // Interact with nearby object
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void Interact();

    // Check for nearby interactables
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void CheckForInteractables();

    // Enter nearby spaceship
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void EnterSpaceship();

    // ============================================================================
    // VR FUNCTIONS
    // ============================================================================

    // Initialize VR systems
    UFUNCTION(BlueprintCallable, Category = "VR")
    void InitializeVR();

    // Toggle VR mode
    UFUNCTION(BlueprintCallable, Category = "VR")
    void SetVRMode(bool bEnabled);

    // VR teleport
    UFUNCTION(BlueprintCallable, Category = "VR")
    void VRTeleport(FVector Location);

    // VR grab
    UFUNCTION(BlueprintCallable, Category = "VR")
    void VRGrabLeft();

    UFUNCTION(BlueprintCallable, Category = "VR")
    void VRGrabRight();

    UFUNCTION(BlueprintCallable, Category = "VR")
    void VRReleaseLeft();

    UFUNCTION(BlueprintCallable, Category = "VR")
    void VRReleaseRight();

    // ============================================================================
    // STATUS FUNCTIONS
    // ============================================================================

    // Get character status
    UFUNCTION(BlueprintCallable, Category = "Status")
    FString GetCharacterStatus() const;

    // Is character on ground?
    UFUNCTION(BlueprintCallable, Category = "Status")
    bool IsOnGround() const;

    // Get current gravity
    UFUNCTION(BlueprintCallable, Category = "Status")
    float GetCurrentGravity() const;

    // Update stamina
    UFUNCTION(BlueprintCallable, Category = "Status")
    void UpdateStamina(float DeltaTime);

    // ============================================================================
    // EVENTS
    // ============================================================================

    // Called when entering ship
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnEnteringShip(ASpaceship* Ship);

    // Called when landing on surface
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnLandedOnSurface();

protected:
    // Internal functions
    void UpdateMovementSpeed();
    void UpdateCamera(float DeltaTime);
    void HandleGravity();
    void CheckNearbyShip();
    void UpdateVRPhysicsGrab(float DeltaTime);

    // Enhanced Input
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* SprintAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* CrouchAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* InteractAction;

private:
    // Camera settings
    float BaseTurnRate;
    float BaseLookUpRate;

    // Interaction tracking
    float LastInteractionCheckTime;
    float InteractionCheckInterval;

    // VR state
    bool bVRInitialized;

    // VR grab state
    AActor* LeftHandGrabbedActor;
    AActor* RightHandGrabbedActor;
    UPrimitiveComponent* LeftHandGrabbedComponent;
    UPrimitiveComponent* RightHandGrabbedComponent;
    FVector LeftHandGrabOffset;
    FVector RightHandGrabOffset;
    FRotator LeftHandGrabRotationOffset;
    FRotator RightHandGrabRotationOffset;

    // VR grab detection
    float VRGrabRadius;
    float VRGrabForceStrength;
    bool bLeftHandPhysicsGrab;
    bool bRightHandPhysicsGrab;
};
