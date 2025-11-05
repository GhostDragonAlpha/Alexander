// Copyright Epic Games, Inc. All Rights Reserved.

#include "LandingGuidanceWidget.h"
#include "LandingPad.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ULandingGuidanceWidget::ULandingGuidanceWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PreviousPosition = FVector::ZeroVector;
	PreviousTime = 0.0f;
	SmoothedVerticalSpeed = 0.0f;
	SmoothedHorizontalSpeed = 0.0f;
}

void ULandingGuidanceWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Get player pawn if not set
	if (!PlayerPawn)
	{
		PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	}

	// Initialize previous position and time
	if (PlayerPawn)
	{
		PreviousPosition = PlayerPawn->GetActorLocation();
	}
	PreviousTime = GetWorld()->GetTimeSeconds();

	// Initial update
	UpdateGuidanceDisplay();
}

void ULandingGuidanceWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Update guidance display every frame
	UpdateGuidanceDisplay();

	// Update visibility based on conditions
	if (ShouldShowGuidance() && bShowGuidance)
	{
		SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

// ============================================================================
// PUBLIC METHODS
// ============================================================================

void ULandingGuidanceWidget::SetTargetPad(ALandingPad* NewTargetPad)
{
	TargetPad = NewTargetPad;

	// Update pad name display
	if (TargetPad && PadNameText)
	{
		PadNameText->SetText(FText::FromString(TargetPad->PadName));
	}
}

void ULandingGuidanceWidget::SetPlayerPawn(APawn* NewPlayerPawn)
{
	PlayerPawn = NewPlayerPawn;

	// Reset velocity tracking
	if (PlayerPawn)
	{
		PreviousPosition = PlayerPawn->GetActorLocation();
	}
	PreviousTime = GetWorld()->GetTimeSeconds();
	SmoothedVerticalSpeed = 0.0f;
	SmoothedHorizontalSpeed = 0.0f;
}

void ULandingGuidanceWidget::SetGuidanceVisible(bool bShow)
{
	bShowGuidance = bShow;
}

void ULandingGuidanceWidget::UpdateGuidanceDisplay()
{
	if (!PlayerPawn || !TargetPad)
	{
		return;
	}

	// Calculate all values
	float Altitude = CalculateAltitude();
	float VerticalSpeed = CalculateVerticalSpeed();
	float HorizontalSpeed = CalculateHorizontalSpeed();
	float Distance = CalculateDistance();
	float Alignment = CalculateAlignment();

	// Update displays
	UpdateAltitudeDisplay(Altitude);
	UpdateSpeedDisplay(VerticalSpeed, HorizontalSpeed);
	UpdateDistanceDisplay(Distance);
	UpdateAlignmentDisplay(Alignment);
	UpdateStatusMessage();
}

// ============================================================================
// INTERNAL UPDATE METHODS
// ============================================================================

void ULandingGuidanceWidget::UpdateAltitudeDisplay(float Altitude)
{
	if (!AltitudeText)
	{
		return;
	}

	// Convert from cm to meters
	float AltitudeMeters = Altitude / 100.0f;

	// Format based on altitude
	FString AltitudeString;
	if (AltitudeMeters >= 1000.0f)
	{
		// Show in kilometers for high altitudes
		AltitudeString = FString::Printf(TEXT("%.2f km"), AltitudeMeters / 1000.0f);
	}
	else
	{
		// Show in meters for low altitudes
		AltitudeString = FString::Printf(TEXT("%.0f m"), AltitudeMeters);
	}

	AltitudeText->SetText(FText::FromString(AltitudeString));

	// Color based on altitude
	FLinearColor AltitudeColor = FLinearColor::White;
	if (AltitudeMeters < 50.0f)
	{
		AltitudeColor = DangerDescentColor;
	}
	else if (AltitudeMeters < 100.0f)
	{
		AltitudeColor = WarningDescentColor;
	}
	else
	{
		AltitudeColor = SafeDescentColor;
	}

	AltitudeText->SetColorAndOpacity(FSlateColor(AltitudeColor));
}

void ULandingGuidanceWidget::UpdateSpeedDisplay(float VerticalSpeed, float HorizontalSpeed)
{
	// Update vertical speed
	if (VerticalSpeedText)
	{
		FString VerticalSpeedString = FString::Printf(TEXT("%.1f m/s"), VerticalSpeed);
		VerticalSpeedText->SetText(FText::FromString(VerticalSpeedString));

		FLinearColor VerticalSpeedColor = GetVerticalSpeedColor(VerticalSpeed);
		VerticalSpeedText->SetColorAndOpacity(FSlateColor(VerticalSpeedColor));
	}

	// Update horizontal speed
	if (HorizontalSpeedText)
	{
		FString HorizontalSpeedString = FString::Printf(TEXT("%.1f m/s"), HorizontalSpeed);
		HorizontalSpeedText->SetText(FText::FromString(HorizontalSpeedString));

		FLinearColor HorizontalSpeedColor = GetHorizontalSpeedColor(HorizontalSpeed);
		HorizontalSpeedText->SetColorAndOpacity(FSlateColor(HorizontalSpeedColor));
	}
}

void ULandingGuidanceWidget::UpdateDistanceDisplay(float Distance)
{
	if (!DistanceText)
	{
		return;
	}

	// Convert from cm to meters
	float DistanceMeters = Distance / 100.0f;

	// Format based on distance
	FString DistanceString;
	if (DistanceMeters >= 1000.0f)
	{
		// Show in kilometers for long distances
		DistanceString = FString::Printf(TEXT("%.2f km"), DistanceMeters / 1000.0f);
	}
	else
	{
		// Show in meters for short distances
		DistanceString = FString::Printf(TEXT("%.0f m"), DistanceMeters);
	}

	DistanceText->SetText(FText::FromString(DistanceString));

	// Color based on distance
	FLinearColor DistanceColor = FLinearColor::White;
	if (DistanceMeters < 100.0f)
	{
		DistanceColor = SafeDescentColor;
	}
	else if (DistanceMeters < 500.0f)
	{
		DistanceColor = WarningDescentColor;
	}

	DistanceText->SetColorAndOpacity(FSlateColor(DistanceColor));
}

void ULandingGuidanceWidget::UpdateAlignmentDisplay(float Alignment)
{
	// Update alignment text
	if (AlignmentText)
	{
		int32 AlignmentPercent = FMath::RoundToInt(Alignment * 100.0f);
		FString AlignmentString = FString::Printf(TEXT("%d%%"), AlignmentPercent);
		AlignmentText->SetText(FText::FromString(AlignmentString));

		FLinearColor AlignmentColor = GetAlignmentColor(Alignment);
		AlignmentText->SetColorAndOpacity(FSlateColor(AlignmentColor));
	}

	// Update alignment progress bar
	if (AlignmentBar)
	{
		AlignmentBar->SetPercent(Alignment);

		FLinearColor AlignmentColor = GetAlignmentColor(Alignment);
		AlignmentBar->SetFillColorAndOpacity(AlignmentColor);
	}

	// Update alignment indicator image
	if (AlignmentIndicator)
	{
		FLinearColor AlignmentColor = GetAlignmentColor(Alignment);
		AlignmentIndicator->SetColorAndOpacity(AlignmentColor);
	}
}

void ULandingGuidanceWidget::UpdateStatusMessage()
{
	if (!StatusText || !TargetPad)
	{
		return;
	}

	float Altitude = CalculateAltitude() / 100.0f; // Convert to meters
	float VerticalSpeed = CalculateVerticalSpeed();
	float HorizontalSpeed = CalculateHorizontalSpeed();
	float Distance = CalculateDistance() / 100.0f; // Convert to meters
	float Alignment = CalculateAlignment();

	FString StatusMessage;

	// Determine status message based on conditions
	if (Distance > 1000.0f)
	{
		StatusMessage = TEXT("Approaching landing zone");
	}
	else if (Alignment < 0.5f)
	{
		StatusMessage = TEXT("Align with approach vector");
	}
	else if (HorizontalSpeed > WarningHorizontalSpeed)
	{
		StatusMessage = TEXT("Reduce horizontal speed");
	}
	else if (VerticalSpeed < DangerDescentRate)
	{
		StatusMessage = TEXT("WARNING: Descent rate too high!");
	}
	else if (VerticalSpeed < WarningDescentRate)
	{
		StatusMessage = TEXT("Reduce descent rate");
	}
	else if (Altitude < 50.0f && FMath::Abs(VerticalSpeed) > 2.0f)
	{
		StatusMessage = TEXT("Slow descent for landing");
	}
	else if (Altitude < 50.0f && Alignment > 0.8f && FMath::Abs(VerticalSpeed) < 2.0f)
	{
		StatusMessage = TEXT("Landing approach good");
	}
	else if (Altitude < 100.0f)
	{
		StatusMessage = TEXT("Final approach");
	}
	else
	{
		StatusMessage = TEXT("Continue approach");
	}

	StatusText->SetText(FText::FromString(StatusMessage));

	// Color based on status
	FLinearColor StatusColor = FLinearColor::White;
	if (StatusMessage.Contains(TEXT("WARNING")))
	{
		StatusColor = DangerDescentColor;
	}
	else if (StatusMessage.Contains(TEXT("Reduce")) || StatusMessage.Contains(TEXT("Slow")))
	{
		StatusColor = WarningDescentColor;
	}
	else if (StatusMessage.Contains(TEXT("good")))
	{
		StatusColor = SafeDescentColor;
	}

	StatusText->SetColorAndOpacity(FSlateColor(StatusColor));
}

FLinearColor ULandingGuidanceWidget::GetAlignmentColor(float Alignment) const
{
	if (Alignment >= 0.8f)
	{
		return GoodAlignmentColor;
	}
	else if (Alignment >= 0.5f)
	{
		return ModerateAlignmentColor;
	}
	else
	{
		return PoorAlignmentColor;
	}
}

FLinearColor ULandingGuidanceWidget::GetVerticalSpeedColor(float VerticalSpeed) const
{
	if (VerticalSpeed < DangerDescentRate)
	{
		return DangerDescentColor;
	}
	else if (VerticalSpeed < WarningDescentRate)
	{
		return WarningDescentColor;
	}
	else
	{
		return SafeDescentColor;
	}
}

FLinearColor ULandingGuidanceWidget::GetHorizontalSpeedColor(float HorizontalSpeed) const
{
	if (HorizontalSpeed > WarningHorizontalSpeed)
	{
		return DangerDescentColor;
	}
	else if (HorizontalSpeed > SafeHorizontalSpeed)
	{
		return WarningDescentColor;
	}
	else
	{
		return SafeDescentColor;
	}
}

bool ULandingGuidanceWidget::ShouldShowGuidance() const
{
	if (!PlayerPawn || !TargetPad)
	{
		return false;
	}

	// Check distance
	float Distance = CalculateDistance() / 100.0f; // Convert to meters
	if (Distance > MaxGuidanceDistance)
	{
		return false;
	}

	// Check altitude
	float Altitude = CalculateAltitude() / 100.0f; // Convert to meters
	if (Altitude < MinGuidanceAltitude || Altitude > MaxGuidanceAltitude)
	{
		return false;
	}

	return true;
}

// ============================================================================
// CALCULATION METHODS
// ============================================================================

float ULandingGuidanceWidget::CalculateAltitude() const
{
	if (!PlayerPawn || !TargetPad)
	{
		return 0.0f;
	}

	FVector PlayerLocation = PlayerPawn->GetActorLocation();
	FVector PadLocation = TargetPad->GetActorLocation();

	// Calculate altitude as vertical distance
	float Altitude = PlayerLocation.Z - PadLocation.Z;

	return FMath::Max(0.0f, Altitude);
}

float ULandingGuidanceWidget::CalculateVerticalSpeed() const
{
	if (!PlayerPawn)
	{
		return 0.0f;
	}

	FVector CurrentPosition = PlayerPawn->GetActorLocation();
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float DeltaTime = CurrentTime - PreviousTime;

	if (DeltaTime <= 0.0f)
	{
		return SmoothedVerticalSpeed;
	}

	// Calculate vertical velocity in cm/s
	float VerticalVelocity = (CurrentPosition.Z - PreviousPosition.Z) / DeltaTime;

	// Convert to m/s
	float VerticalSpeedMS = VerticalVelocity / 100.0f;

	// Apply smoothing
	SmoothedVerticalSpeed = FMath::Lerp(VerticalSpeedMS, SmoothedVerticalSpeed, SpeedSmoothingFactor);

	// Update previous values
	const_cast<ULandingGuidanceWidget*>(this)->PreviousPosition = CurrentPosition;
	const_cast<ULandingGuidanceWidget*>(this)->PreviousTime = CurrentTime;

	return SmoothedVerticalSpeed;
}

float ULandingGuidanceWidget::CalculateHorizontalSpeed() const
{
	if (!PlayerPawn)
	{
		return 0.0f;
	}

	FVector CurrentPosition = PlayerPawn->GetActorLocation();
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float DeltaTime = CurrentTime - PreviousTime;

	if (DeltaTime <= 0.0f)
	{
		return SmoothedHorizontalSpeed;
	}

	// Calculate horizontal velocity (ignore Z component)
	FVector HorizontalDelta = CurrentPosition - PreviousPosition;
	HorizontalDelta.Z = 0.0f;
	float HorizontalVelocity = HorizontalDelta.Size() / DeltaTime;

	// Convert to m/s
	float HorizontalSpeedMS = HorizontalVelocity / 100.0f;

	// Apply smoothing
	SmoothedHorizontalSpeed = FMath::Lerp(HorizontalSpeedMS, SmoothedHorizontalSpeed, SpeedSmoothingFactor);

	return SmoothedHorizontalSpeed;
}

float ULandingGuidanceWidget::CalculateDistance() const
{
	if (!PlayerPawn || !TargetPad)
	{
		return 0.0f;
	}

	FVector PlayerLocation = PlayerPawn->GetActorLocation();
	FVector PadLocation = TargetPad->GetActorLocation();

	// Calculate 3D distance
	float Distance = FVector::Dist(PlayerLocation, PadLocation);

	return Distance;
}

float ULandingGuidanceWidget::CalculateAlignment() const
{
	if (!PlayerPawn || !TargetPad)
	{
		return 0.0f;
	}

	// Use the landing pad's approach corridor calculation
	float Distance;
	float Alignment;
	TargetPad->IsInApproachCorridor(PlayerPawn, Distance, Alignment);

	return Alignment;
}
