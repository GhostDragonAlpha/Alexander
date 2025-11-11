// Copyright Epic Games, Inc. All Rights Reserved.

#include "PerformanceSelfMonitor.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Stats/Stats.h"
#include "HAL/PlatformProcess.h"
#include "GenericPlatform/GenericPlatformMemory.h"

UPerformanceSelfMonitor::UPerformanceSelfMonitor()
{
	PrimaryComponentTick.bCanEverTick = true;
	bIsMonitoring = false;
	TargetFPS = 90.0f;
	MaxMemoryUsageMB = 8192.0f;
	MonitoringStartTime = 0.0f;
	LastFrameTime = 0.0f;
	FrameCount = 0;
	TotalFrameTime = 0.0f;
	MinFrameTime = FLT_MAX;
	MaxFrameTime = 0.0f;
}

void UPerformanceSelfMonitor::BeginPlay()
{
	Super::BeginPlay();
	ResetMetrics();
}

void UPerformanceSelfMonitor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsMonitoring)
	{
		UpdateMetrics(DeltaTime);
	}
}

void UPerformanceSelfMonitor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bIsMonitoring)
	{
		StopMonitoring();
	}
	Super::EndPlay(EndPlayReason);
}

void UPerformanceSelfMonitor::StartMonitoring()
{
	if (bIsMonitoring)
	{
		return;
	}

	ResetMetrics();
	bIsMonitoring = true;
	MonitoringStartTime = GetWorld()->GetTimeSeconds();
	LastFrameTime = MonitoringStartTime;

	UE_LOG(LogTemp, Log, TEXT("Performance monitoring started"));
}

void UPerformanceSelfMonitor::StopMonitoring()
{
	if (!bIsMonitoring)
	{
		return;
	}

	bIsMonitoring = false;
	CalculateStatistics();
	FinalMetrics = CurrentMetrics;

	float TotalTime = GetWorld()->GetTimeSeconds() - MonitoringStartTime;
	UE_LOG(LogTemp, Log, TEXT("Performance monitoring stopped. Duration: %f seconds"), TotalTime);
}

void UPerformanceSelfMonitor::UpdateMetrics(float DeltaTime)
{
	FrameCount++;
	TotalFrameTime += DeltaTime;

	// Calculate FPS
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float FrameTime = CurrentTime - LastFrameTime;
	LastFrameTime = CurrentTime;

	if (FrameTime > 0.0f)
	{
		float FPS = 1.0f / FrameTime;
		FPSHistory.Add(FPS);
		FrameTimeHistory.Add(FrameTime * 1000.0f); // Convert to ms

		// Update min/max frame times
		MinFrameTime = FMath::Min(MinFrameTime, FrameTime);
		MaxFrameTime = FMath::Max(MaxFrameTime, FrameTime);
	}

	// Update memory usage
	UpdateMemoryUsage();

	// Update rendering stats
	UpdateRenderingStats();

	// Check performance targets
	CheckPerformanceTargets();
}

void UPerformanceSelfMonitor::UpdateMemoryUsage()
{
	FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
	float UsedMemoryMB = MemoryStats.UsedPhysical / (1024.0f * 1024.0f);

	MemoryUsageHistory.Add(UsedMemoryMB);
	CurrentMetrics.PeakMemoryUsageMB = FMath::Max(CurrentMetrics.PeakMemoryUsageMB, UsedMemoryMB);
}

void UPerformanceSelfMonitor::UpdateRenderingStats()
{
	// Get viewport stats if available
	if (GEngine && GEngine->GameViewport)
	{
		// Note: In a real implementation, you'd get actual rendering stats
		// This is a simplified version
		CurrentMetrics.DrawCalls = DrawCallCount.GetValue();
		CurrentMetrics.TrianglesRendered = TriangleCount.GetValue();
	}
}

void UPerformanceSelfMonitor::CheckPerformanceTargets()
{
	// Check FPS
	if (!FPSHistory.IsEmpty())
	{
		float CurrentFPS = FPSHistory.Last();
		if (CurrentFPS < TargetFPS * 0.8f) // 80% of target
		{
			FString Warning = FString::Printf(TEXT("Low FPS: %.1f (target: %.1f)"), CurrentFPS, TargetFPS);
			PerformanceWarnings.AddUnique(Warning);
		}
	}

	// Check memory usage
	if (CurrentMetrics.PeakMemoryUsageMB > MaxMemoryUsageMB)
	{
		FString Warning = FString::Printf(TEXT("High memory usage: %.1f MB (max: %.1f MB)"), 
			CurrentMetrics.PeakMemoryUsageMB, MaxMemoryUsageMB);
		PerformanceWarnings.AddUnique(Warning);
	}
}

void UPerformanceSelfMonitor::CalculateStatistics()
{
	if (FrameCount == 0)
	{
		return;
	}

	// Calculate FPS statistics
	if (!FPSHistory.IsEmpty())
	{
		float SumFPS = 0.0f;
		float MinFPS = FLT_MAX;
		float MaxFPS = 0.0f;

		for (float FPS : FPSHistory)
		{
			SumFPS += FPS;
			MinFPS = FMath::Min(MinFPS, FPS);
			MaxFPS = FMath::Max(MaxFPS, FPS);
		}

		CurrentMetrics.AverageFPS = SumFPS / FPSHistory.Num();
		CurrentMetrics.MinFPS = MinFPS;
		CurrentMetrics.MaxFPS = MaxFPS;
	}

	// Calculate frame time statistics
	if (!FrameTimeHistory.IsEmpty())
	{
		float SumFrameTime = 0.0f;
		for (float FrameTime : FrameTimeHistory)
		{
			SumFrameTime += FrameTime;
		}
		CurrentMetrics.AverageFrameTime = SumFrameTime / FrameTimeHistory.Num();
	}

	// Calculate memory statistics
	if (!MemoryUsageHistory.IsEmpty())
	{
		float SumMemory = 0.0f;
		for (float Memory : MemoryUsageHistory)
		{
			SumMemory += Memory;
		}
		CurrentMetrics.AverageMemoryUsageMB = SumMemory / MemoryUsageHistory.Num();
	}

	// Set other metrics
	CurrentMetrics.TestDuration = GetWorld()->GetTimeSeconds() - MonitoringStartTime;
	CurrentMetrics.FramesRendered = FrameCount;

	// Estimate CPU/GPU usage (simplified)
	CurrentMetrics.CPUUsagePercent = FMath::Clamp((CurrentMetrics.AverageFrameTime / 16.67f) * 100.0f, 0.0f, 100.0f);
	CurrentMetrics.GPUUsagePercent = CurrentMetrics.CPUUsagePercent * 0.9f; // Assume GPU similar to CPU
}

void UPerformanceSelfMonitor::SetPerformanceTargets(float InTargetFPS, float InMaxMemoryMB)
{
	TargetFPS = InTargetFPS;
	MaxMemoryUsageMB = InMaxMemoryMB;
}

bool UPerformanceSelfMonitor::WereTargetsMet(FString& OutFailureReason) const
{
	if (!FinalMetrics.FramesRendered > 0)
	{
		OutFailureReason = TEXT("No metrics collected");
		return false;
	}

	bool bTargetsMet = true;
	OutFailureReason = TEXT("");

	// Check FPS target
	if (FinalMetrics.AverageFPS < TargetFPS)
	{
		bTargetsMet = false;
		OutFailureReason += FString::Printf(TEXT("Average FPS %.1f below target %.1f. "), 
			FinalMetrics.AverageFPS, TargetFPS);
	}

	// Check memory target
	if (FinalMetrics.PeakMemoryUsageMB > MaxMemoryUsageMB)
	{
		bTargetsMet = false;
		OutFailureReason += FString::Printf(TEXT("Peak memory %.1f MB exceeded limit %.1f MB. "), 
			FinalMetrics.PeakMemoryUsageMB, MaxMemoryUsageMB);
	}

	return bTargetsMet;
}

FString UPerformanceSelfMonitor::GetPerformanceReport() const
{
	FString Report = TEXT("=== PERFORMANCE REPORT ===\n");
	Report += FString::Printf(TEXT("Test Duration: %.2f seconds\n"), FinalMetrics.TestDuration);
	Report += FString::Printf(TEXT("Frames Rendered: %d\n"), FinalMetrics.FramesRendered);
	Report += TEXT("\n--- FPS Statistics ---\n");
	Report += FString::Printf(TEXT("Average FPS: %.1f\n"), FinalMetrics.AverageFPS);
	Report += FString::Printf(TEXT("Min FPS: %.1f\n"), FinalMetrics.MinFPS);
	Report += FString::Printf(TEXT("Max FPS: %.1f\n"), FinalMetrics.MaxFPS);
	Report += TEXT("\n--- Frame Time Statistics ---\n");
	Report += FString::Printf(TEXT("Average Frame Time: %.2f ms\n"), FinalMetrics.AverageFrameTime);
	Report += TEXT("\n--- Memory Usage ---\n");
	Report += FString::Printf(TEXT("Peak Memory: %.1f MB\n"), FinalMetrics.PeakMemoryUsageMB);
	Report += FString::Printf(TEXT("Average Memory: %.1f MB\n"), FinalMetrics.AverageMemoryUsageMB);
	Report += TEXT("\n--- Resource Usage ---\n");
	Report += FString::Printf(TEXT("CPU Usage: %.1f%%\n"), FinalMetrics.CPUUsagePercent);
	Report += FString::Printf(TEXT("GPU Usage: %.1f%%\n"), FinalMetrics.GPUUsagePercent);
	Report += FString::Printf(TEXT("Draw Calls: %d\n"), FinalMetrics.DrawCalls);
	Report += FString::Printf(TEXT("Triangles: %d\n"), FinalMetrics.TrianglesRendered);

	if (!PerformanceWarnings.IsEmpty())
	{
		Report += TEXT("\n--- Performance Warnings ---\n");
		for (const FString& Warning : PerformanceWarnings)
		{
			Report += Warning + TEXT("\n");
		}
	}

	return Report;
}

void UPerformanceSelfMonitor::ResetMetrics()
{
	CurrentMetrics = FPerformanceMetrics();
	FinalMetrics = FPerformanceMetrics();
	FrameCount = 0;
	TotalFrameTime = 0.0f;
	MinFrameTime = FLT_MAX;
	MaxFrameTime = 0.0f;
	FPSHistory.Empty();
	MemoryUsageHistory.Empty();
	FrameTimeHistory.Empty();
	PerformanceWarnings.Empty();
	DrawCallCount.Reset();
	TriangleCount.Reset();
}