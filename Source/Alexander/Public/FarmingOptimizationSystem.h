// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FarmingOptimizationSystem.generated.h"

// Forward declarations
class AFarmPlot;
class UInstancedStaticMeshComponent;

/**
 * LOD level for crop rendering
 */
UENUM(BlueprintType)
enum class ECropLODLevel : uint8
{
	High = 0,      // 0-50m: Full detail
	Medium = 1,    // 50-150m: Reduced detail
	Low = 2,       // 150-500m: Simple mesh
	VeryLow = 3,   // 500m+: Billboard or culled
	Culled = 4     // Too far to render
};

/**
 * Spatial partition cell for farm plots
 */
USTRUCT()
struct FSpatialPartitionCell
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<AFarmPlot*> FarmPlots;

	UPROPERTY()
	FVector CellCenter;

	UPROPERTY()
	float CellSize;

	UPROPERTY()
	bool bIsActive;

	FSpatialPartitionCell()
		: CellSize(10000.0f) // 100m cells
		, bIsActive(false)
	{
	}
};

/**
 * Crop instance batch for efficient rendering
 */
USTRUCT()
struct FCropInstanceBatch
{
	GENERATED_BODY()

	UPROPERTY()
	UStaticMesh* CropMesh;

	UPROPERTY()
	TArray<FTransform> Transforms;

	UPROPERTY()
	TArray<FLinearColor> Colors;

	UPROPERTY()
	ECropLODLevel LODLevel;

	FCropInstanceBatch()
		: CropMesh(nullptr)
		, LODLevel(ECropLODLevel::High)
	{
	}
};

/**
 * Farm update priority based on distance and activity
 */
USTRUCT()
struct FFarmUpdatePriority
{
	GENERATED_BODY()

	UPROPERTY()
	AFarmPlot* FarmPlot;

	UPROPERTY()
	float DistanceToPlayer;

	UPROPERTY()
	float UpdateFrequency; // Seconds between updates

	UPROPERTY()
	float TimeSinceLastUpdate;

	UPROPERTY()
	bool bNeedsUpdate;

	FFarmUpdatePriority()
		: FarmPlot(nullptr)
		, DistanceToPlayer(0.0f)
		, UpdateFrequency(1.0f)
		, TimeSinceLastUpdate(0.0f)
		, bNeedsUpdate(true)
	{
	}
};

/**
 * Optimization system for farming operations
 * Handles instance batching, LOD management, and spatial partitioning
 */
UCLASS()
class ALEXANDER_API UFarmingOptimizationSystem : public UObject
{
	GENERATED_BODY()

public:
	UFarmingOptimizationSystem();

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Initialize the optimization system
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Optimization")
	void Initialize(UWorld* World);

	/**
	 * Shutdown and cleanup
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Optimization")
	void Shutdown();

	// ============================================================================
	// FARM REGISTRATION
	// ============================================================================

	/**
	 * Register a farm plot for optimization
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Optimization")
	void RegisterFarmPlot(AFarmPlot* FarmPlot);

	/**
	 * Unregister a farm plot
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Optimization")
	void UnregisterFarmPlot(AFarmPlot* FarmPlot);

	// ============================================================================
	// UPDATE MANAGEMENT
	// ============================================================================

	/**
	 * Update optimization system (called per frame)
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Optimization")
	void Update(float DeltaTime, FVector PlayerLocation);

	/**
	 * Calculate update frequency for a farm based on distance
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Optimization")
	float CalculateUpdateFrequency(float DistanceToPlayer) const;

	/**
	 * Get farms that need updating this frame
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Optimization")
	TArray<AFarmPlot*> GetFarmsNeedingUpdate();

	// ============================================================================
	// LOD MANAGEMENT
	// ============================================================================

	/**
	 * Calculate LOD level based on distance
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Optimization")
	ECropLODLevel CalculateLODLevel(float Distance) const;

	/**
	 * Update LOD for all registered farms
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Optimization")
	void UpdateFarmLODs(FVector ViewerLocation);

	/**
	 * Get LOD distance thresholds
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Optimization")
	void GetLODDistances(float& OutHighDistance, float& OutMediumDistance, float& OutLowDistance, float& OutCullDistance) const;

	// ============================================================================
	// INSTANCE BATCHING
	// ============================================================================

	/**
	 * Batch crop instances for efficient rendering
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Optimization")
	void BatchCropInstances(AFarmPlot* FarmPlot);

	/**
	 * Update all instance batches
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Optimization")
	void UpdateAllInstanceBatches();

	// ============================================================================
	// SPATIAL PARTITIONING
	// ============================================================================

	/**
	 * Build spatial partition grid
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Optimization")
	void BuildSpatialPartition();

	/**
	 * Get farms in a specific area
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Optimization")
	TArray<AFarmPlot*> GetFarmsInArea(FVector Center, float Radius);

	/**
	 * Get spatial cell for a location
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Optimization")
	FIntPoint GetSpatialCell(FVector Location) const;

	/**
	 * Activate/deactivate spatial cells based on player location
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Optimization")
	void UpdateActiveCells(FVector PlayerLocation, float ActivationRadius);

	// ============================================================================
	// STATISTICS
	// ============================================================================

	/**
	 * Get optimization statistics
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Optimization")
	void GetOptimizationStats(int32& OutTotalFarms, int32& OutActiveFarms, int32& OutTotalCrops, int32& OutRenderedCrops) const;

	/**
	 * Get average update frequency
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Optimization")
	float GetAverageUpdateFrequency() const;

protected:
	// World reference
	UPROPERTY()
	UWorld* WorldContext;

	// Registered farm plots
	UPROPERTY()
	TArray<AFarmPlot*> RegisteredFarms;

	// Farm update priorities
	UPROPERTY()
	TArray<FFarmUpdatePriority> FarmUpdatePriorities;

	// Spatial partition grid
	UPROPERTY()
	TMap<FIntPoint, FSpatialPartitionCell> SpatialGrid;

	// Instance batches for rendering
	UPROPERTY()
	TArray<FCropInstanceBatch> InstanceBatches;

	// LOD distance thresholds (in cm)
	UPROPERTY(EditDefaultsOnly, Category = "LOD")
	float LODHighDistance;

	UPROPERTY(EditDefaultsOnly, Category = "LOD")
	float LODMediumDistance;

	UPROPERTY(EditDefaultsOnly, Category = "LOD")
	float LODLowDistance;

	UPROPERTY(EditDefaultsOnly, Category = "LOD")
	float LODCullDistance;

	// Update frequency settings
	UPROPERTY(EditDefaultsOnly, Category = "Update")
	float NearUpdateFrequency; // Updates per second for nearby farms

	UPROPERTY(EditDefaultsOnly, Category = "Update")
	float FarUpdateFrequency; // Updates per second for distant farms

	UPROPERTY(EditDefaultsOnly, Category = "Update")
	float UpdateDistanceThreshold; // Distance threshold for frequency change

	// Spatial partition settings
	UPROPERTY(EditDefaultsOnly, Category = "Spatial")
	float SpatialCellSize; // Size of each spatial cell in cm

	UPROPERTY(EditDefaultsOnly, Category = "Spatial")
	float SpatialActivationRadius; // Radius around player to activate cells

	// Performance tracking
	UPROPERTY()
	int32 TotalCropsRendered;

	UPROPERTY()
	int32 TotalCropsInScene;

	UPROPERTY()
	float AverageFrameTime;

private:
	// Helper to update farm priorities
	void UpdateFarmPriorities(FVector PlayerLocation);

	// Helper to get cell key from location
	FIntPoint LocationToCellKey(FVector Location) const;

	// Helper to check if a cell is active
	bool IsCellActive(FIntPoint CellKey) const;
};
