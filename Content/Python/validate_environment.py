"""
Validation Script - Check if AI Vision Test Environment is properly built
Run in Unreal Python console:
    py exec(open('Content/Python/validate_environment.py').read())
"""

import unreal

def validate_environment():
    """Validate that the test environment was built correctly"""

    unreal.log("")
    unreal.log("=" * 70)
    unreal.log("AI VISION TEST ENVIRONMENT - VALIDATION")
    unreal.log("=" * 70)
    unreal.log("")

    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    issues = []
    warnings = []
    successes = []

    # Check 1: Camera positions
    unreal.log("Checking camera positions...")
    required_cameras = [
        "Cam1_Overview_North",
        "Cam2_Detail_East",
        "Cam3_Detail_West",
        "Cam4_Character_Showcase",
        "Cam5_Isometric"
    ]

    found_cameras = []
    for cam_name in required_cameras:
        cameras = [a for a in all_actors if a and cam_name in str(a.get_name())]
        if cameras:
            found_cameras.append(cam_name)
            successes.append(f"✓ Found camera: {cam_name}")
        else:
            issues.append(f"✗ Missing camera: {cam_name}")

    unreal.log(f"  Found {len(found_cameras)}/{len(required_cameras)} cameras")

    # Check 2: Floor and walls
    unreal.log("Checking basic structure...")
    floor_actors = [a for a in all_actors if a and "Floor_Main" in str(a.get_name())]
    if floor_actors:
        successes.append("✓ Floor found")
    else:
        issues.append("✗ Floor missing")

    wall_count = len([a for a in all_actors if a and "Wall_" in str(a.get_name())])
    if wall_count >= 4:
        successes.append(f"✓ Walls found ({wall_count})")
    else:
        issues.append(f"✗ Expected 4 walls, found {wall_count}")

    # Check 3: Lighting
    unreal.log("Checking lighting...")
    sun = [a for a in all_actors if a and "Sun_Main" in str(a.get_name())]
    sky_light = [a for a in all_actors if a and "SkyLight_Main" in str(a.get_name())]

    if sun:
        successes.append("✓ Directional light (sun) found")
    else:
        issues.append("✗ Directional light missing")

    if sky_light:
        successes.append("✓ Sky light found")
    else:
        issues.append("✗ Sky light missing")

    # Check 4: Zone markers (text labels)
    unreal.log("Checking zone labels...")
    zone_labels = [
        "REFERENCE GRID",
        "MATERIAL TEST ZONE",
        "OBJECT LIBRARY",
        "BEFORE",
        "AFTER"
    ]

    text_actors = [a for a in all_actors if isinstance(a, unreal.TextRenderActor)]
    unreal.log(f"  Found {len(text_actors)} text labels total")

    if len(text_actors) >= 20:  # Should have many text labels
        successes.append(f"✓ Text labels found ({len(text_actors)})")
    else:
        warnings.append(f"⚠ Expected ~50 text labels, found {len(text_actors)}")

    # Check 5: Reference objects
    unreal.log("Checking reference objects...")
    static_mesh_actors = [a for a in all_actors if isinstance(a, unreal.StaticMeshActor)]

    unreal.log(f"  Found {len(static_mesh_actors)} static mesh actors")

    # Should have cubes, spheres, etc.
    if len(static_mesh_actors) >= 15:  # Floor + walls + reference cubes + spheres + pedestals
        successes.append(f"✓ Static mesh objects found ({len(static_mesh_actors)})")
    else:
        warnings.append(f"⚠ Expected ~20+ static mesh actors, found {len(static_mesh_actors)}")

    # Check 6: Post Process Volume (manual step)
    unreal.log("Checking post-process volume...")
    ppv = [a for a in all_actors if isinstance(a, unreal.PostProcessVolume)]

    if ppv:
        successes.append("✓ Post Process Volume found")

        # Check if auto-exposure is disabled
        for pp in ppv:
            settings = pp.settings
            if hasattr(settings, 'auto_exposure_method'):
                if settings.auto_exposure_method == unreal.AutoExposureMethod.AEM_MANUAL:
                    successes.append("  ✓ Auto-exposure is disabled (GOOD!)")
                else:
                    warnings.append("  ⚠ Auto-exposure is enabled - should be disabled for consistent screenshots!")
    else:
        warnings.append("⚠ Post Process Volume not found - you need to add this manually!")
        warnings.append("  This is CRITICAL for consistent screenshot brightness")

    # Display results
    unreal.log("")
    unreal.log("=" * 70)
    unreal.log("VALIDATION RESULTS")
    unreal.log("=" * 70)
    unreal.log("")

    if successes:
        unreal.log("✓ SUCCESSES:")
        for success in successes:
            unreal.log(f"  {success}")
        unreal.log("")

    if warnings:
        unreal.log_warning("⚠ WARNINGS:")
        for warning in warnings:
            unreal.log_warning(f"  {warning}")
        unreal.log("")

    if issues:
        unreal.log_error("✗ ISSUES:")
        for issue in issues:
            unreal.log_error(f"  {issue}")
        unreal.log("")

    # Overall status
    unreal.log("=" * 70)
    if not issues and not warnings:
        unreal.log("✓ VALIDATION PASSED - Environment is perfect!")
        status = "PASSED"
    elif not issues:
        unreal.log("⚠ VALIDATION PASSED WITH WARNINGS - Minor issues to fix")
        status = "PASSED_WITH_WARNINGS"
    else:
        unreal.log_error("✗ VALIDATION FAILED - Critical issues found")
        status = "FAILED"
    unreal.log("=" * 70)
    unreal.log("")

    # Summary statistics
    unreal.log("STATISTICS:")
    unreal.log(f"  Total actors in level: {len(all_actors)}")
    unreal.log(f"  Cameras: {len(found_cameras)}/5")
    unreal.log(f"  Text labels: {len(text_actors)}")
    unreal.log(f"  Static meshes: {len(static_mesh_actors)}")
    unreal.log(f"  Lights: {len(sun) + len(sky_light)}/2")
    unreal.log("")

    # Next steps
    if status == "PASSED":
        unreal.log("READY TO USE! 🎉")
        unreal.log("")
        unreal.log("Start testing:")
        unreal.log("  1. Pilot Cam1_Overview_North (right-click in World Outliner)")
        unreal.log("  2. Take screenshot: py exec(open('Content/Python/snap.py').read())")
        unreal.log("  3. Review screenshot quality")
        unreal.log("")

    elif status == "PASSED_WITH_WARNINGS":
        unreal.log("ALMOST READY!")
        unreal.log("")
        unreal.log("Fix warnings:")
        if "Post Process Volume" in str(warnings):
            unreal.log("  1. Add Post Process Volume:")
            unreal.log("     - Place Actors → Post Process Volume")
            unreal.log("     - Check: Infinite Extent (Unbound)")
            unreal.log("     - Uncheck: Auto Exposure")
        unreal.log("")

    else:
        unreal.log("BUILD INCOMPLETE!")
        unreal.log("")
        unreal.log("Try rebuilding:")
        unreal.log("  py exec(open('Content/Python/run_init_now.py').read())")
        unreal.log("")

    # Show dialog with results
    if status == "PASSED":
        message = f"""Validation PASSED! ✓

Environment is properly built:
- {len(found_cameras)}/5 cameras found
- {len(text_actors)} labels created
- {len(static_mesh_actors)} objects placed
- Lighting configured

Ready to start testing!"""

    elif status == "PASSED_WITH_WARNINGS":
        message = f"""Validation passed with warnings ⚠

Environment is mostly complete:
- {len(found_cameras)}/5 cameras found
- {len(text_actors)} labels created
- {len(static_mesh_actors)} objects placed

Warnings: {len(warnings)}
Check Output Log for details."""

    else:
        message = f"""Validation FAILED ✗

Critical issues found: {len(issues)}

Environment may be incomplete.
Check Output Log for details.

Try rebuilding the environment."""

    unreal.EditorDialog.show_message(
        unreal.Text("Validation Results"),
        unreal.Text(message),
        unreal.AppMsgType.OK
    )

    return status

# Auto-run when executed
if __name__ == "__main__":
    validate_environment()
