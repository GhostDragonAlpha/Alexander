"""
Launch a single game client manually
Simple script for testing - just launch one client
"""

import subprocess
import sys

UNREAL_EDITOR = r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe"
PROJECT_FILE = r"C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject"

# Default port
port = 8080

# Allow port override from command line
if len(sys.argv) > 1:
    port = int(sys.argv[1])

print(f"\n{'='*60}")
print(f"LAUNCHING CLIENT ON PORT {port}")
print(f"{'='*60}\n")

cmd = [
    UNREAL_EDITOR,
    PROJECT_FILE,
    "VRTemplateMap?game=/Script/Alexander.AutomationGameMode",
    "-game",
    "-windowed",
    "-ResX=600",
    "-ResY=400",
    f"-HTTPPort={port}",
    "-nohmd",
    "-log"
]

print(f"Command: {' '.join(cmd)}\n")

# Launch in foreground so you can see output
subprocess.run(cmd)
