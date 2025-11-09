"""
Test script for AssetRegistry endpoints
Demonstrates the 4 new AssetRegistry API endpoints
"""

import requests
import json

BASE_URL = "http://localhost:8080"

def test_assets_search():
    """Test GET/POST /assets/search - Advanced asset search with filters"""
    print("\n=== Testing /assets/search ===")

    # Test 1: Search by class filter
    payload = {
        "class_filter": "/Script/Engine.Blueprint"
    }
    response = requests.post(f"{BASE_URL}/assets/search", json=payload)
    print(f"Search blueprints: {response.status_code}")
    if response.ok:
        data = response.json()
        print(f"  Found {data['data']['count']} blueprints")

    # Test 2: Search by path filter
    payload = {
        "path_filter": "/Game/Content"
    }
    response = requests.post(f"{BASE_URL}/assets/search", json=payload)
    print(f"Search in /Game/Content: {response.status_code}")
    if response.ok:
        data = response.json()
        print(f"  Found {data['data']['count']} assets")

    # Test 3: Search by tag
    payload = {
        "tag_key": "Type",
        "tag_value": "Weapon"
    }
    response = requests.post(f"{BASE_URL}/assets/search", json=payload)
    print(f"Search by tag Type=Weapon: {response.status_code}")
    if response.ok:
        data = response.json()
        print(f"  Found {data['data']['count']} assets")

def test_assets_by_class():
    """Test GET /assets/by_class/{class_name} - Find assets by class"""
    print("\n=== Testing /assets/by_class/{class_name} ===")

    # Test: Find all blueprints
    response = requests.get(f"{BASE_URL}/assets/by_class//Script/Engine.Blueprint")
    print(f"Find all blueprints: {response.status_code}")
    if response.ok:
        data = response.json()
        print(f"  Found {data['data']['count']} blueprints")
        if data['data']['assets']:
            print(f"  Example: {data['data']['assets'][0]['path']}")

    # Test: Find all materials
    response = requests.get(f"{BASE_URL}/assets/by_class//Script/Engine.Material")
    print(f"Find all materials: {response.status_code}")
    if response.ok:
        data = response.json()
        print(f"  Found {data['data']['count']} materials")

def test_assets_by_tag():
    """Test GET /assets/by_tag/{tag_name}/{tag_value} - Find assets by tag"""
    print("\n=== Testing /assets/by_tag/{tag_name}/{tag_value} ===")

    # Test: Find assets with specific tag
    response = requests.get(f"{BASE_URL}/assets/by_tag/Type/Weapon")
    print(f"Find assets with Type=Weapon: {response.status_code}")
    if response.ok:
        data = response.json()
        print(f"  Found {data['data']['count']} assets")
        print(f"  Tag: {data['data']['tag_name']}={data['data']['tag_value']}")

    # Test: Find assets with different tag
    response = requests.get(f"{BASE_URL}/assets/by_tag/Category/Environment")
    print(f"Find assets with Category=Environment: {response.status_code}")
    if response.ok:
        data = response.json()
        print(f"  Found {data['data']['count']} assets")

def test_assets_unused():
    """Test GET /assets/unused - Find unreferenced assets"""
    print("\n=== Testing /assets/unused ===")

    response = requests.get(f"{BASE_URL}/assets/unused")
    print(f"Find unused assets: {response.status_code}")
    if response.ok:
        data = response.json()
        print(f"  Total assets: {data['data']['total_count']}")
        print(f"  Checked: {data['data']['checked_count']}")
        print(f"  Unused: {data['data']['unused_count']}")
        if data['data']['unused_assets']:
            print(f"  First unused asset: {data['data']['unused_assets'][0]['path']}")

if __name__ == "__main__":
    print("AssetRegistry Endpoints Test")
    print("=" * 50)

    try:
        # Test server is running
        response = requests.get(f"{BASE_URL}/status")
        if not response.ok:
            print("ERROR: Automation server is not running!")
            exit(1)

        # Run all tests
        test_assets_search()
        test_assets_by_class()
        test_assets_by_tag()
        test_assets_unused()

        print("\n" + "=" * 50)
        print("All tests completed!")

    except requests.exceptions.ConnectionError:
        print("ERROR: Cannot connect to automation server at", BASE_URL)
        print("Make sure the server is running in Unreal Editor")
