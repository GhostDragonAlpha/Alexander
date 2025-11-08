// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProgressionCalculator.h"
#include "Math/UnrealMathUtility.h"

// Initialize static XP table
const TArray<int32> UProgressionCalculator::XPRequirements = UProgressionCalculator::InitializeXPTable();

TArray<int32> UProgressionCalculator::InitializeXPTable()
{
	TArray<int32> Table;
	Table.SetNum(MaxLevel + 1);

	Table[0] = 0;
	Table[1] = 0;

	// Calculate XP requirements for each level using exponential formula
	for (int32 Level = 2; Level <= MaxLevel; ++Level)
	{
		// Formula: BaseXP * (Level ^ Exponent) + Smoothing * (Level - 1)
		float XPRequired = BaseXP * FMath::Pow(static_cast<float>(Level), XPExponent) + XPSmoothing * (Level - 1);
		Table[Level] = FMath::RoundToInt(XPRequired);
	}

	return Table;
}

// ============================================================================
// LEVEL & XP CALCULATIONS
// ============================================================================

int32 UProgressionCalculator::CalculateXPForLevel(int32 Level)
{
	Level = FMath::Clamp(Level, 1, MaxLevel);
	return XPRequirements[Level];
}

void UProgressionCalculator::GetLevelProgress(int32 CurrentXP, int32& OutCurrentLevel, int32& OutXPForNextLevel, float& OutXPProgress)
{
	// Find current level
	OutCurrentLevel = GetLevelFromXP(CurrentXP);

	if (OutCurrentLevel >= MaxLevel)
	{
		// Max level reached
		OutXPForNextLevel = 0;
		OutXPProgress = 1.0f;
		return;
	}

	// Get XP for current and next level
	int32 CurrentLevelXP = XPRequirements[OutCurrentLevel];
	int32 NextLevelXP = XPRequirements[OutCurrentLevel + 1];

	// Calculate progress
	int32 XPIntoLevel = CurrentXP - CurrentLevelXP;
	int32 XPNeededForLevel = NextLevelXP - CurrentLevelXP;

	OutXPForNextLevel = XPNeededForLevel - XPIntoLevel;
	OutXPProgress = static_cast<float>(XPIntoLevel) / static_cast<float>(XPNeededForLevel);
}

int32 UProgressionCalculator::GetLevelFromXP(int32 TotalXP)
{
	// Binary search through XP table
	for (int32 Level = MaxLevel; Level >= 1; --Level)
	{
		if (TotalXP >= XPRequirements[Level])
		{
			return Level;
		}
	}

	return 1;
}

int32 UProgressionCalculator::CalculateXPReward(const FString& ActivityType, int32 BaseReward, float Multiplier)
{
	float TotalReward = BaseReward * Multiplier;
	return FMath::RoundToInt(TotalReward);
}

// ============================================================================
// ECONOMY CALCULATIONS
// ============================================================================

int32 UProgressionCalculator::CalculatePartCost(const FShipPartData& PartData)
{
	// Base cost from rarity
	int32 BaseCost = GetBaseCostForRarity(PartData.Rarity, false);

	// Category multiplier
	float CategoryMult = GetCategoryMultiplier(PartData.Category);

	// Stat impact multiplier
	float StatImpact = CalculateTotalStatImpact(PartData.StatModifiers);
	float StatMult = 1.0f + (StatImpact * StatImpactMultiplier);

	// Final cost
	float FinalCost = BaseCost * CategoryMult * StatMult;

	return FMath::RoundToInt(FinalCost);
}

int32 UProgressionCalculator::CalculateSkinCost(const FShipSkinData& SkinData)
{
	// Skins are simpler - just base rarity cost
	// In a real implementation, could add theme multipliers based on skin name/tags
	return GetBaseCostForRarity(SkinData.Rarity, true);
}

int32 UProgressionCalculator::CalculateUpgradeCost(const FShipPartData& PartData, int32 UpgradeLevel)
{
	if (UpgradeLevel < 1 || UpgradeLevel > 5)
	{
		return 0;
	}

	// Get base part cost
	int32 BaseCost = CalculatePartCost(PartData);

	// Get upgrade multiplier
	float UpgradeMultiplier = 1.0f;
	switch (UpgradeLevel)
	{
		case 1: UpgradeMultiplier = Upgrade1Multiplier; break;
		case 2: UpgradeMultiplier = Upgrade2Multiplier; break;
		case 3: UpgradeMultiplier = Upgrade3Multiplier; break;
		case 4: UpgradeMultiplier = Upgrade4Multiplier; break;
		case 5: UpgradeMultiplier = Upgrade5Multiplier; break;
	}

	return FMath::RoundToInt(BaseCost * UpgradeMultiplier);
}

int32 UProgressionCalculator::CalculateTotalUpgradeCost(const FShipPartData& PartData)
{
	int32 TotalCost = 0;
	for (int32 i = 1; i <= 5; ++i)
	{
		TotalCost += CalculateUpgradeCost(PartData, i);
	}
	return TotalCost;
}

int32 UProgressionCalculator::CalculateCreditsReward(const FString& ActivityType, int32 BaseReward, float Multiplier, bool bFirstTime)
{
	float TotalReward = BaseReward * Multiplier;

	// Apply first time bonus
	if (bFirstTime)
	{
		TotalReward *= FirstTimeCreditBonus;
	}

	return FMath::RoundToInt(TotalReward);
}

// ============================================================================
// UNLOCK VALIDATION
// ============================================================================

FUnlockInfo UProgressionCalculator::GetUnlocksForLevel(int32 Level)
{
	FUnlockInfo UnlockInfo;

	// This would ideally load from ProgressionSystem.json
	// For now, return basic structure
	// In a real implementation, parse the unlockSchedule from JSON

	UnlockInfo.Description = FString::Printf(TEXT("Unlocks for level %d"), Level);

	// Example unlocks (would come from JSON)
	switch (Level)
	{
		case 1:
			UnlockInfo.Parts.Add(TEXT("engine_basic"));
			UnlockInfo.Parts.Add(TEXT("thruster_basic"));
			UnlockInfo.Skins.Add(TEXT("default"));
			UnlockInfo.Features.Add(TEXT("basic_customization"));
			break;
		case 5:
			UnlockInfo.Parts.Add(TEXT("hull_reinforced"));
			UnlockInfo.Parts.Add(TEXT("shield_basic"));
			UnlockInfo.Features.Add(TEXT("shield_system"));
			break;
		case 10:
			UnlockInfo.Parts.Add(TEXT("engine_race"));
			UnlockInfo.Skins.Add(TEXT("racing_stripes"));
			UnlockInfo.Features.Add(TEXT("loadout_slots_2"));
			break;
		// Add more levels as needed
	}

	return UnlockInfo;
}

bool UProgressionCalculator::IsPartUnlockable(FName PartID, int32 PlayerLevel, int32 Credits, FString& OutReason)
{
	// This would check against data tables and progression data
	// For now, basic validation

	// TODO: Look up part in data table
	// TODO: Check level requirement
	// TODO: Check credit cost

	OutReason = TEXT("Part can be unlocked");
	return true;
}

bool UProgressionCalculator::IsSkinUnlockable(FName SkinID, int32 PlayerLevel, int32 Credits, FString& OutReason)
{
	// Similar to part unlocking
	OutReason = TEXT("Skin can be unlocked");
	return true;
}

// ============================================================================
// STAT CALCULATIONS
// ============================================================================

float UProgressionCalculator::CalculateTotalStatImpact(const FShipStats& Stats)
{
	// Sum all multiplicative stats (values above 1.0 represent bonuses)
	float TotalImpact = 0.0f;

	// Multiplicative stats
	TotalImpact += FMath::Max(0.0f, Stats.ThrustPower - 1.0f);
	TotalImpact += FMath::Max(0.0f, Stats.MaxVelocity - 1.0f);
	TotalImpact += FMath::Max(0.0f, Stats.RotationSpeed - 1.0f);
	TotalImpact += FMath::Max(0.0f, Stats.Acceleration - 1.0f);
	TotalImpact += FMath::Max(0.0f, Stats.WeaponDamage - 1.0f);

	// Additive stats (normalized)
	TotalImpact += Stats.HullIntegrity / 100.0f; // Every 100 hull = 1.0 impact
	TotalImpact += Stats.ShieldStrength / 100.0f;
	TotalImpact += Stats.EnergyCapacity / 100.0f;
	TotalImpact += Stats.EnergyRegenRate / 10.0f; // Every 10 regen = 1.0 impact

	return TotalImpact;
}

float UProgressionCalculator::GetRarityMultiplier(EShipPartRarity Rarity)
{
	switch (Rarity)
	{
		case EShipPartRarity::Common: return 1.0f;
		case EShipPartRarity::Uncommon: return 3.0f;
		case EShipPartRarity::Rare: return 8.0f;
		case EShipPartRarity::Epic: return 20.0f;
		case EShipPartRarity::Legendary: return 50.0f;
		default: return 1.0f;
	}
}

float UProgressionCalculator::GetCategoryMultiplier(EShipPartCategory Category)
{
	switch (Category)
	{
		case EShipPartCategory::Engine: return EngineMultiplier;
		case EShipPartCategory::Thrusters: return ThrusterMultiplier;
		case EShipPartCategory::Hull: return HullMultiplier;
		case EShipPartCategory::Wings: return WingMultiplier;
		case EShipPartCategory::Cockpit: return CockpitMultiplier;
		case EShipPartCategory::Weapon: return WeaponMultiplier;
		case EShipPartCategory::Shield: return ShieldMultiplier;
		case EShipPartCategory::Utility: return UtilityMultiplier;
		default: return 1.0f;
	}
}

// ============================================================================
// LOADOUT COSTS
// ============================================================================

int32 UProgressionCalculator::GetLoadoutSlotCost(int32 SlotNumber)
{
	switch (SlotNumber)
	{
		case 1: return 0;
		case 2: return 5000;
		case 3: return 10000;
		case 4: return 20000;
		case 5: return 35000;
		case 6: return 50000;
		default: return 0;
	}
}

// ============================================================================
// PROGRESSION VALIDATION
// ============================================================================

bool UProgressionCalculator::ValidateProgression(int32 PlayerLevel, int32 TotalCreditsEarned)
{
	// Check if credits earned is reasonable for player level
	int32 ExpectedCredits = GetExpectedCreditsForLevel(PlayerLevel);

	// Allow 3x variance (some players grind more, some less)
	int32 MinExpected = ExpectedCredits / 3;
	int32 MaxExpected = ExpectedCredits * 3;

	return TotalCreditsEarned >= MinExpected && TotalCreditsEarned <= MaxExpected;
}

int32 UProgressionCalculator::GetExpectedCreditsForLevel(int32 Level)
{
	// Rough estimate: average player earns ~3000 credits per level
	// This is based on:
	// - 500cr per level up bonus
	// - ~5 races to level up (250cr base * 1.5 avg multiplier = 375cr)
	// - 5 races * 375cr = 1875cr from racing
	// - Achievement bonuses, discoveries, etc = ~625cr
	// Total: ~3000cr per level

	return Level * 3000;
}

// ============================================================================
// HELPERS
// ============================================================================

int32 UProgressionCalculator::GetBaseCostForRarity(EShipPartRarity Rarity, bool bIsSkin)
{
	if (bIsSkin)
	{
		switch (Rarity)
		{
			case EShipPartRarity::Common: return CommonSkinCost;
			case EShipPartRarity::Uncommon: return UncommonSkinCost;
			case EShipPartRarity::Rare: return RareSkinCost;
			case EShipPartRarity::Epic: return EpicSkinCost;
			case EShipPartRarity::Legendary: return LegendarySkinCost;
			default: return CommonSkinCost;
		}
	}
	else
	{
		switch (Rarity)
		{
			case EShipPartRarity::Common: return CommonPartCost;
			case EShipPartRarity::Uncommon: return UncommonPartCost;
			case EShipPartRarity::Rare: return RarePartCost;
			case EShipPartRarity::Epic: return EpicPartCost;
			case EShipPartRarity::Legendary: return LegendaryPartCost;
			default: return CommonPartCost;
		}
	}
}
