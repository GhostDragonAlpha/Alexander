# Analytics Module - Quick API Reference

## Endpoints

### 1. Start Analytics Session
```http
POST /analytics/start_session
Content-Type: application/json

{
  "build_version": "1.0.0",
  "test_name": "My_Test",
  "platform": "Win64"
}
```

**Response:**
```json
{
  "success": true,
  "message": "Analytics session started",
  "data": {
    "session_id": "abc-123-def-456",
    "start_time": 1234567890.0
  }
}
```

---

### 2. Record Custom Event
```http
POST /analytics/record_event
Content-Type: application/json

{
  "event_name": "Test_Completed",
  "attributes": {
    "status": "passed",
    "duration": "120.5",
    "test_count": "42"
  }
}
```

**Response:**
```json
{
  "success": true,
  "message": "Analytics event recorded",
  "data": {
    "event_name": "Test_Completed",
    "attributes_count": 8,
    "total_events_recorded": 15
  }
}
```

---

### 3. Flush Events
```http
POST /analytics/flush_events
Content-Type: application/json

{}
```

**Response:**
```json
{
  "success": true,
  "message": "Analytics events flushed",
  "data": {
    "events_flushed_count": 42,
    "session_id": "abc-123-def-456"
  }
}
```

---

### 4. Get Session Info
```http
GET /analytics/session_info
```

**Response:**
```json
{
  "success": true,
  "message": "Analytics session info retrieved",
  "data": {
    "session_id": "abc-123-def-456",
    "start_time": 1234567890.0,
    "events_recorded": 25,
    "session_duration_seconds": 180.5,
    "attributes": {
      "build_version": "1.0.0",
      "test_name": "My_Test",
      "platform": "Win64"
    }
  }
}
```

---

### 5. Set Default Attributes
```http
POST /analytics/set_default_attributes
Content-Type: application/json

{
  "attributes": {
    "platform": "Win64",
    "build_config": "Development",
    "test_suite": "Integration_Tests"
  }
}
```

**Response:**
```json
{
  "success": true,
  "message": "Analytics default attributes set",
  "data": {
    "default_attributes_count": 3,
    "current_default_attributes": {
      "platform": "Win64",
      "build_config": "Development",
      "test_suite": "Integration_Tests"
    }
  }
}
```

---

## Auto-Recording Feature

**ALL** API calls (except `/analytics/*` endpoints) are automatically recorded with the following attributes:

- `endpoint_name` - The API endpoint called
- `method` - HTTP method (GET, POST, DELETE)
- `duration_ms` - Request processing time
- `success` - Boolean (true/false)
- `timestamp` - UTC timestamp
- `session_id` - Current session ID
- `error_message` - Error details (if failed)
- Plus all default attributes

**Example Auto-Recorded Event:**
```json
{
  "event_name": "API_Call",
  "endpoint_name": "/spawn_ship",
  "method": "POST",
  "duration_ms": 45.2,
  "success": true,
  "timestamp": "2025-11-09T06:50:15Z",
  "session_id": "abc-123-def-456"
}
```

---

## Python Example

```python
import requests

BASE_URL = "http://localhost:8080"

# 1. Set default attributes (once)
requests.post(f"{BASE_URL}/analytics/set_default_attributes", json={
    "attributes": {
        "platform": "Win64",
        "build_config": "Development"
    }
})

# 2. Start session
response = requests.post(f"{BASE_URL}/analytics/start_session", json={
    "build_version": "1.0.0",
    "test_name": "My_Test",
    "platform": "Windows"
})
session_id = response.json()["data"]["session_id"]

# 3. Record events
requests.post(f"{BASE_URL}/analytics/record_event", json={
    "event_name": "Test_Started",
    "attributes": {
        "test_type": "unit_test"
    }
})

# 4. Make API calls (auto-recorded)
requests.post(f"{BASE_URL}/spawn_ship", json={...})
requests.get(f"{BASE_URL}/status")

# 5. Check session
response = requests.get(f"{BASE_URL}/analytics/session_info")
print(f"Events recorded: {response.json()['data']['events_recorded']}")

# 6. Flush before exit
requests.post(f"{BASE_URL}/analytics/flush_events", json={})
```

---

## Common Event Names

### Test Events
- `Test_Started`
- `Test_Completed`
- `Test_Failed`
- `Test_Suite_Started`
- `Test_Suite_Completed`

### Performance Events
- `Performance_Metric`
- `Frame_Time_Sample`
- `Memory_Usage`
- `Asset_Load_Time`

### API Events
- `API_Call` (auto-recorded)
- `API_Error`
- `API_Timeout`

### Build Events
- `Build_Started`
- `Build_Completed`
- `Build_Failed`
- `Compilation_Error`

---

## Common Attributes

### Standard Attributes
- `platform` - Win64, Linux, Mac
- `build_config` - Development, Shipping, Debug
- `test_suite` - Unit_Tests, Integration_Tests, etc.
- `environment` - CI, Local, Production
- `build_version` - 1.0.0, 2.1.3, etc.

### Test Attributes
- `test_name` - Name of the test
- `test_type` - unit_test, integration_test, e2e_test
- `status` - passed, failed, skipped
- `duration` - Test duration in seconds
- `error_message` - Error details

### Performance Attributes
- `metric_name` - frame_time, memory_usage, etc.
- `value` - Numeric value
- `unit` - ms, MB, fps, etc.
- `threshold` - Expected/max value

---

## Best Practices

1. **Start Session Early**
   - Call `/analytics/start_session` at the beginning of your test
   - Use consistent session attributes

2. **Set Default Attributes**
   - Set once at test suite start
   - Reduces repetition in event recording

3. **Use Descriptive Event Names**
   - Use PascalCase: `Test_Started`, `Build_Completed`
   - Be specific: `Unit_Test_Started` vs just `Test_Started`

4. **Include Context in Attributes**
   - Add relevant metadata
   - Use consistent attribute names across events

5. **Flush Before Exit**
   - Call `/analytics/flush_events` before shutting down
   - Ensures all events are transmitted

6. **Check Session Info**
   - Use `/analytics/session_info` to monitor progress
   - Verify events are being recorded

---

## Troubleshooting

### Analytics Provider Not Configured
```json
{
  "success": false,
  "message": "Analytics provider not configured"
}
```

**Solution:** Ensure Analytics and AnalyticsET modules are enabled in the project.

### No Session ID
Auto-recording only works when a session is started. Call `/analytics/start_session` first.

### Events Not Transmitting
Call `/analytics/flush_events` to force immediate transmission.

---

## File Locations

- **Test Script:** `test_analytics_module.py`
- **Documentation:** `ANALYTICS_MODULE_IMPLEMENTATION.md`
- **Source Code:** `Source/Alexander/Private/AutomationAPIServer.cpp`
- **Header:** `Source/Alexander/Public/AutomationAPIServer.h`
