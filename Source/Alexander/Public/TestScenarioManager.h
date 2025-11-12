// Copyright (c) 2025 Alexander Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TestScenarioManager.generated.h"

// Test status enum
UENUM(BlueprintType)
enum class ETestStatus : uint8
{
	NotStarted		UMETA(DisplayName = "Not Started"),
	InProgress		UMETA(DisplayName = "In Progress"),
	Passed			UMETA(DisplayName = "Passed"),
	Failed			UMETA(DisplayName = "Failed"),
	Timeout			UMETA(DisplayName = "Timeout"),
	Error			UMETA(DisplayName = "Error"),
	Skipped			UMETA(DisplayName = "Skipped")
};

// Test step structure
USTRUCT(BlueprintType)
struct FTestStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	FString StepName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	float Timeout;

	UPROPERTY(BlueprintReadOnly, Category = "Test")
	ETestStatus Status;

	UPROPERTY(BlueprintReadOnly, Category = "Test")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Test")
	float ExecutionTime;

	FTestStep()
	{
		Timeout = 30.0f;
		Status = ETestStatus::NotStarted;
		ExecutionTime = 0.0f;
	}
};

// Test scenario structure
USTRUCT(BlueprintType)
struct FTestScenario
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	FString ScenarioName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	TArray<FTestStep> Steps;

	UPROPERTY(BlueprintReadOnly, Category = "Test")
	ETestStatus Status;

	UPROPERTY(BlueprintReadOnly, Category = "Test")
	float TotalDuration;

	UPROPERTY(BlueprintReadOnly, Category = "Test")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Test")
	int32 CurrentStepIndex;

	FTestScenario()
	{
		Status = ETestStatus::NotStarted;
		TotalDuration = 0.0f;
		CurrentStepIndex = 0;
	}
};

// Test execution result
USTRUCT(BlueprintType)
struct FTestExecutionResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Results")
	FString ScenarioName;

	UPROPERTY(BlueprintReadOnly, Category = "Results")
	ETestStatus Status;

	UPROPERTY(BlueprintReadOnly, Category = "Results")
	float Duration;

	UPROPERTY(BlueprintReadOnly, Category = "Results")
	FString ErrorMessage;

	UPROPERTY(BlueprintReadOnly, Category = "Results")
	TArray<FString> StepResults;

	FTestExecutionResult()
	{
		Status = ETestStatus::NotStarted;
		Duration = 0.0f;
	}
};

// Test step execution delegate (must be declared at top level for UHT)
DECLARE_DELEGATE_RetVal_TwoParams(FTestStepDelegate, bool, const FString& /*Parameters*/, FString& /*OutMessage*/);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UTestScenarioManager : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTestScenarioManager();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Initialize the manager
	UFUNCTION(BlueprintCallable, Category = "Test Scenarios")
	void Initialize();

	// Load test scenarios from JSON file
	UFUNCTION(BlueprintCallable, Category = "Test Scenarios")
	bool LoadScenariosFromFile(const FString& FilePath);

	// Load test scenarios from JSON string
	UFUNCTION(BlueprintCallable, Category = "Test Scenarios")
	bool LoadScenariosFromString(const FString& JsonString);

	// Get all available scenarios
	UFUNCTION(BlueprintCallable, Category = "Test Scenarios")
	TArray<FString> GetAvailableScenarios() const;

	// Get scenario by name
	UFUNCTION(BlueprintCallable, Category = "Test Scenarios")
	FTestScenario GetScenario(const FString& ScenarioName) const;

	// Execute single scenario
	UFUNCTION(BlueprintCallable, Category = "Test Scenarios")
	FTestExecutionResult ExecuteScenario(const FString& ScenarioName);

	// Execute all scenarios
	UFUNCTION(BlueprintCallable, Category = "Test Scenarios")
	TArray<FTestExecutionResult> ExecuteAllScenarios();

	// Execute scenario suite
	UFUNCTION(BlueprintCallable, Category = "Test Scenarios")
	TArray<FTestExecutionResult> ExecuteScenarioSuite(const TArray<FString>& ScenarioNames);

	// Get current execution status
	UFUNCTION(BlueprintCallable, Category = "Test Scenarios")
	ETestStatus GetCurrentStatus() const { return CurrentStatus; }

	// Get current scenario name
	UFUNCTION(BlueprintCallable, Category = "Test Scenarios")
	FString GetCurrentScenario() const { return CurrentScenario; }

	// Stop current execution
	UFUNCTION(BlueprintCallable, Category = "Test Scenarios")
	void StopExecution();

	// Get execution results
	UFUNCTION(BlueprintCallable, Category = "Test Scenarios")
	TArray<FTestExecutionResult> GetExecutionResults() const { return ExecutionResults; }

	// Get success rate
	UFUNCTION(BlueprintCallable, Category = "Test Scenarios")
	float GetSuccessRate() const;

	// Register custom test step handler
	void RegisterStepHandler(const FString& StepType, const FTestStepDelegate& Handler);

protected:
	// Execute test step
	bool ExecuteTestStep(FTestStep& Step);

	// Complete test step
	void CompleteTestStep(FTestStep& Step, bool bSuccess, const FString& Message = TEXT(""));

	// Fail test step
	void FailTestStep(FTestStep& Step, const FString& ErrorMessage);

	// Execute specific test action
	bool ExecuteTestAction(const FString& ActionName, const FString& Parameters);

	// Check test preconditions
	bool CheckPreconditions(const FTestStep& Step);

	// Update scenario status
	void UpdateScenarioStatus(FTestScenario& Scenario);

	// Get predefined scenarios
	void InitializePredefinedScenarios();

	// Complete gameplay loop test
	void SetupCompleteGameLoopTest();

	// Core systems integration test
	void SetupCoreSystemsIntegrationTest();

	// Individual system verification tests
	void SetupIndividualSystemTests();

	// Performance validation tests
	void SetupPerformanceValidationTests();

private:
	// Predefined test scenarios
	TArray<FTestScenario> TestScenarios;

	// Current execution status
	ETestStatus CurrentStatus;

	// Current scenario being executed
	FString CurrentScenario;

	// Current step index
	int32 CurrentStepIndex;

	// Execution results
	TArray<FTestExecutionResult> ExecutionResults;

	// Should stop execution
	bool bShouldStop;

	// Test step handlers
	TMap<FString, FTestStepDelegate> StepHandlers;

	// Execution start time
	float ExecutionStartTime;
};
