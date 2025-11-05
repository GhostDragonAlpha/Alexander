// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "DayNightCycleComponent.generated.h"

/**
 * Time of day enumeration for quick reference
 */
UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
	Night,
	Dawn,
	Morning,
	Noon,
	Afternoon,
	Dusk,
	Evening
};

/**
 * Configuration for day-night cycle behavior
 */
USTRUCT(BlueprintType)
struct FDayNightCycleConfig
{
	GENERATED_BODY()

	// Length of a full day in real-time seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float DayLengthInSeconds = 1200.0f; // 20 minutes default

	// Starting time of day (0-24 hours)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float StartTimeOfDay = 12.0f; // Noon

	// Whether time progresses automatically
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	bool bAutoProgress = true;

	// Time progression speed multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float TimeSpeed = 1.0f;

	// Planet's axial tilt in degrees (affects seasons)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planetary")
	float AxialTilt = 23.5f;

	// Planet's rotation axis
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planetary")
	FVector RotationAxis = FVector(0, 0, 1);
};

/**
 * Sky color configuration for different times of day
 */
USTRUCT(BlueprintType)
struct FSkyColorConfig
{
	GENERATED_BODY()

	// Zenith (top of sky) color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor ZenithColor = FLinearColor(0.034f, 0.113f, 0.345f);

	// Horizon color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor HorizonColor = FLinearColor(0.839f, 0.454f, 0.243f);

	// Sun color
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.87f);

	// Sun intensity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colors")
	float SunIntensity = 10.0f;
};

/**
 * Component for managing day-night cycle with realistic sun position and sky colors
 */
UCLASS(ClassGroup=(Planet), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UDayNightCycleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDayNightCycleComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day-Night Cycle")
	FDayNightCycleConfig CycleConfig;

	// ============================================================================
	// TIME MANAGEMENT
	// ============================================================================

	// Current time of day in hours (0-24)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time")
	float CurrentTimeOfDay = 12.0f;

	// Current day number
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Time")
	int32 CurrentDay = 0;

	// Get current time of day category
	UFUNCTION(BlueprintCallable, Category = "Time")
	ETimeOfDay GetTimeOfDayCategory() const;

	// Set time of day manually
	UFUNCTION(BlueprintCallable, Category = "Time")
	void SetTimeOfDay(float Hours);

	// Advance time by hours
	UFUNCTION(BlueprintCallable, Category = "Time")
	void AdvanceTime(float Hours);

	// ============================================================================
	// SUN POSITION
	// ============================================================================

	// Calculate sun position based on time of day
	UFUNCTION(BlueprintCallable, Category = "Sun")
	FVector CalculateSunPosition() const;

	// Calculate sun direction vector
	UFUNCTION(BlueprintCallable, Category = "Sun")
	FVector CalculateSunDirection() const;

	// Get sun elevation angle in degrees (-90 to 90)
	UFUNCTION(BlueprintCallable, Category = "Sun")
	float GetSunElevation() const;

	// Get sun azimuth angle in degrees (0-360)
	UFUNCTION(BlueprintCallable, Category = "Sun")
	float GetSunAzimuth() const;

	// Check if it's currently daytime
	UFUNCTION(BlueprintCallable, Category = "Sun")
	bool IsDaytime() const;

	// ============================================================================
	// SKY COLORS
	// ============================================================================

	// Sky color presets for different times
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Colors")
	FSkyColorConfig NightColors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Colors")
	FSkyColorConfig DawnColors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Colors")
	FSkyColorConfig DayColors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky Colors")
	FSkyColorConfig DuskColors;

	// Get current sky colors based on time of day
	UFUNCTION(BlueprintCallable, Category = "Sky Colors")
	FSkyColorConfig GetCurrentSkyColors() const;

	// Get interpolated zenith color
	UFUNCTION(BlueprintCallable, Category = "Sky Colors")
	FLinearColor GetZenithColor() const;

	// Get interpolated horizon color
	UFUNCTION(BlueprintCallable, Category = "Sky Colors")
	FLinearColor GetHorizonColor() const;

	// Get interpolated sun color
	UFUNCTION(BlueprintCallable, Category = "Sky Colors")
	FLinearColor GetSunColor() const;

	// Get interpolated sun intensity
	UFUNCTION(BlueprintCallable, Category = "Sky Colors")
	float GetSunIntensity() const;

	// ============================================================================
	// STAR VISIBILITY
	// ============================================================================

	// Star visibility factor (0-1, 0=invisible, 1=fully visible)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stars")
	float StarVisibility = 0.0f;

	// Calculate star visibility based on sun position
	UFUNCTION(BlueprintCallable, Category = "Stars")
	float CalculateStarVisibility() const;

	// ============================================================================
	// TWILIGHT EFFECTS
	// ============================================================================

	// Twilight intensity (0-1)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Twilight")
	float TwilightIntensity = 0.0f;

	// Calculate twilight intensity based on sun elevation
	UFUNCTION(BlueprintCallable, Category = "Twilight")
	float CalculateTwilightIntensity() const;

	// Check if currently in twilight period
	UFUNCTION(BlueprintCallable, Category = "Twilight")
	bool IsTwilight() const;

	// Get twilight color tint
	UFUNCTION(BlueprintCallable, Category = "Twilight")
	FLinearColor GetTwilightColor() const;

	// ============================================================================
	// LIGHT INTEGRATION
	// ============================================================================

	// Reference to directional light (sun)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	ADirectionalLight* SunLight;

	// Reference to sky light
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
	ASkyLight* SkyLight;

	// Update lighting based on time of day
	UFUNCTION(BlueprintCallable, Category = "Lighting")
	void UpdateLighting();

	// ============================================================================
	// EVENTS
	// ============================================================================

	// Called when time of day category changes
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeOfDayChanged, ETimeOfDay, NewTimeOfDay);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTimeOfDayChanged OnTimeOfDayChanged;

	// Called when a new day starts
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewDay, int32, DayNumber);
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNewDay OnNewDay;

private:
	// Previous time of day category for change detection
	ETimeOfDay PreviousTimeCategory;

	// Helper function to interpolate between color configs
	FSkyColorConfig InterpolateSkyColors(const FSkyColorConfig& A, const FSkyColorConfig& B, float Alpha) const;

	// Helper function to get blend factor for time transitions
	float GetTimeBlendFactor(float TimeA, float TimeB, float CurrentTime) const;
};
