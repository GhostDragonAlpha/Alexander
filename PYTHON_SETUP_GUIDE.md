# Python Integration Setup Guide
## Alexander VR Project - Quick Start

---

## 1. Install Python Dependencies

### Step 1: Open Terminal
```bash
cd C:\Users\allen\Desktop\Alexander\Alexander
```

### Step 2: Install Dependencies
```bash
pip install -r requirements.txt
```

### Step 3: Verify Installation
```bash
python -c "import numpy, requests; print('✅ Dependencies installed successfully!')"
```

---

## 2. Verify Unreal Python Integration

### In Unreal Editor

1. Open **Output Log** (Window → Developer Tools → Output Log)

2. Open **Python Console** (Window → Developer Tools → Python Console)

3. Run test command:
```python
import unreal
print("✅ Unreal Python working!")
```

---

## 3. Test API Helper Module

### In Unreal Python Console

```python
# Import helper
from unreal_api_helper import get_unreal_helper

# Create instance
helper = get_unreal_helper()

# Test material creation
mat = helper.create_material("M_Test_Material", "/Game/Test")
print(f"✅ Material created: {mat}")

# Test C++ class loading
actor = helper.spawn_cpp_actor(
    "/Script/Engine.StaticMeshActor",
    unreal.Vector(0, 0, 100)
)
print(f"✅ Actor spawned: {actor}")
```

---

## 4. Run Phase Scripts

### Workflow Configuration

Edit: `Content/Python/workflow_config.json`

```json
{
  "settings": {
    "auto_run": true
  },
  "phases": {
    "phase_space_farming": {
      "enabled": true,
      "script": "space_farming_mechanics.py",
      "description": "Space farming systems"
    }
  }
}
```

### Manual Script Execution

In Unreal Python Console:
```python
# Run space farming setup
exec(open(r'C:\Users\allen\Desktop\Alexander\Alexander\Content\Python\space_farming_mechanics.py').read())
```

---

## 5. Common Issues and Solutions

### Issue: "No module named 'numpy'"
**Solution:**
```bash
pip install numpy
```

### Issue: "No module named 'unreal'"
**Solution:** You must run Python code from **within Unreal Editor**, not from external Python

### Issue: "MaterialEditingLibrary has no attribute 'create_material'"
**Solution:** This function doesn't exist. Use `unreal_api_helper.py` instead:
```python
from unreal_api_helper import get_unreal_helper
helper = get_unreal_helper()
material = helper.create_material("M_Material", "/Game/Materials")
```

### Issue: "LoadBlueprintClass failed for C++ class"
**Solution:** Use `unreal.load_class()` for C++ classes, not `load_blueprint_class()`:
```python
# WRONG
cpp_class = unreal.EditorAssetLibrary.load_blueprint_class("/Script/MyModule.MyClass")

# CORRECT
cpp_class = unreal.load_class(None, "/Script/MyModule.MyClass")
```

---

## 6. API Migration Reference

### Deprecated vs New APIs

| Deprecated (DON'T USE) | New (USE THIS) |
|------------------------|----------------|
| `unreal.EditorLevelLibrary.get_editor_world()` | `unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).get_current_level()` |
| `unreal.EditorLevelLibrary.spawn_actor_from_class()` | `unreal.get_editor_subsystem(unreal.EditorActorSubsystem).spawn_actor_from_class()` |
| `unreal.EditorAssetLibrary.load_asset()` | `unreal.get_editor_subsystem(unreal.EditorAssetSubsystem).load_asset()` |
| `unreal.MaterialEditingLibrary.create_material()` | Use `MaterialFactoryNew()` with `AssetTools` |
| `unreal.EditorAssetLibrary.load_blueprint_class()` (for C++) | `unreal.load_class(None, "/Script/...")` |

---

## 7. Quick API Examples

### Spawn C++ Actor
```python
from unreal_api_helper import get_unreal_helper
helper = get_unreal_helper()

# Spawn Sun (C++ class)
sun = helper.spawn_cpp_actor(
    "/Script/Alexander.Sun",
    unreal.Vector(0, 0, 0)
)
sun.set_actor_label("Main_Sun")
```

### Create Material
```python
from unreal_api_helper import get_unreal_helper
helper = get_unreal_helper()

# Create material
material = helper.create_material(
    "M_SpaceShip",
    "/Game/SpaceShip/Materials"
)
```

### Load Assets
```python
from unreal_api_helper import get_unreal_helper
helper = get_unreal_helper()

# Check if asset exists
exists = helper.does_asset_exist("/Game/MyAsset")

# Load asset
asset = helper.load_asset("/Engine/BasicShapes/Cube")
```

### Create Directory
```python
from unreal_api_helper import get_unreal_helper
helper = get_unreal_helper()

# Create directory
helper.make_directory("/Game/MyNewFolder")
```

---

## 8. Troubleshooting Checklist

- [ ] Python dependencies installed (`pip install -r requirements.txt`)
- [ ] Unreal Editor is running
- [ ] Python console accessible in Unreal
- [ ] `unreal_api_helper.py` exists in `Content/Python/`
- [ ] Output Log shows no import errors
- [ ] Test material creation works
- [ ] Test C++ class spawning works

---

## 9. Next Steps

Once setup is verified:

1. ✅ Test `space_farming_mechanics.py`
2. ✅ Test `setup_solar_system.py`
3. ✅ Enable phase scripts in `workflow_config.json`
4. ✅ Run AI-driven development workflow

---

## 10. Support

### Useful Resources

- **Unreal Python API Docs:** https://docs.unrealengine.com/5.5/en-US/PythonAPI/
- **Python in Unreal:** https://docs.unrealengine.com/5.5/en-US/scripting-the-unreal-editor-using-python/

### Log Files

Check these for errors:
- Unreal Output Log (in editor)
- `Saved/Logs/` directory

### Quick Test Script

Save as `test_setup.py` and run in Unreal Python Console:

```python
"""Quick integration test"""
import unreal

print("="*60)
print("ALEXANDER PYTHON INTEGRATION TEST")
print("="*60)

# Test 1: Import helper
try:
    from unreal_api_helper import get_unreal_helper
    helper = get_unreal_helper()
    print("✅ API Helper loaded")
except Exception as e:
    print(f"❌ API Helper failed: {e}")

# Test 2: Material creation
try:
    mat = helper.create_material("M_IntegrationTest", "/Game/Test")
    print(f"✅ Material creation works: {mat}")
except Exception as e:
    print(f"❌ Material creation failed: {e}")

# Test 3: Actor spawning
try:
    actor = helper.spawn_cpp_actor(
        "/Script/Engine.StaticMeshActor",
        unreal.Vector(0, 0, 100)
    )
    print(f"✅ Actor spawning works: {actor}")
    if actor:
        helper.destroy_actor(actor)
except Exception as e:
    print(f"❌ Actor spawning failed: {e}")

print("="*60)
print("INTEGRATION TEST COMPLETE")
print("="*60)
```

---

**Last Updated:** 2025-11-04
**Status:** Ready for Production
