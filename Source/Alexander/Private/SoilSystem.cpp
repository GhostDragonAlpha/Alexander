// Copyright Epic Games, Inc. All Rights Reserved.

#include "SoilSystem.h"
#include "Math/UnrealMathUtility.h"  // For FMath functions
#include "CropSystem.h"

USoilSystem::USoilSystem()
{
}

FSoilComposition USoilSystem::AnalyzeSoil(FVector Location) const
{
	// Generate soil composition based on location (simplified)
	// In a real implementation, this would sample from terrain data
	
	FSoilComposition Soil;
	
	// Use location to generate consistent but varied soil
	float NoiseX = FMath::PerlinNoise3D(FVector(Location.X * 0.001f, Location.Y * 0.001f, Location.Z * 0.001f));
	float NoiseY = FMath::PerlinNoise3D(FVector(Location.X * 0.002f, Location.Y * 0.002f, Location.Z * 0.002f));
	
	// Base composition
	Soil.ClayContent = FMath::Clamp(0.3f + NoiseX * 0.2f, 0.1f, 0.6f);
	Soil.SandContent = FMath::Clamp(0.4f + NoiseY * 0.3f, 0.1f, 0.7f);
	Soil.SiltContent = FMath::Clamp(1.0f - Soil.ClayContent - Soil.SandContent, 0.1f, 0.5f);
	
	// Organic matter and nutrients
	Soil.OrganicMatter = FMath::Clamp(0.05f + FMath::FRand() * 0.1f, 0.02f, 0.15f);
	Soil.pH = FMath::Clamp(6.5f + FMath::FRand() * 2.0f, 5.5f, 8.5f);
	Soil.Nitrogen = FMath::Clamp(0.02f + FMath::FRand() * 0.03f, 0.01f, 0.05f);
	Soil.Phosphorus = FMath::Clamp(0.01f + FMath::FRand() * 0.02f, 0.005f, 0.03f);
	Soil.Potassium = FMath::Clamp(0.015f + FMath::FRand() * 0.025f, 0.01f, 0.04f);
	
	return Soil;
}

ESoilQuality USoilSystem::EvaluateSoilQuality(const FSoilComposition& Composition) const
{
	float Structure = CalculateSoilStructure(Composition);
	float NutrientBalance = CalculateNutrientBalance(Composition);
	float pHOptimal = CalculateOptimalpH(Composition);
	float Fertility = GetSoilFertility(Composition);
	
	float OverallScore = (Structure + NutrientBalance + pHOptimal + Fertility) / 4.0f;
	
	if (OverallScore >= 0.9f)
	{
		return ESoilQuality::Pristine;
	}
	else if (OverallScore >= 0.75f)
	{
		return ESoilQuality::Excellent;
	}
	else if (OverallScore >= 0.6f)
	{
		return ESoilQuality::Good;
	}
	else if (OverallScore >= 0.4f)
	{
		return ESoilQuality::Fair;
	}
	else
	{
		return ESoilQuality::Poor;
	}
}

float USoilSystem::GetSoilFertility(const FSoilComposition& Composition) const
{
	// Calculate fertility based on nutrient levels and organic matter
	float NitrogenScore = FMath::Clamp(Composition.Nitrogen / 0.04f, 0.0f, 1.0f);
	float PhosphorusScore = FMath::Clamp(Composition.Phosphorus / 0.03f, 0.0f, 1.0f);
	float PotassiumScore = FMath::Clamp(Composition.Potassium / 0.04f, 0.0f, 1.0f);
	float OrganicScore = FMath::Clamp(Composition.OrganicMatter / 0.15f, 0.0f, 1.0f);
	
	return (NitrogenScore + PhosphorusScore + PotassiumScore + OrganicScore) / 4.0f;
}

float USoilSystem::GetWaterRetention(const FSoilComposition& Composition) const
{
	// Clay retains water well, sand drains quickly, silt is moderate
	float ClayRetention = Composition.ClayContent * 0.8f;
	float SandRetention = Composition.SandContent * 0.2f;
	float SiltRetention = Composition.SiltContent * 0.5f;
	float OrganicRetention = Composition.OrganicMatter * 2.0f; // Organic matter greatly improves retention
	
	return FMath::Clamp(ClayRetention + SandRetention + SiltRetention + OrganicRetention, 0.1f, 0.9f);
}

FSoilComposition USoilSystem::ApplyFertilizer(const FSoilComposition& Soil, float Nitrogen, float Phosphorus, float Potassium)
{
	FSoilComposition ModifiedSoil = Soil;
	
	ModifiedSoil.Nitrogen = FMath::Clamp(Soil.Nitrogen + Nitrogen, 0.0f, 0.1f);
	ModifiedSoil.Phosphorus = FMath::Clamp(Soil.Phosphorus + Phosphorus, 0.0f, 0.08f);
	ModifiedSoil.Potassium = FMath::Clamp(Soil.Potassium + Potassium, 0.0f, 0.1f);
	
	return ModifiedSoil;
}

FSoilComposition USoilSystem::ApplyCompost(const FSoilComposition& Soil, float Amount)
{
	FSoilComposition ModifiedSoil = Soil;
	
	// Compost adds organic matter and small amounts of nutrients
	ModifiedSoil.OrganicMatter = FMath::Clamp(Soil.OrganicMatter + Amount * 0.5f, 0.0f, 0.3f);
	ModifiedSoil.Nitrogen = FMath::Clamp(Soil.Nitrogen + Amount * 0.01f, 0.0f, 0.1f);
	ModifiedSoil.Phosphorus = FMath::Clamp(Soil.Phosphorus + Amount * 0.005f, 0.0f, 0.08f);
	ModifiedSoil.Potassium = FMath::Clamp(Soil.Potassium + Amount * 0.008f, 0.0f, 0.1f);
	
	// Compost also helps buffer pH
	float pHAdjustment = (7.0f - Soil.pH) * 0.1f * Amount;
	ModifiedSoil.pH = FMath::Clamp(Soil.pH + pHAdjustment, 5.0f, 9.0f);
	
	return ModifiedSoil;
}

FSoilComposition USoilSystem::AdjustpH(const FSoilComposition& Soil, float TargetpH)
{
	FSoilComposition ModifiedSoil = Soil;
	
	float pHDifference = TargetpH - Soil.pH;
	float AdjustmentRate = 0.1f; // Gradual adjustment
	
	ModifiedSoil.pH = FMath::Clamp(Soil.pH + pHDifference * AdjustmentRate, 5.0f, 9.0f);
	
	return ModifiedSoil;
}

FSoilComposition USoilSystem::TillSoil(const FSoilComposition& Soil)
{
	FSoilComposition TilledSoil = Soil;
	
	// Tilling aerates soil and mixes organic matter
	TilledSoil.OrganicMatter = FMath::Clamp(Soil.OrganicMatter * 1.1f, 0.0f, 0.3f);
	
	// Slight nutrient release from microbial activity
	TilledSoil.Nitrogen = FMath::Clamp(Soil.Nitrogen * 1.05f, 0.0f, 0.1f);
	TilledSoil.Phosphorus = FMath::Clamp(Soil.Phosphorus * 1.02f, 0.0f, 0.08f);
	
	// Improve soil structure
	float TotalParticles = Soil.ClayContent + Soil.SandContent + Soil.SiltContent;
	if (TotalParticles > 0.0f)
	{
		// Move towards more balanced composition
		float TargetRatio = 1.0f / 3.0f;
		TilledSoil.ClayContent = FMath::Lerp(Soil.ClayContent, TargetRatio, 0.1f);
		TilledSoil.SandContent = FMath::Lerp(Soil.SandContent, TargetRatio, 0.1f);
		TilledSoil.SiltContent = FMath::Lerp(Soil.SiltContent, TargetRatio, 0.1f);
	}
	
	return TilledSoil;
}

FSoilComposition USoilSystem::SimulateErosion(const FSoilComposition& Soil, float RainfallIntensity, float Slope)
{
	FSoilComposition ErodedSoil = Soil;
	
	// Erosion affects topsoil (organic matter and nutrients)
	float ErosionFactor = RainfallIntensity * Slope * 0.001f;
	
	ErodedSoil.OrganicMatter = FMath::Max(0.0f, Soil.OrganicMatter - ErosionFactor * 0.5f);
	ErodedSoil.Nitrogen = FMath::Max(0.0f, Soil.Nitrogen - ErosionFactor * 0.1f);
	ErodedSoil.Phosphorus = FMath::Max(0.0f, Soil.Phosphorus - ErosionFactor * 0.05f);
	ErodedSoil.Potassium = FMath::Max(0.0f, Soil.Potassium - ErosionFactor * 0.08f);
	
	// Erosion preferentially removes smaller particles (silt and clay)
	float SiltLoss = ErosionFactor * 0.3f;
	float ClayLoss = ErosionFactor * 0.2f;
	
	ErodedSoil.SiltContent = FMath::Max(0.1f, Soil.SiltContent - SiltLoss);
	ErodedSoil.ClayContent = FMath::Max(0.1f, Soil.ClayContent - ClayLoss);
	
	// Sand content increases proportionally
	float TotalRemaining = ErodedSoil.ClayContent + ErodedSoil.SandContent + ErodedSoil.SiltContent;
	if (TotalRemaining > 0.0f)
	{
		ErodedSoil.SandContent = 1.0f - ErodedSoil.ClayContent - ErodedSoil.SiltContent;
	}
	
	return ErodedSoil;
}

FSoilComposition USoilSystem::SimulateNutrientDepletion(const FSoilComposition& Soil, float CropConsumption)
{
	FSoilComposition DepletedSoil = Soil;
	
	// Crops consume nutrients in different ratios
	float NitrogenConsumption = CropConsumption * 0.4f;
	float PhosphorusConsumption = CropConsumption * 0.2f;
	float PotassiumConsumption = CropConsumption * 0.3f;
	
	DepletedSoil.Nitrogen = FMath::Max(0.0f, Soil.Nitrogen - NitrogenConsumption);
	DepletedSoil.Phosphorus = FMath::Max(0.0f, Soil.Phosphorus - PhosphorusConsumption);
	DepletedSoil.Potassium = FMath::Max(0.0f, Soil.Potassium - PotassiumConsumption);
	
	// Slight organic matter depletion
	DepletedSoil.OrganicMatter = FMath::Max(0.0f, Soil.OrganicMatter - CropConsumption * 0.05f);
	
	return DepletedSoil;
}

FSoilComposition USoilSystem::SimulateCompaction(const FSoilComposition& Soil, float Traffic)
{
	FSoilComposition CompactedSoil = Soil;
	
	// Compaction reduces soil structure and organic matter effectiveness
	float CompactionFactor = FMath::Clamp(Traffic * 0.01f, 0.0f, 0.3f);
	
	CompactedSoil.OrganicMatter = FMath::Max(0.0f, Soil.OrganicMatter * (1.0f - CompactionFactor));
	
	// Compaction can affect nutrient availability
	CompactedSoil.Nitrogen = Soil.Nitrogen * (1.0f - CompactionFactor * 0.5f);
	CompactedSoil.Phosphorus = Soil.Phosphorus * (1.0f - CompactionFactor * 0.3f);
	
	return CompactedSoil;
}

TArray<FString> USoilSystem::GetSoilRecommendations(const FSoilComposition& Soil, ECropType CropType)
{
	TArray<FString> Recommendations;
	
	// pH recommendations
	if (Soil.pH < 6.0f)
	{
		Recommendations.Add(TEXT("Add lime to raise pH"));
	}
	else if (Soil.pH > 7.5f)
	{
		Recommendations.Add(TEXT("Add sulfur to lower pH"));
	}
	
	// Nutrient recommendations
	if (Soil.Nitrogen < 0.02f)
	{
		Recommendations.Add(TEXT("Apply nitrogen-rich fertilizer"));
	}
	if (Soil.Phosphorus < 0.01f)
	{
		Recommendations.Add(TEXT("Add phosphorus supplement"));
	}
	if (Soil.Potassium < 0.015f)
	{
		Recommendations.Add(TEXT("Apply potassium-rich fertilizer"));
	}
	
	// Organic matter recommendations
	if (Soil.OrganicMatter < 0.05f)
	{
		Recommendations.Add(TEXT("Add compost or organic matter"));
	}
	
	// Soil structure recommendations
	float Structure = CalculateSoilStructure(Soil);
	if (Structure < 0.5f)
	{
		Recommendations.Add(TEXT("Till soil to improve structure"));
	}
	
	// Crop-specific recommendations
	float Suitability = GetCropSuitability(Soil, CropType);
	if (Suitability < 0.6f)
	{
		Recommendations.Add(TEXT("Consider a different crop for this soil"));
	}
	
	if (Recommendations.Num() == 0)
	{
		Recommendations.Add(TEXT("Soil is in good condition"));
	}
	
	return Recommendations;
}

float USoilSystem::GetCropSuitability(const FSoilComposition& Soil, ECropType CropType) const
{
	// Get crop requirements (simplified)
	float RequiredpH = 6.5f;
	float RequiredNitrogen = 0.03f;
	float RequiredPhosphorus = 0.02f;
	float RequiredPotassium = 0.025f;
	
	// Adjust requirements based on crop type
	switch (CropType)
	{
		case ECropType::Corn:
			RequiredNitrogen = 0.04f;
			break;
		case ECropType::Tomatoes:
			RequiredpH = 6.2f;
			RequiredPhosphorus = 0.025f;
			break;
		case ECropType::Potatoes:
			RequiredpH = 5.5f;
			RequiredPotassium = 0.03f;
			break;
		case ECropType::SpaceWeed:
			// Space weed is very adaptable
			return 0.9f;
		default:
			break;
	}
	
	// Calculate suitability scores
	float pHScore = 1.0f - FMath::Abs(Soil.pH - RequiredpH) / 3.0f;
	float NitrogenScore = FMath::Clamp(Soil.Nitrogen / RequiredNitrogen, 0.0f, 1.0f);
	float PhosphorusScore = FMath::Clamp(Soil.Phosphorus / RequiredPhosphorus, 0.0f, 1.0f);
	float PotassiumScore = FMath::Clamp(Soil.Potassium / RequiredPotassium, 0.0f, 1.0f);
	float StructureScore = CalculateSoilStructure(Soil);
	
	return (pHScore + NitrogenScore + PhosphorusScore + PotassiumScore + StructureScore) / 5.0f;
}

FString USoilSystem::GetSoilType(const FSoilComposition& Composition) const
{
	float Clay = Composition.ClayContent;
	float Sand = Composition.SandContent;
	float Silt = Composition.SiltContent;
	
	// USDA soil classification
	if (Clay > 0.4f)
	{
		return TEXT("Clay");
	}
	else if (Sand > 0.7f)
	{
		return TEXT("Sand");
	}
	else if (Silt > 0.5f)
	{
		return TEXT("Silt");
	}
	else if (Clay > 0.27f && Sand < 0.45f)
	{
		return TEXT("Clay Loam");
	}
	else if (Sand < 0.52f && Silt > 0.28f)
	{
		return TEXT("Silt Loam");
	}
	else if (Sand < 0.7f && Clay < 0.2f && Silt < 0.5f)
	{
		return TEXT("Loam");
	}
	else if (Sand > 0.43f && Clay < 0.2f)
	{
		return TEXT("Sandy Loam");
	}
	else if (Sand > 0.52f && Clay > 0.07f)
	{
		return TEXT("Sandy Clay Loam");
	}
	else
	{
		return TEXT("Loamy Sand");
	}
}

bool USoilSystem::IsSuitableForFarming(const FSoilComposition& Composition) const
{
	// Check basic requirements for farming
	if (Composition.pH < 5.0f || Composition.pH > 8.5f)
	{
		return false; // pH too extreme
	}
	
	if (Composition.OrganicMatter < 0.01f)
	{
		return false; // Not enough organic matter
	}
	
	float Structure = CalculateSoilStructure(Composition);
	if (Structure < 0.3f)
	{
		return false; // Poor soil structure
	}
	
	// Check for extreme compositions
	if (Composition.ClayContent > 0.8f || Composition.SandContent > 0.9f)
	{
		return false; // Too extreme for most crops
	}
	
	return true;
}

float USoilSystem::CalculateSoilStructure(const FSoilComposition& Composition) const
{
	// Ideal loam has roughly equal parts sand, silt, and clay
	float IdealRatio = 1.0f / 3.0f;
	
	float ClayDeviation = FMath::Abs(Composition.ClayContent - IdealRatio);
	float SandDeviation = FMath::Abs(Composition.SandContent - IdealRatio);
	float SiltDeviation = FMath::Abs(Composition.SiltContent - IdealRatio);
	
	float TotalDeviation = ClayDeviation + SandDeviation + SiltDeviation;
	float StructureScore = FMath::Max(0.0f, 1.0f - TotalDeviation);
	
	// Organic matter improves structure
	float OrganicBonus = FMath::Clamp(Composition.OrganicMatter * 5.0f, 0.0f, 0.3f);
	
	return FMath::Clamp(StructureScore + OrganicBonus, 0.0f, 1.0f);
}

float USoilSystem::CalculateNutrientBalance(const FSoilComposition& Composition) const
{
	// Calculate NPK balance
	float TotalNutrients = Composition.Nitrogen + Composition.Phosphorus + Composition.Potassium;
	
	if (TotalNutrients == 0.0f)
	{
		return 0.0f;
	}
	
	// Ideal ratios (simplified)
	float IdealNitrogenRatio = 0.4f;
	float IdealPhosphorusRatio = 0.3f;
	float IdealPotassiumRatio = 0.3f;
	
	float ActualNitrogenRatio = Composition.Nitrogen / TotalNutrients;
	float ActualPhosphorusRatio = Composition.Phosphorus / TotalNutrients;
	float ActualPotassiumRatio = Composition.Potassium / TotalNutrients;
	
	float NitrogenBalance = 1.0f - FMath::Abs(ActualNitrogenRatio - IdealNitrogenRatio);
	float PhosphorusBalance = 1.0f - FMath::Abs(ActualPhosphorusRatio - IdealPhosphorusRatio);
	float PotassiumBalance = 1.0f - FMath::Abs(ActualPotassiumRatio - IdealPotassiumRatio);
	
	return (NitrogenBalance + PhosphorusBalance + PotassiumBalance) / 3.0f;
}

float USoilSystem::CalculateOptimalpH(const FSoilComposition& Composition) const
{
	// Most crops prefer pH between 6.0 and 7.0
	float OptimalMin = 6.0f;
	float OptimalMax = 7.0f;
	
	if (Composition.pH >= OptimalMin && Composition.pH <= OptimalMax)
	{
		return 1.0f;
	}
	else if (Composition.pH < OptimalMin)
	{
		return FMath::Clamp(Composition.pH / OptimalMin, 0.0f, 1.0f);
	}
	else
	{
		return FMath::Clamp((9.0f - Composition.pH) / (9.0f - OptimalMax), 0.0f, 1.0f);
	}
}