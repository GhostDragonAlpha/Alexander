"""
Startup Camera Capture - Executes during Unreal Editor initialization
This script is designed to be called from init_unreal.py on editor startup
"""

import unreal

print("\n" + "="*70)
print("  📸 CAPTURING ADDITIONAL CAMERA VIEWS (Startup)")
print("="*70 + "\n")

actors = unreal.EditorLevelLibrary.get_all_level_actors()

# Find and capture from Cam_SideView
for actor in actors:
    if not actor:
        continue
    if 'CameraActor' in actor.get_class().get_name():
        label = actor.get_actor_label()
        if 'SideView' in label:
            print(f"📸 Capturing from {label}...")
            unreal.EditorLevelLibrary.set_level_viewport_camera_info(
                actor.get_actor_location(),
                actor.get_actor_rotation()
            )
            unreal.SystemLibrary.execute_console_command(
                None,
                "HighResShot 1920x1080 filename=solar_sideview_fixed"
            )
            print(f"  ✅ Captured: solar_sideview_fixed.png\n")
            break

# Find and capture from Cam_TopDown
for actor in actors:
    if not actor:
        continue
    if 'CameraActor' in actor.get_class().get_name():
        label = actor.get_actor_label()
        if 'TopDown' in label:
            print(f"📸 Capturing from {label}...")
            unreal.EditorLevelLibrary.set_level_viewport_camera_info(
                actor.get_actor_location(),
                actor.get_actor_rotation()
            )
            unreal.SystemLibrary.execute_console_command(
                None,
                "HighResShot 1920x1080 filename=solar_topdown_fixed"
            )
            print(f"  ✅ Captured: solar_topdown_fixed.png\n")
            break

print("="*70)
print("  ✅ ADDITIONAL VIEWS CAPTURED!")
print("="*70)
print()
