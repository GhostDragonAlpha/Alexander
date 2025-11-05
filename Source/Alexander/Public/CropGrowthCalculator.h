// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CropGrowthCalculator.generated.h"

// Forward declarations
class UCropDefinition;
struct FFarmEnvironment;

/**
 * Environmental factors affecting crop growth
 */
USTRUCT(BlueprintType)
struct FEnvironmentalFactors
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Environment")
	float Temperature; // Celsius

	UPROPERTY(BlueprintReadWrite, Category = "Environment")
	float Humidity; // 0-1

	UPROPERTY(BlueprintReadWrite, Category = "Environment")
	float Sunlight; // 0-1

	UPROPERTY(BlueprintReadWrite, Category = "Environment")
	float SoilQuality; // 0-1

	UPROPERTY(BlueprintReadWrite, Category = "Environment")
	float WaterLevel; // 0-1

	UPROPERTY(BlueprintReadWrite, Category = "Environment")
	float Fertility; // 0-1

	FEnvironmentalFactors()
		: Temperature(20.0f)
		, Humidity(0.6f)
		, Sunlight(1.0f)
		, SoilQuality(0.7f)
		, WaterLevel(0.5f)
		, Fertility(1.0f)
	{
	}
};

/**
 * Detailed growth calculation result
 */
USTRUCT(BlueprintType)
struct FGrowthCalculationResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Growth")
	float TotalGrowthModifier; // Combined modifier (0-2)

	UPROPERTY(BlueprintReadOnly, Category = "Growth")
	float TemperatureModifier; // Temperature contribution (0-1.2)

	UPROPERTY(BlueprintReadOnly, Category = "Growth")
	float HumidityModifier; // Humidity contribution (0-1.2)

	UPROPERTY(BlueprintReadOnly, Category = "Growth")
	float SunlightModifier; // Sunlight contribution (0-1.2)

	UPROPERTY(BlueprintReadOnly, Category = "Growth")
	float SoilQualityModifier; // Soil quality contribution (0-1)

	UPROPERTY(BlueprintReadOnly, Category = "Growth")
	float WaterModifier; // Water availability contribution (0-1)

	UPROPERTY(BlueprintReadOnly, Category = "Growth")
	float FertilityModifier; // Soil fertility contribution (0-1)

	UPROPERTY(BlueprintReadOnly, Category = "Growth")
	float HealthModifier; // Crop health contribution (0-1)

	UPROPERTY(BlueprintReadOnly, Category = "Growth")
	float SeasonalModifier; // Seasonal contribution (0.5-1.5)

	UPROPERTY(BlueprintReadOnly, Category = "Growth")
	FString LimitingFactor; // What's limiting growth the most

	FGrowthCalculationResult()
		: TotalGrowthModifier(1.0f)
		, TemperatureModifier(1.0f)
		, HumidityModifier(1.0f)
		, SunlightModifier(1.0f)
		, SoilQualityModifier(1.0f)
		, WaterModifier(1.0f)
		, FertilityModifier(1.0f)
		, HealthModifier(1.0f)
		, SeasonalModifier(1.0f)
		, LimitingFactor(TEXT("None"))
	{
	}
};

/**
 * Crop growth calculator - handles all growth rate calculations
 * Implements environmental factor evaluation for crop growth simulation
 */
UCLASS(BlueprintType)
class ALEXANDER_API UCropGrowthCalculator : public UObject
{
	GENERATED_BODY()

public:
	UCropGrowthCalculator();

	// ============================================================================
	// MAIN CALCULATION FUNCTIONS
	// ============================================================================

	/**
	 * Calculate comprehensive growth rate modifier
	 * @param CropDef The crop definition with requirements
	 * @param Environment Current environmental conditions
	 * @param CropHealth Current health of the crop (0-1)
	 * @param SeasonalModifier Seasonal growth modifier (0.5-1.5, default 1.0)
	 * @return Detailed growth calculation result
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth")
	static FGrowthCalculationResult CalculateGrowthRate(
		const UCropDefinition* CropDef,
		const FEnvironmentalFactors& Environment,
		float CropHealth = 1.0f,
		float SeasonalModifier = 1.0f
	);

	/**
	 * Calculate temperature-based growth modifier
	 * @param CurrentTemp Current temperature in Celsius
	 * @param OptimalMin Minimum optimal temperature
	 * @param OptimalMax Maximum optimal temperature
	 * @return Temperature modifier (0-1.2)
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth|Temperature")
	static float CalculateTemperatureModifier(
		float CurrentTemp,
		float OptimalMin,
		float OptimalMax
	);

	/**
	 * Calculate humidity-based growth modifier
	 * @param CurrentHumidity Current humidity (0-1)
	 * @param OptimalMin Minimum optimal humidity
	 * @param OptimalMax Maximum optimal humidity
	 * @return Humidity modifier (0-1.2)
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth|Humidity")
	static float CalculateHumidityModifier(
		float CurrentHumidity,
		float OptimalMin,
		float OptimalMax
	);

	/**
	 * Calculate sunlight exposure modifier
	 * @param CurrentSunlight Current sunlight intensity (0-1)
	 * @param RequiredSunlight Required sunlight for crop (0-1)
	 * @return Sunlight modifier (0-1.2)
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth|Sunlight")
	static float CalculateSunlightModifier(
		float CurrentSunlight,
		float RequiredSunlight
	);

	/**
	 * Calculate soil quality modifier
	 * @param CurrentQuality Current soil quality (0-1)
	 * @param RequiredQuality Required soil quality (0-1)
	 * @return Soil quality modifier (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth|Soil")
	static float CalculateSoilQualityModifier(
		float CurrentQuality,
		float RequiredQuality
	);

	/**
	 * Calculate water availability modifier
	 * @param WaterLevel Current water level (0-1)
	 * @param WaterConsumption Crop water consumption rate
	 * @return Water modifier (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth|Water")
	static float CalculateWaterModifier(
		float WaterLevel,
		float WaterConsumption
	);

	/**
	 * Calculate fertility modifier
	 * @param Fertility Current soil fertility (0-1)
	 * @return Fertility modifier (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth|Fertility")
	static float CalculateFertilityModifier(float Fertility);

	// ============================================================================
	// HELPER FUNCTIONS
	// ============================================================================

	/**
	 * Determine the most limiting factor for growth
	 * @param Result Growth calculation result
	 * @return Name of the most limiting factor
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth")
	static FString DetermineLimitingFactor(const FGrowthCalculationResult& Result);

	/**
	 * Get growth stage description based on progress
	 * @param GrowthProgress Current growth progress (0-1)
	 * @return Human-readable growth stage
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth")
	static FString GetGrowthStageDescription(float GrowthProgress);

	/**
	 * Calculate expected time to maturity
	 * @param CropDef Crop definition
	 * @param Environment Current environment
	 * @param CurrentProgress Current growth progress (0-1)
	 * @return Estimated seconds until maturity
	 */
	UFUNCTION(BlueprintCallable, Category = "Growth")
	static float CalculateTimeToMaturity(
		const UCropDefinition* CropDef,
		const FEnvironmentalFactors& Environment,
		float CurrentProgress
	);

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	// Temperature tolerance (degrees outside optimal before severe penalty)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Temperature")
float TemperatureTolerance;

	// Temperature penalty rate (per degree outside optimal)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Temperature")
	float TemperaturePenaltyRate;

	// Humidity tolerance (deviation from optimal before penalty)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Humidity")
	float HumidityTolerance;

	// Sunlight bonus for excess light (up to this multiplier)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Sunlight")
	float SunlightBonusMax;

// Minimum growth rate (even in worst conditions)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|General")
	float MinimumGrowthRate;

	// Maximum growth rate (with perfect conditions)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|General")
	float MaximumGrowthRate;

private:
	// Helper to apply smooth falloff curve
	static float ApplySmoothFalloff(float Value, float Optimal, float Tolerance);

	// Helper to calculate Gaussian curve for optimal ranges
	static float GaussianCurve(float Value, float Mean, float StdDev);

private:
	// Static default configuration values for static functions
	static float DefaultTemperatureTolerance;
	static float DefaultTemperaturePenaltyRate;
	static float DefaultHumidityTolerance;
	static float DefaultSunlightBonusMax;
	static float DefaultMinimumGrowthRate;
	static float DefaultMaximumGrowthRate;
};
