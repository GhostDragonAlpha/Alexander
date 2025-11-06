# THE REAL UNREAL ENGINE WORKFLOW

## How Developers Actually Work with Unreal

### The Editor is RUNNING
- Unreal Editor GUI is open
- You're looking at the viewport
- You make changes IN THE EDITOR
- You see results IMMEDIATELY
- You DON'T restart constantly

### When You Restart
- After C++ code changes (need recompile)
- After crashes
- After major settings changes
- Maybe once per session otherwise

### The Actual Loop

```
Editor is RUNNING
    ↓
Take screenshot (see what's visible)
    ↓
Execute Python command IN running editor
    OR
    Edit asset in Content Browser
    OR
    Move actors in viewport
    ↓
Take screenshot (see the change)
    ↓
Is it good?
    YES → Next task
    NO → Undo and try different approach
    ↓
Keep working...
    ↓
Eventually: Save level
    ↓
Maybe: Restart if needed
    ↓
Continue...
```

## The Commands That Actually Matter

### 1. Take Screenshot of GUI Editor
```bash
python take_screenshot.py
```
This captures what YOU (the developer) see in the Unreal Editor window.

### 2. Check if Editor is Running
```bash
ps aux | grep UnrealEditor.exe
# OR
tasklist | findstr UnrealEditor
```

### 3. Send Python Command to RUNNING Editor

**Option A: Via Python module in editor**
- Content/Python scripts run automatically
- Or execute via editor console

**Option B: Write to project, editor auto-detects**
- Save Blueprint
- Editor hot-reloads

**Option C: Remote execution via UDP multicast (CURRENT METHOD)**
```bash
# Enable first: Edit → Project Settings → Python → Remote Execution
# Then use remote_client.py to send commands to running editor/game

# Test connection:
python remote_client.py --test

# Execute command:
python remote_client.py --command "import unreal; unreal.log('Hello from remote!')"

# Execute entire file:
python remote_client.py --file my_script.py
```

**JSON Protocol Format (discovered through testing):**
```json
{
    "magic": "ue_py",
    "version": 1,
    "type": "command",
    "source": "remote",
    "command": "<python_code_here>",
    "unattended": true
}
```
Sent via UDP multicast to 239.0.0.1:6766

### 4. Game/Editor Lifecycle Management

**Close running game/editor (PROPER METHOD):**
```bash
# Send quit command via remote execution:
python remote_client.py --command "import unreal; unreal.get_engine().request_exit(False)"

# Wait a few seconds for graceful shutdown
```

**Close running game/editor (FORCE METHOD - use only if above fails):**
```bash
taskkill /IM UnrealEditor.exe /F
```

**Start editor (GUI mode):**
```bash
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject"
```

**Start game (standalone for testing):**
```bash
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" VRTemplateMap -game -windowed -ResX=1280 -ResY=720
```

**When to restart:**
- After editing C++ and compiling
- After crash
- When editor becomes unresponsive
- When switching between editor and standalone game mode
- NOT every iteration for asset/Blueprint changes

## What I Was Doing Wrong

❌ Launching UnrealEditor-Cmd.exe repeatedly
- This creates NEW editor instances
- They don't show in GUI
- They take forever to start
- They compete with running editor

✓ Should work with EXISTING editor
- It's already open
- It's showing the level
- Just control it

## The Simplified Loop

```python
while True:
    # 1. See current state
    take_screenshot()

    # 2. Decide what to change
    # (Look at screenshot, decide)

    # 3. Make the change
    #    Option A: Write Python script to Content/Python, editor runs it
    #    Option B: Use remote connection to send command
    #    Option C: Programmatically edit .uasset files

    # 4. Wait for editor to process
    time.sleep(2)

    # 5. See result
    take_screenshot()

    # 6. Next iteration (editor still running)
```

## The Right Way to Control Running Editor

**Create action script in watched directory:**
```python
# Write to Content/Python/auto_action.py
with open('Content/Python/auto_action.py', 'w') as f:
    f.write('''
import unreal
world = unreal.EditorLevelLibrary.get_editor_world()
# Do something...
unreal.EditorLevelLibrary.save_current_level()
''')

# Editor picks it up automatically OR
# Trigger reload via console command
```

**Send command via editor console:**
```python
# If editor has console open:
# Type: py exec(open('Content/Python/auto_action.py').read())
```

## Bottom Line

**STOP launching new editor instances every iteration.**

**START working with the running GUI editor.**

It's literally that simple.

## Complete Autonomous Testing Cycle (Current Implementation)

This is the VERIFIED workflow for autonomous testing with C++ code changes:

```bash
# STEP 1: Close running game properly
python remote_client.py --command "import unreal; unreal.get_engine().request_exit(False)"
# Wait 5 seconds for shutdown

# STEP 2: Verify C++ build is current
# Check if Source/ files were modified after Binaries/ were built
# Rebuild if needed

# STEP 3: Launch fresh game instance (windowed for visibility)
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" ^
  "C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" ^
  VRTemplateMap -game -windowed -ResX=1280 -ResY=720
# Wait 15 seconds for game to fully load

# STEP 4: Test remote connection
python remote_client.py --test
# Check game log to verify Python actually executed (not just "bytes sent")

# STEP 5: Run autonomous tests
python remote_client.py --file open_flight_test.py
# Verify with THREE proofs:
#   A. Windowed game (see it visually)
#   B. Screenshot capture
#   C. Log file with results

# STEP 6: Collect results and close
# Read Saved/Logs/Alexander.log for actual execution results
python remote_client.py --command "import unreal; unreal.get_engine().request_exit(False)"
```

**Critical Verification Rules:**
1. NEVER claim success based only on "bytes sent"
2. ALWAYS read game log to verify Python executed
3. ALWAYS capture three proofs: visual + screenshot + log file
4. If C++ code changed, MUST restart game to load new code

## CRITICAL DISCOVERY: Standalone Game Automation

**Problem**: Python Remote Execution (UDP multicast) ONLY works in EDITOR mode, NOT in standalone game (`-game` flag).

**Solution**: Custom HTTP Automation Server for standalone games.

### Two Control Methods:

**EDITOR Mode** (use Python Remote Execution):
```bash
# Works: UDP multicast to editor
python remote_client.py --command "import unreal; unreal.log('test')"
```

**STANDALONE Game Mode** (use HTTP Automation Server):
```bash
# Start game with AutomationGameMode:
UnrealEditor.exe Alexander.uproject VRTemplateMap?game=/Script/Alexander.AutomationGameMode -game -windowed

# Control via HTTP (port 8080):
python automation_http_client.py  # Spawn ship, set input, check velocity
```

### C++ Compilation Requirement

**CRITICAL**: AutomationGameMode and AutomationAPIServer are C++ classes that MUST be compiled before use.

**Files Created** (previous session):
- `Source/Alexander/Public/AutomationGameMode.h`
- `Source/Alexander/Private/AutomationGameMode.cpp`
- `Source/Alexander/Public/AutomationAPIServer.h`
- `Source/Alexander/Private/AutomationAPIServer.cpp`
- `Source/Alexander/Public/VRHandSimulator.h`
- `Source/Alexander/Private/VRHandSimulator.cpp`

**Build Command**:
```bash
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" ^
  Alexander Win64 Development ^
  "Alexander.uproject" -waitmutex
```

**Verification** (check game log after launch):
```
LogLoad: Game class is 'AutomationGameMode'  # SUCCESS
LogLoad: Warning: Failed to find object 'Class /Script/Alexander.AutomationGameMode'  # FAIL - not compiled
```

**Launch Command** (force AutomationGameMode):
```bash
python launch_game_tracked.py  # Now includes ?game=/Script/Alexander.AutomationGameMode parameter
```

Each launch creates new PID = new task/iteration for tracking.

---

## CRITICAL WORKFLOW ORDER (PID 72600, 81932, 89328 discoveries)

### DISCOVERED NOV 6 2025: Proper Build and Test Cycle

**1. MUST CLOSE GAME BEFORE BUILDING**
- Cannot build while game is running
- DLL will be locked by running process
- Use PID-based tracking to close correct instance

**2. CLOSE METHOD: Use Process Kill, Not Remote Execution**
- Remote Execution (`remote_client.py`) ONLY works in EDITOR mode
- Does NOT work with `-game` flag (standalone mode)
- **Correct close method**:
```python
python close_game_tracked.py  # Uses psutil.Process(PID).kill()
```

**3. BUILD CORRECT TARGET**
- **AlexanderEditor** target → creates `UnrealEditor-Alexander.dll` (used by -game launches)
- **Alexander** target → creates `Alexander.exe` (standalone executable, NOT used with -game flag)
- **For our workflow**: Always build AlexanderEditor to update the DLL

**Build command**:
```bash
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" ^
  AlexanderEditor Win64 Development "Alexander.uproject" -waitmutex
```

**4. VERIFY COMPILATION BY CHECKING DLL TIMESTAMP**
- Don't trust "Target is up to date" message
- Manually verify DLL is newer than source files:
```bash
ls -l Binaries/Win64/UnrealEditor-Alexander.dll
ls -l Source/Alexander/Private/AutomationGameMode.cpp
```
- DLL must be NEWER than source file

**5. HANDLE CRASH REPORTS LOCKING DLL**
- If game crashes, CrashReportClientEditor.exe may lock the DLL
- Build will fail with "cannot open file ... being used by another process"
- **Fix**: Kill CrashReportClientEditor.exe before rebuilding:
```bash
taskkill /F /IM CrashReportClientEditor.exe
```

**6. PID-BASED TASK TRACKING**
- Each game launch = new PID = new task/iteration
- Track PID to know which instance you're working with
- Examples from today:
  - **PID 104164**: Discovered C++ not compiled
  - **PID 72600**: Discovered source files newer than DLL
  - **PID 81932**: Discovered Asteroid RootComponent crash
  - **PID 89328**: Discovered wrong build target (Alexander vs AlexanderEditor)

**7. COMPLETE WORKFLOW CYCLE**
```bash
# 1. Close running game (if any)
python close_game_tracked.py

# 2. Make code changes (if needed)
# edit Source/Alexander/...

# 3. Kill crash reporter (if present)
taskkill /F /IM CrashReportClientEditor.exe 2>nul

# 4. Build AlexanderEditor target
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" ^
  AlexanderEditor Win64 Development "Alexander.uproject" -waitmutex

# 5. Verify DLL updated
ls -l Binaries/Win64/UnrealEditor-Alexander.dll

# 6. Launch new task with new PID
python launch_game_tracked.py  # Records new PID

# 7. Wait for load (15-20 seconds)
# Check windowed game instance

# 8. Verify in logs
grep "Game class is\|AutomationGameMode\|HTTP.*8080" Saved/Logs/Alexander.log

# 9. Test automation
python automation_http_client.py
```

**COMMON ERRORS AND FIXES**:
| Error | Cause | Fix |
|-------|-------|-----|
| "Target is up to date" but class not found | DLL not actually rebuilt | Check timestamps, force rebuild |
| "cannot open file ... in use" | CrashReportClient locking DLL | Kill CrashReportClientEditor.exe |
| "Default subobject already exists" | Constructor creating duplicate components | Remove duplicate, use parent's components |
| "Failed to find object 'Class /Script/Alexander.AutomationGameMode'" | C++ not compiled OR wrong DLL loaded | Build AlexanderEditor target, verify DLL timestamp |
| Remote execution quit doesn't work | Standalone mode doesn't support it | Use psutil.Process.kill() instead |
