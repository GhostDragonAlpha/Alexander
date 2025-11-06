# SolarSystem.umap - Level Layout Guide

## Overview
Complete guide for setting up the SolarSystem test level with all celestial bodies, lighting, and player spawn points.

---

## Level Setup Steps

### Step 1: Open and Configure Level

1. **Open Level:**
   - Content Browser → Navigate to `Content/SolarSystem.umap`
   - Double-click to open in editor

2. **Set World Settings:**
   - Window → World Settings
   - Game Mode → Game Mode Override: `BP_SolarSystemGameMode`
   - Kill Z: `-10000000.0` (very low, prevent falling through space)
   - World Bounds Extent: `10000000.0` (very large for space)

3. **Clear Existing Content (if needed):**
   - If level has placeholder content, delete or hide it
   - Keep any useful lighting or post-process volumes

---

## Actor Placement - Compressed Distances Layout

**Recommended for Initial Testing** - All bodies within ~30,000 km

### Coordinate System
```
Origin: (0, 0, 0) - Center of solar system (Sun location)
Axis: X = orbital plane, Y = perpendicular, Z = up
Units: Unreal centimeters (100 cm = 1 meter, 100,000 cm = 1 km)
```

### Sun Placement
```
Actor: BP_Sun
Location: (0, 0, 0)
Rotation: (0, 0, 0)
Scale: (1, 1, 1) - Will be dynamically scaled by CelestialBodyComponent

Notes:
- Center of the solar system
- Primary light source
- Largest gravitational influence
```

### Earth Placement
```
Actor: BP_Earth
Location: (1000000000, 0, 0) cm
         = (10,000,000 meters, 0, 0)
         = (10,000 km, 0, 0) from Sun
Rotation: (0, 0, 0)
Scale: (1, 1, 1)

Notes:
- 10,000 km from Sun (1/15,000 of real distance)
- Visible from spawn point
- Strong enough gravity to demonstrate multi-body physics
```

### Moon Placement
```
Actor: BP_Moon
Location: (1050000000, 0, 0) cm
         = (10,500,000 meters, 0, 0)
         = (10,500 km from Sun, 500 km from Earth)
Rotation: (0, 0, 0)
Scale: (1, 1, 1)

Notes:
- 500 km from Earth (1/768 of real distance)
- Player will spawn near Moon
- Small enough to test scaling from close approach
```

### Mars Placement (Optional - Phase 2)
```
Actor: BP_Mars
Location: (1500000000, 0, 0) cm
         = (15,000,000 meters, 0, 0)
         = (15,000 km from Sun, 5,000 km from Earth)
Rotation: (0, 0, 0)
Scale: (1, 1, 1)

Notes:
- 5,000 km beyond Earth
- Good for testing distant body scaling
- Adds another gravity source
```

### Jupiter Placement (Optional - Future)
```
Actor: BP_Jupiter
Location: (3000000000, 0, 0) cm
         = (30,000 km from Sun, 20,000 km from Earth)
Rotation: (0, 0, 0)
Scale: (1, 1, 1)

Notes:
- Very distant, may not be visible initially
- Massive gravity when approached
- Test extreme distance scaling
```

---

## Player Start Placement

### Primary Spawn Point - Moon Orbit
```
Actor: PlayerStart (or BP_SpacePlayerStart)
Location: (1051000000, 0, 0) cm
         = (10,510 km from Sun, 510 km from Earth, 10 km from Moon)
Rotation: (0, 180, 0) - Facing back toward Earth/Sun
Scale: (1, 1, 1)

Why This Location:
- Close to Moon (10 km) - can see it clearly
- Moon is properly scaled at this distance
- Can see Earth in the distance
- Not too close to any gravity well
- Good starting point for testing

Alternative Rotation: (0, 0, 0) - Facing away into space
```

### Alternative Spawn Points

**Earth Orbit Spawn:**
```
Location: (1004000000, 0, 0) cm = 4,000 km from Sun, 4 km from Earth
Rotation: (0, 180, 0) - Facing Sun
Purpose: Test approaching Earth from orbit
```

**Far Space Spawn:**
```
Location: (2000000000, 0, 0) cm = 20,000 km from Sun
Rotation: (0, 180, 0) - Facing back at solar system
Purpose: Test viewing entire system from distance
```

---

## Lighting Setup

### Sun Directional Light
```
Actor: DirectionalLight
Location: (0, 0, 100000) - Above Sun, exact position doesn't matter
Rotation: Point toward origin or use auto-calculate
Scale: (1, 1, 1)

Light Settings:
- Intensity: 100,000 lux (very bright, simulates Sun)
- Light Color: (255, 247, 230) - Warm white, ~5778K
- Temperature: 5778
- Indirect Lighting Intensity: 1.0
- Volumetric Scattering Intensity: 1.0

Shadows:
- Cast Shadows: Dynamic
- Shadow Resolution Scale: 2.0 (high quality)
- Shadow Bias: 0.5
- Dynamic Shadow Distance: 100,000,000 (very far)

Atmosphere:
- Affects Atmosphere: Yes
- Cast Shadows on Atmosphere: Yes
```

### Optional: Point Light on Sun
```
If BP_Sun doesn't have built-in PointLight:

Actor: PointLight
Location: (0, 0, 0) - Same as Sun
Intensity: 1,000,000 (very bright)
Attenuation Radius: 100,000,000 cm (1000 km)
Light Color: (255, 240, 220) - Warm white
Source Radius: 696,000,000 cm (matches Sun radius)
Cast Shadows: Yes

Note: This may be redundant if BP_Sun has its own light
```

---

## Sky and Atmosphere

### Skybox - Space Background
```
Actor: BP_SkySphere (or use SkyAtmosphere)
Location: (0, 0, 0)
Scale: (1, 1, 1)

Settings:
- Stars: Yes (high density)
- Sun: No (use actual Sun actor instead)
- Clouds: No (this is space)
- Horizon: Black (no horizon in space)

Material: M_Space_Stars
- Base Color: Black (0, 0, 0)
- Emissive: Star field texture (procedural or image)
- Brightness: 0.5-1.0 (visible but not overpowering)
```

### Sky Atmosphere (Optional - For Earth View)
```
This is typically attached to BP_Earth, not a separate actor

If creating standalone:
Actor: SkyAtmosphere
Location: Follow Earth position
Ground Radius: 6371 km (Earth radius)
Atmosphere Height: 100 km
Multi-Scattering: Yes

Note: Better to add SkyAtmosphereComponent directly to BP_Earth
```

---

## Post Processing

### Post Process Volume - Space Environment
```
Actor: PostProcessVolume
Location: (0, 0, 0)
Bounds: Infinite Extent Unbound
Scale: (1, 1, 1)

Settings:

Exposure:
- Metering Mode: Manual
- Exposure Compensation: 0.0
- Min Brightness: 0.0
- Max Brightness: 100.0 (accommodate bright Sun)

Bloom:
- Intensity: 1.5 (make Sun and stars glow)
- Threshold: 1.0
- Size Scale: 8.0 (large bloom for Sun)

Tone Curve:
- Toe: 0.55
- Shoulder: 0.26
- Black Clip: 0.0
- White Clip: 0.04

Color Grading:
- Global Saturation: 1.0
- Global Contrast: 1.1
- Global Gamma: 1.0
- Shadows Max: 0.05 (very dark space)

Lens:
- Lens Flares: Enabled
- Intensity: 1.0
- Tint: White
- Bokeh Size: 0.0 (no depth of field for VR comfort)

Ambient Occlusion: Disabled (not needed in space)
Motion Blur: Disabled (VR comfort)
Depth of Field: Disabled (VR comfort)
```

---

## Additional Actors

### Navigation Volume (Optional)
```
Not needed for space - no navmesh required
Can add if implementing AI ships later
```

### Audio (Ambient Space Sounds)
```
Actor: AmbientSound
Location: Attached to player or (0, 0, 0)
Sound: Space_Ambience_Cue

Settings:
- Volume: 0.3
- Attenuation: None (always audible)
- Auto Activate: Yes

Sound Cue Contents:
- Low frequency hum
- Occasional creaks (ship hull)
- Radio static
- Distant echoes
```

### Trigger Volumes (For Testing)
```
Actor: TriggerBox
Location: Around Earth at 100 km altitude
Size: Large enough to encompass Earth

Purpose: Trigger landing sequence or atmosphere entry effects

On Actor Begin Overlap:
- If overlapping pawn is player:
  - Log: "Entering Earth atmosphere"
  - Play entry effects
  - Adjust camera settings
```

---

## Level Organization - Outliner Folders

Organize actors into folders for clarity:

```
Level Outliner:
├── CelestialBodies
│   ├── BP_Sun
│   ├── BP_Earth
│   ├── BP_Moon
│   └── BP_Mars (if added)
├── Lighting
│   ├── DirectionalLight_Sun
│   └── ExponentialHeightFog (if used)
├── Environment
│   ├── BP_SkySphere
│   └── PostProcessVolume
├── PlayerSpawns
│   ├── PlayerStart_MoonOrbit
│   ├── PlayerStart_EarthOrbit (if added)
│   └── PlayerStart_FarSpace (if added)
└── Debug
    ├── TriggerBox_Earth
    └── Any debug visualization actors
```

---

## Level Streaming (Optional - Advanced)

For very large solar systems with many bodies:

```
Persistent Level: SolarSystem (main level)
Streaming Sublevels:
- SolarSystem_InnerPlanets (Earth, Moon, Mars)
- SolarSystem_OuterPlanets (Jupiter, Saturn, etc.)
- SolarSystem_AsteroidBelt
- SolarSystem_Lighting

Streaming Method: Distance-based
Load Distance: Based on player proximity to body clusters
```

---

## Camera and Viewport Settings

### Editor Viewport
```
To navigate in editor:
- Camera Speed: Very Fast (adjust in viewport settings)
- Far Clip Plane: 100,000,000 cm (1000 km+)

Useful Editor Commands:
- 'G' key: Toggle game view (hide editor icons)
- 'F' key with actor selected: Focus on actor
- Mouse Wheel: Zoom in/out
```

### Player Camera Settings
```
In BP_VRSpaceshipPawn or CameraComponent:
- Field of View: 90-110 (VR standard)
- Far Clip Plane: 1,000,000,000 cm (10,000 km)
- Near Clip Plane: 1.0 cm (very close for cockpit)
```

---

## Level Blueprint (Optional)

For level-specific logic:

```
Open: Blueprints → Open Level Blueprint

Event BeginPlay:
1. Log: "SolarSystem level loaded"
2. Verify all celestial bodies present:
   - Find Actors of Class: BP_Sun, BP_Earth, BP_Moon
   - Log count
3. Verify Game Mode:
   - Get Game Mode → Cast to BP_SolarSystemGameMode
   - If valid: Log "Game Mode OK"
   - If invalid: Log Error
4. Wait 2 seconds (level fully loaded)
5. Print welcome message:
   - "Welcome to Solar System Test Environment"
   - "Press F3 to toggle debug overlay"
   - "Use VR controllers to interact"

Custom Events:
- TeleportPlayerToEarth
- TeleportPlayerToMoon
- ResetSolarSystem
- ToggleTimeWarp
(Bind these to console commands or debug menu)
```

---

## Level Checklist

Before testing:

### Actors Present:
- [ ] BP_Sun at (0, 0, 0)
- [ ] BP_Earth at (1,000,000,000, 0, 0)
- [ ] BP_Moon at (1,050,000,000, 0, 0)
- [ ] PlayerStart at (1,051,000,000, 0, 0)
- [ ] DirectionalLight for Sun
- [ ] BP_SkySphere or SkyAtmosphere
- [ ] PostProcessVolume configured

### World Settings:
- [ ] Game Mode: BP_SolarSystemGameMode
- [ ] Kill Z: -10,000,000 or lower
- [ ] World Bounds: Large enough

### Lighting:
- [ ] Directional light very bright
- [ ] Sun visible and emissive
- [ ] Stars visible in background
- [ ] Bloom effect on Sun

### Distances:
- [ ] All positions correct (check Transform panel)
- [ ] Player spawn 10 km from Moon
- [ ] Distances match specification

### Performance:
- [ ] All static meshes have LODs
- [ ] Collision simplified or disabled where not needed
- [ ] Niagara effects optimized (if present)

---

## Testing Positions

Useful locations to teleport during testing:

```
1. Moon Close-Up:
   Location: (1050100000, 0, 0) = 1 km from Moon surface
   Purpose: Test landing approach scaling

2. Earth-Moon Midpoint:
   Location: (1025000000, 0, 0) = 250 km from each
   Purpose: Test dual gravity influence

3. Far View:
   Location: (3000000000, 0, 0) = 30,000 km from Sun
   Purpose: Test seeing entire system, extreme scaling

4. Sun Close Approach:
   Location: (100000000, 0, 0) = 1,000 km from Sun
   Purpose: Test extreme brightness, large body scaling

Console Commands:
teleport [X] [Y] [Z]
Example: teleport 1025000000 0 0
```

---

## Troubleshooting

**Issue: Celestial bodies not visible**
- Check Far Clip Plane is set high enough
- Verify body materials are emissive (Sun) or lit properly
- Check scale isn't too small or too large

**Issue: Lighting too dark/bright**
- Adjust DirectionalLight intensity
- Modify PostProcessVolume exposure
- Check Sun material emissive strength

**Issue: Player spawns in wrong location**
- Verify PlayerStart location is exact
- Check Game Mode Default Pawn Class is set
- Ensure no other PlayerStarts are primary

**Issue: Performance problems**
- Reduce Niagara effect complexity
- Simplify body materials
- Lower shadow quality
- Disable atmosphere on distant bodies

**Issue: Bodies appear wrong size**
- Verify Radius property is in kilometers
- Check Construction Script scaling math
- Ensure CelestialBodyComponent is enabled

---

## Save and Version Control

1. **Save Level:**
   - File → Save Current Level
   - Or Ctrl+S

2. **Save All:**
   - File → Save All
   - Saves level and all modified Blueprints

3. **Git Commit (if using version control):**
   ```
   git add Content/SolarSystem.umap
   git commit -m "Setup SolarSystem test level with compressed distances"
   ```

---

## Next Steps After Level Setup

1. **Test in Editor:**
   - Click Play (Alt+P)
   - Verify VR mode activates
   - Check all subsystems initialize

2. **Test in VR:**
   - Click VR Preview
   - Verify HMD tracking
   - Check motion controllers work

3. **Test Gameplay:**
   - Approach Moon - verify scaling
   - Fly toward Earth - verify gravity
   - Activate time warp - verify warnings
   - Check chronometer updates

4. **Performance Check:**
   - Enable stat FPS
   - Check frame rate >90 FPS
   - Profile if needed (Session Frontend)

5. **Iterate:**
   - Adjust distances if needed
   - Fine-tune lighting
   - Add more bodies
   - Optimize performance

---

**Created:** 2025-01-05
**Version:** 1.0
**Status:** Ready for implementation
