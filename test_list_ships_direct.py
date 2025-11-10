#!/usr/bin/env python3
"""
Direct test of list_ships endpoint to diagnose connection issues
"""

import requests
import time

API_BASE = "http://localhost:8080"

print("Testing list_ships endpoint directly...")
print("="*70)

# Wait a moment for server
time.sleep(2)

# Try list_ships multiple times
for i in range(3):
    print(f"\nAttempt {i+1}:")
    try:
        response = requests.get(f"{API_BASE}/list_ships", timeout=5)
        print(f"  Status: {response.status_code}")
        print(f"  Response: {response.text[:200]}")
    except Exception as e:
        print(f"  ERROR: {e}")
    time.sleep(1)

print("\n" + "="*70)
print("Testing /status endpoint for comparison...")
try:
    response = requests.get(f"{API_BASE}/status", timeout=5)
    print(f"  Status: {response.status_code}")
    print(f"  Response: {response.text[:200]}")
except Exception as e:
    print(f"  ERROR: {e}")
