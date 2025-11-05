# Quick Start: Landing Pad System

## Overview

The Landing Pad system provides designated landing zones on planetary surfaces with automatic reservation management, approach guidance, and visual indicators.

## Quick Setup (5 Minutes)

### 1. Place a Landing Pad

1. Open your planet level in Unreal Editor
2. In Content Browser, search for `LandingPad`
3. Drag `ALandingPad` into your level
4. Position it on a relatively flat area of your planet surface

### 2. Basic Configuration

Select the landing pad and set these properties:

```
Landing Pad:
  - Pad Name: "Landing Pad Alpha"
  - Pad ID: 1
  - Pad Size: (20.0, 20.0)  // 20m x 20m

Approach:
  - Approach Altitude: 100.0  // 100m above pad
  - Approach Distance: 200.0  // 200m from pad
  - Approach Angle: 0.0       // Straight approach

Visuals:
  - Show Approach Markers: ✓
  - Show Status Lights: ✓
```

### 3. Test in Play Mode

1. Press Play
2. Look for the green arrow showing approach direction
3. The pad should show green status (Available)

## Common Use Cases

### Reserve a Pad from Blueprint

```
// Get reference to landing pad
Landing Pad = [Your Landing Pad Reference]

// Reserve for 5 minutes
Success = Landing Pad->Reserve Pad(Self, 300.0)

if Success:
    // Get approach position
    Approach Position = Landing Pad->Get Approach Position()

    // Navigate ship to approach position
    Navigate To(Approach Position)
```

### Check Approach Alignment

```
// During approach
Is In Corridor = Landing Pad->Is In Approach Corridor(Self, Distance, Alignment)

if Is In Corridor:
    // Show green indicator
    Set HUD Color(Green)
else:
    // Show correction needed
    Set HUD Color(Yellow)

// Display alignment percentage
Display Text("Alignment: " + (Alignment * 100) + "%")
```

### Handle Landing Events

Create a Blueprint child of ALandingPad and override these events:

```
Event On Ship Landed(Ship):
    // Play effects
    Spawn Emitter at Location(Landing Effect, Pad Location)
    Play Sound at Location(Landing Sound, Pad Location)

    // Notify player
    Display Message("Welcome to " + Pad Name)

Event On Ship Departed(Ship):
    // Play takeoff effects
    Spawn Emitter at Location(Takeoff Effect, Pad Location)
    Play Sound at Location(Takeoff Sound, Pad Location)
```

## Status Colors

The landing pad automatically changes color based on status:

- 🟢 **Green**: Available for landing
- 🟡 **Yellow**: Reserved (someone has claimed it)
- 🔴 **Red**: Occupied (ship currently on pad)
- ⚫ **Gray**: Disabled (not accepting landings)

## Approach Guidance

### Visual Markers

When `Show Approach Markers` is enabled:

- Green sphere shows ideal approach position
- Line connects approach position to pad
- Arrow on pad shows approach direction

### Approach Corridor

The system validates if your ship is in the correct approach:

- **Distance**: Within 100m of ideal approach position
- **Alignment**: At least 70% aligned with approach vector

### Getting Approach Info

```cpp
// C++ example
FVector ApproachPos = LandingPad->GetApproachPosition();
FVector ApproachDir = LandingPad->GetApproachDirection();

float Distance;
float Alignment;
bool InCorridor = LandingPad->IsInApproachCorridor(MyShip, Distance, Alignment);
```

## Reservation System

### How It Works

1. **Reserve**: Ship requests pad (default 5 minute hold)
2. **Approach**: Ship navigates to approach position
3. **Land**: Ship enters landing zone, status → Occupied
4. **Depart**: Ship leaves zone, pad released → Available

### Reservation Expiration

Reservations automatically expire after the duration:

- Default: 300 seconds (5 minutes)
- Configurable per reservation
- Automatic release on expiration

### Force Release

For debugging or admin purposes:

```
Landing Pad->Force Release()
```

## Terrain Flattening

### Automatic Flattening

When `Flatten Terrain` is enabled:

- Flattens terrain within `Flatten Radius` (default 25m)
- Smooth blend over `Flatten Blend Distance` (default 5m)
- Checks terrain suitability on placement

### Terrain Suitability

The system checks if terrain is suitable:

- Maximum slope angle (default 5°)
- Samples terrain in circle around pad
- Warns if placed on unsuitable terrain

## Advanced Configuration

### Multiple Pads

For landing zones with multiple pads:

```
Pad 1:
  - Pad ID: 1
  - Pad Name: "Alpha"
  - Approach Angle: 0°

Pad 2:
  - Pad ID: 2
  - Pad Name: "Bravo"
  - Approach Angle: 180°  // Opposite direction

Pad 3:
  - Pad ID: 3
  - Pad Name: "Charlie"
  - Approach Angle: 90°   // Side approach
```

### Custom Approach Patterns

For complex landing zones:

```
// Orbital approach
Approach Altitude: 500.0   // High altitude
Approach Distance: 1000.0  // Far distance
Approach Angle: 0.0

// Ground-level approach
Approach Altitude: 50.0    // Low altitude
Approach Distance: 300.0   // Medium distance
Approach Angle: 0.0

// Side approach (carrier landing)
Approach Altitude: 100.0
Approach Distance: 500.0
Approach Angle: 90.0       // Perpendicular
```

## Troubleshooting

### Pad Not Visible

- Check `Show Approach Markers` is enabled
- Verify pad has a mesh assigned
- Check pad is not below terrain

### Can't Reserve Pad

- Check pad status (must be Available)
- Verify pad is not Disabled
- Check if another ship has reservation

### Ship Not Detected

- Ensure ship is a Pawn or has "Ship" tag
- Check Landing Zone collision is enabled
- Verify ship has collision enabled

### Terrain Warning

- Pad placed on slope > Max Slope Angle
- Move pad to flatter terrain
- Or increase Max Slope Angle (not recommended)

## Blueprint Examples

### Find Nearest Available Pad

```
// Get all landing pads
Get All Actors of Class(Landing Pad) → Pads Array

// Filter available pads
For Each(Pads Array):
    if Pad->Is Available():
        Add to Available Pads Array

// Find nearest
Nearest Pad = None
Nearest Distance = 999999.0

For Each(Available Pads Array):
    Distance = Vector Distance(Ship Location, Pad Location)
    if Distance < Nearest Distance:
        Nearest Distance = Distance
        Nearest Pad = Pad

Return Nearest Pad
```

### Auto-Landing Sequence

```
// Phase 1: Reserve pad
if Landing Pad->Reserve Pad(Self, 300.0):

    // Phase 2: Navigate to approach
    Approach Pos = Landing Pad->Get Approach Position()
    Move To(Approach Pos)

    // Phase 3: Align with approach vector
    Approach Dir = Landing Pad->Get Approach Direction()
    Rotate To(Approach Dir)

    // Phase 4: Check alignment
    Is In Corridor = Landing Pad->Is In Approach Corridor(Self, Dist, Align)

    if Is In Corridor and Align > 0.9:
        // Phase 5: Descend to pad
        Pad Location = Landing Pad->Get Actor Location()
        Descend To(Pad Location)
```

## Performance Notes

- Landing pads are lightweight actors
- Tick only updates reservation timer and visuals
- Collision uses simple box overlap
- Debug visualization can be disabled in shipping builds

## Next Steps

- Implement ship landing controller
- Add landing guidance UI
- Create landing effects (particles, sounds)
- Integrate with refueling/repair systems
- Add traffic control for multiple pads

## Related Systems

- **Planet System**: Provides terrain for pad placement
- **Ship Controller**: Handles ship navigation and landing
- **UI System**: Displays landing guidance and pad status
- **Weather System**: May affect landing conditions (future)

## Support

For issues or questions:

1. Check console logs for warnings
2. Enable debug visualization
3. Verify pad configuration
4. Test with simple ship first

## Version

- Implementation: Task 11.1
- Requirements: 9.1, 9.4
- Status: Complete
