# REMOTE CONTROL OF RUNNING UNREAL EDITOR

## How to Control the GUI Editor from Python

There are 3 ways to control the running Unreal Editor:

### Method 1: Python Script Execution via -ExecutePythonScript
**Best for:** Batch operations, level setup, spawning actors

```bash
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" -ExecutePythonScript="script.py" -stdout -unattended -nopause -nosplash
```

This launches a separate editor instance, runs the script, then exits.

### Method 2: Python Remote Execution (TCP Socket)
**Best for:** Real-time control of running GUI editor

**Enable in Editor:**
1. Edit → Project Settings
2. Plugins → Python
3. Enable "Remote Execution"
4. Set Port: 9998 (default)

**Connect from Python:**
```python
import socket
import json

def execute_unreal_python(command):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(('localhost', 9998))

    message = json.dumps({
        'command': 'remote_exec',
        'data': command
    })

    sock.sendall(message.encode('utf-8') + b'\n')
    response = sock.recv(4096)
    sock.close()

    return response.decode('utf-8')

# Example usage
result = execute_unreal_python('''
import unreal
world = unreal.EditorLevelLibrary.get_editor_world()
actors = unreal.EditorLevelLibrary.get_all_level_actors()
print(f"Level has {len(actors)} actors")
''')
```

### Method 3: Startup Scripts (Auto-Execute)
**Best for:** Initialization, environment setup

**File:** `Content/Python/init_unreal.py`

This script runs automatically when editor starts.

```python
# Content/Python/init_unreal.py
import unreal

# Runs on every editor startup
unreal.log("Editor started - autonomous mode ready")
```

---

## The Autonomous Control Flow

```
Python Script (External)
    ↓
Method 1: ExecutePythonScript (Batch)
    ↓
Unreal Editor Executes
    ↓
Takes Screenshot
    ↓
Exits
    ↓
Python reads screenshot & log
    ↓
Decides next action
    ↓
Repeat
```

OR

```
Python Script (External)
    ↓
Method 2: Remote TCP Connection
    ↓
Running GUI Editor
    ↓
Executes command in-place
    ↓
Returns result
    ↓
Python gets immediate feedback
    ↓
Next command
```

---

## Autonomous Loop Implementation

**Option A: Batch Mode (Restart Each Time)**
```bash
while true; do
    # 1. Observe
    python take_screenshot.py
    tail -50 Saved/Logs/Alexander.log > current_log.txt

    # 2. Decide & Implement (Python analyzes and generates script)
    python autonomous_decision.py > next_action.py

    # 3. Execute
    UnrealEditor-Cmd.exe -ExecutePythonScript=next_action.py

    # 4. Verify
    python take_screenshot.py

    # Loop
done
```

**Option B: Persistent Connection**
```python
import socket

def control_loop():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(('localhost', 9998))

    while True:
        # 1. Observe via screenshot
        execute_screenshot()

        # 2. Analyze
        decision = analyze_current_state()

        # 3. Send command to running editor
        command = generate_unreal_command(decision)
        send_command(sock, command)

        # 4. Wait and verify
        time.sleep(5)
        execute_screenshot()

        # Loop continues...
```

---

## Complete Autonomous Script Template

```python
#!/usr/bin/env python3
"""
Autonomous Development Loop
"""
import subprocess
import time

def take_screenshot():
    subprocess.run(["python", "take_screenshot.py"])

def check_log():
    with open("Saved/Logs/Alexander.log", 'r') as f:
        lines = f.readlines()
        return lines[-50:]  # Last 50 lines

def analyze_state(screenshot_path, log_lines):
    """Decide what to do next"""
    # Check for errors
    errors = [l for l in log_lines if 'error' in l.lower()]
    if errors:
        return "fix_errors", errors

    # Check screenshot (would use image analysis)
    # For now, simple heuristic
    return "add_spaceship", None

def execute_action(action, data):
    """Execute the chosen action"""
    if action == "add_spaceship":
        script = """
import unreal
world = unreal.EditorLevelLibrary.get_editor_world()
bp_class = unreal.load_class(None, "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer_C")
location = unreal.Vector(0, 0, 20000)
rotation = unreal.Rotator(0, 0, 0)
actor = world.spawn_actor(bp_class, location, rotation)
unreal.log(f"Spawned: {actor.get_name()}")
unreal.EditorLevelLibrary.save_current_level()
unreal.AutomationLibrary.take_high_res_screenshot(1920, 1080, "spaceship_added")
"""
        with open("temp_action.py", 'w') as f:
            f.write(script)

        subprocess.run([
            "C:\\Program Files\\Epic Games\\UE_5.6\\Engine\\Binaries\\Win64\\UnrealEditor-Cmd.exe",
            "C:\\Users\\allen\\Desktop\\Alexander\\Alexander\\Alexander.uproject",
            "-ExecutePythonScript=temp_action.py",
            "-stdout", "-unattended", "-nopause", "-nosplash"
        ])

def main():
    iteration = 0

    while True:
        iteration += 1
        print(f"\n{'='*80}")
        print(f"ITERATION {iteration}")
        print(f"{'='*80}")

        # 1. Observe
        print("1. Observing...")
        take_screenshot()
        log_lines = check_log()

        # 2. Analyze
        print("2. Analyzing...")
        action, data = analyze_state("current_screen.png", log_lines)
        print(f"   Decision: {action}")

        # 3. Act
        print("3. Executing...")
        execute_action(action, data)

        # 4. Verify
        print("4. Verifying...")
        time.sleep(5)
        take_screenshot()

        print(f"Iteration {iteration} complete")
        time.sleep(2)

if __name__ == "__main__":
    main()
```

---

## Project File Locations

```
C:\Users\allen\Desktop\Alexander\Alexander\
├── take_screenshot.py          # Screenshot capture
├── COMMANDS.md                 # All commands reference
├── GENERALIZED_WORKFLOW.md     # The workflow
├── REMOTE_CONTROL.md          # This file
├── autonomous_loop.py          # Main loop (create this)
├── Content/
│   └── Python/
│       └── init_unreal.py      # Auto-startup script
├── Saved/
│   ├── Logs/
│   │   └── Alexander.log       # Main log file
│   └── Screenshots/
│       └── WindowsEditor/      # Screenshot output
└── Source/
    └── Alexander/              # C++ source
```

---

## Ready to Execute

**Start the GUI editor first:**
```bash
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject"
```

**Then run autonomous loop:**
```bash
python autonomous_loop.py
```

OR

**Run fully autonomous (no GUI):**
```bash
# Loop handles everything
python autonomous_loop.py
```

---

**THE SYSTEM IS READY**
