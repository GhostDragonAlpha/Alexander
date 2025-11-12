// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PerformanceTestingSystem.generated.h"

/**
 * Performance metrics captured at a specific altitude
 */
// Forward declaration - defined in PerformanceSelfMonitor.h
struct FSelfTestPerformanceMetrics;

/**
 * VR-specific performance metrics
 */
USTRUCT(BlueprintType)
struct FVRPerformanceMetrics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "VR Performance")
	float LeftEyeFrameTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "VR Performance")
	float RightEyeFrameTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "VR Performance")
	float MotionToPhotonLatency = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "VR Performance")
	int32 DroppedFrames = 0;

	UPROPERTY(BlueprintReadOnly, Category = "VR Performance")
	float IPD = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "VR Performance")
	bool bIsReprojecting = false;

	UPROPERTY(BlueprintReadOnly, Category = "VR Performance")
	float HeadsetFPS = 0.0f;

	FVRPerformanceMetrics()
		: LeftEyeFrameTime(0.0f)
		, RightEyeFrameTime(0.0f)
		, MotionToPhotonLatency(0.0f)
		, DroppedFrames(0)
		, IPD(0.0f)
		, bIsReprojecting(false)
		, HeadsetFPS(0.0f)
	{
	}
};

/**
 * Performance test configuration
 */
USTRUCT(BlueprintType)
struct FPerformanceTestConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Config")
	TArray<float> TestAltitudes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Config")
	float SampleDuration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Config")
	int32 SamplesPerAltitude = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Config")
	bool bTestVRPerformance = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Config")
	bool bGenerateReport = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Config")
	FString ReportOutputPath;

	FPerformanceTestConfig()
	{
		// Default test altitudes (in meters)
		TestAltitudes = {100000.0f, 50000.0f, 25000.0f, 10000.0f, 5000.0f, 1000.0f, 500.0f, 100.0f, 10.0f};
		ReportOutputPath = FPaths::ProjectSavedDir() / TEXT("PerformanceReports");
	}
};

/**
 * Performance test results summary
 */
USTRUCT(BlueprintType)
struct FPerformanceTestResults
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Results")
	TArray<FSelfTestPerformanceMetrics> Samples;

	UPROPERTY(BlueprintReadOnly, Category = "Results")
	TArray<FVRPerformanceMetrics> VRSamples;

	UPROPERTY(BlueprintReadOnly, Category = "Results")
	float AverageFrameRate = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Results")
	float MinimumFrameRate = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Results")
	float MaximumFrameRate = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Results")
	float AverageMemoryUsageMB = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Results")
	float PeakMemoryUsageMB = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Results")
	float AverageStreamingBandwidthMBps = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Results")
	float PeakStreamingBandwidthMBps = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Results")
	bool bMeetsVRRequirements = false;

	UPROPERTY(BlueprintReadOnly, Category = "Results")
	FDateTime TestStartTime;

	UPROPERTY(BlueprintReadOnly, Category = "Results")
	FDateTime TestEndTime;

	FPerformanceTestResults()
		: AverageFrameRate(0.0f)
		, MinimumFrameRate(0.0f)
		, MaximumFrameRate(0.0f)
		, AverageMemoryUsageMB(0.0f)
		, PeakMemoryUsageMB(0.0f)
		, AverageStreamingBandwidthMBps(0.0f)
		, PeakStreamingBandwidthMBps(0.0f)
		, bMeetsVRRequirements(false)
		, TestStartTime(FDateTime::Now())
		, TestEndTime(FDateTime::Now())
	{
	}
};

/**
 * Subsystem for comprehensive performance testing
 * Measures frame rates, streaming bandwidth, memory consumption, and VR performance
 * at various altitudes to validate requirements 8.1-8.5
 */
UCLASS()
class ALEXANDER_API UPerformanceTestingSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Start a comprehensive performance test
	 * @param Config Test configuration
	 * @return True if test started successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Performance Testing")
	bool StartPerformanceTest(const FPerformanceTestConfig& Config);

	/**
	 * Stop the current performance test
	 */
	UFUNCTION(BlueprintCallable, Category = "Performance Testing")
	void StopPerformanceTest();

	/**
	 * Check if a performance test is currently running
	 */
	UFUNCTION(BlueprintPure, Category = "Performance Testing")
	bool IsTestRunning() const { return bIsTestRunning; }

	/**
	 * Get the current test progress (0-1)
	 */
	UFUNCTION(BlueprintPure, Category = "Performance Testing")
	float GetTestProgress() const;

	/**
	 * Get the current test results
	 */
	UFUNCTION(BlueprintPure, Category = "Performance Testing")
	const FPerformanceTestResults& GetTestResults() const { return TestResults; }

	/**
	 * Capture current performance metrics
	 */
	UFUNCTION(BlueprintCallable, Category = "Performance Testing")
	FPerformanceMetrics CapturePerformanceMetrics();

	/**
	 * Capture VR-specific performance metrics
	 */
	UFUNCTION(BlueprintCallable, Category = "Performance Testing")
	FVRPerformanceMetrics CaptureVRPerformanceMetrics();

	/**
	 * Generate and save performance report
	 */
	UFUNCTION(BlueprintCallable, Category = "Performance Testing")
	bool GeneratePerformanceReport(const FString& OutputPath);

	/**
	 * Validate if performance meets VR requirements (90 FPS minimum)
	 */
	UFUNCTION(BlueprintPure, Category = "Performance Testing")
	bool ValidateVRPerformance(const FPerformanceTestResults& Results) const;

private:
	// Test state
	bool bIsTestRunning = false;
	FPerformanceTestConfig CurrentConfig;
	FPerformanceTestResults TestResults;
	int32 CurrentAltitudeIndex = 0;
	int32 CurrentSampleIndex = 0;
	float SampleTimer = 0.0f;
	float LastStreamingBytes = 0.0f;
	FDateTime LastStreamingCheckTime;

	// Tick function
	void TickTest(float DeltaTime);

	// Test phases
	void StartAltitudeTest(float Altitude);
	void CompleteAltitudeTest();
	void CompleteTest();

	// Metrics collection
	float MeasureFrameRate() const;
	float MeasureFrameTime() const;
	float MeasureGameThreadTime() const;
	float MeasureRenderThreadTime() const;
	float MeasureGPUTime() const;
	int32 MeasureDrawCalls() const;
	int32 MeasureVisibleTriangles() const;
	float MeasureMemoryUsage() const;
	float MeasureStreamingBandwidth();
	int32 CountActiveTerrainTiles() const;
	int32 CountStreamingTiles() const;

	// VR metrics
	float MeasureMotionToPhotonLatency() const;
	int32 CountDroppedFrames() const;
	bool IsReprojecting() const;
	float GetHeadsetFPS() const;

	// Analysis
	void AnalyzeResults();
	FString GenerateReportText() const;
	FString GenerateCSVReport() const;
	FString GenerateJSONReport() const;

	// Helpers
	FString FormatMetrics(const FPerformanceMetrics& Metrics) const;
	FString FormatVRMetrics(const FVRPerformanceMetrics& Metrics) const;
};
