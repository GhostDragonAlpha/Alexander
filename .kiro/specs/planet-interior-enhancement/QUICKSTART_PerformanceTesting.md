# Quick Start: Performance Testing System

## Overview

The Performance Testing System provides comprehensive performance measurement and validation for the planet interior system. It measures frame rates, streaming bandwidth, memory consumption, and VR performance at various altitudes to validate requirements 8.1-8.5.

## Key Features

- **Frame Rate Measurement**: Captures FPS, frame time, game thread, render thread, and GPU time
- **Memory Tracking**: Monitors memory usage and identifies peak consumption
- **Streaming Bandwidth**: Measures data streaming rates during terrain loading
- **VR Performance**: Validates VR-specific metrics including motion-to-photon latency
- **Multi-Altitude Testing**: Tests performance at different altitudes (orbital to surface)
- **Automated Reporting**: Generates text, CSV, and JSON reports

## Quick Start

### 1. Basic Performance Test

```cpp
// Get the performance testing system
UPerformanceTestingSystem* PerfSystem = GetWorld()->GetSubsystem<UPerformanceTestingSystem>();

// Create test configuration
FPerformanceTestConfig Config;
Config.TestAltitudes = {100000.0f, 50000.0f, 10000.0f, 1000.0f, 100.0f};
Config.SampleDuration = 5.0f;
Config.SamplesPerAltitude = 100;
Config.bTestVRPerformance = true;
Config.bGenerateReport = true;

// Start the test
PerfSystem->StartPerformanceTest(Config);
```

### 2. Monitor Test Progress

```cpp
// Check if test is running
if (PerfSystem->IsTestRunning())
{
    // Get progress (0-1)
    float Progress = PerfSystem->GetTestProgress();
    UE_LOG(LogTemp, Log, TEXT("Test Progress: %.1f%%"), Progress * 100.0f);
}
```

### 3. Capture Single Metrics

```cpp
// Capture current performance metrics
FPerformanceMetrics Metrics = PerfSystem->CapturePerformanceMetrics();

UE_LOG(LogTemp, Log, TEXT("FPS: %.2f | Frame Time: %.2fms | Memory: %.2fMB"),
    Metrics.FrameRate, Metrics.FrameTime, Metrics.MemoryUsageMB);

// Capture VR-specific metrics
FVRPerformanceMetrics VRMetrics = PerfSystem->CaptureVRPerformanceMetrics();

UE_LOG(LogTemp, Log, TEXT("Latency: %.2fms | Dropped Frames: %d"),
    VRMetrics.MotionToPhotonLatency, VRMetrics.DroppedFrames);
```

### 4. Generate Reports

```cpp
// Generate performance report
FString ReportPath = FPaths::ProjectSavedDir() / TEXT("PerformanceReports");
PerfSystem->GeneratePerformanceReport(ReportPath);

// Reports are generated in three formats:
// - PerformanceReport_YYYYMMDD_HHMMSS.txt (human-readable)
// - PerformanceReport_YYYYMMDD_HHMMSS.csv (spreadsheet)
// - PerformanceReport_YYYYMMDD_HHMMSS.json (machine-readable)
```

### 5. Validate VR Performance

```cpp
// Get test results
const FPerformanceTestResults& Results = PerfSystem->GetTestResults();

// Validate against VR requirements (90 FPS minimum)
bool MeetsVRRequirements = PerfSystem->ValidateVRPerformance(Results);

if (MeetsVRRequirements)
{
    UE_LOG(LogTemp, Log, TEXT("VR performance requirements met!"));
}
else
{
    UE_LOG(LogTemp, Warning, TEXT("VR performance below requirements"));
}
```

## Blueprint Usage

### Start Performance Test

1. Get Performance Testing System subsystem
2. Create Performance Test Config struct
3. Set test altitudes array
4. Set sample duration and samples per altitude
5. Call "Start Performance Test"

### Monitor Progress

1. Call "Is Test Running" to check status
2. Call "Get Test Progress" for progress percentage
3. Display progress in UI

### View Results

1. Call "Get Test Results" to retrieve results
2. Access properties:
   - Average Frame Rate
   - Minimum Frame Rate
   - Maximum Frame Rate
   - Average Memory Usage MB
   - Peak Memory Usage MB
   - Meets VR Requirements

## Performance Metrics Explained

### Frame Rate Metrics

- **Frame Rate**: Frames per second (target: 90+ for VR)
- **Frame Time**: Milliseconds per frame (target: <11.1ms for 90 FPS)
- **Game Thread Time**: CPU time for game logic
- **Render Thread Time**: CPU time for rendering commands
- **GPU Time**: GPU processing time

### Memory Metrics

- **Memory Usage MB**: Current physical memory usage
- **Peak Memory Usage MB**: Maximum memory used during test
- Target: < 8GB for planet system (requirement 8.5)

### Streaming Metrics

- **Streaming Bandwidth MBps**: Data loading rate in MB/s
- **Active Terrain Tiles**: Number of loaded terrain tiles
- **Streaming Tiles**: Number of tiles currently loading
- Target: < 100 MB/s bandwidth (requirement 8.5)

### VR Metrics

- **Motion to Photon Latency**: Time from head movement to display update
- **Dropped Frames**: Frames that missed the display deadline
- **Reprojecting**: Whether the system is using reprojection
- **Headset FPS**: Native refresh rate of the VR headset

## Test Altitudes

Default test altitudes cover the full range from orbital to surface:

- **100,000m**: Orbital view
- **50,000m**: High atmosphere entry
- **25,000m**: Mid atmosphere
- **10,000m**: Low atmosphere
- **5,000m**: Approach altitude
- **1,000m**: Landing approach
- **500m**: Low altitude flight
- **100m**: Near surface
- **10m**: Ground level

## Requirements Validation

The system validates against these requirements:

### Requirement 8.1: VR Frame Rate

- **Target**: Minimum 90 FPS in VR
- **Validation**: Checks minimum and average frame rates
- **Status**: Reported in test results

### Requirement 8.2: LOD System

- **Target**: At least 6 detail levels
- **Validation**: Manual verification (not automated)
- **Status**: N/A in automated tests

### Requirement 8.3: Frustum Culling

- **Target**: Avoid rendering outside camera view
- **Validation**: Manual verification (not automated)
- **Status**: N/A in automated tests

### Requirement 8.4: GPU Instancing

- **Target**: Maximum 100,000 instances visible
- **Validation**: Manual verification (not automated)
- **Status**: N/A in automated tests

### Requirement 8.5: Streaming Performance

- **Target**: Maximum 5ms per frame for loading
- **Validation**: Measures streaming bandwidth and timing
- **Status**: Reported in test results

## Report Formats

### Text Report

Human-readable summary with:

- Test duration and timestamps
- Summary statistics
- Requirements validation
- Detailed samples by altitude

### CSV Report

Spreadsheet-compatible format with columns:

- Timestamp
- Altitude
- Frame Rate
- Frame Time
- Thread Times
- Memory Usage
- Streaming Bandwidth
- Terrain Tiles

### JSON Report

Machine-readable format for automated analysis:

```json
{
  "testStartTime": "2025-11-04 10:30:00",
  "testEndTime": "2025-11-04 10:35:00",
  "totalSamples": 900,
  "averageFrameRate": 95.2,
  "minimumFrameRate": 91.5,
  "maximumFrameRate": 100.3,
  "meetsVRRequirements": true,
  "samples": [...]
}
```

## Best Practices

### 1. Test Duration

- Use at least 5 seconds per altitude for stable results
- Capture 100+ samples per altitude for statistical significance
- Allow system to stabilize before starting tests

### 2. Test Conditions

- Close other applications to reduce interference
- Test with representative content (full planet with biomes)
- Test in both editor and packaged builds
- Test on target hardware specifications

### 3. VR Testing

- Always test with VR headset connected
- Ensure headset is tracking properly
- Test with realistic player movement
- Monitor for motion sickness indicators

### 4. Result Analysis

- Compare results across different altitudes
- Look for performance bottlenecks at specific altitudes
- Identify memory spikes during streaming
- Check for frame time consistency

### 5. Optimization

- If FPS < 90, investigate bottlenecks:
  - High GPU time → Reduce draw calls, optimize shaders
  - High game thread time → Optimize game logic
  - High render thread time → Reduce draw calls
  - High memory → Optimize LOD, reduce texture sizes
  - High streaming → Optimize tile sizes, reduce bandwidth

## Troubleshooting

### Test Won't Start

- Check that test altitudes array is not empty
- Verify samples per altitude > 0
- Ensure sample duration > 0

### Inaccurate Results

- Allow system to warm up before testing
- Increase sample count for more stable results
- Close background applications
- Test in packaged build for accurate performance

### VR Metrics Not Captured

- Ensure VR headset is connected and tracking
- Enable VR in project settings
- Check that bTestVRPerformance is true in config

### Reports Not Generated

- Check that output directory exists and is writable
- Verify bGenerateReport is true in config
- Check logs for file write errors

## Console Commands

```
// Start a quick performance test
PerformanceTest.Start

// Stop current test
PerformanceTest.Stop

// Capture single metrics snapshot
PerformanceTest.Capture

// Generate report from last test
PerformanceTest.GenerateReport

// Show current test progress
PerformanceTest.ShowProgress
```

## Integration with Other Systems

### Terrain Streaming

The performance system integrates with terrain streaming to measure:

- Active terrain tiles
- Tiles currently loading
- Streaming bandwidth usage

### VR System

Integrates with Unreal's XR system to capture:

- Motion-to-photon latency
- Dropped frames
- Reprojection status
- Headset refresh rate

### Memory System

Uses platform memory APIs to track:

- Physical memory usage
- Memory allocation patterns
- Peak memory consumption

## Example: Automated Performance Testing

```cpp
void AMyGameMode::RunAutomatedPerformanceTest()
{
    UPerformanceTestingSystem* PerfSystem = GetWorld()->GetSubsystem<UPerformanceTestingSystem>();

    // Configure comprehensive test
    FPerformanceTestConfig Config;
    Config.TestAltitudes = {100000.0f, 50000.0f, 25000.0f, 10000.0f,
                            5000.0f, 1000.0f, 500.0f, 100.0f, 10.0f};
    Config.SampleDuration = 10.0f;
    Config.SamplesPerAltitude = 200;
    Config.bTestVRPerformance = true;
    Config.bGenerateReport = true;
    Config.ReportOutputPath = FPaths::ProjectSavedDir() / TEXT("AutomatedTests");

    // Start test
    if (PerfSystem->StartPerformanceTest(Config))
    {
        UE_LOG(LogTemp, Log, TEXT("Automated performance test started"));

        // Set up completion callback
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, PerfSystem]()
        {
            if (!PerfSystem->IsTestRunning())
            {
                const FPerformanceTestResults& Results = PerfSystem->GetTestResults();

                UE_LOG(LogTemp, Log, TEXT("Test completed:"));
                UE_LOG(LogTemp, Log, TEXT("  Average FPS: %.2f"), Results.AverageFrameRate);
                UE_LOG(LogTemp, Log, TEXT("  Min FPS: %.2f"), Results.MinimumFrameRate);
                UE_LOG(LogTemp, Log, TEXT("  VR Requirements: %s"),
                    Results.bMeetsVRRequirements ? TEXT("PASS") : TEXT("FAIL"));

                // Clear timer
                GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
            }
        }, 1.0f, true);
    }
}
```

## See Also

- [VR Performance Optimization](QUICKSTART_VRPerformance.md)
- [Memory Optimization](QUICKSTART_MemoryOptimization.md)
- [Terrain Optimization](QUICKSTART_TerrainOptimization.md)
- [Testing Guide](TESTING_GUIDE.md)
