"""
Simple Test for Observation Endpoints
Tests /submit_observation and /validate_position without launching clients

Assumes a game client is already running on port 8080
"""

import requests
import json
import math
import time

PORT = 8080
BASE_URL = f"http://localhost:{PORT}"

def normalize_vector(vec):
    """Normalize a 3D vector"""
    magnitude = math.sqrt(vec[0]**2 + vec[1]**2 + vec[2]**2)
    if magnitude == 0:
        return [0, 0, 0]
    return [vec[0]/magnitude, vec[1]/magnitude, vec[2]/magnitude]

def test_server_status():
    """Test if server is running"""
    print("\n" + "="*60)
    print("TESTING SERVER STATUS")
    print("="*60)

    try:
        response = requests.get(f"{BASE_URL}/status", timeout=2)
        if response.status_code == 200:
            data = response.json()
            print(f"[SUCCESS] Server is running")
            print(json.dumps(data, indent=2))
            return True
        else:
            print(f"[FAILED] Status code: {response.status_code}")
            return False
    except Exception as e:
        print(f"[FAILED] Cannot connect to server on port {PORT}")
        print(f"[ERROR] {e}")
        print("\nPlease start a game client first:")
        print(f'  "{r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe"}" \\')
        print(f'    "Alexander.uproject" VRTemplateMap \\')
        print(f'    -game -windowed -ResX=800 -ResY=600 -HTTPPort={PORT} -log')
        return False

def submit_observation(observer_id, target_id, direction, distance, scale_factor=1.0):
    """Submit an observation"""
    url = f"{BASE_URL}/submit_observation"

    payload = {
        "observer_id": observer_id,
        "target_id": target_id,
        "direction": direction,
        "distance": distance,
        "scale_factor": scale_factor,
        "timestamp": time.time()
    }

    try:
        response = requests.post(url, json=payload, timeout=5)
        return response.json()
    except Exception as e:
        print(f"[ERROR] Failed to submit observation: {e}")
        return None

def validate_position(target_id, observations):
    """Validate position using multiple observations"""
    url = f"{BASE_URL}/validate_position"

    payload = {
        "target_id": target_id,
        "observations": observations
    }

    try:
        response = requests.post(url, json=payload, timeout=5)
        return response.json()
    except Exception as e:
        print(f"[ERROR] Failed to validate position: {e}")
        return None

def test_2_observers():
    """Test with 2 observers (distance validation only)"""
    print("\n" + "="*60)
    print("TEST 1: TWO OBSERVERS (Distance Validation)")
    print("="*60)

    target_id = 1
    observations = [
        {
            "observer_id": 1,
            "direction": normalize_vector([1, 0, 0]),
            "distance": 100.0,
            "scale_factor": 1.0
        },
        {
            "observer_id": 2,
            "direction": normalize_vector([0, 1, 0]),
            "distance": 100.0,
            "scale_factor": 1.0
        }
    ]

    print("\nSubmitting observations...")
    for obs in observations:
        result = submit_observation(
            obs["observer_id"],
            target_id,
            obs["direction"],
            obs["distance"],
            obs["scale_factor"]
        )
        if result:
            print(f"  Observer {obs['observer_id']}: {result.get('message', 'OK')}")

    print("\nValidating position...")
    result = validate_position(target_id, observations)

    if result:
        print("\n" + json.dumps(result, indent=2))

        if result.get("success"):
            data = result.get("data", {})
            print(f"\n[RESULT] Valid: {data.get('valid')}")
            print(f"[RESULT] Confidence: {data.get('confidence', 0):.2%}")
            print(f"[RESULT] Method: {data.get('validation_method')}")

            # Expected confidence for 2 observers: 1 - (1/2^1.5) ≈ 0.646 = 64.6%
            expected = 1.0 - (1.0 / (2 ** 1.5))
            print(f"[EXPECTED] Confidence for 2 observers: {expected:.2%}")
        return result.get("success", False)
    return False

def test_4_observers():
    """Test with 4 observers (tetrahedron validation)"""
    print("\n" + "="*60)
    print("TEST 2: FOUR OBSERVERS (Tetrahedron Validation)")
    print("="*60)

    # Target at (100, 200, 50)
    target_position = [100.0, 200.0, 50.0]
    target_id = 2

    # 4 observers forming tetrahedron
    observer_positions = [
        [0.0, 0.0, 0.0],      # Observer 1
        [200.0, 0.0, 0.0],    # Observer 2
        [100.0, 400.0, 0.0],  # Observer 3
        [100.0, 200.0, 100.0] # Observer 4
    ]

    observations = []

    print("\nCalculating observations from tetrahedron corners:")
    for i, obs_pos in enumerate(observer_positions):
        # Vector from observer to target
        delta = [
            target_position[0] - obs_pos[0],
            target_position[1] - obs_pos[1],
            target_position[2] - obs_pos[2]
        ]

        # Distance
        distance = math.sqrt(delta[0]**2 + delta[1]**2 + delta[2]**2)

        # Direction
        direction = normalize_vector(delta)

        observation = {
            "observer_id": i + 1,
            "direction": direction,
            "distance": distance,
            "scale_factor": 1.0
        }

        observations.append(observation)

        print(f"  Observer {i+1} at {obs_pos}")
        print(f"    Direction: [{direction[0]:.4f}, {direction[1]:.4f}, {direction[2]:.4f}]")
        print(f"    Distance: {distance:.2f}")

    print(f"\nTarget position: {target_position}")

    print("\nSubmitting observations...")
    for obs in observations:
        result = submit_observation(
            obs["observer_id"],
            target_id,
            obs["direction"],
            obs["distance"],
            obs["scale_factor"]
        )
        if result:
            print(f"  Observer {obs['observer_id']}: {result.get('message', 'OK')}")

    print("\nValidating position...")
    result = validate_position(target_id, observations)

    if result:
        print("\n" + json.dumps(result, indent=2))

        if result.get("success"):
            data = result.get("data", {})
            triangulated = data.get('triangulated_position', [0, 0, 0])

            print(f"\n[RESULT] Valid: {data.get('valid')}")
            print(f"[RESULT] Confidence: {data.get('confidence', 0):.2%}")
            print(f"[RESULT] Method: {data.get('validation_method')}")
            print(f"[RESULT] Geometric Error: {data.get('geometric_error', 0):.4f}")
            print(f"[RESULT] Triangulated: [{triangulated[0]:.2f}, {triangulated[1]:.2f}, {triangulated[2]:.2f}]")
            print(f"[EXPECTED] Target: [{target_position[0]:.2f}, {target_position[1]:.2f}, {target_position[2]:.2f}]")

            # Calculate position error
            error = math.sqrt(
                (triangulated[0] - target_position[0])**2 +
                (triangulated[1] - target_position[1])**2 +
                (triangulated[2] - target_position[2])**2
            )
            print(f"[RESULT] Position Error: {error:.2f} units")

            # Expected confidence for 4 observers: 1 - (1/4^1.5) ≈ 0.875 = 87.5%
            expected = 1.0 - (1.0 / (4 ** 1.5))
            print(f"[EXPECTED] Confidence for 4 observers: {expected:.2%}")

        return result.get("success", False)
    return False

def main():
    """Run all tests"""
    print("\n" + "="*60)
    print("OBSERVATION ENDPOINT TESTING")
    print("="*60)

    # Check server status
    if not test_server_status():
        print("\n[ABORTED] Server not running")
        return

    # Run tests
    time.sleep(1)
    test_2_observers()

    time.sleep(1)
    test_4_observers()

    print("\n" + "="*60)
    print("ALL TESTS COMPLETE")
    print("="*60)

if __name__ == "__main__":
    main()
