// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShipCustomizationComponent.h"
#include "CustomizationMenuManager.generated.h"

/**
 * Menu State - Tracks current menu navigation state
 */
UENUM(BlueprintType)
enum class ECustomizationMenuState : uint8
{
	Closed UMETA(DisplayName = "Closed"),
	CategorySelection UMETA(DisplayName = "Category Selection"),
	PartBrowser UMETA(DisplayName = "Part Browser"),
	SkinBrowser UMETA(DisplayName = "Skin Browser"),
	LoadoutManager UMETA(DisplayName = "Loadout Manager"),
	StatsComparison UMETA(DisplayName = "Stats Comparison")
};

/**
 * Filter Settings for Part Browser
 */
USTRUCT(BlueprintType)
struct FPartFilterSettings
{
	GENERATED_BODY()

	// Filter by category
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	EShipPartCategory Category = EShipPartCategory::Engine;

	// Filter by rarity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	TArray<EShipPartRarity> AllowedRarities;

	// Show only unlocked parts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	bool bShowOnlyUnlocked = true;

	// Show only owned parts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	bool bShowOnlyOwned = false;

	// Search by name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	FString SearchText;

	// Minimum level requirement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	int32 MinLevelRequirement = 0;

	// Maximum level requirement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	int32 MaxLevelRequirement = 999;

	// Sort by
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	FString SortBy = "Name"; // "Name", "Rarity", "Level", "Unlock"
};

/**
 * Part Comparison Data - Used for side-by-side comparisons
 */
USTRUCT(BlueprintType)
struct FPartComparisonData
{
	GENERATED_BODY()

	// Current equipped part
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comparison")
	FShipPartData CurrentPart;

	// Part being considered
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comparison")
	FShipPartData ComparedPart;

	// Stat differences (ComparedPart - CurrentPart)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comparison")
	FShipStats StatDifference;

	// Is the compared part better overall?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comparison")
	bool bIsBetter = false;

	// Number of stats improved
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comparison")
	int32 StatsImproved = 0;

	// Number of stats worsened
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comparison")
	int32 StatsWorsened = 0;
};

// ============================================================================
// EVENT DELEGATES
// ============================================================================

// Called when menu state changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMenuStateChanged, ECustomizationMenuState, OldState, ECustomizationMenuState, NewState);

// Called when a part is selected for preview
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartSelected, FShipPartData, PartData);

// Called when a skin is selected for preview
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkinSelected, FShipSkinData, SkinData);

// Called when filter settings change
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFilterChanged);

// Called when menu opens/closes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMenuVisibilityChanged, bool, bIsOpen);

/**
 * Customization Menu Manager
 *
 * AActor-based class that manages the ship customization menu state
 * Handles menu navigation, part filtering, search, and UI state tracking
 *
 * USAGE:
 *   1. Place in level or spawn dynamically
 *   2. Assign ShipCustomizationComponent reference
 *   3. Call OpenMenu() to show menu
 *   4. Bind to events for UI updates
 *   5. Use filter/search functions to populate lists
 *
 * INTEGRATION:
 *   - Works with UShipCustomizationComponent for data access
 *   - Works with UCustomizationUIHelper for formatting
 *   - Designed for UMG widget integration
 *
 * VR NOTES:
 *   - All functions are VR-safe (no mouse-specific code)
 *   - Menu state can be bound to 3D widgets in world space
 *   - Events support spatial UI updates
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom))
class ALEXANDER_API ACustomizationMenuManager : public AActor
{
	GENERATED_BODY()

public:
	ACustomizationMenuManager();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	// ============================================================================
	// CONFIGURATION
	// ============================================================================

	// Reference to the ship's customization component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	UShipCustomizationComponent* CustomizationComponent;

	// Auto-find CustomizationComponent on BeginPlay?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	bool bAutoFindComponent = true;

	// Auto-bind to component events on BeginPlay?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
	bool bAutoBindEvents = true;

	// ============================================================================
	// MENU STATE
	// ============================================================================

	// Current menu state
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu State")
	ECustomizationMenuState CurrentMenuState = ECustomizationMenuState::Closed;

	// Is menu currently open?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu State")
	bool bIsMenuOpen = false;

	// Currently selected category (for part browsing)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu State")
	EShipPartCategory CurrentCategory = EShipPartCategory::Engine;

	// Currently selected part (for preview/comparison)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu State")
	FShipPartData SelectedPart;

	// Currently selected skin (for preview)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu State")
	FShipSkinData SelectedSkin;

	// Is a part currently selected?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu State")
	bool bHasSelectedPart = false;

	// Is a skin currently selected?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Menu State")
	bool bHasSelectedSkin = false;

	// ============================================================================
	// FILTER SETTINGS
	// ============================================================================

	// Current filter settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filters")
	FPartFilterSettings FilterSettings;

	// ============================================================================
	// MENU NAVIGATION
	// ============================================================================

	/**
	 * Open the customization menu
	 * @return True if menu was opened successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	bool OpenMenu();

	/**
	 * Close the customization menu
	 * @return True if menu was closed successfully
	 */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	bool CloseMenu();

	/**
	 * Toggle menu open/closed
	 * @return True if menu is now open, false if closed
	 */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	bool ToggleMenu();

	/**
	 * Navigate to a specific menu state
	 * @param NewState The state to navigate to
	 * @return True if navigation was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	bool NavigateToState(ECustomizationMenuState NewState);

	/**
	 * Go back to previous menu state
	 * @return True if navigation was successful
	 */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	bool NavigateBack();

	/**
	 * Select a category for browsing
	 * @param Category The category to browse
	 * @return True if category was selected
	 */
	UFUNCTION(BlueprintCallable, Category = "Menu")
	bool SelectCategory(EShipPartCategory Category);

	// ============================================================================
	// PART FILTERING & SEARCH
	// ============================================================================

	/**
	 * Get filtered parts based on current filter settings
	 * @return Array of parts matching filter criteria
	 */
	UFUNCTION(BlueprintCallable, Category = "Filtering")
	TArray<FShipPartData> GetFilteredParts();

	/**
	 * Get filtered skins based on search text
	 * @param SearchText Optional search filter
	 * @return Array of skins matching criteria
	 */
	UFUNCTION(BlueprintCallable, Category = "Filtering")
	TArray<FShipSkinData> GetFilteredSkins(const FString& SearchText = "");

	/**
	 * Search parts by name
	 * @param SearchText The text to search for
	 * @return Array of parts matching search
	 */
	UFUNCTION(BlueprintCallable, Category = "Filtering")
	TArray<FShipPartData> SearchPartsByName(const FString& SearchText);

	/**
	 * Filter parts by rarity
	 * @param Parts Parts to filter
	 * @param AllowedRarities Rarities to include
	 * @return Filtered array
	 */
	UFUNCTION(BlueprintCallable, Category = "Filtering")
	TArray<FShipPartData> FilterPartsByRarity(const TArray<FShipPartData>& Parts, const TArray<EShipPartRarity>& AllowedRarities);

	/**
	 * Filter parts by level requirement
	 * @param Parts Parts to filter
	 * @param MinLevel Minimum level requirement
	 * @param MaxLevel Maximum level requirement
	 * @return Filtered array
	 */
	UFUNCTION(BlueprintCallable, Category = "Filtering")
	TArray<FShipPartData> FilterPartsByLevel(const TArray<FShipPartData>& Parts, int32 MinLevel, int32 MaxLevel);

	/**
	 * Sort parts by criteria
	 * @param Parts Parts to sort
	 * @param SortBy Sort criteria ("Name", "Rarity", "Level", "Unlock")
	 * @return Sorted array
	 */
	UFUNCTION(BlueprintCallable, Category = "Filtering")
	TArray<FShipPartData> SortParts(const TArray<FShipPartData>& Parts, const FString& SortBy);

	/**
	 * Update filter settings and refresh filtered parts
	 * @param NewSettings The new filter settings
	 */
	UFUNCTION(BlueprintCallable, Category = "Filtering")
	void UpdateFilterSettings(const FPartFilterSettings& NewSettings);

	/**
	 * Reset filters to default
	 */
	UFUNCTION(BlueprintCallable, Category = "Filtering")
	void ResetFilters();

	// ============================================================================
	// PART SELECTION & COMPARISON
	// ============================================================================

	/**
	 * Select a part for preview/comparison
	 * @param PartData The part to select
	 */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void SelectPart(const FShipPartData& PartData);

	/**
	 * Select a skin for preview
	 * @param SkinData The skin to select
	 */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void SelectSkin(const FShipSkinData& SkinData);

	/**
	 * Clear part selection
	 */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void ClearPartSelection();

	/**
	 * Clear skin selection
	 */
	UFUNCTION(BlueprintCallable, Category = "Selection")
	void ClearSkinSelection();

	/**
	 * Compare selected part with currently equipped part
	 * @return Comparison data with stat differences
	 */
	UFUNCTION(BlueprintCallable, Category = "Comparison")
	FPartComparisonData CompareSelectedPart();

	/**
	 * Calculate stat differences between two parts
	 * @param PartA First part
	 * @param PartB Second part
	 * @return Stat difference (PartB - PartA)
	 */
	UFUNCTION(BlueprintCallable, Category = "Comparison")
	FShipStats CalculateStatDifference(const FShipPartData& PartA, const FShipPartData& PartB);

	/**
	 * Check if a part is an upgrade (better stats overall)
	 * @param CurrentPart Currently equipped part
	 * @param NewPart Part to compare
	 * @param OutImprovedStats Number of improved stats (output)
	 * @param OutWorsenedStats Number of worsened stats (output)
	 * @return True if new part is better overall
	 */
	UFUNCTION(BlueprintCallable, Category = "Comparison")
	bool IsPartUpgrade(const FShipPartData& CurrentPart, const FShipPartData& NewPart, int32& OutImprovedStats, int32& OutWorsenedStats);

	// ============================================================================
	// PURCHASE VALIDATION
	// ============================================================================

	/**
	 * Check if player can afford to purchase a part
	 * @param PartData The part to check
	 * @return True if player has enough credits
	 */
	UFUNCTION(BlueprintCallable, Category = "Purchase")
	bool CanAffordPart(const FShipPartData& PartData);

	/**
	 * Check if player can afford to purchase a skin
	 * @param SkinData The skin to check
	 * @return True if player has enough credits
	 */
	UFUNCTION(BlueprintCallable, Category = "Purchase")
	bool CanAffordSkin(const FShipSkinData& SkinData);

	/**
	 * Check if player meets level requirement for a part
	 * @param PartData The part to check
	 * @return True if player level is high enough
	 */
	UFUNCTION(BlueprintCallable, Category = "Purchase")
	bool MeetsLevelRequirement(const FShipPartData& PartData);

	/**
	 * Get affordability status text for a part
	 * @param PartData The part to check
	 * @return Status text ("Can Purchase", "Insufficient Credits", "Level Too Low", etc.)
	 */
	UFUNCTION(BlueprintCallable, Category = "Purchase")
	FText GetAffordabilityStatus(const FShipPartData& PartData);

	// ============================================================================
	// STATS CALCULATION
	// ============================================================================

	/**
	 * Calculate total stats with selected part equipped (preview)
	 * @param PartData The part to preview
	 * @return Simulated total stats
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	FShipStats CalculateStatsWithPart(const FShipPartData& PartData);

	/**
	 * Get current total stats
	 * @return Current equipped stats
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	FShipStats GetCurrentTotalStats();

	// ============================================================================
	// EVENT BINDINGS
	// ============================================================================

	/**
	 * Bind to ShipCustomizationComponent events
	 * Call this after setting CustomizationComponent
	 */
	UFUNCTION(BlueprintCallable, Category = "Events")
	void BindToComponentEvents();

	/**
	 * Unbind from ShipCustomizationComponent events
	 */
	UFUNCTION(BlueprintCallable, Category = "Events")
	void UnbindFromComponentEvents();

	// ============================================================================
	// EVENT HANDLERS (called by CustomizationComponent)
	// ============================================================================

protected:
	/**
	 * Called when a part is equipped (from component event)
	 * @param Category The category of the equipped part
	 * @param PartID The ID of the equipped part
	 */
	UFUNCTION()
	void OnPartEquippedEvent(EShipPartCategory Category, FName PartID);

	/**
	 * Called when a skin is equipped (from component event)
	 * @param SkinID The ID of the equipped skin
	 */
	UFUNCTION()
	void OnSkinEquippedEvent(FName SkinID);

	/**
	 * Called when a part is unlocked (from component event)
	 * @param PartID The ID of the unlocked part
	 */
	UFUNCTION()
	void OnPartUnlockedEvent(FName PartID);

	/**
	 * Called when player levels up (from component event)
	 * @param OldLevel The previous level
	 * @param NewLevel The new level
	 */
	UFUNCTION()
	void OnLevelUpEvent(int32 OldLevel, int32 NewLevel);

	// ============================================================================
	// MENU EVENTS (for UI binding)
	// ============================================================================

public:
	// Called when menu state changes
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMenuStateChanged OnMenuStateChanged;

	// Called when a part is selected
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPartSelected OnPartSelected;

	// Called when a skin is selected
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSkinSelected OnSkinSelected;

	// Called when filter settings change
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnFilterChanged OnFilterChanged;

	// Called when menu opens or closes
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMenuVisibilityChanged OnMenuVisibilityChanged;

	// ============================================================================
	// HELPERS
	// ============================================================================

protected:
	/**
	 * Find CustomizationComponent on player pawn
	 * @return Found component or nullptr
	 */
	UShipCustomizationComponent* FindCustomizationComponent();

	/**
	 * Validate that CustomizationComponent is valid
	 * @return True if valid
	 */
	bool ValidateComponent();

	/**
	 * Previous menu state (for back navigation)
	 */
	ECustomizationMenuState PreviousMenuState = ECustomizationMenuState::Closed;

	/**
	 * Are events currently bound?
	 */
	bool bEventsBound = false;
};
