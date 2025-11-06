# Autonomous Testing System

## Overview

Complete autonomous testing system for VR spaceship flight game. Combines dedicated server, multiplayer support, and AI-driven testing to enable continuous development without manual intervention.

## Architecture

### C++ Components

#### 1. **AutomationAPIServer** ([AutomationAPIServer.h](Source/Alexander/Public/AutomationAPIServer.h))
- HTTP-style JSON API for remote control
- Ship spawning, input control, position/velocity queries
- Screenshot capture
- Performance tracking
- Rate limiting (60 req/s)

**Key Methods:**
- `HandleSpawnShip()` - Spawn ship at location
- `HandleSetInput()` - Apply thrust/rotation input
- `HandleGetPosition()` - Query ship position
- `HandleGetVelocity()` - Query ship velocity
- `HandleScreenshot()` - Capture screenshot

#### 2. **VRHandSimulator** ([VRHandSimulator.h](Source/Alexander/Public/VRHandSimulator.h))
- Calculus-based motion arc simulation
- Simulates realistic VR hand movements
- 5 motion arc types: Linear, Bezier, Parabolic, Circular, Natural
- Velocity and acceleration calculations (derivatives)
- Debug visualization

**Motion Arc Types:**
- **Linear**: Straight line interpolation
- **Bezier**: Cubic Bezier curve with automatic control points
- **Parabolic**: Ballistic trajectory with gravity-like arc
- **Circular**: Arc along circle segment
- **Natural**: Human-like motion (Bezier + lateral S-curve + ease)

**Calculus Implementation:**
```cpp
// Position: P(t) - parametric equation
FVector CalculateArcPosition(float T, const FMotionProfile& Profile);

// Velocity: dP/dt - first derivative
FVector CalculateVelocity(float T, const FMotionProfile& Profile);

// Acceleration: d²P/dt² - second derivative
FVector CalculateAcceleration(float T, const FMotionProfile& Profile);
```

**Easing Functions:**
- `EaseInOutCubic(t)` - Smooth acceleration/deceleration
- `EaseInOutQuintic(t)` - Ultra-smooth motion (5th degree polynomial)

#### 3. **FlightController** (Updated for Multiplayer)
- Added network replication for multiplayer
- Replicated properties:
  - `RawThrustInput` (X/Y/Z)
  - `RawRotationInput` (Pitch/Yaw/Roll)
  - `SmoothedThrustInput`
  - `SmoothedRotationInput`
  - `AssistMode`

#### 4. **AutomationGameMode** ([AutomationGameMode.h](Source/Alexander/Public/AutomationGameMode.h))
- Dedicated server game mode
- Spawns AutomationAPIServer automatically
- Multiplayer support (up to 8 players)
- Performance metrics tracking
- Player spawn management

### Python Components

#### 1. **automation_client.py**
Main automation client for controlling Unreal server.

**Key Features:**
- Server management (launch/stop dedicated server)
- Ship spawning and tracking
- Input control (thrust, rotation, assist mode)
- Position/velocity queries
- Automated test execution

**Example Usage:**
```python
from automation_client import UnrealAutomationClient

client = UnrealAutomationClient(r"C:\Users\allen\Desktop\Alexander\Alexander")

# Spawn ship
ship_id = client.spawn_ship(position=(0, 0, 300))

# Apply thrust
client.set_ship_input(ship_id, thrust=(1.0, 0.0, 0.0))

# Get position after 3 seconds
time.sleep(3)
position = client.get_ship_position(ship_id)
```

#### 2. **test_flight_multiplayer.py**
Comprehensive test suite for multiplayer flight testing.

**Tests Included:**
1. **Single Ship Movement** - Verify ship spawns and moves
2. **Multiple Ships** - Test 3 ships flying simultaneously
3. **Flight Assist Modes** - Test all 5 assist modes
4. **VR Hand Simulation** - Test calculus-based motion arcs
5. **6DOF Movement** - Test all 6 axes of freedom

**Output:**
- `test_report.json` - Detailed test results
- `automation_log.txt` - Execution logs
- Updates `execution_queue.json` with pass/fail

## Usage

### Quick Start

**Option 1: Automated (Recommended)**
```bash
launch_autonomous_testing.bat
```

**Option 2: Manual Steps**
```bash
# 1. Compile C++ code
launch_autonomous_testing.bat

# 2. Run tests directly
python test_flight_multiplayer.py
```

### Running Specific Tests

```python
from automation_client import UnrealAutomationClient

client = UnrealAutomationClient(r"C:\Path\To\Project")

# Test ship movement only
result = client.test_ship_movement(thrust_duration=3.0, expected_min_distance=100.0)

# Test all assist modes
results = client.test_flight_assist_modes()
```

### VR Hand Simulation Example

```cpp
// C++ - Add VRHandSimulator to actor
UVRHandSimulator* HandSim = CreateDefaultSubobject<UVRHandSimulator>(TEXT("VRHandSimulator"));

// Move hand to flight stick position
FVector StickPosition(50.0f, -25.0f, -20.0f);
HandSim->MoveHandTo(StickPosition, 1.0f, EMotionArcType::Natural);

// Simulate grab
HandSim->SimulateGrab(FlightStickActor, 0.5f);

// Apply stick input (deflect stick)
HandSim->SimulateStickInput(FVector(1, 0, 0), 0.8f, 2.0f);

// Release and return to rest
HandSim->SimulateRelease(HandSim->DefaultRestPosition, 0.5f);
```

## Test Results Format

### test_report.json
```json
{
  "timestamp": "2025-11-06 15:30:00",
  "summary": {
    "total": 5,
    "passed": 5,
    "failed": 0,
    "pass_rate": 100.0,
    "total_duration": 45.2
  },
  "tests": [
    {
      "name": "test_ship_movement",
      "success": true,
      "message": "Ship moved 523.45 units (expected >= 100)",
      "duration": 5.2
    }
  ]
}
```

## Workflow Integration

### Autonomous Development Loop

```python
# 1. Launch server once
client.launch_dedicated_server("VRTemplateMap")

# 2. Run tests
tester = MultiplayerFlightTester(project_path)
tester.run_all_tests()

# 3. Check results
if all_tests_passed:
    # Move to next task
    update_task_queue("T003", "completed")
else:
    # Analyze failures and fix
    analyze_failures()
    apply_fixes()
    # Retry
```

### Task Queue Integration

The system automatically updates `execution_queue.json`:

**On Success:**
```json
{
  "task_id": "T003",
  "status": "completed",
  "completed_at": "2025-11-06T15:30:00Z",
  "findings": "All flight tests passed - ship movement, multiplayer, 6DOF, VR hand simulation"
}
```

**On Failure:**
```json
{
  "task_id": "T003",
  "status": "failed",
  "last_error": "Some flight tests failed - see test_report.json"
}
```

## Key Features

### 1. **Self-Verifying Tests**
- No manual "press Alt+P" required
- Automated spawn, input, measurement
- Pass/fail determination without human intervention

### 2. **Multiplayer Ready**
- Up to 8 simultaneous players
- Network replication of flight inputs
- Spectator support for AI observation

### 3. **VR Simulation Without Hardware**
- Calculus-based realistic hand motion
- Tests VR grab mechanics
- Verifies flight stick interaction

### 4. **6DOF Flight Testing**
- Tests all 6 axes independently:
  - Forward/Backward (X)
  - Left/Right (Y)
  - Up/Down (Z)
  - Pitch, Yaw, Roll rotation

### 5. **Comprehensive Metrics**
- Position tracking
- Velocity/speed measurement
- Distance calculations
- Duration timing
- Screenshot proof

## Advantages Over Previous Approach

### Before: PIE with Manual Testing
```
1. Write code
2. Compile
3. Open editor
4. Press Alt+P
5. Manually test
6. Note results
7. Close editor
8. Repeat
```

**Problems:**
- Requires human intervention
- Can't run during AI context
- No automated verification
- Slow iteration

### After: Dedicated Server with Automation
```
1. Write code
2. Compile once
3. launch_autonomous_testing.bat
4. AI reads results
5. AI continues development
```

**Benefits:**
- Fully autonomous
- Runs during AI session
- Automated verification
- Rapid iteration
- Multiplayer testing included

## Future Enhancements

### Planned Features:
1. **HTTP Server** - True HTTP REST API (currently uses command-based API)
2. **Live HUD Capture** - Screenshot HUD elements during flight
3. **Replay System** - Record and replay test sessions
4. **AI Spectator** - AI joins as spectator to observe live gameplay
5. **Genetic Algorithm Tuning** - Auto-tune flight physics parameters
6. **Waypoint Course Testing** - Automated waypoint navigation tests

### Advanced VR Hand Simulation:
1. **Inverse Kinematics** - Full arm/hand IK for realistic reach
2. **Grip Simulation** - Simulate different grip strengths
3. **Haptic Feedback Sim** - Model expected haptic response
4. **Fatigue Modeling** - Simulate hand fatigue over time

## Troubleshooting

### Build Errors
- Check that Unreal Engine 5.6 is installed at `C:\Program Files\Epic Games\UE_5.6`
- Verify project compiles in Visual Studio first
- Check `Saved/Logs/` for detailed error messages

### Test Failures
- Check `automation_log.txt` for execution details
- Verify ships are spawning with debug visualization
- Check network replication in multiplayer scenarios
- Review `test_report.json` for specific failure messages

### Server Won't Start
- Check if another instance is already running (Task Manager)
- Verify firewall allows UnrealEditor.exe
- Check port 8080 isn't in use
- Review `Saved/Logs/Alexander.log`

## Performance

### Typical Test Times:
- Single ship movement: ~5s
- Multiple ships test: ~8s
- Flight assist modes: ~12s (5 modes × 2s each)
- VR hand simulation: ~3s
- 6DOF movement: ~15s (6 axes × 2.5s each)

**Total test suite: ~45 seconds**

### Resource Usage:
- CPU: ~30% (during tests)
- RAM: ~4GB (editor + server)
- Disk: Minimal (logs only)

## Conclusion

This autonomous testing system solves the fundamental problem: **"If you can't test it, you can't build it."**

By combining dedicated server, multiplayer architecture, VR hand simulation, and comprehensive automated testing, the AI can now:
- Build features autonomously
- Verify they work without human intervention
- Iterate rapidly
- Develop multiplayer functionality
- Test VR interactions without hardware

**The system makes autonomous game development possible.**
