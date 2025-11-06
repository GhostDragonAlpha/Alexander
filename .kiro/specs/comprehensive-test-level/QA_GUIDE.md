# Comprehensive Test Level - QA Guide

## Overview

This guide provides Quality Assurance teams with everything needed to run, interpret, and report on comprehensive test level executions. It covers both manual testing procedures and automated test execution.

## Quick Start

### Launching the Test Level

1. Open Unreal Editor
2. File > Open Level
3. Navigate to: `Content/TestLevels/TestLevel_Comprehensive.umap`
4. Wait for level to load completely (watch bottom left for streaming notifications)

### Running All Tests

**Method 1: Using the Python Test Runner (Recommended)**

```powershell
# Open PowerShell in project root
cd C:\Users\allen\Desktop\Alexander\Alexander

# Run all tests with HTML report
python run_comprehensive_test_level.py --report-format html --output-dir ./test_results

# Check test_results/test_report.html in your browser
```

**Method 2: Using Blueprint UI**

1. In the Comprehensive Test Level, locate the "Test Navigation Hub"
2. Interact with the "Run All Tests" button
3. Watch the Results Panel for real-time progress
4. Report autosaves to `Saved/Tests/TestResults.json`

**Method 3: Using Console Command**

1. In game, open the console (press `~`)
2. Type: `test.runall`
3. Press Enter and wait for completion
4. Results appear in Output Log

## Test Stations and Locations

The test level is organized into 7 distinct test zones. Each has a dedicated area and navigation pad.

### 1. VR Test Station

**Location**: Central Hub, North Teleport Pad
**Category**: VR Interactions
**Duration**: ~5-10 minutes

**Tests Performed**:
- VR Calibration (tracking accuracy <1mm)
- Object grabbing and manipulation
- UI button interaction
- Comfort validation (frame stability)

**Navigation**:
1. Spawn at Central Hub
2. Look for blue "VR TEST" teleport pad
3. Interact with pad to warp to VR Testing Zone
4. Station entrance is to the north

**What to Expect**:
- Calibration grid appears for hand tracking
- Interactive cubes spawn for grab testing
- Menu panels for UI testing
- Clear status indicators for each test

**Performance Target**: 90+ FPS, <1mm tracking error

### 2. Spaceship Flight Test Station

**Location**: Central Hub, Northeast Teleport Pad
**Category**: Flight Physics & Controls
**Duration**: ~10-15 minutes

**Tests Performed**:
- Flight control responsiveness
- Newtonian physics validation
- Navigation waypoint accuracy
- Cockpit system functionality

**Navigation**:
1. Use "SPACESHIP FLIGHT" teleport pad (green)
2. Enter spaceship at spawn point
3. Follow on-screen tutorial prompts
4. Navigate to waypoints at 100m, 1km, 10km distances

**What to Expect**:
- Spaceship spawns at controlled altitude
- Flight console activates with HUD
- Waypoint markers glow when in range
- Real-time FPS counter visible in HUD

**Control Quick Reference**:
- **W/S**: Throttle up/down
- **A/D**: Yaw left/right
- **Q/E**: Roll left/right
- **Mouse Y**: Pitch
- **Space**: Boost
- **Ctrl**: Brake
- **F**: Open flight systems panel

### 3. Celestial Scaling Test Station

**Location**: Central Hub, East Teleport Pad
**Category**: Celestial Physics & Scaling
**Duration**: ~15-20 minutes

**Tests Performed**:
- Planet scale factor validation
- Gravitational force calculations
- VR precision at cosmic distances
- Performance with 50+ bodies

**Navigation**:
1. Use "CELESTIAL SCALING" teleport pad (purple)
2. Player starts at cosmic perspective (10 million km from test planet)
3. Navigation controls move you closer to planet
4. Observe scale transitions

**What to Expect**:
- Starts with tiny planet visible (inverse-square scaling)
- Planet grows visibly as you approach
- Gravity increases realistically
- Debug visualization shows scale factors
- Performance HUD tracks FPS throughout

**Key Metrics**:
- Scale accuracy: Within 1% of theoretical
- Gravity synchronization: Within 5% error
- Frame rate: Maintains 60+ FPS

### 4. Planet Landing Test Station

**Location**: Central Hub, Southeast Teleport Pad
**Category**: Landing Mechanics & Transitions
**Duration**: ~12-18 minutes

**Tests Performed**:
- Atmospheric entry effects
- Landing guidance system
- Terrain LOD transitions
- Physics transition validation

**Navigation**:
1. Use "PLANET LANDING" teleport pad (orange)
2. Board spaceship and follow landing guidance
3. Approach planet at controlled angle
4. Land on designated pads

**Landing Guidance System**:
- **Green Status**: Clear to land
- **Yellow Status**: Approach adjustments needed
- **Red Status**: Landing prohibited
- **Altitude Display**: Real-time distance to surface
- **Velocity Indicator**: Descent speed

**Requirements**:
- Altitude: Descend steadily
- Velocity: <5 m/s at touchdown
- Approach Angle: <45 degrees
- Slope: Landing area <15 degrees

### 5. Farming Test Station

**Location**: Central Hub, West Teleport Pad
**Category**: Agriculture & Growth Systems
**Duration**: ~10-15 minutes (real-time) or 2-3 minutes (accelerated)

**Tests Performed**:
- Seed planting mechanics
- Crop growth rate simulation
- Water requirement validation
- Harvest mechanics

**Navigation**:
1. Use "FARMING" teleport pad (green)
2. Spawn in agricultural test zone
3. Farm plots arranged in organized grid
4. Different crop samples at each growth stage

**What to Expect**:
- Sample plots show crops at: Seedling, Growth, Mature, Harvestable stages
- Interactive plots allow planting with VR hands
- Watering tool available for water testing
- Real-time growth visualization (accelerated for testing)

**Crop Types Tested**:
- Wheat (Temperature: 20C, Water: 100mm)
- Lettuce (Temperature: 18C, Water: 80mm)
- Tomato (Temperature: 25C, Water: 120mm)
- Carrot (Temperature: 17C, Water: 90mm)

**Environmental Variables**:
- Temperature: Adjustable via control panel
- Humidity: Affects growth rate
- Sunlight: Measured in lux
- Water Level: Controlled by watering mechanics

### 6. AI Vision Test Station

**Location**: Central Hub, Northwest Teleport Pad
**Category**: Computer Vision & AI Integration
**Duration**: ~5-10 minutes

**Tests Performed**:
- Screenshot capture quality
- Scene analysis accuracy
- Code generation validation
- Iteration feedback loop

**Navigation**:
1. Use "AI VISION" teleport pad (light blue)
2. Enter test scene gallery
3. Interact with test triggers to capture scenes
4. AI analyzes and generates results

**Test Scenes**:
- Empty room with lighting
- Complex scene with multiple objects
- VR interaction scenario
- Performance stress scene

**What to Expect**:
- Screenshots capture in 4K resolution
- Metadata logged (lighting, objects, materials)
- AI generates improvement suggestions
- Code output appears in results panel
- Quality metrics displayed

### 7. Performance Benchmark Station

**Location**: Central Hub, South Teleport Pad
**Category**: Performance Profiling
**Duration**: ~20-30 minutes

**Tests Performed**:
- Frame rate baseline measurement
- Memory usage profiling
- CPU/GPU time analysis
- Stress test with 100+ actors

**Navigation**:
1. Use "PERFORMANCE" teleport pad (red)
2. Station contains multiple benchmark areas
3. Test equipment and profiling displays
4. Real-time metrics on all screens

**Metrics Captured**:
- Average FPS (Target: 90+)
- Min/Max FPS (Consistency check)
- Average Frame Time (Target: <11.1ms)
- Peak Memory (Target: <3GB)
- CPU Time (Target: <5ms)
- GPU Time (Target: <5ms)
- Draw Calls (Optimization metric)

**Stress Test Progression**:
1. Empty scenario (baseline)
2. 10 actors
3. 25 actors
4. 50 actors
5. 100 actors (failure threshold)

## Understanding Test Results

### JSON Report Format

After running tests, a JSON report is generated. Key fields:

```json
{
  "timestamp": "2024-01-15T10:30:45.123456",
  "total_stations": 7,
  "passed_stations": 7,
  "failed_stations": 0,
  "total_tests": 28,
  "passed_tests": 28,
  "failed_tests": 0,
  "total_duration": 45.23,
  "pass_rate": 100.0,
  "station_results": [
    {
      "station_name": "VRTestStation",
      "category": "VR",
      "status": "PASS",
      "total_tests": 4,
      "passed_tests": 4,
      "failed_tests": 0,
      "duration": 8.45,
      "test_results": [
        {
          "name": "CalibrationTest",
          "status": "PASS",
          "duration": 2.1,
          "message": "Tracking accuracy: 0.8mm (< 1mm threshold)"
        }
      ]
    }
  ]
}
```

### CSV Report Format

Compact tabular format ideal for tracking:

```
Station,Category,Status,Total Tests,Passed,Failed,Duration (s),Test Name,Test Status
VRTestStation,VR,PASS,4,4,0,8.45,CalibrationTest,PASS
VRTestStation,VR,PASS,4,4,0,8.45,ObjectInteractionTest,PASS
...
```

### HTML Report Visualization

Browser-friendly format with:
- Summary cards showing key metrics
- Color-coded station results (green = pass, red = fail)
- Detailed test listings
- Performance graphs
- Regression alerts

**Viewing**:
```powershell
# Open report in default browser
start test_results\test_report.html

# Or open directly in browser
firefox .\test_results\test_report.html
```

## VR Setup Requirements

For proper VR testing, ensure:

### Hardware
- VR Headset (Meta Quest 3, PlayStation VR2, Valve Index, etc.)
- Motion controllers calibrated
- Play space cleared (recommended 2m x 2m minimum)
- USB connection verified

### Software
- Latest VR runtime installed (OpenXR, SteamVR, or platform-specific)
- Unreal Engine VR plugins enabled
- Player seated or standing as appropriate
- Guardian boundary set

### Test Calibration
1. Wear headset and hold controllers at rest
2. Press "Calibrate" button in VR Test Station
3. Stand at world origin (marked with grid)
4. Keep hands still for 2 seconds
5. Calibration complete when green checkmark appears

## Performance Metrics Interpretation

### Frame Rate (FPS)

| Range | Status | Action |
|-------|--------|--------|
| 90+ | Excellent | Meets VR target, no action needed |
| 75-89 | Good | Acceptable, monitor closely |
| 60-74 | Concerning | May cause discomfort, investigate cause |
| <60 | Critical | Unacceptable for VR, immediate action required |

### Frame Time (ms)

| Range | Status | Notes |
|-------|--------|-------|
| <10 | Excellent | Smooth, responsive |
| 10-11 | Good | Adequate for 90 FPS |
| 11-15 | Concerning | Occasional frame drops expected |
| >15 | Critical | Noticeable stuttering |

### Memory Usage

| Range | Status | Action |
|-------|--------|--------|
| <2GB | Excellent | Well optimized |
| 2-3GB | Good | Normal for active features |
| 3-4GB | Concerning | Monitor for leaks |
| >4GB | Critical | Memory leak suspected |

### Performance Regression

- **Detected**: Current metrics worse than baseline by >15% (FPS) or >10% (Memory)
- **Action**: Stop testing, notify developers, capture logs

## Common Test Issues and Solutions

### Issue: VR Tracking Drift

**Symptoms**: Hand position drifts away from actual hand position
**Solution**:
1. Re-calibrate using "VR Calibration Test"
2. Ensure clear line of sight to tracking cameras
3. Move away from magnetic interference
4. Check controller battery levels

### Issue: Spaceship Won't Launch

**Symptoms**: Flight controls unresponsive, ship stays grounded
**Solution**:
1. Verify "Flight Controls Test" passed
2. Check that you're in the spaceship cockpit
3. Ensure all thrusters show green status
4. Restart the station

### Issue: Crops Won't Grow

**Symptoms**: Growth remains at 0% despite elapsed time
**Solution**:
1. Verify environmental conditions in control panel
2. Ensure water level is above minimum
3. Check temperature is within crop requirements
4. Verify sunlight sensor is active
5. Use "Accelerate Growth" button for testing

### Issue: Landing Guidance Red

**Symptoms**: Cannot land on planet, "Landing Prohibited" status
**Solution**:
1. Check approach velocity (<5 m/s requirement)
2. Verify landing pad is not occupied
3. Ensure approach angle <45 degrees
4. Check terrain slope <15 degrees
5. Try different landing pad location

### Issue: Performance Drop During Test

**Symptoms**: FPS decreases as test progresses
**Solution**:
1. Monitor memory usage - check for leaks
2. Pause other applications
3. Enable GPU profiling to identify bottleneck
4. Reduce visual quality if necessary
5. Report to development team if persistent

## Running Tests for Specific Systems

### Testing Only VR Features

```powershell
python run_comprehensive_test_level.py --station VRTestStation --report-format html
```

### Testing Only Flight Physics

```powershell
python run_comprehensive_test_level.py --station SpaceshipFlightTestStation
```

### Testing Performance with Regression Check

```powershell
python run_comprehensive_test_level.py --station PerformanceBenchmarkStation `
  --check-regression --report-format html --output-dir ./perf_results
```

## Test Coverage Matrix

| System | VR | Flight | Celestial | Landing | Farming | AI Vision | Performance |
|--------|----|----|-----------|---------|---------|-----------|-------------|
| Physics | | ✓ | ✓ | ✓ | | | ✓ |
| Rendering | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ |
| Audio | | ✓ | | ✓ | ✓ | | ✓ |
| Input | ✓ | ✓ | | | ✓ | ✓ | |
| Networking | | | | | | | ✓ |
| Memory | | | | | | | ✓ |
| VR Comfort | ✓ | | ✓ | ✓ | ✓ | | ✓ |

## Reporting Test Failures

### When Tests Fail

1. **Note the test name** - Exactly which test failed?
2. **Capture the error message** - What specific validation failed?
3. **Note the conditions** - What were the exact metrics?
4. **Record environment** - VR device, graphics settings, system specs
5. **Capture logs** - Copy relevant log entries from Output Log

### Failure Report Template

```
Test Name: [Station Name] / [Test Name]
Status: FAIL
Error Message: [Exact error from report]
Metrics: [Measured vs. Expected values]
Steps to Reproduce:
1. [First step]
2. [Second step]
3. [Third step]
Environment:
- VR Device: [Device name]
- Graphics Quality: [Ultra/High/Medium/Low]
- Resolution: [Native/2x/0.5x]
- System RAM: [GB]
- GPU: [Model]
Logs Attached: [Y/N]
```

### Where to Report

- **GitHub Issues**: For reproducible bugs
- **Slack #qa-testing**: For quick discussions
- **JIRA**: For formal issue tracking
- **Email (QA Lead)**: For urgent failures

## Continuous Testing

### Daily Test Run Checklist

- [ ] Run full test suite each morning
- [ ] Check for performance regressions
- [ ] Review HTML report for failures
- [ ] File issues for any new failures
- [ ] Archive reports by date
- [ ] Update test baseline if intentional changes made

### Weekly Summary

Prepare weekly report:
- Total tests run
- Pass rate trend
- New failures
- Resolved issues
- Performance trends
- Recommendations

## Troubleshooting

### Test Runner Won't Start

```powershell
# Check Python version
python --version  # Should be 3.7+

# Check Unreal Editor path
where UnrealEditor-Cmd

# Try with full path
python C:\path\to\run_comprehensive_test_level.py
```

### Tests Timeout

```powershell
# Increase timeout to 30 minutes (1800 seconds)
python run_comprehensive_test_level.py --timeout 1800
```

### Can't Find Output Reports

```powershell
# Reports default to project root
Get-ChildItem *.json | Sort-Object LastWriteTime -Descending | Select-Object -First 5

# Or specify output directory
python run_comprehensive_test_level.py --output-dir ./test_results
```

## Resources and Support

- **Test Level Documentation**: See `.kiro/specs/comprehensive-test-level/`
- **Developer Guide**: `DEVELOPER_GUIDE.md` - for test modification
- **CI/CD Integration**: `CI_CD_INTEGRATION.md` - for automation setup
- **Slack Channel**: #testing for questions
- **Test Results Archive**: `Saved/Tests/` directory

## Next Steps

1. Run your first test: `python run_comprehensive_test_level.py`
2. Review the HTML report in a browser
3. Familiarize yourself with each test station
4. Set up daily automated testing (see CI_CD_INTEGRATION.md)
5. Report any issues found

## Quick Reference Commands

```powershell
# Run all tests with HTML output
python run_comprehensive_test_level.py --report-format html

# Run specific station
python run_comprehensive_test_level.py --station VRTestStation

# Run single test
python run_comprehensive_test_level.py --station VRTestStation --test CalibrationTest

# Check for regressions
python run_comprehensive_test_level.py --check-regression

# Extended timeout for slow systems
python run_comprehensive_test_level.py --timeout 1800

# Save reports to specific directory
python run_comprehensive_test_level.py --output-dir ./my_reports
```
