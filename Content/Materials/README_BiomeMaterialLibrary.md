# Biome Material Library

## Overview

The Biome Material Library provides a comprehensive system for managing material assets across all planetary biomes. It includes texture sets, material variations, parameter collections, and dynamic material instances for creating visually diverse and realistic terrain surfaces.

## Architecture

### Core Components

1. **UBiomeMaterialLibrary** - Main material management class
2. **FBiomeMaterialDefinition** - Complete material definition for a biome
3. **FBiomeMaterialVariation** - Individual material variation (6+ per biome)
4. **FPBRTextureSet** - Complete PBR texture set (BaseColor, Normal, Roughness, etc.)
5. **UBiomeMaterialLibraryAsset** - Data asset for storing material library

## Biome Types

The system supports 10 distinct biome types:

1. **Desert** - Sandy dunes, rocky outcrops, sparse vegetation
2. **Grassland** - Rolling plains, grass coverage, scattered rocks
3. **Forest** - Dense vegetation, forest floor, tree coverage
4. **Tundra** - Snow, ice, frozen ground, minimal vegetation
5. **Volcanic** - Lava rock, ash, volcanic terrain
6. **Ocean** - Water surfaces, underwater terrain, coastal areas
7. **Savanna** - Dry grassland, scattered trees, red soil
8. **Rainforest** - Dense jungle, wet ground, thick vegetation
9. **Alpine** - Mountain terrain, snow caps, rocky peaks
10. **Swamp** - Wetlands, muddy ground, water pools

## Material Variations

Each biome includes 6+ material variations for visual diversity:

### Variation Components

Each variation includes 4 material layers:

1. **Ground Material** - Primary surface material
2. **Secondary Material** - Blended secondary surface
3. **Cliff Material** - Steep slope/vertical surfaces (with triplanar projection)
4. **Detail Material** - Close-up micro-detail

### PBR Texture Sets

Each material layer includes complete PBR textures:

- **Base Color (Albedo)** - Surface color and patterns
- **Normal Map** - Surface detail and lighting information
- **Roughness** - Surface roughness (0 = smooth, 1 = rough)
- **Metallic** - Metallic properties (0 = non-metal, 1 = metal)
- **Ambient Occlusion** - Cavity shading and detail
- **Height/Displacement** - Surface depth for parallax mapping

### Tiling Scales

- **Micro Tiling** - High-frequency detail (1-10m scale)
- **Macro Tiling** - Low-frequency patterns (10-100m scale)

## Usage Examples

### Basic Setup

```cpp
// Create material library
UBiomeMaterialLibrary* MaterialLibrary = NewObject<UBiomeMaterialLibrary>(this);

// Initialize with biome definitions
TArray<FBiomeDefinition> Biomes = GetBiomeDefinitions();
MaterialLibrary->Initialize(Biomes);
```

### Loading from Data Asset

```cpp
// Load pre-configured material library
UBiomeMaterialLibraryAsset* LibraryAsset = LoadObject<UBiomeMaterialLibraryAsset>(
    nullptr,
    TEXT("/Game/Materials/BiomeMaterialLibrary")
);

MaterialLibrary->LoadFromAsset(LibraryAsset);
```

### Getting Material Instances

```cpp
// Get material instance for a specific biome and variation
UMaterialInstanceDynamic* DesertMaterial = MaterialLibrary->GetBiomeMaterialInstance(
    EBiomeType::Desert,
    0  // Variation index
);

// Apply to mesh
TerrainMesh->SetMaterial(0, DesertMaterial);
```

### Random Variation

```cpp
// Get random variation for visual diversity
const FBiomeMaterialVariation& RandomVariation = MaterialLibrary->GetRandomVariation(
    EBiomeType::Forest
);
```

### Blended Materials

```cpp
// Create blended material from multiple biomes
TMap<EBiomeType, float> BiomeWeights;
BiomeWeights.Add(EBiomeType::Grassland, 0.6f);
BiomeWeights.Add(EBiomeType::Forest, 0.4f);

UMaterialInstanceDynamic* BlendedMaterial = MaterialLibrary->CreateBlendedMaterial(BiomeWeights);
```

### Applying Texture Sets

```cpp
// Manually apply texture set to material
FPBRTextureSet TextureSet;
TextureSet.BaseColor = LoadTexture(TEXT("/Game/Textures/Grass_BaseColor"));
TextureSet.Normal = LoadTexture(TEXT("/Game/Textures/Grass_Normal"));
TextureSet.Roughness = LoadTexture(TEXT("/Game/Textures/Grass_Roughness"));
TextureSet.TilingScale = 5.0f;

MaterialLibrary->ApplyTextureSet(MaterialInstance, TextureSet, TEXT("Layer0_"));
```

## Material Parameter Collections

Each biome can have an associated Material Parameter Collection for global parameters:

```cpp
// Get parameter collection for a biome
UMaterialParameterCollection* Collection = MaterialLibrary->GetParameterCollection(
    EBiomeType::Desert
);

// Update parameter
MaterialLibrary->UpdateBiomeParameter(
    EBiomeType::Desert,
    TEXT("SandRoughness"),
    0.8f
);
```

## Creating Material Library Assets

### Step 1: Create Data Asset

1. In Content Browser, right-click → Miscellaneous → Data Asset
2. Select `BiomeMaterialLibraryAsset` as the class
3. Name it `DA_BiomeMaterialLibrary`

### Step 2: Configure Biome Materials

For each biome type:

1. Add a new `FBiomeMaterialDefinition` entry
2. Set the `BiomeType` (Desert, Grassland, etc.)
3. Set the `BiomeName`
4. Add 6+ variations

### Step 3: Configure Variations

For each variation:

1. Set `VariationName` (e.g., "Sandy Dunes", "Rocky Desert")
2. Configure **Ground Material** textures
3. Configure **Secondary Material** textures
4. Configure **Cliff Material** textures
5. Configure **Detail Material** textures
6. Set tiling scales and blend parameters

### Step 4: Set Master Material

Assign the master terrain material that supports all required parameters.

## Material Parameter Names

The system uses standardized parameter names:

### Layer Parameters (0-7)

- `Layer{N}_BaseColor` - Base color texture
- `Layer{N}_Normal` - Normal map texture
- `Layer{N}_Roughness` - Roughness texture
- `Layer{N}_Metallic` - Metallic texture
- `Layer{N}_AO` - Ambient occlusion texture
- `Layer{N}_Height` - Height/displacement texture
- `Layer{N}_TilingScale` - Micro tiling scale
- `Layer{N}_MacroTilingScale` - Macro tiling scale
- `Layer{N}_UseTriplanar` - Enable triplanar projection (0/1)

### Biome Blending Parameters

- `BiomeWeight{0-7}` - Weight of each biome layer (0-1)
- `BlendSharpness` - Sharpness of biome transitions
- `CliffSlopeThreshold` - Slope angle for cliff material (degrees)

### Detail Parameters

- `Detail_BaseColor` - Detail texture base color
- `Detail_Normal` - Detail texture normal map
- `Detail_TilingScale` - Detail texture tiling

## Integration with Terrain System

### Biome Manager Integration

```cpp
// Get biome weights at location
FBiomeWeights Weights = BiomeManager->GetBiomeWeightsAtLocation(WorldPosition);

// Convert to material weights
TMap<EBiomeType, float> MaterialWeights;
for (const TPair<int32, float>& Pair : Weights.Weights)
{
    EBiomeType BiomeType = GetBiomeTypeFromIndex(Pair.Key);
    MaterialWeights.Add(BiomeType, Pair.Value);
}

// Create blended material
UMaterialInstanceDynamic* TerrainMaterial = MaterialLibrary->CreateBlendedMaterial(MaterialWeights);
```

### Terrain Material System Integration

```cpp
// Apply biome materials to terrain system
UTerrainMaterialSystem* TerrainSystem = GetTerrainMaterialSystem();

for (int32 i = 0; i < BiomeTypes.Num(); ++i)
{
    EBiomeType BiomeType = BiomeTypes[i];
    const FBiomeMaterialDefinition& BiomeMaterial = MaterialLibrary->GetBiomeMaterial(BiomeType);

    // Configure terrain layer from biome material
    FTerrainMaterialLayerConfig LayerConfig;
    LayerConfig.LayerName = BiomeMaterial.BiomeName;

    if (BiomeMaterial.Variations.Num() > 0)
    {
        const FBiomeMaterialVariation& Variation = BiomeMaterial.Variations[0];
        LayerConfig.TextureSet.BaseColor = Variation.GroundMaterial.BaseColor;
        LayerConfig.TextureSet.Normal = Variation.GroundMaterial.Normal;
        LayerConfig.TextureSet.Roughness = Variation.GroundMaterial.Roughness;
        LayerConfig.TextureSet.TilingScale = Variation.GroundMaterial.TilingScale;
    }

    TerrainSystem->ConfigureLayer(i, LayerConfig);
}
```

## Recommended Texture Resolutions

### Ground Materials (Primary)

- Base Color: 2048x2048 or 4096x4096
- Normal: 2048x2048 or 4096x4096
- Roughness: 1024x1024 or 2048x2048
- Metallic: 1024x1024 or 2048x2048
- AO: 1024x1024 or 2048x2048
- Height: 1024x1024 or 2048x2048

### Detail Materials (Close-up)

- Base Color: 1024x1024 or 2048x2048
- Normal: 1024x1024 or 2048x2048
- Roughness: 512x512 or 1024x1024

### Cliff Materials

- Base Color: 2048x2048 or 4096x4096
- Normal: 2048x2048 or 4096x4096
- Roughness: 1024x1024 or 2048x2048

## Performance Considerations

### Texture Streaming

- Enable texture streaming for all material textures
- Use texture groups for proper LOD management
- Set appropriate mip map settings

### Material Complexity

- Limit active texture samples per material
- Use static switches for optional features
- Implement distance-based feature culling

### Caching

- Material instances are cached automatically
- Cache key: `{BiomeType}_{VariationIndex}`
- Clear cache when switching planets

### Memory Management

- Unload unused biome materials
- Use texture atlases where possible
- Implement LOD for distant terrain

## Biome-Specific Material Guidelines

### Desert

- **Colors**: Tan, beige, orange, red
- **Textures**: Sand dunes, rocky outcrops, cracked earth
- **Roughness**: Medium to high (0.6-0.9)
- **Variations**: Fine sand, coarse sand, rocky desert, dunes, hardpan, salt flats

### Grassland

- **Colors**: Green, yellow-green, brown
- **Textures**: Grass blades, dirt, small rocks
- **Roughness**: Medium (0.5-0.7)
- **Variations**: Short grass, tall grass, dry grass, meadow, prairie, steppe

### Forest

- **Colors**: Dark green, brown, earth tones
- **Textures**: Forest floor, leaves, moss, roots
- **Roughness**: Medium to high (0.6-0.8)
- **Variations**: Pine forest, deciduous, jungle floor, moss-covered, leaf litter, undergrowth

### Tundra

- **Colors**: White, light blue, gray
- **Textures**: Snow, ice, frozen ground, permafrost
- **Roughness**: Low to medium (0.3-0.6)
- **Variations**: Fresh snow, packed snow, ice, frozen tundra, glacial, frost

### Volcanic

- **Colors**: Black, dark gray, red, orange
- **Textures**: Lava rock, basalt, ash, obsidian
- **Roughness**: High (0.7-0.95)
- **Variations**: Cooled lava, volcanic ash, basalt columns, pumice, obsidian, active lava

### Ocean

- **Colors**: Blue, turquoise, dark blue
- **Textures**: Water surface, underwater sand, coral
- **Roughness**: Very low (0.1-0.3)
- **Variations**: Shallow water, deep water, coastal, reef, sandy bottom, rocky bottom

### Savanna

- **Colors**: Yellow, tan, red-brown
- **Textures**: Dry grass, red soil, scattered rocks
- **Roughness**: Medium to high (0.6-0.8)
- **Variations**: Dry savanna, wet savanna, red soil, grassland, scrubland, acacia

### Rainforest

- **Colors**: Dark green, brown, vibrant greens
- **Textures**: Wet ground, moss, dense vegetation
- **Roughness**: Medium (0.5-0.7)
- **Variations**: Jungle floor, moss-covered, wet leaves, muddy, dense undergrowth, canopy floor

### Alpine

- **Colors**: Gray, white, brown
- **Textures**: Rocky peaks, snow, scree, talus
- **Roughness**: High (0.7-0.9)
- **Variations**: Rocky peaks, snow-capped, scree slopes, alpine meadow, glacial, cliff faces

### Swamp

- **Colors**: Dark green, brown, murky
- **Textures**: Mud, water, vegetation, moss
- **Roughness**: Medium (0.5-0.7)
- **Variations**: Muddy swamp, water pools, moss-covered, mangrove, bog, marsh

## Troubleshooting

### Materials Not Appearing

- Verify master terrain material is assigned
- Check that texture references are valid
- Ensure material instances are created
- Verify biome type matches definition

### Texture Stretching

- Enable triplanar projection for cliff materials
- Adjust tiling scales
- Check UV generation in terrain system

### Performance Issues

- Reduce texture resolutions
- Enable texture streaming
- Limit number of active variations
- Use simpler materials for distant terrain

### Blending Artifacts

- Adjust blend sharpness parameter
- Normalize biome weights
- Check transition zone width
- Verify material parameter ranges

## Future Enhancements

- **Seasonal Variations** - Material changes with seasons
- **Dynamic Weathering** - Procedural weathering effects
- **Footprint Deformation** - Dynamic surface deformation
- **Erosion Visualization** - Visual erosion patterns
- **Procedural Variation** - Runtime texture generation
- **Material Layers > 8** - Support for more complex blending

## References

- Design Document: `.kiro/specs/planet-interior-enhancement/design.md`
- Requirements: `.kiro/specs/planet-interior-enhancement/requirements.md`
- Material System: `Content/README_MaterialSystem.md`
- Biome System: `Content/README_BiomeSystem.md`
- Terrain System: `Content/README_TerrainGeneration.md`

## API Reference

### UBiomeMaterialLibrary

```cpp
// Initialize with biome definitions
void Initialize(const TArray<FBiomeDefinition>& InBiomes);

// Load from data asset
void LoadFromAsset(UBiomeMaterialLibraryAsset* LibraryAsset);

// Get material definition
const FBiomeMaterialDefinition& GetBiomeMaterial(EBiomeType BiomeType) const;

// Get material instance
UMaterialInstanceDynamic* GetBiomeMaterialInstance(EBiomeType BiomeType, int32 VariationIndex = 0);

// Get random variation
const FBiomeMaterialVariation& GetRandomVariation(EBiomeType BiomeType) const;

// Create blended material
UMaterialInstanceDynamic* CreateBlendedMaterial(const TMap<EBiomeType, float>& BiomeWeights);

// Apply texture set
void ApplyTextureSet(UMaterialInstanceDynamic* MaterialInstance, const FPBRTextureSet& TextureSet, const FString& LayerPrefix);

// Update parameter
void UpdateBiomeParameter(EBiomeType BiomeType, FName ParameterName, float Value);

// Get parameter collection
UMaterialParameterCollection* GetParameterCollection(EBiomeType BiomeType) const;

// Get variation count
int32 GetVariationCount(EBiomeType BiomeType) const;

// Check validity
bool HasValidMaterials(EBiomeType BiomeType) const;
```

### FBiomeMaterialDefinition

```cpp
// Get random variation
const FBiomeMaterialVariation& GetRandomVariation() const;

// Get variation by index
const FBiomeMaterialVariation& GetVariation(int32 Index) const;
```

### FPBRTextureSet

```cpp
// Check if texture set is valid
bool IsValid() const;
```
