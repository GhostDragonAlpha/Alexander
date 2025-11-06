// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TestDataStructures.h"
#include "TestStation.generated.h"

// Forward declarations
class ATestLevelGameMode;
class UPerformanceProfiler;

/**
 * Delegate for test station events
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTestStationEvent, const FString&, EventMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTestCaseCompleted, const FString&, TestName, ETestStatus, Status);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAllTestsCompleted, const FTestStationResults&, Results);

/**
 * Base class for all test stations in the comprehensive test level
 *
 * Usage:
 * 1. Create derived class (e.g., UVRTestStation)
 * 2. Override RegisterTests() to add test cases
 * 3. Implement test case functions
 * 4. Call RunAllTests() to execute
 */
UCLASS(ClassGroup=(Testing), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UTestStation : public UActorComponent
{
	GENERATED_BODY()

public:
	UTestStation();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// Test execution
	UFUNCTION(BlueprintCallable, Category = "Testing")
	virtual void RunAllTests();

	UFUNCTION(BlueprintCallable, Category = "Testing")
	virtual void RunSingleTest(const FString& TestName);

	UFUNCTION(BlueprintCallable, Category = "Testing")
	virtual void StopAllTests();

	UFUNCTION(BlueprintCallable, Category = "Testing")
	virtual void ResetTests();

	// Test registration (override in derived classes)
	UFUNCTION(BlueprintNativeEvent, Category = "Testing")
	void RegisterTests();
	virtual void RegisterTests_Implementation();

	// Results access
	UFUNCTION(BlueprintPure, Category = "Testing")
	const FTestStationResults& GetResults() const { return Results; }

	UFUNCTION(BlueprintPure, Category = "Testing")
	bool AreTestsRunning() const { return bTestsRunning; }

	UFUNCTION(BlueprintPure, Category = "Testing")
	float GetTestProgress() const;

	UFUNCTION(BlueprintPure, Category = "Testing")
	FString GetCurrentTestName() const;

	// Station info
	UFUNCTION(BlueprintPure, Category = "Testing")
	FString GetStationName() const { return StationName; }

	UFUNCTION(BlueprintCallable, Category = "Testing")
	void SetStationName(const FString& Name) { StationName = Name; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Testing|Events")
	FOnTestStationEvent OnTestStarted;

	UPROPERTY(BlueprintAssignable, Category = "Testing|Events")
	FOnTestCaseCompleted OnTestCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Testing|Events")
	FOnAllTestsCompleted OnAllTestsCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Testing|Events")
	FOnTestStationEvent OnTestError;

protected:
	// Test case registration helpers
	void RegisterTestCase(const FString& TestName, const FString& Description, TFunction<void()> TestFunction, float TimeoutSeconds = 30.0f);

	// Test result helpers
	void StartTest(const FString& TestName);
	void PassTest(const FString& TestName, const FString& Message = TEXT(""));
	void FailTest(const FString& TestName, const FString& ErrorMessage, ETestSeverity Severity = ETestSeverity::Error);
	void SkipTest(const FString& TestName, const FString& Reason = TEXT(""));

	// Performance measurement
	void StartPerformanceCapture();
	FTestPerformanceMetrics StopPerformanceCapture();

	// Logging helpers
	void LogInfo(const FString& Message);
	void LogWarning(const FString& Message);
	void LogError(const FString& Message);

	// Utility functions
	bool CheckCondition(bool Condition, const FString& TestName, const FString& ErrorMessage);
	bool CheckNearlyEqual(float A, float B, float Tolerance, const FString& TestName, const FString& ErrorMessage);
	bool CheckNotNull(UObject* Object, const FString& TestName, const FString& ErrorMessage);

private:
	// Internal test execution
	void ExecuteNextTest();
	void CompleteCurrentTest();
	void HandleTestTimeout();
	void UpdatePerformanceMetrics();

	// Test case structure
	struct FTestCase
	{
		FString Name;
		FString Description;
		TFunction<void()> TestFunction;
		float TimeoutSeconds;
		FTestCaseResult Result;

		FTestCase()
			: Name(TEXT("")), Description(TEXT("")), TimeoutSeconds(30.0f)
		{}

		FTestCase(const FString& InName, const FString& InDesc, TFunction<void()> InFunc, float InTimeout)
			: Name(InName), Description(InDesc), TestFunction(InFunc), TimeoutSeconds(InTimeout), Result(InName)
		{}
	};

protected:
	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing|Config")
	FString StationName = TEXT("Test Station");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing|Config")
	float DefaultTestTimeout = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing|Config")
	bool bAutoStartTests = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing|Config")
	bool bCapturePerformanceMetrics = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing|Config")
	bool bStopOnFirstFailure = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing|Config")
	bool bVerboseLogging = true;

protected:
	// Test execution state
	TArray<FTestCase> RegisteredTests;
	FTestStationResults Results;
	int32 CurrentTestIndex = -1;
	float CurrentTestStartTime = 0.0f;
	bool bTestsRunning = false;
	bool bSingleTestMode = false;

	// Performance tracking
	TArray<float> FrameTimes;
	TArray<float> FPSValues;
	float PerformanceCaptureStartTime = 0.0f;
	int32 PerformanceFrameCount = 0;

	// References
	UPROPERTY()
	ATestLevelGameMode* GameModeRef = nullptr;

	UPROPERTY()
	UPerformanceProfiler* PerformanceProfiler = nullptr;

private:
	// Current test case being executed
	FTestCase* CurrentTest = nullptr;
};
