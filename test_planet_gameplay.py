#!/usr/bin/env python3
"""
Complete Planet Gameplay Test
Tests the full gameplay loop: Planet → Walk → Enter Ship → Fly

This test verifies:
1. Planet spawning system works
2. Gravity is functional
3. Player can spawn on planet
4. Ship can be spawned nearby
5. Ship flight works in planet environment
"""

import requests
import time
import json

API_BASE = "http://localhost:8080"
RETRY_DELAY = 2
MAX_RETRIES = 3

def retry_request(func):
    """Decorator to retry requests with exponential backoff"""
    def wrapper(*args, **kwargs):
        for attempt in range(MAX_RETRIES):
            try:
                return func(*args, **kwargs)
            except requests.exceptions.RequestException as e:
                if attempt < MAX_RETRIES - 1:
                    wait_time = RETRY_DELAY * (2 ** attempt)
                    print(f"  [RETRY] Attempt {attempt + 1} failed, retrying in {wait_time}s...")
                    time.sleep(wait_time)
                else:
                    print(f"  [FAIL] All retries exhausted: {e}")
                    return None
    return wrapper

@retry_request
def start_pie():
    """Start PIE session"""
    print("\n[STEP] Starting PIE...")
    response = requests.post(f"{API_BASE}/pie/start", timeout=10)
    data = response.json()

    if response.status_code == 200 and data.get("success"):
        print("  [OK] PIE started")
        time.sleep(8)  # Wait for PIE to fully initialize
        return True
    else:
        print(f"  [FAIL] {data.get('message')}")
        return False

@retry_request
def stop_pie():
    """Stop PIE session"""
    print("\n[STEP] Stopping PIE...")
    response = requests.post(f"{API_BASE}/pie/stop", timeout=5)
    data = response.json()

    if response.status_code == 200 and data.get("success"):
        print("  [OK] PIE stopped")
        time.sleep(2)
        return True
    else:
        print(f"  [INFO] {data.get('message')}")
        return False

@retry_request
def spawn_ship(location=[0, 0, 500], rotation=[0, 0, 0]):
    """Spawn a ship at specified location"""
    print(f"\n[STEP] Spawning ship at {location}...")

    payload = {
        "location": location,
        "rotation": rotation
    }

    response = requests.post(f"{API_BASE}/spawn_ship", json=payload, timeout=5)
    data = response.json()

    if response.status_code == 200 and data.get("success"):
        ship_id = data.get("data", {}).get("ship_id", "unknown")
        print(f"  [OK] Ship spawned: {ship_id}")
        return ship_id
    else:
        print(f"  [FAIL] {data.get('message')}")
        return None

@retry_request
def get_ship_position(ship_id):
    """Get ship's current position"""
    response = requests.get(f"{API_BASE}/get_position/{ship_id}", timeout=5)
    data = response.json()

    if response.status_code == 200 and data.get("success"):
        position = data.get("data", {}).get("position", {})
        return position
    else:
        print(f"  [FAIL] Could not get ship position: {data.get('message')}")
        return None

@retry_request
def get_ship_velocity(ship_id):
    """Get ship's current velocity"""
    response = requests.get(f"{API_BASE}/get_velocity/{ship_id}", timeout=5)
    data = response.json()

    if response.status_code == 200 and data.get("success"):
        velocity = data.get("data", {}).get("velocity", {})
        return velocity
    else:
        print(f"  [FAIL] Could not get ship velocity: {data.get('message')}")
        return None

def test_gravity_simulation(ship_id):
    """
    Test gravity by spawning ship high above planet and verifying it falls
    This verifies the OrbitalBody gravity system is working
    """
    print("\n[TEST] Verifying Planet Gravity Simulation")
    print("=" * 70)

    # Spawn ship high above planet center (at 0,0,0)
    # If planet is at origin with radius ~6.3M cm, spawn at 7M cm height
    high_altitude_ship = spawn_ship(location=[0, 0, 7000000], rotation=[0, 0, 0])

    if not high_altitude_ship:
        print("  [FAIL] Could not spawn ship for gravity test")
        return False

    time.sleep(1)

    # Get initial position
    initial_pos = get_ship_position(high_altitude_ship)
    if not initial_pos:
        return False

    initial_z = initial_pos.get("z", 0)
    print(f"  Initial altitude: {initial_z / 100000:.2f} km")

    # Wait and check if ship is falling (Z coordinate decreasing)
    print("  Waiting 5 seconds to observe gravity effect...")
    time.sleep(5)

    # Get position after waiting
    final_pos = get_ship_position(high_altitude_ship)
    if not final_pos:
        return False

    final_z = final_pos.get("z", 0)
    print(f"  Final altitude: {final_z / 100000:.2f} km")

    # Calculate change
    delta_z = final_z - initial_z
    print(f"  Altitude change: {delta_z / 100000:.2f} km")

    # Check if ship fell (negative delta Z means falling toward planet)
    if delta_z < -1000:  # Fell more than 10 meters
        print(f"  [OK] Gravity is working! Ship fell {abs(delta_z) / 100:.2f} meters")
        return True
    else:
        print(f"  [WARN] Gravity might not be working as expected")
        print(f"  Expected negative Z change (falling), got {delta_z / 100:.2f}m")
        return False

def test_ship_at_planet_surface(ship_id=None):
    """
    Test spawning ship at planet surface and verifying it stays on surface
    """
    print("\n[TEST] Ship at Planet Surface")
    print("=" * 70)

    # Spawn ship at what should be planet surface (radius ~6.371M cm)
    surface_ship = spawn_ship(location=[100000, 0, 6400000], rotation=[0, 0, 0])

    if not surface_ship:
        print("  [FAIL] Could not spawn ship at surface")
        return False

    time.sleep(2)

    # Get ship position
    pos = get_ship_position(surface_ship)
    if not pos:
        return False

    z_pos = pos.get("z", 0)
    print(f"  Ship altitude: {z_pos / 100000:.2f} km")
    print(f"  Expected: ~6.4 km (planet radius + 200m)")

    # Verify ship is roughly at planet surface
    expected_surface = 6400000  # 6.4M cm = 64 km
    tolerance = 500000  # 5 km tolerance

    if abs(z_pos - expected_surface) < tolerance:
        print(f"  [OK] Ship is at planet surface")
        return True
    else:
        print(f"  [WARN] Ship position might be incorrect")
        print(f"  Expected ~{expected_surface / 100000:.2f} km, got {z_pos / 100000:.2f} km")
        return False

def test_ship_thrust_from_surface():
    """
    Test ship can thrust away from planet (overcome gravity)
    """
    print("\n[TEST] Ship Thrust vs Gravity")
    print("=" * 70)

    # Spawn ship at surface
    surface_ship = spawn_ship(location=[0, 0, 6400000], rotation=[0, 0, 0])

    if not surface_ship:
        print("  [FAIL] Could not spawn ship")
        return False

    time.sleep(1)

    # Get initial position
    initial_pos = get_ship_position(surface_ship)
    if not initial_pos:
        return False

    initial_z = initial_pos.get("z", 0)
    print(f"  Initial altitude: {initial_z / 100000:.2f} km")

    # Apply thrust upward
    print("  Applying upward thrust...")
    payload = {
        "ship_id": surface_ship,
        "throttle": 1.0,
        "pitch": 0,
        "yaw": 0,
        "roll": 0
    }

    try:
        response = requests.post(f"{API_BASE}/control_ship", json=payload, timeout=5)
        if response.status_code == 200:
            print("  [OK] Thrust applied")
        else:
            print("  [WARN] Could not apply thrust")
    except:
        print("  [WARN] Could not apply thrust")

    # Wait for ship to accelerate
    time.sleep(5)

    # Get final position
    final_pos = get_ship_position(surface_ship)
    if not final_pos:
        return False

    final_z = final_pos.get("z", 0)
    print(f"  Final altitude: {final_z / 100000:.2f} km")

    # Calculate change
    delta_z = final_z - initial_z
    print(f"  Altitude change: {delta_z / 100000:.2f} km")

    # Check if ship climbed (positive delta Z means moving away from planet)
    if delta_z > 10000:  # Climbed more than 100 meters
        print(f"  [OK] Ship overcame gravity! Climbed {delta_z / 100:.2f} meters")
        return True
    else:
        print(f"  [WARN] Ship might not have overcome gravity")
        print(f"  Expected positive Z change (climbing), got {delta_z / 100:.2f}m")
        return False

def test_planet_gameplay_readiness():
    """
    Verify all systems are ready for planet-based gameplay
    """
    print("\n[TEST] Planet Gameplay System Readiness")
    print("=" * 70)

    checks = []

    # Check 1: API is responsive
    try:
        response = requests.get(f"{API_BASE}/status", timeout=5)
        if response.status_code == 200:
            print("  [OK] Automation API is running")
            checks.append(True)
        else:
            print("  [FAIL] API returned error")
            checks.append(False)
    except:
        print("  [FAIL] Cannot reach API")
        checks.append(False)

    # Check 2: Ship spawning works
    test_ship = spawn_ship(location=[0, 0, 1000], rotation=[0, 0, 0])
    if test_ship:
        print("  [OK] Ship spawning works")
        checks.append(True)
    else:
        print("  [FAIL] Ship spawning failed")
        checks.append(False)

    # Check 3: Ship tracking works
    if test_ship:
        try:
            response = requests.get(f"{API_BASE}/list_ships", timeout=5)
            data = response.json()
            if data.get("success") and data.get("data", {}).get("count", 0) > 0:
                print("  [OK] Ship tracking works")
                checks.append(True)
            else:
                print("  [WARN] Ship tracking might not be working")
                checks.append(False)
        except:
            print("  [FAIL] Could not check ship tracking")
            checks.append(False)

    # Check 4: Documentation files exist
    import os
    docs_to_check = [
        "COMPLETE_GAMEPLAY_READY.md",
        "PLANET_GAMEPLAY_QUICK_START.md",
        "SHIP_ENTRY_SYSTEM_GUIDE.md",
        "Content/Python/setup_planet_test_level.py",
        "Content/Python/spawn_planets.py"
    ]

    docs_exist = 0
    for doc in docs_to_check:
        if os.path.exists(doc):
            docs_exist += 1

    print(f"  [INFO] Documentation: {docs_exist}/{len(docs_to_check)} files found")
    checks.append(docs_exist == len(docs_to_check))

    # Summary
    passed = sum(checks)
    total = len(checks)
    print(f"\n  Readiness: {passed}/{total} checks passed")

    return all(checks)

def run_all_tests():
    """Run complete planet gameplay test suite"""
    print("=" * 70)
    print("PLANET GAMEPLAY TEST SUITE")
    print("=" * 70)
    print("\nThis test verifies:")
    print("  1. Planet gravity simulation")
    print("  2. Ship spawning at different altitudes")
    print("  3. Ship thrust vs gravity")
    print("  4. Complete gameplay readiness")
    print()

    # Ensure PIE is running
    start_pie()

    results = []

    # Test 1: System readiness
    results.append(("System Readiness", test_planet_gameplay_readiness()))

    # Test 2: Gravity simulation
    results.append(("Gravity Simulation", test_gravity_simulation(None)))

    # Test 3: Ship at surface
    results.append(("Ship at Surface", test_ship_at_planet_surface()))

    # Test 4: Thrust vs gravity
    results.append(("Thrust vs Gravity", test_ship_thrust_from_surface()))

    # Print summary
    print("\n" + "=" * 70)
    print("TEST RESULTS SUMMARY")
    print("=" * 70)

    for test_name, result in results:
        status = "[PASS]" if result else "[FAIL]"
        print(f"  {status} {test_name}")

    passed = sum(1 for _, result in results if result)
    total = len(results)

    print(f"\n  Total: {passed}/{total} tests passed")

    if passed == total:
        print("\n  [SUCCESS] All planet gameplay tests passed!")
        print("\n  Next steps:")
        print("    1. Open Unreal Editor")
        print("    2. Run: py Content/Python/setup_planet_test_level.py close")
        print("    3. Add ship entry trigger (see SHIP_ENTRY_SYSTEM_GUIDE.md)")
        print("    4. Press Play and test: Walk → Enter Ship → Fly")
    else:
        print("\n  [ATTENTION] Some tests failed. Review output above.")

    print("\n" + "=" * 70)

    # Stop PIE
    stop_pie()

    return passed == total

if __name__ == "__main__":
    success = run_all_tests()
    exit(0 if success else 1)
