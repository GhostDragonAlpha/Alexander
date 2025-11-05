// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"
#include "VRSpaceshipPawn.generated.h"

// Forward declarations
class USceneComponent;
class UCameraComponent;
class UMotionControllerComponent;
class UPlayerOriginManager;
class UStaticMeshComponent;
class ASpaceship;
class UInputComponent;
class UGrabComponent;

/**
 * VR Spaceship Pawn
 *
 * Full C++ implementation of VR spaceship control for space exploration
 * Replaces Blueprint-only VR pawn for easier manipulation and better performance
 *
 * Key Features:
 * - VR origin at (0,0,0) for tracking accuracy (Unreal VR best practice)
 * - Motion controller support for hand tracking
 * - Integration with PlayerOriginManager for high-precision positioning
 * - Integration with ASpaceship for physics and systems
 * - Full network replication support
 * - Cockpit mesh visualization
 *
 * This pawn acts as the VR "camera rig" while the actual spaceship physics
 * are handled by the ASpaceship actor. This separation maintains VR tracking
 * precision while allowing complex ship physics.
 */
UCLASS(Blueprintable, BlueprintType)
class ALEXANDER_API AVRSpaceshipPawn : public APawn
{
	GENERATED_BODY()

public:
	AVRSpaceshipPawn();

	// ============================================================================
	// PAWN LIFECYCLE
	// ============================================================================

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	// ============================================================================
	// CORE VR COMPONENTS
	// ============================================================================

	/**
	 * VR Origin - ALWAYS at (0,0,0) for accurate VR tracking
	 * All VR components are children of this
	 * Never move this component!
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Components")
	USceneComponent* VROrigin;

	/**
	 * VR Camera - Tracks HMD position/rotation
	 * Relative to VROrigin
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Components")
	UCameraComponent* VRCamera;

	/**
	 * Left hand motion controller
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Components")
	UMotionControllerComponent* LeftHand;

	/**
	 * Right hand motion controller
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Components")
	UMotionControllerComponent* RightHand;

	/**
	 * Player Origin Manager - High-precision position tracking
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Components")
	UPlayerOriginManager* OriginManager;

	/**
	 * Cockpit mesh - Visual representation of ship interior
	 * Attached to VROrigin so it moves with VR space
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR Components")
	UStaticMeshComponent* CockpitMesh;

	// ============================================================================
	// SPACESHIP INTEGRATION
	// ============================================================================

	/**
	 * Reference to the spaceship this pawn controls
	 * The spaceship handles physics, systems, and multiplayer state
	 * This pawn handles VR visualization and input
	 */
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Spaceship")
	ASpaceship* PossessedSpaceship;

	/**
	 * Should this pawn automatically find and possess a spaceship on BeginPlay?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship")
	bool bAutoPossessSpaceship;

	/**
	 * Class of spaceship to spawn if none exists
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship")
	TSubclassOf<ASpaceship> SpaceshipClass;

	/**
	 * Manually possess a spaceship
	 */
	UFUNCTION(BlueprintCallable, Category = "Spaceship")
	void PossessSpaceship(ASpaceship* Spaceship);

	/**
	 * Release the current spaceship
	 */
	UFUNCTION(BlueprintCallable, Category = "Spaceship")
	void UnpossessSpaceship();

	// ============================================================================
	// VR INTERACTION
	// ============================================================================

	/**
	 * Grab an actor with left hand
	 */
	UFUNCTION(BlueprintCallable, Category = "VR Interaction")
	void GrabWithLeftHand(AActor* Actor);

	/**
	 * Release left hand grab
	 */
	UFUNCTION(BlueprintCallable, Category = "VR Interaction")
	void ReleaseLeftHand();

	/**
	 * Grab an actor with right hand
	 */
	UFUNCTION(BlueprintCallable, Category = "VR Interaction")
	void GrabWithRightHand(AActor* Actor);

	/**
	 * Release right hand grab
	 */
	UFUNCTION(BlueprintCallable, Category = "VR Interaction")
	void ReleaseRightHand();

	/**
	 * Currently grabbed actor (left hand)
	 */
	UPROPERTY(BlueprintReadOnly, Category = "VR Interaction")
	AActor* LeftHandGrabbedActor;

	/**
	 * Currently grabbed actor (right hand)
	 */
	UPROPERTY(BlueprintReadOnly, Category = "VR Interaction")
	AActor* RightHandGrabbedActor;

	// ============================================================================
	// VR TRACKING & VALIDATION
	// ============================================================================

	/**
	 * Validate VR tracking is functioning correctly
	 * Ensures precision is maintained
	 */
	UFUNCTION(BlueprintCallable, Category = "VR")
	void ValidateVRTracking();

	/**
	 * Get HMD position relative to VR origin
	 */
	UFUNCTION(BlueprintCallable, Category = "VR")
	FVector GetHMDPosition() const;

	/**
	 * Get HMD rotation relative to VR origin
	 */
	UFUNCTION(BlueprintCallable, Category = "VR")
	FRotator GetHMDRotation() const;

	/**
	 * Reset VR tracking origin
	 */
	UFUNCTION(BlueprintCallable, Category = "VR")
	void ResetVROrigin();

	// ============================================================================
	// INPUT HANDLING
	// ============================================================================

	/** Enable VR input processing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	bool bEnableVRInput;

	/** Input sensitivity multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float InputSensitivity;

protected:
	// ============================================================================
	// INPUT ACTIONS (Bind to Enhanced Input or legacy input)
	// ============================================================================

	/** Move forward/backward */
	void MoveForward(float Value);

	/** Move right/left */
	void MoveRight(float Value);

	/** Move up/down */
	void MoveUp(float Value);

	/** Pitch input */
	void PitchInput(float Value);

	/** Yaw input */
	void YawInput(float Value);

	/** Roll input */
	void RollInput(float Value);

	/** Left hand trigger pressed */
	void OnLeftTriggerPressed();
	void OnLeftTriggerReleased();

	/** Right hand trigger pressed */
	void OnRightTriggerPressed();
	void OnRightTriggerReleased();

	/** Left hand grip pressed */
	void OnLeftGripPressed();
	void OnLeftGripReleased();

	/** Right hand grip pressed */
	void OnRightGripPressed();
	void OnRightGripReleased();

	// ============================================================================
	// VR SETUP
	// ============================================================================

	/**
	 * Setup VR components hierarchy
	 * Called in constructor
	 */
	void SetupVRComponents();

	/**
	 * Update hand tracking visualization
	 * Called each tick
	 */
	void UpdateHandTracking();

	/**
	 * Update spaceship controls based on input
	 */
	void UpdateSpaceshipControls(float DeltaTime);

	// ============================================================================
	// DEBUG & UTILITIES
	// ============================================================================

	/** Show debug information */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugInfo;

	/** Draw debug visualization */
	void DrawDebugVisualization();

	/** Get VR status information */
	UFUNCTION(BlueprintCallable, Category = "VR")
	FString GetVRStatusInfo() const;

private:
	// ============================================================================
	// INTERNAL STATE
	// ============================================================================

	/** Cached input values */
	FVector CurrentThrustInput;
	FVector CurrentRotationInput;

	/** VR tracking state */
	bool bIsVRTrackingValid;
	float LastTrackingValidationTime;

	/** Hand tracking state */
	bool bLeftHandTracking;
	bool bRightHandTracking;

	// ============================================================================
	// INTERNAL FUNCTIONS
	// ============================================================================

	/** Find or spawn spaceship */
	void FindOrSpawnSpaceship();

	/** Update VR origin if needed (for universe recentering) */
	void UpdateVROriginTransform();

	/** Check if VR is enabled and available */
	bool IsVREnabled() const;
};
