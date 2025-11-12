// Copyright Epic Games, Inc. All Rights Reserved.

#include "Testing/TestUtilities.h"
#include "TestScenarioManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
// #include "HeadMountedDisplayFunctionLibrary.h" // UE5.6: Moved or deprecated
#include "IXRTrackingSystem.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformTime.h"

// ===== Test Assertions =====

bool UTestUtilities::AssertTrue(bool Condition, const FString& Message)
{
	if (!Condition)
	{
		UE_LOG(LogTemp, Error, TEXT("[ASSERT FAILED] %s"), *Message);
	}
	return Condition;
}

bool UTestUtilities::AssertFalse(bool Condition, const FString& Message)
{
	return AssertTrue(!Condition, Message);
}

bool UTestUtilities::AssertIsValid(UObject* Object, const FString& Message)
{
	if (!IsValid(Object))
	{
		UE_LOG(LogTemp, Error, TEXT("[ASSERT FAILED] %s"), *Message);
		return false;
	}
	return true;
}

bool UTestUtilities::AssertNearlyEqual(float A, float B, float Tolerance, const FString& Message)
{
	if (!FMath::IsNearlyEqual(A, B, Tolerance))
	{
		UE_LOG(LogTemp, Error, TEXT("[ASSERT FAILED] %s (A: %.4f, B: %.4f, Tolerance: %.4f)"),
			*Message, A, B, Tolerance);
		return false;
	}
	return true;
}

bool UTestUtilities::AssertVectorsNearlyEqual(const FVector& A, const FVector& B, float Tolerance, const FString& Message)
{
	if (!A.Equals(B, Tolerance))
	{
		UE_LOG(LogTemp, Error, TEXT("[ASSERT FAILED] %s (A: %s, B: %s, Tolerance: %.4f)"),
			*Message, *A.ToString(), *B.ToString(), Tolerance);
		return false;
	}
	return true;
}

bool UTestUtilities::AssertInRange(float Value, float Min, float Max, const FString& Message)
{
	if (Value < Min || Value > Max)
	{
		UE_LOG(LogTemp, Error, TEXT("[ASSERT FAILED] %s (Value: %.4f, Range: [%.4f, %.4f])"),
			*Message, Value, Min, Max);
		return false;
	}
	return true;
}

bool UTestUtilities::AssertStringsEqual(const FString& A, const FString& B, bool bCaseSensitive, const FString& Message)
{
	bool bEqual = bCaseSensitive ? A.Equals(B) : A.Equals(B, ESearchCase::IgnoreCase);
	if (!bEqual)
	{
		UE_LOG(LogTemp, Error, TEXT("[ASSERT FAILED] %s (A: '%s', B: '%s')"), *Message, *A, *B);
		return false;
	}
	return true;
}

// ===== Performance Measurement =====

float UTestUtilities::StartPerformanceTimer()
{
	return FPlatformTime::Seconds();
}

float UTestUtilities::StopPerformanceTimer(float TimerHandle)
{
	return FPlatformTime::Seconds() - TimerHandle;
}

float UTestUtilities::GetCurrentFPS(UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		float DeltaTime = World->GetDeltaSeconds();
		return DeltaTime > 0.0f ? 1.0f / DeltaTime : 0.0f;
	}
	return 0.0f;
}

float UTestUtilities::GetCurrentFrameTime(UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		return World->GetDeltaSeconds() * 1000.0f; // Convert to ms
	}
	return 0.0f;
}

bool UTestUtilities::MeetsVRPerformanceTargets(const FTestPerformanceMetrics& Metrics)
{
	return Metrics.MeetsVRTargets();
}

float UTestUtilities::GetMemoryUsageMB()
{
	FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
	return MemoryStats.UsedPhysical / (1024.0f * 1024.0f);
}

// ===== VR Testing Utilities =====

bool UTestUtilities::IsVRHeadsetConnected()
{
	// TODO: IsHeadMountedDisplayEnabled() is deprecated in UE5.6 - using stub (false) for tests
	return false;
}

float UTestUtilities::GetVRTrackingQuality(UObject* WorldContextObject)
{
	if (GEngine && GEngine->XRSystem.IsValid())
	{
		// Simplified - actual implementation would check tracking state
		return IsVRHeadsetConnected() ? 1.0f : 0.0f;
	}
	return 0.0f;
}

float UTestUtilities::GetVRHandDistance(UObject* WorldContextObject)
{
	// This would require actual VR hand component references
	// Placeholder implementation
	return 0.0f;
}

bool UTestUtilities::IsVRHandTracked(UObject* WorldContextObject, bool bLeftHand)
{
	// Placeholder - would check actual hand tracking state
	return IsVRHeadsetConnected();
}

// ===== Math Utilities =====

float UTestUtilities::GetActorDistance(AActor* A, AActor* B)
{
	if (!A || !B)
	{
		return 0.0f;
	}
	return FVector::Dist(A->GetActorLocation(), B->GetActorLocation());
}

float UTestUtilities::CalculateVelocity(const FVector& StartPos, const FVector& EndPos, float DeltaTime)
{
	if (DeltaTime <= 0.0f)
	{
		return 0.0f;
	}
	return FVector::Dist(StartPos, EndPos) / DeltaTime;
}

float UTestUtilities::CalculateAcceleration(float StartVelocity, float EndVelocity, float DeltaTime)
{
	if (DeltaTime <= 0.0f)
	{
		return 0.0f;
	}
	return (EndVelocity - StartVelocity) / DeltaTime;
}

bool UTestUtilities::IsAngleNearlyEqual(float A, float B, float ToleranceDegrees)
{
	float NormA = NormalizeAngle(A);
	float NormB = NormalizeAngle(B);
	float Diff = FMath::Abs(NormA - NormB);
	return Diff <= ToleranceDegrees;
}

float UTestUtilities::NormalizeAngle(float AngleDegrees)
{
	float Angle = FMath::Fmod(AngleDegrees, 360.0f);
	if (Angle > 180.0f)
	{
		Angle -= 360.0f;
	}
	else if (Angle < -180.0f)
	{
		Angle += 360.0f;
	}
	return Angle;
}

// ===== String Utilities =====

FString UTestUtilities::FormatTestResult(const FTestCaseResult& Result)
{
	return Result.ToString();
}

FString UTestUtilities::FormatPerformanceMetrics(const FTestPerformanceMetrics& Metrics)
{
	return Metrics.ToString();
}

FString UTestUtilities::FormatDuration(float Seconds)
{
	if (Seconds < 1.0f)
	{
		return FString::Printf(TEXT("%.0fms"), Seconds * 1000.0f);
	}
	else if (Seconds < 60.0f)
	{
		return FString::Printf(TEXT("%.1fs"), Seconds);
	}
	else if (Seconds < 3600.0f)
	{
		int32 Minutes = FMath::FloorToInt(Seconds / 60.0f);
		int32 Secs = FMath::FloorToInt(Seconds) % 60;
		return FString::Printf(TEXT("%dm %ds"), Minutes, Secs);
	}
	else
	{
		int32 Hours = FMath::FloorToInt(Seconds / 3600.0f);
		int32 Minutes = (FMath::FloorToInt(Seconds) % 3600) / 60;
		return FString::Printf(TEXT("%dh %dm"), Hours, Minutes);
	}
}

FString UTestUtilities::FormatFileSize(int64 Bytes)
{
	if (Bytes < 1024)
	{
		return FString::Printf(TEXT("%lld B"), Bytes);
	}
	else if (Bytes < 1024 * 1024)
	{
		return FString::Printf(TEXT("%.1f KB"), Bytes / 1024.0f);
	}
	else if (Bytes < 1024 * 1024 * 1024)
	{
		return FString::Printf(TEXT("%.1f MB"), Bytes / (1024.0f * 1024.0f));
	}
	else
	{
		return FString::Printf(TEXT("%.1f GB"), Bytes / (1024.0f * 1024.0f * 1024.0f));
	}
}

FLinearColor UTestUtilities::GetTestStatusColor(ETestStatus Status)
{
	switch (Status)
	{
	case ETestStatus::Passed:
		return FLinearColor::Green;
	case ETestStatus::Failed:
		return FLinearColor::Red;
	case ETestStatus::InProgress:
		return FLinearColor::Yellow;
	case ETestStatus::Skipped:
		return FLinearColor(0.5f, 0.5f, 0.5f); // Gray
	case ETestStatus::Timeout:
		return FLinearColor(1.0f, 0.5f, 0.0f); // Orange
	case ETestStatus::Error:
		return FLinearColor(0.8f, 0.0f, 0.8f); // Purple
	case ETestStatus::NotStarted:
	default:
		return FLinearColor::White;
	}
}

// ===== Logging Utilities =====

void UTestUtilities::LogTestInfo(const FString& TestName, const FString& Message)
{
	UE_LOG(LogTemp, Log, TEXT("[TEST: %s] %s"), *TestName, *Message);
}

void UTestUtilities::LogTestWarning(const FString& TestName, const FString& Message)
{
	UE_LOG(LogTemp, Warning, TEXT("[TEST: %s] %s"), *TestName, *Message);
}

void UTestUtilities::LogTestError(const FString& TestName, const FString& Message)
{
	UE_LOG(LogTemp, Error, TEXT("[TEST: %s] %s"), *TestName, *Message);
}

void UTestUtilities::PrintTestMessage(const FString& Message, FLinearColor Color, float Duration)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Duration, Color.ToFColor(true), Message);
	}
}

// ===== Test Data Helpers =====

FTestCaseResult UTestUtilities::CreateTestCaseResult(const FString& TestName, const FString& Description)
{
	FTestCaseResult Result(TestName);
	Result.Description = Description;
	return Result;
}

FTestStationResults UTestUtilities::CreateTestStationResults(const FString& StationName)
{
	return FTestStationResults(StationName);
}

bool UTestUtilities::AllStationTestsPassed(const FTestStationResults& Results)
{
	return Results.AllTestsPassed();
}

int32 UTestUtilities::GetFailedTestCount(const FTestStationResults& Results)
{
	return Results.GetFailedTests();
}

float UTestUtilities::GetPassRate(const FTestStationResults& Results)
{
	return Results.GetPassRate();
}

// ===== Wait/Delay Utilities =====

void UTestUtilities::WaitForCondition(UObject* WorldContextObject, bool& Condition, float TimeoutSeconds, bool& bTimedOut, FLatentActionInfo LatentInfo)
{
	// This would require a custom latent action implementation
	// For now, this is a placeholder
	bTimedOut = false;
}

void UTestUtilities::WaitForObject(UObject* WorldContextObject, UObject*& Object, float TimeoutSeconds, bool& bTimedOut, FLatentActionInfo LatentInfo)
{
	// This would require a custom latent action implementation
	// For now, this is a placeholder
	bTimedOut = false;
}

// ===== Comparison Utilities =====

int32 UTestUtilities::CompareTestResults(const FTestCaseResult& A, const FTestCaseResult& B)
{
	// Sort by status priority: Failed > Error > Timeout > InProgress > Passed > Skipped > NotStarted
	auto GetPriority = [](const EAlexanderTestStatus TestStatus) -> int32
	{
		switch (TestStatus)
		{
		case EAlexanderTestStatus::Failed: return 0;
		case EAlexanderTestStatus::Timeout: return 1;
		case EAlexanderTestStatus::Running: return 2;
		case EAlexanderTestStatus::Passed: return 3;
		case EAlexanderTestStatus::Skipped: return 4;
		case EAlexanderTestStatus::NotRun: return 5;
		default: return 6;
		}
	};

	int32 PriorityA = GetPriority(A.Status);
	int32 PriorityB = GetPriority(B.Status);

	if (PriorityA < PriorityB) return -1;
	if (PriorityA > PriorityB) return 1;
	return 0;
}

TArray<FTestCaseResult> UTestUtilities::SortTestResultsByStatus(const TArray<FTestCaseResult>& Results)
{
	TArray<FTestCaseResult> Sorted = Results;
	Sorted.Sort([](const FTestCaseResult& A, const FTestCaseResult& B)
	{
		return CompareTestResults(A, B) < 0;
	});
	return Sorted;
}

TArray<FTestCaseResult> UTestUtilities::SortTestResultsByTime(const TArray<FTestCaseResult>& Results)
{
	TArray<FTestCaseResult> Sorted = Results;
	Sorted.Sort([](const FTestCaseResult& A, const FTestCaseResult& B)
	{
		return A.ExecutionTimeSeconds > B.ExecutionTimeSeconds; // Descending order
	});
	return Sorted;
}
