// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CropDefinition.h"
#include "CropFactory.generated.h"

/**
 * Factory class for creating predefined crop definitions
 * Provides easy access to create all standard crop types
 */
UCLASS(BlueprintType)
class ALEXANDER_API UCropFactory : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Create all standard crop definitions
	 * @return Array of all created crop definitions
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Factory")
	static TArray<UCropDefinition*> CreateAllStandardCrops();

	/**
	 * Create Space Wheat crop definition
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Factory")
	static UCropDefinition* CreateSpaceWheat();

	/**
	 * Create Lunar Potato crop definition
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Factory")
	static UCropDefinition* CreateLunarPotato();

	/**
	 * Create Martian Tomato crop definition
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Factory")
	static UCropDefinition* CreateMartianTomato();

	/**
	 * Create Titan Corn crop definition
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Factory")
	static UCropDefinition* CreateTitanCorn();

	/**
	 * Create Venusian Pepper crop definition
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Factory")
	static UCropDefinition* CreateVenusianPepper();

	/**
	 * Create Europa Kelp crop definition
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Factory")
	static UCropDefinition* CreateEuropaKelp();

	/**
	 * Create Ganymede Berry crop definition
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Factory")
	static UCropDefinition* CreateGanymedeBerry();

	/**
	 * Create Callisto Mushroom crop definition
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Factory")
	static UCropDefinition* CreateCallistoMushroom();

	/**
	 * Create Io Cactus crop definition
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Factory")
	static UCropDefinition* CreateIoCactus();

	/**
	 * Create Enceladus Rice crop definition
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Factory")
	static UCropDefinition* CreateEnceladusRice();

	/**
	 * Create Triton Herb crop definition
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Factory")
	static UCropDefinition* CreateTritonHerb();

	/**
	 * Create Pluto Flower crop definition
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Factory")
	static UCropDefinition* CreatePlutoFlower();

private:
	/**
	 * Helper to create a crop definition with common setup
	 */
	static UCropDefinition* CreateCropBase(
		const FString& Name,
		const FString& Description,
		float GrowthDuration,
		float TempMin, float TempMax,
		float HumidMin, float HumidMax,
		float WaterConsumption,
		float SoilQuality,
		float Sunlight,
		const TArray<EBiomeType>& Biomes,
		int32 YieldMin, int32 YieldMax,
		float Price,
		const FString& ItemID
	);
};
