"""
Test script for Quality/Scalability endpoints in AutomationAPIServer

Demonstrates usage of the 4 new quality control endpoints:
- GET /quality/get_settings
- POST /quality/set_quality_level
- POST /quality/set_custom
- POST /quality/benchmark
"""

import requests
import json
import time

# Base URL for the API
BASE_URL = "http://localhost:8080"

def print_response(endpoint, response):
    """Pretty print API response"""
    print(f"\n{'='*80}")
    print(f"Endpoint: {endpoint}")
    print(f"Status Code: {response.status_code}")
    print(f"Response:")
    print(json.dumps(response.json(), indent=2))
    print('='*80)

def test_get_quality_settings():
    """Test GET /quality/get_settings - Get current quality settings"""
    print("\n[TEST 1] Getting current quality settings...")

    response = requests.get(f"{BASE_URL}/quality/get_settings")
    print_response("GET /quality/get_settings", response)

    return response.json()

def test_set_quality_level(level):
    """Test POST /quality/set_quality_level - Set quality preset"""
    quality_names = ["Low", "Medium", "High", "Epic", "Cinematic"]
    print(f"\n[TEST 2] Setting quality level to {level} ({quality_names[level]})...")

    data = {
        "level": level
    }

    response = requests.post(
        f"{BASE_URL}/quality/set_quality_level",
        json=data
    )
    print_response("POST /quality/set_quality_level", response)

    return response.json()

def test_set_custom_quality():
    """Test POST /quality/set_custom - Set individual quality settings"""
    print("\n[TEST 3] Setting custom quality (High shadows, Medium everything else)...")

    data = {
        "view_distance": 2,
        "shadows": 3,          # High shadows
        "post_process": 2,
        "textures": 2,
        "effects": 2,
        "foliage": 2,
        "anti_aliasing": 2,
        "global_illumination": 2,
        "reflections": 2,
        "shading": 2
    }

    response = requests.post(
        f"{BASE_URL}/quality/set_custom",
        json=data
    )
    print_response("POST /quality/set_custom", response)

    return response.json()

def test_quality_benchmark():
    """Test POST /quality/benchmark - Run quality benchmark"""
    print("\n[TEST 4] Running quality benchmark (auto-detect best settings)...")
    print("NOTE: This may take 10-30 seconds to run...")

    data = {
        "duration_multiplier": 1.0  # Standard benchmark duration
    }

    response = requests.post(
        f"{BASE_URL}/quality/benchmark",
        json=data
    )
    print_response("POST /quality/benchmark", response)

    return response.json()

def main():
    """Main test flow"""
    print("="*80)
    print("QUALITY/SCALABILITY ENDPOINTS TEST SUITE")
    print("="*80)

    try:
        # Test 1: Get current settings
        current_settings = test_get_quality_settings()
        time.sleep(1)

        # Test 2: Set quality to Low
        test_set_quality_level(0)  # 0 = Low
        time.sleep(1)

        # Verify it changed
        test_get_quality_settings()
        time.sleep(1)

        # Test 3: Set quality to Epic
        test_set_quality_level(3)  # 3 = Epic
        time.sleep(1)

        # Verify it changed
        test_get_quality_settings()
        time.sleep(1)

        # Test 4: Set custom quality settings
        test_set_custom_quality()
        time.sleep(1)

        # Verify custom settings applied
        test_get_quality_settings()
        time.sleep(1)

        # Test 5: Run benchmark (auto-detect best settings)
        test_quality_benchmark()
        time.sleep(1)

        # Verify benchmark settings applied
        test_get_quality_settings()

        print("\n" + "="*80)
        print("ALL TESTS COMPLETED SUCCESSFULLY!")
        print("="*80)

    except requests.exceptions.ConnectionError:
        print("\nERROR: Could not connect to Automation API Server")
        print("Make sure the Unreal Editor is running with the AutomationAPIServer active")
    except Exception as e:
        print(f"\nERROR: {str(e)}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()
