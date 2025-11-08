// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PowerSystem.h"
#include "HarvestingMachine.generated.h"

// Forward declarations
class AFarmPlot;
class UCropDefinition;
class UBoxComponent;
class UStaticMeshComponent;

/**
 * Harvested crop storage entry
 */
USTRUCT(BlueprintType)
struct FHarvestedCropEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Harvest")
	FString CropID;

	UPROPERTY(BlueprintReadWrite, Category = "Harvest")
	int32 Quantity;

	UPROPERTY(BlueprintReadWrite, Category = "Harvest")
	float TotalValue;

	FHarvestedCropEntry()
		: Quantity(0)
		, TotalValue(0.0f)
	{
	}
};

/**
 * Wrapper struct for TArray<FIntPoint> to work in TMap
 */
USTRUCT()
struct FFarmPlotCellArray
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FIntPoint> Cells;
};

/**
 * Harvesting machine configuration
 */
USTRUCT(BlueprintType)
struct FHarvestingMachineConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float HarvestRadius; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float HarvestInterval; // seconds between harvest cycles

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 MaxStorageCapacity; // maximum items stored

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bAutoHarvest; // automatically harvest when crops are ready

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bRequiresPower; // requires power to operate

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float PowerConsumption; // watts

	FHarvestingMachineConfig()
		: HarvestRadius(10.0f)
		, HarvestInterval(5.0f)
		, MaxStorageCapacity(1000)
		, bAutoHarvest(true)
		, bRequiresPower(true)
		, PowerConsumption(500.0f)
	{
	}
};

/**
 * Automated harvesting machine actor
 * Automatically collects mature crops within a designated area
 * Requires 750W of power to operate
 */
UCLASS()
class ALEXANDER_API AHarvestingMachine : public AActor, public IPowerConsumer
{
	GENERATED_BODY()

public:
	AHarvestingMachine();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ============================================================================
	// COMPONENTS
	// ============================================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* RootComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MachineMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* HarvestZone;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvesting")
	FHarvestingMachineConfig Config;

	// ============================================================================
	// STATE
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Harvesting")
	bool bIsActive;

	UPROPERTY(BlueprintReadOnly, Category = "Harvesting")
	bool bHasPower;

	UPROPERTY(BlueprintReadOnly, Category = "Harvesting")
	float TimeSinceLastHarvest;

	UPROPERTY(BlueprintReadOnly, Category = "Harvesting")
	int32 CurrentStorageCount;

	UPROPERTY(BlueprintReadOnly, Category = "Harvesting")
	TArray<FHarvestedCropEntry> StoredCrops;

	// ============================================================================
	// FARM PLOT TRACKING
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Harvesting")
	TArray<AFarmPlot*> ManagedPlots;

	// Plot cell mapping (C++ only due to TArray in TMap limitation)
	UPROPERTY()
	TMap<AFarmPlot*, FFarmPlotCellArray> PlotCellMap;

	// ============================================================================
	// STATISTICS
	// ============================================================================

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	int32 TotalCropsHarvested;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float TotalValueHarvested;

	UPROPERTY(BlueprintReadOnly, Category = "Statistics")
	float TotalPowerConsumed; // kWh

	// ============================================================================
	// HARVESTING OPERATIONS
	// ============================================================================

	/**
	 * Start automatic harvesting
	 */
	UFUNCTION(BlueprintCallable, Category = "Harvesting")
	void StartHarvesting();

	/**
	 * Stop automatic harvesting
	 */
	UFUNCTION(BlueprintCallable, Category = "Harvesting")
	void StopHarvesting();

	/**
	 * Perform a single harvest cycle
	 * @return Number of crops harvested
	 */
	UFUNCTION(BlueprintCallable, Category = "Harvesting")
	int32 PerformHarvestCycle();

	/**
	 * Harvest a specific crop cell
	 */
	UFUNCTION(BlueprintCallable, Category = "Harvesting")
	bool HarvestCropCell(AFarmPlot* Plot, FIntPoint GridPosition);

	/**
	 * Check if a crop is ready for harvest
	 */
	UFUNCTION(BlueprintCallable, Category = "Harvesting")
	bool IsCropReadyForHarvest(AFarmPlot* Plot, FIntPoint GridPosition) const;

	// ============================================================================
	// PLOT BOUNDARY DETECTION
	// ============================================================================

	/**
	 * Detect all farm plots within harvest radius
	 */
	UFUNCTION(BlueprintCallable, Category = "Harvesting")
	void DetectFarmPlots();

	/**
	 * Check if a location is within harvest radius
	 */
	UFUNCTION(BlueprintCallable, Category = "Harvesting")
	bool IsLocationInHarvestZone(FVector Location) const;

	/**
	 * Get all crop cells within harvest zone for a plot
	 */
	UFUNCTION(BlueprintCallable, Category = "Harvesting")
	TArray<FIntPoint> GetCropCellsInZone(AFarmPlot* Plot) const;

	/**
	 * Update managed plots and their cells
	 */
	UFUNCTION(BlueprintCallable, Category = "Harvesting")
	void UpdateManagedPlots();

	// ============================================================================
	// STORAGE SYSTEM
	// ============================================================================

	/**
	 * Add harvested crop to storage
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage")
	bool AddToStorage(const FString& CropID, int32 Quantity, float Value);

	/**
	 * Remove crops from storage
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage")
	bool RemoveFromStorage(const FString& CropID, int32 Quantity);

	/**
	 * Get stored quantity of a specific crop
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage")
	int32 GetStoredQuantity(const FString& CropID) const;

	/**
	 * Get total value of stored crops
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage")
	float GetTotalStoredValue() const;

	/**
	 * Check if storage is full
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage")
	bool IsStorageFull() const;

	/**
	 * Get storage fill percentage (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage")
	float GetStorageFillPercentage() const;

	/**
	 * Empty all storage
	 */
	UFUNCTION(BlueprintCallable, Category = "Storage")
	void EmptyStorage();

	// ============================================================================
	// POWER SYSTEM
	// ============================================================================

	/**
	 * Set power state
	 */
	UFUNCTION(BlueprintCallable, Category = "Power")
	void SetPowerState(bool bPowered);

	/**
	 * Get current power consumption
	 */
	UFUNCTION(BlueprintCallable, Category = "Power")
	float GetCurrentPowerConsumption() const;

	/**
	 * Check if machine has sufficient power
	 */
	UFUNCTION(BlueprintCallable, Category = "Power")
	bool HasSufficientPower() const;

	// ============================================================================
	// IPowerConsumer Interface
	// ============================================================================

	virtual float GetPowerRequirement_Implementation() const override;
	virtual void SetPowered_Implementation(bool bInIsPowered) override;
	virtual bool IsPowered_Implementation() const override;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	/**
	 * Set harvest radius
	 */
	UFUNCTION(BlueprintCallable, Category = "Configuration")
	void SetHarvestRadius(float Radius);

	/**
	 * Set harvest interval
	 */
	UFUNCTION(BlueprintCallable, Category = "Configuration")
	void SetHarvestInterval(float Interval);

	/**
	 * Set auto-harvest mode
	 */
	UFUNCTION(BlueprintCallable, Category = "Configuration")
	void SetAutoHarvest(bool bAuto);

	/**
	 * Get configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Configuration")
	FHarvestingMachineConfig GetConfiguration() const { return Config; }

	// ============================================================================
	// STATISTICS
	// ============================================================================

	/**
	 * Get harvest statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	void GetHarvestStatistics(int32& OutTotalHarvested, float& OutTotalValue, float& OutPowerUsed) const;

	/**
	 * Reset statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	void ResetStatistics();

protected:
	// Update harvest zone size
	void UpdateHarvestZoneSize();

	// Process automatic harvesting
	void ProcessAutoHarvest(float DeltaTime);

	// Update power consumption
	void UpdatePowerConsumption(float DeltaTime);

	// Find crop entry in storage
	FHarvestedCropEntry* FindStorageEntry(const FString& CropID);

	// Calculate world position of a crop cell
	FVector GetCropCellWorldPosition(AFarmPlot* Plot, FIntPoint GridPosition) const;
};
