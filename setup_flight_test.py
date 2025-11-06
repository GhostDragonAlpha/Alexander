"""
Automatically create a playable Flight Test level with spaceship and planet
"""
import unreal

def log(msg):
    print(f"[FlightTest] {msg}")
    unreal.log(msg)

def create_flight_test_level():
    """Create a simple level for testing spaceship flight"""

    log("=" * 80)
    log("CREATING FLIGHT TEST LEVEL")
    log("=" * 80)

    # Create new level
    log("\n1. Creating new level...")
    editor_level_lib = unreal.EditorLevelLibrary()
    editor_asset_lib = unreal.EditorAssetLibrary()

    # Create level at /Game/FlightTest
    level_path = "/Game/FlightTest"
    success = editor_level_lib.new_level(level_path)

    if not success:
        log("✗ Failed to create level, trying to load existing...")
        editor_level_lib.load_level(level_path)
    else:
        log("✓ Created new level")

    world = editor_level_lib.get_editor_world()

    #  2. Add directional light
    log("\n2. Adding lighting...")
    light_class = unreal.DirectionalLight
    light = editor_level_lib.spawn_actor_from_class(
        light_class,
        unreal.Vector(0, 0, 500),
        unreal.Rotator(-45, 0, 0)
    )
    if light:
        log(f"✓ Added DirectionalLight")

    # 3. Add Sky Atmosphere
    log("\n3. Adding sky atmosphere...")
    sky_class = unreal.SkyAtmosphere
    sky = editor_level_lib.spawn_actor_from_class(
        sky_class,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )
    if sky:
        log("✓ Added SkyAtmosphere")

    # 4. Load and place spaceship
    log("\n4. Placing spaceship...")
    spaceship_path = "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer"

    if editor_asset_lib.does_asset_exist(spaceship_path):
        spaceship_class = unreal.load_class(None, spaceship_path + ".BP_VRSpaceshipPlayer_C")

        if spaceship_class:
            spaceship = editor_level_lib.spawn_actor_from_class(
                spaceship_class,
                unreal.Vector(0, 0, 200),  # 2m above ground
                unreal.Rotator(0, 0, 0)
            )
            if spaceship:
                log(f"✓ Placed spaceship at origin")
            else:
                log("✗ Failed to spawn spaceship")
        else:
            log("✗ Could not load spaceship class")
    else:
        log(f"✗ Spaceship blueprint not found at {spaceship_path}")

    # 5. Create a simple planet nearby
    log("\n5. Creating test planet...")
    sphere_class = unreal.StaticMeshActor
    planet = editor_level_lib.spawn_actor_from_class(
        sphere_class,
        unreal.Vector(0, 50000, 0),  # 500m away
        unreal.Rotator(0, 0, 0)
    )

    if planet:
        # Load sphere mesh
        sphere_mesh = unreal.load_asset("/Engine/BasicShapes/Sphere")
        if sphere_mesh:
            static_mesh_comp = planet.get_component_by_class(unreal.StaticMeshComponent)
            if static_mesh_comp:
                static_mesh_comp.set_static_mesh(sphere_mesh)
                # Scale to be planet-sized (10km diameter = 10000m = 1000000cm)
                static_mesh_comp.set_world_scale3d(unreal.Vector(50000, 50000, 50000))
                log("✓ Created test planet (10km diameter, 500m away)")

    # 6. Set up player start
    log("\n6. Setting up player start...")
    player_start_class = unreal.PlayerStart
    player_start = editor_level_lib.spawn_actor_from_class(
        player_start_class,
        unreal.Vector(-200, 0, 100),  # Behind spaceship
        unreal.Rotator(0, 0, 0)
    )
    if player_start:
        log("✓ Added PlayerStart")

    # 7. Save level
    log("\n7. Saving level...")
    if editor_level_lib.save_current_level():
        log("✓ Level saved successfully")
    else:
        log("✗ Failed to save level")

    # Summary
    log("\n" + "=" * 80)
    log("FLIGHT TEST LEVEL SETUP COMPLETE!")
    log("=" * 80)
    log("\nTo test:")
    log("1. Open FlightTest level")
    log("2. Click 'Play in VR' (or PIE)")
    log("3. You should spawn near the spaceship")
    log("4. Walk to spaceship and enter it")
    log("5. Use VR controllers to fly toward the planet")
    log("=" * 80)

    return True

if __name__ == "__main__":
    try:
        create_flight_test_level()
    except Exception as e:
        log(f"ERROR: {str(e)}")
        import traceback
        log(traceback.format_exc())
