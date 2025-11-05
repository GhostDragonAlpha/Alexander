"""
One-time initialization - Run this to build the AI Vision Test Environment
Execute in Unreal Python console:
    py exec(open('Content/Python/run_init_now.py').read())
"""

import unreal
import os

# Get the script directory
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
BUILD_SCRIPT = os.path.join(SCRIPT_DIR, 'build_test_environment.py')

def run_initialization():
    """Execute the test environment builder"""

    unreal.log("")
    unreal.log("=" * 70)
    unreal.log("AI VISION TEST ENVIRONMENT - ONE-TIME INITIALIZATION")
    unreal.log("=" * 70)
    unreal.log("")

    # Check if already initialized
    unreal.log("Checking for existing environment...")
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    existing = [a for a in all_actors if a and "Cam1_Overview_North" in str(a.get_name())]

    if existing:
        unreal.log_warning("⚠ Test environment already exists!")
        unreal.log_warning(f"  Found {len(existing)} camera markers")
        unreal.log("")
        unreal.log("Options:")
        unreal.log("  1. Delete existing actors and run again")
        unreal.log("  2. Create a new level and run again")
        unreal.log("  3. Continue anyway (may create duplicates)")
        unreal.log("")

        response = unreal.EditorDialog.show_message(
            unreal.Text("Environment Already Exists"),
            unreal.Text("Test environment already exists. Rebuild anyway?\n\nThis may create duplicate actors."),
            unreal.AppMsgType.YES_NO
        )

        if response == unreal.AppReturnType.NO:
            unreal.log("✗ Initialization cancelled by user")
            return False

        unreal.log("⚠ User chose to rebuild - continuing...")
        unreal.log("")

    # Verify build script exists
    if not os.path.exists(BUILD_SCRIPT):
        error_msg = f"Build script not found: {BUILD_SCRIPT}"
        unreal.log_error(error_msg)
        unreal.EditorDialog.show_message(
            unreal.Text("Error"),
            unreal.Text(f"{error_msg}\n\nMake sure build_test_environment.py exists in Content/Python/"),
            unreal.AppMsgType.OK
        )
        return False

    unreal.log(f"✓ Build script found: {BUILD_SCRIPT}")
    unreal.log("")

    # Show starting dialog
    unreal.log("=" * 70)
    unreal.log("STARTING BUILD PROCESS...")
    unreal.log("=" * 70)
    unreal.log("")
    unreal.log("This will take approximately 30 seconds.")
    unreal.log("Watch the Output Log for progress...")
    unreal.log("")

    # Execute the build script
    try:
        with open(BUILD_SCRIPT, 'r') as f:
            build_code = f.read()

        # Run the builder
        exec(build_code, {'__name__': '__main__'})

        # Success!
        unreal.log("")
        unreal.log("=" * 70)
        unreal.log("✓ INITIALIZATION COMPLETE!")
        unreal.log("=" * 70)
        unreal.log("")

        # Show completion dialog
        success_message = """AI Vision Test Environment created successfully!

NEXT STEPS:

1. Add Post Process Volume:
   - Place Actors → Post Process Volume
   - Check: Infinite Extent (Unbound)
   - Uncheck: Auto Exposure
   - Set fixed exposure value

2. Assign materials to test spheres (Zone 2):
   - Find spheres labeled Metal, Glass, Plastic, etc.
   - Assign corresponding materials

3. Place your assets in Object Library (Zone 3):
   - Place weapon models on pedestals

4. Test screenshots:
   - Pilot Cam1_Overview_North
   - Run: py exec(open('Content/Python/snap.py').read())

Check the Output Log for detailed build information!"""

        unreal.EditorDialog.show_message(
            unreal.Text("Success!"),
            unreal.Text(success_message),
            unreal.AppMsgType.OK
        )

        return True

    except Exception as e:
        error_msg = f"Build failed: {str(e)}"
        unreal.log_error("=" * 70)
        unreal.log_error("✗ BUILD FAILED!")
        unreal.log_error("=" * 70)
        unreal.log_error(error_msg)
        unreal.log_error("")

        import traceback
        unreal.log_error("Full traceback:")
        unreal.log_error(traceback.format_exc())

        unreal.EditorDialog.show_message(
            unreal.Text("Build Failed"),
            unreal.Text(f"{error_msg}\n\nCheck Output Log for details."),
            unreal.AppMsgType.OK
        )

        return False

# Auto-run when this script is executed
if __name__ == "__main__":
    run_initialization()
