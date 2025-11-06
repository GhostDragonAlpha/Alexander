#!/usr/bin/env python3
"""
Test multi-client system with proper port separation
"""
import requests
import json

CLIENTS = [
    {"port": 8080, "ship_location": [0, 0, 500]},
    {"port": 8081, "ship_location": [1000, 0, 500]},
    {"port": 8082, "ship_location": [0, 1000, 500]},
]

def test_all_clients():
    print("="*60)
    print("MULTI-CLIENT PORT SEPARATION TEST")
    print("="*60)

    # Test 1: Verify all clients report correct ports
    print("\nTest 1: Port Reporting")
    print("-" * 60)
    for client in CLIENTS:
        port = client["port"]
        try:
            response = requests.get(f"http://localhost:{port}/status", timeout=3)
            data = response.json()
            reported_port = data.get("port")
            status = "[PASS]" if reported_port == port else f"[FAIL] (reports {reported_port})"
            print(f"  Port {port}: {status}")
        except Exception as e:
            print(f"  Port {port}: [ERROR] - {e}")

    # Test 2: Spawn ships on all clients
    print("\nTest 2: Spawning Ships")
    print("-" * 60)
    for client in CLIENTS:
        port = client["port"]
        loc = client["ship_location"]
        try:
            payload = {
                "ship_id": "ship_1",
                "location": loc
            }
            response = requests.post(
                f"http://localhost:{port}/spawn_ship",
                json=payload,
                timeout=10
            )
            data = response.json()
            success = data.get("success", False)
            status = "[PASS]" if success else f"[FAIL] - {data.get('message', 'Unknown error')}"
            print(f"  Port {port}: {status} at {loc}")
        except Exception as e:
            print(f"  Port {port}: [ERROR] - {e}")

    # Test 3: Verify ship positions
    print("\nTest 3: Position Reporting")
    print("-" * 60)
    for client in CLIENTS:
        port = client["port"]
        expected_loc = client["ship_location"]
        try:
            response = requests.get(f"http://localhost:{port}/get_position?ship_id=ship_1", timeout=3)
            data = response.json()
            if data.get("success"):
                actual_loc = data["data"]["position"]
                match = (
                    abs(actual_loc[0] - expected_loc[0]) < 1 and
                    abs(actual_loc[1] - expected_loc[1]) < 1 and
                    abs(actual_loc[2] - expected_loc[2]) < 1
                )
                status = "[PASS]" if match else f"[FAIL] (got {actual_loc})"
                print(f"  Port {port}: {status}")
            else:
                print(f"  Port {port}: [FAIL] - {data.get('message', 'Unknown error')}")
        except Exception as e:
            print(f"  Port {port}: [ERROR] - {e}")

    print("\n" + "="*60)
    print("TEST COMPLETE")
    print("="*60)

if __name__ == "__main__":
    test_all_clients()
