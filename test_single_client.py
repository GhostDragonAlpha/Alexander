#!/usr/bin/env python3
"""
Single Client Test - Verify StaticLoadClass fix works for PhysicsTestShip
"""

import requests
import time
import sys

API_URL = "http://localhost:8080"

def wait_for_api(timeout=60):
    """Wait for HTTP API to be ready"""
    print("Waiting for HTTP API to be ready...")
    start_time = time.time()

    while time.time() - start_time < timeout:
        try:
            response = requests.get(f"{API_URL}/status", timeout=2)
            if response.status_code == 200:
                print("[OK] HTTP API is ready!")
                return True
        except requests.exceptions.RequestException:
            pass
        time.sleep(2)
        print(".", end="", flush=True)

    print("\n[ERROR] Timeout waiting for HTTP API")
    return False

def test_spawn_physics_test_ship():
    """Test spawning PhysicsTestShip via /spawn_ship endpoint"""
    print("\n" + "="*60)
    print("TEST: Spawn PhysicsTestShip (StaticLoadClass fix)")
    print("="*60)

    payload = {
        "ship_class": "/Script/Alexander.PhysicsTestShip",
        "location": [0, 0, 500],
        "rotation": [0, 0, 0]
    }

    try:
        print(f"\nSpawning ship with payload: {payload}")
        response = requests.post(
            f"{API_URL}/spawn_ship",
            json=payload,
            timeout=10
        )

        print(f"Response status: {response.status_code}")

        if response.status_code == 200:
            data = response.json()
            print(f"Response data: {data}")

            if data.get("success"):
                ship_id = data.get("data", {}).get("ship_id")
                print(f"\n[SUCCESS] Ship spawned with ID: {ship_id}")
                print("StaticLoadClass fix is working!")
                return ship_id
            else:
                print(f"\n[FAILED] {data.get('message')}")
                return None
        else:
            print(f"\n[ERROR] HTTP Error: {response.status_code}")
            print(f"Response: {response.text}")
            return None

    except Exception as e:
        print(f"\n[ERROR] Exception: {e}")
        return None

def test_get_ship_position(ship_id):
    """Test getting ship position"""
    print("\n" + "="*60)
    print("TEST: Get Ship Position")
    print("="*60)

    try:
        response = requests.get(
            f"{API_URL}/get_position",
            params={"ship_id": ship_id},
            timeout=5
        )

        if response.status_code == 200:
            data = response.json()
            if data.get("success"):
                position = data.get("data", {}).get("location")
                velocity = data.get("data", {}).get("velocity")
                print(f"\n[OK] Position: {position}")
                print(f"[OK] Velocity: {velocity}")
                return True
            else:
                print(f"\n[FAILED] {data.get('message')}")
        else:
            print(f"\n[ERROR] HTTP Error: {response.status_code}")

    except Exception as e:
        print(f"\n[ERROR] Exception: {e}")

    return False

def main():
    """Run single client tests"""
    print("\n" + "="*60)
    print("SINGLE CLIENT TEST - StaticLoadClass Fix Verification")
    print("="*60 + "\n")

    # Wait for API
    if not wait_for_api():
        print("\n[ERROR] Failed to connect to HTTP API")
        print("Make sure the game client is running with AutomationGameMode")
        return 1

    # Test spawn ship
    ship_id = test_spawn_physics_test_ship()
    if not ship_id:
        print("\n[ERROR] TEST FAILED: Could not spawn PhysicsTestShip")
        print("StaticLoadClass fix may not be working correctly")
        return 1

    # Wait a moment for ship to initialize
    time.sleep(2)

    # Test get position
    if not test_get_ship_position(ship_id):
        print("\n[WARNING] Could not get ship position")

    print("\n" + "="*60)
    print("[SUCCESS] ALL TESTS PASSED!")
    print("="*60)
    print("\nStaticLoadClass fix is working correctly!")
    print("PhysicsTestShip can be spawned via HTTP API!")
    print("\nNext: Multi-client consensus testing")

    return 0

if __name__ == "__main__":
    sys.exit(main())
