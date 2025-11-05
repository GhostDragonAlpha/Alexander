# Environmental Modifiers System - Quick Start Guide

## Overview

The Environmental Modifier System calculates environmental effects on crop growth including:

- **Humidity-based water consumption** (up to 300% increase in dry conditions)
- **Temperature growth modifiers** (0.3x to 1.5x growth rate)
- **Biome-specific challenges** (drought, pests, extreme weather)
- **Native crop bonuses** (up to 30% growth bonus)

## Key Features

### 1. Water Consumption Modifiers

Low humidity dramatically increases water consumption:

- **Optimal humidity**: 90-100% normal water consumption
- **Low humidity (< 30%)**: Up to 300% water consumption
- **High humidity (> 80%)**: Reduced water consumption (50-90%)

```cpp
// Calculate water consumption multiplier
float Multiplier = UEnvironmentalModifierSystem::CalculateWaterConsumptionMultiplier(
    CurrentHumidity,
    CropDef->OptimalHumidity.Min,
    CropDef->OptimalHumidity.Max
);

// Get adjusted water consumption
float AdjustedConsumption = UEnvironmentalModifierSystem::CalculateAdjustedWaterConsumption(
    CropDef->WaterConsumption,
    CurrentHumidity,
    CropDef->OptimalHumidity.Min,
    CropDef->OptimalHumidity.Max
);
```

### 2. Temperature Growth Modifiers

Temperature affects growth rate significantly:

- **Optimal range**: 100-120% growth rate
- **Outside optimal**: Exponential penalty
- **Extreme cold (< 0°C)**: 30-50% growth rate
- **Extreme heat (> 40°C)**: 30-50% growth rate

```cpp
// Calculate temperature modifier
float TempModifier = UEnvironmentalModifierSystem::CalculateTemperatureGrowthModifier(
    CurrentTemperature,
    CropDef->OptimalTemperature.Min,
    CropDef->OptimalTemperature.Max
);

// Check for extreme conditions
bool bExtreme = UEnvironmentalModifierSystem::IsExtremeTemperature(CurrentTemperature);
```

### 3. Biome-Specific Challenges

Each biome has unique challenges:

**Grassland**:

- Pests and disease (moderate frequency)
- Generally favorable conditions

**Forest**:

- Pests, disease, poor soil
- Shade-related challenges

**Desert**:

- Drought, heat waves, poor soil, high winds
- Very challenging conditions

**Tundra**:

- Cold snaps, poor soil, high winds
- Short growing season

**Volcanic**:

- Heat waves, initially poor soil
- Fertile but harsh

**Ocean**:

- Flooding, high winds
- Specialized environment

```cpp
// Get active challenges
TArray<EEnvironmentalChallenge> Challenges = ModifierSystem->GetActiveChallenges(
    BiomeType,
    Temperature,
    Humidity,
    SoilQuality
);

// Calculate challenge penalty
float ChallengeModifier = ModifierSystem->CalculateChallengeModifier(Challenges, CropDef);
```

### 4. Native Crop Bonuses

Native crops receive growth bonuses:

- **Grassland natives**: +25% growth
- **Volcanic natives**: +30% growth (most fertile)
- **Forest/Ocean natives**: +20% growth
- **Desert/Tundra natives**: +15% growth

```cpp
// Calculate native crop bonus
float NativeBonus = UEnvironmentalModifierSystem::CalculateNativeCropBonus(
    bIsNativeCrop,
    BiomeType
);

// Get benefits description
FString Benefits = UEnvironmentalModifierSystem::GetNativeCropBenefitsDescription(BiomeType);
```

## Complete Calculation

Calculate all environmental modifiers at once:

```cpp
// Create modifier system
UEnvironmentalModifierSystem* ModifierSystem = NewObject<UEnvironmentalModifierSystem>();
ModifierSystem->Initialize();

// Calculate all modifiers
FEnvironmentalModifierResult Result = ModifierSystem->CalculateEnvironmentalModifiers(
    CropDef,
    BiomeType,
    Temperature,
    Humidity,
    bIsNativeCrop
);

// Access results
float WaterMultiplier = Result.WaterConsumptionMultiplier;  // 0.5 - 3.0
float TempModifier = Result.TemperatureGrowthModifier;      // 0.3 - 1.5
float BiomeModifier = Result.BiomeCompatibilityModifier;    // 0.5 - 1.2
float NativeBonus = Result.NativeCropBonus;                 // 1.0 - 1.3
float ChallengeModifier = Result.ChallengeModifier;         // 0.5 - 1.0
float TotalModifier = Result.TotalGrowthModifier;           // Combined

// Get active challenges
TArray<EEnvironmentalChallenge> ActiveChallenges = Result.ActiveChallenges;

// Get human-readable summary
FString Summary = Result.ModifierSummary;
```

## Integration with Growth System

Integrate environmental modifiers with crop growth:

```cpp
// Calculate base growth rate
FGrowthCalculationResult GrowthResult = UCropGrowthCalculator::CalculateGrowthRate(
    CropDef,
    Environment,
    CropHealth,
    SeasonalModifier
);

// Apply environmental modifiers
FEnvironmentalModifierResult EnvModifiers = ModifierSystem->CalculateEnvironmentalModifiers(
    CropDef,
    BiomeType,
    Environment.Temperature,
    Environment.Humidity,
    bIsNativeCrop
);

// Combine modifiers
float FinalGrowthRate = GrowthResult.TotalGrowthModifier * EnvModifiers.TotalGrowthModifier;

// Apply to growth progress
float GrowthDelta = (DeltaTime / CropDef->GrowthDuration) * FinalGrowthRate;
GrowthProgress += GrowthDelta;

// Calculate adjusted water consumption
float WaterNeeded = EnvModifiers.WaterConsumptionMultiplier * CropDef->WaterConsumption * DeltaTime;
```

## Challenge Descriptions

Get human-readable challenge information:

```cpp
// Get challenge description
FString Description = UEnvironmentalModifierSystem::GetChallengeDescription(
    EEnvironmentalChallenge::Drought
);

// Get biome-specific challenges
TArray<EEnvironmentalChallenge> BiomeChallenges = ModifierSystem->GetBiomeChallenges(BiomeType);

// Generate modifier summary
FString Summary = UEnvironmentalModifierSystem::GenerateModifierSummary(Result);
```

## Example: Complete Growth Update

```cpp
void AFarmPlot::UpdateCropGrowth(float DeltaTime)
{
    // Get environmental conditions
    FFarmEnvironment Environment = GetEnvironmentalConditions();

    // Calculate base growth
    FGrowthCalculationResult GrowthResult = UCropGrowthCalculator::CalculateGrowthRate(
        CropDef,
        Environment.ToEnvironmentalFactors(),
        CropHealth,
        SeasonalModifier
    );

    // Calculate environmental modifiers
    FEnvironmentalModifierResult EnvModifiers = EnvironmentalModifierSystem->CalculateEnvironmentalModifiers(
        CropDef,
        CurrentBiome,
        Environment.Temperature,
        Environment.Humidity,
        bIsNativeCrop
    );

    // Apply combined modifiers
    float FinalGrowthRate = GrowthResult.TotalGrowthModifier * EnvModifiers.TotalGrowthModifier;
    float GrowthDelta = (DeltaTime / CropDef->GrowthDuration) * FinalGrowthRate;
    GrowthProgress = FMath::Clamp(GrowthProgress + GrowthDelta, 0.0f, 1.0f);

    // Update water consumption
    float WaterMultiplier = EnvModifiers.WaterConsumptionMultiplier;
    float WaterNeeded = CropDef->WaterConsumption * WaterMultiplier * (DeltaTime / 86400.0f); // Per day to per second
    WaterLevel = FMath::Max(0.0f, WaterLevel - WaterNeeded);

    // Check for challenges
    if (EnvModifiers.ActiveChallenges.Num() > 0)
    {
        // Apply challenge effects to crop health
        float HealthLoss = EnvModifiers.ActiveChallenges.Num() * 0.01f * DeltaTime; // 1% per challenge per second
        CropHealth = FMath::Max(0.0f, CropHealth - HealthLoss);
    }

    // Log status
    UE_LOG(LogTemp, Log, TEXT("Crop Growth: %.1f%%, Rate: %.2fx, Water: %.1f%%, Challenges: %d"),
        GrowthProgress * 100.0f,
        FinalGrowthRate,
        WaterLevel * 100.0f,
        EnvModifiers.ActiveChallenges.Num()
    );
}
```

## Configuration

Adjust system parameters in the Environmental Modifier System:

```cpp
// Water consumption ranges
MinWaterConsumptionMultiplier = 0.5f;  // 50% minimum
MaxWaterConsumptionMultiplier = 3.0f;  // 300% maximum

// Temperature modifier ranges
MinTemperatureModifier = 0.3f;  // 30% minimum
MaxTemperatureModifier = 1.5f;  // 150% maximum

// Native crop bonus
NativeCropBonusAmount = 0.3f;  // 30% bonus

// Challenge severity
BaseChallengeModifier = 0.8f;  // 20% penalty per challenge
```

## Best Practices

1. **Always check for native crops** - They provide significant bonuses
2. **Monitor humidity in dry biomes** - Water consumption can triple
3. **Plan for biome challenges** - Each biome has unique difficulties
4. **Use resistant crops** - Drought/pest resistance reduces challenge penalties
5. **Combine with growth calculator** - Use both systems for complete simulation
6. **Update regularly** - Recalculate modifiers when conditions change

## Performance Notes

- Calculations are lightweight and can run every frame
- Challenge determination uses random values - cache results if needed
- Biome challenge configs are set up once during initialization
- All static functions are thread-safe

## Testing

Test environmental modifiers:

```cpp
// Test water consumption in drought
float DroughtMultiplier = UEnvironmentalModifierSystem::CalculateWaterConsumptionMultiplier(
    0.1f,  // 10% humidity
    0.4f,  // Optimal min
    0.7f   // Optimal max
);
// Expected: ~2.5-3.0x multiplier

// Test temperature extremes
float ColdModifier = UEnvironmentalModifierSystem::CalculateTemperatureGrowthModifier(
    -5.0f,  // -5°C
    15.0f,  // Optimal min
    25.0f   // Optimal max
);
// Expected: ~0.3-0.4x modifier

// Test native crop bonus
float Bonus = UEnvironmentalModifierSystem::CalculateNativeCropBonus(
    true,  // Is native
    EBiomeType::Volcanic
);
// Expected: 1.3x (30% bonus)
```

## See Also

- **CropGrowthCalculator**: Base growth rate calculations
- **BiomeCompatibilitySystem**: Crop-biome compatibility
- **CropHealthSystem**: Health and disease mechanics
- **FarmingSubsystem**: Overall farming management
