# Quick Start: Landing Detection System

## Overview

The Landing Detection System provides automatic validation and confirmation of spacecraft landings on planetary surfaces. It checks slope angles, surface types, and clearance to ensure safe landings.

## Quick Setup (5 minutes)

### 1. Add Component to Your Spaceship

**In C++**:

```cpp
// In your spaceship header
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landing")
ULandingDetectionComponent* LandingDetection;

// In constructor
LandingDetection = CreateDefaultSubobject<ULandingDetectionComponent>(TEXT("LandingDetection"));
```

**In Blueprint**:

1. Open your spaceship Blueprint
2. Click "Add Component"
3. Search for "Landing Detection Component"
4. Add it to your actor

### 2. Configure Basic Settings

**Recommended Settings**:

- Max Slope Angle: `15.0` (degrees)
- Max Landing Velocity: `500.0` (cm/s = 5 m/s)
- Min Clearance Radius: `1000.0` (cm = 10 meters)
- Show Debug Visualization: `true` (for testing)

### 3. Bind to Events

**In C++**:

```cpp
void ASpaceship::BeginPlay()
{
    Super::BeginPlay();

    if (LandingDetection)
    {
        LandingDetection->OnLandingConfirmed.AddDynamic(this, &ASpaceship::HandleLandingConfirmed);
        LandingDetection->OnTakeoff.AddDynamic(this, &ASpaceship::HandleTakeoff);
        LandingDetection->OnUnsafeLandingZone.AddDynamic(this, &ASpaceship::HandleUnsafeLanding);
    }
}

void ASpaceship::HandleLandingConfirmed(const FLandingConfirmation& Confirmation)
{
    UE_LOG(LogTemp, Log, TEXT("Landed at %s"), *Confirmation.LandingLocation.ToString());
    // Disable flight controls, play animation, etc.
}

void ASpaceship::HandleTakeoff()
{
    UE_LOG(LogTemp, Log, TEXT("Taking off"));
    // Re-enable flight controls
}

void ASpaceship::HandleUnsafeLanding(const FString& Reason)
{
    UE_LOG(LogTemp, Warning, TEXT("Unsafe landing: %s"), *Reason);
    // Show warning to player
}
```

**In Blueprint**:

1. Select Landing Detection Component
2. In Details panel, scroll to Events
3. Click "+" next to "On Landing Confirmed"
4. Implement your landing logic
5. Repeat for "On Takeoff" and "On Unsafe Landing Zone"

## Common Use Cases

### Use Case 1: Automatic Landing Detection

The component automatically detects and confirms landings:

```cpp
// No code needed! Component handles it automatically
// Just bind to OnLandingConfirmed event
```

**How it works**:

1. Component monitors ground distance every frame
2. When actor touches ground, starts timer
3. After 0.5 seconds on ground, validates landing zone
4. If safe, confirms landing and broadcasts event
5. When actor leaves ground, cancels landing

### Use Case 2: Manual Landing Validation

Check if current location is safe before landing:

```cpp
void ASpaceship::CheckLandingZone()
{
    FVector CurrentLocation = GetActorLocation();
    FLandingZoneValidation Validation = LandingDetection->ValidateLandingZone(CurrentLocation);

    if (Validation.bIsSafe)
    {
        // Show green landing indicator
        ShowLandingIndicator(FColor::Green);
    }
    else
    {
        // Show red landing indicator with reason
        ShowLandingIndicator(FColor::Red);
        ShowWarning(Validation.FailureReason);
    }
}
```

### Use Case 3: Landing Approach Detection

Detect when player is approaching for landing:

```cpp
void ASpaceship::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    float DistanceToGround;
    float ApproachAngle;

    if (LandingDetection->IsInLandingApproach(DistanceToGround, ApproachAngle))
    {
        // Show landing guidance UI
        if (LandingGuidanceWidget)
        {
            LandingGuidanceWidget->SetVisibility(ESlateVisibility::Visible);
            LandingGuidanceWidget->UpdateAltitude(DistanceToGround / 100.0f); // Convert to meters
            LandingGuidanceWidget->UpdateApproachAngle(ApproachAngle);
        }

        // Deploy landing gear when close
        if (DistanceToGround < 2000.0f && !bLandingGearDeployed)
        {
            DeployLandingGear();
        }
    }
    else
    {
        // Hide landing guidance UI
        if (LandingGuidanceWidget)
        {
            LandingGuidanceWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}
```

### Use Case 4: Find Safe Landing Zone

Search for a safe landing location nearby:

```cpp
void ASpaceship::FindEmergencyLandingZone()
{
    FVector CurrentLocation = GetActorLocation();
    FVector SafeLocation;

    // Search within 10km radius
    if (LandingDetection->FindNearestSafeLandingZone(CurrentLocation, 1000000.0f, SafeLocation))
    {
        // Navigate to safe location
        UE_LOG(LogTemp, Log, TEXT("Safe landing zone found at %s"), *SafeLocation.ToString());
        SetAutopilotDestination(SafeLocation);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("No safe landing zone found!"));
        // Emergency procedures
    }
}
```

### Use Case 5: Manual Landing Confirmation

Force landing confirmation (e.g., when player presses "Land" button):

```cpp
void ASpaceship::OnLandButtonPressed()
{
    FVector CurrentVelocity = GetVelocity();

    if (LandingDetection->ConfirmLanding(CurrentVelocity))
    {
        // Landing successful
        UE_LOG(LogTemp, Log, TEXT("Landing confirmed"));
        DisableFlightControls();
        PlayLandingAnimation();
        PlayLandingSound();
    }
    else
    {
        // Landing failed - warning already shown via OnUnsafeLandingZone event
        UE_LOG(LogTemp, Warning, TEXT("Cannot land here"));
    }
}
```

## Understanding Validation Results

### FLandingZoneValidation Structure

```cpp
struct FLandingZoneValidation
{
    bool bIsSafe;                    // Overall result
    float SlopeAngle;                // Terrain slope (degrees)
    ESurfaceType SurfaceType;        // Surface material
    float ClearanceDistance;         // Distance to obstacles (cm)
    bool bIsFlatEnough;              // Slope check passed
    bool bIsLandableSurface;         // Surface check passed
    bool bHasClearance;              // Clearance check passed
    FString FailureReason;           // Why it failed
};
```

**Example Usage**:

```cpp
FLandingZoneValidation Validation = LandingDetection->ValidateLandingZone(Location);

if (!Validation.bIsSafe)
{
    if (!Validation.bIsFlatEnough)
    {
        // Terrain too steep
        UE_LOG(LogTemp, Warning, TEXT("Slope: %.1f° (max 15°)"), Validation.SlopeAngle);
    }
    else if (!Validation.bIsLandableSurface)
    {
        // Unsafe surface (water, lava, etc.)
        UE_LOG(LogTemp, Warning, TEXT("Surface: %s"), *UEnum::GetValueAsString(Validation.SurfaceType));
    }
    else if (!Validation.bHasClearance)
    {
        // Obstacles nearby
        UE_LOG(LogTemp, Warning, TEXT("Clearance: %.1fm"), Validation.ClearanceDistance / 100.0f);
    }
}
```

## Surface Types

### Landable Surfaces ✓

- **Rock**: Solid, safe for landing
- **Dirt**: Stable, safe for landing
- **Sand**: Stable enough for landing
- **Grass**: Safe for landing
- **Snow**: Packed snow is safe
- **Metal**: Landing pads (always safe)

### Non-Landable Surfaces ✗

- **Water**: Ship would sink
- **Lava**: Ship would be destroyed
- **Ice**: Too slippery, unstable

### Customizing Landable Surfaces

```cpp
// Add ice as landable (if you have special landing gear)
LandingDetection->LandableSurfaceTypes.Add(ESurfaceType::Ice);

// Remove sand as landable (if too soft for your ship)
LandingDetection->LandableSurfaceTypes.Remove(ESurfaceType::Sand);
```

## Debug Visualization

### Enabling Debug Mode

**In Editor**:

1. Select your spaceship in the level
2. Find Landing Detection Component
3. Check "Show Debug Visualization"

**In Code**:

```cpp
LandingDetection->bShowDebugVisualization = true;
```

### What You'll See

- **Green Circle**: Safe landing zone
- **Red Circle**: Unsafe landing zone
- **Yellow Spheres**: Slope sample points
- **Status Text**: Detailed validation info
- **Cyan Sphere**: Current landing location (when landed)

### Interpreting Debug Info

```
Landing Zone: SAFE
Slope: 8.3°
Surface: Rock
Clearance: 15.2m
```

- **Landing Zone**: Overall safety status
- **Slope**: Terrain angle (must be ≤ 15°)
- **Surface**: Detected surface type
- **Clearance**: Distance to nearest obstacle

## Troubleshooting

### Problem: Landing not detected

**Solutions**:

1. Check that actor has physics enabled
2. Verify actor is within 2m of ground
3. Check that ground has collision enabled
4. Ensure component is ticking

### Problem: False "unsafe" warnings

**Solutions**:

1. Adjust `MaxSlopeAngle` if terrain is naturally steep
2. Adjust `MinClearanceRadius` if obstacles are acceptable
3. Check surface type detection with debug visualization
4. Verify physical materials are set up correctly

### Problem: Landing too sensitive

**Solutions**:

1. Increase `MinGroundContactTime` (default 0.5s)
2. Increase ground distance threshold (default 2m)
3. Adjust `MaxLandingVelocity` if landing too fast

### Problem: Performance issues

**Solutions**:

1. Disable debug visualization in shipping builds
2. Reduce `SlopeSampleCount` (default 8)
3. Increase `MinClearanceRadius` to reduce sweep checks
4. Only enable component when near ground

## Best Practices

### 1. Always Bind to Events

```cpp
// Good: Handle all events
LandingDetection->OnLandingConfirmed.AddDynamic(this, &ASpaceship::HandleLanding);
LandingDetection->OnTakeoff.AddDynamic(this, &ASpaceship::HandleTakeoff);
LandingDetection->OnUnsafeLandingZone.AddDynamic(this, &ASpaceship::HandleUnsafe);

// Bad: Only handling one event
LandingDetection->OnLandingConfirmed.AddDynamic(this, &ASpaceship::HandleLanding);
```

### 2. Validate Before Manual Landing

```cpp
// Good: Check before confirming
FLandingZoneValidation Validation = LandingDetection->ValidateLandingZone(GetActorLocation());
if (Validation.bIsSafe)
{
    LandingDetection->ConfirmLanding(GetVelocity());
}

// Bad: Confirm without checking
LandingDetection->ConfirmLanding(GetVelocity());
```

### 3. Use Landing Approach Detection

```cpp
// Good: Show guidance when approaching
if (LandingDetection->IsInLandingApproach(Distance, Angle))
{
    ShowLandingGuidance();
}

// Bad: Always show guidance
ShowLandingGuidance();
```

### 4. Disable Debug in Shipping

```cpp
// Good: Only debug in development
#if !UE_BUILD_SHIPPING
    LandingDetection->bShowDebugVisualization = true;
#endif

// Bad: Debug always enabled
LandingDetection->bShowDebugVisualization = true;
```

## Integration with Other Systems

### Landing Pads

Landing detection automatically recognizes landing pads:

```cpp
void ASpaceship::HandleLandingConfirmed(const FLandingConfirmation& Confirmation)
{
    if (Confirmation.bOnLandingPad)
    {
        // Landed on designated pad
        ALandingPad* Pad = Cast<ALandingPad>(Confirmation.LandingPad);
        if (Pad)
        {
            UE_LOG(LogTemp, Log, TEXT("Landed on pad: %s"), *Pad->PadName);
            // Access pad services, refueling, etc.
        }
    }
    else
    {
        // Landed on natural terrain
        UE_LOG(LogTemp, Log, TEXT("Landed on terrain"));
    }
}
```

### Landing Guidance UI

Combine with landing guidance widget:

```cpp
void ASpaceship::Tick(float DeltaTime)
{
    float Distance, Angle;
    if (LandingDetection->IsInLandingApproach(Distance, Angle))
    {
        // Update guidance UI
        LandingGuidanceWidget->UpdateAltitude(Distance / 100.0f);
        LandingGuidanceWidget->UpdateApproachAngle(Angle);

        // Validate current zone
        FLandingZoneValidation Validation = LandingDetection->ValidateLandingZone(GetActorLocation());
        LandingGuidanceWidget->UpdateZoneSafety(Validation.bIsSafe);
        LandingGuidanceWidget->UpdateSlopeAngle(Validation.SlopeAngle);
    }
}
```

### Flight Controls

Adjust controls based on landing state:

```cpp
void ASpaceship::UpdateFlightControls()
{
    if (LandingDetection->IsLanded())
    {
        // Disable flight controls
        bCanFly = false;
        bCanBoost = false;

        // Enable ground controls
        bCanWalk = true;
    }
    else
    {
        // Enable flight controls
        bCanFly = true;
        bCanBoost = true;

        // Disable ground controls
        bCanWalk = false;
    }
}
```

## Next Steps

1. **Test in Editor**: Enable debug visualization and test various landing scenarios
2. **Tune Parameters**: Adjust slope angle, clearance, and velocity for your game
3. **Integrate UI**: Connect to landing guidance widget
4. **Add Audio**: Play landing sounds on confirmation
5. **Add Animations**: Trigger landing gear deployment and landing animations

## Additional Resources

- **Full Documentation**: `TASK_11.3_IMPLEMENTATION.md`
- **Landing Pad System**: `QUICKSTART_LandingPads.md`
- **Landing Guidance UI**: `QUICKSTART_LandingGuidanceUI.md`
- **API Reference**: See `LandingDetectionComponent.h` for complete API

## Support

For issues or questions:

1. Check debug visualization to understand validation failures
2. Review troubleshooting section above
3. Consult full implementation documentation
4. Check that all requirements are met (slope, surface, clearance)
