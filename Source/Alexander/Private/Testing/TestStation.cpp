// Copyright Epic Games, Inc. All Rights Reserved.

#include "Testing/TestStation.h"
#include "Testing/TestLevelGameMode.h"
#include "PerformanceProfiler.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UTestStation::UTestStation()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UTestStation::BeginPlay()
{
	Super::BeginPlay();

	// Get game mode reference
	if (UWorld* World = GetWorld())
	{
		GameModeRef = Cast<ATestLevelGameMode>(UGameplayStatics::GetGameMode(World));
	}

	// Find or create performance profiler
	if (bCapturePerformanceMetrics)
	{
		if (AActor* Owner = GetOwner())
		{
			PerformanceProfiler = Owner->FindComponentByClass<UPerformanceProfiler>();
			if (!PerformanceProfiler)
			{
				PerformanceProfiler = NewObject<UPerformanceProfiler>(Owner, UPerformanceProfiler::StaticClass());
				if (PerformanceProfiler)
				{
					PerformanceProfiler->RegisterComponent();
				}
			}
		}
	}

	// Register test cases
	RegisterTests();

	// Initialize results
	Results = FTestStationResults(StationName);

	// Auto-start if configured
	if (bAutoStartTests)
	{
		RunAllTests();
	}
}

void UTestStation::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bTestsRunning && CurrentTest)
	{
		// Update performance metrics
		if (bCapturePerformanceMetrics)
		{
			UpdatePerformanceMetrics();
		}

		// Check for timeout
		float CurrentTime = GetWorld()->GetTimeSeconds();
		float ElapsedTime = CurrentTime - CurrentTestStartTime;
		if (ElapsedTime > CurrentTest->TimeoutSeconds)
		{
			HandleTestTimeout();
		}
	}
}

void UTestStation::RegisterTests_Implementation()
{
	// Override in derived classes
}

void UTestStation::RunAllTests()
{
	if (bTestsRunning)
	{
		LogWarning(TEXT("Tests are already running"));
		return;
	}

	if (RegisteredTests.Num() == 0)
	{
		LogWarning(TEXT("No tests registered"));
		return;
	}

	LogInfo(FString::Printf(TEXT("Starting %d tests for station: %s"), RegisteredTests.Num(), *StationName));

	// Reset state
	bTestsRunning = true;
	bSingleTestMode = false;
	CurrentTestIndex = -1;
	Results = FTestStationResults(StationName);
	Results.StartTime = FDateTime::Now();

	// Start first test
	ExecuteNextTest();
}

void UTestStation::RunSingleTest(const FString& TestName)
{
	if (bTestsRunning)
	{
		LogWarning(TEXT("Tests are already running"));
		return;
	}

	// Find test by name
	int32 TestIndex = RegisteredTests.IndexOfByPredicate([&TestName](const FTestCase& Test)
	{
		return Test.Name.Equals(TestName, ESearchCase::IgnoreCase);
	});

	if (TestIndex == INDEX_NONE)
	{
		LogError(FString::Printf(TEXT("Test not found: %s"), *TestName));
		return;
	}

	LogInfo(FString::Printf(TEXT("Running single test: %s"), *TestName));

	// Run only this test
	bTestsRunning = true;
	bSingleTestMode = true;
	CurrentTestIndex = TestIndex - 1; // Will be incremented in ExecuteNextTest
	Results = FTestStationResults(StationName);
	Results.StartTime = FDateTime::Now();

	ExecuteNextTest();
}

void UTestStation::StopAllTests()
{
	if (!bTestsRunning)
	{
		return;
	}

	LogWarning(TEXT("Stopping all tests"));

	bTestsRunning = false;
	CurrentTest = nullptr;
	CurrentTestIndex = -1;

	// Mark current test as stopped if any
	if (Results.TestCases.Num() > 0)
	{
		FTestCaseResult& LastTest = Results.TestCases.Last();
		if (LastTest.Status == ETestStatus::Running)
		{
			LastTest.Status = ETestStatus::Failed;
			LastTest.ErrorMessage = TEXT("Test stopped by user");
			LastTest.EndTime = FDateTime::Now();
		}
	}

	Results.EndTime = FDateTime::Now();
	Results.TotalExecutionTimeSeconds = (Results.EndTime - Results.StartTime).GetTotalSeconds();
}

void UTestStation::ResetTests()
{
	StopAllTests();
	Results = FTestStationResults(StationName);
	LogInfo(TEXT("Tests reset"));
}

float UTestStation::GetTestProgress() const
{
	if (RegisteredTests.Num() == 0)
	{
		return 0.0f;
	}
	return (float)(CurrentTestIndex + 1) / RegisteredTests.Num();
}

FString UTestStation::GetCurrentTestName() const
{
	if (CurrentTest)
	{
		return CurrentTest->Name;
	}
	return TEXT("");
}

void UTestStation::RegisterTestCase(const FString& TestName, const FString& Description, TFunction<void()> TestFunction, float TimeoutSeconds)
{
	if (!TestFunction)
	{
		LogError(FString::Printf(TEXT("Cannot register test '%s': TestFunction is null"), *TestName));
		return;
	}

	FTestCase NewTest(TestName, Description, TestFunction, TimeoutSeconds > 0.0f ? TimeoutSeconds : DefaultTestTimeout);
	RegisteredTests.Add(NewTest);

	if (bVerboseLogging)
	{
		LogInfo(FString::Printf(TEXT("Registered test: %s - %s"), *TestName, *Description));
	}
}

void UTestStation::StartTest(const FString& TestName)
{
	if (CurrentTest && CurrentTest->Name == TestName)
	{
		CurrentTest->Result.Start();
		LogInfo(FString::Printf(TEXT("[START] %s"), *TestName));
		OnTestStarted.Broadcast(TestName);
	}
}

void UTestStation::PassTest(const FString& TestName, const FString& Message)
{
	if (CurrentTest && CurrentTest->Name == TestName)
	{
		CurrentTest->Result.Pass(Message);

		if (bCapturePerformanceMetrics)
		{
			CurrentTest->Result.PerformanceMetrics = StopPerformanceCapture();
		}

		LogInfo(FString::Printf(TEXT("[PASS] %s%s"), *TestName, Message.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(" - %s"), *Message)));
		OnTestCompleted.Broadcast(TestName, ETestStatus::Passed);

		CompleteCurrentTest();
	}
}

void UTestStation::FailTest(const FString& TestName, const FString& ErrorMessage, ETestSeverity Severity)
{
	if (CurrentTest && CurrentTest->Name == TestName)
	{
		CurrentTest->Result.Fail(ErrorMessage, Severity);

		if (bCapturePerformanceMetrics)
		{
			CurrentTest->Result.PerformanceMetrics = StopPerformanceCapture();
		}

		LogError(FString::Printf(TEXT("[FAIL] %s: %s"), *TestName, *ErrorMessage));
		OnTestCompleted.Broadcast(TestName, ETestStatus::Failed);
		OnTestError.Broadcast(ErrorMessage);

		CompleteCurrentTest();
	}
}

void UTestStation::SkipTest(const FString& TestName, const FString& Reason)
{
	if (CurrentTest && CurrentTest->Name == TestName)
	{
		CurrentTest->Result.Skip(Reason);
		LogWarning(FString::Printf(TEXT("[SKIP] %s%s"), *TestName, Reason.IsEmpty() ? TEXT("") : *FString::Printf(TEXT(" - %s"), *Reason)));
		OnTestCompleted.Broadcast(TestName, ETestStatus::Skipped);

		CompleteCurrentTest();
	}
}

void UTestStation::StartPerformanceCapture()
{
	FrameTimes.Empty();
	FPSValues.Empty();
	PerformanceCaptureStartTime = GetWorld()->GetTimeSeconds();
	PerformanceFrameCount = 0;
}

FTestPerformanceMetrics UTestStation::StopPerformanceCapture()
{
	FTestPerformanceMetrics Metrics;

	if (FrameTimes.Num() > 0)
	{
		float TotalFrameTime = 0.0f;
		float MinFPS = FPSValues.Num() > 0 ? FPSValues[0] : 0.0f;
		float MaxFPS = FPSValues.Num() > 0 ? FPSValues[0] : 0.0f;
		float TotalFPS = 0.0f;

		for (float FPS : FPSValues)
		{
			TotalFPS += FPS;
			MinFPS = FMath::Min(MinFPS, FPS);
			MaxFPS = FMath::Max(MaxFPS, FPS);
		}

		for (float FrameTime : FrameTimes)
		{
			TotalFrameTime += FrameTime;
		}

		Metrics.AverageFPS = FPSValues.Num() > 0 ? TotalFPS / FPSValues.Num() : 0.0f;
		Metrics.MinFPS = MinFPS;
		Metrics.MaxFPS = MaxFPS;
		Metrics.AverageFrameTime = FrameTimes.Num() > 0 ? TotalFrameTime / FrameTimes.Num() : 0.0f;
	}

	Metrics.TestDurationSeconds = GetWorld()->GetTimeSeconds() - PerformanceCaptureStartTime;

	// Get additional metrics from performance profiler if available
	if (PerformanceProfiler)
	{
		// Note: This would integrate with the actual PerformanceProfiler API
		// For now, using placeholder values
		Metrics.GameThreadTime = Metrics.AverageFrameTime * 0.6f;
		Metrics.RenderThreadTime = Metrics.AverageFrameTime * 0.7f;
		Metrics.GPUTime = Metrics.AverageFrameTime * 0.8f;
	}

	return Metrics;
}

void UTestStation::LogInfo(const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("[%s] %s"), *StationName, *Message);
}

void UTestStation::LogWarning(const FString& Message)
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] %s"), *StationName, *Message);
}

void UTestStation::LogError(const FString& Message)
{
	UE_LOG(LogTemp, Error, TEXT("[%s] %s"), *StationName, *Message);
}

bool UTestStation::CheckCondition(bool Condition, const FString& TestName, const FString& ErrorMessage)
{
	if (!Condition)
	{
		FailTest(TestName, ErrorMessage);
		return false;
	}
	return true;
}

bool UTestStation::CheckNearlyEqual(float A, float B, float Tolerance, const FString& TestName, const FString& ErrorMessage)
{
	if (!FMath::IsNearlyEqual(A, B, Tolerance))
	{
		FString DetailedError = FString::Printf(TEXT("%s (Expected: %.4f, Got: %.4f, Tolerance: %.4f)"),
			*ErrorMessage, B, A, Tolerance);
		FailTest(TestName, DetailedError);
		return false;
	}
	return true;
}

bool UTestStation::CheckNotNull(UObject* Object, const FString& TestName, const FString& ErrorMessage)
{
	if (!Object)
	{
		FailTest(TestName, ErrorMessage);
		return false;
	}
	return true;
}

void UTestStation::ExecuteNextTest()
{
	// Check if we're in single test mode
	if (bSingleTestMode && CurrentTestIndex >= 0)
	{
		// Single test complete
		bTestsRunning = false;
		Results.EndTime = FDateTime::Now();
		Results.TotalExecutionTimeSeconds = (Results.EndTime - Results.StartTime).GetTotalSeconds();
		OnAllTestsCompleted.Broadcast(Results);
		LogInfo(FString::Printf(TEXT("Single test complete: %s"), *Results.ToString()));
		return;
	}

	// Move to next test
	CurrentTestIndex++;

	// Check if all tests complete
	if (CurrentTestIndex >= RegisteredTests.Num())
	{
		bTestsRunning = false;
		Results.EndTime = FDateTime::Now();
		Results.TotalExecutionTimeSeconds = (Results.EndTime - Results.StartTime).GetTotalSeconds();
		OnAllTestsCompleted.Broadcast(Results);
		LogInfo(FString::Printf(TEXT("All tests complete: %s"), *Results.ToString()));
		return;
	}

	// Get current test
	CurrentTest = &RegisteredTests[CurrentTestIndex];
	CurrentTestStartTime = GetWorld()->GetTimeSeconds();

	// Start performance capture
	if (bCapturePerformanceMetrics)
	{
		StartPerformanceCapture();
	}

	// Start test
	StartTest(CurrentTest->Name);

	// Execute test function
	// TODO: Enable exception handling by adding /EHsc to build settings if needed
	// try
	// {
		if (CurrentTest->TestFunction)
		{
			CurrentTest->TestFunction();
		}
		else
		{
			FailTest(CurrentTest->Name, TEXT("Test function is null"), ETestSeverity::Critical);
		}
	// }
	// catch (const std::exception& e)
	// {
	// 	FailTest(CurrentTest->Name, FString::Printf(TEXT("Exception: %s"), ANSI_TO_TCHAR(e.what())), ETestSeverity::Critical);
	// }
}

void UTestStation::CompleteCurrentTest()
{
	if (CurrentTest)
	{
		// Add result to station results
		Results.AddTestCase(CurrentTest->Result);

		// Check if we should stop on failure
		if (bStopOnFirstFailure && CurrentTest->Result.IsFailed())
		{
			LogError(TEXT("Stopping tests due to failure (bStopOnFirstFailure = true)"));
			StopAllTests();
			return;
		}

		// Execute next test
		ExecuteNextTest();
	}
}

void UTestStation::HandleTestTimeout()
{
	if (CurrentTest)
	{
		LogError(FString::Printf(TEXT("Test timed out after %.1f seconds: %s"),
			CurrentTest->TimeoutSeconds, *CurrentTest->Name));

		CurrentTest->Result.Timeout(CurrentTest->TimeoutSeconds);

		if (bCapturePerformanceMetrics)
		{
			CurrentTest->Result.PerformanceMetrics = StopPerformanceCapture();
		}

		OnTestCompleted.Broadcast(CurrentTest->Name, ETestStatus::Timeout);
		OnTestError.Broadcast(FString::Printf(TEXT("Timeout: %s"), *CurrentTest->Name));

		CompleteCurrentTest();
	}
}

void UTestStation::UpdatePerformanceMetrics()
{
	if (!GetWorld())
	{
		return;
	}

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	if (DeltaTime > 0.0f)
	{
		float CurrentFPS = 1.0f / DeltaTime;
		float CurrentFrameTime = DeltaTime * 1000.0f; // Convert to ms

		FPSValues.Add(CurrentFPS);
		FrameTimes.Add(CurrentFrameTime);
		PerformanceFrameCount++;
	}
}
