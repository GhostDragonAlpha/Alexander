# Task 11.3 Implementation: Landing Detection System

## Overview

This document describes the implementation of the landing detection system for the planet interior enhancement feature. The system provides comprehensive landing validation including slope angle checking, surface type detection, safe landing zone identification, and landing confirmation.

## Implementation Details

### 1. Landing Detection Component

**Class**: `ULandingDetectionComponent`

A component that can be attached to any actor (typically a spaceship) to provide landing detection and validation capabilities.

**Key Features**:

- Slope angle validation (configurable max angle, default 15°)
- Surface type detection and validation
- Safe landing zone detection with clearance checking
- Automatic landing confirmation system
- Landing approach detection
- Debug visualization

### 2. Slope Angle Validation

**Methods Implemented**:

- `ValidateSlopeAngle()` - Validates if slope is within acceptable range
- `CalculateSlopeAngle()` - Calculates slope using multiple sample points
- `GetSurfaceNormal()` - Gets surface normal at a location

**Behavior**:

- Samples terrain at 8 points in a circle around the landing location
- Calculates height differences to determine slope angle
- Configurable sample radius (default 5 meters)
- Returns slope angle in degrees
- Validates against maximum slope angle (default 15°)

**Algorithm**:

1. Sample terrain heights at multiple points in a circle
2. Find minimum and maximum heights
3. Calculate slope angle using: `atan2(height_difference, sample_radius)`
4. Convert to degrees and compare against threshold

### 3. Surface Type Detection

**Surface Types Supported**:

- Rock
- Dirt
- Sand
- Grass
- Snow
- Ice
- Water
- Lava
- Metal (Landing Pads)
- Unknown

**Methods Implemented**:

- `DetectSurfaceType()` - Detects surface type at a location
- `IsSurfaceTypeLandable()` - Checks if surface type is safe
- `GetSurfaceTypeFromPhysMaterial()` - Maps physical materials to surface types

**Landable Surfaces** (default):

- Rock ✓
- Dirt ✓
- Sand ✓
- Grass ✓
- Snow ✓
- Metal (Landing Pads) ✓

**Non-Landable Surfaces**:

- Water ✗
- Lava ✗
- Ice ✗ (too slippery)

**Detection Method**:

1. Perform line trace to ground
2. Check if hit actor is a landing pad (automatic Metal type)
3. Get physical material from hit result
4. Map physical material name to surface type enum
5. Validate against landable surface types set

### 4. Safe Landing Zone Detection

**Methods Implemented**:

- `ValidateLandingZone()` - Comprehensive landing zone validation
- `CheckClearance()` - Checks for obstacles around landing zone
- `FindNearestSafeLandingZone()` - Searches for safe landing location

**Validation Checks**:

1. **Slope Validation**:

   - Checks if terrain slope is ≤ 15° (configurable)
   - Returns failure if too steep

2. **Surface Type Validation**:

   - Detects surface type at location
   - Checks if surface is in landable types set
   - Returns failure if unsafe surface (water, lava, ice)

3. **Clearance Validation**:
   - Checks for obstacles in cylindrical volume around landing zone
   - Default clearance radius: 10 meters
   - Checks at multiple heights (0m, 2.5m, 5m)
   - Uses sphere sweeps to detect obstacles
   - Returns failure if insufficient clearance

**Validation Result Structure**:

```cpp
struct FLandingZoneValidation
{
    bool bIsSafe;                    // Overall safety status
    float SlopeAngle;                // Calculated slope angle
    ESurfaceType SurfaceType;        // Detected surface type
    float ClearanceDistance;         // Distance to nearest obstacle
    bool bIsFlatEnough;              // Slope check result
    bool bIsLandableSurface;         // Surface type check result
    bool bHasClearance;              // Clearance check result
    FString FailureReason;           // Detailed failure reason
};
```

### 5. Landing Confirmation System

**Methods Implemented**:

- `ConfirmLanding()` - Confirms landing at current location
- `CancelLanding()` - Cancels landing (takeoff)
- `IsLanded()` - Checks if currently landed
- `GetLandingConfirmation()` - Gets landing confirmation data
- `IsInLandingApproach()` - Checks if in landing approach

**Landing Confirmation Process**:

1. **Validation Phase**:

   - Validates landing zone (slope, surface, clearance)
   - Checks landing velocity (max 5 m/s by default)
   - Detects if landing on a designated landing pad

2. **Confirmation Phase**:

   - Records landing location
   - Records landing time
   - Records landing velocity
   - Identifies landing pad (if applicable)
   - Broadcasts `OnLandingConfirmed` event

3. **Auto-Confirmation**:
   - Automatically confirms landing after 0.5 seconds on ground
   - Monitors ground contact continuously
   - Cancels landing when actor leaves ground

**Landing Confirmation Structure**:

```cpp
struct FLandingConfirmation
{
    bool bIsLanded;              // Landing status
    FVector LandingLocation;     // Where landed
    float LandingTime;           // When landed (game time)
    float LandingVelocity;       // Landing speed (cm/s)
    bool bOnLandingPad;          // On designated pad
    AActor* LandingPad;          // Landing pad actor reference
};
```

### 6. Landing Approach Detection

**Method**: `IsInLandingApproach()`

**Behavior**:

- Checks if actor is within 50 meters of ground
- Calculates approach angle (angle between velocity and down vector)
- Returns true if descending toward ground
- Provides distance to ground and approach angle

**Use Cases**:

- Triggering landing guidance UI
- Activating landing gear
- Adjusting flight controls for landing mode
- Playing landing approach audio

### 7. Configuration Options

**Configurable Parameters**:

```cpp
// Slope validation
float MaxSlopeAngle = 15.0f;              // Maximum safe slope (degrees)
int32 SlopeSampleCount = 8;               // Number of sample points
float SlopeSampleRadius = 500.0f;         // Sample radius (cm)

// Clearance checking
float MinClearanceRadius = 1000.0f;       // Minimum clearance (cm)
float ClearanceCheckHeight = 500.0f;      // Height to check (cm)

// Landing velocity
float MaxLandingVelocity = 500.0f;        // Maximum safe velocity (cm/s)

// Landable surfaces
TSet<ESurfaceType> LandableSurfaceTypes;  // Set of safe surface types

// Debug
bool bShowDebugVisualization = false;     // Enable debug drawing
```

### 8. Events and Delegates

**Event Delegates**:

1. **OnLandingConfirmed**:

   - Fired when landing is confirmed
   - Provides `FLandingConfirmation` data
   - Use for: disabling flight controls, playing landing animation, etc.

2. **OnTakeoff**:

   - Fired when actor leaves ground after landing
   - Use for: re-enabling flight controls, playing takeoff animation, etc.

3. **OnUnsafeLandingZone**:
   - Fired when attempting to land in unsafe zone
   - Provides failure reason string
   - Use for: displaying warning to player, preventing landing, etc.

### 9. Integration with Landing Pads

**Automatic Detection**:

- Detects when landing on `ALandingPad` actors
- Automatically sets surface type to Metal
- Records landing pad reference in confirmation
- Landing pads are always considered safe surfaces

**Landing Pad Benefits**:

- Guaranteed flat surface (0° slope)
- Guaranteed clearance
- Designated safe zones
- Integration with pad reservation system

### 10. Debug Visualization

**When Enabled** (`bShowDebugVisualization = true`):

**Visual Elements**:

- Green/Red circle showing landing zone (safe/unsafe)
- Yellow spheres at slope sample points
- Status text showing:
  - Landing zone safety status
  - Slope angle
  - Surface type
  - Clearance distance
- Cyan sphere at landing location when landed
- "LANDED" text when confirmed

**Usage**:

- Enable in editor for testing
- Disable in shipping builds for performance
- Useful for tuning landing parameters

## Usage Examples

### Example 1: Basic Landing Detection

```cpp
// Add component to spaceship
ULandingDetectionComponent* LandingDetection = CreateDefaultSubobject<ULandingDetectionComponent>(TEXT("LandingDetection"));

// Configure parameters
LandingDetection->MaxSlopeAngle = 15.0f;
LandingDetection->MaxLandingVelocity = 500.0f;
LandingDetection->bShowDebugVisualization = true;

// Bind to events
LandingDetection->OnLandingConfirmed.AddDynamic(this, &ASpaceship::OnLanded);
LandingDetection->OnTakeoff.AddDynamic(this, &ASpaceship::OnTookOff);
LandingDetection->OnUnsafeLandingZone.AddDynamic(this, &ASpaceship::OnUnsafeLanding);
```

### Example 2: Manual Landing Validation

```cpp
// Check if current location is safe for landing
FVector CurrentLocation = GetActorLocation();
FLandingZoneValidation Validation = LandingDetection->ValidateLandingZone(CurrentLocation);

if (Validation.bIsSafe)
{
    UE_LOG(LogTemp, Log, TEXT("Safe to land!"));
}
else
{
    UE_LOG(LogTemp, Warning, TEXT("Unsafe: %s"), *Validation.FailureReason);
}
```

### Example 3: Finding Safe Landing Zone

```cpp
// Search for nearest safe landing zone
FVector CurrentLocation = GetActorLocation();
FVector SafeLocation;

if (LandingDetection->FindNearestSafeLandingZone(CurrentLocation, 10000.0f, SafeLocation))
{
    // Navigate to safe location
    NavigateToLocation(SafeLocation);
}
else
{
    UE_LOG(LogTemp, Warning, TEXT("No safe landing zone found nearby"));
}
```

### Example 4: Landing Approach Detection

```cpp
// Check if in landing approach
float DistanceToGround;
float ApproachAngle;

if (LandingDetection->IsInLandingApproach(DistanceToGround, ApproachAngle))
{
    // Show landing guidance UI
    ShowLandingGuidance(DistanceToGround, ApproachAngle);

    // Deploy landing gear
    if (DistanceToGround < 2000.0f) // Within 20m
    {
        DeployLandingGear();
    }
}
```

### Example 5: Manual Landing Confirmation

```cpp
// Manually confirm landing (e.g., when player presses land button)
FVector CurrentVelocity = GetVelocity();

if (LandingDetection->ConfirmLanding(CurrentVelocity))
{
    // Landing successful
    DisableFlightControls();
    PlayLandingAnimation();
}
else
{
    // Landing failed - warning already broadcast via OnUnsafeLandingZone
}
```

## Requirements Satisfied

This implementation satisfies **Requirement 9.3** from the design document:

- ✓ **Slope angle validation**: Maximum 15° slope angle check with configurable threshold
- ✓ **Surface type checking**: Detects and validates 9 different surface types
- ✓ **Safe landing zone detection**: Comprehensive validation including slope, surface, and clearance
- ✓ **Landing confirmation system**: Automatic and manual confirmation with detailed tracking

## Performance Considerations

**Optimization Strategies**:

1. **Tick Optimization**:

   - Component ticks in `TG_PostPhysics` group
   - Only performs checks when needed
   - Auto-confirmation uses simple ground distance check

2. **Trace Optimization**:

   - Uses single line traces for most checks
   - Sphere sweeps only for clearance validation
   - Ignores owner actor in all traces

3. **Sample Count**:

   - Default 8 sample points for slope (configurable)
   - 12 radial samples for clearance
   - 3 height levels for clearance

4. **Debug Visualization**:
   - Only active when explicitly enabled
   - Can be disabled in shipping builds
   - No performance impact when disabled

**Performance Targets**:

- Slope validation: < 0.5ms
- Surface type detection: < 0.1ms
- Clearance checking: < 1.0ms
- Total validation: < 2ms per call

## Testing Recommendations

### Unit Tests

1. **Slope Validation**:

   - Test flat terrain (0°) - should pass
   - Test 15° slope - should pass
   - Test 20° slope - should fail
   - Test 45° slope - should fail

2. **Surface Type Detection**:

   - Test each surface type
   - Test landing pad detection
   - Test physical material mapping

3. **Clearance Checking**:
   - Test open area - should pass
   - Test area with obstacles - should fail
   - Test area with distant obstacles - should pass

### Integration Tests

1. **Landing Sequence**:

   - Approach from altitude
   - Verify approach detection
   - Land on flat terrain
   - Verify landing confirmation
   - Takeoff
   - Verify landing cancellation

2. **Landing Pad Integration**:

   - Land on designated pad
   - Verify pad detection
   - Verify Metal surface type
   - Verify landing confirmation includes pad reference

3. **Unsafe Landing Attempts**:
   - Attempt landing on steep slope
   - Attempt landing on water
   - Attempt landing near obstacles
   - Verify appropriate warnings

### Visual Tests

1. **Debug Visualization**:

   - Enable debug drawing
   - Verify circle colors (green/red)
   - Verify sample point visualization
   - Verify status text accuracy

2. **Landing Scenarios**:
   - Test various terrain types
   - Test different slope angles
   - Test different clearance situations
   - Verify visual feedback matches validation

## Future Enhancements

Potential improvements for future iterations:

1. **Advanced Surface Detection**:

   - Terrain moisture detection
   - Surface stability analysis
   - Dynamic surface changes (e.g., shifting sand)

2. **Weather Integration**:

   - Wind speed affecting landing difficulty
   - Visibility affecting landing guidance
   - Rain/snow affecting surface friction

3. **Damage System**:

   - Hard landing damage based on velocity
   - Slope landing damage
   - Unsafe surface damage over time

4. **AI Integration**:

   - Auto-landing system using safe zone detection
   - Landing approach path planning
   - Emergency landing zone selection

5. **Multiplayer Support**:
   - Landing zone reservation
   - Collision avoidance during landing
   - Synchronized landing confirmations

## Conclusion

The landing detection system provides a comprehensive and robust solution for validating landing zones and confirming landings. The system is highly configurable, performant, and integrates seamlessly with the existing landing pad system. All requirements from the specification have been met, and the implementation is ready for integration with spaceship flight controls and landing guidance UI.
