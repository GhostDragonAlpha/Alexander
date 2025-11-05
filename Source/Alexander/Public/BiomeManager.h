// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PlanetConfiguration.h"
#include "BiomeManager.generated.h"

// Forward declarations
class UProceduralNoiseGenerator;

/**
 * Biome weights at a specific location
 */
USTRUCT(BlueprintType)
struct FBiomeWeights
{
	GENERATED_BODY()

	// Map of biome index to weight (0-1)
	UPROPERTY(BlueprintReadOnly, Category = "Biome")
	TMap<int32, float> Weights;

	// Get dominant biome index
	int32 GetDominantBiome() const;

	// Normalize weights to sum to 1.0
	void Normalize();
};

/**
 * Biome distribution parameters
 */
USTRUCT(BlueprintType)
struct FBiomeDistributionConfig
{
	GENERATED_BODY()

	// 3D noise parameters for biome placement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float BiomeNoiseScale = 5000.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	int32 BiomeNoiseOctaves = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise")
	float BiomeNoisePersistence = 0.5f;

	// Temperature gradient parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature")
	float EquatorTemperature = 30.0f; // Celsius

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature")
	float PoleTemperature = -40.0f; // Celsius

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature")
	float TemperatureVariation = 10.0f; // Random variation

	// Moisture parameters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moisture")
	float MoistureNoiseScale = 3000.0f; // meters

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moisture")
	int32 MoistureNoiseOctaves = 2;

	// Altitude influence
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Altitude")
	float AltitudeInfluence = 0.3f; // 0-1, how much altitude affects biome

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Altitude")
	float SnowLineAltitude = 3000.0f; // meters above sea level
};

/**
 * Manages biome distribution across a planet
 */
UCLASS(BlueprintType)
class ALEXANDER_API UBiomeManager : public UObject
{
	GENERATED_BODY()

public:
	UBiomeManager();

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Initialize biome manager with planet configuration
	 * @param InBiomes Array of biome definitions
	 * @param InPlanetRadius Planet radius in kilometers
	 * @param InSeed Random seed for biome distribution
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome")
	void Initialize(const TArray<FBiomeDefinition>& InBiomes, float InPlanetRadius, int32 InSeed);

	// ============================================================================
	// BIOME QUERIES
	// ============================================================================

	/**
	 * Get biome weights at a specific location
	 * @param WorldPosition World space position
	 * @return Biome weights structure
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome")
	FBiomeWeights GetBiomeWeightsAtLocation(FVector WorldPosition) const;

	/**
	 * Get dominant biome at a specific location
	 * @param WorldPosition World space position
	 * @return Index of dominant biome, or -1 if none
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome")
	int32 GetDominantBiomeAtLocation(FVector WorldPosition) const;

	/**
	 * Get biome definition by index
	 * @param BiomeIndex Index of biome
	 * @return Biome definition
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome")
	const FBiomeDefinition& GetBiomeDefinition(int32 BiomeIndex) const;

	// ============================================================================
	// ENVIRONMENTAL QUERIES
	// ============================================================================

	/**
	 * Get temperature at location based on latitude and altitude
	 * @param WorldPosition World space position
	 * @return Temperature in Celsius
	 */
	UFUNCTION(BlueprintCallable, Category = "Environment")
	float GetTemperatureAtLocation(FVector WorldPosition) const;

	/**
	 * Get moisture level at location
	 * @param WorldPosition World space position
	 * @return Moisture level (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Environment")
	float GetMoistureAtLocation(FVector WorldPosition) const;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FBiomeDistributionConfig DistributionConfig;

protected:
	// Biome definitions
	UPROPERTY()
	TArray<FBiomeDefinition> Biomes;

	// Planet properties
	UPROPERTY()
	float PlanetRadius; // kilometers

	UPROPERTY()
	FVector PlanetCenter;

	UPROPERTY()
	int32 Seed;

	// Noise generator for biome distribution
	UPROPERTY()
	UProceduralNoiseGenerator* NoiseGenerator;

	// ============================================================================
	// INTERNAL METHODS
	// ============================================================================

	/**
	 * Calculate latitude from world position (-90 to 90 degrees)
	 */
	float CalculateLatitude(FVector WorldPosition) const;

	/**
	 * Calculate altitude above sea level in meters
	 */
	float CalculateAltitude(FVector WorldPosition) const;

	/**
	 * Generate 3D noise value for biome distribution
	 */
	float GenerateBiomeNoise(FVector WorldPosition) const;

	/**
	 * Generate moisture map value
	 */
	float GenerateMoistureNoise(FVector WorldPosition) const;

	/**
	 * Calculate biome suitability based on temperature and moisture
	 */
	float CalculateBiomeSuitability(const FBiomeDefinition& Biome, float Temperature, float Moisture, float Altitude) const;
};
