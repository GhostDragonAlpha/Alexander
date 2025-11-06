# World Partition Streaming Configuration Guide

## Overview

This guide explains how to configure Unreal Engine 5.6+ World Partition streaming for the comprehensive test level. World Partition enables efficient streaming of large levels with minimal memory overhead and smooth performance.

**Key Features:**
- Spatial grid-based cell streaming
- Automatic LOD transitions during cell loading
- Dynamic loading/unloading based on player distance
- Preloading strategies for seamless transitions
- Sub-region support for performance zones

---

## Table of Contents

1. [World Partition Basics](#world-partition-basics)
2. [Test Level Grid Configuration](#test-level-grid-configuration)
3. [Streaming Cell Setup](#streaming-cell-setup)
4. [Dynamic Loading/Unloading](#dynamic-loadingunloading)
5. [Optimization Tips](#optimization-tips)
6. [Preloading Strategies](#preloading-strategies)
7. [Performance Considerations](#performance-considerations)
8. [Troubleshooting](#troubleshooting)

---

## World Partition Basics

### What is World Partition?

World Partition is UE5's spatial streaming system that automatically divides levels into grid cells and loads/unloads them based on player position.

### Benefits for Test Level

```
Traditional Approach:
- Entire 100km^2 level in memory
- Memory: ~4-8 GB at startup
- Load time: 5-10 minutes
- Streaming: Limited flexibility

World Partition Approach:
- Only ~1km^2 (active area) loaded
- Memory: ~800MB-1.2GB at runtime
- Load time: <1 minute
- Streaming: Dynamic based on player movement
```

### Cell Size Calculation

```
Grid Cell Size = (MaxDistance / ExpectedCellsPerSide)

For test level:
- Map Size: 100 km x 100 km
- Target Cell Size: 2 km x 2 km
- Grid Cells: 50 x 50 = 2,500 cells
- Memory Per Cell: ~4-8 MB
- Maximum Concurrent Loaded: 16 cells = 64-128 MB
```

---

## Test Level Grid Configuration

### Step 1: Enable World Partition

**In Unreal Editor:**

```
1. Open TestLevel_Comprehensive map
2. Window → World Partition → Enable World Partition
3. Set World Partition Settings:
```

### Step 2: Configure Grid Settings

**Location:** Project Settings → World Partition

```ini
[/Script/Engine.WorldPartitionSettings]

; Grid cell size in Unreal units (1 UU = 1 cm)
; For 2km cells: 2000 * 100 = 200,000 UU
WorldPartitionGridSize=200000

; Maximum number of loaded cells
WorldPartitionMaxLoadedCells=16

; Cell preloading distance (cells ahead of player)
WorldPartitionPreloadDistance=300000

; Unload threshold - cells this far from player are unloaded
WorldPartitionUnloadDistance=600000

; Enable debug visualization
bWorldPartitionDebug=true
```

### Step 3: Define World Bounds

```
World Bounds (UU):
Min: (-5000000, -5000000, -5000000)  // -50 million cm (-500 km)
Max: (5000000, 5000000, 5000000)      // 50 million cm (500 km)

This allows:
- Space simulation up to 100 million km
- Surface detail at 1:1 scale
- Terrain up to 100km^2
```

### Step 4: Test Level Specific Configuration

```ini
[TestLevel_Comprehensive World Partition Config]

; Cell size: 2km x 2km x 2km
CellSize=200000

; Test zones (each 2km x 2km)
; SpaceshipLaunchZone: (0-2km, 0-2km)
; FarmingTestZone: (2-4km, 0-2km)
; CelestialScalingZone: (4-6km, 0-2km)
; LandingTestZone: (6-8km, 0-2km)
; PerformanceZone: (8-10km, 0-2km)

; Central Hub: (0km, 0km)
HubLocation=(0, 0, 100000)

; Preload radius (3 cells = 6km)
PreloadRadius=600000

; Unload threshold (5 cells = 10km)
UnloadThreshold=1000000
```

---

## Streaming Cell Setup

### Cell Types for Test Zones

Each test zone should have dedicated streaming cells configured for optimal loading.

#### 1. Spaceship Launch Zone

**Cells to Configure:**
```
- LaunchPad_Main (Cell 0,0)
- SpaceshipGarage (Cell 0,1)
- HangarArea (Cell 1,0)
- FuelDepot (Cell 1,1)

Total Cells: 4 (8 km^2)
```

**Cell Settings:**

```ini
[LaunchPad_Main Cell]
CellSize=200000
LoadingPriority=10  ; High priority - loaded first
PreloadDistance=150000
UnloadDistance=500000
bCompressed=false  ; No compression for immediate access

[SpaceshipGarage Cell]
LoadingPriority=9
PreloadDistance=150000
bCompressed=false

[HangarArea Cell]
LoadingPriority=8
PreloadDistance=150000
bCompressed=true  ; Can be compressed

[FuelDepot Cell]
LoadingPriority=7
PreloadDistance=150000
bCompressed=true
```

#### 2. Spaceship Flight Test Zone

**Configuration:**
```
- FlightCorridor_High (100km altitude)
- FlightCorridor_Mid (10km altitude)
- FlightCorridor_Low (1km altitude)
- ObstacleCourse (Dynamic loading)

Total Cells: 8 (16 km^2)
```

**Special Handling:**
```cpp
// Flight zone requires special streaming logic
// to ensure smooth transitions at high speeds

struct FlightZoneStreamingConfig {
    // Preload cells ahead of spaceship trajectory
    float PreloadAheadDistance = 500000;  // 5 km ahead

    // Maximum velocity before aggressive preloading
    float MaxVelocity = 100000;  // 100 km/h

    // Dynamically adjust preload based on speed
    float DynamicPreloadFactor = MaxVelocity / CurrentVelocity;
};
```

#### 3. Farming Test Zone

**Cells:**
```
- FarmPlots_North (Cell 2,0)
- FarmPlots_South (Cell 2,1)
- FarmTools (Cell 3,0)
- HarvestArea (Cell 3,1)

Total Cells: 4 (8 km^2)
```

**Cell Settings:**

```ini
[FarmPlots_North Cell]
LoadingPriority=8
PreloadDistance=100000  ; Smaller preload for stationary zone
bCompressed=false

[FarmPlots_South Cell]
LoadingPriority=7
PreloadDistance=100000
bCompressed=false

[FarmTools Cell]
LoadingPriority=6
PreloadDistance=100000
bCompressed=true

[HarvestArea Cell]
LoadingPriority=5
PreloadDistance=100000
bCompressed=true
```

#### 4. Celestial Scaling Test Zone

**Configuration:**
```
- CelestialBodies_Prime (Main bodies)
- TestAsteroids (50+ small bodies)
- DebugVisualization (Lines, markers)

Unique Challenge: Bodies move during test
- Must update cell registration dynamically
- Preloading based on orbital path
```

**Special Streaming Logic:**

```cpp
class UCelestialZoneStreamingManager : public UActorComponent {
public:
    // Update streaming cells based on celestial body positions
    void UpdateCelestialStreamingCells();

private:
    // Predict which cells will contain bodies in next frame
    void PredictCelestialPositions(float DeltaTime);

    // Queue cells for preloading
    void PreloadCelestialCells();

    // Cache body positions for smooth updates
    TMap<UCelestialBodyComponent*, FVector> CachedPositions;
};
```

#### 5. Planet Landing Test Zone

**Configuration:**
```
- PlanetSurface_Main (Detailed terrain, 10 km^2)
- AtmosphereZone (Cloud particles, fog)
- LandingZones (4 landing pads)
- TerrainLOD_Far (Distant terrain, low detail)

Total Cells: 6 (12 km^2)
```

**Cell Settings:**

```ini
[PlanetSurface_Main Cell]
LoadingPriority=9
PreloadDistance=200000  ; Preload as player approaches
bCompressed=false
bForceSync=true  ; Important for landing sequence

[AtmosphereZone Cell]
LoadingPriority=8
PreloadDistance=200000
bCompressed=true

[LandingZones Cell]
LoadingPriority=10  ; Highest priority
PreloadDistance=300000
bForceSync=true

[TerrainLOD_Far Cell]
LoadingPriority=4
PreloadDistance=150000
bCompressed=true
```

#### 6. Performance Benchmark Zone

**Configuration:**
```
- StressTestArea_Part1 (50 celestial bodies)
- StressTestArea_Part2 (100+ actors)
- BenchmarkMetrics (Profiling displays)

Total Cells: 4 (8 km^2)
```

**Cell Settings:**

```ini
[StressTestArea_Part1 Cell]
LoadingPriority=6
PreloadDistance=100000
bCompressed=true

[StressTestArea_Part2 Cell]
LoadingPriority=5
PreloadDistance=100000
bCompressed=true

[BenchmarkMetrics Cell]
LoadingPriority=7
PreloadDistance=50000
bCompressed=false
```

---

## Dynamic Loading/Unloading

### Streaming Manager Implementation

```cpp
class ATestLevelStreamingManager : public AActor {
public:
    virtual void Tick(float DeltaTime) override;

protected:
    // Main streaming logic
    void UpdateStreamingCells();

    // Determine which cells should be loaded
    void CalculateRequiredCells();

    // Load cell with priority queue
    void LoadCellWithPriority(int32 CellX, int32 CellY);

    // Unload cells outside threshold
    void UnloadDistantCells();

private:
    // Current active cells
    TSet<FIntPoint> LoadedCells;
    TSet<FIntPoint> LoadingCells;

    // Priority queue for streaming
    TArray<FStreamingCellRequest> StreamingQueue;

    // Max concurrent loads
    static const int32 MaxConcurrentLoads = 4;

    // Performance thresholds
    float LoadDistanceThreshold = 300000;  // 3 km
    float UnloadDistanceThreshold = 600000;  // 6 km
};
```

### Streaming Request Structure

```cpp
struct FStreamingCellRequest {
    int32 CellX, CellY;
    EStreamingAction Action;  // Load, Unload, Preload
    int32 Priority;
    float RequestTime;

    bool operator<(const FStreamingCellRequest& Other) const {
        return Priority > Other.Priority;  // Higher priority first
    }
};

enum EStreamingAction {
    ESA_Load = 0,
    ESA_Unload = 1,
    ESA_Preload = 2,
    ESA_Sync = 3  // Synchronous load (blocking)
};
```

### Implementation Example

```cpp
void ATestLevelStreamingManager::UpdateStreamingCells() {
    if (!PlayerCharacter) return;

    // Get player world position (accounting for universe translation)
    FVector PlayerPos = PlayerCharacter->GetActorLocation();
    FIntPoint PlayerCell = WorldToCell(PlayerPos);

    // Calculate cells that should be loaded
    TSet<FIntPoint> RequiredCells;
    for (int32 X = -2; X <= 2; ++X) {
        for (int32 Y = -2; Y <= 2; ++Y) {
            RequiredCells.Add(PlayerCell + FIntPoint(X, Y));
        }
    }

    // Unload cells no longer needed
    for (const FIntPoint& Cell : LoadedCells) {
        if (!RequiredCells.Contains(Cell)) {
            float Distance = FVector::Dist(PlayerPos, CellToWorld(Cell));
            if (Distance > UnloadDistanceThreshold) {
                UnloadCell(Cell);
                LoadedCells.Remove(Cell);
            }
        }
    }

    // Queue cells for loading
    for (const FIntPoint& Cell : RequiredCells) {
        if (!LoadedCells.Contains(Cell) && !LoadingCells.Contains(Cell)) {
            // Calculate priority based on distance
            FVector CellCenter = CellToWorld(Cell);
            float Distance = FVector::Dist(PlayerPos, CellCenter);
            int32 Priority = (UnloadDistanceThreshold - Distance) / 10000;

            FStreamingCellRequest Request;
            Request.CellX = Cell.X;
            Request.CellY = Cell.Y;
            Request.Priority = Priority;
            Request.Action = ESA_Load;

            StreamingQueue.Add(Request);
        }
    }

    // Sort by priority and process
    StreamingQueue.Sort();

    int32 LoadsThisFrame = 0;
    while (!StreamingQueue.IsEmpty() && LoadsThisFrame < MaxConcurrentLoads) {
        FStreamingCellRequest Request = StreamingQueue.Pop();
        LoadCellWithPriority(Request.CellX, Request.CellY);
        LoadsThisFrame++;
    }
}
```

---

## Optimization Tips

### 1. Reduce Draw Calls in Cells

```
Target: 100-200 draw calls per cell

Techniques:
- Merge static meshes within cells
- Use master materials to reduce material count
- Combine textures into atlases
- Use LOD models appropriately
```

### 2. Optimize Texture Streaming

```ini
[TextureStreamingSettings]

; Max texture streaming pool
PoolSize=512  ; MB

; Update frequency
UpdateInterval=0.5  ; seconds

; Priority for test level cells
HighPriorityCells=(0,0), (0,1), (1,0), (1,1)
```

### 3. Preload Critical Cells

```cpp
// Preload landing zone when approaching planet
if (PlayerDistanceToPlanet < 50000) {  // 500 km
    PreloadCellsAroundLocation(LandingZoneLocation, 3);
}

// Preload cells ahead of spaceship trajectory
FVector TrajectoryPoint = CurrentVelocity.GetSafeNormal() * 500000;
PreloadCellsAroundLocation(PlayerPos + TrajectoryPoint, 2);
```

### 4. Use Compressed Cells for Background Content

```
Compressed Cells:
- HangarArea: 50% size reduction
- FarmTools: 30% size reduction
- BenchmarkMetrics: 40% size reduction

Uncompressed Cells:
- LaunchPad_Main: Full quality
- LandingZones: Full quality
- PlanetSurface_Main: Full quality
```

### 5. Batch Cell Unloading

```cpp
// Don't unload immediately, batch unload after delay
void ATestLevelStreamingManager::ScheduleCellUnload(FIntPoint Cell) {
    UnloadQueue.Add(Cell);

    // Unload all queued cells after 2 seconds
    if (UnloadTimer < 0.0f) {
        UnloadTimer = 2.0f;
    }
}

void ATestLevelStreamingManager::Tick(float DeltaTime) {
    UnloadTimer -= DeltaTime;
    if (UnloadTimer <= 0.0f) {
        for (const FIntPoint& Cell : UnloadQueue) {
            UnloadCell(Cell);
        }
        UnloadQueue.Empty();
    }
}
```

---

## Preloading Strategies

### Strategy 1: Distance-Based Preloading

```cpp
float GetPreloadDistance(float CurrentVelocity) {
    // Higher velocity = larger preload distance
    // Base: 300km (3 cells)
    // Max: 500km (5 cells) at max velocity

    float MaxVelocity = 100000;  // 100 km/h
    float BasePreload = 300000;
    float MaxPreload = 500000;

    float Ratio = FMath::Min(CurrentVelocity / MaxVelocity, 1.0f);
    return FMath::Lerp(BasePreload, MaxPreload, Ratio);
}
```

### Strategy 2: Predictive Preloading

```cpp
void PreloadAheadOfTrajectory() {
    // Calculate future player position
    FVector FuturePos = PlayerPos + (Velocity * 5.0f);  // 5 seconds ahead

    // Preload cells ahead of trajectory
    FIntPoint FutureCell = WorldToCell(FuturePos);
    for (int32 X = -1; X <= 1; ++X) {
        for (int32 Y = -1; Y <= 1; ++Y) {
            FIntPoint TargetCell = FutureCell + FIntPoint(X, Y);
            if (!LoadedCells.Contains(TargetCell)) {
                PreloadCell(TargetCell);
            }
        }
    }
}
```

### Strategy 3: Test Zone Specific Preloading

```cpp
void PreloadTestZone(ETestZone Zone) {
    switch (Zone) {
        case ETestZone::Farming:
            // Preload all 4 farming cells
            PreloadCell(FIntPoint(2, 0));
            PreloadCell(FIntPoint(2, 1));
            PreloadCell(FIntPoint(3, 0));
            PreloadCell(FIntPoint(3, 1));
            break;

        case ETestZone::Landing:
            // Preload landing zone with high priority
            PreloadCellWithPriority(FIntPoint(6, 0), 10);
            PreloadCellWithPriority(FIntPoint(6, 1), 9);
            // Preload surrounding cells with lower priority
            PreloadCell(FIntPoint(5, 0));
            PreloadCell(FIntPoint(7, 0));
            break;

        // ... other zones
    }
}
```

---

## Performance Considerations

### Memory Budget

```
Typical Memory Usage:

16 Loaded Cells @ 8MB per cell = 128 MB
Terrain Systems = 256 MB
Celestial Bodies = 64 MB
Particle Effects = 64 MB
UI/HUD = 32 MB
Streaming Buffers = 128 MB
Engine Overhead = 512 MB
────────────────────────────
Total: ~1.2 GB (Quest 3)
```

### Frame Time Budget

```
Target: 11.1 ms per frame (90 FPS)

Cell Loading: 1-2 ms
Streaming Updates: 0.5-1.0 ms
Physics: 2-3 ms
Rendering: 4-5 ms
VR Processing: 1-2 ms
Other Systems: 1-2 ms
────────────────────────────
Total: ~11 ms (Within budget)
```

### Streaming Load Times

```
Cell Load Time by Size:

Small Cell (2km^2): 0.5-1.0 seconds
Medium Cell (4km^2): 1.5-2.5 seconds
Large Cell (8km^2): 3-5 seconds

Time by Priority:
High Priority (Sync): Blocking, <1 frame
Normal Priority: 1-3 frames
Low Priority: 3-10 frames
```

---

## Troubleshooting

### Issue: Cells loading too slowly

**Symptoms:**
- Pop-in visible during flight
- Terrain appears incomplete
- Actors appear late

**Solutions:**

```cpp
1. Increase preload distance:
   UnloadDistanceThreshold = 1000000;  // 10 km instead of 6 km

2. Use synchronous loading for critical cells:
   Request.Action = ESA_Sync;  // Block until loaded

3. Reduce cell size (more, smaller cells):
   WorldPartitionGridSize = 100000;  // 1 km cells instead of 2 km

4. Check disk I/O:
   - Ensure SSD for level data
   - Profile streaming times
   - Consider async loading on different thread
```

### Issue: Memory usage too high

**Symptoms:**
- Out of memory crash on Quest 3
- Stuttering from GC
- Memory growing unbounded

**Solutions:**

```cpp
1. Reduce max loaded cells:
   MaxConcurrentLoads = 2;  // Load slower but use less memory

2. Compress more cells:
   bCompressed = true;  // Trade CPU for memory

3. Reduce terrain resolution:
   TerrainStreamingBudget = 256;  // MB

4. Profile memory usage:
   stat memory
   stat streaming

5. Force unload distant cells:
   UnloadDistance = 500000;  // More aggressive unloading
```

### Issue: Stutter during cell transitions

**Symptoms:**
- Frame rate drops during loading
- Visible pause when crossing cell boundary
- Controller input lag during load

**Solutions:**

```cpp
1. Reduce load requests per frame:
   MaxConcurrentLoads = 1;

2. Spread loads over multiple frames:
   ChunkLoadingEnabled = true;
   ChunkSize = 1024;  // KB per frame

3. Use lower LOD during loading:
   ForceLOD = 2;  // Higher LOD number = lower quality
   RestoreLODAfterLoad = true;

4. Increase frame buffer:
   MaxFrameLatency = 2;  // Allow 2 frames of buffering

5. Check for other expensive operations:
   stat unit  // Monitor Game Thread, Render Thread
```

### Issue: Celestial bodies disappear unexpectedly

**Symptoms:**
- Planets/moons vanish during flight
- Celestial scaling zone doesn't stream
- Navigation becomes impossible

**Solutions:**

```cpp
1. Enable celestial body streaming override:
   class UCelestialBodyComponent {
   public:
       virtual void OnStreamingUnload() {
           // Don't actually unload, just move out of view
           SetActorHiddenInGame(true);
       }
   };

2. Preload celestial cells always:
   if (ActiveTestZone == ETestZone::CelestialScaling) {
       PreloadCell(FIntPoint(4, 0));  // Always keep loaded
       PreloadCell(FIntPoint(5, 0));
   }

3. Check cell registration:
   Ensure bodies register with streaming system on spawn:
   RegisterStreamingCell(ActorLocation);
```

### Issue: Landing zone doesn't load in time

**Symptoms:**
- Player collides with invisible terrain
- Landing pad not present when landing
- Physics don't apply correctly

**Solutions:**

```cpp
1. Sync load landing cells:
   if (PlayerDistanceToPlanet < 100000) {
       PreloadCell(FIntPoint(6, 0), ESA_Sync);  // Blocking load
   }

2. Preload earlier:
   PreloadDistance = 500000;  // Start preload at 500km away

3. Increase landing cell priority:
   LandingPad.StreamingPriority = 10;  // Highest

4. Verify cell bounds:
   LandingPadCell should be fully within cell boundary
   Check: ActorLocation inside CellBounds

5. Check for stuck loads:
   if (LoadingCells.Contains(Cell) && LoadTimeSeconds > 10) {
       // Force complete load
       ForceLoadCell(Cell);
   }
```

### Issue: Performance degrades over time

**Symptoms:**
- FPS starts at 90, drops to 60 after 30 minutes
- Memory steadily increases
- Frame time becomes inconsistent

**Solutions:**

```cpp
1. Check for memory leaks:
   Ensure cells properly unload
   Check actor cleanup on unload
   Monitor streaming stats

2. Rebuild cell streaming data:
   World Partition → Rebuild
   Regenerate streaming cells

3. Clear streaming cache:
   stat memory
   Identify large persistent allocations

4. Verify GC settings:
   r.Streaming.MaxPoolSize = 512  // Limit texture pool

5. Profile hot spots:
   stat unit  // Check frame time growth
   stat memory  // Check memory growth
   stat streaming  // Check streaming activity

6. Restart streaming manager:
   On extended tests, consider periodic restart
   Monitor if issue repeats on next session
```

---

## Advanced Topics

### Custom Streaming Policies

```cpp
// Implement custom streaming based on test requirements
class IStreamingPolicy {
public:
    virtual TSet<FIntPoint> GetRequiredCells(
        const FVector& PlayerPos,
        float Velocity,
        ETestZone CurrentZone) = 0;
};

class FlightSpeedAdaptivePolicy : public IStreamingPolicy {
    virtual TSet<FIntPoint> GetRequiredCells(...) override {
        // Larger preload for high-speed flight
        int32 PreloadRadius = Velocity > 50000 ? 3 : 2;
        return CalculateRadialCells(PlayerPos, PreloadRadius);
    }
};
```

### Streaming Analytics

```cpp
struct FStreamingAnalytics {
    float AverageCellLoadTime;
    int32 CellLoadCount;
    int32 PopInCount;
    float AveragePopInDistance;
    float MaxMemoryUsed;

    void PrintReport() {
        UE_LOG(LogStreaming, Warning,
            TEXT("Load Time: %.2fms, Pop-ins: %d, Max Mem: %.1fMB"),
            AverageCellLoadTime, PopInCount, MaxMemoryUsed);
    }
};
```

---

## Related Documentation

- [Integration Tests Guide](INTEGRATION_TESTS.md)
- [Implementation Status](IMPLEMENTATION_STATUS.md)
- [CI/CD Pipeline Configuration](.github/workflows/ci.yml)
- [UE5.6 World Partition Documentation](https://docs.unrealengine.com/5.6/en-US/world-partition/)

