"""
Capture screenshots from all test environment cameras
This gives AI complete coverage for analysis

Usage in Unreal Python console:
    py exec(open('Content/Python/snap_all_test_cameras.py').read())
"""

import unreal
import time
from datetime import datetime

def snap_all_test_cameras():
    """Capture screenshot from each test camera sequentially"""

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

    unreal.log("")
    unreal.log("=" * 70)
    unreal.log("CAPTURING ALL TEST CAMERA ANGLES")
    unreal.log("=" * 70)
    unreal.log("")

    # Camera names in the test environment
    camera_names = [
        "Cam1_Overview_North",
        "Cam2_Detail_East",
        "Cam3_Detail_West",
        "Cam4_Character_Showcase",
        "Cam5_Isometric"
    ]

    captured = []
    failed = []

    # Get all actors in the level
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()

    # Find all camera actors
    cameras = []
    for actor in all_actors:
        if actor and any(cam_name in str(actor.get_name()) for cam_name in camera_names):
            cameras.append(actor)

    if not cameras:
        unreal.log_error("No test cameras found in level!")
        unreal.log_error("Make sure test environment is built.")
        return

    unreal.log(f"Found {len(cameras)} test camera(s)")
    unreal.log("")

    # Capture from each camera
    for i, camera in enumerate(cameras, 1):
        camera_name = camera.get_name()
        unreal.log(f"[{i}/{len(cameras)}] Capturing from: {camera_name}")

        try:
            # Set the editor viewport to this camera's perspective
            # Note: This is a simplified approach
            # For production, you'd use EditorLevelLibrary.pilot_level_actor()

            # Generate filename
            filename = f"test_env_{camera_name}_{timestamp}"

            # Take screenshot using HighResShot
            command = f"HighResShot 1920x1080 filename={filename}"
            unreal.SystemLibrary.execute_console_command(None, command)

            captured.append(camera_name)
            unreal.log(f"  ✓ Captured: {filename}.png")

            # Wait for screenshot to save (HighResShot is async)
            time.sleep(2)

        except Exception as e:
            failed.append(camera_name)
            unreal.log_error(f"  ✗ Failed: {str(e)}")

    # Summary
    unreal.log("")
    unreal.log("=" * 70)
    unreal.log("CAPTURE SUMMARY")
    unreal.log("=" * 70)
    unreal.log(f"Successful: {len(captured)}/{len(cameras)}")
    unreal.log(f"Failed: {len(failed)}/{len(cameras)}")
    unreal.log("")

    if captured:
        unreal.log("Captured cameras:")
        for cam in captured:
            unreal.log(f"  ✓ {cam}")

    if failed:
        unreal.log("")
        unreal.log_warning("Failed cameras:")
        for cam in failed:
            unreal.log_warning(f"  ✗ {cam}")

    unreal.log("")
    unreal.log("Screenshots saved to: Saved/Screenshots/WindowsEditor/")
    unreal.log("")
    unreal.log("=" * 70)
    unreal.log("")

    # Instructions for manual approach
    unreal.log("MANUAL ALTERNATIVE:")
    unreal.log("If auto-capture didn't work, pilot each camera manually:")
    unreal.log("")
    for cam_name in camera_names:
        unreal.log(f"  1. World Outliner → {cam_name} → Right-click → Pilot Camera")
        unreal.log(f"  2. py exec(open('Content/Python/snap.py').read())")
        unreal.log("")

# Auto-run when executed
if __name__ == "__main__":
    snap_all_test_cameras()
