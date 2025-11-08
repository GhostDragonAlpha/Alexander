// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShipCustomizationProfiler.h"
#include "ShipCustomizationComponent.h"
#include "HAL/PlatformTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Engine/DataTable.h"

UShipCustomizationProfiler::UShipCustomizationProfiler()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // Update metrics 10 times per second

	CachedCustomizationComponent = nullptr;

	// Set default budgets
	PerformanceBudgets.MaxCPUTimePerFrame = 0.5f;
	PerformanceBudgets.MaxMemoryUsage = 1048576; // 1 MB
	PerformanceBudgets.MaxNetworkBytesPerSecond = 10240; // 10 KB/s
}

void UShipCustomizationProfiler::BeginPlay()
{
	Super::BeginPlay();

	if (bProfilingEnabled)
	{
		UE_LOG(LogTemp, Log, TEXT("ShipCustomizationProfiler: Profiling enabled"));
		StartProfilingSession(TEXT("DefaultSession"));
	}
}

void UShipCustomizationProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bProfilingEnabled)
	{
		return;
	}

	// Update memory metrics
	UpdateMemoryMetrics();

	// Update network tracking
	NetworkTrackingTime += DeltaTime;

	// Calculate frame time impact
	CurrentMetrics.FrameTimeImpact = CalculateFrameTimeImpact();

	// Check performance budgets
	CheckPerformanceBudgets();

	// Update averages
	UpdateAverageMetrics();
	UpdatePeakMetrics();

	// Real-time display
	if (bRealTimeDisplay && TotalSamples % 10 == 0) // Every 10 samples
	{
		LogCurrentMetrics();
	}

	// Reset current frame metrics
	CurrentMetrics.TotalFrameTime = 0.0f;
	CurrentMetrics.SampleCount++;
	TotalSamples++;
}

// ============================================================================
// PROFILING SESSION
// ============================================================================

void UShipCustomizationProfiler::StartProfilingSession(const FString& SessionName)
{
	CurrentSession.SessionName = SessionName;
	CurrentSession.StartTime = FDateTime::Now();
	CurrentSession.Samples.Empty();

	ResetCurrentMetrics();
	AverageMetrics = FCustomizationProfilingMetrics();
	PeakMetrics = FCustomizationProfilingMetrics();

	TotalSamples = 0;

	UE_LOG(LogTemp, Log, TEXT("ShipCustomizationProfiler: Started session '%s'"), *SessionName);
}

void UShipCustomizationProfiler::EndProfilingSession()
{
	CurrentSession.EndTime = FDateTime::Now();

	// Calculate session averages and peaks
	CurrentSession.AverageMetrics = AverageMetrics;
	CurrentSession.PeakMetrics = PeakMetrics;

	FTimespan Duration = CurrentSession.EndTime - CurrentSession.StartTime;

	UE_LOG(LogTemp, Log, TEXT("ShipCustomizationProfiler: Ended session '%s'"), *CurrentSession.SessionName);
	UE_LOG(LogTemp, Log, TEXT("  Duration: %.2f seconds"), Duration.GetTotalSeconds());
	UE_LOG(LogTemp, Log, TEXT("  Samples: %d"), CurrentSession.Samples.Num());
	UE_LOG(LogTemp, Log, TEXT("  Average CPU Time: %.3f ms"), AverageMetrics.TotalFrameTime);
	UE_LOG(LogTemp, Log, TEXT("  Peak CPU Time: %.3f ms"), PeakMetrics.TotalFrameTime);
	UE_LOG(LogTemp, Log, TEXT("  Average Memory: %lld bytes (%.2f MB)"), AverageMetrics.TotalMemoryUsage, AverageMetrics.TotalMemoryUsage / (1024.0f * 1024.0f));
	UE_LOG(LogTemp, Log, TEXT("  Peak Memory: %lld bytes (%.2f MB)"), PeakMetrics.TotalMemoryUsage, PeakMetrics.TotalMemoryUsage / (1024.0f * 1024.0f));
}

bool UShipCustomizationProfiler::ExportSessionToCSV(const FString& FilePath)
{
	if (CurrentSession.Samples.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShipCustomizationProfiler: No samples to export"));
		return false;
	}

	FString CSVContent;

	// Header
	CSVContent += TEXT("Sample,CalculateStatsTime,ApplyStatsTime,EquipPartTime,LoadLoadoutTime,UpdateVisualsTime,TotalFrameTime,");
	CSVContent += TEXT("LoadoutMemory,UnlockedPartsMemory,DataTableMemory,TotalMemory,");
	CSVContent += TEXT("NetworkBytes,NetworkUpdateCount,ReplicationRate,FrameImpact,WithinCPUBudget,WithinMemoryBudget,WithinNetworkBudget\n");

	// Data rows
	for (int32 i = 0; i < CurrentSession.Samples.Num(); i++)
	{
		const FCustomizationProfilingMetrics& Sample = CurrentSession.Samples[i];

		CSVContent += FString::Printf(TEXT("%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,"),
			i,
			Sample.CalculateStatsTime,
			Sample.ApplyStatsTime,
			Sample.EquipPartTime,
			Sample.LoadLoadoutTime,
			Sample.UpdateVisualsTime,
			Sample.TotalFrameTime);

		CSVContent += FString::Printf(TEXT("%lld,%lld,%lld,%lld,"),
			Sample.LoadoutMemoryUsage,
			Sample.UnlockedPartsMemoryUsage,
			Sample.DataTableMemoryUsage,
			Sample.TotalMemoryUsage);

		CSVContent += FString::Printf(TEXT("%d,%d,%.2f,%.2f,%d,%d,%d\n"),
			Sample.LoadoutReplicationBytes,
			Sample.NetworkUpdateCount,
			Sample.AverageReplicationRate,
			Sample.FrameTimeImpact,
			Sample.bWithinCPUBudget ? 1 : 0,
			Sample.bWithinMemoryBudget ? 1 : 0,
			Sample.bWithinNetworkBudget ? 1 : 0);
	}

	// Save to file
	FString FullPath = FPaths::ProjectSavedDir() / TEXT("Profiling") / FilePath;
	bool bSuccess = FFileHelper::SaveStringToFile(CSVContent, *FullPath);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("ShipCustomizationProfiler: Exported session to %s"), *FullPath);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ShipCustomizationProfiler: Failed to export session to %s"), *FullPath);
	}

	return bSuccess;
}

// ============================================================================
// TIMING MEASUREMENTS
// ============================================================================

void UShipCustomizationProfiler::BeginTiming(const FString& OperationName)
{
	if (!bProfilingEnabled)
	{
		return;
	}

	TimingStartTimes.Add(OperationName, FPlatformTime::Seconds());
}

void UShipCustomizationProfiler::EndTiming(const FString& OperationName)
{
	if (!bProfilingEnabled)
	{
		return;
	}

	double* StartTime = TimingStartTimes.Find(OperationName);
	if (!StartTime)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShipCustomizationProfiler: No start time found for operation '%s'"), *OperationName);
		return;
	}

	double ElapsedSeconds = FPlatformTime::Seconds() - *StartTime;
	float ElapsedMS = static_cast<float>(ElapsedSeconds * 1000.0);

	// Log if detailed logging is enabled
	if (bDetailedLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("ShipCustomizationProfiler: %s took %.3f ms"), *OperationName, ElapsedMS);
	}

	// Remove timing entry
	TimingStartTimes.Remove(OperationName);
}

void UShipCustomizationProfiler::RecordCalculateStatsTime(float TimeMS)
{
	if (!bProfilingEnabled)
	{
		return;
	}

	CurrentMetrics.CalculateStatsTime = TimeMS;
	CurrentMetrics.TotalFrameTime += TimeMS;

	if (bDetailedLogging)
	{
		UE_LOG(LogTemp, Verbose, TEXT("ShipCustomizationProfiler: CalculateStats = %.3f ms"), TimeMS);
	}
}

void UShipCustomizationProfiler::RecordApplyStatsTime(float TimeMS)
{
	if (!bProfilingEnabled)
	{
		return;
	}

	CurrentMetrics.ApplyStatsTime = TimeMS;
	CurrentMetrics.TotalFrameTime += TimeMS;

	if (bDetailedLogging)
	{
		UE_LOG(LogTemp, Verbose, TEXT("ShipCustomizationProfiler: ApplyStats = %.3f ms"), TimeMS);
	}
}

void UShipCustomizationProfiler::RecordEquipPartTime(float TimeMS)
{
	if (!bProfilingEnabled)
	{
		return;
	}

	CurrentMetrics.EquipPartTime = TimeMS;
	CurrentMetrics.TotalFrameTime += TimeMS;

	if (bDetailedLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("ShipCustomizationProfiler: EquipPart = %.3f ms"), TimeMS);
	}
}

void UShipCustomizationProfiler::RecordLoadLoadoutTime(float TimeMS)
{
	if (!bProfilingEnabled)
	{
		return;
	}

	CurrentMetrics.LoadLoadoutTime = TimeMS;
	CurrentMetrics.TotalFrameTime += TimeMS;

	if (bDetailedLogging)
	{
		UE_LOG(LogTemp, Log, TEXT("ShipCustomizationProfiler: LoadLoadout = %.3f ms"), TimeMS);
	}
}

void UShipCustomizationProfiler::RecordUpdateVisualsTime(float TimeMS)
{
	if (!bProfilingEnabled)
	{
		return;
	}

	CurrentMetrics.UpdateVisualsTime = TimeMS;
	CurrentMetrics.TotalFrameTime += TimeMS;

	if (bDetailedLogging)
	{
		UE_LOG(LogTemp, Verbose, TEXT("ShipCustomizationProfiler: UpdateVisuals = %.3f ms"), TimeMS);
	}
}

// ============================================================================
// MEMORY MEASUREMENTS
// ============================================================================

int64 UShipCustomizationProfiler::CalculateLoadoutMemory()
{
	UShipCustomizationComponent* CustomizationComp = GetCustomizationComponent();
	if (!CustomizationComp)
	{
		return 0;
	}

	int64 TotalBytes = 0;

	// Size of FShipLoadout struct base
	TotalBytes += sizeof(FShipLoadout);

	// Size of EquippedParts TMap
	const auto& EquippedParts = CustomizationComp->CurrentLoadout.EquippedParts;
	TotalBytes += EquippedParts.Num() * (sizeof(EShipPartCategory) + sizeof(FName));

	// FText allocations (approximate)
	TotalBytes += 128; // Approximate for LoadoutName

	return TotalBytes;
}

int64 UShipCustomizationProfiler::CalculateUnlockedPartsMemory()
{
	UShipCustomizationComponent* CustomizationComp = GetCustomizationComponent();
	if (!CustomizationComp)
	{
		return 0;
	}

	int64 TotalBytes = 0;

	// UnlockedParts array
	const auto& UnlockedParts = CustomizationComp->ProgressionData.UnlockedParts;
	TotalBytes += UnlockedParts.Num() * sizeof(FName);

	// UnlockedSkins array
	const auto& UnlockedSkins = CustomizationComp->ProgressionData.UnlockedSkins;
	TotalBytes += UnlockedSkins.Num() * sizeof(FName);

	// SavedLoadouts array
	const auto& SavedLoadouts = CustomizationComp->ProgressionData.SavedLoadouts;
	for (const FShipLoadout& Loadout : SavedLoadouts)
	{
		TotalBytes += sizeof(FShipLoadout);
		TotalBytes += Loadout.EquippedParts.Num() * (sizeof(EShipPartCategory) + sizeof(FName));
		TotalBytes += 128; // Approximate for FText
	}

	return TotalBytes;
}

int64 UShipCustomizationProfiler::CalculateDataTableMemory()
{
	UShipCustomizationComponent* CustomizationComp = GetCustomizationComponent();
	if (!CustomizationComp)
	{
		return 0;
	}

	int64 TotalBytes = 0;

	// ShipPartsDataTable
	if (CustomizationComp->ShipPartsDataTable)
	{
		TArray<FName> RowNames = CustomizationComp->ShipPartsDataTable->GetRowNames();
		TotalBytes += RowNames.Num() * sizeof(FShipPartData);
	}

	// ShipSkinsDataTable
	if (CustomizationComp->ShipSkinsDataTable)
	{
		TArray<FName> RowNames = CustomizationComp->ShipSkinsDataTable->GetRowNames();
		TotalBytes += RowNames.Num() * sizeof(FShipSkinData);
	}

	return TotalBytes;
}

void UShipCustomizationProfiler::UpdateMemoryMetrics()
{
	if (!bProfilingEnabled)
	{
		return;
	}

	CurrentMetrics.LoadoutMemoryUsage = CalculateLoadoutMemory();
	CurrentMetrics.UnlockedPartsMemoryUsage = CalculateUnlockedPartsMemory();
	CurrentMetrics.DataTableMemoryUsage = CalculateDataTableMemory();
	CurrentMetrics.TotalMemoryUsage = CurrentMetrics.LoadoutMemoryUsage +
									   CurrentMetrics.UnlockedPartsMemoryUsage +
									   CurrentMetrics.DataTableMemoryUsage;
}

// ============================================================================
// NETWORK MEASUREMENTS
// ============================================================================

void UShipCustomizationProfiler::RecordNetworkReplication(int32 BytesSent)
{
	if (!bProfilingEnabled)
	{
		return;
	}

	CurrentMetrics.LoadoutReplicationBytes += BytesSent;
	CurrentMetrics.NetworkUpdateCount++;

	NetworkBytesAccumulator += BytesSent;

	if (bDetailedLogging)
	{
		UE_LOG(LogTemp, Verbose, TEXT("ShipCustomizationProfiler: Network replication = %d bytes"), BytesSent);
	}
}

float UShipCustomizationProfiler::GetAverageReplicationRate() const
{
	if (NetworkTrackingTime <= 0.0f)
	{
		return 0.0f;
	}

	return NetworkBytesAccumulator / NetworkTrackingTime; // Bytes per second
}

// ============================================================================
// FRAME IMPACT MEASUREMENTS
// ============================================================================

float UShipCustomizationProfiler::CalculateFrameTimeImpact() const
{
	// Assuming 60 FPS target = 16.67ms per frame
	const float TargetFrameTime = 16.67f;

	if (TargetFrameTime <= 0.0f)
	{
		return 0.0f;
	}

	return (CurrentMetrics.TotalFrameTime / TargetFrameTime) * 100.0f; // Percentage
}

void UShipCustomizationProfiler::CheckPerformanceBudgets()
{
	CurrentMetrics.bWithinCPUBudget = CurrentMetrics.TotalFrameTime <= PerformanceBudgets.MaxCPUTimePerFrame;
	CurrentMetrics.bWithinMemoryBudget = CurrentMetrics.TotalMemoryUsage <= PerformanceBudgets.MaxMemoryUsage;

	float ReplicationRate = GetAverageReplicationRate();
	CurrentMetrics.bWithinNetworkBudget = ReplicationRate <= PerformanceBudgets.MaxNetworkBytesPerSecond;
	CurrentMetrics.AverageReplicationRate = ReplicationRate;

	// Log warnings if budgets are exceeded
	if (!CurrentMetrics.bWithinCPUBudget && bDetailedLogging)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShipCustomizationProfiler: CPU budget exceeded! %.3f ms / %.3f ms"),
			CurrentMetrics.TotalFrameTime, PerformanceBudgets.MaxCPUTimePerFrame);
	}

	if (!CurrentMetrics.bWithinMemoryBudget && bDetailedLogging)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShipCustomizationProfiler: Memory budget exceeded! %lld bytes / %lld bytes"),
			CurrentMetrics.TotalMemoryUsage, PerformanceBudgets.MaxMemoryUsage);
	}

	if (!CurrentMetrics.bWithinNetworkBudget && bDetailedLogging)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShipCustomizationProfiler: Network budget exceeded! %.2f bytes/s / %d bytes/s"),
			ReplicationRate, PerformanceBudgets.MaxNetworkBytesPerSecond);
	}
}

// ============================================================================
// REPORTING
// ============================================================================

FString UShipCustomizationProfiler::GetProfilingSummary() const
{
	FString Summary;

	Summary += FString::Printf(TEXT("Ship Customization Profiling Summary\n"));
	Summary += FString::Printf(TEXT("=====================================\n\n"));
	Summary += FString::Printf(TEXT("CPU Timing:\n"));
	Summary += FString::Printf(TEXT("  Total Frame Time: %.3f ms (%.2f%% of frame budget)\n"),
		CurrentMetrics.TotalFrameTime, CurrentMetrics.FrameTimeImpact);
	Summary += FString::Printf(TEXT("  CalculateStats: %.3f ms\n"), CurrentMetrics.CalculateStatsTime);
	Summary += FString::Printf(TEXT("  ApplyStats: %.3f ms\n"), CurrentMetrics.ApplyStatsTime);
	Summary += FString::Printf(TEXT("  EquipPart: %.3f ms\n"), CurrentMetrics.EquipPartTime);
	Summary += FString::Printf(TEXT("  LoadLoadout: %.3f ms\n"), CurrentMetrics.LoadLoadoutTime);
	Summary += FString::Printf(TEXT("  UpdateVisuals: %.3f ms\n\n"), CurrentMetrics.UpdateVisualsTime);

	Summary += FString::Printf(TEXT("Memory Usage:\n"));
	Summary += FString::Printf(TEXT("  Total: %lld bytes (%.2f KB)\n"),
		CurrentMetrics.TotalMemoryUsage, CurrentMetrics.TotalMemoryUsage / 1024.0f);
	Summary += FString::Printf(TEXT("  Loadout: %lld bytes\n"), CurrentMetrics.LoadoutMemoryUsage);
	Summary += FString::Printf(TEXT("  Unlocked Parts: %lld bytes\n"), CurrentMetrics.UnlockedPartsMemoryUsage);
	Summary += FString::Printf(TEXT("  Data Tables: %lld bytes\n\n"), CurrentMetrics.DataTableMemoryUsage);

	Summary += FString::Printf(TEXT("Network:\n"));
	Summary += FString::Printf(TEXT("  Replication Rate: %.2f bytes/s\n"), CurrentMetrics.AverageReplicationRate);
	Summary += FString::Printf(TEXT("  Update Count: %d\n"), CurrentMetrics.NetworkUpdateCount);
	Summary += FString::Printf(TEXT("  Total Bytes: %d\n\n"), CurrentMetrics.LoadoutReplicationBytes);

	Summary += FString::Printf(TEXT("Performance Budgets:\n"));
	Summary += FString::Printf(TEXT("  CPU: %s (%.3f / %.3f ms)\n"),
		CurrentMetrics.bWithinCPUBudget ? TEXT("PASS") : TEXT("FAIL"),
		CurrentMetrics.TotalFrameTime, PerformanceBudgets.MaxCPUTimePerFrame);
	Summary += FString::Printf(TEXT("  Memory: %s (%lld / %lld bytes)\n"),
		CurrentMetrics.bWithinMemoryBudget ? TEXT("PASS") : TEXT("FAIL"),
		CurrentMetrics.TotalMemoryUsage, PerformanceBudgets.MaxMemoryUsage);
	Summary += FString::Printf(TEXT("  Network: %s (%.2f / %d bytes/s)\n"),
		CurrentMetrics.bWithinNetworkBudget ? TEXT("PASS") : TEXT("FAIL"),
		CurrentMetrics.AverageReplicationRate, PerformanceBudgets.MaxNetworkBytesPerSecond);

	return Summary;
}

FString UShipCustomizationProfiler::GetDetailedReport() const
{
	FString Report = GetProfilingSummary();

	Report += FString::Printf(TEXT("\n\nDetailed Statistics:\n"));
	Report += FString::Printf(TEXT("  Total Samples: %d\n"), TotalSamples);

	Report += FString::Printf(TEXT("\nAverage Metrics:\n"));
	Report += FString::Printf(TEXT("  CPU Time: %.3f ms\n"), AverageMetrics.TotalFrameTime);
	Report += FString::Printf(TEXT("  Memory: %lld bytes\n"), AverageMetrics.TotalMemoryUsage);

	Report += FString::Printf(TEXT("\nPeak Metrics:\n"));
	Report += FString::Printf(TEXT("  CPU Time: %.3f ms\n"), PeakMetrics.TotalFrameTime);
	Report += FString::Printf(TEXT("  Memory: %lld bytes\n"), PeakMetrics.TotalMemoryUsage);

	return Report;
}

void UShipCustomizationProfiler::LogCurrentMetrics() const
{
	UE_LOG(LogTemp, Log, TEXT("ShipCustomizationProfiler: CPU=%.3fms Mem=%lldKB Net=%.2fb/s Frame=%.2f%%"),
		CurrentMetrics.TotalFrameTime,
		CurrentMetrics.TotalMemoryUsage / 1024,
		CurrentMetrics.AverageReplicationRate,
		CurrentMetrics.FrameTimeImpact);
}

// ============================================================================
// AUTOMATIC INTEGRATION
// ============================================================================

UShipCustomizationComponent* UShipCustomizationProfiler::GetCustomizationComponent() const
{
	if (CachedCustomizationComponent)
	{
		return CachedCustomizationComponent;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	CachedCustomizationComponent = Owner->FindComponentByClass<UShipCustomizationComponent>();

	return CachedCustomizationComponent;
}

// ============================================================================
// PROTECTED HELPERS
// ============================================================================

void UShipCustomizationProfiler::UpdateAverageMetrics()
{
	if (TotalSamples == 0)
	{
		return;
	}

	float Alpha = 1.0f / FMath::Max(TotalSamples, 1);

	// Exponential moving average
	AverageMetrics.CalculateStatsTime = FMath::Lerp(AverageMetrics.CalculateStatsTime, CurrentMetrics.CalculateStatsTime, Alpha);
	AverageMetrics.ApplyStatsTime = FMath::Lerp(AverageMetrics.ApplyStatsTime, CurrentMetrics.ApplyStatsTime, Alpha);
	AverageMetrics.EquipPartTime = FMath::Lerp(AverageMetrics.EquipPartTime, CurrentMetrics.EquipPartTime, Alpha);
	AverageMetrics.LoadLoadoutTime = FMath::Lerp(AverageMetrics.LoadLoadoutTime, CurrentMetrics.LoadLoadoutTime, Alpha);
	AverageMetrics.UpdateVisualsTime = FMath::Lerp(AverageMetrics.UpdateVisualsTime, CurrentMetrics.UpdateVisualsTime, Alpha);
	AverageMetrics.TotalFrameTime = FMath::Lerp(AverageMetrics.TotalFrameTime, CurrentMetrics.TotalFrameTime, Alpha);

	AverageMetrics.TotalMemoryUsage = static_cast<int64>(FMath::Lerp(static_cast<float>(AverageMetrics.TotalMemoryUsage), static_cast<float>(CurrentMetrics.TotalMemoryUsage), Alpha));
	AverageMetrics.AverageReplicationRate = FMath::Lerp(AverageMetrics.AverageReplicationRate, CurrentMetrics.AverageReplicationRate, Alpha);

	// Store sample for session
	if (CurrentSession.SessionName.Len() > 0)
	{
		CurrentSession.Samples.Add(CurrentMetrics);
	}
}

void UShipCustomizationProfiler::UpdatePeakMetrics()
{
	PeakMetrics.CalculateStatsTime = FMath::Max(PeakMetrics.CalculateStatsTime, CurrentMetrics.CalculateStatsTime);
	PeakMetrics.ApplyStatsTime = FMath::Max(PeakMetrics.ApplyStatsTime, CurrentMetrics.ApplyStatsTime);
	PeakMetrics.EquipPartTime = FMath::Max(PeakMetrics.EquipPartTime, CurrentMetrics.EquipPartTime);
	PeakMetrics.LoadLoadoutTime = FMath::Max(PeakMetrics.LoadLoadoutTime, CurrentMetrics.LoadLoadoutTime);
	PeakMetrics.UpdateVisualsTime = FMath::Max(PeakMetrics.UpdateVisualsTime, CurrentMetrics.UpdateVisualsTime);
	PeakMetrics.TotalFrameTime = FMath::Max(PeakMetrics.TotalFrameTime, CurrentMetrics.TotalFrameTime);

	PeakMetrics.TotalMemoryUsage = FMath::Max(PeakMetrics.TotalMemoryUsage, CurrentMetrics.TotalMemoryUsage);
	PeakMetrics.LoadoutMemoryUsage = FMath::Max(PeakMetrics.LoadoutMemoryUsage, CurrentMetrics.LoadoutMemoryUsage);
	PeakMetrics.UnlockedPartsMemoryUsage = FMath::Max(PeakMetrics.UnlockedPartsMemoryUsage, CurrentMetrics.UnlockedPartsMemoryUsage);
	PeakMetrics.DataTableMemoryUsage = FMath::Max(PeakMetrics.DataTableMemoryUsage, CurrentMetrics.DataTableMemoryUsage);

	PeakMetrics.AverageReplicationRate = FMath::Max(PeakMetrics.AverageReplicationRate, CurrentMetrics.AverageReplicationRate);
}

void UShipCustomizationProfiler::ResetCurrentMetrics()
{
	CurrentMetrics = FCustomizationProfilingMetrics();
}
