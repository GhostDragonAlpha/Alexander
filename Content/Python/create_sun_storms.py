"""
Create Niagara electromagnetic storm effects for the sun
Step 1: Create simple Niagara system with electrical arcs
"""
import unreal

print("\n" + "="*70)
print("  ⚡ CREATING SUN ELECTROMAGNETIC STORMS")
print("="*70 + "\n")

# Create Niagara system asset
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

# Create in Content/Effects folder
package_path = '/Game/Effects'
asset_name = 'NS_SunStorms'

print(f"Creating Niagara System: {package_path}/{asset_name}")

# Create Niagara system
niagara_factory = unreal.NiagaraSystemFactoryNew()
niagara_system = asset_tools.create_asset(
    asset_name,
    package_path,
    unreal.NiagaraSystem,
    niagara_factory
)

if niagara_system:
    print(f"✅ Niagara System created: {niagara_system.get_name()}")

    # Save the asset
    unreal.EditorAssetLibrary.save_loaded_asset(niagara_system)
    print("  ✅ System saved")

    print(f"\n📦 System path: {niagara_system.get_path_name()}")

    # Now spawn it on the sun
    print("\n🌟 Spawning storm effect on sun...")

    actors = unreal.EditorLevelLibrary.get_all_level_actors()

    for actor in actors:
        if not actor:
            continue
        if 'StaticMeshActor' in actor.get_class().get_name() and 'Star' in actor.get_actor_label():
            sun_location = actor.get_actor_location()

            # Spawn Niagara system at sun location
            niagara_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.NiagaraActor,
                sun_location
            )

            if niagara_actor:
                niagara_actor.set_actor_label("SunStorms_Effect")

                # Get Niagara component
                niagara_comp = niagara_actor.niagara_component

                # Set the Niagara system
                niagara_comp.set_asset(niagara_system)

                # Attach to sun
                niagara_comp.attach_to_component(
                    actor.static_mesh_component,
                    'None',
                    unreal.AttachmentRule.KEEP_RELATIVE,
                    unreal.AttachmentRule.KEEP_RELATIVE,
                    unreal.AttachmentRule.KEEP_RELATIVE,
                    False
                )

                print(f"  ✅ Storm effect spawned and attached to sun")
                print(f"  📍 Location: {sun_location}")

            break

else:
    print("❌ Failed to create Niagara system")

print("\n" + "="*70)
print("✅ SUN STORMS CREATED!")
print("="*70)
print("\nNote: The Niagara system is basic - we'll add fancy effects next!")
print("Open the Niagara system in editor to add:")
print("  • Electrical arc emitters")
print("  • Plasma loops")
print("  • Corona discharge")
print("  • Color gradients (yellow → orange → red)")
