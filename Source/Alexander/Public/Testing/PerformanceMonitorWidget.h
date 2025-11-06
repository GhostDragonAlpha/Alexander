// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TestDataStructures.h"
#include "PerformanceMonitorWidget.generated.h"

// Forward declarations
class ATestLevelGameMode;
class UTextBlock;
class UProgressBar;
class UButton;
class UCanvasPanel;
class UBorder;
class UVerticalBox;

/**
 * Performance warning level for visual feedback
 */
UENUM(BlueprintType)
enum class EPerformanceWarningLevel : uint8
{
	Normal UMETA(DisplayName = "Normal"),
	Warning UMETA(DisplayName = "Warning"),
	Critical UMETA(DisplayName = "Critical")
};

/**
 * Frame data for performance graph
 */
USTRUCT(BlueprintType)
struct FFrameData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Performance")
	float FPS = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Performance")
	float FrameTimeMS = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Performance")
	float GameThreadMS = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Performance")
	float RenderThreadMS = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Performance")
	float GPUMS = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Performance")
	FDateTime Timestamp;

	FFrameData()
		: FPS(0.0f), FrameTimeMS(0.0f), GameThreadMS(0.0f),
		  RenderThreadMS(0.0f), GPUMS(0.0f)
	{}
};

/**
 * UMG Widget for real-time performance monitoring
 *
 * Features:
 * - Real-time FPS graph (last 100 frames)
 * - Memory usage tracking and graph
 * - Frame time breakdown (game/render/GPU threads)
 * - VR-specific metrics (motion-to-photon latency, dropped frames)
 * - Color-coded performance warnings
 * - Performance statistics (average/min/max)
 * - Toggle show/hide functionality
 *
 * Usage:
 * 1. Create UMG Blueprint inheriting from this class
 * 2. Bind widget components using matching names (see BindWidget properties)
 * 3. Customize graph appearance in UMG Designer
 * 4. Place in level as overlay (togglable with hotkey)
 */
UCLASS()
class ALEXANDER_API UPerformanceMonitorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPerformanceMonitorWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;

public:
	// Visibility controls
	UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
	void ShowMonitor();

	UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
	void HideMonitor();

	UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
	void ToggleMonitor();

	UFUNCTION(BlueprintPure, Category = "Performance Monitor")
	bool IsMonitorVisible() const { return bIsVisible; }

	// Performance data access
	UFUNCTION(BlueprintPure, Category = "Performance Monitor")
	float GetCurrentFPS() const;

	UFUNCTION(BlueprintPure, Category = "Performance Monitor")
	float GetAverageFPS() const;

	UFUNCTION(BlueprintPure, Category = "Performance Monitor")
	float GetMinFPS() const;

	UFUNCTION(BlueprintPure, Category = "Performance Monitor")
	float GetMaxFPS() const;

	UFUNCTION(BlueprintPure, Category = "Performance Monitor")
	float GetCurrentFrameTimeMS() const;

	UFUNCTION(BlueprintPure, Category = "Performance Monitor")
	float GetMemoryUsageMB() const;

	UFUNCTION(BlueprintPure, Category = "Performance Monitor")
	EPerformanceWarningLevel GetWarningLevel() const;

	// VR metrics
	UFUNCTION(BlueprintPure, Category = "Performance Monitor|VR")
	float GetMotionToPhotonLatency() const;

	UFUNCTION(BlueprintPure, Category = "Performance Monitor|VR")
	int32 GetDroppedFrames() const;

	UFUNCTION(BlueprintPure, Category = "Performance Monitor|VR")
	float GetReprojectionRatio() const;

	UFUNCTION(BlueprintPure, Category = "Performance Monitor|VR")
	bool IsVRMode() const { return bIsVRMode; }

	// Statistics controls
	UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
	void ResetStatistics();

	UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
	void CaptureSnapshot();

	UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
	FTestPerformanceMetrics GetCurrentMetrics() const;

	// Graph controls
	UFUNCTION(BlueprintCallable, Category = "Performance Monitor|Graph")
	void SetGraphHistorySize(int32 NewSize);

	UFUNCTION(BlueprintCallable, Category = "Performance Monitor|Graph")
	void ClearGraphHistory();

	// Blueprint implementable events
	UFUNCTION(BlueprintImplementableEvent, Category = "Performance Monitor|Events")
	void OnPerformanceWarning(EPerformanceWarningLevel Level, const FString& Message);

	UFUNCTION(BlueprintImplementableEvent, Category = "Performance Monitor|Events")
	void OnMetricsUpdated(const FTestPerformanceMetrics& Metrics);

	UFUNCTION(BlueprintImplementableEvent, Category = "Performance Monitor|Events")
	void OnVRFrameDropped(int32 TotalDropped);

protected:
	// Update functions
	void UpdateFPSDisplay();
	void UpdateMemoryDisplay();
	void UpdateThreadTimesDisplay();
	void UpdateVRMetricsDisplay();
	void UpdateGraphs();
	void UpdateWarningLevel();

	// Performance calculation
	void CaptureFrameData();
	void CalculateStatistics();
	float CalculateFPS(float DeltaTime) const;
	float GetGameThreadTime() const;
	float GetRenderThreadTime() const;
	float GetGPUTime() const;

	// Graph drawing (to be implemented in Blueprint or using Slate)
	void DrawFPSGraph();
	void DrawMemoryGraph();
	void DrawFrameTimeGraph();

	// Color helpers
	FLinearColor GetFPSColor(float FPS) const;
	FLinearColor GetMemoryColor(float MemoryMB) const;
	FLinearColor GetWarningColor(EPerformanceWarningLevel Level) const;

	// Button callbacks
	UFUNCTION()
	void OnToggleVisibilityClicked();

	UFUNCTION()
	void OnResetStatsClicked();

	UFUNCTION()
	void OnCaptureSnapshotClicked();

protected:
	// Widget bindings - Must match names in UMG Designer
	// FPS section
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentFPSText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AverageFPSText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MinFPSText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MaxFPSText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* FPSProgressBar;

	// Frame time section
	UPROPERTY(meta = (BindWidget))
	UTextBlock* FrameTimeText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GameThreadTimeText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RenderThreadTimeText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GPUTimeText;

	// Memory section
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MemoryUsageText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* MemoryProgressBar;

	// VR metrics section (optional - hidden if not in VR)
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* VRMetricsBox;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MotionToPhotonText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DroppedFramesText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ReprojectionText;

	// Graphs
	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* FPSGraphCanvas;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* MemoryGraphCanvas;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* FrameTimeGraphCanvas;

	// Warning indicator
	UPROPERTY(meta = (BindWidget))
	UBorder* WarningBorder;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WarningText;

	// Control buttons
	UPROPERTY(meta = (BindWidget))
	UButton* ToggleButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ResetStatsButton;

	UPROPERTY(meta = (BindWidget))
	UButton* SnapshotButton;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor|Config")
	int32 GraphHistorySize = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor|Config")
	float UpdateInterval = 0.016f; // Update every frame (60 FPS)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor|Config")
	bool bStartVisible = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor|Config")
	bool bShowGraphs = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor|Config")
	bool bShowVRMetrics = true;

	// VR Performance thresholds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor|VR Thresholds")
	float VRTargetFPS = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor|VR Thresholds")
	float VRWarningFPS = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor|VR Thresholds")
	float VRCriticalFPS = 60.0f;

	// General performance thresholds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor|Thresholds")
	float TargetFPS = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor|Thresholds")
	float WarningFPS = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor|Thresholds")
	float CriticalFPS = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor|Thresholds")
	float MemoryWarningMB = 2048.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor|Thresholds")
	float MemoryCriticalMB = 3072.0f;

	// Visual styling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor|Styling")
	FLinearColor NormalColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor|Styling")
	FLinearColor WarningColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor|Styling")
	FLinearColor CriticalColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitor|Styling")
	float GraphLineThickness = 2.0f;

private:
	// Internal state
	UPROPERTY()
	ATestLevelGameMode* GameModeRef = nullptr;

	bool bIsVisible = true;
	bool bIsVRMode = false;

	// Performance data history
	TArray<FFrameData> FrameHistory;
	int32 FrameHistoryIndex = 0;

	// Statistics
	float TotalFPS = 0.0f;
	float MinFPSRecorded = FLT_MAX;
	float MaxFPSRecorded = 0.0f;
	int32 FrameCount = 0;
	float StatisticsStartTime = 0.0f;

	// VR metrics
	int32 TotalDroppedFrames = 0;
	int32 LastFrameNumber = 0;

	// Timing
	float TimeSinceLastUpdate = 0.0f;
	float LastDeltaTime = 0.0f;

	// Warning state
	EPerformanceWarningLevel CurrentWarningLevel = EPerformanceWarningLevel::Normal;
	EPerformanceWarningLevel LastWarningLevel = EPerformanceWarningLevel::Normal;
};
