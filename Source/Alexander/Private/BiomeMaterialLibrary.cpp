// Copyright Epic Games, Inc. All Rights Reserved.

#include "BiomeMaterialLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Engine/World.h"

UBiomeMaterialLibrary::UBiomeMaterialLibrary()
{
	MasterTerrainMaterial = nullptr;
}

void UBiomeMaterialLibrary::Initialize(const TArray<FBiomeDefinition>& InBiomes)
{
	BiomeMaterials.Empty();
	CachedInstances.Empty();

	// Create material definitions for each biome
	for (const FBiomeDefinition& Biome : InBiomes)
	{
		FBiomeMaterialDefinition MaterialDef = CreateDefaultBiomeMaterial(Biome.BiomeType);
		MaterialDef.BiomeName = Biome.BiomeName;
		
		// Convert biome material layers to variations
		if (Biome.MaterialLayers.Num() > 0)
		{
			for (int32 i = 0; i < Biome.MaterialLayers.Num(); ++i)
			{
				FBiomeMaterialVariation Variation;
				Variation.VariationName = FString::Printf(TEXT("Variation_%d"), i);
				
				const FTerrainMaterialLayer& Layer = Biome.MaterialLayers[i];
				
				// Set ground material from layer
				Variation.GroundMaterial.BaseColor = Layer.BaseColorTexture;
				Variation.GroundMaterial.Normal = Layer.NormalTexture;
				Variation.GroundMaterial.Roughness = Layer.RoughnessTexture;
				Variation.GroundMaterial.Metallic = Layer.MetallicTexture;
				Variation.GroundMaterial.TilingScale = Layer.TilingScale;
				Variation.BlendSharpness = Layer.BlendSharpness;
				
				MaterialDef.Variations.Add(Variation);
			}
		}
		
		BiomeMaterials.Add(Biome.BiomeType, MaterialDef);
	}

	UE_LOG(LogTemp, Log, TEXT("BiomeMaterialLibrary: Initialized with %d biomes"), BiomeMaterials.Num());
}

void UBiomeMaterialLibrary::LoadFromAsset(UBiomeMaterialLibraryAsset* LibraryAsset)
{
	if (!LibraryAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("BiomeMaterialLibrary: Cannot load from null asset"));
		return;
	}

	BiomeMaterials.Empty();
	CachedInstances.Empty();

	MasterTerrainMaterial = LibraryAsset->MasterTerrainMaterial;

	// Load all biome material definitions
	for (const FBiomeMaterialDefinition& Def : LibraryAsset->BiomeMaterialDefinitions)
	{
		BiomeMaterials.Add(Def.BiomeType, Def);
	}

	UE_LOG(LogTemp, Log, TEXT("BiomeMaterialLibrary: Loaded %d biome materials from asset"), BiomeMaterials.Num());
}

const FBiomeMaterialDefinition& UBiomeMaterialLibrary::GetBiomeMaterial(EBiomeType BiomeType) const
{
	if (const FBiomeMaterialDefinition* Found = BiomeMaterials.Find(BiomeType))
	{
		return *Found;
	}

	// Return default if not found
	static FBiomeMaterialDefinition Default;
	UE_LOG(LogTemp, Warning, TEXT("BiomeMaterialLibrary: Material not found for biome type %d"), (int32)BiomeType);
	return Default;
}

UMaterialInstanceDynamic* UBiomeMaterialLibrary::GetBiomeMaterialInstance(EBiomeType BiomeType, int32 VariationIndex)
{
	// Check cache first
	FString CacheKey = GenerateCacheKey(BiomeType, VariationIndex);
	if (UMaterialInstanceDynamic** Cached = CachedInstances.Find(CacheKey))
	{
		if (*Cached && (*Cached)->IsValidLowLevel())
		{
			return *Cached;
		}
	}

	// Get biome material definition
	const FBiomeMaterialDefinition& BiomeMaterial = GetBiomeMaterial(BiomeType);
	if (BiomeMaterial.Variations.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("BiomeMaterialLibrary: No variations for biome type %d"), (int32)BiomeType);
		return nullptr;
	}

	// Get variation
	const FBiomeMaterialVariation& Variation = BiomeMaterial.GetVariation(VariationIndex);
	
	// Create material instance
	UMaterialInstanceDynamic* MaterialInstance = CreateMaterialFromVariation(Variation);
	
	// Cache it
	if (MaterialInstance)
	{
		CachedInstances.Add(CacheKey, MaterialInstance);
	}

	return MaterialInstance;
}

const FBiomeMaterialVariation& UBiomeMaterialLibrary::GetRandomVariation(EBiomeType BiomeType) const
{
	const FBiomeMaterialDefinition& BiomeMaterial = GetBiomeMaterial(BiomeType);
	return BiomeMaterial.GetRandomVariation();
}

UMaterialInstanceDynamic* UBiomeMaterialLibrary::CreateBlendedMaterial(const TMap<EBiomeType, float>& BiomeWeights)
{
	if (!MasterTerrainMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("BiomeMaterialLibrary: Cannot create blended material without master material"));
		return nullptr;
	}

	// Create dynamic material instance
	UMaterialInstanceDynamic* BlendedMaterial = UMaterialInstanceDynamic::Create(MasterTerrainMaterial, this);
	if (!BlendedMaterial)
	{
		return nullptr;
	}

	// Apply each biome's textures with weights
	int32 LayerIndex = 0;
	for (const TPair<EBiomeType, float>& Pair : BiomeWeights)
	{
		if (LayerIndex >= 8) // Maximum 8 layers
		{
			break;
		}

		const FBiomeMaterialDefinition& BiomeMaterial = GetBiomeMaterial(Pair.Key);
		if (BiomeMaterial.Variations.Num() > 0)
		{
			const FBiomeMaterialVariation& Variation = BiomeMaterial.GetRandomVariation();
			FString LayerPrefix = FString::Printf(TEXT("Layer%d_"), LayerIndex);
			
			ApplyTextureSet(BlendedMaterial, Variation.GroundMaterial, LayerPrefix);
			
			// Set blend weight
			FName WeightParam = FName(*FString::Printf(TEXT("BiomeWeight%d"), LayerIndex));
			BlendedMaterial->SetScalarParameterValue(WeightParam, Pair.Value);
		}

		LayerIndex++;
	}

	return BlendedMaterial;
}

void UBiomeMaterialLibrary::ApplyTextureSet(UMaterialInstanceDynamic* MaterialInstance, const FPBRTextureSet& TextureSet, const FString& LayerPrefix)
{
	if (!MaterialInstance)
	{
		return;
	}

	// Apply textures
	if (TextureSet.BaseColor)
	{
		MaterialInstance->SetTextureParameterValue(FName(*(LayerPrefix + TEXT("BaseColor"))), TextureSet.BaseColor);
	}
	if (TextureSet.Normal)
	{
		MaterialInstance->SetTextureParameterValue(FName(*(LayerPrefix + TEXT("Normal"))), TextureSet.Normal);
	}
	if (TextureSet.Roughness)
	{
		MaterialInstance->SetTextureParameterValue(FName(*(LayerPrefix + TEXT("Roughness"))), TextureSet.Roughness);
	}
	if (TextureSet.Metallic)
	{
		MaterialInstance->SetTextureParameterValue(FName(*(LayerPrefix + TEXT("Metallic"))), TextureSet.Metallic);
	}
	if (TextureSet.AmbientOcclusion)
	{
		MaterialInstance->SetTextureParameterValue(FName(*(LayerPrefix + TEXT("AO"))), TextureSet.AmbientOcclusion);
	}
	if (TextureSet.Height)
	{
		MaterialInstance->SetTextureParameterValue(FName(*(LayerPrefix + TEXT("Height"))), TextureSet.Height);
	}

	// Apply tiling scales
	MaterialInstance->SetScalarParameterValue(FName(*(LayerPrefix + TEXT("TilingScale"))), TextureSet.TilingScale);
	MaterialInstance->SetScalarParameterValue(FName(*(LayerPrefix + TEXT("MacroTilingScale"))), TextureSet.MacroTilingScale);
}

void UBiomeMaterialLibrary::UpdateBiomeParameter(EBiomeType BiomeType, FName ParameterName, float Value)
{
	UMaterialParameterCollection* Collection = GetParameterCollection(BiomeType);
	if (!Collection)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UMaterialParameterCollectionInstance* Instance = World->GetParameterCollectionInstance(Collection);
	if (Instance)
	{
		Instance->SetScalarParameterValue(ParameterName, Value);
	}
}

UMaterialParameterCollection* UBiomeMaterialLibrary::GetParameterCollection(EBiomeType BiomeType) const
{
	const FBiomeMaterialDefinition& BiomeMaterial = GetBiomeMaterial(BiomeType);
	return BiomeMaterial.ParameterCollection;
}

int32 UBiomeMaterialLibrary::GetVariationCount(EBiomeType BiomeType) const
{
	const FBiomeMaterialDefinition& BiomeMaterial = GetBiomeMaterial(BiomeType);
	return BiomeMaterial.Variations.Num();
}

bool UBiomeMaterialLibrary::HasValidMaterials(EBiomeType BiomeType) const
{
	const FBiomeMaterialDefinition& BiomeMaterial = GetBiomeMaterial(BiomeType);
	
	if (BiomeMaterial.Variations.Num() == 0)
	{
		return false;
	}

	// Check if at least one variation has valid textures
	for (const FBiomeMaterialVariation& Variation : BiomeMaterial.Variations)
	{
		if (Variation.GroundMaterial.IsValid())
		{
			return true;
		}
	}

	return false;
}

FBiomeMaterialDefinition UBiomeMaterialLibrary::CreateDefaultBiomeMaterial(EBiomeType BiomeType)
{
	FBiomeMaterialDefinition MaterialDef;
	MaterialDef.BiomeType = BiomeType;

	// Set biome name based on type
	switch (BiomeType)
	{
	case EBiomeType::Desert:
		MaterialDef.BiomeName = TEXT("Desert");
		break;
	case EBiomeType::Grassland:
		MaterialDef.BiomeName = TEXT("Grassland");
		break;
	case EBiomeType::Forest:
		MaterialDef.BiomeName = TEXT("Forest");
		break;
	case EBiomeType::Tundra:
		MaterialDef.BiomeName = TEXT("Tundra");
		break;
	case EBiomeType::Volcanic:
		MaterialDef.BiomeName = TEXT("Volcanic");
		break;
	case EBiomeType::Ocean:
		MaterialDef.BiomeName = TEXT("Ocean");
		break;
	case EBiomeType::Savanna:
		MaterialDef.BiomeName = TEXT("Savanna");
		break;
	case EBiomeType::Rainforest:
		MaterialDef.BiomeName = TEXT("Rainforest");
		break;
	case EBiomeType::Alpine:
		MaterialDef.BiomeName = TEXT("Alpine");
		break;
	case EBiomeType::Swamp:
		MaterialDef.BiomeName = TEXT("Swamp");
		break;
	default:
		MaterialDef.BiomeName = TEXT("Unknown");
		break;
	}

	// Create default variations (6+ as per requirements)
	for (int32 i = 0; i < 6; ++i)
	{
		FBiomeMaterialVariation Variation;
		Variation.VariationName = FString::Printf(TEXT("Variation_%d"), i + 1);
		Variation.BlendSharpness = 0.5f;
		Variation.CliffSlopeThreshold = 45.0f;
		Variation.bUseTriplanarProjection = true;
		
		// Set default tiling scales
		Variation.GroundMaterial.TilingScale = 5.0f;
		Variation.GroundMaterial.MacroTilingScale = 50.0f;
		Variation.SecondaryMaterial.TilingScale = 3.0f;
		Variation.SecondaryMaterial.MacroTilingScale = 30.0f;
		Variation.CliffMaterial.TilingScale = 2.0f;
		Variation.CliffMaterial.MacroTilingScale = 20.0f;
		Variation.DetailMaterial.TilingScale = 1.0f;
		Variation.DetailMaterial.MacroTilingScale = 10.0f;
		
		MaterialDef.Variations.Add(Variation);
	}

	return MaterialDef;
}

FString UBiomeMaterialLibrary::GenerateCacheKey(EBiomeType BiomeType, int32 VariationIndex) const
{
	return FString::Printf(TEXT("%d_%d"), (int32)BiomeType, VariationIndex);
}

UMaterialInstanceDynamic* UBiomeMaterialLibrary::CreateMaterialFromVariation(const FBiomeMaterialVariation& Variation)
{
	if (!MasterTerrainMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("BiomeMaterialLibrary: Cannot create material without master material"));
		return nullptr;
	}

	// Create dynamic material instance
	UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(MasterTerrainMaterial, this);
	if (!MaterialInstance)
	{
		return nullptr;
	}

	// Apply ground material (Layer 0)
	ApplyTextureSet(MaterialInstance, Variation.GroundMaterial, TEXT("Layer0_"));

	// Apply secondary material (Layer 1)
	if (Variation.SecondaryMaterial.IsValid())
	{
		ApplyTextureSet(MaterialInstance, Variation.SecondaryMaterial, TEXT("Layer1_"));
	}

	// Apply cliff material (Layer 5 - reserved for cliffs)
	if (Variation.CliffMaterial.IsValid())
	{
		ApplyTextureSet(MaterialInstance, Variation.CliffMaterial, TEXT("Layer5_"));
		MaterialInstance->SetScalarParameterValue(TEXT("Layer5_UseTriplanar"), Variation.bUseTriplanarProjection ? 1.0f : 0.0f);
		MaterialInstance->SetScalarParameterValue(TEXT("CliffSlopeThreshold"), Variation.CliffSlopeThreshold);
	}

	// Apply detail material (for close-up viewing)
	if (Variation.DetailMaterial.IsValid())
	{
		ApplyTextureSet(MaterialInstance, Variation.DetailMaterial, TEXT("Detail_"));
	}

	// Set blend sharpness
	MaterialInstance->SetScalarParameterValue(TEXT("BlendSharpness"), Variation.BlendSharpness);

	return MaterialInstance;
}
