# Crop Inspection System - Quick Start Guide

## Overview

The VR Crop Inspection System allows players to look at crops and see detailed information including health status, growth progress, and any issues requiring attention. The system uses look-at detection with a 2-meter range and displays a comprehensive UI overlay.

## Core Components

### 1. VRCropInspectionComponent

- **Purpose**: Detects when player looks at crops and manages UI display
- **Location**: `Source/Alexander/Public/VRCropInspectionComponent.h`
- **Key Features**:
  - Look-at detection with configurable range (default: 2m)
  - Angle tolerance for detection (default: 15°)
  - Dwell time before showing info (default: 0.5s)
  - Continuous updates while looking at crop

### 2. VRCropInfoWidget

- **Purpose**: UI widget displaying crop information
- **Location**: `Source/Alexander/Public/VRCropInfoWidget.h`
- **Displays**:
  - Crop name and growth stage
  - Growth progress percentage with color-coded bar
  - Health status with color indicators
  - Primary issues (water deficit, temperature stress, etc.)
  - Warning icons for needs (water, fertilizer)
  - Time until harvest

## Quick Setup

### Step 1: Add Component to VR Pawn

```cpp
// In your VR pawn header
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Farming")
UVRCropInspectionComponent* CropInspectionComponent;

// In constructor
CropInspectionComponent = CreateDefaultSubobject<UVRCropInspectionComponent>(TEXT("CropInspection"));
```

### Step 2: Create UI Widget Blueprint

1. Create a new Widget Blueprint based on `UVRCropInfoWidget`
2. Add the following UI elements (must match binding names):
   - **CropNameText** (TextBlock) - Displays crop name
   - **GrowthStageText** (TextBlock) - Shows current growth stage
   - **GrowthProgressBar** (ProgressBar) - Visual growth progress
   - **GrowthProgressText** (TextBlock) - Growth percentage
   - **HealthStatusText** (TextBlock) - Health status description
   - **HealthProgressBar** (ProgressBar) - Visual health indicator
   - **HealthPercentageText** (TextBlock) - Health percentage
   - **PrimaryIssueText** (TextBlock) - Main problem affecting crop
   - **TimeToHarvestText** (TextBlock) - Estimated time to maturity
   - **NeedsWaterIcon** (Image) - Water warning indicator
   - **NeedsFertilizerIcon** (Image) - Fertilizer warning indicator
   - **WarningsContainer** (VerticalBox) - Container for warnings

### Step 3: Configure Component

```cpp
// Set widget class
CropInspectionComponent->CropInfoWidgetClass = YourWidgetBlueprintClass;

// Optional: Adjust detection parameters
CropInspectionComponent->InspectionRange = 200.0f; // 2 meters in cm
CropInspectionComponent->LookAtAngleTolerance = 15.0f; // degrees
CropInspectionComponent->LookAtDuration = 0.5f; // seconds

// Optional: Enable debug visualization
CropInspectionComponent->bShowDebugVisualization = true;
```

## Usage

### Automatic Operation

The system works automatically once configured:

1. Player looks at a crop within 2 meters
2. After 0.5 seconds of looking, info UI appears
3. UI updates continuously while looking at crop
4. UI hides when player looks away

### Manual Control (Blueprint/C++)

```cpp
// Get inspection data for specific crop
FCropInspectionData Data = CropInspectionComponent->GetCropInspectionData(FarmPlot, GridPosition);

// Manually show/hide info
CropInspectionComponent->ShowCropInfo(Data);
CropInspectionComponent->HideCropInfo();

// Check if currently looking at crop
bool bLooking = CropInspectionComponent->bIsLookingAtCrop;
```

## Displayed Information

### Growth Information

- **Crop Name**: Type of crop (e.g., "Wheat", "Tomato")
- **Growth Stage**: Current stage name (e.g., "Seedling", "Mature")
- **Progress**: Percentage complete (0-100%)
- **Progress Bar**: Color-coded visual indicator
  - Red (0-50%): Early growth
  - Yellow (50-75%): Mid growth
  - Green (75-100%): Near maturity

### Health Information

- **Health Status**: Descriptive status
  - Excellent (90-100%)
  - Good (70-90%)
  - Fair (50-70%)
  - Poor (30-50%)
  - Critical (10-30%)
  - Dying (0-10%)
- **Health Percentage**: Numeric health value
- **Health Bar**: Color-coded to match status
- **Primary Issue**: Main problem affecting crop
  - "Water Deficit"
  - "Temperature Stress"
  - "Nutrient Deficiency"
  - "Pest Damage"
  - "Disease"
  - "None" (healthy)

### Warnings

- **Water Icon**: Shown when crop needs watering
- **Fertilizer Icon**: Shown when soil needs nutrients

### Harvest Timing

- **Time to Harvest**: Estimated time until ready
  - Format: "Xs", "Xm Ys", "Xh Ym"
  - Shows "Ready to Harvest!" when mature

## Configuration Options

### Detection Parameters

```cpp
// Range in centimeters (200cm = 2m)
InspectionRange = 200.0f;

// Angle tolerance in degrees (15° = narrow cone)
LookAtAngleTolerance = 15.0f;

// Dwell time before showing UI (seconds)
LookAtDuration = 0.5f;
```

### Debug Visualization

Enable to see detection system in action:

```cpp
bShowDebugVisualization = true;
```

Shows:

- Cyan sphere: Detection range
- Green line: Look direction
- Yellow cone: Look-at tolerance cone
- Red sphere: Currently inspected crop
- Red line: Connection to inspected crop

## Integration with Existing Systems

### Health System Integration

- Uses `UCropHealthSystem` for health calculations
- Displays health status with color coding
- Shows primary health issues
- Indicates needs (water, fertilizer, pest control)

### Growth System Integration

- Uses `CropGrowthStageManager` for stage information
- Displays current growth stage name
- Shows progress within overall growth cycle
- Calculates time to harvest based on current conditions

### Farm Plot Integration

- Automatically finds nearby farm plots
- Detects crops in grid cells
- Retrieves environmental conditions
- Accesses crop cell data

## UI Customization

### Widget Styling

The widget can be fully customized in Blueprint:

1. **Layout**: Arrange elements as desired
2. **Colors**: Customize text and bar colors
3. **Fonts**: Change text styles and sizes
4. **Icons**: Replace warning icons with custom images
5. **Animations**: Add fade in/out transitions
6. **Background**: Add panel backgrounds or borders

### Example UI Layout

```
┌─────────────────────────────┐
│  🌾 Wheat                   │
│  Stage: Flowering           │
│  ▓▓▓▓▓▓▓▓░░ 75%            │
│                             │
│  Health: Good               │
│  ▓▓▓▓▓▓▓▓░░ 85%            │
│                             │
│  Issue: Water Deficit       │
│  💧 Needs Water             │
│                             │
│  Harvest in: 5m 30s         │
└─────────────────────────────┘
```

## Performance Considerations

### Optimization Features

- Only checks nearby farm plots (within range)
- Skips empty crop cells
- Updates UI only when visible
- Uses efficient distance checks
- Minimal per-frame overhead

### Best Practices

- Keep inspection range reasonable (2-3m)
- Use appropriate angle tolerance (10-20°)
- Limit number of farm plots in scene
- Consider LOD for distant farms

## Troubleshooting

### UI Not Showing

1. Check widget class is assigned
2. Verify widget elements are bound correctly
3. Ensure player has camera component
4. Check inspection range is sufficient
5. Enable debug visualization to see detection

### Incorrect Information

1. Verify farm plot reference is valid
2. Check crop cell has valid crop type
3. Ensure environmental systems are active
4. Verify health/growth systems are updating

### Performance Issues

1. Reduce inspection range
2. Decrease update frequency
3. Optimize UI complexity
4. Limit number of farm plots
5. Disable debug visualization

## Example Usage Scenarios

### Scenario 1: Checking Crop Health

```
Player looks at wilting crop
→ UI shows "Health: Poor (35%)"
→ Primary Issue: "Water Deficit"
→ Water icon displayed
→ Player knows to water the crop
```

### Scenario 2: Monitoring Growth

```
Player inspects young crop
→ UI shows "Stage: Seedling"
→ Progress: 25%
→ Harvest in: 15m 30s
→ Player knows when to return
```

### Scenario 3: Identifying Problems

```
Player checks struggling crop
→ UI shows "Health: Critical (15%)"
→ Primary Issue: "Temperature Stress"
→ Player realizes biome is unsuitable
```

## Future Enhancements

Potential additions:

- Pest/disease visual indicators
- Yield prediction based on health
- Comparison with optimal conditions
- Historical growth data
- Crop recommendations
- Voice feedback for VR
- Haptic feedback on detection

## Related Systems

- **CropHealthSystem**: Provides health calculations
- **CropGrowthCalculator**: Calculates growth rates
- **CropGrowthStageManager**: Manages growth stages
- **FarmPlot**: Contains crop data
- **BiomeCompatibilitySystem**: Affects crop suitability
- **WeatherComponent**: Influences crop conditions

## Testing

### Manual Testing

1. Create a farm plot with crops
2. Add inspection component to VR pawn
3. Look at crops from various distances
4. Verify UI appears after dwell time
5. Check all information displays correctly
6. Test with crops in different states

### Debug Commands

```cpp
// Enable debug visualization
CropInspectionComponent->bShowDebugVisualization = true;

// Check current state
UE_LOG(LogTemp, Log, TEXT("Looking at crop: %s"),
    CropInspectionComponent->bIsLookingAtCrop ? TEXT("Yes") : TEXT("No"));
```

## Summary

The Crop Inspection System provides an intuitive VR interface for monitoring crop health and growth. By simply looking at crops, players receive comprehensive information to make informed farming decisions. The system integrates seamlessly with existing health and growth systems while maintaining good performance.
