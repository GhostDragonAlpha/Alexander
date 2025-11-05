# Farm Status UI - Quick Start Guide

## Overview

The Farm Status UI Widget provides real-time visualization of farm plot conditions including soil quality, water levels, fertility, nutrient levels (NPK), and a visual crop grid showing the status of all planted crops.

## Features

- **Soil Quality Display**: Shows overall soil quality as percentage and progress bar
- **Water Level Display**: Displays current water level with visual indicator
- **Fertility Display**: Shows soil fertility that depletes with crop growth
- **Nutrient Levels (NPK)**: Detailed display of Nitrogen, Phosphorus, and Potassium levels
- **Crop Grid Visualization**: Color-coded grid showing status of each crop cell
- **Real-time Statistics**: Planted cells, mature crops, crops needing water, average health
- **Auto-updating**: Configurable update interval for real-time monitoring

## Quick Setup

### 1. Create Widget Blueprint

1. In Content Browser, right-click → User Interface → Widget Blueprint
2. Name it `WBP_FarmStatus`
3. Open the widget blueprint
4. Set parent class to `FarmStatusWidget` (C++ class)

### 2. Design the UI Layout

Add these widgets with the exact names (required for binding):

#### Soil Quality Section

- `SoilQualityText` (TextBlock) - Displays percentage
- `SoilQualityBar` (ProgressBar) - Visual bar

#### Water Level Section

- `WaterLevelText` (TextBlock) - Displays percentage
- `WaterLevelBar` (ProgressBar) - Visual bar

#### Fertility Section

- `FertilityText` (TextBlock) - Displays percentage
- `FertilityBar` (ProgressBar) - Visual bar

#### Nutrient Levels Section (Optional)

- `NitrogenText` (TextBlock) - N level percentage
- `NitrogenBar` (ProgressBar) - N visual bar
- `PhosphorusText` (TextBlock) - P level percentage
- `PhosphorusBar` (ProgressBar) - P visual bar
- `PotassiumText` (TextBlock) - K level percentage
- `PotassiumBar` (ProgressBar) - K visual bar

#### Statistics Section

- `PlantedCellsText` (TextBlock) - Shows planted/total cells
- `MatureCropsText` (TextBlock) - Number of mature crops
- `CropsNeedingWaterText` (TextBlock) - Crops needing water
- `AverageHealthText` (TextBlock) - Average crop health

#### Crop Grid Visualization

- `CropGridPanel` (CanvasPanel) - Container for grid visualization

### 3. Configure Widget Settings

In the widget's Details panel, configure:

```
Update Interval: 0.5 (seconds between updates)
Show Nutrient Details: true (show NPK levels)
Show Crop Grid: true (show visual grid)
Grid Cell Size: 10.0 (pixels per cell)
Grid Cell Spacing: 2.0 (pixels between cells)
```

#### Color Configuration

```
Healthy Crop Color: Green (0, 1, 0)
Needs Water Color: Yellow (1, 1, 0)
Unhealthy Crop Color: Red (1, 0, 0)
Empty Cell Color: Dark Gray (0.3, 0.3, 0.3)
Mature Crop Color: Bright Green (0, 1, 0.5)
```

## Usage in Blueprints

### Display Farm Status

```cpp
// Get or create the widget
UFarmStatusWidget* StatusWidget = CreateWidget<UFarmStatusWidget>(GetWorld(), FarmStatusWidgetClass);

// Bind to a farm plot
StatusWidget->SetFarmPlot(MyFarmPlot);

// Add to viewport
StatusWidget->AddToViewport();
```

### Update Manually

```cpp
// Force immediate update
StatusWidget->UpdateStatus();

// Update specific sections
StatusWidget->UpdateSoilQualityDisplay();
StatusWidget->UpdateWaterLevelDisplay();
StatusWidget->UpdateCropGridVisualization();
```

### Get Statistics

```cpp
// Get current farm statistics
FFarmPlotStatistics Stats = StatusWidget->CalculateStatistics();

UE_LOG(LogTemp, Log, TEXT("Planted: %d/%d"), Stats.PlantedCells, Stats.TotalCells);
UE_LOG(LogTemp, Log, TEXT("Mature: %d"), Stats.MatureCrops);
UE_LOG(LogTemp, Log, TEXT("Need Water: %d"), Stats.CropsNeedingWater);
UE_LOG(LogTemp, Log, TEXT("Avg Health: %.1f%%"), Stats.AverageHealth * 100.0f);
```

## Grid Visualization

The crop grid uses color coding to show crop status at a glance:

- **Dark Gray**: Empty cell (no crop planted)
- **Yellow-Green Gradient**: Growing crop (yellow = young, green = mature)
- **Bright Green**: Mature crop ready for harvest
- **Yellow**: Crop needs water
- **Red**: Unhealthy crop (health < 50%)

### Grid Cell Status

```cpp
// Get status for specific grid position
FFarmGridCellStatus CellStatus = StatusWidget->GetGridCellStatus(FIntPoint(5, 3));

if (CellStatus.bHasCrop)
{
    UE_LOG(LogTemp, Log, TEXT("Crop: %s"), *CellStatus.CropName);
    UE_LOG(LogTemp, Log, TEXT("Growth: %.1f%%"), CellStatus.GrowthProgress * 100.0f);
    UE_LOG(LogTemp, Log, TEXT("Health: %.1f%%"), CellStatus.Health * 100.0f);
    UE_LOG(LogTemp, Log, TEXT("Needs Water: %s"), CellStatus.bNeedsWater ? TEXT("Yes") : TEXT("No"));
}
```

## Example UI Layout

```
┌─────────────────────────────────────────┐
│         Farm Plot Status                │
├─────────────────────────────────────────┤
│ Soil Quality:  [████████░░] 85%         │
│ Water Level:   [██████░░░░] 60%         │
│ Fertility:     [███████░░░] 72%         │
├─────────────────────────────────────────┤
│ Nutrients (NPK)                         │
│ Nitrogen:      [█████░░░░░] 50%         │
│ Phosphorus:    [███████░░░] 70%         │
│ Potassium:     [████████░░] 80%         │
├─────────────────────────────────────────┤
│ Statistics                              │
│ Planted: 45/64                          │
│ Mature Crops: 12                        │
│ Need Water: 8                           │
│ Avg Health: 87%                         │
├─────────────────────────────────────────┤
│ Crop Grid (8x8)                         │
│ ┌──────────────────┐                    │
│ │▓▓▓▓▓▓▓▓          │                    │
│ │▓▓▓▓▓▓▓▓          │                    │
│ │▓▓▓▓▓▓▓▓          │                    │
│ │▓▓▓▓▓▓▓▓          │                    │
│ │▓▓▓▓▓▓▓▓          │                    │
│ │▓▓▓▓▓▓▓▓          │                    │
│ │                  │                    │
│ │                  │                    │
│ └──────────────────┘                    │
└─────────────────────────────────────────┘
```

## VR Integration

For VR display, consider:

1. **3D Widget Component**: Attach to player or world space
2. **Scale**: Adjust for comfortable viewing distance
3. **Update Rate**: Lower update interval (1.0s) to reduce overhead
4. **Simplified View**: Hide nutrient details for cleaner display

```cpp
// Create 3D widget for VR
UWidgetComponent* WidgetComp = NewObject<UWidgetComponent>(this);
WidgetComp->SetWidgetClass(FarmStatusWidgetClass);
WidgetComp->SetDrawSize(FVector2D(800, 600));
WidgetComp->SetWorldLocation(FarmPlot->GetActorLocation() + FVector(0, 0, 200));

// Get the widget and bind to farm
UFarmStatusWidget* StatusWidget = Cast<UFarmStatusWidget>(WidgetComp->GetUserWidgetObject());
if (StatusWidget)
{
    StatusWidget->SetFarmPlot(FarmPlot);
    StatusWidget->UpdateInterval = 1.0f; // Slower updates for VR
    StatusWidget->bShowNutrientDetails = false; // Simplified view
}
```

## Performance Considerations

- **Update Interval**: Default 0.5s is good for active monitoring. Increase to 1-2s for background farms
- **Grid Visualization**: Disable for large farms (>16x16) to reduce widget complexity
- **Nutrient Details**: Hide if not needed to simplify UI
- **Multiple Widgets**: Limit to 3-5 active farm status widgets simultaneously

## Troubleshooting

### Widget Not Updating

- Check that `FarmPlot` is set via `SetFarmPlot()`
- Verify widget is added to viewport
- Ensure `UpdateInterval` is > 0

### Grid Not Showing

- Verify `bShowCropGrid` is true
- Check that `CropGridPanel` is bound correctly
- Ensure `GridCellSize` and `GridCellSpacing` are reasonable values

### Colors Not Showing

- Verify progress bars have proper fill color settings
- Check that text blocks support color changes
- Ensure color properties are set in widget configuration

## Integration with Other Systems

### Irrigation System (Task 16.1)

- Water level updates automatically when irrigation is active
- Visual indicator shows when water is being added

### Crop Growth (Task 13.2)

- Growth progress updates in real-time
- Grid visualization shows growth stages with color gradient

### Soil System (Task 12.4)

- Soil quality reflects all soil factors (nutrients, pH, compaction)
- Nutrient bars show depletion as crops grow

### Weather Integration (Task 13.4)

- Water level increases during rain
- Statistics update to show weather effects on crops

## Next Steps

- Create VR-specific farm status display (Task 17.2)
- Add alert notifications for critical conditions
- Implement historical data tracking and graphs
- Add crop-specific detail view on grid cell hover/selection
