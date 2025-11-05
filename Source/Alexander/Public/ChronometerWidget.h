// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChronometerWidget.generated.h"

// Forward declarations
class URelativitySystem;
class UPlayerTimeComponent;
class UTextBlock;
class UProgressBar;
class UImage;
class APlayerController;

/**
 * Chronometer Widget
 * Displays three synchronized clocks showing different time scales:
 * 1. Real Time - Player's actual session time
 * 2. Universal Time - World time with warp applied
 * 3. Biological Time - Player's age considering relativity
 *
 * Also shows time debt, time dilation factor, and warnings
 */
UCLASS()
class ALEXANDER_API UChronometerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UChronometerWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	// ============================================================================
	// UI ELEMENTS (Bind in Blueprint)
	// ============================================================================

	// Real time display (session time)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* RealTimeText;

	// Universal time display (world time)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* UniversalTimeText;

	// Biological time display (player age)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* BiologicalTimeText;

	// Time debt display
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TimeDebtText;

	// Time dilation factor display
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TimeDilationText;

	// Time dilation progress bar (visual indicator)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UProgressBar* TimeDilationBar;

	// Warning icon (shown when extreme dilation)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UImage* WarningIcon;

	// Time warp indicator text
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* TimeWarpIndicatorText;

	// Velocity display (m/s and % of c)
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* VelocityText;

	// Altitude display
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UTextBlock* AltitudeText;

	// ============================================================================
	// UPDATE FUNCTIONS
	// ============================================================================

	/**
	 * Update all chronometer displays
	 * Called each tick or manually
	 */
	UFUNCTION(BlueprintCallable, Category = "Chronometer")
	void UpdateChronometer();

	/**
	 * Update real time display
	 */
	UFUNCTION(BlueprintCallable, Category = "Chronometer")
	void UpdateRealTime();

	/**
	 * Update universal time display
	 */
	UFUNCTION(BlueprintCallable, Category = "Chronometer")
	void UpdateUniversalTime();

	/**
	 * Update biological time display
	 */
	UFUNCTION(BlueprintCallable, Category = "Chronometer")
	void UpdateBiologicalTime();

	/**
	 * Update time debt display
	 */
	UFUNCTION(BlueprintCallable, Category = "Chronometer")
	void UpdateTimeDebt();

	/**
	 * Update time dilation display
	 */
	UFUNCTION(BlueprintCallable, Category = "Chronometer")
	void UpdateTimeDilation();

	/**
	 * Update velocity and altitude displays
	 */
	UFUNCTION(BlueprintCallable, Category = "Chronometer")
	void UpdatePhysicalData();

	// ============================================================================
	// VISIBILITY & WARNINGS
	// ============================================================================

	/**
	 * Show/hide the chronometer
	 */
	UFUNCTION(BlueprintCallable, Category = "Chronometer")
	void SetChronometerVisible(bool bVisible);

	/**
	 * Show warning for extreme time dilation
	 */
	UFUNCTION(BlueprintCallable, Category = "Chronometer")
	void ShowExtremeDilationWarning(bool bShow);

	/**
	 * Update time warp indicator
	 */
	UFUNCTION(BlueprintCallable, Category = "Chronometer")
	void UpdateTimeWarpIndicator(float WarpFactor);

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	// Auto-update the chronometer each tick?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chronometer|Config")
	bool bAutoUpdate = true;

	// Update frequency (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chronometer|Config", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float UpdateFrequency = 0.5f;

	// Show warnings for extreme dilation?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chronometer|Config")
	bool bShowWarnings = true;

	// Show velocity and altitude data?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chronometer|Config")
	bool bShowPhysicalData = true;

	// Compact mode (smaller display)?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chronometer|Config")
	bool bCompactMode = false;

	// ============================================================================
	// COLOR CONFIGURATION
	// ============================================================================

	// Normal time dilation color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chronometer|Colors")
	FLinearColor NormalColor = FLinearColor::Green;

	// Moderate time dilation color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chronometer|Colors")
	FLinearColor ModerateColor = FLinearColor::Yellow;

	// Extreme time dilation color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chronometer|Colors")
	FLinearColor ExtremeColor = FLinearColor::Red;

	// ============================================================================
	// FORMATTING
	// ============================================================================

	/**
	 * Format time in seconds to HH:MM:SS
	 */
	UFUNCTION(BlueprintPure, Category = "Chronometer|Formatting")
	FString FormatTimeHMS(double Seconds) const;

	/**
	 * Format time with date (Year, Day, HH:MM:SS)
	 */
	UFUNCTION(BlueprintPure, Category = "Chronometer|Formatting")
	FString FormatTimeWithDate(double Seconds) const;

	/**
	 * Format velocity
	 */
	UFUNCTION(BlueprintPure, Category = "Chronometer|Formatting")
	FString FormatVelocity(float MetersPerSecond) const;

	/**
	 * Format altitude
	 */
	UFUNCTION(BlueprintPure, Category = "Chronometer|Formatting")
	FString FormatAltitude(float Meters) const;

	/**
	 * Get color for time dilation value
	 */
	UFUNCTION(BlueprintPure, Category = "Chronometer|Formatting")
	FLinearColor GetDilationColor(float DilationFactor) const;

private:
	// ============================================================================
	// REFERENCES
	// ============================================================================

	UPROPERTY()
	URelativitySystem* RelativitySystem;

	UPROPERTY()
	UPlayerTimeComponent* PlayerTimeComponent;

	UPROPERTY()
	APlayerController* OwningPlayer;

	// ============================================================================
	// INTERNAL STATE
	// ============================================================================

	// Update timer
	float UpdateTimer;

	// Last known time warp factor
	float LastTimeWarpFactor;

	// Is showing warning?
	bool bShowingWarning;

	// Helper functions
	void InitializeReferences();
	void UpdateWarningState();
	FString GetTimeString(double Seconds, bool bIncludeDate = false) const;
};
