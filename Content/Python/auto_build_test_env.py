"""
Auto-Build Test Environment
This script automatically creates and builds the AI Vision Test Environment
Can be executed via command line or integrated into init_unreal.py
"""

import unreal
import os
import time

def auto_build_test_environment():
    """Fully automated test environment creation"""

    unreal.log("")
    unreal.log("=" * 70)
    unreal.log("AUTO-BUILD: AI Vision Test Environment")
    unreal.log("=" * 70)
    unreal.log("")

    # Step 1: Create new level
    unreal.log("Step 1: Creating new level 'AIVisionTestArena'...")

    try:
        # Create a new empty level
        editor_level_lib = unreal.EditorLevelLibrary

        # Try to load existing level first
        level_path = "/Game/AIVisionTestArena"
        level_exists = unreal.EditorAssetLibrary.does_asset_exist(level_path)

        if level_exists:
            unreal.log_warning(f"  Level already exists: {level_path}")
            unreal.log("  Loading existing level...")
            editor_level_lib.load_level(level_path)
        else:
            unreal.log("  Creating new empty level...")
            # UE 5.6: new_level() takes only one argument (full path)
            editor_level_lib.new_level(level_path)

        unreal.log("  ✓ Level ready")

    except Exception as e:
        unreal.log_warning(f"  Could not create new level: {e}")
        unreal.log("  Continuing with current level...")

    unreal.log("")

    # Step 2: Check if environment already exists
    unreal.log("Step 2: Checking for existing environment...")

    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    existing_cameras = [a for a in all_actors if a and "Cam1_Overview_North" in str(a.get_name())]

    if existing_cameras:
        unreal.log_warning("  ⚠ Environment already exists!")
        unreal.log(f"  Found {len(existing_cameras)} existing camera markers")
        unreal.log("  Skipping build to avoid duplicates.")
        unreal.log("")
        unreal.log("  To rebuild: Delete existing actors or use a fresh level")
        return False

    unreal.log("  ✓ No existing environment found - proceeding with build")
    unreal.log("")

    # Step 3: Execute the builder
    unreal.log("Step 3: Building test environment...")
    unreal.log("  This will take approximately 30 seconds...")
    unreal.log("")

    try:
        # Get the build script path
        script_dir = r'C:\Users\allen\Desktop\Alexander\Alexander\Content\Python'
        build_script = os.path.join(script_dir, 'build_test_environment.py')

        if not os.path.exists(build_script):
            unreal.log_error(f"  Build script not found: {build_script}")
            return False

        unreal.log(f"  Executing: {build_script}")
        unreal.log("")

        # Execute the build script
        with open(build_script, 'r') as f:
            build_code = f.read()

        # Run it with the correct module name
        exec(build_code, {'__name__': '__main__'})

        unreal.log("")
        unreal.log("=" * 70)
        unreal.log("✓ BUILD COMPLETE!")
        unreal.log("=" * 70)
        unreal.log("")

        # Step 4: Save the level
        unreal.log("Step 4: Saving level...")
        try:
            unreal.EditorLevelLibrary.save_current_level()
            unreal.log("  ✓ Level saved")
        except Exception as e:
            unreal.log_warning(f"  Could not auto-save level: {e}")
            unreal.log("  Please save manually: File → Save Current Level")

        unreal.log("")

        # Step 5: Next steps
        unreal.log("=" * 70)
        unreal.log("NEXT STEPS")
        unreal.log("=" * 70)
        unreal.log("")
        unreal.log("1. Add Post Process Volume (CRITICAL!):")
        unreal.log("   - Place Actors → Post Process Volume")
        unreal.log("   - Details: Check 'Infinite Extent (Unbound)'")
        unreal.log("   - Details: Uncheck 'Auto Exposure'")
        unreal.log("")
        unreal.log("2. Assign materials to test spheres (Zone 2)")
        unreal.log("   - Find spheres: Metal, Glass, Plastic, Stone, Ceramic, Emissive")
        unreal.log("   - Assign corresponding materials")
        unreal.log("")
        unreal.log("3. Test screenshots:")
        unreal.log("   - Pilot Cam1_Overview_North (right-click in World Outliner)")
        unreal.log("   - Run: py exec(open('Content/Python/snap.py').read())")
        unreal.log("")
        unreal.log("4. Validate build:")
        unreal.log("   - Run: py exec(open('Content/Python/validate_environment.py').read())")
        unreal.log("")
        unreal.log("=" * 70)
        unreal.log("")

        return True

    except Exception as e:
        unreal.log_error("=" * 70)
        unreal.log_error("✗ BUILD FAILED!")
        unreal.log_error("=" * 70)
        unreal.log_error(f"Error: {str(e)}")
        unreal.log_error("")

        import traceback
        unreal.log_error("Full traceback:")
        unreal.log_error(traceback.format_exc())

        return False

# Auto-execute when this script is run
if __name__ == "__main__":
    auto_build_test_environment()
