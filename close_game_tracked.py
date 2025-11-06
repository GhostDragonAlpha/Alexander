"""
Close Tracked Game Instance
Uses PID to verify closure
"""

import json
import time
import psutil
from pathlib import Path
import remote_client

PID_FILE = Path("running_game_pid.json")

def find_alexander_game():
    """Find running Alexander game by process name and command line"""
    for proc in psutil.process_iter(['pid', 'name', 'cmdline']):
        try:
            if 'UnrealEditor' in proc.info['name']:
                cmdline = ' '.join(proc.info['cmdline']) if proc.info['cmdline'] else ''
                # Look for Alexander.uproject AND -game flag (standalone mode)
                if 'Alexander.uproject' in cmdline and '-game' in cmdline:
                    return {
                        'pid': proc.info['pid'],
                        'cmdline': cmdline,
                        'name': proc.info['name']
                    }
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            pass
    return None

def get_running_game():
    """Get info about running game from PID file"""
    if not PID_FILE.exists():
        return None
    with open(PID_FILE, 'r') as f:
        return json.load(f)

def is_process_running(pid):
    """Check if process with PID is still running"""
    try:
        process = psutil.Process(pid)
        return process.is_running()
    except (psutil.NoSuchProcess, psutil.AccessDenied):
        return False

def close_game():
    """Close the tracked game instance"""

    print("=" * 70)
    print("CLOSING ALEXANDER GAME")
    print("=" * 70)
    print()

    # First, try to find game by scanning processes
    print("[INFO] Searching for Alexander game process...")
    game_info = find_alexander_game()

    if not game_info:
        print("[INFO] Game not found by process scan, checking PID file...")
        game_info = get_running_game()

    if not game_info:
        print("[WARN] No Alexander game found running")
        print("[INFO] Checked for: UnrealEditor.exe with Alexander.uproject -game")
        return False

    pid = game_info['pid']
    print(f"[OK] Found Alexander game")
    print(f"[OK] PID: {pid}")
    print(f"[OK] Process: {game_info['name']}")
    print()

    # Check if still running
    if not is_process_running(pid):
        print(f"[INFO] Process {pid} is not running (already closed)")
        PID_FILE.unlink()
        print(f"[OK] Removed PID file")
        return True

    print(f"[OK] Process {pid} is running")
    print()

    # Send quit command via remote execution
    print("[INFO] Sending quit command via remote execution...")
    quit_command = "import unreal; unreal.get_engine().request_exit(False)"

    if remote_client.send_command(quit_command):
        print("[OK] Quit command sent")
        print()

        # Wait and verify closure
        print("[INFO] Waiting for graceful shutdown...")
        for i in range(10):  # Wait up to 10 seconds
            time.sleep(1)
            if not is_process_running(pid):
                print(f"[OK] Process {pid} closed successfully")
                PID_FILE.unlink()
                print(f"[OK] Removed PID file")
                print()
                return True
            print(f"  Waiting... ({i+1}/10)")

        print("[WARN] Process still running after 10 seconds")
        print("[WARN] May need force kill")
        return False

    else:
        print("[FAIL] Failed to send quit command")
        return False

if __name__ == "__main__":
    print()
    success = close_game()

    if success:
        print("=" * 70)
        print("GAME CLOSED SUCCESSFULLY")
        print("=" * 70)
    else:
        print("=" * 70)
        print("GAME CLOSURE INCOMPLETE")
        print("=" * 70)
        print()
        print("To force close:")
        print(f"  taskkill /F /PID {get_running_game()['pid'] if get_running_game() else 'UNKNOWN'}")
