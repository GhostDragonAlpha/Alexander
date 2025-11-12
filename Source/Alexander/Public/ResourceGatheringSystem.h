// Copyright (c) 2025 Alexander Project. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/SystemModuleBase.h"
#include "ResourceGatheringSystem.generated.h"

/**
 * ResourceGatheringSystem - Asteroid mining and resource collection
 * 
 * This system handles scanning for resources, mining asteroids, and collecting materials.
 * It works with the FlightController to mine when near asteroids and with InventoryManager
 * to store collected resources.
 * 
 * Implements the 5-method LEGO pattern from SystemModuleBase.
 * 
 * Communication via EventBus:
 * - Subscribes to: "ShipMoved", "MiningLaserActivated"
 * - Publishes: "ResourceScanned", "ResourceCollected", "MiningComplete"
 */
UCLASS(Blueprintable, BlueprintType)
class ALEXANDER_API UResourceGatheringSystem : public USystemModuleBase
{
	GENERATED_BODY()
	
public:
	// Implement the 5 LEGO methods
	virtual void InitializeModule() override;
	virtual void ShutdownModule() override;
	virtual void UpdateModule(float DeltaTime) override;
	virtual FString GetSystemName() const override;
	virtual bool IsSystemHealthy() const override;
	
	// Resource gathering methods
	UFUNCTION(BlueprintCallable, Category = "Resources")
	void StartMining(AActor* Target);
	
	UFUNCTION(BlueprintCallable, Category = "Resources")
	void StopMining();
	
	UFUNCTION(BlueprintCallable, Category = "Resources")
	bool IsMining() const { return bIsCurrentlyMining; }
	
	UFUNCTION(BlueprintCallable, Category = "Resources")
	float GetCurrentYield() const { return CurrentYield; }
	
protected:
	// Event handlers
	void HandleShipMoved(const FSystemEvent& Event);
	void HandleMiningLaserActivated(const FSystemEvent& Event);
	
	// Mining logic
	void ScanForResources();
	void CollectResources(float DeltaTime);
	void CompleteMining();
	
	// Mining state
	UPROPERTY()
	AActor* CurrentMiningTarget = nullptr;
	
	UPROPERTY()
	bool bIsCurrentlyMining = false;
	
	UPROPERTY()
	float MiningProgress = 0.0f;
	
	UPROPERTY()
	float CurrentYield = 0.0f;
	
	UPROPERTY()
	float MiningEfficiency = 1.0f;
	
	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining Settings")
	float MiningRange = 500.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining Settings")
	float MiningSpeed = 10.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mining Settings")
	float BaseYieldPerSecond = 5.0f;
};