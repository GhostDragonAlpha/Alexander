# AUTONOMOUS DEVELOPMENT COMMANDS

## The 11-Step Loop Commands

### 1. OBSERVE
```bash
# STEP 1a: Maximize Unreal Editor window
python maximize_unreal.py

# STEP 1b: Take screenshot
python take_screenshot.py

# STEP 1c: Check logs and status
tail -50 Saved/Logs/Alexander.log
grep "error C" Saved/Logs/Alexander.log
git status
ls -lt Source/Alexander/Private/ | head -10
ls -lt Content/ | head -10
```

### 2. ANALYZE
```bash
grep "Crash" Saved/Logs/Alexander.log
grep "error C" Saved/Logs/Alexander.log
grep "warning" Saved/Logs/Alexander.log
```

### 3. PLAN
```bash
find Source -name "*Pattern*"
find Content -name "*Pattern*"
grep -r "ClassName" Source/
grep -r "FunctionName" Source/
```

### 4. IMPLEMENT (C++)
```bash
# Read file
cat Source/Alexander/Private/FileName.cpp

# Use Edit tool for changes
Edit Source/Alexander/Private/FileName.cpp
Edit Source/Alexander/Public/FileName.h
```

### 4. IMPLEMENT (Python/Unreal)
```python
import unreal

# Load asset
asset = unreal.EditorAssetLibrary.load_asset("/Game/Path/Asset")

# Spawn actor
world = unreal.EditorLevelLibrary.get_editor_world()
location = unreal.Vector(0, 0, 0)
rotation = unreal.Rotator(0, 0, 0)
actor = world.spawn_actor(ActorClass, location, rotation)

# Get all actors
actors = unreal.EditorLevelLibrary.get_all_level_actors()

# Set actor location
actor.set_actor_location(unreal.Vector(100, 200, 300), False, True)

# Select actors
unreal.EditorLevelLibrary.set_selected_level_actors([actor])
```

### 5. BUILD
```bash
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" Alexander Win64 Development "C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" -WaitMutex
tail -100 Saved/Logs/Alexander.log | grep "error"
grep "error C" Saved/Logs/Alexander.log | head -20
```

### 6. VISUAL VERIFY
```bash
python take_screenshot.py
Read current_screen.png
```

### 7. FUNCTIONAL TEST
```bash
python test_script.py
```

```python
# Or via Unreal Python
import unreal
unreal.AutomationLibrary.take_high_res_screenshot(1920, 1080, "test_result")
```

### 8. CHECK SIDE EFFECTS
```bash
tail -100 Saved/Logs/Alexander.log | grep -i "error\|warning"
grep "Crash\|Assert\|Fatal" Saved/Logs/Alexander.log
```

### 9. POLISH
```bash
grep "TODO" Source/Alexander/Private/*.cpp
grep "FPS" Saved/Logs/Alexander.log | tail -20
python take_screenshot.py
```

### 10. DOCUMENT
```bash
echo "Change description" >> CHANGELOG.md
git add Source/Alexander/Private/File.cpp
git add Source/Alexander/Public/File.h
git commit -m "Description"
```

```python
# Save level
import unreal
unreal.EditorLevelLibrary.save_current_level()
```

### 11. NEXT TASK
```bash
python take_screenshot.py
tail -50 Saved/Logs/Alexander.log
# → Loop back to step 1
```

---

## Quick Reference Commands

### Screenshot
```bash
python take_screenshot.py
Read current_screen.png
```

### Log Checking
```bash
tail -50 Saved/Logs/Alexander.log
tail -100 Saved/Logs/Alexander.log | grep "error"
grep "error C" Saved/Logs/Alexander.log
grep "warning" Saved/Logs/Alexander.log
grep "Crash" Saved/Logs/Alexander.log
```

### File Finding
```bash
find Source -name "*.cpp"
find Source -name "*.h"
find Content -name "*.uasset"
find Source -name "*Spaceship*"
ls -lt Source/Alexander/Private/ | head -10
```

### Code Search
```bash
grep -r "ClassName" Source/
grep -r "FunctionName" Source/
grep "TODO" Source/Alexander/Private/*.cpp
```

### Building
```bash
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" Alexander Win64 Development "C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" -WaitMutex
```

### Git
```bash
git status
git diff
git add .
git commit -m "Message"
git log --oneline -20
```

---

## Unreal Python API Commands

### Level Operations
```python
import unreal

# Get editor world
world = unreal.EditorLevelLibrary.get_editor_world()

# Load level
unreal.EditorLevelLibrary.load_level("/Game/LevelName")

# Save level
unreal.EditorLevelLibrary.save_current_level()

# Get all actors
actors = unreal.EditorLevelLibrary.get_all_level_actors()

# Select actors
unreal.EditorLevelLibrary.set_selected_level_actors([actor1, actor2])
```

### Spawn Actors
```python
# Spawn from class
location = unreal.Vector(0, 0, 0)
rotation = unreal.Rotator(0, 0, 0)
actor = world.spawn_actor(ActorClass, location, rotation)

# Spawn from blueprint
bp_class = unreal.load_class(None, "/Game/Path/BP_Actor.BP_Actor_C")
actor = world.spawn_actor(bp_class, location, rotation)

# Spawn specific actors
unreal.EditorLevelLibrary.spawn_actor_from_class(ActorClass, location, rotation)
```

### Actor Manipulation
```python
# Get actor location
location = actor.get_actor_location()

# Set actor location
actor.set_actor_location(unreal.Vector(100, 200, 300), False, True)

# Get actor rotation
rotation = actor.get_actor_rotation()

# Set actor rotation
actor.set_actor_rotation(unreal.Rotator(0, 90, 0), True)

# Get actor scale
scale = actor.get_actor_scale3d()

# Set actor scale
actor.set_actor_scale3d(unreal.Vector(2, 2, 2))

# Destroy actor
actor.destroy_actor()
```

### Asset Operations
```python
# Load asset
asset = unreal.EditorAssetLibrary.load_asset("/Game/Path/Asset")

# Check if asset exists
exists = unreal.EditorAssetLibrary.does_asset_exist("/Game/Path/Asset")

# Get asset path
path = unreal.EditorAssetLibrary.get_path_name_for_loaded_asset(asset)

# Save asset
unreal.EditorAssetLibrary.save_asset("/Game/Path/Asset")
```

### Screenshot
```python
# Take screenshot
unreal.AutomationLibrary.take_high_res_screenshot(1920, 1080, "screenshot_name")

# Screenshot with specific camera
unreal.AutomationLibrary.take_automation_screenshot("test_name")
```

### Component Access
```python
# Get component by class
component = actor.get_component_by_class(unreal.StaticMeshComponent)

# Get all components
components = actor.get_components_by_class(unreal.ActorComponent)

# Set static mesh
mesh_component.set_static_mesh(mesh_asset)

# Set material
mesh_component.set_material(0, material_asset)
```

### Logging
```python
# Log message
unreal.log("Message")

# Log warning
unreal.log_warning("Warning message")

# Log error
unreal.log_error("Error message")
```

---

## The Complete Fast Loop (Copy-Paste Ready)

```bash
# 1. Observe
python take_screenshot.py && tail -50 Saved/Logs/Alexander.log

# 2-3. Analyze & Plan (human judgment)

# 4. Implement
# (Edit files as needed)

# 5. Build (if C++)
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" Alexander Win64 Development "C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" -WaitMutex

# 6. Verify
python take_screenshot.py

# 7. Test
python test_feature.py

# 8. Check
tail -100 Saved/Logs/Alexander.log | grep -i "error"

# 9-10. Polish & Document
git add . && git commit -m "Feature implemented"

# 11. Next → Loop back to step 1
```

---

## Python Script Template for Unreal Automation

```python
#!/usr/bin/env python3
"""
Autonomous development iteration script
"""
import unreal

def main():
    # Get world
    world = unreal.EditorLevelLibrary.get_editor_world()

    # Log start
    unreal.log("=== Starting iteration ===")

    # Your implementation here
    # Example: Spawn spaceship
    bp_class = unreal.load_class(None, "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer_C")
    if bp_class:
        location = unreal.Vector(0, 0, 20000)
        rotation = unreal.Rotator(0, 0, 0)
        actor = world.spawn_actor(bp_class, location, rotation)
        unreal.log(f"Spawned: {actor.get_name()}")

    # Take screenshot
    unreal.AutomationLibrary.take_high_res_screenshot(1920, 1080, "iteration_result")

    # Save level
    unreal.EditorLevelLibrary.save_current_level()

    unreal.log("=== Iteration complete ===")

if __name__ == "__main__":
    main()
```

---

## Execute This Script Via:
```bash
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" -ExecutePythonScript="script.py" -stdout -unattended -nopause -nosplash
```

---

**THESE ARE THE COMMANDS. EXECUTE THEM IN ORDER. REPEAT FOREVER.**
