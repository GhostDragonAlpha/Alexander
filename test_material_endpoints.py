#!/usr/bin/env python3
"""
Test Material and Texture Query Endpoints
"""

import requests
import json

BASE_URL = "http://localhost:8080"

def test_materials_list():
    """Test GET /materials/list"""
    print("\n=== Testing GET /materials/list ===")

    # Test without filter
    response = requests.get(f"{BASE_URL}/materials/list")
    print(f"Status: {response.status_code}")
    data = response.json()
    print(f"Success: {data.get('success')}")
    print(f"Message: {data.get('message')}")
    if 'data' in data:
        count = data['data'].get('count', 0)
        print(f"Materials found: {count}")
        if count > 0:
            print(f"First material: {data['data']['materials'][0]}")

    # Test with filter
    print("\n--- Testing with path filter ---")
    response = requests.post(f"{BASE_URL}/materials/list",
                            json={"path_filter": "/Game/"})
    data = response.json()
    print(f"Success: {data.get('success')}")
    if 'data' in data:
        print(f"Filtered materials found: {data['data'].get('count', 0)}")


def test_material_properties():
    """Test GET /materials/get_properties/{path}"""
    print("\n=== Testing GET /materials/get_properties ===")

    # First get a list of materials
    response = requests.get(f"{BASE_URL}/materials/list")
    data = response.json()

    if data.get('success') and 'data' in data:
        materials = data['data'].get('materials', [])
        if materials:
            # Test with first material
            material_path = materials[0]['path']
            print(f"Testing material: {material_path}")

            response = requests.get(f"{BASE_URL}/materials/get_properties/{material_path}")
            print(f"Status: {response.status_code}")
            prop_data = response.json()
            print(f"Success: {prop_data.get('success')}")
            print(f"Message: {prop_data.get('message')}")

            if 'data' in prop_data:
                props = prop_data['data']
                print(f"\nMaterial Properties:")
                print(f"  Name: {props.get('material_name')}")
                print(f"  Two-sided: {props.get('two_sided')}")
                print(f"  Blend mode: {props.get('blend_mode')}")
                print(f"  Shading model: {props.get('shading_model')}")
                print(f"  Textures: {props.get('num_textures')}")

                textures = props.get('textures', [])
                if textures:
                    print(f"  First texture: {textures[0]}")
        else:
            print("No materials found to test")
    else:
        print("Failed to get material list")


def test_textures_list():
    """Test GET /textures/list"""
    print("\n=== Testing GET /textures/list ===")

    # Test without filter
    response = requests.get(f"{BASE_URL}/textures/list")
    print(f"Status: {response.status_code}")
    data = response.json()
    print(f"Success: {data.get('success')}")
    print(f"Message: {data.get('message')}")

    if 'data' in data:
        count = data['data'].get('count', 0)
        print(f"Textures found: {count}")
        if count > 0:
            first_tex = data['data']['textures'][0]
            print(f"\nFirst texture:")
            print(f"  Name: {first_tex.get('name')}")
            print(f"  Path: {first_tex.get('path')}")
            print(f"  Width: {first_tex.get('width', 'N/A')}")
            print(f"  Height: {first_tex.get('height', 'N/A')}")
            print(f"  Format: {first_tex.get('format', 'N/A')}")

    # Test with filter
    print("\n--- Testing with path filter ---")
    response = requests.post(f"{BASE_URL}/textures/list",
                            json={"path_filter": "/Game/"})
    data = response.json()
    print(f"Success: {data.get('success')}")
    if 'data' in data:
        print(f"Filtered textures found: {data['data'].get('count', 0)}")


def main():
    print("Material and Texture Query Endpoints Test Suite")
    print("=" * 60)
    print(f"Testing against: {BASE_URL}")
    print("\nNOTE: Make sure Unreal Editor is running with AutomationAPIServer active")
    print("=" * 60)

    try:
        # Test connectivity
        response = requests.get(f"{BASE_URL}/status", timeout=2)
        if response.status_code == 200:
            print("Server is UP and responding")
        else:
            print(f"Server responded with status: {response.status_code}")
    except Exception as e:
        print(f"ERROR: Cannot connect to server at {BASE_URL}")
        print(f"Error: {e}")
        print("\nPlease ensure:")
        print("1. Unreal Editor is running")
        print("2. Play In Editor (PIE) is active")
        print("3. AutomationAPIServer is enabled and listening on port 8080")
        return

    # Run tests
    try:
        test_materials_list()
        test_material_properties()
        test_textures_list()

        print("\n" + "=" * 60)
        print("All tests completed!")
        print("=" * 60)

    except Exception as e:
        print(f"\nERROR during testing: {e}")
        import traceback
        traceback.print_exc()


if __name__ == "__main__":
    main()
