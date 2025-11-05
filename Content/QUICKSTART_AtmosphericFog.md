# Quick Start: Atmospheric Fog System

## 5-Minute Setup Guide

### Step 1: Verify Component Setup (1 minute)

The atmospheric fog component is automatically added to planets. Verify it's present:

1. Open your planet Blueprint or C++ class
2. Check that `FogComponent` exists in the components list
3. Verify it's attached to the root component

### Step 2: Configure Basic Fog (2 minutes)

Set up basic fog parameters in your planet configuration:

**In Blueprint:**

```
1. Select your Planet actor
2. Find "Fog Component" in the Details panel
3. Expand "Fog Settings"
4. Set these values:
   - Base Fog Density: 0.02
   - Height Falloff: 0.2
   - Max Fog Altitude: 10.0 km
   - Ground Fog Color: Light blue-gray (0.7, 0.75, 0.8)
```

**In C++ (Planet Configuration):**

```cpp
// In your planet's BeginPlay or configuration
FAtmosphericFogConfig FogConfig;
FogConfig.BaseFogDensity = 0.02f;
FogConfig.HeightFalloff = 0.2f;
FogConfig.MaxFogAltitude = 10.0f;
FogConfig.GroundFogColor = FLinearColor(0.7f, 0.75f, 0.8f);

FogComponent->InitializeFog(PlanetRadius, FogConfig);
```

### Step 3: Enable Volumetric Fog (1 minute)

For Lumen integration and light shafts:

```
1. In Fog Component settings
2. Check "Enable Volumetric Fog"
3. Set Volumetric Fog Scattering Distribution: 0.2
4. Set Volumetric Fog Albedo: White (0.9, 0.9, 0.9)
```

### Step 4: Test in Play Mode (1 minute)

1. Press Play
2. Fly toward the planet
3. Observe fog density increasing as you approach surface
4. Notice distant objects becoming hazier

## Common Configurations

### Earth-Like Planet

```cpp
FogConfig.BaseFogDensity = 0.02f;
FogConfig.HeightFalloff = 0.2f;
FogConfig.GroundFogColor = FLinearColor(0.7f, 0.75f, 0.8f);
FogConfig.bEnableVolumetricFog = true;
```

### Desert Planet (Dusty)

```cpp
FogConfig.BaseFogDensity = 0.03f;
FogConfig.HeightFalloff = 0.15f;
FogConfig.GroundFogColor = FLinearColor(0.85f, 0.75f, 0.65f);
FogConfig.DirectionalInscatteringIntensity = 0.8f;
```

### Ice Planet (Clear)

```cpp
FogConfig.BaseFogDensity = 0.01f;
FogConfig.HeightFalloff = 0.25f;
FogConfig.GroundFogColor = FLinearColor(0.8f, 0.85f, 0.95f);
FogConfig.AtmosphericPerspectiveIntensity = 0.6f;
```

### Alien Planet (Thick Atmosphere)

```cpp
FogConfig.BaseFogDensity = 0.05f;
FogConfig.HeightFalloff = 0.1f;
FogConfig.GroundFogColor = FLinearColor(0.6f, 0.8f, 0.7f);
FogConfig.MaxFogAltitude = 15.0f;
```

## Adjusting Fog at Runtime

### Blueprint

```blueprint
// Get fog component
FogComp = Planet->GetComponentByClass(AtmosphericFogComponent)

// Create new settings
NewSettings = Make FAtmosphericFogConfig
NewSettings.BaseFogDensity = 0.05
NewSettings.HeightFalloff = 0.3

// Apply settings
FogComp->ApplyFogSettings(NewSettings)
```

### C++

```cpp
// Get fog component
UAtmosphericFogComponent* FogComp = Planet->FogComponent;

// Modify settings
FAtmosphericFogConfig NewSettings = FogComp->FogSettings;
NewSettings.BaseFogDensity = 0.05f;
NewSettings.HeightFalloff = 0.3f;

// Apply
FogComp->ApplyFogSettings(NewSettings);
```

## Troubleshooting

### "I don't see any fog"

- Check `bUseExponentialHeightFog` is enabled
- Increase `BaseFogDensity` to 0.05 or higher
- Verify you're close enough to the planet surface
- Check that atmosphere is enabled on the planet

### "Fog is too thick"

- Reduce `BaseFogDensity` to 0.01 or lower
- Increase `HeightFalloff` to 0.3 or higher
- Increase `FogStartDistance` to push fog further away

### "Performance is poor"

- Disable `bEnableVolumetricFog`
- Increase `UpdateFrequency` to 0.2 or higher
- Set `FogCutoffDistance` to limit fog range

### "Fog color doesn't match atmosphere"

- Link fog component to atmosphere: `FogComponent->AtmosphereComponent = AtmosphereComponent`
- Match `GroundFogColor` with atmosphere's `GroundAlbedo`
- Match `InscatteringColor` with atmosphere's `RayleighScatteringCoefficient`

## Next Steps

1. **Integrate with Weather**: Adjust fog density based on weather conditions
2. **Add Material Effects**: Use `MF_AtmosphericPerspective` in your materials
3. **Optimize Performance**: Tune update frequency and volumetric fog settings
4. **Create Presets**: Save different fog configurations for various planet types

## Advanced Features

### Dynamic Fog Based on Time of Day

```cpp
// In your day/night cycle system
float TimeOfDay = GetTimeOfDay();  // 0-24 hours
float FogMultiplier = 1.0f;

// More fog at dawn/dusk
if (TimeOfDay >= 5.0f && TimeOfDay <= 7.0f)
    FogMultiplier = 2.0f;  // Morning fog
else if (TimeOfDay >= 18.0f && TimeOfDay <= 20.0f)
    FogMultiplier = 1.5f;  // Evening haze

FogConfig.BaseFogDensity = 0.02f * FogMultiplier;
FogComponent->ApplyFogSettings(FogConfig);
```

### Weather-Responsive Fog

```cpp
// In weather system
void OnWeatherChanged(EWeatherType NewWeather)
{
    FAtmosphericFogConfig FogConfig = FogComponent->FogSettings;

    switch (NewWeather)
    {
        case EWeatherType::Clear:
            FogConfig.BaseFogDensity = 0.01f;
            break;
        case EWeatherType::Fog:
            FogConfig.BaseFogDensity = 0.1f;
            break;
        case EWeatherType::Rain:
            FogConfig.BaseFogDensity = 0.04f;
            break;
    }

    FogComponent->ApplyFogSettings(FogConfig);
}
```

## Resources

- Full Documentation: `Content/README_AtmosphericFog.md`
- Material Function: `Content/Materials/MF_AtmosphericPerspective.txt`
- Atmospheric Rendering: `Content/README_AtmosphericRendering.md`
- Example Planets: `Content/Planets/ExamplePlanets/`

## Support

For issues or questions:

1. Check the full documentation in `README_AtmosphericFog.md`
2. Review the implementation in `AtmosphericFogComponent.h/cpp`
3. Examine example planet configurations
4. Test with different fog presets to understand the parameters
