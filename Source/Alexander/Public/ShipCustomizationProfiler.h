// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShipCustomizationProfiler.generated.h"

/**
 * Performance profiling metrics for ship customization
 */
USTRUCT(BlueprintType)
struct FCustomizationProfilingMetrics
{
	GENERATED_BODY()

	// CPU Timing Metrics (in milliseconds)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CPU")
	float CalculateStatsTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CPU")
	float ApplyStatsTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CPU")
	float EquipPartTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CPU")
	float LoadLoadoutTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CPU")
	float UpdateVisualsTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CPU")
	float TotalFrameTime = 0.0f;

	// Memory Metrics (in bytes)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
	int64 LoadoutMemoryUsage = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
	int64 UnlockedPartsMemoryUsage = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
	int64 DataTableMemoryUsage = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
	int64 TotalMemoryUsage = 0;

	// Network Metrics (in bytes)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
	int32 LoadoutReplicationBytes = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
	int32 NetworkUpdateCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Network")
	float AverageReplicationRate = 0.0f;

	// Frame Impact Metrics
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	float FrameTimeImpact = 0.0f; // Percentage of frame budget used

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	int32 SampleCount = 0;

	// Performance Budget Flags
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	bool bWithinCPUBudget = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	bool bWithinMemoryBudget = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	bool bWithinNetworkBudget = true;
};

/**
 * Performance budget thresholds
 */
USTRUCT(BlueprintType)
struct FPerformanceBudgets
{
	GENERATED_BODY()

	// CPU budget (milliseconds per frame)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budgets")
	float MaxCPUTimePerFrame = 0.5f; // 0.5ms target (0.03% of 16.67ms frame)

	// Memory budget (bytes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budgets")
	int64 MaxMemoryUsage = 1048576; // 1 MB target

	// Network budget (bytes per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budgets")
	int32 MaxNetworkBytesPerSecond = 10240; // 10 KB/s target
};

/**
 * Profiling session data
 */
USTRUCT(BlueprintType)
struct FProfilingSession
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Session")
	FString SessionName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Session")
	FDateTime StartTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Session")
	FDateTime EndTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Session")
	TArray<FCustomizationProfilingMetrics> Samples;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Session")
	FCustomizationProfilingMetrics AverageMetrics;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Session")
	FCustomizationProfilingMetrics PeakMetrics;
};

/**
 * Ship Customization Profiler Component
 * Tracks performance metrics for ship customization operations
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Profiling), meta = (BlueprintSpawnableComponent))
class ALEXANDER_API UShipCustomizationProfiler : public UActorComponent
{
	GENERATED_BODY()

public:
	UShipCustomizationProfiler();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ============================================================================
	// PROFILING CONFIGURATION
	// ============================================================================

	// Enable profiling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
	bool bProfilingEnabled = true;

	// Enable detailed logging
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
	bool bDetailedLogging = false;

	// Enable real-time display
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
	bool bRealTimeDisplay = true;

	// Performance budgets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
	FPerformanceBudgets PerformanceBudgets;

	// ============================================================================
	// CURRENT METRICS
	// ============================================================================

	// Current frame metrics
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics")
	FCustomizationProfilingMetrics CurrentMetrics;

	// Running average metrics
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics")
	FCustomizationProfilingMetrics AverageMetrics;

	// Peak metrics
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Metrics")
	FCustomizationProfilingMetrics PeakMetrics;

	// ============================================================================
	// PROFILING SESSION
	// ============================================================================

	// Current profiling session
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Session")
	FProfilingSession CurrentSession;

	// Start a new profiling session
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	void StartProfilingSession(const FString& SessionName);

	// End current profiling session
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	void EndProfilingSession();

	// Export session data to CSV
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	bool ExportSessionToCSV(const FString& FilePath);

	// ============================================================================
	// TIMING MEASUREMENTS
	// ============================================================================

	// Begin timing a specific operation
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	void BeginTiming(const FString& OperationName);

	// End timing a specific operation
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	void EndTiming(const FString& OperationName);

	// Record CalculateStats execution time
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	void RecordCalculateStatsTime(float TimeMS);

	// Record ApplyStats execution time
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	void RecordApplyStatsTime(float TimeMS);

	// Record EquipPart execution time
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	void RecordEquipPartTime(float TimeMS);

	// Record LoadLoadout execution time
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	void RecordLoadLoadoutTime(float TimeMS);

	// Record UpdateVisuals execution time
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	void RecordUpdateVisualsTime(float TimeMS);

	// ============================================================================
	// MEMORY MEASUREMENTS
	// ============================================================================

	// Calculate loadout memory usage
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	int64 CalculateLoadoutMemory();

	// Calculate unlocked parts memory usage
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	int64 CalculateUnlockedPartsMemory();

	// Calculate data table memory usage
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	int64 CalculateDataTableMemory();

	// Update all memory metrics
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	void UpdateMemoryMetrics();

	// ============================================================================
	// NETWORK MEASUREMENTS
	// ============================================================================

	// Record network replication event
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	void RecordNetworkReplication(int32 BytesSent);

	// Get average replication rate
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	float GetAverageReplicationRate() const;

	// ============================================================================
	// FRAME IMPACT MEASUREMENTS
	// ============================================================================

	// Calculate frame time impact
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	float CalculateFrameTimeImpact() const;

	// Check if within performance budgets
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	void CheckPerformanceBudgets();

	// ============================================================================
	// REPORTING
	// ============================================================================

	// Get profiling summary
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	FString GetProfilingSummary() const;

	// Get detailed report
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	FString GetDetailedReport() const;

	// Log current metrics
	UFUNCTION(BlueprintCallable, Category = "Profiling")
	void LogCurrentMetrics() const;

	// ============================================================================
	// AUTOMATIC INTEGRATION
	// ============================================================================

	// Get ShipCustomizationComponent reference
	class UShipCustomizationComponent* GetCustomizationComponent() const;

protected:
	// Update running averages
	void UpdateAverageMetrics();

	// Update peak metrics
	void UpdatePeakMetrics();

	// Reset current metrics
	void ResetCurrentMetrics();

	// Timing state
	TMap<FString, double> TimingStartTimes;

	// Network tracking
	float NetworkBytesAccumulator = 0.0f;
	float NetworkTrackingTime = 0.0f;

	// Sample tracking
	int32 TotalSamples = 0;

	// Cached component reference
	UPROPERTY()
	mutable class UShipCustomizationComponent* CachedCustomizationComponent;
};
