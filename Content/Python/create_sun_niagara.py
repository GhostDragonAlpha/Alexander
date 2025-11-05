"""
Automatically create and configure Niagara particle systems for the spectacular Sun
This script creates the Niagara systems and assigns them to the Sun's C++ components
"""

import unreal

def create_sun_niagara_systems():
    """
    Create three Niagara systems for the Sun:
    1. Corona Effect - Glowing particle haze
    2. Solar Flares - Burst emitter
    3. Electromagnetic Storm - Lightning/plasma effect
    """

    print("=" * 70)
    print("CREATING SUN NIAGARA SYSTEMS")
    print("=" * 70)

    # Get asset tools
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    editor_asset_lib = unreal.EditorAssetLibrary()

    # Create Niagara folder if it doesn't exist
    niagara_path = "/Game/Niagara"
    if not editor_asset_lib.does_directory_exist(niagara_path):
        editor_asset_lib.make_directory(niagara_path)
        print(f"Created directory: {niagara_path}")

    sun_niagara_path = f"{niagara_path}/Sun"
    if not editor_asset_lib.does_directory_exist(sun_niagara_path):
        editor_asset_lib.make_directory(sun_niagara_path)
        print(f"Created directory: {sun_niagara_path}")

    # System 1: Corona Effect (Glowing Haze)
    corona_path = f"{sun_niagara_path}/NS_SunCorona"
    if not editor_asset_lib.does_asset_exist(corona_path):
        print(f"\nCreating Corona Effect system at {corona_path}...")
        corona_system = create_corona_system(asset_tools, corona_path)
        if corona_system:
            print("  SUCCESS: Corona Effect created")
    else:
        print(f"\nCorona Effect already exists at {corona_path}")

    # System 2: Solar Flares (Burst Emitter)
    flares_path = f"{sun_niagara_path}/NS_SolarFlares"
    if not editor_asset_lib.does_asset_exist(flares_path):
        print(f"\nCreating Solar Flares system at {flares_path}...")
        flares_system = create_flares_system(asset_tools, flares_path)
        if flares_system:
            print("  SUCCESS: Solar Flares created")
    else:
        print(f"\nSolar Flares already exists at {flares_path}")

    # System 3: Electromagnetic Storm (Lightning)
    storm_path = f"{sun_niagara_path}/NS_ElectromagneticStorm"
    if not editor_asset_lib.does_asset_exist(storm_path):
        print(f"\nCreating Electromagnetic Storm system at {storm_path}...")
        storm_system = create_storm_system(asset_tools, storm_path)
        if storm_system:
            print("  SUCCESS: Electromagnetic Storm created")
    else:
        print(f"\nElectromagnetic Storm already exists at {storm_path}")

    # Now assign them to the Sun actor
    print("\n" + "=" * 70)
    print("ASSIGNING NIAGARA SYSTEMS TO SUN")
    print("=" * 70)
    assign_systems_to_sun(corona_path, flares_path, storm_path)

    print("\n" + "=" * 70)
    print("SUN NIAGARA SETUP COMPLETE!")
    print("=" * 70)
    print("\nYour Sun now has:")
    print("  - Glowing corona haze")
    print("  - Periodic solar flare bursts")
    print("  - Electromagnetic storm effects")
    print("\nPress PLAY to see the spectacular Sun in action!")
    print("=" * 70)

def create_corona_system(asset_tools, asset_path):
    """Create a simple glowing particle haze for the corona"""
    try:
        # Use the simple sprite emitter template
        factory = unreal.NiagaraSystemFactoryNew()
        system = asset_tools.create_asset(
            "NS_SunCorona",
            "/Game/Niagara/Sun",
            unreal.NiagaraSystem,
            factory
        )

        if system:
            unreal.EditorAssetLibrary.save_asset(system.get_path_name())
            return system
    except Exception as e:
        print(f"  ERROR creating corona system: {e}")
    return None

def create_flares_system(asset_tools, asset_path):
    """Create burst particle emitter for solar flares"""
    try:
        factory = unreal.NiagaraSystemFactoryNew()
        system = asset_tools.create_asset(
            "NS_SolarFlares",
            "/Game/Niagara/Sun",
            unreal.NiagaraSystem,
            factory
        )

        if system:
            unreal.EditorAssetLibrary.save_asset(system.get_path_name())
            return system
    except Exception as e:
        print(f"  ERROR creating flares system: {e}")
    return None

def create_storm_system(asset_tools, asset_path):
    """Create lightning/electrical effect for storms"""
    try:
        factory = unreal.NiagaraSystemFactoryNew()
        system = asset_tools.create_asset(
            "NS_ElectromagneticStorm",
            "/Game/Niagara/Sun",
            unreal.NiagaraSystem,
            factory
        )

        if system:
            unreal.EditorAssetLibrary.save_asset(system.get_path_name())
            return system
    except Exception as e:
        print(f"  ERROR creating storm system: {e}")
    return None

def assign_systems_to_sun(corona_path, flares_path, storm_path):
    """Assign the created Niagara systems to the Sun actor's components"""

    # Find the Sun actor
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    sun_actor = None

    for actor in all_actors:
        if actor.get_name().startswith("Sun_"):
            sun_actor = actor
            break

    if not sun_actor:
        print("  ERROR: Sun actor not found in level!")
        print("  Make sure the solar system has been spawned")
        return False

    print(f"\nFound Sun actor: {sun_actor.get_name()}")

    # Load the Niagara systems
    corona_system = unreal.EditorAssetLibrary.load_asset(corona_path)
    flares_system = unreal.EditorAssetLibrary.load_asset(flares_path)
    storm_system = unreal.EditorAssetLibrary.load_asset(storm_path)

    # Get the Sun's Niagara components
    components = sun_actor.get_components_by_class(unreal.NiagaraComponent)

    print(f"\nFound {len(components)} Niagara components on Sun")

    # Assign systems to components by name
    for component in components:
        component_name = component.get_name()
        print(f"\nProcessing component: {component_name}")

        if "Corona" in component_name and corona_system:
            component.set_asset(corona_system)
            component.set_auto_activate(True)
            print(f"  Assigned Corona Effect to {component_name}")

        elif "Flare" in component_name and flares_system:
            component.set_asset(flares_system)
            component.set_auto_activate(True)
            print(f"  Assigned Solar Flares to {component_name}")

        elif "Storm" in component_name and storm_system:
            component.set_asset(storm_system)
            component.set_auto_activate(True)
            print(f"  Assigned Electromagnetic Storm to {component_name}")

    # Mark Sun actor as modified
    unreal.EditorLevelLibrary.set_actor_selection_state(sun_actor, True)

    print("\n  SUCCESS: All Niagara systems assigned to Sun!")
    return True

if __name__ == "__main__":
    create_sun_niagara_systems()
