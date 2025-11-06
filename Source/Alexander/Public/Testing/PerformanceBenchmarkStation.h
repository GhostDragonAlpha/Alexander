// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Testing/TestStation.h"
#include "PerformanceBenchmarkStation.generated.h"

// Forward declarations
class UPerformanceProfiler;
class UMemoryOptimizationManager;
class UTickOptimizationManager;
class UPerformanceTestingSystem;
class AOrbitalBody;
class ASpaceship;

/**
 * Benchmark result data structure
 */
USTRUCT(BlueprintType)
struct FBenchmarkResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Benchmark")
	FString BenchmarkName;

	UPROPERTY(BlueprintReadWrite, Category = "Benchmark")
	float AverageFPS = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Benchmark")
	float MinFPS = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Benchmark")
	float MaxFPS = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Benchmark")
	float AverageFrameTimeMs = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Benchmark")
	float GameThreadMs = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Benchmark")
	float RenderThreadMs = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Benchmark")
	float GPUMs = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Benchmark")
	float MemoryUsageMB = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Benchmark")
	float PeakMemoryMB = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Benchmark")
	int32 DrawCalls = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Benchmark")
	int32 Triangles = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Benchmark")
	float TestDuration = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Benchmark")
	bool bMetVRTarget = false;

	FBenchmarkResult() {}
};

/**
 * Stress test configuration
 */
USTRUCT(BlueprintType)
struct FStressTestConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress Test")
	int32 CelestialBodyCount = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress Test")
	int32 ActorCount = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress Test")
	float TestDuration = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress Test")
	bool bEnablePhysics = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stress Test")
	bool bEnableCollision = true;

	FStressTestConfig() {}
};

/**
 * Performance Benchmark Station
 *
 * Comprehensive performance testing and benchmarking system.
 * Measures frame rates, memory usage, CPU/GPU profiling, stress tests,
 * VR performance, streaming, tick performance, and network replication.
 *
 * Test Cases:
 * 1. Frame Rate Testing - Average, min, max FPS over time
 * 2. Memory Usage - Consumption, leaks, GC performance
 * 3. CPU/GPU Profiling - Thread times, bottleneck detection
 * 4. Stress Test - 50+ celestial bodies, 100+ actors
 * 5. VR Performance - Motion-to-photon latency, dropped frames
 * 6. Streaming Performance - Asset loading, texture streaming
 * 7. Tick Performance - Component tick overhead
 * 8. Network Performance - Replication bandwidth
 * 9. Benchmark Report Generation - Detailed CSV/JSON reports
 * 10. Regression Detection - Compare against baseline metrics
 *
 * Requirements: 7.1, 7.2, 7.3, 7.4, 7.5
 */
UCLASS(ClassGroup=(Testing), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UPerformanceBenchmarkStation : public UTestStation
{
	GENERATED_BODY()

public:
	UPerformanceBenchmarkStation();

protected:
	virtual void BeginPlay() override;
	virtual void RegisterTests_Implementation() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Test configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Testing|Config")
	float StandardTestDuration = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Testing|Config")
	float LongTestDuration = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Testing|Config")
	float VRFPSTarget = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Testing|Config")
	float StandardFPSTarget = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Testing|Config")
	float MemoryWarningThresholdMB = 3072.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Testing|Config")
	FStressTestConfig StressConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Testing|Config")
	FString BaselineReportPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Testing|Config")
	FString BenchmarkOutputDirectory = TEXT("Saved/Benchmarks");

	// Spawnable classes for stress testing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Testing|Spawning")
	TSubclassOf<AOrbitalBody> CelestialBodyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Testing|Spawning")
	TSubclassOf<AActor> StressTestActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Testing|Spawning")
	TSubclassOf<ASpaceship> SpaceshipClass;

protected:
	// Test case implementations

	/** Test 1.1: Frame Rate - Average FPS */
	void Test_AverageFPS();

	/** Test 1.2: Frame Rate - Minimum FPS */
	void Test_MinimumFPS();

	/** Test 1.3: Frame Rate - Maximum FPS */
	void Test_MaximumFPS();

	/** Test 1.4: Frame Rate - Frame Time Stability */
	void Test_FrameTimeStability();

	/** Test 2.1: Memory - Current Usage */
	void Test_MemoryUsage();

	/** Test 2.2: Memory - Peak Usage */
	void Test_PeakMemory();

	/** Test 2.3: Memory - Leak Detection */
	void Test_MemoryLeaks();

	/** Test 2.4: Memory - Garbage Collection Performance */
	void Test_GarbageCollection();

	/** Test 2.5: Memory - Memory Growth Rate */
	void Test_MemoryGrowth();

	/** Test 3.1: CPU Profiling - Game Thread Time */
	void Test_GameThreadTime();

	/** Test 3.2: CPU Profiling - Render Thread Time */
	void Test_RenderThreadTime();

	/** Test 3.3: CPU Profiling - GPU Time */
	void Test_GPUTime();

	/** Test 3.4: CPU Profiling - Bottleneck Detection */
	void Test_BottleneckDetection();

	/** Test 3.5: CPU Profiling - Draw Call Count */
	void Test_DrawCalls();

	/** Test 4.1: Stress Test - Celestial Bodies (50+) */
	void Test_CelestialBodyStress();

	/** Test 4.2: Stress Test - Actor Count (100+) */
	void Test_ActorCountStress();

	/** Test 4.3: Stress Test - Combined Load */
	void Test_CombinedStress();

	/** Test 4.4: Stress Test - Physics Performance */
	void Test_PhysicsStress();

	/** Test 5.1: VR Performance - Frame Rate (90+ FPS) */
	void Test_VRFrameRate();

	/** Test 5.2: VR Performance - Motion-to-Photon Latency */
	void Test_MotionToPhotonLatency();

	/** Test 5.3: VR Performance - Dropped Frames */
	void Test_DroppedFrames();

	/** Test 5.4: VR Performance - Reprojection Ratio */
	void Test_ReprojectionRatio();

	/** Test 6.1: Streaming - Asset Loading Time */
	void Test_AssetLoadingTime();

	/** Test 6.2: Streaming - Texture Streaming Performance */
	void Test_TextureStreaming();

	/** Test 6.3: Streaming - Bandwidth Usage */
	void Test_StreamingBandwidth();

	/** Test 6.4: Streaming - LOD Transitions */
	void Test_LODTransitions();

	/** Test 7.1: Tick Performance - Component Tick Overhead */
	void Test_TickOverhead();

	/** Test 7.2: Tick Performance - Tick Budget Compliance */
	void Test_TickBudget();

	/** Test 7.3: Tick Performance - Optimization Effectiveness */
	void Test_TickOptimization();

	/** Test 8.1: Network Performance - Replication Bandwidth */
	void Test_ReplicationBandwidth();

	/** Test 8.2: Network Performance - Actor Replication Count */
	void Test_ActorReplication();

	/** Test 9.1: Report Generation - CSV Export */
	void Test_CSVReportGeneration();

	/** Test 9.2: Report Generation - JSON Export */
	void Test_JSONReportGeneration();

	/** Test 9.3: Report Generation - Detailed Analysis */
	void Test_DetailedAnalysis();

	/** Test 10.1: Regression Detection - Baseline Comparison */
	void Test_BaselineComparison();

	/** Test 10.2: Regression Detection - Performance Degradation */
	void Test_PerformanceDegradation();

	// Helper functions
	void InitializePerformanceComponents();
	void CleanupPerformanceComponents();
	FBenchmarkResult RunBenchmark(const FString& BenchmarkName, float Duration);
	void SpawnStressTestObjects();
	void CleanupStressTestObjects();
	void CaptureBaselineMetrics();
	bool CompareAgainstBaseline(const FBenchmarkResult& Current, float TolerancePercent);
	bool ExportBenchmarkToCSV(const FString& Filename);
	bool ExportBenchmarkToJSON(const FString& Filename);
	FString GenerateBenchmarkReport();
	float MeasureFPS(float Duration);
	float MeasureFrameTime();
	float MeasureGameThread();
	float MeasureRenderThread();
	float MeasureGPU();
	float MeasureMemory();
	int32 MeasureDrawCalls();
	int32 MeasureTriangles();
	bool DetectMemoryLeaks(float Duration);
	float MeasureGCPerformance();
	TArray<FString> DetectBottlenecks();
	float MeasureMotionToPhoton();
	int32 CountDroppedFrames(float Duration);
	float MeasureReprojection();
	float MeasureStreamingBandwidth();
	float MeasureTickTime(const FString& ComponentName);
	float MeasureNetworkBandwidth();

private:
	// References to performance systems (PerformanceProfiler inherited from base class)

	UPROPERTY()
	UMemoryOptimizationManager* MemoryManager = nullptr;

	UPROPERTY()
	UTickOptimizationManager* TickManager = nullptr;

	UPROPERTY()
	UPerformanceTestingSystem* PerformanceSystem = nullptr;

	// Benchmark results
	TArray<FBenchmarkResult> BenchmarkResults;
	FBenchmarkResult BaselineResult;
	bool bHasBaseline = false;

	// Spawned stress test objects
	UPROPERTY()
	TArray<AOrbitalBody*> SpawnedCelestialBodies;

	UPROPERTY()
	TArray<AActor*> SpawnedStressActors;

	// Performance tracking
	TArray<float> FrameTimeSamples;
	TArray<float> FPSSamples;
	TArray<float> MemorySamples;
	float InitialMemoryMB = 0.0f;
	float PeakMemoryMB = 0.0f;

	// Test state
	bool bComponentsInitialized = false;
	float TestStartTime = 0.0f;

	// VR metrics
	int32 TotalFramesCounted = 0;
	int32 DroppedFramesCounted = 0;
};
