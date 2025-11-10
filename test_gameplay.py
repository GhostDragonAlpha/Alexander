#!/usr/bin/env python3
"""
Comprehensive gameplay test - Tests actual game functionality
Uses automation API to verify game systems work correctly
"""

import requests
import time
import sys

API_BASE = "http://localhost:8080"
MAX_RETRIES = 3  # Retry up to 3 times on transient failures
RETRY_DELAYS = [0.5, 1.0, 2.0]  # Exponential backoff delays

def retry_request(func):
    """
    Decorator that retries a function with exponential backoff on connection errors.
    Handles transient failures like connection aborts (errno 10053).
    """
    def wrapper(*args, **kwargs):
        last_exception = None
        for attempt in range(MAX_RETRIES):
            try:
                return func(*args, **kwargs)
            except requests.exceptions.ConnectionError as e:
                last_exception = e
                if attempt < MAX_RETRIES - 1:
                    delay = RETRY_DELAYS[attempt]
                    print(f"    [RETRY] Connection error, retrying in {delay}s... (attempt {attempt + 1}/{MAX_RETRIES})")
                    time.sleep(delay)
                else:
                    print(f"    [RETRY FAILED] All {MAX_RETRIES} attempts failed")
            except Exception as e:
                # Don't retry on other exceptions (like JSON errors, validation errors)
                raise
        # If we exhausted all retries, raise the last exception
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
            time.sleep(8)  # Wait for PIE to fully start
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

def send_ship_input_with_retry(ship_id, name, inputs):
    """Helper function to send ship input with retry logic"""
    @retry_request
    def send_input():
        payload = {"ship_id": ship_id, **inputs}
        response = requests.post(f"{API_BASE}/set_input", json=payload, timeout=5)
        data = response.json()

        if response.status_code == 200 and data.get("success"):
            return True, "Success"
        else:
            return False, data.get('message', 'Unknown error')

    try:
        success, message = send_input()
        if success:
            print(f"  [OK] {name}: {message}")
        else:
            print(f"  [FAIL] {name}: {message}")
        return success
    except Exception as e:
        print(f"  [FAIL] {name}: {e}")
        return False

def test_ship_controls(ship_id):
    """Test ship control inputs"""
    print(f"\n[STEP] Testing ship controls for {ship_id}...")

    test_inputs = [
        ("Forward thrust", {"throttle": 0.5, "pitch": 0, "yaw": 0, "roll": 0}),
        ("Pitch up", {"throttle": 0, "pitch": 1.0, "yaw": 0, "roll": 0}),
        ("Yaw right", {"throttle": 0, "pitch": 0, "yaw": 1.0, "roll": 0}),
        ("Roll left", {"throttle": 0, "pitch": 0, "yaw": 0, "roll": -1.0}),
    ]

    results = []
    for name, inputs in test_inputs:
        result = send_ship_input_with_retry(ship_id, name, inputs)
        results.append(result)
        if result:
            time.sleep(0.5)  # Brief delay between inputs

    return all(results)

@retry_request
def test_ship_position(ship_id):
    """Test getting ship position"""
    print(f"\n[STEP] Testing position tracking for {ship_id}...")
    response = requests.get(f"{API_BASE}/get_position/{ship_id}", timeout=5)
    data = response.json()

    if response.status_code == 200 and data.get("success"):
        position = data.get("data", {}).get("position", {})
        print(f"  [OK] Position: X={position.get('x'):.1f}, Y={position.get('y'):.1f}, Z={position.get('z'):.1f}")
        return True
    else:
        print(f"  [FAIL] {data.get('message')}")
        return False

@retry_request
def test_ship_velocity(ship_id):
    """Test getting ship velocity"""
    print(f"\n[STEP] Testing velocity tracking for {ship_id}...")
    response = requests.get(f"{API_BASE}/get_velocity/{ship_id}", timeout=5)
    data = response.json()

    if response.status_code == 200 and data.get("success"):
        velocity = data.get("data", {}).get("velocity", {})
        speed = data.get("data", {}).get("speed", 0)
        print(f"  [OK] Velocity: X={velocity.get('x'):.1f}, Y={velocity.get('y'):.1f}, Z={velocity.get('z'):.1f}")
        print(f"       Speed: {speed:.1f} units/s")
        return True
    else:
        print(f"  [FAIL] {data.get('message')}")
        return False

@retry_request
def test_ship_list():
    """Test listing all ships"""
    print(f"\n[STEP] Testing ship list...")
    response = requests.get(f"{API_BASE}/list_ships", timeout=5)
    data = response.json()

    if response.status_code == 200 and data.get("success"):
        ships = data.get("data", {}).get("ships", [])
        print(f"  [OK] Found {len(ships)} ship(s)")
        for ship in ships:
            print(f"       - {ship.get('ship_id')}: {ship.get('ship_name')}")
        return True
    else:
        print(f"  [FAIL] {data.get('message')}")
        return False

def main():
    print("="*70)
    print("AUTOMATED GAMEPLAY TESTING - ITERATIVE DEVELOPMENT")
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

    # Run gameplay tests
    tests = []

    # Test ship systems
    tests.append(("Ship List", test_ship_list()))
    tests.append(("Ship Position", test_ship_position(ship_id)))
    tests.append(("Ship Velocity", test_ship_velocity(ship_id)))
    tests.append(("Ship Controls", test_ship_controls(ship_id)))

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
        print("\n*** ALL GAMEPLAY TESTS PASSED ***")
        print("Game systems are working correctly!")
    else:
        print(f"\n*** {total - passed} TEST(S) FAILED ***")
        print("Issues found - need to fix game systems")

    sys.exit(0 if passed == total else 1)

if __name__ == "__main__":
    main()
