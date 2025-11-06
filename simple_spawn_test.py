"""
Simple Ship Spawn Test
Sends command to RUNNING editor to spawn and test ship
Run this AFTER starting the editor with start_editor_with_remote.bat
"""

import remote_client
import time

print("="*70)
print("SIMPLE SHIP SPAWN TEST")
print("="*70)
print()
print("Make sure the Unreal Editor is already running!")
print("If not, run: start_editor_with_remote.bat")
print()
input("Press Enter when editor is loaded and ready...")

# Test 1: Connection test
print("\n[TEST 1] Testing connection to editor...")
if remote_client.test_connection():
    print("[OK] Connection successful")
else:
    print("[FAIL] Connection failed - is editor running?")
    exit(1)

time.sleep(2)

# Test 2: Spawn ship
print("\n[TEST 2] Spawning ship...")

spawn_script = """
import unreal

print("="*70)
print("SPAWNING SHIP")
print("="*70)

# Get editor world
editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world = editor_subsystem.get_editor_world()

if not world:
    print("[FAIL] No editor world found")
else:
    print(f"[OK] Editor world: {world.get_name()}")

    # Load ship blueprint
    ship_path = "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer"
    print(f"[INFO] Loading blueprint: {ship_path}")

    ship_asset = unreal.EditorAssetLibrary.load_asset(ship_path)

    if not ship_asset:
        print(f"[FAIL] Could not load blueprint: {ship_path}")
    else:
        print(f"[OK] Blueprint loaded: {ship_asset.get_name()}")

        # Get blueprint class
        ship_class = ship_asset.generated_class()
        print(f"[OK] Got class: {ship_class.get_name()}")

        # Spawn location
        spawn_location = unreal.Vector(0, 0, 300)
        spawn_rotation = unreal.Rotator(0, 0, 0)

        print(f"[INFO] Spawning at location: {spawn_location}")

        # Spawn ship
        ship = actor_subsystem.spawn_actor_from_class(
            ship_class,
            spawn_location,
            spawn_rotation
        )

        if ship:
            print("="*70)
            print("[SUCCESS] SHIP SPAWNED!")
            print("="*70)
            print(f"Ship Name: {ship.get_name()}")
            print(f"Ship Class: {ship.get_class().get_name()}")
            print(f"Location: {ship.get_actor_location()}")
            print("="*70)

            # Check for FlightController
            flight_controller = ship.get_component_by_class(unreal.FlightController.static_class())
            if flight_controller:
                print(f"[OK] FlightController found on ship")
            else:
                print(f"[WARN] No FlightController on ship")

        else:
            print("[FAIL] Failed to spawn ship")

print()
print("Script execution complete")
"""

if remote_client.send_command(spawn_script):
    print("[OK] Spawn command sent to editor")
    print()
    print("Check the Unreal Editor's Output Log to see results!")
    print("Look for '[SUCCESS] SHIP SPAWNED!' message")
else:
    print("[FAIL] Failed to send spawn command")

print()
print("="*70)
print("TEST COMPLETE")
print("="*70)
print()
print("To test flight:")
print("  1. Check editor - ship should be spawned at (0, 0, 300)")
print("  2. You can press Play in editor to see if ship flies")
print("  3. Or run more Python commands to apply thrust and test")
