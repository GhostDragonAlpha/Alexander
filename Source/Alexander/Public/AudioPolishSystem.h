// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "AudioPolishSystem.generated.h"

/**
 * Biome audio configuration
 */
USTRUCT(BlueprintType)
struct FBiomeAudioConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* AmbientSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* WindSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TArray<USoundBase*> AnimalSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float AmbientVolume = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float WindVolume = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float AnimalSoundFrequency = 10.0f; // Seconds between sounds
};

/**
 * Weather audio configuration
 */
USTRUCT(BlueprintType)
struct FWeatherAudioConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* RainSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* ThunderSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* WindSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* SnowSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* SandstormSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float BaseVolume = 0.6f;
};

/**
 * Farming tool audio configuration
 */
USTRUCT(BlueprintType)
struct FFarmingToolAudioConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* PlantingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* WateringSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* HarvestingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* HoeingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* FertilizingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float ToolVolume = 0.7f;
};

/**
 * System for polishing audio effects
 */
UCLASS()
class ALEXANDER_API UAudioPolishSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Subsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Biome audio
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetBiomeAudio(FName BiomeName, const FBiomeAudioConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void TransitionToBiomeAudio(FName BiomeName, float TransitionDuration = 2.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayAnimalSound(FName BiomeName);

	// Weather audio
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetWeatherAudio(const FWeatherAudioConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayWeatherSound(FName WeatherType, float Intensity = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopWeatherSound(float FadeOutDuration = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayThunderSound(FVector Location);

	// Farming tool audio
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetFarmingToolAudio(const FFarmingToolAudioConfig& Config);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayPlantingSound(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayWateringSound(FVector Location, bool bLoop = true);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopWateringSound();

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayHarvestingSound(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayHoeingSound(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayFertilizingSound(FVector Location);

	// Atmospheric entry audio
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayAtmosphericEntrySound(float Intensity = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void UpdateAtmosphericEntryAudio(float Altitude, float Speed);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopAtmosphericEntrySound(float FadeOutDuration = 2.0f);

	// Master volume controls
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetMasterVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetAmbientVolume(float Volume);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void SetEffectsVolume(float Volume);

	// 3D audio positioning
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void UpdateListenerPosition(FVector Location, FRotator Rotation);

protected:
	void UpdateBiomeAudio(float DeltaTime);
	void UpdateWeatherAudio(float DeltaTime);
	void PlayRandomAnimalSound();

private:
	// Biome audio
	UPROPERTY()
	TMap<FName, FBiomeAudioConfig> BiomeAudioConfigs;

	UPROPERTY()
	UAudioComponent* CurrentBiomeAmbient;

	UPROPERTY()
	UAudioComponent* CurrentBiomeWind;

	UPROPERTY()
	FName CurrentBiome;

	UPROPERTY()
	float AnimalSoundTimer = 0.0f;

	// Weather audio
	UPROPERTY()
	FWeatherAudioConfig WeatherAudioConfig;

	UPROPERTY()
	UAudioComponent* CurrentWeatherSound;

	// Farming tool audio
	UPROPERTY()
	FFarmingToolAudioConfig FarmingToolAudioConfig;

	UPROPERTY()
	UAudioComponent* WateringAudioComponent;

	// Atmospheric entry audio
	UPROPERTY()
	UAudioComponent* AtmosphericEntryAudio;

	// Volume settings
	UPROPERTY()
	float MasterVolume = 1.0f;

	UPROPERTY()
	float AmbientVolume = 1.0f;

	UPROPERTY()
	float EffectsVolume = 1.0f;

	FTimerHandle UpdateTimerHandle;
};
