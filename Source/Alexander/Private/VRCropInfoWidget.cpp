// Copyright Epic Games, Inc. All Rights Reserved.

#include "VRCropInfoWidget.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "VRCropInspectionComponent.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Blueprint/UserWidget.h"              // For UUserWidget
#include "Camera/CameraComponent.h"             // For UCameraComponent

UVRCropInfoWidget::UVRCropInfoWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UVRCropInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize widget state
	SetVisibility(ESlateVisibility::Hidden);
}

// ============================================================================
// UPDATE FUNCTIONS
// ============================================================================

void UVRCropInfoWidget::UpdateCropInfo(const FCropInspectionData& InspectionData)
{
	// Update crop name
	UpdateCropName(InspectionData.CropName);

	// Update growth information
	UpdateGrowthInfo(InspectionData.GrowthStageName, InspectionData.GrowthProgress);

	// Update health information
	UpdateHealthInfo(InspectionData.Health, InspectionData.HealthStatus, InspectionData.HealthStatusColor);

	// Update issues and warnings
	UpdateIssuesAndWarnings(InspectionData.PrimaryIssue, InspectionData.bNeedsWater, InspectionData.bNeedsFertilizer);

	// Update time to harvest
	UpdateTimeToHarvest(InspectionData.TimeToHarvest);
}

void UVRCropInfoWidget::UpdateCropName(const FString& CropName)
{
	SetTextSafe(CropNameText, CropName);
}

void UVRCropInfoWidget::UpdateGrowthInfo(const FString& StageName, float Progress)
{
	// Update stage name
	SetTextSafe(GrowthStageText, FString::Printf(TEXT("Stage: %s"), *StageName));

	// Update progress bar
	FLinearColor ProgressColor = GetProgressBarColor(Progress);
	SetProgressBarSafe(GrowthProgressBar, Progress, ProgressColor);

	// Update progress percentage
	SetTextSafe(GrowthProgressText, FString::Printf(TEXT("%.0f%%"), Progress * 100.0f));
}

void UVRCropInfoWidget::UpdateHealthInfo(float Health, const FString& Status, const FLinearColor& StatusColor)
{
	// Update health status text
	SetTextSafe(HealthStatusText, FString::Printf(TEXT("Health: %s"), *Status));

	// Set health status color
	if (HealthStatusText)
	{
		HealthStatusText->SetColorAndOpacity(FSlateColor(StatusColor));
	}

	// Update health progress bar
	SetProgressBarSafe(HealthProgressBar, Health, StatusColor);

	// Update health percentage
	SetTextSafe(HealthPercentageText, FString::Printf(TEXT("%.0f%%"), Health * 100.0f));
}

void UVRCropInfoWidget::UpdateIssuesAndWarnings(const FString& PrimaryIssue, bool bNeedsWater, bool bNeedsFertilizer)
{
	// Update primary issue
	if (PrimaryIssue.IsEmpty() || PrimaryIssue == TEXT("None"))
	{
		SetTextSafe(PrimaryIssueText, TEXT("No Issues"));
		if (PrimaryIssueText)
		{
			PrimaryIssueText->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
		}
	}
	else
	{
		SetTextSafe(PrimaryIssueText, FString::Printf(TEXT("Issue: %s"), *PrimaryIssue));
		if (PrimaryIssueText)
		{
			PrimaryIssueText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.5f, 0.0f))); // Orange
		}
	}

	// Update warning icons
	SetImageVisibilitySafe(NeedsWaterIcon, bNeedsWater);
	SetImageVisibilitySafe(NeedsFertilizerIcon, bNeedsFertilizer);
}

void UVRCropInfoWidget::UpdateTimeToHarvest(float TimeSeconds)
{
	if (TimeSeconds <= 0.0f)
	{
		SetTextSafe(TimeToHarvestText, TEXT("Ready to Harvest!"));
		if (TimeToHarvestText)
		{
			TimeToHarvestText->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
		}
	}
	else
	{
		FString TimeString = FormatTimeDuration(TimeSeconds);
		SetTextSafe(TimeToHarvestText, FString::Printf(TEXT("Harvest in: %s"), *TimeString));
		if (TimeToHarvestText)
		{
			TimeToHarvestText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		}
	}
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

FString UVRCropInfoWidget::FormatTimeDuration(float Seconds) const
{
	if (Seconds < 60.0f)
	{
		return FString::Printf(TEXT("%.0fs"), Seconds);
	}
	else if (Seconds < 3600.0f)
	{
		int32 Minutes = FMath::FloorToInt(Seconds / 60.0f);
		int32 Secs = FMath::FloorToInt(Seconds) % 60;
		if (Secs > 0)
		{
			return FString::Printf(TEXT("%dm %ds"), Minutes, Secs);
		}
		else
		{
			return FString::Printf(TEXT("%dm"), Minutes);
		}
	}
	else
	{
		int32 Hours = FMath::FloorToInt(Seconds / 3600.0f);
		int32 Minutes = FMath::FloorToInt((Seconds - Hours * 3600.0f) / 60.0f);
		if (Minutes > 0)
		{
			return FString::Printf(TEXT("%dh %dm"), Hours, Minutes);
		}
		else
		{
			return FString::Printf(TEXT("%dh"), Hours);
		}
	}
}

FLinearColor UVRCropInfoWidget::GetProgressBarColor(float Progress) const
{
	// Color gradient from red (0%) to yellow (50%) to green (100%)
	if (Progress < 0.5f)
	{
		// Red to Yellow
		float T = Progress * 2.0f;
		return FMath::Lerp(FLinearColor::Red, FLinearColor::Yellow, T);
	}
	else
	{
		// Yellow to Green
		float T = (Progress - 0.5f) * 2.0f;
		return FMath::Lerp(FLinearColor::Yellow, FLinearColor::Green, T);
	}
}

void UVRCropInfoWidget::SetTextSafe(UTextBlock* TextBlock, const FString& Text)
{
	if (TextBlock)
	{
		TextBlock->SetText(FText::FromString(Text));
	}
}

void UVRCropInfoWidget::SetProgressBarSafe(UProgressBar* ProgressBar, float Progress, const FLinearColor& Color)
{
	if (ProgressBar)
	{
		ProgressBar->SetPercent(Progress);
		ProgressBar->SetFillColorAndOpacity(Color);
	}
}

void UVRCropInfoWidget::SetImageVisibilitySafe(UImage* Image, bool bVisible)
{
	if (Image)
	{
		Image->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}
