// Copyright Epic Games, Inc. All Rights Reserved.

#include "BiomeCompatibilitySystem.h"
#include "CropDefinition.h"

UBiomeCompatibilitySystem::UBiomeCompatibilitySystem()
{
}

void UBiomeCompatibilitySystem::Initialize()
{
	SetupDefaultGrowthModifiers();
	SetupDefaultBiomeCropMappings();
}

// ============================================================================
// COMPATIBILITY CHECKS
// ============================================================================

bool UBiomeCompatibilitySystem::IsCropCompatibleWithBiome(const UCropDefinition* CropDef, EBiomeType BiomeType) const
{
	if (!CropDef)
	{
		return false;
	}

	// Check if crop explicitly lists this biome as suitable
	if (CropDef->SuitableBiomes.Contains(BiomeType))
	{
		return true;
	}

	// Check if crop has high adaptability
	if (CropDef->BiomeAdaptability >= 0.7f)
	{
		return true;
	}

	// Check if it's in any of our biome crop lists
	const FBiomeCropList* CropList = GetBiomeCropList(BiomeType);
	if (CropList)
	{
		// Check native crops
		for (const TSoftObjectPtr<UCropDefinition>& CropPtr : CropList->NativeCrops)
		{
			if (CropPtr.Get() == CropDef)
			{
				return true;
			}
		}

		// Check compatible crops
		for (const TSoftObjectPtr<UCropDefinition>& CropPtr : CropList->CompatibleCrops)
		{
			if (CropPtr.Get() == CropDef)
			{
				return true;
			}
		}

		// Check challenging crops
		for (const TSoftObjectPtr<UCropDefinition>& CropPtr : CropList->ChallengingCrops)
		{
			if (CropPtr.Get() == CropDef)
			{
				return true;
			}
		}
	}

	return false;
}

FBiomeCompatibilityScore UBiomeCompatibilitySystem::CalculateCompatibilityScore(
	const UCropDefinition* CropDef,
	EBiomeType BiomeType,
	float Temperature,
	float Humidity,
	float SoilQuality) const
{
	FBiomeCompatibilityScore Score;
	Score.BiomeType = BiomeType;

	if (!CropDef)
	{
		Score.CompatibilityScore = 0.0f;
		Score.GrowthRateModifier = 0.0f;
		Score.CompatibilityReason = TEXT("Invalid crop definition");
		return Score;
	}

	// Check if native crop
	Score.bIsNative = IsNativeCrop(CropDef, BiomeType);

	// Calculate individual compatibility factors
	float TempCompatibility = CalculateTemperatureCompatibility(CropDef, Temperature);
	float HumidityCompatibility = CalculateHumidityCompatibility(CropDef, Humidity);
	float SoilCompatibility = CalculateSoilCompatibility(CropDef, SoilQuality);

	// Calculate biome compatibility
	float BiomeCompatibility = 0.5f; // Default
	if (CropDef->SuitableBiomes.Contains(BiomeType))
	{
		BiomeCompatibility = 1.0f;
	}
	else if (Score.bIsNative)
	{
		BiomeCompatibility = 1.0f;
	}
	else
	{
		// Use adaptability as fallback
		BiomeCompatibility = CropDef->BiomeAdaptability;
	}

	// Overall compatibility is weighted average
	Score.CompatibilityScore = (
		BiomeCompatibility * 0.3f +
		TempCompatibility * 0.25f +
		HumidityCompatibility * 0.25f +
		SoilCompatibility * 0.2f
	);

	// Calculate growth rate modifier
	float BiomeModifier = GetBiomeGrowthModifier(CropDef, BiomeType);
	Score.GrowthRateModifier = BiomeModifier * Score.CompatibilityScore;
	Score.GrowthRateModifier = FMath::Clamp(Score.GrowthRateModifier, 0.3f, 1.5f);

	// Generate compatibility reason
	if (Score.CompatibilityScore >= 0.8f)
	{
		Score.CompatibilityReason = TEXT("Excellent conditions for growth");
	}
	else if (Score.CompatibilityScore >= 0.6f)
	{
		Score.CompatibilityReason = TEXT("Good conditions for growth");
	}
	else if (Score.CompatibilityScore >= 0.4f)
	{
		Score.CompatibilityReason = TEXT("Challenging but possible");
	}
	else
	{
		Score.CompatibilityReason = TEXT("Poor conditions - growth will be slow");
	}

	return Score;
}

float UBiomeCompatibilitySystem::GetBiomeGrowthModifier(const UCropDefinition* CropDef, EBiomeType BiomeType) const
{
	if (!CropDef)
	{
		return 0.5f;
	}

	// Check if native crop - gets best modifier
	if (IsNativeCrop(CropDef, BiomeType))
	{
		return 1.2f; // 20% bonus for native crops
	}

	// Check if in suitable biomes list
	if (CropDef->SuitableBiomes.Contains(BiomeType))
	{
		return 1.0f; // Normal growth rate
	}

	// Get base biome modifier
	const float* BaseModifier = BiomeBaseGrowthModifiers.Find(BiomeType);
	float BiomeModifier = BaseModifier ? *BaseModifier : 1.0f;

	// Apply crop adaptability
	float AdaptabilityFactor = FMath::Lerp(0.5f, 1.0f, CropDef->BiomeAdaptability);

	return BiomeModifier * AdaptabilityFactor;
}

// ============================================================================
// CROP LISTS
// ============================================================================

TArray<UCropDefinition*> UBiomeCompatibilitySystem::GetCropsForBiome(EBiomeType BiomeType) const
{
	TArray<UCropDefinition*> AllCrops;

	const FBiomeCropList* CropList = GetBiomeCropList(BiomeType);
	if (CropList)
	{
		// Add native crops
		for (const TSoftObjectPtr<UCropDefinition>& CropPtr : CropList->NativeCrops)
		{
			if (UCropDefinition* Crop = CropPtr.Get())
			{
				AllCrops.Add(Crop);
			}
		}

		// Add compatible crops
		for (const TSoftObjectPtr<UCropDefinition>& CropPtr : CropList->CompatibleCrops)
		{
			if (UCropDefinition* Crop = CropPtr.Get())
			{
				AllCrops.Add(Crop);
			}
		}

		// Add challenging crops
		for (const TSoftObjectPtr<UCropDefinition>& CropPtr : CropList->ChallengingCrops)
		{
			if (UCropDefinition* Crop = CropPtr.Get())
			{
				AllCrops.Add(Crop);
			}
		}
	}

	return AllCrops;
}

TArray<UCropDefinition*> UBiomeCompatibilitySystem::GetNativeCropsForBiome(EBiomeType BiomeType) const
{
	TArray<UCropDefinition*> NativeCrops;

	const FBiomeCropList* CropList = GetBiomeCropList(BiomeType);
	if (CropList)
	{
		for (const TSoftObjectPtr<UCropDefinition>& CropPtr : CropList->NativeCrops)
		{
			if (UCropDefinition* Crop = CropPtr.Get())
			{
				NativeCrops.Add(Crop);
			}
		}
	}

	return NativeCrops;
}

TArray<UCropDefinition*> UBiomeCompatibilitySystem::GetCompatibleCropsForBiome(EBiomeType BiomeType) const
{
	TArray<UCropDefinition*> CompatibleCrops;

	const FBiomeCropList* CropList = GetBiomeCropList(BiomeType);
	if (CropList)
	{
		for (const TSoftObjectPtr<UCropDefinition>& CropPtr : CropList->CompatibleCrops)
		{
			if (UCropDefinition* Crop = CropPtr.Get())
			{
				CompatibleCrops.Add(Crop);
			}
		}
	}

	return CompatibleCrops;
}

TArray<UCropDefinition*> UBiomeCompatibilitySystem::GetChallengingCropsForBiome(EBiomeType BiomeType) const
{
	TArray<UCropDefinition*> ChallengingCrops;

	const FBiomeCropList* CropList = GetBiomeCropList(BiomeType);
	if (CropList)
	{
		for (const TSoftObjectPtr<UCropDefinition>& CropPtr : CropList->ChallengingCrops)
		{
			if (UCropDefinition* Crop = CropPtr.Get())
			{
				ChallengingCrops.Add(Crop);
			}
		}
	}

	return ChallengingCrops;
}

// ============================================================================
// BIOME SUITABILITY
// ============================================================================

float UBiomeCompatibilitySystem::CalculateBiomeSuitability(
	EBiomeType BiomeType,
	float Temperature,
	float Humidity,
	float SoilQuality) const
{
	// Base suitability per biome type
	float BaseSuitability = 0.5f;

	switch (BiomeType)
	{
	case EBiomeType::Grassland:
		BaseSuitability = 0.9f; // Excellent for farming
		break;
	case EBiomeType::Forest:
		BaseSuitability = 0.7f; // Good but needs clearing
		break;
	case EBiomeType::Desert:
		BaseSuitability = 0.4f; // Challenging, needs irrigation
		break;
	case EBiomeType::Tundra:
		BaseSuitability = 0.3f; // Very challenging, cold
		break;
	case EBiomeType::Volcanic:
		BaseSuitability = 0.6f; // Fertile soil but harsh conditions
		break;
	case EBiomeType::Ocean:
		BaseSuitability = 0.1f; // Not suitable for traditional farming
		break;
	default:
		BaseSuitability = 0.5f;
		break;
	}

	// Adjust for environmental factors
	float TempFactor = 1.0f;
	if (Temperature < 5.0f || Temperature > 35.0f)
	{
		TempFactor = 0.5f; // Extreme temperatures reduce suitability
	}
	else if (Temperature >= 15.0f && Temperature <= 25.0f)
	{
		TempFactor = 1.2f; // Ideal temperature range
	}

	float HumidityFactor = FMath::Clamp(Humidity * 1.5f, 0.5f, 1.2f);
	float SoilFactor = FMath::Clamp(SoilQuality * 1.3f, 0.3f, 1.3f);

	float FinalSuitability = BaseSuitability * TempFactor * HumidityFactor * SoilFactor;
	return FMath::Clamp(FinalSuitability, 0.0f, 1.0f);
}

TArray<UCropDefinition*> UBiomeCompatibilitySystem::GetRecommendedCrops(
	EBiomeType BiomeType,
	float Temperature,
	float Humidity,
	float SoilQuality,
	int32 MaxResults) const
{
	TArray<UCropDefinition*> AllCrops = GetCropsForBiome(BiomeType);
	
	// Score each crop
	struct FCropScore
	{
		UCropDefinition* Crop;
		float Score;
	};

	TArray<FCropScore> ScoredCrops;
	for (UCropDefinition* Crop : AllCrops)
	{
		FBiomeCompatibilityScore CompatScore = CalculateCompatibilityScore(
			Crop, BiomeType, Temperature, Humidity, SoilQuality);
		
		FCropScore CropScore;
		CropScore.Crop = Crop;
		CropScore.Score = CompatScore.CompatibilityScore;
		ScoredCrops.Add(CropScore);
	}

	// Sort by score (highest first)
	ScoredCrops.Sort([](const FCropScore& A, const FCropScore& B) {
		return A.Score > B.Score;
	});

	// Return top N crops
	TArray<UCropDefinition*> RecommendedCrops;
	for (int32 i = 0; i < FMath::Min(MaxResults, ScoredCrops.Num()); ++i)
	{
		RecommendedCrops.Add(ScoredCrops[i].Crop);
	}

	return RecommendedCrops;
}

// ============================================================================
// INTERNAL METHODS
// ============================================================================

float UBiomeCompatibilitySystem::CalculateTemperatureCompatibility(const UCropDefinition* CropDef, float Temperature) const
{
	if (!CropDef)
	{
		return 0.0f;
	}

	// Check if within optimal range
	if (CropDef->OptimalTemperature.Contains(Temperature))
	{
		return 1.0f;
	}

	// Calculate deviation from optimal range
	float Deviation = FMath::Min(
		FMath::Abs(Temperature - CropDef->OptimalTemperature.Min),
		FMath::Abs(Temperature - CropDef->OptimalTemperature.Max)
	);

	// Reduce compatibility by 5% per degree outside optimal range
	float Compatibility = FMath::Max(0.0f, 1.0f - (Deviation * 0.05f));

	return Compatibility;
}

float UBiomeCompatibilitySystem::CalculateHumidityCompatibility(const UCropDefinition* CropDef, float Humidity) const
{
	if (!CropDef)
	{
		return 0.0f;
	}

	// Check if within optimal range
	if (CropDef->OptimalHumidity.Contains(Humidity))
	{
		return 1.0f;
	}

	// Calculate deviation from optimal range
	float Deviation = FMath::Min(
		FMath::Abs(Humidity - CropDef->OptimalHumidity.Min),
		FMath::Abs(Humidity - CropDef->OptimalHumidity.Max)
	);

	// Reduce compatibility proportionally to deviation
	float Compatibility = FMath::Max(0.0f, 1.0f - Deviation);

	return Compatibility;
}

float UBiomeCompatibilitySystem::CalculateSoilCompatibility(const UCropDefinition* CropDef, float SoilQuality) const
{
	if (!CropDef)
	{
		return 0.0f;
	}

	// If soil quality meets requirement, full compatibility
	if (SoilQuality >= CropDef->SoilQualityRequirement)
	{
		return 1.0f;
	}

	// Otherwise, scale based on how close we are to requirement
	float Ratio = SoilQuality / FMath::Max(0.1f, CropDef->SoilQualityRequirement);
	return FMath::Clamp(Ratio, 0.0f, 1.0f);
}

bool UBiomeCompatibilitySystem::IsNativeCrop(const UCropDefinition* CropDef, EBiomeType BiomeType) const
{
	if (!CropDef)
	{
		return false;
	}

	const FBiomeCropList* CropList = GetBiomeCropList(BiomeType);
	if (CropList)
	{
		for (const TSoftObjectPtr<UCropDefinition>& CropPtr : CropList->NativeCrops)
		{
			if (CropPtr.Get() == CropDef)
			{
				return true;
			}
		}
	}

	return false;
}

const FBiomeCropList* UBiomeCompatibilitySystem::GetBiomeCropList(EBiomeType BiomeType) const
{
	for (const FBiomeCropList& CropList : BiomeCropLists)
	{
		if (CropList.BiomeType == BiomeType)
		{
			return &CropList;
		}
	}

	return nullptr;
}

void UBiomeCompatibilitySystem::SetupDefaultBiomeCropMappings()
{
	// Note: This creates the structure for biome-crop mappings
	// Actual crop assets should be assigned in the editor or through data assets
	
	BiomeCropLists.Empty();

	// Grassland - Best for general farming
	FBiomeCropList GrasslandCrops;
	GrasslandCrops.BiomeType = EBiomeType::Grassland;
	// Native: Wheat, Corn, Soybeans (to be assigned in editor)
	// Compatible: Most vegetables and grains
	// Challenging: Tropical fruits, desert plants
	BiomeCropLists.Add(GrasslandCrops);

	// Forest - Good for shade-tolerant crops
	FBiomeCropList ForestCrops;
	ForestCrops.BiomeType = EBiomeType::Forest;
	// Native: Mushrooms, Berries, Root vegetables
	// Compatible: Leafy greens, herbs
	// Challenging: Sun-loving crops
	BiomeCropLists.Add(ForestCrops);

	// Desert - Requires irrigation, good for heat-tolerant crops
	FBiomeCropList DesertCrops;
	DesertCrops.BiomeType = EBiomeType::Desert;
	// Native: Cacti, Date palms, Agave
	// Compatible: Melons, peppers, drought-resistant grains
	// Challenging: Water-intensive crops
	BiomeCropLists.Add(DesertCrops);

	// Tundra - Cold-hardy crops only
	FBiomeCropList TundraCrops;
	TundraCrops.BiomeType = EBiomeType::Tundra;
	// Native: Hardy grains, root vegetables, berries
	// Compatible: Cold-season vegetables
	// Challenging: Warm-weather crops
	BiomeCropLists.Add(TundraCrops);

	// Volcanic - Fertile soil but harsh conditions
	FBiomeCropList VolcanicCrops;
	VolcanicCrops.BiomeType = EBiomeType::Volcanic;
	// Native: Heat-resistant plants, mineral-loving crops
	// Compatible: Various vegetables with good soil
	// Challenging: Delicate crops
	BiomeCropLists.Add(VolcanicCrops);

	// Ocean - Limited farming options (aquaculture)
	FBiomeCropList OceanCrops;
	OceanCrops.BiomeType = EBiomeType::Ocean;
	// Native: Seaweed, kelp, aquatic plants
	// Compatible: Floating gardens, hydroponics
	// Challenging: Traditional land crops
	BiomeCropLists.Add(OceanCrops);
}

void UBiomeCompatibilitySystem::SetupDefaultGrowthModifiers()
{
	BiomeBaseGrowthModifiers.Empty();

	// Growth rate modifiers per biome (1.0 = normal)
	BiomeBaseGrowthModifiers.Add(EBiomeType::Grassland, 1.0f);   // Ideal conditions
	BiomeBaseGrowthModifiers.Add(EBiomeType::Forest, 0.85f);     // Reduced sunlight
	BiomeBaseGrowthModifiers.Add(EBiomeType::Desert, 0.7f);      // Water scarcity
	BiomeBaseGrowthModifiers.Add(EBiomeType::Tundra, 0.6f);      // Cold temperatures
	BiomeBaseGrowthModifiers.Add(EBiomeType::Volcanic, 0.9f);    // Fertile but harsh
	BiomeBaseGrowthModifiers.Add(EBiomeType::Ocean, 0.5f);       // Very limited
}
