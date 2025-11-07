#!/usr/bin/env python3
"""
Test physics movement and position tracking across multiple clients
"""
import requests
import json
import time

CLIENTS = [
    {"port": 8080, "ship_id": "ship_1"},
    {"port": 8081, "ship_id": "ship_1"},
    {"port": 8082, "ship_id": "ship_1"},
]

def get_position(port, ship_id):
    """Get ship position from a client"""
    try:
        response = requests.get(
            f"http://localhost:{port}/get_position?ship_id={ship_id}",
            timeout=3
        )
        data = response.json()
        if data.get("success"):
            return data["data"]["position"]
        return None
    except Exception as e:
        print(f"Error getting position from port {port}: {e}")
        return None

def apply_thrust(port, ship_id, thrust_x=0, thrust_y=0, thrust_z=0):
    """Apply thrust to a ship"""
    try:
        payload = {
            "ship_id": ship_id,
            "thrust_x": thrust_x,
            "thrust_y": thrust_y,
            "thrust_z": thrust_z
        }
        response = requests.post(
            f"http://localhost:{port}/apply_thrust",
            json=payload,
            timeout=3
        )
        return response.json()
    except Exception as e:
        print(f"Error applying thrust on port {port}: {e}")
        return {"success": False, "message": str(e)}

def test_physics_movement():
    print("="*60)
    print("PHYSICS MOVEMENT TEST")
    print("="*60)

    # Test 1: Get initial positions
    print("\nTest 1: Recording Initial Positions")
    print("-" * 60)
    initial_positions = {}
    for client in CLIENTS:
        port = client["port"]
        ship_id = client["ship_id"]
        pos = get_position(port, ship_id)
        if pos:
            initial_positions[port] = pos
            print(f"  Port {port}: {pos}")
        else:
            print(f"  Port {port}: [ERROR] Could not get position")

    # Test 2: Apply upward thrust on all clients
    print("\nTest 2: Applying Upward Thrust (Z=1000)")
    print("-" * 60)
    for client in CLIENTS:
        port = client["port"]
        ship_id = client["ship_id"]
        result = apply_thrust(port, ship_id, thrust_z=1000)
        status = "[PASS]" if result.get("success") else f"[FAIL] {result.get('message', 'Unknown error')}"
        print(f"  Port {port}: {status}")

    # Test 3: Wait and measure position changes
    print("\nTest 3: Waiting 2 seconds for physics update...")
    time.sleep(2)

    print("\nTest 4: Recording New Positions")
    print("-" * 60)
    new_positions = {}
    movements = {}
    for client in CLIENTS:
        port = client["port"]
        ship_id = client["ship_id"]
        pos = get_position(port, ship_id)
        if pos and port in initial_positions:
            new_positions[port] = pos
            # Calculate movement
            dx = pos[0] - initial_positions[port][0]
            dy = pos[1] - initial_positions[port][1]
            dz = pos[2] - initial_positions[port][2]
            movements[port] = [dx, dy, dz]
            print(f"  Port {port}: {pos}")
            print(f"    Movement: [{dx:.2f}, {dy:.2f}, {dz:.2f}]")

            # Check if moved upward
            moved = abs(dz) > 1.0  # Moved more than 1 unit
            status = "[PASS]" if moved else "[FAIL] No significant movement"
            print(f"    Status: {status}")
        else:
            print(f"  Port {port}: [ERROR] Could not get new position")

    # Test 5: Cross-client position comparison
    print("\nTest 5: Cross-Client Position Query")
    print("-" * 60)
    print("Testing if Client A can query positions from Client B & C...")

    # For now, each client is independent, so we just verify they all moved
    if len(movements) == 3:
        all_moved_up = all(movements[port][2] > 1.0 for port in movements)
        status = "[PASS] All clients independently tracked upward movement" if all_moved_up else "[FAIL] Not all clients moved"
        print(f"  {status}")

        # Show movement comparison
        print("\n  Movement Comparison:")
        for port in sorted(movements.keys()):
            print(f"    Port {port}: Z-axis movement = {movements[port][2]:.2f} units")
    else:
        print("  [FAIL] Could not get movement data from all clients")

    print("\n" + "="*60)
    print("PHYSICS MOVEMENT TEST COMPLETE")
    print("="*60)
    print("\nNOTE: Currently each client runs an independent physics simulation.")
    print("Next step: Implement position broadcasting so clients can share data.")

if __name__ == "__main__":
    test_physics_movement()
