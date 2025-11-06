// Copyright Epic Games, Inc. All Rights Reserved.

#include "PerformanceProfiler.h"
#include "Engine/World.h"
#include "GameFramework/GameState.h"
#include "HAL/Platform.h"
#include "Containers/List.h"

UPerformanceProfiler::UPerformanceProfiler()
	: ProfilingLevel(EProfilingLevel::Basic)
	, bEnableAutoExport(false)
	, AutoExportInterval(60.0f)
	, ExportDirectory(TEXT("Saved/Profiling"))
	, bDisplayOverlay(false)
	, OverlayUpdateRate(0.1f)
	, MaxStoredFrames(3600) // 1 minute at 60 FPS
	, bIsProfiling(false)
	, TimeSinceLastExport(0.0f)
	, TimeSinceLastOverlayUpdate(0.0f)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.016f; // ~60 FPS
}

void UPerformanceProfiler::BeginPlay()
{
	Super::BeginPlay();
	
	if (ProfilingLevel != EProfilingLevel::None)
	{
		StartProfiling();
	}
}

void UPerformanceProfiler::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopProfiling();
	Super::EndPlay(EndPlayReason);
}

void UPerformanceProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!bIsProfiling || ProfilingLevel == EProfilingLevel::None)
	{
		return;
	}
	
	// Capture frame metrics
	CaptureFrameMetrics();
	
	// Check thresholds
	CheckPerformanceThresholds();
	
	// Update overlay
	if (bDisplayOverlay)
	{
		TimeSinceLastOverlayUpdate += DeltaTime;
		if (TimeSinceLastOverlayUpdate >= OverlayUpdateRate)
		{
			UpdateOverlay();
			TimeSinceLastOverlayUpdate = 0.0f;
		}
	}
	
	// Auto-export if enabled
	if (bEnableAutoExport)
	{
		TimeSinceLastExport += DeltaTime;
		if (TimeSinceLastExport >= AutoExportInterval)
		{
			AutoExport();
			TimeSinceLastExport = 0.0f;
		}
	}
}

void UPerformanceProfiler::StartProfiling()
{
	bIsProfiling = true;
	FrameHistory.Reset();
	SystemData.Reset();
	CurrentWarnings.Reset();
}

void UPerformanceProfiler::StopProfiling()
{
	bIsProfiling = false;
}

void UPerformanceProfiler::ResetMetrics()
{
	FrameHistory.Reset();
	SystemData.Reset();
	CurrentWarnings.Reset();
	TimeSinceLastExport = 0.0f;
}

void UPerformanceProfiler::SetProfilingLevel(EProfilingLevel NewLevel)
{
	ProfilingLevel = NewLevel;
	
	if (ProfilingLevel == EProfilingLevel::None)
	{
		StopProfiling();
	}
	else if (!bIsProfiling)
	{
		StartProfiling();
	}
}

void UPerformanceProfiler::RegisterSystem(const FString& SystemName, EPerformanceCategory Category)
{
	if (!SystemData.Contains(SystemName))
	{
		FPerformanceProfilerData NewData;
		NewData.SystemName = SystemName;
		NewData.Category = Category;
		SystemData.Add(SystemName, NewData);
		SystemCategories.Add(SystemName, Category);
	}
}

void UPerformanceProfiler::UnregisterSystem(const FString& SystemName)
{
	SystemData.Remove(SystemName);
	SystemTickStartTimes.Remove(SystemName);
	SystemCategories.Remove(SystemName);
}

void UPerformanceProfiler::BeginSystemTick(const FString& SystemName)
{
	if (ProfilingLevel == EProfilingLevel::None)
	{
		return;
	}
	
	SystemTickStartTimes.FindOrAdd(SystemName) = FPlatformTime::Seconds();
}

void UPerformanceProfiler::EndSystemTick(const FString& SystemName)
{
	if (ProfilingLevel == EProfilingLevel::None)
	{
		return;
	}
	
	double* StartTimePtr = SystemTickStartTimes.Find(SystemName);
	if (StartTimePtr)
	{
		double TickTime = (FPlatformTime::Seconds() - *StartTimePtr) * 1000.0; // Convert to ms
		UpdateSystemMetrics(SystemName, (float)TickTime);
	}
}

FAlexanderFrameMetrics UPerformanceProfiler::GetCurrentFrameMetrics() const
{
	return CurrentFrame;
}

FAlexanderFrameMetrics UPerformanceProfiler::GetAverageFrameMetrics() const
{
	if (FrameHistory.Num() == 0)
	{
		return FAlexanderFrameMetrics();
	}
	
	FAlexanderFrameMetrics Average;
	for (const auto& Frame : FrameHistory)
	{
		Average.FrameTimeMs += Frame.FrameTimeMs;
		Average.GameThreadTimeMs += Frame.GameThreadTimeMs;
		Average.RenderThreadTimeMs += Frame.RenderThreadTimeMs;
		Average.GPUTimeMs += Frame.GPUTimeMs;
		Average.DrawCalls += Frame.DrawCalls;
		Average.Triangles += Frame.Triangles;
		Average.MemoryUsedMB += Frame.MemoryUsedMB;
		Average.ActiveActors += Frame.ActiveActors;
		Average.FPS += Frame.FPS;
	}
	
	int32 Count = FrameHistory.Num();
	Average.FrameTimeMs /= Count;
	Average.GameThreadTimeMs /= Count;
	Average.RenderThreadTimeMs /= Count;
	Average.GPUTimeMs /= Count;
	Average.DrawCalls /= Count;
	Average.Triangles /= Count;
	Average.MemoryUsedMB /= Count;
	Average.ActiveActors /= Count;
	Average.FPS /= Count;
	
	return Average;
}

TArray<FPerformanceProfilerData> UPerformanceProfiler::GetSystemPerformanceData() const
{
	TArray<FPerformanceProfilerData> Result;
	for (const auto& Pair : SystemData)
	{
		Result.Add(Pair.Value);
	}
	return Result;
}

FPerformanceProfilerData UPerformanceProfiler::GetSystemData(const FString& SystemName) const
{
	const FPerformanceProfilerData* Data = SystemData.Find(SystemName);
	return Data ? *Data : FPerformanceProfilerData();
}

float UPerformanceProfiler::GetAverageFPS() const
{
	FAlexanderFrameMetrics Average = GetAverageFrameMetrics();
	return Average.FPS;
}

float UPerformanceProfiler::GetCurrentFPS() const
{
	return CurrentFrame.FPS;
}

float UPerformanceProfiler::GetMemoryUsageMB() const
{
	return GetPlatformMemoryUsage();
}

bool UPerformanceProfiler::ExportToCSV(const FString& Filename)
{
	FString FilePath = GetExportFilePath(Filename);
	
	// Create CSV content
	FString CSVContent = TEXT("FrameNumber,FrameTime,GameThreadTime,RenderThreadTime,GPUTime,DrawCalls,Triangles,Memory,ActiveActors,FPS\n");
	
	for (int32 i = 0; i < FrameHistory.Num(); ++i)
	{
		const FAlexanderFrameMetrics& Frame = FrameHistory[i];
		CSVContent += FString::Printf(
			TEXT("%d,%.2f,%.2f,%.2f,%.2f,%d,%d,%.2f,%d,%.2f\n"),
			i,
			Frame.FrameTimeMs,
			Frame.GameThreadTimeMs,
			Frame.RenderThreadTimeMs,
			Frame.GPUTimeMs,
			Frame.DrawCalls,
			Frame.Triangles,
			Frame.MemoryUsedMB,
			Frame.ActiveActors,
			Frame.FPS
		);
	}
	
	return FFileHelper::SaveStringToFile(CSVContent, *FilePath);
}

bool UPerformanceProfiler::ExportSystemReport(const FString& Filename)
{
	FString FilePath = GetExportFilePath(Filename);
	
	FString Report = GeneratePerformanceReport();
	return FFileHelper::SaveStringToFile(Report, *FilePath);
}

FString UPerformanceProfiler::GeneratePerformanceReport() const
{
	FString Report = TEXT("=== Performance Report ===\n\n");
	
	// Current metrics
	FAlexanderFrameMetrics Current = GetCurrentFrameMetrics();
	FAlexanderFrameMetrics Average = GetAverageFrameMetrics();
	
	Report += FString::Printf(TEXT("Current Frame Time: %.2f ms (%.1f FPS)\n"), Current.FrameTimeMs, Current.FPS);
	Report += FString::Printf(TEXT("Average Frame Time: %.2f ms (%.1f FPS)\n"), Average.FrameTimeMs, Average.FPS);
	Report += FString::Printf(TEXT("Memory Usage: %.2f MB\n\n"), GetMemoryUsageMB());
	
	// System metrics
	Report += TEXT("System Metrics:\n");
	for (const auto& Pair : SystemData)
	{
		const FPerformanceProfilerData& Data = Pair.Value;
		Report += FString::Printf(
			TEXT("  %s: Avg=%.2f ms, Max=%.2f ms, Min=%.2f ms, Ticks=%d\n"),
			*Data.SystemName,
			Data.AverageTickTimeMs,
			Data.MaxTickTimeMs,
			Data.MinTickTimeMs,
			Data.TickCount
		);
	}
	
	// Warnings
	if (CurrentWarnings.Num() > 0)
	{
		Report += TEXT("\nWarnings:\n");
		for (const FString& Warning : CurrentWarnings)
		{
			Report += FString::Printf(TEXT("  - %s\n"), *Warning);
		}
	}
	
	return Report;
}

TArray<FString> UPerformanceProfiler::GetBottleneckSystems(int32 TopN) const
{
	TArray<FString> Bottlenecks;
	
	TArray<FPerformanceProfilerData> SystemList;
	for (const auto& Pair : SystemData)
	{
		SystemList.Add(Pair.Value);
	}
	
	// Sort by average tick time (descending)
	SystemList.Sort([](const FPerformanceProfilerData& A, const FPerformanceProfilerData& B)
	{
		return A.AverageTickTimeMs > B.AverageTickTimeMs;
	});
	
	for (int32 i = 0; i < FMath::Min(TopN, SystemList.Num()); ++i)
	{
		Bottlenecks.Add(SystemList[i].SystemName);
	}
	
	return Bottlenecks;
}

bool UPerformanceProfiler::IsPerformanceHealthy() const
{
	FAlexanderFrameMetrics Current = GetCurrentFrameMetrics();
	
	return Current.FrameTimeMs < Thresholds.WarningFrameTimeMs &&
		   GetMemoryUsageMB() < Thresholds.WarningMemoryMB &&
		   Current.DrawCalls < Thresholds.WarningDrawCalls;
}

void UPerformanceProfiler::CaptureFrameMetrics()
{
	// Get frame delta time from world
	float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
	CurrentFrame.FrameTimeMs = DeltaTime * 1000.0f;
	CurrentFrame.FPS = 1.0f / FMath::Max(0.001f, DeltaTime);
	CurrentFrame.MemoryUsedMB = GetPlatformMemoryUsage();
	CurrentFrame.DrawCalls = GetDrawCallCount();
	CurrentFrame.Triangles = GetTriangleCount();
	
	FrameHistory.Add(CurrentFrame);
	TrimFrameHistory();
}

void UPerformanceProfiler::UpdateSystemMetrics(const FString& SystemName, float TickTime)
{
	FPerformanceProfilerData* Data = SystemData.Find(SystemName);
	if (!Data)
	{
		return;
	}
	
	Data->TickCount++;
	Data->MaxTickTimeMs = FMath::Max(Data->MaxTickTimeMs, TickTime);
	Data->MinTickTimeMs = FMath::Min(Data->MinTickTimeMs, TickTime);
	
	// Calculate running average
	Data->AverageTickTimeMs = ((Data->AverageTickTimeMs * (Data->TickCount - 1)) + TickTime) / Data->TickCount;
}

void UPerformanceProfiler::CheckPerformanceThresholds()
{
	CurrentWarnings.Reset();
	
	if (CurrentFrame.FrameTimeMs > Thresholds.CriticalFrameTimeMs)
	{
		CurrentWarnings.Add(FString::Printf(TEXT("Critical frame time: %.2f ms"), CurrentFrame.FrameTimeMs));
		OnPerformanceWarning.Broadcast(EPerformanceCategory::Rendering, TEXT("Frame time critical"));
	}
	
	if (GetMemoryUsageMB() > Thresholds.CriticalMemoryMB)
	{
		CurrentWarnings.Add(FString::Printf(TEXT("Critical memory usage: %.2f MB"), GetMemoryUsageMB()));
		OnPerformanceWarning.Broadcast(EPerformanceCategory::Custom, TEXT("Memory usage critical"));
	}
}

void UPerformanceProfiler::UpdateOverlay()
{
	// Placeholder for HUD overlay updates
}

void UPerformanceProfiler::AutoExport()
{
	ExportToCSV(TEXT("Performance_Auto.csv"));
	ExportSystemReport(TEXT("Performance_Report_Auto.txt"));
}

FString UPerformanceProfiler::GetExportFilePath(const FString& Filename) const
{
	return FPaths::ProjectSavedDir() / ExportDirectory / Filename;
}

void UPerformanceProfiler::TrimFrameHistory()
{
	if (FrameHistory.Num() > MaxStoredFrames)
	{
		FrameHistory.RemoveAt(0, FrameHistory.Num() - MaxStoredFrames);
	}
}

float UPerformanceProfiler::GetPlatformMemoryUsage() const
{
	// Simplified memory reporting
	return FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
}

int32 UPerformanceProfiler::GetDrawCallCount() const
{
	// Placeholder for actual draw call count
	return 0;
}

int32 UPerformanceProfiler::GetTriangleCount() const
{
	// Placeholder for actual triangle count
	return 0;
}

float UPerformanceProfiler::GetGPUTime() const
{
	// Placeholder for GPU time measurement
	return 0.0f;
}

TArray<FString> UPerformanceProfiler::GetPerformanceWarnings() const
{
	TArray<FString> Warnings;
	
	if (ProfileData.FrameMetrics.Num() == 0)
	{
		return Warnings;
	}
	
	// Check frame time threshold
	float AverageFrameTime = 0.0f;
	for (const FAlexanderFrameMetrics& Metric : ProfileData.FrameMetrics)
	{
		AverageFrameTime += Metric.FrameTime;
	}
	AverageFrameTime /= ProfileData.FrameMetrics.Num();
	
	// Target frame time for 60 FPS is ~16.67ms
	if (AverageFrameTime > 20.0f)
	{
		Warnings.Add(FString::Printf(TEXT("High frame time detected: %.2f ms (target: 16.67 ms)"), AverageFrameTime));
	}
	
	// Check memory usage
	float AverageMemory = 0.0f;
	for (const FAlexanderFrameMetrics& Metric : ProfileData.FrameMetrics)
	{
		AverageMemory += Metric.MemoryUsage;
	}
	AverageMemory /= ProfileData.FrameMetrics.Num();
	
	if (AverageMemory > 2000.0f) // Warn if over 2GB
	{
		Warnings.Add(FString::Printf(TEXT("High memory usage: %.2f MB"), AverageMemory / 1024.0f));
	}
	
	// Check for frame time variance (stuttering)
	float MaxFrameTime = ProfileData.FrameMetrics[0].FrameTime;
	float MinFrameTime = ProfileData.FrameMetrics[0].FrameTime;
	for (const FAlexanderFrameMetrics& Metric : ProfileData.FrameMetrics)
	{
		MaxFrameTime = FMath::Max(MaxFrameTime, Metric.FrameTime);
		MinFrameTime = FMath::Min(MinFrameTime, Metric.FrameTime);
	}
	
	float FrameTimeVariance = MaxFrameTime - MinFrameTime;
	if (FrameTimeVariance > 10.0f) // Large variance indicates stuttering
	{
		Warnings.Add(FString::Printf(TEXT("Frame time variance detected: %.2f ms (min: %.2f, max: %.2f)"), 
			FrameTimeVariance, MinFrameTime, MaxFrameTime));
	}
	
	return Warnings;
}

