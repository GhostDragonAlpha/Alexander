"""
Step 1: Inspect what material the star currently has
"""
import unreal

print("\n" + "="*70)
print("  🔍 INSPECTING STAR MATERIAL")
print("="*70 + "\n")

actors = unreal.EditorLevelLibrary.get_all_level_actors()

for actor in actors:
    if not actor:
        continue
    if 'StaticMeshActor' in actor.get_class().get_name() and 'Star' in actor.get_actor_label():
        print(f"Found: {actor.get_actor_label()}")
        print(f"Class: {actor.get_class().get_name()}")

        # Get static mesh component
        static_mesh_comp = actor.static_mesh_component
        if static_mesh_comp:
            print(f"\nStatic Mesh: {static_mesh_comp.static_mesh}")

            # Get materials
            num_materials = static_mesh_comp.get_num_materials()
            print(f"Number of materials: {num_materials}")

            for i in range(num_materials):
                material = static_mesh_comp.get_material(i)
                print(f"  Material {i}: {material}")

        # Get transform info
        location = actor.get_actor_location()
        scale = actor.get_actor_scale3d()
        print(f"\nLocation: {location}")
        print(f"Scale: {scale}")

        break

print("\n" + "="*70)
print("✅ Inspection complete!")
print("="*70)
