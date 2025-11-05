"""
Debug: Check why emissive material isn't visible
"""
import unreal

print("\n" + "="*70)
print("  🔍 CHECKING EMISSIVE MATERIAL")
print("="*70 + "\n")

# Check if star has the material applied
actors = unreal.EditorLevelLibrary.get_all_level_actors()

for actor in actors:
    if not actor:
        continue
    if 'StaticMeshActor' in actor.get_class().get_name() and 'Star' in actor.get_actor_label():
        print(f"Found star: {actor.get_actor_label()}")

        mesh_comp = actor.static_mesh_component
        current_mat = mesh_comp.get_material(0)
        print(f"Current material: {current_mat}")

        # Let's try a VERY bright simple emissive material
        print("\n🔧 Creating super bright test material...")

        # Load material
        mat = unreal.EditorAssetLibrary.load_asset('/Game/Materials/M_Sun_Animated')

        if mat:
            print(f"✅ Loaded animated material")

            # Create material instance dynamic so we can change parameters at runtime
            mat_instance = unreal.MaterialInstanceDynamic.create(mat, mesh_comp)

            # Set VERY high intensity
            mat_instance.set_scalar_parameter_value('BaseIntensity', 1000.0)
            mat_instance.set_scalar_parameter_value('PulseSpeed', 0.5)

            # Apply it
            mesh_comp.set_material(0, mat_instance)

            print("✅ Applied material with intensity: 1000.0")
            print("   If you still can't see it, check:")
            print("   • Viewport > View Mode > Lit (not Wireframe)")
            print("   • Viewport > Show > Post Processing")
            print("   • Eye Adaptation might be too dark")

        break

print("\n" + "="*70)
print("✅ CHECK COMPLETE")
print("="*70)
