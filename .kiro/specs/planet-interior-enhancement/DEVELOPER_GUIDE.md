# Planet Interior Enhancement - Developer Guide

## Table of Contents

1. [Planet Configuration Workflow](#planet-configuration-workflow)
2. [Biome Creation Guide](#biome-creation-guide)
3. [Farming System Tutorial](#farming-system-tutorial)
4. [Performance Best Practices](#performance-best-practices)
5. [Common Issues and Solutions](#common-issues-and-solutions)

## Planet Configuration Workflow

### Overview

The planet system uses a data-driven approach where each planet is configured through a `UPlanetConfiguration` DataAsset. This guide walks through creating and configuring a new planet.

### Step 1: Create Planet Configuration Asset

1. In Content Browser, right-click and select **Miscellaneous > Data Asset**
2. Choose `UPlanetConfiguration` as the class
3. Name it `DA_Planet_[YourPlanetName]` (e.g., `DA_Planet_Verdant`)
4. Open the asset to begin configuration

### Step 2: Configure Basic Properties

```cpp
// Identity Settings
Planet Name: "Verdant"
Planet Type: Terrestrial

// Physical Properties
Radius: 6371.0 km (Earth-like)
Mass: 5.972e24 kg
Rotation Period: 24.0 hours
Axial Tilt: 23.5 degrees
```

### Step 3: Configure Terrain

The terrain configuration controls the procedural generation of the planet's surface.

```cpp
// Terrain Config
Terrain Seed: 12345 (any integer for reproducible generation)
Base Elevation: 0.0 (sea level reference)
Elevation Variance: 8000.0 (max height difference in meters)
Roughness: 0.6 (0-1, higher = more detailed terrain)
```

**Terrain Parameters Explained:**

- **Seed**: Determines the random generation pattern. Same seed = same planet
- **Base Elevation**: Reference point for sea level (usually 0)
- **Elevation Variance**: Maximum height of mountains above sea level
- **Roughness**: Controls terrain detail frequency (0.3 = smooth, 0.8 = very rough)

### Step 4: Configure Atmosphere

```cpp
// Atmosphere Settings
Has Atmosphere: true

// Scattering Parameters (for Earth-like atmosphere)
Rayleigh Scattering Coefficient: (5.8, 13.5, 33.1) // RGB values
Rayleigh Scale Height: 8.0 km
Mie Scattering Coefficient: (2.0, 2.0, 2.0)
Mie Scale Height: 1.2 km
Mie Anisotropy: 0.8 (forward scattering)

// Atmosphere Properties
Atmosphere Height: 100.0 km
Ground Albedo: (0.3, 0.3, 0.3) // Surface reflectivity
Fog Density: 0.02
Fog Height Falloff: 0.2
```

**Atmosphere Tips:**

- Blue skies: Increase Rayleigh blue channel (33.1)
- Red/orange sunsets: Increase Mie scattering
- Alien atmospheres: Adjust RGB ratios (e.g., purple sky: high red + blue)
- Thicker atmosphere: Increase fog density and scale heights

### Step 5: Add Biomes

See [Biome Creation Guide](#biome-creation-guide) for detailed biome setup.

### Step 6: Configure Weather

```cpp
// Weather Presets
Add multiple weather presets for variety:

Clear Weather:
- Cloud Coverage: 0.1
- Precipitation: None
- Wind Speed: 5 m/s

Rainy Weather:
- Cloud Coverage: 0.8
- Precipitation: Rain
- Wind Speed: 15 m/s

Storm Weather:
- Cloud Coverage: 1.0
- Precipitation: Heavy Rain
- Wind Speed: 30 m/s
```

### Step 7: Spawn Planet in Level

```cpp
// In your level blueprint or C++
APlanet* Planet = GetWorld()->SpawnActor<APlanet>(APlanet::StaticClass());
Planet->SetPlanetConfiguration(YourConfigAsset);
Planet->GeneratePlanetTerrain();
```

## Biome Creation Guide

### Understanding Biomes

Biomes are distinct environmental regions with unique terrain, vegetation, climate, and visual characteristics. Each planet can have multiple biomes that blend naturally.

### Creating a New Biome

#### Step 1: Define Biome Data Structure

Create a new `FBiomeDefinition` in your planet configuration:

```cpp
Biome Name: "Temperate Forest"
Biome Color: (0.2, 0.6, 0.2) // Green for debug visualization

// Terrain Properties
Base Elevation: 100.0 meters (average height)
Elevation Variance: 500.0 meters (hills and valleys)
Roughness: 0.5 (moderate detail)
```

#### Step 2: Configure Material Layers

Each biome uses a layered material system. Add layers in order of priority:

```cpp
Material Layers:

Layer 0 - Base Soil:
- Material: M_Soil_Rich
- Blend Weight: 1.0 (always present)
- Altitude Range: 0-1000m

Layer 1 - Grass:
- Material: M_Grass_Temperate
- Blend Weight: 0.8
- Altitude Range: 0-800m
- Slope Limit: 0-30 degrees

Layer 2 - Rock (steep slopes):
- Material: M_Rock_Granite
- Blend Weight: 1.0
- Slope Threshold: 45+ degrees

Layer 3 - Forest Floor:
- Material: M_ForestFloor_Leaves
- Blend Weight: 0.6
- Requires: Dense vegetation nearby
```

**Material Layer Tips:**

- Lower layers are base, higher layers overlay
- Use slope angle to automatically place cliff materials
- Altitude ranges create natural transitions (grass to snow)
- Blend weights control layer visibility (0 = invisible, 1 = opaque)

#### Step 3: Add Vegetation

```cpp
Vegetation Types:

Oak Tree:
- Mesh: SM_Oak_Tree_01
- Density: 0.3 (30% coverage)
- Min Scale: 0.8
- Max Scale: 1.2
- Altitude Range: 0-600m
- Slope Limit: 0-25 degrees

Grass Clumps:
- Mesh: SM_Grass_Clump
- Density: 0.9 (90% coverage)
- Min Scale: 0.5
- Max Scale: 1.5
- Altitude Range: 0-800m
- Slope Limit: 0-35 degrees

Bushes:
- Mesh: SM_Bush_01
- Density: 0.4
- Min Scale: 0.7
- Max Scale: 1.3
```

**Vegetation Best Practices:**

- Use LOD meshes for performance
- Keep total density under 1.0 to avoid overlap
- Use instanced static meshes for rendering
- Limit high-poly vegetation to close range
- Consider VR performance (target 90 FPS)

#### Step 4: Configure Environmental Parameters

```cpp
// Climate Settings
Temperature: 15.0 Celsius (average)
Humidity: 0.6 (60% - moderate moisture)

// Allowed Weather Types
- Clear
- Partly Cloudy
- Light Rain
- Heavy Rain
- Fog

// Seasonal Variation (optional)
Temperature Range: 5.0 to 25.0 Celsius
Humidity Range: 0.4 to 0.8
```

#### Step 5: Set Biome Distribution

Biomes are distributed across the planet using noise-based rules:

```cpp
// In Planet Configuration
Biome Distribution Rules:

Temperate Forest:
- Latitude Range: 30-60 degrees (mid-latitudes)
- Altitude Range: 0-1000m
- Moisture Requirement: 0.5-0.9 (high)
- Temperature Range: 5-25 Celsius

Desert:
- Latitude Range: 15-35 degrees (subtropical)
- Altitude Range: 0-500m
- Moisture Requirement: 0.0-0.3 (low)
- Temperature Range: 20-45 Celsius

Tundra:
- Latitude Range: 60-90 degrees (polar)
- Altitude Range: 0-500m
- Moisture Requirement: 0.2-0.6
- Temperature Range: -30 to 5 Celsius
```

### Biome Blending

Biomes automatically blend at boundaries using a 50-100m transition zone:

```cpp
// Blend calculation (automatic)
float BlendWeight = CalculateBiomeBlend(
    CurrentLocation,
    BiomeA_Center,
    BiomeB_Center,
    TransitionWidth
);

// Results in smooth material and vegetation transitions
```

### Testing Your Biome

1. **Visual Check**: Fly over the planet and verify biome appearance
2. **Transition Check**: Look for smooth blending between biomes
3. **Performance Check**: Monitor FPS in biome (target 90+ in VR)
4. **Vegetation Check**: Ensure proper density and distribution
5. **Weather Check**: Test all weather types in the biome

### Common Biome Types Reference

**Desert Biome:**

- Sand materials, cacti, rocks
- Low humidity (0.1-0.3), high temperature (25-45°C)
- Sandstorm weather, clear skies
- Minimal vegetation density (0.1-0.2)

**Ocean Biome:**

- Water shader, underwater terrain
- High humidity (0.9-1.0), moderate temperature (10-20°C)
- Rain, fog, storms
- Underwater vegetation (kelp, coral)

**Volcanic Biome:**

- Lava materials, ash, obsidian
- Low humidity (0.1-0.3), very high temperature (30-60°C)
- Ash clouds, heat shimmer
- Minimal vegetation, heat-resistant plants

**Ice/Tundra Biome:**

- Snow and ice materials
- Low humidity (0.2-0.5), very low temperature (-40 to 0°C)
- Snow, blizzards, clear cold
- Sparse vegetation (moss, small shrubs)

## Farming System Tutorial

### Getting Started with Farming

The farming system allows players to establish farms on planetary surfaces, grow crops, and manage agricultural operations.

### Step 1: Understanding Farm Requirements

Before creating a farm, understand the requirements:

```cpp
// Location Requirements
- Slope: Less than 15 degrees (relatively flat)
- Biome: Must support farming (check biome's BaseSoilQuality > 0.3)
- Accessibility: Near landing zone or player base
- Water: Access to water table or irrigation

// Environmental Factors
- Temperature: Affects crop growth rate
- Humidity: Affects water consumption
- Sunlight: Day/night cycle impacts growth
- Weather: Rain waters crops, storms damage them
```

### Step 2: Creating a Farm Plot

```cpp
// In C++ or Blueprint
UFarmingSubsystem* FarmingSystem = GetWorld()->GetSubsystem<UFarmingSubsystem>();

// Check if location is suitable
FVector FarmLocation = GetDesiredLocation();
FBiomeDefinition CurrentBiome = Planet->GetBiomeAtLocation(FarmLocation);

if (FarmingSystem->IsSuitableForFarming(FarmLocation, CurrentBiome))
{
    // Create farm plot
    AFarmPlot* NewFarm = FarmingSystem->CreateFarmPlot(
        FarmLocation,
        FVector2D(20.0f, 20.0f) // 20m x 20m plot
    );

    // Configure plot
    NewFarm->GridResolution = 10; // 10x10 grid = 100 planting spots
}
```

### Step 3: Understanding Crop Definitions

Each crop has specific requirements and characteristics:

```cpp
// Example: Wheat Crop
Crop Name: "Wheat"
Growth Duration: 600 seconds (10 minutes real-time)

// Requirements
Optimal Temperature: 15-25 Celsius
Optimal Humidity: 0.4-0.7
Water Consumption: 50 liters per day
Soil Quality Requirement: 0.5 (moderate)
Suitable Biomes: Grassland, Temperate Forest

// Yield
Yield Amount: 10-15 units
Harvest Item: Wheat Grain
Sell Price: 5 credits per unit

// Growth Stages (4 stages)
Stage 0: Seedling (0-25% growth)
Stage 1: Young Plant (25-50% growth)
Stage 2: Mature Plant (50-75% growth)
Stage 3: Ready to Harvest (75-100% growth)
```

### Step 4: Planting Crops

```cpp
// Get crop definition
UCropDefinition* WheatCrop = LoadObject<UCropDefinition>(
    nullptr,
    TEXT("/Game/Farming/Crops/DA_Crop_Wheat")
);

// Plant at grid position
FIntPoint GridPos(5, 5); // Center of 10x10 grid
bool bSuccess = FarmPlot->PlantCrop(GridPos, WheatCrop);

if (bSuccess)
{
    UE_LOG(LogFarming, Log, TEXT("Wheat planted successfully"));
}
```

### Step 5: Managing Crop Growth

Crops grow automatically based on environmental conditions:

```cpp
// Growth calculation (automatic in AFarmPlot::UpdateCropGrowth)
float GrowthRate = CalculateGrowthRate(
    CurrentTemperature,
    CurrentHumidity,
    SoilQuality,
    WaterLevel,
    SunlightExposure
);

// Growth modifiers
if (Temperature < OptimalMin || Temperature > OptimalMax)
    GrowthRate *= 0.5; // 50% slower outside optimal range

if (WaterLevel < 0.3)
    GrowthRate *= 0.3; // Severe penalty for drought

if (SoilQuality < CropRequirement)
    GrowthRate *= SoilQuality / CropRequirement;

// Apply growth
CropCell.GrowthProgress += GrowthRate * DeltaTime / GrowthDuration;
```

### Step 6: Watering and Maintenance

```cpp
// Manual watering
FarmPlot->WaterPlot(100.0f); // Add 100 liters of water

// Fertilizing
FarmPlot->FertilizePlot(50.0f); // Add 50 units of fertilizer

// Check status
FFarmEnvironment Conditions = FarmPlot->GetEnvironmentalConditions();
UE_LOG(LogFarming, Log, TEXT("Soil Quality: %f"), Conditions.SoilQuality);
UE_LOG(LogFarming, Log, TEXT("Water Level: %f"), Conditions.WaterLevel);
UE_LOG(LogFarming, Log, TEXT("Fertility: %f"), Conditions.Fertility);
```

### Step 7: Harvesting

```cpp
// Harvest crop at grid position
FIntPoint HarvestPos(5, 5);
FHarvestResult Result = FarmPlot->HarvestCrop(HarvestPos);

if (Result.bSuccess)
{
    UE_LOG(LogFarming, Log, TEXT("Harvested %d %s"),
        Result.Quantity,
        *Result.ItemName);

    // Add to player inventory
    PlayerInventory->AddItem(Result.ItemDefinition, Result.Quantity);
}
```

### Step 8: Automated Farming Systems

For large-scale farming, use automation:

```cpp
// Place irrigation system
AIrrigationSystem* Irrigation = GetWorld()->SpawnActor<AIrrigationSystem>(
    AIrrigationSystem::StaticClass(),
    FarmLocation
);
Irrigation->SetCoverageRadius(10.0f); // Waters 10m radius
Irrigation->SetWaterRate(100.0f); // 100 liters per hour

// Place harvesting machine
AHarvestingMachine* Harvester = GetWorld()->SpawnActor<AHarvestingMachine>(
    AHarvestingMachine::StaticClass(),
    FarmLocation
);
Harvester->SetTargetFarmPlot(FarmPlot);
Harvester->SetHarvestInterval(600.0f); // Check every 10 minutes

// Power system
APowerSystem* PowerSystem = GetWorld()->SpawnActor<APowerSystem>(
    APowerSystem::StaticClass(),
    FarmLocation
);
PowerSystem->AddPowerConsumer(Irrigation);
PowerSystem->AddPowerConsumer(Harvester);

// Add solar panels
ASolarPanel* Solar = GetWorld()->SpawnActor<ASolarPanel>(
    ASolarPanel::StaticClass(),
    FarmLocation + FVector(0, 0, 500)
);
PowerSystem->AddPowerGenerator(Solar);
```

### Farming Best Practices

1. **Location Selection:**

   - Choose biomes with high soil quality (0.7+)
   - Flat terrain (slope < 10 degrees)
   - Near water sources
   - Protected from severe weather

2. **Crop Selection:**

   - Match crops to biome (check SuitableBiomes array)
   - Consider temperature and humidity ranges
   - Plan for seasonal variations
   - Diversify crops for risk management

3. **Resource Management:**

   - Monitor water levels daily
   - Fertilize after 3-5 harvests
   - Rotate crops to maintain soil health
   - Use automation for efficiency

4. **Performance Optimization:**
   - Limit farm plot size (max 20x20 recommended)
   - Use instanced meshes for crops
   - Update growth in batches
   - Cull distant farm plots

## Performance Best Practices

### VR Performance Requirements

The planet system must maintain 90 FPS minimum in VR to prevent motion sickness:

```cpp
// Target Performance Metrics
Frame Time: < 11.1ms (90 FPS)
Frame Time Variance: < 2ms (consistent timing)
Draw Calls: < 5000 per frame
Visible Triangles: < 10 million
Texture Memory: < 4GB
System Memory: < 8GB
```

### LOD (Level of Detail) Optimization

#### Terrain LOD Configuration

```cpp
// In APlanet or terrain configuration
FLODConfiguration LODSettings;

// LOD distances (adjust based on planet size)
LODSettings.LODDistances = {
    500.0f,    // LOD 0: 0-500m (highest detail)
    2000.0f,   // LOD 1: 500m-2km
    10000.0f,  // LOD 2: 2km-10km
    50000.0f,  // LOD 3: 10km-50km
    200000.0f, // LOD 4: 50km-200km
    1000000.0f // LOD 5: 200km+ (lowest detail)
};

// Vertex density per LOD
LODSettings.VertexDensity = {
    1.0f,   // LOD 0: 1m per vertex
    5.0f,   // LOD 1: 5m per vertex
    25.0f,  // LOD 2: 25m per vertex
    100.0f, // LOD 3: 100m per vertex
    500.0f, // LOD 4: 500m per vertex
    2000.0f // LOD 5: 2km per vertex
};
```

**LOD Best Practices:**

- Use Nanite for automatic LOD (UE5.6 feature)
- Implement geomorphing to smooth LOD transitions
- Update LOD based on viewer position every frame
- Prioritize LOD updates for visible tiles
- Use distance-based texture streaming

#### Vegetation LOD

```cpp
// Vegetation LOD settings
struct FVegetationLOD
{
    float Distance;
    int32 InstanceCount;
    UStaticMesh* LODMesh;
};

// Example: Tree LOD configuration
TArray<FVegetationLOD> TreeLODs = {
    { 100.0f, 1000, HighDetailTreeMesh },  // Close range
    { 500.0f, 5000, MediumDetailTreeMesh }, // Mid range
    { 2000.0f, 10000, LowDetailTreeMesh },  // Far range
    { 10000.0f, 0, nullptr }                // Cull beyond 10km
};
```

### Streaming and Memory Management

#### Terrain Tile Streaming

```cpp
// Streaming configuration
struct FStreamingConfig
{
    int32 MaxActiveTiles = 500;        // Maximum loaded tiles
    float MaxLoadTimePerFrame = 5.0f;  // ms per frame for loading
    float TileUnloadDistance = 5000.0f; // Unload tiles beyond this
    int32 StreamingPoolSize = 2048;    // MB for streaming pool
};

// Tile priority calculation
float CalculateTilePriority(FTerrainTile* Tile, FVector ViewerPos)
{
    float Distance = FVector::Dist(Tile->WorldPosition, ViewerPos);
    float Priority = 1.0f / (Distance + 1.0f);

    // Boost priority for tiles in view frustum
    if (IsInViewFrustum(Tile))
        Priority *= 2.0f;

    return Priority;
}
```

**Streaming Best Practices:**

- Load tiles asynchronously on background threads
- Prioritize tiles by distance and visibility
- Implement LRU (Least Recently Used) cache eviction
- Limit loading to 5ms per frame maximum
- Pre-load tiles in flight direction

### Rendering Optimization

#### Material Optimization

```cpp
// Use material instances for dynamic parameters
UMaterialInstanceDynamic* TerrainMaterial =
    UMaterialInstanceDynamic::Create(TerrainMasterMaterial, this);

// Batch material parameter updates
TerrainMaterial->SetVectorParameterValue("BiomeColor", BiomeColor);
TerrainMaterial->SetScalarParameterValue("Roughness", Roughness);
TerrainMaterial->SetScalarParameterValue("Metallic", Metallic);

// Use texture arrays for biome materials (reduces draw calls)
UTexture2DArray* BiomeTextureArray;
TerrainMaterial->SetTextureParameterValue("BiomeTextures", BiomeTextureArray);
```

**Material Best Practices:**

- Limit material complexity (< 300 instructions)
- Use texture arrays instead of multiple textures
- Minimize texture samples (< 16 per material)
- Use material instances, not unique materials
- Enable material caching
- Use LOD materials for distant objects

#### Instancing and Batching

```cpp
// Use instanced static meshes for vegetation
UInstancedStaticMeshComponent* GrassInstances =
    CreateDefaultSubobject<UInstancedStaticMeshComponent>("GrassInstances");

// Add instances in batches
TArray<FTransform> InstanceTransforms;
for (int32 i = 0; i < 10000; ++i)
{
    FTransform Transform = CalculateGrassTransform(i);
    InstanceTransforms.Add(Transform);
}

// Batch add (more efficient than individual adds)
GrassInstances->AddInstances(InstanceTransforms, false);
GrassInstances->BuildTreeIfOutdated(true, true);
```

**Instancing Best Practices:**

- Use instanced static meshes for repeated objects
- Batch instance additions (don't add one at a time)
- Limit instances per component (< 100,000)
- Use hierarchical instanced static meshes (HISM) for culling
- Enable GPU instancing in project settings

### Physics and Collision Optimization

```cpp
// Simplified collision for terrain
UBodySetup* BodySetup = TerrainMesh->GetBodySetup();
BodySetup->CollisionTraceFlag = CTF_UseSimpleAsComplex;

// Use simple collision shapes
FKBoxElem BoxCollision;
BoxCollision.SetTransform(FTransform::Identity);
BoxCollision.X = TileSize;
BoxCollision.Y = TileSize;
BoxCollision.Z = 100.0f;
BodySetup->AggGeom.BoxElems.Add(BoxCollision);

// Disable collision for distant tiles
if (DistanceToViewer > 1000.0f)
{
    TerrainMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
```

**Physics Best Practices:**

- Use simple collision shapes (boxes, spheres)
- Disable collision for distant objects
- Use collision LOD (simpler shapes at distance)
- Limit physics simulation to nearby objects
- Use async physics for non-critical objects

### CPU Optimization

#### Multi-Threading

```cpp
// Generate terrain tiles on background threads
AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, TileCoords]()
{
    // Generate terrain data (CPU intensive)
    FTerrainTile NewTile = GenerateTerrainTile(TileCoords);

    // Return to game thread for mesh creation
    AsyncTask(ENamedThreads::GameThread, [this, NewTile]()
    {
        CreateTerrainMesh(NewTile);
    });
});
```

**Multi-Threading Best Practices:**

- Generate terrain on background threads
- Process vegetation placement asynchronously
- Calculate LOD transitions off main thread
- Use parallel-for for batch operations
- Avoid locks and synchronization when possible

### GPU Optimization

#### Texture Optimization

```cpp
// Use texture streaming
UTexture2D* TerrainTexture = LoadObject<UTexture2D>(...);
TerrainTexture->LODGroup = TEXTUREGROUP_World;
TerrainTexture->MipGenSettings = TMGS_FromTextureGroup;
TerrainTexture->bUseLegacyGamma = false;

// Enable texture compression
TerrainTexture->CompressionSettings = TC_Default;
TerrainTexture->SRGB = true;

// Set appropriate texture sizes
// Base Color: 2048x2048
// Normal: 2048x2048
// Roughness/Metallic: 1024x1024 (can be smaller)
```

**Texture Best Practices:**

- Use texture streaming (automatic in UE5)
- Compress textures appropriately (BC1, BC3, BC5)
- Use mipmaps for all textures
- Limit texture resolution (2048x2048 max for terrain)
- Use texture atlases to reduce draw calls
- Enable virtual texturing for large terrains

#### Draw Call Reduction

```cpp
// Combine meshes where possible
UProceduralMeshComponent* CombinedMesh = CreateCombinedMesh(
    SmallRocks,
    Pebbles,
    GroundClutter
);

// Use material instances to batch draws
for (UStaticMeshComponent* Mesh : TerrainMeshes)
{
    Mesh->SetMaterial(0, SharedMaterialInstance);
}

// Enable mesh merging in project settings
// Edit > Project Settings > Engine > Rendering
// Enable "Merge Static Meshes"
```

**Draw Call Best Practices:**

- Target < 5000 draw calls per frame
- Use instancing for repeated objects
- Merge static meshes where appropriate
- Use material instances, not unique materials
- Enable GPU Scene (UE5 feature)
- Use Nanite for automatic optimization

### Profiling and Debugging

#### Performance Profiling Commands

```cpp
// In-game console commands (press ` key)

// Show frame time breakdown
stat unit

// Show rendering stats
stat fps
stat gpu
stat scenerendering

// Show memory usage
stat memory
stat streaming

// Show draw calls
stat rhi

// Show LOD information
stat levels

// Profile specific systems
stat game      // Game thread performance
stat physics   // Physics simulation
stat streaming // Asset streaming
```

#### Unreal Insights

```cpp
// Enable tracing
// Console: trace.start
// Or command line: -trace=cpu,gpu,frame,log

// Capture session
// Console: trace.stop
// Open in Unreal Insights for detailed analysis

// Key metrics to monitor:
// - Frame time consistency
// - GPU bottlenecks
// - CPU thread utilization
// - Memory allocations
// - Asset loading times
```

### Performance Targets by Platform

```cpp
// VR Headsets (High Priority)
Target FPS: 90 (11.1ms frame time)
Minimum FPS: 72 (13.9ms frame time)
Resolution: 2016x2240 per eye (Quest 2)
Draw Calls: < 3000
Triangles: < 5 million visible

// Desktop VR (PC)
Target FPS: 90-120
Resolution: 2880x1600 per eye (Index)
Draw Calls: < 5000
Triangles: < 10 million visible

// Flat Screen (Non-VR)
Target FPS: 60
Resolution: 1920x1080 to 3840x2160
Draw Calls: < 8000
Triangles: < 15 million visible
```

## Common Issues and Solutions

### Issue 1: LOD Popping

**Symptom:** Visible "popping" when terrain switches between detail levels.

**Solution:**

```cpp
// Enable geomorphing for smooth transitions
FLODConfiguration LODSettings;
LODSettings.bEnableGeomorphing = true;
LODSettings.GeomorphingDuration = 0.5f; // 0.5 second transition

// Implement vertex morphing in material
// Lerp between LOD positions based on distance
FVector MorphedPosition = FMath::Lerp(
    CurrentLODPosition,
    NextLODPosition,
    MorphAlpha
);
```

### Issue 2: Terrain Streaming Stutters

**Symptom:** Frame rate drops when loading new terrain tiles.

**Solution:**

```cpp
// Limit loading time per frame
float MaxLoadTime = 5.0f; // ms
float StartTime = FPlatformTime::Seconds();

while (HasTilesToLoad() &&
       (FPlatformTime::Seconds() - StartTime) < MaxLoadTime)
{
    LoadNextTile();
}

// Use async loading
FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
Streamable.RequestAsyncLoad(
    TilesToLoad,
    FStreamableDelegate::CreateUObject(this, &APlanet::OnTilesLoaded)
);
```

### Issue 3: Biome Transitions Look Harsh

**Symptom:** Sharp boundaries between biomes instead of smooth blending.

**Solution:**

```cpp
// Increase transition zone width
float TransitionWidth = 100.0f; // meters

// Use smooth interpolation
float BiomeBlend = FMath::SmoothStep(0.0f, 1.0f, BlendAlpha);

// Blend multiple biome properties
FLinearColor BlendedColor =
    BiomeA.Color * (1.0f - BiomeBlend) +
    BiomeB.Color * BiomeBlend;

// Use noise to vary transition
float NoiseValue = PerlinNoise(Location.X, Location.Y);
BiomeBlend += NoiseValue * 0.2f; // Add variation
BiomeBlend = FMath::Clamp(BiomeBlend, 0.0f, 1.0f);
```

### Issue 4: Atmospheric Entry Effects Not Visible

**Symptom:** No visual effects when entering atmosphere.

**Solution:**

```cpp
// Ensure atmosphere component is active
AtmosphereComponent->SetActive(true);
AtmosphereComponent->SetVisibility(true);

// Check altitude threshold
float AtmosphereStartAltitude = 100000.0f; // 100km
if (CurrentAltitude < AtmosphereStartAltitude)
{
    // Enable atmospheric effects
    EnableAtmosphericFog(true);
    EnableAtmosphericScattering(true);
}

// Verify material parameters
UMaterialInstanceDynamic* AtmoMaterial =
    AtmosphereComponent->GetDynamicMaterial();
AtmoMaterial->SetScalarParameterValue("Density", 0.02f);
AtmoMaterial->SetScalarParameterValue("Height", 100.0f);
```

### Issue 5: Crops Not Growing

**Symptom:** Planted crops remain at 0% growth.

**Solution:**

```cpp
// Verify farm plot is updating
void AFarmPlot::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateCropGrowth(DeltaTime); // Ensure this is called
}

// Check environmental conditions
FFarmEnvironment Env = GetEnvironmentalConditions();
UE_LOG(LogFarming, Warning, TEXT("Temperature: %f"), Env.Temperature);
UE_LOG(LogFarming, Warning, TEXT("Water Level: %f"), Env.WaterLevel);
UE_LOG(LogFarming, Warning, TEXT("Soil Quality: %f"), Env.SoilQuality);

// Verify crop requirements are met
if (Env.Temperature < Crop->OptimalTemperature.Min ||
    Env.Temperature > Crop->OptimalTemperature.Max)
{
    UE_LOG(LogFarming, Warning, TEXT("Temperature out of range!"));
}
```

### Issue 6: VR Performance Below 90 FPS

**Symptom:** Frame rate drops in VR causing discomfort.

**Solution:**

```cpp
// Enable VR-specific optimizations
UVRPerformanceOptimizer* VROptimizer =
    GetWorld()->GetSubsystem<UVRPerformanceOptimizer>();

VROptimizer->SetDynamicResolution(true);
VROptimizer->SetFoveatedRendering(true);
VROptimizer->SetAggressiveLOD(true);

// Reduce draw distance
GEngine->GetWorldContextFromWorld(GetWorld())->World()->
    GetWorldSettings()->MaxDrawDistance = 5000.0f;

// Lower vegetation density
BiomeDefinition.VegetationDensity *= 0.5f;

// Use simpler materials
TerrainMaterial->SetScalarParameterValue("DetailLevel", 0.5f);
```

### Issue 7: Memory Leaks

**Symptom:** Memory usage increases over time, eventual crash.

**Solution:**

```cpp
// Properly clean up terrain tiles
void FQuadTreeTileManager::UnloadTile(FTerrainTile* Tile)
{
    if (Tile->MeshComponent)
    {
        Tile->MeshComponent->DestroyComponent();
        Tile->MeshComponent = nullptr;
    }

    // Clear arrays
    Tile->Vertices.Empty();
    Tile->Indices.Empty();
    Tile->Normals.Empty();

    Tile->bIsLoaded = false;
}

// Use weak pointers for references
TWeakObjectPtr<AFarmPlot> FarmPlotRef;

// Implement proper cleanup
void APlanet::BeginDestroy()
{
    // Clean up subsystems
    if (TerrainManager)
    {
        TerrainManager->UnloadAllTiles();
        TerrainManager = nullptr;
    }

    Super::BeginDestroy();
}
```

### Issue 8: Weather Not Changing

**Symptom:** Weather stays the same, no transitions.

**Solution:**

```cpp
// Verify weather component is ticking
WeatherComponent->SetComponentTickEnabled(true);

// Check weather change timer
void UWeatherComponent::Tick(float DeltaTime)
{
    WeatherTimer += DeltaTime;

    if (WeatherTimer >= WeatherChangeInterval)
    {
        WeatherTimer = 0.0f;
        SelectNewWeather();
    }

    UpdateWeatherTransition(DeltaTime);
}

// Ensure weather presets are configured
if (WeatherPresets.Num() == 0)
{
    UE_LOG(LogWeather, Error, TEXT("No weather presets configured!"));
}
```

### Getting Help

If you encounter issues not covered here:

1. **Check Logs:** Look in `Saved/Logs/` for error messages
2. **Use Profiler:** Run `stat unit` and `stat gpu` to identify bottlenecks
3. **Enable Debugging:** Set `LogPlanet` and `LogFarming` to Verbose
4. **Review Documentation:** Check QUICKSTART guides in `.kiro/specs/`
5. **Test in Isolation:** Create minimal test case to reproduce issue

### Additional Resources

- **Unreal Engine Documentation:** https://docs.unrealengine.com
- **VR Best Practices:** https://docs.unrealengine.com/en-US/SharingAndReleasing/XRDevelopment/VR/
- **Performance Guidelines:** https://docs.unrealengine.com/en-US/TestingAndOptimization/PerformanceAndProfiling/
- **Material Editor:** https://docs.unrealengine.com/en-US/RenderingAndGraphics/Materials/

---

**Document Version:** 1.0  
**Last Updated:** 2025-11-04  
**Maintained By:** Alexander Development Team
