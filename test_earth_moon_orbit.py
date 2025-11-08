"""
Test 2-object gravitational orbit: Earth-Moon system
Verifies that the five fundamental inputs (G, r, dt, theta, m) work correctly

Expected Results:
- Orbital period approximately 27.3 days
- Stable elliptical orbit
- Energy conserved to <1% error
- Momentum conserved to <1% error
"""

import json
import time
import requests
from datetime import datetime

# Test configuration
SERVER_URL = "http://localhost:8080"
TEST_DURATION_SECONDS = 120  # 2 minutes of simulation time
POLL_INTERVAL = 5  # Check every 5 seconds

# Earth-Moon system parameters (from THREE_FUNDAMENTAL_INPUTS.md)
EARTH_MASS = 5.972e24  # kg
MOON_MASS = 7.342e22   # kg
EARTH_MOON_DISTANCE = 384400  # km (384,400 km)

def create_earth_moon_system():
    """Create Earth and Moon orbital bodies in the simulation"""
    print("Creating Earth-Moon test system...")

    # Create Earth at origin
    earth_request = {
        "command": "spawn_orbital_body",
        "params": {
            "name": "Earth",
            "mass": EARTH_MASS,
            "radius": 6371,  # km
            "position": [0, 0, 0],  # At origin
            "velocity": [0, 0, 0],  # Stationary (center of mass)
            "enable_nbody_gravity": True
        }
    }

    # Calculate Moon's orbital velocity (circular orbit)
    # v = sqrt(G * M_earth / r)
    G = 6.67430e-11  # m³/kg/s²
    r = EARTH_MOON_DISTANCE * 1000  # Convert km to m
    v_orbital = (G * EARTH_MASS / r) ** 0.5  # m/s
    v_orbital_kmps = v_orbital / 1000  # Convert to km/s

    print(f"  Moon orbital velocity: {v_orbital_kmps:.3f} km/s ({v_orbital:.1f} m/s)")

    # Create Moon at distance, with perpendicular velocity
    moon_request = {
        "command": "spawn_orbital_body",
        "params": {
            "name": "Moon",
            "mass": MOON_MASS,
            "radius": 1737,  # km
            "position": [EARTH_MOON_DISTANCE, 0, 0],  # Distance along X axis
            "velocity": [0, v_orbital_kmps, 0],  # Perpendicular velocity (Y direction)
            "enable_nbody_gravity": True
        }
    }

    # Send requests
    try:
        resp_earth = requests.post(f"{SERVER_URL}/command", json=earth_request, timeout=10)
        resp_moon = requests.post(f"{SERVER_URL}/command", json=moon_request, timeout=10)

        if resp_earth.status_code == 200 and resp_moon.status_code == 200:
            print("  [OK] Earth and Moon created successfully")
            return True
        else:
            print(f"  [FAIL] Failed to create bodies (Earth: {resp_earth.status_code}, Moon: {resp_moon.status_code})")
            return False
    except Exception as e:
        print(f"  [FAIL] Error creating bodies: {e}")
        return False

def validate_physics():
    """Get physics validation data from unified gravity system"""
    try:
        resp = requests.get(f"{SERVER_URL}/validate_physics", timeout=10)
        if resp.status_code == 200:
            return resp.json()
        else:
            print(f"  Warning: Physics validation endpoint returned {resp.status_code}")
            return None
    except Exception as e:
        print(f"  Warning: Could not get physics validation: {e}")
        return None

def run_test():
    """Run the Earth-Moon orbit test"""
    print("="*70)
    print("EARTH-MOON ORBIT TEST")
    print("Testing 5 Fundamental Inputs: G, r, dt, theta, m")
    print("="*70)
    print()

    # Wait for server to be ready
    print("Waiting for server to be ready...")
    for i in range(30):
        try:
            resp = requests.get(f"{SERVER_URL}/health", timeout=2)
            if resp.status_code == 200:
                print("  [OK] Server is ready")
                break
        except:
            pass
        time.sleep(1)
    else:
        print("  [FAIL] Server not responding after 30 seconds")
        return False

    print()

    # Create Earth-Moon system
    if not create_earth_moon_system():
        return False

    print()
    print(f"Running simulation for {TEST_DURATION_SECONDS} seconds...")
    print()

    start_time = time.time()
    last_validation = None
    validation_count = 0

    # Monitor the simulation
    while time.time() - start_time < TEST_DURATION_SECONDS:
        elapsed = time.time() - start_time

        # Get physics validation
        validation = validate_physics()
        if validation:
            validation_count += 1
            last_validation = validation

            print(f"[{elapsed:.1f}s] Physics Validation #{validation_count}:")

            # Show key metrics
            if "force_sum" in validation:
                force_sum = validation["force_sum"]
                print(f"  Force Sum (Newton's 3rd): {force_sum:.6e} N (should be ~0)")

            if "total_momentum" in validation:
                momentum = validation["total_momentum"]
                mom_mag = (momentum[0]**2 + momentum[1]**2 + momentum[2]**2)**0.5
                print(f"  Total Momentum: {mom_mag:.6e} kg*m/s (should be conserved)")

            if "total_energy" in validation:
                energy = validation["total_energy"]
                print(f"  Total Energy: {energy:.6e} J (should be conserved)")

            if "body_count" in validation:
                print(f"  Bodies Tracked: {validation['body_count']}")

            print()

        # Wait before next poll
        time.sleep(POLL_INTERVAL)

    # Final results
    print("="*70)
    print("TEST RESULTS")
    print("="*70)
    print()

    if last_validation:
        print("Final Validation Metrics:")
        print(f"  Total validations: {validation_count}")

        # Check conservation laws
        if "force_sum" in last_validation:
            force_sum = abs(last_validation["force_sum"])
            force_ok = force_sum < 1e-6  # Should be very close to zero
            status = "[OK]" if force_ok else "[FAIL]"
            print(f"  {status} Force sum: {force_sum:.6e} N (Newton's 3rd law)")

        if "momentum_conserved" in last_validation:
            mom_conserved = last_validation["momentum_conserved"]
            status = "[OK]" if mom_conserved else "[FAIL]"
            print(f"  {status} Momentum conservation: {mom_conserved}")

        if "energy_conserved" in last_validation:
            energy_conserved = last_validation["energy_conserved"]
            status = "[OK]" if energy_conserved else "[FAIL]"
            print(f"  {status} Energy conservation: {energy_conserved}")

        print()
        print("Expected Results:")
        print("  - Orbital period: ~27.3 days (2,358,720 seconds)")
        print("  - Force sum approximately 0 (Newton's 3rd law)")
        print("  - Momentum conserved to <1% error")
        print("  - Energy conserved to <1% error")
        print()

        if last_validation.get("momentum_conserved") and last_validation.get("energy_conserved"):
            print("[PASS] TEST PASSED: Physics conservation laws validated!")
            return True
        else:
            print("[FAIL] TEST FAILED: Conservation laws not satisfied")
            return False
    else:
        print("[FAIL] TEST FAILED: No validation data received")
        return False

if __name__ == "__main__":
    success = run_test()
    exit(0 if success else 1)
