"""
Standalone Flight Test
Tests ship spawning and flight in STANDALONE GAME mode
Sends commands to running game via Python Remote Execution
"""

import remote_client
import time

print("="*70)
print("STANDALONE GAME FLIGHT TEST")
print("="*70)
print()
print("Prerequisites:")
print("  1. Game must be running (launch_standalone_game.bat)")
print("  2. Python Remote Execution enabled in Project Settings")
print()
input("Press Enter when game is loaded and ready...")

# Test 1: Connection
print("\n[TEST 1/5] Testing connection to game...")
if remote_client.test_connection():
    print("[OK] Connected to game")
else:
    print("[FAIL] Cannot connect - is game running?")
    exit(1)

time.sleep(2)

# Test 2: Spawn ship
print("\n[TEST 2/5] Spawning ship in game world...")

spawn_script = """
import unreal
import time

print("\\n" + "="*70)
print("SPAWNING SHIP IN GAME")
print("="*70)

# Get game world (not editor world!)
world = unreal.get_world()

if not world:
    print("[FAIL] No game world found")
else:
    print(f"[OK] Game world: {world.get_name()}")

    # Load ship blueprint
    ship_path = "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer_C"
    print(f"[INFO] Loading: {ship_path}")

    # Load as class
    ship_class = unreal.load_class(None, ship_path)

    if not ship_class:
        print(f"[FAIL] Could not load class: {ship_path}")
    else:
        print(f"[OK] Class loaded: {ship_class.get_name()}")

        # Spawn parameters
        spawn_location = unreal.Vector(0, 0, 300)
        spawn_rotation = unreal.Rotator(0, 0, 0)

        print(f"[INFO] Spawning at: ({spawn_location.x}, {spawn_location.y}, {spawn_location.z})")

        # Spawn in game world
        spawn_params = unreal.SpawnActorParameters()
        spawn_params.location = spawn_location
        spawn_params.rotation = spawn_rotation

        ship = unreal.GameplayStatics.begin_spawning_actor_from_class(
            world,
            ship_class,
            unreal.Transform(spawn_location, spawn_rotation, unreal.Vector(1, 1, 1))
        )

        if ship:
            unreal.GameplayStatics.finish_spawning_actor(ship, unreal.Transform(spawn_location, spawn_rotation, unreal.Vector(1, 1, 1)))

            print("\\n" + "="*70)
            print("[SUCCESS] SHIP SPAWNED IN GAME!")
            print("="*70)
            print(f"Ship: {ship.get_name()}")
            print(f"Location: {ship.get_actor_location()}")
            print("="*70)

            # Store ship reference globally so we can access it later
            unreal.set_editor_property(world, "test_ship", ship)
            print("[INFO] Ship stored for testing")

        else:
            print("[FAIL] Spawn failed")

print("\\nSpawn script complete")
"""

if remote_client.send_command(spawn_script):
    print("[OK] Spawn command sent")
    print("[INFO] Check game window - ship should appear")
    time.sleep(3)
else:
    print("[FAIL] Failed to send spawn command")
    exit(1)

# Test 3: Apply thrust
print("\n[TEST 3/5] Applying forward thrust...")

thrust_script = """
import unreal

print("\\n" + "="*70)
print("APPLYING THRUST")
print("="*70)

world = unreal.get_world()
if not world:
    print("[FAIL] No world")
else:
    # Find our spawned ship
    all_actors = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Actor.static_class())

    ship = None
    for actor in all_actors:
        if "BP_VRSpaceshipPlayer" in actor.get_name():
            ship = actor
            break

    if not ship:
        print("[FAIL] Ship not found in world")
    else:
        print(f"[OK] Found ship: {ship.get_name()}")

        # Record initial position
        initial_pos = ship.get_actor_location()
        print(f"[INFO] Initial position: ({initial_pos.x:.2f}, {initial_pos.y:.2f}, {initial_pos.z:.2f})")

        # Get FlightController
        flight_controller = ship.get_component_by_class(unreal.FlightController.static_class())

        if not flight_controller:
            print("[FAIL] No FlightController on ship")
        else:
            print("[OK] FlightController found")

            # Apply maximum forward thrust
            thrust = unreal.Vector(1.0, 0.0, 0.0)
            flight_controller.set_thrust_input(thrust)

            print(f"[OK] Forward thrust applied: {thrust}")
            print("[INFO] Ship should start moving forward (X+)")

            # Store initial position for later verification
            import unreal
            # We'll check position after a few seconds

print("\\nThrust applied - ship should be flying!")
"""

if remote_client.send_command(thrust_script):
    print("[OK] Thrust command sent")
    print("[INFO] Ship should now be moving forward")
    print("[INFO] Waiting 3 seconds for movement...")
    time.sleep(3)
else:
    print("[FAIL] Failed to send thrust command")
    exit(1)

# Test 4: Check position after movement
print("\n[TEST 4/5] Checking ship movement...")

check_script = """
import unreal

print("\\n" + "="*70)
print("CHECKING MOVEMENT")
print("="*70)

world = unreal.get_world()
if world:
    # Find ship
    all_actors = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Actor.static_class())

    ship = None
    for actor in all_actors:
        if "BP_VRSpaceshipPlayer" in actor.get_name():
            ship = actor
            break

    if ship:
        # Get current position and velocity
        position = ship.get_actor_location()
        velocity = ship.get_velocity()
        speed = velocity.length()

        print(f"[INFO] Current position: ({position.x:.2f}, {position.y:.2f}, {position.z:.2f})")
        print(f"[INFO] Velocity: ({velocity.x:.2f}, {velocity.y:.2f}, {velocity.z:.2f})")
        print(f"[INFO] Speed: {speed:.2f} units/sec")

        # Check if ship moved significantly from spawn point (0, 0, 300)
        spawn_point = unreal.Vector(0, 0, 300)
        distance_moved = (position - spawn_point).length()

        print(f"\\n[INFO] Distance from spawn: {distance_moved:.2f} units")

        if distance_moved > 50:  # Moved more than 50 units
            print("\\n" + "="*70)
            print("[SUCCESS] SHIP IS FLYING!")
            print("="*70)
            print(f"Ship moved {distance_moved:.2f} units from spawn point")
            print(f"Current speed: {speed:.2f} units/sec")
            print("="*70)
        elif speed > 10:  # Has velocity even if not far yet
            print("\\n[INFO] Ship has velocity but hasn't moved far yet")
            print(f"[INFO] Current speed: {speed:.2f} units/sec")
            print("[INFO] Ship is accelerating - SUCCESS!")
        else:
            print("\\n[WARN] Ship may not be moving")
            print(f"[WARN] Distance moved: {distance_moved:.2f}")
            print(f"[WARN] Speed: {speed:.2f}")
    else:
        print("[FAIL] Ship not found")
else:
    print("[FAIL] No world")
"""

if remote_client.send_command(check_script):
    print("[OK] Position check sent")
    print("[INFO] Check game log output above")
    time.sleep(2)
else:
    print("[FAIL] Failed to check position")

# Test 5: Summary
print("\n[TEST 5/5] Test summary...")
print()
print("="*70)
print("TEST COMPLETE")
print("="*70)
print()
print("Results should show in the game's log window:")
print("  - Look for '[SUCCESS] SHIP IS FLYING!'")
print("  - Check distance moved and speed values")
print()
print("Visual confirmation:")
print("  - Look at game window")
print("  - Ship should be visible and moving")
print()
print("To shutdown:")
print("  - Close game window")
print("  - Or run: taskkill /IM UnrealEditor.exe /F")
print()
print("="*70)
