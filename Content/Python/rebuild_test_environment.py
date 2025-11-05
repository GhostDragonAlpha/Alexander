"""
Rebuild Test Environment (Fixed for UE 5.6)
Run this in Unreal Python console to rebuild the test environment with API fixes

Usage:
    py exec(open('Content/Python/rebuild_test_environment.py').read())
"""

import unreal

def rebuild_environment():
    """Clean rebuild of test environment in current level"""

    unreal.log("")
    unreal.log("=" * 70)
    unreal.log("REBUILDING TEST ENVIRONMENT (UE 5.6 Fixed)")
    unreal.log("=" * 70)
    unreal.log("")

    # Step 1: Clean up any incomplete build
    unreal.log("Step 1: Cleaning up previous incomplete build...")
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()

    cleanup_keywords = ["Label_", "Cam1_Overview", "Cam2_Detail", "Cam3_Detail", "Cam4_Character", "Cam5_Isometric"]
    cleaned = 0

    for actor in all_actors:
        if actor:
            actor_name = str(actor.get_name())
            for keyword in cleanup_keywords:
                if keyword in actor_name:
                    unreal.EditorLevelLibrary.destroy_actor(actor)
                    cleaned += 1
                    break

    unreal.log(f"  Cleaned up {cleaned} actors from previous build")
    unreal.log("")

    # Step 2: Run the fixed build script
    unreal.log("Step 2: Building test environment with fixed scripts...")
    unreal.log("")

    try:
        import os
        script_dir = r'C:\Users\allen\Desktop\Alexander\Alexander\Content\Python'
        build_script = os.path.join(script_dir, 'build_test_environment.py')

        if not os.path.exists(build_script):
            unreal.log_error(f"Build script not found: {build_script}")
            return False

        with open(build_script, 'r') as f:
            build_code = f.read()

        # Execute the fixed build script
        exec(build_code, {'__name__': '__main__'})

        unreal.log("")
        unreal.log("=" * 70)
        unreal.log("✓ REBUILD COMPLETE!")
        unreal.log("=" * 70)
        unreal.log("")
        unreal.log("Next steps:")
        unreal.log("  1. Run validation: py exec(open('Content/Python/validate_environment.py').read())")
        unreal.log("  2. Take screenshots: py exec(open('Content/Python/snap_all_test_cameras.py').read())")
        unreal.log("")

        return True

    except Exception as e:
        unreal.log_error("=" * 70)
        unreal.log_error("✗ REBUILD FAILED!")
        unreal.log_error("=" * 70)
        unreal.log_error(f"Error: {str(e)}")
        unreal.log_error("")

        import traceback
        unreal.log_error("Full traceback:")
        unreal.log_error(traceback.format_exc())

        return False

# Auto-execute
if __name__ == "__main__":
    rebuild_environment()
