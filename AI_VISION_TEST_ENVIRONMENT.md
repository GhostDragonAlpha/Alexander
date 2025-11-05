# AI Vision Test Environment Design
## Optimized for Screenshot-Based Development Workflow

---

## ⚠️ DEVELOPMENT WORKFLOW - READ THIS FIRST

**Making changes to this environment:**

Any modifications to `build_test_environment.py` (zones, cameras, lighting, objects) require **restarting Unreal Engine**.

**Process:**
1. Edit `Content/Python/build_test_environment.py`
2. Close Unreal or run `BUILD_TEST_ENV_AUTO.bat`
3. Unreal restarts → Phase 0 executes with changes
4. Verify in rebuilt environment

**Why?** Environment builds during Unreal startup (Phase 0). Running editor won't apply changes until restarted.

**Recent Example**: Lighting improvements applied 11/3/2025 - see [PHASE_0_IMPROVEMENTS_COMPLETE.md](../PHASE_0_IMPROVEMENTS_COMPLETE.md)

---

## 🎯 Design Philosophy

This test environment is purpose-built for your AI Vision System's screenshot methodology:
- **Auto-capture friendly**: Clear zones visible from fixed camera positions
- **Iteration tracking**: Easy to compare changes between screenshots
- **Visual reference points**: Grid markers and labeled zones for AI spatial awareness
- **Progress validation**: Before/after comparison areas
- **Material/lighting testing**: Dedicated zones for visual quality assessment

---

## 🗺️ Environment Layout (Top-Down View)

```
                    NORTH (Camera Position 1)
                           ↑
    ┌────────────────────────────────────────────┐
    │                                            │
    │  [1] REFERENCE GRID       [2] MATERIAL     │
    │      ZONE                     TEST ZONE    │
    │   • Measurement cubes        • Sample      │
W   │   • Scale markers            spheres      │   E
E   │   • Distance rulers          • Lighting   │   A
S   │                              demos        │   S
T ← │                                            │ → T
    │  [3] OBJECT               [4] ITERATION    │
    │      LIBRARY                  COMPARISON   │
    │   • Asset samples          • Before area  │
    │   • Primitives             • After area   │
    │   • Weapons, items         • Change zone  │
    │                                            │
    └────────────────────────────────────────────┘
                           ↓
                    SOUTH (Camera Position 2)

    [5] ELEVATED SHOWCASE AREA (Background)
        • Lit display zone
        • Character/NPC testing
        • VR scale reference
```

---

## 📐 Zone Specifications

### **Zone 1: Reference Grid (Northwest)**
**Purpose**: Spatial measurement & AI coordinate understanding

**Contents**:
- **10x10 grid** marked on floor (1 meter squares)
- **Numbered markers** (1-10) on X and Y axes
- **Height reference poles** at 1m, 2m, 3m heights
- **Corner cubes** of known sizes:
  - Small: 0.25m (red)
  - Medium: 0.5m (green)
  - Large: 1.0m (blue)
- **Distance markers**: Signs showing "1M", "5M", "10M" from camera

**Camera Angle**: 45° from corner, capturing full grid
**AI Test**: Can AI measure distances? Detect object sizes? Understand spatial relationships?

---

### **Zone 2: Material Test (Northeast)**
**Purpose**: Material recognition & lighting quality assessment

**Contents**:
- **6 Material Test Spheres** (arranged in 2 rows of 3):
  - Row 1: Metal, Glass, Plastic
  - Row 2: Rough Stone, Smooth Ceramic, Emissive
- **Lighting Comparison**:
  - Left half: Warm lighting (3000K)
  - Right half: Cool lighting (6500K)
- **Label plates** beneath each sphere with material name
- **Reflective test card** (chrome sphere + matte sphere side-by-side)

**Camera Angle**: Straight-on view of sphere array
**AI Test**: Can AI identify materials? Assess lighting quality? Detect reflections?

---

### **Zone 3: Object Library (Southwest)**
**Purpose**: Asset recognition & classification testing

**Contents**:
- **Organized display pedestals** (white cubes, 0.5m height):
  1. **Primitives Section**:
     - Sphere, Cube, Cylinder, Cone
  2. **Weapons Section**:
     - Pistol, Rifle (your current gray models)
  3. **Props Section**:
     - Crate, Barrel, Tool items
  4. **VR Objects**:
     - Grabbable items at proper VR scale

- **Clear labels** on pedestals
- **Consistent spacing**: 2m between items
- **Turntable markers**: Rotation indicators (N/E/S/W) around each pedestal

**Camera Angle**: Rotating view or multiple angles
**AI Test**: Can AI identify objects? Classify by type? Assess VR scale appropriateness?

---

### **Zone 4: Iteration Comparison (Southeast)**
**Purpose**: Before/after validation & change detection

**Layout**:
```
┌─────────────────────┬─────────────────────┐
│   BEFORE AREA       │   AFTER AREA        │
│                     │                     │
│   [Original State]  │   [Modified State]  │
│                     │                     │
│   • Same objects    │   • Changed objects │
│   • Same lighting   │   • New lighting    │
│   • Reference pose  │   • Same pose       │
└─────────────────────┴─────────────────────┘
         ↓                       ↓
    [CHANGE ZONE - highlighted differences]
```

**Contents**:
- **Identical setups** in both halves
- **Change indicators**: Purple highlight for modified objects
- **Version labels**: "V1", "V2", "V3" markers
- **Annotation boards**: What changed and why

**Camera Angle**: Split-screen view or side-by-side capture
**AI Test**: Can AI detect changes? Identify improvements? Validate requested modifications?

---

### **Zone 5: Elevated Showcase (Background Center)**
**Purpose**: Character/NPC testing & VR scale reference

**Contents**:
- **Lit display alcove** (your current setup - keep it!)
- **Character mannequin** on pedestal with proper lighting
- **VR height markers**: Floor to 1.6m (sitting), 1.8m (standing)
- **Interactive elements**: Buttons, levers, holographic displays
- **Scale reference human silhouette** (life-size cutout)

**Camera Angle**: Visible from all other zones as reference
**AI Test**: Does character scale look right? Are VR elements at proper height?

---

## 📷 Camera Placement Strategy

### **Fixed Camera Positions** (for auto-capture consistency)

**5-Camera System - Optimized for AI Vision Analysis**

**Camera 1: "Cam1_Overview_North"**
- Position: (0, -1500, 500) UU = (0, -15m, 5m)
- Rotation: Pitch -15°, Yaw 0°, Roll 0°
- FOV: 90° (wide angle)
- **Captures**: Full environment overview, all 4 zones visible
- **AI Use**: Context, spatial relationships, overall layout validation

**Camera 2: "Cam2_Detail_East"**
- Position: (1500, 0, 200) UU = (15m, 0, 2m)
- Rotation: Pitch 0°, Yaw 180°, Roll 0° (looking west)
- FOV: 60° (detail view)
- **Captures**: Zones 2 & 4 (Material Test + Iteration Comparison)
- **AI Use**: Material identification, change detection, iteration comparison

**Camera 3: "Cam3_Detail_West"**
- Position: (-1500, 0, 200) UU = (-15m, 0, 2m)
- Rotation: Pitch 0°, Yaw 0°, Roll 0° (looking east)
- FOV: 60° (detail view)
- **Captures**: Zones 1 & 3 (Reference Grid + Object Library)
- **AI Use**: Spatial measurements, object identification, grid reading

**Camera 4: "Cam4_Character_Showcase"**
- Position: (0, 500, 180) UU = (0, 5m, 1.8m) *eye level*
- Rotation: Pitch 0°, Yaw 180°, Roll 0° (looking north)
- FOV: 70° (medium view)
- **Captures**: Zone 5 (Background showcase area)
- **AI Use**: VR scale validation, character proportions, eye-level perspective

**Camera 5: "Cam5_Isometric"**
- Position: (2000, -2000, 1500) UU = (20m, -20m, 15m)
- Rotation: Pitch -30°, Yaw 45°, Roll 0° (3/4 view)
- FOV: 50° (tight view)
- **Captures**: Clean technical overview from elevated corner
- **AI Use**: 3D structure understanding, technical documentation

---

## 🎨 Visual Design Guidelines

### **Color Coding System**
- **Reference elements**: Blue
- **Test objects**: White/Gray
- **Changed/modified items**: Purple (your current highlight color)
- **Success indicators**: Green
- **Error/problem areas**: Red
- **Measurement aids**: Yellow

### **Lighting Setup**
```
Primary Lighting:
  • Directional Light (Sun): 45° angle, 5 lux
  • Sky Atmosphere: Clear blue sky
  • Ambient Occlusion: Enabled

Zone-Specific Lights:
  • Zone 1: Neutral white (5000K), overhead
  • Zone 2: Split (warm/cool) as specified
  • Zone 3: Spotlight per pedestal (focused)
  • Zone 4: Matching lights in both halves
  • Zone 5: Dramatic 3-point lighting (your current setup)

Post-Process:
  • Auto-exposure: DISABLED (consistent screenshots)
  • Bloom: Minimal (0.2)
  • Ambient Cubemap: Enabled
```

### **Material Standards**
- **Floor**: Dark gray with subtle grid (not distracting)
- **Walls**: Medium gray, matte
- **Pedestals**: Pure white, slight roughness (0.3)
- **Labels**: Black text on white plates
- **Highlight objects**: Emissive purple (0.5 intensity)

---

## 🔄 Screenshot Workflow Integration

### **Screenshot Methods**

**Method 1: Single Camera (Manual)**
```python
# 1. Pilot desired camera in World Outliner
#    Right-click camera → Pilot Camera
# 2. Take screenshot
py exec(open('Content/Python/snap.py').read())
```
**Output**: `snap_YYYYMMDD_HHMMSS.png`
**Use**: Quick single-angle capture for focused analysis

**Method 2: All Cameras (Multi-Angle)**
```python
# Captures from all 5 cameras sequentially
py exec(open('Content/Python/snap_all_test_cameras.py').read())
```
**Output**: 5 screenshots:
- `test_env_Cam1_Overview_North_[timestamp].png`
- `test_env_Cam2_Detail_East_[timestamp].png`
- `test_env_Cam3_Detail_West_[timestamp].png`
- `test_env_Cam4_Character_Showcase_[timestamp].png`
- `test_env_Cam5_Isometric_[timestamp].png`

**Use**: Complete environment analysis, iteration documentation

### **Workflow Examples**

**Quick Check** (1 screenshot):
```python
# Pilot Cam1_Overview_North
py exec(open('Content/Python/snap.py').read())
# Send snap_*.png to AI for quick validation
```

**Detailed Analysis** (5 screenshots):
```python
py exec(open('Content/Python/snap_all_test_cameras.py').read())
# Send all test_env_*.png files to AI for comprehensive analysis
```

**Iteration Comparison** (2 screenshots):
```python
# Before change:
py exec(open('Content/Python/snap.py').read())  # iteration_before.png

# Make modifications...

# After change:
py exec(open('Content/Python/snap.py').read())  # iteration_after.png

# Send both to AI: "What changed between these screenshots?"
```

### **Screenshot Storage**

**Location**: `Alexander/Saved/Screenshots/WindowsEditor/`

**Naming Conventions**:
- Manual single: `snap_20251103_153045.png`
- Multi-camera: `test_env_Cam1_Overview_North_20251103_153045.png`
- Iteration markers: `iteration_5.png` (manually renamed for reference)

### **AI Analysis Workflow**

```
1. Screenshot captured → Saved to Screenshots/WindowsEditor/

2. AI Backend receives screenshot:
   POST /api/analyze_scene
   {
     "image_path": "auto_20251103_095100.png",
     "camera_position": "Overview North",
     "zones_visible": [1, 2, 3, 4],
     "previous_iteration": "iteration_5.png"
   }

3. AI analyzes:
   ✓ Which zones are visible
   ✓ What objects are present
   ✓ Material/lighting quality
   ✓ Scale correctness
   ✓ Changes from previous iteration

4. AI generates actions:
   [
     {
       "action": "improve_lighting_zone_2",
       "reason": "Cool lighting too harsh, reduce intensity 30%",
       "zone": 2,
       "priority": 8
     },
     {
       "action": "add_object_zone_3",
       "reason": "Props section empty, add crate asset",
       "zone": 3,
       "priority": 6
     }
   ]

5. You approve/execute actions

6. New screenshot confirms changes
```

---

## 🎮 Practical Implementation

### **Step 1: Build the Base Environment** (30 min)

1. Create new level: `AIVisionTestArena`
2. Set floor size: 40m x 40m
3. Add walls (4m height) around perimeter
4. Paint floor grid (10x10 squares, 1m each)
5. Set up basic lighting (sun + sky)

### **Step 2: Create Zone 1 - Reference Grid** (20 min)

```python
# Unreal Python script to auto-generate grid
import unreal

def create_reference_grid():
    # Create numbered floor markers
    for x in range(11):
        for y in range(11):
            # Spawn text actor at grid intersection
            location = unreal.Vector(x * 100, y * 100, 1)  # 1m = 100 units
            # Create number label

    # Create height reference poles
    for height in [100, 200, 300]:  # 1m, 2m, 3m
        # Spawn cylinder with height marker
        pass

    # Create corner reference cubes
    sizes = {"small": 25, "medium": 50, "large": 100}
    colors = {"small": (1,0,0), "medium": (0,1,0), "large": (0,0,1)}
    for size_name, size in sizes.items():
        # Spawn colored cube
        pass

create_reference_grid()
```

### **Step 3: Create Zone 2 - Material Test** (20 min)

- Place 6 spheres in 2x3 grid
- Assign different materials to each
- Set up split lighting (2 spotlights, different temperatures)
- Add label text renderers below each sphere

### **Step 4: Create Zone 3 - Object Library** (30 min)

- Create white pedestal blueprint (reusable)
- Place pedestals in organized rows
- Add your current assets (spheres, cubes, weapons)
- Add text labels to each pedestal
- Ensure 2m spacing between items

### **Step 5: Create Zone 4 - Comparison Area** (15 min)

- Duplicate a simple setup in both halves
- Mark "BEFORE" and "AFTER" areas with text
- Add purple emissive material for "changed" highlights

### **Step 6: Set Up Cameras** (20 min)

```python
# Create camera positions
import unreal

camera_positions = {
    "Overview_North": {"loc": (0, -1500, 500), "rot": (-15, 0, 0)},
    "Detail_East": {"loc": (1500, 0, 200), "rot": (0, -180, 0)},
    "Detail_West": {"loc": (-1500, 0, 200), "rot": (0, 0, 0)},
    "Character_Showcase": {"loc": (0, 500, 180), "rot": (0, 180, 0)},
    "Isometric": {"loc": (2000, -2000, 1500), "rot": (-30, 45, 0)},
}

for name, transform in camera_positions.items():
    # Spawn CineCameraActor
    # Set transform
    # Set FOV
    # Name actor for easy selection
    pass
```

### **Step 7: Configure Post-Processing** (10 min)

- Add Post Process Volume (set to infinite)
- **Disable auto-exposure** (critical for consistent screenshots!)
- Set fixed exposure value
- Minimal bloom
- No chromatic aberration

### **Step 8: Test Screenshot Workflow** (15 min)

1. Position viewport at Camera 1 position
2. Run: `py exec(open('Content/Python/snap.py').read())`
3. Check saved screenshot quality
4. Repeat for other camera positions
5. Verify all zones are clearly visible

---

## 📊 Testing Checklist

After building the environment, validate each test capability:

- [ ] **Spatial awareness**: Can AI measure distances using reference grid?
- [ ] **Object recognition**: Can AI identify objects in library zone?
- [ ] **Material identification**: Can AI name materials on test spheres?
- [ ] **Lighting assessment**: Can AI judge lighting quality?
- [ ] **Change detection**: Can AI spot differences in comparison zone?
- [ ] **Scale validation**: Does AI recognize correct VR scale?
- [ ] **Iteration tracking**: Can AI compare iteration_N vs iteration_N+1?
- [ ] **Quality feedback**: Does AI provide actionable improvement suggestions?

---

## 🚀 Advanced Features (Optional)

### **Dynamic Test Scenarios**

Create blueprints that can auto-configure the environment for specific tests:

**BP_TestScenario_Lighting**:
- Cycles through different lighting conditions
- Takes screenshot at each setting
- AI evaluates which looks best

**BP_TestScenario_Materials**:
- Swaps materials on objects
- Captures each variation
- AI identifies materials and quality

**BP_TestScenario_Scale**:
- Places same object at different sizes
- AI determines which scale is appropriate for VR

### **Annotation System**

**Overlay Information on Screenshots**:
- Frame counter
- Timestamp
- Camera position name
- Active zones
- Modified objects highlighted

This helps AI understand context without needing external metadata.

---

## 📝 Documentation Integration

### **Screenshot Naming Convention**

```
Format: [type]_[date]_[time]_[camera]_[iteration].png

Examples:
- auto_20251103_095100_cam1_iter12.png
- manual_20251103_143022_overview_iter45.png
- reference_grid_close.png (your current reference)
- iteration_5.png (milestone captures)
```

### **AI Analysis Log**

After each screenshot analysis, log results:

```json
{
  "screenshot": "auto_20251103_095100.png",
  "timestamp": "2025-11-03T09:51:00Z",
  "camera": "Overview_North",
  "zones_analyzed": [1, 2, 3, 4],
  "ai_observations": [
    "Zone 2: Purple spheres detected (material test)",
    "Zone 3: Weapons on pedestals, gray untextured",
    "Zone 5: Character visible with lighting",
    "Grid reference visible, spatial measurement possible"
  ],
  "suggestions": [
    "Add textures to weapon models (priority 8)",
    "Adjust purple material - too bright (priority 6)",
    "Add more variety to object library (priority 4)"
  ],
  "iteration": 5,
  "changes_from_previous": "Added purple materials to test spheres"
}
```

---

## 🎯 Success Metrics

Your test environment is working when:

1. **AI can understand screenshots without human explanation**
2. **Iteration comparisons clearly show progress**
3. **Changes are easily visible between captures**
4. **AI provides specific, actionable feedback**
5. **You can rapidly test changes with manual snap.py captures**
6. **Auto-captures provide consistent reference points**
7. **All 4 zones serve clear testing purposes**

---

## 🔥 Next Steps

1. **Build Zone 1 first** (Reference Grid) - establishes foundation
2. **Set up Camera 1** (Overview) - test screenshot quality
3. **Create one test object in Zone 3** - validate workflow
4. **Take comparison screenshots** - verify AI can detect
5. **Iterate and expand** based on what works

**Estimated total build time**: 2-3 hours
**Payoff**: Systematic, professional AI testing environment that supports your autonomous development vision!

---

Ready to build this? I can help generate the Python scripts to auto-create the grid, pedestals, and camera positions!
