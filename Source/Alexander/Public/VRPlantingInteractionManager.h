// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VRPlantingInteractionManager.generated.h"

// Forward declarations
class AVRSeedItem;
class AFarmPlot;
class UVRPlantingGestureDetector;
class UCropDefinition;
class UHapticFeedbackEffect_Base;

/**
 * Haptic feedback configuration for planting
 */
USTRUCT(BlueprintType)
struct FPlantingHapticConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptics")
	UHapticFeedbackEffect_Base* PlantingHapticEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptics")
	float PlantingIntensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptics")
	float GrabIntensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Haptics")
	float InvalidPlacementIntensity;

	FPlantingHapticConfig()
		: PlantingHapticEffect(nullptr)
		, PlantingIntensity(0.7f)
		, GrabIntensity(0.4f)
		, InvalidPlacementIntensity(0.3f)
	{
	}
};

/**
 * Manager component for VR planting interactions
 * Coordinates seed grabbing, gesture detection, and planting operations
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UVRPlantingInteractionManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRPlantingInteractionManager();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// SEED INTERACTION
	// ============================================================================

	/**
	 * Spawn a seed item for planting
	 */
	UFUNCTION(BlueprintCallable, Category = "Planting")
	AVRSeedItem* SpawnSeedItem(UCropDefinition* CropType, FVector Location, int32 SeedCount = 1);

	/**
	 * Handle seed grab by VR hand
	 */
	UFUNCTION(BlueprintCallable, Category = "Planting")
	void OnSeedGrabbed(AVRSeedItem* SeedItem, AActor* HandActor);

	/**
	 * Handle seed release by VR hand
	 */
	UFUNCTION(BlueprintCallable, Category = "Planting")
	void OnSeedReleased(AVRSeedItem* SeedItem);

	/**
	 * Get currently held seed item
	 */
	UFUNCTION(BlueprintCallable, Category = "Planting")
	AVRSeedItem* GetHeldSeedItem() const { return CurrentSeedItem; }

	// ============================================================================
	// PLANTING OPERATIONS
	// ============================================================================

	/**
	 * Attempt to plant seed at location
	 */
	UFUNCTION(BlueprintCallable, Category = "Planting")
	bool PlantSeedAtLocation(AVRSeedItem* SeedItem, FVector Location);

	/**
	 * Attempt to plant seed in grid
	 */
	UFUNCTION(BlueprintCallable, Category = "Planting")
	bool PlantSeedInGrid(AVRSeedItem* SeedItem, AFarmPlot* FarmPlot, FIntPoint GridPosition);

	/**
	 * Check if location is valid for planting
	 */
	UFUNCTION(BlueprintCallable, Category = "Planting")
	bool IsValidPlantingLocation(FVector Location, AFarmPlot*& OutFarmPlot, FIntPoint& OutGridPosition);

	// ============================================================================
	// HAPTIC FEEDBACK
	// ============================================================================

	/**
	 * Play haptic feedback for successful planting
	 */
	UFUNCTION(BlueprintCallable, Category = "Planting|Haptics")
	void PlayPlantingHaptics(AActor* HandActor);

	/**
	 * Play haptic feedback for grabbing seed
	 */
	UFUNCTION(BlueprintCallable, Category = "Planting|Haptics")
	void PlayGrabHaptics(AActor* HandActor);

	/**
	 * Play haptic feedback for invalid placement
	 */
	UFUNCTION(BlueprintCallable, Category = "Planting|Haptics")
	void PlayInvalidPlacementHaptics(AActor* HandActor);

	/**
	 * Get controller hand from hand actor
	 */
	UFUNCTION(BlueprintCallable, Category = "Planting|Haptics")
	EControllerHand GetControllerHand(AActor* HandActor);

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planting")
	TSubclassOf<AVRSeedItem> SeedItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planting")
	FPlantingHapticConfig HapticConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planting")
	bool bEnableGestureDetection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planting")
	bool bAutoPlantOnRelease;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planting")
	float PlantingRange;

	// ============================================================================
	// VISUAL FEEDBACK
	// ============================================================================

	/**
	 * Show planting preview at location
	 */
	UFUNCTION(BlueprintCallable, Category = "Planting|Visual")
	void ShowPlantingPreview(FVector Location, bool bIsValid);

	/**
	 * Hide planting preview
	 */
	UFUNCTION(BlueprintCallable, Category = "Planting|Visual")
	void HidePlantingPreview();

	/**
	 * Update planting preview position
	 */
	UFUNCTION(BlueprintCallable, Category = "Planting|Visual")
	void UpdatePlantingPreview(FVector Location);

protected:
	// Current seed item being held
	UPROPERTY()
	AVRSeedItem* CurrentSeedItem;

	// Current hand actor holding seed
	UPROPERTY()
	AActor* CurrentHandActor;

	// Gesture detector component
	UPROPERTY()
	UVRPlantingGestureDetector* GestureDetector;

	// Planting preview actor
	UPROPERTY()
	AActor* PlantingPreviewActor;

	// ============================================================================
	// HELPER FUNCTIONS
	// ============================================================================

	// Initialize gesture detector
	void InitializeGestureDetector();

	// Create planting preview actor
	void CreatePlantingPreview();

	// Find farm plot at location
	AFarmPlot* FindFarmPlotAtLocation(FVector Location);

	// Calculate grid position from world location
	FIntPoint CalculateGridPosition(AFarmPlot* FarmPlot, FVector WorldLocation);

	// Play haptic effect on controller
	void PlayHapticEffect(AActor* HandActor, float Intensity, float Duration = 0.2f);
};
