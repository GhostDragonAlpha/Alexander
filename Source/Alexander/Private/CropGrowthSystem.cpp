// Copyright Epic Games, Inc. All Rights Reserved.

#include "CropGrowthSystem.h"
#include "Math/UnrealMathUtility.h"

UCropGrowthSystem::UCropGrowthSystem()
{
	InitializeCropDatabase();
}

FCropGrowthData UCropGrowthSystem::GetCropGrowthData(ECropTypeExtended CropType) const
{
	if (CropGrowthDataMap.Contains(CropType))
	{
		return CropGrowthDataMap[CropType];
	}

	// Return default wheat data if not found
	FCropGrowthData DefaultData;
	DefaultData.CropType = ECropTypeExtended::Wheat;
	DefaultData.CropName = TEXT("Wheat");
	DefaultData.BaseGrowthTime = 60.0f;
	DefaultData.WaterNeed = 0.5f;
	DefaultData.NutrientNeed = 0.4f;
	DefaultData.OptimalTemp = 20.0f;
	DefaultData.TempTolerance = 10.0f;
	DefaultData.BaseYield = 15;
	DefaultData.MarketValue = 10.0f;

	return DefaultData;
}

TArray<ECropTypeExtended> UCropGrowthSystem::GetAllAvailableCrops() const
{
	TArray<ECropTypeExtended> AvailableCrops;
	for (const auto& CropPair : CropGrowthDataMap)
	{
		AvailableCrops.Add(CropPair.Key);
	}
	return AvailableCrops;
}

TArray<ECropTypeExtended> UCropGrowthSystem::GetCropsByCategory(const FString& Category) const
{
	TArray<ECropTypeExtended> CropsInCategory;
	
	for (const auto& CropPair : CropGrowthDataMap)
	{
		if (CropPair.Value.Category == Category)
		{
			CropsInCategory.Add(CropPair.Key);
		}
	}
	
	return CropsInCategory;
}

TArray<ECropTypeExtended> UCropGrowthSystem::GetCropsForEnvironment(float Temperature, float Humidity, float LightLevel) const
{
	TArray<ECropTypeExtended> SuitableCrops;
	
	for (const auto& CropPair : CropGrowthDataMap)
	{
		const FCropGrowthData& CropData = CropPair.Value;
		
		// Check temperature suitability
		float TempDiff = FMath::Abs(Temperature - CropData.OptimalTemp);
		if (TempDiff <= CropData.TempTolerance)
		{
			// Check humidity suitability
			float HumidityDiff = FMath::Abs(Humidity - CropData.OptimalHumidity);
			if (HumidityDiff <= 0.3f) // Within 30% of optimal
			{
				// Check light requirement
				if (LightLevel >= CropData.LightRequirement * 0.8f) // At least 80% of required light
				{
					SuitableCrops.Add(CropPair.Key);
				}
			}
		}
	}
	
	return SuitableCrops;
}

float UCropGrowthSystem::CalculateGrowthRate(ECropTypeExtended CropType, float Temperature, float Humidity, float SoilQuality, float LightLevel, EFertilizerType FertilizerType) const
{
	const FCropGrowthData& CropData = GetCropGrowthData(CropType);
	
	// Base growth rate
	float BaseRate = 1.0f / CropData.BaseGrowthTime;
	
	// Temperature factor (optimal = 1.0, decreases as we move away)
	float TempDiff = FMath::Abs(Temperature - CropData.OptimalTemp);
	float TempFactor = 1.0f - (TempDiff / CropData.TempTolerance);
	TempFactor = FMath::Clamp(TempFactor, 0.1f, 1.2f); // Never completely stop, can be slightly better than optimal
	
	// Humidity factor
	float HumidityDiff = FMath::Abs(Humidity - CropData.OptimalHumidity);
	float HumidityFactor = 1.0f - (HumidityDiff * 2.0f); // Humidity has stronger effect
	HumidityFactor = FMath::Clamp(HumidityFactor, 0.2f, 1.2f);
	
	// Soil quality factor
	float SoilFactor = FMath::Clamp(SoilQuality, 0.3f, 1.5f); // Poor soil slows, excellent soil boosts
	
	// Light factor
	float LightFactor = FMath::Clamp(LightLevel / CropData.LightRequirement, 0.3f, 1.2f);
	
	// Fertilizer factor
	float FertilizerFactor = GetFertilizerEffectiveness(FertilizerType, CropType);
	
	// Combine all factors
	float TotalFactor = TempFactor * HumidityFactor * SoilFactor * LightFactor * FertilizerFactor;
	
	// Apply growth difficulty
	float FinalRate = BaseRate * TotalFactor * (1.0f - CropData.GrowthDifficulty * 0.3f);
	
	return FMath::Max(FinalRate, 0.0f);
}

float UCropGrowthSystem::CalculateWaterNeeds(ECropTypeExtended CropType, float Temperature, float Humidity, float GrowthStage) const
{
	const FCropGrowthData& CropData = GetCropGrowthData(CropType);
	
	// Base water need
	float BaseNeed = CropData.WaterNeed;
	
	// Temperature effect (hotter = more water needed)
	float TempFactor = 1.0f + (FMath::Max(0.0f, Temperature - 20.0f) / 20.0f);
	
	// Humidity effect (more humid = less water needed)
	float HumidityFactor = 1.0f - (Humidity * 0.5f);
	
	// Growth stage effect (flowering/fruiting needs more water)
	float StageFactor = 1.0f;
	if (GrowthStage > 0.3f && GrowthStage < 0.8f) // Vegetative to fruiting
	{
		StageFactor = 1.5f;
	}
	
	return BaseNeed * TempFactor * HumidityFactor * StageFactor;
}

float UCropGrowthSystem::CalculateNutrientNeeds(ECropTypeExtended CropType, float GrowthStage) const
{
	const FCropGrowthData& CropData = GetCropGrowthData(CropType);
	
	// Base nutrient need
	float BaseNeed = CropData.NutrientNeed;
	
	// Growth stage effect (peak growth needs more nutrients)
	float StageFactor = 1.0f;
	if (GrowthStage > 0.2f && GrowthStage < 0.7f) // Active growing period
	{
		StageFactor = 1.8f;
	}
	else if (GrowthStage >= 0.7f) // Mature stage needs less
	{
		StageFactor = 0.6f;
	}
	
	return BaseNeed * StageFactor;
}

void UCropGrowthSystem::UpdateCropGrowth(FActiveCrop& Crop, float DeltaTime, float Temperature, float Humidity, float SoilQuality, float LightLevel)
{
	// Get crop data
	const FCropGrowthData& CropData = GetCropGrowthData(Crop.CropType);
	
	// Calculate growth rate
	float GrowthRate = CalculateGrowthRate(Crop.CropType, Temperature, Humidity, SoilQuality, LightLevel, Crop.PreferredFertilizer);
	
	// Update growth progress
	float GrowthDelta = GrowthRate * DeltaTime;
	Crop.GrowthProgress = FMath::Clamp(Crop.GrowthProgress + GrowthDelta, 0.0f, 1.0f);
	
	// Update growth stage
	Crop.CurrentStage = GetGrowthStageFromProgress(Crop.CropType, Crop.GrowthProgress);
	
	// Update water level (decreases over time)
	float WaterConsumption = CalculateWaterNeeds(Crop.CropType, Temperature, Humidity, Crop.GrowthProgress);
	Crop.WaterLevel = FMath::Clamp(Crop.WaterLevel - WaterConsumption * DeltaTime * 0.1f, 0.0f, 1.0f);
	
	// Update nutrient level (decreases over time)
	float NutrientConsumption = CalculateNutrientNeeds(Crop.CropType, Crop.GrowthProgress);
	Crop.NutrientLevel = FMath::Clamp(Crop.NutrientLevel - NutrientConsumption * DeltaTime * 0.05f, 0.0f, 1.0f);
	
	// Check for pests and disease
	CheckForPests(Crop, DeltaTime, Temperature, Humidity);
	CheckForDisease(Crop, DeltaTime, Temperature, Humidity);
	
	// Calculate environmental stress
	float Stress = CalculateEnvironmentalStress(Crop, Temperature, Humidity, SoilQuality, LightLevel);
	
	// Update health based on water, nutrients, pests, disease, and stress
	float HealthDelta = 0.0f;
	
	// Water effect
	if (Crop.WaterLevel < 0.2f)
	{
		HealthDelta -= 0.2f * DeltaTime; // Dehydration
	}
	else if (Crop.WaterLevel > 0.8f)
	{
		HealthDelta -= 0.05f * DeltaTime; // Overwatering
	}
	else
	{
		HealthDelta += 0.05f * DeltaTime; // Optimal water
	}
	
	// Nutrient effect
	if (Crop.NutrientLevel < 0.2f)
	{
		HealthDelta -= 0.15f * DeltaTime; // Malnutrition
	}
	else if (Crop.NutrientLevel > 0.7f)
	{
		HealthDelta += 0.03f * DeltaTime; // Well nourished
	}
	
	// Pest effect
	if (Crop.PestLevel > 0.0f)
	{
		float PestDamage = GetPestDamageRate(Crop.CurrentPest) * Crop.PestLevel;
		HealthDelta -= PestDamage * DeltaTime;
	}
	
	// Disease effect
	if (Crop.DiseaseLevel > 0.0f)
	{
		float DiseaseDamage = GetDiseaseDamageRate(Crop.CurrentDisease) * Crop.DiseaseLevel;
		HealthDelta -= DiseaseDamage * DeltaTime;
	}
	
	// Stress effect
	HealthDelta -= Stress * DeltaTime * 0.1f;
	
	// Apply health delta
	Crop.Health = FMath::Clamp(Crop.Health + HealthDelta, 0.0f, 1.0f);
	
	// Update quality modifier based on health and care
	if (Crop.Health > 0.8f)
	{
		Crop.QualityModifier = FMath::Min(Crop.QualityModifier + 0.01f * DeltaTime, 1.5f);
	}
	else if (Crop.Health < 0.5f)
	{
		Crop.QualityModifier = FMath::Max(Crop.QualityModifier - 0.02f * DeltaTime, 0.5f);
	}
}

void UCropGrowthSystem::WaterCrop(FActiveCrop& Crop, float WaterAmount)
{
	Crop.WaterLevel = FMath::Clamp(Crop.WaterLevel + WaterAmount, 0.0f, 1.0f);
}

void UCropGrowthSystem::FertilizeCrop(FActiveCrop& Crop, float FertilizerAmount, EFertilizerType FertilizerType)
{
	float Effectiveness = GetFertilizerEffectiveness(FertilizerType, Crop.CropType);
	Crop.NutrientLevel = FMath::Clamp(Crop.NutrientLevel + (FertilizerAmount * Effectiveness), 0.0f, 1.0f);
}

void UCropGrowthSystem::ApplyPesticide(FActiveCrop& Crop, float Effectiveness)
{
	Crop.PestLevel = FMath::Clamp(Crop.PestLevel - Effectiveness, 0.0f, 1.0f);
	if (Crop.PestLevel < 0.1f)
	{
		Crop.CurrentPest = EPestType::None;
	}
}

void UCropGrowthSystem::ApplyFungicide(FActiveCrop& Crop, float Effectiveness)
{
	Crop.DiseaseLevel = FMath::Clamp(Crop.DiseaseLevel - Effectiveness, 0.0f, 1.0f);
	if (Crop.DiseaseLevel < 0.1f)
	{
		Crop.CurrentDisease = EDiseaseType::None;
	}
}

void UCropGrowthSystem::CheckForPests(FActiveCrop& Crop, float DeltaTime, float Temperature, float Humidity)
{
	const FCropGrowthData& CropData = GetCropGrowthData(Crop.CropType);
	
	// Calculate probability of pest infestation
	float PestProbability = CalculatePestProbability(Temperature, Humidity, CropData.PestResistance);
	
	// Check for new pest infestation
	if (Crop.PestLevel < 0.1f && FMath::FRand() < PestProbability * DeltaTime)
	{
		// New pest infestation
		Crop.PestLevel = FMath::FRandRange(0.1f, 0.3f);
		
		// Randomly select pest type
		int32 PestTypeIndex = FMath::RandRange(0, 5);
		Crop.CurrentPest = static_cast<EPestType>(PestTypeIndex);
	}
	else if (Crop.PestLevel > 0.0f)
	{
		// Existing pests spread
		Crop.PestLevel = FMath::Min(Crop.PestLevel + 0.05f * DeltaTime, 1.0f);
	}
}

void UCropGrowthSystem::CheckForDisease(FActiveCrop& Crop, float DeltaTime, float Temperature, float Humidity)
{
	const FCropGrowthData& CropData = GetCropGrowthData(Crop.CropType);
	
	// Calculate probability of disease
	float DiseaseProbability = CalculateDiseaseProbability(Temperature, Humidity, CropData.DiseaseResistance);
	
	// Check for new disease
	if (Crop.DiseaseLevel < 0.1f && FMath::FRand() < DiseaseProbability * DeltaTime)
	{
		// New disease
		Crop.DiseaseLevel = FMath::FRandRange(0.1f, 0.3f);
		
		// Randomly select disease type
		int32 DiseaseTypeIndex = FMath::RandRange(0, 5);
		Crop.CurrentDisease = static_cast<EDiseaseType>(DiseaseTypeIndex);
	}
	else if (Crop.DiseaseLevel > 0.0f)
	{
		// Existing disease spreads
		Crop.DiseaseLevel = FMath::Min(Crop.DiseaseLevel + 0.03f * DeltaTime, 1.0f);
	}
}

ECropGrowthStage UCropGrowthSystem::GetGrowthStageFromProgress(ECropTypeExtended CropType, float Progress) const
{
	const FCropGrowthData& CropData = GetCropGrowthData(CropType);
	
	if (Progress < CropData.SeedStageDuration)
	{
		return ECropGrowthStage::Seed;
	}
	else if (Progress < CropData.SeedStageDuration + CropData.SproutStageDuration)
	{
		return ECropGrowthStage::Sprout;
	}
	else if (Progress < CropData.SeedStageDuration + CropData.SproutStageDuration + CropData.GrowingStageDuration)
	{
		return ECropGrowthStage::Vegetative;
	}
	else if (Progress < 0.9f)
	{
		return ECropGrowthStage::Flowering;
	}
	else if (Progress < 1.0f)
	{
		return ECropGrowthStage::Fruiting;
	}
	else
	{
		return ECropGrowthStage::Mature;
	}
}

int32 UCropGrowthSystem::CalculateFinalYield(const FActiveCrop& Crop, const FCropGrowthData& CropData) const
{
	// Base yield modified by health and quality
	float HealthFactor = Crop.Health;
	float QualityFactor = Crop.QualityModifier;
	
	// Pest and disease reduce yield
	float PestFactor = 1.0f - (Crop.PestLevel * 0.5f);
	float DiseaseFactor = 1.0f - (Crop.DiseaseLevel * 0.7f);
	
	int32 FinalYield = FMath::RoundToInt(CropData.BaseYield * HealthFactor * QualityFactor * PestFactor * DiseaseFactor);
	
	return FMath::Max(FinalYield, 1); // Minimum yield of 1
}

float UCropGrowthSystem::CalculateCropQuality(const FActiveCrop& Crop) const
{
	// Quality based on health, water/nutrient levels, and lack of pests/disease
	float Quality = Crop.Health * 0.4f;
	Quality += Crop.WaterLevel * 0.2f;
	Quality += Crop.NutrientLevel * 0.2f;
	Quality += (1.0f - Crop.PestLevel) * 0.1f;
	Quality += (1.0f - Crop.DiseaseLevel) * 0.1f;
	
	return FMath::Clamp(Quality * Crop.QualityModifier, 0.0f, 1.0f);
}

float UCropGrowthSystem::GetFertilizerEffectiveness(EFertilizerType FertilizerType, ECropTypeExtended CropType) const
{
	const FCropGrowthData& CropData = GetCropGrowthData(CropType);
	
	// Base effectiveness
	float BaseEffectiveness = 0.5f;
	
	// Type-specific effectiveness
	switch (FertilizerType)
	{
		case EFertilizerType::Basic:
			BaseEffectiveness = 0.6f;
			break;
		case EFertilizerType::Premium:
			BaseEffectiveness = 0.9f;
			break;
		case EFertilizerType::Specialized:
			// Extra effective if matches crop preference
			BaseEffectiveness = (FertilizerType == CropData.PreferredFertilizer) ? 1.2f : 0.7f;
			break;
		case EFertilizerType::Organic:
			BaseEffectiveness = 0.7f;
			break;
		case EFertilizerType::Synthetic:
			BaseEffectiveness = 0.8f;
			break;
	}
	
	return BaseEffectiveness;
}

float UCropGrowthSystem::GetPestDamageRate(EPestType PestType) const
{
	switch (PestType)
	{
		case EPestType::Aphids:
			return 0.1f;
		case EPestType::SpiderMites:
			return 0.15f;
		case EPestType::Caterpillars:
			return 0.2f;
		case EPestType::RootRot:
			return 0.25f;
		case EPestType::Mildew:
			return 0.12f;
		case EPestType::Rodents:
			return 0.3f;
		default:
			return 0.0f;
	}
}

float UCropGrowthSystem::GetDiseaseDamageRate(EDiseaseType DiseaseType) const
{
	switch (DiseaseType)
	{
		case EDiseaseType::Blight:
			return 0.3f;
		case EDiseaseType::Rust:
			return 0.2f;
		case EDiseaseType::Wilt:
			return 0.25f;
		case EDiseaseType::Mold:
			return 0.15f;
		case EDiseaseType::Virus:
			return 0.35f;
		default:
			return 0.0f;
	}
}

float UCropGrowthSystem::GetStageThreshold(ECropGrowthStage Stage, const FCropGrowthData& CropData) const
{
	switch (Stage)
	{
		case ECropGrowthStage::Seed:
			return CropData.SeedStageDuration;
		case ECropGrowthStage::Sprout:
			return CropData.SeedStageDuration + CropData.SproutStageDuration;
		case ECropGrowthStage::Vegetative:
			return CropData.SeedStageDuration + CropData.SproutStageDuration + CropData.GrowingStageDuration;
		case ECropGrowthStage::Flowering:
			return 0.9f;
		case ECropGrowthStage::Fruiting:
			return 1.0f;
		default:
			return 1.0f;
	}
}

float UCropGrowthSystem::CalculateEnvironmentalStress(const FActiveCrop& Crop, float Temperature, float Humidity, float SoilQuality, float LightLevel) const
{
	const FCropGrowthData& CropData = GetCropGrowthData(Crop.CropType);
	
	float Stress = 0.0f;
	
	// Temperature stress
	float TempDiff = FMath::Abs(Temperature - CropData.OptimalTemp);
	Stress += FMath::Clamp(TempDiff / CropData.TempTolerance, 0.0f, 1.0f) * 0.3f;
	
	// Humidity stress
	float HumidityDiff = FMath::Abs(Humidity - CropData.OptimalHumidity);
	Stress += FMath::Clamp(HumidityDiff * 2.0f, 0.0f, 1.0f) * 0.2f;
	
	// Soil quality stress
	Stress += FMath::Clamp((1.0f - SoilQuality) * 0.5f, 0.0f, 1.0f) * 0.25f;
	
	// Light stress
	float LightDeficit = FMath::Max(0.0f, CropData.LightRequirement - LightLevel);
	Stress += FMath::Clamp(LightDeficit * 2.0f, 0.0f, 1.0f) * 0.25f;
	
	return FMath::Clamp(Stress, 0.0f, 1.0f);
}

float UCropGrowthSystem::CalculatePestProbability(float Temperature, float Humidity, float PestResistance) const
{
	// Pests thrive in warm, humid conditions
	float TempFactor = FMath::Clamp((Temperature - 15.0f) / 20.0f, 0.0f, 1.0f);
	float HumidityFactor = FMath::Clamp(Humidity, 0.0f, 1.0f);
	
	float BaseProbability = (TempFactor + HumidityFactor) / 2.0f * 0.1f; // Base 10% chance per second under ideal conditions
	
	// Reduce by pest resistance
	return BaseProbability * (1.0f - PestResistance);
}

float UCropGrowthSystem::CalculateDiseaseProbability(float Temperature, float Humidity, float DiseaseResistance) const
{
	// Diseases thrive in specific conditions
	float TempFactor = FMath::Clamp(1.0f - FMath::Abs(Temperature - 20.0f) / 15.0f, 0.0f, 1.0f);
	float HumidityFactor = FMath::Clamp(Humidity, 0.3f, 0.9f); // Moderate to high humidity
	
	float BaseProbability = (TempFactor + HumidityFactor) / 2.0f * 0.05f; // Base 5% chance per second
	
	// Reduce by disease resistance
	return BaseProbability * (1.0f - DiseaseResistance);
}

void UCropGrowthSystem::InitializeCropDatabase()
{
	// Clear existing data
	CropDatabase.Empty();
	CropGrowthDataMap.Empty();

	// === FOOD CROPS (10 types) ===

	// Wheat
	FCropGrowthData Wheat;
	Wheat.CropType = ECropTypeExtended::Wheat;
	Wheat.CropName = TEXT("Wheat");
	Wheat.BaseGrowthTime = 120.0f; // 2 minutes
	Wheat.WaterNeed = 0.6f;
	Wheat.NutrientNeed = 0.5f;
	Wheat.OptimalTemp = 20.0f;
	Wheat.TempTolerance = 8.0f;
	Wheat.OptimalHumidity = 0.6f;
	Wheat.LightRequirement = 0.8f;
	Wheat.BaseYield = 20;
	Wheat.MarketValue = 8.0f;
	Wheat.Category = TEXT("Food");
	Wheat.GrowthDifficulty = 0.3f;
	Wheat.PestResistance = 0.7f;
	Wheat.DiseaseResistance = 0.8f;
	Wheat.PreferredSeason = 0.5f; // Spring
	Wheat.SoilQualityRequirement = 0.3f;
	Wheat.PreferredFertilizer = EFertilizerType::Basic;
	CropDatabase.Add(Wheat);
	CropGrowthDataMap.Add(ECropTypeExtended::Wheat, Wheat);

	// Corn
	FCropGrowthData Corn;
	Corn.CropType = ECropTypeExtended::Corn;
	Corn.CropName = TEXT("Corn");
	Corn.BaseGrowthTime = 180.0f; // 3 minutes
	Corn.WaterNeed = 0.8f;
	Corn.NutrientNeed = 0.7f;
	Corn.OptimalTemp = 25.0f;
	Corn.TempTolerance = 6.0f;
	Corn.OptimalHumidity = 0.7f;
	Corn.LightRequirement = 0.9f;
	Corn.BaseYield = 15;
	Corn.MarketValue = 12.0f;
	Corn.Category = TEXT("Food");
	Corn.GrowthDifficulty = 0.4f;
	Corn.PestResistance = 0.6f;
	Corn.DiseaseResistance = 0.6f;
	Corn.PreferredSeason = 0.7f; // Summer
	Corn.SoilQualityRequirement = 0.4f;
	Corn.PreferredFertilizer = EFertilizerType::Premium;
	CropDatabase.Add(Corn);
	CropGrowthDataMap.Add(ECropTypeExtended::Corn, Corn);

	// Tomatoes
	FCropGrowthData Tomatoes;
	Tomatoes.CropType = ECropTypeExtended::Tomatoes;
	Tomatoes.CropName = TEXT("Tomatoes");
	Tomatoes.BaseGrowthTime = 90.0f; // 1.5 minutes
	Tomatoes.WaterNeed = 0.9f;
	Tomatoes.NutrientNeed = 0.8f;
	Tomatoes.OptimalTemp = 22.0f;
	Tomatoes.TempTolerance = 5.0f;
	Tomatoes.OptimalHumidity = 0.7f;
	Tomatoes.LightRequirement = 0.9f;
	Tomatoes.BaseYield = 25;
	Tomatoes.MarketValue = 18.0f;
	Tomatoes.Category = TEXT("Food");
	Tomatoes.GrowthDifficulty = 0.6f;
	Tomatoes.PestResistance = 0.5f;
	Tomatoes.DiseaseResistance = 0.5f;
	Tomatoes.PreferredSeason = 0.6f; // Late spring/summer
	Tomatoes.SoilQualityRequirement = 0.5f;
	Tomatoes.PreferredFertilizer = EFertilizerType::Premium;
	CropDatabase.Add(Tomatoes);
	CropGrowthDataMap.Add(ECropTypeExtended::Tomatoes, Tomatoes);

	// Potatoes
	FCropGrowthData Potatoes;
	Potatoes.CropType = ECropTypeExtended::Potatoes;
	Potatoes.CropName = TEXT("Potatoes");
	Potatoes.BaseGrowthTime = 150.0f; // 2.5 minutes
	Potatoes.WaterNeed = 0.7f;
	Potatoes.NutrientNeed = 0.6f;
	Potatoes.OptimalTemp = 18.0f;
	Potatoes.TempTolerance = 10.0f;
	Potatoes.OptimalHumidity = 0.6f;
	Potatoes.LightRequirement = 0.7f;
	Potatoes.BaseYield = 30;
	Potatoes.MarketValue = 10.0f;
	Potatoes.Category = TEXT("Food");
	Potatoes.GrowthDifficulty = 0.3f;
	Potatoes.PestResistance = 0.8f;
	Potatoes.DiseaseResistance = 0.6f;
	Potatoes.PreferredSeason = 0.4f; // Cooler season
	Potatoes.SoilQualityRequirement = 0.3f;
	Potatoes.PreferredFertilizer = EFertilizerType::Basic;
	CropDatabase.Add(Potatoes);
	CropGrowthDataMap.Add(ECropTypeExtended::Potatoes, Potatoes);

	// Carrots
	FCropGrowthData Carrots;
	Carrots.CropType = ECropTypeExtended::Carrots;
	Carrots.CropName = TEXT("Carrots");
	Carrots.BaseGrowthTime = 80.0f; // 1.3 minutes
	Carrots.WaterNeed = 0.6f;
	Carrots.NutrientNeed = 0.4f;
	Carrots.OptimalTemp = 16.0f;
	Carrots.TempTolerance = 8.0f;
	Carrots.OptimalHumidity = 0.6f;
	Carrots.LightRequirement = 0.8f;
	Carrots.BaseYield = 22;
	Carrots.MarketValue = 14.0f;
	Carrots.Category = TEXT("Food");
	Carrots.GrowthDifficulty = 0.2f;
	Carrots.PestResistance = 0.8f;
	Carrots.DiseaseResistance = 0.7f;
	Carrots.PreferredSeason = 0.4f; // Cool season
	Carrots.SoilQualityRequirement = 0.2f;
	Carrots.PreferredFertilizer = EFertilizerType::Organic;
	CropDatabase.Add(Carrots);
	CropGrowthDataMap.Add(ECropTypeExtended::Carrots, Carrots);

	// Lettuce
	FCropGrowthData Lettuce;
	Lettuce.CropType = ECropTypeExtended::Lettuce;
	Lettuce.CropName = TEXT("Lettuce");
	Lettuce.BaseGrowthTime = 45.0f; // 45 seconds
	Lettuce.WaterNeed = 0.8f;
	Lettuce.NutrientNeed = 0.3f;
	Lettuce.OptimalTemp = 15.0f;
	Lettuce.TempTolerance = 7.0f;
	Lettuce.OptimalHumidity = 0.7f;
	Lettuce.LightRequirement = 0.7f;
	Lettuce.BaseYield = 18;
	Lettuce.MarketValue = 6.0f;
	Lettuce.Category = TEXT("Food");
	Lettuce.GrowthDifficulty = 0.2f;
	Lettuce.PestResistance = 0.6f;
	Lettuce.DiseaseResistance = 0.6f;
	Lettuce.PreferredSeason = 0.3f; // Cool season
	Lettuce.SoilQualityRequirement = 0.2f;
	Lettuce.PreferredFertilizer = EFertilizerType::Organic;
	CropDatabase.Add(Lettuce);
	CropGrowthDataMap.Add(ECropTypeExtended::Lettuce, Lettuce);

	// Strawberries
	FCropGrowthData Strawberries;
	Strawberries.CropType = ECropTypeExtended::Strawberries;
	Strawberries.CropName = TEXT("Strawberries");
	Strawberries.BaseGrowthTime = 100.0f; // 1.7 minutes
	Strawberries.WaterNeed = 0.7f;
	Strawberries.NutrientNeed = 0.6f;
	Strawberries.OptimalTemp = 19.0f;
	Strawberries.TempTolerance = 6.0f;
	Strawberries.OptimalHumidity = 0.6f;
	Strawberries.LightRequirement = 0.8f;
	Strawberries.BaseYield = 12;
	Strawberries.MarketValue = 28.0f;
	Strawberries.Category = TEXT("Food");
	Strawberries.GrowthDifficulty = 0.7f;
	Strawberries.PestResistance = 0.4f;
	Strawberries.DiseaseResistance = 0.5f;
	Strawberries.PreferredSeason = 0.5f; // Spring
	Strawberries.SoilQualityRequirement = 0.6f;
	Strawberries.PreferredFertilizer = EFertilizerType::Premium;
	CropDatabase.Add(Strawberries);
	CropGrowthDataMap.Add(ECropTypeExtended::Strawberries, Strawberries);

	// Rice
	FCropGrowthData Rice;
	Rice.CropType = ECropTypeExtended::Rice;
	Rice.CropName = TEXT("Rice");
	Rice.BaseGrowthTime = 140.0f; // 2.3 minutes
	Rice.WaterNeed = 1.0f; // Very high water need
	Rice.NutrientNeed = 0.7f;
	Rice.OptimalTemp = 24.0f;
	Rice.TempTolerance = 5.0f;
	Rice.OptimalHumidity = 0.8f;
	Rice.LightRequirement = 0.9f;
	Rice.BaseYield = 25;
	Rice.MarketValue = 11.0f;
	Rice.Category = TEXT("Food");
	Rice.GrowthDifficulty = 0.5f;
	Rice.PestResistance = 0.5f;
	Rice.DiseaseResistance = 0.4f;
	Rice.PreferredSeason = 0.8f; // Summer
	Rice.SoilQualityRequirement = 0.4f;
	Rice.PreferredFertilizer = EFertilizerType::Premium;
	CropDatabase.Add(Rice);
	CropGrowthDataMap.Add(ECropTypeExtended::Rice, Rice);

	// Beans
	FCropGrowthData Beans;
	Beans.CropType = ECropTypeExtended::Beans;
	Beans.CropName = TEXT("Beans");
	Beans.BaseGrowthTime = 70.0f; // 1.2 minutes
	Beans.WaterNeed = 0.5f;
	Beans.NutrientNeed = 0.4f;
	Beans.OptimalTemp = 21.0f;
	Beans.TempTolerance = 8.0f;
	Beans.OptimalHumidity = 0.6f;
	Beans.LightRequirement = 0.8f;
	Beans.BaseYield = 20;
	Beans.MarketValue = 9.0f;
	Beans.Category = TEXT("Food");
	Beans.GrowthDifficulty = 0.3f;
	Beans.PestResistance = 0.7f;
	Beans.DiseaseResistance = 0.7f;
	Beans.PreferredSeason = 0.6f; // Summer
	Beans.SoilQualityRequirement = 0.3f;
	Beans.PreferredFertilizer = EFertilizerType::Basic;
	CropDatabase.Add(Beans);
	CropGrowthDataMap.Add(ECropTypeExtended::Beans, Beans);

	// Onions
	FCropGrowthData Onions;
	Onions.CropType = ECropTypeExtended::Onions;
	Onions.CropName = TEXT("Onions");
	Onions.BaseGrowthTime = 85.0f; // 1.4 minutes
	Onions.WaterNeed = 0.6f;
	Onions.NutrientNeed = 0.4f;
	Onions.OptimalTemp = 17.0f;
	Onions.TempTolerance = 9.0f;
	Onions.OptimalHumidity = 0.5f;
	Onions.LightRequirement = 0.8f;
	Onions.BaseYield = 28;
	Onions.MarketValue = 8.0f;
	Onions.Category = TEXT("Food");
	Onions.GrowthDifficulty = 0.2f;
	Onions.PestResistance = 0.8f;
	Onions.DiseaseResistance = 0.7f;
	Onions.PreferredSeason = 0.4f; // Cool season
	Onions.SoilQualityRequirement = 0.2f;
	Onions.PreferredFertilizer = EFertilizerType::Basic;
	CropDatabase.Add(Onions);
	CropGrowthDataMap.Add(ECropTypeExtended::Onions, Onions);

	// === MEDICAL CROPS (4 types) ===

	// Medical Herbs
	FCropGrowthData MedicalHerbs;
	MedicalHerbs.CropType = ECropTypeExtended::MedicalHerbs;
	MedicalHerbs.CropName = TEXT("Medical Herbs");
	MedicalHerbs.BaseGrowthTime = 60.0f; // 1 minute
	MedicalHerbs.WaterNeed = 0.5f;
	MedicalHerbs.NutrientNeed = 0.5f;
	MedicalHerbs.OptimalTemp = 20.0f;
	MedicalHerbs.TempTolerance = 10.0f;
	MedicalHerbs.OptimalHumidity = 0.6f;
	MedicalHerbs.LightRequirement = 0.7f;
	MedicalHerbs.BaseYield = 10;
	MedicalHerbs.MarketValue = 45.0f;
	MedicalHerbs.Category = TEXT("Medical");
	MedicalHerbs.GrowthDifficulty = 0.6f;
	MedicalHerbs.PestResistance = 0.7f;
	MedicalHerbs.DiseaseResistance = 0.6f;
	MedicalHerbs.PreferredSeason = 0.5f;
	MedicalHerbs.SoilQualityRequirement = 0.4f;
	MedicalHerbs.PreferredFertilizer = EFertilizerType::Specialized;
	CropDatabase.Add(MedicalHerbs);
	CropGrowthDataMap.Add(ECropTypeExtended::MedicalHerbs, MedicalHerbs);

	// Pharmaceutical Plants
	FCropGrowthData PharmaceuticalPlants;
	PharmaceuticalPlants.CropType = ECropTypeExtended::PharmaceuticalPlants;
	PharmaceuticalPlants.CropName = TEXT("Pharmaceutical Plants");
	PharmaceuticalPlants.BaseGrowthTime = 200.0f; // 3.3 minutes
	PharmaceuticalPlants.WaterNeed = 0.7f;
	PharmaceuticalPlants.NutrientNeed = 0.8f;
	PharmaceuticalPlants.OptimalTemp = 22.0f;
	PharmaceuticalPlants.TempTolerance = 4.0f;
	PharmaceuticalPlants.OptimalHumidity = 0.7f;
	PharmaceuticalPlants.LightRequirement = 0.8f;
	PharmaceuticalPlants.BaseYield = 5;
	PharmaceuticalPlants.MarketValue = 120.0f;
	PharmaceuticalPlants.Category = TEXT("Medical");
	PharmaceuticalPlants.GrowthDifficulty = 0.9f;
	PharmaceuticalPlants.PestResistance = 0.4f;
	PharmaceuticalPlants.DiseaseResistance = 0.5f;
	PharmaceuticalPlants.PreferredSeason = 0.6f;
	PharmaceuticalPlants.SoilQualityRequirement = 0.7f;
	PharmaceuticalPlants.PreferredFertilizer = EFertilizerType::Specialized;
	CropDatabase.Add(PharmaceuticalPlants);
	CropGrowthDataMap.Add(ECropTypeExtended::PharmaceuticalPlants, PharmaceuticalPlants);

	// Aloe Vera
	FCropGrowthData AloeVera;
	AloeVera.CropType = ECropTypeExtended::AloeVera;
	AloeVera.CropName = TEXT("Aloe Vera");
	AloeVera.BaseGrowthTime = 110.0f; // 1.8 minutes
	AloeVera.WaterNeed = 0.3f; // Low water need
	AloeVera.NutrientNeed = 0.3f;
	AloeVera.OptimalTemp = 23.0f;
	AloeVera.TempTolerance = 12.0f;
	AloeVera.OptimalHumidity = 0.4f;
	AloeVera.LightRequirement = 0.8f;
	AloeVera.BaseYield = 8;
	AloeVera.MarketValue = 35.0f;
	AloeVera.Category = TEXT("Medical");
	AloeVera.GrowthDifficulty = 0.4f;
	AloeVera.PestResistance = 0.9f;
	AloeVera.DiseaseResistance = 0.8f;
	AloeVera.PreferredSeason = 0.7f; // Warm season
	AloeVera.SoilQualityRequirement = 0.3f;
	AloeVera.PreferredFertilizer = EFertilizerType::Organic;
	CropDatabase.Add(AloeVera);
	CropGrowthDataMap.Add(ECropTypeExtended::AloeVera, AloeVera);

	// Lavender
	FCropGrowthData Lavender;
	Lavender.CropType = ECropTypeExtended::Lavender;
	Lavender.CropName = TEXT("Lavender");
	Lavender.BaseGrowthTime = 95.0f; // 1.6 minutes
	Lavender.WaterNeed = 0.4f;
	Lavender.NutrientNeed = 0.4f;
	Lavender.OptimalTemp = 19.0f;
	Lavender.TempTolerance = 8.0f;
	Lavender.OptimalHumidity = 0.5f;
	Lavender.LightRequirement = 0.9f;
	Lavender.BaseYield = 12;
	Lavender.MarketValue = 40.0f;
	Lavender.Category = TEXT("Medical");
	Lavender.GrowthDifficulty = 0.5f;
	Lavender.PestResistance = 0.8f;
	Lavender.DiseaseResistance = 0.7f;
	Lavender.PreferredSeason = 0.5f;
	Lavender.SoilQualityRequirement = 0.4f;
	Lavender.PreferredFertilizer = EFertilizerType::Organic;
	CropDatabase.Add(Lavender);
	CropGrowthDataMap.Add(ECropTypeExtended::Lavender, Lavender);

	// === INDUSTRIAL CROPS (4 types) ===

	// Cotton
	FCropGrowthData Cotton;
	Cotton.CropType = ECropTypeExtended::Cotton;
	Cotton.CropName = TEXT("Cotton");
	Cotton.BaseGrowthTime = 160.0f; // 2.7 minutes
	Cotton.WaterNeed = 0.7f;
	Cotton.NutrientNeed = 0.6f;
	Cotton.OptimalTemp = 24.0f;
	Cotton.TempTolerance = 7.0f;
	Cotton.OptimalHumidity = 0.6f;
	Cotton.LightRequirement = 0.9f;
	Cotton.BaseYield = 8;
	Cotton.MarketValue = 25.0f;
	Cotton.Category = TEXT("Industrial");
	Cotton.GrowthDifficulty = 0.6f;
	Cotton.PestResistance = 0.5f;
	Cotton.DiseaseResistance = 0.5f;
	Cotton.PreferredSeason = 0.7f; // Summer
	Cotton.SoilQualityRequirement = 0.5f;
	Cotton.PreferredFertilizer = EFertilizerType::Premium;
	CropDatabase.Add(Cotton);
	CropGrowthDataMap.Add(ECropTypeExtended::Cotton, Cotton);

	// Biofuel Crops
	FCropGrowthData BiofuelCrops;
	BiofuelCrops.CropType = ECropTypeExtended::BiofuelCrops;
	BiofuelCrops.CropName = TEXT("Biofuel Crops");
	BiofuelCrops.BaseGrowthTime = 130.0f; // 2.2 minutes
	BiofuelCrops.WaterNeed = 0.6f;
	BiofuelCrops.NutrientNeed = 0.5f;
	BiofuelCrops.OptimalTemp = 22.0f;
	BiofuelCrops.TempTolerance = 9.0f;
	BiofuelCrops.OptimalHumidity = 0.6f;
	BiofuelCrops.LightRequirement = 0.8f;
	BiofuelCrops.BaseYield = 35;
	BiofuelCrops.MarketValue = 15.0f;
	BiofuelCrops.Category = TEXT("Industrial");
	BiofuelCrops.GrowthDifficulty = 0.4f;
	BiofuelCrops.PestResistance = 0.7f;
	BiofuelCrops.DiseaseResistance = 0.7f;
	BiofuelCrops.PreferredSeason = 0.6f;
	BiofuelCrops.SoilQualityRequirement = 0.3f;
	BiofuelCrops.PreferredFertilizer = EFertilizerType::Basic;
	CropDatabase.Add(BiofuelCrops);
	CropGrowthDataMap.Add(ECropTypeExtended::BiofuelCrops, BiofuelCrops);

	// Rubber Plants
	FCropGrowthData RubberPlants;
	RubberPlants.CropType = ECropTypeExtended::RubberPlants;
	RubberPlants.CropName = TEXT("Rubber Plants");
	RubberPlants.BaseGrowthTime = 300.0f; // 5 minutes
	RubberPlants.WaterNeed = 0.8f;
	RubberPlants.NutrientNeed = 0.7f;
	RubberPlants.OptimalTemp = 26.0f;
	RubberPlants.TempTolerance = 5.0f;
	RubberPlants.OptimalHumidity = 0.8f;
	RubberPlants.LightRequirement = 0.8f;
	RubberPlants.BaseYield = 5;
	RubberPlants.MarketValue = 50.0f;
	RubberPlants.Category = TEXT("Industrial");
	RubberPlants.GrowthDifficulty = 0.8f;
	RubberPlants.PestResistance = 0.6f;
	RubberPlants.DiseaseResistance = 0.6f;
	RubberPlants.PreferredSeason = 0.8f; // Hot season
	RubberPlants.SoilQualityRequirement = 0.6f;
	RubberPlants.PreferredFertilizer = EFertilizerType::Specialized;
	CropDatabase.Add(RubberPlants);
	CropGrowthDataMap.Add(ECropTypeExtended::RubberPlants, RubberPlants);

	// Bamboo
	FCropGrowthData Bamboo;
	Bamboo.CropType = ECropTypeExtended::Bamboo;
	Bamboo.CropName = TEXT("Bamboo");
	Bamboo.BaseGrowthTime = 90.0f; // 1.5 minutes
	Bamboo.WaterNeed = 0.7f;
	Bamboo.NutrientNeed = 0.5f;
	Bamboo.OptimalTemp = 22.0f;
	Bamboo.TempTolerance = 10.0f;
	Bamboo.OptimalHumidity = 0.7f;
	Bamboo.LightRequirement = 0.8f;
	Bamboo.BaseYield = 40;
	Bamboo.MarketValue = 12.0f;
	Bamboo.Category = TEXT("Industrial");
	Bamboo.GrowthDifficulty = 0.3f;
	Bamboo.PestResistance = 0.9f;
	Bamboo.DiseaseResistance = 0.8f;
	Bamboo.PreferredSeason = 0.7f; // Warm season
	Bamboo.SoilQualityRequirement = 0.3f;
	Bamboo.PreferredFertilizer = EFertilizerType::Basic;
	CropDatabase.Add(Bamboo);
	CropGrowthDataMap.Add(ECropTypeExtended::Bamboo, Bamboo);

	// === ILLEGAL/SPECIAL CROPS (2 types) ===

	// Space Weed
	FCropGrowthData SpaceWeed;
	SpaceWeed.CropType = ECropTypeExtended::SpaceWeed;
	SpaceWeed.CropName = TEXT("Space Weed");
	SpaceWeed.BaseGrowthTime = 40.0f; // 40 seconds
	SpaceWeed.WaterNeed = 0.3f;
	SpaceWeed.NutrientNeed = 0.2f;
	SpaceWeed.OptimalTemp = 15.0f;
	SpaceWeed.TempTolerance = 20.0f; // Very tolerant
	SpaceWeed.OptimalHumidity = 0.5f;
	SpaceWeed.LightRequirement = 0.5f;
	SpaceWeed.BaseYield = 10;
	SpaceWeed.MarketValue = 80.0f; // High value
	SpaceWeed.Category = TEXT("Illegal");
	SpaceWeed.GrowthDifficulty = 0.1f; // Very easy
	SpaceWeed.PestResistance = 0.9f;
	SpaceWeed.DiseaseResistance = 0.9f;
	SpaceWeed.PreferredSeason = 0.5f; // Grows anytime
	SpaceWeed.SoilQualityRequirement = 0.1f; // Grows anywhere
	SpaceWeed.PreferredFertilizer = EFertilizerType::Basic;
	CropDatabase.Add(SpaceWeed);
	CropGrowthDataMap.Add(ECropTypeExtended::SpaceWeed, SpaceWeed);

	// Quantum Plants
	FCropGrowthData QuantumPlants;
	QuantumPlants.CropType = ECropTypeExtended::QuantumPlants;
	QuantumPlants.CropName = TEXT("Quantum Plants");
	QuantumPlants.BaseGrowthTime = 250.0f; // 4.2 minutes
	QuantumPlants.WaterNeed = 0.6f;
	QuantumPlants.NutrientNeed = 0.9f;
	QuantumPlants.OptimalTemp = 20.0f;
	QuantumPlants.TempTolerance = 15.0f;
	QuantumPlants.OptimalHumidity = 0.6f;
	QuantumPlants.LightRequirement = 0.7f;
	QuantumPlants.BaseYield = 8;
	QuantumPlants.MarketValue = 150.0f; // Very high value
	QuantumPlants.Category = TEXT("Special");
	QuantumPlants.GrowthDifficulty = 0.9f; // Very difficult
	QuantumPlants.PestResistance = 0.8f;
	QuantumPlants.DiseaseResistance = 0.7f;
	QuantumPlants.PreferredSeason = 0.5f;
	QuantumPlants.SoilQualityRequirement = 0.7f;
	QuantumPlants.PreferredFertilizer = EFertilizerType::Specialized;
	CropDatabase.Add(QuantumPlants);
	CropGrowthDataMap.Add(ECropTypeExtended::QuantumPlants, QuantumPlants);

	UE_LOG(LogTemp, Log, TEXT("CropGrowthSystem: Initialized %d crop types"), CropDatabase.Num());
}