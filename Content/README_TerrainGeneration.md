# Procedural Terrain Generation System

This document describes the procedural terrain generation system implemented for the Alexander planet interior enhancement.

## Overview

The terrain generation system consists of four main components:

1. **Procedural Noise Generator** - Generates various types of noise for natural terrain
2. **Terrain Tile Generator** - Creates heightmaps and meshes for terrain tiles
3. **Geological Feature Generator** - Adds realistic geological features
4. **Erosion Simulator** - Simulates natural weathering processes

## Components

### 1. Procedural Noise Generator (`UProceduralNoiseGenerator`)

Provides multiple noise algorithms for terrain generation:

- **Perlin Noise** - Classic smooth noise for rolling terrain
- **Simplex Noise** - Improved Perlin with better performance
- **Ridged Multifractal** - Sharp peaks for mountain ranges
- **Voronoi Noise** - Cell-based patterns for craters
- **Domain Warping** - Natural-looking feature distortion

**Key Features:**

- Multi-octave fractal noise (Fractal Brownian Motion)
- Configurable frequency, amplitude, persistence, and lacunarity
- 2D and 3D noise support
- Seed-based deterministic generation

**Example Usage:**

```cpp
FNoiseConfig Config;
Config.NoiseType = ENoiseType::Perlin;
Config.Seed = 12345;
Config.Frequency = 0.001f;
Config.Octaves = 6;
Config.Persistence = 0.5f;
Config.Lacunarity = 2.0f;

float NoiseValue = UProceduralNoiseGenerator::FractalNoise2D(X, Y, Config);
```

### 2. Terrain Tile Generator (`UTerrainTileGenerator`)

Generates terrain tiles with heightmaps and mesh data:

**Features:**

- Heightmap generation from noise
- Mesh generation with vertices, triangles, normals, UVs, and tangents
- LOD support for different detail levels
- Configurable resolution and tile size

**Data Structure:**

```cpp
FTerrainTileData TileData = UTerrainTileGenerator::GenerateTerrainTile(
    TilePosition,      // World position
    TileSize,          // Size in meters
    Resolution,        // Vertices per side
    LODLevel,          // Detail level
    Config             // Generation config
);
```

**Generated Data:**

- `Vertices` - 3D positions
- `Triangles` - Triangle indices
- `Normals` - Surface normals for lighting
- `UVs` - Texture coordinates
- `Tangents` - Tangent vectors for normal mapping
- `HeightData` - Raw height values for queries

### 3. Geological Feature Generator (`UGeologicalFeatureGenerator`)

Adds realistic geological features to terrain:

**Feature Types:**

#### Mountain Ranges

- Ridged multifractal noise for peaks
- Configurable length, width, height, and rotation
- Multiple peaks with natural variation
- Roughness control

```cpp
FMountainRangeConfig MountainConfig;
MountainConfig.Position = FVector2D(0, 0);
MountainConfig.Length = 5000.0f;
MountainConfig.Height = 2000.0f;
MountainConfig.Roughness = 0.7f;

float Height = UGeologicalFeatureGenerator::GenerateMountainRange(X, Y, MountainConfig, Seed);
```

#### Valleys

- Smooth depressions between terrain features
- Linear or curved paths
- Configurable width and depth

#### Canyons

- Steep-walled depressions
- Polyline-based paths
- Adjustable steepness

#### Craters

- Impact crater simulation
- Raised rims and bowl-shaped depressions
- Procedural crater fields using Voronoi

```cpp
float CraterHeight = UGeologicalFeatureGenerator::GenerateCraterField(
    X, Y,
    Seed,
    Density,      // Craters per km²
    MinRadius,    // Minimum crater size
    MaxRadius     // Maximum crater size
);
```

#### Plateaus

- Flat elevated regions
- Steep edges with configurable steepness
- Natural blending with surrounding terrain

### 4. Erosion Simulator (`UErosionSimulator`)

Simulates natural weathering processes:

#### Hydraulic Erosion (Water)

- Particle-based water droplet simulation
- Erosion on steep slopes
- Sediment transport and deposition
- Realistic drainage patterns

```cpp
FHydraulicErosionConfig Config;
Config.Iterations = 100;
Config.ErosionStrength = 0.3f;
Config.DepositionStrength = 0.3f;

UErosionSimulator::ApplyHydraulicErosion(HeightData, Resolution, TileSize, Config, Seed);
```

#### Thermal Erosion (Gravity)

- Slope-based material movement
- Talus angle (angle of repose) simulation
- Smooths steep slopes naturally

```cpp
FThermalErosionConfig Config;
Config.Iterations = 50;
Config.TalusAngle = 45.0f;  // degrees

UErosionSimulator::ApplyThermalErosion(HeightData, Resolution, TileSize, Config);
```

#### Wind Erosion

- Directional erosion based on wind
- Abrasion on windward slopes
- Deposition on leeward slopes

```cpp
FWindErosionConfig Config;
Config.WindDirection = FVector2D(1.0f, 0.0f);
Config.WindStrength = 0.2f;

UErosionSimulator::ApplyWindErosion(HeightData, Resolution, TileSize, Config);
```

#### Drainage Patterns

- Automatic river network generation
- Flow accumulation calculation
- Stream order classification

```cpp
TArray<FDrainagePattern> Patterns = UErosionSimulator::GenerateDrainagePatterns(
    HeightData,
    Resolution,
    TileSize,
    MinStreamLength
);
```

## Typical Workflow

### Basic Terrain Generation

```cpp
// 1. Configure terrain generation
FTerrainGenerationConfig Config;
Config.Seed = 12345;
Config.BaseElevation = 0.0f;
Config.ElevationRange = 1000.0f;
Config.NoiseScale = 0.001f;
Config.NoiseOctaves = 6;

// 2. Generate terrain tile
FTerrainTileData Tile = UTerrainTileGenerator::GenerateTerrainTile(
    FVector2D(0, 0),  // Position
    1000.0f,          // Size
    64,               // Resolution
    0,                // LOD
    Config
);

// 3. Add geological features
FMountainRangeConfig MountainConfig;
// ... configure mountain ...

for (int32 Y = 0; Y < Tile.Resolution; ++Y)
{
    for (int32 X = 0; X < Tile.Resolution; ++X)
    {
        float WorldX = /* calculate world position */;
        float WorldY = /* calculate world position */;

        float MountainHeight = UGeologicalFeatureGenerator::GenerateMountainRange(
            WorldX, WorldY, MountainConfig, Config.Seed
        );

        Tile.HeightData[Y * Tile.Resolution + X] += MountainHeight;
    }
}

// 4. Apply erosion
FHydraulicErosionConfig ErosionConfig;
UErosionSimulator::ApplyHydraulicErosion(
    Tile.HeightData,
    Tile.Resolution,
    Tile.TileSize,
    ErosionConfig,
    Config.Seed
);

// 5. Regenerate mesh from modified heightmap
UTerrainTileGenerator::GenerateMeshFromHeightmap(
    Tile.HeightData,
    Tile.TileSize,
    Tile.Resolution,
    Tile.Vertices,
    Tile.Triangles,
    Tile.Normals,
    Tile.UVs,
    Tile.Tangents
);
```

## Integration with Planet System

The terrain generation system integrates with the existing `APlanet` class:

```cpp
void APlanet::GeneratePlanetTerrain()
{
    // Use TerrainConfig from planet configuration
    FTerrainGenerationConfig GenConfig;
    GenConfig.Seed = TerrainSeed;
    GenConfig.BaseElevation = 0.0f;
    GenConfig.ElevationRange = TerrainConfig.Amplitude;
    GenConfig.NoiseScale = 1.0f / TerrainConfig.Scale;
    GenConfig.NoiseOctaves = TerrainConfig.Octaves;
    GenConfig.NoisePersistence = TerrainConfig.Roughness;

    // Generate terrain tiles based on viewer position
    // Apply geological features based on planet type
    // Apply erosion based on planet age and climate
}
```

## Performance Considerations

- **Noise Generation**: O(n) per sample, where n is number of octaves
- **Heightmap Generation**: O(resolution²) per tile
- **Mesh Generation**: O(resolution²) per tile
- **Erosion**: O(iterations × resolution²)

**Optimization Tips:**

- Use lower resolutions for distant LODs
- Cache generated tiles
- Generate tiles asynchronously on background threads
- Use GPU compute shaders for large-scale erosion (future enhancement)

## Requirements Satisfied

This implementation satisfies the following requirements from the design document:

- **Requirement 4.1**: Multi-octave Perlin/Simplex noise ✓
- **Requirement 4.2**: Configurable terrain parameters ✓
- **Requirement 4.3**: Realistic geological features (mountains, valleys, canyons, craters, plateaus) ✓
- **Requirement 4.4**: Erosion simulation (hydraulic, thermal, wind) ✓
- **Requirement 4.4**: Realistic drainage patterns ✓

## LOD and Streaming System

The terrain system includes a sophisticated LOD (Level of Detail) and streaming system for optimal performance:

### 5. QuadTree Tile Manager (`UQuadTreeTileManager`)

Manages hierarchical terrain tiles using a quadtree structure:

**Features:**

- Hierarchical tile subdivision based on viewer distance
- 6 LOD levels (0 = highest detail, 5 = lowest)
- Distance-based LOD calculation
- Automatic tile merging when viewer moves away
- Visibility culling for off-screen tiles

**LOD Distances (Default):**

- LOD 0: 0-500m (highest detail, ~1m per vertex)
- LOD 1: 500m-2km (~5m per vertex)
- LOD 2: 2km-10km (~25m per vertex)
- LOD 3: 10km-50km (~100m per vertex)
- LOD 4: 50km-200km (~500m per vertex)
- LOD 5: 200km+ (lowest detail, ~2km per vertex)

**Example Usage:**

```cpp
// Initialize quadtree
UQuadTreeTileManager* TileManager = NewObject<UQuadTreeTileManager>();

FQuadTreeLODConfig LODConfig;
LODConfig.MaxLODLevel = 6;
LODConfig.BaseTileSize = 500.0f;
LODConfig.TileResolution = 64;
LODConfig.ViewDistance = 50000.0f;

TileManager->Initialize(
    FVector2D(0, 0),  // Root position
    100000.0f,        // Root size (covers entire terrain)
    LODConfig
);

// Update each frame
TileManager->UpdateVisibleTiles(PlayerPosition, DeltaTime);

// Get active tiles for rendering
TArray<FTerrainTileData> ActiveTiles = TileManager->GetActiveTiles();
```

**Key Methods:**

- `CalculateLODLevel()` - Determines appropriate LOD based on distance
- `SubdivideTile()` - Splits tile into 4 children for higher detail
- `MergeTiles()` - Combines 4 children back to parent for lower detail
- `UpdateVisibilityCulling()` - Hides tiles outside view distance
- `GetTileAtPosition()` - Queries tile at specific world position

### 6. Terrain Streaming Manager (`UTerrainStreamingManager`)

Handles asynchronous tile generation and caching:

**Features:**

- Background thread tile generation (non-blocking)
- Priority-based loading queue
- LRU (Least Recently Used) tile cache
- Frame-time budgeting (max 5ms per frame)
- Automatic cache eviction when full

**Configuration:**

```cpp
FStreamingConfig Config;
Config.MaxFrameTimeMS = 5.0f;           // Max time per frame
Config.MaxTilesPerFrame = 4;            // Max tiles to process per frame
Config.MaxCacheSize = 500;              // Max cached tiles
Config.bUseBackgroundThread = true;     // Enable async generation
Config.NumWorkerThreads = 2;            // Number of worker threads

UTerrainStreamingManager* StreamingManager = NewObject<UTerrainStreamingManager>();
StreamingManager->Initialize(Config);
```

**Request Tile Loading:**

```cpp
// Request tile with priority
int32 RequestID = StreamingManager->RequestTileLoad(
    TilePosition,
    TileSize,
    LODLevel,
    Resolution,
    GenerationConfig,
    ETileLoadPriority::High,
    ViewerPosition
);

// Check if ready
if (StreamingManager->IsTileReady(RequestID))
{
    FTerrainTileData TileData;
    if (StreamingManager->GetLoadedTile(RequestID, TileData))
    {
        // Use tile data for rendering
    }
}
```

**Cache Management:**

- Automatic caching of generated tiles
- Cache hit/miss tracking
- LRU eviction when cache is full
- Manual cache clearing available

**Statistics:**

```cpp
FStreamingStats Stats = StreamingManager->GetStreamingStats();
UE_LOG(LogTemp, Log, TEXT("Pending: %d, Cached: %d, Cache Hits: %d, Avg Load Time: %.2fms"),
    Stats.PendingRequests,
    Stats.CachedTiles,
    Stats.CacheHits,
    Stats.AverageLoadTime);
```

### 7. LOD Transition Manager (`ULODTransitionManager`)

Provides smooth transitions between LOD levels to eliminate popping:

**Features:**

- Vertex geomorphing between LOD levels
- Smooth 0.5-second transitions
- Distance-based fading
- Normal interpolation for lighting continuity
- Cubic easing for natural motion

**Configuration:**

```cpp
FLODTransitionConfig Config;
Config.bEnableGeomorphing = true;       // Enable vertex morphing
Config.bEnableDistanceFade = true;      // Enable distance fade
Config.TransitionDuration = 0.5f;       // 0.5 seconds
Config.FadeDistanceRange = 100.0f;      // Fade over 100m
Config.MaxConcurrentTransitions = 20;   // Max simultaneous transitions
Config.bInterpolateNormals = true;      // Smooth lighting
Config.bUseSmoothEasing = true;         // Cubic easing

ULODTransitionManager* TransitionManager = NewObject<ULODTransitionManager>();
TransitionManager->Initialize(Config);
```

**Start Transition:**

```cpp
// When LOD changes
TransitionManager->StartTransition(
    TilePosition,
    OldLODLevel,
    NewLODLevel,
    OldTileData,
    NewTileData,
    ViewerPosition
);

// Update each frame
TransitionManager->UpdateTransitions(DeltaTime, ViewerPosition);

// Get geomorphed data for rendering
FTerrainTileData GeomorphedData;
if (TransitionManager->GetGeomorphedTileData(TilePosition, GeomorphedData))
{
    // Render with smooth transition
}
```

**Geomorphing Algorithm:**

1. Interpolate vertex positions between old and new LOD
2. Apply smooth cubic easing for natural motion
3. Interpolate normals for consistent lighting
4. Handle different resolutions by sampling from higher-res mesh
5. Complete transition after configured duration

## Complete Integration Example

```cpp
// Setup all systems
UQuadTreeTileManager* TileManager = NewObject<UQuadTreeTileManager>();
UTerrainStreamingManager* StreamingManager = NewObject<UTerrainStreamingManager>();
ULODTransitionManager* TransitionManager = NewObject<ULODTransitionManager>();

// Initialize
TileManager->Initialize(RootPosition, RootSize, LODConfig);
StreamingManager->Initialize(StreamingConfig);
TransitionManager->Initialize(TransitionConfig);

// Game loop
void UpdateTerrain(float DeltaTime, FVector PlayerPosition)
{
    // Update quadtree LOD
    TileManager->UpdateVisibleTiles(PlayerPosition, DeltaTime);

    // Get active tiles
    TArray<FTerrainTileData> ActiveTiles = TileManager->GetActiveTiles();

    // Request loading for tiles that need it
    for (const FTerrainTileData& Tile : ActiveTiles)
    {
        if (!Tile.bIsGenerated)
        {
            StreamingManager->RequestTileLoad(
                Tile.WorldPosition,
                Tile.TileSize,
                Tile.LODLevel,
                Tile.Resolution,
                TerrainConfig,
                ETileLoadPriority::Normal,
                FVector2D(PlayerPosition)
            );
        }
    }

    // Process completed loads
    StreamingManager->Update(DeltaTime);

    // Update transitions
    TransitionManager->UpdateTransitions(DeltaTime, FVector2D(PlayerPosition));

    // Render tiles with transitions
    for (const FTerrainTileData& Tile : ActiveTiles)
    {
        FTerrainTileData RenderData = Tile;

        // Check if transitioning
        if (TransitionManager->GetGeomorphedTileData(Tile.WorldPosition, RenderData))
        {
            // Use geomorphed data for smooth transition
        }

        // Render tile
        RenderTerrainTile(RenderData);
    }
}
```

## Performance Targets

The LOD and streaming system is designed to meet these performance targets:

- **Frame Rate**: 90 FPS minimum in VR (11.1ms frame time)
- **Terrain Generation**: < 5ms per tile (enforced by frame budget)
- **LOD Transitions**: < 1ms per transition
- **Memory Usage**: < 8GB for complete planet system
- **Streaming**: < 100MB/s bandwidth
- **Active Tiles**: < 500 tiles simultaneously
- **Cache Size**: 500 tiles (configurable)

## Requirements Satisfied

The LOD and streaming system satisfies these requirements:

- **Requirement 1.2**: Dynamic LOD transitions without visible popping ✓
- **Requirement 8.2**: LOD system with 6+ detail levels ✓
- **Requirement 8.3**: Frustum culling for off-screen terrain ✓
- **Requirement 8.5**: Asynchronous streaming with 5ms frame budget ✓

## Future Enhancements

- GPU-accelerated erosion for real-time terrain modification
- Biome-specific terrain generation
- Cave system integration
- Tectonic plate simulation
- Vegetation placement based on terrain features
- Material layer generation based on slope and altitude
- GPU compute shader for tile generation
- Predictive tile loading based on player movement
- Texture streaming integration
- Nanite integration for automatic LOD
