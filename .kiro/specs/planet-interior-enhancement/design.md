# Design Document

## Overview

This design document outlines the technical architecture for transforming the current basic planet system into a Star Citizen-quality planetary environment system. The design leverages Unreal Engine 5.6's advanced rendering features including Nanite, Lumen, Virtual Shadow Maps, and World Partition for massive open worlds.

## Architecture

### System Components

```
PlanetSystem
├── APlanetActor (Enhanced)
│   ├── PlanetMeshComponent (Nanite-enabled)
│   ├── AtmosphereComponent
│   ├── CloudComponent
│   └── WeatherComponent
├── TerrainGenerationSubsystem
│   ├── ProceduralNoiseGenerator
│   ├── BiomeManager
│   ├── ErosionSimulator
│   └── CaveGenerator
├── MaterialSystem
│   ├── LayeredTerrainMaterial
│   ├── BiomeMaterialLibrary
│   └── DynamicMaterialInstances
├── LODManagementSystem
│   ├── QuadTreeTileManager
│   ├── StreamingController
│   └── MeshSimplification
└── TransitionSystem
    ├── SpaceToAtmosphereController
    ├── PhysicsTransitionManager
    └── RenderingTransitionManager
```

## Components and Interfaces

### 1. Enhanced Planet Actor (APlanet)

**Purpose**: Core planet entity managing all planetary systems and rendering.

**Key Properties**:

```cpp
class ALEXANDER_API APlanet : public AOrbitalBody
{
    // Terrain Generation
    UPROPERTY(EditAnywhere, Category = "Terrain")
    FPlanetTerrainConfig TerrainConfig;

    UPROPERTY(EditAnywhere, Category = "Terrain")
    int32 TerrainSeed;

    UPROPERTY(EditAnywhere, Category = "Terrain")
    float PlanetRadius; // In kilometers

    // Biome System
    UPROPERTY(EditAnywhere, Category = "Biomes")
    TArray<FBiomeDefinition> Biomes;

    UPROPERTY(EditAnywhere, Category = "Biomes")
    UCurveFloat* LatitudeBiomeDistribution;

    // Atmosphere
    UPROPERTY(EditAnywhere, Category = "Atmosphere")
    FAtmosphereConfig AtmosphereSettings;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UAtmosphereComponent* AtmosphereComponent;

    // Weather
    UPROPERTY(EditAnywhere, Category = "Weather")
    TArray<FWeatherPreset> WeatherPresets;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UWeatherComponent* WeatherComponent;

    // Clouds
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UVolumetricCloudComponent* CloudComponent;

    // LOD System
    UPROPERTY(EditAnywhere, Category = "Performance")
    FLODConfiguration LODSettings;

    // Materials
    UPROPERTY(EditAnywhere, Category = "Materials")
    UMaterialInterface* TerrainMasterMaterial;

    UPROPERTY()
    TMap<FString, UMaterialInstanceDynamic*> BiomeMaterials;
};
```

**Key Methods**:

- `GeneratePlanetTerrain()` - Initializes procedural terrain generation
- `UpdateLOD(FVector ViewerLocation)` - Manages terrain detail levels
- `TransitionToSurface(AActor* IncomingActor)` - Handles space-to-surface transition
- `GetTerrainHeightAtLocation(FVector2D Coordinates)` - Queries terrain elevation
- `GetBiomeAtLocation(FVector2D Coordinates)` - Returns biome type at coordinates

### 2. Terrain Generation Subsystem

**Purpose**: Procedurally generates realistic planetary terrain using noise algorithms and geological simulation.

**Architecture**:

```cpp
UCLASS()
class UTerrainGenerationSubsystem : public UWorldSubsystem
{
    // Noise Generation
    UPROPERTY()
    UProceduralNoiseGenerator* NoiseGenerator;

    // Biome Management
    UPROPERTY()
    UBiomeManager* BiomeManager;

    // Erosion Simulation
    UPROPERTY()
    UErosionSimulator* ErosionSimulator;

    // Cave Generation
    UPROPERTY()
    UCaveGenerator* CaveGenerator;

public:
    // Generate heightmap for terrain tile
    FTerrainTile GenerateTerrainTile(FVector2D TileCoordinates, int32 LODLevel);

    // Apply erosion to heightmap
    void ApplyErosion(FTerrainTile& Tile, float ErosionStrength);

    // Generate cave systems
    TArray<FCaveSystem> GenerateCaves(FVector2D RegionCoordinates);
};
```

**Noise Generation Strategy**:

- Multi-octave Perlin/Simplex noise for base terrain
- Ridged multifractal noise for mountain ranges
- Voronoi cells for crater generation
- Domain warping for natural-looking features
- Layered noise with different frequencies for detail levels

### 3. Biome System

**Purpose**: Defines and manages distinct environmental regions with unique characteristics.

**Biome Definition**:

```cpp
USTRUCT(BlueprintType)
struct FBiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    FString BiomeName;

    UPROPERTY(EditAnywhere)
    FLinearColor BiomeColor; // For debug visualization

    // Terrain Properties
    UPROPERTY(EditAnywhere)
    float BaseElevation;

    UPROPERTY(EditAnywhere)
    float ElevationVariance;

    UPROPERTY(EditAnywhere)
    float Roughness; // 0-1, affects terrain detail

    // Material Layers
    UPROPERTY(EditAnywhere)
    TArray<FTerrainMaterialLayer> MaterialLayers;

    // Vegetation
    UPROPERTY(EditAnywhere)
    TArray<FVegetationDefinition> VegetationTypes;

    UPROPERTY(EditAnywhere)
    float VegetationDensity; // 0-1

    // Environmental
    UPROPERTY(EditAnywhere)
    TArray<FWeatherType> AllowedWeather;

    UPROPERTY(EditAnywhere)
    float Temperature; // Celsius

    UPROPERTY(EditAnywhere)
    float Humidity; // 0-1
};
```

**Biome Blending**:

- Use 3D noise for biome distribution across planet
- Smooth interpolation between adjacent biomes (50-100m transition zones)
- Altitude-based biome variation (snow caps on mountains)
- Latitude-based temperature gradients
- Moisture maps for desert/forest distribution

### 4. Atmospheric System

**Purpose**: Renders realistic atmospheric effects including scattering, fog, and sky colors.

**Implementation**:

```cpp
UCLASS()
class UAtmosphereComponent : public USceneComponent
{
    // Scattering Parameters
    UPROPERTY(EditAnywhere, Category = "Scattering")
    FLinearColor RayleighScatteringCoefficient;

    UPROPERTY(EditAnywhere, Category = "Scattering")
    float RayleighScaleHeight; // km

    UPROPERTY(EditAnywhere, Category = "Scattering")
    FLinearColor MieScatteringCoefficient;

    UPROPERTY(EditAnywhere, Category = "Scattering")
    float MieScaleHeight; // km

    UPROPERTY(EditAnywhere, Category = "Scattering")
    float MieAnisotropy; // -1 to 1

    // Atmosphere Properties
    UPROPERTY(EditAnywhere, Category = "Atmosphere")
    float AtmosphereHeight; // km

    UPROPERTY(EditAnywhere, Category = "Atmosphere")
    float PlanetRadius; // km

    UPROPERTY(EditAnywhere, Category = "Atmosphere")
    FLinearColor GroundAlbedo;

    // Fog
    UPROPERTY(EditAnywhere, Category = "Fog")
    float FogDensity;

    UPROPERTY(EditAnywhere, Category = "Fog")
    float FogHeightFalloff;

public:
    // Calculate atmospheric color at view direction
    FLinearColor CalculateAtmosphericColor(FVector ViewDirection, FVector SunDirection);

    // Get fog density at altitude
    float GetFogDensityAtAltitude(float Altitude);
};
```

**Rendering Technique**:

- Use Unreal's Sky Atmosphere component as base
- Custom atmospheric scattering shader for planet-specific colors
- Pre-computed lookup tables for performance
- Real-time ray marching for close-up atmospheric effects
- Integration with Lumen for realistic atmospheric lighting

### 5. Volumetric Cloud System

**Purpose**: Renders realistic 3D clouds with lighting and shadows.

**Implementation**:

```cpp
UCLASS()
class UCloudComponent : public UVolumetricCloudComponent
{
    UPROPERTY(EditAnywhere, Category = "Clouds")
    float CloudLayerHeight; // km above surface

    UPROPERTY(EditAnywhere, Category = "Clouds")
    float CloudLayerThickness; // km

    UPROPERTY(EditAnywhere, Category = "Clouds")
    float CloudCoverage; // 0-1

    UPROPERTY(EditAnywhere, Category = "Clouds")
    float CloudDensity;

    UPROPERTY(EditAnywhere, Category = "Clouds")
    FVector2D WindDirection;

    UPROPERTY(EditAnywhere, Category = "Clouds")
    float WindSpeed; // m/s

    UPROPERTY(EditAnywhere, Category = "Noise")
    int32 CloudSeed;

public:
    // Update cloud positions based on wind
    void UpdateCloudMovement(float DeltaTime);

    // Adjust cloud coverage for weather
    void SetWeatherCoverage(float Coverage, float TransitionTime);
};
```

**Cloud Rendering**:

- Use Unreal's Volumetric Cloud component
- 3D Perlin-Worley noise for cloud shapes
- Multiple noise octaves for detail
- Ray marching through volume for lighting
- Shadow casting on terrain
- Weather-driven cloud coverage changes

### 6. LOD and Streaming System

**Purpose**: Manages terrain detail levels and asynchronous loading for performance.

**QuadTree Tile System**:

```cpp
struct FTerrainTile
{
    FVector2D WorldPosition;
    int32 LODLevel; // 0 = highest detail
    float TileSize; // meters

    // Mesh Data
    TArray<FVector> Vertices;
    TArray<int32> Indices;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;

    // Material Data
    TArray<FLinearColor> BiomeWeights;

    // State
    bool bIsLoaded;
    bool bIsVisible;
    UStaticMeshComponent* MeshComponent;
};

class FQuadTreeTileManager
{
    // Root tile covers entire planet
    TSharedPtr<FTerrainTile> RootTile;

    // Active tiles currently loaded
    TMap<FIntPoint, TSharedPtr<FTerrainTile>> ActiveTiles;

    // LOD selection based on distance
    int32 CalculateLODLevel(FVector ViewerPosition, FVector TileCenter);

    // Subdivide tile into 4 children
    void SubdivideTile(TSharedPtr<FTerrainTile> ParentTile);

    // Merge 4 child tiles back to parent
    void MergeTiles(TSharedPtr<FTerrainTile> ParentTile);

    // Update visible tiles based on viewer position
    void UpdateVisibleTiles(FVector ViewerPosition, float ViewDistance);
};
```

**LOD Distances** (for 6 levels):

- LOD 0: 0-500m (highest detail, ~1m per vertex)
- LOD 1: 500m-2km (~5m per vertex)
- LOD 2: 2km-10km (~25m per vertex)
- LOD 3: 10km-50km (~100m per vertex)
- LOD 4: 50km-200km (~500m per vertex)
- LOD 5: 200km+ (lowest detail, ~2km per vertex)

### 7. Material System

**Purpose**: Layered material system for realistic terrain appearance.

**Master Material Structure**:

```
TerrainMasterMaterial
├── Layer 0: Base Rock
├── Layer 1: Dirt/Soil
├── Layer 2: Grass
├── Layer 3: Sand
├── Layer 4: Snow
├── Layer 5: Cliff Rock
├── Layer 6: Wet Surface
└── Layer 7: Custom/Special

Blending Inputs:
- Biome Weights (texture)
- Slope Angle (calculated)
- Altitude (world position)
- Moisture (texture)
- Temperature (texture)
```

**Material Features**:

- PBR workflow (Base Color, Normal, Roughness, Metallic, AO)
- Triplanar projection for vertical surfaces
- Distance-based texture blending (macro/micro detail)
- Parallax occlusion mapping for depth
- Tessellation for geometric detail
- Dynamic wetness from weather
- Footprint/vehicle track deformation

### 8. Weather System

**Purpose**: Dynamic weather conditions affecting visuals and gameplay.

**Weather State Machine**:

```cpp
UENUM()
enum class EWeatherType : uint8
{
    Clear,
    PartlyCloudy,
    Overcast,
    LightRain,
    HeavyRain,
    Thunderstorm,
    LightSnow,
    HeavySnow,
    Sandstorm,
    Fog
};

UCLASS()
class UWeatherComponent : public UActorComponent
{
    UPROPERTY()
    EWeatherType CurrentWeather;

    UPROPERTY()
    EWeatherType TargetWeather;

    UPROPERTY()
    float TransitionProgress; // 0-1

    UPROPERTY(EditAnywhere)
    float WeatherChangeInterval; // seconds

    // Particle Systems
    UPROPERTY()
    UNiagaraComponent* PrecipitationSystem;

    UPROPERTY()
    UNiagaraComponent* WindParticles;

public:
    void TransitionToWeather(EWeatherType NewWeather, float Duration);
    void UpdateWeatherEffects(float DeltaTime);
    FLinearColor GetWeatherLightingMultiplier();
};
```

**Weather Effects**:

- Cloud coverage changes
- Precipitation particles (rain, snow, dust)
- Wind forces on vegetation and particles
- Lighting adjustments (darker during storms)
- Fog density changes
- Sound effects (rain, thunder, wind)
- Wetness on surfaces

### 9. Space-to-Surface Transition System

**Purpose**: Seamless transition from orbital flight to surface landing.

**Transition Phases**:

```cpp
UENUM()
enum class ETransitionPhase : uint8
{
    Orbital,        // > 100km altitude
    HighAtmosphere, // 100km - 50km
    MidAtmosphere,  // 50km - 10km
    LowAtmosphere,  // 10km - 1km
    Surface         // < 1km
};

class FTransitionController
{
    ETransitionPhase CurrentPhase;

    // Physics Transition
    void UpdatePhysicsForPhase(ETransitionPhase Phase);
    void ApplyAtmosphericDrag(float Altitude, float Velocity);
    void ApplyLiftForces(float Altitude, float AirSpeed, float AoA);

    // Rendering Transition
    void UpdateRenderingForPhase(ETransitionPhase Phase);
    void AdjustLODDistances(float Altitude);
    void EnableTerrainStreaming(bool bEnable);

    // Camera Transition
    void AdjustCameraSettings(float Altitude);
    void UpdateFogSettings(float Altitude);
};
```

**Transition Details**:

- Gradual LOD increase as altitude decreases
- Atmospheric effects fade in below 100km
- Terrain streaming starts at 50km
- Physics changes from orbital to atmospheric at 100km
- Camera far clip plane adjusts with altitude
- Fog density increases near surface

### 10. Cave System

**Purpose**: Procedurally generated underground exploration areas.

**Cave Generation**:

```cpp
struct FCaveSystem
{
    FVector EntranceLocation;
    TArray<FCaveChamber> Chambers;
    TArray<FCaveTunnel> Tunnels;
    int32 MaxDepth; // meters below surface
};

struct FCaveChamber
{
    FVector CenterLocation;
    float Radius;
    float Height;
    TArray<FVector> Stalactites;
    TArray<FVector> Stalagmites;
    TArray<FResourceNode> Resources;
};

class UCaveGenerator
{
public:
    // Generate cave system at location
    FCaveSystem GenerateCave(FVector SurfaceLocation, int32 Seed);

    // Create cave mesh geometry
    UProceduralMeshComponent* CreateCaveMesh(const FCaveSystem& Cave);

    // Place cave decorations
    void PopulateCaveFeatures(FCaveSystem& Cave);
};
```

**Cave Features**:

- Worm-based tunnel generation
- Cellular automata for chamber shapes
- Stalactite/stalagmite placement
- Underground water pools
- Mineral deposits and resources
- Ambient lighting from bioluminescence
- Echo-based audio

## Data Models

### Planet Configuration Asset

```cpp
UCLASS(BlueprintType)
class UPlanetConfiguration : public UDataAsset
{
    GENERATED_BODY()

public:
    // Identity
    UPROPERTY(EditAnywhere, Category = "Identity")
    FString PlanetName;

    UPROPERTY(EditAnywhere, Category = "Identity")
    EPlanetType PlanetType; // Terrestrial, Gas Giant, Ice, Desert, etc.

    // Physical Properties
    UPROPERTY(EditAnywhere, Category = "Physical")
    float Radius; // km

    UPROPERTY(EditAnywhere, Category = "Physical")
    float Mass; // kg

    UPROPERTY(EditAnywhere, Category = "Physical")
    float RotationPeriod; // hours

    UPROPERTY(EditAnywhere, Category = "Physical")
    float AxialTilt; // degrees

    // Terrain
    UPROPERTY(EditAnywhere, Category = "Terrain")
    FPlanetTerrainConfig TerrainConfig;

    UPROPERTY(EditAnywhere, Category = "Terrain")
    TArray<FBiomeDefinition> Biomes;

    // Atmosphere
    UPROPERTY(EditAnywhere, Category = "Atmosphere")
    bool bHasAtmosphere;

    UPROPERTY(EditAnywhere, Category = "Atmosphere", meta = (EditCondition = "bHasAtmosphere"))
    FAtmosphereConfig AtmosphereConfig;

    // Weather
    UPROPERTY(EditAnywhere, Category = "Weather")
    TArray<FWeatherPreset> WeatherPresets;

    // Resources
    UPROPERTY(EditAnywhere, Category = "Resources")
    TArray<FResourceDistribution> ResourceDistributions;
};
```

## Error Handling

### Terrain Generation Failures

- **Issue**: Noise generation produces invalid values (NaN, Inf)
- **Handling**: Clamp all noise values to valid ranges, log warnings, use fallback flat terrain

### LOD Transition Popping

- **Issue**: Visible mesh changes during LOD transitions
- **Handling**: Implement geomorphing between LOD levels, smooth vertex positions over 0.5 seconds

### Streaming Stalls

- **Issue**: Terrain loading causes frame rate drops
- **Handling**: Limit loading to 5ms per frame, prioritize tiles by distance, use background threads

### Memory Overflow

- **Issue**: Too many terrain tiles loaded simultaneously
- **Handling**: Implement tile cache with LRU eviction, maximum 500 active tiles, unload distant tiles

### Physics Transition Glitches

- **Issue**: Ship behavior changes abruptly during atmosphere entry
- **Handling**: Smooth interpolation of physics parameters over 5-second transition period

## Testing Strategy

### Unit Tests

- Noise generation produces consistent results for same seed
- Biome blending weights sum to 1.0
- LOD calculation returns correct level for given distance
- Atmospheric scattering produces valid colors
- Cave generation creates connected tunnel systems

### Integration Tests

- Complete space-to-surface transition without crashes
- Weather transitions complete smoothly
- Material system blends correctly across biomes
- Streaming system loads tiles in correct order
- Physics behaves correctly at all altitudes

### Performance Tests

- Maintain 90 FPS in VR at surface level
- Terrain generation completes within 5ms per tile
- Memory usage stays below 8GB for planet system
- Streaming bandwidth stays below 100MB/s
- Draw calls stay below 5000 per frame

### Visual Quality Tests

- Planets look realistic from orbital distance
- No visible LOD popping during approach
- Atmospheric effects match reference images
- Biome transitions appear natural
- Materials respond correctly to lighting

### VR Comfort Tests

- No judder during atmospheric entry
- Smooth camera movement at all altitudes
- No disorienting visual artifacts
- Consistent frame timing (< 2ms variance)
- Proper IPD and scale throughout

## Performance Targets

- **Frame Rate**: 90 FPS minimum in VR (11.1ms frame time)
- **Terrain Generation**: < 5ms per tile
- **LOD Transitions**: < 1ms per transition
- **Memory Usage**: < 8GB for complete planet system
- **Streaming**: < 100MB/s bandwidth
- **Draw Calls**: < 5000 per frame
- **Visible Triangles**: < 10 million
- **Texture Memory**: < 4GB for terrain textures

## Implementation Notes

### Unreal Engine 5.6 Features to Leverage

1. **Nanite**: Use for terrain meshes to eliminate LOD popping
2. **Lumen**: Global illumination for realistic lighting
3. **Virtual Shadow Maps**: High-quality shadows from sun and clouds
4. **World Partition**: Stream massive planet surfaces
5. **Niagara**: Particle effects for weather and atmosphere
6. **Chaos Physics**: Realistic vehicle and character physics
7. **MetaSounds**: Dynamic audio for weather and environment

### Third-Party Plugins (Optional)

- **Voxel Plugin**: Alternative terrain system with caves
- **Ultra Dynamic Sky**: Enhanced atmospheric effects
- **Landscape Pro**: Advanced terrain tools
- **Runtime Mesh Component**: Dynamic mesh generation

### 11. Farming System Integration

**Purpose**: Enable players to establish and manage farms on planetary surfaces with realistic crop growth and environmental interactions.

**Farming Architecture**:

```cpp
UCLASS()
class UFarmingSubsystem : public UWorldSubsystem
{
    UPROPERTY()
    TArray<AFarmPlot*> ActiveFarms;

    UPROPERTY()
    UFarmingDatabase* CropDatabase;

public:
    // Create new farm plot
    AFarmPlot* CreateFarmPlot(FVector Location, FVector2D Size);

    // Update all active farms
    void UpdateFarms(float DeltaTime);

    // Check if location is suitable for farming
    bool IsSuitableForFarming(FVector Location, FBiomeDefinition Biome);
};

UCLASS()
class AFarmPlot : public AActor
{
    GENERATED_BODY()

    // Plot Properties
    UPROPERTY(EditAnywhere, Category = "Farm")
    FVector2D PlotSize; // meters

    UPROPERTY(EditAnywhere, Category = "Farm")
    int32 GridResolution; // cells per side

    UPROPERTY()
    TArray<FCropCell> CropGrid;

    // Environmental Factors
    UPROPERTY()
    float SoilQuality; // 0-1

    UPROPERTY()
    float WaterLevel; // 0-1

    UPROPERTY()
    float Fertility; // 0-1, depletes with use

    // Visual Components
    UPROPERTY(VisibleAnywhere)
    UInstancedStaticMeshComponent* CropInstances;

    UPROPERTY(VisibleAnywhere)
    UProceduralMeshComponent* SoilMesh;

public:
    // Plant crop at grid position
    bool PlantCrop(FIntPoint GridPosition, UCropDefinition* CropType);

    // Harvest crop at position
    FHarvestResult HarvestCrop(FIntPoint GridPosition);

    // Water the plot
    void WaterPlot(float WaterAmount);

    // Fertilize the plot
    void FertilizePlot(float FertilizerAmount);

    // Update crop growth
    void UpdateCropGrowth(float DeltaTime);

    // Get environmental conditions
    FFarmEnvironment GetEnvironmentalConditions();
};

struct FCropCell
{
    UCropDefinition* CropType;
    float GrowthProgress; // 0-1
    float Health; // 0-1
    bool bNeedsWater;
    bool bHasPests;
    FDateTime PlantedTime;
};

UCLASS(BlueprintType)
class UCropDefinition : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Identity")
    FString CropName;

    UPROPERTY(EditAnywhere, Category = "Identity")
    UStaticMesh* CropMesh;

    UPROPERTY(EditAnywhere, Category = "Identity")
    TArray<UStaticMesh*> GrowthStageMeshes; // 4-6 stages

    // Growth Requirements
    UPROPERTY(EditAnywhere, Category = "Growth")
    float GrowthDuration; // real-time seconds

    UPROPERTY(EditAnywhere, Category = "Growth")
    FFloatRange OptimalTemperature; // Celsius

    UPROPERTY(EditAnywhere, Category = "Growth")
    FFloatRange OptimalHumidity; // 0-1

    UPROPERTY(EditAnywhere, Category = "Growth")
    float WaterConsumption; // per day

    UPROPERTY(EditAnywhere, Category = "Growth")
    float SoilQualityRequirement; // 0-1

    UPROPERTY(EditAnywhere, Category = "Growth")
    TArray<EBiomeType> SuitableBiomes;

    // Yield
    UPROPERTY(EditAnywhere, Category = "Harvest")
    FIntRange YieldAmount;

    UPROPERTY(EditAnywhere, Category = "Harvest")
    UItemDefinition* HarvestItem;

    UPROPERTY(EditAnywhere, Category = "Harvest")
    float SellPrice;
};
```

**Farming Features**:

1. **Soil System**:

   - Soil quality varies by biome and location
   - Fertility depletes with repeated planting
   - Requires fertilizer to restore fertility
   - Erosion affects soil quality over time
   - Visual representation of soil health (color, texture)

2. **Crop Growth Simulation**:

   - Real-time growth based on environmental conditions
   - Multiple growth stages with visual changes
   - Temperature affects growth rate
   - Humidity affects water needs
   - Sunlight exposure (day/night cycle impact)
   - Weather effects (rain waters crops, storms damage them)

3. **Irrigation System**:

   - Manual watering by player
   - Automated irrigation systems (buildable)
   - Rain provides natural watering
   - Water table depth varies by location
   - Wells can be dug for water access

4. **Environmental Integration**:

   - Biome determines suitable crops
   - Temperature from planet's climate system
   - Humidity from weather system
   - Seasonal variations (if planet has seasons)
   - Day/night cycle affects growth

5. **Farm Management**:

   - Grid-based planting system
   - Crop rotation for soil health
   - Pest management (optional challenge)
   - Disease spread between crops
   - Harvest timing affects yield quality

6. **VR Interaction**:
   - Hand-plant seeds with motion controllers
   - Physical watering can interaction
   - Grab and harvest crops
   - Inspect crop health up close
   - Use farming tools (hoe, rake, etc.)

**Farming-Planet Integration Points**:

```cpp
// In APlanet class
UPROPERTY()
UFarmingSubsystem* FarmingSubsystem;

// Biome affects farming
struct FBiomeDefinition
{
    // ... existing properties ...

    UPROPERTY(EditAnywhere)
    float BaseSoilQuality; // 0-1

    UPROPERTY(EditAnywhere)
    TArray<UCropDefinition*> NativeCrops;

    UPROPERTY(EditAnywhere)
    float WaterTableDepth; // meters
};

// Weather affects farming
class UWeatherComponent
{
    // ... existing properties ...

    // Notify farms of rain
    void OnRainStart();
    void OnRainEnd();

    // Get precipitation amount
    float GetPrecipitationRate(); // mm per hour
};

// Terrain provides farming locations
class UTerrainGenerationSubsystem
{
    // ... existing methods ...

    // Find flat areas suitable for farming
    TArray<FFarmableLocation> FindFarmableLocations(FVector2D SearchArea);

    // Get soil composition at location
    FSoilComposition GetSoilAtLocation(FVector Location);
};
```

**Farming Progression**:

1. **Basic Farming** (Early Game):

   - Small hand-tilled plots
   - Manual watering
   - Simple crops (wheat, vegetables)
   - Harvest by hand

2. **Advanced Farming** (Mid Game):

   - Larger mechanized plots
   - Irrigation systems
   - Diverse crop types
   - Automated harvesting

3. **Industrial Farming** (Late Game):
   - Massive farm complexes
   - Hydroponics/aeroponics
   - Genetically modified crops
   - Fully automated systems

**Visual Quality**:

- High-detail crop models with LOD
- Realistic plant animation (wind sway)
- Growth stages clearly visible
- Soil texture changes with health
- Water puddles after rain
- Footprints in tilled soil
- Tool marks and furrows

### Development Phases

**Phase 1** (Weeks 1-2): Enhanced terrain with LOD
**Phase 2** (Weeks 3-4): Atmospheric system and clouds
**Phase 3** (Weeks 5-6): Biome system and materials
**Phase 4** (Weeks 7-8): Weather and dynamic effects
**Phase 5** (Weeks 9-10): Space-to-surface transition
**Phase 6** (Weeks 11-12): Cave systems and polish
**Phase 7** (Weeks 13-14): Farming system integration
**Phase 8** (Weeks 15-16): Farming VR interactions and polish
