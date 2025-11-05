# Precipitation Effects Implementation Guide

## Overview

This guide explains how to create and configure Niagara particle systems for weather precipitation effects including rain, snow, sandstorms, and fog.

## Implementation Status

✅ **C++ Integration Complete**

- Weather component updated with precipitation system support
- Automatic precipitation spawning based on weather type
- Smooth transitions between precipitation types
- Intensity blending during weather transitions

## Required Niagara Systems

The following Niagara systems need to be created in the Unreal Editor:

### 1. Rain Particle System (`NS_Rain`)

**Location**: `Content/Niagara/Weather/NS_Rain.uasset`

**Parameters**:

- `Intensity` (Float): Controls rain intensity (0.0 - 1.0)
- `SpawnRate` (Float): Number of particles per second (0 - 1000)

**Emitter Configuration**:

- **Spawn Rate**: Controlled by `SpawnRate` parameter
- **Lifetime**: 2-4 seconds (randomized)
- **Velocity**: Downward (-Z) with slight wind offset
  - Base: (0, 0, -800) cm/s
  - Random: ±50 cm/s in X/Y
- **Size**:
  - Length: 20-40 cm (stretched droplets)
  - Width: 1-2 cm
- **Color**: Light blue-white with transparency
  - RGB: (0.8, 0.9, 1.0)
  - Alpha: 0.3-0.6
- **Spawn Volume**: Box emitter following camera
  - Size: 2000x2000x1000 cm
  - Offset: Above camera by 500 cm
- **Collision**: Optional ground collision with splash effect

**Visual Effects**:

- Stretched sprite for motion blur
- Fade in/out over lifetime
- Slight turbulence for realism
- Optional splash particles on impact

### 2. Snow Particle System (`NS_Snow`)

**Location**: `Content/Niagara/Weather/NS_Snow.uasset`

**Parameters**:

- `Intensity` (Float): Controls snow intensity (0.0 - 1.0)
- `SpawnRate` (Float): Number of particles per second (0 - 1000)

**Emitter Configuration**:

- **Spawn Rate**: Controlled by `SpawnRate` parameter
- **Lifetime**: 5-8 seconds (randomized)
- **Velocity**: Gentle downward with drift
  - Base: (0, 0, -200) cm/s
  - Wind drift: ±100 cm/s in X/Y
  - Turbulence: Perlin noise for floating effect
- **Size**:
  - Diameter: 3-8 cm (varied snowflake sizes)
- **Color**: Pure white with high alpha
  - RGB: (1.0, 1.0, 1.0)
  - Alpha: 0.7-0.9
- **Rotation**: Slow random rotation for realism
- **Spawn Volume**: Box emitter following camera
  - Size: 2500x2500x1500 cm
  - Offset: Above camera by 800 cm

**Visual Effects**:

- Soft particle blending
- Gentle swaying motion
- Accumulation on surfaces (optional)
- Sparkle effect in light

### 3. Sandstorm Particle System (`NS_Sandstorm`)

**Location**: `Content/Niagara/Weather/NS_Sandstorm.uasset`

**Parameters**:

- `Intensity` (Float): Controls sandstorm intensity (0.0 - 1.0)
- `SpawnRate` (Float): Number of particles per second (0 - 2000)

**Emitter Configuration**:

- **Spawn Rate**: Controlled by `SpawnRate` parameter (higher than rain/snow)
- **Lifetime**: 3-6 seconds (randomized)
- **Velocity**: Horizontal with strong wind
  - Base: (500, 0, -50) cm/s (adjust direction as needed)
  - Random: ±200 cm/s in all axes
  - Strong turbulence for chaotic movement
- **Size**:
  - Diameter: 5-15 cm (dust particles)
- **Color**: Sandy brown with varying opacity
  - RGB: (0.8, 0.6, 0.3)
  - Alpha: 0.2-0.5 (creates haze effect)
- **Spawn Volume**: Large box emitter
  - Size: 5000x5000x2000 cm
  - Surrounds camera

**Visual Effects**:

- High particle count for density
- Strong turbulence and swirling
- Reduced visibility (fog-like)
- Dust trails and streaks
- Color variation for depth

### 4. Fog Particle System (`NS_Fog`)

**Location**: `Content/Niagara/Weather/NS_Fog.uasset`

**Parameters**:

- `Intensity` (Float): Controls fog density (0.0 - 1.0)
- `SpawnRate` (Float): Number of particles per second (0 - 500)

**Emitter Configuration**:

- **Spawn Rate**: Controlled by `SpawnRate` parameter (lower, larger particles)
- **Lifetime**: 10-15 seconds (long-lived)
- **Velocity**: Very slow drift
  - Base: (20, 0, 0) cm/s
  - Random: ±30 cm/s in all axes
  - Gentle turbulence
- **Size**:
  - Diameter: 200-500 cm (large, soft particles)
- **Color**: Gray-white with low opacity
  - RGB: (0.9, 0.9, 0.9)
  - Alpha: 0.1-0.3 (very transparent, builds up)
- **Spawn Volume**: Large sphere around camera
  - Radius: 3000 cm
  - Fills space around player

**Visual Effects**:

- Very soft particle blending
- Large, billowy clouds
- Slow, gentle movement
- Depth-based fading
- Volumetric appearance

## C++ Integration

### Weather Component Setup

The `UPlanetWeatherComponent` automatically manages precipitation effects:

```cpp
// Precipitation systems are assigned in editor
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precipitation")
UNiagaraSystem* RainParticleSystem;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precipitation")
UNiagaraSystem* SnowParticleSystem;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precipitation")
UNiagaraSystem* SandstormParticleSystem;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precipitation")
UNiagaraSystem* FogParticleSystem;
```

### Automatic Precipitation Control

The system automatically:

1. Spawns appropriate precipitation based on current weather
2. Adjusts intensity based on weather severity
3. Smoothly transitions between precipitation types
4. Stops precipitation when weather clears

### Intensity Mapping

Weather types map to intensities as follows:

| Weather Type  | Precipitation System | Intensity |
| ------------- | -------------------- | --------- |
| Clear         | None                 | 0.0       |
| Partly Cloudy | None                 | 0.0       |
| Overcast      | None                 | 0.0       |
| Light Rain    | Rain                 | 0.4       |
| Heavy Rain    | Rain                 | 0.8       |
| Thunderstorm  | Rain                 | 1.0       |
| Light Snow    | Snow                 | 0.4       |
| Heavy Snow    | Snow                 | 0.8       |
| Sandstorm     | Sandstorm            | 0.9       |
| Fog           | Fog                  | 0.7       |

## Creating Niagara Systems in Unreal Editor

### Step 1: Create Folder Structure

1. Navigate to `Content/Niagara/`
2. Create new folder: `Weather`

### Step 2: Create Rain System

1. Right-click in `Content/Niagara/Weather/`
2. Select `Niagara System` → `New system from selected emitters`
3. Choose `Simple Sprite Burst` as template
4. Name it `NS_Rain`
5. Configure emitter properties:
   - Add User Parameter: `Intensity` (Float, default 0.5)
   - Add User Parameter: `SpawnRate` (Float, default 500)
   - Set Spawn Rate module to use `SpawnRate` parameter
   - Configure velocity, size, color as specified above
   - Add Camera Offset module to follow player
   - Add Collision module for ground impacts (optional)

### Step 3: Create Snow System

1. Duplicate `NS_Rain` or create new system
2. Name it `NS_Snow`
3. Modify properties:
   - Reduce velocity (slower fall)
   - Add turbulence for floating effect
   - Increase particle size
   - Change color to white
   - Add rotation module
   - Adjust spawn volume (larger, higher)

### Step 4: Create Sandstorm System

1. Create new Niagara System
2. Name it `NS_Sandstorm`
3. Configure for high particle count:
   - Horizontal velocity with strong wind
   - High spawn rate
   - Sandy color with low alpha
   - Strong turbulence
   - Large spawn volume surrounding camera

### Step 5: Create Fog System

1. Create new Niagara System
2. Name it `NS_Fog`
3. Configure for volumetric effect:
   - Very large particles (200-500 cm)
   - Low opacity (0.1-0.3)
   - Slow movement
   - Long lifetime
   - Sphere spawn volume around camera

## Blueprint Setup

### Assigning Particle Systems to Planet

1. Select your Planet actor in the level
2. Find the `PlanetWeatherComponent`
3. In the Details panel, under `Precipitation` category:
   - Assign `NS_Rain` to `Rain Particle System`
   - Assign `NS_Snow` to `Snow Particle System`
   - Assign `NS_Sandstorm` to `Sandstorm Particle System`
   - Assign `NS_Fog` to `Fog Particle System`

### Testing Precipitation

Use console commands or Blueprint to test:

```cpp
// Transition to rainy weather
WeatherComponent->TransitionToWeather(EWeatherType::HeavyRain, 10.0f);

// Immediate snow
WeatherComponent->SetWeatherImmediate(EWeatherType::HeavySnow);

// Test sandstorm
WeatherComponent->TransitionToWeather(EWeatherType::Sandstorm, 15.0f);
```

## Performance Considerations

### Optimization Tips

1. **Particle Count Limits**:

   - Rain: Max 1000 particles
   - Snow: Max 1000 particles
   - Sandstorm: Max 2000 particles (higher for density)
   - Fog: Max 500 particles (large particles)

2. **Culling**:

   - Enable distance culling beyond 5000 cm
   - Use camera-relative spawning
   - Limit spawn volume to visible area

3. **LOD System**:

   - Reduce particle count at lower LODs
   - Simplify effects when far from camera
   - Disable collision at distance

4. **VR Optimization**:
   - Keep particle count conservative
   - Use instanced rendering
   - Optimize material complexity
   - Test frame rate in VR (target 90 FPS)

### Memory Usage

- Each system: ~5-10 MB
- Active particles: ~1-2 MB per 1000 particles
- Total budget: ~50 MB for all weather effects

## Visual Quality Guidelines

### Realism Targets

1. **Rain**:

   - Visible individual droplets
   - Motion blur streaks
   - Splash effects on impact
   - Proper lighting interaction

2. **Snow**:

   - Soft, floating appearance
   - Gentle swaying motion
   - Sparkle in light
   - Accumulation effect (optional)

3. **Sandstorm**:

   - Dense, obscuring effect
   - Chaotic, swirling motion
   - Reduced visibility
   - Dust trails and streaks

4. **Fog**:
   - Volumetric appearance
   - Depth-based fading
   - Soft, billowy clouds
   - Atmospheric perspective

## Troubleshooting

### Particles Not Spawning

- Check that Niagara systems are assigned in Weather Component
- Verify weather type requires precipitation
- Check spawn rate parameter is > 0
- Ensure component is ticking

### Performance Issues

- Reduce spawn rate
- Decrease particle lifetime
- Enable distance culling
- Simplify particle materials
- Reduce spawn volume size

### Visual Artifacts

- Adjust particle size and alpha
- Check sorting and blending modes
- Verify camera offset is working
- Test depth fade settings

## Future Enhancements

Potential improvements for precipitation effects:

1. **Surface Interaction**:

   - Puddle formation from rain
   - Snow accumulation on surfaces
   - Dust settling from sandstorms

2. **Audio Integration**:

   - Rain patter sounds
   - Wind howling in sandstorms
   - Thunder during storms

3. **Gameplay Effects**:

   - Reduced visibility in heavy weather
   - Slippery surfaces from rain/snow
   - Wind forces affecting player movement

4. **Advanced Effects**:
   - Lightning strikes during thunderstorms
   - Rainbow effects after rain
   - Heat shimmer in sandstorms
   - Volumetric fog with light shafts

## References

- Requirement 7.3: Weather particle effects
- Design Document: Weather System section
- UPlanetWeatherComponent: C++ implementation
- Niagara Documentation: Unreal Engine 5.6

## Summary

The precipitation effects system provides realistic weather visualization through Niagara particle systems. The C++ integration is complete and automatically manages precipitation based on weather state. The Niagara systems need to be created in the Unreal Editor following the specifications in this guide.
