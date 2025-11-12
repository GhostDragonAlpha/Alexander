// Copyright (c) 2025 Alexander Project. All rights reserved.

#include "FlightGameMode.h"
#include "Core/SystemRegistry.h"
#include "FlightController.h"
#include "ResourceGatheringSystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

AFlightGameMode::AFlightGameMode()
{
	// Set default pawn class (will be overridden in Blueprint)
	DefaultPawnClass = nullptr;
}

void AFlightGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize game systems
	InitializeGameSystems();
}

void AFlightGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Update all systems through the registry
	if (SystemRegistry && bSystemsInitialized)
	{
		SystemRegistry->UpdateAllModules(DeltaTime);
	}
}

void AFlightGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Shutdown all systems
	if (SystemRegistry && bSystemsInitialized)
	{
		SystemRegistry->ShutdownAllModules();
		bSystemsInitialized = false;
	}
	
	Super::EndPlay(EndPlayReason);
}

void AFlightGameMode::InitializeGameSystems()
{
	UE_LOG(LogTemp, Log, TEXT("FlightGameMode: Initializing game systems..."));
	
	// Get or create the SystemRegistry
	SystemRegistry = USystemRegistry::Get(this);
	if (!SystemRegistry)
	{
		UE_LOG(LogTemp, Error, TEXT("FlightGameMode: Failed to get SystemRegistry!"));
		return;
	}
	
	// Create and register the FlightController
	FlightController = NewObject<UFlightController>(this);
	if (FlightController)
	{
		SystemRegistry->RegisterModule(FlightController);
		UE_LOG(LogTemp, Log, TEXT("FlightGameMode: Registered FlightController"));
	}
	
	// Create and register the ResourceGatheringSystem
	ResourceGatheringSystem = NewObject<UResourceGatheringSystem>(this);
	if (ResourceGatheringSystem)
	{
		SystemRegistry->RegisterModule(ResourceGatheringSystem);
		UE_LOG(LogTemp, Log, TEXT("FlightGameMode: Registered ResourceGatheringSystem"));
	}
	
	// Initialize all registered modules
	SystemRegistry->InitializeAllModules();
	bSystemsInitialized = true;
	
	UE_LOG(LogTemp, Log, TEXT("FlightGameMode: All systems initialized successfully"));
}

UFlightController* AFlightGameMode::GetFlightController() const
{
	return FlightController;
}

UResourceGatheringSystem* AFlightGameMode::GetResourceGatheringSystem() const
{
	return ResourceGatheringSystem;
}