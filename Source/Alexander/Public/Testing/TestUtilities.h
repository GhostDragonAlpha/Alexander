// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TestDataStructures.h"
#include "TestUtilities.generated.h"

/**
 * Blueprint function library for test utilities
 *
 * Provides helper functions for:
 * - Test assertions and validation
 * - Performance measurement
 * - VR testing utilities
 * - Math validation
 * - String formatting and logging
 */
UCLASS()
class ALEXANDER_API UTestUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ===== Test Assertions =====

	/** Check if condition is true */
	UFUNCTION(BlueprintCallable, Category = "Testing|Assertions")
	static bool AssertTrue(bool Condition, const FString& Message = TEXT("Assertion failed"));

	/** Check if condition is false */
	UFUNCTION(BlueprintCallable, Category = "Testing|Assertions")
	static bool AssertFalse(bool Condition, const FString& Message = TEXT("Assertion failed"));

	/** Check if object is valid (not null) */
	UFUNCTION(BlueprintCallable, Category = "Testing|Assertions")
	static bool AssertIsValid(UObject* Object, const FString& Message = TEXT("Object is null"));

	/** Check if two floats are nearly equal */
	UFUNCTION(BlueprintCallable, Category = "Testing|Assertions")
	static bool AssertNearlyEqual(float A, float B, float Tolerance = 0.001f, const FString& Message = TEXT("Values not nearly equal"));

	/** Check if two vectors are nearly equal */
	UFUNCTION(BlueprintCallable, Category = "Testing|Assertions")
	static bool AssertVectorsNearlyEqual(const FVector& A, const FVector& B, float Tolerance = 0.001f, const FString& Message = TEXT("Vectors not nearly equal"));

	/** Check if value is within range */
	UFUNCTION(BlueprintCallable, Category = "Testing|Assertions")
	static bool AssertInRange(float Value, float Min, float Max, const FString& Message = TEXT("Value out of range"));

	/** Check if two strings are equal */
	UFUNCTION(BlueprintCallable, Category = "Testing|Assertions")
	static bool AssertStringsEqual(const FString& A, const FString& B, bool bCaseSensitive = true, const FString& Message = TEXT("Strings not equal"));

	// ===== Performance Measurement =====

	/** Start a performance timer and return a timer handle */
	UFUNCTION(BlueprintCallable, Category = "Testing|Performance")
	static float StartPerformanceTimer();

	/** Stop performance timer and return elapsed seconds */
	UFUNCTION(BlueprintCallable, Category = "Testing|Performance")
	static float StopPerformanceTimer(float TimerHandle);

	/** Get current FPS */
	UFUNCTION(BlueprintPure, Category = "Testing|Performance")
	static float GetCurrentFPS(UObject* WorldContextObject);

	/** Get current frame time in milliseconds */
	UFUNCTION(BlueprintPure, Category = "Testing|Performance")
	static float GetCurrentFrameTime(UObject* WorldContextObject);

	/** Check if performance meets VR targets (90+ FPS) */
	UFUNCTION(BlueprintPure, Category = "Testing|Performance")
	static bool MeetsVRPerformanceTargets(const FTestPerformanceMetrics& Metrics);

	/** Get memory usage in MB */
	UFUNCTION(BlueprintPure, Category = "Testing|Performance")
	static float GetMemoryUsageMB();

	// ===== VR Testing Utilities =====

	/** Check if VR headset is connected */
	UFUNCTION(BlueprintPure, Category = "Testing|VR")
	static bool IsVRHeadsetConnected();

	/** Get VR tracking state quality (0.0 = no tracking, 1.0 = perfect tracking) */
	UFUNCTION(BlueprintPure, Category = "Testing|VR")
	static float GetVRTrackingQuality(UObject* WorldContextObject);

	/** Get distance between VR hand controllers */
	UFUNCTION(BlueprintPure, Category = "Testing|VR")
	static float GetVRHandDistance(UObject* WorldContextObject);

	/** Check if VR hand controller is tracked */
	UFUNCTION(BlueprintPure, Category = "Testing|VR")
	static bool IsVRHandTracked(UObject* WorldContextObject, bool bLeftHand);

	// ===== Math Utilities =====

	/** Calculate distance between two actors */
	UFUNCTION(BlueprintPure, Category = "Testing|Math")
	static float GetActorDistance(AActor* A, AActor* B);

	/** Calculate velocity from two positions and time */
	UFUNCTION(BlueprintPure, Category = "Testing|Math")
	static float CalculateVelocity(const FVector& StartPos, const FVector& EndPos, float DeltaTime);

	/** Calculate acceleration from two velocities and time */
	UFUNCTION(BlueprintPure, Category = "Testing|Math")
	static float CalculateAcceleration(float StartVelocity, float EndVelocity, float DeltaTime);

	/** Check if angle is within tolerance */
	UFUNCTION(BlueprintPure, Category = "Testing|Math")
	static bool IsAngleNearlyEqual(float A, float B, float ToleranceDegrees = 1.0f);

	/** Normalize angle to [-180, 180] range */
	UFUNCTION(BlueprintPure, Category = "Testing|Math")
	static float NormalizeAngle(float AngleDegrees);

	// ===== String Utilities =====

	/** Format test result as colored string */
	UFUNCTION(BlueprintPure, Category = "Testing|Formatting")
	static FString FormatTestResult(const FTestCaseResult& Result);

	/** Format performance metrics as string */
	UFUNCTION(BlueprintPure, Category = "Testing|Formatting")
	static FString FormatPerformanceMetrics(const FTestPerformanceMetrics& Metrics);

	/** Format duration as human-readable string (e.g., "1m 30s") */
	UFUNCTION(BlueprintPure, Category = "Testing|Formatting")
	static FString FormatDuration(float Seconds);

	/** Format file size as human-readable string (e.g., "1.5 MB") */
	UFUNCTION(BlueprintPure, Category = "Testing|Formatting")
	static FString FormatFileSize(int64 Bytes);

	/** Get test status color (for UI) */
	UFUNCTION(BlueprintPure, Category = "Testing|Formatting")
	static FLinearColor GetTestStatusColor(ETestStatus Status);

	// ===== Logging Utilities =====

	/** Log test message with formatting */
	UFUNCTION(BlueprintCallable, Category = "Testing|Logging")
	static void LogTestInfo(const FString& TestName, const FString& Message);

	/** Log test warning with formatting */
	UFUNCTION(BlueprintCallable, Category = "Testing|Logging")
	static void LogTestWarning(const FString& TestName, const FString& Message);

	/** Log test error with formatting */
	UFUNCTION(BlueprintCallable, Category = "Testing|Logging")
	static void LogTestError(const FString& TestName, const FString& Message);

	/** Print to screen with color and duration */
	UFUNCTION(BlueprintCallable, Category = "Testing|Logging")
	static void PrintTestMessage(const FString& Message, FLinearColor Color = FLinearColor::White, float Duration = 5.0f);

	// ===== Test Data Helpers =====

	/** Create a new test case result */
	UFUNCTION(BlueprintPure, Category = "Testing|Data")
	static FTestCaseResult CreateTestCaseResult(const FString& TestName, const FString& Description = TEXT(""));

	/** Create a new test station results */
	UFUNCTION(BlueprintPure, Category = "Testing|Data")
	static FTestStationResults CreateTestStationResults(const FString& StationName);

	/** Check if all tests in station passed */
	UFUNCTION(BlueprintPure, Category = "Testing|Data")
	static bool AllStationTestsPassed(const FTestStationResults& Results);

	/** Get failed test count for station */
	UFUNCTION(BlueprintPure, Category = "Testing|Data")
	static int32 GetFailedTestCount(const FTestStationResults& Results);

	/** Get pass rate percentage */
	UFUNCTION(BlueprintPure, Category = "Testing|Data")
	static float GetPassRate(const FTestStationResults& Results);

	// ===== Wait/Delay Utilities =====

	/** Wait for condition to become true (with timeout) */
	UFUNCTION(BlueprintCallable, Category = "Testing|Async", meta = (Latent, WorldContext = "WorldContextObject", LatentInfo = "LatentInfo"))
	static void WaitForCondition(UObject* WorldContextObject, bool& Condition, float TimeoutSeconds, bool& bTimedOut, FLatentActionInfo LatentInfo);

	/** Wait for object to become valid (with timeout) */
	UFUNCTION(BlueprintCallable, Category = "Testing|Async", meta = (Latent, WorldContext = "WorldContextObject", LatentInfo = "LatentInfo"))
	static void WaitForObject(UObject* WorldContextObject, UObject*& Object, float TimeoutSeconds, bool& bTimedOut, FLatentActionInfo LatentInfo);

	// ===== Comparison Utilities =====

	/** Compare two test results */
	UFUNCTION(BlueprintPure, Category = "Testing|Comparison")
	static int32 CompareTestResults(const FTestCaseResult& A, const FTestCaseResult& B);

	/** Sort test results by status */
	UFUNCTION(BlueprintCallable, Category = "Testing|Comparison")
	static TArray<FTestCaseResult> SortTestResultsByStatus(const TArray<FTestCaseResult>& Results);

	/** Sort test results by execution time */
	UFUNCTION(BlueprintCallable, Category = "Testing|Comparison")
	static TArray<FTestCaseResult> SortTestResultsByTime(const TArray<FTestCaseResult>& Results);
};
