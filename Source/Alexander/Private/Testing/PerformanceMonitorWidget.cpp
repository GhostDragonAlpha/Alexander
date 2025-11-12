// Copyright Epic Games, Inc. All Rights Reserved.

#include "Testing/PerformanceMonitorWidget.h"
#include "Testing/TestLevelGameMode.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"
// #include "HeadMountedDisplayFunctionLibrary.h" // UE5.6: Moved or deprecated
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "RHI.h"

UPerformanceMonitorWidget::UPerformanceMonitorWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GraphHistorySize = 100;
	UpdateInterval = 0.016f;
	bStartVisible = true;
	bShowGraphs = true;
	bShowVRMetrics = true;
	VRTargetFPS = 90.0f;
	VRWarningFPS = 80.0f;
	VRCriticalFPS = 60.0f;
	TargetFPS = 60.0f;
	WarningFPS = 45.0f;
	CriticalFPS = 30.0f;
	MemoryWarningMB = 2048.0f;
	MemoryCriticalMB = 3072.0f;
	GraphLineThickness = 2.0f;

	bIsVisible = true;
	bIsVRMode = false;
	FrameHistoryIndex = 0;
	TotalFPS = 0.0f;
	MinFPSRecorded = FLT_MAX;
	MaxFPSRecorded = 0.0f;
	FrameCount = 0;
	StatisticsStartTime = 0.0f;
	TotalDroppedFrames = 0;
	LastFrameNumber = 0;
	TimeSinceLastUpdate = 0.0f;
	LastDeltaTime = 0.0f;
}

void UPerformanceMonitorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get game mode reference
	GameModeRef = Cast<ATestLevelGameMode>(UGameplayStatics::GetGameMode(this));

	// Check if VR mode
	// TODO: IsHeadMountedDisplayEnabled() is deprecated in UE5.6 - using stub (false) for tests
	bIsVRMode = false;

	// Hide VR metrics if not in VR mode
	if (VRMetricsBox && !bIsVRMode)
	{
		VRMetricsBox->SetVisibility(ESlateVisibility::Collapsed);
	}

	// Bind button events
	if (ToggleButton)
	{
		ToggleButton->OnClicked.AddDynamic(this, &UPerformanceMonitorWidget::OnToggleVisibilityClicked);
	}

	if (ResetStatsButton)
	{
		ResetStatsButton->OnClicked.AddDynamic(this, &UPerformanceMonitorWidget::OnResetStatsClicked);
	}

	if (SnapshotButton)
	{
		SnapshotButton->OnClicked.AddDynamic(this, &UPerformanceMonitorWidget::OnCaptureSnapshotClicked);
	}

	// Initialize history array
	FrameHistory.Reserve(GraphHistorySize);

	// Set initial visibility
	if (!bStartVisible)
	{
		HideMonitor();
	}

	// Initialize statistics
	ResetStatistics();
}

void UPerformanceMonitorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bIsVisible)
	{
		return;
	}

	LastDeltaTime = InDeltaTime;
	TimeSinceLastUpdate += InDeltaTime;

	if (TimeSinceLastUpdate >= UpdateInterval)
	{
		CaptureFrameData();
		CalculateStatistics();

		UpdateFPSDisplay();
		UpdateMemoryDisplay();
		UpdateThreadTimesDisplay();

		if (bIsVRMode && bShowVRMetrics)
		{
			UpdateVRMetricsDisplay();
		}

		if (bShowGraphs)
		{
			UpdateGraphs();
		}

		UpdateWarningLevel();

		TimeSinceLastUpdate = 0.0f;

		// Notify Blueprint
		OnMetricsUpdated(GetCurrentMetrics());
	}
}

void UPerformanceMonitorWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UPerformanceMonitorWidget::ShowMonitor()
{
	bIsVisible = true;
	SetVisibility(ESlateVisibility::Visible);
}

void UPerformanceMonitorWidget::HideMonitor()
{
	bIsVisible = false;
	SetVisibility(ESlateVisibility::Collapsed);
}

void UPerformanceMonitorWidget::ToggleMonitor()
{
	if (bIsVisible)
	{
		HideMonitor();
	}
	else
	{
		ShowMonitor();
	}
}

float UPerformanceMonitorWidget::GetCurrentFPS() const
{
	return CalculateFPS(LastDeltaTime);
}

float UPerformanceMonitorWidget::GetAverageFPS() const
{
	return FrameCount > 0 ? TotalFPS / FrameCount : 0.0f;
}

float UPerformanceMonitorWidget::GetMinFPS() const
{
	return MinFPSRecorded != FLT_MAX ? MinFPSRecorded : 0.0f;
}

float UPerformanceMonitorWidget::GetMaxFPS() const
{
	return MaxFPSRecorded;
}

float UPerformanceMonitorWidget::GetCurrentFrameTimeMS() const
{
	return LastDeltaTime * 1000.0f;
}

float UPerformanceMonitorWidget::GetMemoryUsageMB() const
{
	FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
	return MemoryStats.UsedPhysical / (1024.0f * 1024.0f);
}

EPerformanceWarningLevel UPerformanceMonitorWidget::GetWarningLevel() const
{
	return CurrentWarningLevel;
}

float UPerformanceMonitorWidget::GetMotionToPhotonLatency() const
{
	if (!bIsVRMode)
	{
		return 0.0f;
	}

	// This would require VR API to get actual motion-to-photon latency
	// For now, estimate based on frame time
	return GetCurrentFrameTimeMS();
}

int32 UPerformanceMonitorWidget::GetDroppedFrames() const
{
	return TotalDroppedFrames;
}

float UPerformanceMonitorWidget::GetReprojectionRatio() const
{
	if (!bIsVRMode)
	{
		return 0.0f;
	}

	// This would require VR API to get actual reprojection ratio
	// For now, estimate based on dropped frames
	return FrameCount > 0 ? (float)TotalDroppedFrames / FrameCount : 0.0f;
}

void UPerformanceMonitorWidget::ResetStatistics()
{
	TotalFPS = 0.0f;
	MinFPSRecorded = FLT_MAX;
	MaxFPSRecorded = 0.0f;
	FrameCount = 0;
	TotalDroppedFrames = 0;
	LastFrameNumber = GFrameCounter;
	StatisticsStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	ClearGraphHistory();
}

void UPerformanceMonitorWidget::CaptureSnapshot()
{
	FTestPerformanceMetrics Snapshot = GetCurrentMetrics();

	FString SnapshotText = FString::Printf(
		TEXT("=== Performance Snapshot ===\n")
		TEXT("Time: %s\n")
		TEXT("FPS: %.1f (avg: %.1f, min: %.1f, max: %.1f)\n")
		TEXT("Frame Time: %.2fms\n")
		TEXT("Game Thread: %.2fms\n")
		TEXT("Render Thread: %.2fms\n")
		TEXT("GPU: %.2fms\n")
		TEXT("Memory: %.1fMB\n"),
		*FDateTime::Now().ToString(),
		GetCurrentFPS(), GetAverageFPS(), GetMinFPS(), GetMaxFPS(),
		GetCurrentFrameTimeMS(),
		Snapshot.GameThreadTime,
		Snapshot.RenderThreadTime,
		Snapshot.GPUTime,
		GetMemoryUsageMB()
	);

	if (bIsVRMode)
	{
		SnapshotText += FString::Printf(
			TEXT("Motion-to-Photon: %.2fms\n")
			TEXT("Dropped Frames: %d\n")
			TEXT("Reprojection: %.2f%%\n"),
			GetMotionToPhotonLatency(),
			GetDroppedFrames(),
			GetReprojectionRatio() * 100.0f
		);
	}

	UE_LOG(LogTemp, Log, TEXT("%s"), *SnapshotText);
}

FTestPerformanceMetrics UPerformanceMonitorWidget::GetCurrentMetrics() const
{
	FTestPerformanceMetrics Metrics;

	Metrics.AverageFPS = GetAverageFPS();
	Metrics.MinFPS = GetMinFPS();
	Metrics.MaxFPS = GetMaxFPS();
	Metrics.AverageFrameTime = GetCurrentFrameTimeMS();

	Metrics.GameThreadTime = GetGameThreadTime();
	Metrics.RenderThreadTime = GetRenderThreadTime();
	Metrics.GPUTime = GetGPUTime();

	Metrics.MemoryUsageMB = GetMemoryUsageMB();

	Metrics.MotionToPhotonLatency = GetMotionToPhotonLatency();
	Metrics.DroppedFrames = GetDroppedFrames();
	Metrics.ReprojectionRatio = GetReprojectionRatio();

	float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Metrics.TestDurationSeconds = CurrentTime - StatisticsStartTime;

	return Metrics;
}

void UPerformanceMonitorWidget::SetGraphHistorySize(int32 NewSize)
{
	GraphHistorySize = FMath::Clamp(NewSize, 10, 500);
	FrameHistory.Reserve(GraphHistorySize);

	// Trim if necessary
	while (FrameHistory.Num() > GraphHistorySize)
	{
		FrameHistory.RemoveAt(0);
	}
}

void UPerformanceMonitorWidget::ClearGraphHistory()
{
	FrameHistory.Empty();
	FrameHistoryIndex = 0;
}

void UPerformanceMonitorWidget::UpdateFPSDisplay()
{
	float CurrentFPS = GetCurrentFPS();
	float AvgFPS = GetAverageFPS();
	float MinFPS = GetMinFPS();
	float MaxFPS = GetMaxFPS();

	if (CurrentFPSText)
	{
		CurrentFPSText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), CurrentFPS)));
		CurrentFPSText->SetColorAndOpacity(FSlateColor(GetFPSColor(CurrentFPS)));
	}

	if (AverageFPSText)
	{
		AverageFPSText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), AvgFPS)));
	}

	if (MinFPSText)
	{
		MinFPSText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), MinFPS)));
	}

	if (MaxFPSText)
	{
		MaxFPSText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), MaxFPS)));
	}

	if (FPSProgressBar)
	{
		float TargetFPS_Local = bIsVRMode ? VRTargetFPS : TargetFPS;
		float Progress = FMath::Clamp(CurrentFPS / TargetFPS_Local, 0.0f, 1.0f);
		FPSProgressBar->SetPercent(Progress);
		FPSProgressBar->SetFillColorAndOpacity(GetFPSColor(CurrentFPS));
	}
}

void UPerformanceMonitorWidget::UpdateMemoryDisplay()
{
	float MemoryMB = GetMemoryUsageMB();

	if (MemoryUsageText)
	{
		MemoryUsageText->SetText(FText::FromString(FString::Printf(TEXT("%.1f MB"), MemoryMB)));
		MemoryUsageText->SetColorAndOpacity(FSlateColor(GetMemoryColor(MemoryMB)));
	}

	if (MemoryProgressBar)
	{
		float Progress = FMath::Clamp(MemoryMB / MemoryCriticalMB, 0.0f, 1.0f);
		MemoryProgressBar->SetPercent(Progress);
		MemoryProgressBar->SetFillColorAndOpacity(GetMemoryColor(MemoryMB));
	}
}

void UPerformanceMonitorWidget::UpdateThreadTimesDisplay()
{
	if (FrameTimeText)
	{
		FrameTimeText->SetText(FText::FromString(FString::Printf(TEXT("%.2f ms"), GetCurrentFrameTimeMS())));
	}

	if (GameThreadTimeText)
	{
		GameThreadTimeText->SetText(FText::FromString(FString::Printf(TEXT("%.2f ms"), GetGameThreadTime())));
	}

	if (RenderThreadTimeText)
	{
		RenderThreadTimeText->SetText(FText::FromString(FString::Printf(TEXT("%.2f ms"), GetRenderThreadTime())));
	}

	if (GPUTimeText)
	{
		GPUTimeText->SetText(FText::FromString(FString::Printf(TEXT("%.2f ms"), GetGPUTime())));
	}
}

void UPerformanceMonitorWidget::UpdateVRMetricsDisplay()
{
	if (MotionToPhotonText)
	{
		MotionToPhotonText->SetText(FText::FromString(FString::Printf(TEXT("%.2f ms"), GetMotionToPhotonLatency())));
	}

	if (DroppedFramesText)
	{
		DroppedFramesText->SetText(FText::FromString(FString::Printf(TEXT("%d"), GetDroppedFrames())));
	}

	if (ReprojectionText)
	{
		ReprojectionText->SetText(FText::FromString(FString::Printf(TEXT("%.2f%%"), GetReprojectionRatio() * 100.0f)));
	}
}

void UPerformanceMonitorWidget::UpdateGraphs()
{
	DrawFPSGraph();
	DrawMemoryGraph();
	DrawFrameTimeGraph();
}

void UPerformanceMonitorWidget::UpdateWarningLevel()
{
	float CurrentFPS = GetCurrentFPS();
	float MemoryMB = GetMemoryUsageMB();

	EPerformanceWarningLevel NewLevel = EPerformanceWarningLevel::Normal;

	// Check FPS thresholds
	if (bIsVRMode)
	{
		if (CurrentFPS < VRCriticalFPS || MemoryMB > MemoryCriticalMB)
		{
			NewLevel = EPerformanceWarningLevel::Critical;
		}
		else if (CurrentFPS < VRWarningFPS || MemoryMB > MemoryWarningMB)
		{
			NewLevel = EPerformanceWarningLevel::Warning;
		}
	}
	else
	{
		if (CurrentFPS < CriticalFPS || MemoryMB > MemoryCriticalMB)
		{
			NewLevel = EPerformanceWarningLevel::Critical;
		}
		else if (CurrentFPS < WarningFPS || MemoryMB > MemoryWarningMB)
		{
			NewLevel = EPerformanceWarningLevel::Warning;
		}
	}

	CurrentWarningLevel = NewLevel;

	// Update warning display
	if (WarningBorder)
	{
		WarningBorder->SetBrushColor(GetWarningColor(CurrentWarningLevel));
	}

	if (WarningText)
	{
		FString WarningMessage;
		switch (CurrentWarningLevel)
		{
		case EPerformanceWarningLevel::Normal:
			WarningMessage = TEXT("Performance Normal");
			break;
		case EPerformanceWarningLevel::Warning:
			WarningMessage = TEXT("Performance Warning");
			break;
		case EPerformanceWarningLevel::Critical:
			WarningMessage = TEXT("Performance Critical!");
			break;
		}
		WarningText->SetText(FText::FromString(WarningMessage));
	}

	// Trigger event if warning level changed
	if (CurrentWarningLevel != LastWarningLevel && CurrentWarningLevel != EPerformanceWarningLevel::Normal)
	{
		FString Message = FString::Printf(TEXT("Performance warning: FPS %.1f, Memory %.1fMB"), CurrentFPS, MemoryMB);
		OnPerformanceWarning(CurrentWarningLevel, Message);
	}

	LastWarningLevel = CurrentWarningLevel;
}

void UPerformanceMonitorWidget::CaptureFrameData()
{
	FFrameData NewFrame;
	NewFrame.FPS = GetCurrentFPS();
	NewFrame.FrameTimeMS = GetCurrentFrameTimeMS();
	NewFrame.GameThreadMS = GetGameThreadTime();
	NewFrame.RenderThreadMS = GetRenderThreadTime();
	NewFrame.GPUMS = GetGPUTime();
	NewFrame.Timestamp = FDateTime::Now();

	// Add to history
	if (FrameHistory.Num() < GraphHistorySize)
	{
		FrameHistory.Add(NewFrame);
	}
	else
	{
		FrameHistory[FrameHistoryIndex] = NewFrame;
		FrameHistoryIndex = (FrameHistoryIndex + 1) % GraphHistorySize;
	}
}

void UPerformanceMonitorWidget::CalculateStatistics()
{
	float CurrentFPS = GetCurrentFPS();

	TotalFPS += CurrentFPS;
	FrameCount++;

	if (CurrentFPS < MinFPSRecorded)
	{
		MinFPSRecorded = CurrentFPS;
	}

	if (CurrentFPS > MaxFPSRecorded)
	{
		MaxFPSRecorded = CurrentFPS;
	}

	// Check for dropped frames in VR
	if (bIsVRMode)
	{
		int32 CurrentFrameNumber = GFrameCounter;
		int32 ExpectedFrames = CurrentFrameNumber - LastFrameNumber;
		if (ExpectedFrames > 1)
		{
			int32 DroppedThisFrame = ExpectedFrames - 1;
			TotalDroppedFrames += DroppedThisFrame;
			OnVRFrameDropped(TotalDroppedFrames);
		}
		LastFrameNumber = CurrentFrameNumber;
	}
}

float UPerformanceMonitorWidget::CalculateFPS(float DeltaTime) const
{
	return DeltaTime > 0.0f ? 1.0f / DeltaTime : 0.0f;
}

float UPerformanceMonitorWidget::GetGameThreadTime() const
{
	// Get game thread time from stats system
	return FPlatformTime::ToMilliseconds(GGameThreadTime);
}

float UPerformanceMonitorWidget::GetRenderThreadTime() const
{
	// Get render thread time from stats system
	return FPlatformTime::ToMilliseconds(GRenderThreadTime);
}

float UPerformanceMonitorWidget::GetGPUTime() const
{
	// Get GPU time from RHI (GGPUFrameTime deprecated in UE 5.6)
	uint32 GPUCycles = RHIGetGPUFrameCycles();
	return FPlatformTime::ToMilliseconds(GPUCycles);
}

void UPerformanceMonitorWidget::DrawFPSGraph()
{
	// Graph drawing is best handled in Blueprint using Canvas or custom Slate widgets
	// Base implementation is empty - Blueprint can override
}

void UPerformanceMonitorWidget::DrawMemoryGraph()
{
	// Graph drawing is best handled in Blueprint using Canvas or custom Slate widgets
}

void UPerformanceMonitorWidget::DrawFrameTimeGraph()
{
	// Graph drawing is best handled in Blueprint using Canvas or custom Slate widgets
}

FLinearColor UPerformanceMonitorWidget::GetFPSColor(float FPS) const
{
	float TargetFPS_Local = bIsVRMode ? VRTargetFPS : TargetFPS;
	float WarningFPS_Local = bIsVRMode ? VRWarningFPS : WarningFPS;
	float CriticalFPS_Local = bIsVRMode ? VRCriticalFPS : CriticalFPS;

	if (FPS >= TargetFPS_Local)
	{
		return NormalColor;
	}
	else if (FPS >= WarningFPS_Local)
	{
		return WarningColor;
	}
	else
	{
		return CriticalColor;
	}
}

FLinearColor UPerformanceMonitorWidget::GetMemoryColor(float MemoryMB) const
{
	if (MemoryMB < MemoryWarningMB)
	{
		return NormalColor;
	}
	else if (MemoryMB < MemoryCriticalMB)
	{
		return WarningColor;
	}
	else
	{
		return CriticalColor;
	}
}

FLinearColor UPerformanceMonitorWidget::GetWarningColor(EPerformanceWarningLevel Level) const
{
	switch (Level)
	{
	case EPerformanceWarningLevel::Normal:
		return NormalColor;
	case EPerformanceWarningLevel::Warning:
		return WarningColor;
	case EPerformanceWarningLevel::Critical:
		return CriticalColor;
	default:
		return NormalColor;
	}
}

void UPerformanceMonitorWidget::OnToggleVisibilityClicked()
{
	ToggleMonitor();
}

void UPerformanceMonitorWidget::OnResetStatsClicked()
{
	ResetStatistics();
}

void UPerformanceMonitorWidget::OnCaptureSnapshotClicked()
{
	CaptureSnapshot();
}
