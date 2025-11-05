# Advanced Material Features Implementation

## Overview

This document describes the implementation of advanced material features for the terrain system, including:

1. **Parallax Occlusion Mapping** - Adds depth to surfaces without geometry
2. **Dynamic Tessellation** - Adds geometric detail based on distance
3. **Dynamic Wetness System** - Weather-driven surface wetness
4. **Macro/Micro Detail Blending** - Distance-based texture detail

These features work together to create Star Citizen-quality terrain materials that maintain visual fidelity at all distances while optimizing performance.

## Architecture

### System Components

```
TerrainMaterialSystem
├── Advanced Features Configuration
│   ├── Parallax Occlusion Settings
│   ├── Tessellation Settings
│   ├── Wetness Settings
│   └── Macro/Micro Blending Settings
├── Material Functions
│   ├── MF_ParallaxOcclusionMapping
│   ├── MF_Tessellation
│   ├── MF_DynamicWetness
│   └── MF_MacroMicroBlending
└── Runtime Updates
    ├── Distance-based feature toggling
    ├── Weather integration
    └── Performance optimization
```

## Feature Details

### 1. Parallax Occlusion Mapping (POM)

**Purpose**: Adds convincing depth to flat surfaces using height maps.

**Implementation**:

- Ray marching through height field
- 8-32 steps based on distance and quality
- Binary search refinement for accuracy
- Automatic disable beyond 50m

**Parameters**:

- `ParallaxDepthScale`: 0.03-0.05 (depth amount)
- `ParallaxSteps`: 8-32 (quality vs performance)
- `EnableParallaxOcclusion`: 0/1 (on/off)

**Usage**:

```cpp
// Update based on view distance
MaterialSystem->UpdateParallaxOcclusion(MaterialInstance, ViewDistance);
```

**Visual Impact**:

- Self-shadowing on surfaces
- Correct silhouette edges
- Depth perception on flat terrain
- Works best with high-contrast height maps

### 2. Dynamic Tessellation

**Purpose**: Adds geometric detail to terrain without high-poly base meshes.

**Implementation**:

- Distance-based tessellation factor (exponential falloff)
- PN Triangles tessellation mode
- Height map-driven displacement
- Adaptive tessellation based on curvature

**Parameters**:

- `TessellationMultiplier`: 1.0-4.0 (density)
- `MaxTessellationDistance`: 20-100m (range)
- `DisplacementScale`: 0.1-10.0 (height)
- `EnableTessellation`: 0/1 (on/off)

**Usage**:

```cpp
// Update based on view distance
MaterialSystem->UpdateTessellation(MaterialInstance, ViewDistance);
```

**Visual Impact**:

- Realistic surface relief
- Smooth terrain silhouettes
- Shadows cast by displaced geometry
- Eliminates need for high-poly meshes

**Performance**:

- Base mesh: ~1000 triangles
- With tessellation (2.0x): ~4000 triangles
- With tessellation (4.0x): ~16000 triangles

### 3. Dynamic Wetness System

**Purpose**: Realistic surface wetness from weather conditions.

**Implementation**:

- Rain accumulation based on surface angle
- Darkening of wet surfaces
- Roughness reduction (increased specularity)
- Puddle formation in low areas
- Animated rain ripples

**Parameters**:

- `WetnessIntensity`: 0.0-1.0 (amount)
- `EnableDynamicWetness`: 0/1 (on/off)

**Usage**:

```cpp
// Update from weather system
float Wetness = WeatherComponent->GetWetnessAmount();
MaterialSystem->UpdateWetness(MaterialInstance, Wetness);
```

**Visual Impact**:

- Surfaces darken when wet (realistic light absorption)
- Increased specularity (water reflection)
- Puddle formation in depressions
- Animated ripples during rain
- Non-uniform drying patterns

**Weather Integration**:

```cpp
// Weather types affect wetness
switch(CurrentWeather)
{
    case EWeatherType::LightRain:
        TargetWetness = 0.4f;
        break;
    case EWeatherType::HeavyRain:
        TargetWetness = 0.9f;
        break;
    case EWeatherType::Thunderstorm:
        TargetWetness = 1.0f;
        break;
    default:
        TargetWetness = 0.0f;
        break;
}
```

### 4. Macro/Micro Detail Blending

**Purpose**: Maintains visual quality at all distances by blending detail levels.

**Implementation**:

- High-frequency micro textures for close viewing
- Low-frequency macro textures for distant viewing
- Smooth distance-based blending
- Per-layer blend distances
- Height-corrected blending for displacement

**Parameters**:

- `MicroTiling`: 5.0-10.0 (high frequency)
- `MacroTiling`: 0.1-0.5 (low frequency)
- `MacroBlendDistance`: 50-100m (blend start)
- `MacroBlendRange`: 25-50m (transition range)

**Usage**:

```cpp
// Update based on view distance
MaterialSystem->UpdateMacroMicroBlending(MaterialInstance, ViewDistance);
```

**Visual Impact**:

- Maintains detail at all distances
- Eliminates texture repetition at distance
- Smooth transitions prevent popping
- Optimizes performance (fewer high-res samples far away)

**Tiling Guidelines**:

| Terrain Type | Micro Tiling | Macro Tiling | Blend Start |
| ------------ | ------------ | ------------ | ----------- |
| Rocky        | 8.0          | 0.3          | 30m         |
| Grass        | 10.0         | 0.5          | 20m         |
| Sand         | 5.0          | 0.2          | 50m         |
| Snow         | 6.0          | 0.25         | 40m         |

## Integration

### Complete Update Flow

```cpp
void APlanet::UpdateTerrainMaterials(FVector ViewerLocation)
{
    // Calculate view distance
    float ViewDistance = FVector::Dist(ViewerLocation, GetActorLocation());

    // Get material instance
    UMaterialInstanceDynamic* MatInst = GetTerrainMaterialInstance();

    // Update all advanced features
    MaterialSystem->UpdateAdvancedFeatures(MatInst, ViewDistance);

    // Update wetness from weather
    if (WeatherComponent)
    {
        float Wetness = WeatherComponent->GetWetnessAmount();
        MaterialSystem->UpdateWetness(MatInst, Wetness);
    }

    // Update environmental blending
    FVector2D Coordinates = WorldToTerrainCoordinates(ViewerLocation);
    float SlopeAngle = GetSlopeAtLocation(Coordinates);
    float Altitude = GetAltitudeAtLocation(Coordinates);
    TMap<int32, float> BiomeWeights = GetBiomeWeightsAtLocation(Coordinates);
    float Moisture = GetMoistureAtLocation(Coordinates);
    float Temperature = GetTemperatureAtLocation(Coordinates);

    MaterialSystem->CalculateAndApplyBlendWeights(
        MatInst,
        SlopeAngle,
        Altitude,
        BiomeWeights,
        Moisture,
        Temperature
    );
}
```

### Initialization

```cpp
void APlanet::InitializeAdvancedMaterialFeatures()
{
    // Configure advanced features
    FAdvancedMaterialFeatures Features;

    // Parallax Occlusion
    Features.bEnableParallaxOcclusion = true;
    Features.ParallaxDepthScale = 0.05f;

    // Tessellation
    Features.bEnableTessellation = true;
    Features.TessellationMultiplier = 2.0f;
    Features.MaxTessellationDistance = 50.0f;

    // Wetness
    Features.bEnableDynamicWetness = true;
    Features.WetnessIntensity = 0.0f; // Updated by weather

    // Apply to material system
    MaterialSystem->ConfigureAdvancedFeatures(Features);
}
```

## Performance Optimization

### Distance-Based Feature Toggling

```cpp
void OptimizeFeaturesByDistance(float ViewDistance)
{
    if (ViewDistance < 10.0f)
    {
        // Close: All features enabled
        EnableParallax = true;
        EnableTessellation = true;
        ParallaxSteps = 32;
        TessellationMultiplier = 3.0f;
    }
    else if (ViewDistance < 50.0f)
    {
        // Medium: Tessellation only
        EnableParallax = false;
        EnableTessellation = true;
        TessellationMultiplier = 2.0f;
    }
    else
    {
        // Far: Macro textures only
        EnableParallax = false;
        EnableTessellation = false;
    }
}
```

### Quality Levels

| Quality | Parallax | Tessellation | Max Tess Distance | Wetness |
| ------- | -------- | ------------ | ----------------- | ------- |
| Low     | Disabled | Disabled     | 0m                | Basic   |
| Medium  | 8 steps  | 1.0x         | 30m               | Full    |
| High    | 16 steps | 2.0x         | 50m               | Full    |
| Epic    | 32 steps | 3.0x         | 75m               | Full    |

### VR Optimization

```cpp
void OptimizeForVR()
{
    // Reduce tessellation distance for VR performance
    AdvancedFeatures.MaxTessellationDistance = 30.0f;
    AdvancedFeatures.TessellationMultiplier = 1.5f;

    // Reduce parallax steps
    ParallaxSteps = 12;

    // Ensure 90 FPS minimum
    if (CurrentFrameRate < 90.0f)
    {
        // Further reduce features
        AdvancedFeatures.MaxTessellationDistance *= 0.75f;
    }
}
```

## Material Setup in Unreal Editor

### Master Material Creation

1. **Create Material**: `M_TerrainMaster`
2. **Set Properties**:

   - Material Domain: Surface
   - Blend Mode: Opaque
   - Shading Model: Default Lit
   - Enable Tessellation: True
   - D3D11 Tessellation Mode: PN Triangles

3. **Add Material Functions**:

   - Import `MF_ParallaxOcclusionMapping`
   - Import `MF_Tessellation`
   - Import `MF_DynamicWetness`
   - Import `MF_MacroMicroBlending`

4. **Connect Outputs**:
   - Base Color → Base Color
   - Normal → Normal
   - Roughness → Roughness
   - Metallic → Metallic
   - World Displacement → World Displacement
   - Tessellation Multiplier → Tessellation Multiplier

### Material Instance Creation

```cpp
// Create dynamic material instance
UMaterialInstanceDynamic* MatInst = MaterialSystem->CreateTerrainMaterialInstance();

// Apply to terrain mesh
TerrainMesh->SetMaterial(0, MatInst);
```

## Testing and Validation

### Visual Quality Tests

- [ ] Parallax visible on close surfaces (< 10m)
- [ ] No UV stretching on vertical surfaces
- [ ] Smooth transition between micro and macro textures
- [ ] Wetness darkens and smooths surfaces realistically
- [ ] Tessellation adds visible geometric detail
- [ ] No visible seams between terrain tiles
- [ ] Material blending is smooth across biomes

### Performance Tests

- [ ] Maintains 90 FPS in VR at all distances
- [ ] Tessellation disabled beyond max distance
- [ ] Parallax disabled beyond 50m
- [ ] Memory usage within budget (< 4GB textures)
- [ ] Draw calls remain reasonable (< 5000)

### Integration Tests

- [ ] Weather affects wetness correctly
- [ ] Distance updates features appropriately
- [ ] LOD transitions are smooth
- [ ] All 8 material layers work correctly
- [ ] Biome blending works with advanced features

## Troubleshooting

### Parallax Issues

**Problem**: UV swimming or artifacts

- **Solution**: Reduce `ParallaxDepthScale` to 0.03-0.04
- **Solution**: Increase `ParallaxSteps` for better quality

**Problem**: Performance drops

- **Solution**: Reduce max parallax distance
- **Solution**: Lower parallax steps at medium distance

### Tessellation Issues

**Problem**: Cracks between tiles

- **Solution**: Ensure consistent tessellation at boundaries
- **Solution**: Match displacement scale across tiles

**Problem**: Performance drops

- **Solution**: Reduce `MaxTessellationDistance`
- **Solution**: Lower `TessellationMultiplier`

### Wetness Issues

**Problem**: Too shiny

- **Solution**: Increase minimum roughness from 0.05 to 0.1
- **Solution**: Reduce metallic boost

**Problem**: Not visible

- **Solution**: Increase darkening factor (reduce from 0.6 to 0.5)
- **Solution**: Increase roughness reduction

### Macro/Micro Issues

**Problem**: Visible transition line

- **Solution**: Increase blend range
- **Solution**: Use smoothstep instead of linear blend

**Problem**: Texture repetition still visible

- **Solution**: Reduce macro tiling further
- **Solution**: Use larger macro textures

## Future Enhancements

- **Footprint/Track Deformation**: Dynamic surface deformation from player/vehicles
- **Seasonal Variations**: Material changes with seasons
- **Erosion Visualization**: Visual erosion patterns over time
- **Procedural Weathering**: Automatic weathering effects on surfaces
- **GPU-Driven Selection**: Compute shader-based material blending
- **Virtual Texturing**: Streaming for massive texture sets

## References

- Design Document: `.kiro/specs/planet-interior-enhancement/design.md`
- Requirements: `.kiro/specs/planet-interior-enhancement/requirements.md` (1.4, 1.5, 6.3)
- Material System: `Content/README_MaterialSystem.md`
- Material Functions:
  - `Content/Materials/MF_ParallaxOcclusionMapping.txt`
  - `Content/Materials/MF_Tessellation.txt`
  - `Content/Materials/MF_DynamicWetness.txt`
  - `Content/Materials/MF_MacroMicroBlending.txt`
- Master Material: `Content/Materials/M_TerrainMaster.txt`

## Implementation Status

✅ **Completed**:

- C++ infrastructure for advanced features
- Material function specifications
- Distance-based feature toggling
- Weather integration for wetness
- Macro/micro blending system
- Tessellation system
- Parallax occlusion mapping system
- Performance optimization framework

📋 **Next Steps** (for Unreal Editor):

1. Create material functions in Unreal Editor
2. Build master material with all features
3. Create material instances for testing
4. Integrate with existing terrain system
5. Performance profiling and optimization
6. Visual quality validation
