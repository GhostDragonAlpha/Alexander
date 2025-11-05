# Quick Start: Crop Definition System

## Overview

The Crop Definition System provides a comprehensive library of 13+ crop types with unique growth requirements, biome compatibility, and harvest yields. Each crop is defined as a `UCropDefinition` data asset with configurable parameters.

## Available Crops

### Staple Grains

1. **Wheat** - Universal grain (15 min, 12 credits)
2. **Corn** - High-yield warm climate (20 min, 15 credits)
3. **Rice** - Water-intensive wetland crop (25 min, 18 credits)
4. **Barley** - Cold-resistant grain (14 min, 13 credits)

### Vegetables

5. **Tomatoes** - Quick perennial (10 min, 8 credits, 30% regrowth)
6. **Potatoes** - Hardy root vegetable (13 min, 10 credits)
7. **Carrots** - Fast-growing root (8 min, 6 credits)
8. **Turnips** - Frost-tolerant (7 min, 7 credits)

### Legumes

9. **Soybeans** - Soil-improving protein crop (17 min, 20 credits)
10. **Beans** - Quick legume (9 min, 14 credits, 40% regrowth)

### Specialty Crops

11. **Cotton** - Fiber for textiles (30 min, 35 credits)
12. **Coffee** - High-value perennial (40 min, 60 credits, 80% regrowth)
13. **Cacao** - Luxury chocolate crop (45 min, 80 credits, 90% regrowth)

## Using the Factory in C++

### Create All Crops

```cpp
#include "CropDefinitionFactory.h"

// Create all standard crops
TArray<UCropDefinition*> AllCrops = UCropDefinitionFactory::CreateAllCropDefinitions(this);

// Iterate through crops
for (UCropDefinition* Crop : AllCrops)
{
    UE_LOG(LogTemp, Log, TEXT("Crop: %s - Price: %.2f"), *Crop->CropName, Crop->SellPrice);
}
```

### Create Specific Crop

```cpp
// Create a specific crop by name
UCropDefinition* Wheat = UCropDefinitionFactory::CreateCropByName(TEXT("Wheat"), this);

// Or use direct function
UCropDefinition* Tomatoes = UCropDefinitionFactory::CreateTomatoes(this);
```

### Check Biome Compatibility

```cpp
UCropDefinition* Crop = UCropDefinitionFactory::CreateWheat(this);

// Check if suitable for a biome
bool bSuitable = Crop->IsSuitableForBiome(EBiomeType::Grassland);

// Get all suitable biomes
for (EBiomeType Biome : Crop->SuitableBiomes)
{
    // Process each suitable biome
}
```

### Calculate Growth Modifiers

```cpp
UCropDefinition* Crop = UCropDefinitionFactory::CreateCorn(this);

// Environmental conditions
float Temperature = 25.0f; // Celsius
float Humidity = 0.6f; // 0-1
float Sunlight = 0.9f; // 0-1
float SoilQuality = 0.7f; // 0-1

// Calculate growth rate modifier
float GrowthModifier = Crop->CalculateGrowthModifier(Temperature, Humidity, Sunlight, SoilQuality);

// Apply to growth rate
float ActualGrowthRate = BaseGrowthRate * GrowthModifier;
```

### Get Growth Stage Information

```cpp
UCropDefinition* Crop = UCropDefinitionFactory::CreateTomatoes(this);

// Get current growth stage
float GrowthProgress = 0.65f; // 65% grown
int32 StageIndex = Crop->GetGrowthStageIndex(GrowthProgress);
FString StageName = Crop->GetGrowthStageName(GrowthProgress);
UStaticMesh* StageMesh = Crop->GetGrowthStageMesh(GrowthProgress);

UE_LOG(LogTemp, Log, TEXT("Stage %d: %s"), StageIndex, *StageName);
```

## Using in Blueprints

### Create Crop Library

1. Create a Blueprint Actor or Component
2. Add a variable: `TArray<UCropDefinition*> CropLibrary`
3. In BeginPlay or Construction Script:

```
// Call UCropDefinitionFactory::CreateAllCropDefinitions
// Store result in CropLibrary array
```

### Access Crop Properties

```
// Get crop from library
Crop = CropLibrary[0]

// Read properties
CropName = Crop.CropName
GrowthDuration = Crop.GrowthDuration
SellPrice = Crop.SellPrice
```

### Check Compatibility

```
// Check if crop works in current biome
IsSuitable = Crop.IsSuitableForBiome(CurrentBiome)

// Calculate growth modifier
GrowthModifier = Crop.CalculateGrowthModifier(Temp, Humidity, Sunlight, SoilQuality)
```

## Crop Properties Reference

### Identity

- `CropName` - Display name
- `CropDescription` - Detailed description
- `CropIcon` - UI icon texture
- `HarvestItemID` - Item identifier for inventory

### Visual

- `CropMesh` - Mature crop mesh
- `GrowthStageMeshes` - Array of meshes for growth stages
- `CropMaterial` - Material override

### Growth Requirements

- `GrowthDuration` - Time to maturity (seconds)
- `OptimalTemperature` - Temperature range (°C)
- `OptimalHumidity` - Humidity range (0-1)
- `WaterConsumption` - Liters per day
- `SoilQualityRequirement` - Minimum soil quality (0-1)
- `SunlightRequirement` - Sunlight needs (0-1)

### Biome

- `SuitableBiomes` - Array of compatible biomes
- `BiomeAdaptability` - Growth outside optimal biomes (0-1)

### Harvest

- `YieldAmount` - Min/max harvest units
- `SellPrice` - Credits per unit
- `RegrowthChance` - Probability of regrowth (0-1)

### Special Properties

- `bRequiresFertilizer` - Needs fertilizer
- `bRequiresIrrigation` - Needs irrigation system
- `bResistantToPests` - Pest resistance
- `bResistantToDrought` - Drought tolerance
- `FertilityDepletion` - Soil depletion per harvest (negative = improvement)

## Crop Strategy Tips

### Quick Profit

- **Turnips** (7 min) - Fastest growth
- **Carrots** (8 min) - Fast and pest-resistant
- **Beans** (9 min) - Quick with regrowth

### Long-term Investment

- **Cacao** (45 min, 80 credits) - Highest value
- **Coffee** (40 min, 60 credits) - High value perennial
- **Cotton** (30 min, 35 credits) - Good mid-tier

### Soil Management

- **Soybeans** - Improves soil (-0.05 depletion)
- **Beans** - Slight soil improvement (-0.03)
- **Turnips** - Minimal depletion (0.04)

### Biome-Specific

**Grassland (Most Versatile):**

- Wheat, Corn, Tomatoes, Potatoes, Soybeans, Beans, Cotton, Barley, Turnips

**Forest (Shade Crops):**

- Coffee, Cacao, Tomatoes, Potatoes, Beans, Rice, Carrots, Turnips

**Desert (Heat-Tolerant):**

- Cotton, Wheat, Beans, Barley

**Tundra (Cold-Hardy):**

- Barley, Turnips, Potatoes, Wheat, Carrots

**Ocean/Coastal:**

- Rice (wetland specialist)

## Integration with Farming System

### In FarmPlot

```cpp
// Plant a crop
UCropDefinition* CropType = UCropDefinitionFactory::CreateWheat(this);
bool bSuccess = FarmPlot->PlantCrop(GridPosition, CropType);

// Update growth
float DeltaTime = GetWorld()->GetDeltaSeconds();
float EnvironmentalModifier = CropType->CalculateGrowthModifier(Temp, Humidity, Sun, Soil);
float GrowthIncrement = (DeltaTime / CropType->GrowthDuration) * EnvironmentalModifier;
CropCell.GrowthProgress += GrowthIncrement;
```

### In Farming Subsystem

```cpp
// Initialize crop database
void UFarmingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Create crop library
    AvailableCrops = UCropDefinitionFactory::CreateAllCropDefinitions(this);

    // Index by name for quick lookup
    for (UCropDefinition* Crop : AvailableCrops)
    {
        CropsByName.Add(Crop->CropName, Crop);
    }
}

// Get crop by name
UCropDefinition* UFarmingSubsystem::GetCropDefinition(const FString& CropName)
{
    UCropDefinition** Found = CropsByName.Find(CropName);
    return Found ? *Found : nullptr;
}
```

## Testing Crops

### Test Growth Calculation

```cpp
// Test crop in different conditions
UCropDefinition* Crop = UCropDefinitionFactory::CreateCorn(this);

// Optimal conditions
float OptimalModifier = Crop->CalculateGrowthModifier(25.0f, 0.6f, 0.9f, 0.7f);
// Should be close to 1.0

// Poor conditions
float PoorModifier = Crop->CalculateGrowthModifier(10.0f, 0.3f, 0.5f, 0.3f);
// Should be significantly less than 1.0
```

### Test Biome Compatibility

```cpp
UCropDefinition* Rice = UCropDefinitionFactory::CreateRice(this);

// Should be true
bool bOcean = Rice->IsSuitableForBiome(EBiomeType::Ocean);

// Should be false
bool bDesert = Rice->IsSuitableForBiome(EBiomeType::Desert);
```

## Performance Considerations

- Crop definitions are lightweight data objects
- Create once and reuse (don't recreate every frame)
- Store in subsystem or manager for global access
- Use pointers/references to avoid copying

## Future Enhancements

Potential additions to the crop system:

- Seasonal variations
- Crop diseases and pests
- Hybrid/GMO crops
- Companion planting bonuses
- Crop rotation benefits
- Weather-specific crops (monsoon rice, etc.)
- Alien/exotic space crops

## See Also

- `CropDefinition.h` - Core crop data structure
- `CropDefinitionFactory.h` - Factory for creating crops
- `CROP_DEFINITIONS_LIBRARY.md` - Complete crop specifications
- `FarmingSubsystem.h` - Farming system integration
- `FarmPlot.h` - Plot-level crop management
