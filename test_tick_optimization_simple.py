#!/usr/bin/env python3
"""
Simple test script for TickOptimizationManager HTTP API endpoints
Tests basic connectivity without requiring a running game
"""

import requests
import json
import sys

# Configuration
BASE_URL = "http://localhost:8080"
TIMEOUT = 5

def test_endpoint(endpoint, method="GET", data=None):
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
            
        print(f"  {method} {endpoint} - Status: {response.status_code}")
        
        if response.status_code == 200:
            try:
                return response.json()
            except:
                return response.text
        else:
            print(f"    Expected status 200, got {response.status_code}")
            return None
            
    except requests.exceptions.RequestException as e:
        print(f"  {method} {endpoint} - Error: {e}")
        return None

def main():
    print("TickOptimizationManager API Endpoint Test")
    print("=" * 40)
    
    # Check if server is running
    print("\n1. Checking server status...")
    try:
        response = requests.get(f"{BASE_URL}/status", timeout=TIMEOUT)
        if response.status_code == 200:
            print("  Server is accessible")
        else:
            print(f"  Server returned status {response.status_code}")
            return False
    except requests.exceptions.RequestException as e:
        print(f"  Server is not accessible: {e}")
        print("  Please start the game with AutomationAPIServer enabled")
        return False
    
    # Test tick stats endpoint
    print("\n2. Testing /tick_stats endpoint...")
    stats = test_endpoint("/tick_stats")
    if stats:
        print("  Response received successfully")
        if isinstance(stats, dict):
            print(f"  Success: {stats.get('success', 'N/A')}")
            if stats.get('data'):
                print(f"  Current FPS: {stats['data'].get('current_fps', 'N/A')}")
                print(f"  Target FPS: {stats['data'].get('target_fps', 'N/A')}")
    else:
        print("  Failed to get tick stats")
    
    # Test dormant actors endpoint
    print("\n3. Testing /dormant_actors endpoint...")
    dormant = test_endpoint("/dormant_actors")
    if dormant:
        print("  Response received successfully")
        if isinstance(dormant, dict):
            print(f"  Success: {dormant.get('success', 'N/A')}")
            if dormant.get('data', {}).get('dormant_actors'):
                count = len(dormant['data']['dormant_actors'])
                print(f"  Dormant actors found: {count}")
    else:
        print("  Failed to get dormant actors")
    
    # Test set actor priority endpoint
    print("\n4. Testing /set_actor_priority endpoint...")
    priority_data = {
        "actor_id": "TestActor",
        "priority": "Low"
    }
    priority_result = test_endpoint("/set_actor_priority", method="POST", data=priority_data)
    if priority_result:
        print("  Response received successfully")
        if isinstance(priority_result, dict):
            print(f"  Success: {priority_result.get('success', 'N/A')}")
            print(f"  Message: {priority_result.get('message', 'N/A')}")
    
    # Test reset tick optimization endpoint
    print("\n5. Testing /reset_tick_optimization endpoint...")
    reset_result = test_endpoint("/reset_tick_optimization", method="POST", data={})
    if reset_result:
        print("  Response received successfully")
        if isinstance(reset_result, dict):
            print(f"  Success: {reset_result.get('success', 'N/A')}")
            print(f"  Message: {reset_result.get('message', 'N/A')}")
    
    print("\n" + "=" * 40)
    print("Test completed")
    return True

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)