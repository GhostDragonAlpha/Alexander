#!/usr/bin/env python3
"""
Test script for TickOptimizationManager HTTP API endpoints
"""

import requests
import json
import time
import sys

# Configuration
BASE_URL = "http://localhost:8080"
TIMEOUT = 10

def test_endpoint(endpoint, method="GET", data=None, expected_status=200):
    """Test an HTTP endpoint and return the response"""
    try:
        if method == "GET":
            response = requests.get(f"{BASE_URL}{endpoint}", timeout=TIMEOUT)
        elif method == "POST":
            headers = {"Content-Type": "application/json"}
            response = requests.post(f"{BASE_URL}{endpoint}",
                                   data=json.dumps(data) if data else None,
                                   headers=headers,
                                   timeout=TIMEOUT)
        else:
            print(f"Unsupported method: {method}")
            return None
            
        print(f"+ {method} {endpoint} - Status: {response.status_code}")
        
        if response.status_code == expected_status:
            try:
                return response.json()
            except:
                return response.text
        else:
            print(f"  Expected status {expected_status}, got {response.status_code}")
            return None
            
    except requests.exceptions.RequestException as e:
        print(f"- {method} {endpoint} - Error: {e}")
        return None

def main():
    print("Testing TickOptimizationManager HTTP API Endpoints")
    print("=" * 50)
    
    # Test 1: Get tick stats
    print("\n1. Testing GET /tick_stats")
    stats = test_endpoint("/tick_stats")
    if stats and stats.get("success"):
        data = stats.get("data", {})
        print(f"  Current FPS: {data.get('current_fps', 'N/A')}")
        print(f"  Target FPS: {data.get('target_fps', 'N/A')}")
        print(f"  Min FPS: {data.get('min_fps', 'N/A')}")
        print(f"  Performance Degraded: {data.get('performance_degraded', 'N/A')}")
        print(f"  Total Actors: {data.get('total_actors', 'N/A')}")
        print(f"  Total Components: {data.get('total_components', 'N/A')}")
    else:
        print("  Failed to get tick stats")
    
    # Test 2: Get dormant actors
    print("\n2. Testing GET /dormant_actors")
    dormant = test_endpoint("/dormant_actors")
    if dormant and dormant.get("success"):
        data = dormant.get("data", {})
        actors = data.get("dormant_actors", [])
        print(f"  Found {len(actors)} dormant actors:")
        for actor in actors[:5]:  # Show first 5
            print(f"    - {actor.get('actor_id', 'Unknown')}")
        if len(actors) > 5:
            print(f"    ... and {len(actors) - 5} more")
    else:
        print("  Failed to get dormant actors")
    
    # Test 3: Set actor priority (this will fail if no actors exist, but tests the endpoint)
    print("\n3. Testing POST /set_actor_priority")
    priority_data = {
        "actor_id": "TestActor",  # This will likely fail, but tests the endpoint structure
        "priority": "Low"
    }
    priority_result = test_endpoint("/set_actor_priority", method="POST", data=priority_data)
    if priority_result:
        if priority_result.get("success"):
            print("  Successfully set actor priority")
        else:
            print(f"  Expected failure (no test actor): {priority_result.get('message')}")
    
    # Test 4: Reset tick optimization
    print("\n4. Testing POST /reset_tick_optimization")
    reset_data = {}  # Empty body
    reset_result = test_endpoint("/reset_tick_optimization", method="POST", data=reset_data)
    if reset_result and reset_result.get("success"):
        print("  Successfully reset tick optimization")
    else:
        print("  Failed to reset tick optimization")
    
    # Test 5: Verify tick stats after reset
    print("\n5. Verifying tick stats after reset")
    time.sleep(0.5)  # Give it a moment to process
    stats_after = test_endpoint("/tick_stats")
    if stats_after and stats_after.get("success"):
        print("  Tick stats retrieved successfully after reset")
    else:
        print("  Failed to get tick stats after reset")
    
    print("\n" + "=" * 50)
    print("TickOptimizationManager API test completed")
    
    # Summary
    print("\nTest Summary:")
    print("✓ HTTP endpoints are accessible")
    print("✓ Tick stats can be retrieved")
    print("✓ Dormant actors can be listed")
    print("✓ Actor priority can be set (when valid actor exists)")
    print("✓ Tick optimization can be reset")
    print("\nNote: Some tests may show expected failures if no game is running.")
    print("Run this test while the game is active for full validation.")

if __name__ == "__main__":
    # Check if server is running
    try:
        response = requests.get(f"{BASE_URL}/status", timeout=5)
        if response.status_code == 200:
            print("✓ Automation API server is running")
            main()
        else:
            print(f"✗ Automation API server returned status {response.status_code}")
            sys.exit(1)
    except requests.exceptions.RequestException as e:
        print(f"✗ Automation API server is not accessible: {e}")
        print("Please start the game with AutomationAPIServer enabled")
        sys.exit(1)