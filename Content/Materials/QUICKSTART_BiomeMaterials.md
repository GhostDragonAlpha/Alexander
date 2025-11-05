# Quick Start: Biome Material Library

## Overview

This guide will help you quickly set up and use the Biome Material Library system for your planetary terrain.

## Prerequisites

- Master terrain material created (see `README_MaterialSystem.md`)
- Biome definitions configured (see `README_BiomeSystem.md`)
- Texture assets prepared for each biome

## Step 1: Create Material Library Asset

1. **Create Data Asset**

   - In Content Browser, navigate to `Content/Materials/`
   - Right-click → Miscellaneous → Data Asset
   - Select `BiomeMaterialLibraryAsset`
   - Name it `DA_BiomeMaterialLibrary`

2. **Set Master Material**
   - Open `DA_BiomeMaterialLibrary`
   - Set `Master Terrain Material` to your terrain master material
   - Save the asset

## Step 2: Configure Desert Biome (Example)

### Add Biome Definition

1. In `DA_BiomeMaterialLibrary`, add new entry to `Biome Material Definitions`
2. Set `Biome Type` to `Desert`
3. Set `Biome Name` to "Desert"

### Add Variation 1: Sandy Dunes

1. Add new variation to `Variations` array
2. Set `Variation Name` to "Sandy Dunes"

**Ground Material:**

- Base Color: `T_Sand_Dunes_BaseColor`
- Normal: `T_Sand_Dunes_Normal`
- Roughness: `T_Sand_Dunes_Roughness`
- Tiling Scale: `5.0`
- Macro Tiling Scale: `50.0`

**Cliff Material:**

- Base Color: `T_Desert_Rock_BaseColor`
- Normal: `T_Desert_Rock_Normal`
- Roughness: `T_Desert_Rock_Roughness`
- Tiling Scale: `2.0`

**Properties:**

- Blend Sharpness: `0.5`
- Cliff Slope Threshold: `45.0`
- Use Triplanar Projection: `true`

### Add Variation 2: Rocky Desert

Repeat the process with different textures:

- Ground: Rocky sand textures
- Cliff: Weathered rock textures

### Add 4 More Variations

Create variations for:

- Variation 3: Coarse Sand
- Variation 4: Desert Hardpan
- Variation 5: Salt Flats
- Variation 6: Mixed Desert

## Step 3: Configure Remaining Biomes

Repeat Step 2 for each biome type:

- **Grassland** (6 variations)
- **Forest** (6 variations)
- **Tundra** (6 variations)
- **Volcanic** (6 variations)
- **Ocean** (6 variations)
- **Savanna** (6 variations)
- **Rainforest** (6 variations)
- **Alpine** (6 variations)
- **Swamp** (6 variations)

## Step 4: Initialize in Code

### In Planet Actor

```cpp
// In APlanet::BeginPlay()
void APlanet::BeginPlay()
{
    Super::BeginPlay();

    // Create material library
    BiomeMaterialLibrary = NewObject<UBiomeMaterialLibrary>(this);

    // Load from asset
    UBiomeMaterialLibraryAsset* LibraryAsset = LoadObject<UBiomeMaterialLibraryAsset>(
        nullptr,
        TEXT("/Game/Materials/DA_BiomeMaterialLibrary")
    );

    if (LibraryAsset)
    {
        BiomeMaterialLibrary->LoadFromAsset(LibraryAsset);
        UE_LOG(LogTemp, Log, TEXT("Biome Material Library loaded successfully"));
    }
}
```

### Add to Planet Header

```cpp
// In APlanet.h
UPROPERTY()
UBiomeMaterialLibrary* BiomeMaterialLibrary;
```

## Step 5: Apply Materials to Terrain

### Get Material for Biome

```cpp
// Get material instance for a specific biome
UMaterialInstanceDynamic* DesertMaterial = BiomeMaterialLibrary->GetBiomeMaterialInstance(
    EBiomeType::Desert,
    0  // Use first variation
);

// Apply to terrain mesh
TerrainMesh->SetMaterial(0, DesertMaterial);
```

### Use Random Variation

```cpp
// Get random variation for visual diversity
int32 VariationIndex = FMath::RandRange(0, BiomeMaterialLibrary->GetVariationCount(EBiomeType::Desert) - 1);

UMaterialInstanceDynamic* Material = BiomeMaterialLibrary->GetBiomeMaterialInstance(
    EBiomeType::Desert,
    VariationIndex
);
```

### Blend Multiple Biomes

```cpp
// Create blended material from biome weights
TMap<EBiomeType, float> BiomeWeights;
BiomeWeights.Add(EBiomeType::Grassland, 0.6f);
BiomeWeights.Add(EBiomeType::Forest, 0.3f);
BiomeWeights.Add(EBiomeType::Tundra, 0.1f);

UMaterialInstanceDynamic* BlendedMaterial = BiomeMaterialLibrary->CreateBlendedMaterial(BiomeWeights);
TerrainMesh->SetMaterial(0, BlendedMaterial);
```

## Step 6: Integrate with Biome Manager

```cpp
void APlanet::UpdateTerrainMaterialAtLocation(FVector WorldPosition)
{
    // Get biome weights from biome manager
    FBiomeWeights Weights = BiomeManager->GetBiomeWeightsAtLocation(WorldPosition);

    // Convert to material weights
    TMap<EBiomeType, float> MaterialWeights;
    for (const TPair<int32, float>& Pair : Weights.Weights)
    {
        EBiomeType BiomeType = GetBiomeTypeFromIndex(Pair.Key);
        MaterialWeights.Add(BiomeType, Pair.Value);
    }

    // Create and apply blended material
    UMaterialInstanceDynamic* TerrainMaterial = BiomeMaterialLibrary->CreateBlendedMaterial(MaterialWeights);

    if (TerrainMaterial)
    {
        TerrainMesh->SetMaterial(0, TerrainMaterial);
    }
}
```

## Step 7: Test in Editor

### Visual Testing

1. Place planet actor in level
2. Play in editor
3. Verify materials appear correctly
4. Check biome transitions
5. Test different variations

### Console Commands

```
// Test specific biome material
py exec(open('test_biome_material.py').read())
```

## Common Issues and Solutions

### Issue: Materials Not Appearing

**Solution:**

- Verify `DA_BiomeMaterialLibrary` is loaded
- Check texture references are valid
- Ensure master material is assigned
- Verify biome type matches definition

### Issue: Textures Look Stretched

**Solution:**

- Enable triplanar projection for cliff materials
- Adjust tiling scales in variation settings
- Check UV generation in terrain system

### Issue: Performance Problems

**Solution:**

- Reduce texture resolutions
- Enable texture streaming
- Limit number of active variations
- Use LOD for distant terrain

### Issue: Blending Looks Wrong

**Solution:**

- Adjust blend sharpness parameter
- Normalize biome weights (should sum to 1.0)
- Check transition zone width in biome manager
- Verify material parameter ranges

## Texture Naming Convention

Use consistent naming for easy organization:

```
T_{BiomeName}_{MaterialType}_{TextureType}

Examples:
- T_Desert_Sand_BaseColor
- T_Desert_Sand_Normal
- T_Desert_Rock_BaseColor
- T_Grassland_Grass_BaseColor
- T_Forest_Ground_Normal
```

## Recommended Texture Sources

### Free Resources

- **Quixel Megascans** - High-quality PBR textures
- **Polyhaven** - Free CC0 textures
- **Texture Haven** - Seamless PBR textures
- **CC0 Textures** - Public domain textures

### Commercial Resources

- **Substance Source** - Procedural materials
- **Poliigon** - High-quality textures
- **Texture.com** - Large texture library

## Next Steps

1. **Create Parameter Collections** - For global biome parameters
2. **Add Weather Integration** - Dynamic wetness and effects
3. **Implement LOD System** - Distance-based material simplification
4. **Add Seasonal Variations** - Material changes with seasons
5. **Create Material Presets** - Pre-configured biome sets

## Additional Resources

- Full Documentation: `README_BiomeMaterialLibrary.md`
- Material System: `README_MaterialSystem.md`
- Biome System: `README_BiomeSystem.md`
- Advanced Features: `README_AdvancedMaterialFeatures.md`

## Example: Complete Desert Setup

```cpp
// Create desert biome material definition
FBiomeMaterialDefinition DesertMaterial;
DesertMaterial.BiomeType = EBiomeType::Desert;
DesertMaterial.BiomeName = TEXT("Desert");

// Variation 1: Sandy Dunes
FBiomeMaterialVariation SandyDunes;
SandyDunes.VariationName = TEXT("Sandy Dunes");
SandyDunes.GroundMaterial.BaseColor = LoadTexture(TEXT("/Game/Textures/T_Desert_Sand_BaseColor"));
SandyDunes.GroundMaterial.Normal = LoadTexture(TEXT("/Game/Textures/T_Desert_Sand_Normal"));
SandyDunes.GroundMaterial.Roughness = LoadTexture(TEXT("/Game/Textures/T_Desert_Sand_Roughness"));
SandyDunes.GroundMaterial.TilingScale = 5.0f;
SandyDunes.CliffMaterial.BaseColor = LoadTexture(TEXT("/Game/Textures/T_Desert_Rock_BaseColor"));
SandyDunes.CliffMaterial.Normal = LoadTexture(TEXT("/Game/Textures/T_Desert_Rock_Normal"));
SandyDunes.CliffMaterial.TilingScale = 2.0f;
SandyDunes.bUseTriplanarProjection = true;
SandyDunes.CliffSlopeThreshold = 45.0f;

DesertMaterial.Variations.Add(SandyDunes);

// Add 5 more variations...
// (Rocky Desert, Coarse Sand, Hardpan, Salt Flats, Mixed)

// Add to library
BiomeMaterials.Add(EBiomeType::Desert, DesertMaterial);
```

## Performance Targets

- **Texture Memory**: < 500MB per biome
- **Material Complexity**: < 200 instructions
- **Variation Switching**: < 1ms
- **Blended Material Creation**: < 5ms
- **Cache Hit Rate**: > 90%

## Validation Checklist

- [ ] All 10 biome types configured
- [ ] Each biome has 6+ variations
- [ ] All textures are valid and loaded
- [ ] Master material is assigned
- [ ] Tiling scales are appropriate
- [ ] Cliff materials use triplanar projection
- [ ] Blend sharpness is configured
- [ ] Materials appear correctly in-game
- [ ] Performance is acceptable
- [ ] Biome transitions are smooth

## Support

For issues or questions:

- Check documentation in `Content/Materials/`
- Review design document: `.kiro/specs/planet-interior-enhancement/design.md`
- Test with example materials in `Content/Materials/Examples/`
