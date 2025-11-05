# Harvest Tracking System - Quick Start Guide

## Overview

The Harvest Tracking System (Task 17.4) automatically records all crop harvests, calculates economic value, stores historical data, and provides comprehensive statistics visualization.

## Key Features

- **Automatic Recording**: Every harvest is automatically recorded with detailed information
- **Economic Calculation**: Calculates sell value based on yield, quality, and crop type
- **Historical Storage**: Maintains records of 10+ harvests (configurable up to 100)
- **Statistics Visualization**: Displays aggregate statistics per crop type
- **Recent Harvests**: Shows timeline of recent harvest activity

## Core Components

### 1. FHarvestRecord Structure

Stores detailed information about each harvest:

```cpp
struct FHarvestRecord
{
    FDateTime HarvestTime;           // When harvested
    FString CropName;                // Crop type name
    UCropDefinition* CropType;       // Crop definition reference
    int32 YieldAmount;               // Number of items harvested
    float Quality;                   // Quality (0-1)
    float EconomicValue;             // Calculated sell price
    FIntPoint GridPosition;          // Where it was harvested
    float GrowthDuration;            // Time from plant to harvest
    float AverageSoilQuality;        // Avg soil quality during growth
    float AverageFertility;          // Avg fertility during growth
};
```

### 2. FHarvestStatistics Structure

Aggregates statistics for each crop type:

```cpp
struct FHarvestStatistics
{
    FString CropName;
    int32 TotalHarvests;             // Number of times harvested
    int32 TotalYield;                // Total items harvested
    float AverageYield;              // Average per harvest
    float AverageQuality;            // Average quality
    float TotalEconomicValue;        // Total money earned
    float AverageEconomicValue;      // Average per harvest
    float AverageGrowthDuration;     // Average time to maturity
    float BestQuality;               // Best quality achieved
    int32 BestYield;                 // Best yield achieved
    FDateTime FirstHarvest;          // First time harvested
    FDateTime LastHarvest;           // Most recent harvest
};
```

## Usage

### Recording Harvests (Automatic)

Harvests are automatically recorded when you call `HarvestCrop()`:

```cpp
// Harvest a crop
FHarvestResult Result = FarmPlot->HarvestCrop(FIntPoint(5, 5));

// Harvest is automatically recorded in history
// No additional code needed!
```

### Querying Harvest History

```cpp
// Get all harvest records
TArray<FHarvestRecord> AllHarvests = FarmPlot->GetHarvestHistory();

// Get harvests for specific crop
TArray<FHarvestRecord> WheatHarvests = FarmPlot->GetHarvestHistoryForCrop(WheatCropDef);

// Get statistics for specific crop
FHarvestStatistics WheatStats = FarmPlot->GetHarvestStatistics(WheatCropDef);

// Get statistics for all crops
TArray<FHarvestStatistics> AllStats = FarmPlot->GetAllHarvestStatistics();
```

### Economic Value Queries

```cpp
// Get total economic value of all harvests
float TotalValue = FarmPlot->GetTotalEconomicValue();

// Get economic value for specific crop
float WheatValue = FarmPlot->GetEconomicValueForCrop(WheatCropDef);

// Calculate value for a potential harvest
float EstimatedValue = FarmPlot->CalculateEconomicValue(CropType, YieldAmount, Quality);
```

### Managing History

```cpp
// Trim history to keep only recent records
FarmPlot->TrimHarvestHistory(50); // Keep last 50 harvests

// History is automatically trimmed when it exceeds MaxHarvestRecords
// Default: 100 records
```

## Economic Value Calculation

Economic value is calculated using the formula:

```
EconomicValue = BasePrice × YieldAmount × QualityMultiplier

Where:
- BasePrice: From CropDefinition->SellPrice
- YieldAmount: Number of items harvested
- QualityMultiplier: Lerp(0.5, 1.5, Quality)
  - Low quality (0.0): 0.5x price
  - Medium quality (0.5): 1.0x price
  - High quality (1.0): 1.5x price
```

### Example Calculation

```
Wheat Harvest:
- Base Price: $10 per unit
- Yield: 25 units
- Quality: 0.8 (80%)

Quality Multiplier = Lerp(0.5, 1.5, 0.8) = 1.3
Economic Value = $10 × 25 × 1.3 = $325
```

## UI Integration

### FarmStatusWidget Display

The harvest tracking display shows:

1. **Total Economic Value**: Sum of all harvest values
2. **Total Harvests Count**: Number of harvests recorded
3. **Statistics Grid**: Per-crop statistics table
4. **Recent Harvests Grid**: Timeline of recent harvests

### Configuring Display

```cpp
// Enable/disable harvest tracking display
FarmStatusWidget->bShowHarvestTracking = true;

// Set maximum statistics to display
FarmStatusWidget->MaxHarvestStatsDisplayed = 5;

// Set maximum recent harvests to display
FarmStatusWidget->MaxRecentHarvestsDisplayed = 10;
```

### Statistics Grid Columns

- **Crop**: Crop type name
- **Harvests**: Total number of harvests
- **Avg Yield**: Average yield per harvest
- **Avg Quality**: Average quality percentage
- **Total Value**: Total economic value earned
- **Best Yield**: Highest yield achieved

### Recent Harvests Grid Columns

- **Time**: Time since harvest (e.g., "5m ago", "2h ago")
- **Crop**: Crop type name
- **Yield**: Number of items harvested
- **Quality**: Quality percentage
- **Value**: Economic value

## Quality Tracking

The system tracks average soil quality and fertility during crop growth:

```cpp
// Soil quality and fertility are sampled every tick
// Averages are calculated when crop is harvested

FHarvestRecord Record = ...;
float AvgSoilQuality = Record.AverageSoilQuality;  // 0-1
float AvgFertility = Record.AverageFertility;      // 0-1
```

This allows you to correlate harvest quality with growing conditions.

## Performance Considerations

### Memory Management

- Default: Keeps last 100 harvest records
- Automatically trims when limit exceeded
- Configurable via `MaxHarvestRecords` property

### Sampling Rate

- Soil quality and fertility sampled every tick
- Minimal performance impact (simple accumulation)
- Resets after each harvest

### UI Update Rate

- Updates at widget's `UpdateInterval` (default: 0.5s)
- Only updates when `bShowHarvestTracking` is true
- Grids rebuilt only when data changes

## Blueprint Access

All harvest tracking functions are Blueprint-callable:

```cpp
UFUNCTION(BlueprintCallable)
TArray<FHarvestRecord> GetHarvestHistory() const;

UFUNCTION(BlueprintCallable)
FHarvestStatistics GetHarvestStatistics(UCropDefinition* CropType) const;

UFUNCTION(BlueprintCallable)
float GetTotalEconomicValue() const;
```

## Example: Analyzing Farm Performance

```cpp
// Get all statistics
TArray<FHarvestStatistics> AllStats = FarmPlot->GetAllHarvestStatistics();

// Find most profitable crop
FHarvestStatistics* BestCrop = nullptr;
float HighestValue = 0.0f;

for (FHarvestStatistics& Stats : AllStats)
{
    if (Stats.TotalEconomicValue > HighestValue)
    {
        HighestValue = Stats.TotalEconomicValue;
        BestCrop = &Stats;
    }
}

if (BestCrop)
{
    UE_LOG(LogTemp, Log, TEXT("Most profitable crop: %s ($%.2f total)"),
        *BestCrop->CropName, BestCrop->TotalEconomicValue);
}

// Find crop with best quality
FHarvestStatistics* HighestQuality = nullptr;
float BestQuality = 0.0f;

for (FHarvestStatistics& Stats : AllStats)
{
    if (Stats.AverageQuality > BestQuality)
    {
        BestQuality = Stats.AverageQuality;
        HighestQuality = &Stats;
    }
}

if (HighestQuality)
{
    UE_LOG(LogTemp, Log, TEXT("Highest quality crop: %s (%.1f%% avg)"),
        *HighestQuality->CropName, HighestQuality->AverageQuality * 100.0f);
}
```

## Debugging

### Logging

Harvest recording includes detailed logging:

```
FarmPlot: Recorded harvest - Wheat x25, Value: 325.00, Quality: 0.80, Duration: 600.5s
FarmPlot: Trimmed harvest history - Removed 10 old records, keeping 100
```

### Inspecting Records

```cpp
// Print all harvest records
TArray<FHarvestRecord> History = FarmPlot->GetHarvestHistory();
for (const FHarvestRecord& Record : History)
{
    UE_LOG(LogTemp, Log, TEXT("Harvest: %s x%d, Quality: %.2f, Value: $%.2f"),
        *Record.CropName,
        Record.YieldAmount,
        Record.Quality,
        Record.EconomicValue);
}
```

## Integration with Other Systems

### Alert System

Harvest tracking can trigger alerts:

```cpp
// Example: Alert when total value reaches milestone
float TotalValue = FarmPlot->GetTotalEconomicValue();
if (TotalValue >= 10000.0f)
{
    AlertSystem->TriggerAlert(
        EFarmAlertType::HarvestReady,
        FarmPlot,
        TEXT("Farm value milestone reached!"),
        FString::Printf(TEXT("Total farm value: $%.2f"), TotalValue)
    );
}
```

### Crop Monitoring

Harvest statistics inform crop selection:

```cpp
// Get statistics for all crops
TArray<FHarvestStatistics> Stats = FarmPlot->GetAllHarvestStatistics();

// Recommend crops based on performance
for (const FHarvestStatistics& CropStats : Stats)
{
    if (CropStats.AverageQuality > 0.8f && CropStats.TotalHarvests >= 5)
    {
        UE_LOG(LogTemp, Log, TEXT("Recommended crop: %s (High quality, proven track record)"),
            *CropStats.CropName);
    }
}
```

## Best Practices

1. **Regular Monitoring**: Check harvest statistics to optimize crop selection
2. **Quality Focus**: High quality crops earn 50% more than low quality
3. **History Management**: Trim history periodically if memory is a concern
4. **Economic Planning**: Use total value to track farm profitability
5. **Performance Analysis**: Compare growth duration to optimize planting schedules

## Troubleshooting

### No Harvests Recorded

- Ensure `HarvestCrop()` returns `bSuccess = true`
- Check that crop was fully mature (`GrowthProgress >= 1.0`)
- Verify `CropType` is valid

### Incorrect Economic Values

- Check `CropDefinition->SellPrice` is set correctly
- Verify quality calculation (health × fertility)
- Ensure quality multiplier is in range [0.5, 1.5]

### Missing Statistics

- Statistics only generated for crops with harvests
- Check `GetHarvestHistoryForCrop()` returns records
- Verify crop type pointer matches

## Future Enhancements

Potential additions to the harvest tracking system:

- Export harvest data to CSV/JSON
- Seasonal trend analysis
- Profit margin calculations (cost vs. revenue)
- Harvest prediction based on historical data
- Achievement system for harvest milestones
- Leaderboards for farm performance

## Summary

The Harvest Tracking System provides comprehensive harvest analytics:

✅ Automatic recording of all harvests
✅ Economic value calculation
✅ Historical data storage (10+ harvests)
✅ Statistics visualization per crop type
✅ Recent harvest timeline
✅ Quality and performance tracking
✅ Blueprint and C++ API
✅ UI integration with FarmStatusWidget

This system enables data-driven farming decisions and provides players with meaningful feedback on their farming performance.
