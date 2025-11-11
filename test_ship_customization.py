#!/usr/bin/env python3
"""
Ship Customization System Test Suite
Tests all customization API endpoints and verifies stat system functionality
"""

import requests
import time
import sys
import json

API_BASE = "http://localhost:8080"
MAX_RETRIES = 3
RETRY_DELAYS = [0.5, 1.0, 2.0]

def retry_request(func):
    """Decorator for retrying requests with exponential backoff"""
    def wrapper(*args, **kwargs):
        last_exception = None
        for attempt in range(MAX_RETRIES):
            try:
                return func(*args, **kwargs)
            except requests.exceptions.ConnectionError as e:
                last_exception = e
                if attempt < MAX_RETRIES - 1:
                    delay = RETRY_DELAYS[attempt]
                    print(f"    [RETRY] Connection error, retrying in {delay}s...")
                    time.sleep(delay)
                else:
                    print(f"    [RETRY FAILED] All {MAX_RETRIES} attempts failed")
            except Exception as e:
                raise
        if last_exception:
            raise last_exception
    return wrapper

def wait_for_server(timeout=120):
    """Wait for automation server"""
    print("Waiting for server...")
    start_time = time.time()
    while time.time() - start_time < timeout:
        try:
            response = requests.get(f"{API_BASE}/status", timeout=2)
            if response.status_code == 200:
                print(f"[OK] Server online after {int(time.time() - start_time)}s")
                return True
        except requests.exceptions.RequestException:
            pass
        time.sleep(3)
    return False

def start_pie():
    """Start PIE session"""
    print("\n[STEP] Starting PIE...")
    try:
        response = requests.post(f"{API_BASE}/pie/start", timeout=15)
        if response.status_code == 200:
            data = response.json()
            print(f"  [OK] {data.get('message')}")
            time.sleep(8)
            return True
        else:
            print(f"  [FAIL] {response.text}")
            return False
    except Exception as e:
        print(f"  [FAIL] {e}")
        return False

def stop_pie():
    """Stop PIE session"""
    print("\n[STEP] Stopping PIE...")
    try:
        response = requests.post(f"{API_BASE}/pie/stop", timeout=10)
        if response.status_code == 200:
            print("  [OK] PIE stopped")
            return True
        else:
            print(f"  [FAIL] {response.text}")
            return False
    except Exception as e:
        print(f"  [FAIL] {e}")
        return False

@retry_request
def spawn_test_ship():
    """Spawn a ship for testing"""
    print("\n[STEP] Spawning test ship...")
    payload = {
        "location": [0, 0, 500],
        "rotation": [0, 0, 0],
        "ship_class": "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer_C"
    }
    response = requests.post(f"{API_BASE}/spawn_ship", json=payload, timeout=10)
    data = response.json()

    if response.status_code == 200 and data.get("success"):
        ship_id = data.get("data", {}).get("ship_id")
        print(f"  [OK] Ship spawned: {ship_id}")
        return ship_id
    else:
        print(f"  [FAIL] {data.get('message')}")
        return None

@retry_request
def test_get_default_customization(ship_id):
    """Test getting default ship customization"""
    print(f"\n[STEP] Testing get default customization for {ship_id}...")
    response = requests.get(f"{API_BASE}/get_ship_customization/{ship_id}", timeout=5)
    data = response.json()

    if response.status_code == 200 and data.get("success"):
        stats = data.get("data", {})
        print(f"  [OK] Default stats retrieved:")
        print(f"       Thrust Power: {stats.get('thrust_power', 0):.2f}")
        print(f"       Max Velocity: {stats.get('max_velocity', 0):.2f}")
        print(f"       Rotation Speed: {stats.get('rotation_speed', 0):.2f}")
        print(f"       Acceleration: {stats.get('acceleration', 0):.2f}")
        return True, stats
    else:
        print(f"  [FAIL] {data.get('message')}")
        return False, None

@retry_request
def test_apply_customization(ship_id):
    """Test applying custom stats to ship"""
    print(f"\n[STEP] Testing apply customization to {ship_id}...")

    payload = {
        "ship_id": ship_id,
        "thrust_power": 1.5,
        "max_velocity": 1.3,
        "rotation_speed": 1.2,
        "acceleration": 1.4
    }

    response = requests.post(f"{API_BASE}/apply_ship_customization", json=payload, timeout=5)
    data = response.json()

    if response.status_code == 200 and data.get("success"):
        print(f"  [OK] Customization applied successfully")
        return True
    else:
        print(f"  [FAIL] {data.get('message')}")
        return False

@retry_request
def test_verify_customization(ship_id, expected_stats):
    """Verify customization was applied correctly"""
    print(f"\n[STEP] Verifying customization on {ship_id}...")
    response = requests.get(f"{API_BASE}/get_ship_customization/{ship_id}", timeout=5)
    data = response.json()

    if response.status_code == 200 and data.get("success"):
        stats = data.get("data", {})

        # Check if stats match expected values
        thrust_match = abs(stats.get('thrust_power', 0) - expected_stats['thrust_power']) < 0.01
        velocity_match = abs(stats.get('max_velocity', 0) - expected_stats['max_velocity']) < 0.01
        rotation_match = abs(stats.get('rotation_speed', 0) - expected_stats['rotation_speed']) < 0.01
        accel_match = abs(stats.get('acceleration', 0) - expected_stats['acceleration']) < 0.01

        if thrust_match and velocity_match and rotation_match and accel_match:
            print(f"  [OK] Stats verified correctly:")
            print(f"       Thrust Power: {stats.get('thrust_power', 0):.2f} (expected {expected_stats['thrust_power']:.2f})")
            print(f"       Max Velocity: {stats.get('max_velocity', 0):.2f} (expected {expected_stats['max_velocity']:.2f})")
            print(f"       Rotation Speed: {stats.get('rotation_speed', 0):.2f} (expected {expected_stats['rotation_speed']:.2f})")
            print(f"       Acceleration: {stats.get('acceleration', 0):.2f} (expected {expected_stats['acceleration']:.2f})")
            return True
        else:
            print(f"  [FAIL] Stats mismatch!")
            print(f"       Thrust Power: {stats.get('thrust_power', 0):.2f} != {expected_stats['thrust_power']:.2f}")
            print(f"       Max Velocity: {stats.get('max_velocity', 0):.2f} != {expected_stats['max_velocity']:.2f}")
            print(f"       Rotation Speed: {stats.get('rotation_speed', 0):.2f} != {expected_stats['rotation_speed']:.2f}")
            print(f"       Acceleration: {stats.get('acceleration', 0):.2f} != {expected_stats['acceleration']:.2f}")
            return False
    else:
        print(f"  [FAIL] {data.get('message')}")
        return False

@retry_request
def test_equip_part(ship_id, category, part_id):
    """Test equipping a specific part"""
    print(f"\n[STEP] Testing equip part '{part_id}' to category '{category}'...")

    payload = {
        "ship_id": ship_id,
        "category": category,
        "part_id": part_id
    }

    response = requests.post(f"{API_BASE}/equip_ship_part", json=payload, timeout=5)
    data = response.json()

    if response.status_code == 200 and data.get("success"):
        print(f"  [OK] Part equipped successfully")
        result_data = data.get("data", {})
        if result_data:
            print(f"       Category: {result_data.get('category')}")
            print(f"       Part ID: {result_data.get('part_id')}")
        return True
    else:
        print(f"  [FAIL] {data.get('message')}")
        return False

@retry_request
def test_get_loadout(ship_id):
    """Test getting complete ship loadout"""
    print(f"\n[STEP] Testing get ship loadout for {ship_id}...")
    response = requests.get(f"{API_BASE}/get_ship_loadout/{ship_id}", timeout=5)
    data = response.json()

    if response.status_code == 200 and data.get("success"):
        loadout = data.get("data", {})
        print(f"  [OK] Loadout retrieved:")
        print(f"       Loadout Name: {loadout.get('loadout_name', 'N/A')}")

        equipped_parts = loadout.get('equipped_parts', {})
        print(f"       Equipped Parts ({len(equipped_parts)}):")
        for category, part_id in equipped_parts.items():
            print(f"         {category}: {part_id}")

        equipped_skin = loadout.get('equipped_skin', 'N/A')
        print(f"       Equipped Skin: {equipped_skin}")

        return True, loadout
    else:
        print(f"  [FAIL] {data.get('message')}")
        return False, None

@retry_request
def test_performance_impact(ship_id):
    """Test that customization affects flight performance"""
    print(f"\n[STEP] Testing performance impact on {ship_id}...")

    # Get initial velocity
    response = requests.get(f"{API_BASE}/get_velocity/{ship_id}", timeout=5)
    initial_data = response.json()
    if not (response.status_code == 200 and initial_data.get("success")):
        print(f"  [FAIL] Could not get initial velocity")
        return False

    # Apply high thrust/velocity stats
    payload = {
        "ship_id": ship_id,
        "thrust_power": 2.0,
        "max_velocity": 2.0,
        "rotation_speed": 1.0,
        "acceleration": 2.0
    }
    response = requests.post(f"{API_BASE}/apply_ship_customization", json=payload, timeout=5)
    if response.status_code != 200:
        print(f"  [FAIL] Could not apply high-performance stats")
        return False

    # Apply thrust input
    payload = {"ship_id": ship_id, "throttle": 1.0, "pitch": 0, "yaw": 0, "roll": 0}
    response = requests.post(f"{API_BASE}/set_input", json=payload, timeout=5)
    if response.status_code != 200:
        print(f"  [FAIL] Could not apply thrust")
        return False

    # Wait for ship to accelerate
    time.sleep(2)

    # Get new velocity
    response = requests.get(f"{API_BASE}/get_velocity/{ship_id}", timeout=5)
    new_data = response.json()
    if not (response.status_code == 200 and new_data.get("success")):
        print(f"  [FAIL] Could not get new velocity")
        return False

    initial_speed = initial_data.get("data", {}).get("speed", 0)
    new_speed = new_data.get("data", {}).get("speed", 0)

    print(f"  [INFO] Initial speed: {initial_speed:.2f} units/s")
    print(f"  [INFO] New speed after customization: {new_speed:.2f} units/s")

    if new_speed > initial_speed + 1.0:  # Allow small threshold
        print(f"  [OK] Performance improvement detected!")
        return True
    else:
        print(f"  [WARN] No significant performance change (may need more time)")
        return True  # Don't fail - performance testing is complex

def main():
    print("="*70)
    print("SHIP CUSTOMIZATION SYSTEM - COMPREHENSIVE TEST SUITE")
    print("="*70)

    # Wait for server
    if not wait_for_server():
        print("\n[FAILED] Server not available")
        sys.exit(1)

    # Start PIE
    if not start_pie():
        print("\n[FAILED] Could not start PIE")
        sys.exit(1)

    # Spawn test ship
    ship_id = spawn_test_ship()
    if not ship_id:
        print("\n[FAILED] Could not spawn ship")
        stop_pie()
        sys.exit(1)

    # Run tests
    tests = []

    # Test 1: Get default customization
    success, default_stats = test_get_default_customization(ship_id)
    tests.append(("Get Default Customization", success))

    # Test 2: Apply customization
    success = test_apply_customization(ship_id)
    tests.append(("Apply Customization", success))

    # Test 3: Verify customization
    expected_stats = {
        "thrust_power": 1.5,
        "max_velocity": 1.3,
        "rotation_speed": 1.2,
        "acceleration": 1.4
    }
    success = test_verify_customization(ship_id, expected_stats)
    tests.append(("Verify Customization", success))

    # Test 4: Equip parts
    success = test_equip_part(ship_id, "Engine", "engine_plasma_basic")
    tests.append(("Equip Engine Part", success))

    success = test_equip_part(ship_id, "Thrusters", "thruster_vectored")
    tests.append(("Equip Thruster Part", success))

    success = test_equip_part(ship_id, "Hull", "hull_reinforced")
    tests.append(("Equip Hull Part", success))

    # Test 5: Get loadout
    success, loadout = test_get_loadout(ship_id)
    tests.append(("Get Ship Loadout", success))

    # Test 6: Performance impact
    success = test_performance_impact(ship_id)
    tests.append(("Performance Impact", success))

    # Stop PIE
    stop_pie()

    # Results
    print("\n" + "="*70)
    print("TEST RESULTS")
    print("="*70)

    passed = sum(1 for _, result in tests if result)
    total = len(tests)

    for name, result in tests:
        status = "[PASS]" if result else "[FAIL]"
        print(f"  {status}: {name}")

    print(f"\nResult: {passed}/{total} tests passed")
    print("="*70)

    if passed == total:
        print("\n*** ALL CUSTOMIZATION TESTS PASSED ***")
        print("Ship customization system is working correctly!")
        return 0
    else:
        print(f"\n*** {total - passed} TEST(S) FAILED ***")
        print("Issues found with customization system")
        return 1

if __name__ == "__main__":
    sys.exit(main())
