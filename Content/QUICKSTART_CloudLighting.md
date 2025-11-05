# Quick Start: Cloud Lighting and Shadows

## Overview

This guide shows you how to set up and use the cloud lighting and shadow system for realistic volumetric clouds.

## Prerequisites

- Planet actor with `UPlanetCloudComponent`
- `UDayNightCycleComponent` for sun direction
- Volumetric cloud material set up

## Basic Setup

### 1. Add Components to Planet

```cpp
// In your Planet actor
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
UPlanetCloudComponent* CloudComponent;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
UDayNightCycleComponent* DayNightCycle;
```

### 2. Initialize in Constructor

```cpp
APlanet::APlanet()
{
    // Create day-night cycle first
    DayNightCycle = CreateDefaultSubobject<UDayNightCycleComponent>(TEXT("DayNightCycle"));

    // Create cloud component
    CloudComponent = CreateDefaultSubobject<UPlanetCloudComponent>(TEXT("CloudComponent"));
    CloudComponent->SetupAttachment(RootComponent);
}
```

### 3. Configure Lighting Parameters

In the editor or code, set these parameters on the Cloud Component:

**Basic Lighting**:

- `ScatteringIntensity`: 1.0 (overall brightness)
- `PhaseG`: 0.6 (forward scattering, 0.5-0.8 typical)
- `AmbientLightIntensity`: 0.2 (ambient contribution)
- `AmbientLightColor`: (0.5, 0.6, 0.8) - sky blue tint

**Shadow Quality**:

- `LightRayMarchSteps`: 6 (more = better quality, lower performance)
- `ShadowStepSize`: 0.5 km (smaller = more accurate)
- `ShadowDensityMultiplier`: 1.0 (higher = darker shadows)

**Shadow Intensity**:

- `SelfShadowIntensity`: 0.8 (cloud self-shadowing)
- `TerrainShadowIntensity`: 0.7 (ground shadows)

## Usage Examples

### Blueprint: Get Terrain Shadow

```
Event Tick
  -> Get Cloud Component
  -> Get Player Location
  -> Get Sun Direction (from Cloud Component)
  -> Calculate Terrain Shadow
    - Terrain Position: Player Location
    - Sun Direction: Sun Direction
  -> Print String (Shadow Intensity)
```

### Blueprint: Apply Shadow to Light

```
Event Tick
  -> Get Cloud Component
  -> Calculate Terrain Shadow
  -> Get Directional Light
  -> Set Intensity
    - New Intensity = Base Intensity * (1 - Shadow Intensity)
```

### C++: Query Cloud Shadow

```cpp
void AMyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Get cloud component from planet
    UPlanetCloudComponent* Clouds = Planet->CloudComponent;

    // Calculate shadow at character position
    FVector CharPos = GetActorLocation();
    FVector SunDir = Clouds->GetSunDirection();
    float Shadow = Clouds->CalculateTerrainShadow(CharPos, SunDir);

    // Apply to character lighting
    ApplyShadowToLighting(Shadow);
}
```

### C++: Calculate Cloud Lighting

```cpp
// Sample lighting at a specific cloud position
FVector CloudPosition = FVector(100.0f, 200.0f, 5.0f); // km
float HeightInLayer = 0.5f; // Middle of cloud layer
FVector SunDirection = CloudComponent->GetSunDirection();

float LightEnergy = CloudComponent->CalculateCloudLighting(
    CloudPosition,
    HeightInLayer,
    SunDirection
);

// LightEnergy is 0-1, where 1 = fully lit, 0 = fully shadowed
```

## Material Setup

### Cloud Material Parameters

Add these parameters to your volumetric cloud material:

**Lighting**:

- `SunDirection` (Vector3)
- `ScatteringIntensity` (Scalar)
- `PhaseG` (Scalar)
- `AmbientLightIntensity` (Scalar)
- `AmbientLightColor` (Vector3)

**Shadows**:

- `LightRayMarchSteps` (Scalar)
- `ShadowStepSize` (Scalar)
- `ShadowDensityMultiplier` (Scalar)
- `SelfShadowIntensity` (Scalar)

### Material Function: Ray March Shadow

```
Custom Node: RayMarchShadow
Inputs:
  - Position (Vector3)
  - SunDirection (Vector3)
  - StepSize (Scalar)
  - NumSteps (Scalar)

Code:
  float opticalDepth = 0.0;
  float3 pos = Position;

  for (int i = 0; i < NumSteps; i++)
  {
      pos += SunDirection * StepSize;
      float density = SampleCloudDensity(pos);
      opticalDepth += density * StepSize;
  }

  return exp(-opticalDepth * Extinction);
```

### Material Function: Apply Lighting

```
CloudDensity = Sample Cloud Density
ShadowFactor = Ray March Shadow
Phase = Henyey-Greenstein Phase Function
DirectLight = SunColor * ShadowFactor * Phase * ScatteringIntensity
AmbientLight = AmbientLightColor * AmbientLightIntensity
TotalLight = DirectLight + AmbientLight
FinalColor = CloudAlbedo * TotalLight * CloudDensity
```

## Configuration Presets

### Preset 1: Earth-like Cumulus Clouds

```cpp
// Bright, fluffy clouds with soft shadows
CloudComponent->ScatteringIntensity = 1.2f;
CloudComponent->PhaseG = 0.6f;
CloudComponent->AmbientLightIntensity = 0.25f;
CloudComponent->SelfShadowIntensity = 0.7f;
CloudComponent->TerrainShadowIntensity = 0.6f;
CloudComponent->LightRayMarchSteps = 6;
```

### Preset 2: Storm Clouds

```cpp
// Dark, dramatic clouds with strong shadows
CloudComponent->ScatteringIntensity = 0.8f;
CloudComponent->PhaseG = 0.7f;
CloudComponent->AmbientLightIntensity = 0.15f;
CloudComponent->SelfShadowIntensity = 0.9f;
CloudComponent->TerrainShadowIntensity = 0.85f;
CloudComponent->ShadowDensityMultiplier = 1.5f;
CloudComponent->LightRayMarchSteps = 8;
```

### Preset 3: Alien Planet - Orange Atmosphere

```cpp
// Exotic lighting with colored ambient
CloudComponent->ScatteringIntensity = 1.0f;
CloudComponent->PhaseG = 0.8f;
CloudComponent->AmbientLightIntensity = 0.3f;
CloudComponent->AmbientLightColor = FLinearColor(0.8f, 0.5f, 0.3f); // Orange
CloudComponent->SelfShadowIntensity = 0.8f;
CloudComponent->TerrainShadowIntensity = 0.7f;
```

### Preset 4: Performance Mode

```cpp
// Optimized for lower-end hardware
CloudComponent->LightRayMarchSteps = 4;
CloudComponent->ShadowStepSize = 1.0f;
CloudComponent->ShadowDensityMultiplier = 0.8f;
CloudComponent->TerrainShadowIntensity = 0.5f;
```

## Testing Your Setup

### Visual Checks

1. **Shadow Movement**:

   - Set time of day to noon
   - Enable wind on clouds
   - Observe shadows moving across terrain

2. **Self-Shadowing**:

   - Look at thick clouds from the side
   - Bottom should be darker than top
   - Edges toward sun should be brighter

3. **Terrain Shadows**:

   - Fly above clouds
   - Look down at terrain
   - Should see cloud shadow patterns

4. **Sun Angle**:
   - Change time of day
   - Shadows should lengthen at dawn/dusk
   - No shadows at night

### Performance Checks

```cpp
// Add to your HUD or debug display
void AMyHUD::DrawHUD()
{
    Super::DrawHUD();

    // Show cloud lighting stats
    if (CloudComponent)
    {
        FString Stats = FString::Printf(
            TEXT("Cloud Lighting:\n")
            TEXT("Ray March Steps: %d\n")
            TEXT("Shadow Step Size: %.2f km\n")
            TEXT("Self-Shadow: %.2f\n")
            TEXT("Terrain Shadow: %.2f"),
            CloudComponent->LightRayMarchSteps,
            CloudComponent->ShadowStepSize,
            CloudComponent->SelfShadowIntensity,
            CloudComponent->TerrainShadowIntensity
        );

        DrawText(Stats, FLinearColor::White, 10, 100);
    }
}
```

## Troubleshooting

### Shadows Too Dark

- Increase `AmbientLightIntensity` (try 0.3-0.4)
- Decrease `ShadowDensityMultiplier` (try 0.7-0.8)
- Decrease `SelfShadowIntensity` (try 0.6-0.7)

### Shadows Too Light

- Increase `ShadowDensityMultiplier` (try 1.2-1.5)
- Increase `SelfShadowIntensity` (try 0.9)
- Increase `TerrainShadowIntensity` (try 0.8-0.9)

### Performance Issues

- Reduce `LightRayMarchSteps` (try 4)
- Increase `ShadowStepSize` (try 1.0 km)
- Reduce `TerrainShadowIntensity` (less terrain shadow queries)

### Shadows Not Moving

- Check that wind is enabled on cloud component
- Verify `WindSpeed` > 0
- Ensure `UpdateCloudMovement()` is being called

### No Shadows at All

- Verify `DayNightCycleComponent` is set
- Check that sun is above horizon (`GetSunElevation() > 0`)
- Ensure `bEnableVolumetricClouds` is true
- Check that `CloudDensity` > 0

### Shadows Not Aligned with Sun

- Verify `GetSunDirection()` returns correct direction
- Check `DayNightCycleComponent` is properly configured
- Ensure material is using `SunDirection` parameter

## Advanced Usage

### Dynamic Shadow Intensity

```cpp
// Adjust shadow intensity based on weather
void UWeatherComponent::UpdateCloudShadows()
{
    float WeatherIntensity = GetCurrentWeatherIntensity();

    // Storms have darker shadows
    CloudComponent->ShadowDensityMultiplier = FMath::Lerp(
        1.0f,  // Clear weather
        1.8f,  // Storm
        WeatherIntensity
    );

    CloudComponent->TerrainShadowIntensity = FMath::Lerp(
        0.6f,  // Clear
        0.9f,  // Storm
        WeatherIntensity
    );
}
```

### Colored Cloud Lighting

```cpp
// Tint clouds based on time of day
void UpdateCloudLighting(float TimeOfDay)
{
    FLinearColor SunColor;

    if (TimeOfDay < 6.0f || TimeOfDay > 18.0f)
    {
        // Night - blue ambient
        SunColor = FLinearColor(0.3f, 0.4f, 0.6f);
    }
    else if (TimeOfDay < 8.0f || TimeOfDay > 16.0f)
    {
        // Dawn/Dusk - orange/red
        SunColor = FLinearColor(1.0f, 0.6f, 0.3f);
    }
    else
    {
        // Day - white/yellow
        SunColor = FLinearColor(1.0f, 0.95f, 0.87f);
    }

    CloudComponent->AmbientLightColor = SunColor;
}
```

### Shadow-Based Gameplay

```cpp
// Hide from enemies in cloud shadows
bool AStealthCharacter::IsInShadow()
{
    FVector CharPos = GetActorLocation();
    FVector SunDir = CloudComponent->GetSunDirection();
    float Shadow = CloudComponent->CalculateTerrainShadow(CharPos, SunDir);

    // Consider in shadow if > 50% shadowed
    return Shadow > 0.5f;
}
```

## Next Steps

1. **Integrate with Weather**: Connect cloud lighting to weather system
2. **Add God Rays**: Implement volumetric light shafts
3. **Optimize Material**: Implement GPU ray marching in material
4. **Add Effects**: Lightning, cloud glow, etc.
5. **Test in VR**: Ensure comfortable performance in VR

## Reference

- Implementation: `TASK_7.3_IMPLEMENTATION.md`
- Component: `PlanetCloudComponent.h/cpp`
- Requirements: Requirement 2.3 (Volumetric clouds with lighting)
- Related: Task 7.1 (Cloud component), Task 7.2 (Cloud shapes)
