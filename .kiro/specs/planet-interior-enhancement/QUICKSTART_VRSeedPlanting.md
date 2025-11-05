# Quick Start: VR Seed Planting System

## Overview

The VR Seed Planting system enables players to physically plant seeds in farm plots using VR motion controllers with grab interactions, gesture detection, and haptic feedback.

## Core Components

### 1. VRSeedItem (AVRSeedItem)

Physical seed actor that can be grabbed and planted.

**Key Features:**

- VR grab interaction support
- Planting target detection
- Visual feedback for valid planting areas
- Seed count management
- Haptic feedback integration

### 2. VRPlantingGestureDetector (UVRPlantingGestureDetector)

Detects planting gestures from hand motion.

**Gesture Types:**

- **Downward Motion**: Fast downward hand movement
- **Release Near Ground**: Releasing seed close to ground
- **Press and Release**: Button press while near ground

### 3. VRPlantingInteractionManager (UVRPlantingInteractionManager)

Coordinates all planting interactions.

**Responsibilities:**

- Spawn seed items
- Handle grab/release events
- Validate planting locations
- Play haptic feedback
- Show planting preview

## Setup Instructions

### 1. Add Components to VR Pawn

```cpp
// In your VR Pawn class
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Planting")
UVRPlantingInteractionManager* PlantingManager;

// In constructor
PlantingManager = CreateDefaultSubobject<UVRPlantingInteractionManager>(TEXT("PlantingManager"));
```

### 2. Configure Seed Item Class

In Blueprint or C++:

```cpp
// Set the seed item class to spawn
PlantingManager->SeedItemClass = AVRSeedItem::StaticClass();
```

### 3. Setup Haptic Feedback

```cpp
// Configure haptic settings
PlantingManager->HapticConfig.PlantingIntensity = 0.7f;
PlantingManager->HapticConfig.GrabIntensity = 0.4f;
PlantingManager->HapticConfig.InvalidPlacementIntensity = 0.3f;
```

### 4. Enable Gesture Detection

```cpp
PlantingManager->bEnableGestureDetection = true;
PlantingManager->bAutoPlantOnRelease = true;
```

## Usage Examples

### Spawning Seeds

```cpp
// Spawn a seed item for wheat
UCropDefinition* WheatCrop = LoadObject<UCropDefinition>(nullptr, TEXT("/Game/Crops/Wheat"));
FVector SpawnLocation = PlayerHand->GetActorLocation();

AVRSeedItem* Seed = PlantingManager->SpawnSeedItem(WheatCrop, SpawnLocation, 10);
```

### Handling Grab Events

```cpp
// When player grabs seed (called from VR grab component)
void OnGrabSeed(AVRSeedItem* Seed, AActor* Hand)
{
    PlantingManager->OnSeedGrabbed(Seed, Hand);
}

// When player releases seed
void OnReleaseSeed(AVRSeedItem* Seed)
{
    PlantingManager->OnSeedReleased(Seed);
}
```

### Manual Planting

```cpp
// Plant seed at specific location
FVector PlantLocation = GetPlantingLocation();
bool bPlanted = PlantingManager->PlantSeedAtLocation(CurrentSeed, PlantLocation);

if (bPlanted)
{
    UE_LOG(LogTemp, Log, TEXT("Seed planted successfully!"));
}
```

## Integration with Farm Plots

The system automatically detects farm plots and calculates grid positions:

```cpp
// Check if location is valid for planting
AFarmPlot* FarmPlot = nullptr;
FIntPoint GridPosition;

if (PlantingManager->IsValidPlantingLocation(Location, FarmPlot, GridPosition))
{
    // Location is valid, can plant here
    PlantingManager->PlantSeedInGrid(Seed, FarmPlot, GridPosition);
}
```

## Gesture Detection Configuration

Adjust gesture detection parameters:

```cpp
// Get gesture detector
UVRPlantingGestureDetector* GestureDetector = PlantingManager->GetGestureDetector();

// Configure detection thresholds
GestureDetector->MinDownwardVelocity = 50.0f; // cm/s
GestureDetector->MaxPlantingHeight = 30.0f; // cm
GestureDetector->GestureTimeWindow = 0.5f; // seconds
GestureDetector->MinConfidenceThreshold = 0.7f; // 70%

// Enable debug visualization
GestureDetector->bDebugVisualization = true;
```

## Haptic Feedback Types

### Planting Haptics

Played when seed is successfully planted:

- **Intensity**: 0.7 (strong)
- **Duration**: 0.3 seconds
- **Pattern**: Single strong pulse

### Grab Haptics

Played when seed is grabbed:

- **Intensity**: 0.4 (medium)
- **Duration**: 0.1 seconds
- **Pattern**: Quick pulse

### Invalid Placement Haptics

Played when trying to plant in invalid location:

- **Intensity**: 0.3 (light)
- **Duration**: 0.15 seconds
- **Pattern**: Double pulse

## Visual Feedback

The system provides visual feedback for planting:

```cpp
// Show preview at location
PlantingManager->ShowPlantingPreview(Location, bIsValid);

// Update preview as hand moves
PlantingManager->UpdatePlantingPreview(CurrentLocation);

// Hide preview
PlantingManager->HidePlantingPreview();
```

## Blueprint Integration

### Creating Seed Items in Blueprint

1. Create Blueprint based on `AVRSeedItem`
2. Set static mesh for seed visual
3. Configure grab collision sphere
4. Set seed properties (color, count)

### Handling VR Grab Events

1. In VR Pawn Blueprint, add `VRPlantingInteractionManager` component
2. Connect grab events from VR hands:
   - `OnGrabbed` → Call `OnSeedGrabbed`
   - `OnReleased` → Call `OnSeedReleased`

### Custom Haptic Effects

1. Create Haptic Feedback Effect asset
2. Assign to `PlantingHapticConfig.PlantingHapticEffect`
3. Configure intensity and duration

## Testing

### Debug Visualization

Enable debug visualization to see:

- Motion history trail (yellow lines)
- Velocity vectors (green arrows)
- Ground detection rays (blue lines)
- Gesture confidence text

```cpp
GestureDetector->bDebugVisualization = true;
```

### Console Commands

```
// Spawn test seed
SpawnSeed Wheat 10

// Toggle gesture detection
ToggleGestureDetection

// Show planting debug info
ShowPlantingDebug
```

## Performance Considerations

- **Gesture Detection**: Runs at 60 FPS, minimal overhead
- **Collision Detection**: Uses sphere traces, optimized for VR
- **Haptic Feedback**: Async, no frame impact
- **Visual Feedback**: Material parameters, GPU-side

## Common Issues

### Seeds Not Planting

- Check farm plot collision is enabled
- Verify grid resolution is set correctly
- Ensure crop type is assigned to seed

### Gesture Not Detecting

- Increase `MaxPlantingHeight` threshold
- Decrease `MinDownwardVelocity` requirement
- Enable debug visualization to see motion

### Haptics Not Working

- Verify controller is connected
- Check haptic effect asset is assigned
- Ensure intensity values are > 0

## Requirements Satisfied

This implementation satisfies requirements:

- **13.1**: Hand-planting of seeds using VR motion controller grab and release gestures
- **13.4**: Haptic feedback when planting, watering, and harvesting with intensity based on action type

## Next Steps

1. Create Blueprint seed item variants for different crops
2. Design custom haptic effects for each action
3. Add visual effects for planting (particles, sounds)
4. Implement seed inventory system
5. Add tutorial for VR planting mechanics
