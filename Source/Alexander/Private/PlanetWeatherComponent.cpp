// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlanetWeatherComponent.h"
#include "PlanetCloudComponent.h"
#include "PlanetAtmosphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UPlanetWeatherComponent::UPlanetWeatherComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz update for smooth transitions
	
	// Default configuration
	WeatherChangeInterval = 300.0f;
	MinimumStableDuration = 120.0f;
	DefaultTransitionDuration = 60.0f;
	bEnableAutomaticWeatherChanges = true;
	
	CurrentState = EWeatherState::Stable;
	CurrentWeather = EWeatherType::Clear;
	TimeInStableState = 0.0f;
	TimeSinceLastEvaluation = 0.0f;
}

void UPlanetWeatherComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize with clear weather
	CurrentWeather = EWeatherType::Clear;
	CurrentState = EWeatherState::Stable;
	
	// Start evaluation timer if automatic changes enabled
	if (bEnableAutomaticWeatherChanges && WeatherPresets.Num() > 0)
	{
		EnterEvaluatingState();
	}
}

void UPlanetWeatherComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Update state machine
	UpdateStateMachine(DeltaTime);
	
	// Update weather effects
	UpdateWeatherEffects(DeltaTime);
}

// ===== State Machine Implementation =====

void UPlanetWeatherComponent::UpdateStateMachine(float DeltaTime)
{
	switch (CurrentState)
	{
	case EWeatherState::Stable:
		HandleStableState(DeltaTime);
		break;
		
	case EWeatherState::Transitioning:
		HandleTransitioningState(DeltaTime);
		break;
		
	case EWeatherState::Evaluating:
		HandleEvaluatingState(DeltaTime);
		break;
	}
}

void UPlanetWeatherComponent::HandleStableState(float DeltaTime)
{
	TimeInStableState += DeltaTime;
	TimeSinceLastEvaluation += DeltaTime;
	
	// Check if it's time to evaluate weather change
	if (bEnableAutomaticWeatherChanges && TimeSinceLastEvaluation >= WeatherChangeInterval)
	{
		EnterEvaluatingState();
	}
}

void UPlanetWeatherComponent::HandleTransitioningState(float DeltaTime)
{
	// Update transition progress
	ActiveTransition.ElapsedTime += DeltaTime;
	ActiveTransition.Progress = ActiveTransition.ElapsedTime / ActiveTransition.Duration;
	
	// Check if transition is complete
	if (ActiveTransition.IsComplete())
	{
		// Finalize transition
		CurrentWeather = ActiveTransition.ToWeather;
		EnterStableState();
	}
}

void UPlanetWeatherComponent::HandleEvaluatingState(float DeltaTime)
{
	// Check if minimum stable duration has passed
	if (TimeInStableState < MinimumStableDuration)
	{
		// Not ready to change yet, go back to stable
		EnterStableState();
		return;
	}
	
	// Select next weather
	EWeatherType NextWeather = SelectNextWeather();
	
	// Check if weather should change
	if (NextWeather != CurrentWeather)
	{
		// Start transition
		float TransitionDuration = DefaultTransitionDuration;
		
		// Find preset for transition duration override
		for (const FWeatherPreset& Preset : WeatherPresets)
		{
			if (Preset.WeatherType == NextWeather && Preset.TransitionDuration > 0.0f)
			{
				TransitionDuration = Preset.TransitionDuration;
				break;
			}
		}
		
		EnterTransitioningState(NextWeather, TransitionDuration);
	}
	else
	{
		// Stay in current weather
		EnterStableState();
	}
}

void UPlanetWeatherComponent::EnterStableState()
{
	CurrentState = EWeatherState::Stable;
	TimeInStableState = 0.0f;
	TimeSinceLastEvaluation = 0.0f;
}

void UPlanetWeatherComponent::EnterTransitioningState(EWeatherType ToWeather, float Duration)
{
	CurrentState = EWeatherState::Transitioning;
	
	ActiveTransition.FromWeather = CurrentWeather;
	ActiveTransition.ToWeather = ToWeather;
	ActiveTransition.Duration = FMath::Max(Duration, 1.0f); // Minimum 1 second
	ActiveTransition.ElapsedTime = 0.0f;
	ActiveTransition.Progress = 0.0f;
}

void UPlanetWeatherComponent::EnterEvaluatingState()
{
	CurrentState = EWeatherState::Evaluating;
}

// ===== Weather Selection =====

EWeatherType UPlanetWeatherComponent::SelectNextWeather()
{
	if (WeatherPresets.Num() == 0)
	{
		return CurrentWeather;
	}
	
	// Calculate total weight
	float TotalWeight = 0.0f;
	for (const FWeatherPreset& Preset : WeatherPresets)
	{
		TotalWeight += Preset.Probability;
	}
	
	if (TotalWeight <= 0.0f)
	{
		return CurrentWeather;
	}
	
	// Random selection based on weights
	float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
	float AccumulatedWeight = 0.0f;
	
	for (const FWeatherPreset& Preset : WeatherPresets)
	{
		AccumulatedWeight += Preset.Probability;
		if (RandomValue <= AccumulatedWeight)
		{
			// Check if transition is valid
			if (IsValidWeatherTransition(CurrentWeather, Preset.WeatherType))
			{
				return Preset.WeatherType;
			}
		}
	}
	
	// Fallback to current weather
	return CurrentWeather;
}

bool UPlanetWeatherComponent::IsValidWeatherTransition(EWeatherType From, EWeatherType To) const
{
	// All transitions are valid by default
	// Can add logic here to prevent certain transitions (e.g., Clear -> Blizzard)
	return true;
}

// ===== Weather Control =====

void UPlanetWeatherComponent::TransitionToWeather(EWeatherType NewWeather, float Duration)
{
	if (NewWeather == CurrentWeather)
	{
		return;
	}
	
	float TransitionDuration = Duration > 0.0f ? Duration : DefaultTransitionDuration;
	EnterTransitioningState(NewWeather, TransitionDuration);
}

void UPlanetWeatherComponent::SetWeatherImmediate(EWeatherType NewWeather)
{
	CurrentWeather = NewWeather;
	EnterStableState();
	
	// Update effects immediately
	UpdateWeatherEffects(0.0f);
}

// ===== Weather Queries =====

FLinearColor UPlanetWeatherComponent::GetWeatherLightingMultiplier() const
{
	FLinearColor CurrentColor = FLinearColor::White;
	FLinearColor TargetColor = FLinearColor::White;
	
	// Get colors based on weather types
	auto GetColorForWeather = [](EWeatherType Weather) -> FLinearColor
	{
		switch (Weather)
		{
		case EWeatherType::Clear:
			return FLinearColor::White;
		case EWeatherType::PartlyCloudy:
			return FLinearColor(0.95f, 0.95f, 0.98f);
		case EWeatherType::Cloudy:
			return FLinearColor(0.8f, 0.8f, 0.85f);
		case EWeatherType::Overcast:
			return FLinearColor(0.6f, 0.6f, 0.65f);
		case EWeatherType::Foggy:
			return FLinearColor(0.7f, 0.7f, 0.75f);
		case EWeatherType::LightRain:
			return FLinearColor(0.7f, 0.7f, 0.8f);
		case EWeatherType::Rain:
			return FLinearColor(0.5f, 0.5f, 0.6f);
		case EWeatherType::HeavyRain:
			return FLinearColor(0.4f, 0.4f, 0.5f);
		case EWeatherType::Thunderstorm:
			return FLinearColor(0.3f, 0.3f, 0.4f);
		case EWeatherType::LightSnow:
			return FLinearColor(0.9f, 0.9f, 0.95f);
		case EWeatherType::Snow:
			return FLinearColor(0.85f, 0.85f, 0.9f);
		case EWeatherType::Blizzard:
			return FLinearColor(0.6f, 0.6f, 0.7f);
		case EWeatherType::Sandstorm:
			return FLinearColor(0.8f, 0.7f, 0.5f);
		case EWeatherType::AcidRain:
			return FLinearColor(0.6f, 0.7f, 0.5f);
		case EWeatherType::ToxicFog:
			return FLinearColor(0.5f, 0.7f, 0.4f);
		default:
			return FLinearColor::White;
		}
	};
	
	if (CurrentState == EWeatherState::Transitioning)
	{
		CurrentColor = GetColorForWeather(ActiveTransition.FromWeather);
		TargetColor = GetColorForWeather(ActiveTransition.ToWeather);
		return FMath::Lerp(CurrentColor, TargetColor, ActiveTransition.GetAlpha());
	}
	
	return GetColorForWeather(CurrentWeather);
}

float UPlanetWeatherComponent::GetWetnessAmount() const
{
	float CurrentWetness = 0.0f;
	float TargetWetness = 0.0f;
	
	// Get wetness based on weather type
	auto GetWetnessForWeather = [](EWeatherType Weather) -> float
	{
		switch (Weather)
		{
		case EWeatherType::LightRain:
			return 0.3f;
		case EWeatherType::Rain:
			return 0.6f;
		case EWeatherType::HeavyRain:
			return 0.9f;
		case EWeatherType::Thunderstorm:
			return 1.0f;
		case EWeatherType::AcidRain:
			return 0.8f;
		case EWeatherType::Foggy:
			return 0.2f;
		case EWeatherType::ToxicFog:
			return 0.3f;
		default:
			return 0.0f;
		}
	};
	
	if (CurrentState == EWeatherState::Transitioning)
	{
		CurrentWetness = GetWetnessForWeather(ActiveTransition.FromWeather);
		TargetWetness = GetWetnessForWeather(ActiveTransition.ToWeather);
		return FMath::Lerp(CurrentWetness, TargetWetness, ActiveTransition.GetAlpha());
	}
	
	return GetWetnessForWeather(CurrentWeather);
}

float UPlanetWeatherComponent::GetCloudCoverageForWeather(EWeatherType WeatherType) const
{
	switch (WeatherType)
	{
	case EWeatherType::Clear:
		return 0.0f;
	case EWeatherType::PartlyCloudy:
		return 0.3f;
	case EWeatherType::Cloudy:
		return 0.6f;
	case EWeatherType::Overcast:
		return 0.9f;
	case EWeatherType::Foggy:
		return 0.5f;
	case EWeatherType::LightRain:
	case EWeatherType::Rain:
	case EWeatherType::HeavyRain:
	case EWeatherType::Thunderstorm:
		return 0.95f;
	case EWeatherType::LightSnow:
	case EWeatherType::Snow:
	case EWeatherType::Blizzard:
		return 0.85f;
	case EWeatherType::Sandstorm:
		return 0.7f;
	case EWeatherType::AcidRain:
	case EWeatherType::ToxicFog:
		return 0.8f;
	default:
		return 0.0f;
	}
}

float UPlanetWeatherComponent::GetCloudDensityForWeather(EWeatherType WeatherType) const
{
	switch (WeatherType)
	{
	case EWeatherType::Clear:
		return 0.1f;
	case EWeatherType::PartlyCloudy:
		return 0.3f;
	case EWeatherType::Cloudy:
		return 0.5f;
	case EWeatherType::Overcast:
		return 0.7f;
	case EWeatherType::Foggy:
		return 0.9f;
	case EWeatherType::LightRain:
		return 0.6f;
	case EWeatherType::Rain:
		return 0.7f;
	case EWeatherType::HeavyRain:
		return 0.8f;
	case EWeatherType::Thunderstorm:
		return 0.9f;
	case EWeatherType::LightSnow:
		return 0.5f;
	case EWeatherType::Snow:
		return 0.6f;
	case EWeatherType::Blizzard:
		return 0.9f;
	case EWeatherType::Sandstorm:
		return 0.85f;
	case EWeatherType::AcidRain:
		return 0.75f;
	case EWeatherType::ToxicFog:
		return 0.95f;
	default:
		return 0.2f;
	}
}

bool UPlanetWeatherComponent::IsStormyWeather(EWeatherType WeatherType) const
{
	return WeatherType == EWeatherType::HeavyRain ||
	       WeatherType == EWeatherType::Thunderstorm ||
	       WeatherType == EWeatherType::Blizzard ||
	       WeatherType == EWeatherType::Sandstorm;
}

float UPlanetWeatherComponent::GetInterpolatedWeatherValue(float CurrentValue, float TargetValue) const
{
	if (CurrentState == EWeatherState::Transitioning)
	{
		return FMath::Lerp(CurrentValue, TargetValue, ActiveTransition.GetAlpha());
	}
	return CurrentValue;
}

// ===== Weather Lighting =====

float UPlanetWeatherComponent::GetWeatherFogDensity() const
{
	float CurrentDensity = 0.0f;
	float TargetDensity = 0.0f;
	
	auto GetDensityForWeather = [](EWeatherType Weather) -> float
	{
		switch (Weather)
		{
		case EWeatherType::Clear:
		case EWeatherType::PartlyCloudy:
			return 0.0f;
		case EWeatherType::Cloudy:
			return 0.01f;
		case EWeatherType::Overcast:
			return 0.02f;
		case EWeatherType::Foggy:
			return 0.1f;
		case EWeatherType::LightRain:
			return 0.03f;
		case EWeatherType::Rain:
			return 0.05f;
		case EWeatherType::HeavyRain:
			return 0.08f;
		case EWeatherType::Thunderstorm:
			return 0.1f;
		case EWeatherType::LightSnow:
			return 0.04f;
		case EWeatherType::Snow:
			return 0.07f;
		case EWeatherType::Blizzard:
			return 0.15f;
		case EWeatherType::Sandstorm:
			return 0.12f;
		case EWeatherType::AcidRain:
			return 0.06f;
		case EWeatherType::ToxicFog:
			return 0.2f;
		default:
			return 0.0f;
		}
	};
	
	if (CurrentState == EWeatherState::Transitioning)
	{
		CurrentDensity = GetDensityForWeather(ActiveTransition.FromWeather);
		TargetDensity = GetDensityForWeather(ActiveTransition.ToWeather);
		return FMath::Lerp(CurrentDensity, TargetDensity, ActiveTransition.GetAlpha());
	}
	
	return GetDensityForWeather(CurrentWeather);
}

FLinearColor UPlanetWeatherComponent::GetWeatherFogColor() const
{
	auto GetFogColorForWeather = [](EWeatherType Weather) -> FLinearColor
	{
		switch (Weather)
		{
		case EWeatherType::Clear:
		case EWeatherType::PartlyCloudy:
			return FLinearColor(0.8f, 0.85f, 0.9f);
		case EWeatherType::Cloudy:
		case EWeatherType::Overcast:
			return FLinearColor(0.7f, 0.7f, 0.75f);
		case EWeatherType::Foggy:
			return FLinearColor(0.85f, 0.85f, 0.9f);
		case EWeatherType::LightRain:
		case EWeatherType::Rain:
		case EWeatherType::HeavyRain:
			return FLinearColor(0.6f, 0.6f, 0.65f);
		case EWeatherType::Thunderstorm:
			return FLinearColor(0.4f, 0.4f, 0.5f);
		case EWeatherType::LightSnow:
		case EWeatherType::Snow:
		case EWeatherType::Blizzard:
			return FLinearColor(0.95f, 0.95f, 1.0f);
		case EWeatherType::Sandstorm:
			return FLinearColor(0.9f, 0.8f, 0.6f);
		case EWeatherType::AcidRain:
			return FLinearColor(0.7f, 0.85f, 0.6f);
		case EWeatherType::ToxicFog:
			return FLinearColor(0.6f, 0.9f, 0.5f);
		default:
			return FLinearColor(0.8f, 0.8f, 0.85f);
		}
	};
	
	if (CurrentState == EWeatherState::Transitioning)
	{
		FLinearColor FromColor = GetFogColorForWeather(ActiveTransition.FromWeather);
		FLinearColor ToColor = GetFogColorForWeather(ActiveTransition.ToWeather);
		return FMath::Lerp(FromColor, ToColor, ActiveTransition.GetAlpha());
	}
	
	return GetFogColorForWeather(CurrentWeather);
}

float UPlanetWeatherComponent::GetCloudCoverageOcclusion() const
{
	float Coverage = GetCloudCoverageForWeather(CurrentWeather);
	
	if (CurrentState == EWeatherState::Transitioning)
	{
		float FromCoverage = GetCloudCoverageForWeather(ActiveTransition.FromWeather);
		float ToCoverage = GetCloudCoverageForWeather(ActiveTransition.ToWeather);
		Coverage = FMath::Lerp(FromCoverage, ToCoverage, ActiveTransition.GetAlpha());
	}
	
	// Map coverage to occlusion (0.0 = no occlusion, 1.0 = full occlusion)
	return Coverage * 0.5f; // Max 50% occlusion from clouds
}

float UPlanetWeatherComponent::GetStormDarknessFactor() const
{
	if (IsStormyWeather(CurrentWeather))
	{
		if (CurrentState == EWeatherState::Transitioning)
		{
			bool FromStormy = IsStormyWeather(ActiveTransition.FromWeather);
			bool ToStormy = IsStormyWeather(ActiveTransition.ToWeather);
			
			if (FromStormy && !ToStormy)
			{
				return FMath::Lerp(0.6f, 0.0f, ActiveTransition.GetAlpha());
			}
			else if (!FromStormy && ToStormy)
			{
				return FMath::Lerp(0.0f, 0.6f, ActiveTransition.GetAlpha());
			}
		}
		return 0.6f; // 60% darkening during storms
	}
	
	return 0.0f;
}

float UPlanetWeatherComponent::GetAtmosphericScatteringMultiplier() const
{
	auto GetScatteringForWeather = [](EWeatherType Weather) -> float
	{
		switch (Weather)
		{
		case EWeatherType::Clear:
			return 1.0f;
		case EWeatherType::PartlyCloudy:
			return 0.95f;
		case EWeatherType::Cloudy:
			return 0.85f;
		case EWeatherType::Overcast:
			return 0.7f;
		case EWeatherType::Foggy:
			return 0.5f;
		case EWeatherType::LightRain:
			return 0.8f;
		case EWeatherType::Rain:
			return 0.7f;
		case EWeatherType::HeavyRain:
			return 0.6f;
		case EWeatherType::Thunderstorm:
			return 0.5f;
		case EWeatherType::LightSnow:
			return 0.85f;
		case EWeatherType::Snow:
			return 0.75f;
		case EWeatherType::Blizzard:
			return 0.4f;
		case EWeatherType::Sandstorm:
			return 0.3f;
		case EWeatherType::AcidRain:
			return 0.65f;
		case EWeatherType::ToxicFog:
			return 0.4f;
		default:
			return 1.0f;
		}
	};
	
	if (CurrentState == EWeatherState::Transitioning)
	{
		float FromScattering = GetScatteringForWeather(ActiveTransition.FromWeather);
		float ToScattering = GetScatteringForWeather(ActiveTransition.ToWeather);
		return FMath::Lerp(FromScattering, ToScattering, ActiveTransition.GetAlpha());
	}
	
	return GetScatteringForWeather(CurrentWeather);
}

float UPlanetWeatherComponent::GetSunIntensityMultiplier() const
{
	auto GetSunIntensityForWeather = [](EWeatherType Weather) -> float
	{
		switch (Weather)
		{
		case EWeatherType::Clear:
			return 1.0f;
		case EWeatherType::PartlyCloudy:
			return 0.9f;
		case EWeatherType::Cloudy:
			return 0.7f;
		case EWeatherType::Overcast:
			return 0.5f;
		case EWeatherType::Foggy:
			return 0.4f;
		case EWeatherType::LightRain:
			return 0.6f;
		case EWeatherType::Rain:
			return 0.5f;
		case EWeatherType::HeavyRain:
			return 0.4f;
		case EWeatherType::Thunderstorm:
			return 0.3f;
		case EWeatherType::LightSnow:
			return 0.7f;
		case EWeatherType::Snow:
			return 0.6f;
		case EWeatherType::Blizzard:
			return 0.3f;
		case EWeatherType::Sandstorm:
			return 0.2f;
		case EWeatherType::AcidRain:
			return 0.45f;
		case EWeatherType::ToxicFog:
			return 0.25f;
		default:
			return 1.0f;
		}
	};
	
	if (CurrentState == EWeatherState::Transitioning)
	{
		float FromIntensity = GetSunIntensityForWeather(ActiveTransition.FromWeather);
		float ToIntensity = GetSunIntensityForWeather(ActiveTransition.ToWeather);
		return FMath::Lerp(FromIntensity, ToIntensity, ActiveTransition.GetAlpha());
	}
	
	return GetSunIntensityForWeather(CurrentWeather);
}

float UPlanetWeatherComponent::GetSkyLightIntensityMultiplier() const
{
	auto GetSkyLightIntensityForWeather = [](EWeatherType Weather) -> float
	{
		switch (Weather)
		{
		case EWeatherType::Clear:
			return 1.0f;
		case EWeatherType::PartlyCloudy:
			return 0.95f;
		case EWeatherType::Cloudy:
			return 0.85f;
		case EWeatherType::Overcast:
			return 0.7f;
		case EWeatherType::Foggy:
			return 0.6f;
		case EWeatherType::LightRain:
			return 0.75f;
		case EWeatherType::Rain:
			return 0.65f;
		case EWeatherType::HeavyRain:
			return 0.55f;
		case EWeatherType::Thunderstorm:
			return 0.45f;
		case EWeatherType::LightSnow:
			return 0.8f;
		case EWeatherType::Snow:
			return 0.7f;
		case EWeatherType::Blizzard:
			return 0.5f;
		case EWeatherType::Sandstorm:
			return 0.4f;
		case EWeatherType::AcidRain:
			return 0.6f;
		case EWeatherType::ToxicFog:
			return 0.4f;
		default:
			return 1.0f;
		}
	};
	
	if (CurrentState == EWeatherState::Transitioning)
	{
		float FromIntensity = GetSkyLightIntensityForWeather(ActiveTransition.FromWeather);
		float ToIntensity = GetSkyLightIntensityForWeather(ActiveTransition.ToWeather);
		return FMath::Lerp(FromIntensity, ToIntensity, ActiveTransition.GetAlpha());
	}
	
	return GetSkyLightIntensityForWeather(CurrentWeather);
}

// ===== Initialization =====

void UPlanetWeatherComponent::InitializeWeather(const TArray<FWeatherPreset>& Presets)
{
	WeatherPresets = Presets;
	
	if (bEnableAutomaticWeatherChanges && WeatherPresets.Num() > 0)
	{
		EnterEvaluatingState();
	}
}

void UPlanetWeatherComponent::SetCloudComponent(UPlanetCloudComponent* InCloudComponent)
{
	CloudComponent = InCloudComponent;
}

void UPlanetWeatherComponent::SetAtmosphereComponent(UPlanetAtmosphereComponent* InAtmosphereComponent)
{
	AtmosphereComponent = InAtmosphereComponent;
}

// ===== Effect Updates =====

void UPlanetWeatherComponent::UpdateWeatherEffects(float DeltaTime)
{
	UpdateCloudEffects(DeltaTime);
	UpdateAtmosphereEffects(DeltaTime);
	UpdateFogEffects(DeltaTime);
	UpdatePrecipitationEffects(DeltaTime);
}

void UPlanetWeatherComponent::UpdateCloudEffects(float DeltaTime)
{
	if (!CloudComponent)
	{
		return;
	}
	
	// Update cloud coverage and density based on weather
	float TargetCoverage = GetCloudCoverageForWeather(CurrentWeather);
	float TargetDensity = GetCloudDensityForWeather(CurrentWeather);
	
	if (CurrentState == EWeatherState::Transitioning)
	{
		float FromCoverage = GetCloudCoverageForWeather(ActiveTransition.FromWeather);
		float ToCoverage = GetCloudCoverageForWeather(ActiveTransition.ToWeather);
		TargetCoverage = FMath::Lerp(FromCoverage, ToCoverage, ActiveTransition.GetAlpha());
		
		float FromDensity = GetCloudDensityForWeather(ActiveTransition.FromWeather);
		float ToDensity = GetCloudDensityForWeather(ActiveTransition.ToWeather);
		TargetDensity = FMath::Lerp(FromDensity, ToDensity, ActiveTransition.GetAlpha());
	}
	
	// Apply to cloud component
	// CloudComponent->SetCloudCoverage(TargetCoverage);
	// CloudComponent->SetCloudDensity(TargetDensity);
}

void UPlanetWeatherComponent::UpdateAtmosphereEffects(float DeltaTime)
{
	if (!AtmosphereComponent)
	{
		return;
	}
	
	// Update atmosphere scattering based on weather
	float ScatteringMultiplier = GetAtmosphericScatteringMultiplier();
	
	// Apply to atmosphere component
	// AtmosphereComponent->SetScatteringMultiplier(ScatteringMultiplier);
}

void UPlanetWeatherComponent::UpdateFogEffects(float DeltaTime)
{
	// Fog effects are queried by other systems via GetWeatherFogDensity() and GetWeatherFogColor()
	// This allows flexibility in how fog is implemented (exponential height fog, volumetric fog, etc.)
}

void UPlanetWeatherComponent::UpdatePrecipitationEffects(float DeltaTime)
{
	// Determine which precipitation system should be active
	UNiagaraSystem* TargetSystem = nullptr;
	float TargetIntensity = 0.0f;
	
	auto GetPrecipitationForWeather = [this](EWeatherType Weather, UNiagaraSystem*& OutSystem, float& OutIntensity)
	{
		OutSystem = nullptr;
		OutIntensity = 0.0f;
		
		switch (Weather)
		{
		case EWeatherType::LightRain:
			OutSystem = RainParticleSystem;
			OutIntensity = 0.3f;
			break;
		case EWeatherType::Rain:
			OutSystem = RainParticleSystem;
			OutIntensity = 0.6f;
			break;
		case EWeatherType::HeavyRain:
		case EWeatherType::AcidRain:
			OutSystem = RainParticleSystem;
			OutIntensity = 1.0f;
			break;
		case EWeatherType::Thunderstorm:
			OutSystem = RainParticleSystem;
			OutIntensity = 1.0f;
			break;
		case EWeatherType::LightSnow:
			OutSystem = SnowParticleSystem;
			OutIntensity = 0.3f;
			break;
		case EWeatherType::Snow:
			OutSystem = SnowParticleSystem;
			OutIntensity = 0.6f;
			break;
		case EWeatherType::Blizzard:
			OutSystem = SnowParticleSystem;
			OutIntensity = 1.0f;
			break;
		case EWeatherType::Sandstorm:
			OutSystem = SandstormParticleSystem;
			OutIntensity = 1.0f;
			break;
		case EWeatherType::Foggy:
		case EWeatherType::ToxicFog:
			OutSystem = FogParticleSystem;
			OutIntensity = 0.8f;
			break;
		}
	};
	
	if (CurrentState == EWeatherState::Transitioning)
	{
		UNiagaraSystem* FromSystem = nullptr;
		float FromIntensity = 0.0f;
		GetPrecipitationForWeather(ActiveTransition.FromWeather, FromSystem, FromIntensity);
		
		UNiagaraSystem* ToSystem = nullptr;
		float ToIntensity = 0.0f;
		GetPrecipitationForWeather(ActiveTransition.ToWeather, ToSystem, ToIntensity);
		
		// During transition, use target system with interpolated intensity
		if (ToSystem)
		{
			TargetSystem = ToSystem;
			TargetIntensity = FMath::Lerp(0.0f, ToIntensity, ActiveTransition.GetAlpha());
		}
		else if (FromSystem)
		{
			TargetSystem = FromSystem;
			TargetIntensity = FMath::Lerp(FromIntensity, 0.0f, ActiveTransition.GetAlpha());
		}
	}
	else
	{
		GetPrecipitationForWeather(CurrentWeather, TargetSystem, TargetIntensity);
	}
	
	SetPrecipitationSystem(TargetSystem, TargetIntensity);
}

void UPlanetWeatherComponent::SetPrecipitationSystem(UNiagaraSystem* System, float Intensity)
{
	if (!System || Intensity <= 0.0f)
	{
		StopPrecipitation();
		return;
	}
	
	// Check if we need to spawn a new system
	if (!ActivePrecipitationSystem || !ActivePrecipitationSystem->GetAsset() || 
	    ActivePrecipitationSystem->GetAsset() != System)
	{
		// Stop current system
		StopPrecipitation();
		
		// Spawn new system
		if (AActor* Owner = GetOwner())
		{
			ActivePrecipitationSystem = UNiagaraFunctionLibrary::SpawnSystemAttached(
				System,
				Owner->GetRootComponent(),
				NAME_None,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::KeepRelativeOffset,
				true
			);
		}
	}
	
	// Update intensity
	if (ActivePrecipitationSystem)
	{
		ActivePrecipitationSystem->SetFloatParameter(TEXT("Intensity"), Intensity);
	}
}

void UPlanetWeatherComponent::StopPrecipitation()
{
	if (ActivePrecipitationSystem)
	{
		ActivePrecipitationSystem->DestroyComponent();
		ActivePrecipitationSystem = nullptr;
	}
}

void UPlanetWeatherComponent::CalculateWeatherLighting()
{
	// This is called by other systems that need to update lighting based on weather
	// All lighting calculations are available via the query functions
}

// ===== Farming System Integration =====

float UPlanetWeatherComponent::GetCurrentTemperature() const
{
	// Base temperature varies by weather type
	float BaseTemp = 20.0f; // Default comfortable temperature
	
	switch (CurrentWeather)
	{
		case EWeatherType::Clear:
			BaseTemp = 22.0f;
			break;
		case EWeatherType::PartlyCloudy:
			BaseTemp = 20.0f;
			break;
		case EWeatherType::Overcast:
			BaseTemp = 18.0f;
			break;
		case EWeatherType::LightRain:
		case EWeatherType::HeavyRain:
		case EWeatherType::Thunderstorm:
			BaseTemp = 15.0f;
			break;
		case EWeatherType::LightSnow:
		case EWeatherType::HeavySnow:
			BaseTemp = -5.0f;
			break;
		case EWeatherType::Sandstorm:
			BaseTemp = 35.0f;
			break;
		case EWeatherType::Fog:
			BaseTemp = 12.0f;
			break;
	}
	
	// If transitioning, interpolate between weather types
	if (IsTransitioning())
	{
		float TargetTemp = BaseTemp;
		// Calculate source temperature (simplified)
		float SourceTemp = 20.0f;
		BaseTemp = FMath::Lerp(SourceTemp, TargetTemp, ActiveTransition.GetAlpha());
	}
	
	return BaseTemp;
}

float UPlanetWeatherComponent::GetCurrentHumidity() const
{
	// Base humidity varies by weather type
	float BaseHumidity = 0.5f;
	
	switch (CurrentWeather)
	{
		case EWeatherType::Clear:
			BaseHumidity = 0.3f;
			break;
		case EWeatherType::PartlyCloudy:
			BaseHumidity = 0.5f;
			break;
		case EWeatherType::Overcast:
			BaseHumidity = 0.7f;
			break;
		case EWeatherType::LightRain:
			BaseHumidity = 0.85f;
			break;
		case EWeatherType::HeavyRain:
		case EWeatherType::Thunderstorm:
			BaseHumidity = 0.95f;
			break;
		case EWeatherType::LightSnow:
		case EWeatherType::HeavySnow:
			BaseHumidity = 0.6f;
			break;
		case EWeatherType::Sandstorm:
			BaseHumidity = 0.1f;
			break;
		case EWeatherType::Fog:
			BaseHumidity = 1.0f;
			break;
	}
	
	// If transitioning, interpolate
	if (IsTransitioning())
	{
		float TargetHumidity = BaseHumidity;
		float SourceHumidity = 0.5f;
		BaseHumidity = FMath::Lerp(SourceHumidity, TargetHumidity, ActiveTransition.GetAlpha());
	}
	
	return BaseHumidity;
}

float UPlanetWeatherComponent::GetPrecipitationRate() const
{
	// Precipitation rate in mm per hour
	float PrecipRate = 0.0f;
	
	switch (CurrentWeather)
	{
		case EWeatherType::Clear:
		case EWeatherType::PartlyCloudy:
		case EWeatherType::Overcast:
		case EWeatherType::Sandstorm:
		case EWeatherType::Fog:
			PrecipRate = 0.0f;
			break;
		case EWeatherType::LightRain:
			PrecipRate = 2.5f; // Light rain
			break;
		case EWeatherType::HeavyRain:
			PrecipRate = 10.0f; // Heavy rain
			break;
		case EWeatherType::Thunderstorm:
			PrecipRate = 25.0f; // Very heavy rain
			break;
		case EWeatherType::LightSnow:
			PrecipRate = 1.0f; // Light snow (water equivalent)
			break;
		case EWeatherType::HeavySnow:
			PrecipRate = 5.0f; // Heavy snow (water equivalent)
			break;
	}
	
	// If transitioning, interpolate
	if (IsTransitioning())
	{
		float TargetRate = PrecipRate;
		float SourceRate = 0.0f;
		PrecipRate = FMath::Lerp(SourceRate, TargetRate, ActiveTransition.GetAlpha());
	}
	
	return PrecipRate;
}
