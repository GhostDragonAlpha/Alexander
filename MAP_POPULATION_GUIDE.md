# Map Population Guide - FlightTest & SolarSystem Maps

**Purpose**: Populate FlightTest and SolarSystem maps with baseline content for profiling and testing
**Date**: November 8, 2025
**Status**: Ready for Implementation

---

## Table of Contents
1. [Overview](#overview)
2. [Prerequisites](#prerequisites)
3. [FlightTest Map Population](#flighttest-map-population)
4. [SolarSystem Map Population](#solarsystem-map-population)
5. [Testing & Validation](#testing--validation)
6. [Performance Baselines](#performance-baselines)
7. [Troubleshooting](#troubleshooting)

---

## Overview

### Goals
- Create playable test environments for profiling
- Establish performance baselines
- Enable automated testing workflows
- Support VR testing and validation

### Map Purposes
| Map | Purpose | Content Type | Target FPS |
|-----|---------|--------------|------------|
| **FlightTest** | Flight mechanics testing, 6DOF validation | 3-5 spaceships, obstacles, test course | 90+ (VR) |
| **SolarSystem** | Open space testing, orbital mechanics | 5 space stations, planets, open space | 90+ (VR) |

### Time Estimates
- **FlightTest Map**: 45-60 minutes
- **SolarSystem Map**: 60-90 minutes
- **Testing & Validation**: 30 minutes
- **Total**: 2.5 - 3 hours

---

## Prerequisites

### Before You Begin
1. Ensure Unreal Engine 5.6 is installed
2. Verify project compiles successfully
3. Have both maps accessible in Content Browser
4. Run autonomous_station_generator.py (for SolarSystem map)

### Required Assets
```
Engine Assets:
- /Engine/BasicShapes/Cube
- /Engine/BasicShapes/Sphere
- /Engine/BasicShapes/Cylinder
- /Engine/EngineSky/BP_Sky_Sphere
- /Engine/MapTemplates/Sky/SunSky

Project Blueprints:
- /Game/Blueprints/BP_Spaceship (or C++ ASpaceship class)
- /Game/Blueprints/Stations/Generated/* (from autonomous_station_generator.py)
```

### Tools Needed
- Unreal Editor 5.6
- Python environment (for station generation)
- Text editor (for coordinate reference)

---

## FlightTest Map Population

### Map Purpose
Flight mechanics testing with controlled environment for profiling 6DOF flight system.

### Step-by-Step Instructions

#### 1. Open FlightTest Map
1. Launch Unreal Editor
2. File > Open Level > FlightTest
3. Verify map loads successfully

#### 2. Lighting Setup

**A. Directional Light (Sun)**
```
Actor: Directional Light
Location: (0, 0, 5000)
Rotation: (315, 0, 0) - 45-degree angle for realistic lighting
Intensity: 3.0 lux
Color: (255, 247, 230) - Warm sunlight
```

Settings:
```
Light:
- Source Angle: 0.5357 (realistic sun)
- Indirect Lighting Intensity: 1.0
- Volumetric Scattering Intensity: 1.0

Cascaded Shadow Maps:
- Dynamic Shadow Distance: 20000
- Num Dynamic Shadow Cascades: 3
- Cascade Distribution Exponent: 3.0

Atmosphere:
- Atmosphere Sun Light: TRUE
```

**B. Sky Light**
```
Actor: Sky Light
Location: (0, 0, 0)
Intensity: 1.0
Color: (135, 206, 235) - Sky blue
```

Settings:
```
Light:
- Source Type: SLS_CapturedScene
- Cubemap Resolution: 128 (for performance)
- Real Time Capture: TRUE (for VR)
- Lower Hemisphere is Black: FALSE

Distance Field:
- Cast Shadows: TRUE
- Occlusion Max Distance: 1000
```

**C. Sky Sphere (Optional for Visual Quality)**
```
Actor: BP_Sky_Sphere (from Engine content)
Location: (0, 0, 0)
```

Settings:
```
- Directional Light Actor: [Reference your Directional Light]
- Sun Brightness: 50.0
- Stars Brightness: 0.1 (minimal for flight test)
- Cloud Speed: 0.0 (static)
```

#### 3. Post-Processing Volume

**Create Post-Process Volume**
```
Actor: Post Process Volume
Location: (0, 0, 0)
Scale: (1000, 1000, 1000) - Large enough to cover test area
Infinite Extent (Unbound): TRUE
```

**VR-Optimized Settings**
```
Exposure:
- Method: Auto Exposure Histogram
- Min Brightness: 0.5
- Max Brightness: 2.0
- Speed Up: 3.0
- Speed Down: 1.0

Lens:
- Motion Blur Amount: 0.0 (CRITICAL for VR comfort)
- Bloom Intensity: 0.3 (subtle)

Anti-Aliasing:
- Method: TAA (Temporal Anti-Aliasing)
- TAA Samples: 8

VR Comfort:
- Vignette Intensity: 0.0 (avoid in VR)
- Grain Intensity: 0.0
- Chromatic Aberration: 0.0 (causes nausea)
```

#### 4. Place Test Spaceships

**Ship Placement Pattern: Linear Test Course**

**Ship 1 - Player Spawn**
```
Blueprint: BP_Spaceship
Name: PlayerShip
Location: (0, 0, 200)
Rotation: (0, 0, 0)
Tags: [PlayerStart, TestShip]

Settings:
- Auto Possess Player: Player 0
- Flight Mode: Assisted
- Max Thrust: 100000.0
- VR Enabled: TRUE
```

**Ship 2 - Static Target**
```
Blueprint: BP_Spaceship
Name: TargetShip_1
Location: (5000, 0, 200)
Rotation: (0, 90, 0) - Facing player
Tags: [Target, TestShip]

Settings:
- Auto Possess: Disabled
- Simulate Physics: FALSE (static for now)
```

**Ship 3 - Offset Target**
```
Blueprint: BP_Spaceship
Name: TargetShip_2
Location: (5000, 3000, 500)
Rotation: (0, 180, 0)
Tags: [Target, TestShip]
```

**Ship 4 - High Altitude**
```
Blueprint: BP_Spaceship
Name: TargetShip_3
Location: (10000, -2000, 2000)
Rotation: (45, 270, 0)
Tags: [Target, TestShip]
```

**Ship 5 - Docking Practice**
```
Blueprint: BP_Spaceship (optional, if you want 5)
Name: DockingTarget
Location: (2500, 5000, 200)
Rotation: (0, 45, 0)
Tags: [Docking, TestShip]

Settings:
- Flight Mode: Docking
```

#### 5. Camera Setup

**Spectator Camera (for non-VR testing)**
```
Actor: Camera Actor
Name: SpectatorCamera
Location: (0, -1000, 500)
Rotation: (0, 0, 0) - Looking toward player spawn
```

Settings:
```
Camera:
- FOV: 90
- Aspect Ratio: 16:9

Post Process:
- Override Post Process Settings: FALSE (inherit from volume)
```

#### 6. Collision and Navigation

**Flight Boundaries (Invisible Walls)**
```
Actor: Box Trigger
Name: FlightBounds
Location: (5000, 0, 1000)
Scale: (200, 200, 50) - 20km x 20km x 5km flight area

Collision:
- Generate Overlap Events: TRUE
- Collision Preset: OverlapAll
```

**Blueprint Logic**:
```cpp
// In FlightBounds Blueprint, Event Graph
OnComponentBeginOverlap
-> Cast to ASpaceship
-> Branch (Is Valid?)
   TRUE -> Print String "Warning: Leaving test area"
   TRUE -> Play Sound (optional warning beep)
```

**Obstacle Course (Optional - for advanced testing)**
```
Actor 1: Static Mesh (Cube)
Location: (3000, 0, 200)
Scale: (5, 5, 5) - 500cm cube
Mesh: /Engine/BasicShapes/Cube
Material: M_Basic_Floor (or any visible material)

Actor 2: Static Mesh (Cube)
Location: (3000, 1000, 600)
Scale: (3, 3, 3)

Actor 3: Static Mesh (Cube)
Location: (3000, -1000, -200)
Scale: (4, 4, 4)
```

#### 7. FlightTest Map Checklist

**Pre-Save Validation**
```
Lighting:
[ ] Directional Light placed and configured
[ ] Sky Light captures scene properly
[ ] Exposure settings tested (not too bright/dark)
[ ] No harsh shadows causing VR discomfort

Spaceships:
[ ] 3-5 ships placed at specified coordinates
[ ] Player ship set to Auto Possess Player 0
[ ] All ships have collision enabled
[ ] All ships have FlightController component

Environment:
[ ] Post-Process Volume covers entire area
[ ] Motion blur disabled (VR comfort)
[ ] Flight boundaries in place
[ ] Obstacle course visible (if added)

Performance:
[ ] Map runs at 90+ FPS in VR (empty)
[ ] Lighting builds successfully
[ ] No missing asset warnings
[ ] Stat commands work (stat FPS, stat Unit)
```

**Save Map**: File > Save Current Level

---

## SolarSystem Map Population

### Map Purpose
Open space testing with orbital mechanics, space stations, and large-scale navigation.

### Step-by-Step Instructions

#### 1. Open SolarSystem Map
1. Launch Unreal Editor
2. File > Open Level > SolarSystem
3. Verify map loads successfully

#### 2. Generate Space Stations

**Run Python Script (in Unreal Editor Python Console)**
```python
import autonomous_station_generator
autonomous_station_generator.generate_all_stations()
```

This creates 5 stations:
- TradingHub (500m diameter, complex)
- ResearchFacility (250m diameter, medium)
- MilitaryOutpost (300m diameter, medium)
- MiningPlatform (150m diameter, simple)
- TradingOutpost (200m diameter, simple)

#### 3. Position Space Stations

**Station Placement Coordinates**

**Station 1: TradingHub (Central Hub)**
```
Name: TradingHub_Main
Location: (0, 0, 0) - Map center
Rotation: (0, 0, 0)
Scale: (1, 1, 1) - Default size (500m)

Description: Central commerce hub, player spawn point
Lighting: Add 3-4 Point Lights around docking bays
  - Intensity: 5000
  - Color: (255, 255, 200) - Warm station lights
  - Attenuation Radius: 2000
```

**Station 2: ResearchFacility (North)**
```
Name: ResearchFacility_North
Location: (50000, 0, 10000) - 50km north, 10km up
Rotation: (0, 45, 0)
Scale: (1, 1, 1) - Default size (250m)

Description: Scientific outpost
Lighting: 2 Point Lights
  - Intensity: 3000
  - Color: (200, 220, 255) - Cool blue scientific lights
```

**Station 3: MilitaryOutpost (East)**
```
Name: MilitaryOutpost_East
Location: (0, 60000, 5000) - 60km east, 5km up
Rotation: (0, 90, 0)
Scale: (1, 1, 1) - Default size (300m)

Description: Defense platform
Lighting: 3 Point Lights (red warning lights)
  - Intensity: 4000
  - Color: (255, 50, 50) - Red alert lights
```

**Station 4: MiningPlatform (South-West)**
```
Name: MiningPlatform_SW
Location: (-40000, -40000, -8000) - 56km SW, 8km down
Rotation: (0, 225, 0)
Scale: (1, 1, 1) - Default size (150m)

Description: Ore processing facility
Lighting: 1-2 Point Lights (industrial yellow)
  - Intensity: 3500
  - Color: (255, 200, 100)
```

**Station 5: TradingOutpost (Far North-East)**
```
Name: TradingOutpost_NE
Location: (70000, 70000, 15000) - 99km NE, 15km up
Rotation: (0, 135, 0)
Scale: (1, 1, 1) - Default size (200m)

Description: Remote trading post
Lighting: 2 Point Lights
  - Intensity: 3000
  - Color: (255, 220, 180)
```

#### 4. Place Planets (Primitive Spheres for Baseline)

**Planet 1: Terrestrial Planet (Blue-Green)**
```
Actor: Static Mesh (Sphere)
Name: Planet_Terrestrial
Location: (-100000, -100000, -20000) - 141km SW, 20km down
Scale: (200, 200, 200) - 20km diameter
Mesh: /Engine/BasicShapes/Sphere

Material:
- Base Color: (80, 120, 255) - Blue
- Metallic: 0.0
- Roughness: 0.7
- Emissive: (0, 0, 0) - No emission

Collision:
- Collision Preset: BlockAll
- Generate Overlap Events: TRUE
```

**Planet 2: Gas Giant (Orange-Brown)**
```
Actor: Static Mesh (Sphere)
Name: Planet_GasGiant
Location: (150000, 150000, 30000) - 212km NE, 30km up
Scale: (500, 500, 500) - 50km diameter (massive)
Mesh: /Engine/BasicShapes/Sphere

Material:
- Base Color: (200, 120, 60) - Orange-brown
- Metallic: 0.0
- Roughness: 0.5
- Emissive: (10, 5, 2) - Slight glow
```

**Planet 3: Barren Moon (Gray)**
```
Actor: Static Mesh (Sphere)
Name: Moon_Barren
Location: (20000, -30000, 0) - Near TradingHub
Scale: (50, 50, 50) - 5km diameter
Mesh: /Engine/BasicShapes/Sphere

Material:
- Base Color: (100, 100, 100) - Gray
- Metallic: 0.1
- Roughness: 0.9
```

#### 5. Lighting for Space Environment

**A. Directional Light (Distant Star)**
```
Actor: Directional Light
Name: DistantStar_Light
Location: (0, 0, 100000) - High above
Rotation: (280, 0, 0) - Steep angle
Intensity: 2.0 lux (dimmer than FlightTest - space is darker)
Color: (255, 240, 220) - Slightly cooler than normal sun
```

Settings:
```
Light:
- Source Angle: 0.1 (distant star, smaller apparent size)
- Indirect Lighting Intensity: 0.5 (less bounce in space)
- Volumetric Scattering Intensity: 0.3

Shadows:
- Dynamic Shadow Distance: 100000 (large scale)
- Num Dynamic Shadow Cascades: 4
```

**B. Sky Light (Ambient Space Light)**
```
Actor: Sky Light
Name: AmbientSpaceLight
Location: (0, 0, 0)
Intensity: 0.3 (very low - space is dark)
Color: (10, 10, 20) - Deep space blue-black
```

Settings:
```
Light:
- Source Type: SLS_CapturedScene
- Cubemap Resolution: 64 (low res for performance)
- Real Time Capture: FALSE (static for performance)
- Lower Hemisphere is Black: TRUE (space below is black)
```

**C. Station Lights (Point Lights)**
Already placed in step 3 with each station.

#### 6. Skybox/Background Setup

**Starfield Background**
```
Actor: BP_Sky_Sphere (or custom starfield material)
Name: Starfield
Location: (0, 0, 0)
Scale: (1000, 1000, 1000) - Massive skybox

Settings:
- Directional Light Actor: [Reference DistantStar_Light]
- Sun Brightness: 5.0 (dimmer for space)
- Stars Brightness: 5.0 (BRIGHT - stars visible in space)
- Cloud Speed: 0.0
- Cloud Opacity: 0.0 (no clouds in space)
- Horizon Falloff: 0.0 (sharp horizon)
```

**Alternative: Custom Space Skybox Material**
If BP_Sky_Sphere doesn't work well for space:
```
Create Material: M_SpaceSkybox
- Shading Model: Unlit
- Blend Mode: Opaque
- Emissive Color: Texture Sample (starfield texture) * 2.0

Apply to a massive inverted sphere mesh (Scale: -1000, -1000, -1000)
```

#### 7. Test Spawn Points

**Player Spawn (at TradingHub)**
```
Actor: Player Start
Name: PlayerSpawn_TradingHub
Location: (500, 0, 200) - 500m from hub center
Rotation: (0, 180, 0) - Facing hub
```

**Alternate Spawn (at ResearchFacility)**
```
Actor: Player Start
Name: PlayerSpawn_Research
Location: (50500, 0, 10200) - Near ResearchFacility
Rotation: (0, 225, 0)
```

**VR Testing Spawn (open space)**
```
Actor: Player Start
Name: PlayerSpawn_OpenSpace
Location: (5000, 5000, 1000) - Open space between stations
Rotation: (0, 0, 0)
```

#### 8. Place Spaceship for Testing

**Test Ship (at TradingHub)**
```
Blueprint: BP_Spaceship
Name: TestShip_SolarSystem
Location: (1000, 0, 200) - 1km from hub
Rotation: (0, 180, 0)

Settings:
- Auto Possess: Player 0
- Flight Mode: Newtonian (for space)
- Max Thrust: 100000.0
- VR Enabled: TRUE
- Use Origin Centered Physics: TRUE (CRITICAL for large distances)
```

#### 9. Post-Processing (Space Environment)

**Create Post-Process Volume**
```
Actor: Post Process Volume
Location: (0, 0, 0)
Infinite Extent: TRUE

Settings:
Exposure:
- Method: Manual (space doesn't auto-adjust)
- Exposure Compensation: -1.0 (darker for space)

Tone Mapping:
- Slope: 0.9
- Toe: 0.3
- Shoulder: 0.2
- Black Clip: 0.0 (preserve deep blacks of space)
- White Clip: 0.04

Anti-Aliasing:
- Method: TAA
- Samples: 8

VR Comfort:
- Motion Blur: 0.0
- Vignette: 0.0
- Chromatic Aberration: 0.0
- Bloom Intensity: 0.5 (stars should bloom slightly)
```

#### 10. SolarSystem Map Checklist

**Pre-Save Validation**
```
Stations:
[ ] 5 stations generated and placed at coordinates
[ ] Each station has proper lighting
[ ] Station scales are correct (500m, 300m, 250m, 200m, 150m)
[ ] All station components visible

Planets:
[ ] 3 planets/moons placed (1 terrestrial, 1 gas giant, 1 moon)
[ ] Planet scales realistic (5km, 50km, 20km)
[ ] Materials applied and visible
[ ] Collision enabled

Lighting:
[ ] Directional Light (distant star) configured
[ ] Sky Light (ambient space) very dim
[ ] Station lights illuminate docking areas
[ ] Exposure settings prevent over-bright stars

Environment:
[ ] Starfield/skybox visible and realistic
[ ] Post-Process Volume set to space-appropriate settings
[ ] No motion blur (VR comfort)

Gameplay:
[ ] Test spaceship placed and configured
[ ] Player Start actors placed (3 spawn points)
[ ] Origin-centered physics enabled on spaceship
[ ] Large-distance handling verified

Performance:
[ ] Map runs at 90+ FPS with all content
[ ] No missing asset warnings
[ ] Lighting builds in reasonable time (<5 min)
[ ] Stat commands work
```

**Save Map**: File > Save Current Level

---

## Testing & Validation

### Testing Procedure

#### 1. FlightTest Map Testing

**A. Load and Visual Check**
```
1. File > Open Level > FlightTest
2. Press "Play in Editor" (PIE)
3. Verify:
   - Player spawns in Ship 1
   - All 3-5 ships visible
   - Lighting looks correct (not too bright/dark)
   - No missing textures or pink materials
```

**B. Flight Mechanics Test**
```
1. Use WASD + Mouse to control ship
2. Test 6DOF movement:
   - Forward/Backward thrust
   - Strafe left/right
   - Vertical thrust up/down
   - Pitch, yaw, roll rotation
3. Fly toward each target ship
4. Verify collision detection
```

**C. VR Test (if VR headset available)**
```
1. VR Preview (Alt+P or VR Preview button)
2. Check:
   - No motion blur
   - Smooth 90 FPS
   - Head tracking works
   - Controllers tracked
   - No nausea-inducing effects
```

**D. Performance Test**
```
1. Press ` (backtick) to open console
2. Type: stat FPS
3. Verify: 90+ FPS average
4. Type: stat Unit
5. Verify: Frame time < 11.1ms
6. Type: stat ShipCustomization (if component present)
```

#### 2. SolarSystem Map Testing

**A. Load and Visual Check**
```
1. File > Open Level > SolarSystem
2. Press "Play in Editor"
3. Verify:
   - TradingHub visible at center
   - All 5 stations visible in distance
   - Planets visible and scaled correctly
   - Starfield/space background present
   - Station lights visible
```

**B. Navigation Test**
```
1. Spawn at TradingHub
2. Fly toward each station:
   - ResearchFacility (50km north)
   - MilitaryOutpost (60km east)
   - MiningPlatform (56km SW)
   - TradingOutpost (99km NE)
3. Verify:
   - Origin recentering works (no jitter)
   - Distances feel realistic
   - Station details load as you approach
```

**C. Large-Distance Test**
```
1. Fly to gas giant (212km away)
2. Monitor console for warnings
3. Verify:
   - No precision errors logged
   - Ship position stable
   - Planetary gravity affects ship (if implemented)
```

**D. Performance Test**
```
1. Console: stat FPS
2. Verify: 90+ FPS with all stations loaded
3. Console: stat Unit
4. Check frame time < 11.1ms
5. Console: stat SceneRendering
6. Verify draw calls reasonable (<5000)
```

#### 3. Profiling Subsystem Test

**Verify Profiling Infrastructure**
```
1. Open either map
2. Console: stat PerformanceProfiler
3. Verify:
   - Profiling subsystem active
   - Systems being tracked:
     * FlightController
     * OrbitalMechanics
     * PhysicsConsensus
     * Spaceship
     * AutomationEfficiencySystem
   - Frame samples accumulating
```

**Run Automated Profiling Script**
```bash
# From project root
python run_automated_profiling.py

# Expected output:
# - Map loads successfully
# - Profiling runs for 60 seconds
# - JSON report generated
# - TEST_REPORT.json created with FPS data
```

#### 4. Validation Checklist

**FlightTest Map**
```
Visual:
[ ] All ships visible and properly lit
[ ] Lighting realistic (not too bright/dark)
[ ] Obstacles visible (if added)
[ ] Sky/background appropriate

Gameplay:
[ ] Player spawns correctly
[ ] Flight controls responsive
[ ] 6DOF movement works
[ ] Collision detection active

Performance:
[ ] 90+ FPS in editor
[ ] Frame time < 11.1ms
[ ] No frame drops or hitches
[ ] Memory usage reasonable (<4GB)

Profiling:
[ ] Profiling subsystem active
[ ] Systems tracked correctly
[ ] JSON export works
```

**SolarSystem Map**
```
Visual:
[ ] All 5 stations visible
[ ] Planets visible and scaled correctly
[ ] Starfield background present
[ ] Station lights visible from distance

Gameplay:
[ ] Player spawns at TradingHub
[ ] Navigation to all stations possible
[ ] Large distances handled correctly
[ ] No precision jitter

Performance:
[ ] 90+ FPS with all content
[ ] Frame time < 11.1ms
[ ] Draw calls optimized (<5000)
[ ] Memory usage reasonable (<6GB)

Profiling:
[ ] Profiling subsystem active
[ ] Origin-centered physics tracked
[ ] Network replication tracked (if multiplayer)
```

---

## Performance Baselines

### Expected Performance Targets

#### FlightTest Map
```
Hardware: Mid-range VR PC (RTX 3070 / RX 6700 XT)

Minimum Targets:
- Average FPS: 90+
- Frame Time: <11.1ms
- CPU Time: <8ms
- GPU Time: <11ms
- Memory: <4GB
- Draw Calls: <2000

Optimal Targets:
- Average FPS: 120+
- Frame Time: <8.3ms
- CPU Time: <5ms
- GPU Time: <8ms
- Memory: <3GB
- Draw Calls: <1500
```

#### SolarSystem Map
```
Hardware: Mid-range VR PC

Minimum Targets:
- Average FPS: 90+
- Frame Time: <11.1ms
- CPU Time: <8ms
- GPU Time: <11ms
- Memory: <6GB (more stations/planets)
- Draw Calls: <5000

Optimal Targets:
- Average FPS: 120+
- Frame Time: <8.3ms
- CPU Time: <5ms
- GPU Time: <8ms
- Memory: <5GB
- Draw Calls: <3000
```

### System-Specific Targets

**From Performance Budget Document**:
```
Ship Customization: 0.5ms per frame
N-Body Physics (per ship): 0.3ms per frame
Flight Controller: <2ms per frame
Orbital Mechanics: <3ms per frame
Physics Consensus: <5ms per frame
```

**Multi-Instance Scenarios**:
```
FlightTest (5 ships):
- Ship systems: 5 × 0.5ms = 2.5ms
- N-body physics: 5 × 0.3ms = 1.5ms
- Total custom systems: ~4ms
- Remaining for engine: ~7ms

SolarSystem (1 ship + 5 stations):
- Ship systems: 1 × 0.5ms = 0.5ms
- Station systems: minimal (static)
- N-body physics: 1 × 0.3ms = 0.3ms
- Orbital mechanics: <3ms
- Total custom systems: ~4ms
- Remaining for engine: ~7ms
```

### Performance Monitoring Commands

**Console Commands**:
```
stat FPS              - Show FPS counter
stat Unit             - Show frame time breakdown
stat SceneRendering   - Show draw calls and rendering stats
stat Memory           - Show memory usage
stat Engine           - Show engine performance
stat Physics          - Show physics simulation time
stat ShipCustomization - Show custom system performance
stat PerformanceProfiler - Show profiling subsystem data

r.ScreenPercentage 100 - Set render resolution (100 = native)
r.VSync 0             - Disable VSync for testing
t.MaxFPS 120          - Uncap framerate
```

**Unreal Insights Profiling**:
```
1. Console: trace.start
2. Play for 60 seconds
3. Console: trace.stop
4. File saved to: Saved/Profiling/UnrealInsights/
5. Open with UnrealInsights.exe
```

### Automated Profiling

**Run Automated Test**:
```bash
# From project root
python run_automated_profiling.py

# Output files:
# - TEST_REPORT.json (performance data)
# - TEST_REPORT.md (human-readable summary)
# - profiling_output.log (detailed log)
```

**Expected JSON Output**:
```json
{
  "map": "FlightTest",
  "duration_seconds": 60,
  "total_frames": 5400,
  "average_fps": 90.0,
  "min_fps": 85.2,
  "max_fps": 120.5,
  "95th_percentile_fps": 89.8,
  "systems_profiled": {
    "FlightController": {
      "total_time_ms": 108.5,
      "average_time_ms": 1.8,
      "max_time_ms": 2.4
    },
    "Spaceship": {
      "total_time_ms": 162.0,
      "average_time_ms": 2.7,
      "max_time_ms": 3.1
    }
  }
}
```

---

## Troubleshooting

### Common Issues

#### Issue 1: Map Loads but is Completely Black

**Cause**: Missing or misconfigured lighting

**Solution**:
```
1. Verify Directional Light exists and is enabled
2. Check Directional Light intensity (should be 2.0-3.0)
3. Verify Sky Light exists
4. Rebuild lighting: Build > Build Lighting Only
5. Check Post-Process Volume exposure settings
```

#### Issue 2: Ships are Invisible or Missing

**Cause**: Blueprint reference broken or asset not compiled

**Solution**:
```
1. Verify BP_Spaceship exists in /Game/Blueprints/
2. Check if C++ ASpaceship class compiled successfully
3. Re-compile project: Build > Compile
4. Replace ships with fresh blueprint instances
5. Check Content Browser for missing assets
```

#### Issue 3: VR Performance Below 90 FPS

**Cause**: Too many draw calls or unoptimized rendering

**Solution**:
```
1. Console: stat SceneRendering
2. Check draw calls (should be <2000 for FlightTest, <5000 for SolarSystem)
3. Reduce post-processing quality:
   - Lower TAA samples (8 -> 4)
   - Reduce bloom intensity
   - Disable expensive effects
4. Console: r.ScreenPercentage 90 (render at 90% resolution)
5. Build lighting (dynamic lights are expensive)
```

#### Issue 4: Stations Don't Generate (SolarSystem)

**Cause**: Python script failed or asset paths incorrect

**Solution**:
```
1. Open Output Log (Window > Developer Tools > Output Log)
2. Check for Python errors
3. Verify autonomous_station_generator.py exists in Content/Python/
4. Manually run script:
   import autonomous_station_generator
   autonomous_station_generator.list_stations()
   autonomous_station_generator.generate_single_station('TradingHub')
5. Check /Game/Blueprints/Stations/Generated/ for created assets
```

#### Issue 5: Precision Jitter at Large Distances (SolarSystem)

**Cause**: Origin-centered physics not enabled

**Solution**:
```
1. Select spaceship in Outliner
2. Details panel > Origin System section
3. Check "Use Origin Centered Physics" = TRUE
4. Verify OriginManager component exists
5. Console: log LogOriginSystem Verbose
6. Monitor for recentering messages
```

#### Issue 6: Lighting Build Takes Forever

**Cause**: High-quality lightmass settings

**Solution**:
```
1. World Settings > Lightmass Settings
2. Set Static Lighting Level Scale: 1.0 (default)
3. Set Num Indirect Lighting Bounces: 3 (lower for faster)
4. For testing, use dynamic lighting (no build required)
5. Uncheck "Force No Precomputed Lighting" in Post-Process Volume
```

#### Issue 7: Profiling Returns 0 FPS / No Data

**Cause**: No profiled actors in map or profiling subsystem not active

**Solution**:
```
1. Verify spaceship has FlightController component
2. Check Details panel > Profiling > "Enable Profiling" = TRUE
3. Console: stat PerformanceProfiler
4. Verify subsystem shows "Active"
5. Play in Editor for at least 5 seconds before checking
6. Check TEST_REPORT.json for "systems_profiled" entries
```

#### Issue 8: Post-Processing Causes VR Nausea

**Cause**: Motion blur, chromatic aberration, or vignette enabled

**Solution**:
```
Post-Process Volume settings (CRITICAL for VR):
- Motion Blur Amount: 0.0 (DISABLE)
- Chromatic Aberration: 0.0 (DISABLE)
- Vignette Intensity: 0.0 (DISABLE)
- Bloom Intensity: <0.5 (subtle only)
- Depth of Field: 0.0 (DISABLE for VR)

Console commands:
r.MotionBlurQuality 0
r.DepthOfFieldQuality 0
```

#### Issue 9: Collision Not Working

**Cause**: Collision presets incorrect or physics disabled

**Solution**:
```
1. Select spaceship
2. Details > Collision
3. Collision Preset: "PhysicsActor" or "Pawn"
4. Generate Overlap Events: TRUE
5. Simulate Physics: TRUE (for physics-based ships)
6. Check Collision Complexity: "Use Complex Collision as Simple"
```

#### Issue 10: Automated Profiling Script Fails

**Cause**: Python dependencies missing or Unreal Editor path wrong

**Solution**:
```bash
# Verify Python environment
python --version  # Should be 3.9+

# Install dependencies
pip install -r requirements.txt

# Check UnrealEditor path in script
# run_automated_profiling.py line ~20
# Should point to:
# C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe

# Run with verbose logging
python run_automated_profiling.py --verbose

# Check logs
cat profiling_output.log
```

### Debug Commands

**Useful Console Commands for Debugging**:
```
ShowFlag.Collision 1       - Visualize collision meshes
ShowFlag.Bounds 1          - Show actor bounds
ShowFlag.VisualizeHDR 1    - Check exposure issues
ShowFlag.PostProcessing 0  - Disable post-processing
stat Levels                - Show loaded streaming levels
stat Particles             - Show particle performance
obj list class=Spaceship   - List all spaceship instances
getall Spaceship Name      - Get all spaceship names
```

---

## Appendix

### Quick Reference Coordinates

**FlightTest Map**:
```
Player Ship: (0, 0, 200)
Target Ships: (5000, 0, 200), (5000, 3000, 500), (10000, -2000, 2000)
Lights: Sun at (0, 0, 5000), Sky Light at (0, 0, 0)
```

**SolarSystem Map**:
```
TradingHub: (0, 0, 0)
ResearchFacility: (50000, 0, 10000)
MilitaryOutpost: (0, 60000, 5000)
MiningPlatform: (-40000, -40000, -8000)
TradingOutpost: (70000, 70000, 15000)

Planets:
- Terrestrial: (-100000, -100000, -20000)
- Gas Giant: (150000, 150000, 30000)
- Moon: (20000, -30000, 0)
```

### Asset Paths Reference

```
Engine Assets:
/Engine/BasicShapes/Cube
/Engine/BasicShapes/Sphere
/Engine/BasicShapes/Cylinder
/Engine/EngineSky/BP_Sky_Sphere

Project Blueprints:
/Game/Blueprints/BP_Spaceship
/Game/Blueprints/Stations/Generated/BP_TradingHub_Station
/Game/Blueprints/Stations/Generated/BP_ResearchFacility_Station
/Game/Blueprints/Stations/Generated/BP_MilitaryOutpost_Station
/Game/Blueprints/Stations/Generated/BP_MiningPlatform_Station
/Game/Blueprints/Stations/Generated/BP_TradingOutpost_Station
```

### Completion Summary

**Upon completion of this guide, you will have**:
- FlightTest map with 3-5 test spaceships
- SolarSystem map with 5 space stations and 3 planets
- Proper lighting for both maps (VR-optimized)
- Performance baselines established
- Automated profiling ready to run
- Test environment for VR validation

**Total estimated time**: 2.5 - 3 hours
**Critical settings verified**: 12 checkboxes per map = 24 total
**Performance validated**: 90+ FPS target

---

**Last Updated**: November 8, 2025
**Status**: Ready for Implementation
**Author**: AI Agent System
**Related Documents**:
- MAP_CONTENT_STATUS.md
- PERFORMANCE_BUDGET.md
- PHASE9_PERFORMANCE_OPTIMIZATION_PLAN.md
- AUTOMATED_PROFILING_COMPLETE.md
