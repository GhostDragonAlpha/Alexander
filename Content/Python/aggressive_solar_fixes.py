"""
Aggressive Solar System Fixes
The gentle adjustments didn't work - time for dramatic changes
"""

import unreal

print("\n" + "="*70)
print("  ⚡ AGGRESSIVE LIGHTING FIXES")
print("="*70 + "\n")

actors = unreal.EditorLevelLibrary.get_all_level_actors()

# Fix 1: MUCH dimmer star light (10K → 500)
print("📉 Drastically reducing star light...")
for actor in actors:
    if not actor:
        continue
    if 'Light' in actor.get_class().get_name() and 'Star' in actor.get_actor_label():
        light_comp = actor.root_component
        light_comp.set_editor_property("intensity", 500.0)  # 200x less than original!
        print(f"  ✅ Star light: → 500 intensity")
        break

# Fix 2: Remove ambient skylight completely
print("\n🌌 Removing ambient skylight (pure darkness)...")
for actor in actors:
    if not actor:
        continue
    if 'SkyLight' in actor.get_class().get_name():
        label = actor.get_actor_label()
        if 'Ambient' in label:
            actor.destroy_actor()
            print(f"  ✅ Deleted: {label} (space is dark!)")
            break

# Fix 3: Actually DELETE floor actors (not just hide)
print("\n🗑️ DELETING floor planes...")
floors_deleted = 0
for actor in actors:
    if not actor:
        continue
    label = actor.get_actor_label()
    class_name = actor.get_class().get_name()
    # Delete any floor/ground/plane actors
    if (label and ('Floor' in label or 'Ground' in label)) or 'Plane' in class_name:
        if 'Planet' not in label:  # Don't delete planets!
            print(f"  ❌ Deleting: {label}")
            actor.destroy_actor()
            floors_deleted += 1

print(f"  ✅ Deleted {floors_deleted} floor actors")

# Fix 4: Set camera to manual exposure
print("\n📸 Setting camera to manual exposure...")
for actor in actors:
    if not actor:
        continue
    if 'CameraActor' in actor.get_class().get_name():
        if 'Closeup' in actor.get_actor_label() or 'StarCloseup' in actor.get_actor_label():
            camera_comp = actor.get_camera_component()

            # Set to manual exposure mode
            camera_comp.set_editor_property("exposure_method", unreal.CameraExposureMethod.MANUAL)
            camera_comp.set_editor_property("exposure_compensation", -4.0)  # Much darker

            print(f"  ✅ Camera: Manual exposure, -4.0 compensation")
            break

# Fix 5: Make star sphere MUCH smaller so it's not filling frame
print("\n⭐ Resizing star sphere...")
for actor in actors:
    if not actor:
        continue
    if 'StaticMeshActor' in actor.get_class().get_name() and 'Star' in actor.get_actor_label():
        current_scale = actor.get_actor_scale3d()
        new_scale = unreal.Vector(0.5, 0.5, 0.5)  # 4x smaller
        actor.set_actor_scale3d(new_scale)
        print(f"  ✅ Star: {current_scale.x} → 0.5 scale")
        break

# Fix 6: Position camera to see planets, not just star
print("\n📍 Repositioning camera...")
for actor in actors:
    if not actor:
        continue
    if 'CameraActor' in actor.get_class().get_name():
        if 'SideView' in actor.get_actor_label():
            # Point this at the solar system from a good angle
            new_loc = unreal.Vector(500, -800, 400)
            new_rot = unreal.Rotator(-15, 30, 0)  # Slight downward angle
            actor.set_actor_location(new_loc, False, False)
            actor.set_actor_rotation(new_rot, False)
            print(f"  ✅ Cam_SideView repositioned for better system view")
            break

print("\n" + "="*70)
print("  ✅ AGGRESSIVE FIXES COMPLETE!")
print("="*70)

print("\n📊 Extreme Changes:")
print("  • Star light: 500 intensity (200x reduction from original)")
print("  • Ambient light: DELETED (pure darkness)")
print("  • Floor: DELETED (no ground)")
print("  • Camera: Manual exposure, -4.0 compensation")
print("  • Star sphere: 0.5 scale (smaller)")
print("  • Camera: Repositioned for system view")

print("\n🎯 Expected Result:")
print("  • BLACK space background")
print("  • Star as bright point")
print("  • Planets faintly visible")
print("  • Dramatic light falloff")
print("\n💡 This should look like ACTUAL space!")
print()
