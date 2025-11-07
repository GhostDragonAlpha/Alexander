# IMPROVED WORKFLOW - Working with Running Editor

## The Core Principle

**Keep the Unreal Editor RUNNING. Control it remotely. Don't restart constantly.**

---

## One-Time Setup (Do This Once)

### 1. Enable Remote Execution

**In Unreal Editor:**
1. Edit → Project Settings
2. Search "Python"
3. Under "Python" section:
   - ✓ Enable Remote Execution
   - Multicast Group: `239.0.0.1`
   - Multicast Port: `6766`
4. Restart editor (one last time!)

---

## The Daily Workflow

### Step 1: Start Editor (Once Per Day)

```bash
python start_editor_once.py
```

**What this does:**
- Checks if editor is already running
- If not, starts it
- Waits for full load
- Leaves it RUNNING

**Result:** Unreal Editor GUI is open on your screen

---

### Step 2: See What's Currently Visible

```bash
python take_editor_screenshot.py
```

**What this does:**
- Captures the actual editor window
- Shows you what's on screen right now
- Saves to Screenshots/ folder

**Use this:** Before and after every change to verify results

---

### Step 3: Make Changes (Without Restarting!)

**Test remote connection:**
```bash
python remote_client.py --test
```

**Execute a Python script in running editor:**
```bash
python remote_client.py --file "your_script.py"
```

**Send a single command:**
```bash
python remote_client.py --command "import unreal; unreal.log('Hello')"
```

**The editor stays RUNNING through all of this!**

---

### Step 4: Verify Changes

```bash
python take_editor_screenshot.py
```

Compare before/after screenshots to see what changed.

---

### Step 5: Iterate (Editor Still Running!)

```
1. Take screenshot (see current state)
2. Send remote command (make change)
3. Wait 2-5 seconds
4. Take screenshot (see new state)
5. Repeat!
```

**The editor NEVER closes!**

---

## Example: Fix Ship Flight

### Current Task: Get ship flying in VRTemplateMap

```bash
# 1. Start editor (once)
python start_editor_once.py

# 2. See current state
python take_editor_screenshot.py

# 3. Apply GameMode fix to spawn ship
python remote_client.py --file "fix_gamemode_for_flight.py"

# 4. Wait for processing
# (watch editor screen for 5 seconds)

# 5. Check result
python take_editor_screenshot.py

# 6. If ship visible in viewport → Test PIE
python remote_client.py --file "test_pie_flight.py"

# 7. Verify PIE works
python take_editor_screenshot.py

# Done! Ship is now flyable.
```

**Note:** Editor stayed open the ENTIRE TIME!

---

## When to Restart Editor

**Only restart if:**
- ❌ After C++ code changes (need recompile)
- ❌ After crash
- ❌ Editor becomes unresponsive
- ❌ After enabling Remote Execution for first time

**Don't restart for:**
- ✓ Python script changes
- ✓ Blueprint edits
- ✓ Material changes
- ✓ Adding actors to level
- ✓ Lighting changes

---

## Controlling PIE (Play In Editor) from Python

### The PIE Workflow - NEVER Kill Processes!

**IMPORTANT:** Do NOT kill UnrealEditor processes when testing. Control PIE from within the running editor using Python commands.

### CRITICAL: Check for Compilation Errors FIRST

**Before attempting to start PIE, ALWAYS check the editor's Output Log for compilation errors.**

When the editor starts or when C++ code changes:
1. The editor attempts to compile the project modules
2. If compilation fails, the editor shows errors in the Output Log
3. **PIE will not work properly with compilation errors** - it may show the PIE window but game logic won't run
4. You MUST fix compilation errors before PIE will function

**How to check for compilation errors:**
```bash
# Check the editor logs for compilation failures
# Look for lines like:
#   "error LNK2019: unresolved external symbol"
#   "fatal error LNK1120: X unresolved externals"
#   "Incompatible or missing module: Alexander"
```

**Common compilation errors:**
- Missing module dependencies in Build.cs files (e.g., UMG, UnrealEd)
- Unresolved external symbols from missing #include or module links
- Blueprint-generated code referencing missing C++ symbols

**Fix workflow:**
1. Check logs for compilation errors
2. If errors exist, fix them (update Build.cs, add includes, etc.)
3. Let editor automatically recompile, or close and reopen editor
4. Verify "Module loaded successfully" in logs
5. ONLY THEN attempt to start PIE

### Start PIE Session

```python
import unreal

# Start Play In Editor
unreal.EditorLevelLibrary.editor_play_simulate()
```

**Or via remote:**
```bash
python remote_client.py --command "import unreal; unreal.EditorLevelLibrary.editor_play_simulate()"
```

### Stop PIE Session

```python
import unreal

# Stop Play In Editor
unreal.EditorLevelLibrary.stop_play_session()
```

**Or via remote:**
```bash
python remote_client.py --command "import unreal; unreal.EditorLevelLibrary.stop_play_session()"
```

### Check PIE State

```python
import unreal

# Check if PIE is currently running
is_playing = unreal.EditorLevelLibrary.is_playing()
print(f"PIE Running: {is_playing}")
```

### Complete PIE Testing Workflow

```bash
# 1. Editor is already running (launched once)

# 2. Start PIE to test AutomationGameMode
python remote_client.py --command "import unreal; unreal.EditorLevelLibrary.editor_play_simulate()"

# 3. Wait for PIE to fully start (5-10 seconds)
# Watch the editor Output Log for:
#   "AutomationGameMode: Server started"
#   "AutomationAPI: TCP server listening on port 8080"

# 4. Test the automation server endpoints
python test_automation_server.py

# 5. Stop PIE when done
python remote_client.py --command "import unreal; unreal.EditorLevelLibrary.stop_play_session()"

# 6. Make changes if needed, then repeat steps 2-5
```

### Iteration Example: Test → Fix → Test

```bash
# Test current state
python remote_client.py --command "import unreal; unreal.EditorLevelLibrary.editor_play_simulate()"
# (wait for start)
python test_automation_server.py
# (test fails)
python remote_client.py --command "import unreal; unreal.EditorLevelLibrary.stop_play_session()"

# Fix the code in your editor/IDE
# (make changes to C++ or Python files)

# Recompile if C++ changed
python remote_client.py --file "recompile_module.py"

# Test again
python remote_client.py --command "import unreal; unreal.EditorLevelLibrary.editor_play_simulate()"
# (wait for start)
python test_automation_server.py
# (test passes!)
python remote_client.py --command "import unreal; unreal.EditorLevelLibrary.stop_play_session()"

# Editor NEVER closed during this entire process!
```

### Key Rules for PIE Testing

1. **NEVER** use `taskkill` or `Stop-Process` on UnrealEditor
2. **ALWAYS** use Python commands to start/stop PIE
3. **ALWAYS** wait 5-10 seconds after starting PIE before testing
4. **ALWAYS** check the Output Log to verify game mode and server started
5. **Editor stays RUNNING** through all PIE sessions

### Common PIE Mistakes (DON'T DO THESE)

```bash
# ❌ WRONG - Killing editor process
taskkill /IM UnrealEditor.exe /F

# ❌ WRONG - Launching standalone game
UnrealEditor-Cmd.exe ... -game

# ❌ WRONG - Closing and relaunching editor
# (Editor should stay open!)

# ✓ CORRECT - Using PIE commands
python remote_client.py --command "import unreal; unreal.EditorLevelLibrary.editor_play_simulate()"
```

### Troubleshooting PIE

**PIE won't start?**
- Check Output Log for errors
- Verify GameMode is set correctly in DefaultEngine.ini
- Check that level is loaded in editor

**Automation server returns 404?**
- Don't use `-HTTPPort` flag (causes port conflicts)
- Verify AutomationGameMode is set as default in Config/DefaultEngine.ini
- Check Output Log shows "AutomationAPI: TCP server listening on port 8080"

**PIE seems stuck?**
- Stop PIE: `python remote_client.py --command "import unreal; unreal.EditorLevelLibrary.stop_play_session()"`
- Wait 5 seconds
- Start PIE again

---

## Task Queue Processing

### Follow execution_queue.json automatically:

```python
# Process tasks in order, editor stays running
for task in task_queue:
    # 1. Screenshot before
    take_screenshot()

    # 2. Execute task script via remote
    remote_client.execute(task.script)

    # 3. Wait for completion
    time.sleep(task.estimated_duration)

    # 4. Screenshot after
    take_screenshot()

    # 5. Update task status
    task.status = "completed"
    save_queue()

    # Next task (editor still running!)
```

---

## Comparison: Old vs New

### ❌ OLD WAY (Wrong)
```bash
# Launch editor
UnrealEditor-Cmd.exe ... script1.py  # Takes 60 seconds
# Editor closes

# Launch editor again
UnrealEditor-Cmd.exe ... script2.py  # Takes 60 seconds
# Editor closes

# Launch editor again
UnrealEditor-Cmd.exe ... script3.py  # Takes 60 seconds
# Editor closes

# Total: 3 minutes, 3 restarts
```

### ✓ NEW WAY (Correct)
```bash
# Start editor ONCE
python start_editor_once.py  # Takes 60 seconds

# Execute all scripts in running editor
python remote_client.py --file script1.py  # Takes 2 seconds
python remote_client.py --file script2.py  # Takes 2 seconds
python remote_client.py --file script3.py  # Takes 2 seconds

# Editor STILL RUNNING
# Total: 66 seconds, 1 start, 0 restarts
```

**Speedup: 3x faster!**

---

## Files Reference

### Workflow Control
- `start_editor_once.py` - Start/check editor
- `remote_client.py` - Send commands to running editor
- `take_editor_screenshot.py` - Capture GUI state

### One-Time Setup
- `enable_remote_execution.py` - Setup guide

### Task Scripts (Send via remote_client.py)
- `fix_gamemode_for_flight.py` - Make ship spawn
- `test_pie_flight.py` - Test Play In Editor
- `setup_dual_lighting.py` - Fix lighting
- `trigger_lighting_build.py` - Rebuild lighting
- (Any Python script can run in editor)

---

## Troubleshooting

### Remote commands not working?
1. Check editor is running: `tasklist | findstr UnrealEditor`
2. Check remote execution enabled (see setup above)
3. Check firewall not blocking UDP 6766
4. Restart editor once after enabling remote execution

### Screenshot shows wrong window?
1. Make sure Unreal Editor is the active/focused window
2. Maximize the editor window
3. Try again

### Editor crashed?
1. That's the ONLY time you restart it
2. Run `python start_editor_once.py` again
3. Continue where you left off

---

## Summary

**The workflow is NOT:**
```
Launch editor → Do thing → Close editor → Launch editor → Do thing → Close editor
```

**The workflow IS:**
```
Launch editor ONCE → Do thing → Do thing → Do thing → Do thing → Eventually close
```

**Simple as that.**
