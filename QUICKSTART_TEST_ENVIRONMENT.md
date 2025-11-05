# Quick Start: AI Vision Test Environment

## 🚀 Automated Setup (EASIEST)

**The test environment builds automatically when you start Unreal Editor!**

### How It Works:
- `init_unreal.py` includes **Phase 0** that builds the test environment
- Runs automatically on Unreal startup (no manual commands needed)
- Only builds once (checks for existing cameras, skips if found)
- Takes ~30 seconds during first startup

### To Trigger Build:
```batch
# Simply restart Unreal Editor
BUILD_TEST_ENV_AUTO.bat
```

This script:
1. ✓ Closes Unreal Editor (if running)
2. ✓ Launches Unreal with your project
3. ✓ `init_unreal.py` Phase 0 builds test environment automatically
4. ✓ Continues with Phases 1-5 (your spaceship build)

**Build time**: ~30 seconds (Phase 0) + ~90 seconds (Phases 1-5)

---

## 🔧 Manual Build (Alternative)

If you prefer to build manually or rebuild:

### Step 1: Create/Open Level
Open any level in Unreal Editor (test environment can be built in current level)

### Step 2: Run Builder Directly

Open **Python Console** (Window → Developer Tools → Output Log → Python tab):

```python
py exec(open('Content/Python/auto_build_test_env.py').read())
```

**This will:**
- ✓ Create AIVisionTestArena level (if needed)
- ✓ Build floor and walls
- ✓ Create 4 test zones with labels
- ✓ Place reference grid with measurements
- ✓ Position 5 cameras
- ✓ Set up basic lighting

**Build time**: ~30 seconds

---

## 📸 Test Your Screenshots

### Method 1: Single Camera (Quick Check)

1. Open **World Outliner** (Window → World Outliner)
2. Find camera: `Cam1_Overview_North`
3. **Right-click** → **Pilot Camera**
4. Your viewport now shows what this camera sees
5. Take screenshot:
   ```python
   py exec(open('Content/Python/snap.py').read())
   ```
   **Output**: `snap_YYYYMMDD_HHMMSS.png`

### Method 2: All Cameras (Complete Coverage)

Capture from all 5 test cameras in one command:
```python
py exec(open('Content/Python/snap_all_test_cameras.py').read())
```

**Output**: 5 timestamped screenshots:
- `test_env_Cam1_Overview_North_[timestamp].png`
- `test_env_Cam2_Detail_East_[timestamp].png`
- `test_env_Cam3_Detail_West_[timestamp].png`
- `test_env_Cam4_Character_Showcase_[timestamp].png`
- `test_env_Cam5_Isometric_[timestamp].png`

**Use this for**: Full environment analysis, iteration documentation

### Method 3: Free Camera (Custom Angles)

1. Fly viewport to any position (WASD + mouse)
2. Take screenshot:
   ```python
   py exec(open('Content/Python/snap.py').read())
   ```

**All screenshots save to**: `Alexander/Saved/Screenshots/WindowsEditor/`

---

## 🎨 Manual Finishing Touches (15 min)

The auto-builder creates the structure, but you need to add:

### 1. **Post Process Volume** (CRITICAL!)
This ensures consistent screenshot brightness.

**Steps:**
1. **Place Actors** → Search "Post Process"
2. Drag **Post Process Volume** into level
3. In **Details** panel:
   - Check: **Infinite Extent (Unbound)**
   - Expand: **Lens → Exposure**
   - **Uncheck** "Auto Exposure"
   - Set **Exposure Compensation**: 0.0
   - Set **Min/Max Brightness**: 1.0

**Why?** Auto-exposure makes screenshots inconsistent. The AI needs fixed lighting to compare iterations!

### 2. **Assign Materials to Test Spheres (Zone 2)**

Find the 6 spheres in the Material Test Zone:
- Select sphere → Details → Static Mesh → Materials
- Assign different materials:
  - **Metal**: M_Metal_Gold or M_Metal_Steel
  - **Glass**: M_Glass
  - **Plastic**: M_Plastic
  - **Stone**: M_CobbleStone_Rough
  - **Ceramic**: M_Ceramic_Tile
  - **Emissive**: M_Emissive (create one with Emissive Color)

You can use Starter Content materials or create your own.

### 3. **Place Your Assets in Object Library (Zone 3)**

**Find your pedestals** in World Outliner (search "Pedestal_"):
- `Pedestal_Sphere`, `Pedestal_Cube`, etc.

**Place objects on pedestals:**
1. Select your weapon model (pistol/rifle)
2. Move it above the corresponding pedestal
3. Set Z location to ~50 (on top of pedestal)

**Your current assets to place:**
- Sphere, Cube (already there as primitives)
- Your gray pistol model
- Your gray rifle model

### 4. **Set Up Zone 5 (Optional)**

If you want to keep your current character showcase:
1. Move your existing lit alcove to position: (0, 0, 200)
2. Move your character mannequin there
3. Keep the warm lighting

Or build a new one:
- Create cube walls for alcove
- Add 3 point lights (key, fill, rim)
- Place character at eye level (180cm)

---

## 📊 Verify Everything Works

### Test Checklist:

#### Camera Test
- [ ] Pilot **Cam1_Overview_North** - can you see all 4 zones?
- [ ] Pilot **Cam2_Detail_East** - clear view of Material + Comparison zones?
- [ ] Pilot **Cam3_Detail_West** - clear view of Reference + Object zones?

#### Screenshot Test
- [ ] From Cam1, run `snap.py` - screenshot saved?
- [ ] Check screenshot quality - is it bright enough?
- [ ] Can you read the text labels in the screenshot?

#### Lighting Test
- [ ] Move around - is lighting consistent everywhere?
- [ ] No dark corners where objects disappear?
- [ ] Post Process Volume set to **Infinite** and **No Auto-Exposure**?

---

## 🎯 Using the Environment

### Take Reference Screenshots (First Time)

Visit each camera position and capture reference images:

```python
# From Cam1_Overview_North
py exec(open('Content/Python/snap.py').read())
# Manually rename to: reference_overview.png

# From Cam2_Detail_East
py exec(open('Content/Python/snap.py').read())
# Rename to: reference_materials.png

# From Cam3_Detail_West
py exec(open('Content/Python/snap.py').read())
# Rename to: reference_objects.png
```

These become your baseline for comparison.

### Test Iteration Workflow

1. **Make a change** (e.g., change material on a sphere to purple)
2. **Take screenshot** from same camera position
   ```python
   py exec(open('Content/Python/snap.py').read())
   ```
3. **Compare** new screenshot to reference
4. **Ask AI**: "Compare iteration_5.png to reference_overview.png - what changed?"

### Test AI Vision Analysis

Move an object, then ask:
```python
# Take before screenshot
py exec(open('Content/Python/snap.py').read())

# Move the sphere 2 meters
# (manually move it in editor)

# Take after screenshot
py exec(open('Content/Python/snap.py').read())
```

Show both screenshots to AI:
> "I moved an object between these two screenshots. Can you identify which object moved and by how much?"

---

## 🔧 Troubleshooting

### "I don't see any text labels"
- Text renders might be too small. Zoom in or increase scale in script
- Make sure you're viewing in **Lit** mode (not Wireframe)

### "Everything is too dark/bright"
- **Critical**: Did you disable Auto-Exposure in Post Process Volume?
- Adjust Directional Light intensity (select Sun_Main, change Intensity)

### "Screenshots are inconsistent brightness"
- **Auto-exposure is still on!** Check Post Process Volume settings
- Make sure PPV has "Infinite Extent" checked

### "Can't find the cameras"
- Open **World Outliner**
- Search: "Cam1" or "Cam2" etc.
- They should all be named: Cam1_Overview_North, etc.

### "Python script failed"
- Make sure **Python Editor Script Plugin** is enabled
- Check Output Log for error messages
- Try running script again (it's safe to re-run)

---

## 📐 Understanding the Zones

### **Zone 1: Reference Grid** (Northwest / Left-Top)
- **Purpose**: Spatial measurement
- **Use**: "How far is Object A from Object B?"
- **Test**: AI should be able to measure distances using grid

### **Zone 2: Material Test** (Northeast / Right-Top)
- **Purpose**: Material/lighting quality
- **Use**: "What material is this sphere?"
- **Test**: AI should identify materials and assess lighting

### **Zone 3: Object Library** (Southwest / Left-Bottom)
- **Purpose**: Object recognition
- **Use**: "What objects are visible?"
- **Test**: AI should classify and name objects

### **Zone 4: Comparison** (Southeast / Right-Bottom)
- **Purpose**: Change detection
- **Use**: "What changed between iterations?"
- **Test**: AI should spot differences

---

## 🎮 Next Level: Advanced Usage

### Auto-Capture Setup

Set up the AIScreenshotCapture actor from your C++ plugin:

1. Place `AAIScreenshotCapture` actor in level
2. Set auto-capture interval (e.g., 60 seconds)
3. As you work, it automatically captures iteration screenshots
4. AI backend receives screenshots and analyzes continuously

### Dynamic Test Scenarios

Create Blueprint that cycles through scenarios:
- Swap materials on spheres automatically
- Move objects to different positions
- Change lighting conditions
- AI evaluates each variation

### Integration with AI Backend

Your Python backend (`vision_analyzer.py`) can:
1. Monitor screenshot folder for new images
2. Auto-analyze when new screenshot appears
3. Send suggestions back to Unreal
4. Execute approved changes automatically

---

## 📝 What You Built

**Physical Layout:**
```
     NORTH
       ↑
  [1] | [2]
  ----+----
  [3] | [4]
       ↓
     SOUTH

[1] = Reference Grid (measurements)
[2] = Material Test (6 spheres)
[3] = Object Library (pedestals)
[4] = Iteration Comparison (before/after)

5 Cameras placed around perimeter
```

**Asset Count:**
- ~50 text labels
- 3 reference cubes (small/medium/large)
- 3 height poles
- 6 material test spheres
- ~10 pedestals for objects
- 4 walls + floor
- 2 lights (sun + sky)
- 5 cameras

---

## 🚀 You're Ready!

You now have a professional AI vision testing environment that:

✅ Provides consistent screenshot reference points
✅ Allows easy iteration comparison
✅ Tests AI spatial awareness, object recognition, material ID
✅ Supports your autonomous development workflow
✅ Integrates with your snap.py screenshot system

**Next:** Start testing! Make changes, take screenshots, and see what your AI vision system can detect!

---

## 📚 Related Docs

- [Full Environment Design](AI_VISION_TEST_ENVIRONMENT.md) - Detailed specifications
- [Screenshot Commands](SCREENSHOT_COMMANDS.md) - How to use snap.py
- [AI Vision System](AI_VISION_SYSTEM.md) - Complete system architecture

---

**Questions? Issues?**

If something doesn't work:
1. Check **Output Log** for Python errors
2. Verify all plugins enabled (Python Editor Script Plugin)
3. Make sure you saved the level after running the script
4. Try rebuilding from a fresh Empty Level

**Happy testing!** 🎮🤖
