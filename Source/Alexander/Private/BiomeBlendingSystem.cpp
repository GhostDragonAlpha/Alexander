// Copyright Epic Games, Inc. All Rights Reserved.

#include "BiomeBlendingSystem.h"
#include "BiomeManager.h"
#include "Kismet/KismetMathLibrary.h"

UBiomeBlendingSystem::UBiomeBlendingSystem()
{
	BiomeManager = nullptr;
}

void UBiomeBlendingSystem::Initialize(UBiomeManager* InBiomeManager)
{
	BiomeManager = InBiomeManager;
	
	if (BiomeManager)
	{
		UE_LOG(LogTemp, Log, TEXT("BiomeBlendingSystem initialized"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("BiomeBlendingSystem initialized with null BiomeManager"));
	}
}

FBlendedTerrainParameters UBiomeBlendingSystem::GetBlendedParameters(FVector WorldPosition) const
{
	FBlendedTerrainParameters Result;

	if (!BiomeManager)
	{
		return Result;
	}

	// Get smoothed biome weights
	FBiomeWeights Weights = CalculateSmoothedWeights(WorldPosition);

	// Blend parameters from all contributing biomes
	float TotalElevation = 0.0f;
	float TotalRoughness = 0.0f;
	float TotalTemperature = 0.0f;
	float TotalHumidity = 0.0f;
	float TotalVegetationDensity = 0.0f;

	for (const auto& Pair : Weights.Weights)
	{
		int32 BiomeIndex = Pair.Key;
		float Weight = Pair.Value;

		const FBiomeDefinition& Biome = BiomeManager->GetBiomeDefinition(BiomeIndex);

		TotalElevation += Biome.BaseElevation * Weight;
		TotalRoughness += Biome.Roughness * Weight;
		TotalTemperature += Biome.Temperature * Weight;
		TotalHumidity += Biome.Humidity * Weight;
		TotalVegetationDensity += Biome.VegetationDensity * Weight;
	}

	Result.Elevation = TotalElevation;
	Result.Roughness = TotalRoughness;
	Result.Temperature = TotalTemperature;
	Result.Humidity = TotalHumidity;
	Result.VegetationDensity = TotalVegetationDensity;
	Result.BiomeWeights = Weights.Weights;

	return Result;
}

FBiomeWeights UBiomeBlendingSystem::CalculateSmoothedWeights(FVector WorldPosition) const
{
	FBiomeWeights Result;

	if (!BiomeManager)
	{
		return Result;
	}

	// Get base biome weights
	Result = BiomeManager->GetBiomeWeightsAtLocation(WorldPosition);

	// Apply transition smoothing
	ApplyTransitionSmoothing(Result, WorldPosition);

	// Filter out biomes below minimum weight threshold
	TArray<int32> BiomesToRemove;
	for (const auto& Pair : Result.Weights)
	{
		if (Pair.Value < TransitionConfig.MinimumBiomeWeight)
		{
			BiomesToRemove.Add(Pair.Key);
		}
	}

	for (int32 BiomeIndex : BiomesToRemove)
	{
		Result.Weights.Remove(BiomeIndex);
	}

	// Re-normalize after filtering
	Result.Normalize();

	return Result;
}

bool UBiomeBlendingSystem::IsInTransitionZone(FVector WorldPosition) const
{
	FBiomeWeights Weights = CalculateSmoothedWeights(WorldPosition);
	
	// If more than one biome has significant weight, we're in a transition zone
	int32 SignificantBiomes = 0;
	for (const auto& Pair : Weights.Weights)
	{
		if (Pair.Value > 0.2f) // 20% threshold for "significant"
		{
			SignificantBiomes++;
		}
	}

	return SignificantBiomes > 1;
}

float UBiomeBlendingSystem::GetTransitionFactor(FVector WorldPosition) const
{
	FBiomeWeights Weights = CalculateSmoothedWeights(WorldPosition);

	if (Weights.Weights.Num() == 0)
	{
		return 0.0f;
	}

	// Calculate entropy of weight distribution
	// Higher entropy = more blending
	float Entropy = 0.0f;
	for (const auto& Pair : Weights.Weights)
	{
		if (Pair.Value > 0.0f)
		{
			Entropy -= Pair.Value * FMath::Loge(Pair.Value);
		}
	}

	// Normalize entropy to 0-1 range
	// Maximum entropy for N biomes is ln(N)
	float MaxEntropy = FMath::Loge(static_cast<float>(Weights.Weights.Num()));
	float NormalizedEntropy = MaxEntropy > 0.0f ? Entropy / MaxEntropy : 0.0f;

	return FMath::Clamp(NormalizedEntropy, 0.0f, 1.0f);
}

TArray<FVegetationDefinition> UBiomeBlendingSystem::MixVegetationTypes(const FBiomeWeights& BiomeWeights) const
{
	TArray<FVegetationDefinition> MixedVegetation;

	if (!BiomeManager)
	{
		return MixedVegetation;
	}

	// Collect vegetation from all contributing biomes
	for (const auto& Pair : BiomeWeights.Weights)
	{
		int32 BiomeIndex = Pair.Key;
		float Weight = Pair.Value;

		const FBiomeDefinition& Biome = BiomeManager->GetBiomeDefinition(BiomeIndex);

		// Add vegetation types with adjusted spawn probability
		for (const FVegetationDefinition& VegDef : Biome.VegetationTypes)
		{
			FVegetationDefinition AdjustedVeg = VegDef;
			AdjustedVeg.SpawnProbability *= Weight; // Scale by biome weight
			MixedVegetation.Add(AdjustedVeg);
		}
	}

	return MixedVegetation;
}

TArray<FTerrainMaterialLayer> UBiomeBlendingSystem::BlendMaterialLayers(const FBiomeWeights& BiomeWeights) const
{
	TArray<FTerrainMaterialLayer> BlendedLayers;

	if (!BiomeManager)
	{
		return BlendedLayers;
	}

	// Collect material layers from all contributing biomes
	for (const auto& Pair : BiomeWeights.Weights)
	{
		int32 BiomeIndex = Pair.Key;
		float Weight = Pair.Value;

		const FBiomeDefinition& Biome = BiomeManager->GetBiomeDefinition(BiomeIndex);

		// Add material layers (in practice, these would be blended in the material shader)
		for (const FTerrainMaterialLayer& Layer : Biome.MaterialLayers)
		{
			FTerrainMaterialLayer WeightedLayer = Layer;
			// Store weight in blend sharpness for now (would be handled differently in real implementation)
			WeightedLayer.BlendSharpness = Weight;
			BlendedLayers.Add(WeightedLayer);
		}
	}

	return BlendedLayers;
}

void UBiomeBlendingSystem::ApplyTransitionSmoothing(FBiomeWeights& Weights, FVector WorldPosition) const
{
	if (!TransitionConfig.bUseDistanceBlending)
	{
		return;
	}

	// Sample weights in a radius around the position
	int32 NumSamples = 8; // Sample in 8 directions
	FBiomeWeights SampledWeights = SampleWeightsInRadius(
		WorldPosition,
		TransitionConfig.TransitionZoneWidth,
		NumSamples
	);

	// Blend current weights with sampled weights based on smoothness
	for (auto& Pair : Weights.Weights)
	{
		int32 BiomeIndex = Pair.Key;
		float CurrentWeight = Pair.Value;

		float* SampledWeight = SampledWeights.Weights.Find(BiomeIndex);
		float TargetWeight = SampledWeight ? *SampledWeight : 0.0f;

		// Interpolate between current and sampled weight
		Pair.Value = FMath::Lerp(CurrentWeight, TargetWeight, TransitionConfig.TransitionSmoothness);
	}

	// Add any biomes that appeared in samples but not in current weights
	for (const auto& SampledPair : SampledWeights.Weights)
	{
		if (!Weights.Weights.Contains(SampledPair.Key))
		{
			float BlendedWeight = SampledPair.Value * TransitionConfig.TransitionSmoothness;
			if (BlendedWeight > TransitionConfig.MinimumBiomeWeight)
			{
				Weights.Weights.Add(SampledPair.Key, BlendedWeight);
			}
		}
	}

	// Re-normalize
	Weights.Normalize();
}

float UBiomeBlendingSystem::CalculateDistanceWeight(FVector WorldPosition, FVector SamplePosition) const
{
	float Distance = FVector::Dist(WorldPosition, SamplePosition);
	float NormalizedDistance = Distance / TransitionConfig.TransitionZoneWidth;

	// Use smoothstep for smooth falloff
	return 1.0f - SmoothStep(0.0f, 1.0f, NormalizedDistance);
}

FBiomeWeights UBiomeBlendingSystem::SampleWeightsInRadius(FVector WorldPosition, float Radius, int32 NumSamples) const
{
	FBiomeWeights AveragedWeights;

	if (!BiomeManager || NumSamples <= 0)
	{
		return AveragedWeights;
	}

	// Sample in a circle around the position
	float AngleStep = 360.0f / NumSamples;

	for (int32 i = 0; i < NumSamples; i++)
	{
		float Angle = i * AngleStep;
		float AngleRad = FMath::DegreesToRadians(Angle);

		// Calculate sample position
		FVector Offset = FVector(
			FMath::Cos(AngleRad) * Radius,
			FMath::Sin(AngleRad) * Radius,
			0.0f
		);

		FVector SamplePosition = WorldPosition + Offset;

		// Get weights at sample position
		FBiomeWeights SampleWeights = BiomeManager->GetBiomeWeightsAtLocation(SamplePosition);

		// Accumulate weights
		for (const auto& Pair : SampleWeights.Weights)
		{
			float* ExistingWeight = AveragedWeights.Weights.Find(Pair.Key);
			if (ExistingWeight)
			{
				*ExistingWeight += Pair.Value;
			}
			else
			{
				AveragedWeights.Weights.Add(Pair.Key, Pair.Value);
			}
		}
	}

	// Average the accumulated weights
	for (auto& Pair : AveragedWeights.Weights)
	{
		Pair.Value /= NumSamples;
	}

	return AveragedWeights;
}

float UBiomeBlendingSystem::SmoothStep(float Edge0, float Edge1, float X) const
{
	// Clamp X to [0, 1] range
	float T = FMath::Clamp((X - Edge0) / (Edge1 - Edge0), 0.0f, 1.0f);
	
	// Smoothstep formula: 3t^2 - 2t^3
	return T * T * (3.0f - 2.0f * T);
}

float UBiomeBlendingSystem::BlendValues(float Value1, float Value2, float Weight) const
{
	return FMath::Lerp(Value1, Value2, Weight);
}
