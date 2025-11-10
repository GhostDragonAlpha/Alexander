#!/usr/bin/env python3
"""
Test script for Physics Query Endpoints
Demonstrates usage of the new physics endpoints in the Automation API
"""

import requests
import json

BASE_URL = "http://localhost:8080"

def test_raycast():
    """Test POST /physics/raycast endpoint"""
    print("\n=== Testing Physics Raycast ===")

    # Raycast downward from a point 1000 units above origin
    payload = {
        "start": [0, 0, 1000],
        "end": [0, 0, -1000],
        "channel": "Visibility"
    }

    response = requests.post(f"{BASE_URL}/physics/raycast", json=payload)
    result = response.json()

    print(f"Status: {response.status_code}")
    print(f"Response: {json.dumps(result, indent=2)}")

    if result.get('success') and result.get('data', {}).get('hit'):
        data = result['data']
        print(f"✓ Hit detected!")
        print(f"  Actor: {data.get('actor_name')}")
        print(f"  Component: {data.get('component')}")
        print(f"  Location: {data.get('location')}")
        print(f"  Normal: {data.get('normal')}")
        print(f"  Distance: {data.get('distance')}")
    else:
        print("✗ No hit detected")

    return result

def test_sphere_trace():
    """Test POST /physics/sphere_trace endpoint"""
    print("\n=== Testing Physics Sphere Trace ===")

    # Sphere sweep from origin to 500 units forward with 50 unit radius
    payload = {
        "start": [0, 0, 100],
        "end": [500, 0, 100],
        "radius": 50.0,
        "channel": "WorldDynamic"
    }

    response = requests.post(f"{BASE_URL}/physics/sphere_trace", json=payload)
    result = response.json()

    print(f"Status: {response.status_code}")
    print(f"Response: {json.dumps(result, indent=2)}")

    if result.get('success') and result.get('data', {}).get('hit'):
        data = result['data']
        print(f"✓ Hit detected!")
        print(f"  Actor: {data.get('actor_name')}")
        print(f"  Component: {data.get('component')}")
        print(f"  Location: {data.get('location')}")
        print(f"  Normal: {data.get('normal')}")
        print(f"  Impact Point: {data.get('impact_point')}")
        print(f"  Distance: {data.get('distance')}")
        print(f"  Radius: {data.get('radius')}")
    else:
        print("✗ No hit detected")

    return result

def test_gravity():
    """Test GET /physics/gravity endpoint"""
    print("\n=== Testing Physics Gravity ===")

    response = requests.get(f"{BASE_URL}/physics/gravity")
    result = response.json()

    print(f"Status: {response.status_code}")
    print(f"Response: {json.dumps(result, indent=2)}")

    if result.get('success'):
        data = result['data']
        print(f"✓ Gravity retrieved!")
        print(f"  Current Gravity Z: {data.get('current_gravity')}")
        print(f"  Default Gravity Z: {data.get('default_gravity')}")
    else:
        print("✗ Failed to retrieve gravity")

    return result

def test_all_collision_channels():
    """Test raycast with different collision channels"""
    print("\n=== Testing Different Collision Channels ===")

    channels = ["Visibility", "Camera", "WorldStatic", "WorldDynamic", "Pawn", "Vehicle"]

    for channel in channels:
        payload = {
            "start": [0, 0, 1000],
            "end": [0, 0, -1000],
            "channel": channel
        }

        response = requests.post(f"{BASE_URL}/physics/raycast", json=payload)
        result = response.json()

        if result.get('success'):
            hit = result.get('data', {}).get('hit', False)
            status = "HIT" if hit else "NO HIT"
            print(f"  {channel:15s}: {status}")
        else:
            print(f"  {channel:15s}: ERROR")

def main():
    """Run all tests"""
    print("Physics Query Endpoints Test Suite")
    print("===================================")
    print(f"Base URL: {BASE_URL}")

    try:
        # Test status endpoint first to verify connection
        response = requests.get(f"{BASE_URL}/status")
        if response.status_code == 200:
            print("✓ Server connection successful")
        else:
            print("✗ Server connection failed")
            return

        # Run tests
        test_gravity()
        test_raycast()
        test_sphere_trace()
        test_all_collision_channels()

        print("\n=== All Tests Complete ===")

    except requests.exceptions.ConnectionError:
        print(f"✗ ERROR: Could not connect to {BASE_URL}")
        print("  Make sure the Automation API Server is running in Unreal Engine")
    except Exception as e:
        print(f"✗ ERROR: {e}")

if __name__ == "__main__":
    main()
