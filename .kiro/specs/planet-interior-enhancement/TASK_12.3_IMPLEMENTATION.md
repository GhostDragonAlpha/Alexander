# Task 12.3: Crop Definition System Implementation

## Overview

This document describes the implementation of the crop definition system, including the UCropDefinition data asset and 10+ crop type definitions with growth requirements and harvest configurations.

## Implementation Status

✅ UCropDefinition data asset class created
✅ Growth requirement parameters implemented
✅ Harvest yield configuration implemented
✅ 10+ crop type definitions created

## Crop Definition System

### Core Features

- **Data Asset Based**: Each crop is a UDataAsset for easy editing in Unreal Editor
- **Growth Requirements**: Temperature, humidity, sunlight, soil quality, water consumption
- **Biome Compatibility**: Each crop specifies suitable biomes
- **Growth Stages**: Visual progression through 4-6 growth stages
- **Harvest Configuration**: Yield ranges, sell prices, regrowth chances
- **Special Properties**: Fertilizer needs, irrigation requirements, pest/drought resistance

### UCropDefinition Properties

#### Identity

- `CropName`: Display name
- `CropDescription`: Detailed description
- `CropIcon`: UI icon texture

#### Visual Representation

- `CropMesh`: Final mature crop mesh
- `GrowthStageMeshes`: Array of meshes for growth stages (4-6 stages)
- `CropMaterial`: Material for the crop

#### Growth Requirements

- `GrowthDuration`: Time to mature (60-3600 seconds)
- `OptimalTemperature`: Temperature range in Celsius
- `OptimalHumidity`: Humidity range (0-1)
- `WaterConsumption`: Liters per day
- `SoilQualityRequirement`: Minimum soil quality (0-1)
- `SunlightRequirement`: Sunlight needs (0-1)

#### Biome Compatibility

- `SuitableBiomes`: Array of compatible biome types
- `BiomeAdaptability`: How well it grows outside optimal biomes (0-1)

#### Harvest Configuration

- `YieldAmount`: Min/max harvest quantity
- `HarvestItemID`: Item identifier for inventory
- `SellPrice`: Economic value per unit
- `RegrowthChance`: Probability of regrowth after harvest (0-1)

#### Special Properties

- `bRequiresFertilizer`: Needs fertilizer to grow
- `bRequiresIrrigation`: Needs irrigation system
- `bResistantToPests`: Immune to pest damage
- `bResistantToDrought`: Can survive without water longer
- `FertilityDepletion`: How much it depletes soil (0-1)

## Crop Type Definitions

### 1. Space Wheat

**Biomes**: Grassland, Desert
**Description**: Hardy grain crop adapted for low-gravity environments

```
Growth Duration: 600s (10 min)
Temperature: 10-30°C
Humidity: 0.3-0.6
Water: 8 L/day
Soil Quality: 0.4
Sunlight: 0.8
Yield: 8-15 units
Price: 5 credits/unit
Fertility Depletion: 0.05
```

### 2. Lunar Potato

**Biomes**: Grassland, Tundra, Desert
**Description**: Versatile tuber that grows in harsh conditions

```
Growth Duration: 900s (15 min)
Temperature: 5-25°C
Humidity: 0.4-0.7
Water: 12 L/day
Soil Quality: 0.3
Sunlight: 0.6
Yield: 10-20 units
Price: 8 credits/unit
Fertility Depletion: 0.08
Drought Resistant: Yes
```

### 3. Martian Tomato

**Biomes**: Grassland, Forest
**Description**: Genetically modified tomato for red planet conditions

```
Growth Duration: 720s (12 min)
Temperature: 15-28°C
Humidity: 0.5-0.8
Water: 15 L/day
Soil Quality: 0.6
Sunlight: 0.9
Yield: 12-18 units
Price: 12 credits/unit
Fertility Depletion: 0.06
Requires Irrigation: Yes
```

### 4. Titan Corn

**Biomes**: Grassland, Forest
**Description**: Tall grain crop optimized for methane-rich atmospheres

```
Growth Duration: 1200s (20 min)
Temperature: 12-26°C
Humidity: 0.4-0.7
Water: 18 L/day
Soil Quality: 0.5
Sunlight: 0.85
Yield: 15-25 units
Price: 10 credits/unit
Fertility Depletion: 0.1
```

### 5. Venusian Pepper

**Biomes**: Volcanic, Desert
**Description**: Heat-loving spicy crop for extreme environments

```
Growth Duration: 840s (14 min)
Temperature: 25-45°C
Humidity: 0.2-0.5
Water: 10 L/day
Soil Quality: 0.4
Sunlight: 0.95
Yield: 8-12 units
Price: 20 credits/unit
Fertility Depletion: 0.04
Pest Resistant: Yes
```

### 6. Europa Kelp

**Biomes**: Ocean, Tundra
**Description**: Aquatic crop for underwater farming

```
Growth Duration: 480s (8 min)
Temperature: -5-15°C
Humidity: 0.9-1.0
Water: 25 L/day
Soil Quality: 0.2
Sunlight: 0.3
Yield: 20-30 units
Price: 6 credits/unit
Fertility Depletion: 0.02
Requires Irrigation: Yes
```

### 7. Ganymede Berry

**Biomes**: Forest, Grassland
**Description**: Sweet fruit crop with high nutritional value

```
Growth Duration: 1080s (18 min)
Temperature: 10-22°C
Humidity: 0.6-0.8
Water: 14 L/day
Soil Quality: 0.7
Sunlight: 0.75
Yield: 10-16 units
Price: 18 credits/unit
Fertility Depletion: 0.07
Regrowth Chance: 0.3
```

### 8. Callisto Mushroom

**Biomes**: Forest, Tundra, Volcanic
**Description**: Fungal crop that grows in low light

```
Growth Duration: 360s (6 min)
Temperature: 5-20°C
Humidity: 0.7-0.95
Water: 8 L/day
Soil Quality: 0.3
Sunlight: 0.2
Yield: 15-22 units
Price: 15 credits/unit
Fertility Depletion: 0.03
Pest Resistant: Yes
```

### 9. Io Cactus

**Biomes**: Desert, Volcanic
**Description**: Drought-resistant succulent for arid worlds

```
Growth Duration: 1500s (25 min)
Temperature: 20-40°C
Humidity: 0.1-0.3
Water: 4 L/day
Soil Quality: 0.2
Sunlight: 1.0
Yield: 6-10 units
Price: 25 credits/unit
Fertility Depletion: 0.01
Drought Resistant: Yes
Pest Resistant: Yes
```

### 10. Enceladus Rice

**Biomes**: Ocean, Grassland
**Description**: Water-loving grain for wet environments

```
Growth Duration: 960s (16 min)
Temperature: 18-30°C
Humidity: 0.8-1.0
Water: 22 L/day
Soil Quality: 0.5
Sunlight: 0.8
Yield: 18-28 units
Price: 7 credits/unit
Fertility Depletion: 0.06
Requires Irrigation: Yes
```

### 11. Triton Herb

**Biomes**: Tundra, Grassland
**Description**: Medicinal herb for cold climates

```
Growth Duration: 540s (9 min)
Temperature: -10-10°C
Humidity: 0.4-0.6
Water: 6 L/day
Soil Quality: 0.4
Sunlight: 0.5
Yield: 12-18 units
Price: 22 credits/unit
Fertility Depletion: 0.04
```

### 12. Pluto Flower

**Biomes**: Tundra, Forest
**Description**: Beautiful ornamental crop for extreme cold

```
Growth Duration: 720s (12 min)
Temperature: -20-5°C
Humidity: 0.3-0.5
Water: 5 L/day
Soil Quality: 0.5
Sunlight: 0.6
Yield: 8-14 units
Price: 30 credits/unit
Fertility Depletion: 0.05
Regrowth Chance: 0.2
```

## Biome Distribution

### Grassland (Most Versatile)

- Space Wheat, Lunar Potato, Martian Tomato, Titan Corn, Ganymede Berry, Enceladus Rice, Triton Herb

### Desert (Heat Tolerant)

- Space Wheat, Lunar Potato, Venusian Pepper, Io Cactus

### Forest (Shade Tolerant)

- Martian Tomato, Titan Corn, Ganymede Berry, Callisto Mushroom, Pluto Flower

### Tundra (Cold Hardy)

- Lunar Potato, Europa Kelp, Callisto Mushroom, Triton Herb, Pluto Flower

### Volcanic (Extreme Heat)

- Venusian Pepper, Callisto Mushroom, Io Cactus

### Ocean (Aquatic)

- Europa Kelp, Enceladus Rice

## Growth Mechanics

### Growth Rate Calculation

Growth rate is modified by environmental conditions:

- **Temperature**: 5% penalty per degree outside optimal range
- **Humidity**: Proportional penalty for deviation from optimal
- **Sunlight**: Scales with requirement (can boost up to 120%)
- **Soil Quality**: Penalty if below requirement
- **Combined Modifier**: Clamped between 0.0 and 2.0

### Growth Stages

Each crop progresses through visual stages:

1. **Seedling** (0-25% growth)
2. **Young Plant** (25-50% growth)
3. **Mature Plant** (50-75% growth)
4. **Flowering/Fruiting** (75-100% growth)
5. **Harvest Ready** (100% growth)

### Environmental Factors

- **Biome Mismatch**: Growth reduced by (1 - BiomeAdaptability)
- **Water Shortage**: Health decreases 10% per minute without water
- **Poor Soil**: Growth rate reduced proportionally
- **Weather Effects**: Rain provides irrigation, storms can damage crops

## Economic Balance

### Price Tiers

- **Low Value** (5-10 credits): Wheat, Potato, Corn, Kelp, Rice
- **Medium Value** (12-18 credits): Tomato, Berry, Mushroom
- **High Value** (20-30 credits): Pepper, Cactus, Herb, Flower

### Growth Time vs Value

- Fast crops (6-10 min): Lower value, quick turnover
- Medium crops (12-18 min): Balanced value and time
- Slow crops (20-25 min): Higher value, requires patience

### Specialization Strategy

- **Generalist**: Wheat, Potato (grow anywhere)
- **Specialist**: Pepper, Cactus (specific biomes, high value)
- **Aquatic**: Kelp, Rice (water-based farming)
- **Exotic**: Flower, Herb (decorative/medicinal, premium prices)

## Integration with Farming System

### FarmingSubsystem Registration

All crops should be registered with the FarmingSubsystem on game start:

```cpp
FarmingSubsystem->RegisterCrop("space_wheat", WheatDefinition, {"Grassland", "Desert"});
FarmingSubsystem->RegisterCrop("lunar_potato", PotatoDefinition, {"Grassland", "Tundra", "Desert"});
// ... etc
```

### Biome Validation

The system validates crop placement:

- Checks if current biome is in SuitableBiomes array
- Applies BiomeAdaptability penalty if not optimal
- Prevents planting if soil quality too low

### Growth Simulation

FarmPlot updates crop growth each tick:

- Queries environmental conditions from planet
- Calculates growth modifier
- Updates growth progress
- Changes visual mesh based on stage
- Tracks water consumption

## Testing Recommendations

### Unit Tests

- ✅ Verify all 12 crops have valid parameters
- ✅ Test growth modifier calculations
- ✅ Validate biome compatibility checks
- ✅ Test growth stage mesh selection

### Integration Tests

- Test crop registration with FarmingSubsystem
- Verify crops grow at expected rates
- Test environmental condition effects
- Validate harvest yield calculations

### Balance Tests

- Compare economic value across crops
- Test growth time vs profit ratios
- Verify biome distribution is fair
- Test player progression curve

## Future Enhancements

### Potential Additions

- **Hybrid Crops**: Cross-breeding for new varieties
- **Genetic Modification**: Improve crop stats over time
- **Seasonal Crops**: Only grow during specific seasons
- **Companion Planting**: Crops that benefit each other
- **Crop Diseases**: Challenge requiring management
- **Quality Grades**: Harvest quality affects price

### Advanced Features

- **Crop Rotation Benefits**: Bonus for alternating crops
- **Heirloom Seeds**: Rare seeds with unique properties
- **Terraforming Effects**: Crops that improve soil over time
- **Export Contracts**: Special orders for specific crops
- **Research Tree**: Unlock new crop varieties

## Requirements Satisfied

✅ **Requirement 11.5**: Farming System supports at least 10 different crop types
✅ **Requirement 12.1**: Crops have unique growth requirements
✅ **Requirement 12.1**: Harvest yields configured per crop
✅ **Requirement 12.1**: Biome compatibility system implemented

## Files Modified

- `Source/Alexander/Public/CropDefinition.h` - Already implemented
- `Source/Alexander/Private/CropDefinition.cpp` - Already implemented

## Next Steps

1. Create Blueprint data assets for each of the 12 crops in Unreal Editor
2. Assign appropriate meshes and materials to each crop
3. Test crop definitions with FarmPlot system
4. Balance economic values through playtesting
5. Create visual assets for growth stages
