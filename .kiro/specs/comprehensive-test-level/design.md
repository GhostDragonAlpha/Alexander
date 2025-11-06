# Comprehensive Test Level - Design Document

## Overview

The Comprehensive Test Level is a dedicated testing environment that validates all major systems in Alexander. This level serves as both a QA tool and a showcase environment, providing structured test scenarios for VR interactions, spaceship flight, celestial scaling, planet landing, farming mechanics, AI vision integration, and performance validation.

The design leverages Unreal Engine 5.6's World Partition system for efficient streaming, Blueprint-based test stations for easy modification, and automated validation systems for continuous integration.

## Architecture

### Level Structure

```
TestLevel_Comprehensive (World Partition Map)
├── CentralHub (Always Loaded)
│   ├── SpawnPoint
│   ├── NavigationUI
│   └── TeleportStations
├── VRTestZone (Streaming Cell 1)
│   ├── CalibrationStation
│   ├── InteractionTests
│   └── ComfortValidation
├── SpaceFlightZone (Streaming Cell 2)
│   ├── SpaceshipSpawn
│   ├── FlightWaypoints
│   └── ObstacleCourse
├── CelestialScalingZone (Streaming Cell 3)
│   ├── TestStar
│   ├── TestPlanet
│   ├── TestMoon
│   └── DebugVisualization
├── PlanetLandingZone (Streaming Cell 4)
│   ├── LandablePlanet
│   ├── LandingPads
│   └── TerrainVariety
├── FarmingZone (Streaming Cell 5)
│   ├── FarmPlots
│   ├── CropSamples
│   └── VRTools
├── AIVisionZone (Streaming Cell 6)
│   ├── TriggerVolumes
│   ├── TestScenes
│   └── ValidationMarkers
└── PerformanceZone (Streaming Cell 7)
    ├── StressTestArea
    ├── BenchmarkStations
    └── ProfilingTools
```

## Components and Interfaces

### 1. Test Level Game Mode (ATestLevelGameMode)

**Purpose**: Manages test execution, validation, and reporting.

**Key Properties**:

```cpp
class ALEXANDER_API ATestLevelGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    // Test Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
    TArray<UTestStation*> RegisteredTestStations;

    UPROPERTY(BlueprintReadOnly, Category = "Testing")
    FTestResults CurrentTestResults;

    UPROPERTY(EditAnywhere, Category = "Testing")
    bool bAutoRunTests = false;

    UPROPERTY(EditAnywhere, Category = "Testing")
    float TestTimeout = 300.0f; // 5 minutes per test

    // Performance Monitoring
    UPROPERTY()
    UPerformanceProfiler* Profiler;

    UPROPERTY()
    UMemoryOptimizationManager* MemoryManager;

    UPROPERTY()
    UTickOptimizationManager* TickManager;

    // Test Execution
    UFUNCTION(BlueprintCallable, Category = "Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    void RunTestStation(FName StationName);

    UFUNCTION(BlueprintCallable, Category = "Testing")
    FTestResults GetTestResults();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    void ExportTestReport(FString FilePath);
};
```

### 2. Test Station Base Class (UTestStation)

**Purpose**: Base class for all test stations providing common functionality.

**Key Properties**:

```cpp
UCLASS(Blueprintable, Abstract)
class ALEXANDER_API UTestStation : public UActorComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Station")
    FName StationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Station")
    FText StationDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Station")
    ETestCategory Category;

    UPROPERTY(BlueprintReadOnly, Category = "Test Station")
    ETestStatus CurrentStatus;

    UPROPERTY(EditAnywhere, Category = "Test Station")
    TArray<FTestCase> TestCases;

    // Test Execution
    UFUNCTION(BlueprintNativeEvent, Category = "Testing")
    void InitializeTest();

    UFUNCTION(BlueprintNativeEvent, Category = "Testing")
    void RunTest();

    UFUNCTION(BlueprintNativeEvent, Category = "Testing")
    void ValidateTest();

    UFUNCTION(BlueprintNativeEvent, Category = "Testing")
    void CleanupTest();

    // Results
    UFUNCTION(BlueprintCallable, Category = "Testing")
    FTestStationResults GetResults();
};
```

### 3. VR Test Station (UVRTestStation)

**Purpose**: Validates VR interactions, tracking, and comfort.

**Test Cases**:

1. **VR Calibration Test**

   - Verify player at world origin (0,0,0)
   - Measure hand tracking accuracy (target: <1mm error)
   - Test head tracking precision
   - Validate controller input response time (<16ms)

2. **Grab and Manipulation Test**

   - Test object grabbing with both hands
   - Verify physics interactions
   - Test throw mechanics
   - Validate haptic feedback

3. **UI Interaction Test**

   - Test VR menu interactions
   - Verify pointer accuracy
   - Test button presses
   - Validate text input

4. **Comfort Validation Test**
   - Measure frame rate consistency (target: 90+ FPS)
   - Test locomotion comfort
   - Verify no judder or stuttering
   - Validate IPD settings

**Implementation**:

```cpp
class ALEXANDER_API UVRTestStation : public UTestStation
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "VR Testing")
    TArray<AActor*> InteractiveObjects;

    UPROPERTY(EditAnywhere, Category = "VR Testing")
    TArray<UWidgetComponent*> TestUIWidgets;

    UPROPERTY(EditAnywhere, Category = "VR Testing")
    float TrackingAccuracyThreshold = 1.0f; // mm

    UPROPERTY(EditAnywhere, Category = "VR Testing")
    float MinimumFPS = 90.0f;

    virtual void RunTest_Implementation() override;

private:
    void TestHandTracking();
    void TestObjectInteraction();
    void TestUIInteraction();
    void TestComfort();

    FVector MeasureTrackingError();
    float MeasureFrameRate();
};
```

### 4. Spaceship Flight Test Station (USpaceshipFlightTestStation)

**Purpose**: Validates spaceship flight mechanics, physics, and controls.

**Test Cases**:

1. **Basic Flight Controls Test**

   - Test throttle response
   - Verify pitch/yaw/roll controls
   - Test boost and brake
   - Validate 6DOF movement

2. **Physics Validation Test**

   - Verify Newtonian physics
   - Test momentum conservation
   - Validate collision detection
   - Test damage system

3. **Waypoint Navigation Test**

   - Navigate to waypoints at 100m, 1km, 10km, 100km
   - Measure navigation accuracy
   - Test autopilot system
   - Validate distance calculations

4. **Cockpit Interaction Test**
   - Test all cockpit controls
   - Verify holographic displays
   - Test VR hand interactions
   - Validate button feedback

**Implementation**:

```cpp
class ALEXANDER_API USpaceshipFlightTestStation : public UTestStation
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Flight Testing")
    TSubclassOf<AActor> SpaceshipClass;

    UPROPERTY(EditAnywhere, Category = "Flight Testing")
    TArray<FVector> WaypointLocations;

    UPROPERTY(EditAnywhere, Category = "Flight Testing")
    TArray<AActor*> ObstacleActors;

    UPROPERTY(EditAnywhere, Category = "Flight Testing")
    float NavigationAccuracyThreshold = 100.0f; // cm

    UPROPERTY(EditAnywhere, Category = "Flight Testing")
    float InputResponseThreshold = 16.0f; // ms

    virtual void RunTest_Implementation() override;

private:
    void TestFlightControls();
    void TestPhysics();
    void TestNavigation();
    void TestCockpitInteraction();

    AActor* SpawnedSpaceship;
    int32 CurrentWaypointIndex;
};
```

### 5. Celestial Scaling Test Station (UCelestialScalingTestStation)

**Purpose**: Validates the celestial scaling system with dynamic planet scaling and gravitational physics.

**Test Cases**:

1. **Scale Factor Validation Test**

   - Approach planet from 10 million km to surface
   - Verify inverse-square scaling law (2x closer = 4x bigger)
   - Measure scale factor accuracy
   - Validate smooth transitions

2. **Gravitational Synchronization Test**

   - Verify gravity matches visual scale
   - Test multi-body gravitational forces
   - Validate force vector calculations
   - Test gravitational influence radius

3. **VR Precision Test**

   - Verify player remains at origin
   - Test universe translation accuracy
   - Measure floating-point precision
   - Validate no tracking drift

4. **Performance Test**
   - Test with 50+ celestial bodies
   - Measure frame time impact
   - Verify 90+ FPS maintained
   - Profile CPU/GPU usage

**Implementation**:

```cpp
class ALEXANDER_API UCelestialScalingTestStation : public UTestStation
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Celestial Testing")
    TArray<AActor*> TestCelestialBodies;

    UPROPERTY(EditAnywhere, Category = "Celestial Testing")
    TArray<float> TestDistances; // km

    UPROPERTY(EditAnywhere, Category = "Celestial Testing")
    float ScaleAccuracyThreshold = 0.01f; // 1% error

    UPROPERTY(EditAnywhere, Category = "Celestial Testing")
    float GravityAccuracyThreshold = 0.05f; // 5% error

    UPROPERTY(EditAnywhere, Category = "Celestial Testing")
    int32 StressTestBodyCount = 50;

    virtual void RunTest_Implementation() override;

private:
    void TestScaling();
    void TestGravity();
    void TestVRPrecision();
    void TestPerformance();

    void MovePlayerToDistance(float DistanceKm);
    float MeasureScaleFactor(AActor* CelestialBody);
    FVector MeasureGravitationalForce();
};
```

### 6. Planet Landing Test Station (UPlanetLandingTestStation)

**Purpose**: Validates planet landing mechanics and space-to-surface transitions.

**Test Cases**:

1. **Approach and Landing Test**

   - Test atmospheric entry effects
   - Verify landing guidance UI
   - Test landing pad detection
   - Validate smooth touchdown

2. **Terrain LOD Test**

   - Verify LOD transitions
   - Test terrain streaming
   - Validate no popping artifacts
   - Measure load times

3. **Physics Transition Test**

   - Test gravity transition
   - Verify atmospheric drag
   - Test flight model changes
   - Validate collision detection

4. **Scale Transition Test**
   - Verify scaling reaches 1:1 at surface
   - Test coordinate system transition
   - Validate no visual glitches
   - Test player orientation

**Implementation**:

```cpp
class ALEXANDER_API UPlanetLandingTestStation : public UTestStation
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Landing Testing")
    AActor* TestPlanet;

    UPROPERTY(EditAnywhere, Category = "Landing Testing")
    TArray<FVector> LandingPadLocations;

    UPROPERTY(EditAnywhere, Category = "Landing Testing")
    float ApproachStartDistance = 100000.0f; // meters

    UPROPERTY(EditAnywhere, Category = "Landing Testing")
    float LandingAccuracyThreshold = 500.0f; // cm

    UPROPERTY(EditAnywhere, Category = "Landing Testing")
    float MaxLODTransitionTime = 0.1f; // seconds

    virtual void RunTest_Implementation() override;

private:
    void TestApproach();
    void TestLanding();
    void TestLODTransitions();
    void TestPhysicsTransition();
    void TestScaleTransition();

    FVector CurrentApproachDistance;
    bool bLandingComplete;
};
```

### 7. Farming System Test Station (UFarmingTestStation)

**Purpose**: Validates farming mechanics including planting, growth, and harvesting.

**Test Cases**:

1. **Planting System Test**

   - Test seed placement with VR hands
   - Verify soil plot detection
   - Test planting validation
   - Validate seed inventory

2. **Growth System Test**

   - Test growth stage progression
   - Verify growth rate calculations
   - Test environmental modifiers
   - Validate visual updates

3. **Watering System Test**

   - Test watering tool interaction
   - Verify moisture calculations
   - Test water distribution
   - Validate visual feedback

4. **Harvesting System Test**
   - Test crop harvesting with VR
   - Verify yield calculations
   - Test inventory integration
   - Validate crop removal

**Implementation**:

```cpp
class ALEXANDER_API UFarmingTestStation : public UTestStation
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Farming Testing")
    TArray<AActor*> FarmPlots;

    UPROPERTY(EditAnywhere, Category = "Farming Testing")
    TArray<TSubclassOf<AActor>> CropTypes;

    UPROPERTY(EditAnywhere, Category = "Farming Testing")
    TArray<AActor*> CropSamples; // Different growth stages

    UPROPERTY(EditAnywhere, Category = "Farming Testing")
    float AcceleratedGrowthRate = 100.0f; // For testing

    UPROPERTY(EditAnywhere, Category = "Farming Testing")
    bool bTestRealTimeGrowth = false;

    virtual void RunTest_Implementation() override;

private:
    void TestPlanting();
    void TestGrowth();
    void TestWatering();
    void TestHarvesting();

    void PlantTestCrop(FVector Location, TSubclassOf<AActor> CropClass);
    void AccelerateGrowth(AActor* Crop);
};
```

### 8. AI Vision Test Station (UAIVisionTestStation)

**Purpose**: Validates AI vision system integration and screenshot analysis.

**Test Cases**:

1. **Screenshot Capture Test**

   - Test viewport capture
   - Verify metadata collection
   - Test image quality
   - Validate file output

2. **Scene Analysis Test**

   - Test AI vision analysis
   - Verify object detection
   - Test quality assessment
   - Validate suggestions

3. **Code Generation Test**

   - Test Blueprint generation
   - Verify C++ code output
   - Test material modifications
   - Validate actor spawning

4. **Iteration Test**
   - Test feedback loop
   - Verify improvement tracking
   - Test goal achievement
   - Validate convergence

**Implementation**:

```cpp
class ALEXANDER_API UAIVisionTestStation : public UTestStation
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "AI Vision Testing")
    TArray<UBoxComponent*> TriggerVolumes;

    UPROPERTY(EditAnywhere, Category = "AI Vision Testing")
    TArray<AActor*> TestScenes;

    UPROPERTY(EditAnywhere, Category = "AI Vision Testing")
    FString AIBackendURL = "http://localhost:8000";

    UPROPERTY(EditAnywhere, Category = "AI Vision Testing")
    bool bAutoExecuteAISuggestions = false;

    UPROPERTY(EditAnywhere, Category = "AI Vision Testing")
    int32 MaxIterations = 10;

    virtual void RunTest_Implementation() override;

private:
    void TestScreenshotCapture();
    void TestSceneAnalysis();
    void TestCodeGeneration();
    void TestIteration();

    void CaptureTestScreenshot(FString SceneName);
    void SendToAIBackend(FString ImagePath, FString Metadata);
    void ExecuteAICode(FString Code);
};
```

### 9. Performance Benchmark Station (UPerformanceBenchmarkStation)

**Purpose**: Validates performance across all systems and platforms.

**Test Cases**:

1. **Frame Rate Test**

   - Measure average FPS
   - Test frame time consistency
   - Verify VR performance (90+ FPS)
   - Test under load

2. **Memory Usage Test**

   - Measure memory consumption
   - Test memory leaks
   - Verify garbage collection
   - Test streaming efficiency

3. **CPU/GPU Profiling Test**

   - Profile tick times
   - Measure draw calls
   - Test GPU utilization
   - Validate optimization systems

4. **Stress Test**
   - Test with 50+ celestial bodies
   - Spawn 100+ actors
   - Test complex scenes
   - Validate performance targets

**Implementation**:

```cpp
class ALEXANDER_API UPerformanceBenchmarkStation : public UTestStation
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Performance Testing")
    float TargetFPS = 90.0f;

    UPROPERTY(EditAnywhere, Category = "Performance Testing")
    float MaxMemoryMB = 3072.0f;

    UPROPERTY(EditAnywhere, Category = "Performance Testing")
    float MaxFrameTimeMs = 11.1f;

    UPROPERTY(EditAnywhere, Category = "Performance Testing")
    int32 StressTestActorCount = 100;

    UPROPERTY(EditAnywhere, Category = "Performance Testing")
    float BenchmarkDuration = 60.0f; // seconds

    virtual void RunTest_Implementation() override;

private:
    void TestFrameRate();
    void TestMemoryUsage();
    void TestCPUGPUProfile();
    void TestStressScenario();

    void SpawnStressTestActors();
    void MeasurePerformanceMetrics();
    void GenerateBenchmarkReport();

    TArray<FPerformanceSample> PerformanceSamples;
};
```

## Data Models

### Test Results Structure

```cpp
USTRUCT(BlueprintType)
struct FTestResults
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FDateTime TestStartTime;

    UPROPERTY(BlueprintReadWrite)
    FDateTime TestEndTime;

    UPROPERTY(BlueprintReadWrite)
    int32 TotalTests;

    UPROPERTY(BlueprintReadWrite)
    int32 PassedTests;

    UPROPERTY(BlueprintReadWrite)
    int32 FailedTests;

    UPROPERTY(BlueprintReadWrite)
    TArray<FTestStationResults> StationResults;

    UPROPERTY(BlueprintReadWrite)
    FPerformanceMetrics OverallPerformance;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> Errors;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> Warnings;
};

USTRUCT(BlueprintType)
struct FTestStationResults
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FName StationName;

    UPROPERTY(BlueprintReadWrite)
    ETestStatus Status;

    UPROPERTY(BlueprintReadWrite)
    float ExecutionTime;

    UPROPERTY(BlueprintReadWrite)
    TArray<FTestCaseResult> TestCaseResults;

    UPROPERTY(BlueprintReadWrite)
    FString ErrorMessage;
};

USTRUCT(BlueprintType)
struct FTestCaseResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString TestName;

    UPROPERTY(BlueprintReadWrite)
    bool bPassed;

    UPROPERTY(BlueprintReadWrite)
    float MeasuredValue;

    UPROPERTY(BlueprintReadWrite)
    float ExpectedValue;

    UPROPERTY(BlueprintReadWrite)
    FString Details;
};
```

### Performance Metrics Structure

```cpp
USTRUCT(BlueprintType)
struct FPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float AverageFPS;

    UPROPERTY(BlueprintReadWrite)
    float MinFPS;

    UPROPERTY(BlueprintReadWrite)
    float MaxFPS;

    UPROPERTY(BlueprintReadWrite)
    float AverageFrameTimeMs;

    UPROPERTY(BlueprintReadWrite)
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadWrite)
    float PeakMemoryMB;

    UPROPERTY(BlueprintReadWrite)
    int32 DrawCalls;

    UPROPERTY(BlueprintReadWrite)
    int32 Triangles;

    UPROPERTY(BlueprintReadWrite)
    float CPUTimeMs;

    UPROPERTY(BlueprintReadWrite)
    float GPUTimeMs;
};
```

## Level Layout and Navigation

### Central Hub Design

The central hub is the starting point and navigation center:

- **Spawn Point**: Player spawns at world origin (0,0,0)
- **Navigation Kiosk**: Interactive UI showing all test stations
- **Teleport Pads**: Quick travel to each test zone
- **Results Display**: Real-time test results visualization
- **Settings Panel**: Configure test parameters

### Test Zone Layout

Each test zone is a separate streaming cell:

1. **VR Test Zone** (500m x 500m)

   - Calibration area at origin
   - Interactive object arena
   - UI testing space
   - Comfort validation area

2. **Space Flight Zone** (100km x 100km x 100km)

   - Spaceship spawn point
   - Waypoint markers at various distances
   - Obstacle course
   - Open space for free flight

3. **Celestial Scaling Zone** (10 million km radius)

   - Test star at center
   - Test planet at 1 million km
   - Test moon orbiting planet
   - Debug visualization spheres

4. **Planet Landing Zone** (Planet surface + 100km atmosphere)

   - Landable planet with varied terrain
   - Multiple landing pads
   - Atmospheric entry zone
   - Surface exploration area

5. **Farming Zone** (1km x 1km)

   - Multiple farm plots
   - Crop samples at all growth stages
   - VR tool stations
   - Environmental control panel

6. **AI Vision Zone** (1km x 1km)

   - Multiple test scenes
   - Trigger volumes for auto-capture
   - Validation markers
   - Results display

7. **Performance Zone** (2km x 2km)
   - Benchmark stations
   - Stress test spawners
   - Profiling displays
   - Report generation area

## Testing Strategy

### Automated Testing

The test level supports fully automated testing:

```cpp
// Run all tests automatically
void ATestLevelGameMode::RunAllTests()
{
    TestResults = FTestResults();
    TestResults.TestStartTime = FDateTime::Now();

    for (UTestStation* Station : RegisteredTestStations)
    {
        Station->InitializeTest();
        Station->RunTest();
        Station->ValidateTest();

        FTestStationResults Results = Station->GetResults();
        TestResults.StationResults.Add(Results);

        if (Results.Status == ETestStatus::Passed)
        {
            TestResults.PassedTests++;
        }
        else
        {
            TestResults.FailedTests++;
        }

        Station->CleanupTest();
    }

    TestResults.TestEndTime = FDateTime::Now();
    TestResults.TotalTests = TestResults.PassedTests + TestResults.FailedTests;

    ExportTestReport("TestResults_" + FDateTime::Now().ToString());
}
```

### Manual Testing

The level also supports manual testing:

- **Interactive Stations**: Players can manually trigger tests
- **Step-by-Step Mode**: Execute tests one at a time
- **Debug Visualization**: Real-time display of test progress
- **Tutorial Prompts**: Guidance for manual testing

### Continuous Integration

The test level integrates with CI/CD:

```python
# Python script for CI integration
import unreal

# Load test level
unreal.EditorLoadingAndSavingUtils.load_map('/Game/TestLevel_Comprehensive')

# Get game mode
game_mode = unreal.GameplayStatics.get_game_mode(unreal.EditorLevelLibrary.get_editor_world())

# Run all tests
game_mode.run_all_tests()

# Wait for completion
while game_mode.is_testing():
    time.sleep(1.0)

# Get results
results = game_mode.get_test_results()

# Export report
game_mode.export_test_report('CI_TestResults')

# Exit with appropriate code
sys.exit(0 if results.failed_tests == 0 else 1)
```

## Error Handling

### Test Failure Handling

```cpp
void UTestStation::HandleTestFailure(FString ErrorMessage)
{
    CurrentStatus = ETestStatus::Failed;

    // Log error
    UE_LOG(LogTestLevel, Error, TEXT("%s Test Failed: %s"),
        *StationName.ToString(), *ErrorMessage);

    // Add to results
    FTestStationResults Results = GetResults();
    Results.ErrorMessage = ErrorMessage;

    // Notify game mode
    if (ATestLevelGameMode* GameMode = Cast<ATestLevelGameMode>(GetWorld()->GetAuthGameMode()))
    {
        GameMode->OnTestStationFailed(this, ErrorMessage);
    }

    // Cleanup
    CleanupTest();
}
```

### Timeout Handling

```cpp
void UTestStation::StartTestWithTimeout(float TimeoutSeconds)
{
    // Start test
    RunTest();

    // Set timeout timer
    GetWorld()->GetTimerManager().SetTimer(
        TimeoutHandle,
        [this]()
        {
            if (CurrentStatus == ETestStatus::Running)
            {
                HandleTestFailure("Test timed out");
            }
        },
        TimeoutSeconds,
        false
    );
}
```

### Recovery Mechanisms

```cpp
void ATestLevelGameMode::RecoverFromTestFailure(UTestStation* FailedStation)
{
    // Cleanup failed test
    FailedStation->CleanupTest();

    // Reset test environment
    ResetTestEnvironment();

    // Continue with next test
    int32 FailedIndex = RegisteredTestStations.IndexOfByKey(FailedStation);
    if (FailedIndex + 1 < RegisteredTestStations.Num())
    {
        RunTestStation(RegisteredTestStations[FailedIndex + 1]->StationName);
    }
}
```

## Integration with Existing Systems

### Performance Profiler Integration

```cpp
void UTestStation::RunTest_Implementation()
{
    // Get profiler
    if (UPerformanceProfiler* Profiler = GetProfiler())
    {
        // Register test system
        Profiler->RegisterSystem(StationName.ToString(), EPerformanceCategory::Testing);

        // Begin profiling
        Profiler->BeginSystemTick(StationName.ToString());
    }

    // Run test logic
    ExecuteTestLogic();

    // End profiling
    if (UPerformanceProfiler* Profiler = GetProfiler())
    {
        Profiler->EndSystemTick(StationName.ToString());

        // Get metrics
        FFrameMetrics Metrics = Profiler->GetCurrentFrameMetrics();
        TestResults.PerformanceMetrics = ConvertToPerformanceMetrics(Metrics);
    }
}
```

### Memory Manager Integration

```cpp
void UPerformanceBenchmarkStation::TestMemoryUsage()
{
    if (UMemoryOptimizationManager* MemoryMgr = GetMemoryManager())
    {
        // Get baseline
        float BaselineMemory = MemoryMgr->GetMemoryUsageMB();

        // Run stress test
        SpawnStressTestActors();

        // Measure peak
        float PeakMemory = MemoryMgr->GetMemoryUsageMB();

        // Cleanup
        CleanupStressTestActors();

        // Force GC
        MemoryMgr->ForceGarbageCollection(true);

        // Measure after cleanup
        float AfterCleanup = MemoryMgr->GetMemoryUsageMB();

        // Validate
        bool bPassed = (PeakMemory < MaxMemoryMB) &&
                       (AfterCleanup <= BaselineMemory * 1.1f);

        AddTestResult("Memory Usage", bPassed, PeakMemory, MaxMemoryMB);
    }
}
```

### Celestial Scaling System Integration

```cpp
void UCelestialScalingTestStation::TestScaling()
{
    // Get celestial scaling components
    for (AActor* Body : TestCelestialBodies)
    {
        if (UCelestialBodyComponent* Component = Body->FindComponentByClass<UCelestialBodyComponent>())
        {
            // Test at various distances
            for (float Distance : TestDistances)
            {
                // Move player to distance
                MovePlayerToDistance(Distance);

                // Wait for scale update
                FPlatformProcess::Sleep(0.1f);

                // Measure scale factor
                float MeasuredScale = MeasureScaleFactor(Body);

                // Calculate expected scale (inverse-square law)
                float ExpectedScale = CalculateExpectedScale(Distance);

                // Validate
                float Error = FMath::Abs(MeasuredScale - ExpectedScale) / ExpectedScale;
                bool bPassed = Error < ScaleAccuracyThreshold;

                AddTestResult(
                    FString::Printf(TEXT("Scale at %f km"), Distance),
                    bPassed,
                    MeasuredScale,
                    ExpectedScale
                );
            }
        }
    }
}
```

## Visualization and UI

### Debug Visualization

The test level includes comprehensive debug visualization:

```cpp
void ATestLevelGameMode::DrawDebugInfo()
{
    if (!bShowDebugInfo) return;

    // Draw test station locations
    for (UTestStation* Station : RegisteredTestStations)
    {
        FVector Location = Station->GetOwner()->GetActorLocation();
        FColor Color = GetStatusColor(Station->CurrentStatus);

        DrawDebugSphere(GetWorld(), Location, 500.0f, 16, Color, false, -1.0f, 0, 10.0f);
        DrawDebugString(GetWorld(), Location + FVector(0, 0, 600),
            Station->StationName.ToString(), nullptr, Color, 0.0f, true);
    }

    // Draw performance metrics
    if (Profiler)
    {
        FString DebugText = FString::Printf(
            TEXT("FPS: %.1f | Frame Time: %.2fms | Memory: %.1fMB"),
            Profiler->GetCurrentFPS(),
            Profiler->GetAverageFrameTime(),
            MemoryManager->GetMemoryUsageMB()
        );

        DrawDebugString(GetWorld(), FVector(0, 0, 2000), DebugText,
            nullptr, FColor::Green, 0.0f, true, 2.0f);
    }
}
```

### UI Widgets

**Test Results Widget** (UMG):

- Real-time test progress
- Pass/fail indicators
- Performance graphs
- Error messages

**Navigation Widget** (UMG):

- Test station list
- Teleport buttons
- Test status indicators
- Quick actions

**Performance Monitor Widget** (UMG):

- FPS graph
- Memory usage graph
- CPU/GPU time
- Draw call count

## Performance Considerations

### Streaming Optimization

```cpp
void ATestLevelGameMode::OptimizeStreaming()
{
    // Only load active test zone
    for (UTestStation* Station : RegisteredTestStations)
    {
        bool bShouldLoad = (Station == CurrentTestStation);

        if (UWorldPartitionStreamingSource* StreamingSource = GetStreamingSource())
        {
            StreamingSource->SetStreamingState(
                Station->GetStreamingCell(),
                bShouldLoad ? EStreamingState::Loaded : EStreamingState::Unloaded
            );
        }
    }
}
```

### Memory Management

```cpp
void UTestStation::CleanupTest()
{
    // Destroy spawned actors
    for (AActor* Actor : SpawnedTestActors)
    {
        if (Actor && !Actor->IsPendingKill())
        {
            Actor->Destroy();
        }
    }
    SpawnedTestActors.Empty();

    // Return pooled objects
    if (UMemoryOptimizationManager* MemoryMgr = GetMemoryManager())
    {
        for (AActor* Actor : PooledTestActors)
        {
            MemoryMgr->ReturnPooledObject(Actor);
        }
    }
    PooledTestActors.Empty();

    // Force GC if needed
    if (bForceGCAfterTest)
    {
        GetMemoryManager()->ForceGarbageCollection(true);
    }
}
```

### VR Performance

```cpp
void ATestLevelGameMode::EnsureVRPerformance()
{
    // Check FPS
    float CurrentFPS = Profiler->GetCurrentFPS();
    if (CurrentFPS < 90.0f)
    {
        // Apply performance optimizations
        TickManager->ApplyPerformanceStrategy();

        // Reduce visual quality if needed
        if (CurrentFPS < 60.0f)
        {
            ApplyLowQualitySettings();
        }
    }
}
```

## Future Enhancements

1. **Automated Regression Testing**: Compare results against baseline
2. **Network Testing**: Multiplayer validation
3. **Platform-Specific Tests**: VisionOS, Android, etc.
4. **Load Testing**: Sustained performance over time
5. **Visual Regression**: Screenshot comparison
6. **Audio Testing**: Sound system validation
7. **Accessibility Testing**: VR comfort and accessibility
8. **Localization Testing**: Multi-language support

## Conclusion

The Comprehensive Test Level provides a complete testing solution for Alexander, validating all major systems in a structured, automated way. The modular design allows easy addition of new test stations, while the integration with existing optimization systems ensures accurate performance measurement.
