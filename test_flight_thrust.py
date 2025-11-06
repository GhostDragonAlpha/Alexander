"""
Test Flight Thrust
Applies thrust to spawned ship and measures movement
Run this AFTER simple_spawn_test.py successfully spawned a ship
"""

import remote_client
import time

print("="*70)
print("FLIGHT THRUST TEST")
print("="*70)
print()

# Apply thrust test
print("[TEST] Applying forward thrust to ship...")

thrust_script = """
import unreal
import time

print("="*70)
print("FLIGHT THRUST TEST")
print("="*70)

# Find the ship we spawned
all_actors = unreal.EditorLevelLibrary.get_all_level_actors()

ship = None
for actor in all_actors:
    if "BP_VRSpaceshipPlayer" in actor.get_name():
        ship = actor
        break

if not ship:
    print("[FAIL] No ship found in level")
    print("[INFO] Available actors:")
    for actor in all_actors[:10]:  # Show first 10
        print(f"  - {actor.get_name()}")
else:
    print(f"[OK] Found ship: {ship.get_name()}")

    # Get initial position
    initial_pos = ship.get_actor_location()
    print(f"[INFO] Initial position: ({initial_pos.x:.2f}, {initial_pos.y:.2f}, {initial_pos.z:.2f})")

    # Get FlightController component
    flight_controller = ship.get_component_by_class(unreal.FlightController.static_class())

    if not flight_controller:
        print("[FAIL] Ship has no FlightController component")
    else:
        print(f"[OK] FlightController found")

        # Apply forward thrust
        thrust_input = unreal.Vector(1.0, 0.0, 0.0)  # Full forward thrust
        print(f"[INFO] Applying thrust: {thrust_input}")

        flight_controller.set_thrust_input(thrust_input)
        print("[OK] Thrust applied!")

        print()
        print("[INFO] Thrust is now active on the ship")
        print("[INFO] To see movement:")
        print("  1. Press Play (Alt+P) in the editor")
        print("  2. Wait a few seconds")
        print("  3. Press Escape to stop")
        print("  4. Check if ship moved from initial position")
        print()
        print(f"[INFO] Expected: Ship should move in +X direction (forward)")
        print(f"[INFO] Initial X: {initial_pos.x:.2f}")
        print(f"[INFO] After 3 seconds, X should be > {initial_pos.x + 100:.2f}")

print("="*70)
"""

if remote_client.send_command(thrust_script):
    print("[OK] Thrust command sent")
    print()
    print("="*70)
    print("NEXT STEPS:")
    print("="*70)
    print()
    print("1. Check Output Log - did it find the ship?")
    print("2. Press Alt+P (Play In Editor) to start physics simulation")
    print("3. Watch the ship - it should start moving forward (X direction)")
    print("4. After 3-5 seconds, press Escape to stop")
    print("5. Check if ship moved from initial position")
    print()
    print("If ship moves = SUCCESS!")
else:
    print("[FAIL] Failed to send command")
