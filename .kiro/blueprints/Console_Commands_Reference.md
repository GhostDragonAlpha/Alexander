# Console Commands Reference - Celestial Scaling System

## Overview
Quick reference for useful console commands during Blueprint implementation and testing.

**Access Console:** Press `~` (tilde key) during Play mode

---

## General Debugging Commands

### Performance Monitoring

```
stat fps
```
Display current frames per second

```
stat unit
```
Show frame time breakdown:
- Frame: Total frame time
- Game: Game thread time
- Draw: Render thread time
- GPU: GPU time

```
stat scenerendering
```
Detailed rendering statistics

```
stat game
```
Game thread statistics

```
stat memory
```
Memory usage statistics

```
stat levels
```
Level streaming statistics

---

## Player Movement Commands

### Teleport Commands

```
teleport X Y Z
```
Teleport player to absolute world coordinates (in cm)

**Examples:**
```
teleport 1051000000 0 0    // Moon orbit (10 km from Moon)
teleport 1025000000 0 0    // Earth-Moon midpoint
teleport 1000000000 0 0    // Earth orbit
teleport 100000000 0 0     // Near Sun (1,000 km)
teleport 3000000000 0 0    // Far view (30,000 km from Sun)
teleport 0 0 0             // Sun center (WARNING: extreme gravity!)
```

### Free Camera Mode

```
(Press F8 key - no console needed)
```
Toggle between player pawn and free camera
- Free camera: Fly anywhere with WASD + Mouse
- Return to pawn: Press F8 again

```
ghost
```
Enable no-clip mode (fly through objects)

```
walk
```
Disable no-clip mode (return to normal physics)

```
fly
```
Enable flying mode for player pawn

---

## Celestial Scaling Debug Commands

### Display Virtual Position

```
getall PlayerOriginManager VirtualPosition
```
Show current virtual position of all PlayerOriginManager instances

```
getall CelestialBodyComponent BodyID
```
List all registered celestial bodies

```
getall CelestialBodyComponent CurrentScaleFactor
```
Show current scale factors of all bodies

---

## Subsystem Testing Commands

### Trigger Universe Recentering

No direct command, but you can:
```
teleport [current X + 10000000] [Y] [Z]
```
Move >10 km to trigger recentering

### Check Subsystem Status

```
obj list class=GravitySimulator
```
List GravitySimulator instances

```
obj list class=RelativitySystem
```
List RelativitySystem instances

```
obj list class=CelestialBodyRegistry
```
List CelestialBodyRegistry instances

---

## VR Commands

### Toggle VR Mode

```
stereo on
```
Enable VR stereo rendering

```
stereo off
```
Disable VR stereo rendering

```
vr.bEnableStereo 1
```
Enable VR (alternative)

```
vr.bEnableStereo 0
```
Disable VR (alternative)

### HMD Commands

```
hmd enable
```
Enable head-mounted display

```
hmd disable
```
Disable head-mounted display

```
hmd stats
```
Show HMD statistics

```
hmd vsync 1
```
Enable V-Sync for VR

---

## Rendering Commands

### View Modes

```
viewmode lit
```
Normal lit view (default)

```
viewmode unlit
```
Unlit view (see emissive materials clearly)

```
viewmode wireframe
```
Wireframe view (see geometry)

```
viewmode detaillighting
```
Show lighting only

```
viewmode shadercomplexity
```
Show shader complexity (performance)

### Field of View

```
fov 90
```
Set field of view to 90 degrees (desktop)

```
fov 110
```
Wider FOV for better peripheral vision

**Note:** FOV doesn't affect VR (HMD controls FOV)

### Camera Settings

```
r.ViewDistanceScale 10
```
Increase view distance (default 1.0)

```
r.ViewDistanceScale.SecondaryScale 10
```
Secondary view distance scale

---

## Level and World Commands

### Reload Level

```
restartlevel
```
Restart current level (reloads everything)

```
open SolarSystem
```
Open SolarSystem level

### Time Control

```
slomo 0.5
```
Slow motion (0.5 = half speed)

```
slomo 2.0
```
Fast motion (2.0 = double speed)

```
slomo 1.0
```
Normal speed

**Note:** This is different from time warp (affects game simulation, not relativity)

```
pause
```
Pause the game

---

## Debugging Visuals

### Show Debug Info

```
show bounds
```
Show actor bounds

```
show collision
```
Show collision shapes

```
show navigation
```
Show navigation mesh

### Draw Debug Shapes

```
au.Debug.VisualizeQueries 1
```
Visualize physics queries

```
p.Chaos.DebugDraw.Enabled 1
```
Enable Chaos physics debug drawing

---

## Gravity and Physics Commands

### Physics Debugging

```
show particles
```
Show Niagara particle systems debug info

```
p.Gravity -980
```
Change global gravity (cm/s², -980 = Earth standard)

**Note:** This affects global gravity, not our multi-body system

---

## Network Commands (Multiplayer)

### Server/Client Commands

```
stat net
```
Show network statistics
- Ping, packet loss, bandwidth

```
net pktlag 100
```
Simulate network lag (100ms)

```
net pktloss 5
```
Simulate packet loss (5%)

---

## Custom Commands for Our System

You can add these by creating exec functions in C++ or Blueprint:

### Example Custom Commands (If Implemented)

```
ListCelestialBodies
```
List all registered celestial bodies with distances

```
SetTimeWarp 10.0
```
Activate time warp at 10× speed

```
ToggleDebugVisualization
```
Toggle debug visualization on/off

```
SetGravityMode MultiBody
```
Change gravity simulation mode

```
TeleportToBody Earth
```
Teleport player to orbit around specific body

```
ResetOrigin
```
Force universe recentering to player position

---

## Testing Sequences

### Quick Functionality Test

```
// 1. Check FPS
stat fps

// 2. Check subsystems loaded
obj list class=GravitySimulator

// 3. Teleport near Moon
teleport 1051000000 0 0

// 4. Enter free camera
(Press F8)

// 5. Fly toward Moon
(WASD to approach)

// 6. Check scaling working
(Moon should get larger)

// 7. Check performance
stat unit

// 8. Return to player
(Press F8)
```

### Gravity Test Sequence

```
// 1. Teleport to Earth-Moon midpoint
teleport 1025000000 0 0

// 2. Enable flying
fly

// 3. Disable thrust (if spaceship)
// Should see gravity pulling you

// 4. Check which direction
// Should pull toward Earth (stronger)
```

### Extreme Distance Test

```
// 1. Teleport very far
teleport 3000000000 0 0

// 2. Look back at solar system
// Sun should still be visible

// 3. Check performance
stat fps

// 4. Teleport back
teleport 1051000000 0 0
```

### VR Test Sequence

```
// 1. Check VR enabled
hmd enable

// 2. Check stereo rendering
stereo on

// 3. View HMD stats
hmd stats

// 4. Check performance
stat fps
// Should be >90 FPS
```

---

## Output Log Monitoring

### View Output Log

While in Editor:
- Window → Developer Tools → Output Log

### Filter Log Categories

In Output Log search box:
```
LogCelestialScaling
LogGravitySimulator
LogRelativitySystem
LogPlayerOrigin
LogTemp
```

### Log Verbosity

In DefaultEngine.ini or console:
```
log LogCelestialScaling Verbose
log LogCelestialScaling Display
log LogCelestialScaling Warning
log LogCelestialScaling Error
```

---

## Screenshot and Recording

### Take Screenshot

```
shot
```
Take screenshot (saved to Saved/Screenshots/)

```
shot showui
```
Screenshot including UI

```
highresshot 2
```
Take high-resolution screenshot (2× resolution)

### Recording

```
StartMovieCapture
```
Start recording video

```
StopMovieCapture
```
Stop recording video

---

## Useful Blueprint Testing Commands

### Print String Visibility

```
// If your Blueprint print strings not showing:
// Check viewport top-right: "Messages" dropdown → "Log Messages"
```

### Force Garbage Collection

```
obj gc
```
Force garbage collection (clear unused memory)

### List All Actors

```
obj list class=Actor
```
List all actors in level

```
obj list class=CelestialBodyBase
```
List all celestial bodies (if class name matches)

---

## Performance Optimization Commands

### Disable Features for Testing

```
r.PostProcessing 0
```
Disable post-processing

```
r.Bloom 0
```
Disable bloom

```
r.MotionBlur 0
```
Disable motion blur

```
r.DepthOfField 0
```
Disable depth of field

```
r.Shadows 0
```
Disable shadows (major performance gain)

```
r.Tonemapper 0
```
Disable tonemapper

### Re-enable Everything

```
r.PostProcessing 1
r.Bloom 1
r.Shadows 1
```

---

## Common Test Coordinates

Quick reference for teleport commands:

| Location | Command | Purpose |
|----------|---------|---------|
| **Moon Orbit** | `teleport 1051000000 0 0` | Default spawn (10 km from Moon) |
| **Moon Surface** | `teleport 1050100000 0 0` | Close approach (1 km from Moon) |
| **Earth Orbit** | `teleport 1004000000 0 0` | 4 km from Earth |
| **Earth-Moon Midpoint** | `teleport 1025000000 0 0` | Test multi-body gravity |
| **Far View** | `teleport 2000000000 0 0` | View whole system (20,000 km) |
| **Near Sun** | `teleport 100000000 0 0` | Test extreme gravity (1,000 km) |
| **Extreme Distance** | `teleport 10000000000 0 0` | Stress test (100,000 km) |

---

## Emergency Commands

### If Stuck or Crashed

```
restartlevel
```
Restart level

```
exit
```
Close Unreal Editor

### If Performance Tanks

```
r.Shadows 0
stat fps
obj gc
```

### If VR Not Working

```
hmd enable
stereo on
vr.bEnableStereo 1
```

---

## Creating Custom Exec Commands

To add your own console commands in Blueprint or C++:

### In C++ (GameMode or PlayerController)

```cpp
UFUNCTION(Exec, Category = "Solar System")
void TeleportToBody(FString BodyID)
{
    // Find body by ID
    // Calculate orbit distance
    // Teleport player
}

UFUNCTION(Exec, Category = "Solar System")
void SetTimeWarp(float Factor)
{
    // Call RelativitySystem
}
```

### In Blueprint

Enable "Execute in Editor" on Blueprint functions, then call them from console:
```
ke * FunctionName
```

---

**Created:** 2025-01-05
**Version:** 1.0
**For Use With:** SolarSystem level testing and debugging
