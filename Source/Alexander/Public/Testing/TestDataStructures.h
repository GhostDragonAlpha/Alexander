// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TestDataStructures.generated.h"

/**
 * Test result status enumeration
 */
// Forward declaration for test status enum
enum class EAlexanderTestStatus : uint8;

/**
 * Test severity for errors and warnings
 */
UENUM(BlueprintType)
enum class ETestSeverity : uint8
{
	Info UMETA(DisplayName = "Info"),
	Warning UMETA(DisplayName = "Warning"),
	Error UMETA(DisplayName = "Error"),
	Critical UMETA(DisplayName = "Critical")
};

/**
 * Performance metrics captured during tests
 */
USTRUCT(BlueprintType)
struct FTestPerformanceMetrics
{
	GENERATED_BODY()

	// Frame metrics
	UPROPERTY(BlueprintReadWrite, Category = "Performance")
	float AverageFPS = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Performance")
	float MinFPS = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Performance")
	float MaxFPS = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Performance")
	float AverageFrameTime = 0.0f;

	// Thread times (ms)
	UPROPERTY(BlueprintReadWrite, Category = "Performance")
	float GameThreadTime = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Performance")
	float RenderThreadTime = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Performance")
	float GPUTime = 0.0f;

	// Memory metrics (MB)
	UPROPERTY(BlueprintReadWrite, Category = "Performance")
	float MemoryUsageMB = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Performance")
	float PeakMemoryMB = 0.0f;

	// Rendering metrics
	UPROPERTY(BlueprintReadWrite, Category = "Performance")
	int32 DrawCalls = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Performance")
	int32 TriangleCount = 0;

	// VR-specific metrics
	UPROPERTY(BlueprintReadWrite, Category = "Performance|VR")
	float MotionToPhotonLatency = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Performance|VR")
	int32 DroppedFrames = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Performance|VR")
	float ReprojectionRatio = 0.0f;

	// Test duration
	UPROPERTY(BlueprintReadWrite, Category = "Performance")
	float TestDurationSeconds = 0.0f;

	FTestPerformanceMetrics()
		: AverageFPS(0.0f), MinFPS(0.0f), MaxFPS(0.0f), AverageFrameTime(0.0f),
		  GameThreadTime(0.0f), RenderThreadTime(0.0f), GPUTime(0.0f),
		  MemoryUsageMB(0.0f), PeakMemoryMB(0.0f), DrawCalls(0), TriangleCount(0),
		  MotionToPhotonLatency(0.0f), DroppedFrames(0), ReprojectionRatio(0.0f),
		  TestDurationSeconds(0.0f)
	{}

	// Check if performance meets VR targets (90+ FPS)
	bool MeetsVRTargets() const
	{
		return AverageFPS >= 90.0f && MinFPS >= 80.0f && DroppedFrames == 0;
	}

	// Generate human-readable summary
	FString ToString() const
	{
		return FString::Printf(TEXT("FPS: %.1f (min: %.1f, max: %.1f), Frame Time: %.2fms, Memory: %.1fMB, Draw Calls: %d"),
			AverageFPS, MinFPS, MaxFPS, AverageFrameTime, MemoryUsageMB, DrawCalls);
	}
};

/**
 * Individual test case result
 */
USTRUCT(BlueprintType)
struct FTestCaseResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	FString TestName;

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	EAlexanderTestStatus Status = EAlexanderTestStatus::NotRun;

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	FString Description;

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	ETestSeverity Severity = ETestSeverity::Info;

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	float ExecutionTimeSeconds = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	FDateTime StartTime;

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	FDateTime EndTime;

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	FTestPerformanceMetrics PerformanceMetrics;

	// Custom validation data (key-value pairs)
	UPROPERTY(BlueprintReadWrite, Category = "Test")
	TMap<FString, FString> CustomData;

	FTestCaseResult()
		: TestName(TEXT("")), Status(ETestStatus::NotRun), Description(TEXT("")),
		  ErrorMessage(TEXT("")), Severity(ETestSeverity::Info), ExecutionTimeSeconds(0.0f)
	{}

	explicit FTestCaseResult(const FString& InTestName)
		: TestName(InTestName), Status(ETestStatus::NotRun), Description(TEXT("")),
		  ErrorMessage(TEXT("")), Severity(ETestSeverity::Info), ExecutionTimeSeconds(0.0f)
	{}

	// Mark test as started
	void Start()
	{
		Status = EAlexanderTestStatus::Running;
		StartTime = FDateTime::Now();
	}

	// Mark test as passed
	void Pass(const FString& Message = TEXT(""))
	{
		Status = EAlexanderTestStatus::Passed;
		EndTime = FDateTime::Now();
		ExecutionTimeSeconds = (EndTime - StartTime).GetTotalSeconds();
		if (!Message.IsEmpty())
		{
			Description = Message;
		}
	}

	// Mark test as failed
	void Fail(const FString& Error, ETestSeverity InSeverity = ETestSeverity::Error)
	{
		Status = EAlexanderTestStatus::Failed;
		EndTime = FDateTime::Now();
		ExecutionTimeSeconds = (EndTime - StartTime).GetTotalSeconds();
		ErrorMessage = Error;
		Severity = InSeverity;
	}

	// Mark test as timeout
	void Timeout(float TimeoutSeconds)
	{
		Status = EAlexanderTestStatus::Timeout;
		EndTime = FDateTime::Now();
		ExecutionTimeSeconds = TimeoutSeconds;
		ErrorMessage = FString::Printf(TEXT("Test exceeded timeout of %.1f seconds"), TimeoutSeconds);
		Severity = ETestSeverity::Error;
	}

	// Mark test as skipped
	void Skip(const FString& Reason = TEXT(""))
	{
		Status = EAlexanderTestStatus::Skipped;
		ErrorMessage = Reason;
	}

	// Check if test passed
	bool IsPassed() const { return Status == EAlexanderTestStatus::Passed; }
	bool IsFailed() const { return Status == EAlexanderTestStatus::Failed; }
	bool IsRunning() const { return Status == EAlexanderTestStatus::Running; }

	// Generate summary string
	FString ToString() const
	{
		FString StatusStr = UEnum::GetValueAsString(Status);
		if (Status == ETestStatus::Passed)
		{
			return FString::Printf(TEXT("[PASS] %s (%.2fs)"), *TestName, ExecutionTimeSeconds);
		}
		else if (Status == ETestStatus::Failed)
		{
			return FString::Printf(TEXT("[FAIL] %s: %s"), *TestName, *ErrorMessage);
		}
		else
		{
			return FString::Printf(TEXT("[%s] %s"), *StatusStr, *TestName);
		}
	}
};

/**
 * Results for an entire test station
 */
USTRUCT(BlueprintType)
struct FTestStationResults
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	FString StationName;

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	TArray<FTestCaseResult> TestCases;

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	FDateTime StartTime;

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	FDateTime EndTime;

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	float TotalExecutionTimeSeconds = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	FTestPerformanceMetrics AggregatedPerformanceMetrics;

	FTestStationResults()
		: StationName(TEXT("")), TotalExecutionTimeSeconds(0.0f)
	{}

	explicit FTestStationResults(const FString& InStationName)
		: StationName(InStationName), TotalExecutionTimeSeconds(0.0f)
	{}

	// Add a test case
	void AddTestCase(const FTestCaseResult& TestCase)
	{
		TestCases.Add(TestCase);
	}

	// Get statistics
	int32 GetTotalTests() const { return TestCases.Num(); }

	int32 GetPassedTests() const
	{
		return TestCases.FilterByPredicate([](const FTestCaseResult& Test) { return Test.IsPassed(); }).Num();
	}

	int32 GetFailedTests() const
	{
		return TestCases.FilterByPredicate([](const FTestCaseResult& Test) { return Test.IsFailed(); }).Num();
	}

	int32 GetSkippedTests() const
	{
		return TestCases.FilterByPredicate([](const FTestCaseResult& Test) { return Test.Status == EAlexanderTestStatus::Skipped; }).Num();
	}

	float GetPassRate() const
	{
		int32 Total = GetTotalTests();
		return Total > 0 ? (float)GetPassedTests() / Total * 100.0f : 0.0f;
	}

	bool AllTestsPassed() const
	{
		return GetTotalTests() > 0 && GetFailedTests() == 0;
	}

	// Generate summary
	FString ToString() const
	{
		return FString::Printf(TEXT("%s: %d/%d passed (%.1f%%), %.2fs"),
			*StationName, GetPassedTests(), GetTotalTests(), GetPassRate(), TotalExecutionTimeSeconds);
	}
};

/**
 * Complete test results for all stations
 */
USTRUCT(BlueprintType)
struct FTestResults
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	TArray<FTestStationResults> StationResults;

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	FDateTime TestRunStartTime;

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	FDateTime TestRunEndTime;

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	float TotalExecutionTimeSeconds = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	FString Platform;

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	FString BuildConfiguration;

	UPROPERTY(BlueprintReadWrite, Category = "Test")
	FString EngineVersion;

	FTestResults()
		: TotalExecutionTimeSeconds(0.0f), Platform(TEXT("")),
		  BuildConfiguration(TEXT("")), EngineVersion(TEXT(""))
	{}

	// Add station results
	void AddStationResults(const FTestStationResults& Results)
	{
		StationResults.Add(Results);
	}

	// Get overall statistics
	int32 GetTotalTests() const
	{
		int32 Total = 0;
		for (const FTestStationResults& Station : StationResults)
		{
			Total += Station.GetTotalTests();
		}
		return Total;
	}

	int32 GetPassedTests() const
	{
		int32 Passed = 0;
		for (const FTestStationResults& Station : StationResults)
		{
			Passed += Station.GetPassedTests();
		}
		return Passed;
	}

	int32 GetFailedTests() const
	{
		int32 Failed = 0;
		for (const FTestStationResults& Station : StationResults)
		{
			Failed += Station.GetFailedTests();
		}
		return Failed;
	}

	float GetOverallPassRate() const
	{
		int32 Total = GetTotalTests();
		return Total > 0 ? (float)GetPassedTests() / Total * 100.0f : 0.0f;
	}

	bool AllTestsPassed() const
	{
		for (const FTestStationResults& Station : StationResults)
		{
			if (!Station.AllTestsPassed())
			{
				return false;
			}
		}
		return GetTotalTests() > 0;
	}

	// Export to JSON string
	FString ToJSON() const
	{
		FString JSON = TEXT("{\n");
		JSON += FString::Printf(TEXT("  \"platform\": \"%s\",\n"), *Platform);
		JSON += FString::Printf(TEXT("  \"buildConfiguration\": \"%s\",\n"), *BuildConfiguration);
		JSON += FString::Printf(TEXT("  \"engineVersion\": \"%s\",\n"), *EngineVersion);
		JSON += FString::Printf(TEXT("  \"startTime\": \"%s\",\n"), *TestRunStartTime.ToString());
		JSON += FString::Printf(TEXT("  \"endTime\": \"%s\",\n"), *TestRunEndTime.ToString());
		JSON += FString::Printf(TEXT("  \"totalExecutionTime\": %.2f,\n"), TotalExecutionTimeSeconds);
		JSON += FString::Printf(TEXT("  \"totalTests\": %d,\n"), GetTotalTests());
		JSON += FString::Printf(TEXT("  \"passedTests\": %d,\n"), GetPassedTests());
		JSON += FString::Printf(TEXT("  \"failedTests\": %d,\n"), GetFailedTests());
		JSON += FString::Printf(TEXT("  \"passRate\": %.2f,\n"), GetOverallPassRate());
		JSON += TEXT("  \"stations\": [\n");

		for (int32 i = 0; i < StationResults.Num(); ++i)
		{
			const FTestStationResults& Station = StationResults[i];
			JSON += TEXT("    {\n");
			JSON += FString::Printf(TEXT("      \"name\": \"%s\",\n"), *Station.StationName);
			JSON += FString::Printf(TEXT("      \"totalTests\": %d,\n"), Station.GetTotalTests());
			JSON += FString::Printf(TEXT("      \"passedTests\": %d,\n"), Station.GetPassedTests());
			JSON += FString::Printf(TEXT("      \"failedTests\": %d,\n"), Station.GetFailedTests());
			JSON += FString::Printf(TEXT("      \"passRate\": %.2f\n"), Station.GetPassRate());
			JSON += TEXT("    }");
			if (i < StationResults.Num() - 1)
			{
				JSON += TEXT(",");
			}
			JSON += TEXT("\n");
		}

		JSON += TEXT("  ]\n");
		JSON += TEXT("}\n");
		return JSON;
	}

	// Export to CSV string
	FString ToCSV() const
	{
		FString CSV = TEXT("Station,Test Name,Status,Duration (s),Error Message\n");
		for (const FTestStationResults& Station : StationResults)
		{
			for (const FTestCaseResult& TestCase : Station.TestCases)
			{
				FString StatusStr = UEnum::GetValueAsString(TestCase.Status);
				FString ErrorMsg = TestCase.ErrorMessage.Replace(TEXT(","), TEXT(";"));
				CSV += FString::Printf(TEXT("%s,%s,%s,%.2f,%s\n"),
					*Station.StationName, *TestCase.TestName, *StatusStr,
					TestCase.ExecutionTimeSeconds, *ErrorMsg);
			}
		}
		return CSV;
	}

	// Generate summary report
	FString ToString() const
	{
		FString Report = TEXT("=== COMPREHENSIVE TEST RESULTS ===\n\n");
		Report += FString::Printf(TEXT("Platform: %s\n"), *Platform);
		Report += FString::Printf(TEXT("Build: %s\n"), *BuildConfiguration);
		Report += FString::Printf(TEXT("Engine: %s\n\n"), *EngineVersion);
		Report += FString::Printf(TEXT("Total Tests: %d\n"), GetTotalTests());
		Report += FString::Printf(TEXT("Passed: %d\n"), GetPassedTests());
		Report += FString::Printf(TEXT("Failed: %d\n"), GetFailedTests());
		Report += FString::Printf(TEXT("Pass Rate: %.1f%%\n"), GetOverallPassRate());
		Report += FString::Printf(TEXT("Total Time: %.2fs\n\n"), TotalExecutionTimeSeconds);

		Report += TEXT("=== STATION RESULTS ===\n\n");
		for (const FTestStationResults& Station : StationResults)
		{
			Report += FString::Printf(TEXT("%s\n"), *Station.ToString());
		}

		return Report;
	}
};
