# BP_VRSpaceshipPlayer - Configuration Guide

## Overview
Update the existing BP_VRSpaceshipPlayer to use the new C++ AVRSpaceshipPawn class with celestial scaling integration.

---

## CRITICAL: Parent Class Change

**Current Parent:** Probably `Pawn` or Blueprint-based parent
**New Parent:** `AVRSpaceshipPawn` (C++ class from Source/Alexander/Public/VRSpaceshipPawn.h)

### How to Change Parent Class:

1. Open BP_VRSpaceshipPlayer in Blueprint Editor
2. Click "Class Settings" button in toolbar
3. In Details panel, find "Parent Class"
4. Click dropdown → Search for "VRSpaceshipPawn"
5. Select `VRSpaceshipPawn` (C++ class)
6. **Important:** Blueprint will warn about reparenting - Click "Reparent"
7. Compile Blueprint
8. Fix any errors (components may need reconnecting)

**Why:** The C++ AVRSpaceshipPawn provides:
- VR origin at (0,0,0) for tracking precision
- Motion controller integration
- PlayerOriginManager auto-creation
- Spaceship possession system
- Network replication

---

## Components (Inherited from C++)

These components are automatically created by the C++ parent class:

### VROrigin (USceneComponent)
```
Created by: C++ constructor
Purpose: Root component, always at (0, 0, 0)
Properties: Cannot move in local space
Children: VRCamera, LeftHand, RightHand, CockpitMesh
```

### VRCamera (UCameraComponent)
```
Created by: C++ constructor
Parent: VROrigin
Purpose: VR headset tracking
Properties:
- bUsePawnControlRotation: false
- bLockToHmd: true
```

### LeftHand (UMotionControllerComponent)
```
Created by: C++ constructor
Parent: VROrigin
Purpose: Left VR controller
Properties:
- MotionSource: Left
- Hand: EControllerHand::Left
- bDisplayDeviceModel: true (if supported in UE 5.6)
```

### RightHand (UMotionControllerComponent)
```
Created by: C++ constructor
Parent: VROrigin
Purpose: Right VR controller
Properties:
- MotionSource: Right
- Hand: EControllerHand::Right
- bDisplayDeviceModel: true (if supported in UE 5.6)
```

### CockpitMesh (UStaticMeshComponent)
```
Created by: C++ constructor
Parent: VROrigin
Purpose: Visual cockpit around player
Properties:
- Static Mesh: Set in Blueprint (SM_Cockpit or similar)
- Collision: NoCollision
- Cast Shadow: Yes
```

### OriginManager (UPlayerOriginManager)
```
Created by: C++ constructor
Parent: VROrigin
Purpose: Manages player virtual position and universe recentering
Properties: Configured in BeginPlay
```

### TimeComponent (UPlayerTimeComponent)
```
Created by: C++ constructor
Parent: VROrigin
Purpose: Tracks biological age and time debt
Properties: Auto-registers with RelativitySystem
```

---

## Blueprint Configuration

### Class Defaults

#### VR Settings
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR")
bool bEnableVR = true; // Master VR toggle

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VR")
float HandTrackingPrecision = 0.001; // 1mm precision target
```

#### Spaceship Settings
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship")
bool bAutoPossessSpaceship = true; // Auto-find and possess spaceship on spawn

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship")
TSubclassOf<ASpaceship> SpaceshipClass; // Set to BP_Spaceship or similar

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship")
float SpaceshipSearchRadius = 100000.0; // 1 km search radius (cm)
```

#### Origin Management
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Origin")
float PrecisionThreshold = 1000000.0; // 10 km in cm

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Origin")
bool bAutoRecenterUniverse = true;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Origin")
bool bShowOriginDebugInfo = false; // Debug visualization
```

---

## Blueprint Graphs to Add/Modify

### Event BeginPlay (Override)
```
Purpose: Initialize VR pawn and possess spaceship

Blueprint Graph:
1. Call Parent BeginPlay (C++ initialization)

2. Verify VR is Available:
   - Node: Is VR Enabled (from Head Mounted Display library)
   - Branch: If not enabled
     - Log Warning: "VR not available, some features may not work"
   - If enabled:
     - Log: "VR initialized successfully"

3. Configure OriginManager:
   - Get OriginManager component reference
   - If valid:
     - Call: SetPrecisionThreshold(PrecisionThreshold)
     - Call: SetAutoRecenterEnabled(bAutoRecenterUniverse)
     - Call: SetShowDebugInfo(bShowOriginDebugInfo)
     - Log: "OriginManager configured"

4. Configure TimeComponent:
   - Get TimeComponent reference
   - If valid:
     - Already registered by C++, just verify
     - Log: "TimeComponent active"

5. Setup Cockpit Mesh (if not set in C++):
   - Get CockpitMesh component
   - If Static Mesh not set:
     - Set Static Mesh: SM_SpaceshipCockpit (or similar)
     - Set Material: M_Cockpit
     - Set Relative Location/Rotation to position around player

6. Find and Possess Spaceship (if bAutoPossessSpaceship):
   - Delay: 0.1 seconds (let level load)
   - Find Actors of Class: ASpaceship
   - Filter by distance (within SpaceshipSearchRadius)
   - If found:
     - Cast to AVRSpaceshipPawn (self)
     - Call: PossessSpaceship(FoundSpaceship)
     - Log: "Spaceship possessed: [Spaceship Name]"
   - If not found and bAutoPossessSpaceship:
     - Spawn Actor: SpaceshipClass at player location
     - Call: PossessSpaceship(SpawnedSpaceship)
     - Log: "Spaceship spawned and possessed"

7. Setup UI Widgets:
   - Create Widget: WBP_Chronometer
   - Add to Viewport
   - Store reference in variable

8. Log: "VR Spaceship Player initialized successfully"
```

### Function: PossessSpaceship (Override from C++)
```
Parameters:
- Spaceship (ASpaceship): The spaceship to possess

Purpose: Link VR pawn to spaceship physics

Blueprint Graph (extends C++ implementation):
1. Call Parent PossessSpaceship (handles C++ linkage)

2. Verify possession successful:
   - Get PossessedSpaceship variable from C++
   - If not null:
     - Get Spaceship's OriginManager
     - Link to VR Pawn's OriginManager (same virtual position)
     - Log: "Spaceship possession successful"
   - Else:
     - Log Error: "Failed to possess spaceship"
```

### Function: UpdateVirtualPosition (Already in C++)
```
Note: Implemented in C++, but you can extend in Blueprint

Purpose: Called every frame to update position based on spaceship movement

Can add Blueprint events:
- OnVirtualPositionChanged (delegate)
- OnUniverseRecentered (delegate)
```

### Event: On Motion Controller Grab (Custom)
```
Purpose: Handle VR controller interactions

Blueprint Graph:
1. On Left Controller Trigger Pressed:
   - Perform line trace from LeftHand
   - If hit cockpit control:
     - Activate control (throttle, flight stick, etc.)

2. On Right Controller Trigger Pressed:
   - Perform line trace from RightHand
   - If hit cockpit control:
     - Activate control
```

---

## Input Bindings

### VR Input Actions

Add these in Project Settings → Input:

```
Action Mappings:
- VR_GrabLeft: Motion Controller (L) Trigger
- VR_GrabRight: Motion Controller (R) Trigger
- VR_MenuLeft: Motion Controller (L) Face Button 1
- VR_MenuRight: Motion Controller (R) Face Button 1

Axis Mappings:
- VR_ThumbstickLeft_X: Motion Controller (L) Thumbstick X-Axis
- VR_ThumbstickLeft_Y: Motion Controller (L) Thumbstick Y-Axis
- VR_ThumbstickRight_X: Motion Controller (R) Thumbstick X-Axis
- VR_ThumbstickRight_Y: Motion Controller (R) Thumbstick Y-Axis
```

### Bind in Blueprint

In Event Graph:
```
Event: VR_GrabLeft
→ Handle left controller grab

Event: VR_GrabRight
→ Handle right controller grab

Event: VR_MenuLeft
→ Toggle menu or HUD

Axis Event: VR_ThumbstickLeft_Y
→ Control throttle or vertical movement

Axis Event: VR_ThumbstickRight_X
→ Control yaw rotation
```

---

## Cockpit Setup

### Option 1: Simple Cockpit (Placeholder)
```
CockpitMesh:
- Static Mesh: SM_Cube (scaled to cockpit shape)
- Material: M_Glass (semi-transparent)
- Scale: (2.0, 2.0, 1.5) - Large enough to see through
- Location: (50, 0, 0) - In front of player head
```

### Option 2: Detailed Cockpit (Recommended)
```
CockpitMesh:
- Static Mesh: SM_SpaceshipCockpit (custom mesh)
- Material: M_CockpitGlass (glass) + M_CockpitMetal (frame)
- Include:
  - Flight stick model
  - Throttle model
  - Button panels
  - Holographic displays
  - Instrument cluster

Add child components:
- FlightStick (StaticMeshComponent)
- Throttle (StaticMeshComponent)
- ButtonPanel_Left (StaticMeshComponent)
- ButtonPanel_Right (StaticMeshComponent)
- HolographicDisplay (WidgetComponent for 3D UI)
```

---

## Widget Component Setup (3D UI in Cockpit)

### Add HolographicDisplay (UWidgetComponent)
```
Component: HolographicDisplay
Parent: CockpitMesh
Type: WidgetComponent

Properties:
- Widget Class: WBP_CockpitHUD
- Draw Size: (1920, 1080)
- Pivot: (0.5, 0.5)
- Location: (60, 0, 20) - Above instrument panel
- Rotation: (0, 180, 0) - Facing player
- Blend Mode: Transparent
- Background Color: None
- Space: World (renders in 3D space)

Content (WBP_CockpitHUD):
- Velocity display
- Altitude display
- Target info
- Chronometer (small version)
- Warning lights
```

---

## Network Replication

### Replication Settings
```
Class Settings:
- Replicates: True
- Replicate Movement: False (movement via OriginManager)
```

### Replicated Variables
```cpp
UPROPERTY(Replicated)
ASpaceship* PossessedSpaceship; // Already handled in C++

UPROPERTY(Replicated)
FVector VirtualPosition; // Already handled in C++ OriginManager
```

### Client Prediction
```
The C++ AVRSpaceshipPawn already handles:
- VR tracking (local only, not replicated - too high frequency)
- Input prediction
- Server reconciliation via OriginManager
```

---

## Testing Checklist

After configuring BP_VRSpaceshipPlayer:

### VR Initialization
- [ ] Pawn spawns in VR mode
- [ ] HMD tracking works
- [ ] Head rotation controls camera
- [ ] No jitter or drift

### Motion Controllers
- [ ] Left controller visible
- [ ] Right controller visible
- [ ] Controller tracking smooth
- [ ] Hand positions accurate

### Origin Management
- [ ] OriginManager component present
- [ ] Virtual position tracked
- [ ] Universe recenters at threshold
- [ ] No visible teleporting

### Spaceship Possession
- [ ] Spaceship found/spawned
- [ ] Possession successful
- [ ] VR pawn position linked to spaceship
- [ ] Movement synchronized

### UI Widgets
- [ ] Chronometer displays
- [ ] Shows all three time clocks
- [ ] Updates correctly
- [ ] HUD readable in VR

### Cockpit
- [ ] Cockpit mesh visible
- [ ] Positioned correctly around player
- [ ] Materials render properly
- [ ] Interactive elements accessible

### Input
- [ ] Controller inputs recognized
- [ ] Grab actions work
- [ ] Thumbstick controls spaceship
- [ ] Menu button toggles UI

---

## Common Issues & Solutions

**Issue: Parent class change breaks Blueprint**
- Solution: Before changing, document all components and connections
- After reparenting, manually reconnect any broken links
- Recompile multiple times if needed

**Issue: VR not working**
- Solution: Verify VR plugin enabled (Edit → Plugins → Virtual Reality)
- Check SteamVR or Oculus Runtime is running
- Ensure HMD is connected and recognized

**Issue: Controllers not visible**
- Solution: Check MotionControllerComponent settings
- Verify MotionSource is set (Left/Right)
- Some UE versions need manual mesh assignment

**Issue: Spaceship not found/possessed**
- Solution: Verify spaceship exists in level
- Check SpaceshipClass is set correctly
- Increase SpaceshipSearchRadius if needed
- Check BeginPlay delay is sufficient

**Issue: Universe recentering causes VR jitter**
- Solution: Verify VROrigin stays at (0,0,0)
- Check recentering only moves world actors, not VR components
- Ensure PlayerOriginManager doesn't modify VR transform

**Issue: Cockpit too close/far**
- Solution: Adjust CockpitMesh relative location
- Typical values: X: 40-80 cm in front, Y: 0, Z: 0-20 cm up
- Test in VR, adjust until comfortable

---

## Performance Optimization

### VR-Specific Settings
```
Project Settings → Engine → Rendering:
- Forward Shading: Enabled (better VR performance)
- Multi-View: Enabled (if supported)
- Instanced Stereo: Enabled

VR Settings:
- Pixel Density: 1.0-1.4 (balance quality/performance)
- Anti-Aliasing: MSAA 4x or TAA
```

### Component Tick Settings
```
OriginManager:
- Tick Interval: 0.0 (every frame for precision)

TimeComponent:
- Tick Interval: 0.5 (low frequency updates fine)

CockpitMesh:
- Tick: Disabled (static)
```

---

## Advanced: Hand Interaction System

For full VR immersion, add hand interactions:

```cpp
// In Blueprint
Function: TraceFromHand

Parameters:
- Hand (UMotionControllerComponent): Left or Right
- TraceDistance (Float): 100 cm default

Returns:
- HitActor (AActor): What was hit
- HitLocation (FVector): Where it was hit

Implementation:
1. Get Hand world location and rotation
2. Calculate end point: Start + (Forward × TraceDistance)
3. Line Trace by Channel: ECC_Visibility
4. If hit:
   - Check if actor has interaction interface
   - Call interaction function
   - Provide haptic feedback
5. Return hit results
```

---

## Recommended Default Values

```
bEnableVR: true
bAutoPossessSpaceship: true
SpaceshipClass: BP_Spaceship (or your spaceship Blueprint)
SpaceshipSearchRadius: 100000.0 (1 km)
HandTrackingPrecision: 0.001 (1mm)
PrecisionThreshold: 1000000.0 (10 km)
bAutoRecenterUniverse: true
bShowOriginDebugInfo: false (true for testing)
```

---

**Created:** 2025-01-05
**Version:** 1.0
**Next:** Create UI widget specifications
