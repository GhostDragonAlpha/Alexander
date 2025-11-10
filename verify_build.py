#!/usr/bin/env python3
"""
Simple automated build verification test.
Tests that the project launches and basic automation API works.
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
                print(f"[OK] Server online after {int(time.time() - start_time)}s")
                return True
        except requests.exceptions.RequestException:
            pass
        time.sleep(3)
    return False

def test_api_status():
    """Test that API status endpoint works"""
    print("\n[TEST] API Status")
    try:
        response = requests.get(f"{API_BASE}/status", timeout=5)
        data = response.json()
        print(f"  Running: {data.get('running', 'unknown')}")
        print(f"  Port: {data.get('port', 'N/A')}")
        return response.status_code == 200
    except Exception as e:
        print(f"  [FAIL] {e}")
        return False

def test_pie_control():
    """Test PIE (Play in Editor) control"""
    print("\n[TEST] PIE Control")
    try:
        # Start PIE
        response = requests.post(f"{API_BASE}/pie/start", timeout=10)
        if response.status_code != 200:
            print(f"  [FAIL] Failed to start PIE: {response.text}")
            return False
        print("  [OK] PIE started")

        time.sleep(5)

        # Stop PIE
        response = requests.post(f"{API_BASE}/pie/stop", timeout=10)
        if response.status_code != 200:
            print(f"  [FAIL] Failed to stop PIE: {response.text}")
            return False
        print("  [OK] PIE stopped")

        return True
    except Exception as e:
        print(f"  [FAIL] {e}")
        return False

def test_world_info():
    """Test getting world information"""
    print("\n[TEST] World Info")
    try:
        response = requests.get(f"{API_BASE}/world/info", timeout=5)
        data = response.json()
        print(f"  World Name: {data.get('world_name', 'N/A')}")
        print(f"  Map Name: {data.get('map_name', 'N/A')}")
        return response.status_code == 200
    except Exception as e:
        print(f"  [FAIL] {e}")
        return False

def main():
    print("="*60)
    print("ALEXANDER PROJECT - AUTOMATED BUILD VERIFICATION")
    print("="*60)

    # Wait for server
    if not wait_for_server():
        print("\n[FAILED] Automation server did not start")
        sys.exit(1)

    # Run tests
    tests = [
        ("API Status", test_api_status),
        ("World Info", test_world_info),
        ("PIE Control", test_pie_control),
    ]

    results = []
    for name, test_func in tests:
        try:
            result = test_func()
            results.append((name, result))
        except Exception as e:
            print(f"\n[ERROR] Test '{name}' crashed: {e}")
            results.append((name, False))

    # Summary
    print("\n" + "="*60)
    print("TEST SUMMARY")
    print("="*60)
    passed = sum(1 for _, r in results if r)
    total = len(results)
    for name, result in results:
        status = "[PASS]" if result else "[FAIL]"
        print(f"  {status}: {name}")

    print(f"\nResult: {passed}/{total} tests passed")
    print("="*60)

    if passed == total:
        print("\n*** ALL TESTS PASSED ***")
    else:
        print(f"\n*** {total - passed} TEST(S) FAILED ***")

    sys.exit(0 if passed == total else 1)

if __name__ == "__main__":
    main()
