# Quick Start: Advanced Material Features

## 5-Minute Setup Guide

### Step 1: Initialize Material System (C++)

```cpp
// In your Planet class BeginPlay
void APlanet::BeginPlay()
{
    Super::BeginPlay();

    // Create material system
    MaterialSystem = NewObject<UTerrainMaterialSystem>(this);

    // Load master material
    UMaterialInterface* MasterMat = LoadObject<UMaterialInterface>(
        nullptr,
        TEXT("/Game/Materials/M_TerrainMaster")
    );

    // Initialize
    MaterialSystem->Initialize(MasterMat);

    // Configure advanced features
    FAdvancedMaterialFeatures Features =
        UAdvancedMaterialFeaturesHelper::CreateDefaultAdvancedFeatures();
    MaterialSystem->ConfigureAdvancedFeatures(Features);

    // Create material instance
    TerrainMaterial = MaterialSystem->CreateTerrainMaterialInstance();
    TerrainMesh->SetMaterial(0, TerrainMaterial);
}
```

### Step 2: Update Every Frame (C++)

```cpp
void APlanet::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Get viewer location
    FVector ViewerLocation = GetPlayerCameraLocation();
    float ViewDistance = FVector::Dist(ViewerLocation, GetActorLocation());

    // Update all advanced features
    MaterialSystem->UpdateAdvancedFeatures(TerrainMaterial, ViewDistance);

    // Update wetness from weather
    if (WeatherComponent)
    {
        float Wetness = WeatherComponent->GetWetnessAmount();
        MaterialSystem->UpdateWetness(TerrainMaterial, Wetness);
    }
}
```

### Step 3: Blueprint Setup (Alternative)

```
1. Add TerrainMaterialSystem component to Planet actor
2. In Event BeginPlay:
   - Call "Initialize" with Master Material
   - Call "Create Default Advanced Features"
   - Call "Configure Advanced Features"
   - Call "Create Terrain Material Instance"
   - Set material on mesh

3. In Event Tick:
   - Get player camera location
   - Calculate distance to planet
   - Call "Update Advanced Features"
   - Call "Update Wetness" (if weather active)
```

## Feature-Specific Quick Starts

### Parallax Occlusion Mapping

```cpp
// Enable/disable
Features.bEnableParallaxOcclusion = true;
Features.ParallaxDepthScale = 0.05f; // Adjust depth (0.03-0.08)

// Update based on distance
MaterialSystem->UpdateParallaxOcclusion(TerrainMaterial, ViewDistance);
```

**Visual Check**: Look at terrain up close (< 10m). You should see depth in the surface texture.

### Tessellation

```cpp
// Enable/disable
Features.bEnableTessellation = true;
Features.TessellationMultiplier = 2.0f; // Adjust density (1.0-4.0)
Features.MaxTessellationDistance = 50.0f; // Adjust range (20-100m)

// Update based on distance
MaterialSystem->UpdateTessellation(TerrainMaterial, ViewDistance);
```

**Visual Check**: Look at terrain edges from the side. You should see geometric detail, not just flat polygons.

### Dynamic Wetness

```cpp
// Enable/disable
Features.bEnableDynamicWetness = true;

// Set wetness amount (0-1)
float WetnessAmount = 0.8f; // 0=dry, 1=soaking wet
MaterialSystem->UpdateWetness(TerrainMaterial, WetnessAmount);
```

**Visual Check**: Surfaces should darken and become shinier when wet.

### Macro/Micro Blending

```cpp
// Configure per layer
FTerrainMaterialLayerConfig LayerConfig;
LayerConfig.MacroBlendDistance = 50.0f; // Start blending at 50m
LayerConfig.TextureSet.TilingScale = 5.0f; // Micro tiling
LayerConfig.MacroTextureSet.TilingScale = 0.5f; // Macro tiling

// Update based on distance
MaterialSystem->UpdateMacroMicroBlending(TerrainMaterial, ViewDistance);
```

**Visual Check**: Texture detail should remain visible at all distances without obvious repetition.

## Common Configurations

### VR Optimized

```cpp
FAdvancedMaterialFeatures Features =
    UAdvancedMaterialFeaturesHelper::CreateVROptimizedFeatures();
// Parallax: Enabled, reduced depth
// Tessellation: 1.5x, 30m range
// Wetness: Enabled
```

### High Quality Desktop

```cpp
FAdvancedMaterialFeatures Features =
    UAdvancedMaterialFeaturesHelper::CreateHighQualityFeatures();
// Parallax: Enabled, full depth
// Tessellation: 3.0x, 75m range
// Wetness: Enabled
// Deformation: Enabled
```

### Quality Presets

```cpp
// Apply preset (0=Low, 1=Medium, 2=High, 3=Epic)
UAdvancedMaterialFeaturesHelper::ApplyQualityPreset(Features, 2);
```

## Performance Monitoring

```cpp
// Check if VR frame rate is acceptable
float CurrentFPS = GetCurrentFrameRate();
bool IsAcceptable = UAdvancedMaterialFeaturesHelper::IsVRFrameRateAcceptable(CurrentFPS);

if (!IsAcceptable)
{
    // Adjust features for performance
    float AdjustmentFactor = UAdvancedMaterialFeaturesHelper::GetPerformanceAdjustmentFactor(
        CurrentFPS,
        90.0f // Target FPS
    );

    UAdvancedMaterialFeaturesHelper::AdjustFeaturesForPerformance(
        Features,
        AdjustmentFactor
    );

    // Reapply adjusted features
    MaterialSystem->ConfigureAdvancedFeatures(Features);
}
```

## Debugging

### Log Current Configuration

```cpp
UAdvancedMaterialFeaturesHelper::LogAdvancedFeatures(
    Features,
    TEXT("Current Material Features")
);
```

### Visual Debug

```cpp
// In editor, enable:
// - Show → Visualize → Tessellation
// - Show → Visualize → Material Complexity
// - Show → Visualize → Shader Complexity
```

### Console Commands

```
// Toggle features
r.Tessellation 0/1
r.ParallaxOcclusion 0/1

// View stats
stat GPU
stat SceneRendering
stat RHI
```

## Troubleshooting

### "Features not visible"

- Check material instance is applied to mesh
- Verify master material has feature nodes
- Check view distance (features may be disabled at distance)

### "Performance issues"

- Reduce tessellation distance
- Lower tessellation multiplier
- Disable parallax beyond 25m
- Use VR-optimized preset

### "Compilation errors"

- Ensure all header files are included
- Rebuild project
- Check Unreal Engine version (5.6+)

## Next Steps

1. **Create Master Material**: Build material in Unreal Editor with all features
2. **Configure Layers**: Set up 8 material layers with textures
3. **Test Performance**: Profile in VR and adjust settings
4. **Integrate Weather**: Connect wetness to weather system
5. **Optimize**: Use quality presets and distance-based toggling

## File References

- **C++ Headers**:
  - `Source/Alexander/Public/TerrainMaterialSystem.h`
  - `Source/Alexander/Public/AdvancedMaterialFeaturesHelper.h`
- **C++ Implementation**:
  - `Source/Alexander/Private/TerrainMaterialSystem.cpp`
  - `Source/Alexander/Private/AdvancedMaterialFeaturesHelper.cpp`
- **Material Functions**:
  - `Content/Materials/MF_ParallaxOcclusionMapping.txt`
  - `Content/Materials/MF_Tessellation.txt`
  - `Content/Materials/MF_DynamicWetness.txt`
  - `Content/Materials/MF_MacroMicroBlending.txt`
- **Documentation**:
  - `Content/README_AdvancedMaterialFeatures.md`
  - `Content/README_MaterialSystem.md`

## Support

For issues or questions:

1. Check `Content/README_AdvancedMaterialFeatures.md` for detailed documentation
2. Review material function specifications in `Content/Materials/`
3. Check requirements in `.kiro/specs/planet-interior-enhancement/requirements.md`
