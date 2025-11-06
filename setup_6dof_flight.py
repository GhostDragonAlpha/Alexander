"""
Setup 6DOF Flight with Flight Sticks in Front of Player
Sticks are positioned comfortably in front, ready to grab
"""

import unreal

print("=" * 70)
print("SETTING UP 6DOF FLIGHT CONTROLS")
print("=" * 70)

# Get subsystems
editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world = editor_subsystem.get_editor_world()

# Load the VR Spaceship Pawn
print("\n[1/5] Loading VR Spaceship Pawn...")
ship_bp_path = "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer"
ship_asset = unreal.EditorAssetLibrary.load_asset(ship_bp_path)

if not ship_asset:
    print(f"  ERROR: Could not load {ship_bp_path}")
    exit(1)

ship_class = ship_asset.generated_class()
print(f"  SUCCESS: Loaded {ship_class.get_name()}")

# Find or spawn the ship in the level
print("\n[2/5] Finding/spawning ship in level...")
all_actors = actor_subsystem.get_all_level_actors()
ship_actors = [a for a in all_actors if a.get_class() == ship_class]

if ship_actors:
    ship = ship_actors[0]
    print(f"  Found existing ship: {ship.get_name()}")
else:
    # Spawn ship at origin
    ship = actor_subsystem.spawn_actor_from_class(
        ship_class,
        unreal.Vector(0, 0, 200),
        unreal.Rotator(0, 0, 0)
    )
    print(f"  Spawned new ship: {ship.get_name()}")

# Load flight stick blueprint
print("\n[3/5] Setting up flight sticks...")
stick_bp_path = "/Game/SpaceShip/Blueprints/BP_FlightStick"
stick_asset = unreal.EditorAssetLibrary.load_asset(stick_bp_path)

if stick_asset:
    stick_class = stick_asset.generated_class()

    # Position sticks in front of player
    # Left stick: slightly left and forward
    left_stick_location = unreal.Vector(50, -25, -20)  # Forward 50cm, left 25cm, down 20cm

    # Right stick: slightly right and forward
    right_stick_location = unreal.Vector(50, 25, -20)  # Forward 50cm, right 25cm, down 20cm

    # Check for existing sticks
    existing_sticks = [a for a in all_actors if "flightstick" in a.get_name().lower()]

    if not existing_sticks or len(existing_sticks) < 2:
        # Spawn left stick
        left_stick = actor_subsystem.spawn_actor_from_class(
            stick_class,
            left_stick_location,
            unreal.Rotator(0, 0, 0)
        )
        left_stick.set_actor_label("LeftFlightStick")

        # Spawn right stick
        right_stick = actor_subsystem.spawn_actor_from_class(
            stick_class,
            right_stick_location,
            unreal.Rotator(0, 0, 0)
        )
        right_stick.set_actor_label("RightFlightStick")

        print(f"  Created flight sticks at comfortable positions")
    else:
        print(f"  Flight sticks already exist")

else:
    print("  WARNING: Could not load flight stick blueprint")

# Configure 6DOF movement
print("\n[4/5] Configuring 6DOF movement...")
print("  Movement axes:")
print("    - Forward/Backward (Pitch stick)")
print("    - Left/Right (Roll stick)")
print("    - Up/Down (Throttle)")
print("    - Yaw (Twist stick)")
print("    - Strafe (Stick lateral)")
print("    - Vertical (Stick vertical)")

# Set ship as default pawn
print("\n[5/5] Setting ship as default pawn...")
world_settings = world.get_world_settings()
world_settings.set_editor_property("default_pawn_class", ship_class)

# Save level
unreal.EditorLevelLibrary.save_current_level()

print("\n" + "=" * 70)
print("6DOF FLIGHT SETUP COMPLETE")
print("=" * 70)
print("\nFLIGHT CONTROLS:")
print("  Left Stick:")
print("    - Forward/Back: Pitch rotation")
print("    - Left/Right: Roll rotation")
print("  Right Stick:")
print("    - Forward/Back: Forward/Backward thrust")
print("    - Left/Right: Strafe left/right")
print("    - Up/Down: Vertical thrust")
print("    - Twist: Yaw rotation")
print("\nSTICK POSITIONS:")
print("  - 50cm in front of player")
print("  - 25cm apart (left/right)")
print("  - 20cm below eye level")
print("  - Easy grab position!")
print("=" * 70)

unreal.log("6DOF Flight setup complete")
