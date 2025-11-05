// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnvironmentalModifierSystem.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "CropDefinition.h"

UEnvironmentalModifierSystem::UEnvironmentalModifierSystem()
{
}

void UEnvironmentalModifierSystem::Initialize()
{
	SetupDefaultBiomeChallenges();
}

// ============================================================================
// MAIN CALCULATION
// ============================================================================

FEnvironmentalModifierResult UEnvironmentalModifierSystem::CalculateEnvironmentalModifiers(
	const UCropDefinition* CropDef,
	EBiomeType BiomeType,
	float Temperature,
	float Humidity,
	bool bIsNativeCrop) const
{
	FEnvironmentalModifierResult Result;

	if (!CropDef)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnvironmentalModifierSystem: Null crop definition"));
		Result.ModifierSummary = TEXT("Invalid crop definition");
		return Result;
	}

	// Calculate water consumption multiplier based on humidity
	Result.WaterConsumptionMultiplier = CalculateWaterConsumptionMultiplier(
		Humidity,
		CropDef->OptimalHumidity.Min,
		CropDef->OptimalHumidity.Max
	);

	// Calculate temperature growth modifier
	Result.TemperatureGrowthModifier = CalculateTemperatureGrowthModifier(
		Temperature,
		CropDef->OptimalTemperature.Min,
		CropDef->OptimalTemperature.Max
	);

	// Calculate biome compatibility modifier
	Result.BiomeCompatibilityModifier = CalculateBiomeCompatibilityModifier(CropDef, BiomeType);

	// Calculate native crop bonus
	Result.NativeCropBonus = CalculateNativeCropBonus(bIsNativeCrop, BiomeType);

	// Get active challenges
	Result.ActiveChallenges = GetActiveChallenges(BiomeType, Temperature, Humidity, 0.7f);

	// Calculate challenge modifier
	Result.ChallengeModifier = CalculateChallengeModifier(Result.ActiveChallenges, CropDef);

	// Calculate total growth modifier (multiplicative)
	Result.TotalGrowthModifier = 
		Result.TemperatureGrowthModifier *
		Result.BiomeCompatibilityModifier *
		Result.NativeCropBonus *
		Result.ChallengeModifier;

	// Clamp to reasonable range
	Result.TotalGrowthModifier = FMath::Clamp(Result.TotalGrowthModifier, 0.1f, 2.0f);

	// Generate summary
	Result.ModifierSummary = GenerateModifierSummary(Result);

	return Result;
}

// ============================================================================
// WATER CONSUMPTION
// ============================================================================

float UEnvironmentalModifierSystem::CalculateWaterConsumptionMultiplier(
	float Humidity,
	float OptimalHumidityMin,
	float OptimalHumidityMax)
{
	// Clamp humidity to valid range
	Humidity = FMath::Clamp(Humidity, 0.0f, 1.0f);

	// Calculate optimal humidity midpoint
	float OptimalMid = (OptimalHumidityMin + OptimalHumidityMax) * 0.5f;

	// Within optimal range - normal water consumption
	if (Humidity >= OptimalHumidityMin && Humidity <= OptimalHumidityMax)
	{
		// Slight reduction in water needs at optimal humidity
		return FMath::Lerp(0.9f, 1.0f, FMath::Abs(Humidity - OptimalMid) / (OptimalHumidityMax - OptimalHumidityMin));
	}

	// Below optimal humidity - increased water consumption
	if (Humidity < OptimalHumidityMin)
	{
		float Deficit = OptimalHumidityMin - Humidity;
		
		// Exponential increase in water consumption as humidity drops
		// At 0% humidity, water consumption can reach 300%
		float Multiplier = 1.0f + (Deficit * 2.5f);
		
		// Apply exponential curve for severe drought conditions
		if (Humidity < 0.2f)
		{
			Multiplier *= 1.0f + (0.2f - Humidity) * 2.0f;
		}
		
		return FMath::Clamp(Multiplier, 1.0f, 3.0f);
	}

	// Above optimal humidity - reduced water consumption (plants get moisture from air)
	float Excess = Humidity - OptimalHumidityMax;
	float Multiplier = 1.0f - (Excess * 0.5f);
	return FMath::Clamp(Multiplier, 0.5f, 1.0f);
}

float UEnvironmentalModifierSystem::CalculateAdjustedWaterConsumption(
	float BaseWaterConsumption,
	float Humidity,
	float OptimalHumidityMin,
	float OptimalHumidityMax)
{
	float Multiplier = CalculateWaterConsumptionMultiplier(Humidity, OptimalHumidityMin, OptimalHumidityMax);
	return BaseWaterConsumption * Multiplier;
}

// ============================================================================
// TEMPERATURE MODIFIERS
// ============================================================================

float UEnvironmentalModifierSystem::CalculateTemperatureGrowthModifier(
	float Temperature,
	float OptimalTempMin,
	float OptimalTempMax)
{
	// Calculate optimal temperature midpoint
	float OptimalMid = (OptimalTempMin + OptimalTempMax) * 0.5f;
	float OptimalRange = OptimalTempMax - OptimalTempMin;

	// Within optimal range - excellent growth
	if (Temperature >= OptimalTempMin && Temperature <= OptimalTempMax)
	{
		// Bonus for being near the center of optimal range
		float DeviationFromCenter = FMath::Abs(Temperature - OptimalMid);
		float CenterFactor = 1.0f - (DeviationFromCenter / (OptimalRange * 0.5f));
		
		// Up to 20% bonus at optimal center
		return FMath::Lerp(1.0f, 1.2f, CenterFactor * 0.5f);
	}

	// Outside optimal range - calculate penalty
	float Deviation;
	if (Temperature < OptimalTempMin)
	{
		Deviation = OptimalTempMin - Temperature;
		
		// Cold temperatures have severe impact
		// Exponential penalty below optimal
		float ColdPenalty = FMath::Exp(-Deviation / 10.0f);
		
		// Extreme cold (below 0°C) is very harsh
		if (Temperature < 0.0f)
		{
			ColdPenalty *= 0.5f;
		}
		
		return FMath::Max(0.3f, ColdPenalty);
	}
	else
	{
		Deviation = Temperature - OptimalTempMax;
		
		// Heat stress
		// Linear penalty initially, then exponential for extreme heat
		float HeatPenalty = 1.0f - (Deviation * 0.05f);
		
		// Extreme heat (above 40°C) is very harsh
		if (Temperature > 40.0f)
		{
			float ExtremeHeat = Temperature - 40.0f;
			HeatPenalty *= FMath::Exp(-ExtremeHeat / 5.0f);
		}
		
		return FMath::Max(0.3f, HeatPenalty);
	}
}

bool UEnvironmentalModifierSystem::IsExtremeTemperature(float Temperature)
{
	return Temperature < 0.0f || Temperature > 40.0f;
}

// ============================================================================
// BIOME CHALLENGES
// ============================================================================

TArray<EEnvironmentalChallenge> UEnvironmentalModifierSystem::GetActiveChallenges(
	EBiomeType BiomeType,
	float Temperature,
	float Humidity,
	float SoilQuality) const
{
	TArray<EEnvironmentalChallenge> ActiveChallenges;

	// Get biome-specific challenges
	const FBiomeChallengeConfig* Config = GetBiomeChallengeConfig(BiomeType);
	if (Config)
	{
		// Add common biome challenges based on frequency
		for (EEnvironmentalChallenge Challenge : Config->CommonChallenges)
		{
			// Random chance based on frequency
			float RandomValue = FMath::FRand();
			if (RandomValue < Config->ChallengeFrequency)
			{
				ActiveChallenges.Add(Challenge);
			}
		}
	}

	// Add condition-based challenges
	
	// Drought challenge in low humidity
	if (Humidity < 0.3f)
	{
		ActiveChallenges.AddUnique(EEnvironmentalChallenge::Drought);
	}

	// Flood challenge in high humidity
	if (Humidity > 0.8f)
	{
		ActiveChallenges.AddUnique(EEnvironmentalChallenge::Flood);
	}

	// Heat wave in high temperature
	if (Temperature > 35.0f)
	{
		ActiveChallenges.AddUnique(EEnvironmentalChallenge::HeatWave);
	}

	// Cold snap in low temperature
	if (Temperature < 5.0f)
	{
		ActiveChallenges.AddUnique(EEnvironmentalChallenge::ColdSnap);
	}

	// Poor soil challenge
	if (SoilQuality < 0.4f)
	{
		ActiveChallenges.AddUnique(EEnvironmentalChallenge::PoorSoil);
	}

	return ActiveChallenges;
}

float UEnvironmentalModifierSystem::CalculateChallengeModifier(
	const TArray<EEnvironmentalChallenge>& Challenges,
	const UCropDefinition* CropDef) const
{
	if (Challenges.Num() == 0)
	{
		return 1.0f; // No challenges, no penalty
	}

	float Modifier = 1.0f;

	for (EEnvironmentalChallenge Challenge : Challenges)
	{
		float ChallengePenalty = 0.2f; // Base 20% penalty per challenge

		// Check crop resistances
		if (CropDef)
		{
			switch (Challenge)
			{
			case EEnvironmentalChallenge::Drought:
				if (CropDef->bResistantToDrought)
				{
					ChallengePenalty *= 0.5f; // Halve penalty if resistant
				}
				break;

			case EEnvironmentalChallenge::Pests:
			case EEnvironmentalChallenge::Disease:
				if (CropDef->bResistantToPests)
				{
					ChallengePenalty *= 0.5f;
				}
				break;

			case EEnvironmentalChallenge::PoorSoil:
				// Crops with low soil requirements are less affected
				if (CropDef->SoilQualityRequirement < 0.4f)
				{
					ChallengePenalty *= 0.7f;
				}
				break;

			default:
				break;
			}
		}

		// Apply penalty
		Modifier *= (1.0f - ChallengePenalty);
	}

	// Clamp to reasonable range
	return FMath::Clamp(Modifier, 0.5f, 1.0f);
}

TArray<EEnvironmentalChallenge> UEnvironmentalModifierSystem::GetBiomeChallenges(EBiomeType BiomeType) const
{
	const FBiomeChallengeConfig* Config = GetBiomeChallengeConfig(BiomeType);
	if (Config)
	{
		return Config->CommonChallenges;
	}

	return TArray<EEnvironmentalChallenge>();
}

// ============================================================================
// NATIVE CROP BONUSES
// ============================================================================

float UEnvironmentalModifierSystem::CalculateNativeCropBonus(bool bIsNative, EBiomeType BiomeType)
{
	if (!bIsNative)
	{
		return 1.0f; // No bonus for non-native crops
	}

	// Native crops get bonuses based on biome
	float BonusAmount = 0.2f; // Base 20% bonus

	// Some biomes provide better native crop bonuses
	switch (BiomeType)
	{
	case EBiomeType::Grassland:
		BonusAmount = 0.25f; // 25% bonus - ideal farming conditions
		break;

	case EBiomeType::Forest:
		BonusAmount = 0.20f; // 20% bonus
		break;

	case EBiomeType::Volcanic:
		BonusAmount = 0.30f; // 30% bonus - very fertile soil
		break;

	case EBiomeType::Desert:
	case EBiomeType::Tundra:
		BonusAmount = 0.15f; // 15% bonus - harsh conditions
		break;

	default:
		BonusAmount = 0.20f;
		break;
	}

	return 1.0f + BonusAmount;
}

FString UEnvironmentalModifierSystem::GetNativeCropBenefitsDescription(EBiomeType BiomeType)
{
	switch (BiomeType)
	{
	case EBiomeType::Grassland:
		return TEXT("Native crops thrive in rich grassland soil with 25% growth bonus");

	case EBiomeType::Forest:
		return TEXT("Native crops adapted to forest shade receive 20% growth bonus");

	case EBiomeType::Desert:
		return TEXT("Native crops evolved for arid conditions receive 15% growth bonus");

	case EBiomeType::Tundra:
		return TEXT("Native crops adapted to cold climate receive 15% growth bonus");

	case EBiomeType::Volcanic:
		return TEXT("Native crops benefit from volcanic soil fertility with 30% growth bonus");

	case EBiomeType::Ocean:
		return TEXT("Native aquatic crops receive 20% growth bonus in marine environment");

	default:
		return TEXT("Native crops receive 20% growth bonus in their natural biome");
	}
}

// ============================================================================
// BIOME COMPATIBILITY
// ============================================================================

float UEnvironmentalModifierSystem::CalculateBiomeCompatibilityModifier(
	const UCropDefinition* CropDef,
	EBiomeType BiomeType) const
{
	if (!CropDef)
	{
		return 0.5f;
	}

	// Check if crop lists this biome as suitable
	if (CropDef->SuitableBiomes.Contains(BiomeType))
	{
		return 1.0f; // Perfect compatibility
	}

	// Use crop's adaptability as fallback
	float Adaptability = CropDef->BiomeAdaptability;

	// Biome-specific compatibility adjustments
	float BiomeModifier = 1.0f;

	switch (BiomeType)
	{
	case EBiomeType::Grassland:
		BiomeModifier = 1.1f; // Grassland is generally forgiving
		break;

	case EBiomeType::Desert:
	case EBiomeType::Tundra:
		BiomeModifier = 0.8f; // Harsh biomes are less forgiving
		break;

	case EBiomeType::Ocean:
		BiomeModifier = 0.6f; // Very specialized environment
		break;

	default:
		BiomeModifier = 1.0f;
		break;
	}

	float Compatibility = Adaptability * BiomeModifier;
	return FMath::Clamp(Compatibility, 0.5f, 1.2f);
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

FString UEnvironmentalModifierSystem::GenerateModifierSummary(const FEnvironmentalModifierResult& Result)
{
	TArray<FString> SummaryParts;

	// Water consumption
	if (Result.WaterConsumptionMultiplier > 1.5f)
	{
		SummaryParts.Add(FString::Printf(TEXT("High water demand (%.0f%%)"), 
			Result.WaterConsumptionMultiplier * 100.0f));
	}
	else if (Result.WaterConsumptionMultiplier < 0.8f)
	{
		SummaryParts.Add(TEXT("Reduced water needs"));
	}

	// Temperature
	if (Result.TemperatureGrowthModifier < 0.7f)
	{
		SummaryParts.Add(TEXT("Temperature stress"));
	}
	else if (Result.TemperatureGrowthModifier > 1.1f)
	{
		SummaryParts.Add(TEXT("Ideal temperature"));
	}

	// Native bonus
	if (Result.NativeCropBonus > 1.0f)
	{
		SummaryParts.Add(FString::Printf(TEXT("Native crop bonus (+%.0f%%)"), 
			(Result.NativeCropBonus - 1.0f) * 100.0f));
	}

	// Challenges
	if (Result.ActiveChallenges.Num() > 0)
	{
		for (EEnvironmentalChallenge Challenge : Result.ActiveChallenges)
		{
			SummaryParts.Add(GetChallengeDescription(Challenge));
		}
	}

	// Overall assessment
	if (Result.TotalGrowthModifier > 1.2f)
	{
		SummaryParts.Add(TEXT("Excellent growth conditions"));
	}
	else if (Result.TotalGrowthModifier < 0.7f)
	{
		SummaryParts.Add(TEXT("Poor growth conditions"));
	}

	// Combine all parts
	if (SummaryParts.Num() == 0)
	{
		return TEXT("Normal growth conditions");
	}

	FString Summary;
	for (int32 i = 0; i < SummaryParts.Num(); ++i)
	{
		Summary += SummaryParts[i];
		if (i < SummaryParts.Num() - 1)
		{
			Summary += TEXT(", ");
		}
	}

	return Summary;
}

FString UEnvironmentalModifierSystem::GetChallengeDescription(EEnvironmentalChallenge Challenge)
{
	switch (Challenge)
	{
	case EEnvironmentalChallenge::Drought:
		return TEXT("Drought conditions");

	case EEnvironmentalChallenge::Flood:
		return TEXT("Flooding risk");

	case EEnvironmentalChallenge::HeatWave:
		return TEXT("Heat wave");

	case EEnvironmentalChallenge::ColdSnap:
		return TEXT("Cold snap");

	case EEnvironmentalChallenge::HighWinds:
		return TEXT("High winds");

	case EEnvironmentalChallenge::PoorSoil:
		return TEXT("Poor soil quality");

	case EEnvironmentalChallenge::Pests:
		return TEXT("Pest infestation");

	case EEnvironmentalChallenge::Disease:
		return TEXT("Disease outbreak");

	default:
		return TEXT("Unknown challenge");
	}
}

// ============================================================================
// INTERNAL METHODS
// ============================================================================

void UEnvironmentalModifierSystem::SetupDefaultBiomeChallenges()
{
	BiomeChallengeConfigs.Empty();

	// Grassland - Generally favorable, occasional pests
	FBiomeChallengeConfig GrasslandConfig;
	GrasslandConfig.BiomeType = EBiomeType::Grassland;
	GrasslandConfig.CommonChallenges = {
		EEnvironmentalChallenge::Pests,
		EEnvironmentalChallenge::Disease
	};
	GrasslandConfig.ChallengeSeverity = 0.4f;
	GrasslandConfig.ChallengeFrequency = 0.3f;
	BiomeChallengeConfigs.Add(GrasslandConfig);

	// Forest - Shade, pests, disease
	FBiomeChallengeConfig ForestConfig;
	ForestConfig.BiomeType = EBiomeType::Forest;
	ForestConfig.CommonChallenges = {
		EEnvironmentalChallenge::Pests,
		EEnvironmentalChallenge::Disease,
		EEnvironmentalChallenge::PoorSoil
	};
	ForestConfig.ChallengeSeverity = 0.5f;
	ForestConfig.ChallengeFrequency = 0.4f;
	BiomeChallengeConfigs.Add(ForestConfig);

	// Desert - Drought, heat, poor soil
	FBiomeChallengeConfig DesertConfig;
	DesertConfig.BiomeType = EBiomeType::Desert;
	DesertConfig.CommonChallenges = {
		EEnvironmentalChallenge::Drought,
		EEnvironmentalChallenge::HeatWave,
		EEnvironmentalChallenge::PoorSoil,
		EEnvironmentalChallenge::HighWinds
	};
	DesertConfig.ChallengeSeverity = 0.7f;
	DesertConfig.ChallengeFrequency = 0.6f;
	BiomeChallengeConfigs.Add(DesertConfig);

	// Tundra - Cold, poor soil, short growing season
	FBiomeChallengeConfig TundraConfig;
	TundraConfig.BiomeType = EBiomeType::Tundra;
	TundraConfig.CommonChallenges = {
		EEnvironmentalChallenge::ColdSnap,
		EEnvironmentalChallenge::PoorSoil,
		EEnvironmentalChallenge::HighWinds
	};
	TundraConfig.ChallengeSeverity = 0.7f;
	TundraConfig.ChallengeFrequency = 0.5f;
	BiomeChallengeConfigs.Add(TundraConfig);

	// Volcanic - Heat, poor soil initially, but fertile
	FBiomeChallengeConfig VolcanicConfig;
	VolcanicConfig.BiomeType = EBiomeType::Volcanic;
	VolcanicConfig.CommonChallenges = {
		EEnvironmentalChallenge::HeatWave,
		EEnvironmentalChallenge::PoorSoil
	};
	VolcanicConfig.ChallengeSeverity = 0.6f;
	VolcanicConfig.ChallengeFrequency = 0.4f;
	BiomeChallengeConfigs.Add(VolcanicConfig);

	// Ocean - Flooding, salt, specialized
	FBiomeChallengeConfig OceanConfig;
	OceanConfig.BiomeType = EBiomeType::Ocean;
	OceanConfig.CommonChallenges = {
		EEnvironmentalChallenge::Flood,
		EEnvironmentalChallenge::HighWinds
	};
	OceanConfig.ChallengeSeverity = 0.8f;
	OceanConfig.ChallengeFrequency = 0.5f;
	BiomeChallengeConfigs.Add(OceanConfig);
}

const FBiomeChallengeConfig* UEnvironmentalModifierSystem::GetBiomeChallengeConfig(EBiomeType BiomeType) const
{
	for (const FBiomeChallengeConfig& Config : BiomeChallengeConfigs)
	{
		if (Config.BiomeType == BiomeType)
		{
			return &Config;
		}
	}

	return nullptr;
}
