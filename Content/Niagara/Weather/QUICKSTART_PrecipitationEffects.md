# Quick Start: Precipitation Effects

## Overview

Quick reference for creating weather precipitation Niagara systems.

## Required Systems

Create these 4 Niagara systems in `Content/Niagara/Weather/`:

### 1. NS_Rain

**Template**: Simple Sprite Burst

**Key Settings**:

- Spawn Rate: 500-1000 (parameter-driven)
- Velocity: (0, 0, -800) ± 50
- Size: 20-40 cm length, 1-2 cm width
- Color: (0.8, 0.9, 1.0), Alpha 0.3-0.6
- Lifetime: 2-4 seconds
- Spawn Volume: 2000x2000x1000 cm box above camera

**Parameters to Add**:

- `Intensity` (Float): 0.0-1.0
- `SpawnRate` (Float): 0-1000

### 2. NS_Snow

**Template**: Simple Sprite Burst

**Key Settings**:

- Spawn Rate: 400-800 (parameter-driven)
- Velocity: (0, 0, -200) ± 100 with turbulence
- Size: 3-8 cm diameter
- Color: (1.0, 1.0, 1.0), Alpha 0.7-0.9
- Lifetime: 5-8 seconds
- Rotation: Slow random
- Spawn Volume: 2500x2500x1500 cm box above camera

**Parameters to Add**:

- `Intensity` (Float): 0.0-1.0
- `SpawnRate` (Float): 0-1000

### 3. NS_Sandstorm

**Template**: Simple Sprite Burst

**Key Settings**:

- Spawn Rate: 1000-2000 (parameter-driven)
- Velocity: (500, 0, -50) ± 200 with strong turbulence
- Size: 5-15 cm diameter
- Color: (0.8, 0.6, 0.3), Alpha 0.2-0.5
- Lifetime: 3-6 seconds
- Spawn Volume: 5000x5000x2000 cm box around camera

**Parameters to Add**:

- `Intensity` (Float): 0.0-1.0
- `SpawnRate` (Float): 0-2000

### 4. NS_Fog

**Template**: Simple Sprite Burst

**Key Settings**:

- Spawn Rate: 200-500 (parameter-driven)
- Velocity: (20, 0, 0) ± 30 with gentle turbulence
- Size: 200-500 cm diameter (large!)
- Color: (0.9, 0.9, 0.9), Alpha 0.1-0.3
- Lifetime: 10-15 seconds
- Spawn Volume: 3000 cm radius sphere around camera

**Parameters to Add**:

- `Intensity` (Float): 0.0-1.0
- `SpawnRate` (Float): 0-500

## Creation Steps

### For Each System:

1. **Create System**:

   - Right-click in `Content/Niagara/Weather/`
   - Niagara System → New system from selected emitters
   - Choose template or create empty

2. **Add User Parameters**:

   - Open system
   - In User Parameters panel, click `+`
   - Add `Intensity` (Float, default 0.5)
   - Add `SpawnRate` (Float, default varies by type)

3. **Configure Emitter**:

   - Set Spawn Rate module to use `SpawnRate` parameter
   - Configure Initialize Particle modules:
     - Lifetime
     - Sprite Size
     - Color
     - Velocity
   - Add Update Particle modules:
     - Gravity/Acceleration
     - Drag (if needed)
     - Turbulence
     - Camera Offset (to follow player)

4. **Set Material**:

   - Use soft particle material
   - Enable depth fade
   - Set appropriate blend mode (Translucent)

5. **Test**:
   - Place in level
   - Adjust parameters
   - Check performance

## Assigning to Weather Component

1. Select Planet actor in level
2. Find `PlanetWeatherComponent` in Details
3. Under `Precipitation` category:
   - Rain Particle System → `NS_Rain`
   - Snow Particle System → `NS_Snow`
   - Sandstorm Particle System → `NS_Sandstorm`
   - Fog Particle System → `NS_Fog`

## Testing Commands

```cpp
// In Blueprint or C++
WeatherComponent->TransitionToWeather(EWeatherType::HeavyRain, 10.0f);
WeatherComponent->TransitionToWeather(EWeatherType::HeavySnow, 10.0f);
WeatherComponent->TransitionToWeather(EWeatherType::Sandstorm, 10.0f);
WeatherComponent->TransitionToWeather(EWeatherType::Fog, 10.0f);
```

## Performance Targets

- Rain: ~500-1000 particles
- Snow: ~400-800 particles
- Sandstorm: ~1000-2000 particles
- Fog: ~200-500 particles
- Total: < 5000 particles active
- Frame time: < 2ms for all weather effects
- VR: Maintain 90 FPS

## Common Issues

**Particles not visible**:

- Check spawn volume includes camera
- Verify alpha values aren't too low
- Check material blend mode

**Performance problems**:

- Reduce spawn rate
- Decrease lifetime
- Enable distance culling
- Simplify materials

**Particles don't follow camera**:

- Add Camera Offset module
- Set to Local Space or use Camera Offset Location

## Next Steps

After creating systems:

1. Test each weather type
2. Adjust intensities for visual quality
3. Optimize for VR performance
4. Add audio effects (separate task)
5. Integrate with farming system (crops affected by weather)

## Reference

See `PRECIPITATION_EFFECTS_GUIDE.md` for detailed specifications.
