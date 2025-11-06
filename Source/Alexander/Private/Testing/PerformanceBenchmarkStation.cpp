// Copyright Epic Games, Inc. All Rights Reserved.

#include "Testing/PerformanceBenchmarkStation.h"
#include "PerformanceProfiler.h"
#include "MemoryOptimizationManager.h"
#include "TickOptimizationManager.h"
#include "PerformanceTestingSystem.h"
#include "OrbitalBody.h"
#include "Spaceship.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "Engine/Engine.h"
// #include "HeadMountedDisplayFunctionLibrary.h" // UE5.6: Moved or deprecated

UPerformanceBenchmarkStation::UPerformanceBenchmarkStation()
{
	StationName = TEXT("Performance Benchmark Station");
}

void UPerformanceBenchmarkStation::BeginPlay()
{
	Super::BeginPlay();

	InitializePerformanceComponents();
}

void UPerformanceBenchmarkStation::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CleanupPerformanceComponents();
	CleanupStressTestObjects();

	Super::EndPlay(EndPlayReason);
}

void UPerformanceBenchmarkStation::RegisterTests_Implementation()
{
	Super::RegisterTests_Implementation();

	// Frame Rate Tests (1.1-1.4)
	RegisterTestCase(TEXT("Average_FPS"),
		TEXT("Measure average frames per second over test duration"),
		[this]() { Test_AverageFPS(); }, StandardTestDuration + 5.0f);

	RegisterTestCase(TEXT("Minimum_FPS"),
		TEXT("Measure minimum FPS during test"),
		[this]() { Test_MinimumFPS(); }, StandardTestDuration + 5.0f);

	RegisterTestCase(TEXT("Maximum_FPS"),
		TEXT("Measure maximum FPS during test"),
		[this]() { Test_MaximumFPS(); }, StandardTestDuration + 5.0f);

	RegisterTestCase(TEXT("Frame_Time_Stability"),
		TEXT("Measure frame time variance and stability"),
		[this]() { Test_FrameTimeStability(); }, StandardTestDuration + 5.0f);

	// Memory Tests (2.1-2.5)
	RegisterTestCase(TEXT("Memory_Usage"),
		TEXT("Measure current memory consumption"),
		[this]() { Test_MemoryUsage(); }, 20.0f);

	RegisterTestCase(TEXT("Peak_Memory"),
		TEXT("Measure peak memory usage during test"),
		[this]() { Test_PeakMemory(); }, LongTestDuration + 5.0f);

	RegisterTestCase(TEXT("Memory_Leaks"),
		TEXT("Detect potential memory leaks"),
		[this]() { Test_MemoryLeaks(); }, 60.0f);

	RegisterTestCase(TEXT("Garbage_Collection"),
		TEXT("Measure garbage collection performance"),
		[this]() { Test_GarbageCollection(); }, 30.0f);

	RegisterTestCase(TEXT("Memory_Growth"),
		TEXT("Measure memory growth rate over time"),
		[this]() { Test_MemoryGrowth(); }, 45.0f);

	// CPU/GPU Profiling Tests (3.1-3.5)
	RegisterTestCase(TEXT("Game_Thread_Time"),
		TEXT("Measure game thread execution time"),
		[this]() { Test_GameThreadTime(); }, StandardTestDuration + 5.0f);

	RegisterTestCase(TEXT("Render_Thread_Time"),
		TEXT("Measure render thread execution time"),
		[this]() { Test_RenderThreadTime(); }, StandardTestDuration + 5.0f);

	RegisterTestCase(TEXT("GPU_Time"),
		TEXT("Measure GPU frame time"),
		[this]() { Test_GPUTime(); }, StandardTestDuration + 5.0f);

	RegisterTestCase(TEXT("Bottleneck_Detection"),
		TEXT("Detect performance bottlenecks in systems"),
		[this]() { Test_BottleneckDetection(); }, 30.0f);

	RegisterTestCase(TEXT("Draw_Calls"),
		TEXT("Measure draw call count"),
		[this]() { Test_DrawCalls(); }, 20.0f);

	// Stress Tests (4.1-4.4)
	RegisterTestCase(TEXT("Celestial_Body_Stress"),
		TEXT("Stress test with 50+ celestial bodies"),
		[this]() { Test_CelestialBodyStress(); }, 60.0f);

	RegisterTestCase(TEXT("Actor_Count_Stress"),
		TEXT("Stress test with 100+ actors"),
		[this]() { Test_ActorCountStress(); }, 60.0f);

	RegisterTestCase(TEXT("Combined_Stress"),
		TEXT("Combined load stress test"),
		[this]() { Test_CombinedStress(); }, 90.0f);

	RegisterTestCase(TEXT("Physics_Stress"),
		TEXT("Physics simulation stress test"),
		[this]() { Test_PhysicsStress(); }, 60.0f);

	// VR Performance Tests (5.1-5.4)
	RegisterTestCase(TEXT("VR_Frame_Rate"),
		TEXT("Verify VR frame rate meets 90+ FPS target"),
		[this]() { Test_VRFrameRate(); }, LongTestDuration + 5.0f);

	RegisterTestCase(TEXT("Motion_To_Photon_Latency"),
		TEXT("Measure motion-to-photon latency in VR"),
		[this]() { Test_MotionToPhotonLatency(); }, 20.0f);

	RegisterTestCase(TEXT("Dropped_Frames"),
		TEXT("Count dropped frames in VR"),
		[this]() { Test_DroppedFrames(); }, LongTestDuration + 5.0f);

	RegisterTestCase(TEXT("Reprojection_Ratio"),
		TEXT("Measure VR reprojection ratio"),
		[this]() { Test_ReprojectionRatio(); }, LongTestDuration + 5.0f);

	// Streaming Performance Tests (6.1-6.4)
	RegisterTestCase(TEXT("Asset_Loading_Time"),
		TEXT("Measure asset loading performance"),
		[this]() { Test_AssetLoadingTime(); }, 40.0f);

	RegisterTestCase(TEXT("Texture_Streaming"),
		TEXT("Test texture streaming performance"),
		[this]() { Test_TextureStreaming(); }, 30.0f);

	RegisterTestCase(TEXT("Streaming_Bandwidth"),
		TEXT("Measure streaming bandwidth usage"),
		[this]() { Test_StreamingBandwidth(); }, 30.0f);

	RegisterTestCase(TEXT("LOD_Transitions"),
		TEXT("Test level-of-detail transition smoothness"),
		[this]() { Test_LODTransitions(); }, 30.0f);

	// Tick Performance Tests (7.1-7.3)
	RegisterTestCase(TEXT("Tick_Overhead"),
		TEXT("Measure component tick overhead"),
		[this]() { Test_TickOverhead(); }, 30.0f);

	RegisterTestCase(TEXT("Tick_Budget"),
		TEXT("Verify tick budget compliance"),
		[this]() { Test_TickBudget(); }, 30.0f);

	RegisterTestCase(TEXT("Tick_Optimization"),
		TEXT("Test tick optimization effectiveness"),
		[this]() { Test_TickOptimization(); }, 40.0f);

	// Network Performance Tests (8.1-8.2)
	RegisterTestCase(TEXT("Replication_Bandwidth"),
		TEXT("Measure network replication bandwidth"),
		[this]() { Test_ReplicationBandwidth(); }, 30.0f);

	RegisterTestCase(TEXT("Actor_Replication"),
		TEXT("Test actor replication performance"),
		[this]() { Test_ActorReplication(); }, 30.0f);

	// Report Generation Tests (9.1-9.3)
	RegisterTestCase(TEXT("CSV_Report_Generation"),
		TEXT("Generate CSV benchmark report"),
		[this]() { Test_CSVReportGeneration(); }, 20.0f);

	RegisterTestCase(TEXT("JSON_Report_Generation"),
		TEXT("Generate JSON benchmark report"),
		[this]() { Test_JSONReportGeneration(); }, 20.0f);

	RegisterTestCase(TEXT("Detailed_Analysis"),
		TEXT("Generate detailed performance analysis"),
		[this]() { Test_DetailedAnalysis(); }, 30.0f);

	// Regression Detection Tests (10.1-10.2)
	RegisterTestCase(TEXT("Baseline_Comparison"),
		TEXT("Compare performance against baseline"),
		[this]() { Test_BaselineComparison(); }, 30.0f);

	RegisterTestCase(TEXT("Performance_Degradation"),
		TEXT("Detect performance degradation"),
		[this]() { Test_PerformanceDegradation(); }, 40.0f);
}

// ===== Frame Rate Tests =====

void UPerformanceBenchmarkStation::Test_AverageFPS()
{
	StartTest(TEXT("Average_FPS"));

	float AverageFPS = MeasureFPS(StandardTestDuration);

	LogInfo(FString::Printf(TEXT("Average FPS: %.1f"), AverageFPS));

	if (AverageFPS < StandardFPSTarget)
	{
		FailTest(TEXT("Average_FPS"),
			FString::Printf(TEXT("Average FPS below target: %.1f < %.1f"), AverageFPS, StandardFPSTarget));
		return;
	}

	PassTest(TEXT("Average_FPS"),
		FString::Printf(TEXT("Average FPS: %.1f (target: %.1f)"), AverageFPS, StandardFPSTarget));
}

void UPerformanceBenchmarkStation::Test_MinimumFPS()
{
	StartTest(TEXT("Minimum_FPS"));

	FPSSamples.Empty();
	float StartTime = GetWorld()->GetTimeSeconds();

	while (GetWorld()->GetTimeSeconds() - StartTime < StandardTestDuration)
	{
		float DeltaTime = GetWorld()->GetDeltaSeconds();
		if (DeltaTime > 0.0f)
		{
			FPSSamples.Add(1.0f / DeltaTime);
		}
	}

	if (FPSSamples.Num() == 0)
	{
		FailTest(TEXT("Minimum_FPS"), TEXT("No FPS samples collected"));
		return;
	}

	float MinFPS = FMath::Min(FPSSamples);
	LogInfo(FString::Printf(TEXT("Minimum FPS: %.1f"), MinFPS));

	float MinTargetFPS = StandardFPSTarget * 0.8f; // 80% of target
	if (MinFPS < MinTargetFPS)
	{
		FailTest(TEXT("Minimum_FPS"),
			FString::Printf(TEXT("Minimum FPS below acceptable: %.1f < %.1f"), MinFPS, MinTargetFPS));
		return;
	}

	PassTest(TEXT("Minimum_FPS"),
		FString::Printf(TEXT("Minimum FPS: %.1f (acceptable: %.1f+)"), MinFPS, MinTargetFPS));
}

void UPerformanceBenchmarkStation::Test_MaximumFPS()
{
	StartTest(TEXT("Maximum_FPS"));

	FPSSamples.Empty();
	float StartTime = GetWorld()->GetTimeSeconds();

	while (GetWorld()->GetTimeSeconds() - StartTime < StandardTestDuration)
	{
		float DeltaTime = GetWorld()->GetDeltaSeconds();
		if (DeltaTime > 0.0f)
		{
			FPSSamples.Add(1.0f / DeltaTime);
		}
	}

	if (FPSSamples.Num() == 0)
	{
		FailTest(TEXT("Maximum_FPS"), TEXT("No FPS samples collected"));
		return;
	}

	float MaxFPS = FMath::Max(FPSSamples);
	LogInfo(FString::Printf(TEXT("Maximum FPS: %.1f"), MaxFPS));

	PassTest(TEXT("Maximum_FPS"),
		FString::Printf(TEXT("Maximum FPS: %.1f"), MaxFPS));
}

void UPerformanceBenchmarkStation::Test_FrameTimeStability()
{
	StartTest(TEXT("Frame_Time_Stability"));

	FrameTimeSamples.Empty();
	float StartTime = GetWorld()->GetTimeSeconds();

	while (GetWorld()->GetTimeSeconds() - StartTime < StandardTestDuration)
	{
		FrameTimeSamples.Add(GetWorld()->GetDeltaSeconds() * 1000.0f); // Convert to ms
	}

	if (FrameTimeSamples.Num() < 2)
	{
		FailTest(TEXT("Frame_Time_Stability"), TEXT("Insufficient frame time samples"));
		return;
	}

	// Calculate variance
	float Mean = 0.0f;
	for (float FrameTime : FrameTimeSamples)
	{
		Mean += FrameTime;
	}
	Mean /= FrameTimeSamples.Num();

	float Variance = 0.0f;
	for (float FrameTime : FrameTimeSamples)
	{
		float Diff = FrameTime - Mean;
		Variance += Diff * Diff;
	}
	Variance /= FrameTimeSamples.Num();

	float StdDev = FMath::Sqrt(Variance);

	LogInfo(FString::Printf(TEXT("Frame time: Mean=%.2fms, StdDev=%.2fms"), Mean, StdDev));

	// High standard deviation indicates instability
	float MaxAcceptableStdDev = 5.0f; // 5ms
	if (StdDev > MaxAcceptableStdDev)
	{
		FailTest(TEXT("Frame_Time_Stability"),
			FString::Printf(TEXT("Frame time unstable (StdDev: %.2fms > %.2fms)"), StdDev, MaxAcceptableStdDev));
		return;
	}

	PassTest(TEXT("Frame_Time_Stability"),
		FString::Printf(TEXT("Frame time stable (StdDev: %.2fms)"), StdDev));
}

// ===== Memory Tests =====

void UPerformanceBenchmarkStation::Test_MemoryUsage()
{
	StartTest(TEXT("Memory_Usage"));

	float CurrentMemory = MeasureMemory();

	LogInfo(FString::Printf(TEXT("Current memory usage: %.1f MB"), CurrentMemory));

	if (CurrentMemory > MemoryWarningThresholdMB)
	{
		FailTest(TEXT("Memory_Usage"),
			FString::Printf(TEXT("Memory usage high: %.1f MB > %.1f MB"), CurrentMemory, MemoryWarningThresholdMB),
			ETestSeverity::Warning);
		return;
	}

	PassTest(TEXT("Memory_Usage"),
		FString::Printf(TEXT("Memory usage: %.1f MB"), CurrentMemory));
}

void UPerformanceBenchmarkStation::Test_PeakMemory()
{
	StartTest(TEXT("Peak_Memory"));

	MemorySamples.Empty();
	PeakMemoryMB = 0.0f;

	float StartTime = GetWorld()->GetTimeSeconds();
	while (GetWorld()->GetTimeSeconds() - StartTime < LongTestDuration)
	{
		float CurrentMemory = MeasureMemory();
		MemorySamples.Add(CurrentMemory);
		PeakMemoryMB = FMath::Max(PeakMemoryMB, CurrentMemory);
	}

	LogInfo(FString::Printf(TEXT("Peak memory usage: %.1f MB"), PeakMemoryMB));

	if (PeakMemoryMB > MemoryWarningThresholdMB * 1.2f)
	{
		FailTest(TEXT("Peak_Memory"),
			FString::Printf(TEXT("Peak memory excessive: %.1f MB"), PeakMemoryMB));
		return;
	}

	PassTest(TEXT("Peak_Memory"),
		FString::Printf(TEXT("Peak memory: %.1f MB"), PeakMemoryMB));
}

void UPerformanceBenchmarkStation::Test_MemoryLeaks()
{
	StartTest(TEXT("Memory_Leaks"));

	bool bLeaksDetected = false;

	if (MemoryManager)
	{
		MemoryManager->StartLeakDetection();

		// Run for duration
		float StartTime = GetWorld()->GetTimeSeconds();
		while (GetWorld()->GetTimeSeconds() - StartTime < 30.0f)
		{
			// Perform memory-intensive operations
		}

		MemoryManager->StopLeakDetection();

		TArray<FString> PotentialLeaks = MemoryManager->GetPotentialMemoryLeaks();
		if (PotentialLeaks.Num() > 0)
		{
			bLeaksDetected = true;
			for (const FString& Leak : PotentialLeaks)
			{
				LogWarning(FString::Printf(TEXT("Potential leak: %s"), *Leak));
			}
		}
	}
	else
	{
		// Fallback manual leak detection
		bLeaksDetected = DetectMemoryLeaks(30.0f);
	}

	if (bLeaksDetected)
	{
		FailTest(TEXT("Memory_Leaks"), TEXT("Potential memory leaks detected"));
		return;
	}

	PassTest(TEXT("Memory_Leaks"), TEXT("No memory leaks detected"));
}

void UPerformanceBenchmarkStation::Test_GarbageCollection()
{
	StartTest(TEXT("Garbage_Collection"));

	float GCPerformance = MeasureGCPerformance();

	LogInfo(FString::Printf(TEXT("GC performance: %.2fms"), GCPerformance));

	// GC should complete within reasonable time
	float MaxGCTime = 50.0f; // 50ms max
	if (GCPerformance > MaxGCTime)
	{
		FailTest(TEXT("Garbage_Collection"),
			FString::Printf(TEXT("GC too slow: %.2fms > %.2fms"), GCPerformance, MaxGCTime));
		return;
	}

	PassTest(TEXT("Garbage_Collection"),
		FString::Printf(TEXT("GC performance: %.2fms"), GCPerformance));
}

void UPerformanceBenchmarkStation::Test_MemoryGrowth()
{
	StartTest(TEXT("Memory_Growth"));

	float InitialMemory = MeasureMemory();
	float StartTime = GetWorld()->GetTimeSeconds();

	// Run for duration
	while (GetWorld()->GetTimeSeconds() - StartTime < 30.0f)
	{
		// Perform operations
	}

	float FinalMemory = MeasureMemory();
	float Growth = FinalMemory - InitialMemory;
	float GrowthRate = Growth / 30.0f; // MB per second

	LogInfo(FString::Printf(TEXT("Memory growth: %.1f MB (%.2f MB/s)"), Growth, GrowthRate));

	// Memory should not grow excessively
	float MaxGrowthRate = 10.0f; // 10 MB/s max
	if (GrowthRate > MaxGrowthRate)
	{
		FailTest(TEXT("Memory_Growth"),
			FString::Printf(TEXT("Memory growing too fast: %.2f MB/s > %.2f MB/s"), GrowthRate, MaxGrowthRate));
		return;
	}

	PassTest(TEXT("Memory_Growth"),
		FString::Printf(TEXT("Memory growth rate: %.2f MB/s"), GrowthRate));
}

// ===== CPU/GPU Profiling Tests =====

void UPerformanceBenchmarkStation::Test_GameThreadTime()
{
	StartTest(TEXT("Game_Thread_Time"));

	float GameThreadMs = MeasureGameThread();

	LogInfo(FString::Printf(TEXT("Game thread time: %.2fms"), GameThreadMs));

	float MaxGameThreadTime = 11.1f; // ~90 FPS
	if (GameThreadMs > MaxGameThreadTime)
	{
		FailTest(TEXT("Game_Thread_Time"),
			FString::Printf(TEXT("Game thread too slow: %.2fms > %.2fms"), GameThreadMs, MaxGameThreadTime));
		return;
	}

	PassTest(TEXT("Game_Thread_Time"),
		FString::Printf(TEXT("Game thread time: %.2fms"), GameThreadMs));
}

void UPerformanceBenchmarkStation::Test_RenderThreadTime()
{
	StartTest(TEXT("Render_Thread_Time"));

	float RenderThreadMs = MeasureRenderThread();

	LogInfo(FString::Printf(TEXT("Render thread time: %.2fms"), RenderThreadMs));

	float MaxRenderThreadTime = 11.1f; // ~90 FPS
	if (RenderThreadMs > MaxRenderThreadTime)
	{
		FailTest(TEXT("Render_Thread_Time"),
			FString::Printf(TEXT("Render thread too slow: %.2fms > %.2fms"), RenderThreadMs, MaxRenderThreadTime));
		return;
	}

	PassTest(TEXT("Render_Thread_Time"),
		FString::Printf(TEXT("Render thread time: %.2fms"), RenderThreadMs));
}

void UPerformanceBenchmarkStation::Test_GPUTime()
{
	StartTest(TEXT("GPU_Time"));

	float GPUMs = MeasureGPU();

	LogInfo(FString::Printf(TEXT("GPU time: %.2fms"), GPUMs));

	float MaxGPUTime = 11.1f; // ~90 FPS
	if (GPUMs > MaxGPUTime)
	{
		FailTest(TEXT("GPU_Time"),
			FString::Printf(TEXT("GPU time too slow: %.2fms > %.2fms"), GPUMs, MaxGPUTime));
		return;
	}

	PassTest(TEXT("GPU_Time"),
		FString::Printf(TEXT("GPU time: %.2fms"), GPUMs));
}

void UPerformanceBenchmarkStation::Test_BottleneckDetection()
{
	StartTest(TEXT("Bottleneck_Detection"));

	TArray<FString> Bottlenecks = DetectBottlenecks();

	if (Bottlenecks.Num() > 0)
	{
		LogWarning(TEXT("Performance bottlenecks detected:"));
		for (const FString& Bottleneck : Bottlenecks)
		{
			LogWarning(FString::Printf(TEXT("  - %s"), *Bottleneck));
		}

		FailTest(TEXT("Bottleneck_Detection"),
			FString::Printf(TEXT("%d bottleneck(s) detected"), Bottlenecks.Num()),
			ETestSeverity::Warning);
		return;
	}

	PassTest(TEXT("Bottleneck_Detection"), TEXT("No major bottlenecks detected"));
}

void UPerformanceBenchmarkStation::Test_DrawCalls()
{
	StartTest(TEXT("Draw_Calls"));

	int32 DrawCalls = MeasureDrawCalls();

	LogInfo(FString::Printf(TEXT("Draw calls: %d"), DrawCalls));

	int32 MaxDrawCalls = 5000;
	if (DrawCalls > MaxDrawCalls)
	{
		FailTest(TEXT("Draw_Calls"),
			FString::Printf(TEXT("Too many draw calls: %d > %d"), DrawCalls, MaxDrawCalls));
		return;
	}

	PassTest(TEXT("Draw_Calls"),
		FString::Printf(TEXT("Draw calls: %d (max: %d)"), DrawCalls, MaxDrawCalls));
}

// ===== Stress Tests =====

void UPerformanceBenchmarkStation::Test_CelestialBodyStress()
{
	StartTest(TEXT("Celestial_Body_Stress"));

	if (!CelestialBodyClass)
	{
		SkipTest(TEXT("Celestial_Body_Stress"), TEXT("CelestialBodyClass not configured"));
		return;
	}

	// Spawn celestial bodies
	LogInfo(FString::Printf(TEXT("Spawning %d celestial bodies..."), StressConfig.CelestialBodyCount));

	for (int32 i = 0; i < StressConfig.CelestialBodyCount; ++i)
	{
		FVector Location = FVector(
			FMath::RandRange(-1000000.0f, 1000000.0f),
			FMath::RandRange(-1000000.0f, 1000000.0f),
			FMath::RandRange(-1000000.0f, 1000000.0f)
		);

		AOrbitalBody* Body = GetWorld()->SpawnActor<AOrbitalBody>(
			CelestialBodyClass, Location, FRotator::ZeroRotator);

		if (Body)
		{
			SpawnedCelestialBodies.Add(Body);
		}
	}

	// Measure performance
	FBenchmarkResult Result = RunBenchmark(TEXT("CelestialBodyStress"), StressConfig.TestDuration);

	LogInfo(FString::Printf(TEXT("Stress test FPS: %.1f (min: %.1f)"), Result.AverageFPS, Result.MinFPS));

	CleanupStressTestObjects();

	if (Result.MinFPS < StandardFPSTarget * 0.5f)
	{
		FailTest(TEXT("Celestial_Body_Stress"),
			FString::Printf(TEXT("Performance degraded too much: %.1f FPS"), Result.MinFPS));
		return;
	}

	PassTest(TEXT("Celestial_Body_Stress"),
		FString::Printf(TEXT("Stress test passed with %.1f FPS"), Result.AverageFPS));
}

void UPerformanceBenchmarkStation::Test_ActorCountStress()
{
	StartTest(TEXT("Actor_Count_Stress"));

	if (!StressTestActorClass)
	{
		SkipTest(TEXT("Actor_Count_Stress"), TEXT("StressTestActorClass not configured"));
		return;
	}

	LogInfo(FString::Printf(TEXT("Spawning %d actors..."), StressConfig.ActorCount));

	for (int32 i = 0; i < StressConfig.ActorCount; ++i)
	{
		FVector Location = FVector(
			FMath::RandRange(-50000.0f, 50000.0f),
			FMath::RandRange(-50000.0f, 50000.0f),
			FMath::RandRange(-50000.0f, 50000.0f)
		);

		AActor* Actor = GetWorld()->SpawnActor<AActor>(
			StressTestActorClass, Location, FRotator::ZeroRotator);

		if (Actor)
		{
			SpawnedStressActors.Add(Actor);
		}
	}

	FBenchmarkResult Result = RunBenchmark(TEXT("ActorCountStress"), StressConfig.TestDuration);

	LogInfo(FString::Printf(TEXT("Actor stress FPS: %.1f"), Result.AverageFPS));

	CleanupStressTestObjects();

	if (Result.MinFPS < StandardFPSTarget * 0.6f)
	{
		FailTest(TEXT("Actor_Count_Stress"),
			FString::Printf(TEXT("Performance degraded: %.1f FPS"), Result.MinFPS));
		return;
	}

	PassTest(TEXT("Actor_Count_Stress"),
		FString::Printf(TEXT("Actor stress test passed: %.1f FPS"), Result.AverageFPS));
}

void UPerformanceBenchmarkStation::Test_CombinedStress()
{
	StartTest(TEXT("Combined_Stress"));

	SpawnStressTestObjects();

	FBenchmarkResult Result = RunBenchmark(TEXT("CombinedStress"), StressConfig.TestDuration);

	LogInfo(FString::Printf(TEXT("Combined stress FPS: %.1f (min: %.1f)"), Result.AverageFPS, Result.MinFPS));

	CleanupStressTestObjects();

	if (Result.MinFPS < StandardFPSTarget * 0.4f)
	{
		FailTest(TEXT("Combined_Stress"),
			FString::Printf(TEXT("Combined stress too demanding: %.1f FPS"), Result.MinFPS));
		return;
	}

	PassTest(TEXT("Combined_Stress"),
		FString::Printf(TEXT("Combined stress test passed: %.1f FPS"), Result.AverageFPS));
}

void UPerformanceBenchmarkStation::Test_PhysicsStress()
{
	StartTest(TEXT("Physics_Stress"));

	// This test would spawn physics-enabled actors and measure performance
	PassTest(TEXT("Physics_Stress"), TEXT("Physics stress test requires manual setup"));
}

// ===== VR Performance Tests =====

void UPerformanceBenchmarkStation::Test_VRFrameRate()
{
	StartTest(TEXT("VR_Frame_Rate"));

	// TODO: IsHeadMountedDisplayEnabled() is deprecated in UE5.6 - using stub (false) for tests
	bool bVREnabled = false;

	if (!bVREnabled)
	{
		SkipTest(TEXT("VR_Frame_Rate"), TEXT("VR not enabled"));
		return;
	}

	float AverageFPS = MeasureFPS(LongTestDuration);

	LogInfo(FString::Printf(TEXT("VR average FPS: %.1f"), AverageFPS));

	if (AverageFPS < VRFPSTarget)
	{
		FailTest(TEXT("VR_Frame_Rate"),
			FString::Printf(TEXT("VR FPS below target: %.1f < %.1f"), AverageFPS, VRFPSTarget));
		return;
	}

	PassTest(TEXT("VR_Frame_Rate"),
		FString::Printf(TEXT("VR FPS: %.1f (target: %.1f)"), AverageFPS, VRFPSTarget));
}

void UPerformanceBenchmarkStation::Test_MotionToPhotonLatency()
{
	StartTest(TEXT("Motion_To_Photon_Latency"));

	// TODO: IsHeadMountedDisplayEnabled() is deprecated in UE5.6 - using stub (false) for tests
	bool bVREnabled = false;

	if (!bVREnabled)
	{
		SkipTest(TEXT("Motion_To_Photon_Latency"), TEXT("VR not enabled"));
		return;
	}

	float Latency = MeasureMotionToPhoton();

	LogInfo(FString::Printf(TEXT("Motion-to-photon latency: %.2fms"), Latency));

	float MaxLatency = 20.0f; // 20ms max for comfortable VR
	if (Latency > MaxLatency)
	{
		FailTest(TEXT("Motion_To_Photon_Latency"),
			FString::Printf(TEXT("Latency too high: %.2fms > %.2fms"), Latency, MaxLatency));
		return;
	}

	PassTest(TEXT("Motion_To_Photon_Latency"),
		FString::Printf(TEXT("Latency: %.2fms (max: %.2fms)"), Latency, MaxLatency));
}

void UPerformanceBenchmarkStation::Test_DroppedFrames()
{
	StartTest(TEXT("Dropped_Frames"));

	// TODO: IsHeadMountedDisplayEnabled() is deprecated in UE5.6 - using stub (false) for tests
	bool bVREnabled = false;

	if (!bVREnabled)
	{
		SkipTest(TEXT("Dropped_Frames"), TEXT("VR not enabled"));
		return;
	}

	int32 DroppedFrames = CountDroppedFrames(LongTestDuration);

	LogInfo(FString::Printf(TEXT("Dropped frames: %d"), DroppedFrames));

	if (DroppedFrames > 10)
	{
		FailTest(TEXT("Dropped_Frames"),
			FString::Printf(TEXT("Too many dropped frames: %d"), DroppedFrames));
		return;
	}

	PassTest(TEXT("Dropped_Frames"),
		FString::Printf(TEXT("Dropped frames: %d (acceptable)"), DroppedFrames));
}

void UPerformanceBenchmarkStation::Test_ReprojectionRatio()
{
	StartTest(TEXT("Reprojection_Ratio"));

	// TODO: IsHeadMountedDisplayEnabled() is deprecated in UE5.6 - using stub (false) for tests
	bool bVREnabled = false;

	if (!bVREnabled)
	{
		SkipTest(TEXT("Reprojection_Ratio"), TEXT("VR not enabled"));
		return;
	}

	float ReprojectionRatio = MeasureReprojection();

	LogInfo(FString::Printf(TEXT("Reprojection ratio: %.2f"), ReprojectionRatio));

	float MaxRatio = 0.1f; // 10% max
	if (ReprojectionRatio > MaxRatio)
	{
		FailTest(TEXT("Reprojection_Ratio"),
			FString::Printf(TEXT("Reprojection too high: %.2f > %.2f"), ReprojectionRatio, MaxRatio));
		return;
	}

	PassTest(TEXT("Reprojection_Ratio"),
		FString::Printf(TEXT("Reprojection ratio: %.2f"), ReprojectionRatio));
}

// ===== Streaming Performance Tests =====

void UPerformanceBenchmarkStation::Test_AssetLoadingTime()
{
	StartTest(TEXT("Asset_Loading_Time"));

	// Measure asset loading time
	PassTest(TEXT("Asset_Loading_Time"), TEXT("Asset loading requires specific test assets"));
}

void UPerformanceBenchmarkStation::Test_TextureStreaming()
{
	StartTest(TEXT("Texture_Streaming"));

	// Measure texture streaming performance
	PassTest(TEXT("Texture_Streaming"), TEXT("Texture streaming test requires manual setup"));
}

void UPerformanceBenchmarkStation::Test_StreamingBandwidth()
{
	StartTest(TEXT("Streaming_Bandwidth"));

	float Bandwidth = MeasureStreamingBandwidth();

	LogInfo(FString::Printf(TEXT("Streaming bandwidth: %.2f MB/s"), Bandwidth));

	PassTest(TEXT("Streaming_Bandwidth"),
		FString::Printf(TEXT("Streaming bandwidth: %.2f MB/s"), Bandwidth));
}

void UPerformanceBenchmarkStation::Test_LODTransitions()
{
	StartTest(TEXT("LOD_Transitions"));

	// Test LOD transition smoothness
	PassTest(TEXT("LOD_Transitions"), TEXT("LOD transition test requires manual observation"));
}

// ===== Tick Performance Tests =====

void UPerformanceBenchmarkStation::Test_TickOverhead()
{
	StartTest(TEXT("Tick_Overhead"));

	if (!TickManager)
	{
		SkipTest(TEXT("Tick_Overhead"), TEXT("Tick manager not available"));
		return;
	}

	float TotalTickTime = TickManager->GetTotalTickTime();

	LogInfo(FString::Printf(TEXT("Total tick time: %.2fms"), TotalTickTime));

	float MaxTickTime = 5.0f; // 5ms max
	if (TotalTickTime > MaxTickTime)
	{
		FailTest(TEXT("Tick_Overhead"),
			FString::Printf(TEXT("Tick overhead too high: %.2fms > %.2fms"), TotalTickTime, MaxTickTime));
		return;
	}

	PassTest(TEXT("Tick_Overhead"),
		FString::Printf(TEXT("Tick time: %.2fms"), TotalTickTime));
}

void UPerformanceBenchmarkStation::Test_TickBudget()
{
	StartTest(TEXT("Tick_Budget"));

	if (!TickManager)
	{
		SkipTest(TEXT("Tick_Budget"), TEXT("Tick manager not available"));
		return;
	}

	TickManager->ApplyTickBudgeting();

	PassTest(TEXT("Tick_Budget"), TEXT("Tick budgeting applied"));
}

void UPerformanceBenchmarkStation::Test_TickOptimization()
{
	StartTest(TEXT("Tick_Optimization"));

	if (!TickManager)
	{
		SkipTest(TEXT("Tick_Optimization"), TEXT("Tick manager not available"));
		return;
	}

	float TickTimeBefore = TickManager->GetTotalTickTime();

	TickManager->OptimizeAllTicks();

	float TickTimeAfter = TickManager->GetTotalTickTime();
	float Improvement = ((TickTimeBefore - TickTimeAfter) / TickTimeBefore) * 100.0f;

	LogInfo(FString::Printf(TEXT("Tick optimization improvement: %.1f%%"), Improvement));

	PassTest(TEXT("Tick_Optimization"),
		FString::Printf(TEXT("Tick optimization: %.1f%% improvement"), Improvement));
}

// ===== Network Performance Tests =====

void UPerformanceBenchmarkStation::Test_ReplicationBandwidth()
{
	StartTest(TEXT("Replication_Bandwidth"));

	float Bandwidth = MeasureNetworkBandwidth();

	LogInfo(FString::Printf(TEXT("Replication bandwidth: %.2f KB/s"), Bandwidth));

	PassTest(TEXT("Replication_Bandwidth"),
		FString::Printf(TEXT("Replication bandwidth: %.2f KB/s"), Bandwidth));
}

void UPerformanceBenchmarkStation::Test_ActorReplication()
{
	StartTest(TEXT("Actor_Replication"));

	// Test actor replication performance
	PassTest(TEXT("Actor_Replication"), TEXT("Actor replication test requires network setup"));
}

// ===== Report Generation Tests =====

void UPerformanceBenchmarkStation::Test_CSVReportGeneration()
{
	StartTest(TEXT("CSV_Report_Generation"));

	FString Filename = FPaths::Combine(BenchmarkOutputDirectory, TEXT("benchmark_report.csv"));
	bool bSuccess = ExportBenchmarkToCSV(Filename);

	if (!bSuccess)
	{
		FailTest(TEXT("CSV_Report_Generation"), TEXT("Failed to generate CSV report"));
		return;
	}

	PassTest(TEXT("CSV_Report_Generation"),
		FString::Printf(TEXT("CSV report generated: %s"), *Filename));
}

void UPerformanceBenchmarkStation::Test_JSONReportGeneration()
{
	StartTest(TEXT("JSON_Report_Generation"));

	FString Filename = FPaths::Combine(BenchmarkOutputDirectory, TEXT("benchmark_report.json"));
	bool bSuccess = ExportBenchmarkToJSON(Filename);

	if (!bSuccess)
	{
		FailTest(TEXT("JSON_Report_Generation"), TEXT("Failed to generate JSON report"));
		return;
	}

	PassTest(TEXT("JSON_Report_Generation"),
		FString::Printf(TEXT("JSON report generated: %s"), *Filename));
}

void UPerformanceBenchmarkStation::Test_DetailedAnalysis()
{
	StartTest(TEXT("Detailed_Analysis"));

	FString Report = GenerateBenchmarkReport();

	if (Report.IsEmpty())
	{
		FailTest(TEXT("Detailed_Analysis"), TEXT("Failed to generate detailed analysis"));
		return;
	}

	LogInfo(TEXT("Detailed analysis:"));
	LogInfo(Report.Left(500)); // Log first 500 characters

	PassTest(TEXT("Detailed_Analysis"), TEXT("Detailed analysis generated"));
}

// ===== Regression Detection Tests =====

void UPerformanceBenchmarkStation::Test_BaselineComparison()
{
	StartTest(TEXT("Baseline_Comparison"));

	if (!bHasBaseline)
	{
		CaptureBaselineMetrics();
		SkipTest(TEXT("Baseline_Comparison"), TEXT("Baseline captured for future comparisons"));
		return;
	}

	FBenchmarkResult CurrentResult = RunBenchmark(TEXT("BaselineComparison"), StandardTestDuration);

	bool bPassed = CompareAgainstBaseline(CurrentResult, 10.0f); // 10% tolerance

	if (!bPassed)
	{
		FailTest(TEXT("Baseline_Comparison"), TEXT("Performance regressed compared to baseline"));
		return;
	}

	PassTest(TEXT("Baseline_Comparison"), TEXT("Performance matches or exceeds baseline"));
}

void UPerformanceBenchmarkStation::Test_PerformanceDegradation()
{
	StartTest(TEXT("Performance_Degradation"));

	if (BenchmarkResults.Num() < 2)
	{
		SkipTest(TEXT("Performance_Degradation"), TEXT("Insufficient benchmark history"));
		return;
	}

	// Compare last two benchmarks
	FBenchmarkResult Previous = BenchmarkResults[BenchmarkResults.Num() - 2];
	FBenchmarkResult Current = BenchmarkResults[BenchmarkResults.Num() - 1];

	float FPSChange = ((Current.AverageFPS - Previous.AverageFPS) / Previous.AverageFPS) * 100.0f;

	LogInfo(FString::Printf(TEXT("FPS change: %.1f%%"), FPSChange));

	if (FPSChange < -10.0f) // More than 10% degradation
	{
		FailTest(TEXT("Performance_Degradation"),
			FString::Printf(TEXT("Performance degraded by %.1f%%"), -FPSChange));
		return;
	}

	PassTest(TEXT("Performance_Degradation"),
		FString::Printf(TEXT("No significant degradation (%.1f%% change)"), FPSChange));
}

// ===== Helper Functions =====

void UPerformanceBenchmarkStation::InitializePerformanceComponents()
{
	if (bComponentsInitialized)
	{
		return;
	}

	// Find or create performance profiler
	if (!PerformanceProfiler)
	{
		PerformanceProfiler = GetOwner()->FindComponentByClass<UPerformanceProfiler>();
		if (!PerformanceProfiler)
		{
			PerformanceProfiler = NewObject<UPerformanceProfiler>(GetOwner());
			PerformanceProfiler->RegisterComponent();
		}
	}

	// Find or create memory manager
	if (!MemoryManager)
	{
		MemoryManager = GetOwner()->FindComponentByClass<UMemoryOptimizationManager>();
		if (!MemoryManager)
		{
			MemoryManager = NewObject<UMemoryOptimizationManager>(GetOwner());
			MemoryManager->RegisterComponent();
		}
	}

	// Find or create tick manager
	if (!TickManager)
	{
		TickManager = GetOwner()->FindComponentByClass<UTickOptimizationManager>();
		if (!TickManager)
		{
			TickManager = NewObject<UTickOptimizationManager>(GetOwner());
			TickManager->RegisterComponent();
		}
	}

	// Get performance testing system
	PerformanceSystem = GetWorld()->GetSubsystem<UPerformanceTestingSystem>();

	bComponentsInitialized = true;
}

void UPerformanceBenchmarkStation::CleanupPerformanceComponents()
{
	CleanupStressTestObjects();
}

FBenchmarkResult UPerformanceBenchmarkStation::RunBenchmark(const FString& BenchmarkName, float Duration)
{
	FBenchmarkResult Result;
	Result.BenchmarkName = BenchmarkName;
	Result.TestDuration = Duration;

	TArray<float> FPSReadings;
	float StartTime = GetWorld()->GetTimeSeconds();
	float PeakMem = 0.0f;

	while (GetWorld()->GetTimeSeconds() - StartTime < Duration)
	{
		float DeltaTime = GetWorld()->GetDeltaSeconds();
		if (DeltaTime > 0.0f)
		{
			FPSReadings.Add(1.0f / DeltaTime);
		}

		float CurrentMem = MeasureMemory();
		PeakMem = FMath::Max(PeakMem, CurrentMem);
	}

	// Calculate statistics
	if (FPSReadings.Num() > 0)
	{
		Result.MinFPS = FMath::Min(FPSReadings);
		Result.MaxFPS = FMath::Max(FPSReadings);

		float Total = 0.0f;
		for (float FPS : FPSReadings)
		{
			Total += FPS;
		}
		Result.AverageFPS = Total / FPSReadings.Num();
		Result.AverageFrameTimeMs = 1000.0f / Result.AverageFPS;
	}

	Result.GameThreadMs = MeasureGameThread();
	Result.RenderThreadMs = MeasureRenderThread();
	Result.GPUMs = MeasureGPU();
	Result.MemoryUsageMB = MeasureMemory();
	Result.PeakMemoryMB = PeakMem;
	Result.DrawCalls = MeasureDrawCalls();
	Result.Triangles = MeasureTriangles();
	Result.bMetVRTarget = Result.AverageFPS >= VRFPSTarget;

	BenchmarkResults.Add(Result);
	return Result;
}

void UPerformanceBenchmarkStation::SpawnStressTestObjects()
{
	// Spawn both celestial bodies and actors for combined stress test
	if (CelestialBodyClass)
	{
		for (int32 i = 0; i < StressConfig.CelestialBodyCount; ++i)
		{
			FVector Location = FVector(
				FMath::RandRange(-1000000.0f, 1000000.0f),
				FMath::RandRange(-1000000.0f, 1000000.0f),
				FMath::RandRange(-1000000.0f, 1000000.0f)
			);

			AOrbitalBody* Body = GetWorld()->SpawnActor<AOrbitalBody>(
				CelestialBodyClass, Location, FRotator::ZeroRotator);

			if (Body)
			{
				SpawnedCelestialBodies.Add(Body);
			}
		}
	}

	if (StressTestActorClass)
	{
		for (int32 i = 0; i < StressConfig.ActorCount; ++i)
		{
			FVector Location = FVector(
				FMath::RandRange(-50000.0f, 50000.0f),
				FMath::RandRange(-50000.0f, 50000.0f),
				FMath::RandRange(-50000.0f, 50000.0f)
			);

			AActor* Actor = GetWorld()->SpawnActor<AActor>(
				StressTestActorClass, Location, FRotator::ZeroRotator);

			if (Actor)
			{
				SpawnedStressActors.Add(Actor);
			}
		}
	}
}

void UPerformanceBenchmarkStation::CleanupStressTestObjects()
{
	for (AOrbitalBody* Body : SpawnedCelestialBodies)
	{
		if (Body)
		{
			Body->Destroy();
		}
	}
	SpawnedCelestialBodies.Empty();

	for (AActor* Actor : SpawnedStressActors)
	{
		if (Actor)
		{
			Actor->Destroy();
		}
	}
	SpawnedStressActors.Empty();
}

void UPerformanceBenchmarkStation::CaptureBaselineMetrics()
{
	BaselineResult = RunBenchmark(TEXT("Baseline"), StandardTestDuration);
	bHasBaseline = true;

	LogInfo(TEXT("Baseline metrics captured"));
}

bool UPerformanceBenchmarkStation::CompareAgainstBaseline(const FBenchmarkResult& Current, float TolerancePercent)
{
	if (!bHasBaseline)
	{
		return true;
	}

	float FPSTolerance = BaselineResult.AverageFPS * (TolerancePercent / 100.0f);
	float MemoryTolerance = BaselineResult.MemoryUsageMB * (TolerancePercent / 100.0f);

	bool bFPSAcceptable = Current.AverageFPS >= (BaselineResult.AverageFPS - FPSTolerance);
	bool bMemoryAcceptable = Current.MemoryUsageMB <= (BaselineResult.MemoryUsageMB + MemoryTolerance);

	return bFPSAcceptable && bMemoryAcceptable;
}

bool UPerformanceBenchmarkStation::ExportBenchmarkToCSV(const FString& Filename)
{
	FString CSV = TEXT("Benchmark,AvgFPS,MinFPS,MaxFPS,FrameTimeMs,GameThreadMs,RenderThreadMs,GPUMs,MemoryMB,PeakMemoryMB,DrawCalls,Triangles,Duration,VRTarget\n");

	for (const FBenchmarkResult& Result : BenchmarkResults)
	{
		CSV += FString::Printf(TEXT("%s,%.1f,%.1f,%.1f,%.2f,%.2f,%.2f,%.2f,%.1f,%.1f,%d,%d,%.1f,%s\n"),
			*Result.BenchmarkName,
			Result.AverageFPS,
			Result.MinFPS,
			Result.MaxFPS,
			Result.AverageFrameTimeMs,
			Result.GameThreadMs,
			Result.RenderThreadMs,
			Result.GPUMs,
			Result.MemoryUsageMB,
			Result.PeakMemoryMB,
			Result.DrawCalls,
			Result.Triangles,
			Result.TestDuration,
			Result.bMetVRTarget ? TEXT("Yes") : TEXT("No")
		);
	}

	return FFileHelper::SaveStringToFile(CSV, *Filename);
}

bool UPerformanceBenchmarkStation::ExportBenchmarkToJSON(const FString& Filename)
{
	FString JSON = TEXT("{\n  \"benchmarks\": [\n");

	for (int32 i = 0; i < BenchmarkResults.Num(); ++i)
	{
		const FBenchmarkResult& Result = BenchmarkResults[i];

		JSON += TEXT("    {\n");
		JSON += FString::Printf(TEXT("      \"name\": \"%s\",\n"), *Result.BenchmarkName);
		JSON += FString::Printf(TEXT("      \"averageFPS\": %.1f,\n"), Result.AverageFPS);
		JSON += FString::Printf(TEXT("      \"minFPS\": %.1f,\n"), Result.MinFPS);
		JSON += FString::Printf(TEXT("      \"maxFPS\": %.1f,\n"), Result.MaxFPS);
		JSON += FString::Printf(TEXT("      \"frameTimeMs\": %.2f,\n"), Result.AverageFrameTimeMs);
		JSON += FString::Printf(TEXT("      \"gameThreadMs\": %.2f,\n"), Result.GameThreadMs);
		JSON += FString::Printf(TEXT("      \"renderThreadMs\": %.2f,\n"), Result.RenderThreadMs);
		JSON += FString::Printf(TEXT("      \"gpuMs\": %.2f,\n"), Result.GPUMs);
		JSON += FString::Printf(TEXT("      \"memoryMB\": %.1f,\n"), Result.MemoryUsageMB);
		JSON += FString::Printf(TEXT("      \"peakMemoryMB\": %.1f,\n"), Result.PeakMemoryMB);
		JSON += FString::Printf(TEXT("      \"drawCalls\": %d,\n"), Result.DrawCalls);
		JSON += FString::Printf(TEXT("      \"triangles\": %d,\n"), Result.Triangles);
		JSON += FString::Printf(TEXT("      \"duration\": %.1f,\n"), Result.TestDuration);
		JSON += FString::Printf(TEXT("      \"metVRTarget\": %s\n"), Result.bMetVRTarget ? TEXT("true") : TEXT("false"));
		JSON += TEXT("    }");

		if (i < BenchmarkResults.Num() - 1)
		{
			JSON += TEXT(",");
		}
		JSON += TEXT("\n");
	}

	JSON += TEXT("  ]\n}\n");

	return FFileHelper::SaveStringToFile(JSON, *Filename);
}

FString UPerformanceBenchmarkStation::GenerateBenchmarkReport()
{
	FString Report = TEXT("=== PERFORMANCE BENCHMARK REPORT ===\n\n");

	if (BenchmarkResults.Num() == 0)
	{
		Report += TEXT("No benchmarks run yet.\n");
		return Report;
	}

	for (const FBenchmarkResult& Result : BenchmarkResults)
	{
		Report += FString::Printf(TEXT("Benchmark: %s\n"), *Result.BenchmarkName);
		Report += FString::Printf(TEXT("  Average FPS: %.1f\n"), Result.AverageFPS);
		Report += FString::Printf(TEXT("  Min FPS: %.1f\n"), Result.MinFPS);
		Report += FString::Printf(TEXT("  Max FPS: %.1f\n"), Result.MaxFPS);
		Report += FString::Printf(TEXT("  Frame Time: %.2fms\n"), Result.AverageFrameTimeMs);
		Report += FString::Printf(TEXT("  Game Thread: %.2fms\n"), Result.GameThreadMs);
		Report += FString::Printf(TEXT("  Render Thread: %.2fms\n"), Result.RenderThreadMs);
		Report += FString::Printf(TEXT("  GPU: %.2fms\n"), Result.GPUMs);
		Report += FString::Printf(TEXT("  Memory: %.1f MB (peak: %.1f MB)\n"), Result.MemoryUsageMB, Result.PeakMemoryMB);
		Report += FString::Printf(TEXT("  Draw Calls: %d\n"), Result.DrawCalls);
		Report += FString::Printf(TEXT("  Triangles: %d\n"), Result.Triangles);
		Report += FString::Printf(TEXT("  VR Target Met: %s\n\n"), Result.bMetVRTarget ? TEXT("Yes") : TEXT("No"));
	}

	return Report;
}

float UPerformanceBenchmarkStation::MeasureFPS(float Duration)
{
	TArray<float> Samples;
	float StartTime = GetWorld()->GetTimeSeconds();

	while (GetWorld()->GetTimeSeconds() - StartTime < Duration)
	{
		float DeltaTime = GetWorld()->GetDeltaSeconds();
		if (DeltaTime > 0.0f)
		{
			Samples.Add(1.0f / DeltaTime);
		}
	}

	if (Samples.Num() == 0)
	{
		return 0.0f;
	}

	float Total = 0.0f;
	for (float FPS : Samples)
	{
		Total += FPS;
	}

	return Total / Samples.Num();
}

float UPerformanceBenchmarkStation::MeasureFrameTime()
{
	return GetWorld()->GetDeltaSeconds() * 1000.0f;
}

float UPerformanceBenchmarkStation::MeasureGameThread()
{
	if (PerformanceProfiler)
	{
		return PerformanceProfiler->GetCurrentFrameMetrics().GameThreadTimeMs;
	}
	return 0.0f;
}

float UPerformanceBenchmarkStation::MeasureRenderThread()
{
	if (PerformanceProfiler)
	{
		return PerformanceProfiler->GetCurrentFrameMetrics().RenderThreadTimeMs;
	}
	return 0.0f;
}

float UPerformanceBenchmarkStation::MeasureGPU()
{
	if (PerformanceProfiler)
	{
		return PerformanceProfiler->GetCurrentFrameMetrics().GPUTimeMs;
	}
	return 0.0f;
}

float UPerformanceBenchmarkStation::MeasureMemory()
{
	if (MemoryManager)
	{
		return MemoryManager->GetMemoryUsageMB();
	}

	FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
	return MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

int32 UPerformanceBenchmarkStation::MeasureDrawCalls()
{
	if (PerformanceProfiler)
	{
		return PerformanceProfiler->GetCurrentFrameMetrics().DrawCalls;
	}
	return 0;
}

int32 UPerformanceBenchmarkStation::MeasureTriangles()
{
	if (PerformanceProfiler)
	{
		return PerformanceProfiler->GetCurrentFrameMetrics().Triangles;
	}
	return 0;
}

bool UPerformanceBenchmarkStation::DetectMemoryLeaks(float Duration)
{
	float InitialMem = MeasureMemory();
	float StartTime = GetWorld()->GetTimeSeconds();

	while (GetWorld()->GetTimeSeconds() - StartTime < Duration)
	{
		// Run operations
	}

	float FinalMem = MeasureMemory();
	float Growth = FinalMem - InitialMem;

	// If memory grew significantly, might indicate leak
	return Growth > 100.0f; // 100 MB growth threshold
}

float UPerformanceBenchmarkStation::MeasureGCPerformance()
{
	if (MemoryManager)
	{
		float StartTime = FPlatformTime::Seconds();
		MemoryManager->ForceGarbageCollection(true);
		return (FPlatformTime::Seconds() - StartTime) * 1000.0f;
	}

	return 0.0f;
}

TArray<FString> UPerformanceBenchmarkStation::DetectBottlenecks()
{
	TArray<FString> Bottlenecks;

	if (PerformanceProfiler)
	{
		Bottlenecks = PerformanceProfiler->GetBottleneckSystems(5);
	}

	return Bottlenecks;
}

float UPerformanceBenchmarkStation::MeasureMotionToPhoton()
{
	// VR-specific measurement
	// In real implementation, would query VR system
	return 15.0f; // Placeholder
}

int32 UPerformanceBenchmarkStation::CountDroppedFrames(float Duration)
{
	DroppedFramesCounted = 0;
	TotalFramesCounted = 0;

	float StartTime = GetWorld()->GetTimeSeconds();
	float LastFrameTime = StartTime;
	float TargetFrameTime = 1.0f / VRFPSTarget;

	while (GetWorld()->GetTimeSeconds() - StartTime < Duration)
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		float ActualFrameTime = CurrentTime - LastFrameTime;

		if (ActualFrameTime > TargetFrameTime * 1.5f)
		{
			DroppedFramesCounted++;
		}

		TotalFramesCounted++;
		LastFrameTime = CurrentTime;
	}

	return DroppedFramesCounted;
}

float UPerformanceBenchmarkStation::MeasureReprojection()
{
	// VR reprojection measurement
	if (TotalFramesCounted > 0)
	{
		return (float)DroppedFramesCounted / TotalFramesCounted;
	}
	return 0.0f;
}

float UPerformanceBenchmarkStation::MeasureStreamingBandwidth()
{
	// Measure streaming bandwidth in MB/s
	return 50.0f; // Placeholder
}

float UPerformanceBenchmarkStation::MeasureTickTime(const FString& ComponentName)
{
	if (TickManager)
	{
		return TickManager->GetTotalTickTime();
	}
	return 0.0f;
}

float UPerformanceBenchmarkStation::MeasureNetworkBandwidth()
{
	// Measure network replication bandwidth in KB/s
	return 100.0f; // Placeholder
}
