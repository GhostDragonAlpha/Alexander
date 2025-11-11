#!/usr/bin/env python3
"""
Create planet via API and run gameplay tests
This script spawns a planet actor and verifies gravity works
"""

import requests
import time
import json

API_BASE = "http://localhost:8080"

def spawn_planet_via_api():
    """Spawn a planet actor via the automation API"""
    print("=" * 70)
    print("SPAWNING PLANET VIA AUTOMATION API")
    print("=" * 70)

    # Planet class path
    planet_class = "/Script/Alexander.Planet"

    # Spawn at origin
    payload = {
        "actor_class": planet_class,
        "location": [0, 0, 0],
        "rotation": [0, 0, 0]
    }

    print(f"\n[STEP] Spawning planet at origin...")
    print(f"  Class: {planet_class}")

    try:
        response = requests.post(f"{API_BASE}/spawn_actor", json=payload, timeout=10)
        data = response.json()

        if response.status_code == 200 and data.get("success"):
            print(f"  [OK] Planet spawned successfully!")
            actor_name = data.get("data", {}).get("actor_name", "Unknown")
            print(f"  Actor name: {actor_name}")
            return True
        else:
            print(f"  [FAIL] {data.get('message')}")
            return False
    except Exception as e:
        print(f"  [FAIL] Exception: {e}")
        return False

def verify_planet_exists():
    """Check if planet exists in level"""
    print(f"\n[STEP] Verifying planet exists...")

    # We can't directly query for planets via API, but we can spawn a ship
    # and check if it's affected by gravity later
    print("  [INFO] Will verify via gravity test")
    return True

def main():
    """Main execution"""
    print("\n" + "=" * 70)
    print("PLANET CREATION AND TESTING WORKFLOW")
    print("=" * 70)
    print("\nThis script will:")
    print("  1. Spawn a planet actor in the level")
    print("  2. Run planet gameplay tests")
    print("  3. Verify gravity simulation works")
    print()

    # Check API is available
    try:
        response = requests.get(f"{API_BASE}/status", timeout=5)
        if response.status_code != 200:
            print("[FAIL] Automation API not responding")
            return False
    except:
        print("[FAIL] Cannot connect to automation API")
        print("       Ensure Unreal Editor is running with PIE active")
        return False

    # Start PIE if not running
    print("\n[STEP] Ensuring PIE is running...")
    try:
        response = requests.post(f"{API_BASE}/pie/start", timeout=10)
        data = response.json()
        if data.get("success") or "already running" in data.get("message", "").lower():
            print("  [OK] PIE is running")
            time.sleep(3)
        else:
            print(f"  [WARN] {data.get('message')}")
    except Exception as e:
        print(f"  [WARN] {e}")

    # Spawn planet
    if not spawn_planet_via_api():
        print("\n[ATTENTION] Planet spawning failed")
        print("This might mean:")
        print("  1. Planet class is not available in editor-only mode")
        print("  2. Need to use Python script in Unreal Editor instead")
        print("\nAlternative approach:")
        print("  1. Open Unreal Editor")
        print("  2. In Python console, run:")
        print("     py Content/Python/setup_planet_test_level.py close")
        return False

    # Verify planet exists
    verify_planet_exists()

    print("\n" + "=" * 70)
    print("PLANET CREATION COMPLETE")
    print("=" * 70)
    print("\nNow running planet gameplay tests...")
    print()

    # Run the test suite
    import subprocess
    result = subprocess.run(["python", "test_planet_gameplay.py"],
                          capture_output=False, text=True)

    return result.returncode == 0

if __name__ == "__main__":
    success = main()
    exit(0 if success else 1)
