// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CropDefinition.h"
#include "CropDefinitionFactory.generated.h"

/**
 * Factory class for creating predefined crop definitions
 * Provides easy access to all standard crop types
 */
UCLASS()
class ALEXANDER_API UCropDefinitionFactory : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Create all standard crop definitions
	 * Returns array of all available crops
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Factory")
	static TArray<UCropDefinition*> CreateAllCropDefinitions(UObject* Outer);

	/**
	 * Create a specific crop by name
	 */
	UFUNCTION(BlueprintCallable, Category = "Farming|Factory")
	static UCropDefinition* CreateCropByName(const FString& CropName, UObject* Outer);

	// Individual crop creation functions
	static UCropDefinition* CreateWheat(UObject* Outer);
	static UCropDefinition* CreateCorn(UObject* Outer);
	static UCropDefinition* CreateRice(UObject* Outer);
	static UCropDefinition* CreateTomatoes(UObject* Outer);
	static UCropDefinition* CreatePotatoes(UObject* Outer);
	static UCropDefinition* CreateCarrots(UObject* Outer);
	static UCropDefinition* CreateSoybeans(UObject* Outer);
	static UCropDefinition* CreateBeans(UObject* Outer);
	static UCropDefinition* CreateCotton(UObject* Outer);
	static UCropDefinition* CreateCoffee(UObject* Outer);
	static UCropDefinition* CreateCacao(UObject* Outer);
	static UCropDefinition* CreateBarley(UObject* Outer);
	static UCropDefinition* CreateTurnips(UObject* Outer);

private:
	/**
	 * Helper function to set common properties
	 */
	static void SetupCropBase(UCropDefinition* Crop, UObject* Outer);
};

