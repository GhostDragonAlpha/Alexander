# ⚠️ CRITICAL: WORKFLOW ORDER FOR UNREAL ENGINE CHANGES

## 🚨 THE GOLDEN RULE

**ALWAYS TRIGGER BUILD AFTER ANY SIGNIFICANT CHANGES BEFORE TESTING**

```
1. Make changes (lighting, materials, geometry, etc.)
2. ⚠️ TRIGGER BUILD ⚠️
3. Wait for build completion
4. Then test/capture screenshots
```

**NEVER skip step 2!** Changes won't be visible without rebuilding.

---

## 📋 Change Types That REQUIRE Building

### Lighting Changes
- Creating new lights
- Modifying light intensity
- Changing light mobility (Static/Stationary/Movable)
- Moving lights
- Deleting lights

**Build Command:** `python remote_client.py --file "trigger_lighting_build.py"`

### Material Changes
- Assigning new materials
- Changing material properties
- Creating new materials

**Build Command:** `python remote_client.py --file "trigger_lighting_build.py"`

### Geometry Changes
- Adding new static meshes
- Modifying mesh scales
- Moving objects significantly
- Deleting objects

**Build Command:** `python remote_client.py --file "trigger_lighting_build.py"`

---

## ✅ CORRECT WORKFLOW EXAMPLE

```bash
# 1. Make lighting changes
python remote_client.py --file "setup_dual_lighting.py"

# 2. ⚠️ TRIGGER BUILD (CRITICAL!)
python remote_client.py --file "trigger_lighting_build.py"

# 3. Wait 10-30 seconds for build

# 4. Test/capture
python remote_client.py --file "capture_arena_views.py"
```

---

## ❌ INCORRECT WORKFLOW (Will Fail)

```bash
# 1. Make lighting changes
python remote_client.py --file "setup_dual_lighting.py"

# 2. ❌ NO BUILD TRIGGERED!

# 3. Test immediately (will show old/black scene)
python remote_client.py --file "capture_arena_views.py"  # ❌ FAILS!
```

---

## 🔧 Build Commands Reference

### Full Lighting + Shader Rebuild
```python
# Use: trigger_lighting_build.py
Commands executed:
  - RecompileShaders Changed
  - r.InvalidateCachedShaders 1
  - BuildLighting
  - RerunConstructionScripts
  - RefreshAllNodes
  - Editor.RedrawAllViewports
```

### Quick Viewport Refresh (Minor Changes)
```python
# For small tweaks that don't need full rebuild
Commands:
  - Editor.RedrawAllViewports
  - RerunConstructionScripts
```

---

## 🎯 Common Scenarios

### Scenario 1: Adding New Lights
```bash
# Create lights
python remote_client.py --file "create_new_lights.py"

# ⚠️ MUST BUILD!
python remote_client.py --file "trigger_lighting_build.py"

# Wait...

# Test
python remote_client.py --file "capture_screenshots.py"
```

### Scenario 2: Changing Light Mobility
```bash
# Change skylight to static
python remote_client.py --file "set_skylight_static.py"

# ⚠️ MUST BUILD!
python remote_client.py --file "trigger_lighting_build.py"

# Wait...

# Test
python remote_client.py --file "capture_screenshots.py"
```

### Scenario 3: Fixing Materials
```bash
# Assign materials
python remote_client.py --file "check_materials.py"

# ⚠️ MUST BUILD!
python remote_client.py --file "trigger_lighting_build.py"

# Wait...

# Test
python remote_client.py --file "capture_screenshots.py"
```

---

## ⏱️ Build Times

| Build Type | Typical Duration | When to Use |
|------------|------------------|-------------|
| **Full Lighting Build** | 30-120 seconds | After lighting changes |
| **Shader Recompile** | 10-30 seconds | After material changes |
| **Viewport Refresh** | 1-5 seconds | After minor tweaks |

---

## 🚨 WARNING SIGNS You Forgot to Build

- ❌ Screenshots are completely black
- ❌ Changes not visible in viewport
- ❌ "Too many overlapping movable lights" error persists
- ❌ Materials look wrong/black
- ❌ Objects invisible or flickering

**Solution:** Stop, run `trigger_lighting_build.py`, wait, then try again!

---

## 🔄 Reconnection After Build/Connection Loss

**If remote connection is lost:**

1. **Restart Unreal Editor** (required for reconnection)
   ```bash
   # Simple restart via batch file
   ./APPLY_CHANGES_NO_PAUSE.bat
   ```

2. **Wait 30-60 seconds** for editor to fully load

3. **Test connection**
   ```bash
   python remote_client.py --test
   ```

4. **Continue workflow** once reconnected

**Why connection drops:**
- Build commands can trigger editor refresh
- Heavy operations timeout connection
- Editor restart required to re-establish remote execution

**Solution:** Simply restart editor - it's part of normal workflow!

## 💡 Pro Tips

1. **Build once, test many**: After building, you can capture multiple screenshots without rebuilding
2. **Wait for completion**: Don't capture screenshots while build is running
3. **Check logs**: Build completion shows in Unreal Editor's output log
4. **Restart is normal**: Connection drops during builds are expected - just restart editor
5. **Patience**: Wait for full editor load before testing connection

---

## 📝 Checklist Before Every Test

- [ ] Made changes to scene?
- [ ] Ran `trigger_lighting_build.py`?
- [ ] Waited 10-30 seconds?
- [ ] Checked Unreal Editor is still connected?
- [ ] NOW capture screenshots/test

**Follow this order EVERY TIME!**

---

## 🎓 Why This Matters

Unreal Engine uses **deferred rendering**:
- Changes are queued, not applied immediately
- Build process compiles shaders, recalculates lighting, updates scene
- Without building, you're testing the OLD scene state
- This is **by design** in Unreal Engine - not a bug!

---

## ✅ Remember

> **The workflow is NOT:**
> Change → Test
>
> **The workflow IS:**
> Change → **Build** → Wait → Test

**Make this muscle memory!**
