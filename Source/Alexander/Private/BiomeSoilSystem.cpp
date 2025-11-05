// Copyright Epic Games, Inc. All Rights Reserved.

#include "BiomeSoilSystem.h"
#include "Math/UnrealMathUtility.h"

UBiomeSoilSystem::UBiomeSoilSystem()
{
}

void UBiomeSoilSystem::Initialize(const TArray<FBiomeDefinition>& InBiomes)
{
	BiomeDefinitions = InBiomes;
	BuildBiomeSoilMap();
}

FSoilVariation UBiomeSoilSystem::GetSoilVariationForBiome(EBiomeType BiomeType) const
{
	if (const FSoilVariation* Found = BiomeSoilMap.Find(BiomeType))
	{
		return *Found;
	}

	// Return default if not found
	return CreateDefaultSoilVariation(BiomeType);
}

FSoilVariation UBiomeSoilSystem::GetSoilVariationFromDefinition(const FBiomeDefinition& BiomeDef) const
{
	FSoilVariation Variation;

	// Validate and clamp soil quality to 0.3-1.0 range (Requirement 12.2)
	Variation.SoilQuality = ValidateSoilQuality(BiomeDef.BaseSoilQuality);
	
	// Set soil color from biome
	Variation.SoilColor = BiomeDef.SoilColor;
	
	// Set texture properties
	Variation.Roughness = BiomeDef.SoilRoughness;
	Variation.Moisture = BiomeDef.SoilMoisture;
	Variation.Drainage = BiomeDef.SoilDrainage;
	
	// Set water table depth (Requirement 12.5)
	Variation.WaterTableDepth = BiomeDef.WaterTableDepth;
	
	// Set biome info
	Variation.BiomeType = BiomeDef.BiomeType;
	Variation.BiomeName = BiomeDef.BiomeName;

	return Variation;
}

FSoilVariation UBiomeSoilSystem::GetBlendedSoilVariation(const TMap<int32, float>& BiomeWeights, const TArray<FBiomeDefinition>& BiomeDefinitions) const
{
	FSoilVariation BlendedVariation;

	if (BiomeWeights.Num() == 0 || BiomeDefinitions.Num() == 0)
	{
		return BlendedVariation;
	}

	// Initialize blended values
	float TotalWeight = 0.0f;
	float BlendedQuality = 0.0f;
	FLinearColor BlendedColor = FLinearColor::Black;
	float BlendedRoughness = 0.0f;
	float BlendedMoisture = 0.0f;
	float BlendedDrainage = 0.0f;
	float BlendedWaterTable = 0.0f;

	// Blend all biome properties
	for (const auto& WeightPair : BiomeWeights)
	{
		int32 BiomeIndex = WeightPair.Key;
		float Weight = WeightPair.Value;

		if (BiomeIndex >= 0 && BiomeIndex < BiomeDefinitions.Num())
		{
			const FBiomeDefinition& BiomeDef = BiomeDefinitions[BiomeIndex];
			
			BlendedQuality += ValidateSoilQuality(BiomeDef.BaseSoilQuality) * Weight;
			BlendedColor += BiomeDef.SoilColor * Weight;
			BlendedRoughness += BiomeDef.SoilRoughness * Weight;
			BlendedMoisture += BiomeDef.SoilMoisture * Weight;
			BlendedDrainage += BiomeDef.SoilDrainage * Weight;
			BlendedWaterTable += BiomeDef.WaterTableDepth * Weight;
			
			TotalWeight += Weight;
		}
	}

	// Normalize if needed
	if (TotalWeight > 0.0f && !FMath::IsNearlyEqual(TotalWeight, 1.0f))
	{
		BlendedQuality /= TotalWeight;
		BlendedColor /= TotalWeight;
		BlendedRoughness /= TotalWeight;
		BlendedMoisture /= TotalWeight;
		BlendedDrainage /= TotalWeight;
		BlendedWaterTable /= TotalWeight;
	}

	// Set blended values
	BlendedVariation.SoilQuality = ValidateSoilQuality(BlendedQuality);
	BlendedVariation.SoilColor = BlendedColor;
	BlendedVariation.Roughness = FMath::Clamp(BlendedRoughness, 0.0f, 1.0f);
	BlendedVariation.Moisture = FMath::Clamp(BlendedMoisture, 0.0f, 1.0f);
	BlendedVariation.Drainage = FMath::Clamp(BlendedDrainage, 0.0f, 1.0f);
	BlendedVariation.WaterTableDepth = FMath::Max(0.0f, BlendedWaterTable);
	BlendedVariation.BiomeName = TEXT("Blended");

	return BlendedVariation;
}

float UBiomeSoilSystem::CalculateSoilQualityWithVariation(float BaseSoilQuality, int32 LocalVariationSeed) const
{
	// Validate base quality
	float ValidatedQuality = ValidateSoilQuality(BaseSoilQuality);
	
	// Add local variation using seed (±10% variation)
	FRandomStream RandomStream(LocalVariationSeed);
	float Variation = RandomStream.FRandRange(-0.1f, 0.1f);
	
	float ModifiedQuality = ValidatedQuality + Variation;
	
	// Ensure result stays within valid range
	return ValidateSoilQuality(ModifiedQuality);
}

FLinearColor UBiomeSoilSystem::GetModifiedSoilColor(FLinearColor BaseSoilColor, float MoistureLevel, float HealthLevel) const
{
	FLinearColor ModifiedColor = BaseSoilColor;
	
	// Darken soil when wet (moisture effect)
	float MoistureDarkening = FMath::Clamp(MoistureLevel, 0.0f, 1.0f) * 0.3f;
	ModifiedColor = ModifiedColor * (1.0f - MoistureDarkening);
	
	// Adjust color based on health
	// Unhealthy soil becomes more gray/desaturated
	float HealthFactor = FMath::Clamp(HealthLevel, 0.0f, 1.0f);
	if (HealthFactor < 1.0f)
	{
		FLinearColor GrayColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);
		float DesaturationAmount = (1.0f - HealthFactor) * 0.5f;
		ModifiedColor = FMath::Lerp(ModifiedColor, GrayColor, DesaturationAmount);
	}
	
	// Ensure alpha stays at 1
	ModifiedColor.A = 1.0f;
	
	return ModifiedColor;
}

float UBiomeSoilSystem::CalculateWaterAvailability(float WaterTableDepth, float RecentRainfall) const
{
	// Water availability decreases with depth
	// Shallow water table (0-5m) = high availability
	// Deep water table (>20m) = low availability
	float DepthFactor = 1.0f - FMath::Clamp(WaterTableDepth / 20.0f, 0.0f, 1.0f);
	
	// Recent rainfall increases availability
	// 10mm+ rainfall = significant boost
	float RainfallFactor = FMath::Clamp(RecentRainfall / 10.0f, 0.0f, 1.0f);
	
	// Combine factors (rainfall has more immediate impact)
	float Availability = (DepthFactor * 0.4f) + (RainfallFactor * 0.6f);
	
	return FMath::Clamp(Availability, 0.0f, 1.0f);
}

FString UBiomeSoilSystem::GetSoilTextureDescription(const FSoilVariation& SoilVar) const
{
	FString Description;
	
	// Describe based on roughness and drainage
	if (SoilVar.Roughness > 0.7f && SoilVar.Drainage > 0.6f)
	{
		Description = TEXT("Sandy");
	}
	else if (SoilVar.Roughness < 0.4f && SoilVar.Drainage < 0.4f)
	{
		Description = TEXT("Clay");
	}
	else if (SoilVar.Roughness >= 0.4f && SoilVar.Roughness <= 0.7f)
	{
		Description = TEXT("Loamy");
	}
	else
	{
		Description = TEXT("Mixed");
	}
	
	// Add moisture description
	if (SoilVar.Moisture > 0.7f)
	{
		Description += TEXT(", Wet");
	}
	else if (SoilVar.Moisture < 0.3f)
	{
		Description += TEXT(", Dry");
	}
	
	return Description;
}

float UBiomeSoilSystem::ValidateSoilQuality(float SoilQuality) const
{
	// Requirement 12.2: Soil quality range 0.3-1.0
	return FMath::Clamp(SoilQuality, 0.3f, 1.0f);
}

void UBiomeSoilSystem::BuildBiomeSoilMap()
{
	BiomeSoilMap.Empty();
	
	for (const FBiomeDefinition& BiomeDef : BiomeDefinitions)
	{
		FSoilVariation Variation = GetSoilVariationFromDefinition(BiomeDef);
		BiomeSoilMap.Add(BiomeDef.BiomeType, Variation);
	}
}

FSoilVariation UBiomeSoilSystem::CreateDefaultSoilVariation(EBiomeType BiomeType) const
{
	FSoilVariation Variation;
	Variation.BiomeType = BiomeType;
	
	// Set defaults based on biome type
	switch (BiomeType)
	{
	case EBiomeType::Desert:
		Variation.SoilQuality = 0.3f; // Poor quality
		Variation.SoilColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f); // Sandy tan
		Variation.Roughness = 0.9f; // Very sandy
		Variation.Moisture = 0.1f; // Very dry
		Variation.Drainage = 0.9f; // Fast drainage
		Variation.WaterTableDepth = 50.0f; // Deep water table
		Variation.BiomeName = TEXT("Desert");
		break;
		
	case EBiomeType::Grassland:
		Variation.SoilQuality = 0.7f; // Good quality
		Variation.SoilColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f); // Rich brown
		Variation.Roughness = 0.5f; // Loamy
		Variation.Moisture = 0.5f; // Moderate
		Variation.Drainage = 0.5f; // Moderate drainage
		Variation.WaterTableDepth = 10.0f; // Moderate depth
		Variation.BiomeName = TEXT("Grassland");
		break;
		
	case EBiomeType::Forest:
		Variation.SoilQuality = 0.8f; // High quality
		Variation.SoilColor = FLinearColor(0.3f, 0.25f, 0.15f, 1.0f); // Dark brown
		Variation.Roughness = 0.4f; // Rich loam
		Variation.Moisture = 0.7f; // Moist
		Variation.Drainage = 0.4f; // Slow drainage
		Variation.WaterTableDepth = 5.0f; // Shallow water table
		Variation.BiomeName = TEXT("Forest");
		break;
		
	case EBiomeType::Tundra:
		Variation.SoilQuality = 0.4f; // Poor quality
		Variation.SoilColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f); // Gray
		Variation.Roughness = 0.6f; // Rocky
		Variation.Moisture = 0.6f; // Moist from permafrost
		Variation.Drainage = 0.2f; // Very slow drainage
		Variation.WaterTableDepth = 2.0f; // Shallow (permafrost)
		Variation.BiomeName = TEXT("Tundra");
		break;
		
	case EBiomeType::Volcanic:
		Variation.SoilQuality = 0.9f; // Excellent quality (volcanic soil)
		Variation.SoilColor = FLinearColor(0.2f, 0.15f, 0.1f, 1.0f); // Very dark brown/black
		Variation.Roughness = 0.7f; // Ashy/rocky
		Variation.Moisture = 0.4f; // Moderate
		Variation.Drainage = 0.7f; // Good drainage
		Variation.WaterTableDepth = 15.0f; // Variable depth
		Variation.BiomeName = TEXT("Volcanic");
		break;
		
	case EBiomeType::Ocean:
		Variation.SoilQuality = 0.3f; // Not suitable for farming
		Variation.SoilColor = FLinearColor(0.6f, 0.6f, 0.5f, 1.0f); // Wet sand/silt
		Variation.Roughness = 0.3f; // Silty
		Variation.Moisture = 1.0f; // Saturated
		Variation.Drainage = 0.1f; // Very poor drainage
		Variation.WaterTableDepth = 0.0f; // At surface
		Variation.BiomeName = TEXT("Ocean");
		break;
		
	case EBiomeType::Savanna:
		Variation.SoilQuality = 0.6f; // Moderate quality
		Variation.SoilColor = FLinearColor(0.6f, 0.5f, 0.3f, 1.0f); // Reddish brown
		Variation.Roughness = 0.7f; // Sandy loam
		Variation.Moisture = 0.3f; // Dry
		Variation.Drainage = 0.6f; // Good drainage
		Variation.WaterTableDepth = 20.0f; // Deep
		Variation.BiomeName = TEXT("Savanna");
		break;
		
	case EBiomeType::Rainforest:
		Variation.SoilQuality = 0.7f; // Good quality
		Variation.SoilColor = FLinearColor(0.35f, 0.25f, 0.15f, 1.0f); // Rich dark brown
		Variation.Roughness = 0.3f; // Clay-rich
		Variation.Moisture = 0.9f; // Very moist
		Variation.Drainage = 0.3f; // Poor drainage
		Variation.WaterTableDepth = 3.0f; // Very shallow
		Variation.BiomeName = TEXT("Rainforest");
		break;
		
	case EBiomeType::Alpine:
		Variation.SoilQuality = 0.4f; // Poor quality
		Variation.SoilColor = FLinearColor(0.5f, 0.45f, 0.4f, 1.0f); // Light brown/gray
		Variation.Roughness = 0.8f; // Rocky
		Variation.Moisture = 0.5f; // Moderate
		Variation.Drainage = 0.8f; // Fast drainage (rocky)
		Variation.WaterTableDepth = 30.0f; // Deep (mountainous)
		Variation.BiomeName = TEXT("Alpine");
		break;
		
	case EBiomeType::Swamp:
		Variation.SoilQuality = 0.5f; // Moderate quality
		Variation.SoilColor = FLinearColor(0.25f, 0.25f, 0.2f, 1.0f); // Dark muddy brown
		Variation.Roughness = 0.2f; // Very fine/muddy
		Variation.Moisture = 1.0f; // Saturated
		Variation.Drainage = 0.1f; // Very poor drainage
		Variation.WaterTableDepth = 0.5f; // At or near surface
		Variation.BiomeName = TEXT("Swamp");
		break;
		
	default:
		// Generic default
		Variation.SoilQuality = 0.5f;
		Variation.SoilColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
		Variation.Roughness = 0.5f;
		Variation.Moisture = 0.5f;
		Variation.Drainage = 0.5f;
		Variation.WaterTableDepth = 10.0f;
		Variation.BiomeName = TEXT("Unknown");
		break;
	}
	
	return Variation;
}

FLinearColor UBiomeSoilSystem::BlendSoilColors(FLinearColor ColorA, FLinearColor ColorB, float Weight) const
{
	return FMath::Lerp(ColorA, ColorB, Weight);
}
