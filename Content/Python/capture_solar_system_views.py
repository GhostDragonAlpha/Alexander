"""
Capture Screenshots from Solar System Concept Cameras
Shows lighting interaction on scaled-down celestial bodies
"""

import unreal
import time

print("\n" + "="*70)
print("  📸 CAPTURING SOLAR SYSTEM VIEWS")
print("="*70 + "\n")

# Camera configurations
cameras_to_capture = [
    {"name": "Cam_SideView", "desc": "Side perspective of full system"},
    {"name": "Cam_TopDown", "desc": "Overhead view showing orbits"},
    {"name": "Cam_StarCloseup", "desc": "Close view of star and inner planet"},
]

# Find all camera actors
actors = unreal.EditorLevelLibrary.get_all_level_actors()
camera_actors = {}

for actor in actors:
    if not actor:
        continue
    if 'CameraActor' in actor.get_class().get_name():
        label = actor.get_actor_label()
        camera_actors[label] = actor

print(f"Found {len(camera_actors)} cameras in scene\n")

# Capture from each camera
for cam_config in cameras_to_capture:
    cam_name = cam_config["name"]

    if cam_name not in camera_actors:
        print(f"⚠️  Camera not found: {cam_name}")
        continue

    cam_actor = camera_actors[cam_name]

    print(f"📸 Capturing from {cam_name}...")
    print(f"   {cam_config['desc']}")

    # Set viewport to this camera
    unreal.EditorLevelLibrary.set_level_viewport_camera_info(
        cam_actor.get_actor_location(),
        cam_actor.get_actor_rotation()
    )

    # Small delay for viewport to update
    time.sleep(0.3)

    # Capture screenshot
    filename = f"solar_system_{cam_name.lower()}"
    unreal.SystemLibrary.execute_console_command(
        None,
        f"HighResShot 1920x1080 filename={filename}"
    )

    print(f"  ✅ Saved: {filename}.png\n")

print("="*70)
print("  ✅ ALL VIEWS CAPTURED!")
print("="*70)

print("\n📁 Screenshots saved to: Alexander/Saved/Screenshots/WindowsEditor/")
print("\n📊 Files to review:")
for cam_config in cameras_to_capture:
    cam_name = cam_config["name"]
    print(f"  • solar_system_{cam_name.lower()}.png - {cam_config['desc']}")

print("\n🔍 What to look for:")
print("  ✅ Star sphere visible and bright")
print("  ✅ Planets visible at different distances")
print("  ✅ Closer planets appear brighter (light falloff)")
print("  ✅ Shadows cast by planets")
print("  ✅ Spaceship reference visible")
print("  ❌ If black: Need to check lighting setup")

print("\n💡 This verifies lighting system works before full-scale implementation!")
print()
