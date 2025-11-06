// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "SolarSystemGeneratorWidget.generated.h"

/**
 * Editor Utility Widget for Solar System Blueprint Generation
 *
 * Provides a UI interface in the Editor to run the Blueprint generator
 *
 * Usage:
 * 1. In Content Browser: Right-click → Editor Utilities → Editor Utility Widget
 * 2. Parent Class: SolarSystemGeneratorWidget
 * 3. Design UI with buttons
 * 4. Run from Tools → Blutilities menu
 */
UCLASS()
class ALEXANDEREDITOR_API USolarSystemGeneratorWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	/**
	 * Generate all Blueprints for Solar System
	 * Call this from a button in the widget
	 */
	UFUNCTION(BlueprintCallable, Category = "Solar System | Generator")
	void GenerateAllBlueprints();

	/**
	 * Generate only celestial body Blueprints
	 */
	UFUNCTION(BlueprintCallable, Category = "Solar System | Generator")
	void GenerateCelestialBodies();

	/**
	 * Generate only game mode Blueprint
	 */
	UFUNCTION(BlueprintCallable, Category = "Solar System | Generator")
	void GenerateGameMode();

	/**
	 * Generate only UI widgets
	 */
	UFUNCTION(BlueprintCallable, Category = "Solar System | Generator")
	void GenerateUIWidgets();

	/**
	 * Generate only materials
	 */
	UFUNCTION(BlueprintCallable, Category = "Solar System | Generator")
	void GenerateMaterials();

	/**
	 * Setup SolarSystem level
	 */
	UFUNCTION(BlueprintCallable, Category = "Solar System | Generator")
	void SetupLevel();

protected:
	/** Display progress message */
	UFUNCTION(BlueprintImplementableEvent, Category = "Solar System | Generator")
	void DisplayProgress(const FString& Message);

	/** Display error message */
	UFUNCTION(BlueprintImplementableEvent, Category = "Solar System | Generator")
	void DisplayError(const FString& Message);

	/** Display success message */
	UFUNCTION(BlueprintImplementableEvent, Category = "Solar System | Generator")
	void DisplaySuccess(const FString& Message);
};
