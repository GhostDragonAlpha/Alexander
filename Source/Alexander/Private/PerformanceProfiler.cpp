// Copyright Epic Games, Inc. All Rights Reserved.

#include "PerformanceProfiler.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformTime.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"

UPerformanceProfiler::UPerformanceProfiler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f; // Tick every frame for accurate profiling
    
    // Default settings
    ProfilingLevel = EProfilingLevel::Detailed;
    bEnableAutoExport = false;
    AutoExportInterval = 300.0f; // 5 minutes
    ExportDirectory = FPaths::ProjectSavedDir() + TEXT("Profiling/");
    bDisplayOverlay = false;
    OverlayUpdateRate = 0.5f;
    MaxStoredFrames = 1000;
    
    // Internal state
    bIsProfiling = false;
    TimeSinceLastExport = 0.0f;
    TimeSinceLastOverlayUpdate = 0.0f;
}

void UPerformanceProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-start profiling if level is not None
    if (ProfilingLevel != EProfilingLevel::None)
    {
        StartProfiling();
    }
}

void UPerformanceProfiler::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Export final report if auto-export is enabled
    if (bEnableAutoExport && bIsProfiling)
    {
        ExportSystemReport(TEXT("FinalReport"));
    }
    
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
    
    // Check performance thresholds
    CheckPerformanceThresholds();
    
    // Handle auto-export
    if (bEnableAutoExport)
    {
        TimeSinceLastExport += DeltaTime;
        if (TimeSinceLastExport >= AutoExportInterval)
        {
            AutoExport();
            TimeSinceLastExport = 0.0f;
        }
    }
    
    // Update overlay if enabled
    if (bDisplayOverlay)
    {
        TimeSinceLastOverlayUpdate += DeltaTime;
        if (TimeSinceLastOverlayUpdate >= OverlayUpdateRate)
        {
            UpdateOverlay();
            TimeSinceLastOverlayUpdate = 0.0f;
        }
    }
}

void UPerformanceProfiler::StartProfiling()
{
    if (bIsProfiling)
    {
        return;
    }
    
    bIsProfiling = true;
    ResetMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler: Started profiling at level %d"), (int32)ProfilingLevel);
}

void UPerformanceProfiler::StopProfiling()
{
    if (!bIsProfiling)
    {
        return;
    }
    
    bIsProfiling = false;
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler: Stopped profiling"));
}

void UPerformanceProfiler::ResetMetrics()
{
    FrameHistory.Empty();
    SystemData.Empty();
    SystemTickStartTimes.Empty();
    CurrentWarnings.Empty();
    
    CurrentFrame = FAlexanderFrameMetrics();
}

void UPerformanceProfiler::SetProfilingLevel(EProfilingLevel NewLevel)
{
    ProfilingLevel = NewLevel;
    
    if (NewLevel == EProfilingLevel::None && bIsProfiling)
    {
        StopProfiling();
    }
    else if (NewLevel != EProfilingLevel::None && !bIsProfiling)
    {
        StartProfiling();
    }
}

void UPerformanceProfiler::RegisterSystem(const FString& SystemName, EPerformanceCategory Category)
{
    if (!SystemData.Contains(SystemName))
    {
        FSystemPerformanceData NewData;
        NewData.SystemName = SystemName;
        NewData.Category = Category;
        SystemData.Add(SystemName, NewData);
        SystemCategories.Add(SystemName, Category);
        
        UE_LOG(LogTemp, Verbose, TEXT("Performance Profiler: Registered system '%s'"), *SystemName);
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
    if (ProfilingLevel == EProfilingLevel::Detailed || ProfilingLevel == EProfilingLevel::Exhaustive)
    {
        SystemTickStartTimes.Add(SystemName, FPlatformTime::Seconds());
    }
}

void UPerformanceProfiler::EndSystemTick(const FString& SystemName)
{
    if (ProfilingLevel == EProfilingLevel::Detailed || ProfilingLevel == EProfilingLevel::Exhaustive)
    {
        double* StartTime = SystemTickStartTimes.Find(SystemName);
        if (StartTime)
        {
            double EndTime = FPlatformTime::Seconds();
            float TickTime = (EndTime - *StartTime) * 1000.0f; // Convert to milliseconds
            
            UpdateSystemMetrics(SystemName, TickTime);
            SystemTickStartTimes.Remove(SystemName);
        }
    }
}

void UPerformanceProfiler::CaptureFrameMetrics()
{
    // Get basic frame timing
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    CurrentFrame.FrameTimeMs = World->GetDeltaSeconds() * 1000.0f;
    CurrentFrame.FPS = CurrentFrame.FrameTimeMs > 0.0f ? 1000.0f / CurrentFrame.FrameTimeMs : 0.0f;
    
    // Get memory usage
    CurrentFrame.MemoryUsedMB = GetPlatformMemoryUsage();
    
    // Get actor count
    CurrentFrame.ActiveActors = 0;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        CurrentFrame.ActiveActors++;
    }
    
    // Get rendering stats (only in Exhaustive mode for performance)
    if (ProfilingLevel == EProfilingLevel::Exhaustive)
    {
        CurrentFrame.DrawCalls = GetDrawCallCount();
        CurrentFrame.Triangles = GetTriangleCount();
        CurrentFrame.GPUTimeMs = GetGPUTime();
    }
    
    // Store frame history
    FrameHistory.Add(CurrentFrame);
    TrimFrameHistory();
}

void UPerformanceProfiler::UpdateSystemMetrics(const FString& SystemName, float TickTime)
{
    FSystemPerformanceData* Data = SystemData.Find(SystemName);
    if (!Data)
    {
        // Auto-register if not registered
        RegisterSystem(SystemName, EPerformanceCategory::Custom);
        Data = SystemData.Find(SystemName);
    }
    
    if (Data)
    {
        // Update statistics
        Data->TickCount++;
        Data->MaxTickTimeMs = FMath::Max(Data->MaxTickTimeMs, TickTime);
        Data->MinTickTimeMs = FMath::Min(Data->MinTickTimeMs, TickTime);
        
        // Update running average
        float TotalTime = Data->AverageTickTimeMs * (Data->TickCount - 1);
        Data->AverageTickTimeMs = (TotalTime + TickTime) / Data->TickCount;
    }
}

void UPerformanceProfiler::CheckPerformanceThresholds()
{
    CurrentWarnings.Empty();
    
    // Check frame time
    if (CurrentFrame.FrameTimeMs > Thresholds.CriticalFrameTimeMs)
    {
        FString Warning = FString::Printf(TEXT("CRITICAL: Frame time %.2fms exceeds critical threshold %.2fms"),
            CurrentFrame.FrameTimeMs, Thresholds.CriticalFrameTimeMs);
        CurrentWarnings.Add(Warning);
        OnPerformanceWarning.Broadcast(EPerformanceCategory::Rendering, Warning);
    }
    else if (CurrentFrame.FrameTimeMs > Thresholds.WarningFrameTimeMs)
    {
        FString Warning = FString::Printf(TEXT("WARNING: Frame time %.2fms exceeds warning threshold %.2fms"),
            CurrentFrame.FrameTimeMs, Thresholds.WarningFrameTimeMs);
        CurrentWarnings.Add(Warning);
    }
    
    // Check memory usage
    if (CurrentFrame.MemoryUsedMB > Thresholds.CriticalMemoryMB)
    {
        FString Warning = FString::Printf(TEXT("CRITICAL: Memory usage %.2fMB exceeds critical threshold %.2fMB"),
            CurrentFrame.MemoryUsedMB, Thresholds.CriticalMemoryMB);
        CurrentWarnings.Add(Warning);
        OnPerformanceWarning.Broadcast(EPerformanceCategory::Custom, Warning);
    }
    else if (CurrentFrame.MemoryUsedMB > Thresholds.WarningMemoryMB)
    {
        FString Warning = FString::Printf(TEXT("WARNING: Memory usage %.2fMB exceeds warning threshold %.2fMB"),
            CurrentFrame.MemoryUsedMB, Thresholds.WarningMemoryMB);
        CurrentWarnings.Add(Warning);
    }
    
    // Check draw calls (if available)
    if (CurrentFrame.DrawCalls > Thresholds.CriticalDrawCalls)
    {
        FString Warning = FString::Printf(TEXT("CRITICAL: Draw calls %d exceed critical threshold %d"),
            CurrentFrame.DrawCalls, Thresholds.CriticalDrawCalls);
        CurrentWarnings.Add(Warning);
        OnPerformanceWarning.Broadcast(EPerformanceCategory::Rendering, Warning);
    }
    else if (CurrentFrame.DrawCalls > Thresholds.WarningDrawCalls)
    {
        FString Warning = FString::Printf(TEXT("WARNING: Draw calls %d exceed warning threshold %d"),
            CurrentFrame.DrawCalls, Thresholds.WarningDrawCalls);
        CurrentWarnings.Add(Warning);
    }
}

void UPerformanceProfiler::UpdateOverlay()
{
    // This would display on-screen stats
    // In a real implementation, this would use Canvas or UMG
    FAlexanderFrameMetrics AvgMetrics = GetAverageFrameMetrics();
    
    UE_LOG(LogTemp, Display, TEXT("=== Performance Stats ==="));
    UE_LOG(LogTemp, Display, TEXT("FPS: %.1f | Frame: %.2fms | Memory: %.1fMB | Actors: %d"),
        AvgMetrics.FPS, AvgMetrics.FrameTimeMs, AvgMetrics.MemoryUsedMB, AvgMetrics.ActiveActors);
    
    if (CurrentWarnings.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Active Warnings: %d"), CurrentWarnings.Num());
    }
}

void UPerformanceProfiler::AutoExport()
{
    FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
    FString Filename = FString::Printf(TEXT("AutoExport_%s"), *Timestamp);
    ExportToCSV(Filename);
}

FString UPerformanceProfiler::GetExportFilePath(const FString& Filename) const
{
    FString Directory = ExportDirectory;
    if (!Directory.EndsWith(TEXT("/")))
    {
        Directory += TEXT("/");
    }
    
    // Create directory if it doesn't exist
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*Directory))
    {
        PlatformFile.CreateDirectoryTree(*Directory);
    }
    
    return Directory + Filename;
}

void UPerformanceProfiler::TrimFrameHistory()
{
    if (FrameHistory.Num() > MaxStoredFrames)
    {
        int32 NumToRemove = FrameHistory.Num() - MaxStoredFrames;
        FrameHistory.RemoveAt(0, NumToRemove);
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
    for (const FAlexanderFrameMetrics& Frame : FrameHistory)
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

TArray<FSystemPerformanceData> UPerformanceProfiler::GetSystemPerformanceData() const
{
    TArray<FSystemPerformanceData> Result;
    SystemData.GenerateValueArray(Result);
    return Result;
}

FSystemPerformanceData UPerformanceProfiler::GetSystemData(const FString& SystemName) const
{
    const FSystemPerformanceData* Data = SystemData.Find(SystemName);
    return Data ? *Data : FSystemPerformanceData();
}

float UPerformanceProfiler::GetAverageFPS() const
{
    return GetAverageFrameMetrics().FPS;
}

float UPerformanceProfiler::GetCurrentFPS() const
{
    return CurrentFrame.FPS;
}

float UPerformanceProfiler::GetMemoryUsageMB() const
{
    return CurrentFrame.MemoryUsedMB;
}

bool UPerformanceProfiler::ExportToCSV(const FString& Filename)
{
    FString FilePath = GetExportFilePath(Filename + TEXT(".csv"));
    
    FString CSVContent = TEXT("Frame,FrameTimeMs,FPS,MemoryMB,DrawCalls,Triangles,ActiveActors\n");
    
    for (int32 i = 0; i < FrameHistory.Num(); ++i)
    {
        const FAlexanderFrameMetrics& Frame = FrameHistory[i];
        CSVContent += FString::Printf(TEXT("%d,%.2f,%.1f,%.1f,%d,%d,%d\n"),
            i, Frame.FrameTimeMs, Frame.FPS, Frame.MemoryUsedMB,
            Frame.DrawCalls, Frame.Triangles, Frame.ActiveActors);
    }
    
    bool bSuccess = FFileHelper::SaveStringToFile(CSVContent, *FilePath);
    
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Performance Profiler: Exported CSV to %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Performance Profiler: Failed to export CSV to %s"), *FilePath);
    }
    
    return bSuccess;
}

bool UPerformanceProfiler::ExportSystemReport(const FString& Filename)
{
    FString FilePath = GetExportFilePath(Filename + TEXT(".txt"));
    FString Report = GeneratePerformanceReport();
    
    bool bSuccess = FFileHelper::SaveStringToFile(Report, *FilePath);
    
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Performance Profiler: Exported report to %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Performance Profiler: Failed to export report to %s"), *FilePath);
    }
    
    return bSuccess;
}

FString UPerformanceProfiler::GeneratePerformanceReport() const
{
    FString Report = TEXT("====================================\n");
    Report += TEXT("   PERFORMANCE PROFILING REPORT\n");
    Report += TEXT("====================================\n\n");
    
    // Time information
    Report += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Profiling Level: %d\n"), (int32)ProfilingLevel);
    Report += FString::Printf(TEXT("Frames Captured: %d\n\n"), FrameHistory.Num());
    
    // Average metrics
    FAlexanderFrameMetrics Avg = GetAverageFrameMetrics();
    Report += TEXT("--- Average Frame Metrics ---\n");
    Report += FString::Printf(TEXT("FPS: %.1f\n"), Avg.FPS);
    Report += FString::Printf(TEXT("Frame Time: %.2fms\n"), Avg.FrameTimeMs);
    Report += FString::Printf(TEXT("Memory Usage: %.1fMB\n"), Avg.MemoryUsedMB);
    Report += FString::Printf(TEXT("Active Actors: %d\n"), Avg.ActiveActors);
    Report += FString::Printf(TEXT("Draw Calls: %d\n"), Avg.DrawCalls);
    Report += FString::Printf(TEXT("Triangles: %d\n\n"), Avg.Triangles);
    
    // System performance
    if (SystemData.Num() > 0)
    {
        Report += TEXT("--- System Performance ---\n");
        
        TArray<FSystemPerformanceData> Systems;
        SystemData.GenerateValueArray(Systems);
        
        // Sort by average tick time
        Systems.Sort([](const FSystemPerformanceData& A, const FSystemPerformanceData& B) {
            return A.AverageTickTimeMs > B.AverageTickTimeMs;
        });
        
        for (const FSystemPerformanceData& System : Systems)
        {
            Report += FString::Printf(TEXT("\n%s:\n"), *System.SystemName);
            Report += FString::Printf(TEXT("  Avg Tick: %.3fms\n"), System.AverageTickTimeMs);
            Report += FString::Printf(TEXT("  Min Tick: %.3fms\n"), System.MinTickTimeMs);
            Report += FString::Printf(TEXT("  Max Tick: %.3fms\n"), System.MaxTickTimeMs);
            Report += FString::Printf(TEXT("  Tick Count: %d\n"), System.TickCount);
        }
    }
    
    // Performance warnings
    if (CurrentWarnings.Num() > 0)
    {
        Report += TEXT("\n--- Current Warnings ---\n");
        for (const FString& Warning : CurrentWarnings)
        {
            Report += Warning + TEXT("\n");
        }
    }
    
    // Bottleneck analysis
    TArray<FString> Bottlenecks = GetBottleneckSystems(5);
    if (Bottlenecks.Num() > 0)
    {
        Report += TEXT("\n--- Top 5 Bottleneck Systems ---\n");
        for (int32 i = 0; i < Bottlenecks.Num(); ++i)
        {
            Report += FString::Printf(TEXT("%d. %s\n"), i + 1, *Bottlenecks[i]);
        }
    }
    
    Report += TEXT("\n====================================\n");
    return Report;
}

TArray<FString> UPerformanceProfiler::GetBottleneckSystems(int32 TopN) const
{
    TArray<FSystemPerformanceData> Systems;
    SystemData.GenerateValueArray(Systems);
    
    // Sort by average tick time descending
    Systems.Sort([](const FSystemPerformanceData& A, const FSystemPerformanceData& B) {
        return A.AverageTickTimeMs > B.AverageTickTimeMs;
    });
    
    TArray<FString> Result;
    for (int32 i = 0; i < FMath::Min(TopN, Systems.Num()); ++i)
    {
        Result.Add(FString::Printf(TEXT("%s (%.3fms avg)"),
            *Systems[i].SystemName, Systems[i].AverageTickTimeMs));
    }
    
    return Result;
}

bool UPerformanceProfiler::IsPerformanceHealthy() const
{
    FAlexanderFrameMetrics Avg = GetAverageFrameMetrics();
    
    // Check if metrics are within acceptable ranges
    bool bFrameTimeOk = Avg.FrameTimeMs <= Thresholds.WarningFrameTimeMs;
    bool bMemoryOk = Avg.MemoryUsedMB <= Thresholds.WarningMemoryMB;
    bool bDrawCallsOk = Avg.DrawCalls <= Thresholds.WarningDrawCalls;
    
    return bFrameTimeOk && bMemoryOk && bDrawCallsOk;
}

TArray<FString> UPerformanceProfiler::GetPerformanceWarnings() const
{
    return CurrentWarnings;
}

float UPerformanceProfiler::GetPlatformMemoryUsage() const
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    return MemoryStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
}

int32 UPerformanceProfiler::GetDrawCallCount() const
{
    // This would require RHI stats access
    // Placeholder implementation
    return 0;
}

int32 UPerformanceProfiler::GetTriangleCount() const
{
    // This would require RHI stats access
    // Placeholder implementation
    return 0;
}

float UPerformanceProfiler::GetGPUTime() const
{
    // This would require GPU profiling stats
    // Placeholder implementation
    return 0.0f;
}
