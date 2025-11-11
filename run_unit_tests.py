#!/usr/bin/env python3
"""
API-Based Unit Testing for Alexander Project

This script runs automated unit tests through the Automation API to verify
core functionality without complex visual testing.

Usage:
    python run_unit_tests.py
"""

import requests
import time
import sys
from datetime import datetime
from pathlib import Path

# Configuration
API_BASE = "http://localhost:8080"
OUTPUT_DIR = Path(__file__).parent / "test_results"
TIMEOUT = 10

class TestRunner:
    def __init__(self):
        self.tests_run = 0
        self.tests_passed = 0
        self.tests_failed = 0
        self.results = []

    def test(self, name, func):
        """Run a single test"""
        self.tests_run += 1
        print(f"\n[TEST {self.tests_run}] {name}")

        try:
            result = func()
            if result:
                print(f"  ✅ PASS")
                self.tests_passed += 1
                self.results.append((name, "PASS", None))
                return True
            else:
                print(f"  ❌ FAIL")
                self.tests_failed += 1
                self.results.append((name, "FAIL", "Assertion failed"))
                return False
        except Exception as e:
            print(f"  ❌ ERROR: {e}")
            self.tests_failed += 1
            self.results.append((name, "ERROR", str(e)))
            return False

    def summary(self):
        """Print test summary"""
        print(f"\n{'='*70}")
        print(f"  TEST SUMMARY")
        print(f"{'='*70}\n")
        print(f"Total:  {self.tests_run}")
        print(f"Passed: {self.tests_passed} ✅")
        print(f"Failed: {self.tests_failed} ❌")
        print(f"Success Rate: {(self.tests_passed/self.tests_run*100):.1f}%")

    def save_report(self):
        """Save test report"""
        OUTPUT_DIR.mkdir(exist_ok=True)
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        report_file = OUTPUT_DIR / f"test_report_{timestamp}.txt"

        with open(report_file, 'w') as f:
            f.write(f"Alexander Project - Unit Test Report\n")
            f.write(f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write(f"{'='*70}\n\n")

            f.write(f"Summary:\n")
            f.write(f"  Total Tests: {self.tests_run}\n")
            f.write(f"  Passed: {self.tests_passed}\n")
            f.write(f"  Failed: {self.tests_failed}\n")
            f.write(f"  Success Rate: {(self.tests_passed/self.tests_run*100):.1f}%\n\n")

            f.write(f"Detailed Results:\n")
            f.write(f"{'-'*70}\n")

            for i, (name, status, error) in enumerate(self.results, 1):
                f.write(f"\n{i}. {name}\n")
                f.write(f"   Status: {status}\n")
                if error:
                    f.write(f"   Error: {error}\n")

        print(f"\n📄 Report saved: {report_file}")

# ===== API HELPER FUNCTIONS =====

def api_get(endpoint):
    """Make a GET request to the API"""
    try:
        response = requests.get(f"{API_BASE}{endpoint}", timeout=TIMEOUT)
        return response.json()
    except Exception as e:
        raise Exception(f"API GET failed: {e}")

def api_post(endpoint, data=None):
    """Make a POST request to the API"""
    try:
        response = requests.post(f"{API_BASE}{endpoint}", json=data or {}, timeout=TIMEOUT)
        return response.json()
    except Exception as e:
        raise Exception(f"API POST failed: {e}")

# ===== UNIT TESTS =====

def test_api_status():
    """Test: API server is online"""
    result = api_get("/status")
    return result.get("success") and result.get("data", {}).get("running")

def test_api_port():
    """Test: API is running on correct port"""
    result = api_get("/status")
    return result.get("data", {}).get("port") == 8080

def test_start_pie():
    """Test: Can start PIE"""
    result = api_post("/pie/start")
    time.sleep(2)  # Give PIE time to start
    return result.get("success")

def test_get_player_pawn():
    """Test: Can get player pawn"""
    result = api_get("/get_player_pawn")
    return result.get("success") and result.get("data", {}).get("name")

def test_player_pawn_location():
    """Test: Player pawn has valid location"""
    result = api_get("/get_player_pawn")
    if not result.get("success"):
        return False
    location = result.get("data", {}).get("location")
    return location and len(location) == 3

def test_spawn_ship():
    """Test: Can spawn ship"""
    result = api_post("/spawn_ship", {
        "location": [0, 0, 1000],
        "rotation": [0, 0, 0]
    })
    return result.get("success") and result.get("data", {}).get("ship_id")

def test_list_ships():
    """Test: Can list spawned ships"""
    result = api_get("/list_ships")
    return result.get("success") and "ships" in result.get("data", {})

def test_get_ship_position():
    """Test: Can get ship position"""
    # First spawn a ship
    spawn_result = api_post("/spawn_ship", {
        "location": [1000, 0, 500],
        "rotation": [0, 0, 0]
    })
    if not spawn_result.get("success"):
        return False

    ship_id = spawn_result.get("data", {}).get("ship_id")
    if not ship_id:
        return False

    # Get its position
    pos_result = api_get(f"/get_position/{ship_id}")
    return pos_result.get("success") and pos_result.get("data", {}).get("location")

def test_set_input_forward():
    """Test: Can send input commands"""
    result = api_post("/set_input", {
        "forward": 1.0,
        "right": 0.0,
        "up": 0.0
    })
    return result.get("success")

def test_set_input_turn():
    """Test: Can send turn commands"""
    result = api_post("/set_input", {
        "pitch": 0.5,
        "yaw": 0.5,
        "roll": 0.0
    })
    return result.get("success")

def test_thrust_control():
    """Test: Can control thrust"""
    result = api_post("/set_thrust", {"thrust": 0.5})
    return result.get("success")

def test_stop_pie():
    """Test: Can stop PIE"""
    result = api_post("/pie/stop")
    return result.get("success")

# ===== MAIN TEST SUITE =====

def main():
    print(f"{'='*70}")
    print(f"  ALEXANDER PROJECT - UNIT TEST SUITE")
    print(f"{'='*70}")
    print(f"\nAPI Base: {API_BASE}")
    print(f"Output: {OUTPUT_DIR}")

    runner = TestRunner()

    # Phase 1: API Health
    print(f"\n{'='*70}")
    print(f"  PHASE 1: API Health")
    print(f"{'='*70}")

    runner.test("API server is online", test_api_status)
    runner.test("API is on correct port", test_api_port)

    # Phase 2: PIE Control
    print(f"\n{'='*70}")
    print(f"  PHASE 2: PIE Control")
    print(f"{'='*70}")

    runner.test("Start PIE", test_start_pie)
    time.sleep(3)  # Wait for PIE to fully start

    # Phase 3: Player Pawn
    print(f"\n{'='*70}")
    print(f"  PHASE 3: Player Pawn")
    print(f"{'='*70}")

    runner.test("Get player pawn", test_get_player_pawn)
    runner.test("Player has valid location", test_player_pawn_location)

    # Phase 4: Ship Spawning
    print(f"\n{'='*70}")
    print(f"  PHASE 4: Ship Spawning")
    print(f"{'='*70}")

    runner.test("Spawn ship", test_spawn_ship)
    runner.test("List ships", test_list_ships)
    runner.test("Get ship position", test_get_ship_position)

    # Phase 5: Input Control
    print(f"\n{'='*70}")
    print(f"  PHASE 5: Input Control")
    print(f"{'='*70}")

    runner.test("Send forward input", test_set_input_forward)
    runner.test("Send turn input", test_set_input_turn)
    runner.test("Control thrust", test_thrust_control)

    # Phase 6: Cleanup
    print(f"\n{'='*70}")
    print(f"  PHASE 6: Cleanup")
    print(f"{'='*70}")

    runner.test("Stop PIE", test_stop_pie)

    # Final summary
    runner.summary()
    runner.save_report()

    # Exit code
    return 0 if runner.tests_failed == 0 else 1

if __name__ == "__main__":
    sys.exit(main())
