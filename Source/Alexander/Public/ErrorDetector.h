// Copyright (c) 2025 Alexander Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ErrorDetector.generated.h"

// Error severity enum
UENUM(BlueprintType)
enum class EErrorSeverity : uint8
{
	Info		UMETA(DisplayName = "Info"),
	Warning		UMETA(DisplayName = "Warning"),
	Error		UMETA(DisplayName = "Error"),
	Critical	UMETA(DisplayName = "Critical"),
	Fatal		UMETA(DisplayName = "Fatal")
};

// Error category enum
UENUM(BlueprintType)
enum class EErrorCategory : uint8
{
	Unknown			UMETA(DisplayName = "Unknown"),
	Crash			UMETA(DisplayName = "Crash"),
	Exception		UMETA(DisplayName = "Exception"),
	Performance		UMETA(DisplayName = "Performance"),
	Memory			UMETA(DisplayName = "Memory"),
	Network			UMETA(DisplayName = "Network"),
	Rendering		UMETA(DisplayName = "Rendering"),
	Physics			UMETA(DisplayName = "Physics"),
	AI				UMETA(DisplayName = "AI"),
	Script			UMETA(DisplayName = "Script"),
	Validation		UMETA(DisplayName = "Validation")
};

// Error report structure
USTRUCT(BlueprintType)
struct FErrorReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Error")
	FString ErrorId;

	UPROPERTY(BlueprintReadOnly, Category = "Error")
	EErrorCategory Category;

	UPROPERTY(BlueprintReadOnly, Category = "Error")
	EErrorSeverity Severity;

	UPROPERTY(BlueprintReadOnly, Category = "Error")
	FString Message;

	UPROPERTY(BlueprintReadOnly, Category = "Error")
	FString StackTrace;

	UPROPERTY(BlueprintReadOnly, Category = "Error")
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadOnly, Category = "Error")
	TMap<FString, FString> ContextData;

	UPROPERTY(BlueprintReadOnly, Category = "Error")
	FString ScreenshotPath;

	FErrorReport()
	{
		ErrorId = FGuid::NewGuid().ToString();
		Category = EErrorCategory::Unknown;
		Severity = EErrorSeverity::Error;
		Timestamp = FDateTime::Now();
	}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UErrorDetector : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UErrorDetector();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Initialize the detector
	UFUNCTION(BlueprintCallable, Category = "Error Detection")
	void Initialize();

	// Start monitoring for errors
	UFUNCTION(BlueprintCallable, Category = "Error Detection")
	void StartMonitoring();

	// Stop monitoring
	UFUNCTION(BlueprintCallable, Category = "Error Detection")
	void StopMonitoring();

	// Report an error
	UFUNCTION(BlueprintCallable, Category = "Error Detection")
	void ReportError(EErrorCategory Category, EErrorSeverity Severity, const FString& Message, const FString& StackTrace = TEXT(""));

	// Report error with context
	UFUNCTION(BlueprintCallable, Category = "Error Detection")
	void ReportErrorWithContext(EErrorCategory Category, EErrorSeverity Severity, const FString& Message, const TMap<FString, FString>& Context);

	// Get all error reports
	UFUNCTION(BlueprintCallable, Category = "Error Detection")
	TArray<FErrorReport> GetErrorReports() const { return ErrorReports; }

	// Get errors by severity
	UFUNCTION(BlueprintCallable, Category = "Error Detection")
	TArray<FErrorReport> GetErrorsBySeverity(EErrorSeverity Severity) const;

	// Get errors by category
	UFUNCTION(BlueprintCallable, Category = "Error Detection")
	TArray<FErrorReport> GetErrorsByCategory(EErrorCategory Category) const;

	// Get error count
	UFUNCTION(BlueprintCallable, Category = "Error Detection")
	int32 GetErrorCount() const { return ErrorReports.Num(); }

	// Get error count by severity
	UFUNCTION(BlueprintCallable, Category = "Error Detection")
	int32 GetErrorCountBySeverity(EErrorSeverity Severity) const;

	// Has critical errors
	UFUNCTION(BlueprintCallable, Category = "Error Detection")
	bool HasCriticalErrors() const;

	// Clear error reports
	UFUNCTION(BlueprintCallable, Category = "Error Detection")
	void ClearErrorReports();

	// Export error reports
	UFUNCTION(BlueprintCallable, Category = "Error Detection")
	void ExportErrorReports(const FString& FilePath) const;

	// Set screenshot path for future errors
	UFUNCTION(BlueprintCallable, Category = "Error Detection")
	void SetScreenshotPath(const FString& Path) { CurrentScreenshotPath = Path; }

protected:
	// Monitor for crashes
	void MonitorForCrashes();

	// Monitor for exceptions
	void MonitorForExceptions();

	// Monitor memory usage
	void MonitorMemoryUsage();

	// Monitor performance issues
	void MonitorPerformanceIssues();

	// Handle crash
	void HandleCrash(const FString& CrashInfo);

	// Handle exception
	void HandleException(const FString& ExceptionInfo);

	// Check for memory issues
	bool CheckMemoryIssues(float CurrentUsageMB);

	// Check for performance issues
	bool CheckPerformanceIssues(float FPS, float FrameTime);

	// Generate stack trace
	FString GenerateStackTrace() const;

private:
	// Error reports
	UPROPERTY( Category = "Errors")
	TArray<FErrorReport> ErrorReports;

	// Monitoring active
	UPROPERTY( Category = "State")
	bool bIsMonitoring;

	// Current screenshot path
	UPROPERTY( Category = "State")
	FString CurrentScreenshotPath;

	// Memory threshold for warnings (MB)
	UPROPERTY(EditAnywhere, Category = "Thresholds")
	float MemoryWarningThresholdMB;

	// Memory threshold for errors (MB)
	UPROPERTY(EditAnywhere, Category = "Thresholds")
	float MemoryErrorThresholdMB;

	// FPS threshold for warnings
	UPROPERTY(EditAnywhere, Category = "Thresholds")
	float FPSWarningThreshold;

	// FPS threshold for errors
	UPROPERTY(EditAnywhere, Category = "Thresholds")
	float FPSErrorThreshold;

	// Frame time threshold for warnings (ms)
	UPROPERTY(EditAnywhere, Category = "Thresholds")
	float FrameTimeWarningThresholdMS;

	// Frame time threshold for errors (ms)
	UPROPERTY(EditAnywhere, Category = "Thresholds")
	float FrameTimeErrorThresholdMS;

	// Timer handle for monitoring
	FTimerHandle MonitorTimer;

	// Monitor interval
	UPROPERTY(EditAnywhere, Category = "Configuration")
	float MonitorInterval;
};