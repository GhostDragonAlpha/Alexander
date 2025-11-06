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

**Option C: Remote execution (if enabled)**
```python
# Enable first: Edit → Project Settings → Python → Remote Execution
import socket
sock = socket.socket()
sock.connect(('localhost', 9998))
sock.send(b'import unreal; unreal.log("test")\n')
```

### 4. When to Restart Editor

**Close running editor:**
```bash
taskkill /IM UnrealEditor.exe /F
```

**Start editor again:**
```bash
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" "Alexander.uproject"
```

**When to do this:**
- After editing C++ and compiling
- After crash
- When editor becomes unresponsive
- NOT every iteration

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
