"""
===============================================================================
CUSTOM STARTUP SCRIPT
===============================================================================
This file is YOUR space for custom code that runs on startup.
Edit freely - add your own phases, experimental code, whatever you need!

This runs AFTER the configured phases from workflow_config.json
===============================================================================
"""

import unreal
import sys
import os

print("\n" + "=" * 70)
print("  CUSTOM STARTUP CODE")
print("=" * 70 + "\n")

# ============================================================================
# ✏️ EDITABLE SECTION - ADD YOUR CUSTOM CODE BELOW
# ============================================================================

# Example: Print a message
print("💡 Custom startup script loaded")
print("   Edit this file (custom_startup.py) to add your own code")

# Example: List all actors in current level
# Uncomment to enable:
# all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
# print(f"Current level has {len(all_actors)} actors")

# Example: Run a custom script
# Uncomment and modify path:
# custom_script_path = r'C:\path\to\your\script.py'
# if os.path.exists(custom_script_path):
#     with open(custom_script_path, 'r') as f:
#         exec(f.read())

# Example: Conditional execution based on level
# current_level = unreal.EditorLevelLibrary.get_editor_world()
# if current_level and "TestArena" in current_level.get_name():
#     print("We're in the test arena - doing test arena stuff...")
# elif current_level and "VRTemplate" in current_level.get_name():
#     print("We're in the game level - doing game stuff...")

# ============================================================================
# Add more custom code here...
# ============================================================================

# ==============================================================================
# AUTOMATIC CLEANUP DISABLED
# ==============================================================================
# The cleanup script works but has exec() scope issues when auto-run on startup.
# Instead, use the batch restart method to run cleanup manually when needed.
#
# To clean up duplicates:
# 1. Edit this file and uncomment the section below
# 2. Run APPLY_CHANGES_NO_PAUSE.bat
# 3. Check the log for results
# 4. Comment this section back out
# ==============================================================================

print("\n🗺️ Creating/Loading clean Solar System level...")
sys.path.insert(0, os.path.dirname(__file__))
import create_solar_system_level
create_solar_system_level.create_clean_solar_system_level()

# Uncomment below to run cleanup on next restart:
# import sys
# sys.path.insert(0, os.path.dirname(__file__))
# import cleanup_duplicates

# ============================================================================
# ONE-TIME SOLAR SYSTEM SETUP - ENABLED TO POPULATE NEW LEVEL
# ============================================================================
print("\n🌟 Running solar system setup...")
sys.path.insert(0, os.path.dirname(__file__))
import setup_solar_system
setup_solar_system.setup_solar_system()
print("✅ Solar system setup complete!\n")
# ============================================================================

# ============================================================================
# CREATE CELESTIAL MATERIALS (runs every startup - materials are created once)
# ============================================================================
print("\n🎨 Setting up celestial materials...")
sys.path.insert(0, os.path.dirname(__file__))
import create_celestial_materials
create_celestial_materials.setup_all_materials()
# ============================================================================

# ============================================================================
# ONE-TIME CAMERA SETUP
# ============================================================================
print("\n📷 Setting up camera system...")
sys.path.insert(0, os.path.dirname(__file__))
import setup_cameras
setup_cameras.setup_solar_system_cameras()
# ============================================================================

# ============================================================================
# NIAGARA PARTICLE SYSTEMS FOR SUN
# ============================================================================
print("\n✨ Creating Niagara particle systems for spectacular Sun...")
sys.path.insert(0, os.path.dirname(__file__))
import create_sun_niagara
create_sun_niagara.create_sun_niagara_systems()
# ============================================================================

# ============================================================================
# END OF EDITABLE SECTION
# ============================================================================

print("\n" + "=" * 70)
print("  CUSTOM STARTUP COMPLETE")
print("=" * 70 + "\n")
