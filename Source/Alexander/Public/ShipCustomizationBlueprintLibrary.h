// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ShipCustomizationComponent.h"
#include "ShipPhysicsConfig.h"
#include "ShipCustomizationBlueprintLibrary.generated.h"

/**
 * Ship Customization Blueprint Function Library
 * Provides convenient Blueprint-friendly functions for ship customization
 * Designer-friendly interface for the ship customization system
 */
UCLASS()
class ALEXANDER_API UShipCustomizationBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ============================================================================
	// COMPONENT ACCESS
	// ============================================================================

	/**
	 * Get the ShipCustomizationComponent from any actor
	 * @param Ship The actor to search for the component
	 * @return The ShipCustomizationComponent if found, nullptr otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Get Ship Customization Component"))
	static UShipCustomizationComponent* GetShipCustomizationComponent(AActor* Ship);

	// ============================================================================
	// PHYSICS CONFIGURATION
	// ============================================================================

	/**
	 * Apply a physics configuration preset to a ship
	 * @param Component The ShipCustomizationComponent
	 * @param Config The physics configuration to apply
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Apply Physics Config"))
	static bool ApplyPhysicsConfig(UShipCustomizationComponent* Component, UShipPhysicsConfig* Config);

	// ============================================================================
	// PART MANAGEMENT
	// ============================================================================

	/**
	 * Equip a ship part by ID
	 * @param Component The ShipCustomizationComponent
	 * @param Category The part category (Engine, Thrusters, Hull, etc.)
	 * @param PartID The unique ID of the part to equip
	 * @return True if the part was successfully equipped
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Equip Part by ID"))
	static bool EquipPartByID(UShipCustomizationComponent* Component, EShipPartCategory Category, FName PartID);

	/**
	 * Unequip a ship part from a category
	 * @param Component The ShipCustomizationComponent
	 * @param Category The part category to unequip
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Unequip Part"))
	static bool UnequipPart(UShipCustomizationComponent* Component, EShipPartCategory Category);

	/**
	 * Get the currently equipped part in a category
	 * @param Component The ShipCustomizationComponent
	 * @param Category The part category
	 * @param bFound Whether the part was found (output)
	 * @return The part data
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Get Equipped Part"))
	static FShipPartData GetEquippedPart(UShipCustomizationComponent* Component, EShipPartCategory Category, bool& bFound);

	// ============================================================================
	// SKIN MANAGEMENT
	// ============================================================================

	/**
	 * Equip a ship skin by ID
	 * @param Component The ShipCustomizationComponent
	 * @param SkinID The unique ID of the skin to equip
	 * @return True if the skin was successfully equipped
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Equip Skin by ID"))
	static bool EquipSkinByID(UShipCustomizationComponent* Component, FName SkinID);

	/**
	 * Get the currently equipped skin
	 * @param Component The ShipCustomizationComponent
	 * @param bFound Whether the skin was found (output)
	 * @return The skin data
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Get Equipped Skin"))
	static FShipSkinData GetEquippedSkin(UShipCustomizationComponent* Component, bool& bFound);

	// ============================================================================
	// STATS
	// ============================================================================

	/**
	 * Get the current total stats from all equipped parts
	 * @param Component The ShipCustomizationComponent
	 * @return The total ship stats
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Get Current Stats"))
	static FShipStats GetCurrentStats(UShipCustomizationComponent* Component);

	/**
	 * Calculate and apply stats to the FlightController
	 * @param Component The ShipCustomizationComponent
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Apply Stats to Flight Controller"))
	static bool ApplyStatsToFlightController(UShipCustomizationComponent* Component);

	// ============================================================================
	// LOADOUT MANAGEMENT
	// ============================================================================

	/**
	 * Save the current loadout with a custom name
	 * @param Component The ShipCustomizationComponent
	 * @param LoadoutName The name for this loadout
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Save Loadout"))
	static bool SaveLoadout(UShipCustomizationComponent* Component, const FText& LoadoutName);

	/**
	 * Load a previously saved loadout by index
	 * @param Component The ShipCustomizationComponent
	 * @param LoadoutIndex The index of the loadout to load
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Load Loadout"))
	static bool LoadLoadout(UShipCustomizationComponent* Component, int32 LoadoutIndex);

	/**
	 * Delete a saved loadout
	 * @param Component The ShipCustomizationComponent
	 * @param LoadoutIndex The index of the loadout to delete
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Delete Loadout"))
	static bool DeleteLoadout(UShipCustomizationComponent* Component, int32 LoadoutIndex);

	/**
	 * Get all saved loadouts
	 * @param Component The ShipCustomizationComponent
	 * @return Array of saved loadouts
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Get Saved Loadouts"))
	static TArray<FShipLoadout> GetSavedLoadouts(UShipCustomizationComponent* Component);

	// ============================================================================
	// PROGRESSION
	// ============================================================================

	/**
	 * Unlock a ship part
	 * @param Component The ShipCustomizationComponent
	 * @param PartID The ID of the part to unlock
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Unlock Part"))
	static bool UnlockPart(UShipCustomizationComponent* Component, FName PartID);

	/**
	 * Unlock a ship skin
	 * @param Component The ShipCustomizationComponent
	 * @param SkinID The ID of the skin to unlock
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Unlock Skin"))
	static bool UnlockSkin(UShipCustomizationComponent* Component, FName SkinID);

	/**
	 * Check if a part is unlocked
	 * @param Component The ShipCustomizationComponent
	 * @param PartID The ID of the part to check
	 * @return True if unlocked
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Is Part Unlocked"))
	static bool IsPartUnlocked(UShipCustomizationComponent* Component, FName PartID);

	/**
	 * Check if a skin is unlocked
	 * @param Component The ShipCustomizationComponent
	 * @param SkinID The ID of the skin to check
	 * @return True if unlocked
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Is Skin Unlocked"))
	static bool IsSkinUnlocked(UShipCustomizationComponent* Component, FName SkinID);

	/**
	 * Add XP to the player
	 * @param Component The ShipCustomizationComponent
	 * @param Amount The amount of XP to add
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Add XP"))
	static void AddXP(UShipCustomizationComponent* Component, int32 Amount);

	/**
	 * Add credits to the player
	 * @param Component The ShipCustomizationComponent
	 * @param Amount The amount of credits to add
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Add Credits"))
	static void AddCredits(UShipCustomizationComponent* Component, int32 Amount);

	/**
	 * Get the player's current level
	 * @param Component The ShipCustomizationComponent
	 * @return The player level
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Get Player Level"))
	static int32 GetPlayerLevel(UShipCustomizationComponent* Component);

	/**
	 * Get the player's current credits
	 * @param Component The ShipCustomizationComponent
	 * @return The credit amount
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Get Credits"))
	static int32 GetCredits(UShipCustomizationComponent* Component);

	// ============================================================================
	// DATA QUERIES
	// ============================================================================

	/**
	 * Get all parts in a specific category
	 * @param Component The ShipCustomizationComponent
	 * @param Category The part category to query
	 * @return Array of all parts in the category
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Get Parts in Category"))
	static TArray<FShipPartData> GetPartsInCategory(UShipCustomizationComponent* Component, EShipPartCategory Category);

	/**
	 * Get all unlocked parts in a specific category
	 * @param Component The ShipCustomizationComponent
	 * @param Category The part category to query
	 * @return Array of unlocked parts in the category
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Get Unlocked Parts in Category"))
	static TArray<FShipPartData> GetUnlockedPartsInCategory(UShipCustomizationComponent* Component, EShipPartCategory Category);

	/**
	 * Get all available skins
	 * @param Component The ShipCustomizationComponent
	 * @return Array of all skins
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Get All Skins"))
	static TArray<FShipSkinData> GetAllSkins(UShipCustomizationComponent* Component);

	/**
	 * Get all unlocked skins
	 * @param Component The ShipCustomizationComponent
	 * @return Array of unlocked skins
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Get Unlocked Skins"))
	static TArray<FShipSkinData> GetUnlockedSkins(UShipCustomizationComponent* Component);

	/**
	 * Get part data by ID
	 * @param Component The ShipCustomizationComponent
	 * @param PartID The ID of the part
	 * @param bFound Whether the part was found (output)
	 * @return The part data
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Get Part Data"))
	static FShipPartData GetPartData(UShipCustomizationComponent* Component, FName PartID, bool& bFound);

	/**
	 * Get skin data by ID
	 * @param Component The ShipCustomizationComponent
	 * @param SkinID The ID of the skin
	 * @param bFound Whether the skin was found (output)
	 * @return The skin data
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Get Skin Data"))
	static FShipSkinData GetSkinData(UShipCustomizationComponent* Component, FName SkinID, bool& bFound);

	// ============================================================================
	// SAVE/LOAD
	// ============================================================================

	/**
	 * Save customization data to SaveGame
	 * @param Component The ShipCustomizationComponent
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Save Customization Data"))
	static bool SaveCustomizationData(UShipCustomizationComponent* Component);

	/**
	 * Load customization data from SaveGame
	 * @param Component The ShipCustomizationComponent
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Load Customization Data"))
	static bool LoadCustomizationData(UShipCustomizationComponent* Component);

	// ============================================================================
	// VISUAL UPDATES
	// ============================================================================

	/**
	 * Update ship visual appearance based on equipped parts
	 * @param Component The ShipCustomizationComponent
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Update Ship Visuals"))
	static bool UpdateShipVisuals(UShipCustomizationComponent* Component);

	/**
	 * Apply the equipped skin material to the ship mesh
	 * @param Component The ShipCustomizationComponent
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Apply Skin Material"))
	static bool ApplySkinMaterial(UShipCustomizationComponent* Component);

	// ============================================================================
	// UTILITY FUNCTIONS
	// ============================================================================

	/**
	 * Format ship stats for display (converts to user-friendly strings)
	 * @param Stats The stats to format
	 * @return Formatted text representation
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Format Stats for Display"))
	static FText FormatStatsForDisplay(const FShipStats& Stats);

	/**
	 * Get rarity color for UI
	 * @param Rarity The part/skin rarity
	 * @return Color appropriate for the rarity level
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Get Rarity Color"))
	static FLinearColor GetRarityColor(EShipPartRarity Rarity);

	/**
	 * Compare two ship stats (for upgrade comparisons)
	 * @param StatsA First stats
	 * @param StatsB Second stats
	 * @return Difference (StatsB - StatsA)
	 */
	UFUNCTION(BlueprintCallable, Category = "Ship Customization", meta = (DisplayName = "Compare Ship Stats"))
	static FShipStats CompareShipStats(const FShipStats& StatsA, const FShipStats& StatsB);
};
