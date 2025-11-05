// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChronometerWidget.h"
#include "RelativitySystem.h"
#include "PlayerTimeComponent.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UChronometerWidget::UChronometerWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoUpdate = true;
	UpdateFrequency = 0.5f;
	bShowWarnings = true;
	bShowPhysicalData = true;
	bCompactMode = false;

	NormalColor = FLinearColor::Green;
	ModerateColor = FLinearColor::Yellow;
	ExtremeColor = FLinearColor::Red;

	UpdateTimer = 0.0f;
	LastTimeWarpFactor = 1.0f;
	bShowingWarning = false;
}

void UChronometerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeReferences();

	// Initial update
	UpdateChronometer();
}

void UChronometerWidget::NativeDestruct()
{
	RelativitySystem = nullptr;
	PlayerTimeComponent = nullptr;
	OwningPlayer = nullptr;

	Super::NativeDestruct();
}

void UChronometerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bAutoUpdate)
	{
		return;
	}

	UpdateTimer += InDeltaTime;

	if (UpdateTimer >= UpdateFrequency)
	{
		UpdateTimer = 0.0f;
		UpdateChronometer();
	}
}

// ============================================================================
// UPDATE FUNCTIONS
// ============================================================================

void UChronometerWidget::UpdateChronometer()
{
	if (!RelativitySystem)
	{
		InitializeReferences();
		if (!RelativitySystem)
		{
			return;
		}
	}

	UpdateRealTime();
	UpdateUniversalTime();
	UpdateBiologicalTime();
	UpdateTimeDebt();
	UpdateTimeDilation();

	if (bShowPhysicalData)
	{
		UpdatePhysicalData();
	}

	UpdateWarningState();
}

void UChronometerWidget::UpdateRealTime()
{
	if (!RealTimeText || !PlayerTimeComponent)
	{
		return;
	}

	double RealTime = PlayerTimeComponent->TotalRealTime;
	FString TimeString = bCompactMode ? FormatTimeHMS(RealTime) : GetTimeString(RealTime, false);

	RealTimeText->SetText(FText::FromString(TimeString));
}

void UChronometerWidget::UpdateUniversalTime()
{
	if (!UniversalTimeText || !RelativitySystem)
	{
		return;
	}

	FString TimeString = bCompactMode ?
		FormatTimeHMS(RelativitySystem->GetUniversalTime()) :
		RelativitySystem->GetFormattedUniversalTime();

	UniversalTimeText->SetText(FText::FromString(TimeString));
}

void UChronometerWidget::UpdateBiologicalTime()
{
	if (!BiologicalTimeText || !PlayerTimeComponent)
	{
		return;
	}

	FString AgeString = FString::Printf(TEXT("%.2f years"), PlayerTimeComponent->BiologicalAge);
	BiologicalTimeText->SetText(FText::FromString(AgeString));
}

void UChronometerWidget::UpdateTimeDebt()
{
	if (!TimeDebtText || !PlayerTimeComponent)
	{
		return;
	}

	FString DebtString = PlayerTimeComponent->GetFormattedTimeDebt();

	// Add severity indicator
	int32 Severity = PlayerTimeComponent->GetTimeDebtSeverity();
	FLinearColor Color = FLinearColor::White;

	switch (Severity)
	{
		case 0: Color = FLinearColor::White; break;
		case 1: Color = FLinearColor::Yellow; break;
		case 2: Color = FLinearColor(1.0f, 0.5f, 0.0f); break; // Orange
		case 3: Color = FLinearColor::Red; break;
	}

	TimeDebtText->SetText(FText::FromString(DebtString));
	TimeDebtText->SetColorAndOpacity(FSlateColor(Color));
}

void UChronometerWidget::UpdateTimeDilation()
{
	if (!PlayerTimeComponent)
	{
		return;
	}

	float Dilation = PlayerTimeComponent->CurrentTimeDilation;

	// Update text
	if (TimeDilationText)
	{
		FString DilationString = FString::Printf(TEXT("%.3fx"), Dilation);
		TimeDilationText->SetText(FText::FromString(DilationString));

		FLinearColor Color = GetDilationColor(Dilation);
		TimeDilationText->SetColorAndOpacity(FSlateColor(Color));
	}

	// Update progress bar
	if (TimeDilationBar)
	{
		// Map dilation to 0-1 range (0.5 = 0, 1.0 = 1)
		float BarValue = FMath::Clamp((Dilation - 0.5f) / 0.5f, 0.0f, 1.0f);
		TimeDilationBar->SetPercent(BarValue);

		FLinearColor Color = GetDilationColor(Dilation);
		TimeDilationBar->SetFillColorAndOpacity(Color);
	}

	// Update time warp indicator
	if (RelativitySystem)
	{
		float WarpFactor = RelativitySystem->GetCurrentTimeWarpFactor();
		if (WarpFactor != LastTimeWarpFactor)
		{
			UpdateTimeWarpIndicator(WarpFactor);
			LastTimeWarpFactor = WarpFactor;
		}
	}
}

void UChronometerWidget::UpdatePhysicalData()
{
	if (!PlayerTimeComponent)
	{
		return;
	}

	// Update velocity
	if (VelocityText)
	{
		FString VelString = FormatVelocity(PlayerTimeComponent->CurrentVelocity);
		VelocityText->SetText(FText::FromString(VelString));
	}

	// Update altitude
	if (AltitudeText)
	{
		FString AltString = FormatAltitude(PlayerTimeComponent->CurrentAltitude);
		AltitudeText->SetText(FText::FromString(AltString));
	}
}

// ============================================================================
// VISIBILITY & WARNINGS
// ============================================================================

void UChronometerWidget::SetChronometerVisible(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UChronometerWidget::ShowExtremeDilationWarning(bool bShow)
{
	if (!bShowWarnings || !WarningIcon)
	{
		return;
	}

	bShowingWarning = bShow;
	WarningIcon->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UChronometerWidget::UpdateTimeWarpIndicator(float WarpFactor)
{
	if (!TimeWarpIndicatorText)
	{
		return;
	}

	if (WarpFactor > 1.1f)
	{
		FString WarpString = FString::Printf(TEXT("TIME WARP: %.0fx"), WarpFactor);
		TimeWarpIndicatorText->SetText(FText::FromString(WarpString));
		TimeWarpIndicatorText->SetVisibility(ESlateVisibility::Visible);

		// Pulse effect for high warp
		FLinearColor Color = WarpFactor > 100.0f ? FLinearColor::Red : FLinearColor::Yellow;
		TimeWarpIndicatorText->SetColorAndOpacity(FSlateColor(Color));
	}
	else
	{
		TimeWarpIndicatorText->SetVisibility(ESlateVisibility::Hidden);
	}
}

// ============================================================================
// FORMATTING
// ============================================================================

FString UChronometerWidget::FormatTimeHMS(double Seconds) const
{
	int32 TotalSecs = FMath::FloorToInt(Seconds);
	int32 Hours = TotalSecs / 3600;
	int32 Minutes = (TotalSecs % 3600) / 60;
	int32 Secs = TotalSecs % 60;

	return FString::Printf(TEXT("%02d:%02d:%02d"), Hours, Minutes, Secs);
}

FString UChronometerWidget::FormatTimeWithDate(double Seconds) const
{
	const double SecondsPerDay = 86400.0;

	int32 Days = FMath::FloorToInt(Seconds / SecondsPerDay);
	double Remainder = FMath::Fmod(Seconds, SecondsPerDay);

	FString DatePart = FString::Printf(TEXT("Day %d "), Days + 1);
	FString TimePart = FormatTimeHMS(Remainder);

	return DatePart + TimePart;
}

FString UChronometerWidget::FormatVelocity(float MetersPerSecond) const
{
	const float SpeedOfLight = 299792458.0f;
	float PercentOfC = (MetersPerSecond / SpeedOfLight) * 100.0f;

	// Format based on magnitude
	if (MetersPerSecond < 1000.0f)
	{
		return FString::Printf(TEXT("%.1f m/s"), MetersPerSecond);
	}
	else if (MetersPerSecond < 1000000.0f)
	{
		return FString::Printf(TEXT("%.1f km/s (%.3f%% c)"), MetersPerSecond / 1000.0f, PercentOfC);
	}
	else
	{
		return FString::Printf(TEXT("%.1f%% c"), PercentOfC);
	}
}

FString UChronometerWidget::FormatAltitude(float Meters) const
{
	if (Meters < 1000.0f)
	{
		return FString::Printf(TEXT("%.0f m"), Meters);
	}
	else if (Meters < 1000000.0f)
	{
		return FString::Printf(TEXT("%.1f km"), Meters / 1000.0f);
	}
	else
	{
		return FString::Printf(TEXT("%.1f Mm"), Meters / 1000000.0f);
	}
}

FLinearColor UChronometerWidget::GetDilationColor(float DilationFactor) const
{
	if (DilationFactor >= 0.9f)
	{
		return NormalColor;
	}
	else if (DilationFactor >= 0.5f)
	{
		// Interpolate between moderate and normal
		float Alpha = (DilationFactor - 0.5f) / 0.4f;
		return FMath::Lerp(ModerateColor, NormalColor, Alpha);
	}
	else
	{
		// Interpolate between extreme and moderate
		float Alpha = DilationFactor / 0.5f;
		return FMath::Lerp(ExtremeColor, ModerateColor, Alpha);
	}
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

void UChronometerWidget::InitializeReferences()
{
	// Get owning player
	OwningPlayer = GetOwningPlayer();
	if (!OwningPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChronometerWidget: No owning player"));
		return;
	}

	// Get RelativitySystem
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (GameInstance)
	{
		RelativitySystem = GameInstance->GetSubsystem<URelativitySystem>();
	}

	// Get PlayerTimeComponent from player's pawn
	if (OwningPlayer && OwningPlayer->GetPawn())
	{
		PlayerTimeComponent = OwningPlayer->GetPawn()->FindComponentByClass<UPlayerTimeComponent>();
	}

	if (!RelativitySystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChronometerWidget: RelativitySystem not found"));
	}

	if (!PlayerTimeComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChronometerWidget: PlayerTimeComponent not found"));
	}
}

void UChronometerWidget::UpdateWarningState()
{
	if (!PlayerTimeComponent || !bShowWarnings)
	{
		return;
	}

	bool bShouldShowWarning = PlayerTimeComponent->IsExperiencingExtremeDilation();

	if (bShouldShowWarning != bShowingWarning)
	{
		ShowExtremeDilationWarning(bShouldShowWarning);
	}
}

FString UChronometerWidget::GetTimeString(double Seconds, bool bIncludeDate) const
{
	if (bIncludeDate)
	{
		return FormatTimeWithDate(Seconds);
	}
	else
	{
		return FormatTimeHMS(Seconds);
	}
}
