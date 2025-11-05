# Unreal Editor Setup Guide for First Mission

This guide provides step-by-step instructions for completing the First Mission implementation in Unreal Editor.

## ⚠️ CRITICAL FIRST STEP: Project Compilation

Before doing anything else, you MUST compile the project to make the new C++ classes available.

### Step 1: Open Project in Unreal Editor

1. Navigate to: `C:\Users\allen\Desktop\Alexander\Alexander`
2. Double-click `Alexander.uproject`
3. Unreal Editor will detect new C++ code and prompt to compile
4. Click **Yes** to compile the project
5. Wait for compilation to complete (may take 5-10 minutes)
6. If compilation errors occur, check the Output Log window

### Step 2: Verify Classes Are Available

1. Open Content Browser
2. Enable "Show C++ Classes" in the View Options
3. Navigate to C++ Classes > Alexander
4. Verify these classes exist:
   - `FirstMissionGameMode`
   - `ShipEntryComponent`
   - `SurfacePlayerCharacter`
   - `LandingGuidanceSystem`

---

## 📋 Implementation Checklist

Use this checklist to track your progress:

- [ ] **Phase 1: Project Setup**
  - [ ] Project compiled successfully
  - [ ] All C++ classes visible in Content Browser
  - [ ] No compilation errors in Output Log

- [ ] **Phase 2: Game Mode Setup**
  - [ ] Set FirstMissionGameMode as default in Project Settings
  - [ ] Configure MoonDistance (50000.0 for gameplay)
  - [ ] Set bUseRealisticDistances to false

- [ ] **Phase 3: Enhanced Input Setup**
  - [ ] Create GroundControls Input Mapping Context
  - [ ] Create ShipControls Input Mapping Context
  - [ ] Create VRControls Input Mapping Context (if using VR)
  - [ ] Assign to SurfacePlayerCharacter and Spaceship

- [ ] **Phase 4: Character Setup**
  - [ ] Create SurfacePlayerCharacter Blueprint (BP_SurfacePlayer)
  - [ ] Set up camera components
  - [ ] Configure movement speeds
  - [ ] Test walking/running/jumping

- [ ] **Phase 5: Spaceship Setup**
  - [ ] Find or create Spaceship Blueprint
  - [ ] Add ShipEntryComponent
  - [ ] Configure entry points (cockpit, emergency exit)
  - [ ] Test ship controls

- [ ] **Phase 6: Level Creation**
  - [ ] Create new level: MissionLevel
  - [ ] Add Planet actor at origin
  - [ ] Add Moon actor 50km away
  - [ ] Place Spaceship near spawn point
  - [ ] Add player start location
  - [ ] Configure lighting (sun)

- [ ] **Phase 7: Landing Guidance**
  - [ ] Place LandingGuidanceSystem actors in level
  - [ ] Create Niagara systems for landing beam
  - [ ] Create Niagara systems for landing zone indicator
  - [ ] Test guidance activation near surfaces

- [ ] **Phase 8: UMG Widgets**
  - [ ] Create WBP_MissionHUD widget
  - [ ] Create WBP_MainMenu widget
  - [ ] Bind to C++ events
  - [ ] Test UI updates

- [ ] **Phase 9: Materials & VFX**
  - [ ] Create ship materials
  - [ ] Create planet/moon materials
  - [ ] Create landing zone marker materials
  - [ ] Add particle effects

- [ ] **Phase 10: Audio**
  - [ ] Import audio files
  - [ ] Create sound cues
  - [ ] Hook up to AudioManager
  - [ ] Test audio triggers

- [ ] **Phase 11: Testing**
  - [ ] Test complete mission flow
  - [ ] Verify all 17 stages trigger
  - [ ] Check XP awards
  - [ ] Test VR mode (if applicable)
  - [ ] Performance profiling

---

## 🎮 Phase 3: Enhanced Input Setup (DETAILED)

### Creating Input Mapping Contexts

#### 1. Ground Controls (for SurfacePlayerCharacter)

**File Location:** `Content/Input/IMC_GroundControls`

**Create the Input Mapping Context:**
1. Content Browser → Right-click → Input → Input Mapping Context
2. Name it `IMC_GroundControls`
3. Open the asset

**Add Input Actions:**

Create these Input Actions first (Content/Input/Actions/):
- `IA_Move` (Input Action) - Value Type: Axis2D
- `IA_Look` (Input Action) - Value Type: Axis2D  
- `IA_Jump` (Input Action) - Value Type: Digital (bool)
- `IA_Sprint` (Input Action) - Value Type: Digital (bool)
- `IA_Crouch` (Input Action) - Value Type: Digital (bool)
- `IA_Interact` (Input Action) - Value Type: Digital (bool)

**Map Keys in IMC_GroundControls:**
```
IA_Move:
  - W, S: Forward/Backward (Axis2D Y)
  - A, D: Left/Right (Axis2D X)
  - Left Stick (Gamepad): Both axes

IA_Look:
  - Mouse X, Y: Look around
  - Right Stick (Gamepad): Both axes

IA_Jump:
  - Space: Jump
  - A Button (Gamepad): Jump

IA_Sprint:
  - Left Shift: Sprint
  - Left Stick Click (Gamepad): Sprint

IA_Crouch:
  - C: Crouch
  - B Button (Gamepad): Crouch

IA_Interact:
  - E: Interact
  - X Button (Gamepad): Interact
```

#### 2. Ship Controls (for Spaceship)

**File Location:** `Content/Input/IMC_ShipControls`

**Create Input Actions:**
- `IA_Throttle` (Value Type: Axis1D)
- `IA_Pitch` (Value Type: Axis1D)
- `IA_Yaw` (Value Type: Axis1D)
- `IA_Roll` (Value Type: Axis1D)
- `IA_LandingGear` (Value Type: Digital)
- `IA_ExitShip` (Value Type: Digital)

**Map Keys:**
```
IA_Throttle:
  - W, S: Throttle up/down
  - Right Trigger/Left Trigger (Gamepad)

IA_Pitch:
  - Mouse Y: Pitch
  - Up/Down Arrows
  - Right Stick Y (Gamepad)

IA_Yaw:
  - A, D: Yaw left/right
  - Left/Right Arrows
  - Right Stick X (Gamepad)

IA_Roll:
  - Q, E: Roll left/right
  - Left Bumper/Right Bumper (Gamepad)

IA_LandingGear:
  - G: Toggle landing gear
  - D-Pad Down (Gamepad)

IA_ExitShip:
  - F: Exit ship
  - Y Button (Gamepad)
```

#### 3. Assign Input Contexts to Characters

**In BP_SurfacePlayer (Blueprint):**
1. Open the Blueprint
2. In the Event Graph, find the existing Event BeginPlay
3. The C++ code already handles adding the mapping context
4. Just set the `DefaultMappingContext` variable to `IMC_GroundControls`
5. Set each Input Action variable:
   - `MoveAction` → `IA_Move`
   - `LookAction` → `IA_Look`
   - `JumpAction` → `IA_Jump`
   - etc.

**In BP_Spaceship (Blueprint):**
1. Similar process
2. Set `DefaultMappingContext` to `IMC_ShipControls`
3. Assign appropriate Input Actions

---

## 🎨 Phase 8: Creating UMG Widgets (DETAILED)

### A. Mission HUD Widget (WBP_MissionHUD)

**Create the Widget:**
1. Content Browser → Right-click → User Interface → Widget Blueprint
2. Name: `WBP_MissionHUD`
3. Open the widget

**Widget Hierarchy:**
```
Canvas Panel
├─ VerticalBox (Top Left) - Mission Info
│  ├─ TextBlock - MissionStage
│  ├─ TextBlock - CurrentObjective
│  └─ VerticalBox - ObjectivesList
│     ├─ TextBlock - Objective1
│     ├─ TextBlock - Objective2
│     └─ ... (7 total)
│
├─ HorizontalBox (Top Right) - Stats
│  ├─ TextBlock - XPDisplay
│  └─ ProgressBar - StaminaBar
│
├─ VerticalBox (Bottom Center) - Landing Guidance
│  ├─ TextBlock - AltitudeDisplay
│  ├─ TextBlock - VerticalSpeedDisplay
│  ├─ TextBlock - HorizontalSpeedDisplay
│  ├─ TextBlock - LandingStatusText
│  └─ Image - StatusColorIndicator
│
└─ Border (Bottom Right) - Tutorial Hints
   └─ TextBlock - TutorialText
```

**Blueprint Graph Setup:**

1. **Event Construct:**
```
Event Construct
  → Get Game Mode
    → Cast to FirstMissionGameMode
      → Bind Event to OnMissionStageChanged
      → Bind Event to OnObjectiveCompleted
  → Get All Actors of Class (LandingGuidanceSystem)
    → Get first element
      → Bind Event to OnLandingStatusChanged
```

2. **Update Mission Stage Function:**
```
Function: UpdateMissionStage (EMissionStage NewStage)
  → Switch on NewStage
    → Set MissionStage Text based on stage
    → Show/Hide relevant UI elements
```

3. **Update Landing Guidance Function:**
```
Function: UpdateLandingGuidance
  → Get LandingGuidanceSystem reference
    → Get Guidance Data
      → Set Altitude Text
      → Set Vertical Speed Text
      → Set Status Text
      → Set Status Color (Green/Yellow/Red)
```

4. **Tick Event:**
```
Event Tick
  → If Player in Ship:
      → Update Landing Guidance
      → Hide Stamina Bar
  → Else (Player on foot):
      → Update Stamina Bar
      → Hide Landing Guidance
```

### B. Main Menu Widget (WBP_MainMenu)

**Create the Widget:**
1. Content Browser → Right-click → User Interface → Widget Blueprint
2. Name: `WBP_MainMenu`

**Widget Hierarchy:**
```
Canvas Panel
├─ Image - Background
├─ VerticalBox (Center)
│  ├─ TextBlock - Title: "First Mission: Moon Walk"
│  ├─ Spacer
│  ├─ TextBlock - BriefingText (multi-line)
│  ├─ Spacer
│  ├─ Button - StartMissionButton
│  │  └─ TextBlock - "Start Mission"
│  ├─ Button - TutorialButton
│  │  └─ TextBlock - "Tutorial"
│  ├─ Button - SettingsButton
│  │  └─ TextBlock - "Settings"
│  └─ Button - QuitButton
│     └─ TextBlock - "Quit"
└─ Border (Right Side) - Objectives Preview
   └─ VerticalBox
      ├─ TextBlock - "Mission Objectives:"
      └─ ... (List of 7 objectives)
```

**Button Events:**

1. **StartMissionButton OnClicked:**
```
OnClicked (StartMissionButton)
  → Get Game Mode
    → Cast to FirstMissionGameMode
      → Call StartMission()
  → Remove from Parent (hide menu)
  → Create Widget (WBP_MissionHUD)
    → Add to Viewport
```

2. **TutorialButton OnClicked:**
```
OnClicked (TutorialButton)
  → Create Widget (WBP_TutorialOverlay)
    → Add to Viewport
```

3. **SettingsButton OnClicked:**
```
OnClicked (SettingsButton)
  → Create Widget (WBP_SettingsMenu)
    → Add to Viewport
```

---

## 🗺️ Phase 6: Level Creation (DETAILED)

### Step-by-Step Level Setup

#### 1. Create the Level

1. File → New Level
2. Select "Empty Level"
3. Save As: `Content/Levels/MissionLevel`

#### 2. Add Basic Lighting

1. Place Actors → Lights → Directional Light (Sun)
2. Position at (0, 0, 10000)
3. Rotation: (-45, 0, 0) for realistic sun angle
4. Intensity: 10.0
5. Color: Slight yellow tint (255, 250, 240)

#### 3. Add Skybox

1. Place Actors → Visual Effects → Sky Atmosphere
2. Keep default settings for realistic atmosphere
3. Optional: Add BP_Sky_Sphere for static background stars

#### 4. Place Planet

1. Drag `APlanet` (or BP_Planet) into level
2. Position: (0, 0, 0) - World origin
3. Scale: (100, 100, 100) or desired size
4. Configure Planet Settings:
   - Radius: 6371000 (Earth-sized) or smaller for gameplay
   - Add biomes in Planet details panel
   - Enable collision
   - Set up terrain materials

#### 5. Create Landing Zone on Planet

1. Place Actors → Basic → Plane
2. Position near planet surface
3. Scale to create flat landing area (20, 20, 1)
4. Apply landing zone material (emissive lines)
5. Ensure collision is enabled

#### 6. Place Moon

1. Drag `APlanet` (or BP_Moon) into level
2. Position: (5000000, 0, 0) - 50km away (50000 meters = 5000000 cm)
3. Scale: (27, 27, 27) - About 1/4 planet size
4. Configure Moon Settings:
   - Gray/rocky materials
   - Lower gravity (in World Settings: 162 cm/s²)
   - Collision enabled

#### 7. Create Landing Zone on Moon

1. Similar to planet landing zone
2. Place on moon surface
3. Mark as safe landing area

#### 8. Place Spaceship

1. Drag BP_Spaceship into level
2. Position near player spawn on planet (500, 0, 100)
3. Rotation: (0, 0, 0) - Facing forward
4. **CRITICAL:** Add ShipEntryComponent:
   - Select spaceship
   - Details panel → Add Component → ShipEntryComponent
   - Configure entry points in component details

#### 9. Configure Ship Entry Points

In ShipEntryComponent details:
```
Entry Points (Add New):
1. Cockpit_Main:
   - Type: Cockpit
   - Local Location: (200, 0, 50) - Front of ship
   - Interaction Radius: 200
   - Locked: false

2. Emergency_Exit:
   - Type: Emergency
   - Local Location: (-100, 100, 50) - Side of ship
   - Interaction Radius: 200
   - Locked: false
```

#### 10. Place Player Start

1. Place Actors → Basic → Player Start
2. Position: (0, 0, 100) - Near ship and landing zone
3. Rotation: Facing towards ship

#### 11. Place Landing Guidance Systems

1. Drag ALandingGuidanceSystem into level (2 instances)
2. **Moon Guidance:**
   - Position near moon landing zone
   - Name: "LandingGuidance_Moon"
   - Details → Target Planet: Select Moon actor
3. **Planet Guidance:**
   - Position near planet landing zone
   - Name: "LandingGuidance_Planet"
   - Details → Target Planet: Select Planet actor

#### 12. World Settings Configuration

1. Window → World Settings
2. Game Mode Override: Select FirstMissionGameMode
3. Default Pawn Class: SurfacePlayerCharacter
4. Gravity Z: -980 (Earth-like on planet, will be overridden on moon)

#### 13. Post Process Volume

1. Place Actors → Volumes → Post Process Volume
2. Scale to cover entire playable area
3. Set "Infinite Extent (Unbound)" to true
4. Configure:
   - Bloom: Enabled
   - Auto Exposure: Adjust for space visibility
   - Color Grading: Slight desaturation for space feel

#### 14. Save and Test

1. Ctrl+S to save level
2. PIE (Play In Editor) → Alt+P
3. Verify player spawns correctly
4. Check if ship is visible and accessible

---

## 🔧 Common Issues and Solutions

### Issue: Classes not showing in Content Browser

**Solution:**
1. Close Unreal Editor
2. Delete `Intermediate` and `Binaries` folders
3. Right-click `Alexander.uproject` → Generate Visual Studio project files
4. Open `Alexander.sln` in Visual Studio
5. Build → Build Solution (Ctrl+Shift+B)
6. Open project in Unreal Editor again

### Issue: Can't possess Spaceship after entering

**Solution:**
1. Check ShipEntryComponent is added to Spaceship
2. Verify OwningShip reference is set in BeginPlay
3. Check PlayerController is valid
4. Enable logging: `LogTemp` category set to Verbose in Output Log

### Issue: Landing Guidance not appearing

**Solution:**
1. Verify LandingGuidanceSystem actors are in level
2. Check MaxGuidanceAltitude (default 5000m)
3. Ensure TargetShip and TargetPlanet references are set
4. Enable bShowVisualGuidance in guidance system details
5. Check Niagara components are present and active

### Issue: Input not working

**Solution:**
1. Verify Input Mapping Contexts are created
2. Check Input Actions are assigned to character variables
3. Ensure Enhanced Input plugin is enabled
4. Project Settings → Input → Default Input Component Class: Enhanced Input Component

### Issue: Character falls through planet

**Solution:**
1. Select Planet actor
2. Details → Collision → Collision Presets: BlockAll
3. Ensure planet has collision mesh
4. Check collision complexity setting
5. May need to add collision shapes manually

---

## 📊 Testing Protocol

### Basic Functionality Tests

**Test 1: Character Movement**
- [ ] WASD movement works
- [ ] Mouse look works
- [ ] Jump works (Space)
- [ ] Sprint works (Shift) and drains stamina
- [ ] Stamina regenerates when not sprinting
- [ ] Crouch works (C)

**Test 2: Ship Entry**
- [ ] "Press E to Enter Ship" prompt appears near ship
- [ ] E key enters ship
- [ ] Camera smoothly transitions to cockpit (2 seconds)
- [ ] Player can control ship after entry
- [ ] Cannot enter ship while it's moving

**Test 3: Ship Flight**
- [ ] W/S controls throttle
- [ ] Mouse controls pitch/yaw
- [ ] Q/E controls roll
- [ ] Ship physics feels responsive
- [ ] Can navigate toward moon

**Test 4: Landing Guidance**
- [ ] Guidance activates within 5000m of surface
- [ ] Altitude display shows correct value
- [ ] Vertical speed shows descent rate
- [ ] Status changes from ClearToLand to SpeedTooHigh when descending fast
- [ ] Visual beam appears from ship to ground
- [ ] Landing zone indicator appears on surface
- [ ] Colors change based on status (green/yellow/red)

**Test 5: Landing**
- [ ] Ship can land when guidance shows ClearToLand
- [ ] Cannot exit ship while moving
- [ ] F key exits ship when landed
- [ ] Character spawns outside ship correctly

**Test 6: Mission Flow**
- [ ] Main menu appears on start
- [ ] Start Mission button begins mission
- [ ] Mission HUD appears
- [ ] Objectives update as completed
- [ ] XP awards correctly
- [ ] Stage text updates (17 stages)
- [ ] Mission Complete triggers after return to planet

**Test 7: Performance**
- [ ] Maintains 60+ FPS during flight
- [ ] No stuttering during transitions
- [ ] Memory usage stable
- [ ] No significant frame drops

---

## 🚀 Quick Start Commands

Once compiled, test these console commands in PIE:

```
# Skip to specific mission stage (for testing)
FirstMissionGameMode.DebugSetStage 5

# Complete current objective
FirstMissionGameMode.DebugCompleteObjective Objective_ReachMoon

# Teleport to moon instantly
FirstMissionGameMode.DebugTeleportToMoon

# Toggle landing guidance visibility
FirstMissionGameMode.DebugToggleGuidance

# Show mission state
FirstMissionGameMode.DumpMissionState
```

---

## 📝 Final Checklist Before Distribution

- [ ] All systems tested and working
- [ ] No compiler warnings
- [ ] Package build succeeds
- [ ] VR mode tested (if applicable)
- [ ] Performance profiled (GPU/CPU)
- [ ] Memory leaks checked
- [ ] Save/Load functionality works
- [ ] All audio plays correctly
- [ ] All materials display correctly
- [ ] Multiplayer tested (if applicable)
- [ ] Documentation complete
- [ ] Tutorial system functional
- [ ] Accessibility options configured

---

## 🎓 Additional Resources

**Unreal Engine Documentation:**
- Enhanced Input System: https://docs.unrealengine.com/5.0/enhanced-input-in-unreal-engine/
- UMG UI Designer: https://docs.unrealengine.com/5.0/umg-ui-designer-for-unreal-engine/
- Niagara VFX: https://docs.unrealengine.com/5.0/niagara-overview/
- Blueprint Communication: https://docs.unrealengine.com/5.0/blueprint-communication-usage/

**Video Tutorials:**
- Search YouTube for "UE5 Enhanced Input Tutorial"
- Search for "UE5 UMG HUD Tutorial"
- Search for "UE5 Game Mode Setup"

Good luck with the implementation! The C++ foundation is solid, now it's time to make it visually stunning! 🚀
