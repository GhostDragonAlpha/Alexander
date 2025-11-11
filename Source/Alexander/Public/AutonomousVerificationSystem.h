// Copyright (c) 2025 Alexander Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AutonomousVerificationSystem.generated.h"

// Forward declarations
class UGameLauncher;
class UVisualAnalyzer;
class UTestScenarioManager;
class UPerformanceMonitor;
class UErrorDetector;
class UReportGenerator;

// Test result structure
USTRUCT(BlueprintType)
struct FVerificationTestResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString TestName;

	UPROPERTY(BlueprintReadOnly)
	bool bSuccess;

	UPROPERTY(BlueprintReadOnly)
	FString Message;

	UPROPERTY(BlueprintReadOnly)
	float Duration;

	UPROPERTY(BlueprintReadOnly)
	FString ScreenshotPath;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, FString> AdditionalData;
};

// System status enum
UENUM(BlueprintType)
enum class EVerificationStatus : uint8
{
	Idle			UMETA(DisplayName = "Idle"),
	Running			UMETA(DisplayName = "Running"),
	Paused			UMETA(DisplayName = "Paused"),
	Completed		UMETA(DisplayName = "Completed"),
	Failed			UMETA(DisplayName = "Failed")
};

// Main verification system coordinator
UCLASS(Blueprintable, BlueprintType)
class ALEXANDER_API AAutonomousVerificationSystem : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AAutonomousVerificationSystem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called when actor is destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Start verification process
	UFUNCTION(BlueprintCallable, Category = "Verification")
	void StartVerification(const FString& TestSuiteName = TEXT("CompleteGameLoop"));

	// Stop verification process
	UFUNCTION(BlueprintCallable, Category = "Verification")
	void StopVerification();

	// Pause verification process
	UFUNCTION(BlueprintCallable, Category = "Verification")
	void PauseVerification();

	// Resume verification process
	UFUNCTION(BlueprintCallable, Category = "Verification")
	void ResumeVerification();

	// Get current status
	UFUNCTION(BlueprintCallable, Category = "Verification")
	EVerificationStatus GetStatus() const { return CurrentStatus; }

	// Get test results
	UFUNCTION(BlueprintCallable, Category = "Verification")
	TArray<FVerificationTestResult> GetTestResults() const { return TestResults; }

	// Get overall success rate
	UFUNCTION(BlueprintCallable, Category = "Verification")
	float GetSuccessRate() const;

	// Export report
	UFUNCTION(BlueprintCallable, Category = "Verification")
	void ExportReport(const FString& OutputPath);

	// HTTP API endpoint handlers
	UFUNCTION(BlueprintCallable, Category = "Verification|API")
	void HandleStartRequest(const FString& TestSuiteName);

	UFUNCTION(BlueprintCallable, Category = "Verification|API")
	void HandleStopRequest();

	UFUNCTION(BlueprintCallable, Category = "Verification|API")
	FString HandleStatusRequest();

	UFUNCTION(BlueprintCallable, Category = "Verification|API")
	FString HandleResultsRequest();

protected:
	// Component references
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UGameLauncher* GameLauncherComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UVisualAnalyzer* VisualAnalyzerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UTestScenarioManager* TestScenarioManagerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPerformanceMonitor* PerformanceMonitorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UErrorDetector* ErrorDetectorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UReportGenerator* ReportGeneratorComponent;

	// Current status
	UPROPERTY(BlueprintReadOnly, Category = "Status")
	EVerificationStatus CurrentStatus;

	// Test results
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	TArray<FVerificationTestResult> TestResults;

	// Current test suite
	UPROPERTY(BlueprintReadOnly, Category = "Configuration")
	FString CurrentTestSuite;

	// Verification start time
	UPROPERTY(BlueprintReadOnly, Category = "Timing")
	float VerificationStartTime;

	// Total duration
	UPROPERTY(BlueprintReadOnly, Category = "Timing")
	float TotalDuration;

	// Maximum allowed test time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float MaxTestDuration;

	// Screenshot capture interval
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float ScreenshotInterval;

	// Performance monitoring interval
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	float PerformanceMonitorInterval;

	// Enable visual analysis
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	bool bEnableVisualAnalysis;

	// Enable performance monitoring
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	bool bEnablePerformanceMonitoring;

	// Enable error detection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	bool bEnableErrorDetection;

	// Test scenarios to run
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	TArray<FString> TestScenarios;

private:
	// Initialize components
	void InitializeComponents();

	// Start test execution
	void StartTestExecution();

	// Complete verification
	void CompleteVerification();

	// Fail verification
	void FailVerification(const FString& Reason);

	// Update status
	void UpdateStatus(EVerificationStatus NewStatus);

	// Timer handles
	FTimerHandle TestExecutionTimer;
	FTimerHandle ScreenshotTimer;
	FTimerHandle PerformanceTimer;

	// Test execution in progress
	bool bTestExecutionInProgress;

	// Current test index
	int32 CurrentTestIndex;
};