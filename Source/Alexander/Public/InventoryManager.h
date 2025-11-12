// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ResourceGatheringSystem.h"
#include "InventoryManager.generated.h"

// Forward declarations
class ASpaceship;
class ABaseModule;
class APlayerCharacter;

// Inventory types
UENUM(BlueprintType)
enum class EInventoryType : uint8
{
	ShipCargo UMETA(DisplayName = "Ship Cargo"),
	BaseStorage UMETA(DisplayName = "Base Storage"),
	PersonalInventory UMETA(DisplayName = "Personal Inventory"),
	StationStorage UMETA(DisplayName = "Station Storage"),
	Vault UMETA(DisplayName = "Vault")
};

// Container types
UENUM(BlueprintType)
enum class EContainerType : uint8
{
	Crate UMETA(DisplayName = "Crate"),
	Silo UMETA(DisplayName = "Silo"),
	Vault UMETA(DisplayName = "Vault"),
	SecureContainer UMETA(DisplayName = "Secure Container"),
	PortableContainer UMETA(DisplayName = "Portable Container")
};

// Inventory slot
USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()

	// Resource in this slot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FResourceQuantity Resource;

	// Maximum stack size for this slot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 MaxStackSize;

	// Is slot locked? (cannot be modified)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	bool bIsLocked;

	FInventorySlot()
	{
		MaxStackSize = 999;
		bIsLocked = false;
	}
};

// Inventory container
USTRUCT(BlueprintType)
struct FInventoryContainer
{
	GENERATED_BODY()

	// Container ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	FGuid ContainerID;

	// Container type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	EContainerType ContainerType;

	// Container name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	FString ContainerName;

	// Inventory slots
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	TArray<FInventorySlot> Slots;

	// Maximum weight capacity (kg)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	float MaxWeightCapacity;

	// Maximum volume capacity (cubic meters)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	float MaxVolumeCapacity;

	// Current total weight
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	float CurrentWeight;

	// Current total volume
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	float CurrentVolume;

	// Is container locked?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	bool bIsLocked;

	// Owner ID (ship, base, player, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Container")
	FGuid OwnerID;

	FInventoryContainer()
	{
		ContainerID = FGuid::NewGuid();
		ContainerType = EContainerType::Crate;
		MaxWeightCapacity = 1000.0f;
		MaxVolumeCapacity = 10.0f;
		CurrentWeight = 0.0f;
		CurrentVolume = 0.0f;
		bIsLocked = false;
	}
};

// Inventory transfer result
USTRUCT(BlueprintType)
struct FInventoryTransferResult
{
	GENERATED_BODY()

	// Was transfer successful?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer")
	bool bSuccess;

	// Amount actually transferred
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer")
	int32 AmountTransferred;

	// Failure reason
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transfer")
	FString FailureReason;

	FInventoryTransferResult()
	{
		bSuccess = false;
		AmountTransferred = 0;
	}
};

// Inventory search filter
USTRUCT(BlueprintType)
struct FInventoryFilter
{
	GENERATED_BODY()

	// Resource category to filter by
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	EResourceCategory Category;

	// Resource rarity to filter by
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	EResourceRarity Rarity;

	// Minimum quality
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	EResourceQuality MinQuality;

	// Search text
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	FString SearchText;

	// Resource IDs to include (empty = all)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	TArray<FName> IncludeResourceIDs;

	// Resource IDs to exclude
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	TArray<FName> ExcludeResourceIDs;

	FInventoryFilter()
	{
		Category = EResourceCategory::Metallic;
		Rarity = EResourceRarity::Common;
		MinQuality = EResourceQuality::Damaged;
	}
};

// Wrapper structs for complex template containers (UHT compatibility)
USTRUCT(BlueprintType)
struct FResourceQuantityMap
{
	GENERATED_BODY()
	UPROPERTY()
	TMap<FName, FResourceQuantity> Resources;
};

USTRUCT(BlueprintType)
struct FContainerResources
{
	GENERATED_BODY()
	UPROPERTY()
	TMap<FGuid, FResourceQuantityMap> OwnerResources;
};

USTRUCT(BlueprintType)
struct FGuidArray
{
	GENERATED_BODY()
	UPROPERTY()
	TArray<FGuid> Guids;
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceAdded, FName, ResourceID, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnResourceRemoved, FName, ResourceID, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInventoryTransferred, FName, ResourceID, int32, Quantity, EInventoryType, Destination);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContainerAdded, const FInventoryContainer&, Container);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnContainerRemoved, const FInventoryContainer&, Container);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventorySorted, EInventoryType, InventoryType, const FInventoryFilter&, Filter);

/**
 * Inventory Manager - Unified inventory system for managing resources across ships, bases, and personal storage
 */
UCLASS()
class ALEXANDER_API UInventoryManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UInventoryManager();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Tick function
	virtual void Tick(float DeltaTime);
	virtual bool IsTickable() const { return true; }

	// ============================================================================
	// RESOURCE MANAGEMENT
	// ============================================================================

	// Add resource to inventory
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddResource(FName ResourceID, int32 Quantity, EResourceQuality Quality, EInventoryType InventoryType, FGuid OwnerID);

	// Remove resource from inventory
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveResource(FName ResourceID, int32 Quantity, EInventoryType InventoryType, FGuid OwnerID);

	// Get resource quantity
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetResourceQuantity(FName ResourceID, EInventoryType InventoryType, FGuid OwnerID) const;

	// Check if resource is available
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasResource(FName ResourceID, int32 Quantity, EInventoryType InventoryType, FGuid OwnerID) const;

	// Get all resources in inventory
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FResourceQuantity> GetAllResources(EInventoryType InventoryType, FGuid OwnerID) const;

	// Get resources by category
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FResourceQuantity> GetResourcesByCategory(EResourceCategory Category, EInventoryType InventoryType, FGuid OwnerID) const;

	// Get total inventory value
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	float GetTotalInventoryValue(EInventoryType InventoryType, FGuid OwnerID) const;

	// Get total inventory weight
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	float GetTotalInventoryWeight(EInventoryType InventoryType, FGuid OwnerID) const;

	// Get total inventory volume
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	float GetTotalInventoryVolume(EInventoryType InventoryType, FGuid OwnerID) const;

	// ============================================================================
	// INVENTORY TRANSFERS
	// ============================================================================

	// Transfer resources between inventories
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventoryTransferResult TransferResource(FName ResourceID, int32 Quantity, EInventoryType SourceType, FGuid SourceID, EInventoryType DestinationType, FGuid DestinationID);

	// Transfer all resources
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool TransferAllResources(EInventoryType SourceType, FGuid SourceID, EInventoryType DestinationType, FGuid DestinationID);

	// Can transfer? (checks capacity, weight, volume)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool CanTransferResource(FName ResourceID, int32 Quantity, EInventoryType SourceType, FGuid SourceID, EInventoryType DestinationType, FGuid DestinationID) const;

	// ============================================================================
	// CONTAINER MANAGEMENT
	// ============================================================================

	// Create container
	UFUNCTION(BlueprintCallable, Category = "Containers")
	FGuid CreateContainer(EContainerType ContainerType, FString ContainerName, float MaxWeight, float MaxVolume, FGuid OwnerID);

	// Remove container
	UFUNCTION(BlueprintCallable, Category = "Containers")
	bool RemoveContainer(FGuid ContainerID);

	// Get container info
	UFUNCTION(BlueprintCallable, Category = "Containers")
	FInventoryContainer GetContainer(FGuid ContainerID) const;

	// Get all containers for owner
	UFUNCTION(BlueprintCallable, Category = "Containers")
	TArray<FInventoryContainer> GetContainersForOwner(FGuid OwnerID) const;

	// Get containers by type
	UFUNCTION(BlueprintCallable, Category = "Containers")
	TArray<FInventoryContainer> GetContainersByType(EContainerType ContainerType) const;

	// Add resource to container
	UFUNCTION(BlueprintCallable, Category = "Containers")
	bool AddResourceToContainer(FName ResourceID, int32 Quantity, EResourceQuality Quality, FGuid ContainerID);

	// Remove resource from container
	UFUNCTION(BlueprintCallable, Category = "Containers")
	bool RemoveResourceFromContainer(FName ResourceID, int32 Quantity, FGuid ContainerID);

	// Transfer between containers
	UFUNCTION(BlueprintCallable, Category = "Containers")
	FInventoryTransferResult TransferBetweenContainers(FName ResourceID, int32 Quantity, FGuid SourceContainerID, FGuid DestinationContainerID);

	// ============================================================================
	// INVENTORY SEARCH AND FILTERING
	// ============================================================================

	// Find resources matching filter
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FResourceQuantity> FindResources(const FInventoryFilter& Filter, EInventoryType InventoryType, FGuid OwnerID) const;

	// Sort inventory
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SortInventory(EInventoryType InventoryType, FGuid OwnerID, bool bByValue, bool bByWeight, bool bByName);

	// Search resources by name
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<FResourceQuantity> SearchResources(const FString& SearchText, EInventoryType InventoryType, FGuid OwnerID) const;

	// ============================================================================
	// INVENTORY CAPACITY MANAGEMENT
	// ============================================================================

	// Check if inventory has capacity for resource
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasCapacityForResource(FName ResourceID, int32 Quantity, EInventoryType InventoryType, FGuid OwnerID) const;

	// Get remaining weight capacity
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	float GetRemainingWeightCapacity(EInventoryType InventoryType, FGuid OwnerID) const;

	// Get remaining volume capacity
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	float GetRemainingVolumeCapacity(EInventoryType InventoryType, FGuid OwnerID) const;

	// Update inventory capacity
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UpdateInventoryCapacity(EInventoryType InventoryType, FGuid OwnerID, float NewWeightCapacity, float NewVolumeCapacity);

	// ============================================================================
	// UTILITY FUNCTIONS
	// ============================================================================

	// Consolidate inventory stacks (combines partial stacks)
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ConsolidateInventory(EInventoryType InventoryType, FGuid OwnerID);

	// Clear empty slots
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ClearEmptySlots(EInventoryType InventoryType, FGuid OwnerID);

	// Get inventory statistics
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void GetInventoryStats(EInventoryType InventoryType, FGuid OwnerID, int32& OutTotalSlots, int32& OutUsedSlots, int32& OutEmptySlots) const;

	// ============================================================================
	// DEFAULT INVENTORIES
	// ============================================================================

	// Initialize default inventories for player
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeDefaultInventories(FGuid PlayerID);

	// Initialize ship cargo
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeShipCargo(FGuid ShipID, float MaxWeight, float MaxVolume);

	// Initialize base storage
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitializeBaseStorage(FGuid BaseID, float MaxWeight, float MaxVolume);

	// ============================================================================
	// SAVE/LOAD
	// ============================================================================

	// Save inventory data
	UFUNCTION(BlueprintCallable, Category = "Save/Load")
	void SaveInventoryData();

	// Load inventory data
	UFUNCTION(BlueprintCallable, Category = "Save/Load")
	void LoadInventoryData();

	// ============================================================================
	// STATISTICS
	// ============================================================================

	// Get total resources across all inventories
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	TMap<FName, int32> GetTotalResourcesAcrossAllInventories() const;

	// Get total inventory value across all inventories
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	float GetTotalValueAcrossAllInventories() const;

	// Get resource movement statistics
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	int32 GetTotalResourcesAdded() const { return TotalResourcesAdded; }

	UFUNCTION(BlueprintCallable, Category = "Statistics")
	int32 GetTotalResourcesRemoved() const { return TotalResourcesRemoved; }

	UFUNCTION(BlueprintCallable, Category = "Statistics")
	int32 GetTotalTransfers() const { return TotalTransfers; }

	// Reset statistics
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	void ResetStatistics();

	// ============================================================================
	// EVENTS
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Inventory Events")
	FOnResourceAdded OnResourceAdded;

	UPROPERTY(BlueprintAssignable, Category = "Inventory Events")
	FOnResourceRemoved OnResourceRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Inventory Events")
	FOnInventoryTransferred OnInventoryTransferred;

	UPROPERTY(BlueprintAssignable, Category = "Inventory Events")
	FOnContainerAdded OnContainerAdded;

	UPROPERTY(BlueprintAssignable, Category = "Inventory Events")
	FOnContainerRemoved OnContainerRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Inventory Events")
	FOnInventorySorted OnInventorySorted;

protected:
	// Resource gathering system reference
	UPROPERTY()
	TWeakObjectPtr<UResourceGatheringSystem> ResourceGatheringSystem;

	// Main inventory database: InventoryType -> OwnerID -> ResourceID -> Quantity/Quality
	UPROPERTY()
	TMap<EInventoryType, FContainerResources> Inventories;

	// Containers database: ContainerID -> Container
	UPROPERTY()
	TMap<FGuid, FInventoryContainer> Containers;

	// Container ownership: OwnerID -> ContainerIDs
	UPROPERTY()
	TMap<FGuid, FGuidArray> ContainerOwnership;

	// Statistics
	UPROPERTY()
	int32 TotalResourcesAdded;

	UPROPERTY()
	int32 TotalResourcesRemoved;

	UPROPERTY()
	int32 TotalTransfers;

	// Inventory capacity settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TMap<EInventoryType, float> DefaultWeightCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TMap<EInventoryType, float> DefaultVolumeCapacity;

private:
	// Helper functions
	void InitializeDefaultCapacities();
	float CalculateResourceWeight(FName ResourceID, int32 Quantity) const;
	float CalculateResourceVolume(FName ResourceID, int32 Quantity) const;
	FInventoryTransferResult TransferResourceInternal(FName ResourceID, int32 Quantity, EInventoryType SourceType, FGuid SourceID, EInventoryType DestinationType, FGuid DestinationID);
	bool HasCapacityInternal(FName ResourceID, int32 Quantity, EInventoryType InventoryType, FGuid OwnerID) const;
	TArray<FResourceQuantity> GetResourcesInternal(EInventoryType InventoryType, FGuid OwnerID) const;
	void ConsolidateInventoryInternal(TMap<FName, FResourceQuantity>& Inventory);
	void ClearEmptySlotsInternal(TMap<FName, FResourceQuantity>& Inventory);
};