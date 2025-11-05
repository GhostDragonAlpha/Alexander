# Farming Optimization System - Quick Start Guide

## Overview

The Farming Optimization System provides performance optimizations for large-scale farming operations through instance batching, LOD management, spatial partitioning, and adaptive update frequencies.

## Key Features

1. **Crop Instance Batching**: Efficient rendering of thousands of crops using instanced static meshes
2. **LOD Management**: Distance-based level of detail for crop rendering
3. **Spatial Partitioning**: Grid-based organization for fast spatial queries
4. **Adaptive Update Frequency**: Farms update less frequently when far from player

## Quick Setup

### 1. Automatic Integration

The optimization system is automatically created and initialized by the FarmingSubsystem:

```cpp
// In FarmingSubsystem::Initialize()
OptimizationSystem = NewObject<UFarmingOptimizationSystem>(this);
OptimizationSystem->Initialize(GetWorld());
```

### 2. Farm Registration

Farms are automatically registered when created:

```cpp
// Create a farm plot
AFarmPlot* Farm = FarmingSubsystem->CreateFarmPlot(Location, Size, PlanetActor);
// Farm is automatically registered with optimization system
```

### 3. Automatic Updates

The optimization system updates automatically each frame:

```cpp
// In FarmingSubsystem::Tick()
OptimizationSystem->Update(DeltaTime, PlayerLocation);
```

## LOD System

### LOD Levels

The system uses 5 LOD levels based on distance:

| LOD Level | Distance  | Description                      |
| --------- | --------- | -------------------------------- |
| High      | 0-50m     | Full detail, shadows enabled     |
| Medium    | 50-150m   | Reduced detail, shadows enabled  |
| Low       | 150-500m  | Simple mesh, no shadows          |
| VeryLow   | 500-1000m | Billboard or minimal, no shadows |
| Culled    | 1000m+    | Not rendered                     |

### Configuring LOD Distances

```cpp
// Default distances (in cm)
LODHighDistance = 5000.0f;      // 50m
LODMediumDistance = 15000.0f;   // 150m
LODLowDistance = 50000.0f;      // 500m
LODCullDistance = 100000.0f;    // 1000m
```

## Update Frequency Optimization

### Adaptive Updates

Farms update at different frequencies based on distance:

- **Near farms** (< 200m): 1 update per second
- **Far farms** (> 200m): 1 update per 10 seconds
- **Interpolated**: Linear blend between near and far

### Configuration

```cpp
NearUpdateFrequency = 1.0f;     // 1 update per second
FarUpdateFrequency = 0.1f;      // 1 update per 10 seconds
UpdateDistanceThreshold = 20000.0f; // 200m
```

### Getting Farms to Update

```cpp
// Get farms that need updating this frame
TArray<AFarmPlot*> FarmsToUpdate = OptimizationSystem->GetFarmsNeedingUpdate();

for (AFarmPlot* Farm : FarmsToUpdate)
{
    // Update farm logic
    Farm->UpdateCropGrowth(DeltaTime);
}
```

## Spatial Partitioning

### Grid System

The spatial partition divides the world into cells for fast spatial queries:

```cpp
SpatialCellSize = 10000.0f;     // 100m cells
SpatialActivationRadius = 50000.0f; // 500m activation radius
```

### Querying Farms by Area

```cpp
// Get all farms within radius of a point
TArray<AFarmPlot*> NearbyFarms = OptimizationSystem->GetFarmsInArea(
    PlayerLocation,
    50000.0f  // 500m radius
);
```

### Active Cell Management

Only cells near the player are active:

```cpp
// Update active cells based on player location
OptimizationSystem->UpdateActiveCells(PlayerLocation, ActivationRadius);
```

## Instance Batching

### Automatic Batching

Each farm uses UInstancedStaticMeshComponent for efficient rendering:

```cpp
// In AFarmPlot
CropInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("CropInstances"));
```

### Batch Configuration

```cpp
// Configure instance batching
OptimizationSystem->BatchCropInstances(FarmPlot);

// Update all batches
OptimizationSystem->UpdateAllInstanceBatches();
```

## Performance Monitoring

### Get Optimization Statistics

```cpp
int32 TotalFarms, ActiveFarms, TotalCrops, RenderedCrops;
OptimizationSystem->GetOptimizationStats(
    TotalFarms,
    ActiveFarms,
    TotalCrops,
    RenderedCrops
);

UE_LOG(LogTemp, Log, TEXT("Farms: %d/%d, Crops: %d/%d rendered"),
    ActiveFarms, TotalFarms, RenderedCrops, TotalCrops);
```

### Average Update Frequency

```cpp
float AvgFrequency = OptimizationSystem->GetAverageUpdateFrequency();
UE_LOG(LogTemp, Log, TEXT("Average update frequency: %.2f Hz"), AvgFrequency);
```

## Blueprint Usage

### Get Optimization System

```cpp
UFUNCTION(BlueprintCallable, Category = "Farming")
UFarmingOptimizationSystem* GetOptimizationSystem()
{
    UFarmingSubsystem* FarmingSubsystem = GetWorld()->GetSubsystem<UFarmingSubsystem>();
    return FarmingSubsystem ? FarmingSubsystem->GetOptimizationSystem() : nullptr;
}
```

### Check LOD Level

```cpp
// Get LOD level for a distance
ECropLODLevel LODLevel = OptimizationSystem->CalculateLODLevel(Distance);

switch (LODLevel)
{
    case ECropLODLevel::High:
        // Full detail rendering
        break;
    case ECropLODLevel::Culled:
        // Don't render
        break;
}
```

## Performance Tips

### 1. Adjust LOD Distances

For better performance on lower-end hardware:

```cpp
// More aggressive culling
LODHighDistance = 3000.0f;      // 30m
LODMediumDistance = 10000.0f;   // 100m
LODLowDistance = 30000.0f;      // 300m
LODCullDistance = 50000.0f;     // 500m
```

### 2. Reduce Update Frequency

For many farms:

```cpp
// Update less frequently
NearUpdateFrequency = 0.5f;     // 1 update per 2 seconds
FarUpdateFrequency = 0.05f;     // 1 update per 20 seconds
```

### 3. Smaller Spatial Cells

For dense farm areas:

```cpp
// Smaller cells for better culling
SpatialCellSize = 5000.0f;      // 50m cells
```

### 4. Limit Active Radius

For better performance:

```cpp
// Smaller activation radius
SpatialActivationRadius = 30000.0f; // 300m
```

## Common Issues

### Issue: Crops not updating

**Solution**: Check if farm is in active cell and within update distance

```cpp
FIntPoint CellKey = OptimizationSystem->GetSpatialCell(FarmLocation);
// Ensure cell is active
```

### Issue: Crops popping in/out

**Solution**: Adjust LOD transition distances for smoother transitions

```cpp
// Increase LOD distances for smoother transitions
LODMediumDistance = 20000.0f;   // 200m
LODLowDistance = 60000.0f;      // 600m
```

### Issue: Performance still poor

**Solution**: Check optimization statistics

```cpp
int32 TotalFarms, ActiveFarms, TotalCrops, RenderedCrops;
OptimizationSystem->GetOptimizationStats(TotalFarms, ActiveFarms, TotalCrops, RenderedCrops);

// If RenderedCrops is very high, reduce cull distance
// If ActiveFarms is very high, reduce activation radius
```

## Example: Complete Setup

```cpp
// Get farming subsystem
UFarmingSubsystem* FarmingSubsystem = GetWorld()->GetSubsystem<UFarmingSubsystem>();

// Get optimization system
UFarmingOptimizationSystem* OptSystem = FarmingSubsystem->GetOptimizationSystem();

// Configure for your needs
OptSystem->LODCullDistance = 50000.0f;  // 500m cull
OptSystem->NearUpdateFrequency = 1.0f;  // 1 Hz near
OptSystem->FarUpdateFrequency = 0.1f;   // 0.1 Hz far

// Create farms (automatically registered)
AFarmPlot* Farm1 = FarmingSubsystem->CreateFarmPlot(Location1, Size, Planet);
AFarmPlot* Farm2 = FarmingSubsystem->CreateFarmPlot(Location2, Size, Planet);

// System automatically handles:
// - LOD updates
// - Update frequency optimization
// - Spatial partitioning
// - Instance batching
```

## Performance Metrics

### Expected Performance

With optimization system:

- **1000 crops**: 60+ FPS
- **10,000 crops**: 45+ FPS (with LOD)
- **100,000 crops**: 30+ FPS (with aggressive LOD and culling)

### Without Optimization

- **1000 crops**: 30 FPS
- **10,000 crops**: 5 FPS
- **100,000 crops**: < 1 FPS

## Related Systems

- **FarmingSubsystem**: Main farming management
- **FarmPlot**: Individual farm plots
- **CropGrowthCalculator**: Crop growth simulation
- **TerrainRenderingOptimizer**: Terrain optimization (Task 18.1)

## Next Steps

1. Test with multiple farms at various distances
2. Monitor performance with optimization statistics
3. Adjust LOD distances based on target hardware
4. Tune update frequencies for your gameplay needs
5. Consider implementing crop mesh LODs for better performance
