// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomizationMenuManager.h"
#include "ShipCustomizationComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

ACustomizationMenuManager::ACustomizationMenuManager()
{
	PrimaryActorTick.bCanEverTick = false;

	// Initialize default filter settings
	FilterSettings.Category = EShipPartCategory::Engine;
	FilterSettings.bShowOnlyUnlocked = true;
	FilterSettings.bShowOnlyOwned = false;
	FilterSettings.SearchText = "";
	FilterSettings.MinLevelRequirement = 0;
	FilterSettings.MaxLevelRequirement = 999;
	FilterSettings.SortBy = "Name";

	// Default state
	CurrentMenuState = ECustomizationMenuState::Closed;
	bIsMenuOpen = false;
	bHasSelectedPart = false;
	bHasSelectedSkin = false;
	bEventsBound = false;
}

void ACustomizationMenuManager::BeginPlay()
{
	Super::BeginPlay();

	// Auto-find component if enabled
	if (bAutoFindComponent && !CustomizationComponent)
	{
		CustomizationComponent = FindCustomizationComponent();
	}

	// Auto-bind events if enabled
	if (bAutoBindEvents && CustomizationComponent)
	{
		BindToComponentEvents();
	}

	UE_LOG(LogTemp, Log, TEXT("CustomizationMenuManager initialized (Component: %s)"),
		CustomizationComponent ? TEXT("Found") : TEXT("Not Found"));
}

void ACustomizationMenuManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// ============================================================================
// MENU NAVIGATION
// ============================================================================

bool ACustomizationMenuManager::OpenMenu()
{
	if (bIsMenuOpen)
	{
		return false; // Already open
	}

	if (!ValidateComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot open menu: CustomizationComponent not found"));
		return false;
	}

	bIsMenuOpen = true;
	ECustomizationMenuState OldState = CurrentMenuState;
	CurrentMenuState = ECustomizationMenuState::CategorySelection;

	// Fire events
	OnMenuVisibilityChanged.Broadcast(true);
	OnMenuStateChanged.Broadcast(OldState, CurrentMenuState);

	UE_LOG(LogTemp, Log, TEXT("Customization menu opened"));

	return true;
}

bool ACustomizationMenuManager::CloseMenu()
{
	if (!bIsMenuOpen)
	{
		return false; // Already closed
	}

	bIsMenuOpen = false;
	ECustomizationMenuState OldState = CurrentMenuState;
	CurrentMenuState = ECustomizationMenuState::Closed;

	// Clear selections
	ClearPartSelection();
	ClearSkinSelection();

	// Fire events
	OnMenuVisibilityChanged.Broadcast(false);
	OnMenuStateChanged.Broadcast(OldState, CurrentMenuState);

	UE_LOG(LogTemp, Log, TEXT("Customization menu closed"));

	return true;
}

bool ACustomizationMenuManager::ToggleMenu()
{
	if (bIsMenuOpen)
	{
		return !CloseMenu();
	}
	else
	{
		return OpenMenu();
	}
}

bool ACustomizationMenuManager::NavigateToState(ECustomizationMenuState NewState)
{
	if (!bIsMenuOpen && NewState != ECustomizationMenuState::Closed)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot navigate: menu is closed"));
		return false;
	}

	if (CurrentMenuState == NewState)
	{
		return false; // Already in this state
	}

	// Save previous state for back navigation
	PreviousMenuState = CurrentMenuState;

	// Update state
	ECustomizationMenuState OldState = CurrentMenuState;
	CurrentMenuState = NewState;

	// Fire event
	OnMenuStateChanged.Broadcast(OldState, NewState);

	UE_LOG(LogTemp, Log, TEXT("Menu navigated: %d -> %d"), (int32)OldState, (int32)NewState);

	return true;
}

bool ACustomizationMenuManager::NavigateBack()
{
	if (!bIsMenuOpen)
	{
		return false;
	}

	// Navigate to previous state
	return NavigateToState(PreviousMenuState);
}

bool ACustomizationMenuManager::SelectCategory(EShipPartCategory Category)
{
	CurrentCategory = Category;
	FilterSettings.Category = Category;

	// Navigate to part browser
	NavigateToState(ECustomizationMenuState::PartBrowser);

	// Clear current selection
	ClearPartSelection();

	// Fire filter changed event
	OnFilterChanged.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("Category selected: %d"), (int32)Category);

	return true;
}

// ============================================================================
// PART FILTERING & SEARCH
// ============================================================================

TArray<FShipPartData> ACustomizationMenuManager::GetFilteredParts()
{
	if (!ValidateComponent())
	{
		return TArray<FShipPartData>();
	}

	// Start with all parts in category
	TArray<FShipPartData> Parts = CustomizationComponent->GetPartsInCategory(FilterSettings.Category);

	// Apply unlocked filter
	if (FilterSettings.bShowOnlyUnlocked)
	{
		Parts = Parts.FilterByPredicate([this](const FShipPartData& Part)
		{
			return CustomizationComponent->IsPartUnlocked(Part.PartID);
		});
	}

	// Apply search filter
	if (!FilterSettings.SearchText.IsEmpty())
	{
		Parts = Parts.FilterByPredicate([this](const FShipPartData& Part)
		{
			return Part.DisplayName.ToString().Contains(FilterSettings.SearchText, ESearchCase::IgnoreCase) ||
			       Part.Description.ToString().Contains(FilterSettings.SearchText, ESearchCase::IgnoreCase);
		});
	}

	// Apply rarity filter
	if (FilterSettings.AllowedRarities.Num() > 0)
	{
		Parts = FilterPartsByRarity(Parts, FilterSettings.AllowedRarities);
	}

	// Apply level filter
	Parts = FilterPartsByLevel(Parts, FilterSettings.MinLevelRequirement, FilterSettings.MaxLevelRequirement);

	// Apply sorting
	Parts = SortParts(Parts, FilterSettings.SortBy);

	return Parts;
}

TArray<FShipSkinData> ACustomizationMenuManager::GetFilteredSkins(const FString& SearchText)
{
	if (!ValidateComponent())
	{
		return TArray<FShipSkinData>();
	}

	// Get all skins
	TArray<FShipSkinData> Skins = CustomizationComponent->GetAllSkins();

	// Apply search filter
	if (!SearchText.IsEmpty())
	{
		Skins = Skins.FilterByPredicate([&SearchText](const FShipSkinData& Skin)
		{
			return Skin.DisplayName.ToString().Contains(SearchText, ESearchCase::IgnoreCase) ||
			       Skin.Description.ToString().Contains(SearchText, ESearchCase::IgnoreCase);
		});
	}

	// Sort by name
	Skins.Sort([](const FShipSkinData& A, const FShipSkinData& B)
	{
		return A.DisplayName.ToString() < B.DisplayName.ToString();
	});

	return Skins;
}

TArray<FShipPartData> ACustomizationMenuManager::SearchPartsByName(const FString& SearchText)
{
	if (!ValidateComponent() || SearchText.IsEmpty())
	{
		return TArray<FShipPartData>();
	}

	// Search across ALL categories
	TArray<FShipPartData> Results;

	// Iterate through all categories
	for (int32 i = 0; i < (int32)EShipPartCategory::Utility + 1; i++)
	{
		EShipPartCategory Category = (EShipPartCategory)i;
		TArray<FShipPartData> CategoryParts = CustomizationComponent->GetPartsInCategory(Category);

		for (const FShipPartData& Part : CategoryParts)
		{
			if (Part.DisplayName.ToString().Contains(SearchText, ESearchCase::IgnoreCase) ||
			    Part.Description.ToString().Contains(SearchText, ESearchCase::IgnoreCase))
			{
				Results.Add(Part);
			}
		}
	}

	return Results;
}

TArray<FShipPartData> ACustomizationMenuManager::FilterPartsByRarity(const TArray<FShipPartData>& Parts, const TArray<EShipPartRarity>& AllowedRarities)
{
	return Parts.FilterByPredicate([&AllowedRarities](const FShipPartData& Part)
	{
		return AllowedRarities.Contains(Part.Rarity);
	});
}

TArray<FShipPartData> ACustomizationMenuManager::FilterPartsByLevel(const TArray<FShipPartData>& Parts, int32 MinLevel, int32 MaxLevel)
{
	return Parts.FilterByPredicate([MinLevel, MaxLevel](const FShipPartData& Part)
	{
		return Part.LevelRequirement >= MinLevel && Part.LevelRequirement <= MaxLevel;
	});
}

TArray<FShipPartData> ACustomizationMenuManager::SortParts(const TArray<FShipPartData>& Parts, const FString& SortBy)
{
	TArray<FShipPartData> SortedParts = Parts;

	if (SortBy == "Name")
	{
		SortedParts.Sort([](const FShipPartData& A, const FShipPartData& B)
		{
			return A.DisplayName.ToString() < B.DisplayName.ToString();
		});
	}
	else if (SortBy == "Rarity")
	{
		SortedParts.Sort([](const FShipPartData& A, const FShipPartData& B)
		{
			return (int32)A.Rarity > (int32)B.Rarity; // Legendary first
		});
	}
	else if (SortBy == "Level")
	{
		SortedParts.Sort([](const FShipPartData& A, const FShipPartData& B)
		{
			return A.LevelRequirement < B.LevelRequirement;
		});
	}
	else if (SortBy == "Unlock")
	{
		// Sort by unlock cost (cheapest first)
		SortedParts.Sort([](const FShipPartData& A, const FShipPartData& B)
		{
			return A.UnlockCost < B.UnlockCost;
		});
	}

	return SortedParts;
}

void ACustomizationMenuManager::UpdateFilterSettings(const FPartFilterSettings& NewSettings)
{
	FilterSettings = NewSettings;

	// Fire event
	OnFilterChanged.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("Filter settings updated"));
}

void ACustomizationMenuManager::ResetFilters()
{
	FilterSettings.AllowedRarities.Empty();
	FilterSettings.bShowOnlyUnlocked = true;
	FilterSettings.bShowOnlyOwned = false;
	FilterSettings.SearchText = "";
	FilterSettings.MinLevelRequirement = 0;
	FilterSettings.MaxLevelRequirement = 999;
	FilterSettings.SortBy = "Name";

	// Fire event
	OnFilterChanged.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("Filters reset to default"));
}

// ============================================================================
// PART SELECTION & COMPARISON
// ============================================================================

void ACustomizationMenuManager::SelectPart(const FShipPartData& PartData)
{
	SelectedPart = PartData;
	bHasSelectedPart = true;

	// Navigate to comparison view
	NavigateToState(ECustomizationMenuState::StatsComparison);

	// Fire event
	OnPartSelected.Broadcast(PartData);

	UE_LOG(LogTemp, Log, TEXT("Part selected: %s"), *PartData.DisplayName.ToString());
}

void ACustomizationMenuManager::SelectSkin(const FShipSkinData& SkinData)
{
	SelectedSkin = SkinData;
	bHasSelectedSkin = true;

	// Fire event
	OnSkinSelected.Broadcast(SkinData);

	UE_LOG(LogTemp, Log, TEXT("Skin selected: %s"), *SkinData.DisplayName.ToString());
}

void ACustomizationMenuManager::ClearPartSelection()
{
	bHasSelectedPart = false;
	SelectedPart = FShipPartData();
}

void ACustomizationMenuManager::ClearSkinSelection()
{
	bHasSelectedSkin = false;
	SelectedSkin = FShipSkinData();
}

FPartComparisonData ACustomizationMenuManager::CompareSelectedPart()
{
	FPartComparisonData Comparison;

	if (!bHasSelectedPart || !ValidateComponent())
	{
		return Comparison;
	}

	// Get currently equipped part in same category
	bool bFound = false;
	FShipPartData CurrentPart = CustomizationComponent->GetEquippedPart(SelectedPart.Category, bFound);

	Comparison.CurrentPart = bFound ? CurrentPart : FShipPartData();
	Comparison.ComparedPart = SelectedPart;

	// Calculate stat difference
	if (bFound)
	{
		Comparison.StatDifference = CalculateStatDifference(CurrentPart, SelectedPart);

		// Determine if it's an upgrade
		Comparison.bIsBetter = IsPartUpgrade(CurrentPart, SelectedPart, Comparison.StatsImproved, Comparison.StatsWorsened);
	}
	else
	{
		// No part equipped, any part is an upgrade
		Comparison.StatDifference = SelectedPart.StatModifiers;
		Comparison.bIsBetter = true;
		Comparison.StatsImproved = 10; // All stats
		Comparison.StatsWorsened = 0;
	}

	return Comparison;
}

FShipStats ACustomizationMenuManager::CalculateStatDifference(const FShipPartData& PartA, const FShipPartData& PartB)
{
	FShipStats Difference;

	// Calculate difference (PartB - PartA)
	Difference.ThrustPower = PartB.StatModifiers.ThrustPower - PartA.StatModifiers.ThrustPower;
	Difference.MaxVelocity = PartB.StatModifiers.MaxVelocity - PartA.StatModifiers.MaxVelocity;
	Difference.RotationSpeed = PartB.StatModifiers.RotationSpeed - PartA.StatModifiers.RotationSpeed;
	Difference.Acceleration = PartB.StatModifiers.Acceleration - PartA.StatModifiers.Acceleration;
	Difference.HullIntegrity = PartB.StatModifiers.HullIntegrity - PartA.StatModifiers.HullIntegrity;
	Difference.ShieldStrength = PartB.StatModifiers.ShieldStrength - PartA.StatModifiers.ShieldStrength;
	Difference.WeaponDamage = PartB.StatModifiers.WeaponDamage - PartA.StatModifiers.WeaponDamage;
	Difference.EnergyCapacity = PartB.StatModifiers.EnergyCapacity - PartA.StatModifiers.EnergyCapacity;
	Difference.EnergyRegenRate = PartB.StatModifiers.EnergyRegenRate - PartA.StatModifiers.EnergyRegenRate;
	Difference.Mass = PartB.StatModifiers.Mass - PartA.StatModifiers.Mass;

	return Difference;
}

bool ACustomizationMenuManager::IsPartUpgrade(const FShipPartData& CurrentPart, const FShipPartData& NewPart, int32& OutImprovedStats, int32& OutWorsenedStats)
{
	FShipStats Diff = CalculateStatDifference(CurrentPart, NewPart);

	OutImprovedStats = 0;
	OutWorsenedStats = 0;

	// Count improved/worsened stats
	// Positive changes are improvements (except Mass - lower is better)
	if (Diff.ThrustPower > 0.0f) OutImprovedStats++;
	if (Diff.ThrustPower < 0.0f) OutWorsenedStats++;

	if (Diff.MaxVelocity > 0.0f) OutImprovedStats++;
	if (Diff.MaxVelocity < 0.0f) OutWorsenedStats++;

	if (Diff.RotationSpeed > 0.0f) OutImprovedStats++;
	if (Diff.RotationSpeed < 0.0f) OutWorsenedStats++;

	if (Diff.Acceleration > 0.0f) OutImprovedStats++;
	if (Diff.Acceleration < 0.0f) OutWorsenedStats++;

	if (Diff.HullIntegrity > 0.0f) OutImprovedStats++;
	if (Diff.HullIntegrity < 0.0f) OutWorsenedStats++;

	if (Diff.ShieldStrength > 0.0f) OutImprovedStats++;
	if (Diff.ShieldStrength < 0.0f) OutWorsenedStats++;

	if (Diff.WeaponDamage > 0.0f) OutImprovedStats++;
	if (Diff.WeaponDamage < 0.0f) OutWorsenedStats++;

	if (Diff.EnergyCapacity > 0.0f) OutImprovedStats++;
	if (Diff.EnergyCapacity < 0.0f) OutWorsenedStats++;

	if (Diff.EnergyRegenRate > 0.0f) OutImprovedStats++;
	if (Diff.EnergyRegenRate < 0.0f) OutWorsenedStats++;

	// Mass: lower is better
	if (Diff.Mass < 0.0f) OutImprovedStats++;
	if (Diff.Mass > 0.0f) OutWorsenedStats++;

	// Overall upgrade if more stats improved than worsened
	return OutImprovedStats > OutWorsenedStats;
}

// ============================================================================
// PURCHASE VALIDATION
// ============================================================================

bool ACustomizationMenuManager::CanAffordPart(const FShipPartData& PartData)
{
	if (!ValidateComponent())
	{
		return false;
	}

	int32 Credits = CustomizationComponent->GetCredits();
	return Credits >= PartData.UnlockCost;
}

bool ACustomizationMenuManager::CanAffordSkin(const FShipSkinData& SkinData)
{
	if (!ValidateComponent())
	{
		return false;
	}

	int32 Credits = CustomizationComponent->GetCredits();
	return Credits >= SkinData.UnlockCost;
}

bool ACustomizationMenuManager::MeetsLevelRequirement(const FShipPartData& PartData)
{
	if (!ValidateComponent())
	{
		return false;
	}

	int32 PlayerLevel = CustomizationComponent->GetPlayerLevel();
	return PlayerLevel >= PartData.LevelRequirement;
}

FText ACustomizationMenuManager::GetAffordabilityStatus(const FShipPartData& PartData)
{
	if (!ValidateComponent())
	{
		return FText::FromString("Error: Component not found");
	}

	// Check if already unlocked
	if (CustomizationComponent->IsPartUnlocked(PartData.PartID))
	{
		return FText::FromString("Owned");
	}

	// Check level requirement
	if (!MeetsLevelRequirement(PartData))
	{
		return FText::FromString(FString::Printf(TEXT("Requires Level %d"), PartData.LevelRequirement));
	}

	// Check credits
	if (!CanAffordPart(PartData))
	{
		int32 Needed = PartData.UnlockCost - CustomizationComponent->GetCredits();
		return FText::FromString(FString::Printf(TEXT("Need %d more credits"), Needed));
	}

	// Can purchase
	return FText::FromString("Can Purchase");
}

// ============================================================================
// STATS CALCULATION
// ============================================================================

FShipStats ACustomizationMenuManager::CalculateStatsWithPart(const FShipPartData& PartData)
{
	if (!ValidateComponent())
	{
		return FShipStats();
	}

	// Start with current total stats
	FShipStats SimulatedStats = CustomizationComponent->GetTotalStats();

	// Remove currently equipped part in same category
	bool bFound = false;
	FShipPartData CurrentPart = CustomizationComponent->GetEquippedPart(PartData.Category, bFound);

	if (bFound)
	{
		// Subtract current part stats (by subtracting the modifier)
		// Note: This is simplified - proper implementation would recalculate from scratch
		SimulatedStats.ThrustPower -= CurrentPart.StatModifiers.ThrustPower;
		SimulatedStats.MaxVelocity -= CurrentPart.StatModifiers.MaxVelocity;
		SimulatedStats.RotationSpeed -= CurrentPart.StatModifiers.RotationSpeed;
		SimulatedStats.Acceleration -= CurrentPart.StatModifiers.Acceleration;
		SimulatedStats.HullIntegrity -= CurrentPart.StatModifiers.HullIntegrity;
		SimulatedStats.ShieldStrength -= CurrentPart.StatModifiers.ShieldStrength;
		SimulatedStats.WeaponDamage -= CurrentPart.StatModifiers.WeaponDamage;
		SimulatedStats.EnergyCapacity -= CurrentPart.StatModifiers.EnergyCapacity;
		SimulatedStats.EnergyRegenRate -= CurrentPart.StatModifiers.EnergyRegenRate;
		SimulatedStats.Mass -= CurrentPart.StatModifiers.Mass;
	}

	// Add new part stats
	SimulatedStats = SimulatedStats + PartData.StatModifiers;

	return SimulatedStats;
}

FShipStats ACustomizationMenuManager::GetCurrentTotalStats()
{
	if (!ValidateComponent())
	{
		return FShipStats();
	}

	return CustomizationComponent->GetTotalStats();
}

// ============================================================================
// EVENT BINDINGS
// ============================================================================

void ACustomizationMenuManager::BindToComponentEvents()
{
	if (!CustomizationComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot bind events: CustomizationComponent is null"));
		return;
	}

	if (bEventsBound)
	{
		UE_LOG(LogTemp, Warning, TEXT("Events already bound"));
		return;
	}

	// Bind to component events
	CustomizationComponent->OnPartEquipped.AddDynamic(this, &ACustomizationMenuManager::OnPartEquippedEvent);
	CustomizationComponent->OnSkinEquipped.AddDynamic(this, &ACustomizationMenuManager::OnSkinEquippedEvent);
	CustomizationComponent->OnPartUnlocked.AddDynamic(this, &ACustomizationMenuManager::OnPartUnlockedEvent);
	CustomizationComponent->OnPlayerLevelUp.AddDynamic(this, &ACustomizationMenuManager::OnLevelUpEvent);

	bEventsBound = true;

	UE_LOG(LogTemp, Log, TEXT("Bound to CustomizationComponent events"));
}

void ACustomizationMenuManager::UnbindFromComponentEvents()
{
	if (!CustomizationComponent || !bEventsBound)
	{
		return;
	}

	// Unbind from component events
	CustomizationComponent->OnPartEquipped.RemoveDynamic(this, &ACustomizationMenuManager::OnPartEquippedEvent);
	CustomizationComponent->OnSkinEquipped.RemoveDynamic(this, &ACustomizationMenuManager::OnSkinEquippedEvent);
	CustomizationComponent->OnPartUnlocked.RemoveDynamic(this, &ACustomizationMenuManager::OnPartUnlockedEvent);
	CustomizationComponent->OnPlayerLevelUp.RemoveDynamic(this, &ACustomizationMenuManager::OnLevelUpEvent);

	bEventsBound = false;

	UE_LOG(LogTemp, Log, TEXT("Unbound from CustomizationComponent events"));
}

// ============================================================================
// EVENT HANDLERS
// ============================================================================

void ACustomizationMenuManager::OnPartEquippedEvent(EShipPartCategory Category, FName PartID)
{
	UE_LOG(LogTemp, Log, TEXT("Menu: Part equipped - Category %d, ID %s"), (int32)Category, *PartID.ToString());

	// Clear selection if we just equipped the selected part
	if (bHasSelectedPart && SelectedPart.PartID == PartID)
	{
		ClearPartSelection();
	}

	// Navigate back to category selection or part browser
	if (CurrentMenuState == ECustomizationMenuState::StatsComparison)
	{
		NavigateBack();
	}
}

void ACustomizationMenuManager::OnSkinEquippedEvent(FName SkinID)
{
	UE_LOG(LogTemp, Log, TEXT("Menu: Skin equipped - ID %s"), *SkinID.ToString());

	// Clear selection if we just equipped the selected skin
	if (bHasSelectedSkin && SelectedSkin.SkinID == SkinID)
	{
		ClearSkinSelection();
	}
}

void ACustomizationMenuManager::OnPartUnlockedEvent(FName PartID)
{
	UE_LOG(LogTemp, Log, TEXT("Menu: Part unlocked - ID %s"), *PartID.ToString());

	// Refresh filters to show newly unlocked part
	OnFilterChanged.Broadcast();
}

void ACustomizationMenuManager::OnLevelUpEvent(int32 OldLevel, int32 NewLevel)
{
	UE_LOG(LogTemp, Log, TEXT("Menu: Level up - %d -> %d"), OldLevel, NewLevel);

	// Refresh filters to show newly available parts
	OnFilterChanged.Broadcast();
}

// ============================================================================
// HELPERS
// ============================================================================

UShipCustomizationComponent* ACustomizationMenuManager::FindCustomizationComponent()
{
	// Get player controller
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController not found"));
		return nullptr;
	}

	// Get player pawn
	APawn* PlayerPawn = PC->GetPawn();

	if (!PlayerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Player pawn not found"));
		return nullptr;
	}

	// Find component on pawn
	UShipCustomizationComponent* Component = PlayerPawn->FindComponentByClass<UShipCustomizationComponent>();

	if (!Component)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShipCustomizationComponent not found on player pawn"));
		return nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("Found ShipCustomizationComponent on %s"), *PlayerPawn->GetName());

	return Component;
}

bool ACustomizationMenuManager::ValidateComponent()
{
	if (!CustomizationComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("CustomizationComponent is null"));
		return false;
	}

	return true;
}
