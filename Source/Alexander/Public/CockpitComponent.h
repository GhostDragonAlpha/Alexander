// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Materials/MaterialInterface.h"
#include "CockpitComponent.generated.h"

// Forward declarations
class UCameraComponent;
class UMotionControllerComponent;
class UWidgetInteractionComponent;
class USceneCaptureComponent2D;

/**
 * Cockpit interaction type
 */
UENUM(BlueprintType)
enum class ECockpitInteractionType : uint8
{
	Button UMETA(DisplayName = "Button"),
	Switch UMETA(DisplayName = "Switch"),
	Slider UMETA(DisplayName = "Slider"),
	Throttle UMETA(DisplayName = "Throttle"),
	Stick UMETA(DisplayName = "Stick"),
	Display UMETA(DisplayName = "Display")
};

/**
 * Cockpit control definition
 */
USTRUCT(BlueprintType)
struct FCockpitControl
{
	GENERATED_BODY()

	// Control identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
	FName ControlName = NAME_None;

	// Type of control
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
	ECockpitInteractionType ControlType = ECockpitInteractionType::Button;

	// Visual mesh for the control
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
	UStaticMesh* ControlMesh = nullptr;

	// Control position relative to cockpit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
	FVector RelativePosition = FVector::ZeroVector;

	// Control rotation relative to cockpit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
	FRotator RelativeRotation = FRotator::ZeroRotator;

	// Control scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
	FVector Scale = FVector(1.0f, 1.0f, 1.0f);

	// Material for the control
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
	UMaterialInterface* ControlMaterial = nullptr;

	// Is this control interactive?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
	bool bIsInteractive = true;

	// Current value (for switches, sliders, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
	float CurrentValue = 0.0f;

	// Min/Max values for sliders and throttles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
	FVector2D ValueRange = FVector2D(0.0f, 1.0f);

	// Function to call when interacted with
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
	FName OnInteractFunction = NAME_None;

	// Sound effect for interaction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Control")
	class USoundBase* InteractionSound = nullptr;
};

/**
 * Cockpit display configuration
 */
USTRUCT(BlueprintType)
struct FCockpitDisplay
{
	GENERATED_BODY()

	// Display identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FName DisplayName = NAME_None;

	// Display type (HUD, radar, systems, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FName DisplayType = TEXT("HUD");

	// Display position relative to cockpit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FVector RelativePosition = FVector::ZeroVector;

	// Display size
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FVector2D DisplaySize = FVector2D(512.0f, 512.0f);

	// Widget class for the display
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	TSubclassOf<class UUserWidget> WidgetClass = nullptr;

	// Is this display interactive?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	bool bIsInteractive = false;

	// Render target for the display
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	class UTextureRenderTarget2D* RenderTarget = nullptr;
};

/**
 * VR Cockpit Component - manages all interactive cockpit elements
 * Provides Star Citizen-level detail and interaction
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALEXANDER_API UCockpitComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UCockpitComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ============================================================================
	// COCKPIT CONFIGURATION
	// ============================================================================

	// Cockpit mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cockpit")
	UStaticMeshComponent* CockpitMesh;

	// Camera attachment point
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cockpit")
	USceneComponent* CameraAttachmentPoint;

	// List of all cockpit controls
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
	TArray<FCockpitControl> CockpitControls;

	// List of all cockpit displays
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Displays")
	TArray<FCockpitDisplay> CockpitDisplays;

	// Master material for cockpit elements
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInterface* CockpitMasterMaterial;

	// ============================================================================
	// VR INTERACTION
	// ============================================================================

	// Left motion controller reference
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	UMotionControllerComponent* LeftController;

	// Right motion controller reference
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	UMotionControllerComponent* RightController;

	// Widget interaction for left controller
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	UWidgetInteractionComponent* LeftWidgetInteraction;

	// Widget interaction for right controller
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VR")
	UWidgetInteractionComponent* RightWidgetInteraction;

	// Interaction distance for VR controllers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR")
	float InteractionDistance = 50.0f;

	// Haptic feedback intensity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR")
	float HapticIntensity = 0.5f;

	// ============================================================================
	// COCKPIT SYSTEMS
	// ============================================================================

	// Is cockpit initialized?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsInitialized = false;

	// Current power state (0-100)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	float PowerLevel = 100.0f;

	// Are systems online?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bSystemsOnline = true;

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	// Initialize cockpit with all controls and displays
	UFUNCTION(BlueprintCallable, Category = "Cockpit")
	void InitializeCockpit();

	// Create all control meshes
	UFUNCTION(BlueprintCallable, Category = "Cockpit")
	void CreateControls();

	// Create all display widgets
	UFUNCTION(BlueprintCallable, Category = "Cockpit")
	void CreateDisplays();

	// Setup VR interaction components
	UFUNCTION(BlueprintCallable, Category = "Cockpit")
	void SetupVRInteraction();

	// ============================================================================
	// CONTROL INTERACTION
	// ============================================================================

	// Handle controller interaction with controls
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool HandleControlInteraction(UMotionControllerComponent* Controller, const FVector& HitLocation, const FVector& HitNormal);

	// Get the control at a specific world location
	FCockpitControl* GetControlAtLocation(const FVector& WorldLocation);

	// Interact with a specific control
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void InteractWithControl(FCockpitControl& Control, UMotionControllerComponent* Controller);

	// Update control visual state
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void UpdateControlVisual(FCockpitControl& Control);

	// Play interaction feedback
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void PlayInteractionFeedback(const FCockpitControl& Control, UMotionControllerComponent* Controller);

	// ============================================================================
	// DISPLAY MANAGEMENT
	// ============================================================================

	// Update all displays
	UFUNCTION(BlueprintCallable, Category = "Displays")
	void UpdateDisplays();

	// Get display by name
	FCockpitDisplay* GetDisplay(const FName& DisplayName);

	// Update specific display
	UFUNCTION(BlueprintCallable, Category = "Displays")
	void UpdateDisplay(const FName& DisplayName, class UUserWidget* NewWidget);

	// ============================================================================
	// SYSTEM INTEGRATION
	// ============================================================================

	// Set power level for cockpit systems
	UFUNCTION(BlueprintCallable, Category = "Systems")
	void SetPowerLevel(float NewPowerLevel);

	// Toggle all systems on/off
	UFUNCTION(BlueprintCallable, Category = "Systems")
	void ToggleSystems();

	// Update cockpit based on ship damage
	UFUNCTION(BlueprintCallable, Category = "Systems")
	void ApplyDamageEffects(float DamageAmount);

	// Get cockpit status as text
	UFUNCTION(BlueprintCallable, Category = "Systems")
	FString GetCockpitStatus() const;

	// ============================================================================
	// DEBUG & UTILITIES
	// ============================================================================

	// Show interaction debug info
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowInteractionDebug = false;

	// Highlight interactive controls
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bHighlightInteractiveControls = true;

	// Draw debug information
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DrawDebugInfo();

protected:
	// Internal control components
	UPROPERTY()
	TMap<FName, UStaticMeshComponent*> ControlComponents;

	// Internal display widgets
	UPROPERTY()
	TMap<FName, UWidgetComponent*> DisplayComponents;

	// Currently hovered control
	// Hovered control (C++ only due to struct pointer limitation)
	FCockpitControl* HoveredControl = nullptr;

	// Track last interaction time for cooldown
	UPROPERTY()
	float LastInteractionTime = 0.0f;

	// Interaction cooldown duration
	UPROPERTY()
	float InteractionCooldown = 0.1f;

private:
	// Helper functions
	UStaticMeshComponent* CreateControlMesh(const FCockpitControl& Control);
	UWidgetComponent* CreateDisplayWidget(const FCockpitDisplay& Display);
	void UpdateControlHighlight(FCockpitControl* Control, bool bHighlight);
	bool IsWithinInteractionDistance(const FVector& ControllerLocation, const FVector& ControlLocation);
};