// Copyright Epic Games, Inc. All Rights Reserved.

#include "DayNightCycleComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObject/UObjectIterator.h"
#include "EngineUtils.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Vector.h"
#include "Math/Quat.h"
#include "Math/Rotator.h"

UDayNightCycleComponent::UDayNightCycleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	// Initialize default sky colors
	
	// Night colors (dark blue sky, dim stars visible)
	NightColors.ZenithColor = FLinearColor(0.001f, 0.002f, 0.01f);
	NightColors.HorizonColor = FLinearColor(0.01f, 0.02f, 0.05f);
	NightColors.SunColor = FLinearColor(0.5f, 0.6f, 0.8f);
	NightColors.SunIntensity = 0.1f;

	// Dawn colors (orange/pink horizon, blue zenith)
	DawnColors.ZenithColor = FLinearColor(0.2f, 0.3f, 0.6f);
	DawnColors.HorizonColor = FLinearColor(1.0f, 0.5f, 0.3f);
	DawnColors.SunColor = FLinearColor(1.0f, 0.7f, 0.5f);
	DawnColors.SunIntensity = 3.0f;

	// Day colors (bright blue sky)
	DayColors.ZenithColor = FLinearColor(0.26f, 0.52f, 0.96f);
	DayColors.HorizonColor = FLinearColor(0.89f, 0.92f, 0.98f);
	DayColors.SunColor = FLinearColor(1.0f, 0.98f, 0.95f);
	DayColors.SunIntensity = 10.0f;

	// Dusk colors (orange/red horizon, darkening zenith)
	DuskColors.ZenithColor = FLinearColor(0.1f, 0.15f, 0.3f);
	DuskColors.HorizonColor = FLinearColor(1.0f, 0.4f, 0.2f);
	DuskColors.SunColor = FLinearColor(1.0f, 0.5f, 0.3f);
	DuskColors.SunIntensity = 2.0f;

	CurrentTimeOfDay = 12.0f;
	CurrentDay = 0;
	PreviousTimeCategory = ETimeOfDay::Noon;
}

void UDayNightCycleComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize time from config
	CurrentTimeOfDay = CycleConfig.StartTimeOfDay;
	PreviousTimeCategory = GetTimeOfDayCategory();

	// Find sun light if not assigned
	if (!SunLight)
	{
		// Search for directional light in the level
		for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
		{
			SunLight = *It;
			UE_LOG(LogTemp, Log, TEXT("Day-Night Cycle: Found directional light '%s'"), *SunLight->GetName());
			break;
		}
	}

	// Find sky light if not assigned
	if (!SkyLight)
	{
		// Search for sky light in the level
		for (TActorIterator<ASkyLight> It(GetWorld()); It; ++It)
		{
			SkyLight = *It;
			UE_LOG(LogTemp, Log, TEXT("Day-Night Cycle: Found sky light '%s'"), *SkyLight->GetName());
			break;
		}
	}

	// Initial lighting update
	UpdateLighting();

	UE_LOG(LogTemp, Log, TEXT("Day-Night Cycle initialized: Start time %.2f hours, Day length %.1f seconds"), 
		CurrentTimeOfDay, CycleConfig.DayLengthInSeconds);
}

void UDayNightCycleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Progress time if auto-progress is enabled
	if (CycleConfig.bAutoProgress)
	{
		// Calculate hours to advance based on day length
		float HoursPerSecond = 24.0f / CycleConfig.DayLengthInSeconds;
		float HoursToAdvance = HoursPerSecond * DeltaTime * CycleConfig.TimeSpeed;
		
		AdvanceTime(HoursToAdvance);
	}

	// Update star visibility
	StarVisibility = CalculateStarVisibility();

	// Update twilight intensity
	TwilightIntensity = CalculateTwilightIntensity();

	// Update lighting
	UpdateLighting();

	// Check for time of day category change
	ETimeOfDay CurrentCategory = GetTimeOfDayCategory();
	if (CurrentCategory != PreviousTimeCategory)
	{
		OnTimeOfDayChanged.Broadcast(CurrentCategory);
		PreviousTimeCategory = CurrentCategory;
		
		UE_LOG(LogTemp, Log, TEXT("Time of day changed to: %s"), 
			*UEnum::GetValueAsString(CurrentCategory));
	}
}

// ============================================================================
// TIME MANAGEMENT
// ============================================================================

ETimeOfDay UDayNightCycleComponent::GetTimeOfDayCategory() const
{
	if (CurrentTimeOfDay >= 22.0f || CurrentTimeOfDay < 4.0f)
		return ETimeOfDay::Night;
	else if (CurrentTimeOfDay >= 4.0f && CurrentTimeOfDay < 6.0f)
		return ETimeOfDay::Dawn;
	else if (CurrentTimeOfDay >= 6.0f && CurrentTimeOfDay < 10.0f)
		return ETimeOfDay::Morning;
	else if (CurrentTimeOfDay >= 10.0f && CurrentTimeOfDay < 14.0f)
		return ETimeOfDay::Noon;
	else if (CurrentTimeOfDay >= 14.0f && CurrentTimeOfDay < 18.0f)
		return ETimeOfDay::Afternoon;
	else if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay < 20.0f)
		return ETimeOfDay::Dusk;
	else
		return ETimeOfDay::Evening;
}

void UDayNightCycleComponent::SetTimeOfDay(float Hours)
{
	CurrentTimeOfDay = FMath::Fmod(Hours, 24.0f);
	if (CurrentTimeOfDay < 0.0f)
	{
		CurrentTimeOfDay += 24.0f;
	}
}

void UDayNightCycleComponent::AdvanceTime(float Hours)
{
	float OldTime = CurrentTimeOfDay;
	CurrentTimeOfDay += Hours;

	// Handle day rollover
	while (CurrentTimeOfDay >= 24.0f)
	{
		CurrentTimeOfDay -= 24.0f;
		CurrentDay++;
		OnNewDay.Broadcast(CurrentDay);
		
		UE_LOG(LogTemp, Log, TEXT("New day started: Day %d"), CurrentDay);
	}

	// Handle negative time (going backwards)
	while (CurrentTimeOfDay < 0.0f)
	{
		CurrentTimeOfDay += 24.0f;
		CurrentDay--;
	}
}

// ============================================================================
// SUN POSITION
// ============================================================================

FVector UDayNightCycleComponent::CalculateSunPosition() const
{
	// Calculate sun position based on time of day
	// Sun moves in a circular path around the planet
	
	// Convert time to angle (0-360 degrees)
	// 0:00 = 0°, 6:00 = 90°, 12:00 = 180°, 18:00 = 270°
	float TimeAngle = (CurrentTimeOfDay / 24.0f) * 360.0f;
	
	// Adjust so that 12:00 (noon) is at zenith (90° elevation)
	// Subtract 90 so that 6:00 is at horizon (0°)
	float SunAngle = TimeAngle - 90.0f;
	
	// Convert to radians
	float SunAngleRad = FMath::DegreesToRadians(SunAngle);
	
	// Calculate position on unit sphere
	// X = forward/back, Y = left/right, Z = up/down
	FVector SunPos;
	SunPos.X = FMath::Cos(SunAngleRad);
	SunPos.Y = 0.0f; // Sun moves along X-Z plane
	SunPos.Z = FMath::Sin(SunAngleRad);
	
	// Apply axial tilt
	FQuat TiltRotation = FQuat(CycleConfig.RotationAxis, FMath::DegreesToRadians(CycleConfig.AxialTilt));
	SunPos = TiltRotation.RotateVector(SunPos);
	
	return SunPos.GetSafeNormal();
}

FVector UDayNightCycleComponent::CalculateSunDirection() const
{
	// Sun direction is opposite of sun position (light comes from the sun)
	return -CalculateSunPosition();
}

float UDayNightCycleComponent::GetSunElevation() const
{
	FVector SunPos = CalculateSunPosition();
	
	// Elevation is the angle above the horizon
	// Z component gives us the sine of the elevation angle
	float Elevation = FMath::Asin(SunPos.Z);
	
	return FMath::RadiansToDegrees(Elevation);
}

float UDayNightCycleComponent::GetSunAzimuth() const
{
	FVector SunPos = CalculateSunPosition();
	
	// Azimuth is the compass direction (0° = North, 90° = East, etc.)
	float Azimuth = FMath::Atan2(SunPos.Y, SunPos.X);
	
	// Convert to degrees and normalize to 0-360
	float AzimuthDegrees = FMath::RadiansToDegrees(Azimuth);
	if (AzimuthDegrees < 0.0f)
	{
		AzimuthDegrees += 360.0f;
	}
	
	return AzimuthDegrees;
}

bool UDayNightCycleComponent::IsDaytime() const
{
	// Daytime is when sun elevation is above 0 degrees
	return GetSunElevation() > 0.0f;
}

// ============================================================================
// SKY COLORS
// ============================================================================

FSkyColorConfig UDayNightCycleComponent::GetCurrentSkyColors() const
{
	float SunElevation = GetSunElevation();
	
	// Define elevation thresholds for color transitions
	const float NightElevation = -18.0f;  // Astronomical twilight
	const float DawnDuskElevation = -6.0f; // Civil twilight
	const float DayElevation = 10.0f;      // Full daylight
	
	FSkyColorConfig Result;
	
	if (SunElevation < NightElevation)
	{
		// Full night
		Result = NightColors;
	}
	else if (SunElevation < DawnDuskElevation)
	{
		// Astronomical to civil twilight
		float Alpha = (SunElevation - NightElevation) / (DawnDuskElevation - NightElevation);
		Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
		
		// Determine if dawn or dusk based on time
		bool bIsDawn = (CurrentTimeOfDay >= 0.0f && CurrentTimeOfDay < 12.0f);
		Result = InterpolateSkyColors(NightColors, bIsDawn ? DawnColors : DuskColors, Alpha);
	}
	else if (SunElevation < DayElevation)
	{
		// Civil twilight to full day
		float Alpha = (SunElevation - DawnDuskElevation) / (DayElevation - DawnDuskElevation);
		Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
		
		// Determine if dawn or dusk based on time
		bool bIsDawn = (CurrentTimeOfDay >= 0.0f && CurrentTimeOfDay < 12.0f);
		Result = InterpolateSkyColors(bIsDawn ? DawnColors : DuskColors, DayColors, Alpha);
	}
	else
	{
		// Full daylight
		Result = DayColors;
	}
	
	return Result;
}

FLinearColor UDayNightCycleComponent::GetZenithColor() const
{
	return GetCurrentSkyColors().ZenithColor;
}

FLinearColor UDayNightCycleComponent::GetHorizonColor() const
{
	return GetCurrentSkyColors().HorizonColor;
}

FLinearColor UDayNightCycleComponent::GetSunColor() const
{
	return GetCurrentSkyColors().SunColor;
}

float UDayNightCycleComponent::GetSunIntensity() const
{
	return GetCurrentSkyColors().SunIntensity;
}

// ============================================================================
// STAR VISIBILITY
// ============================================================================

float UDayNightCycleComponent::CalculateStarVisibility() const
{
	float SunElevation = GetSunElevation();
	
	// Stars become visible when sun is below horizon
	// Full visibility at -18° (astronomical twilight)
	// No visibility at 0° (horizon)
	
	const float FullVisibilityElevation = -18.0f;
	const float NoVisibilityElevation = 0.0f;
	
	if (SunElevation >= NoVisibilityElevation)
	{
		return 0.0f; // No stars during day
	}
	else if (SunElevation <= FullVisibilityElevation)
	{
		return 1.0f; // Full star visibility at night
	}
	else
	{
		// Interpolate between full and no visibility
		float Alpha = (NoVisibilityElevation - SunElevation) / (NoVisibilityElevation - FullVisibilityElevation);
		return FMath::Clamp(Alpha, 0.0f, 1.0f);
	}
}

// ============================================================================
// TWILIGHT EFFECTS
// ============================================================================

float UDayNightCycleComponent::CalculateTwilightIntensity() const
{
	float SunElevation = GetSunElevation();
	
	// Twilight occurs when sun is between -18° and +6° elevation
	const float TwilightStart = -18.0f;
	const float TwilightEnd = 6.0f;
	
	if (SunElevation < TwilightStart || SunElevation > TwilightEnd)
	{
		return 0.0f; // No twilight
	}
	
	// Peak twilight intensity at horizon (0°)
	float DistanceFromHorizon = FMath::Abs(SunElevation);
	float MaxDistance = FMath::Max(FMath::Abs(TwilightStart), FMath::Abs(TwilightEnd));
	
	float Intensity = 1.0f - (DistanceFromHorizon / MaxDistance);
	return FMath::Clamp(Intensity, 0.0f, 1.0f);
}

bool UDayNightCycleComponent::IsTwilight() const
{
	return TwilightIntensity > 0.1f;
}

FLinearColor UDayNightCycleComponent::GetTwilightColor() const
{
	// Twilight color varies from orange/red at horizon to purple/blue higher up
	float SunElevation = GetSunElevation();
	
	FLinearColor OrangeRed = FLinearColor(1.0f, 0.4f, 0.2f);
	FLinearColor PurpleBlue = FLinearColor(0.4f, 0.2f, 0.6f);
	
	// Interpolate based on sun elevation
	float Alpha = FMath::Clamp((SunElevation + 6.0f) / 12.0f, 0.0f, 1.0f);
	
	return FMath::Lerp(OrangeRed, PurpleBlue, Alpha);
}

// ============================================================================
// LIGHT INTEGRATION
// ============================================================================

void UDayNightCycleComponent::UpdateLighting()
{
	// Update sun light
	if (SunLight && SunLight->GetLightComponent())
	{
		UDirectionalLightComponent* LightComp = Cast<UDirectionalLightComponent>(SunLight->GetLightComponent());
		
		// Update sun rotation to match calculated position
		FVector SunDirection = CalculateSunDirection();
		FRotator SunRotation = SunDirection.Rotation();
		SunLight->SetActorRotation(SunRotation);
		
		// Update sun color and intensity
		FLinearColor SunColor = GetSunColor();
		float SunIntensity = GetSunIntensity();
		
		LightComp->SetLightColor(SunColor);
		LightComp->SetIntensity(SunIntensity);
		
		// Adjust light based on whether sun is below horizon
		if (!IsDaytime())
		{
			// Reduce intensity significantly when sun is below horizon
			LightComp->SetIntensity(SunIntensity * 0.1f);
		}
	}
	
	// Update sky light
	if (SkyLight && SkyLight->GetLightComponent())
	{
		USkyLightComponent* SkyLightComp = SkyLight->GetLightComponent();
		
		// Update sky light color based on time of day
		FLinearColor SkyColor = GetZenithColor();
		SkyLightComp->SetLightColor(SkyColor);
		
		// Adjust intensity based on time of day
		float BaseIntensity = 1.0f;
		if (!IsDaytime())
		{
			// Reduce sky light at night
			BaseIntensity = 0.1f + (StarVisibility * 0.2f);
		}
		else if (IsTwilight())
		{
			// Moderate intensity during twilight
			BaseIntensity = 0.5f + (TwilightIntensity * 0.3f);
		}
		
		SkyLightComp->SetIntensity(BaseIntensity);
		
		// Recapture sky light to update reflections
		SkyLightComp->RecaptureSky();
	}
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

FSkyColorConfig UDayNightCycleComponent::InterpolateSkyColors(const FSkyColorConfig& A, const FSkyColorConfig& B, float Alpha) const
{
	FSkyColorConfig Result;
	
	Result.ZenithColor = FMath::Lerp(A.ZenithColor, B.ZenithColor, Alpha);
	Result.HorizonColor = FMath::Lerp(A.HorizonColor, B.HorizonColor, Alpha);
	Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
	Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
	
	return Result;
}

float UDayNightCycleComponent::GetTimeBlendFactor(float TimeA, float TimeB, float CurrentTime) const
{
	// Handle wrapping around midnight
	if (TimeB < TimeA)
	{
		if (CurrentTime < TimeA)
		{
			CurrentTime += 24.0f;
		}
		TimeB += 24.0f;
	}
	
	if (CurrentTime < TimeA || CurrentTime > TimeB)
	{
		return 0.0f;
	}
	
	float Range = TimeB - TimeA;
	if (Range < 0.001f)
	{
		return 0.0f;
	}
	
	return (CurrentTime - TimeA) / Range;
}
