"""
Remote development script - connects to running Unreal Editor
Iteratively develops the project using screenshot feedback
"""
import unreal
import time

def take_editor_screenshot(name="dev_iteration"):
    """Take a screenshot of current viewport"""
    unreal.AutomationLibrary.take_high_res_screenshot(1920, 1080, name)
    print(f"Screenshot taken: {name}")

def get_current_level_info():
    """Get info about currently loaded level"""
    world = unreal.EditorLevelLibrary.get_editor_world()
    world_name = world.get_name()

    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    actor_count = len(actors)

    print(f"Current Level: {world_name}")
    print(f"Total Actors: {actor_count}")

    # List some key actors
    for actor in actors[:10]:
        print(f"  - {actor.get_name()} ({actor.get_class().get_name()})")

    return world, actors

def find_spaceship():
    """Find the spaceship blueprint in content"""
    actors = unreal.EditorLevelLibrary.get_all_level_actors()

    for actor in actors:
        if "spaceship" in actor.get_name().lower() or "ship" in actor.get_name().lower():
            print(f"Found spaceship actor: {actor.get_name()}")
            return actor

    print("No spaceship found in current level")
    return None

def add_spaceship_to_level():
    """Add spaceship to current level if not present"""
    world = unreal.EditorLevelLibrary.get_editor_world()

    # Check if spaceship already exists
    ship = find_spaceship()
    if ship:
        print("Spaceship already in level, selecting it")
        unreal.EditorLevelLibrary.set_selected_level_actors([ship])
        return ship

    # Load and spawn spaceship
    spaceship_path = "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer_C"
    spaceship_class = unreal.load_class(None, spaceship_path)

    if spaceship_class:
        location = unreal.Vector(0, 0, 20000)  # 200m above origin
        rotation = unreal.Rotator(0, 0, 0)

        ship = world.spawn_actor(spaceship_class, location, rotation)
        if ship:
            print(f"✓ Spawned spaceship at {location}")
            unreal.EditorLevelLibrary.set_selected_level_actors([ship])
            return ship

    print("✗ Failed to spawn spaceship")
    return None

# Main development loop
print("=" * 80)
print("REMOTE DEVELOPMENT SESSION")
print("=" * 80)

# Step 1: Get current state
print("\n1. Current Level Info:")
world, actors = get_current_level_info()

# Step 2: Take initial screenshot
print("\n2. Taking initial screenshot...")
take_editor_screenshot("iteration_start")

# Step 3: Make improvement - add spaceship if not present
print("\n3. Ensuring spaceship is in level...")
ship = add_spaceship_to_level()

# Step 4: Take screenshot of result
print("\n4. Taking result screenshot...")
take_editor_screenshot("iteration_with_ship")

print("\n" + "=" * 80)
print("Iteration complete. Check Screenshots/WindowsEditor/ folder")
print("=" * 80)
