// Copyright Epic Games, Inc. All Rights Reserved.

#include "AudioPolishSystem.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void UAudioPolishSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Start update timer
	GetWorld()->GetTimerManager().SetTimer(
		UpdateTimerHandle,
		[this]() 
		{ 
			UpdateBiomeAudio(0.1f);
			UpdateWeatherAudio(0.1f);
		},
		0.1f,
		true
	);

	UE_LOG(LogTemp, Log, TEXT("Audio Polish System initialized"));
}

void UAudioPolishSystem::Deinitialize()
{
	// Clean up audio components
	if (CurrentBiomeAmbient)
	{
		CurrentBiomeAmbient->Stop();
		CurrentBiomeAmbient = nullptr;
	}

	if (CurrentBiomeWind)
	{
		CurrentBiomeWind->Stop();
		CurrentBiomeWind = nullptr;
	}

	if (CurrentWeatherSound)
	{
		CurrentWeatherSound->Stop();
		CurrentWeatherSound = nullptr;
	}

	if (WateringAudioComponent)
	{
		WateringAudioComponent->Stop();
		WateringAudioComponent = nullptr;
	}

	if (AtmosphericEntryAudio)
	{
		AtmosphericEntryAudio->Stop();
		AtmosphericEntryAudio = nullptr;
	}

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}

	Super::Deinitialize();
}

void UAudioPolishSystem::SetBiomeAudio(FName BiomeName, const FBiomeAudioConfig& Config)
{
	BiomeAudioConfigs.Add(BiomeName, Config);
	UE_LOG(LogTemp, Log, TEXT("Biome audio configured for: %s"), *BiomeName.ToString());
}

void UAudioPolishSystem::TransitionToBiomeAudio(FName BiomeName, float TransitionDuration)
{
	const FBiomeAudioConfig* Config = BiomeAudioConfigs.Find(BiomeName);
	if (!Config)
	{
		UE_LOG(LogTemp, Warning, TEXT("Biome audio config not found for: %s"), *BiomeName.ToString());
		return;
	}

	// Fade out current biome audio
	if (CurrentBiomeAmbient)
	{
		CurrentBiomeAmbient->FadeOut(TransitionDuration, 0.0f);
	}

	if (CurrentBiomeWind)
	{
		CurrentBiomeWind->FadeOut(TransitionDuration, 0.0f);
	}

	// Create new audio components
	if (Config->AmbientSound)
	{
		CurrentBiomeAmbient = UGameplayStatics::SpawnSound2D(
			GetWorld(),
			Config->AmbientSound,
			Config->AmbientVolume * AmbientVolume * MasterVolume,
			1.0f,
			0.0f,
			nullptr,
			true,
			false
		);

		if (CurrentBiomeAmbient)
		{
			CurrentBiomeAmbient->FadeIn(TransitionDuration, Config->AmbientVolume * AmbientVolume * MasterVolume);
		}
	}

	if (Config->WindSound)
	{
		CurrentBiomeWind = UGameplayStatics::SpawnSound2D(
			GetWorld(),
			Config->WindSound,
			Config->WindVolume * AmbientVolume * MasterVolume,
			1.0f,
			0.0f,
			nullptr,
			true,
			false
		);

		if (CurrentBiomeWind)
		{
			CurrentBiomeWind->FadeIn(TransitionDuration, Config->WindVolume * AmbientVolume * MasterVolume);
		}
	}

	CurrentBiome = BiomeName;
	AnimalSoundTimer = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("Transitioning to biome audio: %s"), *BiomeName.ToString());
}

void UAudioPolishSystem::PlayAnimalSound(FName BiomeName)
{
	const FBiomeAudioConfig* Config = BiomeAudioConfigs.Find(BiomeName);
	if (!Config || Config->AnimalSounds.Num() == 0)
	{
		return;
	}

	// Pick random animal sound
	int32 RandomIndex = FMath::RandRange(0, Config->AnimalSounds.Num() - 1);
	USoundBase* AnimalSound = Config->AnimalSounds[RandomIndex];

	if (AnimalSound)
	{
		// Play at random location around listener
		FVector ListenerLocation = FVector::ZeroVector; // Would get actual listener location
		FVector RandomOffset = FVector(
			FMath::RandRange(-1000.0f, 1000.0f),
			FMath::RandRange(-1000.0f, 1000.0f),
			FMath::RandRange(-200.0f, 200.0f)
		);

		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			AnimalSound,
			ListenerLocation + RandomOffset,
			0.5f * EffectsVolume * MasterVolume
		);
	}
}

void UAudioPolishSystem::SetWeatherAudio(const FWeatherAudioConfig& Config)
{
	WeatherAudioConfig = Config;
	UE_LOG(LogTemp, Log, TEXT("Weather audio configured"));
}

void UAudioPolishSystem::PlayWeatherSound(FName WeatherType, float Intensity)
{
	USoundBase* WeatherSound = nullptr;

	// Select appropriate weather sound
	if (WeatherType == "Rain" || WeatherType == "LightRain")
	{
		WeatherSound = WeatherAudioConfig.RainSound;
	}
	else if (WeatherType == "Snow" || WeatherType == "LightSnow")
	{
		WeatherSound = WeatherAudioConfig.SnowSound;
	}
	else if (WeatherType == "Sandstorm")
	{
		WeatherSound = WeatherAudioConfig.SandstormSound;
	}
	else if (WeatherType == "Wind")
	{
		WeatherSound = WeatherAudioConfig.WindSound;
	}

	if (!WeatherSound)
	{
		return;
	}

	// Stop current weather sound
	if (CurrentWeatherSound)
	{
		CurrentWeatherSound->FadeOut(1.0f, 0.0f);
	}

	// Play new weather sound
	float Volume = WeatherAudioConfig.BaseVolume * Intensity * AmbientVolume * MasterVolume;
	CurrentWeatherSound = UGameplayStatics::SpawnSound2D(
		GetWorld(),
		WeatherSound,
		Volume,
		1.0f,
		0.0f,
		nullptr,
		true,
		false
	);

	if (CurrentWeatherSound)
	{
		CurrentWeatherSound->FadeIn(2.0f, Volume);
	}

	UE_LOG(LogTemp, Log, TEXT("Playing weather sound: %s (Intensity: %.2f)"), *WeatherType.ToString(), Intensity);
}

void UAudioPolishSystem::StopWeatherSound(float FadeOutDuration)
{
	if (CurrentWeatherSound)
	{
		CurrentWeatherSound->FadeOut(FadeOutDuration, 0.0f);
		CurrentWeatherSound = nullptr;
	}
}

void UAudioPolishSystem::PlayThunderSound(FVector Location)
{
	if (WeatherAudioConfig.ThunderSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			WeatherAudioConfig.ThunderSound,
			Location,
			1.0f * EffectsVolume * MasterVolume,
			1.0f,
			0.0f,
			nullptr,
			nullptr
		);

		UE_LOG(LogTemp, Log, TEXT("Thunder sound played at: %s"), *Location.ToString());
	}
}

void UAudioPolishSystem::SetFarmingToolAudio(const FFarmingToolAudioConfig& Config)
{
	FarmingToolAudioConfig = Config;
	UE_LOG(LogTemp, Log, TEXT("Farming tool audio configured"));
}

void UAudioPolishSystem::PlayPlantingSound(FVector Location)
{
	if (FarmingToolAudioConfig.PlantingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			FarmingToolAudioConfig.PlantingSound,
			Location,
			FarmingToolAudioConfig.ToolVolume * EffectsVolume * MasterVolume
		);
	}
}

void UAudioPolishSystem::PlayWateringSound(FVector Location, bool bLoop)
{
	if (!FarmingToolAudioConfig.WateringSound)
	{
		return;
	}

	// Stop existing watering sound
	if (WateringAudioComponent)
	{
		WateringAudioComponent->Stop();
	}

	// Create new watering audio component
	WateringAudioComponent = UGameplayStatics::SpawnSoundAtLocation(
		GetWorld(),
		FarmingToolAudioConfig.WateringSound,
		Location,
		FRotator::ZeroRotator,
		FarmingToolAudioConfig.ToolVolume * EffectsVolume * MasterVolume,
		1.0f,
		0.0f,
		nullptr,
		nullptr,
		true
	);

	if (WateringAudioComponent && bLoop)
	{
		WateringAudioComponent->SetUISound(false);
	}
}

void UAudioPolishSystem::StopWateringSound()
{
	if (WateringAudioComponent)
	{
		WateringAudioComponent->FadeOut(0.5f, 0.0f);
		WateringAudioComponent = nullptr;
	}
}

void UAudioPolishSystem::PlayHarvestingSound(FVector Location)
{
	if (FarmingToolAudioConfig.HarvestingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			FarmingToolAudioConfig.HarvestingSound,
			Location,
			FarmingToolAudioConfig.ToolVolume * EffectsVolume * MasterVolume
		);
	}
}

void UAudioPolishSystem::PlayHoeingSound(FVector Location)
{
	if (FarmingToolAudioConfig.HoeingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			FarmingToolAudioConfig.HoeingSound,
			Location,
			FarmingToolAudioConfig.ToolVolume * EffectsVolume * MasterVolume
		);
	}
}

void UAudioPolishSystem::PlayFertilizingSound(FVector Location)
{
	if (FarmingToolAudioConfig.FertilizingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			FarmingToolAudioConfig.FertilizingSound,
			Location,
			FarmingToolAudioConfig.ToolVolume * EffectsVolume * MasterVolume
		);
	}
}

void UAudioPolishSystem::PlayAtmosphericEntrySound(float Intensity)
{
	// Load atmospheric entry sound (would be actual asset in production)
	// USoundBase* EntrySound = LoadObject<USoundBase>(...);

	if (AtmosphericEntryAudio)
	{
		AtmosphericEntryAudio->Stop();
	}

	// Create atmospheric entry audio
	// AtmosphericEntryAudio = UGameplayStatics::SpawnSound2D(
	//     GetWorld(),
	//     EntrySound,
	//     Intensity * EffectsVolume * MasterVolume,
	//     1.0f,
	//     0.0f,
	//     nullptr,
	//     true,
	//     false
	// );

	UE_LOG(LogTemp, Log, TEXT("Atmospheric entry sound started (Intensity: %.2f)"), Intensity);
}

void UAudioPolishSystem::UpdateAtmosphericEntryAudio(float Altitude, float Speed)
{
	if (!AtmosphericEntryAudio)
	{
		return;
	}

	// Calculate intensity based on altitude and speed
	// Higher speed and lower altitude = more intense
	float AltitudeFactor = FMath::Clamp(1.0f - (Altitude / 100000.0f), 0.0f, 1.0f);
	float SpeedFactor = FMath::Clamp(Speed / 1000.0f, 0.0f, 1.0f);
	float Intensity = (AltitudeFactor + SpeedFactor) * 0.5f;

	// Update volume
	float Volume = Intensity * EffectsVolume * MasterVolume;
	AtmosphericEntryAudio->SetVolumeMultiplier(Volume);

	// Update pitch based on speed
	float Pitch = 0.8f + (SpeedFactor * 0.4f); // 0.8 to 1.2
	AtmosphericEntryAudio->SetPitchMultiplier(Pitch);

	UE_LOG(LogTemp, Verbose, TEXT("Atmospheric entry audio updated: Altitude=%.1f, Speed=%.1f, Intensity=%.2f"), 
		Altitude, Speed, Intensity);
}

void UAudioPolishSystem::StopAtmosphericEntrySound(float FadeOutDuration)
{
	if (AtmosphericEntryAudio)
	{
		AtmosphericEntryAudio->FadeOut(FadeOutDuration, 0.0f);
		AtmosphericEntryAudio = nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("Atmospheric entry sound stopped"));
}

void UAudioPolishSystem::SetMasterVolume(float Volume)
{
	MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);

	// Update all active audio components
	if (CurrentBiomeAmbient)
	{
		CurrentBiomeAmbient->SetVolumeMultiplier(MasterVolume * AmbientVolume);
	}

	if (CurrentBiomeWind)
	{
		CurrentBiomeWind->SetVolumeMultiplier(MasterVolume * AmbientVolume);
	}

	if (CurrentWeatherSound)
	{
		CurrentWeatherSound->SetVolumeMultiplier(MasterVolume * AmbientVolume);
	}

	UE_LOG(LogTemp, Log, TEXT("Master volume set to: %.2f"), Volume);
}

void UAudioPolishSystem::SetAmbientVolume(float Volume)
{
	AmbientVolume = FMath::Clamp(Volume, 0.0f, 1.0f);

	// Update ambient audio components
	if (CurrentBiomeAmbient)
	{
		CurrentBiomeAmbient->SetVolumeMultiplier(MasterVolume * AmbientVolume);
	}

	if (CurrentBiomeWind)
	{
		CurrentBiomeWind->SetVolumeMultiplier(MasterVolume * AmbientVolume);
	}

	if (CurrentWeatherSound)
	{
		CurrentWeatherSound->SetVolumeMultiplier(MasterVolume * AmbientVolume);
	}

	UE_LOG(LogTemp, Log, TEXT("Ambient volume set to: %.2f"), Volume);
}

void UAudioPolishSystem::SetEffectsVolume(float Volume)
{
	EffectsVolume = FMath::Clamp(Volume, 0.0f, 1.0f);

	// Update effects audio components
	if (WateringAudioComponent)
	{
		WateringAudioComponent->SetVolumeMultiplier(MasterVolume * EffectsVolume);
	}

	if (AtmosphericEntryAudio)
	{
		AtmosphericEntryAudio->SetVolumeMultiplier(MasterVolume * EffectsVolume);
	}

	UE_LOG(LogTemp, Log, TEXT("Effects volume set to: %.2f"), Volume);
}

void UAudioPolishSystem::UpdateListenerPosition(FVector Location, FRotator Rotation)
{
	// Update audio listener position
	// This would be implemented with actual audio engine calls
	UE_LOG(LogTemp, Verbose, TEXT("Listener position updated: %s"), *Location.ToString());
}

void UAudioPolishSystem::UpdateBiomeAudio(float DeltaTime)
{
	// Update animal sound timer
	const FBiomeAudioConfig* Config = BiomeAudioConfigs.Find(CurrentBiome);
	if (Config && Config->AnimalSounds.Num() > 0)
	{
		AnimalSoundTimer += DeltaTime;

		if (AnimalSoundTimer >= Config->AnimalSoundFrequency)
		{
			PlayRandomAnimalSound();
			AnimalSoundTimer = 0.0f;
		}
	}
}

void UAudioPolishSystem::UpdateWeatherAudio(float DeltaTime)
{
	// Update weather audio based on conditions
	// This would be implemented with actual weather system integration
}

void UAudioPolishSystem::PlayRandomAnimalSound()
{
	const FBiomeAudioConfig* Config = BiomeAudioConfigs.Find(CurrentBiome);
	if (!Config || Config->AnimalSounds.Num() == 0)
	{
		return;
	}

	// Random chance to play sound (50%)
	if (FMath::RandRange(0.0f, 1.0f) > 0.5f)
	{
		PlayAnimalSound(CurrentBiome);
	}
}
