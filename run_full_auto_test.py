"""
Complete AI Vision System Integration Test + Auto-Setup
This script loads everything needed and runs full integration test
"""

import sys
import os

print("=" * 60)
print("LOADING AI VISION SYSTEM...")
print("=" * 60)

# Step 1: Load AI Executor
print("\n[1/3] Loading AI Executor...")
try:
    exec(open(r'Content/Python/ai_executor.py').read())
    print("✓ AI Executor loaded successfully")
except Exception as e:
    print(f"✗ Failed to load AI Executor: {e}")

# Step 2: Run Integration Test
print("\n[2/3] Running Integration Tests...")
try:
    exec(open(r'test_ai_vision_integration.py').read())
except Exception as e:
    print(f"✗ Integration test error: {e}")
    import traceback
    traceback.print_exc()

# Step 3: Ready for development
print("\n[3/3] System Status Check...")
print("=" * 60)

# Check if we have everything we need
checks_passed = 0
checks_total = 3

# Check 1: unreal module
try:
    import unreal
    print("✓ Unreal Python API: Available")
    checks_passed += 1
except:
    print("✗ Unreal Python API: Not available")

# Check 2: AI backend
try:
    import http.client
    conn = http.client.HTTPConnection("localhost", 8000, timeout=3)
    conn.request("GET", "/api/health")
    response = conn.getresponse()
    if response.status == 200:
        print("✓ AI Backend: Connected")
        checks_passed += 1
    else:
        print("✗ AI Backend: Not responding")
    conn.close()
except Exception as e:
    print(f"✗ AI Backend: Connection failed - {e}")

# Check 3: AI executor functions
if 'start_ai_dev' in dir():
    print("✓ AI Executor: Loaded")
    checks_passed += 1
else:
    print("✗ AI Executor: Not loaded")

print("=" * 60)
print(f"System Ready: {checks_passed}/{checks_total} components operational")
print("=" * 60)

if checks_passed == checks_total:
    print("\n🎉 AI VISION SYSTEM FULLY OPERATIONAL!")
    print("\nQuick Start Commands:")
    print("  start_ai_dev('Build VR spaceship cockpit')")
    print("  ai_status()")
    print("  stop_ai_dev()")
else:
    print("\n⚠ Some components need attention. Check messages above.")
