# Task 15.3 Implementation: VR Harvesting Interaction

## Overview

Implemented a comprehensive VR harvesting interaction system that enables players to physically harvest mature crops using motion controllers with realistic grab mechanics, pull-based detachment, and haptic feedback.

## Implementation Date

November 4, 2025

## Components Implemented

### 1. UVRHarvestableComponent

**File**: `Source/Alexander/Public/VRHarvestableComponent.h`
**File**: `Source/Alexander/Private/VRHarvestableComponent.cpp`

A component that makes individual crops harvestable via VR grab interaction.

**Key Features**:

- Harvest state management (NotReady, ReadyToHarvest, BeingGrabbed, Harvested)
- Growth progress and health tracking
- VR grab detection with configurable radius (default 15cm)
- Pull-based detachment mechanics with force calculation
- Haptic feedback system (grab, detachment progress, completion)
- Visual feedback with highlight colors (green for ready, yellow-orange for grabbed)
- Yield and quality calculation based on crop conditions
- Farm plot integration for harvest notification

**Key Properties**:

```cpp
- HarvestState: Current harvest state
- MinimumGrowthForHarvest: Growth threshold (default 0.95)
- GrabRadius: Detection radius for VR hands
- DetachmentForceThreshold: Force required to detach
- DetachmentTime: Time to fully detach
- bRequiresPullingMotion: Enable pull-based harvesting
- DetachmentDirection: Direction to pull (default up)
- HapticEffects: Grab, detachment, and completion haptics
- Visual colors: ReadyToHarvestColor, GrabbedColor
```

**Key Methods**:

```cpp
- InitializeHarvestable(): Setup with crop data
- UpdateGrowthProgress(): Update when crop grows
- UpdateCropHealth(): Update when health changes
- TryGrab(): Attempt VR grab
- Release(): Release grab
- UpdateGrabInteraction(): Update during grab
- CalculatePullForce(): Calculate force from hand velocity
- ApplyPullForce(): Apply force to detachment progress
- CompleteHarvest(): Finalize harvest and spawn items
- CalculateHarvestYield(): Calculate yield amount
- CalculateHarvestQuality(): Calculate quality (0-1)
- PlayHapticFeedback(): Trigger haptic effects
- UpdateVisualFeedback(): Update highlight colors
```

### 2. UVRHarvestInteractionManager

**File**: `Source/Alexander/Public/VRHarvestInteractionManager.h`
**File**: `Source/Alexander/Private/VRHarvestInteractionManager.cpp`

A manager component that coordinates VR hand interactions with harvestable crops across multiple farm plots.

**Key Features**:

- VR hand tracking (left and right)
- Automatic target detection for nearby harvestable crops
- Grab interaction coordination
- Harvest collection tracking with history
- Daily statistics (total harvested, average quality)
- Farm plot registration and management
- Proximity indicators for UI feedback

**Key Properties**:

```cpp
- LeftHand/RightHand: VR hand actor references
- HandDetectionRadius: Radius to detect crops (default 30cm)
- LeftHandTarget/RightHandTarget: Current harvestable targets
- bLeftHandGrabbing/bRightHandGrabbing: Grab states
- HarvestHistory: Collection of harvest records
- MaxHistorySize: Maximum history entries (default 100)
- TotalHarvestedToday: Daily harvest count
- AverageQualityToday: Daily average quality
- NearbyFarmPlots: Registered farm plots
- FarmPlotDetectionRadius: Farm plot detection range (default 10m)
```

**Key Methods**:

```cpp
- InitializeHands(): Setup VR hand references
- RegisterFarmPlot()/UnregisterFarmPlot(): Manage farm plots
- UpdateHandTargets(): Find nearby harvestable crops
- FindHarvestableNearHand(): Get closest harvestable to hand
- TryGrabLeft()/TryGrabRight(): Attempt grab with hand
- ReleaseLeft()/ReleaseRight(): Release grab
- UpdateGrabInteractions(): Update grab state
- RecordHarvest(): Add harvest to history
- GetTotalHarvested(): Get total for crop type
- GetAverageQuality(): Get average quality for crop type
- GetHarvestStatistics(): Get overall statistics
- GetAllHarvestableCrops(): Get all harvestables in range
- GetReadyCropCount(): Count ready-to-harvest crops
- UpdateDailyStatistics(): Reset daily stats
```

### 3. FHarvestCollection Struct

**Purpose**: Data structure for tracking individual harvest events.

**Properties**:

```cpp
- CropName: Name of harvested crop
- Quantity: Number of items harvested
- Quality: Quality rating (0-1)
- HarvestTime: Timestamp of harvest
```

### 4. EHarvestState Enum

**Purpose**: Defines the current state of a harvestable crop.

**Values**:

- `NotReady`: Crop not mature enough to harvest
- `ReadyToHarvest`: Crop is mature and can be grabbed
- `BeingGrabbed`: Player is currently grabbing the crop
- `Harvested`: Crop has been successfully harvested

## Harvest Mechanics

### Detachment System

The system uses a physics-based pull mechanic:

1. **Hand Velocity Calculation**: Track hand position changes to calculate velocity
2. **Directional Force**: Project velocity onto detachment direction (typically upward)
3. **Force Accumulation**: Accumulate force over time as player pulls
4. **Progress Tracking**: Convert accumulated force to detachment progress (0-1)
5. **Completion**: When progress reaches 1.0, harvest completes

**Formula**:

```cpp
DirectionalVelocity = DotProduct(HandVelocity, DetachmentDirection)
Force = Max(0, DirectionalVelocity) * ScaleFactor
AccumulatedForce += Force * DeltaTime
DetachmentProgress = Clamp((AccumulatedForce - Threshold) / (Threshold * 0.5), 0, 1)
```

### Yield Calculation

Harvest yield is calculated based on crop conditions:

```cpp
BaseYield = CropType->BaseYield
GrowthModifier = Clamp(GrowthProgress, 0.5, 1.0)
HealthModifier = Clamp(CropHealth, 0.3, 1.0)
FinalYield = Max(1, BaseYield * GrowthModifier * HealthModifier)
```

**Factors**:

- Base yield from crop definition
- Growth progress (50-100% range)
- Crop health (30-100% range)
- Minimum yield of 1 item

### Quality Calculation

Harvest quality affects item value:

```cpp
GrowthQuality = Clamp(GrowthProgress, 0, 1)
HealthQuality = Clamp(CropHealth, 0, 1)
Quality = (GrowthQuality + HealthQuality) * 0.5

// Bonus for perfect conditions
if (GrowthQuality >= 0.99 && HealthQuality >= 0.95)
    Quality *= 1.1 // 10% bonus
```

## Haptic Feedback System

### Feedback Types

1. **Grab Haptic**

   - Triggered when player grabs crop
   - Intensity: 0.5 (medium)
   - Purpose: Confirm grab action

2. **Detachment Haptic**

   - Triggered during pull motion
   - Intensity: 0.3-0.8 (increases with progress)
   - Purpose: Provide feedback on detachment progress

3. **Harvest Complete Haptic**
   - Triggered when harvest completes
   - Intensity: 1.0 (strong)
   - Purpose: Confirm successful harvest

### Configuration

```cpp
UPROPERTY(EditAnywhere, Category = "Haptics")
UHapticFeedbackEffect_Base* GrabHapticEffect;

UPROPERTY(EditAnywhere, Category = "Haptics")
UHapticFeedbackEffect_Base* DetachmentHapticEffect;

UPROPERTY(EditAnywhere, Category = "Haptics")
UHapticFeedbackEffect_Base* HarvestCompleteHapticEffect;

UPROPERTY(EditAnywhere, Category = "Haptics")
float HapticIntensityMultiplier = 1.0f; // Global intensity control
```

## Visual Feedback System

### Highlight Colors

1. **Ready to Harvest**: Green glow (RGB: 0.2, 1.0, 0.2)

   - Indicates crop is mature and grabbable
   - Subtle intensity (50% of max)

2. **Being Grabbed**: Yellow-orange glow (RGB: 1.0, 0.8, 0.2)

   - Indicates active grab interaction
   - Pulsing intensity based on detachment progress

3. **Not Ready**: No highlight
   - Crop appears normal without glow

### Material Requirements

Materials must support these parameters:

- `EmissiveColor` (Vector): Color of the highlight glow
- `EmissiveIntensity` (Scalar): Intensity of the glow effect

### Dynamic Material Instance

```cpp
// Created in BeginPlay
UStaticMeshComponent* MeshComp = Owner->FindComponentByClass<UStaticMeshComponent>();
CropMaterialInstance = MeshComp->CreateDynamicMaterialInstance(0);

// Updated in UpdateVisualFeedback
CropMaterialInstance->SetVectorParameterValue(TEXT("EmissiveColor"), Color);
CropMaterialInstance->SetScalarParameterValue(TEXT("EmissiveIntensity"), Intensity);
```

## Integration Points

### Farm Plot Integration

The harvestable component integrates with AFarmPlot:

```cpp
// Initialize with farm plot reference
Harvestable->InitializeHarvestable(CropType, GrowthProgress, Health, FarmPlot, GridPosition);

// Update when crop grows
Harvestable->UpdateGrowthProgress(NewProgress);
Harvestable->UpdateCropHealth(NewHealth);

// Notify on harvest
FarmPlot->HarvestCrop(GridPosition); // Called automatically
```

### VR Player Integration

The interaction manager should be added to the VR player pawn:

```cpp
// In player pawn
UVRHarvestInteractionManager* HarvestManager = CreateDefaultSubobject<UVRHarvestInteractionManager>(TEXT("HarvestManager"));

// Initialize with VR hands
HarvestManager->InitializeHands(LeftHandActor, RightHandActor);

// Handle input
void OnGripPressed()
{
    HarvestManager->TryGrabLeft(); // or TryGrabRight()
}

void OnGripReleased()
{
    HarvestManager->ReleaseLeft(); // or ReleaseRight()
}
```

### Inventory Integration

Placeholder for inventory system integration:

```cpp
void UVRHarvestableComponent::AddToInventory(int32 Amount, float Quality)
{
    // TODO: Integrate with game's inventory system
    // This would add harvested items to player inventory
}
```

## Harvest Collection System

### Recording Harvests

```cpp
FHarvestCollection Harvest;
Harvest.CropName = CropType->CropName;
Harvest.Quantity = YieldAmount;
Harvest.Quality = YieldQuality;
Harvest.HarvestTime = FDateTime::Now();

HarvestManager->RecordHarvest(Harvest);
```

### Statistics Tracking

- **Total Harvested Today**: Sum of all harvested items today
- **Average Quality Today**: Weighted average of harvest quality
- **Crop-Specific Stats**: Total and average quality per crop type
- **History Management**: Automatic cleanup of old entries (max 100)
- **Daily Reset**: Automatic reset at midnight

### Query Methods

```cpp
// Overall statistics
int32 Total;
float AvgQuality;
HarvestManager->GetHarvestStatistics(Total, AvgQuality);

// Crop-specific
int32 TomatoCount = HarvestManager->GetTotalHarvested("Tomato");
float TomatoQuality = HarvestManager->GetAverageQuality("Tomato");

// Ready crops
int32 ReadyCount = HarvestManager->GetReadyCropCount();
```

## Configuration Parameters

### Harvestable Component

```cpp
MinimumGrowthForHarvest = 0.95f;      // 95% growth required
GrabRadius = 15.0f;                    // 15cm grab detection
DetachmentForceThreshold = 50.0f;      // Force units required
DetachmentTime = 0.5f;                 // 0.5 seconds to detach
bRequiresPullingMotion = true;         // Enable pull mechanics
DetachmentDirection = FVector::UpVector; // Pull upward
HapticIntensityMultiplier = 1.0f;      // Full haptic intensity
HighlightIntensity = 2.0f;             // Glow intensity
```

### Interaction Manager

```cpp
HandDetectionRadius = 30.0f;           // 30cm hand detection
FarmPlotDetectionRadius = 1000.0f;     // 10m farm plot detection
MaxHistorySize = 100;                  // 100 harvest records max
```

## Performance Considerations

### Optimization Strategies

1. **Selective Component Addition**: Only add harvestable components to mature crops
2. **Detection Radius**: Limited to 15-30cm for hand detection
3. **Update Frequency**: Hand targets updated every frame, farm plots less frequently
4. **Instanced Rendering**: Use instanced static meshes for crop rendering
5. **LOD System**: Disable interaction for distant crops
6. **Memory Management**: Automatic cleanup of harvested crops

### Memory Usage

- Harvestable component: ~1KB per crop
- Interaction manager: ~5KB + history (100 entries ~10KB)
- Total per farm plot: Depends on crop count

## Testing Recommendations

### Test Scenarios

1. **Basic Harvest**: Grab and pull mature crop to completion
2. **Immature Crop**: Attempt to grab crop below threshold (should fail)
3. **Partial Pull**: Grab and release before detachment completes
4. **Multiple Crops**: Harvest several crops in sequence
5. **Both Hands**: Use both hands simultaneously on different crops
6. **Quality Variation**: Harvest crops with different health levels
7. **Statistics**: Verify harvest tracking and daily statistics
8. **Visual Feedback**: Check highlight colors at different states
9. **Haptic Feedback**: Verify haptic pulses at each stage
10. **Performance**: Test with many harvestable crops

### Debug Visualization

Recommended debug features:

- Draw debug spheres for grab radius
- Display detachment progress as text
- Show hand velocity vectors
- Visualize force accumulation
- Log harvest events

## Requirements Satisfied

This implementation fully satisfies **Requirement 13.4** from the requirements document:

✅ **VR grab for mature crops**

- Implemented grab detection with configurable radius
- State management for grabbable crops
- Hand tracking and target detection

✅ **Crop detachment logic**

- Pull-based detachment mechanics
- Force calculation from hand velocity
- Progress tracking (0-1)
- Configurable detachment parameters

✅ **Harvest collection system**

- Harvest recording with timestamp
- Statistics tracking (total, quality)
- History management with automatic cleanup
- Crop-specific and overall statistics

✅ **Haptic feedback for harvesting**

- Grab haptic (medium intensity)
- Detachment haptic (progressive intensity)
- Harvest complete haptic (strong intensity)
- Configurable haptic effects and intensity

## Future Enhancements

### Potential Improvements

1. **Inventory Integration**: Connect to actual inventory system
2. **Visual Effects**: Add particle effects for harvest (leaves, dust)
3. **Sound Effects**: Crop-specific harvest sounds
4. **Tutorial System**: First-time harvest guidance
5. **UI Display**: Real-time harvest statistics widget
6. **Achievements**: Harvest milestone achievements
7. **Tool Support**: Harvesting tools (sickle, shears) for different crops
8. **Batch Harvesting**: Harvest multiple crops at once
9. **Quality Indicators**: Visual indicators of crop quality before harvest
10. **Seasonal Bonuses**: Harvest bonuses during optimal seasons

### Advanced Features

1. **Damage System**: Improper harvesting reduces yield
2. **Skill System**: Improve harvest efficiency with practice
3. **Crop Varieties**: Different harvest mechanics per crop type
4. **Weather Effects**: Weather impacts harvest difficulty
5. **Time Pressure**: Crops degrade if not harvested promptly
6. **Multiplayer**: Cooperative harvesting mechanics
7. **Automation**: Robotic harvesting assistants
8. **Market Integration**: Dynamic pricing based on quality
9. **Preservation**: Post-harvest storage and processing
10. **Breeding**: Save seeds from high-quality harvests

## Documentation

### Created Files

1. **VRHarvestableComponent.h**: Component header
2. **VRHarvestableComponent.cpp**: Component implementation
3. **VRHarvestInteractionManager.h**: Manager header
4. **VRHarvestInteractionManager.cpp**: Manager implementation
5. **QUICKSTART_VRHarvesting.md**: Quick start guide
6. **TASK_15.3_IMPLEMENTATION.md**: This implementation summary

### Quick Start Guide

See `QUICKSTART_VRHarvesting.md` for:

- Setup instructions
- Usage examples
- Configuration guide
- Integration examples
- Troubleshooting tips

## Conclusion

The VR harvesting interaction system provides an immersive and realistic crop harvesting experience for VR players. The pull-based detachment mechanics, combined with haptic and visual feedback, create a satisfying physical interaction that makes farming feel tangible and rewarding.

The system is designed to be flexible and extensible, with clear integration points for inventory systems, UI, and future enhancements. The harvest collection and statistics tracking provide valuable data for gameplay progression and player feedback.

All requirements from Task 15.3 have been successfully implemented and are ready for integration with the broader farming system.
