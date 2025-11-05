# Visual Effects Polish - Quick Start Guide

## Overview

The Visual Effects Polish System enhances the visual quality of the game through improved particle effects, smooth material transitions, ambient details, and cinematic camera modes.

## Visual Effects Quality Settings

### Setting Quality Level

```cpp
// Get visual effects system
UVisualEffectsPolishSystem* VFXSystem = GetWorld()->GetSubsystem<UVisualEffectsPolishSystem>();

// Set quality level
VFXSystem->SetEffectsQuality(EVisualEffectsQuality::High);
```

### Quality Levels

**Low:**

- Minimal particle count (100)
- No ambient insects, leaves, or pollen
- Basic lighting only
- 30% particle density

**Medium:**

- Moderate particle count (500)
- Ambient insects enabled
- Standard lighting
- 60% particle density

**High:**

- High particle count (1000)
- All ambient details enabled
- Advanced lighting and shadows
- 90% particle density

**Ultra:**

- Very high particle count (2000)
- All features enabled
- Maximum detail
- 100% particle density

**Cinematic:**

- Maximum particle count (5000)
- Sub-emitters enabled
- All advanced features
- Best visual quality

## Enhanced Particle Effects

### Spawning Enhanced Particles

```cpp
// Load particle system
UNiagaraSystem* ParticleSystem = LoadObject<UNiagaraSystem>(
    nullptr,
    TEXT("/Game/Effects/NS_RainEffect")
);

// Spawn with automatic quality enhancement
UNiagaraComponent* Particle = VFXSystem->SpawnEnhancedParticleEffect(
    ParticleSystem,
    FVector(0, 0, 1000),  // Location
    FRotator::ZeroRotator, // Rotation
    FVector(1.0f)          // Scale
);
```

### Enhancing Existing Particles

```cpp
// Enhance an existing particle component
UNiagaraComponent* ExistingParticle = GetParticleComponent();
VFXSystem->EnhanceParticleEffect(ExistingParticle);
```

### Quality-Based Parameters

The system automatically sets these parameters based on quality:

- `ParticleCount` - Number of particles
- `DetailLevel` - Level of detail (0.0-1.0)
- `EnableLighting` - Particle lighting (High+)
- `EnableShadows` - Particle shadows (High+)
- `EnableCollision` - Particle collision (High+)
- `EnableSubEmitters` - Sub-emitters (Cinematic)

## Material Transitions

### Smooth Material Transition

```cpp
// Transition to new material over time
UMeshComponent* Mesh = GetMeshComponent();
UMaterialInterface* NewMaterial = LoadObject<UMaterialInterface>(...);

VFXSystem->TransitionMaterial(
    Mesh,
    NewMaterial,
    2.0f  // Duration in seconds
);
```

### Parameter Transitions

```cpp
// Smoothly change material parameter
UMaterialInstanceDynamic* Material = Mesh->CreateDynamicMaterialInstance(0);

VFXSystem->TransitionMaterialParameter(
    Material,
    "Opacity",      // Parameter name
    0.0f,           // Target value
    1.5f            // Duration
);
```

### Common Use Cases

**Biome Transition:**

```cpp
// Fade between biome materials
VFXSystem->TransitionMaterial(TerrainMesh, DesertMaterial, 3.0f);
```

**Weather Effects:**

```cpp
// Increase wetness during rain
VFXSystem->TransitionMaterialParameter(SurfaceMaterial, "Wetness", 1.0f, 2.0f);
```

**Day/Night:**

```cpp
// Adjust emissive for night
VFXSystem->TransitionMaterialParameter(BuildingMaterial, "EmissiveStrength", 5.0f, 10.0f);
```

## Ambient Details

### Configuration

```cpp
// Configure ambient details
FAmbientDetailConfig Config;
Config.bEnableDustParticles = true;
Config.bEnableInsects = true;
Config.bEnableLeaves = true;
Config.bEnablePollen = true;
Config.ParticleDensity = 0.8f;
Config.SpawnRadius = 50.0f;

VFXSystem->SetAmbientDetailConfig(Config);
```

### Spawning Ambient Details

```cpp
// Spawn ambient details around player
FVector PlayerLocation = GetPlayerLocation();
VFXSystem->SpawnAmbientDetails(PlayerLocation, 100.0f);
```

### Ambient Detail Types

**Dust Particles:**

- Always enabled
- Subtle floating particles
- Biome-specific colors

**Insects:**

- Enabled at Medium+
- Flying insects (butterflies, bees)
- Biome-appropriate species

**Leaves:**

- Enabled at High+
- Falling leaves in forests
- Wind-affected movement

**Pollen:**

- Enabled at High+
- Visible in sunlight
- Seasonal variation

## Cinematic Camera Modes

### Creating Cinematic Camera

```cpp
// Spawn cinematic camera controller
ACinematicCameraController* CinematicCamera = GetWorld()->SpawnActor<ACinematicCameraController>(
    ACinematicCameraController::StaticClass(),
    FVector::ZeroVector,
    FRotator::ZeroRotator
);
```

### Orbit Planet Mode

```cpp
// Orbit around planet
CinematicCamera->SetCameraMode(ECinematicCameraMode::OrbitPlanet);
CinematicCamera->SetOrbitTarget(
    PlanetActor,
    5000.0f,  // Distance
    2000.0f   // Height
);
CinematicCamera->SetOrbitSpeed(30.0f); // Degrees per second
```

### Landing Approach Mode

```cpp
// Follow ship during landing
CinematicCamera->SetCameraMode(ECinematicCameraMode::LandingApproach);
CinematicCamera->SetupLandingApproach(
    PlayerShip,
    LandingPad
);
```

### Farm Overview Mode

```cpp
// Showcase farm
CinematicCamera->SetCameraMode(ECinematicCameraMode::FarmOverview);
CinematicCamera->SetupFarmOverview(
    FarmCenter,
    500.0f  // View radius
);
```

### Biome Showcase Mode

```cpp
// Sweep across biome
CinematicCamera->SetCameraMode(ECinematicCameraMode::BiomeShowcase);
CinematicCamera->SetupBiomeShowcase(
    BiomeCenter,
    2000.0f  // Sweep radius
);
```

### Camera Effects

**Camera Shake:**

```cpp
FCameraShakeSettings ShakeSettings;
ShakeSettings.Intensity = 0.5f;
ShakeSettings.Frequency = 10.0f;
ShakeSettings.bRotational = true;

CinematicCamera->EnableCameraShake(ShakeSettings);
```

**Field of View:**

```cpp
CinematicCamera->SetFieldOfView(90.0f);
```

**Focus Distance:**

```cpp
CinematicCamera->SetFocusDistance(500.0f);
```

**Smooth Transition:**

```cpp
CinematicCamera->SmoothTransitionTo(
    TargetLocation,
    TargetRotation,
    3.0f  // Duration
);
```

## Cinematic Mode

### Enabling Cinematic Mode

```cpp
// Enable full cinematic mode
VFXSystem->EnableCinematicMode(true);
```

This automatically:

- Sets quality to Cinematic
- Enables depth of field
- Adds motion blur
- Applies vignette
- Enhances bloom

### Manual Post-Process Settings

**Depth of Field:**

```cpp
VFXSystem->SetDepthOfFieldSettings(
    500.0f,  // Focal distance
    2.8f     // Aperture (f-stop)
);
```

**Motion Blur:**

```cpp
VFXSystem->SetMotionBlurAmount(0.5f);
```

**Vignette:**

```cpp
VFXSystem->SetVignetteIntensity(0.3f);
```

**Bloom:**

```cpp
VFXSystem->SetBloomIntensity(0.8f);
```

**Color Grading:**

```cpp
VFXSystem->SetColorGrading(
    FLinearColor(1.0f, 0.95f, 0.9f),  // Warm tint
    1.1f,  // Saturation
    1.05f  // Contrast
);
```

## Example Scenarios

### Atmospheric Entry Sequence

```cpp
void PlayAtmosphericEntrySequence()
{
    // Create cinematic camera
    ACinematicCameraController* Camera = SpawnCinematicCamera();

    // Enable cinematic mode
    VFXSystem->EnableCinematicMode(true);

    // Add camera shake for turbulence
    FCameraShakeSettings Shake;
    Shake.Intensity = 1.0f;
    Shake.Frequency = 15.0f;
    Camera->EnableCameraShake(Shake);

    // Spawn heat effects
    SpawnEnhancedParticleEffect(HeatShimmerSystem, ShipLocation);

    // Transition to red-tinted atmosphere
    VFXSystem->SetColorGrading(
        FLinearColor(1.2f, 0.8f, 0.6f),
        0.9f,
        1.1f
    );
}
```

### Farm Showcase

```cpp
void ShowcaseFarm()
{
    // Setup overview camera
    ACinematicCameraController* Camera = SpawnCinematicCamera();
    Camera->SetCameraMode(ECinematicCameraMode::FarmOverview);
    Camera->SetupFarmOverview(FarmCenter, 500.0f);

    // Spawn ambient details
    VFXSystem->SpawnAmbientDetails(FarmCenter, 100.0f);

    // Enhance lighting
    VFXSystem->SetBloomIntensity(0.6f);

    // Add depth of field
    VFXSystem->SetDepthOfFieldSettings(300.0f, 4.0f);
}
```

### Biome Transition

```cpp
void TransitionBiomes(UMaterialInterface* NewBiomeMaterial)
{
    // Smooth material transition
    VFXSystem->TransitionMaterial(TerrainMesh, NewBiomeMaterial, 5.0f);

    // Update ambient details
    FAmbientDetailConfig Config = VFXSystem->GetAmbientDetailConfig();
    Config.ParticleDensity = 0.9f;
    VFXSystem->SetAmbientDetailConfig(Config);

    // Spawn new ambient particles
    VFXSystem->SpawnAmbientDetails(PlayerLocation, 100.0f);
}
```

## Performance Considerations

### Quality vs Performance

- **Low**: Best for VR on mobile (Quest)
- **Medium**: Good for VR on PC
- **High**: Desktop VR with good GPU
- **Ultra**: High-end desktop
- **Cinematic**: Screenshots/videos only

### Optimization Tips

1. **Limit ambient particles** - Max 50 active at once
2. **Use LOD for particles** - Reduce count at distance
3. **Batch material transitions** - Don't transition too many at once
4. **Disable cinematic mode in gameplay** - Only for cutscenes
5. **Adjust quality based on framerate** - Dynamic quality scaling

## Troubleshooting

### Particles Not Appearing

- Check quality level (may be disabled at Low)
- Verify particle system is valid
- Check spawn location is visible
- Ensure particle budget not exceeded

### Material Transitions Stuttering

- Reduce transition duration
- Limit concurrent transitions
- Check material complexity
- Verify no other systems blocking

### Camera Shake Too Intense

- Reduce shake intensity
- Lower shake frequency
- Disable rotational shake
- Check VR comfort settings

---

**Version:** 1.0  
**Last Updated:** 2025-11-04
