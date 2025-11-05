# First Mission Automation Scripts

This folder contains Python automation scripts for implementing the First Mission system in Unreal Editor.

## Prerequisites

1. **Compile C++ Project First!**
   - Open `Alexander.uproject` in Unreal Editor
   - Click "Yes" to compile C++ code
   - Wait for compilation to complete (5-10 minutes)
   - Verify no compilation errors

2. **Enable Python Plugin**
   - Edit → Plugins → Search "Python"
   - Enable "Python Editor Script Plugin"
   - Restart Unreal Editor

3. **Enable Remote Execution**
   - Edit → Editor Preferences → Python
   - Enable "Enable Remote Execution"
   - Note the port (default: 9998)

## Script Execution Order

### Step 1: Initial Setup (After C++ Compilation)

```bash
# Verify C++ classes are available and setup folders
python remote_client.py --file "mission_system_setup.py"

# CRITICAL: Trigger build
python remote_client.py --file "trigger_lighting_build.py"

# Wait 30-60 seconds, then restart editor if connection lost
```

### Step 2: Create Mission Level

```bash
# Create the mission level with basic setup
python remote_client.py --file "create_mission_level.py"

# CRITICAL: Trigger build
python remote_client.py --file "trigger_lighting_build.py"

# Wait for build, then restart editor if connection lost
```

### Step 3: Test Mission Systems

```bash
# Test that mission systems are working
python remote_client.py --file "test_mission_systems.py"

# This will verify:
# - All C++ classes are compiled
# - Mission level exists and loads
# - Required actors are in level
# - Game mode configuration
# - Input actions (will show warnings for manual steps)
# - Blueprint classes (will show warnings for manual steps)
# - UMG widgets (will show warnings for manual steps)
```

### Step 4: After Manual Setup

After completing manual steps (Enhanced Input, Blueprints, UMG):

```bash
# Rerun tests to verify everything
python remote_client.py --file "test_mission_systems.py"
```

## Manual Steps Required

Some tasks cannot be fully automated via Python and require manual work in Unreal Editor:

### 1. Enhanced Input Setup
**Location:** Content/FirstMission/Input/

**Create Input Actions:**
- Right-click → Input → Input Action
- Create these actions:
  - `IA_Move` (Value Type: Axis2D)
  - `IA_Look` (Value Type: Axis2D)
  - `IA_Jump` (Value Type: Digital)
  - `IA_Sprint` (Value Type: Digital)
  - `IA_Crouch` (Value Type: Digital)
  - `IA_Interact` (Value Type: Digital)
  - `IA_Throttle` (Value Type: Axis1D)
  - `IA_Pitch` (Value Type: Axis1D)
  - `IA_Yaw` (Value Type: Axis1D)
  - `IA_Roll` (Value Type: Axis1D)
  - `IA_LandingGear` (Value Type: Digital)
  - `IA_ExitShip` (Value Type: Digital)

**Create Input Mapping Contexts:**
- Right-click → Input → Input Mapping Context
- Create: `IMC_GroundControls`
- Create: `IMC_ShipControls`
- Create: `IMC_VRControls` (if using VR)

**Map Keys:**
See UNREAL_EDITOR_SETUP_GUIDE.md Phase 3 for detailed key mappings.

### 2. Create Blueprint Classes

**BP_FirstMissionGameMode:**
1. Content Browser → Right-click → Blueprint Class
2. Parent Class: Search "FirstMissionGameMode" (C++ class)
3. Name: `BP_FirstMissionGameMode`
4. Save to: Content/FirstMission/Blueprints/

**BP_SurfacePlayer:**
1. Content Browser → Right-click → Blueprint Class
2. Parent Class: Search "SurfacePlayerCharacter" (C++ class)
3. Name: `BP_SurfacePlayer`
4. Open Blueprint:
   - Set DefaultMappingContext: `IMC_GroundControls`
   - Assign all Input Actions (MoveAction → IA_Move, etc.)
5. Save

### 3. Create UMG Widgets

**WBP_MissionHUD:**
1. Content Browser → Right-click → User Interface → Widget Blueprint
2. Name: `WBP_MissionHUD`
3. Follow UNREAL_EDITOR_SETUP_GUIDE.md Phase 8 for widget hierarchy
4. Bind events to FirstMissionGameMode C++ events

**WBP_MainMenu:**
1. Content Browser → Right-click → User Interface → Widget Blueprint
2. Name: `WBP_MainMenu`
3. Follow UNREAL_EDITOR_SETUP_GUIDE.md Phase 8 for widget hierarchy
4. Wire up button events (Start Mission, Settings, Quit)

### 4. Configure Project Settings

**Set Default Game Mode:**
1. Edit → Project Settings
2. Maps & Modes → Default GameMode
3. Select: `BP_FirstMissionGameMode`
4. Default Pawn Class: `BP_SurfacePlayer`

**Set Startup Level:**
1. Edit → Project Settings
2. Maps & Modes → Editor Startup Map
3. Select: `MissionLevel`
4. Game Default Map: `MissionLevel`

### 5. Add Spaceship to Level

The spaceship must have `ShipEntryComponent` added:
1. Select spaceship in level
2. Details panel → Add Component → ShipEntryComponent
3. Configure entry points:
   - Cockpit entry at front
   - Emergency exit at side
4. Set InteractionRadius: 200cm

### 6. Add Landing Guidance Systems

1. Drag `LandingGuidanceSystem` into level (2 instances)
2. Position near landing zones
3. Configure:
   - MaxGuidanceAltitude: 5000m
   - SafeLandingSpeed: 5 m/s
   - bShowVisualGuidance: true

## Troubleshooting

### "C++ classes not found" Error
**Solution:** Compile the project first! Open Alexander.uproject and let it compile.

### "Connection lost" After Build
**Solution:** This is normal! Restart Unreal Editor and reconnect.

### Import Errors in VS Code
**Solution:** These are expected - `unreal` module only exists in Unreal Editor.

### Scripts Not Executing
**Solution:** 
1. Verify Python plugin is enabled
2. Verify Remote Execution is enabled
3. Check Output Log in Unreal Editor for errors

## Reference Documentation

- **UNREAL_EDITOR_SETUP_GUIDE.md** - Detailed step-by-step implementation
- **FIRST_MISSION_PROGRESS.md** - Technical documentation
- **CRITICAL_WORKFLOW_ORDER.md** - Build workflow (MUST READ!)
- **GITHUB_INTEGRATED_WORKFLOW.md** - Git workflow

## Testing Checklist

After setup, verify:
- [ ] C++ classes compiled successfully
- [ ] FirstMissionGameMode set as default
- [ ] BP_SurfacePlayer spawns correctly
- [ ] Input controls work (WASD, mouse, etc.)
- [ ] Can approach and enter spaceship
- [ ] Ship controls work
- [ ] Landing guidance activates near surfaces
- [ ] Mission HUD displays correctly
- [ ] Mission stages progress properly

## Support

If you encounter issues:
1. Check Output Log in Unreal Editor
2. Review error messages carefully
3. Verify you followed CRITICAL_WORKFLOW_ORDER.md
4. Check that all C++ classes compiled
5. Restart Unreal Editor if connection issues persist

---

**Remember:** Always trigger build after making changes, then wait before testing!
