# Quick Start: Atmospheric Entry Effects

## Overview

This guide shows you how to quickly set up and use the atmospheric entry effects system in your spacecraft or vehicle.

## Prerequisites

- TransitionController component set up (see Task 9.1)
- Spacecraft or vehicle with physics enabled
- Player controller with camera

## Basic Setup (5 Minutes)

### Step 1: Add Component to Spacecraft

In your spacecraft C++ class or Blueprint:

```cpp
// In header file
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects")
UAtmosphericEntryEffects* EntryEffects;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transition")
UTransitionController* TransitionController;

// In BeginPlay
void AMySpacecraft::BeginPlay()
{
    Super::BeginPlay();

    // Create transition controller
    TransitionController = NewObject<UTransitionController>(this);
    TransitionController->Initialize(6371.0f, FTransitionPhaseConfig());

    // Create entry effects
    EntryEffects = NewObject<UAtmosphericEntryEffects>(this);
    EntryEffects->RegisterComponent();
    EntryEffects->Initialize(TransitionController);
}

// In Tick
void AMySpacecraft::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update transition controller
    TransitionController->UpdateTransition(
        GetActorLocation(),
        PlanetLocation,
        DeltaTime
    );
}
```

### Step 2: Configure Effects (Optional)

Customize the effects to your liking:

```cpp
// In BeginPlay, after Initialize
EntryEffects->HeatShimmerConfig.DistortionIntensity = 0.7f;
EntryEffects->HeatShimmerConfig.StartAltitude = 85.0f;

EntryEffects->PlasmaConfig.PlasmaColor = FLinearColor(1.0f, 0.3f, 0.1f);
EntryEffects->PlasmaConfig.GlowIntensity = 0.9f;

EntryEffects->SoundConfig.VolumeMultiplier = 0.8f;

EntryEffects->CameraShakeConfig.ShakeClass = UTurbulenceCameraShake::StaticClass();
EntryEffects->CameraShakeConfig.MaxIntensity = 1.2f;
```

### Step 3: Add Sound Assets

1. Import or create three sound assets:

   - Wind sound (looping)
   - Plasma crackling sound (looping)
   - Turbulence buffeting sound (looping)

2. Assign them in code or Blueprint:

```cpp
EntryEffects->SoundConfig.WindSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/AtmosphericWind"));
EntryEffects->SoundConfig.PlasmaSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/PlasmaCrackle"));
EntryEffects->SoundConfig.TurbulenceSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Turbulence"));
```

### Step 4: Create Plasma Niagara System (Optional)

1. Create a new Niagara System
2. Add emitter with these parameters:

   - Spawn Rate: User parameter "SpawnRate"
   - Particle Color: User parameter "PlasmaColor"
   - Particle Size: User parameter "ParticleSize"
   - Intensity: User parameter "Intensity"
   - Glow Intensity: User parameter "GlowIntensity"

3. Assign to component:

```cpp
UNiagaraSystem* PlasmaSystem = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/Effects/NS_PlasmaEntry"));
EntryEffects->PlasmaEffectComponent->SetAsset(PlasmaSystem);
```

## Blueprint Setup

### Add Component

1. Open your spacecraft Blueprint
2. Add Component → Search "Atmospheric Entry Effects"
3. Add Component → Search "Transition Controller"

### Configure in Details Panel

1. Select AtmosphericEntryEffects component
2. Expand "Effects" category
3. Configure:
   - Heat Shimmer Config
   - Plasma Config
   - Sound Config
   - Camera Shake Config

### Initialize in Event Graph

```
Event BeginPlay
  → Get Transition Controller
  → Call Initialize on Entry Effects (pass Transition Controller)
```

## Testing

### Test in Editor

1. Place your spacecraft in the level
2. Position it at 100 km altitude above a planet
3. Play in editor
4. Descend toward the planet
5. Observe effects activating at different altitudes:
   - 90 km: Plasma starts
   - 80 km: Heat shimmer begins
   - 70 km: Peak plasma
   - 60 km: Peak heat shimmer
   - 50 km: Plasma ends
   - 40 km: Heat shimmer ends

### Debug Display

Add debug display to see effect intensities:

```cpp
void AMySpacecraft::Tick(float DeltaTime)
{
    // ... existing code ...

    if (GEngine && EntryEffects)
    {
        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow,
            FString::Printf(TEXT("Altitude: %.1f km"), TransitionController->GetAltitude()));
        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Orange,
            FString::Printf(TEXT("Heat Shimmer: %.2f"), EntryEffects->GetHeatShimmerIntensity()));
        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red,
            FString::Printf(TEXT("Plasma: %.2f"), EntryEffects->GetPlasmaIntensity()));
        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Cyan,
            FString::Printf(TEXT("Sound: %.2f"), EntryEffects->GetEntrySoundVolume()));
        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green,
            FString::Printf(TEXT("Shake: %.2f"), EntryEffects->GetCameraShakeIntensity()));
    }
}
```

## Common Issues

### Effects Not Appearing

**Problem**: No effects visible during descent

**Solutions**:

- Verify TransitionController is initialized
- Check that Initialize() was called on EntryEffects
- Ensure altitude is within effect ranges (40-90 km)
- Verify effects are enabled: `EntryEffects->SetEffectsEnabled(true)`

### No Sound Playing

**Problem**: Entry sounds not audible

**Solutions**:

- Check sound assets are assigned
- Verify audio components are created
- Ensure volume multiplier > 0
- Check that you're in atmosphere (altitude < 100 km)

### Camera Shake Too Strong/Weak

**Problem**: Camera shake uncomfortable or not noticeable

**Solutions**:

- Adjust `CameraShakeConfig.BaseIntensity` and `MaxIntensity`
- Modify shake frequency: `CameraShakeConfig.ShakeFrequency`
- Check turbulence intensity in TransitionController
- For VR, keep intensity lower (0.3-0.8)

### Plasma Particles Not Visible

**Problem**: Plasma effect not showing

**Solutions**:

- Verify Niagara system is assigned
- Check PlasmaEffectComponent is created
- Ensure altitude is in plasma range (50-90 km)
- Verify Niagara system has correct user parameters

## Advanced Configuration

### Custom Altitude Ranges

Adjust when effects appear:

```cpp
// Heat shimmer at lower altitudes
EntryEffects->HeatShimmerConfig.StartAltitude = 70.0f;
EntryEffects->HeatShimmerConfig.PeakAltitude = 50.0f;
EntryEffects->HeatShimmerConfig.EndAltitude = 30.0f;

// Plasma at higher altitudes
EntryEffects->PlasmaConfig.StartAltitude = 100.0f;
EntryEffects->PlasmaConfig.PeakAltitude = 80.0f;
EntryEffects->PlasmaConfig.EndAltitude = 60.0f;
```

### Custom Camera Shake

Create your own camera shake class:

```cpp
UCLASS()
class UMyCustomEntryShake : public UTurbulenceCameraShake
{
    GENERATED_BODY()

    UMyCustomEntryShake()
    {
        RotationAmplitude = FRotator(2.0f, 2.0f, 1.0f);
        Duration = 1.0f;
    }
};

// Assign to effects
EntryEffects->CameraShakeConfig.ShakeClass = UMyCustomEntryShake::StaticClass();
```

### Dynamic Effect Control

Enable/disable effects based on conditions:

```cpp
// Disable effects during combat
if (bInCombat)
{
    EntryEffects->SetEffectsEnabled(false);
}

// Re-enable when safe
if (!bInCombat)
{
    EntryEffects->SetEffectsEnabled(true);
}
```

### Query Effect State

Check current effect intensities:

```cpp
if (EntryEffects->GetPlasmaIntensity() > 0.5f)
{
    // Plasma is strong, maybe show warning
    ShowPlasmaWarning();
}

if (EntryEffects->GetCameraShakeIntensity() > 0.8f)
{
    // Heavy turbulence, reduce player control
    ReduceControlAuthority();
}
```

## Performance Tips

1. **Disable When Not Needed**:

   ```cpp
   if (DistanceToPlanet > 200000.0f) // 200 km
   {
       EntryEffects->SetEffectsEnabled(false);
   }
   ```

2. **Reduce Particle Count**:

   ```cpp
   EntryEffects->PlasmaConfig.SpawnRate = 50.0f; // Lower for better performance
   ```

3. **Simplify Camera Shake**:
   ```cpp
   EntryEffects->CameraShakeConfig.ShakeFrequency = 1.0f; // Less frequent shakes
   ```

## Next Steps

- Integrate with physics system (Task 9.2)
- Add rendering transitions (Task 9.3)
- Create custom Niagara plasma effects
- Design custom entry sounds
- Test in VR for comfort

## Reference

- Full implementation: `TASK_9.4_IMPLEMENTATION.md`
- Transition controller: `TASK_9.1_IMPLEMENTATION.md`
- Physics transitions: `TASK_9.2_IMPLEMENTATION.md`
- Rendering transitions: `TASK_9.3_IMPLEMENTATION.md`
