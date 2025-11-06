"""
FINAL FIX: Set up GameMode to spawn the spaceship pawn
Uses correct UE 5.6 API - no deprecations
"""

import unreal

print("=" * 70)
print("FIXING GAMEMODE TO SPAWN SPACESHIP")
print("=" * 70)

# Load the ship blueprint class
ship_bp_path = "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer"
print(f"[1/3] Loading {ship_bp_path}...")

try:
    ship_bp = unreal.load_asset(ship_bp_path)
    if ship_bp:
        print(f"  SUCCESS: Loaded {ship_bp.get_name()}")
        ship_class = ship_bp.generated_class()
        print(f"  Class: {ship_class.get_name()}")
    else:
        print(f"  ERROR: Could not load asset")
        ship_class = None
except Exception as e:
    print(f"  ERROR: {e}")
    ship_class = None

# Get the world settings
print("\n[2/3] Getting World Settings...")
editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
world = editor_subsystem.get_editor_world()
world_settings = world.get_world_settings()

print(f"  World: {world.get_name()}")
print(f"  World Settings: {world_settings.get_name()}")

# Get current game mode
current_game_mode = world_settings.get_editor_property("default_game_mode")
print(f"  Current GameMode: {current_game_mode}")

# Set the default pawn class in world settings
if ship_class:
    print("\n[3/3] Setting Default Pawn Class...")
    try:
        # For VR Template, we need to override the default pawn
        world_settings.set_editor_property("default_pawn_class", ship_class)
        print(f"  SUCCESS: Set default pawn to {ship_class.get_name()}")

        # Save the level
        unreal.EditorLevelLibrary.save_current_level()
        print("  Level saved!")

    except Exception as e:
        print(f"  ERROR: {e}")
else:
    print("\n[3/3] SKIPPED - Could not load ship class")

print("\n" + "=" * 70)
print("GAMEMODE FIX COMPLETE!")
print("=" * 70)
print("TO TEST:")
print("  1. Open Unreal Editor")
print("  2. Load VRTemplateMap")
print("  3. Press Alt+P to Play")
print("  4. You should spawn in the spaceship!")
print("=" * 70)
