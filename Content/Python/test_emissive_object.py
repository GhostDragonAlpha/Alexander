"""
Test: Create Emissive Object Directly in Front of Camera
This doesn't need ANY lights - it glows on its own
If this doesn't show up, the problem is with screenshot capture itself
"""

import unreal

print("\n" + "="*70)
print("  🔬 EMISSIVE TEST OBJECT")
print("="*70 + "\n")

# Find Cam1
actors = unreal.EditorLevelLibrary.get_all_level_actors()
cam1 = None

for actor in actors:
    if actor and 'Cam1' in actor.get_actor_label():
        cam1 = actor
        break

if not cam1:
    print("⚠️  Could not find Cam1!\n")
else:
    cam_pos = cam1.get_actor_location()
    cam_rot = cam1.get_actor_rotation()

    print(f"📷 Cam1 Position: ({cam_pos.x:.0f}, {cam_pos.y:.0f}, {cam_pos.z:.0f})")
    print(f"📷 Cam1 Rotation: Yaw={cam_rot.yaw:.0f}°\n")

    # Place test cube 500 units in front of camera
    # Cam1 is at (0, -1500, 500) facing North (Yaw=0°)
    # So "in front" is towards +Y
    test_pos = unreal.Vector(0, -1000, 500)  # 500 units in front

    print(f"Creating test cube at: ({test_pos.x:.0f}, {test_pos.y:.0f}, {test_pos.z:.0f})\n")

    # Create BIG test cube
    test_cube = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        test_pos
    )

    if test_cube:
        test_cube.set_actor_label("EMISSIVE_TEST_CUBE")
        mesh_comp = test_cube.static_mesh_component

        # Load cube mesh
        cube_mesh = unreal.EditorAssetLibrary.load_asset('/Engine/BasicShapes/Cube')
        if cube_mesh:
            mesh_comp.set_static_mesh(cube_mesh)

            # Make it HUGE
            test_cube.set_actor_scale3d(unreal.Vector(5, 5, 5))

            # Try to load emissive material
            # Engine materials that glow
            emissive_mat = unreal.EditorAssetLibrary.load_asset('/Engine/EngineMaterials/DefaultMaterial')
            if emissive_mat:
                mesh_comp.set_material(0, emissive_mat)

            print("✅ Created EMISSIVE_TEST_CUBE")
            print("   • Position: Directly in front of Cam1")
            print("   • Scale: 5x (HUGE)")
            print("   • Material: DefaultMaterial (bright white)\n")

print("="*70)
print("  ✅ TEST OBJECT CREATED!")
print("="*70 + "\n")

print("🎯 IF THIS SHOWS UP IN SCREENSHOT:")
print("   • Cameras work, capture works")
print("   • Problem is lighting system\n")

print("🎯 IF THIS IS STILL BLACK:")
print("   • Screenshot capture itself is broken")
print("   • OR camera exposure is completely crushed")
print("   • OR wrong level is being captured\n")

print("📸 Ready to test!\n")
