"""
Quick diagnostic test for /validate_physics endpoint
Checks if the diagnostic logging appears in the server
"""

import time
import requests

SERVER_URL = "http://localhost:8080"

# Wait for server to be ready
print("Waiting for server...")
for i in range(30):
    try:
        resp = requests.get(f"{SERVER_URL}/status", timeout=2)
        if resp.status_code == 200:
            print(f"[OK] Server is ready after {i+1} seconds")
            break
    except:
        pass
    time.sleep(1)
else:
    print("[FAIL] Server not responding after 30 seconds")
    exit(1)

# Test /validate_physics endpoint
print("\nTesting /validate_physics endpoint...")
try:
    resp = requests.get(f"{SERVER_URL}/validate_physics", timeout=10)
    print(f"Status Code: {resp.status_code}")
    print(f"Response Length: {len(resp.text)} bytes")
    print(f"Response: {resp.text[:500]}")  # First 500 chars
except Exception as e:
    print(f"Error: {e}")

print("\nCheck Saved/Logs/Alexander.log for diagnostic messages:")
print("- >>> ROUTE: About to call HandleValidatePhysics()")
print("- AutomationAPI: HandleValidatePhysics - Finding UnifiedGravitySystem")
print("- >>> ROUTE: HandleValidatePhysics() returned")
