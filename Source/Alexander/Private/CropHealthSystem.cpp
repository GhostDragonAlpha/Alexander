// Copyright Epic Games, Inc. All Rights Reserved.

#include "CropHealthSystem.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "CropDefinition.h"
#include "CropGrowthCalculator.h"

// Define static default configuration values
float UCropHealthSystem::DefaultHealthDegradationRate = 0.1f; // 10% per minute without water
float UCropHealthSystem::DefaultWaterGracePeriod = 300.0f; // 5 minutes
float UCropHealthSystem::DefaultWaterWarningThreshold = 0.3f;
float UCropHealthSystem::DefaultWaterCriticalThreshold = 0.1f;
float UCropHealthSystem::DefaultTemperatureStressThreshold = 10.0f; // 10 degrees C
float UCropHealthSystem::DefaultNutrientDeficiencyThreshold = 0.5f;
float UCropHealthSystem::DefaultMaxHealthRecoveryRate = 0.05f; // 5% per second
float UCropHealthSystem::DefaultMinimumViableHealth = 0.05f;

UCropHealthSystem::UCropHealthSystem()
{
	// Initialize configuration values with defaults
	DefaultHealthDegradationRate = 0.1f; // 10% per minute without water
	DefaultWaterGracePeriod = 300.0f; // 5 minutes
	DefaultWaterWarningThreshold = 0.3f;
	WaterCriticalThreshold = 0.1f;
	DefaultTemperatureStressThreshold = 10.0f; // 10 degrees C
	DefaultNutrientDeficiencyThreshold = 0.5f;
	DefaultMaxHealthRecoveryRate = 0.05f; // 5% per second
	MinimumViableHealth = 0.05f;
}

FHealthCalculationResult UCropHealthSystem::CalculateCropHealth(
	const UCropDefinition* CropDef,
	const FEnvironmentalFactors& Environment,
	float CurrentHealth,
	float WaterLevel,
	float SoilFertility,
	float TimeSinceLastWater,
	float DeltaTime)
{
	FHealthCalculationResult Result;
	Result.CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, 1.0f);

	if (!CropDef)
	{
		return Result;
	}

	// Calculate degradation factors
	Result.DegradationFactors = CalculateDegradationFactors(
		CropDef,
		Environment,
		WaterLevel,
		SoilFertility,
		TimeSinceLastWater
	);

	// Calculate health change
	float HealthDelta = 0.0f;

	// Water deficit impact
	if (Result.DegradationFactors.WaterDeficit > 0.0f)
	{
		// Health degrades when water is low
		float WaterDamage = Result.DegradationFactors.WaterDeficit * DefaultHealthDegradationRate * DeltaTime / 60.0f;
		HealthDelta -= WaterDamage;
	}
	else if (WaterLevel > DefaultWaterWarningThreshold && CurrentHealth < 1.0f)
	{
		// Health recovers when water is adequate
		float Recovery = DefaultMaxHealthRecoveryRate * DeltaTime;
		HealthDelta += Recovery;
	}

	// Temperature stress impact
	if (Result.DegradationFactors.TemperatureStress > 0.5f)
	{
		float TempDamage = Result.DegradationFactors.TemperatureStress * 0.02f * DeltaTime;
		HealthDelta -= TempDamage;
	}

	// Nutrient deficiency impact
	if (Result.DegradationFactors.NutrientDeficiency > 0.5f)
	{
		float NutrientDamage = Result.DegradationFactors.NutrientDeficiency * 0.01f * DeltaTime;
		HealthDelta -= NutrientDamage;
	}

	// Apply health change
	Result.HealthChange = HealthDelta;
	Result.CurrentHealth = FMath::Clamp(Result.CurrentHealth + HealthDelta, 0.0f, 1.0f);

	// Determine health status
	Result.HealthStatus = GetHealthStatus(Result.CurrentHealth);

	// Generate visual indicators
	Result.VisualIndicators = GenerateVisualIndicators(
		Result.CurrentHealth,
		Result.DegradationFactors
	);

	// Determine needs
	Result.bNeedsWater = NeedsWater(WaterLevel, CropDef->WaterConsumption, TimeSinceLastWater);
	Result.bNeedsFertilizer = Result.DegradationFactors.NutrientDeficiency > DefaultNutrientDeficiencyThreshold;
	Result.bNeedsPestControl = Result.DegradationFactors.PestDamage > 0.3f;

	// Determine primary issue
	Result.PrimaryIssue = DeterminePrimaryIssue(Result.DegradationFactors);

	return Result;
}

float UCropHealthSystem::UpdateHealthDegradation(
	float CurrentHealth,
	const FHealthDegradationFactors& Factors,
	float DeltaTime)
{
	float HealthDelta = 0.0f;

	// Sum all degradation factors
	float TotalDegradation = 
		Factors.WaterDeficit * 0.5f +
		Factors.TemperatureStress * 0.2f +
		Factors.NutrientDeficiency * 0.15f +
		Factors.PestDamage * 0.1f +
		Factors.DiseaseLevel * 0.05f;

	// Apply degradation
	if (TotalDegradation > 0.0f)
	{
		HealthDelta = -TotalDegradation * DefaultHealthDegradationRate * DeltaTime / 60.0f;
	}

	return FMath::Clamp(CurrentHealth + HealthDelta, 0.0f, 1.0f);
}

bool UCropHealthSystem::NeedsWater(
	float WaterLevel,
	float WaterConsumption,
	float TimeSinceLastWater)
{
	// Check if water level is below warning threshold
	if (WaterLevel < DefaultWaterWarningThreshold)
	{
		return true;
	}

	// Check if it's been too long since last watering
	if (TimeSinceLastWater > DefaultWaterGracePeriod)
	{
		return true;
	}

	// Check if water consumption rate suggests imminent need
	float TimeUntilEmpty = GetTimeUntilWaterNeeded(WaterLevel, WaterConsumption);
	if (TimeUntilEmpty < 60.0f) // Less than 1 minute
	{
		return true;
	}

	return false;
}

float UCropHealthSystem::CalculateWaterDeficit(
	float WaterLevel,
	float WaterConsumption,
	float TimeSinceLastWater)
{
	float Deficit = 0.0f;

	// Water level deficit
	if (WaterLevel < DefaultWaterWarningThreshold)
	{
		float LevelDeficit = 1.0f - (WaterLevel / DefaultWaterWarningThreshold);
		Deficit = FMath::Max(Deficit, LevelDeficit);
	}

	// Time-based deficit
	if (TimeSinceLastWater > DefaultWaterGracePeriod)
	{
		float TimeDeficit = FMath::Min(
			(TimeSinceLastWater - DefaultWaterGracePeriod) / DefaultWaterGracePeriod,
			1.0f
		);
		Deficit = FMath::Max(Deficit, TimeDeficit);
	}

	return FMath::Clamp(Deficit, 0.0f, 1.0f);
}

float UCropHealthSystem::GetTimeUntilWaterNeeded(
	float WaterLevel,
	float WaterConsumption)
{
	if (WaterConsumption <= 0.0f)
	{
		return FLT_MAX; // Infinite time if no consumption
	}

	// Calculate time until water level reaches warning threshold
	float WaterRemaining = FMath::Max(0.0f, WaterLevel - DefaultWaterWarningThreshold);
	float ConsumptionPerSecond = WaterConsumption / 86400.0f; // Convert from per day to per second

	if (ConsumptionPerSecond <= 0.0f)
	{
		return FLT_MAX;
	}

	return WaterRemaining / ConsumptionPerSecond;
}

FHealthVisualIndicators UCropHealthSystem::GenerateVisualIndicators(
	float Health,
	const FHealthDegradationFactors& Factors)
{
	FHealthVisualIndicators Indicators;

	// Calculate leaf color
	Indicators.LeafColor = CalculateLeafColor(
		Health,
		Factors.NutrientDeficiency,
		Factors.WaterDeficit
	);

	// Calculate wilting
	Indicators.WiltingAmount = CalculateWiltingAmount(Health, Factors.WaterDeficit);

	// Calculate scale modifier
	Indicators.ScaleModifier = CalculateScaleModifier(Health);

	// Determine visual flags
	Indicators.bShowWilting = Factors.WaterDeficit > 0.3f;
	Indicators.bShowYellowing = Factors.NutrientDeficiency > 0.4f;
	Indicators.bShowBrownSpots = Factors.PestDamage > 0.3f || Factors.DiseaseLevel > 0.3f;

	return Indicators;
}

FLinearColor UCropHealthSystem::CalculateLeafColor(
	float Health,
	float NutrientDeficiency,
	float WaterDeficit)
{
	// Start with healthy green
	FLinearColor BaseGreen = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);

	// Yellowing from nutrient deficiency
	if (NutrientDeficiency > 0.3f)
	{
		float YellowAmount = (NutrientDeficiency - 0.3f) / 0.7f;
		FLinearColor Yellow = FLinearColor(0.8f, 0.8f, 0.2f, 1.0f);
		BaseGreen = FMath::Lerp(BaseGreen, Yellow, YellowAmount * 0.6f);
	}

	// Browning from water stress
	if (WaterDeficit > 0.4f)
	{
		float BrownAmount = (WaterDeficit - 0.4f) / 0.6f;
		FLinearColor Brown = FLinearColor(0.4f, 0.3f, 0.1f, 1.0f);
		BaseGreen = FMath::Lerp(BaseGreen, Brown, BrownAmount * 0.5f);
	}

	// Overall health affects saturation
	float Saturation = FMath::Lerp(0.3f, 1.0f, Health);
	BaseGreen.R *= Saturation;
	BaseGreen.G *= Saturation;
	BaseGreen.B *= Saturation;

	return BaseGreen;
}

float UCropHealthSystem::CalculateWiltingAmount(
	float Health,
	float WaterDeficit)
{
	// Wilting is primarily caused by water deficit
	float WiltFromWater = WaterDeficit * 0.7f;

	// Poor health also causes wilting
	float WiltFromHealth = (1.0f - Health) * 0.3f;

	// Combine factors
	float TotalWilt = FMath::Clamp(WiltFromWater + WiltFromHealth, 0.0f, 1.0f);

	// Apply non-linear curve for more dramatic wilting at high values
	return FMath::Pow(TotalWilt, 1.5f);
}

float UCropHealthSystem::CalculateScaleModifier(float Health)
{
	// Healthy plants are full size
	// Unhealthy plants shrink slightly
	return FMath::Lerp(0.7f, 1.0f, Health);
}

FHealthDegradationFactors UCropHealthSystem::CalculateDegradationFactors(
	const UCropDefinition* CropDef,
	const FEnvironmentalFactors& Environment,
	float WaterLevel,
	float SoilFertility,
	float TimeSinceLastWater)
{
	FHealthDegradationFactors Factors;

	if (!CropDef)
	{
		return Factors;
	}

	// Water deficit
	Factors.WaterDeficit = CalculateWaterDeficit(
		WaterLevel,
		CropDef->WaterConsumption,
		TimeSinceLastWater
	);

	// Temperature stress
	Factors.TemperatureStress = CalculateTemperatureStress(
		Environment.Temperature,
		CropDef->OptimalTemperature.Min,
		CropDef->OptimalTemperature.Max
	);

	// Nutrient deficiency
	Factors.NutrientDeficiency = CalculateNutrientDeficiency(
		SoilFertility,
		CropDef->SoilQualityRequirement
	);

	// Pest damage and disease are not yet implemented
	Factors.PestDamage = 0.0f;
	Factors.DiseaseLevel = 0.0f;

	// Age factor (not yet implemented)
	Factors.Age = 0.0f;

	return Factors;
}

float UCropHealthSystem::CalculateTemperatureStress(
	float CurrentTemp,
	float OptimalMin,
	float OptimalMax)
{
	// No stress if within optimal range
	if (CurrentTemp >= OptimalMin && CurrentTemp <= OptimalMax)
	{
		return 0.0f;
	}

	// Calculate how far outside optimal range
	float Deviation = 0.0f;
	if (CurrentTemp < OptimalMin)
	{
		Deviation = OptimalMin - CurrentTemp;
	}
	else
	{
		Deviation = CurrentTemp - OptimalMax;
	}

	// Normalize by stress threshold
	float Stress = Deviation / DefaultTemperatureStressThreshold;

	return FMath::Clamp(Stress, 0.0f, 1.0f);
}

float UCropHealthSystem::CalculateNutrientDeficiency(
	float SoilFertility,
	float RequiredQuality)
{
	// No deficiency if soil fertility meets requirements
	if (SoilFertility >= RequiredQuality)
	{
		return 0.0f;
	}

	// Calculate deficiency as percentage below requirement
	float Deficiency = (RequiredQuality - SoilFertility) / RequiredQuality;

	return FMath::Clamp(Deficiency, 0.0f, 1.0f);
}

ECropHealthStatus UCropHealthSystem::GetHealthStatus(float Health)
{
	if (Health >= 0.9f)
	{
		return ECropHealthStatus::Excellent;
	}
	else if (Health >= 0.7f)
	{
		return ECropHealthStatus::Good;
	}
	else if (Health >= 0.5f)
	{
		return ECropHealthStatus::Fair;
	}
	else if (Health >= 0.3f)
	{
		return ECropHealthStatus::Poor;
	}
	else if (Health >= 0.1f)
	{
		return ECropHealthStatus::Critical;
	}
	else
	{
		return ECropHealthStatus::Dying;
	}
}

FString UCropHealthSystem::GetHealthStatusDescription(ECropHealthStatus Status)
{
	switch (Status)
	{
	case ECropHealthStatus::Excellent:
		return TEXT("Excellent - Thriving");
	case ECropHealthStatus::Good:
		return TEXT("Good - Healthy");
	case ECropHealthStatus::Fair:
		return TEXT("Fair - Adequate");
	case ECropHealthStatus::Poor:
		return TEXT("Poor - Struggling");
	case ECropHealthStatus::Critical:
		return TEXT("Critical - Needs Attention");
	case ECropHealthStatus::Dying:
		return TEXT("Dying - Immediate Action Required");
	default:
		return TEXT("Unknown");
	}
}

FLinearColor UCropHealthSystem::GetHealthStatusColor(ECropHealthStatus Status)
{
	switch (Status)
	{
	case ECropHealthStatus::Excellent:
		return FLinearColor::Green;
	case ECropHealthStatus::Good:
		return FLinearColor(0.5f, 1.0f, 0.0f, 1.0f); // Light green
	case ECropHealthStatus::Fair:
		return FLinearColor::Yellow;
	case ECropHealthStatus::Poor:
		return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
	case ECropHealthStatus::Critical:
		return FLinearColor(1.0f, 0.2f, 0.0f, 1.0f); // Red-orange
	case ECropHealthStatus::Dying:
		return FLinearColor::Red;
	default:
		return FLinearColor::Gray;
	}
}

FString UCropHealthSystem::DeterminePrimaryIssue(const FHealthDegradationFactors& Factors)
{
	// Find the most severe factor
	float MaxFactor = 0.0f;
	FString Issue = TEXT("None");

	if (Factors.WaterDeficit > MaxFactor)
	{
		MaxFactor = Factors.WaterDeficit;
		Issue = TEXT("Water Deficit");
	}

	if (Factors.TemperatureStress > MaxFactor)
	{
		MaxFactor = Factors.TemperatureStress;
		Issue = TEXT("Temperature Stress");
	}

	if (Factors.NutrientDeficiency > MaxFactor)
	{
		MaxFactor = Factors.NutrientDeficiency;
		Issue = TEXT("Nutrient Deficiency");
	}

	if (Factors.PestDamage > MaxFactor)
	{
		MaxFactor = Factors.PestDamage;
		Issue = TEXT("Pest Damage");
	}

	if (Factors.DiseaseLevel > MaxFactor)
	{
		MaxFactor = Factors.DiseaseLevel;
		Issue = TEXT("Disease");
	}

	return Issue;
}

