#!/usr/bin/env python3
"""
Quick Position Reporting Test
Tests that all 3 clients can report ship positions correctly
"""

import requests
import time

def test_position_reporting():
    """Test position reporting on all 3 clients"""
    print("="*60)
    print("POSITION REPORTING TEST")
    print("="*60 + "\n")

    clients = [
        {"port": 8080, "ship_location": [0, 0, 500]},
        {"port": 8081, "ship_location": [1000, 0, 500]},
        {"port": 8082, "ship_location": [0, 1000, 500]},
    ]

    # Step 1: Check all clients are running
    print("Step 1: Checking all clients are running...")
    for client in clients:
        try:
            response = requests.get(f"http://localhost:{client['port']}/status", timeout=2)
            if response.status_code == 200:
                data = response.json()
                print(f"  Port {client['port']}: OK ({data.get('tracked_ships', 0)} ships)")
            else:
                print(f"  Port {client['port']}: ERROR - HTTP {response.status_code}")
                return False
        except Exception as e:
            print(f"  Port {client['port']}: ERROR - {e}")
            return False

    print("\nAll clients running!\n")

    # Step 2: Spawn ships in all clients
    print("Step 2: Spawning ships in all clients...")
    ship_ids = {}
    for client in clients:
        payload = {
            "ship_class": "/Script/Alexander.PhysicsTestShip",
            "location": client["ship_location"],
            "rotation": [0, 0, 0]
        }

        try:
            response = requests.post(
                f"http://localhost:{client['port']}/spawn_ship",
                json=payload,
                timeout=5
            )

            if response.status_code == 200:
                data = response.json()
                if data.get("success"):
                    ship_id = data.get("data", {}).get("ship_id")
                    ship_ids[client["port"]] = ship_id
                    print(f"  Port {client['port']}: Spawned {ship_id} at {client['ship_location']}")
                else:
                    print(f"  Port {client['port']}: Failed - {data.get('message')}")
                    return False
            else:
                print(f"  Port {client['port']}: HTTP Error {response.status_code}")
                return False
        except Exception as e:
            print(f"  Port {client['port']}: Exception - {e}")
            return False

    print("\nAll ships spawned!\n")

    # Step 3: Wait for ships to settle
    print("Step 3: Waiting 2 seconds for ships to initialize...")
    time.sleep(2)

    # Step 4: Get positions from all clients (using query parameters)
    print("\nStep 4: Testing position reporting (query parameter style)...")
    for client in clients:
        ship_id = ship_ids[client["port"]]
        try:
            response = requests.get(
                f"http://localhost:{client['port']}/get_position?ship_id={ship_id}",
                timeout=2
            )

            if response.status_code == 200:
                data = response.json()
                if data.get("success"):
                    position = data.get("data", {}).get("location")
                    velocity = data.get("data", {}).get("velocity")
                    print(f"  Port {client['port']} ({ship_id}):")
                    print(f"    Position: {position}")
                    print(f"    Velocity: {velocity}")
                else:
                    print(f"  Port {client['port']}: Failed - {data.get('message')}")
                    return False
            else:
                print(f"  Port {client['port']}: HTTP Error {response.status_code}")
                return False
        except Exception as e:
            print(f"  Port {client['port']}: Exception - {e}")
            return False

    # Step 5: Get positions using path parameter style
    print("\nStep 5: Testing position reporting (path parameter style)...")
    for client in clients:
        ship_id = ship_ids[client["port"]]
        try:
            response = requests.get(
                f"http://localhost:{client['port']}/get_position/{ship_id}",
                timeout=2
            )

            if response.status_code == 200:
                data = response.json()
                if data.get("success"):
                    position = data.get("data", {}).get("location")
                    print(f"  Port {client['port']} ({ship_id}): {position}")
                else:
                    print(f"  Port {client['port']}: Failed - {data.get('message')}")
                    return False
            else:
                print(f"  Port {client['port']}: HTTP Error {response.status_code}")
                return False
        except Exception as e:
            print(f"  Port {client['port']}: Exception - {e}")
            return False

    print("\n" + "="*60)
    print("SUCCESS: Position reporting works on all 3 clients!")
    print("="*60)
    print("\nBoth query parameter and path parameter styles work correctly.")
    print("Ready for full consensus validation testing.")
    return True

if __name__ == "__main__":
    success = test_position_reporting()
    exit(0 if success else 1)
