// Copyright (c) 2025 Alexander Project. All rights reserved.

#include "Core/SystemModuleBase.h"
#include "Core/EventBus.h"
#include "Engine/Engine.h"

void USystemModuleBase::InitializeModule()
{
	bIsInitialized = true;
	bIsHealthy = true;
	LogSystemMessage(TEXT("System initialized successfully"));
}

void USystemModuleBase::ShutdownModule()
{
	bIsInitialized = false;
	bIsHealthy = false;
	LogSystemMessage(TEXT("System shutdown complete"));
}

void USystemModuleBase::UpdateModule(float DeltaTime)
{
	// Base implementation does nothing
	// Derived classes override this with their logic
}

FString USystemModuleBase::GetSystemName() const
{
	return GetClass()->GetName();
}

bool USystemModuleBase::IsSystemHealthy() const
{
	return bIsHealthy && bIsInitialized;
}

UEventBus* USystemModuleBase::GetEventBus() const
{
	// Find EventBus via SystemRegistry
	// This is LAW #1 in action - no direct dependencies
	return nullptr; // Will be implemented once SystemRegistry exists
}

void USystemModuleBase::LogSystemMessage(const FString& Message, bool bError) const
{
	FString SystemName = GetSystemName();
	FString FullMessage = FString::Printf(TEXT("[%s] %s"), *SystemName, *Message);
	
	if (bError)
	{
		UE_LOG(LogTemp, Error, TEXT("%s"), *FullMessage);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *FullMessage);
	}
}

UEventBus* USystemModuleBase::GetEventBus() const
{
	// This will be implemented once SystemRegistry is fully functional
	// For now, return nullptr to avoid compilation errors
	return nullptr;
}

void USystemModuleBase::PublishEvent(const FSystemEvent& Event)
{
	UEventBus* Bus = GetEventBus();
	if (Bus)
	{
		Bus->PublishEvent(Event);
	}
}