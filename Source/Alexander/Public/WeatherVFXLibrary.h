// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "WeatherVFXLibrary.generated.h"

/**
 * Weather type enumeration matching AdvancedWeatherSystem
 */
UENUM(BlueprintType)
enum class EWeatherTypeNew : uint8
{
	Clear UMETA(DisplayName = "Clear"),
	IonStorm UMETA(DisplayName = "Ion Storm"),
	DustStorm UMETA(DisplayName = "Dust Storm"),
	RadiationStorm UMETA(DisplayName = "Radiation Storm"),
	SolarFlare UMETA(DisplayName = "Solar Flare"),
	AsteroidShower UMETA(DisplayName = "Asteroid Shower"),
	NebulaCloud UMETA(DisplayName = "Nebula Cloud"),
	SpaceDebris UMETA(DisplayName = "Space Debris")
};

/**
 * Weather VFX configuration
 */
USTRUCT(BlueprintType)
struct FWeatherVFXConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	UParticleSystem* ParticleSystem = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	float BaseIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	float MaxIntensity = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVector EffectScale = FVector(1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	float Duration = -1.0f; // -1 = infinite

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	bool bAttachToLocation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FLinearColor ColorTint = FLinearColor::White;
};

/**
 * Atmospheric entry effect data
 */
USTRUCT(BlueprintType)
struct FAtmosphericEntryEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
	float Altitude = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
	float Velocity = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
	float HeatIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
	bool bSonicBoom = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
	FLinearColor GlowColor = FLinearColor(1.0f, 0.5f, 0.1f, 1.0f);
};

/**
 * Wrapper struct for TArray<UParticleSystemComponent*> to satisfy UE 5.6 TMap value restrictions
 */
USTRUCT()
struct FParticleSystemComponentArray
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<UParticleSystemComponent*> Components;
};

/**
 * Blueprint function library for weather and environmental VFX management
 */
UCLASS()
class ALEXANDER_API UWeatherVFXLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Spawn weather effect at location
	 * @param WorldContextObject - World context
	 * @param WeatherType - Type of weather to spawn
	 * @param Location - World location
	 * @param Intensity - Effect intensity (0-1)
	 * @return Spawned particle system component
	 */
	UFUNCTION(BlueprintCallable, Category = "Weather|VFX", meta = (WorldContext = "WorldContextObject"))
	static UParticleSystemComponent* SpawnWeatherEffect(
		UObject* WorldContextObject,
		EWeatherTypeNew WeatherType,
		FVector Location,
		float Intensity = 1.0f
	);

	/**
	 * Spawn weather effect with custom configuration
	 * @param WorldContextObject - World context
	 * @param Config - VFX configuration
	 * @param Location - World location
	 * @param Rotation - World rotation
	 * @return Spawned particle system component
	 */
	UFUNCTION(BlueprintCallable, Category = "Weather|VFX", meta = (WorldContext = "WorldContextObject"))
	static UParticleSystemComponent* SpawnWeatherEffectWithConfig(
		UObject* WorldContextObject,
		const FWeatherVFXConfig& Config,
		FVector Location,
		FRotator Rotation = FRotator::ZeroRotator
	);

	/**
	 * Update weather effect intensity
	 * @param Effect - Particle system component to update
	 * @param NewIntensity - New intensity value (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Weather|VFX")
	static void UpdateWeatherIntensity(
		UParticleSystemComponent* Effect,
		float NewIntensity
	);

	/**
	 * Register VFX with VFXManager
	 * @param WorldContextObject - World context
	 * @param ParticleSystem - Particle system to register
	 * @param WeatherType - Weather type association
	 * @return True if registration succeeded
	 */
	UFUNCTION(BlueprintCallable, Category = "Weather|VFX", meta = (WorldContext = "WorldContextObject"))
	static bool RegisterWeatherVFX(
		UObject* WorldContextObject,
		UParticleSystem* ParticleSystem,
		EWeatherTypeNew WeatherType
	);

	/**
	 * Spawn atmospheric entry effect
	 * @param WorldContextObject - World context
	 * @param ShipActor - Ship entering atmosphere
	 * @param EntryData - Entry effect configuration
	 * @return Spawned effect component
	 */
	UFUNCTION(BlueprintCallable, Category = "Weather|VFX", meta = (WorldContext = "WorldContextObject"))
	static UParticleSystemComponent* SpawnAtmosphericEntryEffect(
		UObject* WorldContextObject,
		AActor* ShipActor,
		const FAtmosphericEntryEffect& EntryData
	);

	/**
	 * Update atmospheric entry effect based on altitude and velocity
	 * @param Effect - Effect component to update
	 * @param EntryData - Current entry parameters
	 */
	UFUNCTION(BlueprintCallable, Category = "Weather|VFX")
	static void UpdateAtmosphericEntryEffect(
		UParticleSystemComponent* Effect,
		const FAtmosphericEntryEffect& EntryData
	);

	/**
	 * Spawn environmental hazard VFX (asteroid field, debris, etc.)
	 * @param WorldContextObject - World context
	 * @param HazardType - Type of hazard
	 * @param Location - Hazard location
	 * @param Radius - Hazard radius
	 * @return Spawned effect component
	 */
	UFUNCTION(BlueprintCallable, Category = "Weather|VFX", meta = (WorldContext = "WorldContextObject"))
	static UParticleSystemComponent* SpawnEnvironmentalHazard(
		UObject* WorldContextObject,
		EWeatherTypeNew HazardType,
		FVector Location,
		float Radius = 5000.0f
	);

	/**
	 * Get weather VFX for specific type
	 * @param WorldContextObject - World context
	 * @param WeatherType - Weather type to retrieve
	 * @return Particle system for weather type
	 */
	UFUNCTION(BlueprintCallable, Category = "Weather|VFX", meta = (WorldContext = "WorldContextObject"))
	static UParticleSystem* GetWeatherVFX(
		UObject* WorldContextObject,
		EWeatherTypeNew WeatherType
	);

	/**
	 * Create default weather VFX configuration
	 * @param WeatherType - Weather type
	 * @return Default configuration
	 */
	UFUNCTION(BlueprintPure, Category = "Weather|VFX")
	static FWeatherVFXConfig CreateDefaultWeatherConfig(EWeatherTypeNew WeatherType);

	/**
	 * Calculate intensity from weather parameters
	 * @param BaseIntensity - Base intensity value
	 * @param Distance - Distance from weather center
	 * @param MaxDistance - Maximum effect distance
	 * @return Calculated intensity
	 */
	UFUNCTION(BlueprintPure, Category = "Weather|VFX")
	static float CalculateWeatherIntensity(
		float BaseIntensity,
		float Distance,
		float MaxDistance
	);

	/**
	 * Stop all weather effects of a specific type
	 * @param WorldContextObject - World context
	 * @param WeatherType - Weather type to stop
	 */
	UFUNCTION(BlueprintCallable, Category = "Weather|VFX", meta = (WorldContext = "WorldContextObject"))
	static void StopWeatherEffectsByType(
		UObject* WorldContextObject,
		EWeatherTypeNew WeatherType
	);

	/**
	 * Transition between weather effects smoothly
	 * @param CurrentEffect - Current weather effect
	 * @param NewWeatherType - New weather type
	 * @param TransitionDuration - Duration of transition in seconds
	 */
	UFUNCTION(BlueprintCallable, Category = "Weather|VFX", meta = (WorldContext = "WorldContextObject"))
	static void TransitionWeatherEffect(
		UObject* WorldContextObject,
		UParticleSystemComponent* CurrentEffect,
		EWeatherTypeNew NewWeatherType,
		float TransitionDuration = 2.0f
	);

private:
	// Internal registry of weather VFX
	static TMap<EWeatherTypeNew, UParticleSystem*> WeatherVFXRegistry;

	// Helper to get VFXManager
	static class UVFXManager* GetVFXManager(UObject* WorldContextObject);

	// Active weather effects tracking (UE 5.6 compatible - using wrapper struct)
	static TMap<EWeatherTypeNew, FParticleSystemComponentArray> ActiveWeatherEffects;
};
