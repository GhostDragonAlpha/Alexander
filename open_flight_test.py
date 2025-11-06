"""
Task T003: Get ship flying - Execute in RUNNING editor
This script runs INSIDE Unreal Editor via remote_client.py
"""

import unreal

print("=" * 70)
print("TASK T003: GETTING SHIP FLYING")
print("=" * 70)

# Get subsystems
editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
world = editor_subsystem.get_editor_world()
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

print(f"\n[1/4] Current World: {world.get_name()}")

# Load the ship blueprint
ship_bp_path = "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer"
print(f"\n[2/4] Loading ship blueprint: {ship_bp_path}")

try:
    ship_asset = unreal.EditorAssetLibrary.load_asset(ship_bp_path)
    if ship_asset:
        ship_class = ship_asset.generated_class()
        print(f"  ✓ Ship class loaded: {ship_class.get_name()}")
    else:
        print(f"  ✗ Failed to load ship blueprint")
        ship_class = None
except Exception as e:
    print(f"  ✗ Error: {e}")
    ship_class = None

# Check for existing ship in level
print(f"\n[3/4] Checking level for existing ship...")
all_actors = actor_subsystem.get_all_level_actors()
ships = [a for a in all_actors if "spaceship" in a.get_name().lower() or "vrspaceship" in a.get_name().lower()]

if ships:
    print(f"  ✓ Ship already in level: {ships[0].get_name()}")
elif ship_class:
    print(f"  → Spawning ship in level...")
    location = unreal.Vector(0, 0, 300)
    rotation = unreal.Rotator(0, 0, 0)

    try:
        ship = actor_subsystem.spawn_actor_from_class(ship_class, location, rotation)
        print(f"  ✓ Ship spawned: {ship.get_name()} at {location}")
    except Exception as e:
        print(f"  ✗ Error spawning ship: {e}")

# Set ship as default pawn
print(f"\n[4/4] Setting ship as default pawn...")
world_settings = world.get_world_settings()

if ship_class:
    try:
        world_settings.set_editor_property("default_pawn_class", ship_class)
        print(f"  ✓ Default pawn set to: {ship_class.get_name()}")

        # Save the level
        unreal.EditorLevelLibrary.save_current_level()
        print(f"  ✓ Level saved")

    except Exception as e:
        print(f"  ✗ Error: {e}")

print("\n" + "=" * 70)
print("TASK T003 COMPLETE")
print("=" * 70)
print("\nNEXT STEPS:")
print("  1. Look at Unreal Editor viewport")
print("  2. You should see the ship in the level")
print("  3. Press Alt+P to Play In Editor")
print("  4. You should spawn in the ship!")
print("=" * 70)

unreal.log("Task T003: Ship flight setup complete")
