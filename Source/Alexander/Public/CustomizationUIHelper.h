// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ShipCustomizationComponent.h"
#include "CustomizationUIHelper.generated.h"

/**
 * Customization UI Helper
 *
 * Blueprint Function Library with static helper functions for UI formatting
 * Provides convenient formatting and display helpers for ship customization UI
 *
 * FEATURES:
 *   - Stat formatting (float to "1.5x" string)
 *   - Credit formatting with separators ("1,250 CR")
 *   - XP/Level display formatting
 *   - Stat difference calculations for comparisons
 *   - Color coding for stat changes (green/red)
 *   - Purchase validation helpers
 *   - Icon path utilities
 *
 * INTEGRATION:
 *   - Use with UMG widgets for display formatting
 *   - Works alongside ACustomizationMenuManager for data
 *   - References UShipCustomizationBlueprintLibrary for rarity colors
 *
 * VR NOTES:
 *   - All functions are pure and VR-safe
 *   - Color output works with 3D widget text
 *   - Large text formatting for readability in VR
 */
UCLASS()
class ALEXANDER_API UCustomizationUIHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ============================================================================
	// STAT FORMATTING
	// ============================================================================

	/**
	 * Format a stat multiplier for display
	 * Example: 1.5 -> "1.5x", 0.8 -> "0.8x"
	 * @param StatValue The stat multiplier value
	 * @param DecimalPlaces Number of decimal places (default 1)
	 * @return Formatted text with multiplier suffix
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Formatting", meta = (DisplayName = "Format Stat Multiplier"))
	static FText FormatStatMultiplier(float StatValue, int32 DecimalPlaces = 1);

	/**
	 * Format a raw stat value for display
	 * Example: 100.0 -> "100", 1250.5 -> "1,250.5"
	 * @param StatValue The stat value
	 * @param DecimalPlaces Number of decimal places (default 0)
	 * @return Formatted text with thousand separators
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Formatting", meta = (DisplayName = "Format Stat Value"))
	static FText FormatStatValue(float StatValue, int32 DecimalPlaces = 0);

	/**
	 * Format a stat difference with +/- prefix and color coding
	 * Example: 0.5 -> "+0.5x" (green), -0.3 -> "-0.3x" (red)
	 * @param StatDifference The difference value
	 * @param bIsMultiplier Is this a multiplier stat? (default true)
	 * @param DecimalPlaces Number of decimal places (default 1)
	 * @return Formatted text with +/- prefix
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Formatting", meta = (DisplayName = "Format Stat Difference"))
	static FText FormatStatDifference(float StatDifference, bool bIsMultiplier = true, int32 DecimalPlaces = 1);

	/**
	 * Get color for a stat difference (green for positive, red for negative)
	 * @param StatDifference The difference value
	 * @param bLowerIsBetter For stats like Mass where lower is better (default false)
	 * @return Color (green, red, or white for zero)
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Formatting", meta = (DisplayName = "Get Stat Difference Color"))
	static FLinearColor GetStatDifferenceColor(float StatDifference, bool bLowerIsBetter = false);

	/**
	 * Format all ship stats as multi-line text
	 * @param Stats The stats to format
	 * @param bShowZeroValues Show stats with zero values? (default false)
	 * @return Formatted multi-line text
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Formatting", meta = (DisplayName = "Format Ship Stats"))
	static FText FormatShipStats(const FShipStats& Stats, bool bShowZeroValues = false);

	/**
	 * Format ship stats as a comparison (with differences highlighted)
	 * @param CurrentStats Current equipped stats
	 * @param NewStats New stats to compare
	 * @return Formatted comparison text with color-coded differences
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Formatting", meta = (DisplayName = "Format Stats Comparison"))
	static FText FormatStatsComparison(const FShipStats& CurrentStats, const FShipStats& NewStats);

	// ============================================================================
	// CURRENCY & PROGRESSION FORMATTING
	// ============================================================================

	/**
	 * Format credits with thousand separators and "CR" suffix
	 * Example: 1250 -> "1,250 CR"
	 * @param Credits The credit amount
	 * @return Formatted text with separators and suffix
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Formatting", meta = (DisplayName = "Format Credits"))
	static FText FormatCredits(int32 Credits);

	/**
	 * Format XP amount with thousand separators
	 * Example: 5000 -> "5,000 XP"
	 * @param XP The XP amount
	 * @return Formatted text with separators and suffix
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Formatting", meta = (DisplayName = "Format XP"))
	static FText FormatXP(int32 XP);

	/**
	 * Format level display
	 * Example: 15 -> "Level 15", 1 -> "Level 1"
	 * @param Level The player level
	 * @return Formatted level text
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Formatting", meta = (DisplayName = "Format Level"))
	static FText FormatLevel(int32 Level);

	/**
	 * Format XP progress towards next level
	 * Example: 500/1000 -> "500 / 1,000 XP (50%)"
	 * @param CurrentXP Current XP in level
	 * @param RequiredXP XP required for next level
	 * @return Formatted progress text
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Formatting", meta = (DisplayName = "Format XP Progress"))
	static FText FormatXPProgress(int32 CurrentXP, int32 RequiredXP);

	/**
	 * Calculate XP progress percentage
	 * @param CurrentXP Current XP in level
	 * @param RequiredXP XP required for next level
	 * @return Percentage (0.0 to 1.0)
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Formatting", meta = (DisplayName = "Get XP Progress Percentage"))
	static float GetXPProgressPercentage(int32 CurrentXP, int32 RequiredXP);

	// ============================================================================
	// RARITY & COLOR HELPERS
	// ============================================================================

	/**
	 * Get display color for part/skin rarity
	 * Delegates to ShipCustomizationBlueprintLibrary::GetRarityColor
	 * @param Rarity The rarity level
	 * @return Color for UI display
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Colors", meta = (DisplayName = "Get Rarity Color"))
	static FLinearColor GetRarityColor(EShipPartRarity Rarity);

	/**
	 * Get rarity display name
	 * @param Rarity The rarity level
	 * @return Display name (Common, Rare, etc.)
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Colors", meta = (DisplayName = "Get Rarity Name"))
	static FText GetRarityName(EShipPartRarity Rarity);

	/**
	 * Get category display name
	 * @param Category The part category
	 * @return Display name (Engine, Thrusters, etc.)
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Display", meta = (DisplayName = "Get Category Name"))
	static FText GetCategoryName(EShipPartCategory Category);

	/**
	 * Get category icon name (for texture lookup)
	 * @param Category The part category
	 * @return Icon name string ("Engine", "Thrusters", etc.)
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Display", meta = (DisplayName = "Get Category Icon Name"))
	static FString GetCategoryIconName(EShipPartCategory Category);

	// ============================================================================
	// PURCHASE VALIDATION
	// ============================================================================

	/**
	 * Check if player can afford a part
	 * @param PartData The part to check
	 * @param CurrentCredits Player's current credits
	 * @return True if player has enough credits
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Validation", meta = (DisplayName = "Can Afford Part"))
	static bool CanAffordPart(const FShipPartData& PartData, int32 CurrentCredits);

	/**
	 * Check if player can afford a skin
	 * @param SkinData The skin to check
	 * @param CurrentCredits Player's current credits
	 * @return True if player has enough credits
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Validation", meta = (DisplayName = "Can Afford Skin"))
	static bool CanAffordSkin(const FShipSkinData& SkinData, int32 CurrentCredits);

	/**
	 * Check if player meets level requirement
	 * @param PartData The part to check
	 * @param PlayerLevel Player's current level
	 * @return True if level requirement is met
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Validation", meta = (DisplayName = "Meets Level Requirement"))
	static bool MeetsLevelRequirement(const FShipPartData& PartData, int32 PlayerLevel);

	/**
	 * Get purchase status text
	 * @param PartData The part to check
	 * @param CurrentCredits Player's current credits
	 * @param PlayerLevel Player's current level
	 * @param bIsUnlocked Is the part already unlocked?
	 * @return Status text ("Can Purchase", "Insufficient Credits", etc.)
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Validation", meta = (DisplayName = "Get Purchase Status"))
	static FText GetPurchaseStatus(const FShipPartData& PartData, int32 CurrentCredits, int32 PlayerLevel, bool bIsUnlocked);

	/**
	 * Get purchase status color
	 * @param PartData The part to check
	 * @param CurrentCredits Player's current credits
	 * @param PlayerLevel Player's current level
	 * @param bIsUnlocked Is the part already unlocked?
	 * @return Color for status text (green = can buy, red = cannot, white = owned)
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Validation", meta = (DisplayName = "Get Purchase Status Color"))
	static FLinearColor GetPurchaseStatusColor(const FShipPartData& PartData, int32 CurrentCredits, int32 PlayerLevel, bool bIsUnlocked);

	// ============================================================================
	// ICON & ASSET PATH HELPERS
	// ============================================================================

	/**
	 * Get icon texture path from soft object pointer
	 * @param IconSoftPtr The soft texture pointer
	 * @return Texture path string, or empty if null
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Assets", meta = (DisplayName = "Get Icon Path"))
	static FString GetIconPath(const TSoftObjectPtr<UTexture2D>& IconSoftPtr);

	/**
	 * Load icon texture from soft pointer (synchronous)
	 * WARNING: Use sparingly - prefer async loading in production
	 * @param IconSoftPtr The soft texture pointer
	 * @return Loaded texture or nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Assets", meta = (DisplayName = "Load Icon Texture"))
	static UTexture2D* LoadIconTexture(const TSoftObjectPtr<UTexture2D>& IconSoftPtr);

	/**
	 * Check if icon is valid
	 * @param IconSoftPtr The soft texture pointer
	 * @return True if pointer is not null
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Assets", meta = (DisplayName = "Is Icon Valid"))
	static bool IsIconValid(const TSoftObjectPtr<UTexture2D>& IconSoftPtr);

	// ============================================================================
	// COMPARISON HELPERS
	// ============================================================================

	/**
	 * Calculate stat difference between two parts
	 * @param CurrentPart Currently equipped part
	 * @param NewPart Part to compare
	 * @return Stat difference (NewPart - CurrentPart)
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Comparison", meta = (DisplayName = "Calculate Part Stat Difference"))
	static FShipStats CalculatePartStatDifference(const FShipPartData& CurrentPart, const FShipPartData& NewPart);

	/**
	 * Count how many stats are improved/worsened
	 * @param StatDifference The stat difference
	 * @param OutImproved Number of improved stats (output)
	 * @param OutWorsened Number of worsened stats (output)
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Comparison", meta = (DisplayName = "Count Stat Changes"))
	static void CountStatChanges(const FShipStats& StatDifference, int32& OutImproved, int32& OutWorsened);

	/**
	 * Get overall upgrade rating (0.0 = worse, 0.5 = mixed, 1.0 = better)
	 * @param StatDifference The stat difference
	 * @return Rating from 0.0 to 1.0
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Comparison", meta = (DisplayName = "Get Upgrade Rating"))
	static float GetUpgradeRating(const FShipStats& StatDifference);

	/**
	 * Get upgrade summary text
	 * @param ImprovedStats Number of improved stats
	 * @param WorsenedStats Number of worsened stats
	 * @return Summary text ("Upgrade", "Downgrade", "Mixed", "No Change")
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Comparison", meta = (DisplayName = "Get Upgrade Summary"))
	static FText GetUpgradeSummary(int32 ImprovedStats, int32 WorsenedStats);

	// ============================================================================
	// UTILITY FUNCTIONS
	// ============================================================================

	/**
	 * Format number with thousand separators
	 * Example: 1250 -> "1,250"
	 * @param Number The number to format
	 * @return Formatted text with separators
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Formatting", meta = (DisplayName = "Format Number"))
	static FText FormatNumber(int32 Number);

	/**
	 * Format percentage
	 * Example: 0.75 -> "75%"
	 * @param Percentage The percentage (0.0 to 1.0)
	 * @param DecimalPlaces Number of decimal places (default 0)
	 * @return Formatted percentage text
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Formatting", meta = (DisplayName = "Format Percentage"))
	static FText FormatPercentage(float Percentage, int32 DecimalPlaces = 0);

	/**
	 * Clamp percentage to 0-100 range
	 * @param Percentage The percentage (0.0 to 1.0)
	 * @return Clamped percentage
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Utility", meta = (DisplayName = "Clamp Percentage"))
	static float ClampPercentage(float Percentage);

	/**
	 * Get abbreviated stat name
	 * Example: "ThrustPower" -> "Thrust"
	 * @param StatName The full stat name
	 * @return Abbreviated name
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Display", meta = (DisplayName = "Get Abbreviated Stat Name"))
	static FText GetAbbreviatedStatName(const FString& StatName);

	// ============================================================================
	// COLOR CONSTANTS
	// ============================================================================

	/**
	 * Get positive stat color (green)
	 * @return Green color for positive changes
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Colors", meta = (DisplayName = "Get Positive Color"))
	static FLinearColor GetPositiveColor();

	/**
	 * Get negative stat color (red)
	 * @return Red color for negative changes
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Colors", meta = (DisplayName = "Get Negative Color"))
	static FLinearColor GetNegativeColor();

	/**
	 * Get neutral stat color (white/gray)
	 * @return Neutral color for no change
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Colors", meta = (DisplayName = "Get Neutral Color"))
	static FLinearColor GetNeutralColor();

	/**
	 * Get owned/unlocked color (gold)
	 * @return Gold color for owned items
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Colors", meta = (DisplayName = "Get Owned Color"))
	static FLinearColor GetOwnedColor();

	/**
	 * Get locked color (dark gray)
	 * @return Dark gray for locked items
	 */
	UFUNCTION(BlueprintPure, Category = "UI Helpers|Colors", meta = (DisplayName = "Get Locked Color"))
	static FLinearColor GetLockedColor();
};
