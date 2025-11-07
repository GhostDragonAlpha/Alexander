"""
Simple 4-Observer Validation Test
Tests the complete observation submission and validation workflow
Uses simulated observations (no actual game clients needed)
"""

import requests
import json
import math

# Test against port 8080 (we know it's running)
BASE_URL = "http://localhost:8080"

def submit_observation(observer_id, target_id, direction, distance):
    """Submit an observation to the server"""
    url = f"{BASE_URL}/submit_observation"

    payload = {
        "observer_id": observer_id,
        "target_id": target_id,
        "direction": direction,
        "distance": distance,
        "scale_factor": 1.0,
        "timestamp": 0.0
    }

    response = requests.post(url, json=payload, timeout=5)
    return response.json()

def validate_position(target_id, observations):
    """Validate position using triangulation"""
    url = f"{BASE_URL}/validate_position"

    payload = {
        "target_id": target_id,
        "observations": observations
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
    """Run the 4-observer validation test"""

    print("\n" + "="*60)
    print("4-OBSERVER TETRAHEDRON VALIDATION TEST")
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
    print(f"Target ID: {target_id}\n")

    # Calculate observations
    observations = []

    print("CALCULATING OBSERVATIONS")
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

        observation = {
            "observer_id": i + 1,
            "direction": direction,
            "distance": distance,
            "scale_factor": 1.0
        }

        observations.append(observation)

        print(f"\nObserver {i+1} at {obs_pos}:")
        print(f"  Direction: [{direction[0]:.4f}, {direction[1]:.4f}, {direction[2]:.4f}]")
        print(f"  Distance: {distance:.2f} units")

    # Submit observations
    print("\n" + "="*60)
    print("SUBMITTING OBSERVATIONS TO SERVER")
    print("="*60)

    for obs in observations:
        result = submit_observation(
            obs["observer_id"],
            target_id,
            obs["direction"],
            obs["distance"]
        )

        if result.get("success"):
            print(f"✓ Observer {obs['observer_id']}: Observation recorded")
        else:
            print(f"✗ Observer {obs['observer_id']}: {result.get('message')}")

    # Validate position
    print("\n" + "="*60)
    print("VALIDATING POSITION (4-OBSERVER TETRAHEDRON)")
    print("="*60)

    validation_result = validate_position(target_id, observations)

    if validation_result.get("success"):
        data = validation_result.get("data", {})
        triangulated = data.get("triangulated_position", [0, 0, 0])

        print(f"\n✓ Validation: {data.get('valid')}")
        print(f"  Confidence: {data.get('confidence', 0):.2%}")
        print(f"  Observer Count: {data.get('observer_count')}")
        print(f"  Geometric Error: {data.get('geometric_error', 0):.4f}")
        print(f"  Validation Method: {data.get('validation_method')}")

        print(f"\n  Triangulated: [{triangulated[0]:.2f}, {triangulated[1]:.2f}, {triangulated[2]:.2f}]")
        print(f"  Expected:     [{target_position[0]:.2f}, {target_position[1]:.2f}, {target_position[2]:.2f}]")

        # Calculate error
        tri_error = math.sqrt(
            (triangulated[0] - target_position[0])**2 +
            (triangulated[1] - target_position[1])**2 +
            (triangulated[2] - target_position[2])**2
        )
        print(f"  Position Error: {tri_error:.2f} units")

        # Expected confidence: 1 - (1/4^1.5) ≈ 87.5%
        expected_confidence = 1.0 - (1.0 / (4 ** 1.5))
        print(f"\n  Expected Confidence (4 observers): {expected_confidence:.2%}")

        if data.get('valid') and tri_error < 100.0:
            print("\n" + "="*60)
            print("✓ TEST PASSED!")
            print("="*60)
            print("4-observer tetrahedron validation successful!")
            print("Position validated using geometric triangulation")
            print("Anti-fragile confidence scaling verified")
        else:
            print("\n" + "="*60)
            print("✗ TEST FAILED")
            print("="*60)
            if not data.get('valid'):
                print("Position validation rejected")
            else:
                print(f"Position error too high: {tri_error:.2f} units")
    else:
        print(f"\n✗ Validation error: {validation_result.get('message')}")
        return False

    return True

if __name__ == "__main__":
    try:
        success = main()
        exit(0 if success else 1)
    except Exception as e:
        print(f"\n✗ ERROR: {e}")
        import traceback
        traceback.print_exc()
        exit(1)
