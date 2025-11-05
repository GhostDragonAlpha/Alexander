# Atmospheric Fog System

## Overview

The Atmospheric Fog System provides realistic altitude-based fog effects, exponential height fog, and atmospheric perspective for planetary environments. It integrates seamlessly with Unreal Engine's Exponential Height Fog component and Lumen global illumination to create immersive atmospheric effects.

## Features

### 1. Altitude-Based Fog Density

- Exponential fog density falloff with altitude
- Configurable base density and height falloff parameters
- Maximum fog altitude setting for performance optimization
- Automatic density calculation based on viewer position

### 2. Exponential Height Fog

- Integration with Unreal's Exponential Height Fog component
- Automatic synchronization of fog parameters
- Support for directional inscattering (sun glow through fog)
- Configurable fog start distance and cutoff distance

### 3. Atmospheric Perspective

- Distance-based color shift and haziness
- Smooth blending between base color and fog color
- Desaturation of distant objects for depth perception
- Configurable perspective distance and intensity

### 4. Lumen Integration

- Volumetric fog support for light shafts and scattering
- Automatic integration with Lumen global illumination
- Dynamic fog contribution based on atmospheric conditions
- Support for indirect lighting in fog

## Components

### UAtmosphericFogComponent

Main component that manages all atmospheric fog effects.

**Key Properties:**

- `FogSettings`: Configuration structure for all fog parameters
- `PlanetRadius`: Planet radius for altitude calculations (km)
- `AtmosphereComponent`: Reference to atmosphere component for integration
- `bUseExponentialHeightFog`: Enable Unreal's height fog integration
- `bAutoUpdateFog`: Automatically update fog based on viewer position
- `UpdateFrequency`: How often to update fog (seconds)

**Key Methods:**

- `InitializeFog()`: Initialize fog with planet parameters
- `ApplyFogSettings()`: Apply new fog configuration
- `GetFogDensityAtAltitude()`: Get fog density at specific altitude
- `GetFogColorAtAltitude()`: Get fog color at specific altitude
- `CalculateAtmosphericPerspective()`: Calculate perspective effect
- `UpdateFogForViewerPosition()`: Update fog based on viewer location

### FAtmosphericFogConfig

Configuration structure for fog parameters.

**Properties:**

- `BaseFogDensity`: Base fog density at sea level (default: 0.02)
- `HeightFalloff`: Fog density falloff rate (default: 0.2)
- `MaxFogAltitude`: Maximum altitude for fog visibility (default: 10 km)
- `GroundFogColor`: Fog color at ground level
- `HighAltitudeFogColor`: Fog color at high altitude
- `InscatteringColor`: Light scattered into view
- `DirectionalInscatteringIntensity`: Sun glow intensity (default: 0.5)
- `DirectionalInscatteringExponent`: Sun glow size (default: 4.0)
- `FogStartDistance`: Distance where fog begins (meters)
- `FogCutoffDistance`: Distance where fog ends (0 = infinite)
- `AtmosphericPerspectiveIntensity`: Overall perspective effect strength
- `PerspectiveDistance`: Distance for full perspective (default: 50 km)
- `bEnableVolumetricFog`: Enable volumetric fog for Lumen
- `VolumetricFogScatteringDistribution`: Forward scattering amount
- `VolumetricFogAlbedo`: Color of scattered light
- `VolumetricFogExtinctionScale`: Light absorption scale

## Usage

### Basic Setup

1. **Add Component to Planet**

```cpp
// In APlanet constructor
FogComponent = CreateDefaultSubobject<UAtmosphericFogComponent>(TEXT("FogComponent"));
FogComponent->SetupAttachment(RootComponent);
```

2. **Initialize in BeginPlay**

```cpp
// In APlanet::BeginPlay()
if (FogComponent && PlanetConfig && PlanetConfig->bHasAtmosphere)
{
    FAtmosphericFogConfig FogConfig;
    FogConfig.BaseFogDensity = 0.02f;
    FogConfig.HeightFalloff = 0.2f;
    FogConfig.MaxFogAltitude = 10.0f;

    FogComponent->AtmosphereComponent = AtmosphereComponent;
    FogComponent->InitializeFog(PlanetRadius, FogConfig);
}
```

### Blueprint Usage

```blueprint
// Get fog component
FogComponent = Planet->FogComponent

// Update fog settings
NewFogSettings = Make FAtmosphericFogConfig
NewFogSettings.BaseFogDensity = 0.05
NewFogSettings.HeightFalloff = 0.3
FogComponent->ApplyFogSettings(NewFogSettings)

// Get fog density at altitude
Density = FogComponent->GetFogDensityAtAltitude(5.0)  // 5km

// Get fog color at altitude
Color = FogComponent->GetFogColorAtAltitude(5.0)

// Calculate atmospheric perspective
PerspectiveColor = FogComponent->CalculateAtmosphericPerspective(
    ViewPosition,
    TargetPosition,
    BaseColor
)
```

### Material Integration

Use the `MF_AtmosphericPerspective` material function in your materials:

```
// In terrain or object material
BaseColor = YourMaterialColor
FinalColor = MF_AtmosphericPerspective(
    BaseColor,
    WorldPosition,
    CameraPosition,
    FogColor,
    FogDensity,
    PerspectiveDistance,
    PerspectiveIntensity
)
```

## Configuration Examples

### Clear Day

```cpp
FogConfig.BaseFogDensity = 0.01f;
FogConfig.HeightFalloff = 0.15f;
FogConfig.GroundFogColor = FLinearColor(0.8f, 0.85f, 0.9f);
FogConfig.HighAltitudeFogColor = FLinearColor(0.6f, 0.7f, 0.9f);
FogConfig.AtmosphericPerspectiveIntensity = 0.8f;
```

### Foggy Morning

```cpp
FogConfig.BaseFogDensity = 0.08f;
FogConfig.HeightFalloff = 0.4f;
FogConfig.GroundFogColor = FLinearColor(0.7f, 0.75f, 0.8f);
FogConfig.HighAltitudeFogColor = FLinearColor(0.6f, 0.65f, 0.75f);
FogConfig.AtmosphericPerspectiveIntensity = 1.5f;
FogConfig.FogStartDistance = 100.0f;
```

### High Altitude (Thin Atmosphere)

```cpp
FogConfig.BaseFogDensity = 0.005f;
FogConfig.HeightFalloff = 0.5f;
FogConfig.MaxFogAltitude = 5.0f;
FogConfig.GroundFogColor = FLinearColor(0.5f, 0.6f, 0.8f);
FogConfig.HighAltitudeFogColor = FLinearColor(0.3f, 0.4f, 0.7f);
FogConfig.AtmosphericPerspectiveIntensity = 0.5f;
```

### Alien Planet (Red Atmosphere)

```cpp
FogConfig.BaseFogDensity = 0.03f;
FogConfig.HeightFalloff = 0.2f;
FogConfig.GroundFogColor = FLinearColor(0.9f, 0.6f, 0.5f);
FogConfig.HighAltitudeFogColor = FLinearColor(0.8f, 0.4f, 0.3f);
FogConfig.InscatteringColor = FLinearColor(1.0f, 0.7f, 0.6f);
```

## Integration with Other Systems

### Atmosphere Component

The fog component automatically integrates with `UPlanetAtmosphereComponent`:

- Shares planet radius and atmospheric parameters
- Coordinates fog colors with atmospheric scattering
- Synchronizes altitude calculations

### Weather System

Fog responds to weather conditions:

```cpp
// In weather transition
if (NewWeather == EWeatherType::Fog)
{
    FogConfig.BaseFogDensity *= 5.0f;
    FogComponent->ApplyFogSettings(FogConfig);
}
```

### Material System

Fog parameters are exposed to materials through Material Parameter Collections:

- `MPC_AtmosphericFog`: Contains all fog parameters
- Automatically updated by fog component
- Accessible in all materials

## Performance Optimization

### Update Frequency

```cpp
// Reduce update frequency for better performance
FogComponent->UpdateFrequency = 0.2f;  // Update every 0.2 seconds

// Disable auto-update and manually update when needed
FogComponent->bAutoUpdateFog = false;
FogComponent->UpdateFogForViewerPosition(PlayerLocation);
```

### Volumetric Fog Quality

```cpp
// Adjust volumetric fog quality
FogConfig.bEnableVolumetricFog = true;  // High quality
// or
FogConfig.bEnableVolumetricFog = false;  // Better performance
```

### LOD Integration

```cpp
// Disable atmospheric perspective on distant LODs
if (LODLevel > 3)
{
    FogConfig.AtmosphericPerspectiveIntensity = 0.0f;
}
```

## Debugging

### Visualization

```cpp
// Enable debug visualization
FogComponent->bShowDebugInfo = true;

// Log fog parameters
UE_LOG(LogTemp, Log, TEXT("Fog Density: %.3f"), FogComponent->GetFogDensityAtAltitude(ViewerAltitude));
UE_LOG(LogTemp, Log, TEXT("Fog Color: %s"), *FogComponent->GetFogColorAtAltitude(ViewerAltitude).ToString());
```

### Console Commands

```
// Show exponential height fog
show ExponentialHeightFog

// Show volumetric fog
show VolumetricFog

// Adjust fog density at runtime
r.VolumetricFog.GridPixelSize 8  // Lower = higher quality, worse performance
```

## Common Issues

### Fog Not Visible

- Check `bUseExponentialHeightFog` is true
- Verify `HeightFogComponent` is created
- Ensure `BaseFogDensity` is not too low
- Check fog is positioned at planet surface

### Fog Too Dense

- Reduce `BaseFogDensity`
- Increase `HeightFalloff` for faster altitude falloff
- Adjust `FogStartDistance` to push fog further away

### Performance Issues

- Disable `bEnableVolumetricFog` on lower quality settings
- Increase `UpdateFrequency` to reduce update rate
- Use fog cutoff distance to limit fog range
- Reduce `VolumetricFogScatteringDistribution` quality

### Color Mismatch with Atmosphere

- Ensure fog colors match atmosphere scattering colors
- Link `FogComponent->AtmosphereComponent` properly
- Verify `GroundAlbedo` in atmosphere settings

## Technical Details

### Exponential Fog Formula

```
Density(altitude) = BaseDensity * exp(-altitude * HeightFalloff)
```

### Atmospheric Perspective Formula

```
PerspectiveFactor = saturate(distance / PerspectiveDistance)^2
FogAmount = 1 - exp(-distance * FogDensity)
FinalFog = FogAmount * PerspectiveFactor * Intensity
FinalColor = lerp(BaseColor, FogColor, FinalFog)
```

### Altitude Calculation

```
Altitude = |ViewerPosition - PlanetCenter| - PlanetRadius
```

## Future Enhancements

- [ ] Dynamic fog based on time of day
- [ ] Fog shadows from clouds
- [ ] Localized fog patches
- [ ] Fog interaction with wind
- [ ] Fog density variation based on terrain features
- [ ] Fog particle effects for dense fog
- [ ] Fog sound effects (muffled audio in fog)

## References

- [Unreal Engine Exponential Height Fog](https://docs.unrealengine.com/5.0/en-US/exponential-height-fog-in-unreal-engine/)
- [Lumen Global Illumination](https://docs.unrealengine.com/5.0/en-US/lumen-global-illumination-and-reflections-in-unreal-engine/)
- [Volumetric Fog](https://docs.unrealengine.com/5.0/en-US/volumetric-fog-in-unreal-engine/)
- GPU Gems 2, Chapter 16: Accurate Atmospheric Scattering
