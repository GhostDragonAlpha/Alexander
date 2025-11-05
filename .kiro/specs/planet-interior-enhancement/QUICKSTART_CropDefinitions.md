# Quick Start: Crop Definition System

## Overview

The crop definition system provides 12+ unique crop types with realistic growth requirements, biome compatibility, and harvest configurations for the planetary farming system.

## Available Crops

### Quick Reference Table

| Crop              | Biomes                    | Growth Time | Value     | Special Traits               |
| ----------------- | ------------------------- | ----------- | --------- | ---------------------------- |
| Space Wheat       | Grassland, Desert         | 10 min      | Low       | Hardy, versatile             |
| Lunar Potato      | Grassland, Tundra, Desert | 15 min      | Low       | Drought resistant            |
| Martian Tomato    | Grassland, Forest         | 12 min      | Medium    | Needs irrigation             |
| Titan Corn        | Grassland, Forest         | 20 min      | Medium    | High yield                   |
| Venusian Pepper   | Volcanic, Desert          | 14 min      | High      | Pest resistant, heat-loving  |
| Europa Kelp       | Ocean, Tundra             | 8 min       | Low       | Aquatic, fast growing        |
| Ganymede Berry    | Forest, Grassland         | 18 min      | High      | Regrows after harvest        |
| Callisto Mushroom | Forest, Tundra, Volcanic  | 6 min       | Medium    | Low light, pest resistant    |
| Io Cactus         | Desert, Volcanic          | 25 min      | Very High | Drought/pest resistant       |
| Enceladus Rice    | Ocean, Grassland          | 16 min      | Low       | Needs irrigation, high yield |
| Triton Herb       | Tundra, Grassland         | 9 min       | High      | Medicinal, cold hardy        |
| Pluto Flower      | Tundra, Forest            | 12 min      | Very High | Ornamental, regrows          |

## Using Crop Definitions

### In C++

#### Create Crops Programmatically

```cpp
#include "CropFactory.h"

// Create a single crop
UCropDefinition* Wheat = UCropFactory::CreateSpaceWheat();

// Create all standard crops
TArray<UCropDefinition*> AllCrops = UCropFactory::CreateAllStandardCrops();
```

#### Register with Farming Subsystem

```cpp
#include "FarmingSubsystem.h"

UFarmingSubsystem* FarmingSystem = GetWorld()->GetSubsystem<UFarmingSubsystem>();

// Register individual crop
UCropDefinition* Wheat = UCropFactory::CreateSpaceWheat();
FarmingSystem->RegisterCrop("space_wheat", Wheat, {"Grassland", "Desert"});

// Register all crops
TArray<UCropDefinition*> Crops = UCropFactory::CreateAllStandardCrops();
for (UCropDefinition* Crop : Crops)
{
    TArray<FString> BiomeNames;
    for (EBiomeType Biome : Crop->SuitableBiomes)
    {
        BiomeNames.Add(UEnum::GetValueAsString(Biome));
    }
    FarmingSystem->RegisterCrop(Crop->HarvestItemID, Crop, BiomeNames);
}
```

#### Check Crop Suitability

```cpp
UCropDefinition* Crop = FarmingSystem->GetCropDefinition("space_wheat");

// Check if suitable for biome
bool bSuitable = Crop->IsSuitableForBiome(EBiomeType::Grassland);

// Calculate growth modifier
float Modifier = Crop->CalculateGrowthModifier(
    20.0f,  // Temperature (°C)
    0.5f,   // Humidity (0-1)
    0.8f,   // Sunlight (0-1)
    0.6f    // Soil Quality (0-1)
);

// Get growth stage mesh
UStaticMesh* Mesh = Crop->GetGrowthStageMesh(0.75f); // 75% grown
```

### In Blueprint

#### Get Crop Definition

```
Get Farming Subsystem → Get Crop Definition (CropID: "space_wheat") → Crop Definition
```

#### Check Biome Suitability

```
Crop Definition → Is Suitable For Biome (BiomeType: Grassland) → Boolean
```

#### Calculate Growth Rate

```
Crop Definition → Calculate Growth Modifier
  - Temperature: 20.0
  - Humidity: 0.5
  - Sunlight: 0.8
  - Soil Quality: 0.6
  → Float (Growth Modifier)
```

## Creating Custom Crops

### Method 1: Using CropFactory as Template

```cpp
UCropDefinition* UCropFactory::CreateCustomCrop()
{
    TArray<EBiomeType> Biomes = { EBiomeType::Grassland };

    UCropDefinition* Crop = CreateCropBase(
        TEXT("Custom Crop"),           // Name
        TEXT("Description here"),      // Description
        600.0f,                        // Growth duration (seconds)
        15.0f, 25.0f,                 // Temperature range (°C)
        0.4f, 0.7f,                   // Humidity range (0-1)
        10.0f,                        // Water consumption (L/day)
        0.5f,                         // Soil quality requirement (0-1)
        0.7f,                         // Sunlight requirement (0-1)
        Biomes,                       // Suitable biomes
        10, 20,                       // Yield range
        15.0f,                        // Sell price
        TEXT("custom_crop")           // Item ID
    );

    // Optional: Set special properties
    Crop->bRequiresFertilizer = true;
    Crop->bResistantToDrought = false;
    Crop->FertilityDepletion = 0.06f;
    Crop->BiomeAdaptability = 0.5f;
    Crop->RegrowthChance = 0.2f;

    return Crop;
}
```

### Method 2: Create Data Asset in Editor

1. **Content Browser** → Right-click → **Miscellaneous** → **Data Asset**
2. Select **CropDefinition** as the class
3. Name it (e.g., `DA_CustomCrop`)
4. Open and configure properties:

#### Identity Section

- **Crop Name**: Display name
- **Crop Description**: Detailed description
- **Crop Icon**: UI icon texture

#### Visual Section

- **Crop Mesh**: Final mature mesh
- **Growth Stage Meshes**: Array of 4-6 meshes for growth stages
- **Crop Material**: Material instance

#### Growth Section

- **Growth Duration**: 60-3600 seconds (1-60 minutes)
- **Optimal Temperature**: Min/Max in Celsius
- **Optimal Humidity**: Min/Max (0-1)
- **Water Consumption**: Liters per day
- **Soil Quality Requirement**: 0-1
- **Sunlight Requirement**: 0-1

#### Biome Section

- **Suitable Biomes**: Array of compatible biome types
- **Biome Adaptability**: 0-1 (how well it grows outside optimal biomes)

#### Harvest Section

- **Yield Amount**: Min/Max harvest quantity
- **Harvest Item ID**: Unique identifier
- **Sell Price**: Economic value per unit
- **Regrowth Chance**: 0-1 probability of regrowth

#### Special Section

- **Requires Fertilizer**: Boolean
- **Requires Irrigation**: Boolean
- **Resistant To Pests**: Boolean
- **Resistant To Drought**: Boolean
- **Fertility Depletion**: 0-1 (soil depletion per harvest)

## Crop Selection Strategy

### For Different Biomes

#### Grassland (Most Options)

- **Starter**: Space Wheat (fast, reliable)
- **Mid-game**: Titan Corn (high yield)
- **Advanced**: Ganymede Berry (regrows, valuable)

#### Desert

- **Starter**: Space Wheat (hardy)
- **Mid-game**: Lunar Potato (drought resistant)
- **Advanced**: Io Cactus (very valuable, minimal water)

#### Forest

- **Starter**: Callisto Mushroom (fast, low light)
- **Mid-game**: Martian Tomato (good value)
- **Advanced**: Ganymede Berry (regrows)

#### Tundra (Cold Climate)

- **Starter**: Triton Herb (fast, valuable)
- **Mid-game**: Lunar Potato (hardy)
- **Advanced**: Pluto Flower (very valuable)

#### Volcanic (Extreme Heat)

- **Starter**: Callisto Mushroom (adaptable)
- **Mid-game**: Venusian Pepper (heat-loving)
- **Advanced**: Io Cactus (extreme conditions)

#### Ocean (Aquatic)

- **Starter**: Europa Kelp (fast, high yield)
- **Mid-game**: Enceladus Rice (good yield)
- **Advanced**: Combine with land crops

### Economic Strategies

#### Fast Turnover (6-10 minutes)

- Callisto Mushroom (6 min, 15 credits/unit)
- Europa Kelp (8 min, 6 credits/unit)
- Triton Herb (9 min, 22 credits/unit)
- Space Wheat (10 min, 5 credits/unit)

#### Balanced (12-18 minutes)

- Martian Tomato (12 min, 12 credits/unit)
- Pluto Flower (12 min, 30 credits/unit)
- Venusian Pepper (14 min, 20 credits/unit)
- Enceladus Rice (16 min, 7 credits/unit)
- Ganymede Berry (18 min, 18 credits/unit + regrows)

#### High Value (20-25 minutes)

- Titan Corn (20 min, 10 credits/unit, high yield)
- Io Cactus (25 min, 25 credits/unit)

#### Regrowable (Multiple Harvests)

- Ganymede Berry (30% regrowth chance)
- Pluto Flower (20% regrowth chance)

## Growth Mechanics

### Environmental Factors

#### Temperature

- **Optimal**: Full growth rate
- **Outside Range**: -5% per degree deviation
- **Example**: Wheat optimal 10-30°C, at 35°C = 75% growth rate

#### Humidity

- **Optimal**: Full growth rate
- **Outside Range**: Proportional penalty
- **Example**: Tomato optimal 0.5-0.8, at 0.3 = ~60% growth rate

#### Sunlight

- **Below Requirement**: Proportional penalty
- **Above Requirement**: Bonus up to 120%
- **Example**: Mushroom needs 0.2, at 0.4 = 120% growth rate

#### Soil Quality

- **Below Requirement**: Proportional penalty (min 30%)
- **Above Requirement**: No bonus
- **Example**: Berry needs 0.7, at 0.5 = ~71% growth rate

### Growth Stages

Each crop progresses through visual stages:

1. **Seedling** (0-25%): Small sprout
2. **Young** (25-50%): Growing plant
3. **Mature** (50-75%): Full-size plant
4. **Flowering** (75-100%): Flowers/fruit forming
5. **Harvest Ready** (100%): Ready to harvest

### Water Management

- Each crop consumes water per day
- Without water for 5 minutes: Health decreases 10%/min
- Rain provides natural irrigation
- Irrigation systems automate watering

### Soil Depletion

- Each harvest depletes soil fertility
- Depletion rate varies by crop (0.01-0.1)
- Fertilizer restores fertility
- Crop rotation helps maintain soil health

## Testing Crops

### Unit Test Example

```cpp
// Test crop creation
UCropDefinition* Wheat = UCropFactory::CreateSpaceWheat();
check(Wheat != nullptr);
check(Wheat->CropName == TEXT("Space Wheat"));
check(Wheat->GrowthDuration == 600.0f);

// Test biome suitability
check(Wheat->IsSuitableForBiome(EBiomeType::Grassland));
check(Wheat->IsSuitableForBiome(EBiomeType::Desert));
check(!Wheat->IsSuitableForBiome(EBiomeType::Ocean));

// Test growth modifier
float Modifier = Wheat->CalculateGrowthModifier(20.0f, 0.5f, 0.8f, 0.6f);
check(Modifier > 0.8f && Modifier < 1.2f); // Should be near optimal
```

### Integration Test

```cpp
// Register crops with farming system
UFarmingSubsystem* Farming = GetWorld()->GetSubsystem<UFarmingSubsystem>();
TArray<UCropDefinition*> Crops = UCropFactory::CreateAllStandardCrops();

for (UCropDefinition* Crop : Crops)
{
    TArray<FString> Biomes;
    for (EBiomeType Biome : Crop->SuitableBiomes)
    {
        Biomes.Add(UEnum::GetValueAsString(Biome));
    }
    Farming->RegisterCrop(Crop->HarvestItemID, Crop, Biomes);
}

// Verify registration
UCropDefinition* Retrieved = Farming->GetCropDefinition("space_wheat");
check(Retrieved != nullptr);
check(Retrieved->CropName == TEXT("Space Wheat"));
```

## Troubleshooting

### Crop Not Growing

- Check environmental conditions match requirements
- Verify soil quality meets minimum
- Ensure water is available
- Check biome compatibility

### Low Yield

- Improve soil quality with fertilizer
- Optimize temperature/humidity
- Ensure adequate sunlight
- Check for pest damage (if not resistant)

### Crop Dying

- Water shortage (most common)
- Temperature too extreme
- Soil quality too low
- Storm damage

## Next Steps

1. **Create Visual Assets**: Design meshes for each crop and growth stages
2. **Balance Testing**: Playtest economic values and growth times
3. **Integration**: Connect with FarmPlot system for planting/harvesting
4. **UI Development**: Create crop selection and information displays
5. **Advanced Features**: Add crop diseases, quality grades, breeding

## Related Systems

- **FarmPlot**: Manages individual farm plots and crop growth
- **FarmingSubsystem**: Global farming management and crop database
- **BiomeManager**: Provides environmental conditions for growth
- **WeatherComponent**: Affects irrigation and crop health
- **Planet**: Provides location-based biome and soil data

## References

- `Source/Alexander/Public/CropDefinition.h` - Crop data asset class
- `Source/Alexander/Private/CropDefinition.cpp` - Crop implementation
- `Source/Alexander/Public/CropFactory.h` - Crop creation factory
- `Source/Alexander/Private/CropFactory.cpp` - Factory implementation
- `.kiro/specs/planet-interior-enhancement/TASK_12.3_IMPLEMENTATION.md` - Detailed documentation
