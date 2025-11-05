# VR Harvesting System - Quick Start Guide

## Overview

The VR Harvesting System enables players to physically harvest mature crops using VR motion controllers with realistic grab interactions, detachment mechanics, and haptic feedback.

## Core Components

### 1. UVRHarvestableComponent

Makes individual crops harvestable via VR grab interaction.

**Key Features:**

- Harvest state management (NotReady, ReadyToHarvest, BeingGrabbed, Harvested)
- VR grab detection and interaction
- Pull-based detachment mechanics
- Haptic feedback for grab, detachment, and completion
- Visual feedback (highlight colors)
- Yield calculation based on growth and health

### 2. UVRHarvestInteractionManager

Manages VR hand interactions with harvestable crops across farm plots.

**Key Features:**

- Hand tracking and target detection
- Grab interaction coordination
- Harvest collection tracking
- Farm plot management
- Statistics and history

## Setup Instructions

### Step 1: Add Harvestable Component to Crops

When spawning crop instances in your farm plot:

```cpp
// In AFarmPlot::UpdateCropVisuals() or similar
void AFarmPlot::SpawnCropInstance(FIntPoint GridPosition, UCropDefinition* CropType)
{
    // Spawn crop actor
    AActor* CropActor = GetWorld()->SpawnActor<AActor>(CropActorClass);

    // Add harvestable component
    UVRHarvestableComponent* Harvestable = NewObject<UVRHarvestableComponent>(CropActor);
    CropActor->AddInstanceComponent(Harvestable);
    Harvestable->RegisterComponent();

    // Initialize harvestable
    FCropCell& Cell = CropGrid[GetCellIndex(GridPosition)];
    Harvestable->InitializeHarvestable(
        Cell.CropType,
        Cell.GrowthProgress,
        Cell.Health,
        this,
        GridPosition
    );
}
```

### Step 2: Add Interaction Manager to Player

Add the interaction manager to your VR player pawn:

```cpp
// In AVRPlayerPawn constructor or BeginPlay
UVRHarvestInteractionManager* HarvestManager = CreateDefaultSubobject<UVRHarvestInteractionManager>(TEXT("HarvestManager"));

// Initialize with VR hands
HarvestManager->InitializeHands(LeftHandActor, RightHandActor);
```

### Step 3: Configure Harvest Settings

In your crop definition or harvestable component:

```cpp
// Harvest thresholds
MinimumGrowthForHarvest = 0.95f; // 95% growth required

// Detachment settings
DetachmentTime = 0.5f; // Half second to detach
DetachmentForceThreshold = 50.0f;
bRequiresPullingMotion = true;
DetachmentDirection = FVector::UpVector; // Pull upward

// Grab settings
GrabRadius = 15.0f; // 15cm grab radius

// Visual feedback
ReadyToHarvestColor = FLinearColor(0.2f, 1.0f, 0.2f); // Green glow
GrabbedColor = FLinearColor(1.0f, 0.8f, 0.2f); // Yellow-orange
HighlightIntensity = 2.0f;
```

## Usage

### Harvesting Flow

1. **Crop Maturity**: Crop reaches 95%+ growth progress
2. **Ready State**: Harvestable component shows green highlight
3. **Player Approach**: VR hand enters grab radius (15cm)
4. **Grab Gesture**: Player presses grip button on motion controller
5. **Pull Motion**: Player pulls hand upward
6. **Detachment**: Progress accumulates based on pull force
7. **Harvest Complete**: Crop detaches, haptic feedback plays, item spawned
8. **Collection**: Harvested items added to inventory

### VR Interaction

```cpp
// In your VR input handling
void AVRPlayerPawn::OnGripPressed()
{
    // Try to grab with appropriate hand
    if (bIsLeftHand)
    {
        HarvestManager->TryGrabLeft();
    }
    else
    {
        HarvestManager->TryGrabRight();
    }
}

void AVRPlayerPawn::OnGripReleased()
{
    // Release grab
    if (bIsLeftHand)
    {
        HarvestManager->ReleaseLeft();
    }
    else
    {
        HarvestManager->ReleaseRight();
    }
}
```

### Updating Crop Growth

The harvestable component should be updated when crop growth changes:

```cpp
// In AFarmPlot::UpdateCropGrowth()
void AFarmPlot::UpdateCropGrowth(float DeltaTime)
{
    for (FCropCell& Cell : CropGrid)
    {
        if (Cell.CropType)
        {
            // Update growth
            Cell.GrowthProgress += GrowthRate * DeltaTime;

            // Update harvestable component
            if (UVRHarvestableComponent* Harvestable = FindHarvestableForCell(Cell))
            {
                Harvestable->UpdateGrowthProgress(Cell.GrowthProgress);
                Harvestable->UpdateCropHealth(Cell.Health);
            }
        }
    }
}
```

## Harvest Mechanics

### Detachment System

The detachment system uses pull force calculation:

1. **Hand Velocity**: Calculate velocity from hand position changes
2. **Directional Force**: Project velocity onto detachment direction (up)
3. **Force Accumulation**: Accumulate force over time
4. **Progress Calculation**: Convert accumulated force to detachment progress (0-1)
5. **Completion**: When progress reaches 1.0, harvest completes

### Yield Calculation

Harvest yield is calculated based on:

```cpp
int32 CalculateHarvestYield()
{
    int32 BaseYield = CropType->BaseYield; // From crop definition

    float GrowthModifier = Clamp(GrowthProgress, 0.5f, 1.0f);
    float HealthModifier = Clamp(CropHealth, 0.3f, 1.0f);

    int32 FinalYield = BaseYield * GrowthModifier * HealthModifier;

    return Max(1, FinalYield); // At least 1 item
}
```

### Quality Calculation

Harvest quality affects item value:

```cpp
float CalculateHarvestQuality()
{
    float GrowthQuality = Clamp(GrowthProgress, 0.0f, 1.0f);
    float HealthQuality = Clamp(CropHealth, 0.0f, 1.0f);

    float Quality = (GrowthQuality + HealthQuality) * 0.5f;

    // Bonus for perfect conditions
    if (GrowthQuality >= 0.99f && HealthQuality >= 0.95f)
    {
        Quality *= 1.1f; // 10% bonus
    }

    return Quality;
}
```

## Haptic Feedback

### Feedback Types

1. **Grab Haptic**: Light pulse when grabbing crop (0.5 intensity)
2. **Detachment Haptic**: Increasing intensity as detachment progresses (0.3-0.8)
3. **Harvest Complete Haptic**: Strong pulse when harvest completes (1.0 intensity)

### Configuration

```cpp
// Set haptic effects in Blueprint or C++
UPROPERTY(EditAnywhere, Category = "Haptics")
UHapticFeedbackEffect_Base* GrabHapticEffect;

UPROPERTY(EditAnywhere, Category = "Haptics")
UHapticFeedbackEffect_Base* DetachmentHapticEffect;

UPROPERTY(EditAnywhere, Category = "Haptics")
UHapticFeedbackEffect_Base* HarvestCompleteHapticEffect;

// Adjust intensity
HapticIntensityMultiplier = 1.0f; // 0-1 range
```

## Visual Feedback

### Highlight States

1. **Ready to Harvest**: Green glow (subtle)
2. **Being Grabbed**: Yellow-orange glow (pulsing with detachment progress)
3. **Not Ready**: No highlight

### Material Setup

The system expects materials with these parameters:

- `EmissiveColor` (Vector): Color of the glow
- `EmissiveIntensity` (Scalar): Intensity of the glow

## Harvest Collection

### Recording Harvests

```cpp
// Automatically recorded when harvest completes
FHarvestCollection Harvest;
Harvest.CropName = CropType->CropName;
Harvest.Quantity = YieldAmount;
Harvest.Quality = YieldQuality;
Harvest.HarvestTime = FDateTime::Now();

HarvestManager->RecordHarvest(Harvest);
```

### Statistics

```cpp
// Get harvest statistics
int32 TotalHarvested;
float AverageQuality;
HarvestManager->GetHarvestStatistics(TotalHarvested, AverageQuality);

// Get crop-specific stats
int32 TomatoesHarvested = HarvestManager->GetTotalHarvested("Tomato");
float TomatoQuality = HarvestManager->GetAverageQuality("Tomato");

// Get ready crop count
int32 ReadyCrops = HarvestManager->GetReadyCropCount();
```

## Integration with Farm Plot

### Harvest Notification

When a crop is harvested, the farm plot is notified:

```cpp
void UVRHarvestableComponent::NotifyFarmPlotHarvested()
{
    if (OwningFarmPlot)
    {
        // This calls AFarmPlot::HarvestCrop()
        OwningFarmPlot->HarvestCrop(GridPosition);
    }
}
```

The farm plot should handle this by:

1. Clearing the crop cell
2. Updating visual representation
3. Adjusting soil nutrients
4. Recording harvest data

## Performance Considerations

### Optimization Tips

1. **Limit Active Harvestables**: Only add harvestable components to mature crops
2. **Detection Radius**: Keep hand detection radius reasonable (15-30cm)
3. **Update Frequency**: Update hand targets every frame, but farm plots less frequently
4. **Instanced Meshes**: Use instanced static meshes for crop rendering
5. **LOD**: Disable harvestable interaction for distant crops

### Memory Management

```cpp
// Remove harvestable component when crop is harvested
void UVRHarvestableComponent::CompleteHarvest()
{
    // ... harvest logic ...

    // Destroy crop actor (and component)
    if (AActor* Owner = GetOwner())
    {
        Owner->Destroy();
    }
}
```

## Troubleshooting

### Crops Not Grabbable

- Check `MinimumGrowthForHarvest` threshold
- Verify crop growth progress is >= threshold
- Ensure `bIsGrabbable` is true
- Check `HarvestState` is `ReadyToHarvest`

### Detachment Not Working

- Verify `DetachmentDirection` is set correctly
- Check `DetachmentForceThreshold` isn't too high
- Ensure hand velocity is being calculated
- Verify `bRequiresPullingMotion` setting

### No Haptic Feedback

- Check haptic effects are assigned
- Verify `HapticIntensityMultiplier` > 0
- Ensure player controller is valid
- Check VR input system is active

### Visual Highlight Not Showing

- Verify material has `EmissiveColor` and `EmissiveIntensity` parameters
- Check `CropMaterialInstance` is created
- Ensure `HighlightIntensity` > 0
- Verify material supports emissive

## Testing

### Test Scenarios

1. **Basic Harvest**: Grab and pull mature crop
2. **Immature Crop**: Try to grab crop below threshold (should fail)
3. **Multiple Crops**: Harvest several crops in sequence
4. **Both Hands**: Use both hands simultaneously
5. **Quality Variation**: Harvest crops with different health levels
6. **Statistics**: Verify harvest tracking and statistics

### Debug Commands

```cpp
// Enable debug visualization
ShowDebug VRHarvest

// Log harvest events
LogVRHarvest Verbose

// Test harvest without VR
HarvestCrop <GridX> <GridY>
```

## Requirements Satisfied

This implementation satisfies **Requirement 13.4**:

- ✅ VR grab for mature crops
- ✅ Crop detachment logic with pull mechanics
- ✅ Harvest collection system with tracking
- ✅ Haptic feedback for harvesting actions

## Next Steps

1. Integrate with inventory system for harvested items
2. Add visual effects (particles) for harvest
3. Implement sound effects for different crop types
4. Add tutorial/guidance for first-time harvesting
5. Create UI for harvest statistics display
6. Add achievements for harvest milestones
