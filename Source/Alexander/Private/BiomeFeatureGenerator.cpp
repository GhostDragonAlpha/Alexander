// Copyright Epic Games, Inc. All Rights Reserved.

#include "BiomeFeatureGenerator.h"
#include "ProceduralNoiseGenerator.h"
#include "BiomeManager.h"
#include "BiomeBlendingSystem.h"
#include "Kismet/KismetMathLibrary.h"

UBiomeFeatureGenerator::UBiomeFeatureGenerator()
{
	BiomeManager = nullptr;
	BlendingSystem = nullptr;
	NoiseGenerator = nullptr;
	Seed = 12345;
}

void UBiomeFeatureGenerator::Initialize(UBiomeManager* InBiomeManager, UBiomeBlendingSystem* InBlendingSystem, int32 InSeed)
{
	BiomeManager = InBiomeManager;
	BlendingSystem = InBlendingSystem;
	Seed = InSeed;

	// Create noise generator
	if (!NoiseGenerator)
	{
		NoiseGenerator = NewObject<UProceduralNoiseGenerator>(this);
	}
	// NoiseGenerator->Initialize(Seed); // TODO: Implement Initialize method

	UE_LOG(LogTemp, Log, TEXT("BiomeFeatureGenerator initialized with seed %d"), Seed);
}

TArray<FFeaturePlacement> UBiomeFeatureGenerator::GenerateVegetationPlacements(FVector TileCenter, float TileSize)
{
	TArray<FFeaturePlacement> Placements;

	if (!BiomeManager || !BlendingSystem)
	{
		return Placements;
	}

	// Calculate number of potential spawn points based on spacing
	int32 PointsPerSide = FMath::CeilToInt(TileSize / GenerationConfig.VegetationSpacing);
	int32 TotalPoints = PointsPerSide * PointsPerSide;

	// Limit to max features
	TotalPoints = FMath::Min(TotalPoints, GenerationConfig.MaxFeaturesPerTile);

	// Generate placements
	for (int32 i = 0; i < TotalPoints; i++)
	{
		// Generate position within tile
		FVector Position = GenerateRandomPositionInTile(TileCenter, TileSize, i);

		// Get blended parameters at this location
		FBlendedTerrainParameters BlendedParams = BlendingSystem->GetBlendedParameters(Position);

		// Calculate density multiplier
		float DensityMultiplier = GenerationConfig.bUseNoiseForDensity ? 
			CalculateDensityMultiplier(Position) : 1.0f;

		// Apply vegetation density from blended biomes
		float FinalDensity = BlendedParams.VegetationDensity * DensityMultiplier;

		// Random check against density
		float RandomValue = HashPosition(Position, 0);
		if (RandomValue > FinalDensity)
		{
			continue; // Skip this placement
		}

		// Get mixed vegetation types
		FBiomeWeights Weights;
		Weights.Weights = BlendedParams.Weights;
		TArray<FVegetationDefinition> VegetationTypes = BlendingSystem->MixVegetationTypes(Weights);

		if (VegetationTypes.Num() == 0)
		{
			continue;
		}

		// Select vegetation type based on spawn probabilities
		float SelectionRandom = HashPosition(Position, 1);
		float CumulativeProbability = 0.0f;
		int32 SelectedType = 0;

		for (int32 j = 0; j < VegetationTypes.Num(); j++)
		{
			CumulativeProbability += VegetationTypes[j].SpawnProbability;
			if (SelectionRandom <= CumulativeProbability)
			{
				SelectedType = j;
				break;
			}
		}

		// Calculate slope at this position
		// Use terrain normal to determine slope angle
		FVector SurfaceNormal = CalculateSurfaceNormal(Position);
		float SlopeAngle = FMath::Acos(FVector::DotProduct(SurfaceNormal, FVector::UpVector)) * (180.0f / PI);

		const FVegetationDefinition& SelectedVeg = VegetationTypes[SelectedType];

		if (SlopeAngle < SelectedVeg.MinSlopeAngle || SlopeAngle > SelectedVeg.MaxSlopeAngle)
		{
			continue;
		}

		// Create placement
		FFeaturePlacement Placement;
		Placement.Location = Position;
		Placement.Scale = GetRandomScale(SelectedVeg.ScaleRange, i);
		Placement.Rotation = GetRandomRotation(i, true, FVector::UpVector);
		Placement.FeatureTypeIndex = SelectedType;

		Placements.Add(Placement);
	}

	return Placements;
}

bool UBiomeFeatureGenerator::CanSpawnVegetationAt(FVector Location, float SlopeAngle) const
{
	if (!BiomeManager)
	{
		return false;
	}

	// Get dominant biome
	int32 BiomeIndex = BiomeManager->GetDominantBiomeAtLocation(Location);
	if (BiomeIndex < 0)
	{
		return false;
	}

	const FBiomeDefinition& Biome = BiomeManager->GetBiomeDefinition(BiomeIndex);

	// Check if biome has vegetation
	if (Biome.VegetationTypes.Num() == 0 || Biome.VegetationDensity <= 0.0f)
	{
		return false;
	}

	// Check slope constraints for at least one vegetation type
	for (const FVegetationDefinition& VegDef : Biome.VegetationTypes)
	{
		if (SlopeAngle >= VegDef.MinSlopeAngle && SlopeAngle <= VegDef.MaxSlopeAngle)
		{
			return true;
		}
	}

	return false;
}

TArray<FFeaturePlacement> UBiomeFeatureGenerator::GenerateDetailObjectPlacements(FVector TileCenter, float TileSize)
{
	TArray<FFeaturePlacement> Placements;

	if (!BiomeManager)
	{
		return Placements;
	}

	// Get dominant biome at tile center
	int32 BiomeIndex = BiomeManager->GetDominantBiomeAtLocation(TileCenter);
	if (BiomeIndex < 0)
	{
		return Placements;
	}

	const FBiomeDefinition& Biome = BiomeManager->GetBiomeDefinition(BiomeIndex);
	TArray<FDetailObjectDefinition> DetailObjects = GetDetailObjectsForBiome(BiomeIndex);

	if (DetailObjects.Num() == 0)
	{
		return Placements;
	}

	// Calculate number of potential spawn points
	int32 PointsPerSide = FMath::CeilToInt(TileSize / GenerationConfig.DetailObjectSpacing);
	int32 TotalPoints = PointsPerSide * PointsPerSide;
	TotalPoints = FMath::Min(TotalPoints, GenerationConfig.MaxFeaturesPerTile);

	// Generate placements
	for (int32 i = 0; i < TotalPoints; i++)
	{
		FVector Position = GenerateRandomPositionInTile(TileCenter, TileSize, i + 1000); // Offset seed

		// Select detail object type
		int32 ObjectTypeIndex = FMath::FloorToInt(HashPosition(Position, 2) * DetailObjects.Num());
		ObjectTypeIndex = FMath::Clamp(ObjectTypeIndex, 0, DetailObjects.Num() - 1);

		const FDetailObjectDefinition& DetailObject = DetailObjects[ObjectTypeIndex];

		// Check spawn probability
		float SpawnRandom = HashPosition(Position, 3);
		if (SpawnRandom > DetailObject.SpawnProbability)
		{
			continue;
		}

		// Create placement
		FFeaturePlacement Placement;
		Placement.Location = Position;
		Placement.Scale = GetRandomScale(DetailObject.ScaleRange, i);
		Placement.Rotation = GetRandomRotation(i, DetailObject.bAlignToSurface, FVector::UpVector);
		Placement.FeatureTypeIndex = ObjectTypeIndex;

		Placements.Add(Placement);
	}

	return Placements;
}

TArray<FDetailObjectDefinition> UBiomeFeatureGenerator::GetDetailObjectsForBiome(int32 BiomeIndex) const
{
	TArray<FDetailObjectDefinition> DetailObjects;

	if (!BiomeManager)
	{
		return DetailObjects;
	}

	const FBiomeDefinition& Biome = BiomeManager->GetBiomeDefinition(BiomeIndex);

	// Check if we have configured detail objects for this biome type
	const FBiomeDetailObjects* ConfiguredObjects = BiomeDetailObjects.Find(Biome.BiomeType);
	if (ConfiguredObjects)
	{
		DetailObjects = ConfiguredObjects->DetailObjects;
	}

	return DetailObjects;
}

TArray<FEnvironmentalEffect> UBiomeFeatureGenerator::GetEnvironmentalEffectsAt(FVector Location) const
{
	TArray<FEnvironmentalEffect> Effects;

	if (!BiomeManager)
	{
		return Effects;
	}

	// Get dominant biome
	int32 BiomeIndex = BiomeManager->GetDominantBiomeAtLocation(Location);
	if (BiomeIndex < 0)
	{
		return Effects;
	}

	const FBiomeDefinition& Biome = BiomeManager->GetBiomeDefinition(BiomeIndex);

	// Get configured effects for this biome type
	const FBiomeEnvironmentalEffects* ConfiguredEffects = BiomeEnvironmentalEffects.Find(Biome.BiomeType);
	if (ConfiguredEffects)
	{
		// Filter effects that should spawn at this location
		for (const FEnvironmentalEffect& Effect : ConfiguredEffects->EnvironmentalEffects)
		{
			if (ShouldSpawnEffect(Effect, Location))
			{
				Effects.Add(Effect);
			}
		}
	}

	return Effects;
}

bool UBiomeFeatureGenerator::ShouldSpawnEffect(const FEnvironmentalEffect& Effect, FVector Location) const
{
	// Use hash-based randomness for deterministic spawning
	float SpawnChance = HashPosition(Location, 4);
	
	// Normalize spawn rate to probability (assuming spawn rate is per second, convert to 0-1 range)
	float SpawnProbability = FMath::Clamp(Effect.SpawnRate / 10.0f, 0.0f, 1.0f);

	return SpawnChance < SpawnProbability;
}

TArray<FTerrainMaterialLayer> UBiomeFeatureGenerator::GetTerrainTexturesAt(FVector Location) const
{
	TArray<FTerrainMaterialLayer> MaterialLayers;

	if (!BlendingSystem)
	{
		return MaterialLayers;
	}

	// Get blended parameters
	FBlendedTerrainParameters BlendedParams = BlendingSystem->GetBlendedParameters(Location);

	// Get blended material layers
	FBiomeWeights Weights;
	Weights.Weights = BlendedParams.Weights;
	MaterialLayers = BlendingSystem->BlendMaterialLayers(Weights);

	return MaterialLayers;
}

FVector UBiomeFeatureGenerator::GenerateRandomPositionInTile(FVector TileCenter, float TileSize, int32 Index) const
{
	// Use index-based deterministic randomness
	float RandomX = HashPosition(TileCenter, Index * 2) - 0.5f;
	float RandomY = HashPosition(TileCenter, Index * 2 + 1) - 0.5f;

	// Apply spacing and random offset
	FVector Offset = FVector(
		RandomX * TileSize,
		RandomY * TileSize,
		0.0f
	);

	return TileCenter + Offset;
}

float UBiomeFeatureGenerator::CalculateDensityMultiplier(FVector Location) const
{
	if (!NoiseGenerator)
	{
		return 1.0f;
	}

	// Generate noise for density variation
	FVector NoisePos = Location / 500.0f;
	float NoiseValue = UProceduralNoiseGenerator::PerlinNoise3D(NoisePos.X, NoisePos.Y, NoisePos.Z, 12345);

	// Map from [-1, 1] to [0.5, 1.5] for density variation
	return FMath::Clamp((NoiseValue + 1.0f) * 0.5f + 0.5f, 0.0f, 2.0f);
}

FVector UBiomeFeatureGenerator::GetRandomScale(FVector2D ScaleRange, int32 Index) const
{
	float RandomScale = FMath::Lerp(ScaleRange.X, ScaleRange.Y, HashPosition(FVector(Index, 0, 0), 5));
	return FVector(RandomScale);
}

FRotator UBiomeFeatureGenerator::GetRandomRotation(int32 Index, bool bAlignToSurface, FVector SurfaceNormal) const
{
	// Random yaw rotation
	float RandomYaw = HashPosition(FVector(Index, 0, 0), 6) * 360.0f;

	FRotator Rotation = FRotator(0.0f, RandomYaw, 0.0f);

	if (bAlignToSurface)
	{
		// Align to surface normal (simplified - would use actual terrain normal)
		// For now, just use the provided normal
		FRotator SurfaceRotation = SurfaceNormal.Rotation();
		Rotation = FRotator(SurfaceRotation.Pitch, RandomYaw, SurfaceRotation.Roll);
	}

	return Rotation;
}

float UBiomeFeatureGenerator::HashPosition(FVector Position, int32 Offset) const
{
	// Simple hash function for deterministic pseudo-random values
	int32 X = FMath::FloorToInt(Position.X);
	int32 Y = FMath::FloorToInt(Position.Y);
	int32 Z = FMath::FloorToInt(Position.Z);

	int32 Hash = X * 73856093 ^ Y * 19349663 ^ Z * 83492791 ^ (Seed + Offset) * 50331653;
	Hash = (Hash ^ (Hash >> 16)) * 0x85ebca6b;
	Hash = (Hash ^ (Hash >> 13)) * 0xc2b2ae35;
	Hash = Hash ^ (Hash >> 16);

	// Convert to 0-1 range
	return FMath::Abs(static_cast<float>(Hash) / static_cast<float>(INT_MAX));
}

FVector UBiomeFeatureGenerator::CalculateSurfaceNormal(FVector Position) const
{
	if (!BiomeBlendingSystem)
	{
		return FVector::UpVector;
	}

	// Sample terrain height at neighboring points to calculate normal
	const float SampleDistance = 10.0f; // 10 units

	// Get heights at cardinal directions
	FVector Right = Position + FVector(SampleDistance, 0, 0);
	FVector Left = Position + FVector(-SampleDistance, 0, 0);
	FVector Forward = Position + FVector(0, SampleDistance, 0);
	FVector Back = Position + FVector(0, -SampleDistance, 0);

	// Sample terrain using blended parameters (simplified - would use actual terrain height query)
	FBlendedTerrainParameters ParamsRight = BiomeBlendingSystem->GetBlendedParameters(Right);
	FBlendedTerrainParameters ParamsLeft = BiomeBlendingSystem->GetBlendedParameters(Left);
	FBlendedTerrainParameters ParamsForward = BiomeBlendingSystem->GetBlendedParameters(Forward);
	FBlendedTerrainParameters ParamsBack = BiomeBlendingSystem->GetBlendedParameters(Back);

	// Use terrain parameters as a proxy for height variation
	float HeightRight = ParamsRight.Roughness * 100.0f;
	float HeightLeft = ParamsLeft.Roughness * 100.0f;
	float HeightForward = ParamsForward.Roughness * 100.0f;
	float HeightBack = ParamsBack.Roughness * 100.0f;

	// Calculate gradients
	FVector TangentX = FVector(SampleDistance * 2.0f, 0, HeightRight - HeightLeft);
	FVector TangentY = FVector(0, SampleDistance * 2.0f, HeightForward - HeightBack);

	// Cross product to get normal
	FVector Normal = FVector::CrossProduct(TangentY, TangentX);
	Normal.Normalize();

	return Normal;
}
