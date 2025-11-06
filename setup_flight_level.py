"""
FIX THE PROBLEM: Make the ship flyable in VRTemplateMap
This script SETS UP the level for flight, not just tests
"""

import unreal

print("=" * 70)
print("SETTING UP VRTEMPLATEMAP FOR FLIGHT")
print("=" * 70)

# Load the ship blueprint
print("[1/4] Loading BP_VRSpaceshipPlayer...")
ship_bp_path = "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer"
ship_class = unreal.EditorAssetLibrary.load_blueprint_class(ship_bp_path)

if not ship_class:
    print(f"  ERROR: Could not load ship blueprint at {ship_bp_path}")
    print("  Checking if file exists...")
    exists = unreal.EditorAssetLibrary.does_asset_exist(ship_bp_path)
    print(f"  Asset exists: {exists}")
else:
    print(f"  SUCCESS: Loaded {ship_class.get_name()}")

# Get the current world
world = unreal.EditorLevelLibrary.get_editor_world()
print(f"\n[2/4] Current World: {world.get_name()}")

# Check for existing PlayerStart
all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
player_starts = [a for a in all_actors if a.get_class().get_name() == "PlayerStart"]

if not player_starts:
    print("[3/4] No PlayerStart found - CREATING ONE...")
    # Spawn a PlayerStart
    player_start_class = unreal.load_class(None, "/Script/Engine.PlayerStart")
    spawn_location = unreal.Vector(0, 0, 200)  # Spawn 200 units up
    spawn_rotation = unreal.Rotator(0, 0, 0)
    player_start = unreal.EditorLevelLibrary.spawn_actor_from_class(
        player_start_class,
        spawn_location,
        spawn_rotation
    )
    print(f"  CREATED PlayerStart at {spawn_location}")
else:
    print(f"[3/4] PlayerStart already exists: {player_starts[0].get_name()}")

# Check for existing ship in level
print("\n[4/4] Checking for ship in level...")
ships = [a for a in all_actors if "spaceship" in a.get_name().lower() or "vrspaceship" in a.get_name().lower()]

if not ships and ship_class:
    print("  No ship found - SPAWNING BP_VRSpaceshipPlayer...")
    spawn_location = unreal.Vector(0, 0, 300)  # Spawn above player start
    spawn_rotation = unreal.Rotator(0, 0, 0)

    try:
        ship = unreal.EditorLevelLibrary.spawn_actor_from_class(
            ship_class,
            spawn_location,
            spawn_rotation
        )
        print(f"  SUCCESS: Spawned ship at {spawn_location}")
    except Exception as e:
        print(f"  ERROR spawning ship: {e}")
elif ships:
    print(f"  Ship already in level: {ships[0].get_name()}")

print("\n" + "=" * 70)
print("SETUP COMPLETE")
print("=" * 70)
print("NEXT STEPS:")
print("1. Open Unreal Editor")
print("2. Make sure VRTemplateMap is loaded")
print("3. Press Alt+P to Play In Editor (PIE)")
print("4. Use VR controllers or WASD to fly")
print("=" * 70)
