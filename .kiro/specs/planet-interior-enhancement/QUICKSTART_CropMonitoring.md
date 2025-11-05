# Crop Monitoring Quick Start Guide

## Overview

The Crop Monitoring system provides detailed tracking and visualization of all crops in a farm plot, including growth progress, health status, and estimated harvest times.

## Features

### 1. Growth Progress Display

- Shows average growth progress for each crop type
- Displays individual crop growth in grid visualization
- Color-coded progress indicators (yellow → green as crops mature)

### 2. Estimated Harvest Times

- Calculates time until crops are ready for harvest
- Shows "Ready" for mature crops
- Formats time as hours/minutes or minutes/seconds
- Displays earliest harvest time per crop type

### 3. Crop Health Overview

- Average health percentage per crop type
- Color-coded health indicators:
  - Green: Healthy (>66%)
  - Yellow: Fair (33-66%)
  - Red: Poor (<33%)
- Tracks unhealthy crop count

### 4. Crop Type Filtering

- Filter display to show specific crop type
- View all crops or focus on one type
- Filtered view shows detailed stats for selected crop

## Widget Components

### Required Widget Bindings

```cpp
// Crop Monitoring Display
UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
UTextBlock* CropMonitoringTitleText;

UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
UGridPanel* CropMonitoringGrid;

UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
UTextBlock* FilteredCropTypeText;
```

### Configuration Properties

```cpp
// Show crop monitoring section
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm Status")
bool bShowCropMonitoring = true;

// Maximum number of crop types to display
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Farm Status")
int32 MaxCropTypesDisplayed = 10;
```

## Usage

### Basic Setup

1. **Add Widget Components** in Blueprint:

   - Add `GridPanel` named "CropMonitoringGrid"
   - Add `TextBlock` named "CropMonitoringTitleText"
   - Add `TextBlock` named "FilteredCropTypeText"

2. **Enable Monitoring**:

```cpp
FarmStatusWidget->bShowCropMonitoring = true;
FarmStatusWidget->SetFarmPlot(MyFarmPlot);
```

### Filtering Crops

```cpp
// Filter to show only wheat
UCropDefinition* WheatCrop = GetWheatCropDefinition();
FarmStatusWidget->SetCropTypeFilter(WheatCrop);

// Clear filter to show all crops
FarmStatusWidget->ClearCropTypeFilter();
```

### Getting Monitoring Data

```cpp
// Get all planted crop types
TArray<UCropDefinition*> CropTypes = FarmStatusWidget->GetPlantedCropTypes();

// Get monitoring data for specific crop
FCropMonitoringData WheatData = FarmStatusWidget->GetCropMonitoringData(WheatCrop);

// Get all monitoring data
TArray<FCropMonitoringData> AllData = FarmStatusWidget->GetAllCropMonitoringData();
```

## Monitoring Data Structure

```cpp
struct FCropMonitoringData
{
    UCropDefinition* CropType;           // Crop definition
    FString CropName;                    // Display name
    int32 TotalCount;                    // Total planted
    int32 MatureCount;                   // Ready to harvest
    float AverageGrowthProgress;         // 0-1
    float AverageHealth;                 // 0-1
    FTimespan EstimatedTimeToHarvest;    // Time until first crop ready
    int32 CropsNeedingWater;            // Count needing water
    int32 UnhealthyCrops;               // Count with health < 0.5
};
```

## Display Format

The monitoring grid displays:

| Crop   | Count   | Growth | Health | Harvest Time | Needs Water |
| ------ | ------- | ------ | ------ | ------------ | ----------- |
| Wheat  | 5 / 10  | 75%    | 90%    | 2m 30s       | 2           |
| Corn   | 3 / 8   | 100%   | 85%    | Ready        | 0           |
| Tomato | 12 / 15 | 45%    | 95%    | 5m 15s       | 1           |

### Column Descriptions

- **Crop**: Crop type name
- **Count**: Mature / Total planted
- **Growth**: Average growth progress percentage
- **Health**: Average health percentage
- **Harvest Time**: Time until first crop is ready (or "Ready")
- **Needs Water**: Number of crops needing water

## Color Coding

### Growth Progress

- Yellow (0-33%): Early growth
- Yellow-Green (33-66%): Mid growth
- Green (66-100%): Near mature
- Bright Green (100%): Mature/Ready

### Health Status

- Green (>66%): Healthy
- Yellow (33-66%): Fair condition
- Red (<33%): Poor health

### Water Status

- White: No crops need water
- Yellow: Some crops need water

## Integration with Farm Plot

The monitoring system automatically:

- Updates at configured interval (default 0.5s)
- Queries farm plot for crop data
- Calculates growth rates and harvest times
- Tracks environmental effects on crops

## Blueprint Usage

### Display Crop Monitoring

```cpp
// In Blueprint
Event Construct
{
    // Enable monitoring
    Set Show Crop Monitoring = true

    // Set farm plot
    Set Farm Plot (Farm Plot Reference)
}
```

### Filter by Crop Type

```cpp
// In Blueprint
On Crop Type Selected (Crop Definition)
{
    Set Crop Type Filter (Crop Definition)
}

On Show All Crops Button Clicked
{
    Clear Crop Type Filter
}
```

### Custom Monitoring Display

```cpp
// Get monitoring data in Blueprint
TArray<FCropMonitoringData> MonitoringData = Get All Crop Monitoring Data();

// Process each crop type
ForEach (MonitoringData as Data)
{
    // Create custom UI elements
    // Display Data.CropName, Data.TotalCount, etc.
}
```

## Performance Considerations

- Monitoring updates at `UpdateInterval` (default 0.5s)
- Limited to `MaxCropTypesDisplayed` types (default 10)
- Grid cells are created dynamically
- Efficient crop type collection using TSet

## Requirements Satisfied

This implementation satisfies:

- **Requirement 16.2**: Display growth progress percentage for all planted crops
- **Requirement 16.3**: Calculate and display estimated harvest time based on current conditions
- **Requirement 16.2**: Show crop health information with visual indicators
- **Requirement 16.3**: Provide alerts when crops need watering

## Example Scenarios

### Scenario 1: Monitor All Crops

```cpp
// Show all crops with their status
FarmStatusWidget->ClearCropTypeFilter();
FarmStatusWidget->UpdateCropMonitoring();
```

### Scenario 2: Focus on Specific Crop

```cpp
// Monitor only wheat crops
UCropDefinition* Wheat = LoadObject<UCropDefinition>(nullptr, TEXT("/Game/Crops/Wheat"));
FarmStatusWidget->SetCropTypeFilter(Wheat);
```

### Scenario 3: Check Harvest Readiness

```cpp
TArray<FCropMonitoringData> AllData = FarmStatusWidget->GetAllCropMonitoringData();

for (const FCropMonitoringData& Data : AllData)
{
    if (Data.MatureCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("%s: %d crops ready to harvest"),
               *Data.CropName, Data.MatureCount);
    }
}
```

### Scenario 4: Water Alert System

```cpp
TArray<FCropMonitoringData> AllData = FarmStatusWidget->GetAllCropMonitoringData();

int32 TotalNeedingWater = 0;
for (const FCropMonitoringData& Data : AllData)
{
    TotalNeedingWater += Data.CropsNeedingWater;
}

if (TotalNeedingWater > 0)
{
    // Show water alert notification
    ShowWaterAlert(TotalNeedingWater);
}
```

## Troubleshooting

### Monitoring Not Updating

- Check `bShowCropMonitoring` is true
- Verify `CropMonitoringGrid` widget is bound
- Ensure `UpdateInterval` is reasonable (0.1-1.0s)

### Missing Crop Types

- Check `MaxCropTypesDisplayed` limit
- Verify crops are actually planted in farm plot
- Ensure crop definitions are valid

### Incorrect Harvest Times

- Verify farm plot has `GetTimeToHarvest()` implemented
- Check crop growth duration is set correctly
- Ensure environmental conditions are being calculated

## Next Steps

After implementing crop monitoring:

1. Add sorting options (by name, growth, health)
2. Implement crop type icons in display
3. Add click handlers for crop selection
4. Create detailed crop info popup
5. Add export/logging of monitoring data
