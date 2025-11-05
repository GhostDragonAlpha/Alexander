// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PlanetConfiguration.h"
#include "BiomeManager.h"
#include "BiomeBlendingSystem.h"
#include "BiomeFeatureGenerator.generated.h"

// Forward declarations
class UProceduralNoiseGenerator;
class UInstancedStaticMeshComponent;

/**
 * Feature placement result
 */
USTRUCT(BlueprintType)
struct FFeaturePlacement
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Feature")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Feature")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Feature")
	FVector Scale = FVector::OneVector;

	UPROPERTY(BlueprintReadOnly, Category = "Feature")
	int32 FeatureTypeIndex = 0;
};

/**
 * Detail object definition (rocks, debris, etc.)
 */
USTRUCT(BlueprintType)
struct FDetailObjectDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail")
	UStaticMesh* Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail")
	FVector2D ScaleRange = FVector2D(0.5f, 1.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail")
	float SpawnProbability = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail")
	bool bAlignToSurface = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detail")
	float MaxSlopeAngle = 60.0f;
};

/**
 * Environmental effect definition
 */
USTRUCT(BlueprintType)
struct FEnvironmentalEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FString EffectName = TEXT("Unnamed Effect");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	UParticleSystem* ParticleEffect = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	class UNiagaraSystem* NiagaraEffect = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float SpawnRate = 1.0f; // Per second

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float SpawnRadius = 1000.0f; // cm

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	bool bAttachToGround = true;
};

/**
 * Wrapper for biome detail objects array (needed for TMap compatibility)
 */
USTRUCT(BlueprintType)
struct FBiomeDetailObjects
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Details")
	TArray<FDetailObjectDefinition> DetailObjects;
};

/**
 * Wrapper for biome environmental effects array (needed for TMap compatibility)
 */
USTRUCT(BlueprintType)
struct FBiomeEnvironmentalEffects
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TArray<FEnvironmentalEffect> EnvironmentalEffects;
};

/**
 * Feature generation configuration
 */
USTRUCT(BlueprintType)
struct FFeatureGenerationConfig
{
	GENERATED_BODY()

	// Vegetation placement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	float VegetationSpacing = 200.0f; // cm between plants

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	float VegetationRandomOffset = 100.0f; // cm random offset

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
	bool bUseNoiseForDensity = true;

	// Detail objects (rocks, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Details")
	float DetailObjectSpacing = 300.0f; // cm

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Details")
	float DetailObjectRandomOffset = 150.0f; // cm

	// Performance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	int32 MaxFeaturesPerTile = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float CullingDistance = 10000.0f; // cm

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	bool bUseInstancing = true;
};

/**
 * Generates biome-specific features like vegetation, rocks, and effects
 */
UCLASS(BlueprintType)
class ALEXANDER_API UBiomeFeatureGenerator : public UObject
{
	GENERATED_BODY()

public:
	UBiomeFeatureGenerator();

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Initialize feature generator
	 * @param InBiomeManager Reference to biome manager
	 * @param InBlendingSystem Reference to blending system
	 * @param InSeed Random seed for feature placement
	 */
	UFUNCTION(BlueprintCallable, Category = "Feature Generation")
	void Initialize(UBiomeManager* InBiomeManager, UBiomeBlendingSystem* InBlendingSystem, int32 InSeed);

	// ============================================================================
	// VEGETATION PLACEMENT
	// ============================================================================

	/**
	 * Generate vegetation placements for a terrain tile
	 * @param TileCenter Center of the tile in world space
	 * @param TileSize Size of the tile in cm
	 * @return Array of feature placements
	 */
	UFUNCTION(BlueprintCallable, Category = "Feature Generation")
	TArray<FFeaturePlacement> GenerateVegetationPlacements(FVector TileCenter, float TileSize);

	/**
	 * Check if vegetation can spawn at location
	 * @param Location World space location
	 * @param SlopeAngle Terrain slope angle in degrees
	 * @return True if vegetation can spawn
	 */
	UFUNCTION(BlueprintCallable, Category = "Feature Generation")
	bool CanSpawnVegetationAt(FVector Location, float SlopeAngle) const;

	// ============================================================================
	// DETAIL OBJECT DISTRIBUTION
	// ============================================================================

	/**
	 * Generate detail object placements (rocks, debris, etc.)
	 * @param TileCenter Center of the tile in world space
	 * @param TileSize Size of the tile in cm
	 * @return Array of feature placements
	 */
	UFUNCTION(BlueprintCallable, Category = "Feature Generation")
	TArray<FFeaturePlacement> GenerateDetailObjectPlacements(FVector TileCenter, float TileSize);

	/**
	 * Get detail objects for biome
	 * @param BiomeIndex Index of biome
	 * @return Array of detail object definitions
	 */
	UFUNCTION(BlueprintCallable, Category = "Feature Generation")
	TArray<FDetailObjectDefinition> GetDetailObjectsForBiome(int32 BiomeIndex) const;

	// ============================================================================
	// ENVIRONMENTAL EFFECTS
	// ============================================================================

	/**
	 * Get environmental effects for location
	 * @param Location World space location
	 * @return Array of environmental effects to spawn
	 */
	UFUNCTION(BlueprintCallable, Category = "Feature Generation")
	TArray<FEnvironmentalEffect> GetEnvironmentalEffectsAt(FVector Location) const;

	/**
	 * Should spawn environmental effect at location
	 * @param Effect Effect definition
	 * @param Location World space location
	 * @return True if effect should spawn
	 */
	UFUNCTION(BlueprintCallable, Category = "Feature Generation")
	bool ShouldSpawnEffect(const FEnvironmentalEffect& Effect, FVector Location) const;

	// ============================================================================
	// TEXTURE ASSIGNMENT
	// ============================================================================

	/**
	 * Get terrain textures for location based on biome
	 * @param Location World space location
	 * @return Array of material layers
	 */
	UFUNCTION(BlueprintCallable, Category = "Feature Generation")
	TArray<FTerrainMaterialLayer> GetTerrainTexturesAt(FVector Location) const;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FFeatureGenerationConfig GenerationConfig;

	// Detail object definitions per biome type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	TMap<EBiomeType, FBiomeDetailObjects> BiomeDetailObjects;

	// Environmental effects per biome type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	TMap<EBiomeType, FBiomeEnvironmentalEffects> BiomeEnvironmentalEffects;

protected:
	// References
	UPROPERTY()
	UBiomeManager* BiomeManager;

	UPROPERTY()
	UBiomeBlendingSystem* BlendingSystem;

	UPROPERTY()
	UProceduralNoiseGenerator* NoiseGenerator;

	UPROPERTY()
	int32 Seed;

	// ============================================================================
	// INTERNAL METHODS
	// ============================================================================

	/**
	 * Generate random position within tile
	 */
	FVector GenerateRandomPositionInTile(FVector TileCenter, float TileSize, int32 Index) const;

	/**
	 * Calculate density multiplier from noise
	 */
	float CalculateDensityMultiplier(FVector Location) const;

	/**
	 * Get random scale within range
	 */
	FVector GetRandomScale(FVector2D ScaleRange, int32 Index) const;

	/**
	 * Get random rotation
	 */
	FRotator GetRandomRotation(int32 Index, bool bAlignToSurface, FVector SurfaceNormal) const;

	/**
	 * Hash function for deterministic randomness
	 */
	float HashPosition(FVector Position, int32 Offset) const;

	/**
	 * Calculate surface normal at position
	 */
	FVector CalculateSurfaceNormal(FVector Position) const;
};
