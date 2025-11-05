# VR Watering System - Quick Start Guide

## Overview

The VR Watering System provides an immersive, physics-based watering can interaction for VR farming. Players can grab, tilt, and pour water onto crops with realistic water stream effects and haptic feedback.

## Core Components

### AVRWateringCan

Main actor class that implements the watering can functionality.

**Key Features:**

- VR grab/release interaction
- Physics-based pouring (tilt-activated)
- Water capacity and consumption tracking
- Niagara particle effects for water stream
- Haptic feedback during watering
- Audio feedback (pouring sounds)
- Visual feedback (water level indicator)

## Quick Setup

### 1. Create Watering Can Blueprint

1. Create a new Blueprint based on `AVRWateringCan`
2. Name it `BP_VRWateringCan`
3. Set up the mesh:
   - Assign a watering can static mesh to `CanMesh`
   - Adjust `SpoutLocation` to match the spout position on your mesh
   - Configure `GrabCollision` sphere radius for comfortable grabbing

### 2. Configure Water Properties

In the Blueprint details panel:

```
Water Settings:
- Water Capacity: 10.0 (liters)
- Water Flow Rate: 0.5 (liters/second)
- Min Tilt Angle For Pouring: 45.0 (degrees)
- Max Watering Distance: 200.0 (cm)
```

### 3. Set Up Niagara Water Stream

1. Create a Niagara System for the water stream:

   - Use a GPU Sprite emitter
   - Add gravity force
   - Configure spawn rate based on `StreamIntensity` parameter
   - Add collision module for surface interaction

2. Assign to `Water Stream Niagara System` property

**Required Niagara Parameters:**

- `StreamIntensity` (float): Controls particle spawn rate
- `FlowRate` (float): Controls water flow speed
- `StreamVelocity` (vector): Initial velocity direction

### 4. Configure Effects

```
Effects Settings:
- Water Stream Niagara System: [Your Niagara System]
- Pouring Sound: [Water pouring sound asset]
- Empty Sound: [Empty can sound asset]
```

### 5. Set Up Haptics

```
Haptics Settings:
- Haptic Feedback Interval: 0.1 (10 Hz)
- Haptic Intensity Multiplier: 0.4 (40% intensity)
```

## Usage in VR

### Grabbing the Watering Can

```cpp
// In your VR hand controller
void AVRHandController::TryGrabObject()
{
    // Overlap check with grab collision
    TArray<AActor*> OverlappingActors;
    GetOverlappingActors(OverlappingActors, AVRWateringCan::StaticClass());

    if (OverlappingActors.Num() > 0)
    {
        AVRWateringCan* WateringCan = Cast<AVRWateringCan>(OverlappingActors[0]);
        if (WateringCan)
        {
            WateringCan->OnGrabbed(this);
            CurrentGrabbedObject = WateringCan;
        }
    }
}

void AVRHandController::ReleaseObject()
{
    if (AVRWateringCan* WateringCan = Cast<AVRWateringCan>(CurrentGrabbedObject))
    {
        WateringCan->OnReleased();
        CurrentGrabbedObject = nullptr;
    }
}
```

### Watering Crops

The watering can automatically:

1. Detects tilt angle
2. Starts pouring when tilted > 45 degrees
3. Performs raycast to find farm plots
4. Applies water to affected crops
5. Provides haptic and audio feedback
6. Consumes water from capacity

## Material Setup

### Water Level Visualization

Create a material with these parameters:

```
Material Parameters:
- WaterLevel (Scalar): 0.0 to 1.0
- WaterColor (Vector): RGB color for water
- PlantingFeedback (Vector): Not used in watering can
- FeedbackIntensity (Scalar): Not used in watering can
```

**Example Material Graph:**

1. Use `WaterLevel` to drive a gradient mask
2. Lerp between empty and full colors
3. Apply to emissive or base color
4. Add transparency for water surface

## Integration with Farm Plots

The watering can integrates with `AFarmPlot`:

```cpp
// In AFarmPlot
void AFarmPlot::WaterPlot(float WaterAmount)
{
    // Increase water level
    WaterLevel = FMath::Clamp(WaterLevel + (WaterAmount / PlotSize.X / PlotSize.Y), 0.0f, 1.0f);

    // Update all crops in the plot
    for (FCropCell& Cell : CropGrid)
    {
        if (Cell.CropType)
        {
            Cell.bNeedsWater = false;
            // Improve crop health
            Cell.Health = FMath::Min(1.0f, Cell.Health + 0.1f);
        }
    }
}
```

## Advanced Features

### Refilling Water

```cpp
// At a water source
void AWaterSource::OnOverlapBegin(AActor* OtherActor)
{
    AVRWateringCan* WateringCan = Cast<AVRWateringCan>(OtherActor);
    if (WateringCan)
    {
        WateringCan->RefillWater();
    }
}
```

### Custom Water Effects

Override `UpdateWaterStream()` to customize particle behavior:

```cpp
void AVRWateringCan::UpdateWaterStream(float DeltaTime)
{
    Super::UpdateWaterStream(DeltaTime);

    // Add custom effects
    if (bWaterHittingSurface)
    {
        // Spawn splash effect at hit location
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            SplashEffect,
            WaterHitLocation
        );
    }
}
```

### Haptic Customization

```cpp
void AVRWateringCan::PlayWateringHaptics(float Intensity)
{
    // Use custom haptic effect asset
    if (WateringHapticEffect)
    {
        PC->PlayHapticEffect(
            WateringHapticEffect,
            DetermineGrabbingHand(),
            Intensity,
            false
        );
    }
}
```

## Testing

### In-Editor Testing

1. Place `BP_VRWateringCan` in level
2. Place `BP_FarmPlot` nearby
3. Enable VR preview
4. Grab watering can with motion controller
5. Tilt to pour water onto farm plot
6. Verify:
   - Water stream appears when tilted
   - Haptic feedback pulses during pouring
   - Audio plays while pouring
   - Farm plot receives water
   - Water level decreases

### Debug Visualization

Enable debug drawing in `PerformWateringRaycast()`:

- Green line: Water hitting surface
- Red line: Water not hitting anything

### Console Commands

```
// Show watering can debug info
ShowDebug VRWateringCan

// Refill all watering cans
RefillAllWateringCans

// Set water flow rate
SetWaterFlowRate 1.0
```

## Performance Considerations

- **Niagara Particles**: Limit max particles to 500-1000
- **Raycast Frequency**: Performed every frame when pouring (optimized)
- **Haptic Updates**: Limited to 10 Hz to avoid overwhelming the controller
- **Audio**: Single looping sound, modulated by tilt angle

## Common Issues

### Water Stream Not Appearing

- Check that `WaterStreamNiagaraSystem` is assigned
- Verify `SpoutLocation` is positioned correctly
- Ensure Niagara system has required parameters

### No Haptic Feedback

- Verify `GrabbingHand` is set correctly
- Check `HapticIntensityMultiplier` is > 0
- Ensure VR controllers support haptics

### Water Not Affecting Crops

- Verify `AFarmPlot::WaterPlot()` is implemented
- Check raycast is hitting farm plot collision
- Ensure `MaxWateringDistance` is sufficient

### Pouring Doesn't Start

- Check `MinTiltAngleForPouring` value (try lowering)
- Verify `CurrentWaterAmount` > 0
- Ensure `bIsGrabbed` is true

## Requirements Satisfied

This implementation satisfies:

- **Requirement 13.2**: Physics-based watering can interaction with visible water stream
- **Requirement 13.4**: Haptic feedback when planting, watering, and harvesting

## Next Steps

1. Create Niagara water stream effect
2. Set up watering can mesh and materials
3. Configure audio assets
4. Test VR interaction flow
5. Integrate with automated irrigation systems (Task 15.3)

## Related Systems

- `AVRSeedItem`: VR seed planting
- `AFarmPlot`: Farm plot management
- `UCropGrowthStageManager`: Crop growth stages
- `UCropHealthSystem`: Crop health affected by watering
