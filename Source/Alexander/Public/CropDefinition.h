// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlanetConfiguration.h"
#include "CropDefinition.generated.h"

/**
 * Crop float range structure
 */
USTRUCT(BlueprintType)
struct FCropFloatRange
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range")
	float Min;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range")
	float Max;

	FCropFloatRange()
		: Min(0.0f)
		, Max(1.0f)
	{
	}

	FCropFloatRange(float InMin, float InMax)
		: Min(InMin)
		, Max(InMax)
	{
	}

	bool Contains(float Value) const
	{
		return Value >= Min && Value <= Max;
	}
};

/**
 * Crop integer range structure
 */
USTRUCT(BlueprintType)
struct FCropIntRange
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range")
	int32 Min;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Range")
	int32 Max;

	FCropIntRange()
		: Min(0)
		, Max(10)
	{
	}

	FCropIntRange(int32 InMin, int32 InMax)
		: Min(InMin)
		, Max(InMax)
	{
	}
};

/**
 * Crop definition data asset
 * Defines all properties and requirements for a specific crop type
 */
UCLASS(BlueprintType)
class ALEXANDER_API UCropDefinition : public UDataAsset
{
	GENERATED_BODY()

public:
	UCropDefinition();

	// ============================================================================
	// IDENTITY
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString CropName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText CropDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	UTexture2D* CropIcon;

	// ============================================================================
	// VISUAL REPRESENTATION
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	UStaticMesh* CropMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TArray<UStaticMesh*> GrowthStageMeshes; // 4-6 stages

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	UMaterialInterface* CropMaterial;

	// ============================================================================
	// GROWTH REQUIREMENTS
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth", meta = (ClampMin = "60.0", ClampMax = "3600.0"))
	float GrowthDuration; // real-time seconds (1-60 minutes)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth")
	FCropFloatRange OptimalTemperature; // Celsius

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	FCropFloatRange OptimalHumidity; // 0-1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float WaterConsumption; // liters per day

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SoilQualityRequirement; // 0-1

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Growth", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SunlightRequirement; // 0-1 (0 = shade tolerant, 1 = full sun)

	// ============================================================================
	// BIOME COMPATIBILITY
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
	TArray<EBiomeType> SuitableBiomes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BiomeAdaptability; // 0-1, how well it grows outside optimal biomes

	// ============================================================================
	// HARVEST
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	float BaseYield;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	FCropIntRange YieldAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest")
	FString HarvestItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest", meta = (ClampMin = "0.0"))
	float SellPrice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Harvest", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RegrowthChance; // 0-1, chance crop regrows after harvest

	// ============================================================================
	// SPECIAL PROPERTIES
	// ============================================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special")
	bool bRequiresFertilizer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special")
	bool bRequiresIrrigation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special")
	bool bResistantToPests;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special")
	bool bResistantToDrought;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FertilityDepletion; // 0-1, how much it depletes soil per harvest

	// ============================================================================
	// HELPER FUNCTIONS
	// ============================================================================

	/**
	 * Check if this crop is suitable for a specific biome
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop")
	bool IsSuitableForBiome(EBiomeType BiomeType) const;

	/**
	 * Get growth stage mesh for a given progress (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop")
	UStaticMesh* GetGrowthStageMesh(float GrowthProgress) const;

	/**
	 * Calculate growth rate modifier based on environmental conditions
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop")
	float CalculateGrowthModifier(float Temperature, float Humidity, float Sunlight, float SoilQuality) const;

	/**
	 * Get expected harvest value
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop")
	float GetExpectedHarvestValue() const;

	/**
	 * Get number of growth stages
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop")
	int32 GetNumGrowthStages() const;

	/**
	 * Get growth stage index for progress
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop")
	int32 GetGrowthStageIndex(float GrowthProgress) const;

	/**
	 * Get growth stage name
	 */
	UFUNCTION(BlueprintCallable, Category = "Crop")
	FString GetGrowthStageName(float GrowthProgress) const;
};
