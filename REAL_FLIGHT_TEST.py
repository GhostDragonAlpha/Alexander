"""
REAL FLIGHT TEST - With Actual Verification
A. Non-headless windowed game (you can see it)
B. Screenshot proof (visual verification)
C. Log file with results (text verification)
"""

import remote_client
import time
import json
from pathlib import Path

TEST_RESULTS_FILE = "test_results_REAL.json"
PROJECT_ROOT = Path(__file__).parent

print("="*70)
print("REAL FLIGHT TEST - WITH VERIFICATION")
print("="*70)
print()
print("This test will:")
print("  A. Run windowed game (you can watch)")
print("  B. Take screenshots as proof")
print("  C. Write results to", TEST_RESULTS_FILE)
print()
input("Press Enter when game is running and ready...")

# Test 1: Initialize test results file
print("\n[TEST 1/6] Initializing test results...")

init_test_script = """
import unreal
import json
from pathlib import Path

# Create test results dict
test_results = {
    "test_name": "REAL_FLIGHT_TEST",
    "timestamp": str(unreal.DateTime.now()),
    "steps": []
}

# Write to file
results_file = Path(r"C:/Users/allen/Desktop/Alexander/Alexander/test_results_REAL.json")
with open(results_file, 'w') as f:
    json.dump(test_results, f, indent=2)

unreal.log("[TEST] Test results file initialized")
print("[TEST] Test results file created:", results_file)
"""

if remote_client.send_command(init_test_script):
    print("[OK] Test initialization sent")
    time.sleep(1)
else:
    print("[FAIL] Could not initialize test")
    exit(1)

# Test 2: Spawn ship with verification
print("\n[TEST 2/6] Spawning ship...")

spawn_test_script = """
import unreal
import json
from pathlib import Path

print("\\n" + "="*70)
print("SPAWNING SHIP TEST")
print("="*70)

world = unreal.get_world()
result = {"step": "spawn_ship", "success": False, "message": ""}

if not world:
    result["message"] = "No game world found"
    print("[FAIL]", result["message"])
else:
    # Load ship class
    ship_path = "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer_C"
    ship_class = unreal.load_class(None, ship_path)

    if not ship_class:
        result["message"] = f"Could not load {ship_path}"
        print("[FAIL]", result["message"])
    else:
        # Spawn ship
        spawn_loc = unreal.Vector(0, 0, 500)  # Higher so we can see it
        spawn_rot = unreal.Rotator(0, 0, 0)
        transform = unreal.Transform(spawn_loc, spawn_rot, unreal.Vector(1, 1, 1))

        ship = unreal.GameplayStatics.begin_spawning_actor_from_class(world, ship_class, transform)

        if ship:
            unreal.GameplayStatics.finish_spawning_actor(ship, transform)

            actual_loc = ship.get_actor_location()
            result["success"] = True
            result["message"] = f"Ship spawned at ({actual_loc.x:.1f}, {actual_loc.y:.1f}, {actual_loc.z:.1f})"
            result["ship_name"] = ship.get_name()

            # Store ship for later tests
            world.set_editor_property("test_ship_name", ship.get_name())

            print("[SUCCESS]", result["message"])
            print("Ship name:", ship.get_name())
        else:
            result["message"] = "Spawn failed"
            print("[FAIL]", result["message"])

# Update results file
results_file = Path(r"C:/Users/allen/Desktop/Alexander/Alexander/test_results_REAL.json")
with open(results_file, 'r') as f:
    test_results = json.load(f)
test_results["steps"].append(result)
with open(results_file, 'w') as f:
    json.dump(test_results, f, indent=2)

print("Results written to file")
"""

if remote_client.send_command(spawn_test_script):
    print("[OK] Spawn test sent")
    print("[INFO] Waiting 2 seconds...")
    time.sleep(2)
else:
    print("[FAIL] Could not send spawn test")
    exit(1)

# Test 3: Take screenshot of spawned ship
print("\n[TEST 3/6] Taking screenshot...")

screenshot_script = """
import unreal

print("\\n[SCREENSHOT] Taking screenshot...")

# Take high-res screenshot
screenshot_name = "test_ship_spawned"
unreal.AutomationLibrary.take_automation_screenshot(screenshot_name, 0.0)

print("[OK] Screenshot requested:", screenshot_name)
print("Check: Saved/Screenshots/WindowsEditor/")
"""

if remote_client.send_command(screenshot_script):
    print("[OK] Screenshot command sent")
    time.sleep(2)
else:
    print("[FAIL] Could not take screenshot")

# Test 4: Apply thrust
print("\n[TEST 4/6] Applying thrust...")

thrust_test_script = """
import unreal
import json
from pathlib import Path

print("\\n" + "="*70)
print("THRUST TEST")
print("="*70)

world = unreal.get_world()
result = {"step": "apply_thrust", "success": False, "message": ""}

if world:
    # Get stored ship name
    ship_name = world.get_editor_property("test_ship_name")

    # Find ship
    all_actors = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Actor.static_class())
    ship = None
    for actor in all_actors:
        if ship_name in actor.get_name():
            ship = actor
            break

    if not ship:
        result["message"] = "Ship not found"
        print("[FAIL]", result["message"])
    else:
        # Record initial position
        initial_pos = ship.get_actor_location()
        result["initial_position"] = [initial_pos.x, initial_pos.y, initial_pos.z]

        # Get FlightController
        flight_controller = ship.get_component_by_class(unreal.FlightController.static_class())

        if not flight_controller:
            result["message"] = "No FlightController"
            print("[FAIL]", result["message"])
        else:
            # Apply forward thrust
            thrust = unreal.Vector(1.0, 0.0, 0.0)
            flight_controller.set_thrust_input(thrust)

            result["success"] = True
            result["message"] = "Thrust applied: Forward 1.0"
            result["thrust_vector"] = [1.0, 0.0, 0.0]

            print("[SUCCESS]", result["message"])
            print(f"Initial position: ({initial_pos.x:.1f}, {initial_pos.y:.1f}, {initial_pos.z:.1f})")
else:
    result["message"] = "No world"
    print("[FAIL]", result["message"])

# Update results
results_file = Path(r"C:/Users/allen/Desktop/Alexander/Alexander/test_results_REAL.json")
with open(results_file, 'r') as f:
    test_results = json.load(f)
test_results["steps"].append(result)
with open(results_file, 'w') as f:
    json.dump(test_results, f, indent=2)
"""

if remote_client.send_command(thrust_test_script):
    print("[OK] Thrust test sent")
    print("[INFO] Waiting 3 seconds for movement...")
    time.sleep(3)
else:
    print("[FAIL] Could not send thrust test")

# Test 5: Measure movement
print("\n[TEST 5/6] Measuring movement...")

measure_script = """
import unreal
import json
from pathlib import Path

print("\\n" + "="*70)
print("MOVEMENT MEASUREMENT")
print("="*70)

world = unreal.get_world()
result = {"step": "measure_movement", "success": False, "message": ""}

if world:
    ship_name = world.get_editor_property("test_ship_name")

    # Find ship
    all_actors = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Actor.static_class())
    ship = None
    for actor in all_actors:
        if ship_name in actor.get_name():
            ship = actor
            break

    if ship:
        # Get current state
        final_pos = ship.get_actor_location()
        velocity = ship.get_velocity()
        speed = velocity.length()

        # Calculate distance from spawn point (0, 0, 500)
        spawn_point = unreal.Vector(0, 0, 500)
        distance_moved = (final_pos - spawn_point).length()

        result["final_position"] = [final_pos.x, final_pos.y, final_pos.z]
        result["velocity"] = [velocity.x, velocity.y, velocity.z]
        result["speed"] = speed
        result["distance_moved"] = distance_moved

        # Determine success
        if distance_moved > 50 or speed > 10:
            result["success"] = True
            result["message"] = f"SHIP IS FLYING! Moved {distance_moved:.1f} units, speed {speed:.1f}"
            print("\\n" + "="*70)
            print("[SUCCESS] SHIP IS FLYING!")
            print("="*70)
            print(f"Distance moved: {distance_moved:.1f} units")
            print(f"Current speed: {speed:.1f} units/sec")
            print(f"Position: ({final_pos.x:.1f}, {final_pos.y:.1f}, {final_pos.z:.1f})")
            print(f"Velocity: ({velocity.x:.1f}, {velocity.y:.1f}, {velocity.z:.1f})")
            print("="*70)
        else:
            result["success"] = False
            result["message"] = f"Ship not moving enough. Distance: {distance_moved:.1f}, Speed: {speed:.1f}"
            print("[FAIL]", result["message"])
    else:
        result["message"] = "Ship not found"
        print("[FAIL]", result["message"])
else:
    result["message"] = "No world"
    print("[FAIL]", result["message"])

# Update results
results_file = Path(r"C:/Users/allen/Desktop/Alexander/Alexander/test_results_REAL.json")
with open(results_file, 'r') as f:
    test_results = json.load(f)
test_results["steps"].append(result)

# Mark overall test status
test_results["overall_success"] = result["success"]

with open(results_file, 'w') as f:
    json.dump(test_results, f, indent=2)

print("\\nFinal results written to file")
"""

if remote_client.send_command(measure_script):
    print("[OK] Measurement sent")
    time.sleep(2)
else:
    print("[FAIL] Could not measure movement")

# Test 6: Take final screenshot
print("\n[TEST 6/6] Taking final screenshot...")

final_screenshot_script = """
import unreal

print("\\n[SCREENSHOT] Taking final screenshot...")
unreal.AutomationLibrary.take_automation_screenshot("test_ship_flying", 0.0)
print("[OK] Final screenshot requested")
"""

if remote_client.send_command(final_screenshot_script):
    print("[OK] Final screenshot command sent")
    time.sleep(2)
else:
    print("[FAIL] Could not take final screenshot")

# Read and display results
print("\n" + "="*70)
print("READING ACTUAL TEST RESULTS")
print("="*70)

results_path = PROJECT_ROOT / TEST_RESULTS_FILE

if results_path.exists():
    with open(results_path, 'r') as f:
        results = json.load(f)

    print("\nTest Results from Game:")
    print(json.dumps(results, indent=2))

    print("\n" + "="*70)
    print("VERIFICATION SUMMARY")
    print("="*70)

    print(f"\nA. Visual: Game window shows ship")
    print(f"B. Screenshots: Check Saved/Screenshots/WindowsEditor/")
    print(f"   - test_ship_spawned.png")
    print(f"   - test_ship_flying.png")
    print(f"C. Log file: {TEST_RESULTS_FILE}")

    if results.get("overall_success"):
        print("\n[SUCCESS] *** SHIP ACTUALLY FLEW! ***")

        for step in results["steps"]:
            if step["step"] == "measure_movement" and step.get("success"):
                print(f"\nProof:")
                print(f"  Distance moved: {step['distance_moved']:.1f} units")
                print(f"  Speed: {step['speed']:.1f} units/sec")
                print(f"  Position: {step['final_position']}")
    else:
        print("\n[FAIL] Test did not succeed")
        print("Check individual steps above for details")

else:
    print(f"\n[FAIL] Results file not found: {results_path}")
    print("Commands may not have executed properly")

print("\n" + "="*70)
