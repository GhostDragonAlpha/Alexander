"""
Get PID of Running Unreal Game
"""

import psutil
import json
from pathlib import Path

def find_unreal_process():
    """Find running UnrealEditor process and return info"""

    print("Searching for UnrealEditor process...")
    print()

    for proc in psutil.process_iter(['pid', 'name', 'cmdline', 'create_time']):
        try:
            if 'UnrealEditor' in proc.info['name']:
                pid = proc.info['pid']
                name = proc.info['name']
                cmdline = ' '.join(proc.info['cmdline']) if proc.info['cmdline'] else 'N/A'

                print(f"[FOUND] Process: {name}")
                print(f"[FOUND] PID: {pid}")
                print(f"[FOUND] Command: {cmdline[:200]}...")
                print()

                # Save to file
                pid_data = {
                    "pid": pid,
                    "name": name,
                    "cmdline": cmdline
                }

                with open("running_game_pid.json", 'w') as f:
                    json.dump(pid_data, f, indent=2)

                print(f"[OK] Saved PID to running_game_pid.json")
                return pid

        except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
            pass

    print("[WARN] No UnrealEditor process found")
    return None

if __name__ == "__main__":
    print("=" * 70)
    print("FINDING UNREAL PROCESS PID")
    print("=" * 70)
    print()

    pid = find_unreal_process()

    if pid:
        print()
        print("=" * 70)
        print(f"GAME PID: {pid}")
        print("=" * 70)
        print()
        print("Now you can use this PID to track the game instance")
    else:
        print()
        print("No UnrealEditor process found running")
