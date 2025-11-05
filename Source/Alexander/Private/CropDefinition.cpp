// Copyright Epic Games, Inc. All Rights Reserved.

#include "CropDefinition.h"

UCropDefinition::UCropDefinition()
{
	// Default values for a generic crop
	CropName = TEXT("Generic Crop");
	CropDescription = FText::FromString(TEXT("A basic crop"));
	CropIcon = nullptr;
	CropMesh = nullptr;
	CropMaterial = nullptr;

	// Growth defaults (10 minutes)
	GrowthDuration = 600.0f;
	OptimalTemperature = FCropFloatRange(15.0f, 25.0f);
	OptimalHumidity = FCropFloatRange(0.4f, 0.7f);
	WaterConsumption = 10.0f;
	SoilQualityRequirement = 0.5f;
	SunlightRequirement = 0.7f;

	// Biome defaults
	BiomeAdaptability = 0.5f;
	SuitableBiomes.Add(EBiomeType::Grassland);

	// Harvest defaults
	YieldAmount = FCropIntRange(5, 10);
	HarvestItemID = TEXT("generic_crop");
	SellPrice = 10.0f;
	RegrowthChance = 0.0f;

	// Special properties
	bRequiresFertilizer = false;
	bRequiresIrrigation = false;
	bResistantToPests = false;
	bResistantToDrought = false;
	FertilityDepletion = 0.05f;
}

bool UCropDefinition::IsSuitableForBiome(EBiomeType BiomeType) const
{
	return SuitableBiomes.Contains(BiomeType);
}

UStaticMesh* UCropDefinition::GetGrowthStageMesh(float GrowthProgress) const
{
	if (GrowthStageMeshes.Num() == 0)
	{
		return CropMesh;
	}

	// Map growth progress to stage index
	int32 StageIndex = FMath::FloorToInt(GrowthProgress * GrowthStageMeshes.Num());
	StageIndex = FMath::Clamp(StageIndex, 0, GrowthStageMeshes.Num() - 1);

	return GrowthStageMeshes[StageIndex];
}

float UCropDefinition::CalculateGrowthModifier(float Temperature, float Humidity, float Sunlight, float SoilQuality) const
{
	float Modifier = 1.0f;

	// Temperature factor
	if (!OptimalTemperature.Contains(Temperature))
	{
		float TempDeviation = FMath::Min(
			FMath::Abs(Temperature - OptimalTemperature.Min),
			FMath::Abs(Temperature - OptimalTemperature.Max)
		);
		// Reduce growth by 5% per degree outside optimal range
		Modifier *= FMath::Max(0.1f, 1.0f - (TempDeviation * 0.05f));
	}

	// Humidity factor
	if (!OptimalHumidity.Contains(Humidity))
	{
		float HumidDeviation = FMath::Min(
			FMath::Abs(Humidity - OptimalHumidity.Min),
			FMath::Abs(Humidity - OptimalHumidity.Max)
		);
		// Reduce growth proportionally to deviation
		Modifier *= FMath::Max(0.2f, 1.0f - HumidDeviation);
	}

	// Sunlight factor
	float SunlightFactor = Sunlight / FMath::Max(0.1f, SunlightRequirement);
	Modifier *= FMath::Clamp(SunlightFactor, 0.2f, 1.2f);

	// Soil quality factor
	if (SoilQuality < SoilQualityRequirement)
	{
		float QualityRatio = SoilQuality / SoilQualityRequirement;
		Modifier *= FMath::Max(0.3f, QualityRatio);
	}

	return FMath::Clamp(Modifier, 0.0f, 2.0f);
}

float UCropDefinition::GetExpectedHarvestValue() const
{
	float AverageYield = (YieldAmount.Min + YieldAmount.Max) * 0.5f;
	return AverageYield * SellPrice;
}

int32 UCropDefinition::GetNumGrowthStages() const
{
	return FMath::Max(1, GrowthStageMeshes.Num());
}

int32 UCropDefinition::GetGrowthStageIndex(float GrowthProgress) const
{
	if (GrowthStageMeshes.Num() == 0)
	{
		return 0;
	}

	// Map growth progress to stage index
	int32 StageIndex = FMath::FloorToInt(GrowthProgress * GrowthStageMeshes.Num());
	return FMath::Clamp(StageIndex, 0, GrowthStageMeshes.Num() - 1);
}

FString UCropDefinition::GetGrowthStageName(float GrowthProgress) const
{
	int32 StageIndex = GetGrowthStageIndex(GrowthProgress);
	int32 NumStages = GetNumGrowthStages();

	// Default stage names based on typical crop growth
	TArray<FString> StageNames = {
		TEXT("Seed"),
		TEXT("Sprout"),
		TEXT("Seedling"),
		TEXT("Vegetative"),
		TEXT("Budding"),
		TEXT("Flowering"),
		TEXT("Fruiting"),
		TEXT("Mature")
	};

	if (StageIndex < StageNames.Num())
	{
		return StageNames[StageIndex];
	}

	return FString::Printf(TEXT("Stage %d"), StageIndex + 1);
}
