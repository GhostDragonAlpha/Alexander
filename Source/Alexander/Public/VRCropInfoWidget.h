// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VRCropInfoWidget.generated.h"

// Forward declarations
struct FCropInspectionData;
class UTextBlock;
class UProgressBar;
class UImage;
class UVerticalBox;

/**
 * VR Crop Info Widget
 * Displays crop information when player looks at crops
 */
UCLASS()
class ALEXANDER_API UVRCropInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UVRCropInfoWidget(const FObjectInitializer& ObjectInitializer);

	// ============================================================================
	// UI ELEMENTS (Bind these in Blueprint)
	// ============================================================================

	/** Crop name text */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CropNameText;

	/** Growth stage text */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GrowthStageText;

	/** Growth progress bar */
	UPROPERTY(meta = (BindWidget))
	UProgressBar* GrowthProgressBar;

	/** Growth progress percentage text */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GrowthProgressText;

	/** Health status text */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthStatusText;

	/** Health progress bar */
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthProgressBar;

	/** Health percentage text */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthPercentageText;

	/** Primary issue text */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PrimaryIssueText;

	/** Time to harvest text */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimeToHarvestText;

	/** Needs water indicator */
	UPROPERTY(meta = (BindWidget))
	UImage* NeedsWaterIcon;

	/** Needs fertilizer indicator */
	UPROPERTY(meta = (BindWidget))
	UImage* NeedsFertilizerIcon;

	/** Container for warning icons */
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* WarningsContainer;

	// ============================================================================
	// UPDATE FUNCTIONS
	// ============================================================================

	/**
	 * Update widget with new crop inspection data
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Info")
	void UpdateCropInfo(const FCropInspectionData& InspectionData);

	/**
	 * Update crop name display
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Info")
	void UpdateCropName(const FString& CropName);

	/**
	 * Update growth information
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Info")
	void UpdateGrowthInfo(const FString& StageName, float Progress);

	/**
	 * Update health information
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Info")
	void UpdateHealthInfo(float Health, const FString& Status, const FLinearColor& StatusColor);

	/**
	 * Update issue and warnings
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Info")
	void UpdateIssuesAndWarnings(const FString& PrimaryIssue, bool bNeedsWater, bool bNeedsFertilizer);

	/**
	 * Update time to harvest
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Info")
	void UpdateTimeToHarvest(float TimeSeconds);

	// ============================================================================
	// HELPER FUNCTIONS
	// ============================================================================

	/**
	 * Format time duration for display
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Info")
	FString FormatTimeDuration(float Seconds) const;

	/**
	 * Get color for progress bar based on value
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Info")
	FLinearColor GetProgressBarColor(float Progress) const;

protected:
	virtual void NativeConstruct() override;

	/**
	 * Set text safely (checks for null)
	 */
	void SetTextSafe(UTextBlock* TextBlock, const FString& Text);

	/**
	 * Set progress bar safely (checks for null)
	 */
	void SetProgressBarSafe(UProgressBar* ProgressBar, float Progress, const FLinearColor& Color);

	/**
	 * Set image visibility safely (checks for null)
	 */
	void SetImageVisibilitySafe(UImage* Image, bool bVisible);
};
