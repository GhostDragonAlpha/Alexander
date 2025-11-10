// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Static class that handles auto-starting the AutomationAPIServer in editor mode
 * This enables automated testing to work even when PIE is not running
 */
class FAlexanderSystemAutoStart
{
public:
	/** Initialize and start the automation server in editor mode */
	static void Initialize();

	/** Shutdown the automation server */
	static void Shutdown();

private:
	/** Reference to the automation server */
	static class UAutomationAPIServer* AutomationServer;
};
