"""
Setup camera system for viewing the solar system
Creates multiple camera angles and a camera manager
"""

import unreal

def spawn_camera(location, rotation, name):
    """Spawn a camera actor at the specified location"""

    editor_level_library = unreal.EditorLevelLibrary

    # Spawn camera actor
    camera = editor_level_library.spawn_actor_from_class(
        unreal.CameraActor,
        location,
        rotation
    )

    if camera:
        camera.set_actor_label(name)
        unreal.log(f"Created camera: {name}")
        return camera
    else:
        unreal.log_error(f"Failed to create camera: {name}")
        return None

def setup_solar_system_cameras():
    """Create camera positions for viewing the solar system"""

    unreal.log("=" * 70)
    unreal.log("SETTING UP SOLAR SYSTEM CAMERAS")
    unreal.log("=" * 70)

    # Check if cameras already exist
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    camera_names = [actor.get_actor_label() for actor in all_actors if isinstance(actor, unreal.CameraActor)]

    if "Camera_SolarSystemWide" in camera_names:
        unreal.log("Cameras already exist, skipping creation")
        unreal.log("=" * 70)
        return

    # Camera 1: Wide view of entire solar system (high and angled)
    cam1 = spawn_camera(
        unreal.Vector(0, -30000, 20000),  # Back and high
        unreal.Rotator(-30, 90, 0),  # Looking down at an angle
        "Camera_SolarSystemWide"
    )

    # Camera 2: Side view (cinematic orbit view)
    cam2 = spawn_camera(
        unreal.Vector(25000, 0, 5000),  # To the side
        unreal.Rotator(-10, 180, 0),  # Looking at the sun
        "Camera_SideView"
    )

    # Camera 3: Close-up of inner planet system
    cam3 = spawn_camera(
        unreal.Vector(5000, -8000, 3000),  # Close to inner planets
        unreal.Rotator(-20, 60, 0),
        "Camera_InnerPlanets"
    )

    # Camera 4: Sun close-up
    cam4 = spawn_camera(
        unreal.Vector(0, -5000, 2000),  # Close to sun
        unreal.Rotator(-15, 90, 0),
        "Camera_SunCloseup"
    )

    # Camera 5: Top-down orbital view
    cam5 = spawn_camera(
        unreal.Vector(0, 0, 40000),  # Directly above
        unreal.Rotator(-90, 0, 0),  # Looking straight down
        "Camera_TopDown"
    )

    unreal.log("=" * 70)
    unreal.log("CAMERA SETUP COMPLETE")
    unreal.log("=" * 70)
    unreal.log("Cameras created:")
    unreal.log("  - Camera_SolarSystemWide (wide cinematic view)")
    unreal.log("  - Camera_SideView (side orbital view)")
    unreal.log("  - Camera_InnerPlanets (close-up of inner system)")
    unreal.log("  - Camera_SunCloseup (sun detail view)")
    unreal.log("  - Camera_TopDown (orbital plane view)")
    unreal.log("")
    unreal.log("Use Pilot mode or switch cameras to view different angles")
    unreal.log("=" * 70)

if __name__ == "__main__":
    setup_solar_system_cameras()
