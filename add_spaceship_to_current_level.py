"""
Add spaceship to whatever level is currently open
"""
import unreal

# Get current world
world = unreal.EditorLevelLibrary.get_editor_world()
print(f"Current world: {world.get_name()}")

# Load spaceship class
spaceship_path = "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer_C"
spaceship_class = unreal.load_class(None, spaceship_path)

if spaceship_class:
    # Spawn near origin where camera can see it
    location = unreal.Vector(0, 0, 20000)  # 200m up from origin
    rotation = unreal.Rotator(0, 0, 0)

    spaceship = world.spawn_actor(spaceship_class, location, rotation)

    if spaceship:
        print(f"✓ Spawned spaceship at {location}")
        print(f"  Class: {spaceship.get_class().get_name()}")

        # Select it so it's visible in outliner
        unreal.EditorLevelLibrary.set_selected_level_actors([spaceship])

        # Focus camera on it
        unreal.EditorLevelLibrary.pilot_level_actor(spaceship)

        print("✓ Spaceship selected and camera focused")
    else:
        print("✗ Failed to spawn spaceship")
else:
    print(f"✗ Could not load spaceship class from {spaceship_path}")
