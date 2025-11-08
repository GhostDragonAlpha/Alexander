// Copyright Alexander Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PerformanceProfilerSubsystem.generated.h"

/**
 * Profile data for a single system
 */
USTRUCT(BlueprintType)
struct FProfileData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName SystemName;

	UPROPERTY(BlueprintReadOnly)
	double AverageTimeMs = 0.0;

	UPROPERTY(BlueprintReadOnly)
	double MinTimeMs = 0.0;

	UPROPERTY(BlueprintReadOnly)
	double MaxTimeMs = 0.0;

	UPROPERTY(BlueprintReadOnly)
	int32 SampleCount = 0;

	UPROPERTY(BlueprintReadOnly)
	double TotalTimeMs = 0.0;

	UPROPERTY(BlueprintReadOnly)
	FDateTime StartTime;

	UPROPERTY(BlueprintReadOnly)
	FDateTime EndTime;
};

/**
 * Real-time profiling scope helper
 */
struct FProfileScope
{
	FProfileScope(class UPerformanceProfilerSubsystem* InProfiler, FName InSystemName);
	~FProfileScope();

private:
	class UPerformanceProfilerSubsystem* Profiler;
	FName SystemName;
	double StartTime;
};

/**
 * Performance profiling subsystem
 * Tracks frame time, memory, and system-specific metrics
 */
UCLASS()
class ALEXANDER_API UPerformanceProfilerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Manual profiling
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	void StartProfiling(FName SystemName);

	UFUNCTION(BlueprintCallable, Category = "Profiling")
	void StopProfiling(FName SystemName);

	// Scope-based profiling (C++ only)
	void RecordSample(FName SystemName, double TimeMs);

	// Data retrieval
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	FProfileData GetSystemProfile(FName SystemName) const;

	UFUNCTION(BlueprintCallable, Category = "Profiling")
	TArray<FProfileData> GetAllProfiles() const;

	UFUNCTION(BlueprintCallable, Category = "Profiling")
	TArray<FProfileData> GetTopBottlenecks(int32 Count = 10) const;

	// Reset
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	void ResetProfile(FName SystemName);

	UFUNCTION(BlueprintCallable, Category = "Profiling")
	void ResetAllProfiles();

	// Export
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	void ExportToJSON(const FString& FilePath);

	UFUNCTION(BlueprintCallable, Category = "Profiling")
	void LogTopBottlenecks(int32 Count = 10);

	// Memory tracking
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	int64 GetCurrentMemoryUsageMB() const;

	UFUNCTION(BlueprintCallable, Category = "Profiling")
	int64 GetPeakMemoryUsageMB() const;

	// Frame time tracking
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	double GetAverageFrameTimeMs() const { return AverageFrameTimeMs; }

	UFUNCTION(BlueprintCallable, Category = "Profiling")
	double GetCurrentFPS() const { return AverageFrameTimeMs > 0.0 ? 1000.0 / AverageFrameTimeMs : 0.0; }

	// Enable/disable profiling
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	void SetProfilingEnabled(bool bEnabled) { bProfilingEnabled = bEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Profiling")
	bool IsProfilingEnabled() const { return bProfilingEnabled; }

private:
	// Profile storage
	UPROPERTY()
	TMap<FName, FProfileData> Profiles;

	// Active profiling sessions
	TMap<FName, double> ActiveSessions;

	// Frame time tracking
	double AverageFrameTimeMs = 0.0;
	double FrameTimeSamples = 0.0;
	int32 FrameCount = 0;

	// Memory tracking
	int64 PeakMemoryBytes = 0;

	// State
	bool bProfilingEnabled = true;

	// Helper to update profile
	void UpdateProfile(FName SystemName, double TimeMs);
};

// Macro for easy profiling
#define PROFILE_SCOPE(Profiler, SystemName) FProfileScope ProfileScope##__LINE__(Profiler, SystemName)
