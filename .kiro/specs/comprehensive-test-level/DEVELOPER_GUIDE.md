# Comprehensive Test Level - Developer Guide

## Overview

The Comprehensive Test Level is the definitive testing environment for validating all Alexander systems. This guide covers everything developers need to know about adding tests, modifying existing test stations, and integrating with the automated testing infrastructure.

The test level validates:

- **VR Interactions**: Tracking, hand gestures, UI manipulation
- **Spaceship Flight**: 6DOF physics, navigation, cockpit controls
- **Celestial Scaling**: Dynamic planet scaling, gravitational physics, VR precision
- **Planet Landing**: Atmospheric entry, landing guidance, terrain transitions
- **Farming Systems**: Planting, growth, watering, harvesting mechanics
- **AI Vision Integration**: Screenshot capture, scene analysis, code generation
- **Performance**: Frame rate, memory usage, CPU/GPU profiling, stress testing

## Test Level Architecture

### File Structure

```
TestLevel_Comprehensive/
├── Content/TestLevels/
│   ├── TestLevel_Comprehensive.umap          # Main level map
│   ├── Blueprints/
│   │   ├── BP_TestLevelGameMode.uasset
│   │   ├── BP_VRTestStation.uasset
│   │   ├── BP_SpaceshipFlightTestStation.uasset
│   │   ├── BP_CelestialScalingTestStation.uasset
│   │   ├── BP_PlanetLandingTestStation.uasset
│   │   ├── BP_FarmingTestStation.uasset
│   │   ├── BP_AIVisionTestStation.uasset
│   │   └── BP_PerformanceBenchmarkStation.uasset
│   └── UI/
│       ├── WBP_TestResults.uasset
│       ├── WBP_Navigation.uasset
│       └── WBP_PerformanceMonitor.uasset
├── Source/Alexander/Public/
│   ├── TestLevel/
│   │   ├── TestLevelGameMode.h
│   │   ├── TestStation.h
│   │   ├── VRTestStation.h
│   │   ├── SpaceshipFlightTestStation.h
│   │   ├── CelestialScalingTestStation.h
│   │   ├── PlanetLandingTestStation.h
│   │   ├── FarmingTestStation.h
│   │   ├── AIVisionTestStation.h
│   │   ├── PerformanceBenchmarkStation.h
│   │   ├── TestStructures.h
│   │   └── TestConstants.h
└── Source/Alexander/Private/
    └── TestLevel/
        ├── TestLevelGameMode.cpp
        ├── TestStation.cpp
        ├── [test station implementations]
        └── Tests/
            ├── TestLevel_Tests.cpp
            └── [system-specific tests]
```

### Core Classes

#### TestStation (Base Class)

```cpp
UCLASS(Blueprintable, Abstract)
class ALEXANDER_API UTestStation : public UActorComponent
{
    GENERATED_BODY()

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Station")
    FName StationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Station")
    FText StationDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Station")
    ETestCategory Category;

    // Status
    UPROPERTY(BlueprintReadOnly, Category = "Test Station")
    ETestStatus CurrentStatus;

    // Test Cases
    UPROPERTY(EditAnywhere, Category = "Test Station")
    TArray<FTestCase> TestCases;

    // Execution
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

#### Test Lifecycle

Every test station follows this lifecycle:

1. **Initialize** - Setup test environment, allocate resources
2. **Run** - Execute test logic
3. **Validate** - Verify results against expectations
4. **Cleanup** - Release resources, reset state

```cpp
void UTestStation::RunTestLifecycle()
{
    // Phase 1: Initialize
    InitializeTest();

    if (CurrentStatus == ETestStatus::Failed)
    {
        CleanupTest();
        return;
    }

    // Phase 2: Run
    RunTest();

    // Phase 3: Validate
    ValidateTest();

    // Phase 4: Cleanup (regardless of result)
    CleanupTest();
}
```

## Adding New Test Stations

### Step 1: Create Header File

Create `Source/Alexander/Public/TestLevel/YourTestStation.h`:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "TestStation.h"
#include "YourTestStation.generated.h"

/**
 * UYourTestStation - Validates [system you're testing]
 */
UCLASS(Blueprintable)
class ALEXANDER_API UYourTestStation : public UTestStation
{
    GENERATED_BODY()

public:
    // Configuration
    UPROPERTY(EditAnywhere, Category = "Your System Testing")
    float TestDuration = 30.0f;

    UPROPERTY(EditAnywhere, Category = "Your System Testing")
    bool bVerboseLogging = true;

    // Test Execution
    virtual void InitializeTest_Implementation() override;
    virtual void RunTest_Implementation() override;
    virtual void ValidateTest_Implementation() override;
    virtual void CleanupTest_Implementation() override;

private:
    // Helper methods
    void TestFeature1();
    void TestFeature2();
    void TestFeature3();

    // State
    UPROPERTY()
    AActor* TestActor;

    TArray<FTestResult> LocalTestResults;
};
```

### Step 2: Create Implementation File

Create `Source/Alexander/Private/TestLevel/YourTestStation.cpp`:

```cpp
#include "TestLevel/YourTestStation.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UYourTestStation::InitializeTest_Implementation()
{
    StationName = FName("YourTestStation");
    StationDescription = FText::FromString("Description of what you're testing");
    Category = ETestCategory::Custom;

    CurrentStatus = ETestStatus::Running;

    if (bVerboseLogging)
    {
        UE_LOG(LogTestLevel, Log, TEXT("YourTestStation: Initializing..."));
    }

    // Setup test environment
    // Spawn test actors, configure systems, etc.
}

void UYourTestStation::RunTest_Implementation()
{
    if (bVerboseLogging)
    {
        UE_LOG(LogTestLevel, Log, TEXT("YourTestStation: Running tests..."));
    }

    TestFeature1();
    TestFeature2();
    TestFeature3();

    // Set timeout
    FTimerHandle TimeoutHandle;
    GetWorld()->GetTimerManager().SetTimer(
        TimeoutHandle,
        [this]()
        {
            if (CurrentStatus == ETestStatus::Running)
            {
                CurrentStatus = ETestStatus::Failed;
                AddTestResult("Timeout", false, "Test exceeded time limit");
            }
        },
        TestDuration,
        false
    );
}

void UYourTestStation::ValidateTest_Implementation()
{
    if (bVerboseLogging)
    {
        UE_LOG(LogTestLevel, Log, TEXT("YourTestStation: Validating results..."));
    }

    // Check results
    int32 Passed = 0;
    int32 Failed = 0;

    for (const FTestResult& Result : LocalTestResults)
    {
        if (Result.bPassed)
        {
            Passed++;
        }
        else
        {
            Failed++;
        }
    }

    // Update station results
    CurrentStatus = (Failed == 0) ? ETestStatus::Passed : ETestStatus::Failed;
}

void UYourTestStation::CleanupTest_Implementation()
{
    if (bVerboseLogging)
    {
        UE_LOG(LogTestLevel, Log, TEXT("YourTestStation: Cleaning up..."));
    }

    // Destroy test actors
    if (TestActor && !TestActor->IsPendingKill())
    {
        TestActor->Destroy();
        TestActor = nullptr;
    }

    // Release resources
    LocalTestResults.Empty();
}

void UYourTestStation::TestFeature1()
{
    if (bVerboseLogging)
    {
        UE_LOG(LogTestLevel, Log, TEXT("YourTestStation: Testing Feature 1..."));
    }

    // Test logic here
    bool bResult = CheckFeature1();

    AddTestResult("Feature1Test", bResult, bResult ? "Success" : "Failed");
}

void UYourTestStation::TestFeature2()
{
    // Similar structure
}

void UYourTestStation::TestFeature3()
{
    // Similar structure
}
```

### Step 3: Register with Game Mode

In `TestLevelGameMode.cpp`:

```cpp
void ATestLevelGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Register all test stations
    RegisterTestStation(FindComponentByClass<UVRTestStation>());
    RegisterTestStation(FindComponentByClass<USpaceshipFlightTestStation>());
    RegisterTestStation(FindComponentByClass<UCelestialScalingTestStation>());
    RegisterTestStation(FindComponentByClass<UYourTestStation>());  // Add here
    // ... other stations
}

void ATestLevelGameMode::RegisterTestStation(UTestStation* Station)
{
    if (Station)
    {
        RegisteredTestStations.Add(Station);
        UE_LOG(LogTestLevel, Log, TEXT("Registered test station: %s"),
            *Station->StationName.ToString());
    }
}
```

### Step 4: Create Blueprint

1. In Unreal Editor, create a Blueprint derived from your test station class
2. Name it `BP_YourTestStation`
3. Place it in the level in the appropriate test zone
4. Configure properties in the Details panel

## Writing Test Cases

### Basic Test Structure

```cpp
void UYourTestStation::TestSomething()
{
    // Arrange - Setup test data
    float ExpectedValue = 42.0f;
    FVector TestVector = FVector(1, 2, 3);

    // Act - Execute code being tested
    float ActualValue = YourSystem->CalculateSomething(TestVector);

    // Assert - Verify results
    bool bPassed = FMath::IsNearlyEqual(ActualValue, ExpectedValue, 0.01f);
    AddTestResult("Calculation", bPassed,
        FString::Printf(TEXT("Expected: %f, Got: %f"), ExpectedValue, ActualValue));
}
```

### Helper Function

```cpp
void UTestStation::AddTestResult(
    const FString& TestName,
    bool bPassed,
    const FString& Message = "")
{
    FTestResult Result;
    Result.TestName = TestName;
    Result.bPassed = bPassed;
    Result.Details = Message;

    TestResults.Add(Result);

    // Log immediately
    if (bPassed)
    {
        UE_LOG(LogTestLevel, Log, TEXT("[PASS] %s: %s"), *TestName, *Message);
    }
    else
    {
        UE_LOG(LogTestLevel, Error, TEXT("[FAIL] %s: %s"), *TestName, *Message);
    }
}
```

## Performance Metrics Integration

### Capturing Metrics

```cpp
void UYourTestStation::RunTest_Implementation()
{
    // Get performance profiler
    UPerformanceProfiler* Profiler = GetPerformanceProfiler();
    if (!Profiler) return;

    // Begin profiling
    Profiler->BeginSystemTick(StationName.ToString());

    // Run actual test
    ExecuteTestLogic();

    // End profiling
    Profiler->EndSystemTick(StationName.ToString());

    // Get metrics
    FFrameMetrics Metrics = Profiler->GetCurrentFrameMetrics();

    // Store metrics
    StationResults.PerformanceMetrics.Add(
        TEXT("AverageFPS"), FString::FromInt((int32)Metrics.AverageFPS));
    StationResults.PerformanceMetrics.Add(
        TEXT("FrameTimeMs"), FString::SanitizeFloat(Metrics.AverageFrameTimeMs));
    StationResults.PerformanceMetrics.Add(
        TEXT("MemoryMB"), FString::SanitizeFloat(Metrics.MemoryUsageMB));
}
```

### Defining Performance Thresholds

```cpp
void UYourTestStation::ValidatePerformance()
{
    const float MinimumFPS = 90.0f;
    const float MaxFrameTimeMs = 11.1f;  // ~90 FPS
    const float MaxMemoryMB = 3072.0f;   // 3GB

    FFrameMetrics Metrics = GetPerformanceProfiler()->GetCurrentFrameMetrics();

    AddTestResult("FPS Threshold",
        Metrics.AverageFPS >= MinimumFPS,
        FString::Printf(TEXT("FPS: %.1f (Target: %.1f)"),
            Metrics.AverageFPS, MinimumFPS));

    AddTestResult("Frame Time Threshold",
        Metrics.AverageFrameTimeMs <= MaxFrameTimeMs,
        FString::Printf(TEXT("Frame Time: %.2fms (Max: %.2fms)"),
            Metrics.AverageFrameTimeMs, MaxFrameTimeMs));

    AddTestResult("Memory Threshold",
        Metrics.MemoryUsageMB <= MaxMemoryMB,
        FString::Printf(TEXT("Memory: %.1fMB (Max: %.1fMB)"),
            Metrics.MemoryUsageMB, MaxMemoryMB));
}
```

## Debugging Test Failures

### Debug Visualization

Enable debug drawing to visualize test progress:

```cpp
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT

void UYourTestStation::DrawDebugInfo()
{
    if (!bShowDebugInfo || !GetWorld()) return;

    // Draw test station location
    FVector Location = GetOwner()->GetActorLocation();
    FColor Color = (CurrentStatus == ETestStatus::Passed) ? FColor::Green : FColor::Red;

    DrawDebugSphere(GetWorld(), Location, 500.0f, 16, Color, false, -1.0f, 0, 10.0f);
    DrawDebugString(GetWorld(), Location + FVector(0, 0, 600),
        StationName.ToString(), nullptr, Color, 0.0f, true);
}

#endif
```

### Console Commands

```cpp
// In your game mode
void ATestLevelGameMode::SetupConsoleCommands()
{
    // Run specific test
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("test.run"),
        TEXT("Run specific test station"),
        FConsoleCommandWithArgsDelegate::CreateRaw(this, &ATestLevelGameMode::OnTestCommand),
        ECVF_Default);

    // Show debug info
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("test.debug"),
        TEXT("Toggle test debug visualization"),
        FConsoleCommandDelegate::CreateRaw(this, &ATestLevelGameMode::OnDebugCommand),
        ECVF_Default);
}

void ATestLevelGameMode::OnTestCommand(const TArray<FString>& Args)
{
    if (Args.Num() == 0) return;

    FName StationName(*Args[0]);
    RunTestStation(StationName);
}
```

### Logging and Output

```cpp
void UYourTestStation::RunTest_Implementation()
{
    // Use appropriate log categories
    UE_LOG(LogTestLevel, Log, TEXT("Starting test..."));

    // For warnings and errors
    UE_LOG(LogTestLevel, Warning, TEXT("Unusual condition detected"));
    UE_LOG(LogTestLevel, Error, TEXT("Test failed: %s"), *ErrorMessage);

    // For verbose debugging
    if (bVerboseLogging)
    {
        UE_LOG(LogTestLevel, Verbose, TEXT("Detailed debug info: %f"), SomeValue);
    }
}
```

## Test Categories

### VR Tests

When creating VR tests, ensure:

- VR mode initialization check
- Hand tracking validation
- Input response time measurement (<16ms target)
- Haptic feedback verification
- Comfort validation (no judder, smooth locomotion)

Example:

```cpp
void UVRTestStation::TestHandTracking()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    // Get VR tracking data
    FVector LeftHandPos = GetVRHand(EControllerHand::Left)->GetComponentLocation();
    FVector RightHandPos = GetVRHand(EControllerHand::Right)->GetComponentLocation();

    // Verify tracking consistency
    // Measure accuracy against known positions
}
```

### Physics Tests

For physics-based tests:

- Verify deterministic behavior
- Test with and without physics
- Measure precision over time
- Check for instabilities

Example:

```cpp
void USpaceshipFlightTestStation::TestPhysics()
{
    // Get initial conditions
    FVector InitialVelocity = Spaceship->GetVelocity();
    FVector InitialLocation = Spaceship->GetActorLocation();

    // Apply force
    Spaceship->AddForce(TestForce);

    // Measure results
    FVector FinalVelocity = Spaceship->GetVelocity();
    float DeltaV = FVector::Dist(FinalVelocity, InitialVelocity);

    // Validate physics
    bool bPhysicsCorrect = IsPhysicsAccurate(DeltaV);
    AddTestResult("PhysicsForce", bPhysicsCorrect);
}
```

### Performance Tests

For performance-focused tests:

- Measure baseline performance
- Run stress scenarios
- Compare to thresholds
- Detect regressions

Example:

```cpp
void UPerformanceBenchmarkStation::TestFrameRate()
{
    // Get profiler
    UPerformanceProfiler* Profiler = GetPerformanceProfiler();
    if (!Profiler) return;

    const float BenchmarkDuration = 60.0f;
    float StartTime = GetWorld()->GetTimeSeconds();

    while ((GetWorld()->GetTimeSeconds() - StartTime) < BenchmarkDuration)
    {
        // Let the game run, profiler collects data
    }

    FFrameMetrics Metrics = Profiler->GetCurrentFrameMetrics();

    AddTestResult("AverageFPS",
        Metrics.AverageFPS >= 90.0f,
        FString::Printf(TEXT("Average FPS: %.1f"), Metrics.AverageFPS));
}
```

## Testing Checklist

Before submitting test station code:

- [ ] All test cases pass locally
- [ ] Code compiles without warnings
- [ ] Performance metrics are captured
- [ ] Debug visualization works
- [ ] Error handling is robust
- [ ] Logging is appropriate
- [ ] Cleanup properly releases resources
- [ ] Blueprint is created and configured
- [ ] Documentation is complete
- [ ] CI/CD pipeline passes

## Common Issues and Solutions

### Issue: Tests Pass Locally but Fail in CI

**Solution**: Ensure all dependencies are initialized in BeginPlay, not in constructors.

```cpp
// Wrong - Constructor dependency
AMyActor() {
    System = NewObject<UMySystem>();
}

// Correct - PostInitProperties or BeginPlay
virtual void BeginPlay() override {
    System = NewObject<UMySystem>();
    System->Initialize();
}
```

### Issue: Flaky Tests That Intermittently Fail

**Solution**: Add synchronization points and wait for async operations:

```cpp
// Add delay for async completion
bool bWaitComplete = false;
GetWorld()->GetTimerManager().SetTimer(
    AsyncHandle,
    [&bWaitComplete]() { bWaitComplete = true; },
    0.5f,
    false
);

while (!bWaitComplete)
{
    GetWorld()->Tick(LEVELTICK_All, 0.016f);
}
```

### Issue: Memory Leaks Detected During Tests

**Solution**: Ensure proper cleanup in CleanupTest_Implementation:

```cpp
void UYourTestStation::CleanupTest_Implementation()
{
    // Destroy all spawned actors
    for (AActor* Actor : SpawnedActors)
    {
        if (Actor && !Actor->IsPendingKill())
        {
            Actor->Destroy();
        }
    }
    SpawnedActors.Empty();

    // Clear arrays
    TestData.Empty();
    TestResults.Empty();

    // Force garbage collection if needed
    if (GEngine)
    {
        GEngine->ForceGarbageCollection(true);
    }
}
```

## Python Test Runner Integration

The Python test runner (`run_comprehensive_test_level.py`) can automatically discover and execute test stations:

```python
# Run all tests
python run_comprehensive_test_level.py

# Run specific station
python run_comprehensive_test_level.py --station VRTestStation

# Run with performance regression checking
python run_comprehensive_test_level.py --check-regression --report-format html
```

## Next Steps

1. Review the [QA_GUIDE.md](QA_GUIDE.md) for manual testing procedures
2. Check [CI_CD_INTEGRATION.md](CI_CD_INTEGRATION.md) for automation setup
3. Examine existing test stations in the code for patterns and examples
4. Test your station thoroughly before committing

## Resources

- **Unreal Engine Testing Framework**: https://docs.unrealengine.com/5.6/en-US/automation-system-in-unreal-engine/
- **Performance Profiler Documentation**: See `Source/Alexander/Public/Performance/PerformanceProfiler.h`
- **Test Results Format**: See `Source/Alexander/Public/TestLevel/TestStructures.h`
