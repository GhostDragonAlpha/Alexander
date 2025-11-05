// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlanetConfiguration.h"
#include "PlanetWeatherComponent.generated.h"

/**
 * Weather state machine states
 */
UENUM(BlueprintType)
enum class EWeatherState : uint8
{
	Stable UMETA(DisplayName = "Stable"),           // Weather is stable, no transition
	Transitioning UMETA(DisplayName = "Transitioning"), // Transitioning between weather types
	Evaluating UMETA(DisplayName = "Evaluating")    // Evaluating next weather change
};

/**
 * Weather transition data
 */
USTRUCT(BlueprintType)
struct FWeatherTransition
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transition")
	EWeatherType FromWeather = EWeatherType::Clear;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transition")
	EWeatherType ToWeather = EWeatherType::Clear;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transition")
	float Duration = 60.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transition")
	float ElapsedTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transition")
	float Progress = 0.0f; // 0-1

	// Check if transition is complete
	bool IsComplete() const { return Progress >= 1.0f; }

	// Get interpolation alpha
	float GetAlpha() const { return FMath::Clamp(Progress, 0.0f, 1.0f); }
};

/**
 * Component for managing dynamic weather conditions with state machine
 */
UCLASS(ClassGroup=(Planet), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UPlanetWeatherComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlanetWeatherComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ===== State Machine =====
	
	// Current state of the weather system
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State Machine")
	EWeatherState CurrentState = EWeatherState::Stable;

	// Current active weather type
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State Machine")
	EWeatherType CurrentWeather = EWeatherType::Clear;

	// Active transition data (only valid when CurrentState == Transitioning)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State Machine")
	FWeatherTransition ActiveTransition;

	// ===== Weather Configuration =====
	
	// Available weather presets with probabilities
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Configuration")
	TArray<FWeatherPreset> WeatherPresets;

	// Time between weather evaluations (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Configuration")
	float WeatherChangeInterval = 300.0f;

	// Minimum duration for stable weather before allowing change (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Configuration")
	float MinimumStableDuration = 120.0f;

	// Default transition duration when not specified (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Configuration")
	float DefaultTransitionDuration = 60.0f;

	// Enable automatic weather changes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Configuration")
	bool bEnableAutomaticWeatherChanges = true;

	// ===== State Machine Control =====
	
	// Manually transition to new weather type
	UFUNCTION(BlueprintCallable, Category = "Weather Control")
	void TransitionToWeather(EWeatherType NewWeather, float Duration = -1.0f);

	// Force immediate weather change without transition
	UFUNCTION(BlueprintCallable, Category = "Weather Control")
	void SetWeatherImmediate(EWeatherType NewWeather);

	// Get current state machine state
	UFUNCTION(BlueprintPure, Category = "Weather Control")
	EWeatherState GetCurrentState() const { return CurrentState; }

	// Check if currently transitioning
	UFUNCTION(BlueprintPure, Category = "Weather Control")
	bool IsTransitioning() const { return CurrentState == EWeatherState::Transitioning; }

	// Get transition progress (0-1, only valid during transition)
	UFUNCTION(BlueprintPure, Category = "Weather Control")
	float GetTransitionProgress() const { return ActiveTransition.Progress; }

	// ===== Weather Queries =====
	
	// Get current weather type
	UFUNCTION(BlueprintPure, Category = "Weather Queries")
	EWeatherType GetCurrentWeather() const { return CurrentWeather; }

	// Get weather lighting multiplier
	UFUNCTION(BlueprintPure, Category = "Weather Queries")
	FLinearColor GetWeatherLightingMultiplier() const;

	// Get wetness amount for material system (0-1)
	UFUNCTION(BlueprintPure, Category = "Weather Queries")
	float GetWetnessAmount() const;

	// Get cloud coverage for weather type
	UFUNCTION(BlueprintPure, Category = "Weather Queries")
	float GetCloudCoverageForWeather(EWeatherType WeatherType) const;

	// Get cloud density for weather type
	UFUNCTION(BlueprintPure, Category = "Weather Queries")
	float GetCloudDensityForWeather(EWeatherType WeatherType) const;

	// Check if weather type is stormy
	UFUNCTION(BlueprintPure, Category = "Weather Queries")
	bool IsStormyWeather(EWeatherType WeatherType) const;

	// Get interpolated weather value between current and target
	UFUNCTION(BlueprintPure, Category = "Weather Queries")
	float GetInterpolatedWeatherValue(float CurrentValue, float TargetValue) const;

	// ===== Weather Lighting (Task 8.3) =====
	
	// Get fog density multiplier based on weather
	UFUNCTION(BlueprintPure, Category = "Weather Lighting")
	float GetWeatherFogDensity() const;

	// Get fog color tint based on weather
	UFUNCTION(BlueprintPure, Category = "Weather Lighting")
	FLinearColor GetWeatherFogColor() const;

	// Get ambient light occlusion from cloud coverage
	UFUNCTION(BlueprintPure, Category = "Weather Lighting")
	float GetCloudCoverageOcclusion() const;

	// Get storm darkness factor (0-1, 0=no darkening, 1=very dark)
	UFUNCTION(BlueprintPure, Category = "Weather Lighting")
	float GetStormDarknessFactor() const;

	// Get atmospheric scattering multiplier for weather
	UFUNCTION(BlueprintPure, Category = "Weather Lighting")
	float GetAtmosphericScatteringMultiplier() const;

	// Get sun intensity multiplier based on weather
	UFUNCTION(BlueprintPure, Category = "Weather Lighting")
	float GetSunIntensityMultiplier() const;

	// Get sky light intensity multiplier based on weather
	UFUNCTION(BlueprintPure, Category = "Weather Lighting")
	float GetSkyLightIntensityMultiplier() const;

	// ===== Farming System Integration =====
	
	// Get current temperature in Celsius
	UFUNCTION(BlueprintPure, Category = "Weather Queries")
	float GetCurrentTemperature() const;

	// Get current humidity (0-1)
	UFUNCTION(BlueprintPure, Category = "Weather Queries")
	float GetCurrentHumidity() const;

	// Get precipitation rate in mm per hour
	UFUNCTION(BlueprintPure, Category = "Weather Queries")
	float GetPrecipitationRate() const;

	// ===== Initialization =====
	
	// Initialize weather system with presets
	UFUNCTION(BlueprintCallable, Category = "Weather Setup")
	void InitializeWeather(const TArray<FWeatherPreset>& Presets);

	// Set cloud component reference for weather-driven cloud changes
	UFUNCTION(BlueprintCallable, Category = "Weather Setup")
	void SetCloudComponent(class UPlanetCloudComponent* InCloudComponent);

	// Set atmosphere component reference for weather-driven atmosphere changes
	UFUNCTION(BlueprintCallable, Category = "Weather Setup")
	void SetAtmosphereComponent(class UPlanetAtmosphereComponent* InAtmosphereComponent);

	// ===== Precipitation Particle Systems =====

	// Rain particle system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precipitation")
	class UNiagaraSystem* RainParticleSystem;

	// Snow particle system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precipitation")
	class UNiagaraSystem* SnowParticleSystem;

	// Sandstorm particle system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precipitation")
	class UNiagaraSystem* SandstormParticleSystem;

	// Fog particle system
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Precipitation")
	class UNiagaraSystem* FogParticleSystem;

private:
	// ===== State Machine Implementation =====
	
	// Update state machine
	void UpdateStateMachine(float DeltaTime);

	// State handlers
	void HandleStableState(float DeltaTime);
	void HandleTransitioningState(float DeltaTime);
	void HandleEvaluatingState(float DeltaTime);

	// State transitions
	void EnterStableState();
	void EnterTransitioningState(EWeatherType ToWeather, float Duration);
	void EnterEvaluatingState();

	// ===== Weather Selection =====
	
	// Select next weather based on presets and probabilities
	EWeatherType SelectNextWeather();

	// Check if weather transition is valid
	bool IsValidWeatherTransition(EWeatherType From, EWeatherType To) const;

	// ===== Effect Updates =====
	
	// Update all weather effects
	void UpdateWeatherEffects(float DeltaTime);

	// Update cloud effects based on weather
	void UpdateCloudEffects(float DeltaTime);

	// ===== Internal State =====
	
	// Time spent in current stable state
	float TimeInStableState = 0.0f;

	// Time since last weather evaluation
	float TimeSinceLastEvaluation = 0.0f;

	// Reference to cloud component for weather-driven changes
	UPROPERTY()
	class UPlanetCloudComponent* CloudComponent;

	// Reference to atmosphere component for weather-driven changes
	UPROPERTY()
	class UPlanetAtmosphereComponent* AtmosphereComponent;

	// ===== Precipitation Particle Systems =====
	
	// Active precipitation particle system
	UPROPERTY()
	class UNiagaraComponent* ActivePrecipitationSystem;

	// Update precipitation effects based on weather
	void UpdatePrecipitationEffects(float DeltaTime);

	// Spawn or update precipitation system
	void SetPrecipitationSystem(class UNiagaraSystem* System, float Intensity);

	// Stop precipitation effects
	void StopPrecipitation();

	// ===== Weather Lighting Effects (Task 8.3) =====
	
	// Update atmosphere effects based on weather
	void UpdateAtmosphereEffects(float DeltaTime);

	// Update fog effects based on weather
	void UpdateFogEffects(float DeltaTime);

	// Calculate lighting parameters for current weather
	void CalculateWeatherLighting();
};
