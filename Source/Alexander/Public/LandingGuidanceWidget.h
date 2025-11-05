// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LandingGuidanceWidget.generated.h"

// Forward declarations
class ALandingPad;
class APawn;
class UTextBlock;
class UProgressBar;
class UImage;

/**
 * Landing guidance HUD widget that displays altitude, speed, alignment, and distance information
 * Implements requirement 9.2 from the planet interior enhancement spec
 */
UCLASS()
class ALEXANDER_API ULandingGuidanceWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	ULandingGuidanceWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	// ============================================================================
	// WIDGET BINDINGS (Set in UMG Designer)
	// ============================================================================

	/** Text display for altitude */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* AltitudeText;

	/** Text display for vertical speed */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* VerticalSpeedText;

	/** Text display for horizontal speed */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* HorizontalSpeedText;

	/** Text display for distance to pad */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* DistanceText;

	/** Text display for alignment percentage */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* AlignmentText;

	/** Progress bar for alignment visualization */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UProgressBar* AlignmentBar;

	/** Image for alignment indicator (changes color based on alignment) */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* AlignmentIndicator;

	/** Text display for pad name */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* PadNameText;

	/** Text display for landing status message */
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* StatusText;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	/** Target landing pad to display guidance for */
	UPROPERTY(BlueprintReadWrite, Category = "Landing Guidance")
	ALandingPad* TargetPad;

	/** Player pawn to track */
	UPROPERTY(BlueprintReadWrite, Category = "Landing Guidance")
	APawn* PlayerPawn;

	/** Whether to show the widget */
	UPROPERTY(BlueprintReadWrite, Category = "Landing Guidance")
	bool bShowGuidance = true;

	/** Maximum distance to show guidance (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing Guidance")
	float MaxGuidanceDistance = 5000.0f; // 5km

	/** Minimum altitude to show guidance (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing Guidance")
	float MinGuidanceAltitude = 0.0f;

	/** Maximum altitude to show guidance (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing Guidance")
	float MaxGuidanceAltitude = 10000.0f; // 10km

	// ============================================================================
	// COLOR CONFIGURATION
	// ============================================================================

	/** Color for good alignment (> 80%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor GoodAlignmentColor = FLinearColor::Green;

	/** Color for moderate alignment (50-80%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor ModerateAlignmentColor = FLinearColor::Yellow;

	/** Color for poor alignment (< 50%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor PoorAlignmentColor = FLinearColor::Red;

	/** Color for safe descent rate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor SafeDescentColor = FLinearColor::Green;

	/** Color for warning descent rate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor WarningDescentColor = FLinearColor::Yellow;

	/** Color for dangerous descent rate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor DangerDescentColor = FLinearColor::Red;

	// ============================================================================
	// THRESHOLDS
	// ============================================================================

	/** Safe vertical speed threshold (m/s, negative = descending) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
	float SafeDescentRate = -5.0f;

	/** Warning vertical speed threshold (m/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
	float WarningDescentRate = -10.0f;

	/** Dangerous vertical speed threshold (m/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
	float DangerDescentRate = -15.0f;

	/** Safe horizontal speed threshold (m/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
	float SafeHorizontalSpeed = 10.0f;

	/** Warning horizontal speed threshold (m/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
	float WarningHorizontalSpeed = 20.0f;

	// ============================================================================
	// PUBLIC METHODS
	// ============================================================================

	/**
	 * Set the target landing pad
	 * @param NewTargetPad The landing pad to display guidance for
	 */
	UFUNCTION(BlueprintCallable, Category = "Landing Guidance")
	void SetTargetPad(ALandingPad* NewTargetPad);

	/**
	 * Set the player pawn to track
	 * @param NewPlayerPawn The pawn to track
	 */
	UFUNCTION(BlueprintCallable, Category = "Landing Guidance")
	void SetPlayerPawn(APawn* NewPlayerPawn);

	/**
	 * Show or hide the guidance widget
	 * @param bShow Whether to show the widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Landing Guidance")
	void SetGuidanceVisible(bool bShow);

	/**
	 * Update all guidance displays
	 */
	UFUNCTION(BlueprintCallable, Category = "Landing Guidance")
	void UpdateGuidanceDisplay();

protected:
	// ============================================================================
	// INTERNAL UPDATE METHODS
	// ============================================================================

	/** Update altitude display */
	void UpdateAltitudeDisplay(float Altitude);

	/** Update speed displays */
	void UpdateSpeedDisplay(float VerticalSpeed, float HorizontalSpeed);

	/** Update distance display */
	void UpdateDistanceDisplay(float Distance);

	/** Update alignment display */
	void UpdateAlignmentDisplay(float Alignment);

	/** Update status message */
	void UpdateStatusMessage();

	/** Get color based on alignment value */
	FLinearColor GetAlignmentColor(float Alignment) const;

	/** Get color based on vertical speed */
	FLinearColor GetVerticalSpeedColor(float VerticalSpeed) const;

	/** Get color based on horizontal speed */
	FLinearColor GetHorizontalSpeedColor(float HorizontalSpeed) const;

	/** Check if guidance should be displayed */
	bool ShouldShowGuidance() const;

	/** Calculate altitude above target pad */
	float CalculateAltitude() const;

	/** Calculate vertical speed (m/s) */
	float CalculateVerticalSpeed() const;

	/** Calculate horizontal speed (m/s) */
	float CalculateHorizontalSpeed() const;

	/** Calculate distance to target pad */
	float CalculateDistance() const;

	/** Calculate alignment with approach vector */
	float CalculateAlignment() const;

private:
	/** Previous position for velocity calculation */
	FVector PreviousPosition;

	/** Previous time for velocity calculation */
	float PreviousTime;

	/** Smoothed vertical speed */
	mutable float SmoothedVerticalSpeed;

	/** Smoothed horizontal speed */
	mutable float SmoothedHorizontalSpeed;

	/** Speed smoothing factor (0-1, higher = more smoothing) */
	float SpeedSmoothingFactor = 0.8f;
};
