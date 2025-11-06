"""
Start Unreal Editor ONCE and keep it running
This is the proper workflow - don't restart constantly!
"""

import subprocess
import time
import os

EDITOR_PATH = r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe"
PROJECT_PATH = r"C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject"

print("=" * 70)
print("STARTING UNREAL EDITOR (ONCE)")
print("=" * 70)

# Check if editor is already running
print("\n[1/3] Checking for running editor...")
result = subprocess.run(
    ['tasklist', '/FI', 'IMAGENAME eq UnrealEditor.exe'],
    capture_output=True,
    text=True
)

if "UnrealEditor.exe" in result.stdout:
    print("  [OK] Editor is ALREADY RUNNING")
    print("  --> No need to start again")
else:
    print("  [!] Editor not running")
    print("\n[2/3] Starting Unreal Editor GUI...")

    # Start editor (GUI mode, not headless)
    subprocess.Popen([
        EDITOR_PATH,
        PROJECT_PATH
    ])

    print("  [OK] Editor starting...")
    print("\n[3/3] Waiting for editor to fully load (60 seconds)...")
    time.sleep(60)
    print("  [OK] Editor should be ready!")

print("\n" + "=" * 70)
print("EDITOR IS RUNNING")
print("=" * 70)
print("\nNEXT STEPS:")
print("  1. The Unreal Editor GUI should be visible on your screen")
print("  2. VRTemplateMap should be loaded")
print("  3. Now use remote_client.py to send commands")
print("  4. DO NOT close this editor - keep it running!")
print("=" * 70)
