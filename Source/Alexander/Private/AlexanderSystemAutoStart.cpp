// Copyright Epic Games, Inc. All Rights Reserved.

#include "AlexanderSystemAutoStart.h"
#include "AutomationAPIServer.h"
#include "Engine/Engine.h"

UAutomationAPIServer* FAlexanderSystemAutoStart::AutomationServer = nullptr;

void FAlexanderSystemAutoStart::Initialize()
{
#if WITH_EDITOR
	UE_LOG(LogTemp, Log, TEXT("AlexanderSystemAutoStart: Initializing automation server for editor mode"));

	if (!AutomationServer)
	{
		// Create the automation server component as a standalone object
		AutomationServer = NewObject<UAutomationAPIServer>(
			GetTransientPackage(),
			UAutomationAPIServer::StaticClass(),
			TEXT("EditorAutomationServer"),
			RF_Standalone | RF_Public
		);

		if (AutomationServer)
		{
			// Start the HTTP server
			if (AutomationServer->StartServer())
			{
				UE_LOG(LogTemp, Log, TEXT("AlexanderSystemAutoStart: Automation API Server started successfully on port 8080"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("AlexanderSystemAutoStart: Failed to start automation server"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("AlexanderSystemAutoStart: Failed to create automation server object"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AlexanderSystemAutoStart: Automation server already initialized"));
	}
#endif
}

void FAlexanderSystemAutoStart::Shutdown()
{
#if WITH_EDITOR
	UE_LOG(LogTemp, Log, TEXT("AlexanderSystemAutoStart: Shutting down automation server"));

	if (AutomationServer)
	{
		AutomationServer->StopServer();
		AutomationServer = nullptr;
	}
#endif
}
