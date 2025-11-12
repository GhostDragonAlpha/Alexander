// Copyright Epic Games, Inc. All Rights Reserved.

#include "CraftingSystem.h"
#include "BaseBuildingManager.h"
#include "ResourceGatheringSystem.h"
#include "InventoryManager.h"
#include "SpaceStationHub.h"
#include "Engine/World.h"

UCraftingSystem::UCraftingSystem()
{
	TotalCraftingOperations = 0;
	TotalEnergyConsumed = 0.0f;
	CraftingSkillLevel = 0.0f;
}

void UCraftingSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("CraftingSystem initialized"));

	// Get references to other systems
	ResourceGatheringSystem = GetWorld()->GetSubsystem<UResourceGatheringSystem>();
	// InventoryManager will be set later when available

	// Initialize default blueprints
	InitializeDefaultBlueprints();

	// Reset statistics
	TotalCraftingOperations = 0;
	TotalItemsCrafted.Empty();
	TotalEnergyConsumed = 0.0f;
}

void UCraftingSystem::Deinitialize()
{
	// Clear all data
	Blueprints.Empty();
	CraftingStations.Empty();
	ActiveOperations.Empty();

	Super::Deinitialize();

	UE_LOG(LogTemp, Log, TEXT("CraftingSystem deinitialized"));
}

void UCraftingSystem::Tick(float DeltaTime)
{
	// Update active crafting operations
	for (int32 i = ActiveOperations.Num() - 1; i >= 0; i--)
	{
		FActiveCraftingOperation& Operation = ActiveOperations[i];

		if (!Operation.bIsActive || !Operation.CraftingStation.IsValid())
		{
			continue;
		}

		float CurrentTime = GetWorld()->GetTimeSeconds();
		float ElapsedTime = CurrentTime - Operation.StartTime;

		// Update progress
		float StationSpeed = 1.0f;
		const FCraftingStationInfo* StationInfo = CraftingStations.Find(Operation.CraftingStation.Get());
		if (StationInfo)
		{
			StationSpeed = StationInfo->CraftingSpeedMultiplier;
		}

		float TotalDuration = CalculateCraftingDuration(Operation.Blueprint, Operation.CraftingStation.Get());
		Operation.Progress = FMath::Clamp(ElapsedTime / TotalDuration, 0.0f, 1.0f);

		// Check if operation is complete
		if (Operation.Progress >= 1.0f)
		{
			// Complete crafting operation
			FCraftingResult Result = CompleteCraftingOperation(Operation);

			// Fire completion event
			OnCraftingCompleted.Broadcast(Operation.Blueprint, Result);

			// Remove from active operations
			ActiveOperations.RemoveAt(i);
		}
	}
}

void UCraftingSystem::RegisterBlueprint(const FCraftingBlueprint& Blueprint)
{
	if (Blueprint.BlueprintID.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot register blueprint with empty ID"));
		return;
	}

	Blueprints.Add(Blueprint.BlueprintID, Blueprint);
	UE_LOG(LogTemp, Log, TEXT("Registered crafting blueprint: %s (%s)"), *Blueprint.BlueprintID.ToString(), *Blueprint.DisplayName);
}

FCraftingBlueprint UCraftingSystem::GetBlueprint(FName BlueprintID) const
{
	const FCraftingBlueprint* Blueprint = Blueprints.Find(BlueprintID);
	return Blueprint ? *Blueprint : FCraftingBlueprint();
}

TArray<FCraftingBlueprint> UCraftingSystem::GetAllBlueprints() const
{
	TArray<FCraftingBlueprint> AllBlueprints;
	Blueprints.GenerateValueArray(AllBlueprints);
	return AllBlueprints;
}

TArray<FCraftingBlueprint> UCraftingSystem::GetBlueprintsByTier(ECraftingTier Tier) const
{
	TArray<FCraftingBlueprint> TierBlueprints;
	for (const auto& Pair : Blueprints)
	{
		if (Pair.Value.CraftingTier == Tier)
		{
			TierBlueprints.Add(Pair.Value);
		}
	}
	return TierBlueprints;
}

TArray<FCraftingBlueprint> UCraftingSystem::GetBlueprintsByCategory(ECraftedItemCategory Category) const
{
	TArray<FCraftingBlueprint> CategoryBlueprints;
	for (const auto& Pair : Blueprints)
	{
		if (Pair.Value.ItemCategory == Category)
		{
			CategoryBlueprints.Add(Pair.Value);
		}
	}
	return CategoryBlueprints;
}

TArray<FCraftingBlueprint> UCraftingSystem::GetUnlockedBlueprints() const
{
	TArray<FCraftingBlueprint> UnlockedBlueprints;
	for (const auto& Pair : Blueprints)
	{
		if (Pair.Value.bIsUnlocked)
		{
			UnlockedBlueprints.Add(Pair.Value);
		}
	}
	return UnlockedBlueprints;
}

bool UCraftingSystem::UnlockBlueprint(FName BlueprintID)
{
	FCraftingBlueprint* Blueprint = Blueprints.Find(BlueprintID);
	if (!Blueprint)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot unlock blueprint: Blueprint not found"));
		return false;
	}

	if (Blueprint->bIsUnlocked)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot unlock blueprint: Already unlocked"));
		return false;
	}

	Blueprint->bIsUnlocked = true;

	// Fire unlock event
	OnBlueprintUnlocked.Broadcast(*Blueprint);

	UE_LOG(LogTemp, Log, TEXT("Unlocked crafting blueprint: %s"), *BlueprintID.ToString());

	return true;
}

bool UCraftingSystem::PurchaseBlueprint(FName BlueprintID, int32 PlayerCredits)
{
	FCraftingBlueprint* Blueprint = Blueprints.Find(BlueprintID);
	if (!Blueprint)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot purchase blueprint: Blueprint not found"));
		return false;
	}

	if (Blueprint->bIsUnlocked)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot purchase blueprint: Already unlocked"));
		return false;
	}

	if (Blueprint->UnlockCost > PlayerCredits)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot purchase blueprint: Insufficient credits"));
		return false;
	}

	// Unlock the blueprint
	Blueprint->bIsUnlocked = true;

	// Fire unlock event
	OnBlueprintUnlocked.Broadcast(*Blueprint);

	UE_LOG(LogTemp, Log, TEXT("Purchased crafting blueprint: %s for %d credits"), *BlueprintID.ToString(), Blueprint->UnlockCost);

	return true;
}

bool UCraftingSystem::StartCrafting(FName BlueprintID, ASpaceStationHub* Station)
{
	if (!Station)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start crafting: Invalid station"));
		return false;
	}

	if (!ResourceGatheringSystem.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start crafting: Resource gathering system not available"));
		return false;
	}

	// Get blueprint
	FCraftingBlueprint Blueprint = GetBlueprint(BlueprintID);
	if (!Blueprint.bIsUnlocked)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start crafting: Blueprint not unlocked"));
		return false;
	}

	// Check if station can craft this blueprint
	if (!CanCraftBlueprint(Blueprint, Station))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start crafting: Station cannot craft this blueprint"));
		return false;
	}

	// Check if station is already crafting
	if (IsStationCrafting(Station))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start crafting: Station already crafting"));
		return false;
	}

	// Check if we have required resources
	if (!HasRequiredResources(Blueprint))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot start crafting: Insufficient resources"));
		return false;
	}

	// Consume input resources
	ConsumeResources(Blueprint);

	// Create active operation
	FActiveCraftingOperation Operation;
	Operation.Blueprint = Blueprint;
	Operation.CraftingStation = Station;
	Operation.StartTime = GetWorld()->GetTimeSeconds();
	Operation.bIsActive = true;
	Operation.Progress = 0.0f;
	Operation.QualityMultiplier = 1.0f + (CraftingSkillLevel / 100.0f) * 0.5f; // Up to 50% quality bonus from skill

	ActiveOperations.Add(Operation);

	// Fire started event
	OnCraftingStarted.Broadcast(Blueprint, Station);

	UE_LOG(LogTemp, Log, TEXT("Started crafting operation: %s at station %s"), 
		   *BlueprintID.ToString(), *Station->GetName());

	return true;
}

void UCraftingSystem::CancelCrafting(ASpaceStationHub* Station)
{
	if (!Station)
	{
		return;
	}

	// Find and remove operation for this station
	for (int32 i = ActiveOperations.Num() - 1; i >= 0; i--)
	{
		if (ActiveOperations[i].CraftingStation.Get() == Station)
		{
			// Return consumed resources (partial refund)
			// In a full implementation, this would return a portion of the input resources
			
			ActiveOperations.RemoveAt(i);
			UE_LOG(LogTemp, Log, TEXT("Cancelled crafting operation for station: %s"), *Station->GetName());
			break;
		}
	}
}

bool UCraftingSystem::IsStationCrafting(ASpaceStationHub* Station) const
{
	if (!Station)
	{
		return false;
	}

	for (const FActiveCraftingOperation& Operation : ActiveOperations)
	{
		if (Operation.CraftingStation.Get() == Station && Operation.bIsActive)
		{
			return true;
		}
	}

	return false;
}

float UCraftingSystem::GetCraftingProgress(ASpaceStationHub* Station) const
{
	if (!Station)
	{
		return 0.0f;
	}

	for (const FActiveCraftingOperation& Operation : ActiveOperations)
	{
		if (Operation.CraftingStation.Get() == Station)
		{
			return Operation.Progress;
		}
	}

	return 0.0f;
}

void UCraftingSystem::RegisterCraftingStation(ASpaceStationHub* Station, ECraftingStationType StationType, int32 Tier)
{
	if (!Station)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot register crafting station: Invalid station"));
		return;
	}

	FCraftingStationInfo StationInfo;
	StationInfo.Station = Station;
	StationInfo.StationType = StationType;
	StationInfo.StationTier = Tier;
	StationInfo.bIsOperational = true;
	StationInfo.CraftingSpeedMultiplier = 1.0f + (Tier - 1) * 0.2f; // 20% faster per tier
	StationInfo.EnergyEfficiencyMultiplier = 1.0f - (Tier - 1) * 0.1f; // 10% more efficient per tier

	CraftingStations.Add(Station, StationInfo);

	// Fire station added event
	OnCraftingStationAdded.Broadcast(Station, StationType);

	UE_LOG(LogTemp, Log, TEXT("Registered crafting station: %s (Type: %s, Tier: %d)"), 
		   *Station->GetName(), *UEnum::GetDisplayValueAsText(StationType).ToString(), Tier);
}

void UCraftingSystem::UnregisterCraftingStation(ASpaceStationHub* Station)
{
	if (!Station)
	{
		return;
	}

	// Cancel any active operation for this station
	CancelCrafting(Station);

	// Remove from registry
	CraftingStations.Remove(Station);

	// Fire station removed event
	OnCraftingStationRemoved.Broadcast(Station);

	UE_LOG(LogTemp, Log, TEXT("Unregistered crafting station: %s"), *Station->GetName());
}

FCraftingStationInfo UCraftingSystem::GetCraftingStationInfo(ASpaceStationHub* Station) const
{
	const FCraftingStationInfo* StationInfo = CraftingStations.Find(Station);
	return StationInfo ? *StationInfo : FCraftingStationInfo();
}

TArray<FCraftingStationInfo> UCraftingSystem::GetAllCraftingStations() const
{
	TArray<FCraftingStationInfo> AllStations;
	CraftingStations.GenerateValueArray(AllStations);
	return AllStations;
}

TArray<FCraftingStationInfo> UCraftingSystem::GetStationsByType(ECraftingStationType StationType) const
{
	TArray<FCraftingStationInfo> StationsByType;
	for (const auto& Pair : CraftingStations)
	{
		if (Pair.Value.StationType == StationType)
		{
			StationsByType.Add(Pair.Value);
		}
	}
	return StationsByType;
}

void UCraftingSystem::SetStationOperational(ASpaceStationHub* Station, bool bIsOperational)
{
	if (!Station)
	{
		return;
	}

	FCraftingStationInfo* StationInfo = CraftingStations.Find(Station);
	if (StationInfo)
	{
		StationInfo->bIsOperational = bIsOperational;

		// Cancel operation if station becomes non-operational
		if (!bIsOperational)
		{
			CancelCrafting(Station);
		}

		UE_LOG(LogTemp, Log, TEXT("Crafting station %s operational status: %s"), 
			   *Station->GetName(), bIsOperational ? TEXT("true") : TEXT("false"));
	}
}

void UCraftingSystem::IncreaseCraftingSkill(float Amount)
{
	float OldLevel = CraftingSkillLevel;
	CraftingSkillLevel = FMath::Clamp(CraftingSkillLevel + Amount, 0.0f, 100.0f);

	// Fire skill increased event if level changed
	if (CraftingSkillLevel > OldLevel)
	{
		OnCraftingSkillIncreased.Broadcast();
	}

	UE_LOG(LogTemp, Log, TEXT("Crafting skill increased: %.1f -> %.1f"), OldLevel, CraftingSkillLevel);
}

float UCraftingSystem::GetSkillBonus() const
{
	// Skill provides up to 25% bonus to various crafting aspects
	return (CraftingSkillLevel / 100.0f) * 0.25f;
}

void UCraftingSystem::InitializeDefaultBlueprints()
{
	// Initialize all blueprint categories
	InitializeShipComponentBlueprints();
	InitializeBaseModuleBlueprints();
	InitializeEquipmentBlueprints();
	InitializeConsumableBlueprints();
	InitializeAdvancedBlueprints();

	UE_LOG(LogTemp, Log, TEXT("Initialized default crafting blueprints"));
}

void UCraftingSystem::LoadBlueprintsFromDataAssets()
{
	// Placeholder for loading from data assets
	UE_LOG(LogTemp, Log, TEXT("LoadBlueprintsFromDataAssets not yet implemented"));
}

void UCraftingSystem::ResetStatistics()
{
	TotalCraftingOperations = 0;
	TotalItemsCrafted.Empty();
	TotalEnergyConsumed = 0.0f;
	UE_LOG(LogTemp, Log, TEXT("Crafting statistics reset"));
}

FCraftingResult UCraftingSystem::CompleteCraftingOperation(FActiveCraftingOperation& Operation)
{
	FCraftingResult Result;
	Result.bSuccess = false;

	if (!Operation.CraftingStation.IsValid())
	{
		Result.FailureReason = "Invalid station";
		return Result;
	}

	// Perform crafting
	Result = CraftItem(Operation.Blueprint, Operation.CraftingStation.Get(), Operation.QualityMultiplier);

	return Result;
}

FCraftingResult UCraftingSystem::CraftItem(const FCraftingBlueprint& Blueprint, ASpaceStationHub* Station, float QualityMultiplier)
{
	FCraftingResult Result;
	Result.bSuccess = false;

	if (!Station)
	{
		Result.FailureReason = "Invalid station";
		return Result;
	}

	// Check if we can craft this blueprint
	if (!CanCraftBlueprint(Blueprint, Station))
	{
		Result.FailureReason = "Cannot craft blueprint with this station";
		return Result;
	}

	// Calculate crafting parameters
	float CraftingTime = CalculateCraftingDuration(Blueprint, Station);
	float EnergyRequired = CalculateEnergyConsumption(CraftingTime, Station);

	// Calculate success chance
	float SuccessChance = CalculateSuccessChance(Blueprint, Station);

	if (FMath::FRand() > SuccessChance)
	{
		Result.bSuccess = false;
		Result.FailureReason = "Crafting failed";
		Result.CraftingTime = CraftingTime;
		Result.EnergyConsumed = EnergyRequired * 0.5f; // Half energy on failure
		Result.ExperienceGained = 1.0f; // Small XP on failure

		// Update statistics
		TotalCraftingOperations++;
		TotalEnergyConsumed += Result.EnergyConsumed;

		return Result;
	}

	// Success - produce item
	ProduceItem(Blueprint, Result, QualityMultiplier);

	Result.bSuccess = true;
	Result.CraftingTime = CraftingTime;
	Result.EnergyConsumed = EnergyRequired;
	Result.ExperienceGained = static_cast<int32>(Blueprint.CraftingTier) * 10.0f; // XP based on tier

	// Update statistics
	TotalCraftingOperations++;
	TotalEnergyConsumed += EnergyRequired;

	int32& CraftedAmount = TotalItemsCrafted.FindOrAdd(Blueprint.BlueprintID);
	CraftedAmount++;

	UE_LOG(LogTemp, Log, TEXT("Completed crafting operation: %s (Success: %s, Quality: %s)"), 
		   *Blueprint.BlueprintID.ToString(), Result.bSuccess ? TEXT("true") : TEXT("false"), 
		   *UEnum::GetDisplayValueAsText(Result.ItemQuality).ToString());

	return Result;
}

bool UCraftingSystem::CanCraftBlueprint(const FCraftingBlueprint& Blueprint, ASpaceStationHub* Station) const
{
	if (!Station)
	{
		return false;
	}

	// Get station info
	FCraftingStationInfo StationInfo = GetCraftingStationInfo(Station);
	if (!StationInfo.bIsOperational)
	{
		return false;
	}

	// Check station type
	if (Blueprint.RequiredStation != StationInfo.StationType)
	{
		return false;
	}

	// Check station tier
	if (Blueprint.MinimumStationTier > StationInfo.StationTier)
	{
		return false;
	}

	return true;
}

bool UCraftingSystem::HasRequiredResources(const FCraftingBlueprint& Blueprint) const
{
	if (!InventoryManager.IsValid())
	{
		return false;
	}

	// Check if we have all required resources in station storage
	// TODO: Pass actual station ID from crafting station
	FGuid DummyStationID = FGuid::NewGuid();

	for (const FResourceQuantity& Input : Blueprint.RequiredResources)
	{
		if (!InventoryManager->HasResource(
			Input.ResourceID,
			Input.Quantity,
			EInventoryType::StationStorage,
			DummyStationID))
		{
			UE_LOG(LogTemp, Warning, TEXT("Missing resource: %s (need %d)"),
				*Input.ResourceID.ToString(), Input.Quantity);
			return false;
		}
	}

	return true;
}

void UCraftingSystem::ConsumeResources(const FCraftingBlueprint& Blueprint)
{
	if (!InventoryManager.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot consume resources: InventoryManager not available"));
		return;
	}

	// Remove input resources from station storage
	// TODO: Pass actual station ID from crafting station
	FGuid DummyStationID = FGuid::NewGuid();

	for (const FResourceQuantity& Input : Blueprint.RequiredResources)
	{
		bool bRemoved = InventoryManager->RemoveResource(
			Input.ResourceID,
			Input.Quantity,
			EInventoryType::StationStorage,
			DummyStationID
		);

		if (bRemoved)
		{
			UE_LOG(LogTemp, Log, TEXT("Consumed %d %s for blueprint: %s"),
				Input.Quantity, *Input.ResourceID.ToString(), *Blueprint.BlueprintID.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to consume %d %s for blueprint: %s"),
				Input.Quantity, *Input.ResourceID.ToString(), *Blueprint.BlueprintID.ToString());
		}
	}
}

void UCraftingSystem::ProduceItem(const FCraftingBlueprint& Blueprint, FCraftingResult& Result, float QualityMultiplier)
{
	// Set output item
	Result.ProducedItem = Blueprint.OutputItem;

	// Calculate item quality
	float QualityRoll = FMath::FRand() * QualityMultiplier;
	Result.ItemQuality = CalculateItemQuality(QualityRoll, QualityMultiplier);

	// Add item to inventory
	if (InventoryManager.IsValid())
	{
		// TODO: Pass actual station ID from crafting station
		FGuid DummyStationID = FGuid::NewGuid();

		bool bAdded = InventoryManager->AddResource(
			Blueprint.OutputItem.ResourceID,
			Blueprint.OutputItem.Quantity,
			Result.ItemQuality,
			EInventoryType::StationStorage,
			DummyStationID
		);

		if (bAdded)
		{
			UE_LOG(LogTemp, Log, TEXT("Produced item from blueprint: %s (Quality: %s)"),
				*Blueprint.BlueprintID.ToString(), *UEnum::GetDisplayValueAsText(Result.ItemQuality).ToString());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to add produced item to inventory: %s"),
				*Blueprint.BlueprintID.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot produce item: InventoryManager not available"));
	}
}

float UCraftingSystem::CalculateSuccessChance(const FCraftingBlueprint& Blueprint, ASpaceStationHub* Station) const
{
	float BaseChance = Blueprint.SuccessRate;

	// Apply station tier bonus
	FCraftingStationInfo StationInfo = GetCraftingStationInfo(Station);
	float TierBonus = (StationInfo.StationTier - Blueprint.MinimumStationTier) * 0.05f; // 5% bonus per tier above minimum

	// Apply skill bonus
	float SkillBonus = (CraftingSkillLevel / 100.0f) * 0.15f; // Up to 15% bonus from skill

	return FMath::Clamp(BaseChance + TierBonus + SkillBonus, 0.0f, 1.0f);
}

EResourceQuality UCraftingSystem::CalculateItemQuality(float BaseQuality, float QualityMultiplier) const
{
	// Adjust quality based on multiplier
	float AdjustedQuality = BaseQuality * QualityMultiplier;

	// Convert to quality enum
	if (AdjustedQuality < 0.3f)
	{
		return EResourceQuality::Damaged;
	}
	else if (AdjustedQuality < 0.7f)
	{
		return EResourceQuality::Normal;
	}
	else if (AdjustedQuality < 0.95f)
	{
		return EResourceQuality::Pristine;
	}
	else
	{
		return EResourceQuality::Legendary;
	}
}

float UCraftingSystem::CalculateCraftingDuration(const FCraftingBlueprint& Blueprint, ASpaceStationHub* Station) const
{
	float BaseTime = Blueprint.CraftingTime;

	// Apply station speed multiplier
	FCraftingStationInfo StationInfo = GetCraftingStationInfo(Station);
	float StationSpeed = StationInfo.CraftingSpeedMultiplier;

	// Apply skill bonus
	float SkillMultiplier = 1.0f + (CraftingSkillLevel / 100.0f) * 0.3f;

	return BaseTime / (StationSpeed * SkillMultiplier);
}

float UCraftingSystem::CalculateEnergyConsumption(float Duration, ASpaceStationHub* Station) const
{
	// Base consumption: 10 energy per second
	float BaseConsumption = 10.0f;

	// Apply station efficiency
	FCraftingStationInfo StationInfo = GetCraftingStationInfo(Station);
	float Efficiency = StationInfo.EnergyEfficiencyMultiplier;

	return BaseConsumption * Duration * Efficiency;
}

void UCraftingSystem::InitializeShipComponentBlueprints()
{
	// Basic Engine
	FCraftingBlueprint BasicEngine;
	BasicEngine.BlueprintID = FName("BasicEngine");
	BasicEngine.DisplayName = "Basic Engine";
	BasicEngine.ItemCategory = ECraftedItemCategory::ShipComponent;
	BasicEngine.CraftingTier = ECraftingTier::Tier1_Basic;
	BasicEngine.RequiredStation = ECraftingStationType::Workshop;
	BasicEngine.MinimumStationTier = 1;
	BasicEngine.CraftingTime = 15.0f;
	BasicEngine.EnergyRequired = 20.0f;
	BasicEngine.SuccessRate = 1.0f;
	BasicEngine.bIsUnlocked = true;
	BasicEngine.UnlockMethod = EBlueprintUnlockMethod::Purchase;
	BasicEngine.UnlockCost = 50;
	BasicEngine.Description = "A basic engine for small spacecraft.";

	// Required resources
	FResourceQuantity SteelIngots;
	SteelIngots.ResourceID = FName("SteelIngots");
	SteelIngots.Quantity = 5;
	BasicEngine.RequiredResources.Add(SteelIngots);

	FResourceQuantity CopperIngots;
	CopperIngots.ResourceID = FName("CopperIngots");
	CopperIngots.Quantity = 3;
	BasicEngine.RequiredResources.Add(CopperIngots);

	FResourceQuantity Electronics;
	Electronics.ResourceID = FName("Electronics");
	Electronics.Quantity = 2;
	BasicEngine.RequiredResources.Add(Electronics);

	// Output
	BasicEngine.OutputItem.ResourceID = FName("BasicEngine");
	BasicEngine.OutputItem.Quantity = 1;

	RegisterBlueprint(BasicEngine);

	// Basic Shield Generator
	FCraftingBlueprint BasicShield;
	BasicShield.BlueprintID = FName("BasicShieldGenerator");
	BasicShield.DisplayName = "Basic Shield Generator";
	BasicShield.ItemCategory = ECraftedItemCategory::ShipComponent;
	BasicShield.CraftingTier = ECraftingTier::Tier1_Basic;
	BasicShield.RequiredStation = ECraftingStationType::Workshop;
	BasicShield.MinimumStationTier = 1;
	BasicShield.CraftingTime = 12.0f;
	BasicShield.EnergyRequired = 15.0f;
	BasicShield.SuccessRate = 1.0f;
	BasicShield.bIsUnlocked = true;
	BasicShield.UnlockMethod = EBlueprintUnlockMethod::Purchase;
	BasicShield.UnlockCost = 75;
	BasicShield.Description = "A basic shield generator for spacecraft protection.";

	FResourceQuantity PureSilicon;
	PureSilicon.ResourceID = FName("PureSilicon");
	PureSilicon.Quantity = 4;
	BasicShield.RequiredResources.Add(PureSilicon);

	FResourceQuantity CopperIngots2;
	CopperIngots2.ResourceID = FName("CopperIngots");
	CopperIngots2.Quantity = 2;
	BasicShield.RequiredResources.Add(CopperIngots2);

	BasicShield.OutputItem.ResourceID = FName("BasicShieldGenerator");
	BasicShield.OutputItem.Quantity = 1;

	RegisterBlueprint(BasicShield);
}

void UCraftingSystem::InitializeBaseModuleBlueprints()
{
	// Habitat Quarters Module
	FCraftingBlueprint HabitatModule;
	HabitatModule.BlueprintID = FName("HabitatQuartersModule");
	HabitatModule.DisplayName = "Habitat Quarters Module";
	HabitatModule.ItemCategory = ECraftedItemCategory::BaseModule;
	HabitatModule.CraftingTier = ECraftingTier::Tier2_Standard;
	HabitatModule.RequiredStation = ECraftingStationType::Factory;
	HabitatModule.MinimumStationTier = 1;
	HabitatModule.CraftingTime = 30.0f;
	HabitatModule.EnergyRequired = 50.0f;
	HabitatModule.SuccessRate = 0.95f;
	HabitatModule.bIsUnlocked = false;
	HabitatModule.UnlockMethod = EBlueprintUnlockMethod::Research;
	HabitatModule.UnlockCost = 200;
	HabitatModule.Description = "A habitat module for base construction.";

	FResourceQuantity SteelIngots;
	SteelIngots.ResourceID = FName("SteelIngots");
	SteelIngots.Quantity = 20;
	HabitatModule.RequiredResources.Add(SteelIngots);

	FResourceQuantity Electronics;
	Electronics.ResourceID = FName("Electronics");
	Electronics.Quantity = 10;
	HabitatModule.RequiredResources.Add(Electronics);

	FResourceQuantity Glass;
	Glass.ResourceID = FName("Glass");
	Glass.Quantity = 8;
	HabitatModule.RequiredResources.Add(Glass);

	HabitatModule.OutputItem.ResourceID = FName("HabitatQuartersModule");
	HabitatModule.OutputItem.Quantity = 1;

	RegisterBlueprint(HabitatModule);

	// Solar Panel Module
	FCraftingBlueprint SolarPanelModule;
	SolarPanelModule.BlueprintID = FName("SolarPanelModule");
	SolarPanelModule.DisplayName = "Solar Panel Module";
	SolarPanelModule.ItemCategory = ECraftedItemCategory::BaseModule;
	SolarPanelModule.CraftingTier = ECraftingTier::Tier2_Standard;
	SolarPanelModule.RequiredStation = ECraftingStationType::Factory;
	SolarPanelModule.MinimumStationTier = 1;
	SolarPanelModule.CraftingTime = 25.0f;
	SolarPanelModule.EnergyRequired = 40.0f;
	SolarPanelModule.SuccessRate = 0.96f;
	SolarPanelModule.bIsUnlocked = false;
	SolarPanelModule.UnlockMethod = EBlueprintUnlockMethod::Research;
	SolarPanelModule.UnlockCost = 150;
	SolarPanelModule.Description = "A solar panel module for power generation.";

	FResourceQuantity PureSilicon;
	PureSilicon.ResourceID = FName("PureSilicon");
	PureSilicon.Quantity = 15;
	SolarPanelModule.RequiredResources.Add(PureSilicon);

	FResourceQuantity CopperIngots;
	CopperIngots.ResourceID = FName("CopperIngots");
	CopperIngots.Quantity = 5;
	SolarPanelModule.RequiredResources.Add(CopperIngots);

	FResourceQuantity SteelIngots2;
	SteelIngots2.ResourceID = FName("SteelIngots");
	SteelIngots2.Quantity = 8;
	SolarPanelModule.RequiredResources.Add(SteelIngots2);

	SolarPanelModule.OutputItem.ResourceID = FName("SolarPanelModule");
	SolarPanelModule.OutputItem.Quantity = 1;

	RegisterBlueprint(SolarPanelModule);
}

void UCraftingSystem::InitializeEquipmentBlueprints()
{
	// Mining Laser Upgrade
	FCraftingBlueprint MiningLaserUpgrade;
	MiningLaserUpgrade.BlueprintID = FName("MiningLaserUpgrade");
	MiningLaserUpgrade.DisplayName = "Mining Laser Upgrade";
	MiningLaserUpgrade.ItemCategory = ECraftedItemCategory::Tool;
	MiningLaserUpgrade.CraftingTier = ECraftingTier::Tier2_Standard;
	MiningLaserUpgrade.RequiredStation = ECraftingStationType::Workshop;
	MiningLaserUpgrade.MinimumStationTier = 2;
	MiningLaserUpgrade.CraftingTime = 20.0f;
	MiningLaserUpgrade.EnergyRequired = 30.0f;
	MiningLaserUpgrade.SuccessRate = 0.9f;
	MiningLaserUpgrade.bIsUnlocked = false;
	MiningLaserUpgrade.UnlockMethod = EBlueprintUnlockMethod::Purchase;
	MiningLaserUpgrade.UnlockCost = 100;
	MiningLaserUpgrade.Description = "An upgraded mining laser for more efficient resource extraction.";

	FResourceQuantity AdvancedAlloy;
	AdvancedAlloy.ResourceID = FName("AdvancedAlloy");
	AdvancedAlloy.Quantity = 3;
	MiningLaserUpgrade.RequiredResources.Add(AdvancedAlloy);

	FResourceQuantity Electronics;
	Electronics.ResourceID = FName("Electronics");
	Electronics.Quantity = 5;
	MiningLaserUpgrade.RequiredResources.Add(Electronics);

	FResourceQuantity PureQuartz;
	PureQuartz.ResourceID = FName("PureQuartz");
	PureQuartz.Quantity = 2;
	MiningLaserUpgrade.RequiredResources.Add(PureQuartz);

	MiningLaserUpgrade.OutputItem.ResourceID = FName("MiningLaserUpgrade");
	MiningLaserUpgrade.OutputItem.Quantity = 1;

	RegisterBlueprint(MiningLaserUpgrade);

	// Spacesuit Upgrade
	FCraftingBlueprint SpacesuitUpgrade;
	SpacesuitUpgrade.BlueprintID = FName("SpacesuitUpgrade");
	SpacesuitUpgrade.DisplayName = "Spacesuit Upgrade";
	SpacesuitUpgrade.ItemCategory = ECraftedItemCategory::Equipment;
	SpacesuitUpgrade.CraftingTier = ECraftingTier::Tier3_Advanced;
	SpacesuitUpgrade.RequiredStation = ECraftingStationType::AdvancedFabricator;
	SpacesuitUpgrade.MinimumStationTier = 1;
	SpacesuitUpgrade.CraftingTime = 35.0f;
	SpacesuitUpgrade.EnergyRequired = 45.0f;
	SpacesuitUpgrade.SuccessRate = 0.88f;
	SpacesuitUpgrade.bIsUnlocked = false;
	SpacesuitUpgrade.UnlockMethod = EBlueprintUnlockMethod::Research;
	SpacesuitUpgrade.UnlockCost = 300;
	SpacesuitUpgrade.Description = "An upgraded spacesuit with enhanced protection.";

	FResourceQuantity AdvancedAlloy2;
	AdvancedAlloy2.ResourceID = FName("AdvancedAlloy");
	AdvancedAlloy2.Quantity = 5;
	SpacesuitUpgrade.RequiredResources.Add(AdvancedAlloy2);

	FResourceQuantity ProcessedThorium;
	ProcessedThorium.ResourceID = FName("ProcessedThorium");
	ProcessedThorium.Quantity = 2;
	SpacesuitUpgrade.RequiredResources.Add(ProcessedThorium);

	FResourceQuantity Biofuel;
	Biofuel.ResourceID = FName("Biofuel");
	Biofuel.Quantity = 10;
	SpacesuitUpgrade.RequiredResources.Add(Biofuel);

	SpacesuitUpgrade.OutputItem.ResourceID = FName("SpacesuitUpgrade");
	SpacesuitUpgrade.OutputItem.Quantity = 1;

	RegisterBlueprint(SpacesuitUpgrade);
}

void UCraftingSystem::InitializeConsumableBlueprints()
{
	// Medkit
	FCraftingBlueprint Medkit;
	Medkit.BlueprintID = FName("Medkit");
	Medkit.DisplayName = "Medkit";
	Medkit.ItemCategory = ECraftedItemCategory::Consumable;
	Medkit.CraftingTier = ECraftingTier::Tier1_Basic;
	Medkit.RequiredStation = ECraftingStationType::Workshop;
	Medkit.MinimumStationTier = 1;
	Medkit.CraftingTime = 8.0f;
	Medkit.EnergyRequired = 5.0f;
	Medkit.SuccessRate = 1.0f;
	Medkit.bIsUnlocked = true;
	Medkit.UnlockMethod = EBlueprintUnlockMethod::Purchase;
	Medkit.UnlockCost = 25;
	Medkit.Description = "A basic medkit for healing injuries.";

	FResourceQuantity Biomass;
	Biomass.ResourceID = FName("Biomass");
	Biomass.Quantity = 3;
	Medkit.RequiredResources.Add(Biomass);

	FResourceQuantity ProcessedBiomass;
	ProcessedBiomass.ResourceID = FName("ProcessedBiomass");
	ProcessedBiomass.Quantity = 1;
	Medkit.RequiredResources.Add(ProcessedBiomass);

	Medkit.OutputItem.ResourceID = FName("Medkit");
	Medkit.OutputItem.Quantity = 1;

	RegisterBlueprint(Medkit);

	// Repair Kit
	FCraftingBlueprint RepairKit;
	RepairKit.BlueprintID = FName("RepairKit");
	RepairKit.DisplayName = "Repair Kit";
	RepairKit.ItemCategory = ECraftedItemCategory::Consumable;
	RepairKit.CraftingTier = ECraftingTier::Tier1_Basic;
	RepairKit.RequiredStation = ECraftingStationType::Workshop;
	RepairKit.MinimumStationTier = 1;
	RepairKit.CraftingTime = 6.0f;
	RepairKit.EnergyRequired = 4.0f;
	RepairKit.SuccessRate = 1.0f;
	RepairKit.bIsUnlocked = true;
	RepairKit.UnlockMethod = EBlueprintUnlockMethod::Purchase;
	RepairKit.UnlockCost = 20;
	RepairKit.Description = "A repair kit for fixing equipment and modules.";

	FResourceQuantity SteelIngots;
	SteelIngots.ResourceID = FName("SteelIngots");
	SteelIngots.Quantity = 2;
	RepairKit.RequiredResources.Add(SteelIngots);

	FResourceQuantity CopperIngots;
	CopperIngots.ResourceID = FName("CopperIngots");
	CopperIngots.Quantity = 1;
	RepairKit.RequiredResources.Add(CopperIngots);

	RepairKit.OutputItem.ResourceID = FName("RepairKit");
	RepairKit.OutputItem.Quantity = 1;

	RegisterBlueprint(RepairKit);
}

void UCraftingSystem::InitializeAdvancedBlueprints()
{
	// Quantum Core
	FCraftingBlueprint QuantumCore;
	QuantumCore.BlueprintID = FName("QuantumCore");
	QuantumCore.DisplayName = "Quantum Core";
	QuantumCore.ItemCategory = ECraftedItemCategory::ShipComponent;
	QuantumCore.CraftingTier = ECraftingTier::Tier4_Masterwork;
	QuantumCore.RequiredStation = ECraftingStationType::MasterworkForge;
	QuantumCore.MinimumStationTier = 1;
	QuantumCore.CraftingTime = 60.0f;
	QuantumCore.EnergyRequired = 200.0f;
	QuantumCore.SuccessRate = 0.75f;
	QuantumCore.bIsUnlocked = false;
	QuantumCore.UnlockMethod = EBlueprintUnlockMethod::FactionReward;
	QuantumCore.UnlockCost = 1000;
	QuantumCore.RequiredFaction = "ScienceGuild";
	QuantumCore.RequiredReputation = 5000;
	QuantumCore.Description = "A masterwork quantum core for advanced spacecraft.";

	FResourceQuantity QuantumCrystals;
	QuantumCrystals.ResourceID = FName("QuantumCrystals");
	QuantumCrystals.Quantity = 10;
	QuantumCore.RequiredResources.Add(QuantumCrystals);

	FResourceQuantity EnrichedUranium;
	EnrichedUranium.ResourceID = FName("EnrichedUranium");
	EnrichedUranium.Quantity = 5;
	QuantumCore.RequiredResources.Add(EnrichedUranium);

	FResourceQuantity AdvancedAlloy;
	AdvancedAlloy.ResourceID = FName("AdvancedAlloy");
	AdvancedAlloy.Quantity = 8;
	QuantumCore.RequiredResources.Add(AdvancedAlloy);

	FResourceQuantity PureSilicon;
	PureSilicon.ResourceID = FName("PureSilicon");
	PureSilicon.Quantity = 15;
	QuantumCore.RequiredResources.Add(PureSilicon);

	QuantumCore.OutputItem.ResourceID = FName("QuantumCore");
	QuantumCore.OutputItem.Quantity = 1;

	RegisterBlueprint(QuantumCore);

	// Fusion Reactor
	FCraftingBlueprint FusionReactor;
	FusionReactor.BlueprintID = FName("FusionReactor");
	FusionReactor.DisplayName = "Fusion Reactor";
	FusionReactor.ItemCategory = ECraftedItemCategory::BaseModule;
	FusionReactor.CraftingTier = ECraftingTier::Tier4_Masterwork;
	FusionReactor.RequiredStation = ECraftingStationType::MasterworkForge;
	FusionReactor.MinimumStationTier = 1;
	FusionReactor.CraftingTime = 90.0f;
	FusionReactor.EnergyRequired = 300.0f;
	FusionReactor.SuccessRate = 0.7f;
	FusionReactor.bIsUnlocked = false;
	FusionReactor.UnlockMethod = EBlueprintUnlockMethod::Research;
	FusionReactor.UnlockCost = 2000;
	FusionReactor.Description = "A masterwork fusion reactor for ultimate power generation.";

	FResourceQuantity EnrichedUranium2;
	EnrichedUranium2.ResourceID = FName("EnrichedUranium");
	EnrichedUranium2.Quantity = 10;
	FusionReactor.RequiredResources.Add(EnrichedUranium2);

	FResourceQuantity ProcessedThorium;
	ProcessedThorium.ResourceID = FName("ProcessedThorium");
	ProcessedThorium.Quantity = 8;
	FusionReactor.RequiredResources.Add(ProcessedThorium);

	FResourceQuantity AdvancedAlloy2;
	AdvancedAlloy2.ResourceID = FName("AdvancedAlloy");
	AdvancedAlloy2.Quantity = 15;
	FusionReactor.RequiredResources.Add(AdvancedAlloy2);

	FResourceQuantity QuantumCoreComp;
	QuantumCoreComp.ResourceID = FName("QuantumCore");
	QuantumCoreComp.Quantity = 1;
	FusionReactor.RequiredResources.Add(QuantumCoreComp);

	FusionReactor.OutputItem.ResourceID = FName("FusionReactor");
	FusionReactor.OutputItem.Quantity = 1;

	RegisterBlueprint(FusionReactor);
}