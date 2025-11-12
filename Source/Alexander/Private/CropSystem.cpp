// Copyright Epic Games, Inc. All Rights Reserved.

#include "CropSystem.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions

UCropSystem::UCropSystem()
{
	InitializeCropDatabase();
}

FCropData UCropSystem::GetCropData(ECropTypeExtended CropType) const
{
	if (CropDataMap.Contains(CropType))
	{
		return CropDataMap[CropType];
	}
	
	// Return default crop data if not found
	FCropData DefaultData;
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

TArray<ECropTypeExtended> UCropSystem::GetAvailableCrops() const
{
	TArray<ECropTypeExtended> AvailableCrops;
	
	for (const auto& CropPair : CropDataMap)
	{
		AvailableCrops.Add(CropPair.Key);
	}
	
	return AvailableCrops;
}

TArray<ECropTypeExtended> UCropSystem::GetCropsForClimate(float Temperature, float Humidity) const
{
	TArray<ECropTypeExtended> SuitableCrops;
	
	for (const auto& CropPair : CropDataMap)
	{
		const FCropData& CropData = CropPair.Value;
		float TempDiff = FMath::Abs(Temperature - CropData.OptimalTemp);
		
		if (TempDiff <= CropData.TempTolerance)
		{
			// Check humidity preference
			if ((Humidity > 0.6f && CropData.WaterNeed > 0.5f) ||
				(Humidity <= 0.6f && CropData.WaterNeed <= 0.5f))
			{
				SuitableCrops.Add(CropPair.Key);
			}
		}
	}
	
	return SuitableCrops;
}

float UCropSystem::GetCropMarketValue(ECropTypeExtended CropType) const
{
	FCropData CropData = GetCropData(CropType);
	return CropData.MarketValue;
}

bool UCropSystem::IsCropInSeason(ECropTypeExtended CropType, float SeasonFactor) const
{
	FCropData CropData = GetCropData(CropType);
	
	// SeasonFactor: 0.0 (winter) to 1.0 (summer)
	// Different crops prefer different seasons
	
	switch (CropType)
	{
		case ECropTypeExtended::Wheat:
			// Wheat prefers spring/summer
			return SeasonFactor >= 0.3f && SeasonFactor <= 0.8f;
			
		case ECropTypeExtended::Corn:
			// Corn prefers summer
			return SeasonFactor >= 0.5f && SeasonFactor <= 0.9f;
			
		case ECropTypeExtended::Tomatoes:
			// Tomatoes prefer warm seasons
			return SeasonFactor >= 0.4f && SeasonFactor <= 0.9f;
			
		case ECropTypeExtended::Potatoes:
			// Potatoes prefer cooler seasons
			return SeasonFactor >= 0.2f && SeasonFactor <= 0.7f;
			
		case ECropTypeExtended::SpaceWeed:
			// Space weed grows anywhere
			return true;
			
		case ECropTypeExtended::Rice:
			// Moon melons prefer cooler temperatures
			return SeasonFactor <= 0.5f;
			
		case ECropTypeExtended::QuantumPlants:
			// Quantum potatoes are very adaptable
			return true;
			
		default:
			return true; // Most crops are somewhat adaptable
	}
}

ECropTypeExtended UCropSystem::BreedCrops(ECropTypeExtended Crop1, ECropTypeExtended Crop2)
{
	// Simple breeding system - returns hybrid or better crop
	if (!CanBreedCrops(Crop1, Crop2))
	{
		return Crop1; // Return first crop if breeding not possible
	}
	
	// Breeding combinations
	if ((Crop1 == ECropTypeExtended::Wheat && Crop2 == ECropTypeExtended::Corn) ||
		(Crop1 == ECropTypeExtended::Corn && Crop2 == ECropTypeExtended::Wheat))
	{
		return ECropTypeExtended::QuantumPlants; // Super crop
	}
	
	if ((Crop1 == ECropTypeExtended::Tomatoes && Crop2 == ECropTypeExtended::SpaceWeed) ||
		(Crop1 == ECropTypeExtended::SpaceWeed && Crop2 == ECropTypeExtended::Tomatoes))
	{
		return ECropTypeExtended::Rice; // Exotic crop
	}
	
	// Return the more valuable crop
	float Value1 = GetCropMarketValue(Crop1);
	float Value2 = GetCropMarketValue(Crop2);
	
	return Value1 >= Value2 ? Crop1 : Crop2;
}

bool UCropSystem::CanBreedCrops(ECropTypeExtended Crop1, ECropTypeExtended Crop2) const
{
	// Can't breed a crop with itself
	if (Crop1 == Crop2)
	{
		return false;
	}
	
	// Special crops can't be bred (they're already results of breeding)
	if (Crop1 == ECropTypeExtended::QuantumPlants || Crop1 == ECropTypeExtended::Rice ||
		Crop2 == ECropTypeExtended::QuantumPlants || Crop2 == ECropTypeExtended::Rice)
	{
		return false;
	}
	
	// Basic crops can be bred
	TArray<ECropTypeExtended> BasicCrops = {
		ECropTypeExtended::Wheat, ECropTypeExtended::Corn, ECropTypeExtended::Tomatoes, 
		ECropTypeExtended::Potatoes, ECropTypeExtended::Carrots, ECropTypeExtended::Lettuce,
		ECropTypeExtended::Strawberries
	};
	
	return BasicCrops.Contains(Crop1) && BasicCrops.Contains(Crop2);
}

float UCropSystem::CalculateCropQuality(ECropTypeExtended CropType, ESoilQuality Soil, float GrowthTime) const
{
	FCropData CropData = GetCropData(CropType);
	
	// Base quality from soil
	float SoilQuality = 1.0f;
	switch (Soil)
	{
		case ESoilQuality::Pristine:
			SoilQuality = 1.5f;
			break;
		case ESoilQuality::Excellent:
			SoilQuality = 1.3f;
			break;
		case ESoilQuality::Good:
			SoilQuality = 1.1f;
			break;
		case ESoilQuality::Fair:
			SoilQuality = 0.9f;
			break;
		case ESoilQuality::Poor:
			SoilQuality = 0.7f;
			break;
	}
	
	// Growth time factor (optimal growth time produces best quality)
	float TimeFactor = 1.0f;
	float OptimalTime = CropData.BaseGrowthTime;
	float TimeDiff = FMath::Abs(GrowthTime - OptimalTime);
	
	if (TimeDiff < OptimalTime * 0.1f)
	{
		TimeFactor = 1.2f; // Grown at optimal time
	}
	else if (TimeDiff < OptimalTime * 0.3f)
	{
		TimeFactor = 1.0f; // Good timing
	}
	else if (TimeDiff < OptimalTime * 0.5f)
	{
		TimeFactor = 0.8f; // Too fast or too slow
	}
	else
	{
		TimeFactor = 0.6f; // Poor timing
	}
	
	float Quality = SoilQuality * TimeFactor;
	return FMath::Clamp(Quality, 0.0f, 2.0f);
}

FString UCropSystem::GetQualityDescription(float Quality) const
{
	if (Quality >= 1.5f)
	{
		return TEXT("Premium");
	}
	else if (Quality >= 1.2f)
	{
		return TEXT("Excellent");
	}
	else if (Quality >= 1.0f)
	{
		return TEXT("Good");
	}
	else if (Quality >= 0.8f)
	{
		return TEXT("Fair");
	}
	else
	{
		return TEXT("Poor");
	}
}

void UCropSystem::InitializeCropDatabase()
{
	// Clear existing data
	CropDatabase.Empty();
	CropDataMap.Empty();
	
	// Wheat
	FCropData Wheat;
	Wheat.CropType = ECropTypeExtended::Wheat;
	Wheat.CropName = TEXT("Wheat");
	Wheat.BaseGrowthTime = 60.0f;
	Wheat.WaterNeed = 0.5f;
	Wheat.NutrientNeed = 0.4f;
	Wheat.OptimalTemp = 20.0f;
	Wheat.TempTolerance = 10.0f;
	Wheat.BaseYield = 15;
	Wheat.MarketValue = 10.0f;
	CropDatabase.Add(Wheat);
	CropDataMap.Add(ECropTypeExtended::Wheat, Wheat);
	
	// Corn
	FCropData Corn;
	Corn.CropType = ECropTypeExtended::Corn;
	Corn.CropName = TEXT("Corn");
	Corn.BaseGrowthTime = 80.0f;
	Corn.WaterNeed = 0.7f;
	Corn.NutrientNeed = 0.6f;
	Corn.OptimalTemp = 25.0f;
	Corn.TempTolerance = 8.0f;
	Corn.BaseYield = 12;
	Corn.MarketValue = 15.0f;
	CropDatabase.Add(Corn);
	CropDataMap.Add(ECropTypeExtended::Corn, Corn);
	
	// Tomatoes
	FCropData Tomatoes;
	Tomatoes.CropType = ECropTypeExtended::Tomatoes;
	Tomatoes.CropName = TEXT("Tomatoes");
	Tomatoes.BaseGrowthTime = 45.0f;
	Tomatoes.WaterNeed = 0.8f;
	Tomatoes.NutrientNeed = 0.7f;
	Tomatoes.OptimalTemp = 22.0f;
	Tomatoes.TempTolerance = 5.0f;
	Tomatoes.BaseYield = 20;
	Tomatoes.MarketValue = 20.0f;
	CropDatabase.Add(Tomatoes);
	CropDataMap.Add(ECropTypeExtended::Tomatoes, Tomatoes);
	
	// Potatoes
	FCropData Potatoes;
	Potatoes.CropType = ECropTypeExtended::Potatoes;
	Potatoes.CropName = TEXT("Potatoes");
	Potatoes.BaseGrowthTime = 70.0f;
	Potatoes.WaterNeed = 0.6f;
	Potatoes.NutrientNeed = 0.5f;
	Potatoes.OptimalTemp = 18.0f;
	Potatoes.TempTolerance = 12.0f;
	Potatoes.BaseYield = 18;
	Potatoes.MarketValue = 12.0f;
	CropDatabase.Add(Potatoes);
	CropDataMap.Add(ECropTypeExtended::Potatoes, Potatoes);
	
	// Carrots
	FCropData Carrots;
	Carrots.CropType = ECropTypeExtended::Carrots;
	Carrots.CropName = TEXT("Carrots");
	Carrots.BaseGrowthTime = 55.0f;
	Carrots.WaterNeed = 0.5f;
	Carrots.NutrientNeed = 0.4f;
	Carrots.OptimalTemp = 16.0f;
	Carrots.TempTolerance = 10.0f;
	Carrots.BaseYield = 16;
	Carrots.MarketValue = 14.0f;
	CropDatabase.Add(Carrots);
	CropDataMap.Add(ECropTypeExtended::Carrots, Carrots);
	
	// Lettuce
	FCropData Lettuce;
	Lettuce.CropType = ECropTypeExtended::Lettuce;
	Lettuce.CropName = TEXT("Lettuce");
	Lettuce.BaseGrowthTime = 30.0f;
	Lettuce.WaterNeed = 0.7f;
	Lettuce.NutrientNeed = 0.3f;
	Lettuce.OptimalTemp = 15.0f;
	Lettuce.TempTolerance = 8.0f;
	Lettuce.BaseYield = 10;
	Lettuce.MarketValue = 8.0f;
	CropDatabase.Add(Lettuce);
	CropDataMap.Add(ECropTypeExtended::Lettuce, Lettuce);
	
	// Strawberries
	FCropData Strawberries;
	Strawberries.CropType = ECropTypeExtended::Strawberries;
	Strawberries.CropName = TEXT("Strawberries");
	Strawberries.BaseGrowthTime = 65.0f;
	Strawberries.WaterNeed = 0.6f;
	Strawberries.NutrientNeed = 0.5f;
	Strawberries.OptimalTemp = 19.0f;
	Strawberries.TempTolerance = 7.0f;
	Strawberries.BaseYield = 8;
	Strawberries.MarketValue = 25.0f;
	CropDatabase.Add(Strawberries);
	CropDataMap.Add(ECropTypeExtended::Strawberries, Strawberries);
	
	// Space Weed (special crop)
	FCropData SpaceWeed;
	SpaceWeed.CropType = ECropTypeExtended::SpaceWeed;
	SpaceWeed.CropName = TEXT("Space Weed");
	SpaceWeed.BaseGrowthTime = 25.0f;
	SpaceWeed.WaterNeed = 0.3f;
	SpaceWeed.NutrientNeed = 0.2f;
	SpaceWeed.OptimalTemp = 15.0f;
	SpaceWeed.TempTolerance = 20.0f;
	SpaceWeed.BaseYield = 8;
	SpaceWeed.MarketValue = 50.0f; // High value special crop
	CropDatabase.Add(SpaceWeed);
	CropDataMap.Add(ECropTypeExtended::SpaceWeed, SpaceWeed);
	
	// Moon Melons (special crop)
	FCropData Rice;
	Rice.CropType = ECropTypeExtended::Rice;
	Rice.CropName = TEXT("Moon Melons");
	Rice.BaseGrowthTime = 90.0f;
	Rice.WaterNeed = 0.4f;
	Rice.NutrientNeed = 0.8f;
	Rice.OptimalTemp = 12.0f;
	Rice.TempTolerance = 6.0f;
	Rice.BaseYield = 6;
	Rice.MarketValue = 100.0f; // Very high value
	CropDatabase.Add(Rice);
	CropDataMap.Add(ECropTypeExtended::Rice, Rice);
	
	// Quantum Potatoes (special crop)
	FCropData QuantumPlants;
	QuantumPlants.CropType = ECropTypeExtended::QuantumPlants;
	QuantumPlants.CropName = TEXT("Quantum Potatoes");
	QuantumPlants.BaseGrowthTime = 50.0f;
	QuantumPlants.WaterNeed = 0.6f;
	QuantumPlants.NutrientNeed = 0.9f;
	QuantumPlants.OptimalTemp = 20.0f;
	QuantumPlants.TempTolerance = 15.0f;
	QuantumPlants.BaseYield = 25;
	QuantumPlants.MarketValue = 75.0f; // High value
	CropDatabase.Add(QuantumPlants);
	CropDataMap.Add(ECropTypeExtended::QuantumPlants, QuantumPlants);
}