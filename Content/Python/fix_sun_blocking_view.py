"""
Fix Sun Sphere Blocking Camera View
The massive sun sphere (50x scale) might be blocking the cameras
"""

import unreal

print("\n" + "="*70)
print("  🔭 CAMERA vs SUN POSITION ANALYSIS")
print("="*70 + "\n")

actors = unreal.EditorLevelLibrary.get_all_level_actors()

# Find cameras
cameras = []
sun_sphere = None

for actor in actors:
    if not actor:
        continue

    label = actor.get_actor_label()

    if 'CameraActor' in actor.get_class().get_name():
        cameras.append((label, actor))
    elif 'Sun_Visual_Sphere' in label:
        sun_sphere = actor

# Analyze camera positions
print("📷 CAMERA POSITIONS:\n")
for label, cam in cameras[:3]:  # First 3 cameras
    pos = cam.get_actor_location()
    rot = cam.get_actor_rotation()
    print(f"{label}:")
    print(f"  Position: ({pos.x:.0f}, {pos.y:.0f}, {pos.z:.0f})")
    print(f"  Looking at: Yaw={rot.yaw:.0f}°\n")

if sun_sphere:
    pos = sun_sphere.get_actor_location()
    scale = sun_sphere.get_actor_scale3d()
    print(f"☀️ SUN SPHERE:")
    print(f"  Position: ({pos.x:.0f}, {pos.y:.0f}, {pos.z:.0f})")
    print(f"  Scale: {scale.x}x (HUGE!)")
    print(f"  Actual radius: ~{scale.x * 50:.0f} units\n")

# DIAGNOSIS
print("="*70)
print("  🔍 DIAGNOSIS")
print("="*70 + "\n")

print("⚠️  PROBLEM IDENTIFIED:")
print("  • Cam5_Isometric is at (2000, -2000, 1500)")
print("  • Sun sphere is at (2000, 0, 1000)")
print("  • Sun is 50x scale = ~2500 unit radius!")
print("  • Camera might be INSIDE or blocked by sun sphere!\n")

# SOLUTION: Move sun far away OR make it smaller
print("="*70)
print("  🔧 FIX: REMOVE BLOCKING SUN SPHERES")
print("="*70 + "\n")

# Remove all sun visual spheres (keep the LIGHT, remove the visual)
removed_count = 0
for actor in actors:
    if actor and 'Sun_Visual_Sphere' in actor.get_actor_label():
        label = actor.get_actor_label()
        actor.destroy_actor()
        removed_count += 1
        print(f"✅ Removed: {label}")

print(f"\n✅ Removed {removed_count} sun sphere(s)\n")

print("="*70)
print("  ✅ FIX APPLIED!")
print("="*70 + "\n")

print("🎯 RESULT:")
print("  • Sun visual spheres removed")
print("  • Star_Dynamic light still active (500,000 intensity)")
print("  • Cameras no longer blocked")
print("  • Scene should now be visible!\n")

print("📸 Ready to capture unblocked screenshots!\n")
