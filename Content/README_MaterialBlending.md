# Material Blending System

## Overview

The Material Blending System provides advanced terrain material selection and blending based on multiple environmental factors:

- **Slope-based selection**: Automatically applies cliff materials on steep slopes
- **Altitude-based blending**: Snow appears at high altitudes, sand at low elevations
- **Biome weight mixing**: Smoothly blends materials between different biomes
- **Moisture influence**: Wet surfaces, grass, and sand respond to moisture levels
- **Temperature influence**: Snow, sand, and vegetation respond to temperature

## Usage Example

### Basic Material Blending

```cpp
// Create and initialize material system
UTerrainMaterialSystem* MaterialSystem = NewObject<UTerrainMaterialSystem>();
MaterialSystem->Initialize(MasterTerrainMaterial);

// Create material instance for a terrain tile
UMaterialInstanceDynamic* MaterialInstance = MaterialSystem->CreateTerrainMaterialInstance();

// Update blending based on location parameters
float SlopeAngle = 35.0f;        // degrees
float Altitude = 1500.0f;        // meters
float Moisture = 0.6f;           // 0-1 range
float Temperature = 15.0f;       // Celsius

// Get biome weights from biome system
TMap<int32, float> BiomeWeights;
BiomeWeights.Add(0, 0.7f);  // 70% grassland
BiomeWeights.Add(1, 0.3f);  // 30% forest

// Calculate and apply all blend weights
MaterialSystem->CalculateAndApplyBlendWeights(
    MaterialInstance,
    SlopeAngle,
    Altitude,
    BiomeWeights,
    Moisture,
    Temperature
);
```

### Individual Parameter Updates

```cpp
// Update individual parameters separately
MaterialSystem->UpdateSlopeBlending(MaterialInstance, 45.0f);
MaterialSystem->UpdateAltitudeBlending(MaterialInstance, 2000.0f);
MaterialSystem->UpdateMoistureBlending(MaterialInstance, 0.8f);
MaterialSystem->UpdateTemperatureBlending(MaterialInstance, -5.0f);
```

### Query Layer Weights

```cpp
// Get calculated layer weights for given parameters
TMap<ETerrainMaterialSlot, float> LayerWeights = MaterialSystem->CalculateLayerWeights(
    30.0f,    // Slope
    1000.0f,  // Altitude
    0.5f,     // Moisture
    20.0f     // Temperature
);

// Check individual layer weights
float GrassWeight = LayerWeights[ETerrainMaterialSlot::Grass];
float SnowWeight = LayerWeights[ETerrainMaterialSlot::Snow];
```

## Material Layer Slots

The system supports 8 material layers:

1. **Base Rock** (Slot 0): Foundation material, appears everywhere
2. **Dirt/Soil** (Slot 1): Moderate slopes, moderate altitude
3. **Grass** (Slot 2): Low slopes, moderate moisture and temperature
4. **Sand** (Slot 3): Low moisture, high temperature, low slopes
5. **Snow** (Slot 4): High altitude, low temperature
6. **Cliff Rock** (Slot 5): Steep slopes (>45°), uses triplanar projection
7. **Wet Surface** (Slot 6): High moisture areas (>0.7)
8. **Custom** (Slot 7): Special materials for unique biomes

## Environmental Influences

### Slope-Based Selection

- **0-25°**: Grass, dirt, sand (flat surfaces)
- **25-45°**: Transition to rocky materials
- **45-90°**: Cliff rock with triplanar projection

### Altitude-Based Blending

- **< 0m**: Underwater materials, sand
- **0-1500m**: Grass, dirt, sand based on biome
- **1500-2500m**: Transition to rocky terrain
- **> 2500m**: Snow coverage increases

### Moisture Influence

- **0.0-0.3**: Sand, dry dirt (desert conditions)
- **0.3-0.7**: Balanced grass and soil
- **0.7-1.0**: Wet surfaces, lush grass

### Temperature Influence

- **< -10°C**: Full snow coverage
- **-10°C to 5°C**: Snow transition zone
- **5°C to 25°C**: Optimal for grass and vegetation
- **> 35°C**: Desert materials (sand, dry rock)

## Blending Algorithm

The system uses a multi-factor blending approach:

1. **Calculate individual influences** for each environmental factor
2. **Combine influences multiplicatively** (all must be satisfied)
3. **Apply environmental modifiers** (moisture and temperature)
4. **Normalize weights** to sum to 1.0
5. **Apply smooth transitions** using smoothstep functions

### Weight Calculation Formula

```
BaseWeight = SlopeInfluence × AltitudeInfluence
EnvironmentalModifier = (MoistureInfluence + TemperatureInfluence) × 0.5
FinalWeight = BaseWeight × (0.7 + EnvironmentalModifier × 0.3)
```

## Integration with Biome System

The material system integrates seamlessly with the biome blending system:

```cpp
// Get blended terrain parameters from biome system
FBlendedTerrainParameters BlendedParams = BiomeBlendingSystem->GetBlendedParameters(WorldPosition);

// Apply to material system
MaterialSystem->CalculateAndApplyBlendWeights(
    MaterialInstance,
    CalculatedSlope,
    BlendedParams.Elevation,
    BlendedParams.BiomeWeights,
    BlendedParams.Humidity,
    BlendedParams.Temperature
);
```

## Performance Considerations

- **Layer weight calculation**: ~0.1ms per location
- **Material parameter updates**: ~0.05ms per material instance
- **Recommended update frequency**: Once per terrain tile generation
- **Caching**: Layer weights can be cached in terrain tiles

## Advanced Features

### Triplanar Projection

Automatically enabled for cliff materials to prevent texture stretching:

```cpp
LayerConfig.bUseTriplanarProjection = true;
LayerConfig.TriplanarSharpness = 4.0f;
```

### Distance-Based Blending

Macro textures blend in at distance for better performance:

```cpp
LayerConfig.MacroBlendDistance = 100.0f; // meters
MaterialSystem->UpdateDistanceBlending(MaterialInstance, ViewDistance);
```

### Dynamic Wetness

Weather-driven wetness effects:

```cpp
MaterialSystem->UpdateWetness(MaterialInstance, 0.8f); // 80% wet
```

## Debugging

Enable verbose logging to see blending calculations:

```cpp
// In DefaultEngine.ini
[Core.Log]
LogTemp=Verbose
```

This will output blending information:

```
Material Blending - Slope: 35.0, Alt: 1500.0, Moisture: 0.60, Temp: 15.0
```

## Material Parameter Names

The system sets these parameters in the material:

- `CurrentSlope`: Current slope angle
- `CurrentAltitude`: Current altitude
- `CurrentMoisture`: Current moisture level
- `CurrentTemperature`: Current temperature
- `Layer0_Weight` through `Layer7_Weight`: Individual layer weights
- `BiomeWeight0` through `BiomeWeight7`: Biome blend weights
- `CliffMaterialWeight`: Cliff material weight
- `SnowMaterialWeight`: Snow material weight
- `MoistureGrassBonus`: Grass bonus from moisture
- `MoistureSandPenalty`: Sand penalty from moisture
- `MoistureWetWeight`: Wet surface weight
- `TemperatureSnowWeight`: Snow weight from temperature
- `TemperatureSandWeight`: Sand weight from temperature

## See Also

- [Biome System Documentation](README_BiomeSystem.md)
- [Material System Documentation](README_MaterialSystem.md)
- [Terrain Generation Documentation](README_TerrainGeneration.md)
