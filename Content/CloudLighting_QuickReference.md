# Cloud Lighting Quick Reference

## Key Functions

### Calculate Terrain Shadow

```cpp
float Shadow = CloudComponent->CalculateTerrainShadow(TerrainPosition, SunDirection);
// Returns: 0-1 (0=no shadow, 1=full shadow)
```

### Calculate Cloud Lighting

```cpp
float Light = CloudComponent->CalculateCloudLighting(Position, HeightInLayer, SunDirection);
// Returns: 0-1 (0=dark, 1=bright)
```

### Get Sun Direction

```cpp
FVector SunDir = CloudComponent->GetSunDirection();
// Returns: Normalized sun direction vector
```

## Key Parameters

### Quality Settings

```cpp
LightRayMarchSteps = 6;      // 4-8 typical (higher = better quality)
ShadowStepSize = 0.5f;       // 0.3-1.0 km (smaller = more accurate)
```

### Shadow Intensity

```cpp
SelfShadowIntensity = 0.8f;      // 0-1 (cloud self-shadowing)
TerrainShadowIntensity = 0.7f;   // 0-1 (ground shadows)
ShadowDensityMultiplier = 1.0f;  // 0.5-2.0 (shadow darkness)
```

### Lighting

```cpp
ScatteringIntensity = 1.0f;      // 0.5-2.0 (overall brightness)
PhaseG = 0.6f;                   // 0.5-0.8 (forward scattering)
AmbientLightIntensity = 0.2f;    // 0.1-0.4 (ambient contribution)
```

## Quick Presets

### Earth-like

```cpp
LightRayMarchSteps = 6; PhaseG = 0.6f; AmbientLightIntensity = 0.25f;
```

### Storm

```cpp
LightRayMarchSteps = 8; ShadowDensityMultiplier = 1.5f; SelfShadowIntensity = 0.9f;
```

### Performance

```cpp
LightRayMarchSteps = 4; ShadowStepSize = 1.0f; ShadowDensityMultiplier = 0.8f;
```

## Common Issues

**Shadows too dark**: Increase `AmbientLightIntensity` to 0.3-0.4
**Shadows too light**: Increase `ShadowDensityMultiplier` to 1.2-1.5
**Performance issues**: Reduce `LightRayMarchSteps` to 4, increase `ShadowStepSize` to 1.0
**No shadows**: Check `DayNightCycleComponent` is set and sun is above horizon

## Material Parameters

Pass these to your cloud material:

- `SunDirection` (Vector3)
- `LightRayMarchSteps` (Scalar)
- `ShadowStepSize` (Scalar)
- `AmbientLightIntensity` (Scalar)
- `AmbientLightColor` (Vector3)
- `SelfShadowIntensity` (Scalar)

## Performance Targets

- CPU queries: < 0.5ms per call
- GPU rendering: < 2ms per frame
- VR target: 90 FPS maintained

## Documentation

- Full docs: `TASK_7.3_IMPLEMENTATION.md`
- Quick start: `QUICKSTART_CloudLighting.md`
- Summary: `CLOUD_LIGHTING_SUMMARY.md`
