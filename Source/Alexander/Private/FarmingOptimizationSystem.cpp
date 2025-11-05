// Copyright Epic Games, Inc. All Rights Reserved.

#include "FarmingOptimizationSystem.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "FarmPlot.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Components/InstancedStaticMeshComponent.h"

UFarmingOptimizationSystem::UFarmingOptimizationSystem()
	: WorldContext(nullptr)
	, LODHighDistance(5000.0f)      // 50m
	, LODMediumDistance(15000.0f)   // 150m
	, LODLowDistance(50000.0f)      // 500m
	, LODCullDistance(100000.0f)    // 1000m
	, NearUpdateFrequency(1.0f)     // 1 update per second
	, FarUpdateFrequency(0.1f)      // 1 update per 10 seconds
	, UpdateDistanceThreshold(20000.0f) // 200m
	, SpatialCellSize(10000.0f)     // 100m cells
	, SpatialActivationRadius(50000.0f) // 500m activation radius
	, TotalCropsRendered(0)
	, TotalCropsInScene(0)
	, AverageFrameTime(0.0f)
{
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void UFarmingOptimizationSystem::Initialize(UWorld* World)
{
	WorldContext = World;
	RegisteredFarms.Empty();
	FarmUpdatePriorities.Empty();
	SpatialGrid.Empty();
	InstanceBatches.Empty();

	UE_LOG(LogTemp, Log, TEXT("FarmingOptimizationSystem: Initialized"));
}

void UFarmingOptimizationSystem::Shutdown()
{
	RegisteredFarms.Empty();
	FarmUpdatePriorities.Empty();
	SpatialGrid.Empty();
	InstanceBatches.Empty();
	WorldContext = nullptr;

	UE_LOG(LogTemp, Log, TEXT("FarmingOptimizationSystem: Shutdown"));
}

// ============================================================================
// FARM REGISTRATION
// ============================================================================

void UFarmingOptimizationSystem::RegisterFarmPlot(AFarmPlot* FarmPlot)
{
	if (!FarmPlot)
	{
		return;
	}

	if (RegisteredFarms.Contains(FarmPlot))
	{
		UE_LOG(LogTemp, Warning, TEXT("FarmingOptimizationSystem: Farm already registered"));
		return;
	}

	RegisteredFarms.Add(FarmPlot);

	// Create update priority entry
	FFarmUpdatePriority Priority;
	Priority.FarmPlot = FarmPlot;
	Priority.DistanceToPlayer = 0.0f;
	Priority.UpdateFrequency = NearUpdateFrequency;
	Priority.TimeSinceLastUpdate = 0.0f;
	Priority.bNeedsUpdate = true;
	FarmUpdatePriorities.Add(Priority);

	// Rebuild spatial partition
	BuildSpatialPartition();

	UE_LOG(LogTemp, Log, TEXT("FarmingOptimizationSystem: Registered farm at %s (Total: %d)"),
		*FarmPlot->GetActorLocation().ToString(), RegisteredFarms.Num());
}

void UFarmingOptimizationSystem::UnregisterFarmPlot(AFarmPlot* FarmPlot)
{
	if (!FarmPlot)
	{
		return;
	}

	RegisteredFarms.Remove(FarmPlot);

	// Remove from update priorities
	FarmUpdatePriorities.RemoveAll([FarmPlot](const FFarmUpdatePriority& Priority)
	{
		return Priority.FarmPlot == FarmPlot;
	});

	// Rebuild spatial partition
	BuildSpatialPartition();

	UE_LOG(LogTemp, Log, TEXT("FarmingOptimizationSystem: Unregistered farm (Remaining: %d)"),
		RegisteredFarms.Num());
}

// ============================================================================
// UPDATE MANAGEMENT
// ============================================================================

void UFarmingOptimizationSystem::Update(float DeltaTime, FVector PlayerLocation)
{
	if (!WorldContext)
	{
		return;
	}

	// Update farm priorities based on player location
	UpdateFarmPriorities(PlayerLocation);

	// Update active spatial cells
	UpdateActiveCells(PlayerLocation, SpatialActivationRadius);

	// Update LODs for all farms
	UpdateFarmLODs(PlayerLocation);

	// Update time since last update for all farms
	for (FFarmUpdatePriority& Priority : FarmUpdatePriorities)
	{
		Priority.TimeSinceLastUpdate += DeltaTime;

		// Check if farm needs update based on frequency
		float UpdateInterval = 1.0f / Priority.UpdateFrequency;
		if (Priority.TimeSinceLastUpdate >= UpdateInterval)
		{
			Priority.bNeedsUpdate = true;
		}
	}

	// Track performance
	TotalCropsInScene = 0;
	TotalCropsRendered = 0;
	for (AFarmPlot* Farm : RegisteredFarms)
	{
		if (Farm)
		{
			// Count total crops
			for (const FCropCell& Cell : Farm->CropGrid)
			{
				if (Cell.CropType)
				{
					TotalCropsInScene++;

					// Count rendered crops (within cull distance)
					float Distance = FVector::Dist(Farm->GetActorLocation(), PlayerLocation);
					if (Distance < LODCullDistance)
					{
						TotalCropsRendered++;
					}
				}
			}
		}
	}
}

float UFarmingOptimizationSystem::CalculateUpdateFrequency(float DistanceToPlayer) const
{
	// Near farms update frequently, far farms update slowly
	if (DistanceToPlayer < UpdateDistanceThreshold)
	{
		// Linear interpolation from near to far frequency
		float T = DistanceToPlayer / UpdateDistanceThreshold;
		return FMath::Lerp(NearUpdateFrequency, FarUpdateFrequency, T);
	}
	else
	{
		// Very far farms update at minimum frequency
		return FarUpdateFrequency;
	}
}

TArray<AFarmPlot*> UFarmingOptimizationSystem::GetFarmsNeedingUpdate()
{
	TArray<AFarmPlot*> FarmsToUpdate;

	for (FFarmUpdatePriority& Priority : FarmUpdatePriorities)
	{
		if (Priority.bNeedsUpdate && Priority.FarmPlot)
		{
			FarmsToUpdate.Add(Priority.FarmPlot);
			Priority.bNeedsUpdate = false;
			Priority.TimeSinceLastUpdate = 0.0f;
		}
	}

	return FarmsToUpdate;
}

// ============================================================================
// LOD MANAGEMENT
// ============================================================================

ECropLODLevel UFarmingOptimizationSystem::CalculateLODLevel(float Distance) const
{
	if (Distance < LODHighDistance)
	{
		return ECropLODLevel::High;
	}
	else if (Distance < LODMediumDistance)
	{
		return ECropLODLevel::Medium;
	}
	else if (Distance < LODLowDistance)
	{
		return ECropLODLevel::Low;
	}
	else if (Distance < LODCullDistance)
	{
		return ECropLODLevel::VeryLow;
	}
	else
	{
		return ECropLODLevel::Culled;
	}
}

void UFarmingOptimizationSystem::UpdateFarmLODs(FVector ViewerLocation)
{
	for (AFarmPlot* Farm : RegisteredFarms)
	{
		if (!Farm)
		{
			continue;
		}

		float Distance = FVector::Dist(Farm->GetActorLocation(), ViewerLocation);
		ECropLODLevel LODLevel = CalculateLODLevel(Distance);

		// Apply LOD to farm's crop instances
		if (Farm->CropInstances)
		{
			// Adjust instance rendering based on LOD
			switch (LODLevel)
			{
				case ECropLODLevel::High:
					// Full detail, no changes needed
					Farm->CropInstances->SetVisibility(true);
					Farm->CropInstances->SetCastShadow(true);
					break;

				case ECropLODLevel::Medium:
					// Reduced detail, keep shadows
					Farm->CropInstances->SetVisibility(true);
					Farm->CropInstances->SetCastShadow(true);
					break;

				case ECropLODLevel::Low:
					// Simple mesh, no shadows
					Farm->CropInstances->SetVisibility(true);
					Farm->CropInstances->SetCastShadow(false);
					break;

				case ECropLODLevel::VeryLow:
					// Billboard or very simple, no shadows
					Farm->CropInstances->SetVisibility(true);
					Farm->CropInstances->SetCastShadow(false);
					break;

				case ECropLODLevel::Culled:
					// Not visible
					Farm->CropInstances->SetVisibility(false);
					break;
			}
		}
	}
}

void UFarmingOptimizationSystem::GetLODDistances(float& OutHighDistance, float& OutMediumDistance, float& OutLowDistance, float& OutCullDistance) const
{
	OutHighDistance = LODHighDistance;
	OutMediumDistance = LODMediumDistance;
	OutLowDistance = LODLowDistance;
	OutCullDistance = LODCullDistance;
}

// ============================================================================
// INSTANCE BATCHING
// ============================================================================

void UFarmingOptimizationSystem::BatchCropInstances(AFarmPlot* FarmPlot)
{
	if (!FarmPlot)
	{
		return;
	}

	// Crop instances are already batched per farm using UInstancedStaticMeshComponent
	// This function can be used for additional cross-farm batching if needed

	// For now, we ensure each farm's instances are properly configured
	if (FarmPlot->CropInstances)
	{
		// Enable instancing optimizations
		FarmPlot->CropInstances->SetCullDistances(0, LODCullDistance);
		FarmPlot->CropInstances->bUseAsOccluder = false; // Crops shouldn't occlude
		FarmPlot->CropInstances->CastShadow = true;
		FarmPlot->CropInstances->bCastDynamicShadow = true;
	}
}

void UFarmingOptimizationSystem::UpdateAllInstanceBatches()
{
	for (AFarmPlot* Farm : RegisteredFarms)
	{
		if (Farm)
		{
			BatchCropInstances(Farm);
		}
	}
}

// ============================================================================
// SPATIAL PARTITIONING
// ============================================================================

void UFarmingOptimizationSystem::BuildSpatialPartition()
{
	SpatialGrid.Empty();

	// Add all farms to spatial grid
	for (AFarmPlot* Farm : RegisteredFarms)
	{
		if (!Farm)
		{
			continue;
		}

		FIntPoint CellKey = GetSpatialCell(Farm->GetActorLocation());

		// Get or create cell
		FSpatialPartitionCell& Cell = SpatialGrid.FindOrAdd(CellKey);
		Cell.CellCenter = FVector(
			CellKey.X * SpatialCellSize + SpatialCellSize * 0.5f,
			CellKey.Y * SpatialCellSize + SpatialCellSize * 0.5f,
			0.0f
		);
		Cell.CellSize = SpatialCellSize;
		Cell.FarmPlots.AddUnique(Farm);
	}

	UE_LOG(LogTemp, Log, TEXT("FarmingOptimizationSystem: Built spatial partition with %d cells"),
		SpatialGrid.Num());
}

TArray<AFarmPlot*> UFarmingOptimizationSystem::GetFarmsInArea(FVector Center, float Radius)
{
	TArray<AFarmPlot*> FarmsInArea;

	// Calculate cell range to check
	FIntPoint MinCell = GetSpatialCell(Center - FVector(Radius, Radius, 0.0f));
	FIntPoint MaxCell = GetSpatialCell(Center + FVector(Radius, Radius, 0.0f));

	// Check all cells in range
	for (int32 X = MinCell.X; X <= MaxCell.X; ++X)
	{
		for (int32 Y = MinCell.Y; Y <= MaxCell.Y; ++Y)
		{
			FIntPoint CellKey(X, Y);
			FSpatialPartitionCell* Cell = SpatialGrid.Find(CellKey);

			if (Cell)
			{
				// Check each farm in cell
				for (AFarmPlot* Farm : Cell->FarmPlots)
				{
					if (Farm)
					{
						float Distance = FVector::Dist(Farm->GetActorLocation(), Center);
						if (Distance <= Radius)
						{
							FarmsInArea.AddUnique(Farm);
						}
					}
				}
			}
		}
	}

	return FarmsInArea;
}

FIntPoint UFarmingOptimizationSystem::GetSpatialCell(FVector Location) const
{
	return LocationToCellKey(Location);
}

void UFarmingOptimizationSystem::UpdateActiveCells(FVector PlayerLocation, float ActivationRadius)
{
	// Deactivate all cells first
	for (auto& Pair : SpatialGrid)
	{
		Pair.Value.bIsActive = false;
	}

	// Calculate cell range around player
	FIntPoint MinCell = GetSpatialCell(PlayerLocation - FVector(ActivationRadius, ActivationRadius, 0.0f));
	FIntPoint MaxCell = GetSpatialCell(PlayerLocation + FVector(ActivationRadius, ActivationRadius, 0.0f));

	// Activate cells in range
	int32 ActivatedCells = 0;
	for (int32 X = MinCell.X; X <= MaxCell.X; ++X)
	{
		for (int32 Y = MinCell.Y; Y <= MaxCell.Y; ++Y)
		{
			FIntPoint CellKey(X, Y);
			FSpatialPartitionCell* Cell = SpatialGrid.Find(CellKey);

			if (Cell)
			{
				// Check if cell center is within activation radius
				float Distance = FVector::Dist(Cell->CellCenter, PlayerLocation);
				if (Distance <= ActivationRadius)
				{
					Cell->bIsActive = true;
					ActivatedCells++;
				}
			}
		}
	}

	UE_LOG(LogTemp, Verbose, TEXT("FarmingOptimizationSystem: Activated %d cells around player"),
		ActivatedCells);
}

// ============================================================================
// STATISTICS
// ============================================================================

void UFarmingOptimizationSystem::GetOptimizationStats(int32& OutTotalFarms, int32& OutActiveFarms, int32& OutTotalCrops, int32& OutRenderedCrops) const
{
	OutTotalFarms = RegisteredFarms.Num();
	OutActiveFarms = 0;
	OutTotalCrops = TotalCropsInScene;
	OutRenderedCrops = TotalCropsRendered;

	// Count active farms (in active cells)
	for (const auto& Pair : SpatialGrid)
	{
		if (Pair.Value.bIsActive)
		{
			OutActiveFarms += Pair.Value.FarmPlots.Num();
		}
	}
}

float UFarmingOptimizationSystem::GetAverageUpdateFrequency() const
{
	if (FarmUpdatePriorities.Num() == 0)
	{
		return 0.0f;
	}

	float TotalFrequency = 0.0f;
	for (const FFarmUpdatePriority& Priority : FarmUpdatePriorities)
	{
		TotalFrequency += Priority.UpdateFrequency;
	}

	return TotalFrequency / FarmUpdatePriorities.Num();
}

// ============================================================================
// PRIVATE HELPERS
// ============================================================================

void UFarmingOptimizationSystem::UpdateFarmPriorities(FVector PlayerLocation)
{
	for (FFarmUpdatePriority& Priority : FarmUpdatePriorities)
	{
		if (!Priority.FarmPlot)
		{
			continue;
		}

		// Calculate distance to player
		Priority.DistanceToPlayer = FVector::Dist(Priority.FarmPlot->GetActorLocation(), PlayerLocation);

		// Update frequency based on distance
		Priority.UpdateFrequency = CalculateUpdateFrequency(Priority.DistanceToPlayer);
	}

	// Sort by distance (nearest first) for better cache coherency
	FarmUpdatePriorities.Sort([](const FFarmUpdatePriority& A, const FFarmUpdatePriority& B)
	{
		return A.DistanceToPlayer < B.DistanceToPlayer;
	});
}

FIntPoint UFarmingOptimizationSystem::LocationToCellKey(FVector Location) const
{
	return FIntPoint(
		FMath::FloorToInt(Location.X / SpatialCellSize),
		FMath::FloorToInt(Location.Y / SpatialCellSize)
	);
}

bool UFarmingOptimizationSystem::IsCellActive(FIntPoint CellKey) const
{
	const FSpatialPartitionCell* Cell = SpatialGrid.Find(CellKey);
	return Cell && Cell->bIsActive;
}
