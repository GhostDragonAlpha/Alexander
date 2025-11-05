// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PlanetConfiguration.h"
#include "BiomeSoilSystem.generated.h"

/**
 * Soil variation data for a specific location
 */
USTRUCT(BlueprintType)
struct FSoilVariation
{
	GENERATED_BODY()

	// Soil quality (0.3-1.0 as per requirements)
	UPROPERTY(BlueprintReadOnly, Category = "Soil")
	float SoilQuality;

	// Visual soil color
	UPROPERTY(BlueprintReadOnly, Category = "Soil")
	FLinearColor SoilColor;

	// Soil texture properties
	UPROPERTY(BlueprintReadOnly, Category = "Soil")
	float Roughness; // 0-1, affects visual appearance

	UPROPERTY(BlueprintReadOnly, Category = "Soil")
	float Moisture; // 0-1, base moisture level

	UPROPERTY(BlueprintReadOnly, Category = "Soil")
	float Drainage; // 0-1, water drainage rate

	// Water table depth in meters
	UPROPERTY(BlueprintReadOnly, Category = "Soil")
	float WaterTableDepth;

	// Biome type this soil belongs to
	UPROPERTY(BlueprintReadOnly, Category = "Soil")
	EBiomeType BiomeType;

	// Biome name
	UPROPERTY(BlueprintReadOnly, Category = "Soil")
	FString BiomeName;

	FSoilVariation()
		: SoilQuality(0.5f)
		, SoilColor(FLinearColor(0.4f, 0.3f, 0.2f, 1.0f))
		, Roughness(0.8f)
		, Moisture(0.5f)
		, Drainage(0.5f)
		, WaterTableDepth(10.0f)
		, BiomeType(EBiomeType::Grassland)
		, BiomeName(TEXT("Unknown"))
	{
	}
};

/**
 * System for managing biome-specific soil variations
 * Implements requirements 12.2 and 12.5
 */
UCLASS(BlueprintType)
class ALEXANDER_API UBiomeSoilSystem : public UObject
{
	GENERATED_BODY()

public:
	UBiomeSoilSystem();

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Initialize the soil system with biome definitions
	 * @param InBiomes Array of biome definitions from planet configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Soil")
	void Initialize(const TArray<FBiomeDefinition>& InBiomes);

	// ============================================================================
	// SOIL QUERIES
	// ============================================================================

	/**
	 * Get soil variation data for a specific biome type
	 * @param BiomeType The biome type to query
	 * @return Soil variation data
	 */
	UFUNCTION(BlueprintCallable, Category = "Soil")
	FSoilVariation GetSoilVariationForBiome(EBiomeType BiomeType) const;

	/**
	 * Get soil variation data from biome definition
	 * @param BiomeDef The biome definition
	 * @return Soil variation data
	 */
	UFUNCTION(BlueprintCallable, Category = "Soil")
	FSoilVariation GetSoilVariationFromDefinition(const FBiomeDefinition& BiomeDef) const;

	/**
	 * Get blended soil variation for multiple biomes
	 * @param BiomeWeights Map of biome indices to weights
	 * @param InBiomeDefinitions Array of all biome definitions
	 * @return Blended soil variation
	 */
	UFUNCTION(BlueprintCallable, Category = "Soil")
	FSoilVariation GetBlendedSoilVariation(const TMap<int32, float>& BiomeWeights, const TArray<FBiomeDefinition>& InBiomeDefinitions) const;

	/**
	 * Calculate soil quality with local variation
	 * @param BaseSoilQuality Base quality from biome (0.3-1.0)
	 * @param LocalVariationSeed Seed for local variation
	 * @return Modified soil quality
	 */
	UFUNCTION(BlueprintCallable, Category = "Soil")
	float CalculateSoilQualityWithVariation(float BaseSoilQuality, int32 LocalVariationSeed) const;

	/**
	 * Get soil color with moisture and health modifiers
	 * @param BaseSoilColor Base color from biome
	 * @param MoistureLevel Current moisture (0-1)
	 * @param HealthLevel Soil health (0-1)
	 * @return Modified soil color
	 */
	UFUNCTION(BlueprintCallable, Category = "Soil")
	FLinearColor GetModifiedSoilColor(FLinearColor BaseSoilColor, float MoistureLevel, float HealthLevel) const;

	/**
	 * Calculate water availability based on water table depth and rainfall
	 * @param WaterTableDepth Depth to water table in meters
	 * @param RecentRainfall Recent rainfall in mm
	 * @return Water availability (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Soil")
	float CalculateWaterAvailability(float WaterTableDepth, float RecentRainfall) const;

	// ============================================================================
	// SOIL TEXTURE UTILITIES
	// ============================================================================

	/**
	 * Get soil texture description based on properties
	 * @param SoilVar Soil variation data
	 * @return Human-readable texture description
	 */
	UFUNCTION(BlueprintCallable, Category = "Soil")
	FString GetSoilTextureDescription(const FSoilVariation& SoilVar) const;

	/**
	 * Validate soil quality is within acceptable range (0.3-1.0)
	 * @param SoilQuality Quality value to validate
	 * @return Clamped soil quality
	 */
	UFUNCTION(BlueprintCallable, Category = "Soil")
	float ValidateSoilQuality(float SoilQuality) const;

protected:
	// Cached biome definitions
	UPROPERTY()
	TArray<FBiomeDefinition> BiomeDefinitions;

	// Map of biome type to soil variation
	UPROPERTY()
	TMap<EBiomeType, FSoilVariation> BiomeSoilMap;

	// ============================================================================
	// INTERNAL METHODS
	// ============================================================================

	/**
	 * Build the biome soil map from definitions
	 */
	void BuildBiomeSoilMap();

	/**
	 * Create default soil variation for a biome type
	 */
	FSoilVariation CreateDefaultSoilVariation(EBiomeType BiomeType) const;

	/**
	 * Blend two soil colors
	 */
	FLinearColor BlendSoilColors(FLinearColor ColorA, FLinearColor ColorB, float Weight) const;
};
