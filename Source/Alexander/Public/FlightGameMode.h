// Copyright (c) 2025 Alexander Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FlightGameMode.generated.h"

class USystemRegistry;
class UFlightController;
class UResourceGatheringSystem;

/**
 * FlightGameMode - Minimal game mode for testing the LEGO Architecture
 * 
 * This game mode demonstrates how to properly initialize and use the
 * LEGO Architecture systems. It follows The Three Laws:
 * 
 * LAW #1: No Direct Dependencies - Uses SystemRegistry to find systems
 * LAW #2: Event-Driven Communication - Systems communicate via EventBus
 * LAW #3: Health Monitoring - Always checks IsSystemHealthy()
 */
UCLASS()
class ALEXANDER_API AFlightGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AFlightGameMode();
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Called when the game ends
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// Initialize all systems
	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	void InitializeGameSystems();
	
	// Get the FlightController system
	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	UFlightController* GetFlightController() const;
	
	// Get the ResourceGatheringSystem
	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	UResourceGatheringSystem* GetResourceGatheringSystem() const;
	
protected:
	// The system registry (our LEGO baseplate)
	UPROPERTY()
	USystemRegistry* SystemRegistry = nullptr;
	
	// Cached reference to flight controller
	UPROPERTY()
	UFlightController* FlightController = nullptr;
	
	// Cached reference to resource gathering system
	UPROPERTY()
	UResourceGatheringSystem* ResourceGatheringSystem = nullptr;
	
	// Flag to track if systems are initialized
	bool bSystemsInitialized = false;
};