#!/usr/bin/env python3
"""
Test script to verify spawn_ship endpoint fix.
Tests both scenarios: without PIE and with PIE running.
"""

import requests
import time
import sys

API_BASE = "http://localhost:8080"

def wait_for_server(timeout=120):
    """Wait for automation server to come online"""
    print("Waiting for Unreal Editor and Automation API server to start...")
    start_time = time.time()
    while time.time() - start_time < timeout:
        try:
            response = requests.get(f"{API_BASE}/status", timeout=2)
            if response.status_code == 200:
                elapsed = int(time.time() - start_time)
                print(f"[OK] Server online after {elapsed}s")
                return True
        except requests.exceptions.RequestException:
            pass
        time.sleep(3)
    return False

def test_spawn_without_pie():
    """Test spawn_ship WITHOUT PIE running - should get error, not crash"""
    print("\n[TEST 1] Spawn ship without PIE running")
    print("Expected: Error message (not crash)")

    try:
        payload = {
            "location": [0, 0, 500],
            "ship_class": "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer_C"
        }

        response = requests.post(f"{API_BASE}/spawn_ship", json=payload, timeout=10)

        if response.status_code == 200:
            data = response.json()
            if data.get("success") == False:
                print(f"  [PASS] Got expected error: {data.get('message')}")
                return True
            else:
                print(f"  [UNEXPECTED] Got success when expecting error: {data}")
                return False
        else:
            print(f"  [FAIL] Bad status code: {response.status_code}")
            return False

    except requests.exceptions.Timeout:
        print("  [FAIL] Request timeout - server may have crashed")
        return False
    except Exception as e:
        print(f"  [FAIL] Exception: {e}")
        return False

def test_spawn_with_pie():
    """Test spawn_ship WITH PIE running - should work properly"""
    print("\n[TEST 2] Spawn ship with PIE running")
    print("Starting PIE first...")

    try:
        # Start PIE
        response = requests.post(f"{API_BASE}/pie/start", timeout=15)
        if response.status_code != 200:
            print(f"  [FAIL] Could not start PIE: {response.text}")
            return False
        print("  [OK] PIE started")

        time.sleep(5)

        # Try to spawn ship
        payload = {
            "location": [0, 0, 500],
            "ship_class": "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer_C"
        }

        response = requests.post(f"{API_BASE}/spawn_ship", json=payload, timeout=10)
        data = response.json()

        if response.status_code == 200 and data.get("success") == True:
            print(f"  [PASS] Ship spawned successfully: {data.get('message')}")
            ship_id = data.get('data', {}).get('ship_id')
            print(f"         Ship ID: {ship_id}")
            success = True
        else:
            print(f"  [FAIL] Failed to spawn: {data.get('message')}")
            success = False

        # Stop PIE
        time.sleep(2)
        requests.post(f"{API_BASE}/pie/stop", timeout=10)
        print("  [OK] PIE stopped")

        return success

    except Exception as e:
        print(f"  [FAIL] Exception: {e}")
        # Try to stop PIE if still running
        try:
            requests.post(f"{API_BASE}/pie/stop", timeout=5)
        except:
            pass
        return False

def main():
    print("="*70)
    print("SPAWN_SHIP ENDPOINT FIX VERIFICATION")
    print("="*70)

    # Wait for server
    if not wait_for_server():
        print("\n[FAILED] Automation server did not start")
        sys.exit(1)

    # Run tests
    results = []

    # Test 1: Spawn without PIE (should get error, not crash)
    results.append(("Spawn without PIE", test_spawn_without_pie()))

    # Test 2: Spawn with PIE (should work)
    results.append(("Spawn with PIE", test_spawn_with_pie()))

    # Summary
    print("\n" + "="*70)
    print("TEST SUMMARY")
    print("="*70)

    passed = sum(1 for _, r in results if r)
    total = len(results)

    for name, result in results:
        status = "[PASS]" if result else "[FAIL]"
        print(f"  {status}: {name}")

    print(f"\nResult: {passed}/{total} tests passed")
    print("="*70)

    if passed == total:
        print("\n*** ALL TESTS PASSED - FIX VERIFIED ***")
    else:
        print(f"\n*** {total - passed} TEST(S) FAILED ***")

    sys.exit(0 if passed == total else 1)

if __name__ == "__main__":
    main()
