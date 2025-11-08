"""
Startup script for autonomous profiling
This runs automatically when editor launches via -ExecutePythonScript
Opens SolarSystem level and enters PIE mode for profiling data collection
"""

import unreal
import time

print("=" * 80)
print("AUTONOMOUS PROFILING STARTUP SCRIPT")
print("=" * 80)

# Step 1: Load SolarSystem level
print("[1/2] Loading SolarSystem level...")
level_path = "/Game/SolarSystem"

try:
    # Use editor level library to load the level
    success = unreal.EditorLevelLibrary.load_level(level_path)
    if success:
        print(f"  ✅ Loaded {level_path}")
    else:
        print(f"  ❌ Failed to load {level_path}")
        exit(1)
except Exception as e:
    print(f"  ❌ Error loading level: {e}")
    exit(1)

# Step 2: Enter PIE mode
print("[2/2] Entering PIE (Play In Editor) mode...")
try:
    # Start PIE session
    pie_settings = unreal.LevelEditorPlaySettings()
    unreal.EditorLevelLibrary.editor_play_in_editor()
    print("  ✅ PIE mode started")
    print()
    print("Systems now ticking - profiling data being collected:")
    print("  - OrbitalMechanics")
    print("  - PhysicsConsensus (Predict + Validate)")
    print("  - FlightController")
    print("  - Spaceship")
    print("  - StarSystemManager")
    print()
    print("Editor will run for 3 minutes, then auto-quit to export JSON")
    print("=" * 80)
    
except Exception as e:
    print(f"  ❌ Error starting PIE: {e}")
    exit(1)
