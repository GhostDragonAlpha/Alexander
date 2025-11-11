# Autonomous Playtesting System - API Reference

**Version**: 1.0.0  
**Last Updated**: 2025-11-11

## Table of Contents
1. [Overview](#overview)
2. [Authentication](#authentication)
3. [Port 8080: Game HTTP API](#port-8080-game-http-api)
4. [Port 8081: Analysis API Server](#port-8081-analysis-api-server)
5. [Port 8082: Playtesting Dashboard API](#port-8082-playtesting-dashboard-api)
6. [Error Handling](#error-handling)
7. [Rate Limiting](#rate-limiting)
8. [WebSocket Endpoints](#websocket-endpoints)
9. [Code Examples](#code-examples)

## Overview

The Autonomous Playtesting System provides three distinct HTTP APIs for different purposes:

- **Port 8080**: Game HTTP API - Direct game control and state access
- **Port 8081**: Analysis API Server - Screenshot analysis and visual verification
- **Port 8082**: Playtesting Dashboard API - System monitoring and control

All APIs use JSON for request and response bodies unless otherwise specified.

**Base URLs**:
- Game API: `http://localhost:8080`
- Analysis API: `http://localhost:8081`
- Dashboard API: `http://localhost:8082`

## Authentication

Currently, all APIs operate without authentication on localhost only. For production deployment, consider implementing:

- API key authentication
- JWT tokens
- IP whitelisting
- HTTPS encryption

## Port 8080: Game HTTP API

The Game HTTP API provides direct access to the running Unreal Engine game via the AutomationAPIServer component.

### Base URL
```
http://localhost:8080
```

### Endpoints

#### GET /status
Get the current status of the game and HTTP API server.

**Request**:
```http
GET http://localhost:8080/status
```

**Response**:
```json
{
  "success": true,
  "status": "running",
  "timestamp": 1699999999.123,
  "game_state": "in_game",
  "uptime_seconds": 3600,
  "api_version": "1.0.0",
  "map_name": "VRTemplateMap",
  "game_mode": "AutomationGameMode"
}
```

**Response Fields**:
- `success`: Boolean indicating request success
- `status`: Current server status ("running", "initializing", "error")
- `timestamp`: Unix timestamp of response
- `game_state`: Current game state ("in_game", "menu", "loading")
- `uptime_seconds`: Server uptime in seconds
- `api_version`: API version string
- `map_name`: Current map name
- `game_mode`: Current game mode

**Example Usage**:
```python
import requests

response = requests.get("http://localhost:8080/status")
if response.status_code == 200:
    data = response.json()
    print(f"Game is {data['status']} on map {data['map_name']}")
```

---

#### GET /game_state
Get detailed game state information.

**Request**:
```http
GET http://localhost:8080/game_state
```

**Response**:
```json
{
  "success": true,
  "timestamp": 1699999999.123,
  "player": {
    "position": {"x": 100.5, "y": 200.0, "z": 300.0},
    "rotation": {"pitch": 0.0, "yaw": 45.0, "roll": 0.0},
    "velocity": {"x": 10.0, "y": 0.0, "z": 0.0},
    "speed": 10.0,
    "health": 100,
    "shield": 100,
    "ship_id": "Ship_001"
  },
  "world": {
    "time_seconds": 1234.5,
    "day_night_cycle": 0.7,
    "weather": "clear",
    "gravity": 9.8
  },
  "mission": {
    "current_mission": "first_mission",
    "mission_state": "in_progress",
    "objectives_completed": 2,
    "objectives_total": 5,
    "time_elapsed": 300.0
  },
  "inventory": {
    "credits": 1500,
    "items": [
      {"id": "fuel", "quantity": 100},
      {"id": "ammo", "quantity": 50}
    ]
  },
  "nearby_actors": [
    {
      "id": "asteroid_001",
      "type": "asteroid",
      "position": {"x": 150.0, "y": 250.0, "z": 350.0},
      "distance": 86.6
    }
  ]
}
```

**Response Fields**:
- `player`: Player state information
  - `position`: 3D position coordinates
  - `rotation`: Rotation in degrees
  - `velocity`: Velocity vector
  - `speed`: Current speed
  - `health`: Player health (0-100)
  - `shield`: Shield level (0-100)
  - `ship_id`: Unique ship identifier
- `world`: World state information
  - `time_seconds`: In-game time
  - `day_night_cycle`: Day/night cycle value (0-1)
  - `weather`: Current weather
  - `gravity`: Gravity strength
- `mission`: Mission state
  - `current_mission`: Active mission ID
  - `mission_state`: Mission status
  - `objectives_completed`: Completed objectives
  - `objectives_total`: Total objectives
  - `time_elapsed`: Mission time elapsed
- `inventory`: Player inventory
  - `credits`: Current credits
  - `items`: Array of inventory items
- `nearby_actors`: Nearby game objects
  - `id`: Actor identifier
  - `type`: Actor type
  - `position`: Actor position
  - `distance`: Distance from player

**Example Usage**:
```python
response = requests.get("http://localhost:8080/game_state")
data = response.json()

player_pos = data['player']['position']
print(f"Player at: {player_pos['x']}, {player_pos['y']}, {player_pos['z']}")
```

---

#### GET /performance_metrics
Get current performance metrics.

**Request**:
```http
GET http://localhost:8080/performance_metrics
```

**Response**:
```json
{
  "success": true,
  "timestamp": 1699999999.123,
  "fps": 89.5,
  "frame_time_ms": 11.2,
  "memory_mb": 2456.8,
  "memory_peak_mb": 2512.0,
  "cpu_usage_percent": 45.2,
  "gpu_usage_percent": 78.5,
  "draw_calls": 1250,
  "triangles_rendered": 450000,
  "actor_count": 156,
  "physics_time_ms": 2.1,
  "render_time_ms": 8.5
}
```

**Response Fields**:
- `fps`: Current frames per second
- `frame_time_ms`: Time to render one frame (milliseconds)
- `memory_mb`: Current memory usage in MB
- `memory_peak_mb`: Peak memory usage in MB
- `cpu_usage_percent`: CPU utilization percentage
- `gpu_usage_percent`: GPU utilization percentage
- `draw_calls`: Number of draw calls per frame
- `triangles_rendered`: Number of triangles rendered
- `actor_count`: Number of active actors
- `physics_time_ms`: Physics simulation time
- `render_time_ms`: Rendering time

**Example Usage**:
```python
response = requests.get("http://localhost:8080/performance_metrics")
data = response.json()

if data['fps'] < 30:
    print(f"Low FPS detected: {data['fps']}")
if data['memory_mb'] > 4000:
    print(f"High memory usage: {data['memory_mb']} MB")
```

---

#### POST /spawn_ship
Spawn a ship in the game world.

**Request**:
```http
POST http://localhost:8080/spawn_ship
Content-Type: application/json

{
  "ship_class": "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer_C",
  "location": {
    "x": 0,
    "y": 0,
    "z": 500
  },
  "rotation": {
    "pitch": 0,
    "yaw": 0,
    "roll": 0
  },
  "properties": {
    "health": 100,
    "shield": 100
  }
}
```

**Request Parameters**:
- `ship_class`: Blueprint path to ship class (required)
- `location`: Spawn location coordinates (optional, default: 0,0,500)
- `rotation`: Spawn rotation in degrees (optional, default: 0,0,0)
- `properties`: Additional ship properties (optional)

**Response**:
```json
{
  "success": true,
  "message": "Ship spawned successfully",
  "data": {
    "ship_id": "Ship_001",
    "location": {"x": 0, "y": 0, "z": 500},
    "rotation": {"pitch": 0, "yaw": 0, "roll": 0}
  }
}
```

**Response Fields**:
- `ship_id`: Unique identifier for the spawned ship
- `location`: Actual spawn location
- `rotation`: Actual spawn rotation

**Example Usage**:
```python
import requests

payload = {
    "ship_class": "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer_C",
    "location": {"x": 100, "y": 200, "z": 300}
}

response = requests.post("http://localhost:8080/spawn_ship", json=payload)
data = response.json()

if data['success']:
    ship_id = data['data']['ship_id']
    print(f"Ship spawned: {ship_id}")
```

---

#### POST /set_input
Set input controls for a ship.

**Request**:
```http
POST http://localhost:8080/set_input
Content-Type: application/json

{
  "ship_id": "Ship_001",
  "thrust": {
    "x": 1.0,
    "y": 0.0,
    "z": 0.0
  },
  "rotation": {
    "pitch": 0.0,
    "yaw": 0.5,
    "roll": 0.0
  },
  "assist_mode": 1
}
```

**Request Parameters**:
- `ship_id`: Target ship identifier (required)
- `thrust`: Thrust input vector (-1.0 to 1.0 for each axis)
- `rotation`: Rotation input vector (-1.0 to 1.0 for each axis)
- `assist_mode`: Flight assist mode (0-4)

**Assist Modes**:
- `0`: None - No flight assistance
- `1`: Stability - Basic stability control
- `2`: AutoLevel - Automatic level flight
- `3`: Orbital - Orbital mechanics assistance
- `4`: Docking - Precision docking mode

**Response**:
```json
{
  "success": true,
  "message": "Input applied successfully",
  "data": {
    "ship_id": "Ship_001",
    "thrust_applied": {"x": 1.0, "y": 0.0, "z": 0.0},
    "rotation_applied": {"pitch": 0.0, "yaw": 0.5, "roll": 0.0},
    "assist_mode": 1
  }
}
```

**Example Usage**:
```python
# Apply forward thrust
payload = {
    "ship_id": "Ship_001",
    "thrust": {"x": 1.0, "y": 0.0, "z": 0.0}
}

response = requests.post("http://localhost:8080/set_input", json=payload)

# Apply rotation
payload = {
    "ship_id": "Ship_001",
    "rotation": {"pitch": 0.0, "yaw": 0.5, "roll": 0.0}
}

response = requests.post("http://localhost:8080/set_input", json=payload)
```

---

#### GET /get_position/{ship_id}
Get the current position of a ship.

**Request**:
```http
GET http://localhost:8080/get_position/Ship_001
```

**Response**:
```json
{
  "success": true,
  "ship_id": "Ship_001",
  "position": {
    "x": 150.5,
    "y": 200.0,
    "z": 300.0
  }
}
```

**Response Fields**:
- `ship_id`: The ship identifier
- `position`: Current 3D position coordinates

**Example Usage**:
```python
ship_id = "Ship_001"
response = requests.get(f"http://localhost:8080/get_position/{ship_id}")
data = response.json()

position = data['position']
print(f"Ship position: {position['x']}, {position['y']}, {position['z']}")
```

---

#### GET /get_velocity/{ship_id}
Get the current velocity of a ship.

**Request**:
```http
GET http://localhost:8080/get_velocity/Ship_001
```

**Response**:
```json
{
  "success": true,
  "ship_id": "Ship_001",
  "velocity": {
    "x": 10.0,
    "y": 0.0,
    "z": 0.0
  },
  "speed": 10.0
}
```

**Response Fields**:
- `ship_id`: The ship identifier
- `velocity`: Velocity vector
- `speed`: Current speed magnitude

**Example Usage**:
```python
ship_id = "Ship_001"
response = requests.get(f"http://localhost:8080/get_velocity/{ship_id}")
data = response.json()

print(f"Ship speed: {data['speed']} units/sec")
print(f"Velocity: {data['velocity']}")
```

---

#### POST /execute_command
Execute a console command in the game.

**Request**:
```http
POST http://localhost:8080/execute_command
Content-Type: application/json

{
  "command": "stat fps",
  "parameters": []
}
```

**Request Parameters**:
- `command`: Console command to execute (required)
- `parameters`: Array of command parameters (optional)

**Response**:
```json
{
  "success": true,
  "message": "Command executed successfully",
  "output": "FPS: 89.5"
}
```

**Common Commands**:
- `stat fps` - Show FPS counter
- `stat unit` - Show frame time breakdown
- `showdebug` - Toggle debug display
- `quit` - Exit the game

**Example Usage**:
```python
# Show FPS
payload = {"command": "stat fps"}
response = requests.post("http://localhost:8080/execute_command", json=payload)

# Quit game
payload = {"command": "quit"}
response = requests.post("http://localhost:8080/execute_command", json=payload)
```

---

## Port 8081: Analysis API Server

The Analysis API Server provides visual verification and screenshot analysis capabilities.

### Base URL
```
http://localhost:8081
```

### Endpoints

#### POST /analyze_screenshot
Analyze a screenshot and return comprehensive analysis results.

**Request**:
```http
POST http://localhost:8081/analyze_screenshot
Content-Type: application/json

{
  "screenshot_data": "base64_encoded_image_string",
  "metadata": {
    "capture_timestamp": 1699999999.123,
    "game_state": "in_game",
    "player_position": {"x": 100, "y": 200, "z": 300}
  },
  "analysis_options": {
    "visual_analysis": true,
    "text_recognition": true,
    "object_detection": true,
    "state_comparison": false,
    "issue_classification": true
  }
}
```

**Request Parameters**:
- `screenshot_data`: Base64 encoded image string (required)
- `metadata`: Additional context information (optional)
- `analysis_options`: Which analysis modules to enable (optional)

**Analysis Options**:
- `visual_analysis`: Enable UI element and artifact detection
- `text_recognition`: Enable OCR text extraction
- `object_detection`: Enable game object detection
- `state_comparison`: Compare against expected state (requires expected_state)
- `issue_classification`: Enable AI-powered issue classification

**Response**:
```json
{
  "success": true,
  "analysis_id": "analysis_1699999999",
  "timestamp": 1699999999.123,
  "processing_time": 1.234,
  "results": {
    "analysis_type": "comprehensive",
    "overall_processing_time": 1.234,
    "modules": {
      "visual_analysis": {
        "enabled": true,
        "processing_time": 0.456,
        "results": {
          "ui_elements": [
            {
              "type": "button",
              "position": {"x": 100, "y": 200},
              "size": {"width": 150, "height": 50},
              "confidence": 0.95
            }
          ],
          "visual_artifacts": [],
          "screen_resolution": {"width": 1280, "height": 720}
        }
      },
      "text_recognition": {
        "enabled": true,
        "processing_time": 0.345,
        "results": {
          "text_elements_count": 5,
          "average_confidence": 0.87,
          "detected_languages": ["eng"],
          "text_elements": [
            {
              "text": "MISSION COMPLETE",
              "position": {"x": 640, "y": 50},
              "confidence": 0.92
            }
          ]
        }
      },
      "object_detection": {
        "enabled": true,
        "processing_time": 0.278,
        "results": {
          "objects_detected": 3,
          "model_confidence": 0.89,
          "detected_objects": [
            {
              "class_name": "spaceship",
              "confidence": 0.94,
              "bbox": {"x": 400, "y": 300, "width": 200, "height": 150}
            }
          ]
        }
      },
      "issue_classification": {
        "enabled": true,
        "processing_time": 0.155,
        "results": {
          "total_issues": 1,
          "critical_count": 0,
          "high_count": 0,
          "medium_count": 1,
          "low_count": 0,
          "issue_summary": "Minor UI artifact detected"
        }
      }
    },
    "metadata": {
      "capture_timestamp": 1699999999.123,
      "game_state": "in_game"
    }
  }
}
```

**Response Fields**:
- `analysis_id`: Unique identifier for this analysis
- `processing_time`: Total processing time in seconds
- `modules`: Analysis results from each module
- `visual_analysis`: UI element and artifact detection
- `text_recognition`: OCR text extraction results
- `object_detection`: Game object detection
- `issue_classification`: Classified issues and severity

**Example Usage**:
```python
import requests
import base64

# Read and encode screenshot
with open("screenshot.png", "rb") as f:
    image_data = f.read()
    base64_image = base64.b64encode(image_data).decode('utf-8')

# Prepare request
payload = {
    "screenshot_data": base64_image,
    "metadata": {
        "capture_timestamp": time.time(),
        "game_state": "in_game"
    },
    "analysis_options": {
        "visual_analysis": True,
        "text_recognition": True,
        "object_detection": True,
        "issue_classification": True
    }
}

# Send for analysis
response = requests.post("http://localhost:8081/analyze_screenshot", json=payload)
analysis_results = response.json()

print(f"Detected {analysis_results['results']['object_detection']['results']['objects_detected']} objects")
print(f"Found {analysis_results['results']['text_recognition']['results']['text_elements_count']} text elements")
```

---

#### POST /compare_states
Compare two screenshots and identify differences.

**Request**:
```http
POST http://localhost:8081/compare_states
Content-Type: application/json

{
  "screenshot1_data": "base64_encoded_image_string_1",
  "screenshot2_data": "base64_encoded_image_string_2",
  "metadata": {
    "comparison_type": "before_after",
    "time_delta_seconds": 30.0
  }
}
```

**Request Parameters**:
- `screenshot1_data`: First screenshot (base64 encoded)
- `screenshot2_data`: Second screenshot (base64 encoded)
- `metadata`: Comparison context (optional)

**Response**:
```json
{
  "success": true,
  "analysis_id": "comparison_1699999999",
  "timestamp": 1699999999.123,
  "processing_time": 2.456,
  "results": {
    "comparison_id": "comparison_1699999999",
    "difference_score": 125.7,
    "differences": [
      {
        "type": "object_missing",
        "description": "1 spaceship object(s) missing in second screenshot",
        "severity": "high"
      },
      {
        "type": "text_added",
        "description": "New text in second screenshot: ['MISSION COMPLETE']",
        "severity": "medium"
      },
      {
        "type": "ui_element_added",
        "description": "1 button element(s) added in second screenshot",
        "severity": "low"
      }
    ],
    "classified_issues": {
      "total_issues": 3,
      "by_severity": {
        "critical": 0,
        "high": 1,
        "medium": 1,
        "low": 1
      }
    }
  }
}
```

**Response Fields**:
- `difference_score`: Numerical difference score (higher = more different)
- `differences`: Array of detected differences
- `classified_issues`: Issues classified by severity

**Difference Types**:
- `object_missing`: Objects present in first but not second screenshot
- `object_added`: Objects present in second but not first screenshot
- `text_missing`: Text present in first but not second
- `text_added`: Text present in second but not first
- `ui_element_missing`: UI elements missing
- `ui_element_added`: UI elements added
- `visual_difference`: General visual differences

**Example Usage**:
```python
# Capture two screenshots at different times
with open("screenshot_before.png", "rb") as f:
    screenshot1 = base64.b64encode(f.read()).decode('utf-8')

with open("screenshot_after.png", "rb") as f:
    screenshot2 = base64.b64encode(f.read()).decode('utf-8')

# Compare them
payload = {
    "screenshot1_data": screenshot1,
    "screenshot2_data": screenshot2,
    "metadata": {
        "comparison_type": "before_after",
        "time_delta_seconds": 30.0
    }
}

response = requests.post("http://localhost:8081/compare_states", json=payload)
comparison = response.json()

print(f"Difference score: {comparison['results']['difference_score']}")
for diff in comparison['results']['differences']:
    print(f"- {diff['severity']}: {diff['description']}")
```

---

#### GET /analysis_history
Retrieve analysis history with optional filtering.

**Request**:
```http
GET http://localhost:8081/analysis_history?limit=100&offset=0&type=visual_analysis
```

**Query Parameters**:
- `limit`: Maximum number of results (default: 100, max: 1000)
- `offset`: Number of results to skip (default: 0)
- `type`: Filter by analysis type (optional)

**Response**:
```json
{
  "success": true,
  "total_count": 1250,
  "limit": 100,
  "offset": 0,
  "history": [
    {
      "analysis_id": "analysis_1699999900",
      "timestamp": 1699999900.0,
      "processing_time": 1.234,
      "metadata": {
        "capture_timestamp": 1699999899.0,
        "game_state": "in_game"
      },
      "results": {
        "analysis_type": "comprehensive",
        "modules": {
          "visual_analysis": {
            "enabled": true,
            "processing_time": 0.456
          }
        }
      }
    }
  ]
}
```

**Response Fields**:
- `total_count`: Total number of analysis records
- `limit`: Maximum results in this response
- `offset`: Number of skipped results
- `history`: Array of analysis records

**Example Usage**:
```python
# Get recent analysis history
response = requests.get("http://localhost:8081/analysis_history?limit=50")
history = response.json()

print(f"Total analyses: {history['total_count']}")
for analysis in history['history']:
    print(f"Analysis {analysis['analysis_id']} took {analysis['processing_time']}s")
```

---

#### GET /health
Health check endpoint for the analysis server.

**Request**:
```http
GET http://localhost:8081/health
```

**Response**:
```json
{
  "success": true,
  "status": "healthy",
  "timestamp": 1699999999.123,
  "modules": {
    "visual_analyzer": true,
    "text_recognizer": true,
    "object_detector": true,
    "state_comparator": true,
    "issue_classifier": true
  }
}
```

**Response Fields**:
- `status`: Server health status
- `modules`: Availability of analysis modules

**Example Usage**:
```python
# Check if analysis server is healthy
response = requests.get("http://localhost:8081/health")
health = response.json()

if health['status'] == 'healthy':
    print("Analysis server is ready")
    for module, available in health['modules'].items():
        print(f"  {module}: {'✓' if available else '✗'}")
```

---

#### GET /config
Get current analysis server configuration.

**Request**:
```http
GET http://localhost:8081/config
```

**Response**:
```json
{
  "success": true,
  "config": {
    "server": {
      "host": "localhost",
      "port": 8081,
      "max_file_size": 10485760,
      "timeout": 2.0
    },
    "performance": {
      "max_history_size": 1000
    },
    "analysis_options": {
      "visual_analysis": true,
      "text_recognition": true,
      "object_detection": true,
      "state_comparison": true,
      "issue_classification": true
    }
  }
}
```

**Response Fields**:
- `server`: Server configuration
- `performance`: Performance settings
- `analysis_options`: Enabled analysis modules

---

## Port 8082: Playtesting Dashboard API

The Playtesting Dashboard API provides system monitoring and control capabilities.

### Base URL
```
http://localhost:8082
```

### Endpoints

#### GET /
Get the main dashboard HTML page.

**Request**:
```http
GET http://localhost:8082/
```

**Response**: HTML dashboard page

---

#### GET /api/status
Get overall system status.

**Request**:
```http
GET http://localhost:8082/api/status
```

**Response**:
```json
{
  "success": true,
  "system_status": "running",
  "timestamp": 1699999999.123,
  "components": {
    "game_launcher": {
      "status": "running",
      "pid": 12345,
      "uptime_seconds": 3600
    },
    "screenshot_capture": {
      "status": "running",
      "screenshot_count": 1250,
      "capture_interval": 3.0
    },
    "performance_collector": {
      "status": "running",
      "metrics_collected": 720
    },
    "issue_reporter": {
      "status": "running",
      "issues_detected": 15
    },
    "analysis_server": {
      "status": "running",
      "analyses_completed": 1250
    }
  },
  "session_info": {
    "session_id": "session_20231111_120000",
    "start_time": 1699999999.123,
    "duration_minutes": 60.5,
    "current_iteration": 120
  }
}
```

**Response Fields**:
- `system_status`: Overall system status
- `components`: Status of each system component
- `session_info`: Current session information

---

#### GET /api/test_results
Get test results and statistics.

**Request**:
```http
GET http://localhost:8082/api/test_results?limit=100
```

**Query Parameters**:
- `limit`: Maximum number of results (default: 100)

**Response**:
```json
{
  "success": true,
  "summary": {
    "total_tests": 120,
    "passed": 115,
    "failed": 3,
    "error": 2,
    "success_rate": 95.8
  },
  "recent_results": [
    {
      "iteration": 120,
      "scenario": "first_mission",
      "status": "passed",
      "duration": 45.2,
      "timestamp": 1699999999.123
    }
  ]
}
```

---

#### GET /api/performance
Get performance metrics and trends.

**Request**:
```http
GET http://localhost:8082/api/performance?hours=24
```

**Query Parameters**:
- `hours`: Time range in hours (default: 1)

**Response**:
```json
{
  "success": true,
  "current_metrics": {
    "fps": 89.5,
    "frame_time_ms": 11.2,
    "memory_mb": 2456.8,
    "cpu_usage": 45.2,
    "gpu_usage": 78.5
  },
  "trends": {
    "timestamps": [1699999000, 1699999060, 1699999120],
    "fps_values": [90.1, 89.5, 88.9],
    "memory_values": [2400, 2456, 2512]
  },
  "alerts": [
    {
      "timestamp": 1699999900,
      "type": "low_fps",
      "message": "FPS dropped to 25.3",
      "severity": "high"
    }
  ]
}
```

---

#### GET /api/issues
Get detected issues and reports.

**Request**:
```http
GET http://localhost:8082/api/issues?severity=high&limit=50
```

**Query Parameters**:
- `severity`: Filter by severity (optional)
- `limit`: Maximum number of results (default: 100)

**Response**:
```json
{
  "success": true,
  "total_issues": 15,
  "issues": [
    {
      "issue_id": "low_fps_abc123",
      "timestamp": 1699999900.0,
      "issue_type": "performance",
      "severity": "high",
      "description": "Low FPS detected: 25.3",
      "frequency": 3,
      "first_seen": 1699999600.0,
      "last_seen": 1699999900.0,
      "status": "open",
      "recommended_fix": "Reduce draw calls and optimize shaders"
    }
  ]
}
```

---

#### GET /api/screenshot
Get the latest captured screenshot.

**Request**:
```http
GET http://localhost:8082/api/screenshot
```

**Response**:
```json
{
  "success": true,
  "screenshot": {
    "timestamp": 1699999999.123,
    "filename": "screenshot_20231111_120000_123456.jpg",
    "image_base64": "base64_encoded_image_data",
    "metadata": {
      "capture_number": 1250,
      "game_state": "in_game",
      "dimensions": {"width": 1280, "height": 720}
    }
  }
}
```

---

#### POST /api/control
Control the playtesting system.

**Request**:
```http
POST http://localhost:8082/api/control
Content-Type: application/json

{
  "action": "pause",
  "parameters": {
    "duration_seconds": 300
  }
}
```

**Supported Actions**:
- `pause`: Pause testing
- `resume`: Resume testing
- `stop`: Stop testing and shutdown
- `restart`: Restart all components
- `capture_screenshot`: Trigger immediate screenshot
- `run_test`: Run specific test scenario

**Response**:
```json
{
  "success": true,
  "message": "System paused for 300 seconds",
  "action": "pause",
  "system_status": "paused"
}
```

---

## Error Handling

### Error Response Format

All APIs return errors in a consistent format:

```json
{
  "success": false,
  "error": "Error message describing what went wrong",
  "timestamp": 1699999999.123,
  "error_code": "ERROR_CODE"
}
```

### Common Error Codes

| Error Code | Description | HTTP Status |
|------------|-------------|-------------|
| `INVALID_REQUEST` | Malformed request | 400 |
| `UNAUTHORIZED` | Authentication required | 401 |
| `FORBIDDEN` | Access denied | 403 |
| `NOT_FOUND` | Resource not found | 404 |
| `RATE_LIMITED` | Too many requests | 429 |
| `SERVER_ERROR` | Internal server error | 500 |
| `SERVICE_UNAVAILABLE` | Service temporarily unavailable | 503 |

### Error Handling Example

```python
import requests

try:
    response = requests.get("http://localhost:8080/status")
    response.raise_for_status()  # Raise exception for bad status codes
    
    data = response.json()
    if not data.get('success'):
        print(f"API Error: {data.get('error')}")
        return None
        
    return data
    
except requests.exceptions.RequestException as e:
    print(f"Request failed: {e}")
    return None
```

---

## Rate Limiting

### Current Limits

- **Game API (Port 8080)**: 60 requests per second
- **Analysis API (Port 8081)**: 30 requests per second
- **Dashboard API (Port 8082)**: 100 requests per second

### Rate Limit Headers

Responses include rate limit information:

```http
X-RateLimit-Limit: 60
X-RateLimit-Remaining: 59
X-RateLimit-Reset: 1699999999
```

### Handling Rate Limits

```python
import time
import requests

def make_api_call_with_retry(url, max_retries=3):
    for attempt in range(max_retries):
        response = requests.get(url)
        
        if response.status_code == 429:  # Rate limited
            retry_after = int(response.headers.get('Retry-After', 1))
            print(f"Rate limited. Waiting {retry_after} seconds...")
            time.sleep(retry_after)
            continue
            
        return response
        
    return None
```

---

## WebSocket Endpoints

### Real-time Updates

The system supports WebSocket connections for real-time updates:

**Dashboard WebSocket**:
```
ws://localhost:8082/ws/live
```

**Message Format**:
```json
{
  "type": "screenshot_captured",
  "timestamp": 1699999999.123,
  "data": {
    "screenshot_id": "screenshot_001",
    "analysis_results": {}
  }
}
```

**Message Types**:
- `screenshot_captured`: New screenshot available
- `test_completed`: Test finished
- `issue_detected`: New issue found
- `performance_alert`: Performance threshold exceeded
- `system_status`: System status change

---

## Code Examples

### Complete Workflow Example

```python
import requests
import time
import base64

class PlaytestingClient:
    def __init__(self):
        self.game_api = "http://localhost:8080"
        self.analysis_api = "http://localhost:8081"
        self.dashboard_api = "http://localhost:8082"
    
    def wait_for_game_ready(self, timeout=180):
        """Wait for game to be ready"""
        start_time = time.time()
        while time.time() - start_time < timeout:
            try:
                response = requests.get(f"{self.game_api}/status", timeout=2)
                if response.status_code == 200:
                    return True
            except:
                pass
            time.sleep(2)
        return False
    
    def run_test_scenario(self, scenario_name="first_mission"):
        """Run a complete test scenario"""
        print(f"Starting test scenario: {scenario_name}")
        
        # Wait for game to be ready
        if not self.wait_for_game_ready():
            print("Game failed to start")
            return False
        
        print("Game is ready")
        
        # Get initial game state
        game_state = requests.get(f"{self.game_api}/game_state").json()
        print(f"Player position: {game_state['player']['position']}")
        
        # Spawn ship if needed
        if not game_state['player'].get('ship_id'):
            spawn_response = requests.post(
                f"{self.game_api}/spawn_ship",
                json={
                    "ship_class": "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer_C",
                    "location": {"x": 0, "y": 0, "z": 500}
                }
            ).json()
            
            if spawn_response['success']:
                ship_id = spawn_response['data']['ship_id']
                print(f"Ship spawned: {ship_id}")
            else:
                print("Failed to spawn ship")
                return False
        
        # Run test for specified duration
        test_duration = 60  # seconds
        start_time = time.time()
        
        while time.time() - start_time < test_duration:
            # Get current state
            state = requests.get(f"{self.game_api}/game_state").json()
            perf = requests.get(f"{self.game_api}/performance_metrics").json()
            
            print(f"Time: {time.time() - start_time:.1f}s, FPS: {perf['fps']:.1f}")
            
            # Apply some input
            thrust_x = 0.5 if (time.time() - start_time) < test_duration / 2 else 0
            requests.post(
                f"{self.game_api}/set_input",
                json={
                    "ship_id": ship_id,
                    "thrust": {"x": thrust_x, "y": 0, "z": 0}
                }
            )
            
            time.sleep(1)
        
        # Get final results
        final_state = requests.get(f"{self.game_api}/game_state").json()
        final_perf = requests.get(f"{self.game_api}/performance_metrics").json()
        
        print(f"Test completed!")
        print(f"Final position: {final_state['player']['position']}")
        print(f"Average FPS: {final_perf['fps']:.1f}")
        
        return True

# Usage
client = PlaytestingClient()
client.run_test_scenario("first_mission")
```

### Performance Monitoring Example

```python
import requests
import time
import matplotlib.pyplot as plt

def monitor_performance(duration_minutes=5):
    """Monitor and plot performance metrics"""
    fps_data = []
    memory_data = []
    timestamps = []
    
    start_time = time.time()
    end_time = start_time + (duration_minutes * 60)
    
    print(f"Monitoring performance for {duration_minutes} minutes...")
    
    while time.time() < end_time:
        try:
            response = requests.get("http://localhost:8080/performance_metrics", timeout=2)
            if response.status_code == 200:
                data = response.json()
                
                fps_data.append(data['fps'])
                memory_data.append(data['memory_mb'])
                timestamps.append(time.time() - start_time)
                
                print(f"FPS: {data['fps']:.1f}, Memory: {data['memory_mb']:.0f} MB")
            
            time.sleep(5)  # Collect every 5 seconds
            
        except Exception as e:
            print(f"Error collecting metrics: {e}")
            time.sleep(5)
    
    # Create plots
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))
    
    ax1.plot(timestamps, fps_data, 'b-')
    ax1.set_ylabel('FPS')
    ax1.set_title('FPS Over Time')
    ax1.grid(True)
    
    ax2.plot(timestamps, memory_data, 'r-')
    ax2.set_ylabel('Memory (MB)')
    ax2.set_xlabel('Time (seconds)')
    ax2.set_title('Memory Usage Over Time')
    ax2.grid(True)
    
    plt.tight_layout()
    plt.savefig('performance_report.png')
    print("Performance report saved as performance_report.png")

# Usage
monitor_performance(10)  # Monitor for 10 minutes
```

### Screenshot Analysis Example

```python
import requests
import base64
import time

class ScreenshotAnalyzer:
    def __init__(self):
        self.analysis_api = "http://localhost:8081"
    
    def capture_and_analyze(self, image_path):
        """Capture screenshot and analyze it"""
        # Read and encode image
        with open(image_path, "rb") as f:
            image_data = f.read()
            base64_image = base64.b64encode(image_data).decode('utf-8')
        
        # Prepare analysis request
        payload = {
            "screenshot_data": base64_image,
            "metadata": {
                "capture_timestamp": time.time(),
                "game_state": "in_game"
            },
            "analysis_options": {
                "visual_analysis": True,
                "text_recognition": True,
                "object_detection": True,
                "issue_classification": True
            }
        }
        
        # Send for analysis
        print("Sending screenshot for analysis...")
        response = requests.post(
            f"{self.analysis_api}/analyze_screenshot",
            json=payload,
            timeout=30
        )
        
        if response.status_code == 200:
            results = response.json()
            self.print_analysis_results(results)
            return results
        else:
            print(f"Analysis failed: {response.status_code}")
            return None
    
    def print_analysis_results(self, results):
        """Print analysis results in readable format"""
        if not results.get('success'):
            print(f"Analysis failed: {results.get('error')}")
            return
        
        analysis_results = results['results']
        print(f"\n=== Analysis Results ===")
        print(f"Analysis ID: {results['analysis_id']}")
        print(f"Processing time: {results['processing_time']:.2f}s")
        
        # Visual analysis
        if 'visual_analysis' in analysis_results['modules']:
            va = analysis_results['modules']['visual_analysis']['results']
            print(f"\nVisual Analysis:")
            print(f"  UI elements detected: {len(va.get('ui_elements', []))}")
            print(f"  Visual artifacts: {len(va.get('visual_artifacts', []))}")
        
        # Text recognition
        if 'text_recognition' in analysis_results['modules']:
            tr = analysis_results['modules']['text_recognition']['results']
            print(f"\nText Recognition:")
            print(f"  Text elements: {tr['text_elements_count']}")
            print(f"  Average confidence: {tr['average_confidence']:.2f}")
            
            if tr['text_elements']:
                print(f"  Sample text: {tr['text_elements'][0]['text']}")
        
        # Object detection
        if 'object_detection' in analysis_results['modules']:
            od = analysis_results['modules']['object_detection']['results']
            print(f"\nObject Detection:")
            print(f"  Objects detected: {od['objects_detected']}")
            print(f"  Model confidence: {od['model_confidence']:.2f}")
        
        # Issue classification
        if 'issue_classification' in analysis_results['modules']:
            ic = analysis_results['modules']['issue_classification']['results']
            print(f"\nIssue Classification:")
            print(f"  Total issues: {ic['total_issues']}")
            print(f"  Critical: {ic['critical_count']}")
            print(f"  High: {ic['high_count']}")
            print(f"  Medium: {ic['medium_count']}")
            print(f"  Low: {ic['low_count']}")
            print(f"  Summary: {ic['issue_summary']}")

# Usage
analyzer = ScreenshotAnalyzer()
results = analyzer.capture_and_analyze("screenshot.png")
```

---

## API Version History

### Version 1.0.0 (Current)
- Initial API release
- Game control endpoints
- Screenshot analysis
- Performance monitoring
- Issue detection and reporting

### Planned Features
- WebSocket support for real-time updates
- Batch analysis endpoints
- Custom analysis module support
- Advanced filtering and search
- Export functionality

---

**Next Steps**: See [PLAYTESTING_CONFIGURATION.md](PLAYTESTING_CONFIGURATION.md) for detailed configuration options.