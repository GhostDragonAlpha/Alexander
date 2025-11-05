// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RelativitySystem.h"
#include "TimeWarpWarningWidget.generated.h"

// Forward declarations
class URelativitySystem;
class UTextBlock;
class UButton;
class UProgressBar;
class UImage;

/**
 * Time warp warning severity levels
 */
UENUM(BlueprintType)
enum class ETimeWarpWarningSeverity : uint8
{
	None UMETA(DisplayName = "None"),
	Low UMETA(DisplayName = "Low"),
	Moderate UMETA(DisplayName = "Moderate"),
	High UMETA(DisplayName = "High"),
	Extreme UMETA(DisplayName = "Extreme")
};

/**
 * Time Warp Warning Widget
 * Displays warnings and predictions before player activates time warp
 * Shows consequences like universe time advancement and time debt increase
 */
UCLASS()
class ALEXANDER_API UTimeWarpWarningWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UTimeWarpWarningWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	// ============================================================================
	// UI ELEMENTS (Bind in Blueprint)
	// ============================================================================

	// Warning title
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* WarningTitleText;

	// Time warp factor display
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TimeWarpFactorText;

	// Universal time display (used in UpdateWarningDisplay)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* UniversalTimeText;

	// Universe time advancement prediction
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* UniverseTimeText;

	// Player biological time advancement prediction
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* PlayerTimeText;

	// Time debt increase prediction
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TimeDebtIncreaseText;

	// Warning message
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* WarningMessageText;

	// Severity indicator
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UProgressBar* SeverityBar;

	// Warning icon
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UImage* WarningIcon;

	// Confirm button
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* ConfirmButton;

	// Cancel button
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* CancelButton;

	// Duration input display (if adjustable)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* DurationText;

	// ============================================================================
	// WARNING DISPLAY
	// ============================================================================

	/**
	 * Show time warp warning with predictions
	 * @param WarpFactor Desired time warp factor
	 * @param DurationSeconds How long warp will be active (real time)
	 */
	UFUNCTION(BlueprintCallable, Category = "TimeWarp")
	void ShowTimeWarpWarning(float WarpFactor, float DurationSeconds);

	/**
	 * Hide the warning widget
	 */
	UFUNCTION(BlueprintCallable, Category = "TimeWarp")
	void HideWarning();

	/**
	 * Update warning with new prediction
	 */
	UFUNCTION(BlueprintCallable, Category = "TimeWarp")
	void UpdatePrediction(float WarpFactor, float DurationSeconds);

	// ============================================================================
	// BUTTON CALLBACKS
	// ============================================================================

	/**
	 * Called when player confirms time warp
	 */
	UFUNCTION(BlueprintCallable, Category = "TimeWarp")
	void OnConfirmClicked();

	/**
	 * Called when player cancels time warp
	 */
	UFUNCTION(BlueprintCallable, Category = "TimeWarp")
	void OnCancelClicked();

	// ============================================================================
	// EVENTS (Implement in Blueprint or bind in code)
	// ============================================================================

	/**
	 * Called when player confirms time warp
	 * Implement in Blueprint to activate time warp
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "TimeWarp|Events")
	void OnTimeWarpConfirmed(float WarpFactor);

	/**
	 * Called when player cancels time warp
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "TimeWarp|Events")
	void OnTimeWarpCancelled();

	// ============================================================================
	// SEVERITY CALCULATION
	// ============================================================================

	/**
	 * Get warning severity based on time debt increase
	 */
	UFUNCTION(BlueprintPure, Category = "TimeWarp")
	ETimeWarpWarningSeverity GetWarningSeverity(double TimeDebtIncrease) const;

	/**
	 * Get warning color based on severity
	 */
	UFUNCTION(BlueprintPure, Category = "TimeWarp")
	FLinearColor GetSeverityColor(ETimeWarpWarningSeverity Severity) const;

	/**
	 * Get warning message based on severity
	 */
	UFUNCTION(BlueprintPure, Category = "TimeWarp")
	FString GetWarningMessage(ETimeWarpWarningSeverity Severity) const;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	// Show detailed predictions?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeWarp|Config")
	bool bShowDetailedPredictions = true;

	// Require confirmation for all time warps?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeWarp|Config")
	bool bAlwaysRequireConfirmation = false;

	// Minimum warp factor that requires warning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeWarp|Config", meta = (ClampMin = "1.0"))
	float MinimumWarpFactorForWarning = 10.0f;

	// Minimum time debt increase (hours) that requires warning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeWarp|Config", meta = (ClampMin = "0.0"))
	float MinimumTimeDebtForWarning = 1.0f;

	// ============================================================================
	// COLOR CONFIGURATION
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeWarp|Colors")
	FLinearColor LowSeverityColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeWarp|Colors")
	FLinearColor ModerateSeverityColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeWarp|Colors")
	FLinearColor HighSeverityColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeWarp|Colors")
	FLinearColor ExtremeSeverityColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red

	// ============================================================================
	// HELPER FUNCTIONS
	// ============================================================================

	/**
	 * Check if time warp requires warning
	 */
	UFUNCTION(BlueprintPure, Category = "TimeWarp")
	bool ShouldShowWarning(float WarpFactor, float DurationSeconds) const;

	/**
	 * Format time duration for display
	 */
	UFUNCTION(BlueprintPure, Category = "TimeWarp")
	FString FormatTimeDuration(double Seconds) const;

private:
	// ============================================================================
	// REFERENCES
	// ============================================================================

	UPROPERTY()
	URelativitySystem* RelativitySystem;

	// ============================================================================
	// INTERNAL STATE
	// ============================================================================

	// Current prediction data
	FTimeWarpPrediction CurrentPrediction;

	// Current warp factor being warned about
	float CurrentWarpFactor;

	// Current duration being warned about
	float CurrentDuration;

	// Helper functions
	void InitializeReferences();
	void SetupButtonCallbacks();
	void UpdateWarningDisplay();
	void UpdateSeverityIndicator(ETimeWarpWarningSeverity Severity);
};
