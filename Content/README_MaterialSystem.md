# Terrain Material System

## Overview

The Terrain Material System provides a comprehensive PBR (Physically Based Rendering) material solution for planetary surfaces with support for 8 layered materials, biome blending, and advanced features like triplanar projection, parallax occlusion mapping, and dynamic tessellation.

## Architecture

### Core Components

1. **UTerrainMaterialSystem** - Main material management class
2. **Material Layers** - 8 configurable material slots
3. **PBR Texture Sets** - Complete texture sets for each layer
4. **Advanced Features** - Parallax, tessellation, wetness, deformation

### Material Layer Slots

The system supports 8 material layers that can be configured independently:

0. **Base Rock** - Foundation rock material
1. **Dirt/Soil** - Ground soil material
2. **Grass** - Vegetation ground cover
3. **Sand** - Sandy terrain
4. **Snow** - Snow and ice
5. **Cliff Rock** - Vertical cliff faces (with triplanar projection)
6. **Wet Surface** - Dynamic wetness overlay
7. **Custom** - Special purpose material

## PBR Texture Sets

Each material layer supports a complete PBR texture set:

- **Base Color (Albedo)** - Surface color
- **Normal Map** - Surface detail and lighting
- **Roughness** - Surface roughness (0 = smooth, 1 = rough)
- **Metallic** - Metallic properties (0 = non-metal, 1 = metal)
- **Ambient Occlusion** - Cavity shading
- **Height/Displacement** - Surface depth for parallax

### Texture Tiling

- **Micro Detail** - High-frequency detail textures (1-10m scale)
- **Macro Detail** - Low-frequency textures for distance (100m+ scale)
- **Distance Blending** - Automatic transition based on view distance

## Material Blending

### Biome-Based Blending

Materials automatically blend based on biome weights:

```cpp
// Get blended parameters at location
FBlendedTerrainParameters Params = Planet->GetBlendedBiomeParameters(Location);

// Apply to material
MaterialSystem->SetBiomeBlendWeights(MaterialInstance, Params.BiomeWeights);
```

### Slope-Based Blending

Cliff materials automatically appear on steep slopes:

```cpp
// Configure cliff material for slopes > 45 degrees
LayerConfig.SlopeRange = FVector2D(45.0f, 90.0f);
LayerConfig.bUseTriplanarProjection = true;
```

### Altitude-Based Blending

Snow and other materials blend based on altitude:

```cpp
// Configure snow for high altitudes
LayerConfig.AltitudeRange = FVector2D(2000.0f, 10000.0f); // meters
```

## Advanced Features

### Triplanar Projection

Eliminates texture stretching on vertical surfaces:

```cpp
LayerConfig.bUseTriplanarProjection = true;
LayerConfig.TriplanarSharpness = 4.0f; // Higher = sharper transitions
```

### Parallax Occlusion Mapping

Adds depth to surfaces without additional geometry:

```cpp
AdvancedFeatures.bEnableParallaxOcclusion = true;
AdvancedFeatures.ParallaxDepthScale = 0.05f; // Depth amount
```

### Dynamic Tessellation

Adds geometric detail based on camera distance:

```cpp
AdvancedFeatures.bEnableTessellation = true;
AdvancedFeatures.TessellationMultiplier = 1.0f;
AdvancedFeatures.MaxTessellationDistance = 50.0f; // meters
```

### Dynamic Wetness

Weather-driven surface wetness:

```cpp
AdvancedFeatures.bEnableDynamicWetness = true;

// Update based on weather
float WetnessAmount = WeatherComponent->GetWetnessAmount();
MaterialSystem->UpdateWetness(MaterialInstance, WetnessAmount);
```

## Usage Examples

### Basic Setup

```cpp
// In Planet BeginPlay
void APlanet::InitializeMaterialSystem()
{
    // Create material system
    MaterialSystem = NewObject<UTerrainMaterialSystem>(this);

    // Initialize with master material
    MaterialSystem->Initialize(TerrainMasterMaterial);

    // Configure layers
    ConfigureMaterialLayers();
}
```

### Configuring a Material Layer

```cpp
void ConfigureGrassLayer()
{
    FTerrainMaterialLayerConfig GrassConfig;
    GrassConfig.LayerName = TEXT("Grass");
    GrassConfig.Slot = ETerrainMaterialSlot::Grass;

    // Set textures
    GrassConfig.TextureSet.BaseColor = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Textures/Grass_BaseColor"));
    GrassConfig.TextureSet.Normal = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Textures/Grass_Normal"));
    GrassConfig.TextureSet.Roughness = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Textures/Grass_Roughness"));
    GrassConfig.TextureSet.TilingScale = 5.0f;

    // Set blending ranges
    GrassConfig.SlopeRange = FVector2D(0.0f, 25.0f); // Flat to gentle slopes
    GrassConfig.AltitudeRange = FVector2D(0.0f, 1500.0f); // Low to mid altitude

    MaterialSystem->ConfigureLayer(ETerrainMaterialSlot::Grass, GrassConfig);
}
```

### Runtime Material Updates

```cpp
void APlanet::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Get player/camera location
    FVector ViewerLocation = GetPlayerViewLocation();

    // Update terrain materials
    UpdateTerrainMaterials(ViewerLocation);
}
```

### Applying Biome Materials

```cpp
void APlanet::ApplyBiomeMaterials()
{
    // Create material instance
    UMaterialInstanceDynamic* MaterialInstance = MaterialSystem->CreateTerrainMaterialInstance();
    PlanetMesh->SetMaterial(0, MaterialInstance);

    // Configure layers from biomes
    const TArray<FBiomeDefinition>& Biomes = BiomeManager->GetAllBiomes();
    for (int32 i = 0; i < Biomes.Num(); i++)
    {
        ConfigureLayerFromBiome(i, Biomes[i]);
    }

    // Update textures
    MaterialSystem->UpdateLayerTextures(MaterialInstance);
}
```

## Material Parameter Names

The system uses standardized parameter names in the master material:

### Layer Parameters (0-7)

- `Layer{N}_BaseColor` - Base color texture
- `Layer{N}_Normal` - Normal map texture
- `Layer{N}_Roughness` - Roughness texture
- `Layer{N}_Metallic` - Metallic texture
- `Layer{N}_AO` - Ambient occlusion texture
- `Layer{N}_Height` - Height/displacement texture
- `Layer{N}_TilingScale` - Texture tiling scale
- `Layer{N}_UseTriplanar` - Enable triplanar projection (0/1)
- `Layer{N}_TriplanarSharpness` - Triplanar blend sharpness
- `Layer{N}_SlopeRange` - Slope range (Vector2D: min, max)
- `Layer{N}_AltitudeRange` - Altitude range (Vector2D: min, max)
- `Layer{N}_MacroBlendDistance` - Distance for macro texture blending
- `Layer{N}_MacroBlend` - Current macro blend factor (0-1)

### Biome Blending Parameters

- `BiomeWeight{0-7}` - Weight of each biome (0-1)
- `BiomeIndex{0-7}` - Index of each biome

### Global Parameters

- `GlobalTilingScale` - Global texture scale multiplier
- `CurrentSlope` - Current slope angle (degrees)
- `CurrentAltitude` - Current altitude (meters)
- `CliffMaterialWeight` - Cliff material blend weight
- `SnowMaterialWeight` - Snow material blend weight

### Advanced Feature Parameters

- `EnableParallaxOcclusion` - Enable parallax (0/1)
- `ParallaxDepthScale` - Parallax depth amount
- `EnableTessellation` - Enable tessellation (0/1)
- `TessellationMultiplier` - Tessellation strength
- `TessellationFactor` - Current tessellation factor (0-1)
- `WetnessIntensity` - Surface wetness (0-1)

## Performance Considerations

### Texture Memory

- Use texture streaming for large texture sets
- Implement LOD for distant terrain
- Share textures between similar biomes
- Use texture atlases where possible

### Material Complexity

- Limit active layers to 4-6 for best performance
- Use static switches for optional features
- Implement distance-based feature culling
- Use material quality levels for scalability

### Update Frequency

- Update materials only when viewer moves significantly
- Cache material instances per terrain tile
- Batch parameter updates
- Use async texture loading

## Integration with Other Systems

### Biome System

The material system integrates with the biome system for automatic material selection:

```cpp
// Get biome parameters
FBlendedTerrainParameters Params = BiomeBlendingSystem->GetBlendedParameters(Location);

// Apply to materials
MaterialSystem->SetBiomeBlendWeights(MaterialInstance, Params.BiomeWeights);
```

### Weather System

Weather affects surface wetness and appearance:

```cpp
// Weather updates wetness
float Wetness = WeatherComponent->GetWetnessAmount();
MaterialSystem->UpdateWetness(MaterialInstance, Wetness);
```

### LOD System

Material detail adjusts with LOD level:

```cpp
// Update based on distance
float ViewDistance = CalculateViewDistance();
MaterialSystem->UpdateTessellation(MaterialInstance, ViewDistance);
```

## Future Enhancements

- **Footprint/Track Deformation** - Dynamic surface deformation
- **Seasonal Variations** - Material changes with seasons
- **Erosion Visualization** - Visual erosion patterns
- **Procedural Weathering** - Automatic weathering effects
- **Material Layers > 8** - Support for more complex blending
- **GPU-Driven Material Selection** - Compute shader-based blending

## Troubleshooting

### Materials Not Updating

- Verify master material is assigned
- Check material instance is created
- Ensure UpdateTerrainMaterials is called
- Verify texture references are valid

### Texture Stretching on Cliffs

- Enable triplanar projection for cliff layer
- Adjust triplanar sharpness parameter
- Check slope range configuration

### Performance Issues

- Reduce number of active layers
- Disable tessellation at distance
- Lower texture resolution
- Use simpler material for distant terrain

### Blending Artifacts

- Adjust biome transition zone width
- Increase transition smoothness
- Check biome weight normalization
- Verify slope/altitude ranges don't overlap excessively

## References

- Design Document: `.kiro/specs/planet-interior-enhancement/design.md`
- Requirements: `.kiro/specs/planet-interior-enhancement/requirements.md`
- Biome System: `Content/README_BiomeSystem.md`
- Planet System: `Content/README_PlanetSystem.md`
