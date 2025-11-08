"""
Test HTTP Connection Theory
Force new connection for each request (Connection: close header)
"""

import requests
import json
import math
import time

BASE_URL = "http://localhost:8080"

def submit_observation_with_close(observer_id, target_id, direction, distance, observer_position):
    """Submit observation with Connection: close to force new connection each time"""
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

    # Force new connection each time - no keep-alive
    headers = {"Connection": "close"}

    response = requests.post(url, json=payload, headers=headers, timeout=5)
    return response.json()

def normalize_vector(vec):
    """Normalize a 3D vector"""
    magnitude = math.sqrt(vec[0]**2 + vec[1]**2 + vec[2]**2)
    if magnitude == 0:
        return [0, 0, 0]
    return [vec[0]/magnitude, vec[1]/magnitude, vec[2]/magnitude]

def main():
    """Test with Connection: close headers to isolate HTTP pooling issue"""

    print("\n" + "="*60)
    print("HTTP CONNECTION THEORY TEST")
    print("Force new connection for each request (Connection: close)")
    print("="*60)

    # Test scenario: Target at (1000, 2000, 500)
    target_position = [1000.0, 2000.0, 500.0]
    target_id = 1

    # 4 observers forming a tetrahedron
    observer_positions = [
        [0.0, 0.0, 0.0],          # Observer 1: origin
        [2000.0, 0.0, 0.0],       # Observer 2: +X axis
        [1000.0, 4000.0, 0.0],    # Observer 3: +Y axis
        [1000.0, 2000.0, 1000.0]  # Observer 4: +Z axis
    ]

    print(f"\nTarget Position: {target_position}")
    print(f"Target ID: {target_id}")
    print(f"Testing with Connection: close header (no keep-alive)\n")

    print("SUBMITTING OBSERVATIONS WITH FORCED NEW CONNECTIONS")
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
            result = submit_observation_with_close(
                observer_id=i + 1,
                target_id=target_id,
                direction=direction,
                distance=distance,
                observer_position=obs_pos
            )

            if result.get("success"):
                print(f"  [OK] Observation {i+1} recorded with NEW connection")
            else:
                print(f"  [FAIL] {result.get('message')}")
                return False

            # Small delay
            time.sleep(0.5)

        except Exception as e:
            print(f"  [CRASH] {e}")
            print(f"\nCrashed on observation {i+1}")
            return False

    print("\n" + "="*60)
    print("SUCCESS! All 4 observations submitted with Connection: close")
    print("="*60)
    print("This suggests HTTP keep-alive/pooling may be the root cause!")
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
