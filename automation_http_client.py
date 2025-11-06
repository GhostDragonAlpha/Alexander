"""
Automation HTTP Client
Sends commands to standalone game via AutomationAPIServer (HTTP on port 8080)
"""

import requests
import json
import time

SERVER_URL = "http://localhost:8080"

def spawn_ship(ship_class_path="/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer_C"):
    """Spawn a ship in the game"""

    print(f"\n[API] Spawning ship: {ship_class_path}")

    payload = {
        "ship_class": ship_class_path,
        "location": {"x": 0, "y": 0, "z": 500},
        "rotation": {"pitch": 0, "yaw": 0, "roll": 0}
    }

    try:
        response = requests.post(f"{SERVER_URL}/spawn_ship", json=payload, timeout=5)
        result = response.json()

        if response.status_code == 200 and result.get("success"):
            ship_id = result.get("data", {}).get("ship_id")
            print(f"[OK] Ship spawned: {ship_id}")
            return ship_id
        else:
            print(f"[FAIL] Spawn failed: {result.get('message', 'Unknown error')}")
            return None

    except requests.exceptions.ConnectionRefused:
        print("[FAIL] Cannot connect to AutomationAPIServer")
        print("[INFO] Make sure standalone game is running with Automation GameMode")
        return None
    except Exception as e:
        print(f"[FAIL] Error: {e}")
        return None

def set_input(ship_id, thrust_x=0.0, thrust_y=0.0, thrust_z=0.0,
              rotation_pitch=0.0, rotation_yaw=0.0, rotation_roll=0.0):
    """Set input for a ship"""

    print(f"\n[API] Setting input for ship: {ship_id}")
    print(f"  Thrust: ({thrust_x:.2f}, {thrust_y:.2f}, {thrust_z:.2f})")
    print(f"  Rotation: ({rotation_pitch:.2f}, {rotation_yaw:.2f}, {rotation_roll:.2f})")

    payload = {
        "ship_id": ship_id,
        "thrust": {"x": thrust_x, "y": thrust_y, "z": thrust_z},
        "rotation": {"pitch": rotation_pitch, "yaw": rotation_yaw, "roll": rotation_roll}
    }

    try:
        response = requests.post(f"{SERVER_URL}/set_input", json=payload, timeout=5)
        result = response.json()

        if response.status_code == 200 and result.get("success"):
            print(f"[OK] Input applied")
            return True
        else:
            print(f"[FAIL] Input failed: {result.get('message', 'Unknown error')}")
            return False

    except Exception as e:
        print(f"[FAIL] Error: {e}")
        return False

def get_position(ship_id):
    """Get ship position"""

    try:
        response = requests.get(f"{SERVER_URL}/get_position/{ship_id}", timeout=5)
        result = response.json()

        if response.status_code == 200 and result.get("success"):
            pos = result.get("position", {})
            print(f"[OK] Position: ({pos.get('x', 0):.2f}, {pos.get('y', 0):.2f}, {pos.get('z', 0):.2f})")
            return pos
        else:
            print(f"[FAIL] Get position failed: {result.get('message', 'Unknown error')}")
            return None

    except Exception as e:
        print(f"[FAIL] Error: {e}")
        return None

def get_velocity(ship_id):
    """Get ship velocity"""

    try:
        response = requests.get(f"{SERVER_URL}/get_velocity/{ship_id}", timeout=5)
        result = response.json()

        if response.status_code == 200 and result.get("success"):
            vel = result.get("velocity", {})
            speed = result.get("speed", 0)
            print(f"[OK] Velocity: ({vel.get('x', 0):.2f}, {vel.get('y', 0):.2f}, {vel.get('z', 0):.2f})")
            print(f"[OK] Speed: {speed:.2f} units/sec")
            return vel, speed
        else:
            print(f"[FAIL] Get velocity failed: {result.get('message', 'Unknown error')}")
            return None, 0

    except Exception as e:
        print(f"[FAIL] Error: {e}")
        return None, 0

def test_connection():
    """Test if AutomationAPIServer is running"""

    print("\n[TEST] Testing connection to AutomationAPIServer...")
    print(f"[TEST] URL: {SERVER_URL}")

    try:
        response = requests.get(f"{SERVER_URL}/status", timeout=5)
        result = response.json()

        if response.status_code == 200:
            print(f"[OK] Server is running")
            print(f"[OK] Status: {result.get('status', 'unknown')}")
            return True
        else:
            print(f"[FAIL] Server returned error: {response.status_code}")
            return False

    except requests.exceptions.ConnectionRefused:
        print("[FAIL] Cannot connect to AutomationAPIServer (port 8080)")
        print("[INFO] Make sure:")
        print("  1. Standalone game is running (-game flag)")
        print("  2. Automation GameMode is set as default")
        print("  3. AutomationAPIServer started on port 8080")
        return False
    except Exception as e:
        print(f"[FAIL] Error: {e}")
        return False

if __name__ == "__main__":
    print("=" * 70)
    print("AUTOMATION HTTP CLIENT")
    print("=" * 70)

    # Test connection
    if not test_connection():
        print("\n[FAIL] Cannot connect to server")
        exit(1)

    print("\n" + "=" * 70)
    print("FLIGHT TEST")
    print("=" * 70)

    # Spawn ship
    ship_id = spawn_ship()
    if not ship_id:
        print("\n[FAIL] Cannot spawn ship")
        exit(1)

    # Wait for physics to settle
    print("\n[INFO] Waiting 2 seconds for physics...")
    time.sleep(2)

    # Apply forward thrust
    print("\n[INFO] Applying forward thrust (X+1.0)...")
    set_input(ship_id, thrust_x=1.0)

    # Wait for movement
    print("\n[INFO] Waiting 5 seconds for movement...")
    time.sleep(5)

    # Check position and velocity
    print("\n[INFO] Checking flight status...")
    pos = get_position(ship_id)
    vel, speed = get_velocity(ship_id)

    if pos and (abs(pos.get('x', 0)) > 50 or speed > 10):
        print("\n" + "=" * 70)
        print("[SUCCESS] SHIP IS FLYING!")
        print("=" * 70)
        print(f"Distance from spawn: {abs(pos.get('x', 0)):.2f} units")
        print(f"Speed: {speed:.2f} units/sec")
    else:
        print("\n" + "=" * 70)
        print("[FAIL] Ship not moving significantly")
        print("=" * 70)
