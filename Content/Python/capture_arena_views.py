"""
Capture Screenshots from AI Vision Test Arena Cameras
Shows the newly integrated spaceship cockpit from multiple angles
"""

import unreal
import time

print("\n" + "="*70)
print("  📸 CAPTURING ARENA CAMERA VIEWS")
print("="*70 + "\n")

# Ensure we're in the right level
world = unreal.EditorLevelLibrary.get_editor_world()
level_name = world.get_name()
print(f"Current level: {level_name}\n")

# Find all camera actors
all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
cameras = []

for actor in all_actors:
    if not actor:
        continue

    class_name = actor.get_class().get_name()
    if 'CameraActor' in class_name:
        label = actor.get_actor_label()
        cameras.append((label, actor))

print(f"📊 Found {len(cameras)} cameras in scene\n")

# Priority cameras for cockpit viewing
priority_cameras = ['Cam1_Overview_North', 'Cam2_Detail_East', 'Cam5_Isometric']

# Take screenshots from priority cameras
screenshot_count = 0
for cam_name in priority_cameras:
    # Find the camera
    cam_actor = None
    for label, actor in cameras:
        if cam_name in label:
            cam_actor = actor
            break

    if not cam_actor:
        print(f"⚠️  Camera not found: {cam_name}")
        continue

    print(f"📸 Capturing from {cam_name}...")

    # Set as active camera
    unreal.EditorLevelLibrary.set_level_viewport_camera_info(
        cam_actor.get_actor_location(),
        cam_actor.get_actor_rotation()
    )

    # Small delay to let viewport update
    time.sleep(0.5)

    # Capture screenshot
    timestamp = time.strftime("%Y%m%d_%H%M%S")
    filename = f"arena_{cam_name.lower()}_{timestamp}"

    # Take screenshot
    automation = unreal.AutomationLibrary()
    automation.take_high_res_screenshot(1920, 1080, filename)

    screenshot_count += 1
    print(f"  ✅ Saved: {filename}.png\n")

print("="*70)
print(f"  ✅ CAPTURED {screenshot_count} VIEWS!")
print("="*70)
print(f"\n📁 Screenshots saved to: Alexander/Saved/Screenshots/WindowsEditor/")
print(f"📊 Views captured:")
print(f"  • Cam1_Overview_North - Full arena + cockpit")
print(f"  • Cam2_Detail_East - Side view of cockpit in Zone 4")
print(f"  • Cam5_Isometric - Angled perspective\n")
print(f"🎯 Next: Review screenshots and plan Phase 4 (Flight Physics)\n")
