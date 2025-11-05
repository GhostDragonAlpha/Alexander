// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PlanetConfiguration.h"
#include "BiomeCompatibilitySystem.generated.h"

// Forward declarations
class UCropDefinition;

/**
 * Biome compatibility score for a crop
 */
USTRUCT(BlueprintType)
struct FBiomeCompatibilityScore
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Compatibility")
	EBiomeType BiomeType;

	UPROPERTY(BlueprintReadOnly, Category = "Compatibility")
	float CompatibilityScore; // 0-1, where 1 is perfect compatibility

	UPROPERTY(BlueprintReadOnly, Category = "Compatibility")
	float GrowthRateModifier; // Multiplier for growth rate (0.3 - 1.5)

	UPROPERTY(BlueprintReadOnly, Category = "Compatibility")
	bool bIsNative; // True if this is a native crop for the biome

	UPROPERTY(BlueprintReadOnly, Category = "Compatibility")
	FString CompatibilityReason; // Human-readable explanation

	FBiomeCompatibilityScore()
		: BiomeType(EBiomeType::Grassland)
		, CompatibilityScore(0.5f)
		, GrowthRateModifier(1.0f)
		, bIsNative(false)
		, CompatibilityReason(TEXT(""))
	{
	}
};

/**
 * Biome-specific crop list
 */
USTRUCT(BlueprintType)
struct FBiomeCropList
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crops")
	EBiomeType BiomeType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crops")
	TArray<TSoftObjectPtr<UCropDefinition>> NativeCrops;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crops")
	TArray<TSoftObjectPtr<UCropDefinition>> CompatibleCrops;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crops")
	TArray<TSoftObjectPtr<UCropDefinition>> ChallengingCrops;

	FBiomeCropList()
		: BiomeType(EBiomeType::Grassland)
	{
	}
};

/**
 * Manages crop-biome compatibility and provides growth rate modifiers
 */
UCLASS(BlueprintType)
class ALEXANDER_API UBiomeCompatibilitySystem : public UObject
{
	GENERATED_BODY()

public:
	UBiomeCompatibilitySystem();

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Initialize the compatibility system with biome-crop mappings
	 */
	UFUNCTION(BlueprintCallable, Category = "Compatibility")
	void Initialize();

	// ============================================================================
	// COMPATIBILITY CHECKS
	// ============================================================================

	/**
	 * Check if a crop is compatible with a specific biome
	 * @param CropDef Crop definition to check
	 * @param BiomeType Biome type to check against
	 * @return True if crop can grow in this biome
	 */
	UFUNCTION(BlueprintCallable, Category = "Compatibility")
	bool IsCropCompatibleWithBiome(const UCropDefinition* CropDef, EBiomeType BiomeType) const;

	/**
	 * Calculate compatibility score for a crop in a specific biome
	 * @param CropDef Crop definition
	 * @param BiomeType Biome type
	 * @param Temperature Current temperature in Celsius
	 * @param Humidity Current humidity (0-1)
	 * @param SoilQuality Current soil quality (0-1)
	 * @return Compatibility score structure
	 */
	UFUNCTION(BlueprintCallable, Category = "Compatibility")
	FBiomeCompatibilityScore CalculateCompatibilityScore(
		const UCropDefinition* CropDef,
		EBiomeType BiomeType,
		float Temperature,
		float Humidity,
		float SoilQuality) const;

	/**
	 * Get growth rate modifier for a crop in a specific biome
	 * @param CropDef Crop definition
	 * @param BiomeType Biome type
	 * @return Growth rate multiplier (0.3 - 1.5)
	 */
	UFUNCTION(BlueprintCallable, Category = "Compatibility")
	float GetBiomeGrowthModifier(const UCropDefinition* CropDef, EBiomeType BiomeType) const;

	// ============================================================================
	// CROP LISTS
	// ============================================================================

	/**
	 * Get all crops suitable for a specific biome
	 * @param BiomeType Biome type
	 * @return Array of crop definitions
	 */
	UFUNCTION(BlueprintCallable, Category = "Compatibility")
	TArray<UCropDefinition*> GetCropsForBiome(EBiomeType BiomeType) const;

	/**
	 * Get native crops for a specific biome (best growth rates)
	 * @param BiomeType Biome type
	 * @return Array of native crop definitions
	 */
	UFUNCTION(BlueprintCallable, Category = "Compatibility")
	TArray<UCropDefinition*> GetNativeCropsForBiome(EBiomeType BiomeType) const;

	/**
	 * Get compatible crops for a specific biome (good growth rates)
	 * @param BiomeType Biome type
	 * @return Array of compatible crop definitions
	 */
	UFUNCTION(BlueprintCallable, Category = "Compatibility")
	TArray<UCropDefinition*> GetCompatibleCropsForBiome(EBiomeType BiomeType) const;

	/**
	 * Get challenging crops for a specific biome (reduced growth rates)
	 * @param BiomeType Biome type
	 * @return Array of challenging crop definitions
	 */
	UFUNCTION(BlueprintCallable, Category = "Compatibility")
	TArray<UCropDefinition*> GetChallengingCropsForBiome(EBiomeType BiomeType) const;

	// ============================================================================
	// BIOME SUITABILITY
	// ============================================================================

	/**
	 * Calculate overall biome suitability for farming
	 * @param BiomeType Biome type
	 * @param Temperature Temperature in Celsius
	 * @param Humidity Humidity (0-1)
	 * @param SoilQuality Soil quality (0-1)
	 * @return Suitability score (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Compatibility")
	float CalculateBiomeSuitability(
		EBiomeType BiomeType,
		float Temperature,
		float Humidity,
		float SoilQuality) const;

	/**
	 * Get recommended crops for current conditions
	 * @param BiomeType Biome type
	 * @param Temperature Temperature in Celsius
	 * @param Humidity Humidity (0-1)
	 * @param SoilQuality Soil quality (0-1)
	 * @param MaxResults Maximum number of recommendations
	 * @return Array of recommended crops sorted by suitability
	 */
	UFUNCTION(BlueprintCallable, Category = "Compatibility")
	TArray<UCropDefinition*> GetRecommendedCrops(
		EBiomeType BiomeType,
		float Temperature,
		float Humidity,
		float SoilQuality,
		int32 MaxResults = 5) const;

protected:
	// ============================================================================
	// DATA
	// ============================================================================

	UPROPERTY(EditAnywhere, Category = "Configuration")
	TArray<FBiomeCropList> BiomeCropLists;

	// Growth rate modifiers per biome type
	UPROPERTY(EditAnywhere, Category = "Configuration")
	TMap<EBiomeType, float> BiomeBaseGrowthModifiers;

	// ============================================================================
	// INTERNAL METHODS
	// ============================================================================

	/**
	 * Calculate temperature compatibility factor
	 */
	float CalculateTemperatureCompatibility(const UCropDefinition* CropDef, float Temperature) const;

	/**
	 * Calculate humidity compatibility factor
	 */
	float CalculateHumidityCompatibility(const UCropDefinition* CropDef, float Humidity) const;

	/**
	 * Calculate soil quality compatibility factor
	 */
	float CalculateSoilCompatibility(const UCropDefinition* CropDef, float SoilQuality) const;

	/**
	 * Check if crop is native to biome
	 */
	bool IsNativeCrop(const UCropDefinition* CropDef, EBiomeType BiomeType) const;

	/**
	 * Get biome crop list for a specific biome
	 */
	const FBiomeCropList* GetBiomeCropList(EBiomeType BiomeType) const;

	/**
	 * Setup default biome-crop mappings
	 */
	void SetupDefaultBiomeCropMappings();

	/**
	 * Setup default growth modifiers
	 */
	void SetupDefaultGrowthModifiers();
};
