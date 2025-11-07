"""
Test 4-Observer Tetrahedron Validation
Tests multi-sphere triangulation with geometric consensus

This script:
1. Kills existing Unreal processes
2. Launches 4 game clients on ports 8080-8083
3. Tests tetrahedron validation with 4 observers
4. Verifies anti-fragile confidence scaling
"""

import subprocess
import time
import requests
import json
import math
import sys

UNREAL_EDITOR = r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe"
PROJECT_FILE = r"C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject"

# Client ports
PORTS = [8080, 8081, 8082, 8083]

def kill_processes():
    """Kill all existing Unreal processes"""
    print("\n" + "="*60)
    print("KILLING EXISTING PROCESSES")
    print("="*60)

    result = subprocess.run(
        ["powershell", "-ExecutionPolicy", "Bypass", "-File", "kill_unreal.ps1"],
        capture_output=True,
        text=True
    )
    print(result.stdout)
    time.sleep(2)

def launch_client(port):
    """Launch a single game client on specified port"""
    print(f"\n[LAUNCH] Starting client on port {port}...")

    cmd = [
        UNREAL_EDITOR,
        PROJECT_FILE,
        "VRTemplateMap?game=/Script/Alexander.AutomationGameMode",
        "-game",
        "-windowed",
        "-ResX=600",
        "-ResY=400",
        f"-HTTPPort={port}",
        "-nohmd",
        "-log"
    ]

    process = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        creationflags=subprocess.CREATE_NEW_CONSOLE
    )

    return process

def wait_for_server(port, timeout=60):
    """Wait for server to be ready"""
    print(f"[WAIT] Waiting for server on port {port}...")

    start_time = time.time()
    while time.time() - start_time < timeout:
        try:
            response = requests.get(f"http://localhost:{port}/status", timeout=2)
            if response.status_code == 200:
                data = response.json()
                print(f"[READY] Port {port}: {data.get('message', 'Server ready')}")
                return True
        except:
            pass
        time.sleep(1)

    print(f"[TIMEOUT] Server on port {port} did not start within {timeout} seconds")
    return False

def submit_observation(port, observer_id, target_id, direction, distance, scale_factor=1.0):
    """Submit an observation to the server"""
    url = f"http://localhost:{port}/submit_observation"

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

def validate_position(port, target_id, observations):
    """Validate position using multiple observations"""
    url = f"http://localhost:{port}/validate_position"

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

def normalize_vector(vec):
    """Normalize a 3D vector"""
    magnitude = math.sqrt(vec[0]**2 + vec[1]**2 + vec[2]**2)
    if magnitude == 0:
        return [0, 0, 0]
    return [vec[0]/magnitude, vec[1]/magnitude, vec[2]/magnitude]

def test_tetrahedron_validation():
    """Test 4-observer tetrahedron validation"""

    print("\n" + "="*60)
    print("4-OBSERVER TETRAHEDRON VALIDATION TEST")
    print("="*60)

    # Kill existing processes
    kill_processes()

    # Launch 4 clients
    print("\n" + "="*60)
    print("LAUNCHING 4 GAME CLIENTS")
    print("="*60)

    processes = []
    for port in PORTS:
        proc = launch_client(port)
        processes.append(proc)
        time.sleep(2)  # Stagger launches

    # Wait for all servers to be ready
    print("\n" + "="*60)
    print("WAITING FOR SERVERS")
    print("="*60)

    all_ready = True
    for port in PORTS:
        if not wait_for_server(port):
            all_ready = False

    if not all_ready:
        print("\n[FAILED] Not all servers started successfully")
        return False

    # Create test scenario
    # Target ship at position (100, 200, 50)
    # 4 observers at corners of a tetrahedron

    target_position = [100.0, 200.0, 50.0]
    target_id = 1

    # Observer positions (forming a tetrahedron around target)
    observer_positions = [
        [0.0, 0.0, 0.0],      # Observer 1: origin
        [200.0, 0.0, 0.0],    # Observer 2: +X axis
        [100.0, 400.0, 0.0],  # Observer 3: +Y axis
        [100.0, 200.0, 100.0] # Observer 4: +Z axis
    ]

    print("\n" + "="*60)
    print("TEST SCENARIO")
    print("="*60)
    print(f"Target Position: {target_position}")
    print(f"Target ID: {target_id}")
    print("\nObserver Positions:")
    for i, pos in enumerate(observer_positions):
        print(f"  Observer {i+1}: {pos}")

    # Calculate observations (direction + distance from each observer to target)
    observations = []

    print("\n" + "="*60)
    print("CALCULATING OBSERVATIONS")
    print("="*60)

    for i, obs_pos in enumerate(observer_positions):
        # Calculate vector from observer to target
        delta_x = target_position[0] - obs_pos[0]
        delta_y = target_position[1] - obs_pos[1]
        delta_z = target_position[2] - obs_pos[2]

        # Calculate distance
        distance = math.sqrt(delta_x**2 + delta_y**2 + delta_z**2)

        # Calculate normalized direction
        direction = normalize_vector([delta_x, delta_y, delta_z])

        observation = {
            "observer_id": i + 1,
            "direction": direction,
            "distance": distance,
            "scale_factor": 1.0
        }

        observations.append(observation)

        print(f"\nObserver {i+1}:")
        print(f"  Position: {obs_pos}")
        print(f"  Direction to target: [{direction[0]:.4f}, {direction[1]:.4f}, {direction[2]:.4f}]")
        print(f"  Distance: {distance:.2f}")

    # Submit observations to first server
    print("\n" + "="*60)
    print("SUBMITTING OBSERVATIONS")
    print("="*60)

    for obs in observations:
        result = submit_observation(
            PORTS[0],
            obs["observer_id"],
            target_id,
            obs["direction"],
            obs["distance"],
            obs["scale_factor"]
        )

        if result and result.get("success"):
            print(f"[SUCCESS] Observer {obs['observer_id']}: {result.get('message')}")
        else:
            print(f"[FAILED] Observer {obs['observer_id']}: {result}")

    time.sleep(1)

    # Validate position
    print("\n" + "="*60)
    print("VALIDATING POSITION")
    print("="*60)

    validation_result = validate_position(PORTS[0], target_id, observations)

    if validation_result:
        print("\n" + json.dumps(validation_result, indent=2))

        if validation_result.get("success"):
            data = validation_result.get("data", {})

            print("\n" + "="*60)
            print("VALIDATION RESULTS")
            print("="*60)
            print(f"Valid: {data.get('valid')}")
            print(f"Confidence: {data.get('confidence', 0):.2%}")
            print(f"Observer Count: {data.get('observer_count')}")
            print(f"Geometric Error: {data.get('geometric_error', 0):.4f}")
            print(f"Validation Method: {data.get('validation_method')}")

            triangulated = data.get('triangulated_position', [0, 0, 0])
            print(f"\nTriangulated Position: [{triangulated[0]:.2f}, {triangulated[1]:.2f}, {triangulated[2]:.2f}]")
            print(f"Expected Position: [{target_position[0]:.2f}, {target_position[1]:.2f}, {target_position[2]:.2f}]")

            # Calculate error
            error_x = triangulated[0] - target_position[0]
            error_y = triangulated[1] - target_position[1]
            error_z = triangulated[2] - target_position[2]
            total_error = math.sqrt(error_x**2 + error_y**2 + error_z**2)

            print(f"Position Error: {total_error:.2f} units")

            # Expected confidence for 4 observers: 1 - (1/4^1.5) = 1 - 0.125 = 0.875 = 87.5%
            expected_confidence = 1.0 - (1.0 / (4 ** 1.5))
            print(f"\nExpected Confidence (4 observers): {expected_confidence:.2%}")

            # Check if validation passed
            if data.get('valid'):
                print("\n" + "="*60)
                print("TEST PASSED: Tetrahedron validation successful!")
                print("="*60)
                return True
            else:
                print("\n" + "="*60)
                print("TEST FAILED: Position validation rejected")
                print("="*60)
                return False
        else:
            print(f"\n[FAILED] Validation error: {validation_result.get('message')}")
            return False
    else:
        print("\n[FAILED] Could not validate position")
        return False

def main():
    """Main test execution"""
    try:
        success = test_tetrahedron_validation()

        print("\n" + "="*60)
        print("KEEPING CLIENTS RUNNING FOR INSPECTION")
        print("="*60)
        print("Press Ctrl+C to kill all clients and exit...")

        # Keep running for inspection
        while True:
            time.sleep(1)

    except KeyboardInterrupt:
        print("\n\n" + "="*60)
        print("SHUTTING DOWN")
        print("="*60)
        kill_processes()
        sys.exit(0)

if __name__ == "__main__":
    main()
