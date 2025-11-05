// Copyright Epic Games, Inc. All Rights Reserved.

#include "TimeWarpWarningWidget.h"
#include "RelativitySystem.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

UTimeWarpWarningWidget::UTimeWarpWarningWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bShowDetailedPredictions = true;
	bAlwaysRequireConfirmation = false;
	MinimumWarpFactorForWarning = 10.0f;
	MinimumTimeDebtForWarning = 1.0f;

	LowSeverityColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
	ModerateSeverityColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
	HighSeverityColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);
	ExtremeSeverityColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	CurrentWarpFactor = 1.0f;
	CurrentDuration = 0.0f;
}

void UTimeWarpWarningWidget::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeReferences();
	SetupButtonCallbacks();

	// Start hidden
	SetVisibility(ESlateVisibility::Hidden);
}

void UTimeWarpWarningWidget::NativeDestruct()
{
	RelativitySystem = nullptr;

	Super::NativeDestruct();
}

// ============================================================================
// WARNING DISPLAY
// ============================================================================

void UTimeWarpWarningWidget::ShowTimeWarpWarning(float WarpFactor, float DurationSeconds)
{
	if (!RelativitySystem)
	{
		InitializeReferences();
		if (!RelativitySystem)
		{
			UE_LOG(LogTemp, Warning, TEXT("TimeWarpWarningWidget: Cannot show warning - RelativitySystem not found"));
			return;
		}
	}

	CurrentWarpFactor = WarpFactor;
	CurrentDuration = DurationSeconds;

	// Get prediction from RelativitySystem
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		CurrentPrediction = RelativitySystem->PredictTimeWarpConsequences(WarpFactor, DurationSeconds, PC);
	}

	// Update display
	UpdateWarningDisplay();

	// Show widget
	SetVisibility(ESlateVisibility::Visible);

	UE_LOG(LogTemp, Log, TEXT("TimeWarpWarningWidget: Showing warning for %.1fx warp over %.1fs"),
		WarpFactor, DurationSeconds);
}

void UTimeWarpWarningWidget::HideWarning()
{
	SetVisibility(ESlateVisibility::Hidden);

	UE_LOG(LogTemp, Log, TEXT("TimeWarpWarningWidget: Hidden"));
}

void UTimeWarpWarningWidget::UpdatePrediction(float WarpFactor, float DurationSeconds)
{
	if (!RelativitySystem)
	{
		return;
	}

	CurrentWarpFactor = WarpFactor;
	CurrentDuration = DurationSeconds;

	// Get new prediction
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		CurrentPrediction = RelativitySystem->PredictTimeWarpConsequences(WarpFactor, DurationSeconds, PC);
	}

	// Update display
	UpdateWarningDisplay();
}

// ============================================================================
// BUTTON CALLBACKS
// ============================================================================

void UTimeWarpWarningWidget::OnConfirmClicked()
{
	UE_LOG(LogTemp, Log, TEXT("TimeWarpWarningWidget: Time warp confirmed at %.1fx"), CurrentWarpFactor);

	// Broadcast Blueprint event
	OnTimeWarpConfirmed(CurrentWarpFactor);

	// Hide warning
	HideWarning();
}

void UTimeWarpWarningWidget::OnCancelClicked()
{
	UE_LOG(LogTemp, Log, TEXT("TimeWarpWarningWidget: Time warp cancelled"));

	// Broadcast Blueprint event
	OnTimeWarpCancelled();

	// Hide warning
	HideWarning();
}

// ============================================================================
// SEVERITY CALCULATION
// ============================================================================

ETimeWarpWarningSeverity UTimeWarpWarningWidget::GetWarningSeverity(double TimeDebtIncrease) const
{
	const double SecondsPerHour = 3600.0;
	const double SecondsPerDay = 86400.0;
	const double SecondsPerWeek = 604800.0;

	double Hours = TimeDebtIncrease / SecondsPerHour;

	if (Hours < 1.0)
	{
		return ETimeWarpWarningSeverity::Low;
	}
	else if (Hours < 24.0)
	{
		return ETimeWarpWarningSeverity::Moderate;
	}
	else if (Hours < 168.0) // 1 week
	{
		return ETimeWarpWarningSeverity::High;
	}
	else
	{
		return ETimeWarpWarningSeverity::Extreme;
	}
}

FLinearColor UTimeWarpWarningWidget::GetSeverityColor(ETimeWarpWarningSeverity Severity) const
{
	switch (Severity)
	{
		case ETimeWarpWarningSeverity::None:
		case ETimeWarpWarningSeverity::Low:
			return LowSeverityColor;

		case ETimeWarpWarningSeverity::Moderate:
			return ModerateSeverityColor;

		case ETimeWarpWarningSeverity::High:
			return HighSeverityColor;

		case ETimeWarpWarningSeverity::Extreme:
			return ExtremeSeverityColor;

		default:
			return FLinearColor::White;
	}
}

FString UTimeWarpWarningWidget::GetWarningMessage(ETimeWarpWarningSeverity Severity) const
{
	switch (Severity)
	{
		case ETimeWarpWarningSeverity::None:
			return TEXT("No significant time dilation effects.");

		case ETimeWarpWarningSeverity::Low:
			return TEXT("Minimal time debt. This time warp is safe.");

		case ETimeWarpWarningSeverity::Moderate:
			return TEXT("Warning: Moderate time debt. The universe will age significantly during this warp.");

		case ETimeWarpWarningSeverity::High:
			return TEXT("Caution: High time debt. Days will pass in the universe while you experience less time.");

		case ETimeWarpWarningSeverity::Extreme:
			return TEXT("DANGER: Extreme time debt! Weeks or more will pass in the universe. Consider consequences carefully!");

		default:
			return TEXT("");
	}
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

bool UTimeWarpWarningWidget::ShouldShowWarning(float WarpFactor, float DurationSeconds) const
{
	if (bAlwaysRequireConfirmation && WarpFactor > 1.1f)
	{
		return true;
	}

	if (WarpFactor < MinimumWarpFactorForWarning)
	{
		return false;
	}

	// Calculate potential time debt
	double UniverseAdvancement = static_cast<double>(DurationSeconds) * static_cast<double>(WarpFactor);
	double HoursAdvancement = UniverseAdvancement / 3600.0;

	return HoursAdvancement >= MinimumTimeDebtForWarning;
}

FString UTimeWarpWarningWidget::FormatTimeDuration(double Seconds) const
{
	if (Seconds < 0.001)
	{
		return TEXT("0 seconds");
	}

	const double SecondsPerYear = 31536000.0;
	const double SecondsPerDay = 86400.0;
	const double SecondsPerHour = 3600.0;
	const double SecondsPerMinute = 60.0;

	TArray<FString> Parts;

	// Years
	if (Seconds >= SecondsPerYear)
	{
		int32 Years = FMath::FloorToInt(Seconds / SecondsPerYear);
		Parts.Add(FString::Printf(TEXT("%d year%s"), Years, Years > 1 ? TEXT("s") : TEXT("")));
		Seconds = FMath::Fmod(Seconds, SecondsPerYear);
	}

	// Days
	if (Seconds >= SecondsPerDay)
	{
		int32 Days = FMath::FloorToInt(Seconds / SecondsPerDay);
		Parts.Add(FString::Printf(TEXT("%d day%s"), Days, Days > 1 ? TEXT("s") : TEXT("")));
		Seconds = FMath::Fmod(Seconds, SecondsPerDay);
	}

	// Hours
	if (Seconds >= SecondsPerHour)
	{
		int32 Hours = FMath::FloorToInt(Seconds / SecondsPerHour);
		Parts.Add(FString::Printf(TEXT("%d hour%s"), Hours, Hours > 1 ? TEXT("s") : TEXT("")));
		Seconds = FMath::Fmod(Seconds, SecondsPerHour);
	}

	// Minutes
	if (Seconds >= SecondsPerMinute)
	{
		int32 Minutes = FMath::FloorToInt(Seconds / SecondsPerMinute);
		Parts.Add(FString::Printf(TEXT("%d minute%s"), Minutes, Minutes > 1 ? TEXT("s") : TEXT("")));
		Seconds = FMath::Fmod(Seconds, SecondsPerMinute);
	}

	// Seconds (only if no larger units)
	if (Parts.Num() == 0)
	{
		int32 Secs = FMath::FloorToInt(Seconds);
		Parts.Add(FString::Printf(TEXT("%d second%s"), Secs, Secs != 1 ? TEXT("s") : TEXT("")));
	}

	// Join first 2-3 parts
	int32 MaxParts = FMath::Min(3, Parts.Num());
	FString Result;
	for (int32 i = 0; i < MaxParts; ++i)
	{
		if (i > 0)
		{
			if (i == MaxParts - 1)
			{
				Result += TEXT(" and ");
			}
			else
			{
				Result += TEXT(", ");
			}
		}
		Result += Parts[i];
	}

	return Result;
}

void UTimeWarpWarningWidget::InitializeReferences()
{
	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	if (GameInstance)
	{
		RelativitySystem = GameInstance->GetSubsystem<URelativitySystem>();
	}

	if (!RelativitySystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("TimeWarpWarningWidget: RelativitySystem not found"));
	}
}

void UTimeWarpWarningWidget::SetupButtonCallbacks()
{
	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UTimeWarpWarningWidget::OnConfirmClicked);
	}

	if (CancelButton)
	{
		CancelButton->OnClicked.AddDynamic(this, &UTimeWarpWarningWidget::OnCancelClicked);
	}
}

void UTimeWarpWarningWidget::UpdateWarningDisplay()
{
	// Update title
	if (WarningTitleText)
	{
		ETimeWarpWarningSeverity Severity = GetWarningSeverity(CurrentPrediction.TimeDebtIncrease);
		FString Title = TEXT("TIME WARP WARNING");

		if (Severity == ETimeWarpWarningSeverity::Extreme)
		{
			Title = TEXT("EXTREME TIME WARP WARNING");
		}

		WarningTitleText->SetText(FText::FromString(Title));
	}

	// Update warp factor
	if (TimeWarpFactorText)
	{
		FString FactorString = FString::Printf(TEXT("%.0fx Time Warp"), CurrentWarpFactor);
		TimeWarpFactorText->SetText(FText::FromString(FactorString));
	}

	// Update universe time prediction
	if (UniversalTimeText)
	{
		FString UniverseString = FString::Printf(TEXT("Universe will advance: %s"),
			*FormatTimeDuration(CurrentPrediction.UniverseTimeAdvancement));
		UniversalTimeText->SetText(FText::FromString(UniverseString));
	}

	// Update player time prediction
	if (PlayerTimeText)
	{
		FString PlayerString = FString::Printf(TEXT("You will experience: %s"),
			*FormatTimeDuration(CurrentPrediction.PlayerBiologicalTimeAdvancement));
		PlayerTimeText->SetText(FText::FromString(PlayerString));
	}

	// Update time debt increase
	if (TimeDebtIncreaseText)
	{
		FString DebtString = FString::Printf(TEXT("Time Debt Increase: %s"),
			*CurrentPrediction.FormattedTimeDebt);
		TimeDebtIncreaseText->SetText(FText::FromString(DebtString));

		ETimeWarpWarningSeverity Severity = GetWarningSeverity(CurrentPrediction.TimeDebtIncrease);
		FLinearColor Color = GetSeverityColor(Severity);
		TimeDebtIncreaseText->SetColorAndOpacity(FSlateColor(Color));
	}

	// Update warning message
	if (WarningMessageText)
	{
		ETimeWarpWarningSeverity Severity = GetWarningSeverity(CurrentPrediction.TimeDebtIncrease);
		FString Message = GetWarningMessage(Severity);
		WarningMessageText->SetText(FText::FromString(Message));
	}

	// Update duration display
	if (DurationText)
	{
		FString DurationString = FString::Printf(TEXT("Duration: %.0f seconds"), CurrentDuration);
		DurationText->SetText(FText::FromString(DurationString));
	}

	// Update severity indicator
	ETimeWarpWarningSeverity Severity = GetWarningSeverity(CurrentPrediction.TimeDebtIncrease);
	UpdateSeverityIndicator(Severity);
}

void UTimeWarpWarningWidget::UpdateSeverityIndicator(ETimeWarpWarningSeverity Severity)
{
	FLinearColor Color = GetSeverityColor(Severity);

	// Update severity bar
	if (SeverityBar)
	{
		float BarValue = 0.0f;
		switch (Severity)
		{
			case ETimeWarpWarningSeverity::None:
			case ETimeWarpWarningSeverity::Low:
				BarValue = 0.25f;
				break;
			case ETimeWarpWarningSeverity::Moderate:
				BarValue = 0.5f;
				break;
			case ETimeWarpWarningSeverity::High:
				BarValue = 0.75f;
				break;
			case ETimeWarpWarningSeverity::Extreme:
				BarValue = 1.0f;
				break;
		}

		SeverityBar->SetPercent(BarValue);
		SeverityBar->SetFillColorAndOpacity(Color);
	}

	// Update warning icon
	if (WarningIcon)
	{
		WarningIcon->SetColorAndOpacity(Color);

		// Show/hide based on severity
		bool bShowIcon = (Severity >= ETimeWarpWarningSeverity::Moderate);
		WarningIcon->SetVisibility(bShowIcon ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}
