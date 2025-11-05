// AIVisionSystem.h - Main module header

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FAIVisionSystemModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
