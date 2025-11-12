// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Containers/Map.h"
#include "Containers/Array.h"
#include "HAL/ThreadSafeCounter.h"
#include "PerformanceSelfMonitor.generated.h"

// Performance metrics collected during testing (self-test specific)
USTRUCT(BlueprintType)
struct FSelfTestPerformanceMetrics
{
	GENERATED_BODY()

	// Average FPS during test
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float AverageFPS;

	// Minimum FPS during test
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float MinFPS;

	// Maximum FPS during test
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float MaxFPS;

	// Average frame time in ms
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float AverageFrameTime;

	// Peak memory usage in MB
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float PeakMemoryUsageMB;

	// Average memory usage in MB
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float AverageMemoryUsageMB;

	// Test duration in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float TestDuration;

	// Number of frames rendered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	int32 FramesRendered;

	// CPU usage percentage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float CPUUsagePercent;

	// GPU usage percentage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float GPUUsagePercent;

	// Number of draw calls
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	int32 DrawCalls;

	// Number of triangles rendered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	int32 TrianglesRendered;

	// Shader complexity warnings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	int32 ShaderComplexityWarnings;

	FSelfTestPerformanceMetrics()
	{
		AverageFPS = 0.0f;
		MinFPS = 0.0f;
		MaxFPS = 0.0f;
		AverageFrameTime = 0.0f;
		PeakMemoryUsageMB = 0.0f;
		AverageMemoryUsageMB = 0.0f;
		TestDuration = 0.0f;
		FramesRendered = 0;
		CPUUsagePercent = 0.0f;
		GPUUsagePercent = 0.0f;
		DrawCalls = 0;
		TrianglesRendered = 0;
		ShaderComplexityWarnings = 0;
	}
};

// Performance monitoring component for tracking FPS, memory, and other metrics
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UPerformanceSelfMonitor : public UActorComponent
{
	GENERATED_BODY()

public:
	UPerformanceSelfMonitor();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Start performance monitoring
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void StartMonitoring();

	// Stop performance monitoring
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void StopMonitoring();

	// Get current performance metrics
	UFUNCTION(BlueprintCallable, Category = "Performance")
	FSelfTestPerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

	// Get final performance metrics
	UFUNCTION(BlueprintCallable, Category = "Performance")
	FSelfTestPerformanceMetrics GetFinalMetrics() const { return FinalMetrics; }

	// Check if monitoring is active
	UFUNCTION(BlueprintCallable, Category = "Performance")
	bool IsMonitoring() const { return bIsMonitoring; }

	// Set performance targets
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void SetPerformanceTargets(float TargetFPS, float MaxMemoryMB);

	// Check if performance targets were met
	UFUNCTION(BlueprintCallable, Category = "Performance")
	bool WereTargetsMet(FString& OutFailureReason) const;

	// Get performance report as string
	UFUNCTION(BlueprintCallable, Category = "Performance")
	FString GetPerformanceReport() const;

	// Reset all metrics
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void ResetMetrics();

protected:
	// Update performance metrics
	void UpdateMetrics(float DeltaTime);

	// Collect memory usage
	void UpdateMemoryUsage();

	// Collect rendering stats
	void UpdateRenderingStats();

	// Check performance targets
	void CheckPerformanceTargets();

	// Calculate statistics
	void CalculateStatistics();

	// Current metrics being collected
	UPROPERTY()
	FSelfTestPerformanceMetrics CurrentMetrics;

	// Final metrics after monitoring stops
	UPROPERTY()
	FSelfTestPerformanceMetrics FinalMetrics;

	// Performance targets
	UPROPERTY()
	float TargetFPS;

	UPROPERTY()
	float MaxMemoryUsageMB;

	// Monitoring state
	UPROPERTY()
	bool bIsMonitoring;

	UPROPERTY()
	float MonitoringStartTime;

	UPROPERTY()
	float LastFrameTime;

	UPROPERTY()
	TArray<float> FPSHistory;

	UPROPERTY()
	TArray<float> MemoryUsageHistory;

	UPROPERTY()
	TArray<float> FrameTimeHistory;

	// Frame counting
	UPROPERTY()
	int32 FrameCount;

	UPROPERTY()
	float TotalFrameTime;

	UPROPERTY()
	float MinFrameTime;

	UPROPERTY()
	float MaxFrameTime;

	// Performance warnings
	UPROPERTY()
	TArray<FString> PerformanceWarnings;

	// Stat tracking (non-reflected counters used internally)
	FThreadSafeCounter DrawCallCount;

	FThreadSafeCounter TriangleCount;
};