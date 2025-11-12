// Copyright Epic Games, Inc. All Rights Reserved.

#include "InventoryManager.h"
#include "ResourceGatheringSystem.h"
#include "Spaceship.h"
#include "BaseBuildingManager.h"
#include "Engine/World.h"

UInventoryManager::UInventoryManager()
{
	TotalResourcesAdded = 0;
	TotalResourcesRemoved = 0;
	TotalTransfers = 0;

	InitializeDefaultCapacities();
}

void UInventoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("InventoryManager initialized"));

	// Get reference to resource gathering system
	ResourceGatheringSystem = GetWorld()->GetSubsystem<UResourceGatheringSystem>();

	// Reset statistics
	TotalResourcesAdded = 0;
	TotalResourcesRemoved = 0;
	TotalTransfers = 0;

	UE_LOG(LogTemp, Log, TEXT("InventoryManager initialized with default capacities"));
}

void UInventoryManager::Deinitialize()
{
	// Clear all inventory data
	Inventories.Empty();
	Containers.Empty();
	ContainerOwnership.Empty();

	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("InventoryManager deinitialized"));
}

void UInventoryManager::Tick(float DeltaTime)
{
	// Inventory manager doesn't need per-frame updates
	// This is reserved for future features like resource decay, regeneration, etc.
}

bool UInventoryManager::AddResource(FName ResourceID, int32 Quantity, EResourceQuality Quality, EInventoryType InventoryType, FGuid OwnerID)
{
	if (ResourceID.IsNone() || Quantity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot add resource: Invalid parameters"));
		return false;
	}

	// Check capacity
	if (!HasCapacityInternal(ResourceID, Quantity, InventoryType, OwnerID))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot add resource: Insufficient capacity"));
		return false;
	}

	// Get or create inventory for this owner
	FContainerResources& ContainerResources = Inventories.FindOrAdd(InventoryType);
	TMap<FGuid, FResourceQuantityMap>& TypeInventories = ContainerResources.OwnerResources;
	FResourceQuantityMap& OwnerResourceMap = TypeInventories.FindOrAdd(OwnerID);
	TMap<FName, FResourceQuantity>& OwnerInventory = OwnerResourceMap.Resources;

	// Add to existing stack or create new
	FResourceQuantity& ExistingResource = OwnerInventory.FindOrAdd(ResourceID);
	ExistingResource.ResourceID = ResourceID;
	ExistingResource.Quantity += Quantity;
	ExistingResource.Quality = Quality; // Use the provided quality (could average with existing)

	// Update statistics
	TotalResourcesAdded += Quantity;

	// Fire event
	OnResourceAdded.Broadcast(ResourceID, Quantity);

	UE_LOG(LogTemp, Log, TEXT("Added %d %s to inventory %s (Quality: %d)"), 
		   Quantity, *ResourceID.ToString(), *OwnerID.ToString(), (int32)Quality);

	return true;
}

bool UInventoryManager::RemoveResource(FName ResourceID, int32 Quantity, EInventoryType InventoryType, FGuid OwnerID)
{
	if (ResourceID.IsNone() || Quantity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot remove resource: Invalid parameters"));
		return false;
	}

	// Find inventory
	const FContainerResources* ContainerResources = Inventories.Find(InventoryType);
	if (!ContainerResources)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot remove resource: Inventory type not found"));
		return false;
	}
	const TMap<FGuid, FResourceQuantityMap>& TypeInventories = ContainerResources->OwnerResources;

	const FResourceQuantityMap* OwnerResourceMap = TypeInventories.Find(OwnerID);
	if (!OwnerResourceMap)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot remove resource: Owner inventory not found"));
		return false;
	}
	TMap<FName, FResourceQuantity>& OwnerInventory = const_cast<FResourceQuantityMap*>(OwnerResourceMap)->Resources;

	FResourceQuantity* Resource = OwnerInventory.Find(ResourceID);
	if (!Resource || Resource->Quantity < Quantity)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot remove resource: Insufficient quantity"));
		return false;
	}

	// Remove resource
	Resource->Quantity -= Quantity;

	// Remove entry if quantity reaches zero
	if (Resource->Quantity <= 0)
	{
		OwnerInventory.Remove(ResourceID);
	}

	// Update statistics
	TotalResourcesRemoved += Quantity;

	// Fire event
	OnResourceRemoved.Broadcast(ResourceID, Quantity);

	UE_LOG(LogTemp, Log, TEXT("Removed %d %s from inventory %d"), Quantity, *ResourceID.ToString(), OwnerID);

	return true;
}

int32 UInventoryManager::GetResourceQuantity(FName ResourceID, EInventoryType InventoryType, FGuid OwnerID) const
{
	if (ResourceID.IsNone())
	{
		return 0;
	}

	const FContainerResources* ContainerResources = Inventories.Find(InventoryType);
	if (!ContainerResources)
	{
		return 0;
	}

	const FResourceQuantityMap* OwnerResourceMap = ContainerResources->OwnerResources.Find(OwnerID);
	if (!OwnerResourceMap)
	{
		return 0;
	}

	const FResourceQuantity* Resource = OwnerResourceMap->Resources.Find(ResourceID);
	return Resource ? Resource->Quantity : 0;
}

bool UInventoryManager::HasResource(FName ResourceID, int32 Quantity, EInventoryType InventoryType, FGuid OwnerID) const
{
	return GetResourceQuantity(ResourceID, InventoryType, OwnerID) >= Quantity;
}

TArray<FResourceQuantity> UInventoryManager::GetAllResources(EInventoryType InventoryType, FGuid OwnerID) const
{
	TArray<FResourceQuantity> AllResources;

	const FContainerResources* ContainerResources = Inventories.Find(InventoryType);
	if (!ContainerResources)
	{
		return AllResources;
	}

	const FResourceQuantityMap* OwnerResourceMap = ContainerResources->OwnerResources.Find(OwnerID);
	if (!OwnerResourceMap)
	{
		return AllResources;
	}

	for (const auto& Pair : OwnerResourceMap->Resources)
	{
		AllResources.Add(Pair.Value);
	}

	return AllResources;
}

TArray<FResourceQuantity> UInventoryManager::GetResourcesByCategory(EResourceCategory Category, EInventoryType InventoryType, FGuid OwnerID) const
{
	TArray<FResourceQuantity> CategoryResources;

	if (!ResourceGatheringSystem.IsValid())
	{
		return CategoryResources;
	}

	const FContainerResources* ContainerResources = Inventories.Find(InventoryType);
	if (!ContainerResources)
	{
		return CategoryResources;
	}

	const FResourceQuantityMap* OwnerResourceMap = ContainerResources->OwnerResources.Find(OwnerID);
	if (!OwnerResourceMap)
	{
		return CategoryResources;
	}

	for (const auto& Pair : OwnerResourceMap->Resources)
	{
		FResourceDefinition ResourceDef = ResourceGatheringSystem->GetResourceDefinition(Pair.Key);
		if (ResourceDef.Category == Category)
		{
			CategoryResources.Add(Pair.Value);
		}
	}

	return CategoryResources;
}

float UInventoryManager::GetTotalInventoryValue(EInventoryType InventoryType, FGuid OwnerID) const
{
	float TotalValue = 0.0f;

	if (!ResourceGatheringSystem.IsValid())
	{
		return TotalValue;
	}

	const FContainerResources* ContainerResources = Inventories.Find(InventoryType);
	if (!ContainerResources)
	{
		return TotalValue;
	}

	const FResourceQuantityMap* OwnerResourceMap = ContainerResources->OwnerResources.Find(OwnerID);
	if (!OwnerResourceMap)
	{
		return TotalValue;
	}

	for (const auto& Pair : OwnerResourceMap->Resources)
	{
		FResourceDefinition ResourceDef = ResourceGatheringSystem->GetResourceDefinition(Pair.Key);
		TotalValue += ResourceDef.BaseValue * Pair.Value.Quantity;
	}

	return TotalValue;
}

float UInventoryManager::GetTotalInventoryWeight(EInventoryType InventoryType, FGuid OwnerID) const
{
	float TotalWeight = 0.0f;

	const FContainerResources* ContainerResources = Inventories.Find(InventoryType);
	if (!ContainerResources)
	{
		return TotalWeight;
	}

	const FResourceQuantityMap* OwnerResourceMap = ContainerResources->OwnerResources.Find(OwnerID);
	if (!OwnerResourceMap)
	{
		return TotalWeight;
	}

	for (const auto& Pair : OwnerResourceMap->Resources)
	{
		TotalWeight += CalculateResourceWeight(Pair.Key, Pair.Value.Quantity);
	}

	return TotalWeight;
}

float UInventoryManager::GetTotalInventoryVolume(EInventoryType InventoryType, FGuid OwnerID) const
{
	float TotalVolume = 0.0f;

	const FContainerResources* ContainerResources = Inventories.Find(InventoryType);
	if (!ContainerResources)
	{
		return TotalVolume;
	}

	const FResourceQuantityMap* OwnerResourceMap = ContainerResources->OwnerResources.Find(OwnerID);
	if (!OwnerResourceMap)
	{
		return TotalVolume;
	}

	for (const auto& Pair : OwnerResourceMap->Resources)
	{
		TotalVolume += CalculateResourceVolume(Pair.Key, Pair.Value.Quantity);
	}

	return TotalVolume;
}

FInventoryTransferResult UInventoryManager::TransferResource(FName ResourceID, int32 Quantity, EInventoryType SourceType, FGuid SourceID, EInventoryType DestinationType, FGuid DestinationID)
{
	FInventoryTransferResult Result;
	Result.bSuccess = false;

	if (ResourceID.IsNone() || Quantity <= 0)
	{
		Result.FailureReason = "Invalid parameters";
		return Result;
	}

	// Check if source has resource
	if (!HasResource(ResourceID, Quantity, SourceType, SourceID))
	{
		Result.FailureReason = "Insufficient resources in source";
		return Result;
	}

	// Check if destination has capacity
	if (!HasCapacityInternal(ResourceID, Quantity, DestinationType, DestinationID))
	{
		Result.FailureReason = "Insufficient capacity in destination";
		return Result;
	}

	// Perform transfer
	Result = TransferResourceInternal(ResourceID, Quantity, SourceType, SourceID, DestinationType, DestinationID);

	if (Result.bSuccess)
	{
		// Update statistics
		TotalTransfers++;

		// Fire transfer event
		OnInventoryTransferred.Broadcast(ResourceID, Result.AmountTransferred, DestinationType);
	}

	return Result;
}

bool UInventoryManager::TransferAllResources(EInventoryType SourceType, FGuid SourceID, EInventoryType DestinationType, FGuid DestinationID)
{
	TArray<FResourceQuantity> SourceResources = GetAllResources(SourceType, SourceID);
	bool bAllTransferred = true;

	for (const FResourceQuantity& Resource : SourceResources)
	{
		FInventoryTransferResult Result = TransferResource(Resource.ResourceID, Resource.Quantity, SourceType, SourceID, DestinationType, DestinationID);
		if (!Result.bSuccess)
		{
			bAllTransferred = false;
			// Continue trying to transfer other resources
		}
	}

	return bAllTransferred;
}

bool UInventoryManager::CanTransferResource(FName ResourceID, int32 Quantity, EInventoryType SourceType, FGuid SourceID, EInventoryType DestinationType, FGuid DestinationID) const
{
	if (!HasResource(ResourceID, Quantity, SourceType, SourceID))
	{
		return false;
	}

	return HasCapacityInternal(ResourceID, Quantity, DestinationType, DestinationID);
}

FGuid UInventoryManager::CreateContainer(EContainerType ContainerType, FString ContainerName, float MaxWeight, float MaxVolume, FGuid OwnerID)
{
	FInventoryContainer NewContainer;
	NewContainer.ContainerType = ContainerType;
	NewContainer.ContainerName = ContainerName;
	NewContainer.MaxWeightCapacity = MaxWeight;
	NewContainer.MaxVolumeCapacity = MaxVolume;
	NewContainer.OwnerID = OwnerID;

	// Store container
	FGuid ContainerID = NewContainer.ContainerID;
	Containers.Add(ContainerID, NewContainer);

	// Add to ownership map - FIXED: Use FGuidArray.Guids access
	FGuidArray& OwnerContainerArray = ContainerOwnership.FindOrAdd(OwnerID);
	OwnerContainerArray.Guids.Add(ContainerID);

	// Fire event
	OnContainerAdded.Broadcast(NewContainer);

	UE_LOG(LogTemp, Log, TEXT("Created container: %s (ID: %s, Owner: %s)"), *ContainerName, *ContainerID.ToString(), *OwnerID.ToString());

	return ContainerID;
}

bool UInventoryManager::RemoveContainer(FGuid ContainerID)
{
	FInventoryContainer* Container = Containers.Find(ContainerID);
	if (!Container)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot remove container: Container not found"));
		return false;
	}

	// Remove from ownership map - FIXED: Use FGuidArray.Guids access
	FGuidArray* OwnerContainerArray = ContainerOwnership.Find(Container->OwnerID);
	if (OwnerContainerArray)
	{
		OwnerContainerArray->Guids.Remove(ContainerID);
		if (OwnerContainerArray->Guids.Num() == 0)
		{
			ContainerOwnership.Remove(Container->OwnerID);
		}
	}

	// Remove container
	Containers.Remove(ContainerID);

	// Fire event
	OnContainerRemoved.Broadcast(*Container);

	UE_LOG(LogTemp, Log, TEXT("Removed container: %s"), *ContainerID.ToString());

	return true;
}

FInventoryContainer UInventoryManager::GetContainer(FGuid ContainerID) const
{
	const FInventoryContainer* Container = Containers.Find(ContainerID);
	return Container ? *Container : FInventoryContainer();
}

TArray<FInventoryContainer> UInventoryManager::GetContainersForOwner(FGuid OwnerID) const
{
	TArray<FInventoryContainer> OwnerContainers;

	const FGuidArray* ContainerArray = ContainerOwnership.Find(OwnerID);
	if (!ContainerArray)
	{
		return OwnerContainers;
	}

	for (const FGuid& ContainerID : ContainerArray->Guids)
	{
		const FInventoryContainer* Container = Containers.Find(ContainerID);
		if (Container)
		{
			OwnerContainers.Add(*Container);
		}
	}

	return OwnerContainers;
}

// Duplicate functions removed (RemoveContainer, GetContainer, GetContainersForOwner)
// Original definitions kept above

TArray<FInventoryContainer> UInventoryManager::GetContainersByType(EContainerType ContainerType) const
{
	TArray<FInventoryContainer> TypeContainers;

	for (const auto& Pair : Containers)
	{
		if (Pair.Value.ContainerType == ContainerType)
		{
			TypeContainers.Add(Pair.Value);
		}
	}

	return TypeContainers;
}

bool UInventoryManager::AddResourceToContainer(FName ResourceID, int32 Quantity, EResourceQuality Quality, FGuid ContainerID)
{
	if (ResourceID.IsNone() || Quantity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot add resource to container: Invalid parameters"));
		return false;
	}

	FInventoryContainer* Container = Containers.Find(ContainerID);
	if (!Container)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot add resource to container: Container not found"));
		return false;
	}

	// Check container capacity
	float ResourceWeight = CalculateResourceWeight(ResourceID, Quantity);
	float ResourceVolume = CalculateResourceVolume(ResourceID, Quantity);

	if (Container->CurrentWeight + ResourceWeight > Container->MaxWeightCapacity)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot add resource to container: Weight capacity exceeded"));
		return false;
	}

	if (Container->CurrentVolume + ResourceVolume > Container->MaxVolumeCapacity)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot add resource to container: Volume capacity exceeded"));
		return false;
	}

	// Find existing slot or create new
	bool bFoundSlot = false;
	for (FInventorySlot& Slot : Container->Slots)
	{
		if (Slot.Resource.ResourceID == ResourceID && Slot.Resource.Quality == Quality)
		{
			// Add to existing stack
			int32 SpaceInSlot = Slot.MaxStackSize - Slot.Resource.Quantity;
			int32 AmountToAdd = FMath::Min(Quantity, SpaceInSlot);
			
			Slot.Resource.Quantity += AmountToAdd;
			Container->CurrentWeight += CalculateResourceWeight(ResourceID, AmountToAdd);
			Container->CurrentVolume += CalculateResourceVolume(ResourceID, AmountToAdd);
			
			Quantity -= AmountToAdd;
			bFoundSlot = true;

			if (Quantity <= 0)
			{
				break;
			}
		}
	}

	// Add remaining to new slots
	while (Quantity > 0)
	{
		FInventorySlot NewSlot;
		NewSlot.Resource.ResourceID = ResourceID;
		NewSlot.Resource.Quantity = FMath::Min(Quantity, NewSlot.MaxStackSize);
		NewSlot.Resource.Quality = Quality;

		Container->Slots.Add(NewSlot);
		Container->CurrentWeight += CalculateResourceWeight(ResourceID, NewSlot.Resource.Quantity);
		Container->CurrentVolume += CalculateResourceVolume(ResourceID, NewSlot.Resource.Quantity);

		Quantity -= NewSlot.Resource.Quantity;
		bFoundSlot = true;
	}

	if (bFoundSlot)
	{
		TotalResourcesAdded += Quantity;
		OnResourceAdded.Broadcast(ResourceID, Quantity);
	}

	return bFoundSlot;
}

bool UInventoryManager::RemoveResourceFromContainer(FName ResourceID, int32 Quantity, FGuid ContainerID)
{
	if (ResourceID.IsNone() || Quantity <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot remove resource from container: Invalid parameters"));
		return false;
	}

	FInventoryContainer* Container = Containers.Find(ContainerID);
	if (!Container)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot remove resource from container: Container not found"));
		return false;
	}

	// Find and remove from slots
	int32 RemainingToRemove = Quantity;
	for (int32 i = Container->Slots.Num() - 1; i >= 0 && RemainingToRemove > 0; i--)
	{
		FInventorySlot& Slot = Container->Slots[i];
		if (Slot.Resource.ResourceID == ResourceID)
		{
			int32 AmountToRemove = FMath::Min(RemainingToRemove, Slot.Resource.Quantity);
			
			Slot.Resource.Quantity -= AmountToRemove;
			Container->CurrentWeight -= CalculateResourceWeight(ResourceID, AmountToRemove);
			Container->CurrentVolume -= CalculateResourceVolume(ResourceID, AmountToRemove);
			
			RemainingToRemove -= AmountToRemove;

			// Remove empty slot
			if (Slot.Resource.Quantity <= 0)
			{
				Container->Slots.RemoveAt(i);
			}
		}
	}

	if (RemainingToRemove < Quantity)
	{
		int32 ActuallyRemoved = Quantity - RemainingToRemove;
		TotalResourcesRemoved += ActuallyRemoved;
		OnResourceRemoved.Broadcast(ResourceID, ActuallyRemoved);
		return true;
	}

	return false;
}

FInventoryTransferResult UInventoryManager::TransferBetweenContainers(FName ResourceID, int32 Quantity, FGuid SourceContainerID, FGuid DestinationContainerID)
{
	FInventoryTransferResult Result;
	Result.bSuccess = false;

	if (ResourceID.IsNone() || Quantity <= 0)
	{
		Result.FailureReason = "Invalid parameters";
		return Result;
	}

	// Check source container
	FInventoryContainer* SourceContainer = Containers.Find(SourceContainerID);
	if (!SourceContainer)
	{
		Result.FailureReason = "Source container not found";
		return Result;
	}

	// Check destination container
	FInventoryContainer* DestinationContainer = Containers.Find(DestinationContainerID);
	if (!DestinationContainer)
	{
		Result.FailureReason = "Destination container not found";
		return Result;
	}

	// Calculate total quantity available in source
	int32 AvailableQuantity = 0;
	for (const FInventorySlot& Slot : SourceContainer->Slots)
	{
		if (Slot.Resource.ResourceID == ResourceID)
		{
			AvailableQuantity += Slot.Resource.Quantity;
		}
	}

	if (AvailableQuantity < Quantity)
	{
		Result.FailureReason = "Insufficient quantity in source container";
		return Result;
	}

	// Check destination capacity
	float ResourceWeight = CalculateResourceWeight(ResourceID, Quantity);
	float ResourceVolume = CalculateResourceVolume(ResourceID, Quantity);

	if (DestinationContainer->CurrentWeight + ResourceWeight > DestinationContainer->MaxWeightCapacity)
	{
		Result.FailureReason = "Destination container weight capacity exceeded";
		return Result;
	}

	if (DestinationContainer->CurrentVolume + ResourceVolume > DestinationContainer->MaxVolumeCapacity)
	{
		Result.FailureReason = "Destination container volume capacity exceeded";
		return Result;
	}

	// Perform transfer
	int32 RemainingToTransfer = Quantity;

	// Remove from source
	for (int32 i = SourceContainer->Slots.Num() - 1; i >= 0 && RemainingToTransfer > 0; i--)
	{
		FInventorySlot& Slot = SourceContainer->Slots[i];
		if (Slot.Resource.ResourceID == ResourceID)
		{
			int32 AmountToRemove = FMath::Min(RemainingToTransfer, Slot.Resource.Quantity);
			
			// Add to destination (preserving quality)
			AddResourceToContainer(ResourceID, AmountToRemove, Slot.Resource.Quality, DestinationContainerID);
			
			// Remove from source
			Slot.Resource.Quantity -= AmountToRemove;
			SourceContainer->CurrentWeight -= CalculateResourceWeight(ResourceID, AmountToRemove);
			SourceContainer->CurrentVolume -= CalculateResourceVolume(ResourceID, AmountToRemove);
			
			RemainingToTransfer -= AmountToRemove;

			// Remove empty slot
			if (Slot.Resource.Quantity <= 0)
			{
				SourceContainer->Slots.RemoveAt(i);
			}
		}
	}

	Result.bSuccess = true;
	Result.AmountTransferred = Quantity - RemainingToTransfer;

	if (Result.AmountTransferred > 0)
	{
		TotalTransfers++;
		OnInventoryTransferred.Broadcast(ResourceID, Result.AmountTransferred, EInventoryType::StationStorage);
	}

	return Result;
}

TArray<FResourceQuantity> UInventoryManager::FindResources(const FInventoryFilter& Filter, EInventoryType InventoryType, FGuid OwnerID) const
{
	TArray<FResourceQuantity> FoundResources;

	if (!ResourceGatheringSystem.IsValid())
	{
		return FoundResources;
	}

	const FContainerResources* ContainerResources = Inventories.Find(InventoryType);
	if (!ContainerResources)
	{
		return FoundResources;
	}

	const FResourceQuantityMap* OwnerResourceMap = ContainerResources->OwnerResources.Find(OwnerID);
	if (!OwnerResourceMap)
	{
		return FoundResources;
	}

	for (const auto& Pair : OwnerResourceMap->Resources)
	{
		FResourceDefinition ResourceDef = ResourceGatheringSystem->GetResourceDefinition(Pair.Key);
		
		// Apply filters
		bool bPassesFilter = true;

		if (Filter.Category != EResourceCategory::Metallic && ResourceDef.Category != Filter.Category)
		{
			bPassesFilter = false;
		}

		if (Filter.Rarity != EResourceRarity::Common && ResourceDef.Rarity != Filter.Rarity)
		{
			bPassesFilter = false;
		}

		if (Pair.Value.Quality < Filter.MinQuality)
		{
			bPassesFilter = false;
		}

		if (!Filter.SearchText.IsEmpty() && !ResourceDef.DisplayName.Contains(Filter.SearchText))
		{
			bPassesFilter = false;
		}

		if (Filter.IncludeResourceIDs.Num() > 0 && !Filter.IncludeResourceIDs.Contains(Pair.Key))
		{
			bPassesFilter = false;
		}

		if (Filter.ExcludeResourceIDs.Contains(Pair.Key))
		{
			bPassesFilter = false;
		}

		if (bPassesFilter)
		{
			FoundResources.Add(Pair.Value);
		}
	}

	return FoundResources;
}

void UInventoryManager::SortInventory(EInventoryType InventoryType, FGuid OwnerID, bool bByValue, bool bByWeight, bool bByName)
{
	FContainerResources* ContainerResources = Inventories.Find(InventoryType);
	if (!ContainerResources)
	{
		return;
	}

	FResourceQuantityMap* OwnerResourceMap = ContainerResources->OwnerResources.Find(OwnerID);
	if (!OwnerResourceMap)
	{
		return;
	}

	// Convert to array for sorting
	TArray<FResourceQuantity> Resources;
	for (const auto& Pair : OwnerResourceMap->Resources)
	{
		Resources.Add(Pair.Value);
	}

	// Sort based on parameters
	if (bByValue && ResourceGatheringSystem.IsValid())
	{
		Resources.Sort([this](const FResourceQuantity& A, const FResourceQuantity& B) {
			FResourceDefinition DefA = ResourceGatheringSystem->GetResourceDefinition(A.ResourceID);
			FResourceDefinition DefB = ResourceGatheringSystem->GetResourceDefinition(B.ResourceID);
			return (DefA.BaseValue * A.Quantity) > (DefB.BaseValue * B.Quantity);
		});
	}
	else if (bByWeight)
	{
		Resources.Sort([this](const FResourceQuantity& A, const FResourceQuantity& B) {
			float WeightA = CalculateResourceWeight(A.ResourceID, A.Quantity);
			float WeightB = CalculateResourceWeight(B.ResourceID, B.Quantity);
			return WeightA > WeightB;
		});
	}
	else if (bByName)
	{
		Resources.Sort([](const FResourceQuantity& A, const FResourceQuantity& B) {
			return A.ResourceID.ToString() < B.ResourceID.ToString();
		});
	}

	// Clear and rebuild inventory
	OwnerResourceMap->Resources.Empty();
	for (const FResourceQuantity& Resource : Resources)
	{
		OwnerResourceMap->Resources.Add(Resource.ResourceID, Resource);
	}

	// Fire sort event
	FInventoryFilter Filter;
	OnInventorySorted.Broadcast(InventoryType, Filter);

	UE_LOG(LogTemp, Log, TEXT("Sorted inventory for owner %s"), *OwnerID.ToString());
}

TArray<FResourceQuantity> UInventoryManager::SearchResources(const FString& SearchText, EInventoryType InventoryType, FGuid OwnerID) const
{
	TArray<FResourceQuantity> FoundResources;

	if (!ResourceGatheringSystem.IsValid() || SearchText.IsEmpty())
	{
		return FoundResources;
	}

	const FContainerResources* ContainerResources = Inventories.Find(InventoryType);
	if (!ContainerResources)
	{
		return FoundResources;
	}

	const FResourceQuantityMap* OwnerResourceMap = ContainerResources->OwnerResources.Find(OwnerID);
	if (!OwnerResourceMap)
	{
		return FoundResources;
	}

	FString LowerSearchText = SearchText.ToLower();

	for (const auto& Pair : OwnerResourceMap->Resources)
	{
		FResourceDefinition ResourceDef = ResourceGatheringSystem->GetResourceDefinition(Pair.Key);
		FString LowerDisplayName = ResourceDef.DisplayName.ToLower();

		if (LowerDisplayName.Contains(LowerSearchText) || Pair.Key.ToString().ToLower().Contains(LowerSearchText))
		{
			FoundResources.Add(Pair.Value);
		}
	}

	return FoundResources;
}

bool UInventoryManager::HasCapacityForResource(FName ResourceID, int32 Quantity, EInventoryType InventoryType, FGuid OwnerID) const
{
	return HasCapacityInternal(ResourceID, Quantity, InventoryType, OwnerID);
}

float UInventoryManager::GetRemainingWeightCapacity(EInventoryType InventoryType, FGuid OwnerID) const
{
	float CurrentWeight = GetTotalInventoryWeight(InventoryType, OwnerID);
	float MaxWeight = DefaultWeightCapacity.FindRef(InventoryType);
	return FMath::Max(0.0f, MaxWeight - CurrentWeight);
}

float UInventoryManager::GetRemainingVolumeCapacity(EInventoryType InventoryType, FGuid OwnerID) const
{
	float CurrentVolume = GetTotalInventoryVolume(InventoryType, OwnerID);
	float MaxVolume = DefaultVolumeCapacity.FindRef(InventoryType);
	return FMath::Max(0.0f, MaxVolume - CurrentVolume);
}

void UInventoryManager::UpdateInventoryCapacity(EInventoryType InventoryType, FGuid OwnerID, float NewWeightCapacity, float NewVolumeCapacity)
{
	// Update default capacities (these apply to all owners of this type)
	DefaultWeightCapacity.Add(InventoryType, NewWeightCapacity);
	DefaultVolumeCapacity.Add(InventoryType, NewVolumeCapacity);

	UE_LOG(LogTemp, Log, TEXT("Updated inventory capacities for type %d: Weight=%.1f, Volume=%.1f"), 
		   (int32)InventoryType, NewWeightCapacity, NewVolumeCapacity);
}

void UInventoryManager::ConsolidateInventory(EInventoryType InventoryType, FGuid OwnerID)
{
	FContainerResources* ContainerResources = Inventories.Find(InventoryType);
	if (!ContainerResources)
	{
		return;
	}

	FResourceQuantityMap* OwnerResourceMap = ContainerResources->OwnerResources.Find(OwnerID);
	if (!OwnerResourceMap)
	{
		return;
	}

	ConsolidateInventoryInternal(OwnerResourceMap->Resources);

	UE_LOG(LogTemp, Log, TEXT("Consolidated inventory for owner %s"), *OwnerID.ToString());
}

void UInventoryManager::ClearEmptySlots(EInventoryType InventoryType, FGuid OwnerID)
{
	FContainerResources* ContainerResources = Inventories.Find(InventoryType);
	if (!ContainerResources)
	{
		return;
	}

	FResourceQuantityMap* OwnerResourceMap = ContainerResources->OwnerResources.Find(OwnerID);
	if (!OwnerResourceMap)
	{
		return;
	}

	ClearEmptySlotsInternal(OwnerResourceMap->Resources);

	UE_LOG(LogTemp, Log, TEXT("Cleared empty slots for owner %s"), *OwnerID.ToString());
}

void UInventoryManager::GetInventoryStats(EInventoryType InventoryType, FGuid OwnerID, int32& OutTotalSlots, int32& OutUsedSlots, int32& OutEmptySlots) const
{
	OutTotalSlots = 0;
	OutUsedSlots = 0;
	OutEmptySlots = 0;

	const FContainerResources* ContainerResources = Inventories.Find(InventoryType);
	if (!ContainerResources)
	{
		return;
	}

	const FResourceQuantityMap* OwnerResourceMap = ContainerResources->OwnerResources.Find(OwnerID);
	if (!OwnerResourceMap)
	{
		return;
	}

	OutUsedSlots = OwnerResourceMap->Resources.Num();
	OutTotalSlots = OutUsedSlots; // In this implementation, we don't have a fixed slot count
	OutEmptySlots = 0; // Not applicable in this implementation
}

void UInventoryManager::InitializeDefaultInventories(FGuid PlayerID)
{
	// Initialize personal inventory
	// Default capacities are already set

	UE_LOG(LogTemp, Log, TEXT("Initialized default inventories for player %s"), *PlayerID.ToString());
}

void UInventoryManager::InitializeShipCargo(FGuid ShipID, float MaxWeight, float MaxVolume)
{
	// Create a container for ship cargo
	FGuid ContainerID = CreateContainer(EContainerType::PortableContainer, FString::Printf(TEXT("ShipCargo_%s"), *ShipID.ToString()), MaxWeight, MaxVolume, ShipID);

	UE_LOG(LogTemp, Log, TEXT("Initialized ship cargo for ship %s (Container: %s)"), *ShipID.ToString(), *ContainerID.ToString());
}

void UInventoryManager::InitializeBaseStorage(FGuid BaseID, float MaxWeight, float MaxVolume)
{
	// Create a container for base storage
	FGuid ContainerID = CreateContainer(EContainerType::Silo, FString::Printf(TEXT("BaseStorage_%s"), *BaseID.ToString()), MaxWeight, MaxVolume, BaseID);

	UE_LOG(LogTemp, Log, TEXT("Initialized base storage for base %s (Container: %s)"), *BaseID.ToString(), *ContainerID.ToString());
}

void UInventoryManager::SaveInventoryData()
{
	// Placeholder for save functionality
	UE_LOG(LogTemp, Log, TEXT("SaveInventoryData not yet implemented"));
}

void UInventoryManager::LoadInventoryData()
{
	// Placeholder for load functionality
	UE_LOG(LogTemp, Log, TEXT("LoadInventoryData not yet implemented"));
}

TMap<FName, int32> UInventoryManager::GetTotalResourcesAcrossAllInventories() const
{
	TMap<FName, int32> TotalResources;

	for (const auto& TypePair : Inventories)
	{
		for (const auto& OwnerPair : TypePair.Value.OwnerResources)
		{
			for (const auto& ResourcePair : OwnerPair.Value.Resources)
			{
				int32& TotalAmount = TotalResources.FindOrAdd(ResourcePair.Key);
				TotalAmount += ResourcePair.Value.Quantity;
			}
		}
	}

	return TotalResources;
}

float UInventoryManager::GetTotalValueAcrossAllInventories() const
{
	float TotalValue = 0.0f;

	if (!ResourceGatheringSystem.IsValid())
	{
		return TotalValue;
	}

	for (const auto& TypePair : Inventories)
	{
		for (const auto& OwnerPair : TypePair.Value.OwnerResources)
		{
			for (const auto& ResourcePair : OwnerPair.Value.Resources)
			{
				FResourceDefinition ResourceDef = ResourceGatheringSystem->GetResourceDefinition(ResourcePair.Key);
				TotalValue += ResourceDef.BaseValue * ResourcePair.Value.Quantity;
			}
		}
	}

	return TotalValue;
}

void UInventoryManager::ResetStatistics()
{
	TotalResourcesAdded = 0;
	TotalResourcesRemoved = 0;
	TotalTransfers = 0;
	UE_LOG(LogTemp, Log, TEXT("Inventory statistics reset"));
}

void UInventoryManager::InitializeDefaultCapacities()
{
	// Set default capacities for each inventory type
	DefaultWeightCapacity.Add(EInventoryType::ShipCargo, 5000.0f);
	DefaultWeightCapacity.Add(EInventoryType::BaseStorage, 10000.0f);
	DefaultWeightCapacity.Add(EInventoryType::PersonalInventory, 100.0f);
	DefaultWeightCapacity.Add(EInventoryType::StationStorage, 20000.0f);
	DefaultWeightCapacity.Add(EInventoryType::Vault, 50000.0f);

	DefaultVolumeCapacity.Add(EInventoryType::ShipCargo, 50.0f);
	DefaultVolumeCapacity.Add(EInventoryType::BaseStorage, 100.0f);
	DefaultVolumeCapacity.Add(EInventoryType::PersonalInventory, 5.0f);
	DefaultVolumeCapacity.Add(EInventoryType::StationStorage, 200.0f);
	DefaultVolumeCapacity.Add(EInventoryType::Vault, 500.0f);
}

float UInventoryManager::CalculateResourceWeight(FName ResourceID, int32 Quantity) const
{
	if (!ResourceGatheringSystem.IsValid())
	{
		return Quantity * 1.0f; // Default weight
	}

	FResourceDefinition ResourceDef = ResourceGatheringSystem->GetResourceDefinition(ResourceID);
	return ResourceDef.WeightPerUnit * Quantity;
}

float UInventoryManager::CalculateResourceVolume(FName ResourceID, int32 Quantity) const
{
	if (!ResourceGatheringSystem.IsValid())
	{
		return Quantity * 0.001f; // Default volume
	}

	FResourceDefinition ResourceDef = ResourceGatheringSystem->GetResourceDefinition(ResourceID);
	return ResourceDef.VolumePerUnit * Quantity;
}

FInventoryTransferResult UInventoryManager::TransferResourceInternal(FName ResourceID, int32 Quantity, EInventoryType SourceType, FGuid SourceID, EInventoryType DestinationType, FGuid DestinationID)
{
	FInventoryTransferResult Result;
	Result.bSuccess = false;

	// Remove from source
	if (!RemoveResource(ResourceID, Quantity, SourceType, SourceID))
	{
		Result.FailureReason = "Failed to remove from source";
		return Result;
	}

	// Add to destination
	if (!AddResource(ResourceID, Quantity, EResourceQuality::Normal, DestinationType, DestinationID))
	{
		// If add fails, return to source
		AddResource(ResourceID, Quantity, EResourceQuality::Normal, SourceType, SourceID);
		Result.FailureReason = "Failed to add to destination";
		return Result;
	}

	Result.bSuccess = true;
	Result.AmountTransferred = Quantity;

	return Result;
}

bool UInventoryManager::HasCapacityInternal(FName ResourceID, int32 Quantity, EInventoryType InventoryType, FGuid OwnerID) const
{
	float ResourceWeight = CalculateResourceWeight(ResourceID, Quantity);
	float ResourceVolume = CalculateResourceVolume(ResourceID, Quantity);

	float CurrentWeight = GetTotalInventoryWeight(InventoryType, OwnerID);
	float CurrentVolume = GetTotalInventoryVolume(InventoryType, OwnerID);

	float MaxWeight = DefaultWeightCapacity.FindRef(InventoryType);
	float MaxVolume = DefaultVolumeCapacity.FindRef(InventoryType);

	return (CurrentWeight + ResourceWeight <= MaxWeight) && (CurrentVolume + ResourceVolume <= MaxVolume);
}

TArray<FResourceQuantity> UInventoryManager::GetResourcesInternal(EInventoryType InventoryType, FGuid OwnerID) const
{
	TArray<FResourceQuantity> Resources;

	const FContainerResources* ContainerResources = Inventories.Find(InventoryType);
	if (!ContainerResources)
	{
		return Resources;
	}

	const FResourceQuantityMap* OwnerResourceMap = ContainerResources->OwnerResources.Find(OwnerID);
	if (!OwnerResourceMap)
	{
		return Resources;
	}

	for (const auto& Pair : OwnerResourceMap->Resources)
	{
		Resources.Add(Pair.Value);
	}

	return Resources;
}

void UInventoryManager::ConsolidateInventoryInternal(TMap<FName, FResourceQuantity>& Inventory)
{
	TMap<FName, FResourceQuantity> Consolidated;

	for (const auto& Pair : Inventory)
	{
		FResourceQuantity& Existing = Consolidated.FindOrAdd(Pair.Key);
		Existing.ResourceID = Pair.Value.ResourceID;
		Existing.Quantity += Pair.Value.Quantity;
		// Quality could be averaged here if needed
	}

	Inventory = Consolidated;
}

void UInventoryManager::ClearEmptySlotsInternal(TMap<FName, FResourceQuantity>& Inventory)
{
	TArray<FName> KeysToRemove;
	for (const auto& Pair : Inventory)
	{
		if (Pair.Value.Quantity <= 0)
		{
			KeysToRemove.Add(Pair.Key);
		}
	}

	for (const FName& Key : KeysToRemove)
	{
		Inventory.Remove(Key);
	}
}