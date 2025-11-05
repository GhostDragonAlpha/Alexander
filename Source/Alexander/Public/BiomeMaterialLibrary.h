// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PlanetConfiguration.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "BiomeMaterialLibrary.generated.h"

/**
 * Complete PBR texture set for a material layer
 */
USTRUCT(BlueprintType)
struct FPBRTextureSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* BaseColor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* Normal = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* Roughness = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* Metallic = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* AmbientOcclusion = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	UTexture2D* Height = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	float TilingScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	float MacroTilingScale = 10.0f;

	// Check if texture set is valid
	bool IsValid() const
	{
		return BaseColor != nullptr && Normal != nullptr;
	}
};

/**
 * Material variation for a biome (different texture sets for variety)
 */
USTRUCT(BlueprintType)
struct FBiomeMaterialVariation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation")
	FString VariationName = TEXT("Default");

	// Primary ground material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	FPBRTextureSet GroundMaterial;

	// Secondary material (for blending)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	FPBRTextureSet SecondaryMaterial;

	// Cliff/rock material for steep slopes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	FPBRTextureSet CliffMaterial;

	// Detail material for close-up viewing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
	FPBRTextureSet DetailMaterial;

	// Material properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	float BlendSharpness = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	float CliffSlopeThreshold = 45.0f; // degrees

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Properties")
	bool bUseTriplanarProjection = true;
};

/**
 * Complete material definition for a biome type
 */
USTRUCT(BlueprintType)
struct FBiomeMaterialDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	EBiomeType BiomeType = EBiomeType::Grassland;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString BiomeName = TEXT("Grassland");

	// Multiple variations for visual diversity (6+ per biome as per requirements)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variations")
	TArray<FBiomeMaterialVariation> Variations;

	// Material parameter collection for this biome
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	UMaterialParameterCollection* ParameterCollection = nullptr;

	// Base material instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
	UMaterialInstance* BaseMaterialInstance = nullptr;

	// Get a random variation
	const FBiomeMaterialVariation& GetRandomVariation() const
	{
		if (Variations.Num() > 0)
		{
			int32 Index = FMath::RandRange(0, Variations.Num() - 1);
			return Variations[Index];
		}
		static FBiomeMaterialVariation Default;
		return Default;
	}

	// Get variation by index
	const FBiomeMaterialVariation& GetVariation(int32 Index) const
	{
		if (Variations.IsValidIndex(Index))
		{
			return Variations[Index];
		}
		static FBiomeMaterialVariation Default;
		return Default;
	}
};

/**
 * Biome Material Library - Manages all material assets for biomes
 * Provides material instances, texture sets, and parameter collections
 */
UCLASS(BlueprintType)
class ALEXANDER_API UBiomeMaterialLibrary : public UObject
{
	GENERATED_BODY()

public:
	UBiomeMaterialLibrary();

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Initialize the material library with biome definitions
	 * @param InBiomes Array of biome definitions to create materials for
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Library")
	void Initialize(const TArray<FBiomeDefinition>& InBiomes);

	/**
	 * Load material library from data asset
	 * @param LibraryAsset Data asset containing material definitions
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Library")
	void LoadFromAsset(class UBiomeMaterialLibraryAsset* LibraryAsset);

	// ============================================================================
	// MATERIAL QUERIES
	// ============================================================================

	/**
	 * Get material definition for a biome type
	 * @param BiomeType Type of biome
	 * @return Material definition for the biome
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Library")
	const FBiomeMaterialDefinition& GetBiomeMaterial(EBiomeType BiomeType) const;

	/**
	 * Get material instance for a biome with specific variation
	 * @param BiomeType Type of biome
	 * @param VariationIndex Index of variation (0-based)
	 * @return Dynamic material instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Library")
	UMaterialInstanceDynamic* GetBiomeMaterialInstance(EBiomeType BiomeType, int32 VariationIndex = 0);

	/**
	 * Get random material variation for a biome
	 * @param BiomeType Type of biome
	 * @return Random material variation
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Library")
	const FBiomeMaterialVariation& GetRandomVariation(EBiomeType BiomeType) const;

	// ============================================================================
	// MATERIAL CREATION
	// ============================================================================

	/**
	 * Create a blended material instance from multiple biomes
	 * @param BiomeWeights Map of biome type to weight (0-1)
	 * @return Blended material instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Library")
	UMaterialInstanceDynamic* CreateBlendedMaterial(const TMap<EBiomeType, float>& BiomeWeights);

	/**
	 * Apply texture set to material instance
	 * @param MaterialInstance Material instance to modify
	 * @param TextureSet Texture set to apply
	 * @param LayerPrefix Parameter name prefix (e.g., "Layer0_")
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Library")
	void ApplyTextureSet(UMaterialInstanceDynamic* MaterialInstance, const FPBRTextureSet& TextureSet, const FString& LayerPrefix);

	// ============================================================================
	// PARAMETER COLLECTIONS
	// ============================================================================

	/**
	 * Update parameter collection for a biome
	 * @param BiomeType Type of biome
	 * @param ParameterName Name of parameter to update
	 * @param Value New value for parameter
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Library")
	void UpdateBiomeParameter(EBiomeType BiomeType, FName ParameterName, float Value);

	/**
	 * Get parameter collection for a biome
	 * @param BiomeType Type of biome
	 * @return Material parameter collection
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Library")
	UMaterialParameterCollection* GetParameterCollection(EBiomeType BiomeType) const;

	// ============================================================================
	// UTILITY
	// ============================================================================

	/**
	 * Get number of variations for a biome
	 * @param BiomeType Type of biome
	 * @return Number of variations available
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Library")
	int32 GetVariationCount(EBiomeType BiomeType) const;

	/**
	 * Check if biome has valid materials
	 * @param BiomeType Type of biome
	 * @return True if biome has valid materials
	 */
	UFUNCTION(BlueprintCallable, Category = "Material Library")
	bool HasValidMaterials(EBiomeType BiomeType) const;

protected:
	// Material definitions for each biome type
	UPROPERTY()
	TMap<EBiomeType, FBiomeMaterialDefinition> BiomeMaterials;

	// Master terrain material
	UPROPERTY(EditAnywhere, Category = "Master Material")
	UMaterialInterface* MasterTerrainMaterial;

	// Cached material instances
	UPROPERTY()
	TMap<FString, UMaterialInstanceDynamic*> CachedInstances;

	// ============================================================================
	// INTERNAL METHODS
	// ============================================================================

	/**
	 * Create default material definition for a biome
	 */
	FBiomeMaterialDefinition CreateDefaultBiomeMaterial(EBiomeType BiomeType);

	/**
	 * Generate cache key for material instance
	 */
	FString GenerateCacheKey(EBiomeType BiomeType, int32 VariationIndex) const;

	/**
	 * Create material instance from variation
	 */
	UMaterialInstanceDynamic* CreateMaterialFromVariation(const FBiomeMaterialVariation& Variation);
};

/**
 * Data asset for storing biome material library
 */
UCLASS(BlueprintType)
class ALEXANDER_API UBiomeMaterialLibraryAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Material definitions for all biomes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	TArray<FBiomeMaterialDefinition> BiomeMaterialDefinitions;

	// Master terrain material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Master Material")
	UMaterialInterface* MasterTerrainMaterial;

	/**
	 * Get material definition for a biome type
	 */
	const FBiomeMaterialDefinition* GetBiomeMaterial(EBiomeType BiomeType) const
	{
		for (const FBiomeMaterialDefinition& Def : BiomeMaterialDefinitions)
		{
			if (Def.BiomeType == BiomeType)
			{
				return &Def;
			}
		}
		return nullptr;
	}
};
