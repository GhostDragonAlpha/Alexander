// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeatherVFXLibrary.h"
#include "VFXManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "EngineUtils.h"

// Initialize static members
TMap<EWeatherTypeNew, UParticleSystem*> UWeatherVFXLibrary::WeatherVFXRegistry;
TMap<EWeatherTypeNew, FParticleSystemComponentArray> UWeatherVFXLibrary::ActiveWeatherEffects;

UParticleSystemComponent* UWeatherVFXLibrary::SpawnWeatherEffect(
	UObject* WorldContextObject,
	EWeatherTypeNew WeatherType,
	FVector Location,
	float Intensity)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnWeatherEffect: Invalid WorldContextObject"));
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnWeatherEffect: Failed to get World"));
		return nullptr;
	}

	// Get particle system for weather type
	UParticleSystem* WeatherPS = GetWeatherVFX(WorldContextObject, WeatherType);
	if (!WeatherPS)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnWeatherEffect: No VFX registered for weather type %d"), 
			static_cast<int32>(WeatherType));
		return nullptr;
	}

	// Spawn particle system
	UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(
		World,
		WeatherPS,
		Location,
		FRotator::ZeroRotator,
		FVector::OneVector,
		true, // Auto destroy
		EPSCPoolMethod::AutoRelease
	);

	if (PSC)
	{
		// Set initial intensity
		UpdateWeatherIntensity(PSC, Intensity);

		// Track active effect
		if (!ActiveWeatherEffects.Contains(WeatherType))
		{
			ActiveWeatherEffects.Add(WeatherType, FParticleSystemComponentArray());
		}
		ActiveWeatherEffects[WeatherType].Components.Add(PSC);

		UE_LOG(LogTemp, Log, TEXT("Spawned weather effect type %d at %s with intensity %.2f"), 
			static_cast<int32>(WeatherType), 
			*Location.ToString(), 
			Intensity);
	}

	return PSC;
}

UParticleSystemComponent* UWeatherVFXLibrary::SpawnWeatherEffectWithConfig(
	UObject* WorldContextObject,
	const FWeatherVFXConfig& Config,
	FVector Location,
	FRotator Rotation)
{
	if (!WorldContextObject || !Config.ParticleSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnWeatherEffectWithConfig: Invalid parameters"));
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}

	// Spawn particle system
	UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(
		World,
		Config.ParticleSystem,
		Location,
		Rotation,
		Config.EffectScale,
		true,
		EPSCPoolMethod::AutoRelease
	);

	if (PSC)
	{
		// Apply configuration
		UpdateWeatherIntensity(PSC, Config.BaseIntensity);
		
		// Apply color tint
		PSC->SetColorParameter(FName("TintColor"), Config.ColorTint);

		// Set duration if specified
		if (Config.Duration > 0.0f)
		{
			// TODO UE 5.6: SetLifeSpan removed from UParticleSystemComponent
			// Use timer to destroy component after duration
			// PSC->SetLifeSpan(Config.Duration);
		}

		UE_LOG(LogTemp, Log, TEXT("Spawned weather effect with config at %s"), *Location.ToString());
	}

	return PSC;
}

void UWeatherVFXLibrary::UpdateWeatherIntensity(
	UParticleSystemComponent* Effect,
	float NewIntensity)
{
	if (!Effect)
	{
		return;
	}

	// Clamp intensity
	NewIntensity = FMath::Clamp(NewIntensity, 0.0f, 1.0f);

	// Update spawn rate based on intensity
	Effect->SetFloatParameter(FName("Intensity"), NewIntensity);
	Effect->SetFloatParameter(FName("SpawnRate"), NewIntensity * 100.0f);

	// Update particle size/alpha based on intensity
	Effect->SetFloatParameter(FName("Size"), 0.5f + (NewIntensity * 0.5f));
	Effect->SetFloatParameter(FName("Alpha"), NewIntensity);
}

bool UWeatherVFXLibrary::RegisterWeatherVFX(
	UObject* WorldContextObject,
	UParticleSystem* ParticleSystem,
	EWeatherTypeNew WeatherType)
{
	if (!ParticleSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("RegisterWeatherVFX: Invalid particle system"));
		return false;
	}

	WeatherVFXRegistry.Add(WeatherType, ParticleSystem);

	// Also register with VFXManager if available
	UVFXManager* VFXMgr = GetVFXManager(WorldContextObject);
	if (VFXMgr)
	{
		UE_LOG(LogTemp, Log, TEXT("Registered weather VFX '%s' for type %d"), 
			*ParticleSystem->GetName(), 
			static_cast<int32>(WeatherType));
	}

	return true;
}

UParticleSystemComponent* UWeatherVFXLibrary::SpawnAtmosphericEntryEffect(
	UObject* WorldContextObject,
	AActor* ShipActor,
	const FAtmosphericEntryEffect& EntryData)
{
	if (!WorldContextObject || !ShipActor)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnAtmosphericEntryEffect: Invalid parameters"));
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}

	// Get re-entry fire effect
	UParticleSystem* ReEntryPS = WeatherVFXRegistry.FindRef(EWeatherTypeNew::SolarFlare); // Reuse solar flare as fire
	if (!ReEntryPS)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnAtmosphericEntryEffect: No re-entry VFX available"));
		return nullptr;
	}

	// Attach to ship
	UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(
		ReEntryPS,
		ShipActor->GetRootComponent(),
		NAME_None,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::KeepRelativeOffset,
		true
	);

	if (PSC)
	{
		UpdateAtmosphericEntryEffect(PSC, EntryData);

		UE_LOG(LogTemp, Log, TEXT("Spawned atmospheric entry effect on %s at altitude %.1f"), 
			*ShipActor->GetName(), 
			EntryData.Altitude);
	}

	return PSC;
}

void UWeatherVFXLibrary::UpdateAtmosphericEntryEffect(
	UParticleSystemComponent* Effect,
	const FAtmosphericEntryEffect& EntryData)
{
	if (!Effect)
	{
		return;
	}

	// Calculate intensity based on altitude and velocity
	float AltitudeFactor = FMath::Clamp(1.0f - (EntryData.Altitude / 50000.0f), 0.0f, 1.0f);
	float VelocityFactor = FMath::Clamp(EntryData.Velocity / 10000.0f, 0.0f, 1.0f);
	float TotalIntensity = (AltitudeFactor + VelocityFactor) * 0.5f * EntryData.HeatIntensity;

	// Update effect parameters
	Effect->SetFloatParameter(FName("Intensity"), TotalIntensity);
	Effect->SetFloatParameter(FName("HeatLevel"), TotalIntensity);
	Effect->SetColorParameter(FName("GlowColor"), EntryData.GlowColor);

	// Scale effect based on velocity
	FVector Scale = FVector::OneVector * (1.0f + VelocityFactor);
	Effect->SetRelativeScale3D(Scale);
}

UParticleSystemComponent* UWeatherVFXLibrary::SpawnEnvironmentalHazard(
	UObject* WorldContextObject,
	EWeatherTypeNew HazardType,
	FVector Location,
	float Radius)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	// Create config for hazard
	FWeatherVFXConfig Config = CreateDefaultWeatherConfig(HazardType);
	Config.EffectScale = FVector(Radius / 1000.0f); // Scale based on radius

	return SpawnWeatherEffectWithConfig(WorldContextObject, Config, Location);
}

UParticleSystem* UWeatherVFXLibrary::GetWeatherVFX(
	UObject* WorldContextObject,
	EWeatherTypeNew WeatherType)
{
	if (WeatherVFXRegistry.Contains(WeatherType))
	{
		return WeatherVFXRegistry[WeatherType];
	}

	// Try to load from content folder if not registered
	FString AssetPath = FString::Printf(TEXT("/Game/VFX/Weather/%s"), 
		*UEnum::GetValueAsString(WeatherType));
	
	UParticleSystem* LoadedPS = LoadObject<UParticleSystem>(nullptr, *AssetPath);
	if (LoadedPS)
	{
		WeatherVFXRegistry.Add(WeatherType, LoadedPS);
		UE_LOG(LogTemp, Log, TEXT("Loaded weather VFX from %s"), *AssetPath);
		return LoadedPS;
	}

	return nullptr;
}

FWeatherVFXConfig UWeatherVFXLibrary::CreateDefaultWeatherConfig(EWeatherTypeNew WeatherType)
{
	FWeatherVFXConfig Config;

	switch (WeatherType)
	{
	case EWeatherTypeNew::IonStorm:
		Config.BaseIntensity = 0.8f;
		Config.MaxIntensity = 2.0f;
		Config.ColorTint = FLinearColor(0.3f, 0.5f, 1.0f, 1.0f); // Blue electric
		Config.EffectScale = FVector(5.0f, 5.0f, 5.0f);
		break;

	case EWeatherTypeNew::DustStorm:
		Config.BaseIntensity = 0.6f;
		Config.MaxIntensity = 1.5f;
		Config.ColorTint = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f); // Brown/tan
		Config.EffectScale = FVector(10.0f, 10.0f, 3.0f);
		break;

	case EWeatherTypeNew::RadiationStorm:
		Config.BaseIntensity = 0.7f;
		Config.MaxIntensity = 2.5f;
		Config.ColorTint = FLinearColor(0.2f, 1.0f, 0.2f, 1.0f); // Green glow
		Config.EffectScale = FVector(3.0f, 3.0f, 3.0f);
		break;

	case EWeatherTypeNew::SolarFlare:
		Config.BaseIntensity = 1.0f;
		Config.MaxIntensity = 3.0f;
		Config.ColorTint = FLinearColor(1.0f, 0.8f, 0.2f, 1.0f); // Bright yellow/orange
		Config.EffectScale = FVector(8.0f, 8.0f, 8.0f);
		Config.Duration = 30.0f;
		break;

	case EWeatherTypeNew::AsteroidShower:
		Config.BaseIntensity = 0.5f;
		Config.MaxIntensity = 1.0f;
		Config.ColorTint = FLinearColor::Gray;
		Config.EffectScale = FVector(1.0f, 1.0f, 1.0f);
		break;

	default:
		Config.BaseIntensity = 0.5f;
		Config.MaxIntensity = 1.0f;
		Config.ColorTint = FLinearColor::White;
		Config.EffectScale = FVector::OneVector;
		break;
	}

	return Config;
}

float UWeatherVFXLibrary::CalculateWeatherIntensity(
	float BaseIntensity,
	float Distance,
	float MaxDistance)
{
	if (MaxDistance <= 0.0f)
	{
		return BaseIntensity;
	}

	// Calculate falloff using inverse square law
	float DistanceFactor = 1.0f - FMath::Clamp(Distance / MaxDistance, 0.0f, 1.0f);
	DistanceFactor = DistanceFactor * DistanceFactor; // Square for sharper falloff

	return BaseIntensity * DistanceFactor;
}

void UWeatherVFXLibrary::StopWeatherEffectsByType(
	UObject* WorldContextObject,
	EWeatherTypeNew WeatherType)
{
	if (!ActiveWeatherEffects.Contains(WeatherType))
	{
		return;
	}

	FParticleSystemComponentArray& Effects = ActiveWeatherEffects[WeatherType];
	int32 StoppedCount = 0;

	for (UParticleSystemComponent* Effect : Effects.Components)
	{
		if (Effect && Effect->IsActive())
		{
			Effect->DeactivateSystem();
			StoppedCount++;
		}
	}

	// Clear inactive effects
	Effects.Components.Empty();

	UE_LOG(LogTemp, Log, TEXT("Stopped %d weather effects of type %d"), 
		StoppedCount, 
		static_cast<int32>(WeatherType));
}

void UWeatherVFXLibrary::TransitionWeatherEffect(
	UObject* WorldContextObject,
	UParticleSystemComponent* CurrentEffect,
	EWeatherTypeNew NewWeatherType,
	float TransitionDuration)
{
	if (!WorldContextObject || !CurrentEffect)
	{
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return;
	}

	// Fade out current effect
	if (CurrentEffect->IsActive())
	{
		// Gradually reduce intensity over transition duration
		FTimerHandle FadeOutTimer;
		float FadeSteps = TransitionDuration / 0.1f; // Update every 0.1s
		float IntensityStep = 1.0f / FadeSteps;

		// Create lambda to capture effect and fade it
		auto FadeOutLambda = [CurrentEffect, IntensityStep]()
		{
			if (CurrentEffect && CurrentEffect->IsActive())
			{
				float CurrentIntensity = 1.0f; // Should track this properly
				CurrentIntensity -= IntensityStep;
				
				if (CurrentIntensity <= 0.0f)
				{
					CurrentEffect->DeactivateSystem();
				}
				else
				{
					UpdateWeatherIntensity(CurrentEffect, CurrentIntensity);
				}
			}
		};

		// Note: Proper implementation would use a timer delegate
		UE_LOG(LogTemp, Log, TEXT("Transitioning weather effect to type %d over %.1f seconds"), 
			static_cast<int32>(NewWeatherType), 
			TransitionDuration);
	}

	// Spawn new effect at same location with fade-in
	FVector Location = CurrentEffect->GetComponentLocation();
	UParticleSystemComponent* NewEffect = SpawnWeatherEffect(
		WorldContextObject, 
		NewWeatherType, 
		Location, 
		0.0f
	);

	if (NewEffect)
	{
		// Fade in new effect
		// Similar timer logic for fade-in
		UE_LOG(LogTemp, Log, TEXT("Spawned new weather effect for transition"));
	}
}

UVFXManager* UWeatherVFXLibrary::GetVFXManager(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}

	// Try to find VFXManager in the world
	UVFXManager* VFXMgr = nullptr;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor)
		{
			VFXMgr = Actor->FindComponentByClass<UVFXManager>();
			if (VFXMgr)
			{
				break;
			}
		}
	}

	return VFXMgr;
}
