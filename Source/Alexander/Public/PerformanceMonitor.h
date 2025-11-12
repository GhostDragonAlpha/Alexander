// Copyright (c) 2025 Alexander Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PerformanceMonitor.generated.h"

// Performance metrics structure
USTRUCT(BlueprintType)
struct FPerformanceMetrics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float FPS;

	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float FrameTime;

	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float CPUUsage;

	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float MemoryUsageMB;

	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float GPUMemoryUsageMB;

	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	int32 DrawCalls;

	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	int32 TrianglesRendered;

	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float PhysicsTime;

	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float GameThreadTime;

	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float RenderThreadTime;

	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float RHIThreadTime;

	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	FDateTime Timestamp;

	FPerformanceMetrics()
	{
		FPS = 0.0f;
		FrameTime = 0.0f;
		CPUUsage = 0.0f;
		MemoryUsageMB = 0.0f;
		GPUMemoryUsageMB = 0.0f;
		DrawCalls = 0;
		TrianglesRendered = 0;
		PhysicsTime = 0.0f;
		GameThreadTime = 0.0f;
		RenderThreadTime = 0.0f;
		RHIThreadTime = 0.0f;
		Timestamp = FDateTime::Now();
	}
};

// Performance baseline structure
USTRUCT(BlueprintType)
struct FPerformanceBaseline
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baseline")
	float TargetFPS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baseline")
	float MaxFrameTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baseline")
	float MaxMemoryUsageMB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baseline")
	float MaxCPUPercentage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Baseline")
	int32 MaxDrawCalls;

	FPerformanceBaseline()
	{
		TargetFPS = 90.0f;
		MaxFrameTime = 11.11f;
		MaxMemoryUsageMB = 8192.0f;
		MaxCPUPercentage = 80.0f;
		MaxDrawCalls = 5000;
	}
};

// Performance regression
USTRUCT(BlueprintType)
struct FPerformanceRegression
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Regression")
	FString MetricName;

	UPROPERTY(BlueprintReadOnly, Category = "Regression")
	float CurrentValue;

	UPROPERTY(BlueprintReadOnly, Category = "Regression")
	float BaselineValue;

	UPROPERTY(BlueprintReadOnly, Category = "Regression")
	float DeviationPercent;

	UPROPERTY(BlueprintReadOnly, Category = "Regression")
	FString Severity;

	FPerformanceRegression()
	{
		CurrentValue = 0.0f;
		BaselineValue = 0.0f;
		DeviationPercent = 0.0f;
	}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UPerformanceMonitor : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPerformanceMonitor();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Initialize the monitor
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void Initialize();

	// Start monitoring
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void StartMonitoring(float Interval = 1.0f);

	// Stop monitoring
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void StopMonitoring();

	// Get current metrics
	UFUNCTION(BlueprintCallable, Category = "Performance")
	FPerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

	// Get metrics history
	UFUNCTION(BlueprintCallable, Category = "Performance")
	TArray<FPerformanceMetrics> GetMetricsHistory() const { return MetricsHistory; }

	// Set performance baseline
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void SetBaseline(const FPerformanceBaseline& Baseline);

	// Get performance baseline
	UFUNCTION(BlueprintCallable, Category = "Performance")
	FPerformanceBaseline GetBaseline() const { return Baseline; }

	// Check for performance regressions
	UFUNCTION(BlueprintCallable, Category = "Performance")
	TArray<FPerformanceRegression> CheckForRegressions();

	// Get average metrics over time period
	UFUNCTION(BlueprintCallable, Category = "Performance")
	FPerformanceMetrics GetAverageMetrics(float TimePeriodSeconds) const;

	// Get minimum FPS in time period
	UFUNCTION(BlueprintCallable, Category = "Performance")
	float GetMinimumFPS(float TimePeriodSeconds) const;

	// Get maximum memory usage in time period
	UFUNCTION(BlueprintCallable, Category = "Performance")
	float GetMaximumMemoryUsage(float TimePeriodSeconds) const;

	// Is performance acceptable
	UFUNCTION(BlueprintCallable, Category = "Performance")
	bool IsPerformanceAcceptable() const;

	// Generate performance report
	UFUNCTION(BlueprintCallable, Category = "Performance")
	FString GenerateReport() const;

	// Export metrics to file
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void ExportMetrics(const FString& FilePath) const;

protected:
	// Collect performance metrics
	void CollectMetrics();

	// Update current metrics
	void UpdateMetrics();

	// Calculate FPS
	float CalculateFPS() const;

	// Get memory usage
	float GetMemoryUsage() const;

	// Get CPU usage
	float GetCPUUsage() const;

	// Get GPU memory usage
	float GetGPUMemoryUsage() const;

	// Get draw call count
	int32 GetDrawCallCount() const;

	// Get triangle count
	int32 GetTriangleCount() const;

	// Check metric against baseline
	bool CheckMetricAgainstBaseline(const FString& MetricName, float CurrentValue, float BaselineValue, float Tolerance);

private:
	// Current performance metrics
	FPerformanceMetrics CurrentMetrics;

	// Performance baseline
	FPerformanceBaseline Baseline;

	// Metrics history
	TArray<FPerformanceMetrics> MetricsHistory;

	// Monitoring active
	bool bIsMonitoring;

	// Monitor interval
	float MonitorInterval;

	// Timer handle
	FTimerHandle MonitorTimer;

	// Last frame time
	float LastFrameTime;

	// Frame time history
	TArray<float> FrameTimeHistory;

	// Maximum history size
	int32 MaxHistorySize;
};