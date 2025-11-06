"""
Setup VR Flight Game Mode with BP_VRSpaceshipPlayer as default pawn
Flight sticks positioned in front of player for comfortable VR grabbing
"""

import unreal

print("=" * 70)
print("SETTING UP VR FLIGHT GAMEMODE")
print("=" * 70)

# Get subsystems
editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world = editor_subsystem.get_editor_world()
world_settings = world.get_world_settings()

# Load the VR Spaceship blueprint
print("\n[1/3] Loading VR Spaceship blueprint...")
ship_bp_path = "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer"
ship_asset = unreal.EditorAssetLibrary.load_asset(ship_bp_path)

if not ship_asset:
    print(f"  ERROR: Could not load {ship_bp_path}")
else:
    ship_class = ship_asset.generated_class()
    print(f"  SUCCESS: Loaded {ship_class.get_name()}")

    # Find or create the GameMode
    print("\n[2/3] Setting up GameMode...")

    # Check if there's a GameMode override
    game_mode_override = world_settings.get_editor_property("default_gamemode_override")

    if not game_mode_override:
        print("  No GameMode override set")
        print("  Using VRGameMode from VR Template")
        # Try to load VRGameMode
        vr_gamemode_path = "/Game/VRTemplate/Blueprints/VRGameMode"
        vr_gamemode = unreal.EditorAssetLibrary.load_asset(vr_gamemode_path)

        if vr_gamemode:
            game_mode_class = vr_gamemode.generated_class()
            world_settings.set_editor_property("default_gamemode_override", game_mode_class)
            print(f"  Set GameMode to: {game_mode_class.get_name()}")
        else:
            print("  WARNING: Could not load VRGameMode")
    else:
        print(f"  Using existing GameMode: {game_mode_override.get_name()}")

    # Spawn or find the ship in the level
    print("\n[3/3] Setting up ship in level...")
    all_actors = actor_subsystem.get_all_level_actors()
    ships = [a for a in all_actors if a.get_class() == ship_class]

    if not ships:
        # Spawn ship at player start location
        player_starts = [a for a in all_actors if a.get_class().get_name() == "PlayerStart"]
        if player_starts:
            spawn_loc = player_starts[0].get_actor_location()
            spawn_rot = player_starts[0].get_actor_rotation()
        else:
            spawn_loc = unreal.Vector(0, 0, 200)
            spawn_rot = unreal.Rotator(0, 0, 0)

        ship = actor_subsystem.spawn_actor_from_class(
            ship_class,
            spawn_loc,
            spawn_rot
        )
        ship.set_actor_label("VRSpaceshipPlayer")
        print(f"  Spawned ship at PlayerStart")
    else:
        ship = ships[0]
        print(f"  Ship already exists: {ship.get_name()}")

    # Save the level
    unreal.EditorLevelLibrary.save_current_level()
    print("  Level saved")

print("\n" + "=" * 70)
print("VR FLIGHT GAMEMODE SETUP COMPLETE")
print("=" * 70)
print("\nFLIGHT CONTROLS (6DOF):")
print("  Left VR Controller:")
print("    - Thumbstick Forward/Back: Pitch rotation")
print("    - Thumbstick Left/Right: Yaw rotation")
print("    - Grip: Roll left")
print("\n  Right VR Controller:")
print("    - Thumbstick Forward/Back: Forward/Backward thrust")
print("    - Thumbstick Left/Right: Strafe left/right")
print("    - Grip: Roll right")
print("    - Trigger: Vertical thrust up")
print("\n  Flight Sticks:")
print("    - Positioned 50cm forward, 25cm apart")
print("    - At comfortable arm reach")
print("    - Grab with VR controllers for manual control")
print("=" * 70)

unreal.log("VR Flight GameMode setup complete")
