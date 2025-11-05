# 💥 Editor Crash Recovery Guide

**IMPORTANT:** Editor crashes are NORMAL during development - handle automatically!

---

## Commands That Crash Unreal Editor

### ❌ NEVER Use Via Remote Execution:
- `BuildLighting` - Full lightmap bake, extremely heavy
- `BuildLighting Quality` - Any quality level
- Large batch operations on 100+ actors
- Swarm-related commands

### ⚠️ Use With Caution:
- `RecompileShaders Changed` - Can timeout
- Level switching while build in progress
- Massive scene creation scripts

### ✅ Safe Commands:
- `Editor.RedrawAllViewports` - Viewport refresh only
- `RerunConstructionScripts` - Actor refresh
- `r.SetRes 1920x1080` - Resolution changes
- Screenshot commands (`HighResShot`)

---

## Crash Recovery Workflow

### 1. Detect Crash
**Signs:**
- `[WinError 10054] Connection forcibly closed by remote host`
- Connection test fails repeatedly
- No UnrealEditor process in task list

### 2. Automatic Restart
```bash
./APPLY_CHANGES_NO_PAUSE.bat
```

**What it does:**
- Force closes any stuck editor instances
- Clears autosave/cache files
- Restarts editor cleanly
- Takes 45-60 seconds total

### 3. Wait for Full Load
- **First 10-15s:** Process startup
- **15-45s:** Shader compilation (if needed)
- **45-60s:** Python Remote Execution server starts

### 4. CHECK LOG FIRST! (CRITICAL)
```bash
tail -50 Alexander/Saved/Logs/Alexander.log | grep -E "(Error|Warning|Fatal|Failed|Python|Initialization)" -i
```

**What to look for:**
- ✅ "Shader compilation completed" - Editor fully loaded
- ✅ "LogPython:" messages - Scripts executed
- ❌ "Error" or "Fatal" - Startup failed
- ❌ "Warning" repeated many times - Potential issues

**Example good log:**
```
LogShaderCompilers: Jobs completed 8,910 (100%)
LogPython: Remote execution working
```

**If errors found:**
- Document the error
- Check if related to recent changes
- May need clean slate restart

### 5. Test Connection
```bash
python remote_client.py --test
```

**Expected output:**
```
Success: True
Output: Remote execution working!
```

**If connection fails:**
- Wait 15 more seconds (shaders may still be compiling)
- Check log again for errors
- Verify editor process exists: `tasklist | findstr Unreal`

### 6. Resume Work
- Reload your level if needed
- Re-run your last successful script
- Continue from where you left off

---

## Prevention Strategies

### Use Movable Lights
✅ **Movable lights work immediately - NO building required!**
- Point lights
- Spot lights
- Directional lights (set to Movable)
- Real-time updates

❌ **Static/Stationary lights require building:**
- Must use Build menu in Unreal UI
- Cannot build safely via remote execution
- Use for final baking only

### Safe Lighting Workflow
```python
# ✅ SAFE - Movable lights work instantly
star_light = unreal.PointLight(...)
star_comp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
star_comp.set_editor_property("intensity", 500000.0)

# Just refresh viewport
unreal.SystemLibrary.execute_console_command(None, "Editor.RedrawAllViewports")
```

### Manual Building (When Needed)
1. Open Unreal Editor manually
2. Build → Build Lighting Quality → Preview/Medium
3. Wait for Swarm Agent to complete
4. Save level
5. Resume remote work

---

## Crash Log Analysis

### Check Recent Logs
```bash
tail -100 Alexander/Saved/Logs/Alexander.log | grep -i "error\|crash\|fatal"
```

### Common Crash Causes
1. **Out of memory:** Too many actors/lights
2. **Shader compilation timeout:** Old cache
3. **Build system conflict:** Swarm couldn't initialize
4. **Plugin crash:** Third-party plugins

---

## Integration with Workflow

### Update WORKFLOW_STATUS.md After Crash
- Mark crashed task as "blocked"
- Document crash cause
- Add recovery steps taken
- Note safer alternative approach

### Document in Git Commits
```bash
git commit -m "fix: Use movable lights instead of BuildLighting

BuildLighting command crashes via remote execution.
Switched to movable lights which work immediately.
Added crash recovery documentation.

Related: Crash recovery workflow"
```

---

## Emergency: Editor Won't Start

### If restart fails repeatedly:

1. **Check for stuck processes:**
```bash
tasklist | findstr Unreal
taskkill /F /IM UnrealEditor.exe
```

2. **Clear all caches:**
```bash
cmd.exe /c "START_CLEAN_SLATE.bat"
```

3. **Verify project integrity:**
- Check `.uproject` file exists
- Verify Engine installation
- Test on blank project

4. **Last resort - Manual start:**
- Double-click Alexander.uproject
- Wait for full load
- Test remote connection manually

---

## Success Metrics

**Good crash recovery:**
- Detected crash within 10 seconds
- Restarted automatically
- Reconnected within 60 seconds
- Continued work with safer approach

**Learn from crashes:**
- Each crash = feedback about command limits
- Update documentation with crash causes
- Add safer alternatives
- Prevent future crashes

---

**Remember:** Crashes aren't failures - they're boundaries. Work within them!
