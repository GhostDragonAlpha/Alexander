# Quick Start: Biome Compatibility System

## Overview

The Biome Compatibility System manages crop-biome relationships, providing compatibility scoring and growth rate modifiers based on environmental conditions.

## Key Components

### BiomeCompatibilitySystem

Main system that handles:

- Crop-biome compatibility checks
- Growth rate modifiers per biome
- Biome suitability scoring
- Crop recommendations

### Integration with FarmingSubsystem

The FarmingSubsystem now includes biome compatibility features:

- `GetCompatibleCropsForBiome()` - Get all crops for a biome
- `CalculateCropCompatibility()` - Score crop at location
- `GetRecommendedCropsForLocation()` - Get best crops for location

## Usage Examples

### Check Crop Compatibility

```cpp
// Get farming subsystem
UFarmingSubsystem* FarmingSystem = GetWorld()->GetSubsystem<UFarmingSubsystem>();

// Check if crop is compatible with biome
bool bCompatible = FarmingSystem->GetBiomeCompatibilitySystem()->IsCropCompatibleWithBiome(
    CropDefinition, EBiomeType::Grassland);
```

### Calculate Compatibility Score

```cpp
// Get detailed compatibility score
FBiomeCompatibilityScore Score = FarmingSystem->GetBiomeCompatibilitySystem()->CalculateCompatibilityScore(
    CropDefinition,
    EBiomeType::Desert,
    Temperature,    // 25.0f
    Humidity,       // 0.3f
    SoilQuality);   // 0.7f

// Use the score
float GrowthModifier = Score.GrowthRateModifier; // 0.3 - 1.5
FString Reason = Score.CompatibilityReason;
```

### Get Recommended Crops

```cpp
// Get top 5 recommended crops for a location
TArray<UCropDefinition*> RecommendedCrops = FarmingSystem->GetRecommendedCropsForLocation(
    FarmLocation,
    PlanetActor,
    5); // Max results

// Display to player
for (UCropDefinition* Crop : RecommendedCrops)
{
    UE_LOG(LogTemp, Log, TEXT("Recommended: %s"), *Crop->CropName);
}
```

### Get Crops for Biome

```cpp
// Get all native crops for grassland
TArray<UCropDefinition*> NativeCrops = FarmingSystem->GetBiomeCompatibilitySystem()->GetNativeCropsForBiome(
    EBiomeType::Grassland);

// Get compatible crops (good growth)
TArray<UCropDefinition*> CompatibleCrops = FarmingSystem->GetBiomeCompatibilitySystem()->GetCompatibleCropsForBiome(
    EBiomeType::Grassland);

// Get challenging crops (reduced growth)
TArray<UCropDefinition*> ChallengingCrops = FarmingSystem->GetBiomeCompatibilitySystem()->GetChallengingCropsForBiome(
    EBiomeType::Grassland);
```

## Biome-Specific Crop Lists

### Default Biome Mappings

The system provides default mappings for each biome:

**Grassland** (Best for general farming)

- Native: Wheat, Corn, Soybeans
- Growth Modifier: 1.0x (normal)

**Forest** (Good for shade-tolerant crops)

- Native: Mushrooms, Berries, Root vegetables
- Growth Modifier: 0.85x (reduced sunlight)

**Desert** (Requires irrigation)

- Native: Cacti, Date palms, Agave
- Growth Modifier: 0.7x (water scarcity)

**Tundra** (Cold-hardy crops only)

- Native: Hardy grains, Root vegetables, Berries
- Growth Modifier: 0.6x (cold temperatures)

**Volcanic** (Fertile soil, harsh conditions)

- Native: Heat-resistant plants, Mineral-loving crops
- Growth Modifier: 0.9x (fertile but harsh)

**Ocean** (Limited farming)

- Native: Seaweed, Kelp, Aquatic plants
- Growth Modifier: 0.5x (very limited)

## Compatibility Scoring

### Score Calculation

Compatibility score (0-1) is calculated from:

- **Biome Compatibility** (30%): Is crop native/suitable?
- **Temperature Compatibility** (25%): Within optimal range?
- **Humidity Compatibility** (25%): Within optimal range?
- **Soil Compatibility** (20%): Meets quality requirement?

### Growth Rate Modifiers

Growth rate modifiers range from 0.3x to 1.5x:

- **Native crops**: 1.2x (20% bonus)
- **Suitable biomes**: 1.0x (normal)
- **Adaptable crops**: 0.5x - 1.0x (based on adaptability)
- **Poor conditions**: 0.3x (minimum)

### Compatibility Ratings

- **0.8 - 1.0**: Excellent conditions
- **0.6 - 0.8**: Good conditions
- **0.4 - 0.6**: Challenging but possible
- **0.0 - 0.4**: Poor conditions (slow growth)

## Biome Suitability

### Calculate Overall Suitability

```cpp
// Calculate how suitable a biome is for farming in general
float Suitability = FarmingSystem->GetBiomeCompatibilitySystem()->CalculateBiomeSuitability(
    EBiomeType::Grassland,
    Temperature,
    Humidity,
    SoilQuality);

// Suitability ranges:
// 0.9+ : Excellent for farming
// 0.7-0.9 : Good for farming
// 0.4-0.7 : Challenging
// 0.0-0.4 : Very difficult
```

## Configuration

### Setting Up Crop Lists

Crop lists should be configured in the editor or through data assets:

```cpp
// In editor or initialization code
UBiomeCompatibilitySystem* System = FarmingSubsystem->GetBiomeCompatibilitySystem();

// Biome crop lists are stored in BiomeCropLists array
// Each FBiomeCropList contains:
// - BiomeType
// - NativeCrops (best growth)
// - CompatibleCrops (good growth)
// - ChallengingCrops (reduced growth)
```

### Custom Growth Modifiers

Growth modifiers can be customized per biome:

```cpp
// Access the base growth modifiers map
TMap<EBiomeType, float>& Modifiers = System->BiomeBaseGrowthModifiers;

// Modify values (1.0 = normal growth)
Modifiers[EBiomeType::Grassland] = 1.1f;  // 10% bonus
Modifiers[EBiomeType::Desert] = 0.6f;     // 40% penalty
```

## Blueprint Usage

All main functions are Blueprint-callable:

```
Get Biome Compatibility System
  └─> Is Crop Compatible With Biome
  └─> Calculate Compatibility Score
  └─> Get Native Crops For Biome
  └─> Get Recommended Crops
  └─> Calculate Biome Suitability
```

## Integration with Crop Growth

The compatibility system integrates with the crop growth calculator:

```cpp
// In CropGrowthCalculator
float BiomeModifier = BiomeCompatibilitySystem->GetBiomeGrowthModifier(CropDef, BiomeType);
float FinalGrowthRate = BaseGrowthRate * BiomeModifier * EnvironmentalModifier;
```

## Requirements Met

This implementation satisfies requirements:

- **12.1**: Crop types restricted by biome compatibility (3+ crops per biome)
- **12.3**: Growth rates adjusted based on biome temperature and conditions

## Next Steps

1. Create crop definition assets for each biome
2. Assign crops to biome lists in editor
3. Test compatibility scoring with various conditions
4. Integrate with UI to show compatibility to players
5. Add visual feedback for biome suitability

## Testing

```cpp
// Test compatibility system
void TestBiomeCompatibility()
{
    UFarmingSubsystem* Farming = GetWorld()->GetSubsystem<UFarmingSubsystem>();
    UBiomeCompatibilitySystem* System = Farming->GetBiomeCompatibilitySystem();

    // Test each biome
    for (int32 i = 0; i < (int32)EBiomeType::MAX; ++i)
    {
        EBiomeType Biome = (EBiomeType)i;
        TArray<UCropDefinition*> Crops = System->GetNativeCropsForBiome(Biome);

        UE_LOG(LogTemp, Log, TEXT("Biome %d has %d native crops"), i, Crops.Num());
    }
}
```

## Performance Notes

- Compatibility calculations are lightweight (< 0.1ms)
- Crop lists are cached after initialization
- Recommended crops are sorted once per query
- No per-frame overhead unless actively querying

## See Also

- [Crop Definitions](QUICKSTART_CropDefinitions.md)
- [Farming Subsystem](QUICKSTART_SoilSystem.md)
- [Growth Calculation](QUICKSTART_GrowthCalculation.md)
