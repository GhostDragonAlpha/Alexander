// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PerformanceProfiler.generated.h"

/**
 * Profiling level determines the amount of data collected
 */
UENUM(BlueprintType)
enum class EProfilingLevel : uint8
{
    None        UMETA(DisplayName = "None - Disabled"),
    Basic       UMETA(DisplayName = "Basic - Frame Time Only"),
    Detailed    UMETA(DisplayName = "Detailed - Per-System Metrics"),
    Exhaustive  UMETA(DisplayName = "Exhaustive - Full Profiling")
};

/**
 * Performance category for organizing metrics
 */
UENUM(BlueprintType)
enum class EPerformanceCategory : uint8
{
    Rendering       UMETA(DisplayName = "Rendering"),
    Physics         UMETA(DisplayName = "Physics"),
    Networking      UMETA(DisplayName = "Networking"),
    AI              UMETA(DisplayName = "AI"),
    Gameplay        UMETA(DisplayName = "Gameplay"),
    Audio           UMETA(DisplayName = "Audio"),
    UI              UMETA(DisplayName = "UI"),
    Streaming       UMETA(DisplayName = "Streaming"),
    Custom          UMETA(DisplayName = "Custom")
};

/**
 * Stores performance metrics for a single frame
 */
USTRUCT(BlueprintType)
struct FAlexanderFrameMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeMs;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTimeMs;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTimeMs;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTimeMs;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsedMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FPS;

    FAlexanderFrameMetrics()
        : FrameTimeMs(0.0f)
        , GameThreadTimeMs(0.0f)
        , RenderThreadTimeMs(0.0f)
        , GPUTimeMs(0.0f)
        , DrawCalls(0)
        , Triangles(0)
        , MemoryUsedMB(0.0f)
        , ActiveActors(0)
        , FPS(0.0f)
    {}
};

/**
 * System-specific performance data
 */
USTRUCT(BlueprintType)
struct FPerformanceProfilerData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageTickTimeMs;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxTickTimeMs;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinTickTimeMs;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TickCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerformanceCategory Category;

    FPerformanceProfilerData()
        : SystemName("")
        , AverageTickTimeMs(0.0f)
        , MaxTickTimeMs(0.0f)
        , MinTickTimeMs(FLT_MAX)
        , TickCount(0)
        , MemoryUsageMB(0.0f)
        , Category(EPerformanceCategory::Custom)
    {}
};

/**
 * Performance warning threshold
 */
USTRUCT(BlueprintType)
struct FPerformanceThreshold
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float WarningFrameTimeMs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CriticalFrameTimeMs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float WarningMemoryMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CriticalMemoryMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 WarningDrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 CriticalDrawCalls;

    FPerformanceThreshold()
        : WarningFrameTimeMs(11.1f)    // ~90 FPS
        , CriticalFrameTimeMs(16.6f)    // ~60 FPS
        , WarningMemoryMB(2048.0f)      // 2 GB
        , CriticalMemoryMB(4096.0f)     // 4 GB
        , WarningDrawCalls(2000)
        , CriticalDrawCalls(5000)
    {}
};

/**
 * Delegate for performance warnings
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPerformanceWarning, EPerformanceCategory, Category, const FString&, Message);

/**
 * Performance Profiler Component
 * Tracks and analyzes performance metrics across the game
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UPerformanceProfiler : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerformanceProfiler();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling|Settings")
    EProfilingLevel ProfilingLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling|Settings")
    bool bEnableAutoExport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling|Settings")
    float AutoExportInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling|Settings")
    FString ExportDirectory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling|Settings")
    FPerformanceThreshold Thresholds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling|Settings")
    bool bDisplayOverlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling|Settings")
    float OverlayUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling|Settings")
    int32 MaxStoredFrames;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Profiling|Events")
    FOnPerformanceWarning OnPerformanceWarning;

    // Profiling Control
    UFUNCTION(BlueprintCallable, Category = "Profiling|Control")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Profiling|Control")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Profiling|Control")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Profiling|Control")
    void SetProfilingLevel(EProfilingLevel NewLevel);

    // System Registration
    UFUNCTION(BlueprintCallable, Category = "Profiling|Systems")
    void RegisterSystem(const FString& SystemName, EPerformanceCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Profiling|Systems")
    void UnregisterSystem(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Profiling|Systems")
    void BeginSystemTick(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Profiling|Systems")
    void EndSystemTick(const FString& SystemName);

    // Data Retrieval
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Profiling|Data")
    FAlexanderFrameMetrics GetCurrentFrameMetrics() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Profiling|Data")
    FAlexanderFrameMetrics GetAverageFrameMetrics() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Profiling|Data")
    TArray<FPerformanceProfilerData> GetSystemPerformanceData() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Profiling|Data")
    FPerformanceProfilerData GetSystemData(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Profiling|Data")
    float GetAverageFPS() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Profiling|Data")
    float GetCurrentFPS() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Profiling|Data")
    float GetMemoryUsageMB() const;

    // Export Functions
    UFUNCTION(BlueprintCallable, Category = "Profiling|Export")
    bool ExportToCSV(const FString& Filename);

    UFUNCTION(BlueprintCallable, Category = "Profiling|Export")
    bool ExportSystemReport(const FString& Filename);

    UFUNCTION(BlueprintCallable, Category = "Profiling|Export")
    FString GeneratePerformanceReport() const;

    // Analysis Functions
    UFUNCTION(BlueprintCallable, Category = "Profiling|Analysis")
    TArray<FString> GetBottleneckSystems(int32 TopN = 5) const;

    UFUNCTION(BlueprintCallable, Category = "Profiling|Analysis")
    bool IsPerformanceHealthy() const;

    UFUNCTION(BlueprintCallable, Category = "Profiling|Analysis")
    TArray<FString> GetPerformanceWarnings() const;

private:
    // Internal state
    bool bIsProfiling;
    float TimeSinceLastExport;
    float TimeSinceLastOverlayUpdate;
    
    // Frame data storage
    TArray<FAlexanderFrameMetrics> FrameHistory;
    FAlexanderFrameMetrics CurrentFrame;
    
    // System data storage
    TMap<FString, FPerformanceProfilerData> SystemData;
    TMap<FString, double> SystemTickStartTimes;
    TMap<FString, EPerformanceCategory> SystemCategories;
    
    // Performance tracking
    TArray<FString> CurrentWarnings;
    
    // Internal functions
    void CaptureFrameMetrics();
    void UpdateSystemMetrics(const FString& SystemName, float TickTime);
    void CheckPerformanceThresholds();
    void UpdateOverlay();
    void AutoExport();
    FString GetExportFilePath(const FString& Filename) const;
    void TrimFrameHistory();
    
    // Platform-specific metrics
    float GetPlatformMemoryUsage() const;
    int32 GetDrawCallCount() const;
    int32 GetTriangleCount() const;
    float GetGPUTime() const;
};
