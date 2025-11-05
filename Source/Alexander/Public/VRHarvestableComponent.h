// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VRHarvestableComponent.generated.h"

// Forward declarations
class UCropDefinition;
class UHapticFeedbackEffect_Base;
class USoundBase;

/**
 * Harvest interaction state
 */
UENUM(BlueprintType)
enum class EHarvestState : uint8
{
	NotReady,      // Crop not mature enough
	ReadyToHarvest, // Crop is mature and can be harvested
	BeingGrabbed,  // Player is grabbing the crop
	Harvested      // Crop has been harvested
};

/**
 * Component that makes a crop harvestable via VR grab interaction
 * Attached to crop instances to enable physical harvesting
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UVRHarvestableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRHarvestableComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// HARVEST STATE
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	EHarvestState HarvestState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	float MinimumGrowthForHarvest; // 0-1, typically 0.95 or higher

	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	float CurrentGrowthProgress; // 0-1

	UPROPERTY(BlueprintReadOnly, Category = "Harvest")
	bool bIsGrabbable;

	// ============================================================================
	// CROP REFERENCE
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Crop")
	UCropDefinition* CropType;

	UPROPERTY(BlueprintReadOnly, Category = "Crop")
	float CropHealth; // 0-1

	UPROPERTY(BlueprintReadOnly, Category = "Crop")
	int32 YieldAmount;

	UPROPERTY(BlueprintReadOnly, Category = "Crop")
	float YieldQuality; // 0-1, affects value

	// ============================================================================
	// FARM PLOT REFERENCE
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Farm")
	class AFarmPlot* OwningFarmPlot;

	UPROPERTY(BlueprintReadOnly, Category = "Farm")
	FIntPoint GridPosition;

	// ============================================================================
	// VR INTERACTION
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, Category = "VR")
	bool bIsBeingGrabbed;

	UPROPERTY(BlueprintReadOnly, Category = "VR")
	AActor* GrabbingHand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR")
	float GrabRadius; // Radius for grab detection

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR")
	float DetachmentForceThreshold; // Force required to detach crop

	UPROPERTY(BlueprintReadOnly, Category = "VR")
	float AccumulatedPullForce; // Force accumulated during grab

	// ============================================================================
	// DETACHMENT MECHANICS
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detachment")
	float DetachmentTime; // Time to fully detach (seconds)

	UPROPERTY(BlueprintReadOnly, Category = "Detachment")
	float DetachmentProgress; // 0-1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detachment")
	bool bRequiresPullingMotion; // Require upward pull to harvest

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detachment")
	FVector DetachmentDirection; // Direction to pull (usually up)

	// ============================================================================
	// HAPTIC FEEDBACK
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptics")
	UHapticFeedbackEffect_Base* GrabHapticEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptics")
	UHapticFeedbackEffect_Base* DetachmentHapticEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptics")
	UHapticFeedbackEffect_Base* HarvestCompleteHapticEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptics")
	float HapticIntensityMultiplier; // 0-1

	// ============================================================================
	// AUDIO FEEDBACK
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* GrabSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* DetachmentSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* HarvestCompleteSound;

	// ============================================================================
	// VISUAL FEEDBACK
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FLinearColor ReadyToHarvestColor; // Highlight color when ready

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FLinearColor GrabbedColor; // Color when being grabbed

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	float HighlightIntensity; // Glow intensity

	UPROPERTY()
	UMaterialInstanceDynamic* CropMaterialInstance;

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Initialize harvestable component with crop data
	 */
	UFUNCTION(BlueprintCallable, Category = "Harvest")
	void InitializeHarvestable(
		UCropDefinition* InCropType,
		float InGrowthProgress,
		float InHealth,
		AFarmPlot* InFarmPlot,
		FIntPoint InGridPosition
	);

	/**
	 * Update growth progress (called by farm plot)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harvest")
	void UpdateGrowthProgress(float NewProgress);

	/**
	 * Update crop health (called by farm plot)
	 */
	UFUNCTION(BlueprintCallable, Category = "Harvest")
	void UpdateCropHealth(float NewHealth);

	// ============================================================================
	// VR INTERACTION FUNCTIONS
	// ============================================================================

	/**
	 * Called when VR hand attempts to grab the crop
	 */
	UFUNCTION(BlueprintCallable, Category = "VR")
	bool TryGrab(AActor* GrabbingActor);

	/**
	 * Called when VR hand releases the crop
	 */
	UFUNCTION(BlueprintCallable, Category = "VR")
	void Release();

	/**
	 * Update grab interaction (called during tick while grabbed)
	 */
	UFUNCTION(BlueprintCallable, Category = "VR")
	void UpdateGrabInteraction(float DeltaTime);

	/**
	 * Check if crop can be grabbed
	 */
	UFUNCTION(BlueprintCallable, Category = "VR")
	bool CanBeGrabbed() const;

	/**
	 * Check if crop is ready for harvest
	 */
	UFUNCTION(BlueprintCallable, Category = "VR")
	bool IsReadyForHarvest() const;

	// ============================================================================
	// DETACHMENT LOGIC
	// ============================================================================

	/**
	 * Calculate pull force based on hand movement
	 */
	UFUNCTION(BlueprintCallable, Category = "Detachment")
	float CalculatePullForce(FVector HandVelocity) const;

	/**
	 * Apply pull force to detachment progress
	 */
	UFUNCTION(BlueprintCallable, Category = "Detachment")
	void ApplyPullForce(float Force, float DeltaTime);

	/**
	 * Check if detachment is complete
	 */
	UFUNCTION(BlueprintCallable, Category = "Detachment")
	bool IsDetachmentComplete() const;

	/**
	 * Complete the harvest and spawn harvest item
	 */
	UFUNCTION(BlueprintCallable, Category = "Harvest")
	void CompleteHarvest();

	// ============================================================================
	// HARVEST COLLECTION
	// ============================================================================

	/**
	 * Calculate harvest yield based on health and growth
	 */
	UFUNCTION(BlueprintCallable, Category = "Harvest")
	int32 CalculateHarvestYield() const;

	/**
	 * Calculate harvest quality based on conditions
	 */
	UFUNCTION(BlueprintCallable, Category = "Harvest")
	float CalculateHarvestQuality() const;

	/**
	 * Spawn harvested item in player's hand
	 */
	UFUNCTION(BlueprintCallable, Category = "Harvest")
	AActor* SpawnHarvestedItem();

	/**
	 * Add harvested item to player inventory
	 */
	UFUNCTION(BlueprintCallable, Category = "Harvest")
	void AddToInventory(int32 Amount, float Quality);

	// ============================================================================
	// HAPTIC FEEDBACK FUNCTIONS
	// ============================================================================

	/**
	 * Play haptic feedback on grabbing hand
	 */
	UFUNCTION(BlueprintCallable, Category = "Haptics")
	void PlayHapticFeedback(UHapticFeedbackEffect_Base* HapticEffect, float Intensity);

	/**
	 * Play grab haptic feedback
	 */
	UFUNCTION(BlueprintCallable, Category = "Haptics")
	void PlayGrabHaptics();

	/**
	 * Play detachment progress haptic feedback
	 */
	UFUNCTION(BlueprintCallable, Category = "Haptics")
	void PlayDetachmentHaptics(float Progress);

	/**
	 * Play harvest complete haptic feedback
	 */
	UFUNCTION(BlueprintCallable, Category = "Haptics")
	void PlayHarvestCompleteHaptics();

	// ============================================================================
	// VISUAL FEEDBACK FUNCTIONS
	// ============================================================================

	/**
	 * Update visual highlight based on state
	 */
	UFUNCTION(BlueprintCallable, Category = "Visual")
	void UpdateVisualFeedback();

	/**
	 * Show ready to harvest highlight
	 */
	UFUNCTION(BlueprintCallable, Category = "Visual")
	void ShowReadyHighlight();

	/**
	 * Show grabbed highlight
	 */
	UFUNCTION(BlueprintCallable, Category = "Visual")
	void ShowGrabbedHighlight();

	/**
	 * Hide highlight
	 */
	UFUNCTION(BlueprintCallable, Category = "Visual")
	void HideHighlight();

protected:
	// Check if hand is within grab radius
	bool IsHandInRange(AActor* Hand) const;

	// Get hand velocity for pull force calculation
	FVector GetHandVelocity(AActor* Hand) const;

	// Notify farm plot of harvest
	void NotifyFarmPlotHarvested();

	// Previous hand location for velocity calculation
	FVector PreviousHandLocation;

	// Time tracking
	float TimeSinceGrabbed;
};
