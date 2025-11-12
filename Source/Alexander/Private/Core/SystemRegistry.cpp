// Copyright (c) 2025 Alexander Project. All rights reserved.

#include "Core/SystemRegistry.h"
#include "Core/SystemModuleBase.h"
#include "Core/EventBus.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

USystemRegistry* USystemRegistry::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}
	
	// Get the game instance
	UGameInstance* GameInstance = WorldContextObject->GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}
	
	// Create or get the registry from the game instance
	USystemRegistry* Registry = GameInstance->GetSubsystem<USystemRegistry>();
	if (!Registry)
	{
		Registry = NewObject<USystemRegistry>(GameInstance);
		GameInstance->AddSubsystem(Registry);
	}
	
	return Registry;
}

bool USystemRegistry::RegisterModule(USystemModuleBase* Module)
{
	if (!Module)
	{
		UE_LOG(LogTemp, Error, TEXT("SystemRegistry: Attempted to register null module"));
		return false;
	}
	
	FString ModuleName = Module->GetSystemName();
	
	if (ModuleName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("SystemRegistry: Module has empty name"));
		return false;
	}
	
	if (RegisteredModules.Contains(ModuleName))
	{
		UE_LOG(LogTemp, Warning, TEXT("SystemRegistry: Module '%s' already registered"), *ModuleName);
		return false;
	}
	
	RegisteredModules.Add(ModuleName, Module);
	RegistrationOrder.Add(ModuleName);
	
	UE_LOG(LogTemp, Log, TEXT("SystemRegistry: Registered module '%s'"), *ModuleName);
	return true;
}

bool USystemRegistry::UnregisterModule(const FString& ModuleName)
{
	if (!RegisteredModules.Contains(ModuleName))
	{
		UE_LOG(LogTemp, Warning, TEXT("SystemRegistry: Module '%s' not registered"), *ModuleName);
		return false;
	}
	
	RegisteredModules.Remove(ModuleName);
	RegistrationOrder.Remove(ModuleName);
	
	UE_LOG(LogTemp, Log, TEXT("SystemRegistry: Unregistered module '%s'"), *ModuleName);
	return true;
}

USystemModuleBase* USystemRegistry::GetModule(const FString& ModuleName) const
{
	if (!RegisteredModules.Contains(ModuleName))
	{
		return nullptr;
	}
	
	TWeakObjectPtr<USystemModuleBase> ModulePtr = RegisteredModules[ModuleName];
	return ModulePtr.Get();
}

TArray<FString> USystemRegistry::GetAllModuleNames() const
{
	TArray<FString> Names;
	RegisteredModules.GetKeys(Names);
	return Names;
}

void USystemRegistry::InitializeAllModules()
{
	UE_LOG(LogTemp, Log, TEXT("SystemRegistry: Initializing all modules (%d total)"), RegisteredModules.Num());
	
	for (const FString& ModuleName : RegistrationOrder)
	{
		USystemModuleBase* Module = GetModule(ModuleName);
		if (Module)
		{
			Module->InitializeModule();
		}
	}
	
	// Initialize EventBus
	if (!EventBus)
	{
		EventBus = NewObject<UEventBus>(this);
	}
	EventBus->InitializeEventBus();
}

void USystemRegistry::ShutdownAllModules()
{
	UE_LOG(LogTemp, Log, TEXT("SystemRegistry: Shutting down all modules"));
	
	// Shutdown EventBus first
	if (EventBus)
	{
		EventBus->ShutdownEventBus();
	}
	
	// Shutdown modules in reverse registration order
	for (int32 i = RegistrationOrder.Num() - 1; i >= 0; i--)
	{
		USystemModuleBase* Module = GetModule(RegistrationOrder[i]);
		if (Module)
		{
			Module->ShutdownModule();
		}
	}
}

void USystemRegistry::UpdateAllModules(float DeltaTime)
{
	for (const FString& ModuleName : RegistrationOrder)
	{
		USystemModuleBase* Module = GetModule(ModuleName);
		if (Module && Module->IsSystemHealthy())
		{
			Module->UpdateModule(DeltaTime);
		}
	}
}

TMap<FString, bool> USystemRegistry::GetAllModuleHealth() const
{
	TMap<FString, bool> HealthMap;
	
	for (const auto& Pair : RegisteredModules)
	{
		bool bHealthy = false;
		if (Pair.Value.IsValid())
		{
			bHealthy = Pair.Value->IsSystemHealthy();
		}
		HealthMap.Add(Pair.Key, bHealthy);
	}
	
	return HealthMap;
}