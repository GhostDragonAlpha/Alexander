# Crop Health System - Quick Start Guide

## Overview

The Crop Health System manages crop vitality, visual indicators, and degradation over time based on environmental conditions and care.

## Key Components

### UCropHealthSystem

Main health calculation and management class.

**Location**: `Source/Alexander/Public/CropHealthSystem.h`

## Health Calculation

### Basic Usage

```cpp
// Calculate comprehensive crop health
FHealthCalculationResult HealthResult = UCropHealthSystem::CalculateCropHealth(
    CropDefinition,
    EnvironmentalFactors,
    CurrentHealth,
    WaterLevel,
    SoilFertility,
    TimeSinceLastWater,
    DeltaTime
);

// Access results
float NewHealth = HealthResult.CurrentHealth;
bool NeedsWater = HealthResult.bNeedsWater;
FString Issue = HealthResult.PrimaryIssue;
```

## Health Status Levels

| Status    | Health Range | Description               |
| --------- | ------------ | ------------------------- |
| Excellent | 0.9 - 1.0    | Thriving                  |
| Good      | 0.7 - 0.9    | Healthy                   |
| Fair      | 0.5 - 0.7    | Adequate                  |
| Poor      | 0.3 - 0.5    | Struggling                |
| Critical  | 0.1 - 0.3    | Needs Attention           |
| Dying     | 0.0 - 0.1    | Immediate Action Required |

## Degradation Factors

### Water Deficit

- **Threshold**: Water level < 0.3 or time since last water > 5 minutes
- **Impact**: 10% health loss per minute without water
- **Visual**: Wilting, brown color

### Temperature Stress

- **Threshold**: 10°C outside optimal range
- **Impact**: 2% health loss per second when stressed
- **Visual**: Yellowing, reduced growth

### Nutrient Deficiency

- **Threshold**: Soil fertility < 0.5
- **Impact**: 1% health loss per second
- **Visual**: Yellowing leaves, stunted growth

## Visual Indicators

### Leaf Color

```cpp
FLinearColor LeafColor = UCropHealthSystem::CalculateLeafColor(
    Health,
    NutrientDeficiency,
    WaterDeficit
);
```

**Color Progression**:

- Healthy: Vibrant green (0.2, 0.8, 0.2)
- Nutrient deficient: Yellow-green
- Water stressed: Brown (0.4, 0.3, 0.1)

### Wilting Amount

```cpp
float WiltingAmount = UCropHealthSystem::CalculateWiltingAmount(
    Health,
    WaterDeficit
);
```

**Wilting Scale**: 0.0 (upright) to 1.0 (severely wilted)

### Scale Modifier

```cpp
float ScaleModifier = UCropHealthSystem::CalculateScaleModifier(Health);
```

**Scale Range**: 0.7 (unhealthy) to 1.0 (healthy)

## Water Need Detection

### Check if Crop Needs Water

```cpp
bool NeedsWater = UCropHealthSystem::NeedsWater(
    WaterLevel,
    WaterConsumption,
    TimeSinceLastWater
);
```

### Calculate Water Deficit

```cpp
float Deficit = UCropHealthSystem::CalculateWaterDeficit(
    WaterLevel,
    WaterConsumption,
    TimeSinceLastWater
);
```

### Time Until Water Needed

```cpp
float TimeRemaining = UCropHealthSystem::GetTimeUntilWaterNeeded(
    WaterLevel,
    WaterConsumption
);
```

## Integration with FarmPlot

### Automatic Health Updates

Health is automatically calculated in `AFarmPlot::UpdateCropGrowth()`:

```cpp
// Health calculation happens every tick
FHealthCalculationResult HealthResult = UCropHealthSystem::CalculateCropHealth(
    Cell.CropType,
    EnvFactors,
    Cell.Health,
    WaterLevel,
    Fertility,
    Cell.TimeSinceLastWater,
    DeltaTime
);

// Health is updated
Cell.Health = HealthResult.CurrentHealth;
Cell.bNeedsWater = HealthResult.bNeedsWater;
```

### Visual Updates

Visual indicators are applied in `AFarmPlot::UpdateCropVisuals()`:

```cpp
// Generate visual indicators
FHealthVisualIndicators VisualIndicators = UCropHealthSystem::GenerateVisualIndicators(
    Cell.Health,
    DegradationFactors
);

// Apply to crop instance
FinalScale *= VisualIndicators.ScaleModifier;
FLinearColor HealthColor = VisualIndicators.LeafColor;
```

## Configuration

### Static Configuration Values

Located in `UCropHealthSystem`:

```cpp
// Health degradation rate (per minute without water)
static float HealthDegradationRate = 0.1f; // 10%

// Time without water before degradation starts
static float WaterGracePeriod = 300.0f; // 5 minutes

// Water level warning threshold
static float WaterWarningThreshold = 0.3f;

// Critical water level
static float WaterCriticalThreshold = 0.1f;

// Temperature stress threshold (degrees outside optimal)
static float TemperatureStressThreshold = 10.0f;

// Nutrient deficiency threshold
static float NutrientDeficiencyThreshold = 0.5f;

// Maximum health recovery rate per second
static float MaxHealthRecoveryRate = 0.05f; // 5%

// Minimum viable health
static float MinimumViableHealth = 0.05f;
```

## Health Recovery

### Automatic Recovery

When conditions improve:

- Water level > 0.3: Health recovers at 5% per second
- Optimal temperature: No temperature stress damage
- Adequate nutrients: No nutrient damage

### Manual Recovery

```cpp
// Water the plot to improve health
FarmPlot->WaterPlot(0.5f); // Add 50% water

// Fertilize to restore nutrients
FarmPlot->FertilizePlot(0.3f); // Add 30% fertility
```

## Health Impact on Growth

Health affects growth rate:

```cpp
// Growth is reduced by poor health
float HealthModifier = FMath::Lerp(0.1f, 1.0f, Cell.Health);
float GrowthAmount = BaseGrowthRate * GrowthModifier * HealthModifier * DeltaTime;
```

**Impact**:

- 100% health: Normal growth
- 50% health: 55% growth rate
- 10% health: 19% growth rate (severely stunted)

## Debugging

### Log Health Issues

```cpp
if (Cell.Health < 0.3f)
{
    UE_LOG(LogTemp, Warning, TEXT("Crop health critical! Primary issue: %s"),
        *HealthResult.PrimaryIssue);
}
```

### Get Detailed Health Info

```cpp
FHealthCalculationResult Result = UCropHealthSystem::CalculateCropHealth(...);

UE_LOG(LogTemp, Log, TEXT("Health: %.2f, Status: %s"),
    Result.CurrentHealth,
    *UCropHealthSystem::GetHealthStatusDescription(Result.HealthStatus));

UE_LOG(LogTemp, Log, TEXT("Water Deficit: %.2f, Temp Stress: %.2f, Nutrient Def: %.2f"),
    Result.DegradationFactors.WaterDeficit,
    Result.DegradationFactors.TemperatureStress,
    Result.DegradationFactors.NutrientDeficiency);
```

## Common Issues

### Crops Dying Too Fast

Adjust degradation rate:

```cpp
UCropHealthSystem::HealthDegradationRate = 0.05f; // Slower degradation
```

### Water Warnings Too Frequent

Adjust warning threshold:

```cpp
UCropHealthSystem::WaterWarningThreshold = 0.2f; // Lower threshold
```

### Health Not Recovering

Check recovery rate:

```cpp
UCropHealthSystem::MaxHealthRecoveryRate = 0.1f; // Faster recovery
```

## Best Practices

1. **Monitor Health Regularly**: Check crop health every few seconds
2. **Respond to Warnings**: Water crops when `bNeedsWater` is true
3. **Maintain Soil Quality**: Keep fertility above 0.5 for healthy crops
4. **Balance Temperature**: Ensure crops are in appropriate biomes
5. **Visual Feedback**: Use visual indicators to show players crop status

## Example: Complete Health Management

```cpp
void AMyFarmManager::ManageCropHealth(float DeltaTime)
{
    for (AFarmPlot* Plot : FarmPlots)
    {
        // Check each crop in the plot
        for (int32 i = 0; i < Plot->CropGrid.Num(); ++i)
        {
            FCropCell& Cell = Plot->CropGrid[i];
            if (!Cell.CropType) continue;

            // Get health status
            FEnvironmentalFactors Env = GetEnvironmentalFactors(Plot);
            FHealthCalculationResult Health = UCropHealthSystem::CalculateCropHealth(
                Cell.CropType, Env, Cell.Health,
                Plot->WaterLevel, Plot->Fertility,
                Cell.TimeSinceLastWater, DeltaTime
            );

            // Respond to health issues
            if (Health.bNeedsWater)
            {
                Plot->WaterPlot(0.5f);
            }

            if (Health.bNeedsFertilizer)
            {
                Plot->FertilizePlot(0.3f);
            }

            // Alert player if critical
            if (Health.HealthStatus == ECropHealthStatus::Critical)
            {
                ShowHealthAlert(Cell.CropType->CropName, Health.PrimaryIssue);
            }
        }
    }
}
```

## See Also

- [Crop Growth Calculator](QUICKSTART_GrowthCalculation.md)
- [Growth Stages](QUICKSTART_GrowthStages.md)
- [Crop Definitions](QUICKSTART_CropDefinitions.md)
- [Soil System](QUICKSTART_SoilSystem.md)
