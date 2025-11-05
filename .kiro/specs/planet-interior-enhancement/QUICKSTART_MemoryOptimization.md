# Memory Optimization System - Quick Start Guide

## Overview

The Memory Optimization System manages texture streaming, mesh LOD, and tile cache to keep total memory usage under 8GB for VR performance.

## Key Features

- **Texture Streaming**: Automatically loads/unloads textures based on distance and priority
- **Mesh LOD System**: Dynamically adjusts mesh detail levels based on viewer distance
- **Tile Cache Optimization**: LRU cache for terrain tiles with configurable size limits
- **Memory Monitoring**: Real-time tracking of memory usage across all systems
- **Auto Optimization**: Automatic memory management to stay within budget

## Quick Setup

### 1. Get the Subsystem

```cpp
UMemoryOptimizationSystem* MemorySystem = GetWorld()->GetSubsystem<UMemoryOptimizationSystem>();
```

### 2. Configure Memory Budget (Optional)

```cpp
FMemoryBudgetConfig Budget;
Budget.MaxTotalMemoryMB = 8192;      // 8GB total
Budget.MaxTextureMemoryMB = 4096;    // 4GB for textures
Budget.MaxMeshMemoryMB = 2048;       // 2GB for meshes
Budget.MaxTileCacheMB = 1024;        // 1GB for tile cache
Budget.MaxActiveTiles = 500;

MemorySystem->SetMemoryBudget(Budget);
```

### 3. Register Mesh Components

```cpp
// Register meshes for LOD management
MemorySystem->RegisterMeshComponent(MyStaticMeshComponent);
```

### 4. Update Each Frame

```cpp
void AMyActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector ViewerLocation = GetActorLocation();
    FVector ViewerDirection = GetActorForwardVector();

    // Update texture streaming
    MemorySystem->UpdateTextureStreaming(ViewerLocation, ViewerDirection);

    // Update mesh LODs
    MemorySystem->UpdateMeshLODs(ViewerLocation);
}
```

## Texture Streaming

### Set Texture Priority

```cpp
// Critical textures (player immediate vicinity)
MemorySystem->SetTextureStreamingPriority(MyTexture, ETextureStreamingPriority::Critical);

// High priority (visible and close)
MemorySystem->SetTextureStreamingPriority(MyTexture, ETextureStreamingPriority::High);

// Medium priority (visible but distant)
MemorySystem->SetTextureStreamingPriority(MyTexture, ETextureStreamingPriority::Medium);

// Low priority (just outside view)
MemorySystem->SetTextureStreamingPriority(MyTexture, ETextureStreamingPriority::Low);

// Very low priority (far from player)
MemorySystem->SetTextureStreamingPriority(MyTexture, ETextureStreamingPriority::VeryLow);
```

### Force Stream Texture

```cpp
// Force a texture to full resolution immediately
MemorySystem->ForceStreamTexture(ImportantTexture);
```

### Unload Distant Textures

```cpp
// Unload textures beyond 10km
MemorySystem->UnloadDistantTextures(10000.0f);
```

## Mesh LOD System

### Configure LOD Distances

```cpp
FMeshLODConfig LODConfig;
LODConfig.LOD0Distance = 500.0f;      // 0-500m: Highest detail
LODConfig.LOD1Distance = 2000.0f;     // 500m-2km
LODConfig.LOD2Distance = 10000.0f;    // 2km-10km
LODConfig.LOD3Distance = 50000.0f;    // 10km-50km
LODConfig.MaxLODLevel = 5;            // Maximum LOD level

MemorySystem->SetMeshLODConfig(LODConfig);
```

### Calculate LOD Level

```cpp
FVector MeshLocation = MyMesh->GetComponentLocation();
FVector ViewerLocation = PlayerCamera->GetComponentLocation();

int32 LODLevel = MemorySystem->CalculateLODLevel(MeshLocation, ViewerLocation);
// Returns 0-5 based on distance
```

### Register/Unregister Meshes

```cpp
// Register for automatic LOD management
MemorySystem->RegisterMeshComponent(MyMeshComponent);

// Unregister when no longer needed
MemorySystem->UnregisterMeshComponent(MyMeshComponent);
```

## Tile Cache Management

### Optimize Cache

```cpp
// Manually trigger cache optimization
MemorySystem->OptimizeTileCache();
```

### Set Cache Size

```cpp
// Set maximum cache size to 1GB
MemorySystem->SetMaxTileCacheSize(1024);
```

### Clear Cache

```cpp
// Clear entire tile cache
MemorySystem->ClearTileCache();
```

### Evict Tiles

```cpp
// Evict 10 least recently used tiles
MemorySystem->EvictLeastRecentlyUsedTiles(10);
```

## Memory Monitoring

### Get Overall Stats

```cpp
FMemoryStats Stats = MemorySystem->GetMemoryStats();

UE_LOG(LogTemp, Log, TEXT("Total Memory: %.2f MB"), Stats.TotalMemoryMB);
UE_LOG(LogTemp, Log, TEXT("Texture Memory: %.2f MB"), Stats.TextureMemoryMB);
UE_LOG(LogTemp, Log, TEXT("Mesh Memory: %.2f MB"), Stats.MeshMemoryMB);
UE_LOG(LogTemp, Log, TEXT("Tile Cache: %.2f MB"), Stats.TileCacheMemoryMB);
UE_LOG(LogTemp, Log, TEXT("Within Budget: %s"), Stats.bWithinBudget ? TEXT("Yes") : TEXT("No"));
```

### Get Texture Streaming Stats

```cpp
FTextureStreamingStats TexStats = MemorySystem->GetTextureStreamingStats();

UE_LOG(LogTemp, Log, TEXT("Streamed Textures: %d"), TexStats.StreamedTextures);
UE_LOG(LogTemp, Log, TEXT("Resident Textures: %d"), TexStats.ResidentTextures);
UE_LOG(LogTemp, Log, TEXT("Streaming Bandwidth: %.2f MB/s"), TexStats.StreamingBandwidthMBps);
```

### Get Mesh LOD Stats

```cpp
FMeshLODStats MeshStats = MemorySystem->GetMeshLODStats();

UE_LOG(LogTemp, Log, TEXT("Total Meshes: %d"), MeshStats.TotalMeshes);
UE_LOG(LogTemp, Log, TEXT("Visible Triangles: %d"), MeshStats.VisibleTriangles);

for (const auto& Pair : MeshStats.MeshesByLOD)
{
    UE_LOG(LogTemp, Log, TEXT("LOD %d: %d meshes"), Pair.Key, Pair.Value);
}
```

### Get Tile Cache Stats

```cpp
FTileCacheStats TileStats = MemorySystem->GetTileCacheStats();

UE_LOG(LogTemp, Log, TEXT("Active Tiles: %d"), TileStats.ActiveTiles);
UE_LOG(LogTemp, Log, TEXT("Cache Hits: %d"), TileStats.CacheHits);
UE_LOG(LogTemp, Log, TEXT("Cache Misses: %d"), TileStats.CacheMisses);
UE_LOG(LogTemp, Log, TEXT("Hit Rate: %.2f%%"), TileStats.HitRate * 100.0f);
```

### Check Budget

```cpp
if (!MemorySystem->IsWithinMemoryBudget())
{
    UE_LOG(LogTemp, Warning, TEXT("Memory budget exceeded!"));
    MemorySystem->PerformMemoryOptimization();
}
```

### Log All Stats

```cpp
// Log comprehensive memory statistics
MemorySystem->LogMemoryStats();
```

## Auto Optimization

### Enable/Disable

```cpp
// Enable automatic optimization (default: enabled)
MemorySystem->EnableAutoOptimization(true);

// Disable automatic optimization
MemorySystem->EnableAutoOptimization(false);
```

### Manual Optimization

```cpp
// Manually trigger optimization
MemorySystem->PerformMemoryOptimization();
```

## Blueprint Usage

All functions are BlueprintCallable, so you can use them in Blueprint:

1. Get Memory Optimization System node
2. Call any function (UpdateTextureStreaming, UpdateMeshLODs, etc.)
3. Use Get Memory Stats to display in UI

## Integration with Terrain System

```cpp
// In your terrain streaming manager
void UTerrainStreamingManager::UpdateStreaming(const FVector& ViewerLocation)
{
    UMemoryOptimizationSystem* MemorySystem = GetWorld()->GetSubsystem<UMemoryOptimizationSystem>();

    // Update texture streaming for terrain
    MemorySystem->UpdateTextureStreaming(ViewerLocation, FVector::ForwardVector);

    // Register new terrain tiles
    for (UStaticMeshComponent* TileMesh : NewlyLoadedTiles)
    {
        MemorySystem->RegisterMeshComponent(TileMesh);
    }

    // Set texture priorities based on distance
    for (UTexture* TerrainTexture : VisibleTextures)
    {
        float Distance = CalculateDistanceToViewer(TerrainTexture);
        ETextureStreamingPriority Priority = GetPriorityForDistance(Distance);
        MemorySystem->SetTextureStreamingPriority(TerrainTexture, Priority);
    }

    // Check if we need to free memory
    if (!MemorySystem->IsWithinMemoryBudget())
    {
        MemorySystem->PerformMemoryOptimization();
    }
}
```

## Performance Tips

1. **Update Frequency**: Update texture streaming and mesh LODs every frame for smooth transitions
2. **Priority Management**: Set critical textures (player vicinity) to high priority
3. **Cache Size**: Adjust tile cache size based on available memory
4. **LOD Distances**: Tune LOD distances for your specific content
5. **Auto Optimization**: Keep enabled for automatic memory management
6. **Monitoring**: Check memory stats regularly during development

## Memory Budget Guidelines

- **Total**: 8GB maximum for VR comfort
- **Textures**: 4GB (50% of total)
- **Meshes**: 2GB (25% of total)
- **Tile Cache**: 1GB (12.5% of total)
- **Other**: 1GB (12.5% for systems, audio, etc.)

## Troubleshooting

### Memory Budget Exceeded

```cpp
if (!MemorySystem->IsWithinMemoryBudget())
{
    // Option 1: Unload distant textures
    MemorySystem->UnloadDistantTextures(5000.0f);

    // Option 2: Clear tile cache
    MemorySystem->ClearTileCache();

    // Option 3: Evict LRU tiles
    MemorySystem->EvictLeastRecentlyUsedTiles(50);

    // Option 4: Perform full optimization
    MemorySystem->PerformMemoryOptimization();
}
```

### Low Cache Hit Rate

```cpp
FTileCacheStats Stats = MemorySystem->GetTileCacheStats();
if (Stats.HitRate < 0.5f)
{
    // Increase cache size
    MemorySystem->SetMaxTileCacheSize(2048); // 2GB
}
```

### Too Many Draw Calls

```cpp
FMeshLODStats Stats = MemorySystem->GetMeshLODStats();
if (Stats.VisibleTriangles > 10000000)
{
    // Increase LOD distances to reduce detail
    FMeshLODConfig Config;
    Config.LOD0Distance = 250.0f;  // Reduce high detail range
    Config.LOD1Distance = 1000.0f;
    MemorySystem->SetMeshLODConfig(Config);
}
```

## See Also

- TerrainStreamingManager for terrain-specific streaming
- VRPerformanceOptimizer for overall VR performance
- FarmingOptimizationSystem for farming-specific optimizations
