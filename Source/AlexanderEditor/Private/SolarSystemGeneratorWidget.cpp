// Copyright Epic Games, Inc. All Rights Reserved.

#include "SolarSystemGeneratorWidget.h"
#include "SolarSystemBlueprintGenerator.h"

void USolarSystemGeneratorWidget::GenerateAllBlueprints()
{
	DisplayProgress(TEXT("Starting full Blueprint generation..."));

	USolarSystemBlueprintGenerator* Generator = NewObject<USolarSystemBlueprintGenerator>();
	if (!Generator)
	{
		DisplayError(TEXT("Failed to create generator object"));
		return;
	}

	bool bSuccess = Generator->GenerateAllBlueprints();

	if (bSuccess)
	{
		DisplaySuccess(TEXT("✓ All Blueprints generated successfully!"));
	}
	else
	{
		DisplayError(TEXT("Some Blueprints failed to generate. Check Output Log for details."));
	}
}

void USolarSystemGeneratorWidget::GenerateCelestialBodies()
{
	DisplayProgress(TEXT("Generating celestial body Blueprints..."));

	USolarSystemBlueprintGenerator* Generator = NewObject<USolarSystemBlueprintGenerator>();
	if (!Generator)
	{
		DisplayError(TEXT("Failed to create generator object"));
		return;
	}

	bool bSuccess = Generator->GenerateCelestialBodyBlueprints();

	if (bSuccess)
	{
		DisplaySuccess(TEXT("✓ Celestial body Blueprints generated!"));
	}
	else
	{
		DisplayError(TEXT("Failed to generate celestial bodies. Check Output Log."));
	}
}

void USolarSystemGeneratorWidget::GenerateGameMode()
{
	DisplayProgress(TEXT("Generating game mode Blueprint..."));

	USolarSystemBlueprintGenerator* Generator = NewObject<USolarSystemBlueprintGenerator>();
	if (!Generator)
	{
		DisplayError(TEXT("Failed to create generator object"));
		return;
	}

	bool bSuccess = Generator->GenerateGameModeBlueprint();

	if (bSuccess)
	{
		DisplaySuccess(TEXT("✓ Game mode Blueprint generated!"));
	}
	else
	{
		DisplayError(TEXT("Failed to generate game mode. Check Output Log."));
	}
}

void USolarSystemGeneratorWidget::GenerateUIWidgets()
{
	DisplayProgress(TEXT("Generating UI widget Blueprints..."));

	USolarSystemBlueprintGenerator* Generator = NewObject<USolarSystemBlueprintGenerator>();
	if (!Generator)
	{
		DisplayError(TEXT("Failed to create generator object"));
		return;
	}

	bool bSuccess = Generator->GenerateUIWidgetBlueprints();

	if (bSuccess)
	{
		DisplaySuccess(TEXT("✓ UI widget Blueprints generated!"));
	}
	else
	{
		DisplayError(TEXT("Failed to generate UI widgets. Check Output Log."));
	}
}

void USolarSystemGeneratorWidget::GenerateMaterials()
{
	DisplayProgress(TEXT("Generating materials..."));

	USolarSystemBlueprintGenerator* Generator = NewObject<USolarSystemBlueprintGenerator>();
	if (!Generator)
	{
		DisplayError(TEXT("Failed to create generator object"));
		return;
	}

	bool bSuccess = Generator->GenerateMaterials();

	if (bSuccess)
	{
		DisplaySuccess(TEXT("✓ Materials generated!"));
	}
	else
	{
		DisplayError(TEXT("Failed to generate materials. Check Output Log."));
	}
}

void USolarSystemGeneratorWidget::SetupLevel()
{
	DisplayProgress(TEXT("Setting up SolarSystem level..."));

	USolarSystemBlueprintGenerator* Generator = NewObject<USolarSystemBlueprintGenerator>();
	if (!Generator)
	{
		DisplayError(TEXT("Failed to create generator object"));
		return;
	}

	bool bSuccess = Generator->SetupSolarSystemLevel();

	if (bSuccess)
	{
		DisplaySuccess(TEXT("✓ Level setup complete!"));
	}
	else
	{
		DisplayError(TEXT("Failed to setup level. Check Output Log."));
	}
}
