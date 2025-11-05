# Biome Creation Workflow

## Quick Reference Guide

This guide provides a step-by-step workflow for creating custom biomes for the planet system.

## Prerequisites

- Planet configuration asset created
- Basic understanding of Unreal Engine materials
- Access to terrain textures and vegetation meshes

## Workflow Steps

### 1. Plan Your Biome

Before creating assets, define your biome characteristics:

```
Biome Name: _________________
Climate Type: _________________
Temperature Range: _____ to _____ °C
Humidity Range: _____ to _____
Primary Colors: _________________
Key Features: _________________
```

### 2. Create Material Layers

#### Required Textures per Layer:

- Base Color (Albedo) - 2048x2048
- Normal Map - 2048x2048
- Roughness Map - 1024x1024
- Metallic Map - 1024x1024 (if needed)
- Ambient Occlusion - 1024x1024

#### Material Layer Setup:

```cpp
// Layer 0: Base Ground
Material: M_[BiomeName]_Ground
Blend Mode: Opaque
Blend Weight: 1.0
Altitude: 0-10000m
Slope: 0-90 degrees

// Layer 1: Surface Detail
Material: M_[BiomeName]_Surface
Blend Mode: Blend
Blend Weight: 0.8
Altitude: 0-5000m
Slope: 0-45 degrees

// Layer 2: Cliff/Rock
Material: M_Rock_Generic
Blend Mode: Blend
Blend Weight: 1.0
Altitude: Any
Slope: 45-90 degrees (steep)
```

### 3. Configure Vegetation

#### Vegetation Asset Checklist:

- [ ] LOD0 mesh (< 5000 triangles)
- [ ] LOD1 mesh (< 2000 triangles)
- [ ] LOD2 mesh (< 500 triangles)
- [ ] Collision mesh (simple shapes)
- [ ] Wind animation support
- [ ] Material with LOD

#### Vegetation Placement Rules:

```cpp
// Trees
Density: 0.1 - 0.4 (10-40% coverage)
Min Scale: 0.8
Max Scale: 1.3
Altitude: 0 - 2000m
Slope: 0 - 30 degrees
Spacing: 5 - 15 meters

// Bushes/Shrubs
Density: 0.3 - 0.6
Min Scale: 0.5
Max Scale: 1.5
Altitude: 0 - 3000m
Slope: 0 - 40 degrees
Spacing: 2 - 8 meters

// Grass/Ground Cover
Density: 0.7 - 0.95
Min Scale: 0.8
Max Scale: 1.2
Altitude: 0 - 4000m
Slope: 0 - 50 degrees
Spacing: 0.5 - 2 meters
```

### 4. Set Environmental Parameters

```cpp
// Climate Configuration
Temperature: _____ °C (average)
Temperature Variance: ±_____ °C (seasonal)
Humidity: _____ (0.0 - 1.0)
Humidity Variance: ±_____ (weather dependent)

// Weather Types (check applicable)
[ ] Clear
[ ] Partly Cloudy
[ ] Overcast
[ ] Light Rain
[ ] Heavy Rain
[ ] Thunderstorm
[ ] Light Snow
[ ] Heavy Snow
[ ] Sandstorm
[ ] Fog
[ ] Custom: _________________

// Weather Frequency
Clear: _____%
Cloudy: _____%
Precipitation: _____%
Severe: _____%
```

### 5. Define Distribution Rules

```cpp
// Geographic Distribution
Latitude Range: _____ to _____ degrees
  (0 = equator, 90 = poles)

Altitude Range: _____ to _____ meters
  (relative to sea level)

Moisture Requirement: _____ to _____
  (0.0 = arid, 1.0 = saturated)

Temperature Requirement: _____ to _____ °C

// Biome Priority
Priority: _____ (1-10, higher = more dominant)

// Transition Settings
Transition Width: _____ meters (50-200 recommended)
Blend Smoothness: _____ (0.0-1.0)
```

### 6. Create Biome Data Asset

#### In Unreal Editor:

1. **Content Browser** → Right-click → **Miscellaneous** → **Data Asset**
2. Select `FBiomeDefinition` as class
3. Name: `DA_Biome_[YourBiomeName]`
4. Open and configure all properties

#### Configuration Checklist:

- [ ] Biome name set
- [ ] Biome color assigned (for debug)
- [ ] Base elevation configured
- [ ] Elevation variance set
- [ ] Roughness value set (0.3-0.8)
- [ ] All material layers added
- [ ] Vegetation types configured
- [ ] Vegetation density set
- [ ] Temperature range set
- [ ] Humidity range set
- [ ] Weather types selected
- [ ] Distribution rules configured

### 7. Add to Planet Configuration

```cpp
// In your planet configuration asset
Biomes Array:
  [0] DA_Biome_Ocean
  [1] DA_Biome_Desert
  [2] DA_Biome_Grassland
  [3] DA_Biome_Forest
  [4] DA_Biome_[YourNewBiome]  // Add here
  [5] DA_Biome_Tundra
  [6] DA_Biome_Mountain
```

### 8. Test Your Biome

#### Testing Checklist:

- [ ] Biome appears on planet surface
- [ ] Materials render correctly
- [ ] Vegetation spawns at correct density
- [ ] Transitions to adjacent biomes are smooth
- [ ] Weather effects work properly
- [ ] Performance is acceptable (90+ FPS in VR)
- [ ] Lighting looks correct at different times of day
- [ ] No visual artifacts or z-fighting

#### Testing Commands:

```cpp
// In-game console (` key)

// Teleport to biome
TeleportToBiome [BiomeName]

// Show biome debug info
ShowDebug Biome

// Toggle biome visualization
r.ShowBiomeColors 1

// Check performance
stat fps
stat unit
stat gpu

// Force specific weather
SetWeather [WeatherType]
```

### 9. Optimize Performance

#### Performance Targets:

```cpp
// VR Performance Requirements
Frame Time: < 11.1ms (90 FPS)
Draw Calls: < 500 per biome
Visible Triangles: < 1 million per biome
Texture Memory: < 500MB per biome
```

#### Optimization Checklist:

- [ ] Vegetation uses instanced static meshes
- [ ] LOD distances configured properly
- [ ] Textures compressed (BC1/BC3/BC5)
- [ ] Materials use instances, not unique materials
- [ ] Collision simplified for vegetation
- [ ] Culling distance set appropriately
- [ ] No overlapping vegetation instances

### 10. Document Your Biome

Create a biome documentation file:

```markdown
# [Biome Name] Biome

## Overview

[Brief description of biome appearance and characteristics]

## Configuration

- Temperature: [range]
- Humidity: [range]
- Elevation: [range]
- Latitude: [range]

## Visual Features

- Primary colors: [colors]
- Key vegetation: [list]
- Terrain characteristics: [description]

## Weather Patterns

- Common: [weather types]
- Rare: [weather types]

## Performance Notes

- Draw calls: [number]
- Vegetation instances: [number]
- Memory usage: [MB]

## Known Issues

[Any issues or limitations]
```

## Example Biomes

### Temperate Forest Biome

```cpp
Biome Name: "Temperate Forest"
Temperature: 15°C (range: 5-25°C)
Humidity: 0.6 (range: 0.4-0.8)
Elevation: 100m (variance: 500m)
Roughness: 0.5

Materials:
- Rich soil base
- Grass and moss surface
- Granite cliffs

Vegetation:
- Oak trees (density: 0.3)
- Bushes (density: 0.4)
- Grass (density: 0.9)

Weather:
- Clear: 40%
- Cloudy: 30%
- Rain: 25%
- Fog: 5%
```

### Desert Biome

```cpp
Biome Name: "Arid Desert"
Temperature: 35°C (range: 20-50°C)
Humidity: 0.15 (range: 0.05-0.3)
Elevation: 200m (variance: 300m)
Roughness: 0.4

Materials:
- Sand base
- Rock outcrops
- Dry clay

Vegetation:
- Cacti (density: 0.05)
- Dry shrubs (density: 0.1)
- Sparse grass (density: 0.2)

Weather:
- Clear: 70%
- Sandstorm: 20%
- Cloudy: 10%
```

### Arctic Tundra Biome

```cpp
Biome Name: "Arctic Tundra"
Temperature: -15°C (range: -40-5°C)
Humidity: 0.3 (range: 0.2-0.5)
Elevation: 50m (variance: 200m)
Roughness: 0.3

Materials:
- Snow and ice base
- Frozen soil
- Ice formations

Vegetation:
- Moss (density: 0.3)
- Small shrubs (density: 0.1)
- Lichen (density: 0.4)

Weather:
- Clear cold: 40%
- Snow: 35%
- Blizzard: 15%
- Fog: 10%
```

## Troubleshooting

### Biome Not Appearing

1. Check distribution rules match planet location
2. Verify biome is added to planet configuration
3. Check biome priority vs other biomes
4. Ensure latitude/altitude ranges are correct

### Poor Performance

1. Reduce vegetation density
2. Increase LOD distances
3. Simplify materials
4. Use texture compression
5. Enable instancing

### Harsh Transitions

1. Increase transition width (100-200m)
2. Add noise to blend calculation
3. Ensure adjacent biomes have compatible colors
4. Check material blending settings

### Vegetation Not Spawning

1. Verify vegetation meshes are valid
2. Check density values (> 0)
3. Ensure altitude/slope ranges are correct
4. Check for collision blocking spawns

---

**Version:** 1.0  
**Last Updated:** 2025-11-04
