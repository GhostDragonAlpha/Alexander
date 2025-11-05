# Quick Start: Weather Lighting System

## Overview

The weather lighting system dynamically adjusts lighting, fog, and atmospheric effects based on weather conditions. This guide shows you how to set up and use the system.

## Setup

### 1. Link Components

In your Planet actor's BeginPlay or construction script:

```cpp
// C++ Example
void APlanet::BeginPlay()
{
    Super::BeginPlay();

    // Get components
    UPlanetWeatherComponent* WeatherComp = FindComponentByClass<UPlanetWeatherComponent>();
    UPlanetAtmosphereComponent* AtmosphereComp = FindComponentByClass<UPlanetAtmosphereComponent>();
    UPlanetCloudComponent* CloudComp = FindComponentByClass<UPlanetCloudComponent>();

    // Link components
    if (WeatherComp)
    {
        if (AtmosphereComp)
        {
            WeatherComp->SetAtmosphereComponent(AtmosphereComp);
        }

        if (CloudComp)
        {
            WeatherComp->SetCloudComponent(CloudComp);
        }
    }
}
```

### 2. Add Exponential Height Fog (Optional)

For fog effects to work, add an `ExponentialHeightFog` actor to your level:

1. In the level editor, search for "Exponential Height Fog"
2. Drag it into your level
3. Set base fog density (e.g., 0.02)
4. Weather system will automatically adjust it

## Using Weather Lighting

### Query Lighting Parameters

```cpp
// Get weather component
UPlanetWeatherComponent* Weather = GetWeatherComponent();

// Query lighting values
float SunIntensity = Weather->GetSunIntensityMultiplier();
float SkyLightIntensity = Weather->GetSkyLightIntensityMultiplier();
float FogDensity = Weather->GetWeatherFogDensity();
FLinearColor FogColor = Weather->GetWeatherFogColor();
float CloudOcclusion = Weather->GetCloudCoverageOcclusion();
float StormDarkness = Weather->GetStormDarknessFactor();
```

### Apply to Lights

```cpp
// Apply to directional light (sun)
if (DirectionalLight)
{
    float BaseSunIntensity = 10.0f;
    float WeatherMultiplier = Weather->GetSunIntensityMultiplier();
    DirectionalLight->SetIntensity(BaseSunIntensity * WeatherMultiplier);
}

// Apply to sky light
if (SkyLight)
{
    float BaseSkyIntensity = 1.0f;
    float WeatherMultiplier = Weather->GetSkyLightIntensityMultiplier();
    SkyLight->SetIntensity(BaseSkyIntensity * WeatherMultiplier);
}
```

### Use in Materials

Expose weather lighting to materials via Material Parameter Collection:

```cpp
// Update material parameter collection
if (MaterialParameterCollection)
{
    UKismetMaterialLibrary::SetScalarParameterValue(
        GetWorld(),
        MaterialParameterCollection,
        FName("WeatherFogDensity"),
        Weather->GetWeatherFogDensity()
    );

    UKismetMaterialLibrary::SetVectorParameterValue(
        GetWorld(),
        MaterialParameterCollection,
        FName("WeatherFogColor"),
        Weather->GetWeatherFogColor()
    );
}
```

## Weather Lighting Effects by Type

### Clear Weather

- **Sun Intensity**: 100% (full brightness)
- **Sky Light**: 100%
- **Fog Density**: 0.5x (minimal)
- **Fog Color**: Light blue
- **Darkness**: 0%

### Overcast

- **Sun Intensity**: 60%
- **Sky Light**: 70%
- **Fog Density**: 1.2x
- **Fog Color**: Gray
- **Darkness**: 20%

### Heavy Rain

- **Sun Intensity**: 40%
- **Sky Light**: 55%
- **Fog Density**: 2.0x
- **Fog Color**: Blue-gray
- **Darkness**: 50%

### Thunderstorm

- **Sun Intensity**: 30% (very dim)
- **Sky Light**: 45%
- **Fog Density**: 2.5x
- **Fog Color**: Dark blue-gray
- **Darkness**: 70% (very dark)
- **Cloud Occlusion**: Maximum

### Sandstorm

- **Sun Intensity**: 30%
- **Sky Light**: 40% (lowest)
- **Fog Density**: 3.0x (very dense)
- **Fog Color**: Orange-brown
- **Darkness**: 60%
- **Scattering**: 2.5x (very hazy)

### Fog

- **Sun Intensity**: 40%
- **Sky Light**: 60%
- **Fog Density**: 4.0x (maximum)
- **Fog Color**: Neutral gray-blue
- **Darkness**: 30%

## Blueprint Usage

### Get Weather Lighting in Blueprint

1. Get reference to Weather Component
2. Call any of these nodes:
   - `Get Weather Fog Density`
   - `Get Weather Fog Color`
   - `Get Sun Intensity Multiplier`
   - `Get Sky Light Intensity Multiplier`
   - `Get Cloud Coverage Occlusion`
   - `Get Storm Darkness Factor`

### Example Blueprint: Dynamic Light Adjustment

```
Event Tick
├─ Get Weather Component
├─ Get Sun Intensity Multiplier
├─ Multiply (Base Intensity * Multiplier)
└─ Set Intensity (Directional Light)
```

## Automatic Features

The weather lighting system automatically:

1. **Smooth Transitions**: All lighting parameters interpolate during weather changes
2. **Fog Updates**: Adjusts exponential height fog if present in level
3. **Atmosphere Integration**: Modifies atmospheric scattering and fog density
4. **Cloud Occlusion**: Calculates ambient light blocking from cloud coverage

## Performance

- **CPU Cost**: < 0.1ms per frame
- **Memory**: Negligible (no textures or large data structures)
- **Scalability**: Works on all platforms including VR

## Troubleshooting

### Fog Not Changing

- Ensure `ExponentialHeightFog` actor exists in level
- Check that weather component is ticking
- Verify weather transitions are occurring

### Lighting Not Updating

- Confirm atmosphere component is linked via `SetAtmosphereComponent()`
- Check that lights are being updated with multiplier values
- Verify weather is actually changing (check current weather state)

### Transitions Too Fast/Slow

- Adjust `DefaultTransitionDuration` in weather component (default: 60 seconds)
- Use `TransitionToWeather()` with custom duration parameter

## Advanced Usage

### Custom Weather Lighting

Override lighting values for custom weather types:

```cpp
// Create custom weather preset
FWeatherPreset CustomWeather;
CustomWeather.WeatherType = EWeatherType::Custom;
CustomWeather.Probability = 0.1f;

// Add to weather component
WeatherComponent->WeatherPresets.Add(CustomWeather);

// Extend GetSunIntensityMultiplier() for custom type
```

### Time-of-Day Integration

Combine with day-night cycle for realistic lighting:

```cpp
// Get base lighting from time of day
float TimeOfDayMultiplier = DayNightCycle->GetSunIntensity();

// Apply weather multiplier
float WeatherMultiplier = Weather->GetSunIntensityMultiplier();

// Final intensity
float FinalIntensity = BaseSunIntensity * TimeOfDayMultiplier * WeatherMultiplier;
```

### Post-Process Integration

Use weather lighting in post-process volumes:

```cpp
// Adjust exposure based on storm darkness
float StormDarkness = Weather->GetStormDarknessFactor();
float ExposureCompensation = -StormDarkness * 2.0f; // Darker during storms

PostProcessVolume->Settings.AutoExposureBias = ExposureCompensation;
```

## Next Steps

- Integrate with day-night cycle for time-based lighting
- Add lightning flashes during thunderstorms
- Create material functions using weather parameters
- Set up post-process color grading per weather type
- Add volumetric fog for enhanced depth

## Related Documentation

- `TASK_8.3_IMPLEMENTATION.md` - Full implementation details
- `TASK_7.3_IMPLEMENTATION.md` - Cloud lighting system
- `TASK_6.3_IMPLEMENTATION.md` - Atmosphere system
- `QUICKSTART_CloudLighting.md` - Cloud lighting setup
