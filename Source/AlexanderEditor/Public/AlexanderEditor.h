// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * Alexander Editor Module
 *
 * Provides editor utilities for:
 * - Programmatic Blueprint creation
 * - Material generation
 * - Level setup automation
 * - Asset management
 */
class FAlexanderEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void Shutdown Module() override;

private:
	/** Register menu extensions */
	void RegisterMenuExtensions();

	/** Register editor commands */
	void RegisterCommands();
};
