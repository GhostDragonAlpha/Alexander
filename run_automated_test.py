import requests
import time
import json

API = "http://localhost:8080"

print("=== RUNNING ACTUAL AUTOMATED TEST ===\n")

# Test 1: Start PIE
print("1. Starting PIE...")
r = requests.post(f"{API}/pie/start", timeout=15)
print(f"   Result: {r.status_code} - {r.json()}")
time.sleep(3)

# Test 2: Get world state
print("\n2. Getting world state...")
r = requests.get(f"{API}/world/state", timeout=5)
data = r.json()
print(f"   World loaded: {data.get('success', False)}")
if data.get('world_name'):
    print(f"   World name: {data['world_name']}")

# Test 3: Spawn a ship
print("\n3. Spawning test ship...")
payload = {
    "location": {"x": 0, "y": 0, "z": 1000},
    "rotation": {"pitch": 0, "yaw": 0, "roll": 0}
}
r = requests.post(f"{API}/spawn_ship", json=payload, timeout=10)
result = r.json()
print(f"   Result: {result.get('success', False)}")
if result.get('ship_id'):
    ship_id = result['ship_id']
    print(f"   Ship ID: {ship_id}")
    
    # Test 4: Get ship state
    print("\n4. Getting ship state...")
    r = requests.get(f"{API}/ship_state?ship_id={ship_id}", timeout=5)
    state = r.json()
    if state.get('success'):
        print(f"   Location: {state.get('location')}")
        print(f"   Velocity: {state.get('velocity')}")

# Test 5: Stop PIE
print("\n5. Stopping PIE...")
r = requests.post(f"{API}/pie/stop", timeout=10)
print(f"   Result: {r.status_code} - {r.json()}")

print("\n=== TEST COMPLETE ===")
