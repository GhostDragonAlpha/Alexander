// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PlanetConfiguration.h"
#include "BiomeManager.h"
#include "BiomeBlendingSystem.generated.h"

/**
 * Blended terrain parameters from multiple biomes
 */
USTRUCT(BlueprintType)
struct FBlendedTerrainParameters
{
	GENERATED_BODY()

	// Blended elevation offset
	UPROPERTY(BlueprintReadOnly, Category = "Terrain")
	float Elevation = 0.0f;

	// Blended roughness
	UPROPERTY(BlueprintReadOnly, Category = "Terrain")
	float Roughness = 0.5f;

	// Blended temperature
	UPROPERTY(BlueprintReadOnly, Category = "Terrain")
	float Temperature = 20.0f;

	// Blended humidity
	UPROPERTY(BlueprintReadOnly, Category = "Terrain")
	float Humidity = 0.5f;

	// Blended vegetation density
	UPROPERTY(BlueprintReadOnly, Category = "Vegetation")
	float VegetationDensity = 0.5f;

	// Biome weights for material blending
	UPROPERTY(BlueprintReadOnly, Category = "Materials")
	TMap<int32, float> BiomeWeights;
};

/**
 * Transition zone configuration
 */
USTRUCT(BlueprintType)
struct FBiomeTransitionConfig
{
	GENERATED_BODY()

	// Transition zone width in meters (50-100m as per requirements)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float TransitionZoneWidth = 75.0f;

	// Smoothness of transition (0-1, higher = smoother)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float TransitionSmoothness = 0.7f;

	// Whether to use distance-based blending
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	bool bUseDistanceBlending = true;

	// Minimum weight threshold for biome inclusion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	float MinimumBiomeWeight = 0.05f;
};

/**
 * Manages smooth blending between biomes
 */
UCLASS(BlueprintType)
class ALEXANDER_API UBiomeBlendingSystem : public UObject
{
	GENERATED_BODY()

public:
	UBiomeBlendingSystem();

	// ============================================================================
	// INITIALIZATION
	// ============================================================================

	/**
	 * Initialize blending system with biome manager
	 * @param InBiomeManager Reference to biome manager
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome Blending")
	void Initialize(UBiomeManager* InBiomeManager);

	// ============================================================================
	// BLENDING QUERIES
	// ============================================================================

	/**
	 * Get blended terrain parameters at location
	 * @param WorldPosition World space position
	 * @return Blended terrain parameters
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome Blending")
	FBlendedTerrainParameters GetBlendedParameters(FVector WorldPosition) const;

	/**
	 * Calculate smooth biome weights with transition zones
	 * @param WorldPosition World space position
	 * @return Smoothed biome weights
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome Blending")
	FBiomeWeights CalculateSmoothedWeights(FVector WorldPosition) const;

	/**
	 * Check if position is in a transition zone
	 * @param WorldPosition World space position
	 * @return True if in transition zone (multiple biomes present)
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome Blending")
	bool IsInTransitionZone(FVector WorldPosition) const;

	/**
	 * Get transition factor (0 = pure biome, 1 = maximum blending)
	 * @param WorldPosition World space position
	 * @return Transition factor (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome Blending")
	float GetTransitionFactor(FVector WorldPosition) const;

	// ============================================================================
	// FEATURE MIXING
	// ============================================================================

	/**
	 * Mix vegetation types from multiple biomes
	 * @param BiomeWeights Weights of biomes at location
	 * @return Array of vegetation definitions with spawn probabilities
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome Blending")
	TArray<FVegetationDefinition> MixVegetationTypes(const FBiomeWeights& BiomeWeights) const;

	/**
	 * Blend material layers from multiple biomes
	 * @param BiomeWeights Weights of biomes at location
	 * @return Array of material layers with blend weights
	 */
	UFUNCTION(BlueprintCallable, Category = "Biome Blending")
	TArray<FTerrainMaterialLayer> BlendMaterialLayers(const FBiomeWeights& BiomeWeights) const;

	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	FBiomeTransitionConfig TransitionConfig;

protected:
	// Reference to biome manager
	UPROPERTY()
	UBiomeManager* BiomeManager;

	// ============================================================================
	// INTERNAL METHODS
	// ============================================================================

	/**
	 * Apply smoothing to biome weights based on transition zones
	 */
	void ApplyTransitionSmoothing(FBiomeWeights& Weights, FVector WorldPosition) const;

	/**
	 * Calculate distance-based weight adjustment
	 */
	float CalculateDistanceWeight(FVector WorldPosition, FVector SamplePosition) const;

	/**
	 * Sample biome weights in a radius around position
	 */
	FBiomeWeights SampleWeightsInRadius(FVector WorldPosition, float Radius, int32 NumSamples) const;

	/**
	 * Apply smoothstep function for smooth transitions
	 */
	float SmoothStep(float Edge0, float Edge1, float X) const;

	/**
	 * Blend two float values based on weight
	 */
	float BlendValues(float Value1, float Value2, float Weight) const;
};
