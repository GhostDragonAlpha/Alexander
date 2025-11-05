// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CropHealthSystem.generated.h"

// Forward declarations
class UCropDefinition;
struct FEnvironmentalFactors;

/**
 * Health status indicators
 */
UENUM(BlueprintType)
enum class ECropHealthStatus : uint8
{
	Excellent,      // 0.9-1.0
	Good,           // 0.7-0.9
	Fair,           // 0.5-0.7
	Poor,           // 0.3-0.5
	Critical,       // 0.1-0.3
	Dying           // 0.0-0.1
};

/**
 * Health degradation factors
 */
USTRUCT(BlueprintType)
struct FHealthDegradationFactors
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Health")
	float WaterDeficit; // 0-1, how much water is lacking

	UPROPERTY(BlueprintReadWrite, Category = "Health")
	float TemperatureStress; // 0-1, temperature outside optimal

	UPROPERTY(BlueprintReadWrite, Category = "Health")
	float NutrientDeficiency; // 0-1, soil nutrient levels

	UPROPERTY(BlueprintReadWrite, Category = "Health")
	float PestDamage; // 0-1, pest infestation level

	UPROPERTY(BlueprintReadWrite, Category = "Health")
	float DiseaseLevel; // 0-1, disease severity

	UPROPERTY(BlueprintReadWrite, Category = "Health")
	float Age; // 0-1, crop age relative to lifecycle

	FHealthDegradationFactors()
		: WaterDeficit(0.0f)
		, TemperatureStress(0.0f)
		, NutrientDeficiency(0.0f)
		, PestDamage(0.0f)
		, DiseaseLevel(0.0f)
		, Age(0.0f)
	{
	}
};

/**
 * Visual health indicators
 */
USTRUCT(BlueprintType)
struct FHealthVisualIndicators
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Visual")
	FLinearColor LeafColor; // Color tint for leaves

	UPROPERTY(BlueprintReadWrite, Category = "Visual")
	float WiltingAmount; // 0-1, how much the plant droops

	UPROPERTY(BlueprintReadWrite, Category = "Visual")
	float ScaleModifier; // 0-1, size reduction from poor health

	UPROPERTY(BlueprintReadWrite, Category = "Visual")
	bool bShowBrownSpots; // Show disease/damage spots

	UPROPERTY(BlueprintReadWrite, Category = "Visual")
	bool bShowYellowing; // Show nutrient deficiency

	UPROPERTY(BlueprintReadWrite, Category = "Visual")
	bool bShowWilting; // Show water stress

	FHealthVisualIndicators()
		: LeafColor(FLinearColor::Green)
		, WiltingAmount(0.0f)
		, ScaleModifier(1.0f)
		, bShowBrownSpots(false)
		, bShowYellowing(false)
		, bShowWilting(false)
	{
	}
};

/**
 * Comprehensive health calculation result
 */
USTRUCT(BlueprintType)
struct FHealthCalculationResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float CurrentHealth; // 0-1

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float HealthChange; // Delta per second

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	ECropHealthStatus HealthStatus;

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	FHealthDegradationFactors DegradationFactors;

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	FHealthVisualIndicators VisualIndicators;

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	FString PrimaryIssue; // Main health problem

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	bool bNeedsWater;

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	bool bNeedsFertilizer;

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	bool bNeedsPestControl;

	FHealthCalculationResult()
		: CurrentHealth(1.0f)
		, HealthChange(0.0f)
		, HealthStatus(ECropHealthStatus::Excellent)
		, bNeedsWater(false)
		, bNeedsFertilizer(false)
		, bNeedsPestControl(false)
	{
	}
};

/**
 * Crop health system
 * Manages crop health calculation, degradation, and visual indicators
 */
UCLASS(BlueprintType)
class ALEXANDER_API UCropHealthSystem : public UObject
{
	GENERATED_BODY()

public:
	UCropHealthSystem();

	// ============================================================================
	// HEALTH CALCULATION
	// ============================================================================

	/**
	 * Calculate comprehensive crop health
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Health")
	static FHealthCalculationResult CalculateCropHealth(
		const UCropDefinition* CropDef,
		const FEnvironmentalFactors& Environment,
		float CurrentHealth,
		float WaterLevel,
		float SoilFertility,
		float TimeSinceLastWater,
		float DeltaTime
	);

	/**
	 * Update health over time with degradation
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Health")
	static float UpdateHealthDegradation(
		float CurrentHealth,
		const FHealthDegradationFactors& Factors,
		float DeltaTime
	);

	// ============================================================================
	// WATER NEED DETECTION
	// ============================================================================

	/**
	 * Check if crop needs water
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Health|Water")
	static bool NeedsWater(
		float WaterLevel,
		float WaterConsumption,
		float TimeSinceLastWater
	);

	/**
	 * Calculate water deficit severity
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Health|Water")
	static float CalculateWaterDeficit(
		float WaterLevel,
		float WaterConsumption,
		float TimeSinceLastWater
	);

	/**
	 * Get time until water is critically needed
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Health|Water")
	static float GetTimeUntilWaterNeeded(
		float WaterLevel,
		float WaterConsumption
	);

	// ============================================================================
	// VISUAL INDICATORS
	// ============================================================================

	/**
	 * Generate visual indicators based on health
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Health|Visual")
	static FHealthVisualIndicators GenerateVisualIndicators(
		float Health,
		const FHealthDegradationFactors& Factors
	);

	/**
	 * Calculate leaf color based on health
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Health|Visual")
	static FLinearColor CalculateLeafColor(
		float Health,
		float NutrientDeficiency,
		float WaterDeficit
	);

	/**
	 * Calculate wilting amount
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Health|Visual")
	static float CalculateWiltingAmount(
		float Health,
		float WaterDeficit
	);

	/**
	 * Calculate scale modifier from health
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Health|Visual")
	static float CalculateScaleModifier(float Health);

	// ============================================================================
	// DEGRADATION FACTORS
	// ============================================================================

	/**
	 * Calculate all degradation factors
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Health|Degradation")
	static FHealthDegradationFactors CalculateDegradationFactors(
		const UCropDefinition* CropDef,
		const FEnvironmentalFactors& Environment,
		float WaterLevel,
		float SoilFertility,
		float TimeSinceLastWater
	);

	/**
	 * Calculate temperature stress factor
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Health|Degradation")
	static float CalculateTemperatureStress(
		float CurrentTemp,
		float OptimalMin,
		float OptimalMax
	);

	/**
	 * Calculate nutrient deficiency factor
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Health|Degradation")
	static float CalculateNutrientDeficiency(
		float SoilFertility,
		float RequiredQuality
	);

	// ============================================================================
	// HEALTH STATUS
	// ============================================================================

	/**
	 * Get health status from health value
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Health|Status")
	static ECropHealthStatus GetHealthStatus(float Health);

	/**
	 * Get health status description
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Health|Status")
	static FString GetHealthStatusDescription(ECropHealthStatus Status);

	/**
	 * Get health status color
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Health|Status")
	static FLinearColor GetHealthStatusColor(ECropHealthStatus Status);

	/**
	 * Determine primary health issue
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop Health|Status")
	static FString DeterminePrimaryIssue(const FHealthDegradationFactors& Factors);

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

// Health degradation rate when not watered (per minute)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Degradation")
	float HealthDegradationRate;

	// Time without water before health starts degrading (seconds)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Water")
float WaterGracePeriod;

	// Water level threshold for "needs water" warning
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Water")
	float WaterWarningThreshold;

	// Critical water level (severe health impact)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Water")
	float WaterCriticalThreshold;

// Temperature stress threshold (degrees outside optimal)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Temperature")
	float TemperatureStressThreshold;

	// Nutrient deficiency threshold
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Nutrients")
	float NutrientDeficiencyThreshold;

	// Maximum health recovery rate per second
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|Recovery")
	float MaxHealthRecoveryRate;

	// Minimum health before crop dies
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config|General")
	float MinimumViableHealth;

private:
	// Static default configuration values for static functions
	static float DefaultHealthDegradationRate;
	static float DefaultWaterGracePeriod;
	static float DefaultWaterWarningThreshold;
	static float DefaultWaterCriticalThreshold;
	static float DefaultTemperatureStressThreshold;
	static float DefaultNutrientDeficiencyThreshold;
	static float DefaultMaxHealthRecoveryRate;
	static float DefaultMinimumViableHealth;
};

