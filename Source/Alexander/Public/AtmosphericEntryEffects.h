// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraShakeBase.h"
#include "TransitionController.h"
#include "AtmosphericEntryEffects.generated.h"

/**
 * Configuration for heat shimmer effects
 */
USTRUCT(BlueprintType)
struct FHeatShimmerConfig
{
	GENERATED_BODY()

	// Intensity of heat distortion (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heat Shimmer")
	float DistortionIntensity = 0.5f;

	// Speed of heat wave animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heat Shimmer")
	float AnimationSpeed = 2.0f;

	// Scale of heat distortion pattern
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heat Shimmer")
	float DistortionScale = 1.0f;

	// Altitude threshold where heat shimmer starts (km)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heat Shimmer")
	float StartAltitude = 80.0f;

	// Altitude where heat shimmer peaks (km)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heat Shimmer")
	float PeakAltitude = 60.0f;

	// Altitude where heat shimmer ends (km)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heat Shimmer")
	float EndAltitude = 40.0f;
};

/**
 * Configuration for plasma effects
 */
USTRUCT(BlueprintType)
struct FPlasmaEffectConfig
{
	GENERATED_BODY()

	// Color of plasma glow
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plasma")
	FLinearColor PlasmaColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);

	// Intensity of plasma glow (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plasma")
	float GlowIntensity = 0.8f;

	// Size of plasma particles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plasma")
	float ParticleSize = 50.0f;

	// Spawn rate of plasma particles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plasma")
	float SpawnRate = 100.0f;

	// Altitude threshold where plasma starts (km)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plasma")
	float StartAltitude = 90.0f;

	// Altitude where plasma peaks (km)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plasma")
	float PeakAltitude = 70.0f;

	// Altitude where plasma ends (km)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Plasma")
	float EndAltitude = 50.0f;
};

/**
 * Configuration for entry sound effects
 */
USTRUCT(BlueprintType)
struct FEntrySoundConfig
{
	GENERATED_BODY()

	// Sound for atmospheric entry wind
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* WindSound = nullptr;

	// Sound for plasma crackling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* PlasmaSound = nullptr;

	// Sound for turbulence buffeting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* TurbulenceSound = nullptr;

	// Volume multiplier for entry sounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	float VolumeMultiplier = 1.0f;

	// Pitch variation range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	FVector2D PitchRange = FVector2D(0.8f, 1.2f);
};

/**
 * Configuration for camera shake during turbulence
 */
USTRUCT(BlueprintType)
struct FTurbulenceCameraShakeConfig
{
	GENERATED_BODY()

	// Camera shake class to use
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
	TSubclassOf<UCameraShakeBase> ShakeClass = nullptr;

	// Base shake intensity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
	float BaseIntensity = 0.5f;

	// Maximum shake intensity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
	float MaxIntensity = 1.5f;

	// Shake frequency (shakes per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
	float ShakeFrequency = 2.0f;
};

/**
 * Manages atmospheric entry visual and audio effects
 * Includes heat shimmer, plasma effects, entry sounds, and camera shake
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UAtmosphericEntryEffects : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtmosphericEntryEffects();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Initialize the atmospheric entry effects system
	 * @param InTransitionController - Reference to the transition controller
	 */
	UFUNCTION(BlueprintCallable, Category = "Atmospheric Entry")
	void Initialize(UTransitionController* InTransitionController);

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	// Heat shimmer configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Heat Shimmer")
	FHeatShimmerConfig HeatShimmerConfig;

	// Plasma effect configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Plasma")
	FPlasmaEffectConfig PlasmaConfig;

	// Entry sound configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Sound")
	FEntrySoundConfig SoundConfig;

	// Camera shake configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects|Camera Shake")
	FTurbulenceCameraShakeConfig CameraShakeConfig;

	// ============================================================================
	// EFFECT COMPONENTS
	// ============================================================================

	// Niagara component for plasma effects
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* PlasmaEffectComponent;

	// ============================================================================
	// EFFECT CONTROL
	// ============================================================================

	/**
	 * Enable or disable atmospheric entry effects
	 */
	UFUNCTION(BlueprintCallable, Category = "Atmospheric Entry")
	void SetEffectsEnabled(bool bEnabled);

	/**
	 * Check if effects are currently active
	 */
	UFUNCTION(BlueprintPure, Category = "Atmospheric Entry")
	bool AreEffectsActive() const { return bEffectsActive; }

	/**
	 * Get current heat shimmer intensity (0-1)
	 */
	UFUNCTION(BlueprintPure, Category = "Atmospheric Entry")
	float GetHeatShimmerIntensity() const { return CurrentHeatShimmerIntensity; }

	/**
	 * Get current plasma effect intensity (0-1)
	 */
	UFUNCTION(BlueprintPure, Category = "Atmospheric Entry")
	float GetPlasmaIntensity() const { return CurrentPlasmaIntensity; }

	/**
	 * Get current entry sound volume (0-1)
	 */
	UFUNCTION(BlueprintPure, Category = "Atmospheric Entry")
	float GetEntrySoundVolume() const { return CurrentSoundVolume; }

	/**
	 * Get current camera shake intensity (0-1)
	 */
	UFUNCTION(BlueprintPure, Category = "Atmospheric Entry")
	float GetCameraShakeIntensity() const { return CurrentShakeIntensity; }

protected:
	// ============================================================================
	// INTERNAL METHODS
	// ============================================================================

	/**
	 * Update heat shimmer effects based on altitude
	 */
	void UpdateHeatShimmer(float DeltaTime);

	/**
	 * Update plasma effects based on altitude
	 */
	void UpdatePlasmaEffects(float DeltaTime);

	/**
	 * Update entry sound effects based on altitude and velocity
	 */
	void UpdateEntrySounds(float DeltaTime);

	/**
	 * Update camera shake based on turbulence
	 */
	void UpdateCameraShake(float DeltaTime);

	/**
	 * Calculate heat shimmer intensity based on altitude
	 */
	float CalculateHeatShimmerIntensity() const;

	/**
	 * Calculate plasma effect intensity based on altitude
	 */
	float CalculatePlasmaIntensity() const;

	/**
	 * Calculate entry sound volume based on altitude and velocity
	 */
	float CalculateEntrySoundVolume() const;

	/**
	 * Calculate camera shake intensity based on turbulence
	 */
	float CalculateCameraShakeIntensity() const;

	/**
	 * Apply heat shimmer post-process effect
	 */
	void ApplyHeatShimmerEffect(float Intensity);

	/**
	 * Spawn or update plasma particle effects
	 */
	void UpdatePlasmaParticles(float Intensity);

	/**
	 * Play or update entry sound effects
	 */
	void PlayEntrySounds(float Volume);

	/**
	 * Trigger camera shake for turbulence
	 */
	void TriggerCameraShake(float Intensity);

private:
	// ============================================================================
	// STATE
	// ============================================================================

	// Reference to transition controller
	UPROPERTY()
	UTransitionController* TransitionController;

	// Whether effects are enabled
	UPROPERTY()
	bool bEffectsEnabled;

	// Whether effects are currently active
	UPROPERTY()
	bool bEffectsActive;

	// Current heat shimmer intensity
	UPROPERTY()
	float CurrentHeatShimmerIntensity;

	// Current plasma intensity
	UPROPERTY()
	float CurrentPlasmaIntensity;

	// Current sound volume
	UPROPERTY()
	float CurrentSoundVolume;

	// Current shake intensity
	UPROPERTY()
	float CurrentShakeIntensity;

	// ============================================================================
	// AUDIO COMPONENTS
	// ============================================================================

	// Audio component for wind sound
	UPROPERTY()
	class UAudioComponent* WindAudioComponent;

	// Audio component for plasma sound
	UPROPERTY()
	class UAudioComponent* PlasmaAudioComponent;

	// Audio component for turbulence sound
	UPROPERTY()
	class UAudioComponent* TurbulenceAudioComponent;

	// ============================================================================
	// TIMING
	// ============================================================================

	// Time since last camera shake
	float TimeSinceLastShake;

	// Time accumulator for heat shimmer animation
	float HeatShimmerTimeAccumulator;

	// Time accumulator for plasma animation
	float PlasmaTimeAccumulator;
};
