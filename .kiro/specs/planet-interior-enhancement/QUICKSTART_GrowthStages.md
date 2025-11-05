# Quick Start: Crop Growth Stage System

## Overview

The Growth Stage System provides visual progression for crops as they grow from seed to maturity. Each crop can have 4-8 distinct growth stages with unique meshes, scales, and colors.

## Key Components

### 1. CropGrowthStageManager

Central manager for growth stage logic and transitions.

**Location**: `Source/Alexander/Public/CropGrowthStageManager.h`

**Key Features**:

- Generate default growth stages (4-8 stages)
- Track stage transitions
- Calculate stage-specific visuals (mesh, scale, color)
- Validate stage configurations

### 2. Growth Stage Structure

```cpp
struct FGrowthStage
{
    FString StageName;          // "Seed", "Sprout", "Seedling", etc.
    float MinProgress;          // 0-1, when stage starts
    float MaxProgress;          // 0-1, when stage ends
    UStaticMesh* StageMesh;     // Visual mesh for this stage
    FVector ScaleMultiplier;    // Scale adjustment
    FLinearColor TintColor;     // Color tint
};
```

### 3. Crop Cell Tracking

Each crop cell in a farm plot tracks:

- `GrowthProgress` (0-1): Overall growth completion
- `CurrentGrowthStage`: Current stage index
- `LastGrowthProgress`: For detecting stage changes

## Usage Examples

### Generate Default Stages for a Crop

```cpp
// In CropDefinition or FarmPlot
UCropDefinition* CropDef = GetCropDefinition();
TArray<FGrowthStage> Stages = UCropGrowthStageManager::GenerateDefaultStages(CropDef, 4);
```

### Get Current Stage Information

```cpp
// Get stage index
int32 StageIndex = UCropGrowthStageManager::GetCurrentStageIndex(GrowthProgress, Stages);

// Get stage details
FGrowthStage CurrentStage = UCropGrowthStageManager::GetStageByProgress(GrowthProgress, Stages);

// Get stage name
FString StageName = UCropGrowthStageManager::GetStageName(GrowthProgress, Stages);
```

### Check for Stage Changes

```cpp
// During growth update
float OldProgress = Cell.LastGrowthProgress;
float NewProgress = Cell.GrowthProgress;

if (UCropGrowthStageManager::HasStageChanged(OldProgress, NewProgress, Stages))
{
    // Stage changed - update visuals, play effects, etc.
    int32 NewStage = UCropGrowthStageManager::GetCurrentStageIndex(NewProgress, Stages);
    OnCropStageChanged(Cell, OldStage, NewStage);
}
```

### Get Visual Properties

```cpp
// Get mesh for current stage
UStaticMesh* Mesh = UCropGrowthStageManager::GetStageMesh(GrowthProgress, Stages);

// Get scale for current stage (smoothly interpolated)
FVector Scale = UCropGrowthStageManager::GetStageScale(GrowthProgress, Stages);

// Get color for current stage (smoothly interpolated)
FLinearColor Color = UCropGrowthStageManager::GetStageColor(GrowthProgress, Stages);
```

### Track Progress Within Stage

```cpp
// Get progress within current stage (0-1)
float StageProgress = UCropGrowthStageManager::GetProgressWithinStage(GrowthProgress, Stages);

// Get percentage to next stage
float Percentage = UCropGrowthStageManager::GetPercentageToNextStage(GrowthProgress, Stages);

// Estimate time to next stage
float TimeToNext = UCropGrowthStageManager::EstimateTimeToNextStage(
    GrowthProgress,
    GrowthRate,
    Stages
);
```

## Default Growth Stages

The system generates these default stages:

1. **Seed** (0-25%): Small, brown, minimal mesh
2. **Sprout** (25-50%): Tiny green shoots emerging
3. **Seedling** (50-75%): Young plant with first leaves
4. **Vegetative** (75-87.5%): Growing plant with full foliage
5. **Budding** (87.5-93.75%): Flower buds forming (if applicable)
6. **Flowering** (93.75-96.875%): Flowers blooming
7. **Fruiting** (96.875-98.4375%): Fruit/produce developing
8. **Mature** (98.4375-100%): Ready for harvest

_Note: Number of stages depends on configuration (4-8 stages)_

## Integration with FarmPlot

The FarmPlot automatically:

- Tracks current growth stage for each crop
- Detects stage transitions during growth updates
- Updates visuals when stages change
- Logs stage progression events

### Stage Change Events

When a crop advances to a new stage:

```cpp
void AFarmPlot::OnCropStageChanged(FCropCell& Cell, int32 OldStage, int32 NewStage)
{
    // Log the change
    UE_LOG(LogTemp, Log, TEXT("Crop advanced to stage %d"), NewStage);

    // Trigger effects (add your custom logic here)
    // - Play growth sound
    // - Show particle effect
    // - Update UI notification
    // - Trigger achievement
}
```

## Visual Updates

The system provides smooth visual transitions:

### Mesh Swapping

- Each stage can have a unique static mesh
- Meshes swap instantly when stage changes
- Falls back to default crop mesh if stage mesh not available

### Scale Progression

- Scale smoothly interpolates between stages
- Micro-scaling within each stage (up to 15% growth)
- Prevents sudden size jumps

### Color Transitions

- Colors smoothly blend between stages
- Health affects final color (green = healthy, brown = unhealthy)
- Stage colors: brown (seed) → light green → vibrant green (mature)

## Configuration in CropDefinition

Add growth stage meshes to your crop definitions:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
TArray<UStaticMesh*> GrowthStageMeshes; // 4-6 stages
```

**Best Practices**:

- Provide 4-6 meshes per crop for optimal visual progression
- Ensure meshes are properly scaled (stage 1 smallest, final stage largest)
- Use consistent pivot points across all stage meshes
- Test stage transitions in-game for smooth appearance

## Querying Growth Stage Information

### From Blueprint or C++

```cpp
// Get current stage index
int32 Stage = FarmPlot->GetCropGrowthStage(FIntPoint(X, Y));

// Get stage name for display
FString StageName = FarmPlot->GetCropGrowthStageName(FIntPoint(X, Y));

// Get progress within current stage
float Progress = FarmPlot->GetProgressWithinGrowthStage(FIntPoint(X, Y));
```

### Display in UI

```cpp
// Example UI text
FString DisplayText = FString::Printf(
    TEXT("Stage: %s (%.1f%% to next stage)"),
    *StageName,
    Progress * 100.0f
);
```

## Performance Considerations

- Visual updates occur every 1 second (configurable)
- Immediate update when stage changes
- Instanced static meshes for efficient rendering
- Batch processing for multiple crops

## Validation

Validate stage configuration:

```cpp
FString ErrorMessage;
bool bValid = UCropGrowthStageManager::ValidateStages(Stages, ErrorMessage);

if (!bValid)
{
    UE_LOG(LogTemp, Error, TEXT("Invalid stages: %s"), *ErrorMessage);

    // Auto-fix stage ranges
    Stages = UCropGrowthStageManager::FixStageRanges(Stages);
}
```

## Debugging

Enable verbose logging to track stage changes:

```cpp
// In FarmPlot.cpp, UpdateCropGrowth()
UE_LOG(LogTemp, Verbose, TEXT("Crop growth: %.1f%% (Stage %d: %s)"),
    Cell.GrowthProgress * 100.0f,
    Cell.CurrentGrowthStage,
    *Cell.CropType->GetGrowthStageName(Cell.GrowthProgress)
);
```

## Future Enhancements

Potential additions to the growth stage system:

- Animated transitions between stages
- Stage-specific particle effects
- Sound effects for stage changes
- Weather-dependent stage variations
- Seasonal stage modifications
- Disease/pest visual stages

## Related Systems

- **CropGrowthCalculator**: Calculates growth rates based on environment
- **FarmPlot**: Manages crop grid and visual updates
- **CropDefinition**: Defines crop properties and stage meshes
- **FarmingSubsystem**: Coordinates all farming operations

## Testing

Test the growth stage system:

1. Plant a crop in a farm plot
2. Observe growth progression through stages
3. Check console logs for stage change events
4. Verify smooth visual transitions
5. Test with different numbers of stages (4, 6, 8)
6. Validate stage meshes display correctly

## Summary

The Growth Stage System provides:

- ✅ 4-8 visual growth stages per crop
- ✅ Smooth stage transitions
- ✅ Progress tracking within stages
- ✅ Stage-based mesh swapping
- ✅ Automatic visual updates
- ✅ Event notifications for stage changes
- ✅ Performance-optimized rendering
