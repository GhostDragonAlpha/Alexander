// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ResourceGatheringSystem.h"
#include "CraftingSystem.generated.h"

// Forward declarations
class ABaseModule;
class UInventoryManager;

// Crafting tiers
UENUM(BlueprintType)
enum class ECraftingTier : uint8
{
	Tier1_Basic UMETA(DisplayName = "Tier 1 - Basic"),
	Tier2_Standard UMETA(DisplayName = "Tier 2 - Standard"),
	Tier3_Advanced UMETA(DisplayName = "Tier 3 - Advanced"),
	Tier4_Masterwork UMETA(DisplayName = "Tier 4 - Masterwork")
};

// Crafting station types
UENUM(BlueprintType)
enum class ECraftingStationType : uint8
{
	Workshop UMETA(DisplayName = "Workshop"),
	Factory UMETA(DisplayName = "Factory"),
	AdvancedFabricator UMETA(DisplayName = "Advanced Fabricator"),
	MasterworkForge UMETA(DisplayName = "Masterwork Forge")
};

// Item categories
UENUM(BlueprintType)
enum class ECraftedItemCategory : uint8
{
	ShipComponent UMETA(DisplayName = "Ship Component"),
	BaseModule UMETA(DisplayName = "Base Module"),
	Equipment UMETA(DisplayName = "Equipment"),
	Consumable UMETA(DisplayName = "Consumable"),
	Tool UMETA(DisplayName = "Tool"),
	Weapon UMETA(DisplayName = "Weapon")
};

// Blueprint unlock methods
UENUM(BlueprintType)
enum class EBlueprintUnlockMethod : uint8
{
	Purchase UMETA(DisplayName = "Purchase"),
	Research UMETA(DisplayName = "Research"),
	Discovery UMETA(DisplayName = "Discovery"),
	FactionReward UMETA(DisplayName = "Faction Reward"),
	QuestReward UMETA(DisplayName = "Quest Reward")
};

// Crafting blueprint
USTRUCT(BlueprintType)
struct FCraftingBlueprint
{
	GENERATED_BODY()

	// Blueprint ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	FName BlueprintID;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	FString DisplayName;

	// Item category
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	ECraftedItemCategory ItemCategory;

	// Crafting tier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	ECraftingTier CraftingTier;

	// Required crafting station
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	ECraftingStationType RequiredStation;

	// Minimum station tier required
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	int32 MinimumStationTier;

	// Input resources required
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	TArray<FResourceQuantity> RequiredResources;

	// Output item
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	FResourceQuantity OutputItem;

	// Crafting time in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	float CraftingTime;

	// Energy required
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	float EnergyRequired;

	// Is blueprint unlocked?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	bool bIsUnlocked;

	// Unlock method
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	EBlueprintUnlockMethod UnlockMethod;

	// Unlock cost (if purchased)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	int32 UnlockCost;

	// Required faction reputation (if faction reward)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	FString RequiredFaction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	int32 RequiredReputation;

	// Blueprint description
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	FString Description;

	// Success rate (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blueprint")
	float SuccessRate;

	FCraftingBlueprint()
	{
		CraftingTier = ECraftingTier::Tier1_Basic;
		RequiredStation = ECraftingStationType::Workshop;
		MinimumStationTier = 1;
		CraftingTime = 10.0f;
		EnergyRequired = 10.0f;
		bIsUnlocked = false;
		UnlockMethod = EBlueprintUnlockMethod::Purchase;
		UnlockCost = 100;
		RequiredReputation = 0;
		SuccessRate = 1.0f;
	}
};

// Active crafting operation
USTRUCT(BlueprintType)
struct FActiveCraftingOperation
{
	GENERATED_BODY()

	// Operation ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Operation")
	FGuid OperationID;

	// Blueprint being crafted
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Operation")
	FCraftingBlueprint Blueprint;

	// Crafting station being used
	UPROPERTY()
	TWeakObjectPtr<ABaseModule> CraftingStation;

	// Start time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Operation")
	float StartTime;

	// Is operation active?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Operation")
	bool bIsActive;

	// Progress (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Operation")
	float Progress;

	// Quality multiplier (affects final item quality)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Operation")
	float QualityMultiplier;

	FActiveCraftingOperation()
	{
		OperationID = FGuid::NewGuid();
		StartTime = 0.0f;
		bIsActive = false;
		Progress = 0.0f;
		QualityMultiplier = 1.0f;
	}
};

// Crafting operation result
USTRUCT(BlueprintType)
struct FCraftingResult
{
	GENERATED_BODY()

	// Was crafting successful?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bSuccess;

	// Item produced
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FResourceQuantity ProducedItem;

	// Item quality
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	EResourceQuality ItemQuality;

	// Energy consumed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	float EnergyConsumed;

	// Time taken
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	float CraftingTime;

	// Experience gained
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	float ExperienceGained;

	// Failure reason
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FString FailureReason;

	FCraftingResult()
	{
		bSuccess = false;
		ItemQuality = EResourceQuality::Normal;
		EnergyConsumed = 0.0f;
		CraftingTime = 0.0f;
		ExperienceGained = 0.0f;
	}
};

// Crafting station information
USTRUCT(BlueprintType)
struct FCraftingStationInfo
{
	GENERATED_BODY()

	// Station reference
	UPROPERTY()
	TWeakObjectPtr<ABaseModule> Station;

	// Station type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	ECraftingStationType StationType;

	// Station tier (1-5)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	int32 StationTier;

	// Is station operational?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	bool bIsOperational;

	// Current operation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	FActiveCraftingOperation CurrentOperation;

	// Crafting speed multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	float CraftingSpeedMultiplier;

	// Energy efficiency multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	float EnergyEfficiencyMultiplier;

	FCraftingStationInfo()
	{
		StationType = ECraftingStationType::Workshop;
		StationTier = 1;
		bIsOperational = false;
		CraftingSpeedMultiplier = 1.0f;
		EnergyEfficiencyMultiplier = 1.0f;
	}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBlueprintUnlocked, const FCraftingBlueprint&, Blueprint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCraftingStarted, const FCraftingBlueprint&, Blueprint, ABaseModule*, Station);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCraftingCompleted, const FCraftingBlueprint&, Blueprint, const FCraftingResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCraftingStationAdded, ABaseModule*, Station, ECraftingStationType, StationType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCraftingStationRemoved, ABaseModule*, Station);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCraftingSkillIncreased);

/**
 * Crafting System - Handles blueprints and tiered crafting operations
 */
UCLASS()
class ALEXANDER_API UCraftingSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UCraftingSystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Tick function
	virtual void Tick(float DeltaTime);
	virtual bool IsTickable() const { return true; }

	// ============================================================================
	// BLUEPRINT MANAGEMENT
	// ============================================================================

	// Register a crafting blueprint
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void RegisterBlueprint(const FCraftingBlueprint& Blueprint);

	// Get blueprint by ID
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	FCraftingBlueprint GetBlueprint(FName BlueprintID) const;

	// Get all blueprints
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	TArray<FCraftingBlueprint> GetAllBlueprints() const;

	// Get blueprints by tier
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	TArray<FCraftingBlueprint> GetBlueprintsByTier(ECraftingTier Tier) const;

	// Get blueprints by category
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	TArray<FCraftingBlueprint> GetBlueprintsByCategory(ECraftedItemCategory Category) const;

	// Get unlocked blueprints
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	TArray<FCraftingBlueprint> GetUnlockedBlueprints() const;

	// Unlock blueprint
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	bool UnlockBlueprint(FName BlueprintID);

	// Purchase blueprint
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	bool PurchaseBlueprint(FName BlueprintID, int32 PlayerCredits);

	// ============================================================================
	// CRAFTING OPERATIONS
	// ============================================================================

	// Start crafting operation
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	bool StartCrafting(FName BlueprintID, ABaseModule* Station);

	// Cancel crafting operation
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void CancelCrafting(ABaseModule* Station);

	// Is station currently crafting?
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	bool IsStationCrafting(ABaseModule* Station) const;

	// Get crafting progress (0-1)
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	float GetCraftingProgress(ABaseModule* Station) const;

	// ============================================================================
	// CRAFTING STATION MANAGEMENT
	// ============================================================================

	// Register crafting station
	UFUNCTION(BlueprintCallable, Category = "Crafting Stations")
	void RegisterCraftingStation(ABaseModule* Station, ECraftingStationType StationType, int32 Tier);

	// Unregister crafting station
	UFUNCTION(BlueprintCallable, Category = "Crafting Stations")
	void UnregisterCraftingStation(ABaseModule* Station);

	// Get crafting station info
	UFUNCTION(BlueprintCallable, Category = "Crafting Stations")
	FCraftingStationInfo GetCraftingStationInfo(ABaseModule* Station) const;

	// Get all crafting stations
	UFUNCTION(BlueprintCallable, Category = "Crafting Stations")
	TArray<FCraftingStationInfo> GetAllCraftingStations() const;

	// Get stations by type
	UFUNCTION(BlueprintCallable, Category = "Crafting Stations")
	TArray<FCraftingStationInfo> GetStationsByType(ECraftingStationType StationType) const;

	// Update station operational status
	UFUNCTION(BlueprintCallable, Category = "Crafting Stations")
	void SetStationOperational(ABaseModule* Station, bool bIsOperational);

	// ============================================================================
	// CRAFTING SKILL
	// ============================================================================

	// Get current crafting skill level
	UFUNCTION(BlueprintCallable, Category = "Skill")
	float GetCraftingSkillLevel() const { return CraftingSkillLevel; }

	// Increase crafting skill
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void IncreaseCraftingSkill(float Amount);

	// Get skill bonus multiplier
	UFUNCTION(BlueprintCallable, Category = "Skill")
	float GetSkillBonus() const;

	// ============================================================================
	// DEFAULT BLUEPRINTS
	// ============================================================================

	// Initialize default crafting blueprints
	UFUNCTION(BlueprintCallable, Category = "Blueprints")
	void InitializeDefaultBlueprints();

	// Load blueprints from data assets
	UFUNCTION(BlueprintCallable, Category = "Blueprints")
	void LoadBlueprintsFromDataAssets();

	// ============================================================================
	// STATISTICS
	// ============================================================================

	// Get total crafting operations
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	int32 GetTotalCraftingOperations() const { return TotalCraftingOperations; }

	// Get total items crafted
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	TMap<FName, int32> GetTotalItemsCrafted() const { return TotalItemsCrafted; }

	// Get total energy consumed
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	float GetTotalEnergyConsumed() const { return TotalEnergyConsumed; }

	// Reset statistics
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	void ResetStatistics();

	// ============================================================================
	// EVENTS
	// ============================================================================

	UPROPERTY(BlueprintAssignable, Category = "Crafting Events")
	FOnBlueprintUnlocked OnBlueprintUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Crafting Events")
	FOnCraftingStarted OnCraftingStarted;

	UPROPERTY(BlueprintAssignable, Category = "Crafting Events")
	FOnCraftingCompleted OnCraftingCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Crafting Events")
	FOnCraftingStationAdded OnCraftingStationAdded;

	UPROPERTY(BlueprintAssignable, Category = "Crafting Events")
	FOnCraftingStationRemoved OnCraftingStationRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Crafting Events")
	FOnCraftingSkillIncreased OnCraftingSkillIncreased;

protected:
	// Resource gathering system reference
	UPROPERTY()
	TWeakObjectPtr<UResourceGatheringSystem> ResourceGatheringSystem;

	// Inventory manager reference
	UPROPERTY()
	TWeakObjectPtr<UInventoryManager> InventoryManager;

	// Crafting blueprints database
	UPROPERTY()
	TMap<FName, FCraftingBlueprint> Blueprints;

	// Crafting stations
	UPROPERTY()
	TMap<ABaseModule*, FCraftingStationInfo> CraftingStations;

	// Active crafting operations
	UPROPERTY()
	TArray<FActiveCraftingOperation> ActiveOperations;

	// Statistics
	UPROPERTY()
	int32 TotalCraftingOperations;

	UPROPERTY()
	TMap<FName, int32> TotalItemsCrafted;

	UPROPERTY()
	float TotalEnergyConsumed;

	// Crafting skill level (0-100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	float CraftingSkillLevel;

private:
	// Helper functions
	void InitializeShipComponentBlueprints();
	void InitializeBaseModuleBlueprints();
	void InitializeEquipmentBlueprints();
	void InitializeConsumableBlueprints();
	void InitializeAdvancedBlueprints();
	FCraftingResult CompleteCraftingOperation(FActiveCraftingOperation& Operation);
	bool CanCraftBlueprint(const FCraftingBlueprint& Blueprint, ABaseModule* Station) const;
	bool HasRequiredResources(const FCraftingBlueprint& Blueprint) const;
	void ConsumeResources(const FCraftingBlueprint& Blueprint);
	FCraftingResult CraftItem(const FCraftingBlueprint& Blueprint, ABaseModule* Station, float QualityMultiplier);
	void ProduceItem(const FCraftingBlueprint& Blueprint, FCraftingResult& Result, float QualityMultiplier);
	float CalculateSuccessChance(const FCraftingBlueprint& Blueprint, ABaseModule* Station) const;
	EResourceQuality CalculateItemQuality(float BaseQuality, float QualityMultiplier) const;
	float CalculateCraftingDuration(const FCraftingBlueprint& Blueprint, ABaseModule* Station) const;
	float CalculateEnergyConsumption(float Duration, ABaseModule* Station) const;
};