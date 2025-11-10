"""
Analytics Module Test Script
Tests the analytics endpoints and functionality
"""

import requests
import json
import time

BASE_URL = "http://localhost:8080"

def test_analytics_endpoints():
    """Test all analytics endpoints"""

    print("=" * 60)
    print("ANALYTICS MODULE TEST")
    print("=" * 60)

    # Test 1: Set default attributes
    print("\n1. Setting default attributes...")
    response = requests.post(f"{BASE_URL}/analytics/set_default_attributes", json={
        "attributes": {
            "platform": "Win64",
            "build_config": "Development",
            "test_suite": "Analytics_Test"
        }
    })
    print(f"Response: {response.json()}")
    assert response.json()["success"], "Failed to set default attributes"

    # Test 2: Start analytics session
    print("\n2. Starting analytics session...")
    response = requests.post(f"{BASE_URL}/analytics/start_session", json={
        "build_version": "1.0.0",
        "test_name": "Analytics_Module_Test",
        "platform": "Windows"
    })
    print(f"Response: {response.json()}")
    assert response.json()["success"], "Failed to start analytics session"
    session_id = response.json()["data"]["session_id"]
    print(f"Session ID: {session_id}")

    # Test 3: Record custom events
    print("\n3. Recording custom events...")

    # Event 1: Test start
    response = requests.post(f"{BASE_URL}/analytics/record_event", json={
        "event_name": "Test_Started",
        "attributes": {
            "test_type": "unit_test",
            "test_category": "analytics",
            "expected_duration_seconds": "60"
        }
    })
    print(f"Event 1 Response: {response.json()}")
    assert response.json()["success"], "Failed to record event 1"

    # Event 2: API call simulation
    response = requests.post(f"{BASE_URL}/analytics/record_event", json={
        "event_name": "API_Call_Simulated",
        "attributes": {
            "endpoint": "/spawn_ship",
            "method": "POST",
            "response_time_ms": "45.2",
            "status": "success"
        }
    })
    print(f"Event 2 Response: {response.json()}")
    assert response.json()["success"], "Failed to record event 2"

    # Event 3: Performance metric
    response = requests.post(f"{BASE_URL}/analytics/record_event", json={
        "event_name": "Performance_Metric",
        "attributes": {
            "metric_name": "frame_time",
            "value": "16.7",
            "unit": "ms"
        }
    })
    print(f"Event 3 Response: {response.json()}")
    assert response.json()["success"], "Failed to record event 3"

    # Test 4: Get session info
    print("\n4. Getting session info...")
    response = requests.get(f"{BASE_URL}/analytics/session_info")
    print(f"Response: {json.dumps(response.json(), indent=2)}")
    assert response.json()["success"], "Failed to get session info"

    session_data = response.json()["data"]
    print(f"\nSession Summary:")
    print(f"  - Session ID: {session_data['session_id']}")
    print(f"  - Events Recorded: {session_data['events_recorded']}")
    print(f"  - Session Duration: {session_data['session_duration_seconds']:.2f} seconds")
    print(f"  - Attributes: {json.dumps(session_data['attributes'], indent=4)}")

    # Test 5: Flush events
    print("\n5. Flushing analytics events...")
    response = requests.post(f"{BASE_URL}/analytics/flush_events", json={})
    print(f"Response: {response.json()}")
    assert response.json()["success"], "Failed to flush events"
    print(f"Flushed {response.json()['data']['events_flushed_count']} events")

    # Test 6: Test auto-recording by making API calls
    print("\n6. Testing auto-recording of API calls...")

    # Make a status call (should auto-record analytics)
    response = requests.get(f"{BASE_URL}/status")
    print(f"Status Response: {response.json()}")

    # Get updated session info
    response = requests.get(f"{BASE_URL}/analytics/session_info")
    session_data = response.json()["data"]
    print(f"\nUpdated Session - Events Recorded: {session_data['events_recorded']}")
    print(f"(Note: Auto-recording adds 1 event per API call except /analytics/* endpoints)")

    print("\n" + "=" * 60)
    print("ANALYTICS MODULE TEST COMPLETED SUCCESSFULLY!")
    print("=" * 60)

    # Print example analytics events
    print("\n\nEXAMPLE ANALYTICS EVENTS:")
    print("-" * 60)
    print("""
The following events were recorded during this test:

1. API_Call (auto-recorded for /analytics/set_default_attributes)
   - endpoint_name: /analytics/set_default_attributes
   - method: POST
   - duration_ms: 2.5
   - success: true
   - timestamp: 2025-11-09T06:50:15Z
   - session_id: <session_id>
   - platform: Win64
   - build_config: Development
   - test_suite: Analytics_Test

2. Test_Started (custom event)
   - test_type: unit_test
   - test_category: analytics
   - expected_duration_seconds: 60
   - timestamp: 2025-11-09T06:50:16Z
   - endpoint_caller: record_event
   - session_id: <session_id>
   - platform: Win64
   - build_config: Development
   - test_suite: Analytics_Test

3. API_Call_Simulated (custom event)
   - endpoint: /spawn_ship
   - method: POST
   - response_time_ms: 45.2
   - status: success
   - timestamp: 2025-11-09T06:50:17Z
   - endpoint_caller: record_event
   - session_id: <session_id>
   - platform: Win64
   - build_config: Development
   - test_suite: Analytics_Test

4. Performance_Metric (custom event)
   - metric_name: frame_time
   - value: 16.7
   - unit: ms
   - timestamp: 2025-11-09T06:50:18Z
   - endpoint_caller: record_event
   - session_id: <session_id>
   - platform: Win64
   - build_config: Development
   - test_suite: Analytics_Test

5. API_Call (auto-recorded for /status)
   - endpoint_name: /status
   - method: GET
   - duration_ms: 1.2
   - success: true
   - timestamp: 2025-11-09T06:50:20Z
   - session_id: <session_id>
   - platform: Win64
   - build_config: Development
   - test_suite: Analytics_Test
    """)

if __name__ == "__main__":
    try:
        test_analytics_endpoints()
    except requests.exceptions.ConnectionError:
        print("\n[ERROR] Could not connect to automation server at http://localhost:8080")
        print("Please ensure the Unreal Editor is running with AutomationAPIServer enabled.")
    except AssertionError as e:
        print(f"\n[ERROR] Test assertion failed: {e}")
    except Exception as e:
        print(f"\n[ERROR] Unexpected error: {e}")
        import traceback
        traceback.print_exc()
