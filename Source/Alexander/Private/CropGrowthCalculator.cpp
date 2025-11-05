// Copyright Epic Games, Inc. All Rights Reserved.

#include "CropGrowthCalculator.h"
#include "CropDefinition.h"

// Define static default configuration values
float UCropGrowthCalculator::DefaultTemperatureTolerance = 10.0f;
float UCropGrowthCalculator::DefaultTemperaturePenaltyRate = 0.05f;
float UCropGrowthCalculator::DefaultHumidityTolerance = 0.2f;
float UCropGrowthCalculator::DefaultSunlightBonusMax = 1.2f;
float UCropGrowthCalculator::DefaultMinimumGrowthRate = 0.1f;
float UCropGrowthCalculator::DefaultMaximumGrowthRate = 2.0f;

UCropGrowthCalculator::UCropGrowthCalculator()
{
	// Initialize configuration values with defaults
	DefaultTemperatureTolerance = 10.0f;
	TemperaturePenaltyRate = 0.05f;
	HumidityTolerance = 0.2f;
	DefaultSunlightBonusMax = 1.2f;
	MinimumGrowthRate = 0.1f;
	MaximumGrowthRate = 2.0f;
}

FGrowthCalculationResult UCropGrowthCalculator::CalculateGrowthRate(
	const UCropDefinition* CropDef,
	const FEnvironmentalFactors& Environment,
	float CropHealth,
	float SeasonalModifier)
{
	FGrowthCalculationResult Result;

	if (!CropDef)
	{
		UE_LOG(LogTemp, Warning, TEXT("CropGrowthCalculator: Null crop definition"));
		Result.TotalGrowthModifier = 0.0f;
		Result.LimitingFactor = TEXT("Invalid Crop");
		return Result;
	}

	// Calculate individual factor modifiers
	Result.TemperatureModifier = CalculateTemperatureModifier(
		Environment.Temperature,
		CropDef->OptimalTemperature.Min,
		CropDef->OptimalTemperature.Max
	);

	Result.HumidityModifier = CalculateHumidityModifier(
		Environment.Humidity,
		CropDef->OptimalHumidity.Min,
		CropDef->OptimalHumidity.Max
	);

	Result.SunlightModifier = CalculateSunlightModifier(
		Environment.Sunlight,
		CropDef->SunlightRequirement
	);

	Result.SoilQualityModifier = CalculateSoilQualityModifier(
		Environment.SoilQuality,
		CropDef->SoilQualityRequirement
	);

	Result.WaterModifier = CalculateWaterModifier(
		Environment.WaterLevel,
		CropDef->WaterConsumption
	);

	Result.FertilityModifier = CalculateFertilityModifier(Environment.Fertility);

	Result.HealthModifier = FMath::Clamp(CropHealth, 0.0f, 1.0f);

	Result.SeasonalModifier = FMath::Clamp(SeasonalModifier, 0.5f, 1.5f);

	// Calculate total growth modifier (multiplicative)
	Result.TotalGrowthModifier = 
		Result.TemperatureModifier *
		Result.HumidityModifier *
		Result.SunlightModifier *
		Result.SoilQualityModifier *
		Result.WaterModifier *
		Result.FertilityModifier *
		Result.HealthModifier *
		Result.SeasonalModifier;

	// Clamp to configured min/max
	Result.TotalGrowthModifier = FMath::Clamp(
		Result.TotalGrowthModifier,
		DefaultMinimumGrowthRate,
		DefaultMaximumGrowthRate
	);

	// Determine limiting factor
	Result.LimitingFactor = DetermineLimitingFactor(Result);

	return Result;
}

float UCropGrowthCalculator::CalculateTemperatureModifier(
	float CurrentTemp,
	float OptimalMin,
	float OptimalMax)
{
	// Within optimal range - excellent growth
	if (CurrentTemp >= OptimalMin && CurrentTemp <= OptimalMax)
	{
		// Slight bonus for being in the middle of optimal range
		float OptimalMid = (OptimalMin + OptimalMax) * 0.5f;
		float RangeWidth = OptimalMax - OptimalMin;
		float DeviationFromMid = FMath::Abs(CurrentTemp - OptimalMid);
		float CenterBonus = 1.0f + (0.1f * (1.0f - (DeviationFromMid / (RangeWidth * 0.5f))));
		return FMath::Clamp(CenterBonus, 1.0f, 1.1f);
	}

	// Outside optimal range - calculate penalty
	float Deviation;
	if (CurrentTemp < OptimalMin)
	{
		Deviation = OptimalMin - CurrentTemp;
	}
	else
	{
		Deviation = CurrentTemp - OptimalMax;
	}

	// Apply exponential falloff for temperature deviation
	// More severe penalty as temperature gets further from optimal
	float Penalty = FMath::Exp(-Deviation / DefaultTemperatureTolerance);
	
	// Ensure minimum growth rate
	return FMath::Max(0.1f, Penalty);
}

float UCropGrowthCalculator::CalculateHumidityModifier(
	float CurrentHumidity,
	float OptimalMin,
	float OptimalMax)
{
	// Clamp humidity to valid range
	CurrentHumidity = FMath::Clamp(CurrentHumidity, 0.0f, 1.0f);

	// Within optimal range - excellent growth
	if (CurrentHumidity >= OptimalMin && CurrentHumidity <= OptimalMax)
	{
		// Slight bonus for optimal humidity
		float OptimalMid = (OptimalMin + OptimalMax) * 0.5f;
		float RangeWidth = OptimalMax - OptimalMin;
		float DeviationFromMid = FMath::Abs(CurrentHumidity - OptimalMid);
		float CenterBonus = 1.0f + (0.15f * (1.0f - (DeviationFromMid / (RangeWidth * 0.5f))));
		return FMath::Clamp(CenterBonus, 1.0f, 1.15f);
	}

	// Outside optimal range - calculate penalty
	float Deviation;
	if (CurrentHumidity < OptimalMin)
	{
		Deviation = OptimalMin - CurrentHumidity;
		// Drought stress is more severe
		return FMath::Max(0.2f, 1.0f - (Deviation * 1.5f));
	}
	else
	{
		Deviation = CurrentHumidity - OptimalMax;
		// Excess humidity is less severe than drought
		return FMath::Max(0.4f, 1.0f - Deviation);
	}
}

float UCropGrowthCalculator::CalculateSunlightModifier(
	float CurrentSunlight,
	float RequiredSunlight)
{
	// Clamp values to valid range
	CurrentSunlight = FMath::Clamp(CurrentSunlight, 0.0f, 1.0f);
	RequiredSunlight = FMath::Clamp(RequiredSunlight, 0.0f, 1.0f);

	// Handle shade-tolerant crops (low sunlight requirement)
	if (RequiredSunlight < 0.3f)
	{
		// These crops can handle low light well
		if (CurrentSunlight < RequiredSunlight)
		{
			return FMath::Max(0.5f, CurrentSunlight / RequiredSunlight);
		}
		// But too much light can stress them
		else if (CurrentSunlight > 0.7f)
		{
			return FMath::Max(0.7f, 1.0f - ((CurrentSunlight - 0.7f) * 0.5f));
		}
		return 1.0f;
	}

	// Handle sun-loving crops (high sunlight requirement)
	if (RequiredSunlight > 0.7f)
	{
		// These crops need lots of light
		if (CurrentSunlight < RequiredSunlight)
		{
			// Severe penalty for insufficient light
			float Ratio = CurrentSunlight / RequiredSunlight;
			return FMath::Max(0.2f, Ratio * Ratio); // Quadratic penalty
		}
		// Bonus for excess light
		else
		{
			float Bonus = 1.0f + ((CurrentSunlight - RequiredSunlight) * 0.5f);
			return FMath::Min(DefaultSunlightBonusMax, Bonus);
		}
	}

	// Handle moderate sunlight crops
	if (CurrentSunlight < RequiredSunlight)
	{
		// Linear penalty for insufficient light
		float Ratio = CurrentSunlight / RequiredSunlight;
		return FMath::Max(0.3f, Ratio);
	}
	else
	{
		// Small bonus for extra light
		float Bonus = 1.0f + ((CurrentSunlight - RequiredSunlight) * 0.3f);
		return FMath::Min(1.15f, Bonus);
	}
}

float UCropGrowthCalculator::CalculateSoilQualityModifier(
	float CurrentQuality,
	float RequiredQuality)
{
	// Clamp values to valid range
	CurrentQuality = FMath::Clamp(CurrentQuality, 0.0f, 1.0f);
	RequiredQuality = FMath::Clamp(RequiredQuality, 0.0f, 1.0f);

	// Soil quality below requirement - penalty
	if (CurrentQuality < RequiredQuality)
	{
		float Ratio = CurrentQuality / FMath::Max(0.1f, RequiredQuality);
		// Quadratic penalty for poor soil
		return FMath::Max(0.3f, Ratio * Ratio);
	}

	// Soil quality meets or exceeds requirement
	// Diminishing returns for excess quality
	float Excess = CurrentQuality - RequiredQuality;
	float Bonus = 1.0f + (Excess * 0.2f);
	return FMath::Min(1.2f, Bonus);
}

float UCropGrowthCalculator::CalculateWaterModifier(
	float WaterLevel,
	float WaterConsumption)
{
	// Clamp water level to valid range
	WaterLevel = FMath::Clamp(WaterLevel, 0.0f, 1.0f);

	// Critical water shortage
	if (WaterLevel < 0.2f)
	{
		// Severe penalty - crops are dying
		return FMath::Max(0.1f, WaterLevel * 2.0f); // 0.1 to 0.4 range
	}

	// Low water
	if (WaterLevel < 0.4f)
	{
		// Moderate penalty - crops are stressed
		return FMath::Lerp(0.4f, 0.7f, (WaterLevel - 0.2f) / 0.2f);
	}

	// Adequate water
	if (WaterLevel < 0.6f)
	{
		// Slight penalty - not quite optimal
		return FMath::Lerp(0.7f, 0.95f, (WaterLevel - 0.4f) / 0.2f);
	}

	// Optimal water
	if (WaterLevel < 0.8f)
	{
		// Perfect conditions
		return 1.0f;
	}

	// Excess water (waterlogged)
	// Too much water can reduce oxygen to roots
	float Excess = WaterLevel - 0.8f;
	return FMath::Max(0.8f, 1.0f - (Excess * 0.5f));
}

float UCropGrowthCalculator::CalculateFertilityModifier(float Fertility)
{
	// Clamp fertility to valid range
	Fertility = FMath::Clamp(Fertility, 0.0f, 1.0f);

	// Fertility has a strong impact on growth
	// Use a curve that penalizes low fertility more than it rewards high fertility
	if (Fertility < 0.5f)
	{
		// Below average fertility - increasing penalty
		return FMath::Max(0.3f, Fertility * 1.4f); // 0.3 to 0.7 range
	}
	else
	{
		// Above average fertility - diminishing returns
		float Excess = Fertility - 0.5f;
		return FMath::Lerp(0.7f, 1.0f, Excess * 2.0f);
	}
}

FString UCropGrowthCalculator::DetermineLimitingFactor(const FGrowthCalculationResult& Result)
{
	// Find the factor with the lowest modifier
	float MinModifier = Result.TotalGrowthModifier;
	FString LimitingFactor = TEXT("None");

	struct FFactorCheck
	{
		float Value;
		FString Name;
	};

	TArray<FFactorCheck> Factors = {
		{ Result.TemperatureModifier, TEXT("Temperature") },
		{ Result.HumidityModifier, TEXT("Humidity") },
		{ Result.SunlightModifier, TEXT("Sunlight") },
		{ Result.SoilQualityModifier, TEXT("Soil Quality") },
		{ Result.WaterModifier, TEXT("Water") },
		{ Result.FertilityModifier, TEXT("Fertility") },
		{ Result.HealthModifier, TEXT("Health") }
	};

	for (const FFactorCheck& Factor : Factors)
	{
		if (Factor.Value < MinModifier)
		{
			MinModifier = Factor.Value;
			LimitingFactor = Factor.Name;
		}
	}

	// If all factors are good, no limiting factor
	if (MinModifier > 0.9f)
	{
		LimitingFactor = TEXT("None - Optimal Conditions");
	}

	return LimitingFactor;
}

FString UCropGrowthCalculator::GetGrowthStageDescription(float GrowthProgress)
{
	GrowthProgress = FMath::Clamp(GrowthProgress, 0.0f, 1.0f);

	if (GrowthProgress < 0.15f)
	{
		return TEXT("Seedling");
	}
	else if (GrowthProgress < 0.35f)
	{
		return TEXT("Early Growth");
	}
	else if (GrowthProgress < 0.60f)
	{
		return TEXT("Vegetative");
	}
	else if (GrowthProgress < 0.85f)
	{
		return TEXT("Flowering");
	}
	else if (GrowthProgress < 1.0f)
	{
		return TEXT("Ripening");
	}
	else
	{
		return TEXT("Mature");
	}
}

float UCropGrowthCalculator::CalculateTimeToMaturity(
	const UCropDefinition* CropDef,
	const FEnvironmentalFactors& Environment,
	float CurrentProgress)
{
	if (!CropDef)
	{
		return 0.0f;
	}

	// Clamp progress
	CurrentProgress = FMath::Clamp(CurrentProgress, 0.0f, 1.0f);

	// Already mature
	if (CurrentProgress >= 1.0f)
	{
		return 0.0f;
	}

	// Calculate current growth rate
	FGrowthCalculationResult GrowthResult = CalculateGrowthRate(CropDef, Environment, 1.0f);

	// Remaining progress
	float RemainingProgress = 1.0f - CurrentProgress;

	// Base growth rate (progress per second)
	float BaseGrowthRate = 1.0f / CropDef->GrowthDuration;

	// Actual growth rate with modifiers
	float ActualGrowthRate = BaseGrowthRate * GrowthResult.TotalGrowthModifier;

	// Avoid division by zero
	if (ActualGrowthRate <= 0.0f)
	{
		return 999999.0f; // Effectively infinite time
	}

	// Calculate time to maturity
	float TimeToMaturity = RemainingProgress / ActualGrowthRate;

	return TimeToMaturity;
}

float UCropGrowthCalculator::ApplySmoothFalloff(float Value, float Optimal, float Tolerance)
{
	float Deviation = FMath::Abs(Value - Optimal);
	if (Deviation <= Tolerance)
	{
		return 1.0f;
	}

	// Smooth exponential falloff beyond tolerance
	float ExcessDeviation = Deviation - Tolerance;
	return FMath::Exp(-ExcessDeviation / Tolerance);
}

float UCropGrowthCalculator::GaussianCurve(float Value, float Mean, float StdDev)
{
	float Exponent = -FMath::Square(Value - Mean) / (2.0f * FMath::Square(StdDev));
	return FMath::Exp(Exponent);
}
