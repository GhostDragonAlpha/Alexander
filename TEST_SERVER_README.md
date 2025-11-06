# AI Agent-Controlled Test Server

Automated test execution system for Alexander VR Space Simulation (Unreal Engine 5.6) that allows AI agents like Claude to programmatically control test execution.

## ✅ Installation Complete

All dependencies installed and verified:
- ✅ FastAPI, uvicorn, pydantic, websockets
- ✅ Pillow, imagehash (visual regression)
- ✅ requests, schedule (automation)
- ✅ Unreal Engine 5.6 detected

## 🚀 Quick Start

### Option 1: Use Agent SDK (Recommended for AI Agents)

```python
from agent_sdk import TestServerController

# Agent starts server, runs tests, gets results, stops server
with TestServerController() as server:
    test_id = server.run_tests(["VRTestStation", "SpaceshipFlightTestStation"])
    results = server.wait_for_completion(test_id)
    print(f"Passed: {results['results']['passed']}/{results['results']['total_tests']}")
```

### Option 2: Run Example Scripts

```bash
# Basic test run
python agent_examples/basic_test_run.py

# Iterative debugging workflow
python agent_examples/iterative_debugging.py

# Performance optimization workflow
python agent_examples/performance_optimization.py
```

### Option 3: Manual Server Control

```bash
# Start server manually
python test_server.py

# In another terminal, use curl or Python to interact:
curl http://localhost:8000/api/status

# Open web dashboard
start web_ui.html  # Opens in browser
```

## 📁 Project Structure

```
Alexander/
├── test_server.py                    # FastAPI server (localhost:8000)
├── agent_sdk.py                      # TestServerController for agents
├── ue_test_executor.py               # Unreal Engine launcher/parser
├── web_ui.html                       # Real-time monitoring dashboard
├── verify_installation.py            # Installation verification
│
├── agent_examples/
│   ├── basic_test_run.py            # Simple workflow
│   ├── iterative_debugging.py       # Turn-based debugging
│   └── performance_optimization.py  # Performance tuning
│
├── .kiro/specs/comprehensive-test-level/
│   ├── FINAL_IMPLEMENTATION_SUMMARY.md
│   ├── DEVELOPER_GUIDE.md
│   ├── QA_GUIDE.md
│   └── CI_CD_INTEGRATION.md
│
└── Source/Alexander/
    ├── Public/Testing/              # C++ test infrastructure
    └── Private/Testing/             # C++ implementations
```

## 🤖 How AI Agents Use This

The system uses a **turn-based interaction model**:

1. **Agent starts server** via `TestServerController()`
2. **Agent sends command** (e.g., run tests)
3. **Server executes** and returns complete JSON snapshot
4. **Agent analyzes** results and decides next action
5. **Iterate** until tests pass or issue identified
6. **Agent stops server** automatically

### Example: Claude (AI Agent) Workflow

```python
with TestServerController() as server:
    # Step 1: Run all tests
    test_id = server.run_tests()
    results = server.wait_for_completion(test_id)

    # Step 2: If failures, analyze
    if results['results']['failed'] > 0:
        analysis = server.analyze_failures(test_id)

        # Step 3: Apply auto-fixes
        for failure in analysis['failures']:
            if failure['can_auto_fix']:
                server.apply_fix(failure['suggested_fix'])

        # Step 4: Re-test
        rerun_id = server.rerun_tests(failed_tests)
        rerun_results = server.wait_for_completion(rerun_id)

        # Step 5: Verify improvement
        if rerun_results['results']['failed'] == 0:
            print("All tests now passing!")
```

## 🎯 Available Test Stations

The system includes 7 comprehensive test stations (164 total test cases):

1. **VRTestStation** (15 tests)
   - VR comfort validation (90+ FPS)
   - Controller tracking accuracy
   - Hand presence and interaction
   - Teleportation mechanics

2. **SpaceshipFlightTestStation** (15 tests)
   - Flight physics and controls
   - Weapon systems
   - Shield and power management
   - Docking procedures

3. **CelestialScalingTestStation** (10 tests)
   - Time dilation system
   - Orbital mechanics
   - Scale transitions
   - Relativistic effects

4. **PlanetLandingTestStation** (33 tests)
   - Atmospheric entry
   - Landing zone validation
   - Terrain collision
   - Weather effects

5. **FarmingTestStation** (33 tests)
   - Crop growth cycles
   - Resource management
   - Colony building placement
   - Economic simulation

6. **AIVisionTestStation** (20 tests)
   - Object detection
   - Pathfinding
   - Behavior trees
   - Performance benchmarks

7. **PerformanceBenchmarkStation** (38 tests)
   - FPS monitoring (VR target: 90+)
   - Memory usage tracking
   - Draw call optimization
   - Asset streaming

## 🔧 API Endpoints

### GET /api/status
Get complete server state snapshot (current test, completed tests, uptime, etc.)

### POST /api/run_tests
Execute test stations
```json
{
  "stations": ["VRTestStation", "SpaceshipFlightTestStation"],
  "parallel": true,
  "timeout": 600,
  "settings": {}
}
```

### GET /api/tests/{test_id}
Get results for specific test execution

### POST /api/analyze
Analyze test failures with AI suggestions
```json
{
  "test_id": "test_20250105_123456",
  "focus": "performance"  // or "failures", "all"
}
```

### POST /api/rerun
Rerun specific tests with adjusted settings
```json
{
  "tests": ["VRTestStation"],
  "settings": {
    "fps_tolerance": 5,
    "timeout": 300
  }
}
```

### POST /api/shutdown
Gracefully shutdown the server

## 📊 Web Dashboard

Open `web_ui.html` in your browser while the server is running to see:
- Real-time server status
- Current test execution
- Live console output
- Performance metrics
- API endpoint documentation

Dashboard auto-refreshes every 2 seconds.

## ⚙️ Hardware Optimization

The system automatically leverages your hardware:
- **24 CPU cores**: Parallel test station execution
- **128GB RAM**: Multiple UE instances simultaneously
- **RTX 4090**: Fast rendering and performance benchmarks

## 🧪 Test Results Format

All test results are returned as JSON:

```json
{
  "status": "completed",
  "timestamp": "2025-11-05T20:00:00",
  "results": {
    "total_tests": 30,
    "passed": 28,
    "failed": 2,
    "pass_rate": 93.33,
    "performance": {
      "avg_fps": 95.2,
      "min_fps": 87.5,
      "avg_memory_mb": 2847,
      "meets_vr_target": true
    },
    "stations": [...]
  }
}
```

## 📖 Documentation

Comprehensive guides available in `.kiro/specs/comprehensive-test-level/`:

- **FINAL_IMPLEMENTATION_SUMMARY.md** - Complete overview (50 KB)
- **DEVELOPER_GUIDE.md** - Adding test stations, writing tests
- **QA_GUIDE.md** - Test execution, result interpretation
- **CI_CD_INTEGRATION.md** - GitHub Actions, GitLab CI, Jenkins

## 🐛 Troubleshooting

### Server won't start
```bash
# Check if port 8000 is in use
netstat -an | findstr :8000

# Kill existing server
taskkill /F /IM python.exe /FI "WINDOWTITLE eq test_server.py"
```

### UE Editor not found
Edit `ue_test_executor.py` and update the path:
```python
possible_paths = [
    Path("C:/YOUR/CUSTOM/PATH/UnrealEditor-Cmd.exe"),
    # ... existing paths
]
```

### Tests timeout
Increase timeout in test execution:
```python
server.run_tests(stations=[...], timeout=1200)  # 20 minutes
```

## 🎮 Integration with Claude Code

When using Claude Code (AI coding assistant), I can now:

1. **Automatically run tests** after making code changes
2. **Analyze failures** and suggest fixes
3. **Iteratively debug** until tests pass
4. **Optimize performance** based on benchmarks
5. **Generate test reports** with detailed analysis

Example Claude workflow:
```
User: "Fix the VR comfort issues"

Claude:
1. Runs VRTestStation via agent_sdk
2. Identifies failing comfort tests
3. Analyzes suggested fixes
4. Modifies C++ code
5. Re-runs tests to verify
6. Reports results to user
```

## 📝 License

Part of the Alexander VR Space Simulation project.

## 🤝 Support

For issues or questions:
- Check documentation in `.kiro/specs/comprehensive-test-level/`
- Run `python verify_installation.py` to diagnose issues
- Review test results in JSON format for detailed diagnostics

---

**Status**: ✅ Fully Installed and Ready to Use

**Last Verified**: 2025-11-05

**Unreal Engine**: 5.6 (detected at `C:\Program Files\Epic Games\UE_5.6\`)
