"""
Complete HTTP API Workflow Test
Tests all AutomationAPIServer endpoints in sequence

Prerequisites:
- Game running with AutomationGameMode
- HTTP server listening on port 8080
"""

import requests
import json
import time
from datetime import datetime

SERVER_URL = "http://localhost:8080"

def print_section(title):
    """Print formatted section header"""
    print("\n" + "=" * 70)
    print(f" {title}")
    print("=" * 70)

def test_status():
    """Test GET /status endpoint"""
    print_section("TEST 1: GET /status")

    try:
        response = requests.get(f"{SERVER_URL}/status", timeout=5)
        result = response.json()

        print(f"Status Code: {response.status_code}")
        print(f"Response: {json.dumps(result, indent=2)}")

        if response.status_code == 200:
            print("[✓] Status endpoint working")
            return True
        else:
            print("[✗] Status endpoint failed")
            return False

    except Exception as e:
        print(f"[✗] Error: {e}")
        return False

def test_spawn_ship():
    """Test POST /spawn_ship endpoint"""
    print_section("TEST 2: POST /spawn_ship")

    payload = {
        "ship_class": "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer_C",
        "location": {"x": 1000, "y": 0, "z": 500},
        "rotation": {"pitch": 0, "yaw": 45, "roll": 0}
    }

    print(f"Payload: {json.dumps(payload, indent=2)}")

    try:
        response = requests.post(f"{SERVER_URL}/spawn_ship", json=payload, timeout=5)
        result = response.json()

        print(f"Status Code: {response.status_code}")
        print(f"Response: {json.dumps(result, indent=2)}")

        if response.status_code == 200 and result.get("success"):
            ship_id = result.get("data", {}).get("ship_id")
            print(f"[✓] Ship spawned successfully: {ship_id}")
            return ship_id
        else:
            print("[✗] Ship spawn failed")
            return None

    except Exception as e:
        print(f"[✗] Error: {e}")
        return None

def test_get_player_pawn():
    """Test GET /get_player_pawn endpoint"""
    print_section("TEST 3: GET /get_player_pawn")

    try:
        response = requests.get(f"{SERVER_URL}/get_player_pawn", timeout=5)
        result = response.json()

        print(f"Status Code: {response.status_code}")
        print(f"Response: {json.dumps(result, indent=2)}")

        if response.status_code == 200 and result.get("success"):
            ship_id = result.get("data", {}).get("ship_id")
            has_fc = result.get("data", {}).get("has_flight_controller", False)
            print(f"[✓] Player pawn retrieved: {ship_id}")
            print(f"    Has FlightController: {has_fc}")
            return ship_id
        else:
            print("[✗] Get player pawn failed")
            return None

    except Exception as e:
        print(f"[✗] Error: {e}")
        return None

def test_set_input(ship_id, thrust_x=1.0):
    """Test POST /set_input endpoint"""
    print_section(f"TEST 4: POST /set_input (ship: {ship_id})")

    payload = {
        "ship_id": ship_id,
        "thrust": {"x": thrust_x, "y": 0.0, "z": 0.0},
        "rotation": {"pitch": 0.0, "yaw": 0.0, "roll": 0.0}
    }

    print(f"Payload: {json.dumps(payload, indent=2)}")

    try:
        response = requests.post(f"{SERVER_URL}/set_input", json=payload, timeout=5)
        result = response.json()

        print(f"Status Code: {response.status_code}")
        print(f"Response: {json.dumps(result, indent=2)}")

        if response.status_code == 200 and result.get("success"):
            print("[✓] Input applied successfully")
            return True
        else:
            print("[✗] Set input failed")
            return False

    except Exception as e:
        print(f"[✗] Error: {e}")
        return False

def test_get_position(ship_id):
    """Test GET /get_position endpoint"""
    print_section(f"TEST 5: GET /get_position (ship: {ship_id})")

    try:
        response = requests.get(f"{SERVER_URL}/get_position/{ship_id}", timeout=5)
        result = response.json()

        print(f"Status Code: {response.status_code}")
        print(f"Response: {json.dumps(result, indent=2)}")

        if response.status_code == 200 and result.get("success"):
            pos = result.get("data", {}).get("position", [0, 0, 0])
            print(f"[✓] Position retrieved: ({pos[0]:.2f}, {pos[1]:.2f}, {pos[2]:.2f})")
            return pos
        else:
            print("[✗] Get position failed")
            return None

    except Exception as e:
        print(f"[✗] Error: {e}")
        return None

def test_get_velocity(ship_id):
    """Test GET /get_velocity endpoint"""
    print_section(f"TEST 6: GET /get_velocity (ship: {ship_id})")

    try:
        response = requests.get(f"{SERVER_URL}/get_velocity/{ship_id}", timeout=5)
        result = response.json()

        print(f"Status Code: {response.status_code}")
        print(f"Response: {json.dumps(result, indent=2)}")

        if response.status_code == 200 and result.get("success"):
            vel = result.get("data", {}).get("velocity", [0, 0, 0])
            speed = result.get("data", {}).get("speed", 0)
            print(f"[✓] Velocity retrieved: ({vel[0]:.2f}, {vel[1]:.2f}, {vel[2]:.2f})")
            print(f"    Speed: {speed:.2f} units/sec")
            return vel, speed
        else:
            print("[✗] Get velocity failed")
            return None, 0

    except Exception as e:
        print(f"[✗] Error: {e}")
        return None, 0

def main():
    """Run complete API workflow test"""

    print("\n" + "=" * 70)
    print(" COMPLETE HTTP API WORKFLOW TEST")
    print(" AutomationAPIServer Endpoint Verification")
    print("=" * 70)
    print(f"Server: {SERVER_URL}")
    print(f"Timestamp: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

    test_results = []

    # Test 1: Server status
    result = test_status()
    test_results.append(("GET /status", result))
    if not result:
        print("\n[ABORT] Server not responding")
        return

    # Test 2: Spawn ship
    spawned_ship_id = test_spawn_ship()
    test_results.append(("POST /spawn_ship", spawned_ship_id is not None))

    # Test 3: Get player pawn
    player_pawn_id = test_get_player_pawn()
    test_results.append(("GET /get_player_pawn", player_pawn_id is not None))

    # Test 4: Set input on player pawn
    if player_pawn_id:
        result = test_set_input(player_pawn_id, thrust_x=1.0)
        test_results.append(("POST /set_input (player)", result))

        # Wait for physics
        print("\n[INFO] Waiting 2 seconds for physics simulation...")
        time.sleep(2)

        # Test 5: Get position
        pos = test_get_position(player_pawn_id)
        test_results.append(("GET /get_position (player)", pos is not None))

        # Test 6: Get velocity
        vel, speed = test_get_velocity(player_pawn_id)
        test_results.append(("GET /get_velocity (player)", vel is not None))

    # Test with spawned ship if available
    if spawned_ship_id:
        result = test_set_input(spawned_ship_id, thrust_x=0.5)
        test_results.append(("POST /set_input (spawned)", result))

        time.sleep(1)

        pos = test_get_position(spawned_ship_id)
        test_results.append(("GET /get_position (spawned)", pos is not None))

        vel, speed = test_get_velocity(spawned_ship_id)
        test_results.append(("GET /get_velocity (spawned)", vel is not None))

    # Print summary
    print_section("TEST SUMMARY")

    passed = sum(1 for _, result in test_results if result)
    total = len(test_results)

    for test_name, result in test_results:
        status = "[✓]" if result else "[✗]"
        print(f"{status} {test_name}")

    print(f"\nResults: {passed}/{total} tests passed")

    if passed == total:
        print("\n[SUCCESS] All API endpoints working correctly! 🎉")
    else:
        print(f"\n[PARTIAL] {total - passed} endpoint(s) failed")

    print("\n" + "=" * 70)

if __name__ == "__main__":
    main()
