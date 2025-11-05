// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PlanetConfiguration.h"
#include "EnvironmentalModifierSystem.generated.h"

// Forward declarations
class UCropDefinition;

/**
 * Environmental challenge types
 */
UENUM(BlueprintType)
enum class EEnvironmentalChallenge : uint8
{
	None UMETA(DisplayName = "None"),
	Drought UMETA(DisplayName = "Drought"),
	Flood UMETA(DisplayName = "Flood"),
	HeatWave UMETA(DisplayName = "Heat Wave"),
	ColdSnap UMETA(DisplayName = "Cold Snap"),
	HighWinds UMETA(DisplayName = "High Winds"),
	PoorSoil UMETA(DisplayName = "Poor Soil"),
	Pests UMETA(DisplayName = "Pests"),
	Disease UMETA(DisplayName = "Disease")
};

/**
 * Environmental modifier result
 */
USTRUCT(BlueprintType)
struct FEnvironmentalModifierResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Modifiers")
	float WaterConsumptionMultiplier; // 0.5 - 3.0 (up to 300%)

	UPROPERTY(BlueprintReadOnly, Category = "Modifiers")
	float TemperatureGrowthModifier; // 0.3 - 1.5

	UPROPERTY(BlueprintReadOnly, Category = "Modifiers")
	float BiomeCompatibilityModifier; // 0.5 - 1.2

	UPROPERTY(BlueprintReadOnly, Category = "Modifiers")
	float NativeCropBonus; // 1.0 - 1.3 (up to 30% bonus)

	UPROPERTY(BlueprintReadOnly, Category = "Modifiers")
	float ChallengeModifier; // 0.5 - 1.0 (penalties from challenges)

	UPROPERTY(BlueprintReadOnly, Category = "Modifiers")
	float TotalGrowthModifier; // Combined modifier

	UPROPERTY(BlueprintReadOnly, Category = "Modifiers")
	TArray<EEnvironmentalChallenge> ActiveChallenges;

	UPROPERTY(BlueprintReadOnly, Category = "Modifiers")
	FString ModifierSummary; // Human-readable summary

	FEnvironmentalModifierResult()
		: WaterConsumptionMultiplier(1.0f)
		, TemperatureGrowthModifier(1.0f)
		, BiomeCompatibilityModifier(1.0f)
		, NativeCropBonus(1.0f)
		, ChallengeModifier(1.0f)
		, TotalGrowthModifier(1.0f)
		, ModifierSummary(TEXT(""))
	{
	}
};

/**
 * Biome-specific challenge configuration
 */
USTRUCT(BlueprintType)
struct FBiomeChallengeConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	EBiomeType BiomeType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	TArray<EEnvironmentalChallenge> CommonChallenges;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	float ChallengeSeverity; // 0-1, how severe challenges are in this biome

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Challenge")
	float ChallengeFrequency; // 0-1, how often challenges occur

	FBiomeChallengeConfig()
		: BiomeType(EBiomeType::Grassland)
		, ChallengeSeverity(0.5f)
		, ChallengeFrequency(0.3f)
	{
	}
};

/**
 * Environmental Modifier System
 * Calculates environmental effects on crop growth including:
 * - Humidity-based water consumption (up to 300%)
 * - Temperature growth modifiers
 * - Biome-specific challenges
 * - Native crop bonuses
 */
UCLASS(BlueprintType)
class ALEXANDER_API UEnvironmentalModifierSystem : public UObject
{
	GENERATED_BODY()

public:
	UEnvironmentalModifierSystem();

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Initialize the environmental modifier system
	 */
	UFUNCTION(BlueprintCallable, Category = "Environmental")
	void Initialize();

	// ============================================================================
	// MAIN CALCULATION
	// ============================================================================

	/**
	 * Calculate all environmental modifiers for a crop
	 * @param CropDef Crop definition
	 * @param BiomeType Current biome
	 * @param Temperature Current temperature (Celsius)
	 * @param Humidity Current humidity (0-1)
	 * @param bIsNativeCrop Whether this is a native crop for the biome
	 * @return Complete environmental modifier result
	 */
	UFUNCTION(BlueprintCallable, Category = "Environmental")
	FEnvironmentalModifierResult CalculateEnvironmentalModifiers(
		const UCropDefinition* CropDef,
		EBiomeType BiomeType,
		float Temperature,
		float Humidity,
		bool bIsNativeCrop = false
	) const;

	// ============================================================================
	// WATER CONSUMPTION
	// ============================================================================

	/**
	 * Calculate humidity-based water consumption multiplier
	 * Low humidity increases water consumption up to 300%
	 * @param Humidity Current humidity (0-1)
	 * @param OptimalHumidityMin Crop's minimum optimal humidity
	 * @param OptimalHumidityMax Crop's maximum optimal humidity
	 * @return Water consumption multiplier (0.5 - 3.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Environmental|Water")
	static float CalculateWaterConsumptionMultiplier(
		float Humidity,
		float OptimalHumidityMin,
		float OptimalHumidityMax
	);

	/**
	 * Calculate adjusted water consumption for a crop
	 * @param BaseWaterConsumption Base water consumption from crop definition
	 * @param Humidity Current humidity
	 * @param OptimalHumidityMin Crop's minimum optimal humidity
	 * @param OptimalHumidityMax Crop's maximum optimal humidity
	 * @return Adjusted water consumption in liters per day
	 */
	UFUNCTION(BlueprintCallable, Category = "Environmental|Water")
	static float CalculateAdjustedWaterConsumption(
		float BaseWaterConsumption,
		float Humidity,
		float OptimalHumidityMin,
		float OptimalHumidityMax
	);

	// ============================================================================
	// TEMPERATURE MODIFIERS
	// ============================================================================

	/**
	 * Calculate temperature-based growth modifier
	 * @param Temperature Current temperature (Celsius)
	 * @param OptimalTempMin Crop's minimum optimal temperature
	 * @param OptimalTempMax Crop's maximum optimal temperature
	 * @return Temperature growth modifier (0.3 - 1.5)
	 */
	UFUNCTION(BlueprintCallable, Category = "Environmental|Temperature")
	static float CalculateTemperatureGrowthModifier(
		float Temperature,
		float OptimalTempMin,
		float OptimalTempMax
	);

	/**
	 * Check if temperature is in extreme range
	 * @param Temperature Current temperature
	 * @return True if temperature is extreme (< 0°C or > 40°C)
	 */
	UFUNCTION(BlueprintCallable, Category = "Environmental|Temperature")
	static bool IsExtremeTemperature(float Temperature);

	// ============================================================================
	// BIOME CHALLENGES
	// ============================================================================

	/**
	 * Get active environmental challenges for a biome
	 * @param BiomeType Biome type
	 * @param Temperature Current temperature
	 * @param Humidity Current humidity
	 * @param SoilQuality Current soil quality
	 * @return Array of active challenges
	 */
	UFUNCTION(BlueprintCallable, Category = "Environmental|Challenges")
	TArray<EEnvironmentalChallenge> GetActiveChallenges(
		EBiomeType BiomeType,
		float Temperature,
		float Humidity,
		float SoilQuality
	) const;

	/**
	 * Calculate challenge modifier based on active challenges
	 * @param Challenges Array of active challenges
	 * @param CropDef Crop definition (for resistance checks)
	 * @return Challenge modifier (0.5 - 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Environmental|Challenges")
	float CalculateChallengeModifier(
		const TArray<EEnvironmentalChallenge>& Challenges,
		const UCropDefinition* CropDef
	) const;

	/**
	 * Get biome-specific challenges
	 * @param BiomeType Biome type
	 * @return Array of common challenges for this biome
	 */
	UFUNCTION(BlueprintCallable, Category = "Environmental|Challenges")
	TArray<EEnvironmentalChallenge> GetBiomeChallenges(EBiomeType BiomeType) const;

	// ============================================================================
	// NATIVE CROP BONUSES
	// ============================================================================

	/**
	 * Calculate native crop growth bonus
	 * Native crops get up to 30% growth bonus
	 * @param bIsNative Whether crop is native to biome
	 * @param BiomeType Current biome
	 * @return Native crop bonus (1.0 - 1.3)
	 */
	UFUNCTION(BlueprintCallable, Category = "Environmental|Native")
	static float CalculateNativeCropBonus(bool bIsNative, EBiomeType BiomeType);

	/**
	 * Get native crop benefits description
	 * @param BiomeType Biome type
	 * @return Human-readable description of native crop benefits
	 */
	UFUNCTION(BlueprintCallable, Category = "Environmental|Native")
	static FString GetNativeCropBenefitsDescription(EBiomeType BiomeType);

	// ============================================================================
	// BIOME COMPATIBILITY
	// ============================================================================

	/**
	 * Calculate biome compatibility modifier
	 * @param CropDef Crop definition
	 * @param BiomeType Current biome
	 * @return Compatibility modifier (0.5 - 1.2)
	 */
	UFUNCTION(BlueprintCallable, Category = "Environmental|Biome")
	float CalculateBiomeCompatibilityModifier(
		const UCropDefinition* CropDef,
		EBiomeType BiomeType
	) const;

	// ============================================================================
	// UTILITY FUNCTIONS
	// ============================================================================

	/**
	 * Generate human-readable summary of environmental modifiers
	 * @param Result Environmental modifier result
	 * @return Summary string
	 */
	UFUNCTION(BlueprintCallable, Category = "Environmental")
	static FString GenerateModifierSummary(const FEnvironmentalModifierResult& Result);

	/**
	 * Get challenge description
	 * @param Challenge Challenge type
	 * @return Human-readable description
	 */
	UFUNCTION(BlueprintCallable, Category = "Environmental")
	static FString GetChallengeDescription(EEnvironmentalChallenge Challenge);

protected:
	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	UPROPERTY(EditAnywhere, Category = "Configuration")
	TArray<FBiomeChallengeConfig> BiomeChallengeConfigs;

	// Water consumption multiplier ranges
	UPROPERTY(EditAnywhere, Category = "Configuration|Water")
	float MinWaterConsumptionMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Configuration|Water")
	float MaxWaterConsumptionMultiplier = 3.0f;

	// Temperature modifier ranges
	UPROPERTY(EditAnywhere, Category = "Configuration|Temperature")
	float MinTemperatureModifier = 0.3f;

	UPROPERTY(EditAnywhere, Category = "Configuration|Temperature")
	float MaxTemperatureModifier = 1.5f;

	// Native crop bonus
	UPROPERTY(EditAnywhere, Category = "Configuration|Native")
	float NativeCropBonusAmount = 0.3f; // 30% bonus

	// Challenge severity
	UPROPERTY(EditAnywhere, Category = "Configuration|Challenges")
	float BaseChallengeModifier = 0.8f; // 20% penalty per challenge

	// ============================================================================
	// INTERNAL METHODS
	// ============================================================================

	/**
	 * Setup default biome challenge configurations
	 */
	void SetupDefaultBiomeChallenges();

	/**
	 * Get biome challenge configuration
	 */
	const FBiomeChallengeConfig* GetBiomeChallengeConfig(EBiomeType BiomeType) const;
};
