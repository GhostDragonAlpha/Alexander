"""
Launch Standalone Game with PID Tracking
Starts game and records PID for targeted control
"""

import subprocess
import time
import json
from pathlib import Path

# Configuration
UNREAL_EXE = r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe"
PROJECT_FILE = r"C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject"
MAP_NAME = "VRTemplateMap"
PID_FILE = Path("running_game_pid.json")

def launch_game():
    """Launch standalone game and record PID"""

    print("=" * 70)
    print("LAUNCHING GAME WITH PID TRACKING")
    print("=" * 70)
    print()

    # Build command
    # Note: ?game= URL parameter forces the game mode, overriding level settings
    cmd = [
        UNREAL_EXE,
        PROJECT_FILE,
        MAP_NAME + "?game=/Script/Alexander.AutomationGameMode",
        "-game",
        "-windowed",
        "-ResX=1280",
        "-ResY=720",
        "-log"
    ]

    print(f"[INFO] Starting: {UNREAL_EXE}")
    print(f"[INFO] Project: {PROJECT_FILE}")
    print(f"[INFO] Map: {MAP_NAME}")
    print()

    # Launch process
    process = subprocess.Popen(cmd)
    pid = process.pid

    print(f"[OK] Game launched!")
    print(f"[OK] Process ID (PID): {pid}")
    print()

    # Save PID to file
    pid_data = {
        "pid": pid,
        "exe": UNREAL_EXE,
        "project": PROJECT_FILE,
        "map": MAP_NAME,
        "launched_at": time.strftime("%Y-%m-%d %H:%M:%S")
    }

    with open(PID_FILE, 'w') as f:
        json.dump(pid_data, f, indent=2)

    print(f"[OK] PID saved to: {PID_FILE}")
    print()
    print("=" * 70)
    print("GAME STARTING...")
    print("=" * 70)
    print()
    print("Wait 15-20 seconds for game to fully load")
    print("Then check windowed game instance")
    print()

    return pid

def get_running_pid():
    """Get PID of currently running game"""
    if PID_FILE.exists():
        with open(PID_FILE, 'r') as f:
            data = json.load(f)
        return data
    return None

if __name__ == "__main__":
    # Check if already running
    existing = get_running_pid()
    if existing:
        print(f"[WARN] Game may already be running (PID: {existing['pid']})")
        print(f"[WARN] Launched at: {existing['launched_at']}")
        response = input("Launch anyway? (y/N): ")
        if response.lower() != 'y':
            print("[INFO] Cancelled")
            exit(0)

    pid = launch_game()
    print(f"Game PID: {pid}")
    print(f"Track this PID for all remote commands")
