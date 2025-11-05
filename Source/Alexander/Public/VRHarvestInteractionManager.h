// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VRHarvestInteractionManager.generated.h"

// Forward declarations
class UVRHarvestableComponent;
class AFarmPlot;
class UHapticFeedbackEffect_Base;

/**
 * Harvest collection data
 */
USTRUCT(BlueprintType)
struct FHarvestCollection
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Harvest")
	FString CropName;

	UPROPERTY(BlueprintReadWrite, Category = "Harvest")
	int32 Quantity;

	UPROPERTY(BlueprintReadWrite, Category = "Harvest")
	float Quality;

	UPROPERTY(BlueprintReadWrite, Category = "Harvest")
	FDateTime HarvestTime;

	FHarvestCollection()
		: Quantity(0)
		, Quality(1.0f)
	{
	}
};

/**
 * VR Harvest Interaction Manager
 * Manages VR hand interactions with harvestable crops
 * Handles grab detection, harvest collection, and feedback
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UVRHarvestInteractionManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UVRHarvestInteractionManager();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// VR HAND TRACKING
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, Category = "VR")
	AActor* LeftHand;

	UPROPERTY(BlueprintReadOnly, Category = "VR")
	AActor* RightHand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR")
	float HandDetectionRadius; // Radius to detect harvestable crops

	// ============================================================================
	// INTERACTION STATE
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	UVRHarvestableComponent* LeftHandTarget;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	UVRHarvestableComponent* RightHandTarget;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bLeftHandGrabbing;

	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bRightHandGrabbing;

	// ============================================================================
	// HARVEST COLLECTION
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Collection")
	TArray<FHarvestCollection> HarvestHistory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collection")
	int32 MaxHistorySize; // Maximum harvest history entries

	UPROPERTY(BlueprintReadOnly, Category = "Collection")
	int32 TotalHarvestedToday;

	UPROPERTY(BlueprintReadOnly, Category = "Collection")
	float AverageQualityToday;

	// ============================================================================
	// FARM PLOT TRACKING
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Farm")
	TArray<AFarmPlot*> NearbyFarmPlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm")
	float FarmPlotDetectionRadius; // Radius to detect farm plots

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Initialize with VR hand actors
	 */
	UFUNCTION(BlueprintCallable, Category = "VR")
	void InitializeHands(AActor* InLeftHand, AActor* InRightHand);

	/**
	 * Register a farm plot for harvest tracking
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm")
	void RegisterFarmPlot(AFarmPlot* FarmPlot);

	/**
	 * Unregister a farm plot
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm")
	void UnregisterFarmPlot(AFarmPlot* FarmPlot);

	// ============================================================================
	// INTERACTION DETECTION
	// ============================================================================

	/**
	 * Update hand targets (find nearby harvestable crops)
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void UpdateHandTargets();

	/**
	 * Find harvestable crop near hand
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	UVRHarvestableComponent* FindHarvestableNearHand(AActor* Hand);

	/**
	 * Check if hand is making grab gesture
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool IsHandGrabbing(AActor* Hand) const;

	// ============================================================================
	// GRAB INTERACTION
	// ============================================================================

	/**
	 * Attempt to grab with left hand
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool TryGrabLeft();

	/**
	 * Attempt to grab with right hand
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool TryGrabRight();

	/**
	 * Release left hand grab
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ReleaseLeft();

	/**
	 * Release right hand grab
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ReleaseRight();

	/**
	 * Update grab interactions
	 */
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void UpdateGrabInteractions(float DeltaTime);

	// ============================================================================
	// HARVEST COLLECTION TRACKING
	// ============================================================================

	/**
	 * Record a harvest
	 */
	UFUNCTION(BlueprintCallable, Category = "Collection")
	void RecordHarvest(const FHarvestCollection& Harvest);

	/**
	 * Get total harvested amount for a crop type
	 */
	UFUNCTION(BlueprintCallable, Category = "Collection")
	int32 GetTotalHarvested(const FString& CropName) const;

	/**
	 * Get average quality for a crop type
	 */
	UFUNCTION(BlueprintCallable, Category = "Collection")
	float GetAverageQuality(const FString& CropName) const;

	/**
	 * Clear harvest history
	 */
	UFUNCTION(BlueprintCallable, Category = "Collection")
	void ClearHarvestHistory();

	/**
	 * Get harvest statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Collection")
	void GetHarvestStatistics(int32& OutTotalHarvested, float& OutAverageQuality) const;

	// ============================================================================
	// FARM PLOT QUERIES
	// ============================================================================

	/**
	 * Update nearby farm plots
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm")
	void UpdateNearbyFarmPlots();

	/**
	 * Get all harvestable crops in nearby farm plots
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm")
	TArray<UVRHarvestableComponent*> GetAllHarvestableCrops() const;

	/**
	 * Get number of ready crops in nearby plots
	 */
	UFUNCTION(BlueprintCallable, Category = "Farm")
	int32 GetReadyCropCount() const;

	// ============================================================================
	// VISUAL FEEDBACK
	// ============================================================================

	/**
	 * Show harvest UI feedback
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowHarvestFeedback(const FHarvestCollection& Harvest);

	/**
	 * Update hand proximity indicators
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateProximityIndicators();

protected:
	// Find all harvestable components in range
	TArray<UVRHarvestableComponent*> FindHarvestablesInRange(FVector Location, float Radius) const;

	// Get closest harvestable to location
	UVRHarvestableComponent* GetClosestHarvestable(FVector Location, const TArray<UVRHarvestableComponent*>& Harvestables) const;

	// Update daily statistics
	void UpdateDailyStatistics();

	// Check if it's a new day (reset daily stats)
	bool IsNewDay();

	// Last update time for daily stats
	FDateTime LastDayCheck;
};
