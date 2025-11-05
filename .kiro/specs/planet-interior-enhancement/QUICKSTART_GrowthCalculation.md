# Quick Start: Crop Growth Calculation System

## Overview

The Crop Growth Calculation System provides comprehensive environmental factor evaluation for realistic crop growth simulation. It calculates growth rates based on temperature, humidity, sunlight, soil quality, water availability, and fertility.

## Key Components

### UCropGrowthCalculator

Central calculator class that evaluates all environmental factors affecting crop growth.

**Location**: `Source/Alexander/Public/CropGrowthCalculator.h`

### FEnvironmentalFactors

Structure containing all environmental conditions:

- Temperature (Celsius)
- Humidity (0-1)
- Sunlight (0-1)
- Soil Quality (0-1)
- Water Level (0-1)
- Fertility (0-1)

### FGrowthCalculationResult

Detailed breakdown of growth calculation:

- Individual factor modifiers
- Total growth modifier
- Limiting factor identification

## Usage Examples

### Calculate Growth Rate

```cpp
// Get environmental conditions
FEnvironmentalFactors Environment;
Environment.Temperature = 22.0f;
Environment.Humidity = 0.65f;
Environment.Sunlight = 0.9f;
Environment.SoilQuality = 0.8f;
Environment.WaterLevel = 0.7f;
Environment.Fertility = 0.85f;

// Calculate growth rate
FGrowthCalculationResult Result = UCropGrowthCalculator::CalculateGrowthRate(
    CropDefinition,
    Environment,
    CropHealth
);

// Use the result
float GrowthRate = Result.TotalGrowthModifier;
FString LimitingFactor = Result.LimitingFactor;
```

### Get Crop Growth Details from Farm Plot

```cpp
// Get detailed growth information for a specific crop
FIntPoint GridPosition(5, 5);
FGrowthCalculationResult GrowthDetails = FarmPlot->GetCropGrowthDetails(GridPosition);

UE_LOG(LogTemp, Log, TEXT("Growth Rate: %.2f"), GrowthDetails.TotalGrowthModifier);
UE_LOG(LogTemp, Log, TEXT("Temperature Factor: %.2f"), GrowthDetails.TemperatureModifier);
UE_LOG(LogTemp, Log, TEXT("Limiting Factor: %s"), *GrowthDetails.LimitingFactor);
```

### Calculate Time to Harvest

```cpp
// Get estimated time until crop is ready
float TimeToHarvest = FarmPlot->GetTimeToHarvest(GridPosition);

// Convert to minutes for display
float MinutesToHarvest = TimeToHarvest / 60.0f;
UE_LOG(LogTemp, Log, TEXT("Time to harvest: %.1f minutes"), MinutesToHarvest);
```

## Environmental Factor Details

### Temperature Modifier

- **Optimal Range**: Within crop's optimal temperature range
- **Bonus**: Up to 10% bonus for being in center of optimal range
- **Penalty**: Exponential falloff outside optimal range
- **Minimum**: 10% growth rate even in extreme temperatures

**Formula**:

- Within optimal: 1.0 - 1.1x
- Outside optimal: Exponential decay based on deviation

### Humidity Modifier

- **Optimal Range**: Within crop's optimal humidity range
- **Bonus**: Up to 15% bonus for optimal humidity
- **Drought Stress**: More severe penalty for low humidity (1.5x multiplier)
- **Excess Humidity**: Less severe penalty for high humidity

**Formula**:

- Within optimal: 1.0 - 1.15x
- Below optimal: Max(0.2, 1.0 - deviation \* 1.5)
- Above optimal: Max(0.4, 1.0 - deviation)

### Sunlight Modifier

Handles three crop types differently:

**Shade-Tolerant Crops** (requirement < 0.3):

- Can handle low light well
- Stressed by too much light (> 0.7)
- Range: 0.5 - 1.0x

**Sun-Loving Crops** (requirement > 0.7):

- Need lots of light
- Severe penalty for insufficient light (quadratic)
- Bonus for excess light
- Range: 0.2 - 1.2x

**Moderate Crops**:

- Linear penalty for insufficient light
- Small bonus for extra light
- Range: 0.3 - 1.15x

### Soil Quality Modifier

- **Below Requirement**: Quadratic penalty
- **Above Requirement**: Diminishing returns bonus
- **Range**: 0.3 - 1.2x

### Water Modifier

Tiered system based on water level:

- **Critical (< 0.2)**: 0.1 - 0.4x (crops dying)
- **Low (0.2 - 0.4)**: 0.4 - 0.7x (crops stressed)
- **Adequate (0.4 - 0.6)**: 0.7 - 0.95x (slight stress)
- **Optimal (0.6 - 0.8)**: 1.0x (perfect)
- **Excess (> 0.8)**: 0.8 - 1.0x (waterlogged)

### Fertility Modifier

- **Below Average (< 0.5)**: Increasing penalty (0.3 - 0.7x)
- **Above Average (> 0.5)**: Diminishing returns (0.7 - 1.0x)

## Configuration

Static configuration values in `UCropGrowthCalculator`:

```cpp
// Temperature tolerance (degrees outside optimal before severe penalty)
float TemperatureTolerance = 10.0f;

// Temperature penalty rate (per degree outside optimal)
float TemperaturePenaltyRate = 0.05f;

// Humidity tolerance (deviation from optimal before penalty)
float HumidityTolerance = 0.2f;

// Sunlight bonus for excess light (up to this multiplier)
float SunlightBonusMax = 1.2f;

// Minimum growth rate (even in worst conditions)
float MinimumGrowthRate = 0.1f;

// Maximum growth rate (with perfect conditions)
float MaximumGrowthRate = 2.0f;
```

## Integration with Farm Plot

The growth calculator is automatically used by `AFarmPlot::UpdateCropGrowth()`:

1. Gets current environmental conditions from planet
2. Builds `FEnvironmentalFactors` structure
3. Calls `UCropGrowthCalculator::CalculateGrowthRate()`
4. Applies growth modifier to base growth rate
5. Updates crop progress

## Limiting Factor Detection

The system automatically identifies what's limiting growth:

```cpp
FString LimitingFactor = UCropGrowthCalculator::DetermineLimitingFactor(Result);
```

Possible limiting factors:

- Temperature
- Humidity
- Sunlight
- Soil Quality
- Water
- Fertility
- Health
- None - Optimal Conditions

## Growth Stages

The system provides human-readable growth stage descriptions:

```cpp
FString Stage = UCropGrowthCalculator::GetGrowthStageDescription(GrowthProgress);
```

Stages:

- **Seedling** (0-15%)
- **Early Growth** (15-35%)
- **Vegetative** (35-60%)
- **Flowering** (60-85%)
- **Ripening** (85-100%)
- **Mature** (100%)

## Blueprint Support

All main functions are Blueprint-callable:

- `CalculateGrowthRate`
- `CalculateTemperatureModifier`
- `CalculateHumidityModifier`
- `CalculateSunlightModifier`
- `CalculateSoilQualityModifier`
- `CalculateWaterModifier`
- `CalculateFertilityModifier`
- `DetermineLimitingFactor`
- `GetGrowthStageDescription`
- `CalculateTimeToMaturity`

## Testing

### Test Optimal Conditions

```cpp
FEnvironmentalFactors OptimalEnv;
OptimalEnv.Temperature = 20.0f;
OptimalEnv.Humidity = 0.6f;
OptimalEnv.Sunlight = 1.0f;
OptimalEnv.SoilQuality = 1.0f;
OptimalEnv.WaterLevel = 0.7f;
OptimalEnv.Fertility = 1.0f;

FGrowthCalculationResult Result = UCropGrowthCalculator::CalculateGrowthRate(
    CropDef, OptimalEnv, 1.0f
);

// Should be close to 1.0 or slightly above
check(Result.TotalGrowthModifier >= 0.9f);
```

### Test Stress Conditions

```cpp
FEnvironmentalFactors StressEnv;
StressEnv.Temperature = 5.0f;  // Too cold
StressEnv.Humidity = 0.2f;     // Too dry
StressEnv.Sunlight = 0.3f;     // Low light
StressEnv.SoilQuality = 0.4f;  // Poor soil
StressEnv.WaterLevel = 0.15f;  // Critical water
StressEnv.Fertility = 0.3f;    // Low fertility

FGrowthCalculationResult Result = UCropGrowthCalculator::CalculateGrowthRate(
    CropDef, StressEnv, 0.5f  // Also unhealthy
);

// Should be significantly reduced
check(Result.TotalGrowthModifier < 0.3f);
check(Result.LimitingFactor != TEXT("None"));
```

## Performance Considerations

- All calculations are lightweight mathematical operations
- No memory allocations during calculation
- Safe to call every frame for multiple crops
- Results can be cached if environment doesn't change

## Future Enhancements

Potential additions:

- Seasonal growth modifiers
- Pest/disease impact on growth
- Companion planting bonuses
- Crop rotation benefits
- Microclimate effects
- Wind stress factors
- Altitude effects

## Related Systems

- **CropDefinition**: Defines crop requirements
- **FarmPlot**: Uses calculator for growth updates
- **PlanetWeatherComponent**: Provides temperature/humidity
- **DayNightCycleComponent**: Provides sunlight intensity
- **SoilSystem**: Provides soil quality/fertility

## Requirements Satisfied

This implementation satisfies the following requirements:

- **Requirement 11.2**: Crop growth simulation based on environmental conditions
- **Requirement 15.2**: Growth cycle completion within 10-30 minutes
- **Requirement 15.3**: Growth rate reduction based on environmental factors
