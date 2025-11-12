// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ResourceGatheringSystem.h"
#include "SystemEvents.generated.h"

/**
 * Cross-System Event Broadcasting
 *
 * This file defines delegates for inter-system communication.
 * Systems can broadcast events that other systems can subscribe to,
 * enabling proper data flow and integration throughout the game.
 */

// ============================================================================
// RESOURCE EXTRACTION EVENTS
// ============================================================================

// Broadcast when resources are extracted from asteroids or planets
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnResourceExtracted,
	FName, ResourceType,
	int32, Quantity,
	EResourceQuality, Quality
);

// Broadcast when a mining operation completes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnMiningOperationCompleted,
	FName, OperationType, // "Asteroid" or "Planetary"
	bool, bSuccess
);

// ============================================================================
// CRAFTING AND REFINING EVENTS
// ============================================================================

// Broadcast when an item is crafted
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnItemCrafted,
	FName, ItemType,
	int32, Quantity,
	EResourceQuality, Quality
);

// Broadcast when resources are refined
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnResourcesRefined,
	FName, RecipeID,
	int32, OutputQuantity,
	bool, bSuccess
);

// Broadcast when crafting or refining consumes resources
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnResourcesConsumed,
	FName, ResourceType,
	int32, Quantity
);

// ============================================================================
// INVENTORY EVENTS
// ============================================================================

// Broadcast when resources are added to any inventory
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FOnResourcesAddedToInventory,
	FName, ResourceType,
	int32, Quantity,
	FString, InventoryLocation, // "ShipCargo", "BaseStorage", etc.
	FGuid, OwnerID
);

// Broadcast when resources are removed from any inventory
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FOnResourcesRemovedFromInventory,
	FName, ResourceType,
	int32, Quantity,
	FString, InventoryLocation,
	FGuid, OwnerID
);

// Broadcast when inventory transfer occurs
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(
	FOnInventoryTransferCompleted,
	FName, ResourceType,
	int32, Quantity,
	FString, SourceLocation,
	FString, DestinationLocation,
	bool, bSuccess
);

// ============================================================================
// ECONOMY EVENTS
// ============================================================================

// Broadcast when a trade transaction completes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(
	FOnTradeCompleted,
	FName, ResourceType,
	int32, Quantity,
	float, Price,
	FString, TradingStation
);

// Broadcast when market prices change significantly
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnMarketPriceChanged,
	FName, ResourceType,
	float, OldPrice,
	float, NewPrice
);

// Broadcast when an economic event occurs
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnEconomicEventTriggered,
	FString, EventName,
	FString, EventDescription
);

// ============================================================================
// FARMING EVENTS
// ============================================================================

// Broadcast when crops are harvested
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnCropsHarvested,
	FName, CropType,
	int32, Quantity,
	float, Quality
);

// Broadcast when crops are planted
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnCropsPlanted,
	FName, CropType,
	int32, PlotCount
);

// Broadcast when crop state changes (growth, health, etc.)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnCropStateChanged,
	FName, CropType,
	FString, StateName, // "Growing", "Ready", "Withered"
	float, StateValue
);

// ============================================================================
// PERSISTENCE EVENTS
// ============================================================================

// Broadcast when game state is saved
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnGameStateSaved,
	FString, SaveName,
	bool, bSuccess
);

// Broadcast when game state is loaded
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnGameStateLoaded,
	FString, SaveName,
	bool, bSuccess
);

// ============================================================================
// SYSTEM INITIALIZATION EVENTS
// ============================================================================

// Broadcast when a major system initializes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnSystemInitialized,
	FString, SystemName,
	bool, bSuccess
);

// Broadcast when all systems are ready
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllSystemsReady);

/**
 * System Event Hub - Central hub for cross-system event management
 *
 * This class provides a centralized location for systems to broadcast
 * and subscribe to events, facilitating loose coupling and proper
 * integration between different game systems.
 */
UCLASS()
class ALEXANDER_API USystemEventHub : public UObject
{
	GENERATED_BODY()

public:
	// Singleton instance
	static USystemEventHub* GetInstance();

	// Resource Extraction Events
	UPROPERTY(BlueprintAssignable, Category = "System Events|Resources")
	FOnResourceExtracted OnResourceExtracted;

	UPROPERTY(BlueprintAssignable, Category = "System Events|Resources")
	FOnMiningOperationCompleted OnMiningOperationCompleted;

	// Crafting and Refining Events
	UPROPERTY(BlueprintAssignable, Category = "System Events|Crafting")
	FOnItemCrafted OnItemCrafted;

	UPROPERTY(BlueprintAssignable, Category = "System Events|Crafting")
	FOnResourcesRefined OnResourcesRefined;

	UPROPERTY(BlueprintAssignable, Category = "System Events|Crafting")
	FOnResourcesConsumed OnResourcesConsumed;

	// Inventory Events
	UPROPERTY(BlueprintAssignable, Category = "System Events|Inventory")
	FOnResourcesAddedToInventory OnResourcesAddedToInventory;

	UPROPERTY(BlueprintAssignable, Category = "System Events|Inventory")
	FOnResourcesRemovedFromInventory OnResourcesRemovedFromInventory;

	UPROPERTY(BlueprintAssignable, Category = "System Events|Inventory")
	FOnInventoryTransferCompleted OnInventoryTransferCompleted;

	// Economy Events
	UPROPERTY(BlueprintAssignable, Category = "System Events|Economy")
	FOnTradeCompleted OnTradeCompleted;

	UPROPERTY(BlueprintAssignable, Category = "System Events|Economy")
	FOnMarketPriceChanged OnMarketPriceChanged;

	UPROPERTY(BlueprintAssignable, Category = "System Events|Economy")
	FOnEconomicEventTriggered OnEconomicEventTriggered;

	// Farming Events
	UPROPERTY(BlueprintAssignable, Category = "System Events|Farming")
	FOnCropsHarvested OnCropsHarvested;

	UPROPERTY(BlueprintAssignable, Category = "System Events|Farming")
	FOnCropsPlanted OnCropsPlanted;

	UPROPERTY(BlueprintAssignable, Category = "System Events|Farming")
	FOnCropStateChanged OnCropStateChanged;

	// Persistence Events
	UPROPERTY(BlueprintAssignable, Category = "System Events|Persistence")
	FOnGameStateSaved OnGameStateSaved;

	UPROPERTY(BlueprintAssignable, Category = "System Events|Persistence")
	FOnGameStateLoaded OnGameStateLoaded;

	// System Initialization Events
	UPROPERTY(BlueprintAssignable, Category = "System Events|System")
	FOnSystemInitialized OnSystemInitialized;

	UPROPERTY(BlueprintAssignable, Category = "System Events|System")
	FOnAllSystemsReady OnAllSystemsReady;

private:
	static USystemEventHub* Instance;
};
