// Copyright Epic Games, Inc. All Rights Reserved.

#include "BiomeManager.h"
#include "ProceduralNoiseGenerator.h"
#include "Kismet/KismetMathLibrary.h"

// ============================================================================
// FBiomeWeights Implementation
// ============================================================================

int32 FBiomeWeights::GetDominantBiome() const
{
	int32 DominantBiome = -1;
	float MaxWeight = 0.0f;

	for (const auto& Pair : Weights)
	{
		if (Pair.Value > MaxWeight)
		{
			MaxWeight = Pair.Value;
			DominantBiome = Pair.Key;
		}
	}

	return DominantBiome;
}

void FBiomeWeights::Normalize()
{
	float TotalWeight = 0.0f;
	for (const auto& Pair : Weights)
	{
		TotalWeight += Pair.Value;
	}

	if (TotalWeight > 0.0f)
	{
		for (auto& Pair : Weights)
		{
			Pair.Value /= TotalWeight;
		}
	}
}

// ============================================================================
// UBiomeManager Implementation
// ============================================================================

UBiomeManager::UBiomeManager()
{
	PlanetRadius = 6371.0f; // Earth radius default
	PlanetCenter = FVector::ZeroVector;
	Seed = 12345;
}

void UBiomeManager::Initialize(const TArray<FBiomeDefinition>& InBiomes, float InPlanetRadius, int32 InSeed)
{
	Biomes = InBiomes;
	PlanetRadius = InPlanetRadius;
	Seed = InSeed;

	// Create noise generator if not already created
	if (!NoiseGenerator)
	{
		NoiseGenerator = NewObject<UProceduralNoiseGenerator>(this);
	}

	// Initialize is not a method - remove this call

	UE_LOG(LogTemp, Log, TEXT("BiomeManager initialized with %d biomes, radius %.1f km, seed %d"), 
		Biomes.Num(), PlanetRadius, Seed);
}

FBiomeWeights UBiomeManager::GetBiomeWeightsAtLocation(FVector WorldPosition) const
{
	FBiomeWeights Result;

	if (Biomes.Num() == 0)
	{
		return Result;
	}

	// Calculate environmental parameters
	float Latitude = CalculateLatitude(WorldPosition);
	float Altitude = CalculateAltitude(WorldPosition);
	float Temperature = GetTemperatureAtLocation(WorldPosition);
	float Moisture = GetMoistureAtLocation(WorldPosition);

	// Calculate suitability for each biome
	TArray<float> Suitabilities;
	Suitabilities.SetNum(Biomes.Num());

	for (int32 i = 0; i < Biomes.Num(); i++)
	{
		Suitabilities[i] = CalculateBiomeSuitability(Biomes[i], Temperature, Moisture, Altitude);
		Result.Weights.Add(i, Suitabilities[i]);
	}

	// Normalize weights
	Result.Normalize();

	return Result;
}

int32 UBiomeManager::GetDominantBiomeAtLocation(FVector WorldPosition) const
{
	FBiomeWeights Weights = GetBiomeWeightsAtLocation(WorldPosition);
	return Weights.GetDominantBiome();
}

const FBiomeDefinition& UBiomeManager::GetBiomeDefinition(int32 BiomeIndex) const
{
	static FBiomeDefinition DefaultBiome;
	
	if (Biomes.IsValidIndex(BiomeIndex))
	{
		return Biomes[BiomeIndex];
	}

	return DefaultBiome;
}

float UBiomeManager::GetTemperatureAtLocation(FVector WorldPosition) const
{
	// Calculate latitude-based temperature gradient
	float Latitude = CalculateLatitude(WorldPosition);
	float LatitudeFactor = FMath::Abs(Latitude) / 90.0f; // 0 at equator, 1 at poles

	// Base temperature from latitude
	float BaseTemp = FMath::Lerp(
		DistributionConfig.EquatorTemperature,
		DistributionConfig.PoleTemperature,
		LatitudeFactor
	);

	// Add noise variation
	float NoiseValue = GenerateBiomeNoise(WorldPosition);
	float TempVariation = NoiseValue * DistributionConfig.TemperatureVariation;

	// Altitude effect (temperature decreases with altitude)
	float Altitude = CalculateAltitude(WorldPosition);
	float AltitudeEffect = -0.0065f * Altitude; // Standard lapse rate: -6.5°C per 1000m

	return BaseTemp + TempVariation + AltitudeEffect;
}

float UBiomeManager::GetMoistureAtLocation(FVector WorldPosition) const
{
	// Generate moisture using noise
	float MoistureNoise = GenerateMoistureNoise(WorldPosition);

	// Normalize to 0-1 range
	float Moisture = (MoistureNoise + 1.0f) * 0.5f;

	// Clamp to valid range
	return FMath::Clamp(Moisture, 0.0f, 1.0f);
}

float UBiomeManager::CalculateLatitude(FVector WorldPosition) const
{
	// Calculate vector from planet center to position
	FVector ToPosition = WorldPosition - PlanetCenter;
	ToPosition.Normalize();

	// Latitude is angle from equatorial plane
	// Assuming Z-up coordinate system
	float Latitude = FMath::Asin(ToPosition.Z) * (180.0f / PI);

	return Latitude;
}

float UBiomeManager::CalculateAltitude(FVector WorldPosition) const
{
	// Distance from planet center
	float DistanceFromCenter = FVector::Dist(WorldPosition, PlanetCenter);

	// Convert planet radius from km to cm (Unreal units)
	float PlanetRadiusCm = PlanetRadius * 100000.0f;

	// Altitude in meters
	float AltitudeMeters = (DistanceFromCenter - PlanetRadiusCm) / 100.0f;

	return AltitudeMeters;
}

float UBiomeManager::GenerateBiomeNoise(FVector WorldPosition) const
{
	if (!NoiseGenerator)
	{
		return 0.0f;
	}

	// Normalize position to planet surface
	FVector NormalizedPos = (WorldPosition - PlanetCenter);
	NormalizedPos.Normalize();

	// Scale by noise scale
	FVector NoisePos = NormalizedPos * DistributionConfig.BiomeNoiseScale;

// Generate multi-octave noise
	float NoiseValue = 0.0f;
	float Amplitude = 1.0f;
	float Frequency = 1.0f;
	
	for (int32 i = 0; i < DistributionConfig.BiomeNoiseOctaves; i++)
	{
		NoiseValue += UProceduralNoiseGenerator::PerlinNoise3D(
			NoisePos.X * Frequency,
			NoisePos.Y * Frequency,
			NoisePos.Z * Frequency,
			Seed + i
		) * Amplitude;
		
		Amplitude *= DistributionConfig.BiomeNoisePersistence;
		Frequency *= 2.0f;
	}
	
	return NoiseValue;
}

float UBiomeManager::GenerateMoistureNoise(FVector WorldPosition) const
{
	if (!NoiseGenerator)
	{
		return 0.0f;
	}

	// Normalize position to planet surface
	FVector NormalizedPos = (WorldPosition - PlanetCenter);
	NormalizedPos.Normalize();

	// Scale by moisture noise scale (different from biome noise)
	FVector NoisePos = NormalizedPos * DistributionConfig.MoistureNoiseScale;

// Generate multi-octave noise with different seed offset
	FVector OffsetNoisePos = NoisePos + FVector(1000.0f, 2000.0f, 3000.0f);
	float NoiseValue = 0.0f;
	float Amplitude = 1.0f;
	float Frequency = 1.0f;
	
	for (int32 i = 0; i < DistributionConfig.MoistureNoiseOctaves; i++)
	{
		NoiseValue += UProceduralNoiseGenerator::PerlinNoise3D(
			OffsetNoisePos.X * Frequency,
			OffsetNoisePos.Y * Frequency,
			OffsetNoisePos.Z * Frequency,
			Seed + 1000 + i // Different seed offset
		) * Amplitude;
		
		Amplitude *= 0.5f; // Fixed persistence
		Frequency *= 2.0f;
	}
	
	return NoiseValue;
}

float UBiomeManager::CalculateBiomeSuitability(const FBiomeDefinition& Biome, float Temperature, float Moisture, float Altitude) const
{
	float Suitability = 1.0f;

	// Temperature suitability (Gaussian falloff from biome's preferred temperature)
	float TempDiff = FMath::Abs(Temperature - Biome.Temperature);
	float TempSuitability = FMath::Exp(-TempDiff * TempDiff / 400.0f); // Falloff over ~20°C range

	// Moisture suitability (Gaussian falloff from biome's preferred humidity)
	float MoistureDiff = FMath::Abs(Moisture - Biome.Humidity);
	float MoistureSuitability = FMath::Exp(-MoistureDiff * MoistureDiff / 0.2f); // Falloff over ~0.4 range

	// Altitude influence
	float AltitudeFactor = 1.0f;
	if (Altitude > DistributionConfig.SnowLineAltitude)
	{
		// Above snow line, favor tundra/alpine biomes
		if (Biome.BiomeType == EBiomeType::Tundra || Biome.BiomeType == EBiomeType::Alpine)
		{
			AltitudeFactor = 2.0f;
		}
		else
		{
			AltitudeFactor = 0.1f;
		}
	}

	// Combine factors
	Suitability = TempSuitability * MoistureSuitability * AltitudeFactor;

	// Apply altitude influence from config
	Suitability = FMath::Lerp(Suitability, Suitability * AltitudeFactor, DistributionConfig.AltitudeInfluence);

	return FMath::Max(Suitability, 0.01f); // Minimum suitability to avoid zero weights
}
