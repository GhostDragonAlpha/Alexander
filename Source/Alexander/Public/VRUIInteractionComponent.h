// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/WidgetComponent.h"
#include "InputCoreTypes.h"
#include "VRUIInteractionComponent.generated.h"

// Forward declarations
class UMotionControllerComponent;
class UWidgetInteractionComponent;
class UHapticFeedbackEffect_Base;
class UNiagaraComponent;
class UNiagaraSystem;

/**
 * VR interaction hand type
 */
UENUM(BlueprintType)
enum class EVRInteractionHand : uint8
{
	Left UMETA(DisplayName = "Left Hand"),
	Right UMETA(DisplayName = "Right Hand"),
	Both UMETA(DisplayName = "Both Hands")
};

/**
 * Widget hover state for visual feedback
 */
UENUM(BlueprintType)
enum class EVRWidgetHoverState : uint8
{
	None UMETA(DisplayName = "No Hover"),
	Hovering UMETA(DisplayName = "Hovering"),
	Pressed UMETA(DisplayName = "Pressed"),
	Released UMETA(DisplayName = "Released")
};

/**
 * Haptic feedback pattern configuration
 */
USTRUCT(BlueprintType)
struct FVRHapticPattern
{
	GENERATED_BODY()

	// Haptic effect to play
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptics")
	UHapticFeedbackEffect_Base* HapticEffect = nullptr;

	// Intensity of haptic feedback (0.0 - 1.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Intensity = 0.5f;

	// Duration of haptic feedback in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptics", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float Duration = 0.2f;

	// Should loop the haptic effect?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptics")
	bool bLoop = false;

	FVRHapticPattern()
		: Intensity(0.5f)
		, Duration(0.2f)
		, bLoop(false)
	{
	}
};

/**
 * Widget interaction result
 */
USTRUCT(BlueprintType)
struct FVRWidgetInteractionResult
{
	GENERATED_BODY()

	// Was the interaction successful?
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bSuccess = false;

	// The widget that was interacted with
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	UWidgetComponent* HitWidget = nullptr;

	// Hit location in world space
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FVector HitLocation = FVector::ZeroVector;

	// Hit location in 2D widget space
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FVector2D HitLocationWidget2D = FVector2D::ZeroVector;

	// Distance to the widget
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	float Distance = 0.0f;

	// Current hover state
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	EVRWidgetHoverState HoverState = EVRWidgetHoverState::None;
};

/**
 * VR UI Interaction Component
 * Handles raycast-based UI interaction for VR motion controllers
 * Supports widget interaction, haptic feedback, and visual pointer feedback
 * Optimized for 90 FPS VR performance
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (VR), meta = (BlueprintSpawnableComponent))
class ALEXANDER_API UVRUIInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRUIInteractionComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ============================================================================
	// RAYCAST & HIT DETECTION
	// ============================================================================

	/**
	 * Perform raycast from motion controller to detect UI widgets
	 * @return True if a widget was hit
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	bool PerformRaycast();

	/**
	 * Get the raycast result
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	FVRWidgetInteractionResult GetRaycastResult() const { return LastInteractionResult; }

	/**
	 * Get currently hovered widget
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	UWidgetComponent* GetHoveredWidget() const { return CurrentHoveredWidget; }

	/**
	 * Check if a widget is currently hovered
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	bool IsHoveringWidget() const { return CurrentHoveredWidget != nullptr; }

	// ============================================================================
	// BUTTON PRESS & INTERACTION
	// ============================================================================

	/**
	 * Handle button press on currently hovered widget
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	void HandleButtonPress();

	/**
	 * Handle button release
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	void HandleButtonRelease();

	/**
	 * Simulate click on currently hovered widget
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	void SimulateClick();

	/**
	 * Is a button currently pressed?
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	bool IsButtonPressed() const { return bIsButtonPressed; }

	// ============================================================================
	// POINTER VISUALIZATION
	// ============================================================================

	/**
	 * Update pointer location and visual
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	void UpdatePointerLocation(const FVector& StartLocation, const FVector& EndLocation);

	/**
	 * Show the laser pointer
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	void ShowPointer();

	/**
	 * Hide the laser pointer
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	void HidePointer();

	/**
	 * Set pointer color
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	void SetPointerColor(FLinearColor NewColor);

	/**
	 * Is pointer currently visible?
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	bool IsPointerVisible() const { return bPointerVisible; }

	// ============================================================================
	// HAPTIC FEEDBACK
	// ============================================================================

	/**
	 * Trigger haptic feedback on the current controller
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	void TriggerHapticFeedback(const FVRHapticPattern& Pattern);

	/**
	 * Play haptic feedback for hover event
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	void PlayHoverHaptics();

	/**
	 * Play haptic feedback for press event
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	void PlayPressHaptics();

	/**
	 * Play haptic feedback for release event
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	void PlayReleaseHaptics();

	/**
	 * Stop all haptic feedback
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	void StopHapticFeedback();

	// ============================================================================
	// HAND TRACKING
	// ============================================================================

	/**
	 * Set which hand this component is tracking
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	void SetInteractionHand(EVRInteractionHand NewHand);

	/**
	 * Get current interaction hand
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	EVRInteractionHand GetInteractionHand() const { return InteractionHand; }

	/**
	 * Get the motion controller component for this hand
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	UMotionControllerComponent* GetMotionController() const { return MotionController; }

	/**
	 * Set the motion controller for this component
	 */
	UFUNCTION(BlueprintCallable, Category = "VR UI Interaction")
	void SetMotionController(UMotionControllerComponent* NewController);

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	// Maximum interaction distance in centimeters (default 200cm = 2 meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Interaction", meta = (ClampMin = "10.0", ClampMax = "500.0"))
	float MaxInteractionDistance = 200.0f;

	// Ray trace channel for widget detection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Interaction")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	// Enable haptic feedback
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Interaction")
	bool bEnableHapticFeedback = true;

	// Enable debug visualization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Interaction")
	bool bShowDebugRays = false;

	// Pointer beam thickness in centimeters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Pointer", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float PointerThickness = 0.3f;

	// Pointer color when not hovering
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Pointer")
	FLinearColor PointerColorIdle = FLinearColor(0.3f, 0.5f, 1.0f, 0.6f);

	// Pointer color when hovering over widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Pointer")
	FLinearColor PointerColorHover = FLinearColor(0.0f, 1.0f, 0.3f, 0.8f);

	// Pointer color when button is pressed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Pointer")
	FLinearColor PointerColorPressed = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);

	// Niagara system for laser pointer effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Pointer")
	UNiagaraSystem* LaserPointerEffect = nullptr;

	// ============================================================================
	// HAPTIC PATTERNS
	// ============================================================================

	// Haptic pattern for hover
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Haptics")
	FVRHapticPattern HoverHapticPattern;

	// Haptic pattern for button press
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Haptics")
	FVRHapticPattern PressHapticPattern;

	// Haptic pattern for button release
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Haptics")
	FVRHapticPattern ReleaseHapticPattern;

	// ============================================================================
	// PERFORMANCE OPTIMIZATION
	// ============================================================================

	// Update rate for raycast (frames to skip, 0 = every frame)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Performance", meta = (ClampMin = "0", ClampMax = "5"))
	int32 RaycastSkipFrames = 0;

	// Use simplified collision for raycasts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Performance")
	bool bUseSimplifiedCollision = true;

	// Cache widget interaction component results
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR Performance")
	bool bCacheInteractionResults = true;

protected:
	// ============================================================================
	// INTERNAL STATE
	// ============================================================================

	// Current interaction hand
	UPROPERTY()
	EVRInteractionHand InteractionHand = EVRInteractionHand::Right;

	// Motion controller reference
	UPROPERTY()
	UMotionControllerComponent* MotionController = nullptr;

	// Widget interaction component for raycasting
	UPROPERTY()
	UWidgetInteractionComponent* WidgetInteraction = nullptr;

	// Currently hovered widget
	UPROPERTY()
	UWidgetComponent* CurrentHoveredWidget = nullptr;

	// Previously hovered widget (for hover exit detection)
	UPROPERTY()
	UWidgetComponent* PreviousHoveredWidget = nullptr;

	// Last interaction result
	UPROPERTY()
	FVRWidgetInteractionResult LastInteractionResult;

	// Is button currently pressed?
	bool bIsButtonPressed = false;

	// Is pointer visible?
	bool bPointerVisible = true;

	// Laser pointer visual component (Niagara)
	UPROPERTY()
	UNiagaraComponent* LaserPointerComponent = nullptr;

	// Frame counter for performance optimization
	int32 FrameCounter = 0;

	// Last raycast time for performance tracking
	float LastRaycastTime = 0.0f;

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Initialize the widget interaction component
	 */
	void InitializeWidgetInteraction();

	/**
	 * Initialize the laser pointer visual
	 */
	void InitializeLaserPointer();

	/**
	 * Find and attach to motion controller
	 */
	void FindMotionController();

	// ============================================================================
	// INTERNAL HELPERS
	// ============================================================================

	/**
	 * Update hover state based on raycast result
	 */
	void UpdateHoverState();

	/**
	 * Handle hover enter event
	 */
	void OnHoverEnter(UWidgetComponent* Widget);

	/**
	 * Handle hover exit event
	 */
	void OnHoverExit(UWidgetComponent* Widget);

	/**
	 * Get controller hand from motion controller
	 */
	EControllerHand GetControllerHand() const;

	/**
	 * Play haptic effect on controller
	 */
	void PlayHapticOnController(float Intensity, float Duration);

	/**
	 * Update laser pointer visual based on interaction state
	 */
	void UpdateLaserPointerVisual();

	/**
	 * Check if raycast should be performed this frame (performance optimization)
	 */
	bool ShouldPerformRaycastThisFrame() const;

	/**
	 * Draw debug visualization
	 */
	void DrawDebugVisualization(const FVector& Start, const FVector& End, bool bHit);
};
