// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Blueprint.h"
#include "SolarSystemBlueprintGenerator.generated.h"

/**
 * Solar System Blueprint Generator
 *
 * Programmatically creates all Blueprints required for the SolarSystem level:
 * - BP_CelestialBodyBase (parent class)
 * - BP_Sun, BP_Earth, BP_Moon, BP_Mars (celestial bodies)
 * - BP_SolarSystemGameMode (game mode)
 * - WBP_Chronometer, WBP_DebugOverlay, WBP_TimeWarpWarning (UI widgets)
 *
 * Usage:
 *   USolarSystemBlueprintGenerator* Generator = NewObject<USolarSystemBlueprintGenerator>();
 *   Generator->GenerateAllBlueprints();
 */
UCLASS()
class ALEXANDEREDITOR_API USolarSystemBlueprintGenerator : public UObject
{
	GENERATED_BODY()

public:
	USolarSystemBlueprintGenerator();

	/**
	 * Generate all Blueprints for SolarSystem level
	 * @return True if all Blueprints created successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Solar System | Generator")
	bool GenerateAllBlueprints();

	/**
	 * Generate only celestial body Blueprints
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Solar System | Generator")
	bool GenerateCelestialBodyBlueprints();

	/**
	 * Generate only game mode Blueprint
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Solar System | Generator")
	bool GenerateGameModeBlueprint();

	/**
	 * Generate only UI widget Blueprints
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Solar System | Generator")
	bool GenerateUIWidgetBlueprints();

	/**
	 * Generate all materials
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Solar System | Generator")
	bool GenerateMaterials();

	/**
	 * Setup SolarSystem.umap level
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Solar System | Generator")
	bool SetupSolarSystemLevel();

protected:
	// Blueprint creation functions
	UBlueprint* CreateBlueprintAsset(const FString& AssetName, const FString& PackagePath, UClass* ParentClass);
	bool AddComponentToBlueprint(UBlueprint* Blueprint, UClass* ComponentClass, const FName& ComponentName, USceneComponent* AttachTo = nullptr);
	bool SetBlueprintPropertyValue(UBlueprint* Blueprint, const FName& PropertyName, const FString& PropertyValue);
	bool CompileBlueprint(UBlueprint* Blueprint);

	// Celestial body Blueprint creation
	UBlueprint* CreateBP_CelestialBodyBase();
	UBlueprint* CreateBP_Sun();
	UBlueprint* CreateBP_Earth();
	UBlueprint* CreateBP_Moon();
	UBlueprint* CreateBP_Mars();

	// Game mode Blueprint creation
	UBlueprint* CreateBP_SolarSystemGameMode();

	// VR Pawn configuration (modify existing)
	bool ConfigureBP_VRSpaceshipPlayer();

	// UI Widget creation
	UBlueprint* CreateWBP_Chronometer();
	UBlueprint* CreateWBP_DebugOverlay();
	UBlueprint* CreateWBP_TimeWarpWarning();

	// Material creation
	UMaterial* CreateM_Sun_Emissive();
	UMaterial* CreateM_Earth_Simple();
	UMaterial* CreateM_Moon_Simple();
	UMaterial* CreateM_Mars_Simple();

	// Level setup
	bool PlaceActorsInLevel(UWorld* World);
	bool ConfigureLevelLighting(UWorld* World);
	bool ConfigurePostProcessing(UWorld* World);

	// Utility functions
	FString GetContentPath(const FString& SubPath) const;
	bool SaveAsset(UObject* Asset, const FString& PackagePath);
	void LogProgress(const FString& Message);
	void LogError(const FString& Message);

private:
	// Configuration
	UPROPERTY()
	FString BaseBlueprintPath;

	UPROPERTY()
	FString BaseMaterialPath;

	UPROPERTY()
	FString BaseWidgetPath;

	// Progress tracking
	UPROPERTY()
	int32 TotalSteps;

	UPROPERTY()
	int32 CurrentStep;

	UPROPERTY()
	TArray<FString> CreatedAssets;

	UPROPERTY()
	TArray<FString> FailedAssets;
};
