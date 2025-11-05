"""
Step 3: Apply the sun material to the star sphere
"""
import unreal

print("\n" + "="*70)
print("  🎨 APPLYING SUN MATERIAL TO STAR")
print("="*70 + "\n")

# Load the material we just created
material_path = '/Game/Materials/M_Sun_Simple'
material = unreal.EditorAssetLibrary.load_asset(material_path)

if not material:
    print(f"❌ Could not load material: {material_path}")
    print("Make sure the material was created successfully")
else:
    print(f"✅ Loaded material: {material.get_name()}")

    # Find the star actor
    actors = unreal.EditorLevelLibrary.get_all_level_actors()

    for actor in actors:
        if not actor:
            continue
        if 'StaticMeshActor' in actor.get_class().get_name() and 'Star' in actor.get_actor_label():
            print(f"\n📍 Found star: {actor.get_actor_label()}")

            # Get static mesh component
            static_mesh_comp = actor.static_mesh_component

            if static_mesh_comp:
                # Apply material to all material slots
                num_materials = static_mesh_comp.get_num_materials()
                for i in range(num_materials):
                    static_mesh_comp.set_material(i, material)
                    print(f"  ✅ Applied material to slot {i}")

                print(f"\n🌟 Star now has emissive material!")
                print(f"   The star will glow but won't block the point light")

            break

print("\n" + "="*70)
print("✅ MATERIAL APPLIED!")
print("="*70)
print("\nCheck the viewport - the star should now glow yellow-orange!")
