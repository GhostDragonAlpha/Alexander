#!/usr/bin/env python3
"""
Process Cleanup Utility
Kills old/stale Unreal Editor and Python test processes
"""

import subprocess
import sys

def get_unreal_processes():
    """Get list of UnrealEditor.exe processes"""
    result = subprocess.run(
        ['tasklist', '/FI', 'IMAGENAME eq UnrealEditor.exe', '/FO', 'CSV', '/NH'],
        capture_output=True,
        text=True
    )

    processes = []
    for line in result.stdout.strip().split('\n'):
        if 'UnrealEditor.exe' in line:
            parts = line.split(',')
            if len(parts) >= 2:
                pid = parts[1].strip('"')
                processes.append(pid)

    return processes

def get_python_processes():
    """Get list of python.exe processes"""
    result = subprocess.run(
        ['tasklist', '/FI', 'IMAGENAME eq python.exe', '/FO', 'CSV', '/NH'],
        capture_output=True,
        text=True
    )

    processes = []
    current_pid = str(subprocess.os.getpid())

    for line in result.stdout.strip().split('\n'):
        if 'python.exe' in line:
            parts = line.split(',')
            if len(parts) >= 2:
                pid = parts[1].strip('"')
                if pid != current_pid:  # Don't kill self
                    processes.append(pid)

    return processes

def kill_all_unreal():
    """Kill all UnrealEditor processes"""
    processes = get_unreal_processes()
    print(f"Found {len(processes)} UnrealEditor processes")

    for pid in processes:
        print(f"Killing UnrealEditor PID {pid}...")
        subprocess.run(['taskkill', '/F', '/PID', pid], capture_output=True)

    print(f"Killed {len(processes)} UnrealEditor processes")
    return len(processes)

def kill_all_python():
    """Kill all Python processes except self"""
    processes = get_python_processes()
    print(f"Found {len(processes)} Python processes (excluding self)")

    for pid in processes:
        print(f"Killing Python PID {pid}...")
        subprocess.run(['taskkill', '/F', '/PID', pid], capture_output=True)

    print(f"Killed {len(processes)} Python processes")
    return len(processes)

def main():
    print("=" * 60)
    print("PROCESS CLEANUP UTILITY")
    print("=" * 60 + "\n")

    # Show current status
    unreal_count = len(get_unreal_processes())
    python_count = len(get_python_processes())

    print(f"Current status:")
    print(f"  - UnrealEditor processes: {unreal_count}")
    print(f"  - Python processes: {python_count}")
    print()

    if len(sys.argv) > 1:
        if sys.argv[1] == '--unreal':
            kill_all_unreal()
        elif sys.argv[1] == '--python':
            kill_all_python()
        elif sys.argv[1] == '--all':
            kill_all_unreal()
            kill_all_python()
        else:
            print("Usage:")
            print("  python cleanup_processes.py --unreal  # Kill all Unreal processes")
            print("  python cleanup_processes.py --python  # Kill all Python processes")
            print("  python cleanup_processes.py --all     # Kill both")
    else:
        print("Usage:")
        print("  python cleanup_processes.py --unreal  # Kill all Unreal processes")
        print("  python cleanup_processes.py --python  # Kill all Python processes")
        print("  python cleanup_processes.py --all     # Kill both")

if __name__ == "__main__":
    main()
