"""
Test with 3 observers (odd count) to verify the odd-count theory
If this succeeds without crashing, it supports the theory that even counts cause crashes
"""

import requests
import json
import math
import time

BASE_URL = "http://localhost:8080"

def submit_observation(observer_id, target_id, direction, distance, observer_position):
    """Submit an observation to the server"""
    url = f"{BASE_URL}/submit_observation"

    payload = {
        "observer_id": observer_id,
        "target_id": target_id,
        "observer_position": observer_position,
        "direction": direction,
        "distance": distance,
        "scale_factor": 1.0,
        "timestamp": 0.0
    }

    response = requests.post(url, json=payload, timeout=5)
    return response.json()

def normalize_vector(vec):
    """Normalize a 3D vector"""
    magnitude = math.sqrt(vec[0]**2 + vec[1]**2 + vec[2]**2)
    if magnitude == 0:
        return [0, 0, 0]
    return [vec[0]/magnitude, vec[1]/magnitude, vec[2]/magnitude]

def main():
    """Test with 3 observers forming a triangle (ODD count)"""

    print("\n" + "="*60)
    print("3-OBSERVER TRIANGLE TEST (ODD COUNT)")
    print("="*60)

    # Test scenario: Target at (1000, 2000, 500)
    target_position = [1000.0, 2000.0, 500.0]
    target_id = 1

    # 3 observers forming a triangle (planar)
    observer_positions = [
        [0.0, 0.0, 0.0],          # Observer 1: origin
        [2000.0, 0.0, 0.0],       # Observer 2: +X axis
        [1000.0, 4000.0, 0.0],    # Observer 3: +Y axis
    ]

    print(f"\nTarget Position: {target_position}")
    print(f"Target ID: {target_id}")
    print(f"Observer Count: {len(observer_positions)} (ODD)\n")

    print("SUBMITTING OBSERVATIONS (1, 2, 3)")
    print("="*60)

    for i, obs_pos in enumerate(observer_positions):
        # Vector from observer to target
        delta_x = target_position[0] - obs_pos[0]
        delta_y = target_position[1] - obs_pos[1]
        delta_z = target_position[2] - obs_pos[2]

        # Distance
        distance = math.sqrt(delta_x**2 + delta_y**2 + delta_z**2)

        # Normalized direction
        direction = normalize_vector([delta_x, delta_y, delta_z])

        print(f"\nObserver {i+1} at {obs_pos}:")
        print(f"  Direction: [{direction[0]:.4f}, {direction[1]:.4f}, {direction[2]:.4f}]")
        print(f"  Distance: {distance:.2f} units")

        try:
            result = submit_observation(
                observer_id=i + 1,
                target_id=target_id,
                direction=direction,
                distance=distance,
                observer_position=obs_pos
            )

            if result.get("success"):
                print(f"  [OK] Observation {i+1} recorded successfully")
            else:
                print(f"  [FAIL] {result.get('message')}")
                return False

            # Small delay
            time.sleep(0.1)

        except Exception as e:
            print(f"  [ERROR] {e}")
            return False

    print("\n" + "="*60)
    print("TEST RESULT: ALL 3 OBSERVATIONS SUCCEEDED!")
    print("="*60)
    print("Odd count (3) appears to work without crashing")
    print("This supports the theory that even counts (4) cause the crash")
    return True

if __name__ == "__main__":
    try:
        success = main()
        exit(0 if success else 1)
    except Exception as e:
        print(f"\n[ERROR] {e}")
        import traceback
        traceback.print_exc()
        exit(1)
