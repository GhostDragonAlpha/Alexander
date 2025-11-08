// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ShipCustomizationComponent.h"
#include "ProgressionCalculator.generated.h"

/**
 * Unlock Information
 */
USTRUCT(BlueprintType)
struct FUnlockInfo
{
	GENERATED_BODY()

	// Parts unlocked at this level
	UPROPERTY(BlueprintReadOnly, Category = "Unlock")
	TArray<FName> Parts;

	// Skins unlocked at this level
	UPROPERTY(BlueprintReadOnly, Category = "Unlock")
	TArray<FName> Skins;

	// Features unlocked at this level
	UPROPERTY(BlueprintReadOnly, Category = "Unlock")
	TArray<FString> Features;

	// Description
	UPROPERTY(BlueprintReadOnly, Category = "Unlock")
	FString Description;
};

/**
 * Static utility class for progression and economy calculations
 * Uses formulas from ProgressionSystem.json and EconomyBalance.json
 */
UCLASS()
class ALEXANDER_API UProgressionCalculator : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ============================================================================
	// LEVEL & XP CALCULATIONS
	// ============================================================================

	/**
	 * Calculate total XP required to reach a specific level
	 * @param Level Target level (1-50)
	 * @return Total XP required from level 1
	 */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	static int32 CalculateXPForLevel(int32 Level);

	/**
	 * Calculate XP required for next level from current XP
	 * @param CurrentXP Player's current total XP
	 * @param OutCurrentLevel Returns the current level
	 * @param OutXPForNextLevel Returns XP needed for next level
	 * @param OutXPProgress Returns progress to next level (0.0 - 1.0)
	 */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	static void GetLevelProgress(int32 CurrentXP, int32& OutCurrentLevel, int32& OutXPForNextLevel, float& OutXPProgress);

	/**
	 * Get level from total XP
	 * @param TotalXP Player's total XP
	 * @return Current level
	 */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	static int32 GetLevelFromXP(int32 TotalXP);

	/**
	 * Calculate XP reward for activity
	 * @param ActivityType Type of activity (race, asteroid, mission, etc.)
	 * @param BaseReward Base XP amount
	 * @param Multiplier Difficulty or performance multiplier
	 * @return Total XP reward
	 */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	static int32 CalculateXPReward(const FString& ActivityType, int32 BaseReward, float Multiplier = 1.0f);

	// ============================================================================
	// ECONOMY CALCULATIONS
	// ============================================================================

	/**
	 * Calculate cost of a ship part
	 * @param PartData Part to calculate cost for
	 * @return Credit cost
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	static int32 CalculatePartCost(const FShipPartData& PartData);

	/**
	 * Calculate cost of a ship skin
	 * @param SkinData Skin to calculate cost for
	 * @return Credit cost
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	static int32 CalculateSkinCost(const FShipSkinData& SkinData);

	/**
	 * Calculate upgrade cost for a part
	 * @param PartData Part to upgrade
	 * @param UpgradeLevel Target upgrade level (1-5)
	 * @return Credit cost for this upgrade
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	static int32 CalculateUpgradeCost(const FShipPartData& PartData, int32 UpgradeLevel);

	/**
	 * Calculate total cost to fully upgrade a part
	 * @param PartData Part to calculate for
	 * @return Total credit cost for all 5 upgrades
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	static int32 CalculateTotalUpgradeCost(const FShipPartData& PartData);

	/**
	 * Calculate credits reward for activity
	 * @param ActivityType Type of activity
	 * @param BaseReward Base credit amount
	 * @param Multiplier Performance/difficulty multiplier
	 * @param bFirstTime Is this the first time completing this activity?
	 * @return Total credit reward
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	static int32 CalculateCreditsReward(const FString& ActivityType, int32 BaseReward, float Multiplier = 1.0f, bool bFirstTime = false);

	// ============================================================================
	// UNLOCK VALIDATION
	// ============================================================================

	/**
	 * Get all unlocks available at a specific level
	 * @param Level Level to check
	 * @return Unlock information
	 */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	static FUnlockInfo GetUnlocksForLevel(int32 Level);

	/**
	 * Check if a part can be unlocked by the player
	 * @param PartID Part to check
	 * @param PlayerLevel Player's current level
	 * @param Credits Player's current credits
	 * @param OutReason Returns reason if cannot unlock
	 * @return True if part can be unlocked
	 */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	static bool IsPartUnlockable(FName PartID, int32 PlayerLevel, int32 Credits, FString& OutReason);

	/**
	 * Check if a skin can be unlocked by the player
	 * @param SkinID Skin to check
	 * @param PlayerLevel Player's current level
	 * @param Credits Player's current credits
	 * @param OutReason Returns reason if cannot unlock
	 * @return True if skin can be unlocked
	 */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	static bool IsSkinUnlockable(FName SkinID, int32 PlayerLevel, int32 Credits, FString& OutReason);

	// ============================================================================
	// STAT CALCULATIONS
	// ============================================================================

	/**
	 * Calculate total stat impact (sum of all stat bonuses) for a part
	 * Used for cost calculations
	 * @param Stats Ship stats to evaluate
	 * @return Total stat bonus as percentage
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	static float CalculateTotalStatImpact(const FShipStats& Stats);

	/**
	 * Get rarity multiplier for costs
	 * @param Rarity Part/skin rarity
	 * @return Base cost multiplier
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	static float GetRarityMultiplier(EShipPartRarity Rarity);

	/**
	 * Get category multiplier for part costs
	 * @param Category Part category
	 * @return Category cost multiplier
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	static float GetCategoryMultiplier(EShipPartCategory Category);

	// ============================================================================
	// LOADOUT COSTS
	// ============================================================================

	/**
	 * Get cost to unlock a loadout slot
	 * @param SlotNumber Slot to unlock (1-6, slot 1 is free)
	 * @return Credit cost
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	static int32 GetLoadoutSlotCost(int32 SlotNumber);

	// ============================================================================
	// PROGRESSION VALIDATION
	// ============================================================================

	/**
	 * Validate if player level and credits match expected progression
	 * Used for anti-cheat and balancing
	 * @param PlayerLevel Current level
	 * @param TotalCreditsEarned Total credits earned (not current balance)
	 * @return True if progression seems legitimate
	 */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	static bool ValidateProgression(int32 PlayerLevel, int32 TotalCreditsEarned);

	/**
	 * Get expected credits earned by a certain level
	 * @param Level Target level
	 * @return Approximate total credits earned
	 */
	UFUNCTION(BlueprintCallable, Category = "Progression")
	static int32 GetExpectedCreditsForLevel(int32 Level);

	// ============================================================================
	// HELPERS
	// ============================================================================

	/**
	 * Get base cost for rarity
	 * @param Rarity Item rarity
	 * @param bIsSkin True for skins, false for parts
	 * @return Base credit cost
	 */
	UFUNCTION(BlueprintCallable, Category = "Economy")
	static int32 GetBaseCostForRarity(EShipPartRarity Rarity, bool bIsSkin = false);

private:
	// Progression constants (from ProgressionSystem.json)
	static constexpr int32 MaxLevel = 50;
	static constexpr int32 BaseXP = 100;
	static constexpr float XPExponent = 1.15f;
	static constexpr int32 XPSmoothing = 50;

	// Economy constants (from EconomyBalance.json)
	static constexpr int32 CommonPartCost = 500;
	static constexpr int32 UncommonPartCost = 1500;
	static constexpr int32 RarePartCost = 4000;
	static constexpr int32 EpicPartCost = 10000;
	static constexpr int32 LegendaryPartCost = 25000;

	static constexpr int32 CommonSkinCost = 300;
	static constexpr int32 UncommonSkinCost = 800;
	static constexpr int32 RareSkinCost = 2000;
	static constexpr int32 EpicSkinCost = 5000;
	static constexpr int32 LegendarySkinCost = 12000;

	// Category multipliers
	static constexpr float EngineMultiplier = 1.5f;
	static constexpr float ThrusterMultiplier = 1.2f;
	static constexpr float HullMultiplier = 1.3f;
	static constexpr float WingMultiplier = 1.1f;
	static constexpr float CockpitMultiplier = 1.0f;
	static constexpr float WeaponMultiplier = 1.4f;
	static constexpr float ShieldMultiplier = 1.3f;
	static constexpr float UtilityMultiplier = 1.0f;

	// Upgrade multipliers
	static constexpr float Upgrade1Multiplier = 0.5f;
	static constexpr float Upgrade2Multiplier = 0.75f;
	static constexpr float Upgrade3Multiplier = 1.0f;
	static constexpr float Upgrade4Multiplier = 1.5f;
	static constexpr float Upgrade5Multiplier = 2.0f;

	// Reward multipliers
	static constexpr float FirstTimeCreditBonus = 1.5f;
	static constexpr float StatImpactMultiplier = 0.1f;

	// XP requirement lookup table (pre-calculated for performance)
	static const TArray<int32> XPRequirements;

	// Initialize XP requirements table
	static TArray<int32> InitializeXPTable();
};
