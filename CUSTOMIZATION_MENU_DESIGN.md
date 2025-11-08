# Ship Customization Menu - UI Architecture Design

**Phase 3: Menu Management & UI System**
**Created:** 2025-01-07
**Status:** Core C++ Classes Complete

---

## Table of Contents

1. [System Overview](#system-overview)
2. [Architecture Layers](#architecture-layers)
3. [Widget Hierarchy](#widget-hierarchy)
4. [Data Flow](#data-flow)
5. [Event Binding Architecture](#event-binding-architecture)
6. [State Management](#state-management)
7. [Performance Considerations](#performance-considerations)
8. [VR-Specific Design](#vr-specific-design)
9. [Implementation Guide](#implementation-guide)
10. [API Reference](#api-reference)

---

## System Overview

The Ship Customization Menu system is a modular, event-driven UI architecture designed for both traditional and VR gameplay. It separates concerns between:

- **Data Layer**: `UShipCustomizationComponent` (Phase 2)
- **Business Logic Layer**: `ACustomizationMenuManager` (Phase 3)
- **Presentation Layer**: UMG Widgets (Phase 3, to be implemented)
- **Utilities**: `UCustomizationUIHelper` (Phase 3)

### Key Features

- **Reactive UI**: Event-driven updates via multicast delegates
- **State Management**: Centralized menu state with navigation stack
- **Filtering & Search**: Advanced part filtering with multiple criteria
- **Comparison Tools**: Side-by-side stat comparisons with visual indicators
- **Purchase Validation**: Real-time affordability checks
- **VR Support**: World-space 3D widgets with spatial UI
- **Performance**: Optimized for 90+ FPS in VR

---

## Architecture Layers

```
┌─────────────────────────────────────────────────────┐
│           PRESENTATION LAYER (UMG)                  │
│  - Main Menu Widget                                 │
│  - Category Browser Widget                          │
│  - Part List Widget                                 │
│  - Stats Comparison Widget                          │
│  - Loadout Manager Widget                           │
└─────────────────────────────────────────────────────┘
                         ▲
                         │ Binds to Events
                         │ Calls Functions
                         ▼
┌─────────────────────────────────────────────────────┐
│        BUSINESS LOGIC LAYER (C++)                   │
│  - ACustomizationMenuManager                        │
│    * Menu state management                          │
│    * Filtering & search                             │
│    * Comparison calculations                        │
│    * Purchase validation                            │
│  - UCustomizationUIHelper                           │
│    * Formatting utilities                           │
│    * Color helpers                                  │
│    * Icon utilities                                 │
└─────────────────────────────────────────────────────┘
                         ▲
                         │ Queries Data
                         │ Listens to Events
                         ▼
┌─────────────────────────────────────────────────────┐
│            DATA LAYER (C++)                         │
│  - UShipCustomizationComponent                      │
│    * Loadout management                             │
│    * Stats calculation                              │
│    * Progression tracking                           │
│    * Save/Load system                               │
│  - UShipCustomizationBlueprintLibrary               │
│    * Helper functions                               │
│    * Rarity colors                                  │
└─────────────────────────────────────────────────────┘
                         ▲
                         │ Reads From
                         ▼
┌─────────────────────────────────────────────────────┐
│              DATA STORAGE                           │
│  - DT_ShipParts (DataTable)                         │
│  - DT_ShipSkins (DataTable)                         │
│  - SaveGame (UShipCustomizationSaveGame)            │
└─────────────────────────────────────────────────────┘
```

---

## Widget Hierarchy

### Recommended UMG Structure

```
WBP_CustomizationMenu (Main Container)
│
├── WBP_MenuNavigation (Top Bar)
│   ├── Button_Back
│   ├── Text_MenuTitle
│   └── Button_Close
│
├── WBP_PlayerInfo (Header)
│   ├── Text_PlayerLevel
│   ├── ProgressBar_XP
│   ├── Text_XPProgress
│   └── Text_Credits
│
├── WidgetSwitcher_MenuStates
│   │
│   ├── WBP_CategorySelection (Index 0)
│   │   ├── Button_Engine
│   │   ├── Button_Thrusters
│   │   ├── Button_Hull
│   │   ├── Button_Wings
│   │   ├── Button_Cockpit
│   │   ├── Button_Weapon
│   │   ├── Button_Shield
│   │   ├── Button_Utility
│   │   ├── Button_Skins
│   │   └── Button_Loadouts
│   │
│   ├── WBP_PartBrowser (Index 1)
│   │   ├── WBP_FilterPanel
│   │   │   ├── EditableText_Search
│   │   │   ├── CheckBox_ShowOnlyUnlocked
│   │   │   ├── ComboBox_SortBy
│   │   │   └── WBP_RarityFilters
│   │   │       ├── CheckBox_Common
│   │   │       ├── CheckBox_Uncommon
│   │   │       ├── CheckBox_Rare
│   │   │       ├── CheckBox_Epic
│   │   │       └── CheckBox_Legendary
│   │   │
│   │   ├── ScrollBox_PartList
│   │   │   └── WBP_PartListItem (x N)
│   │   │       ├── Image_Icon
│   │   │       ├── Text_PartName
│   │   │       ├── Text_Rarity
│   │   │       ├── Text_LevelRequirement
│   │   │       ├── Text_UnlockCost
│   │   │       ├── Image_LockedIcon
│   │   │       └── Button_Select
│   │   │
│   │   └── WBP_EquippedPartPreview
│   │       ├── Text_CurrentlyEquipped
│   │       ├── Image_EquippedIcon
│   │       └── WBP_StatsDisplay
│   │
│   ├── WBP_StatsComparison (Index 2)
│   │   ├── WBP_CurrentPart (Left Column)
│   │   │   ├── Text_PartName
│   │   │   ├── Image_Icon
│   │   │   ├── Text_Rarity
│   │   │   └── WBP_StatsDisplay
│   │   │
│   │   ├── WBP_ComparisonArrows (Center)
│   │   │   ├── Image_Arrow
│   │   │   └── Text_UpgradeSummary
│   │   │
│   │   ├── WBP_NewPart (Right Column)
│   │   │   ├── Text_PartName
│   │   │   ├── Image_Icon
│   │   │   ├── Text_Rarity
│   │   │   └── WBP_StatsDisplay
│   │   │
│   │   ├── WBP_StatDifferences
│   │   │   └── WBP_StatDifferenceRow (x 10)
│   │   │       ├── Text_StatName
│   │   │       ├── Text_CurrentValue
│   │   │       ├── Text_Arrow
│   │   │       ├── Text_NewValue
│   │   │       └── Text_Difference (colored)
│   │   │
│   │   └── WBP_ActionButtons
│   │       ├── Button_Equip
│   │       ├── Button_Unlock (if locked)
│   │       └── Text_AffordabilityStatus
│   │
│   ├── WBP_SkinBrowser (Index 3)
│   │   ├── EditableText_Search
│   │   ├── ScrollBox_SkinList
│   │   │   └── WBP_SkinListItem (x N)
│   │   │       ├── Image_Preview
│   │   │       ├── Text_SkinName
│   │   │       ├── Text_Rarity
│   │   │       ├── Text_UnlockCost
│   │   │       └── Button_Select
│   │   │
│   │   └── WBP_ActionButtons
│   │       ├── Button_Equip
│   │       └── Button_Unlock
│   │
│   └── WBP_LoadoutManager (Index 4)
│       ├── ScrollBox_LoadoutList
│       │   └── WBP_LoadoutListItem (x N)
│       │       ├── Text_LoadoutName
│       │       ├── Text_PartsEquipped
│       │       ├── WBP_StatsPreview
│       │       ├── Button_Load
│       │       └── Button_Delete
│       │
│       └── WBP_CurrentLoadout
│           ├── EditableText_LoadoutName
│           ├── Button_SaveLoadout
│           └── WBP_EquippedPartsGrid
│               ├── WBP_EquippedPartSlot (Engine)
│               ├── WBP_EquippedPartSlot (Thrusters)
│               ├── WBP_EquippedPartSlot (Hull)
│               ├── WBP_EquippedPartSlot (Wings)
│               ├── WBP_EquippedPartSlot (Cockpit)
│               ├── WBP_EquippedPartSlot (Weapon)
│               ├── WBP_EquippedPartSlot (Shield)
│               └── WBP_EquippedPartSlot (Utility)
│
└── WBP_ShipPreview (3D Ship Model)
    ├── SceneCapture2D
    └── Image_ShipRender
```

---

## Data Flow

### Opening the Menu

```
Player Input (Press "C")
    │
    ▼
APlayerController::InputAction_OpenCustomization()
    │
    ▼
ACustomizationMenuManager::OpenMenu()
    │
    ├─► Set bIsMenuOpen = true
    ├─► Set CurrentMenuState = CategorySelection
    ├─► OnMenuVisibilityChanged.Broadcast(true)
    └─► OnMenuStateChanged.Broadcast(Closed, CategorySelection)
            │
            ▼
    WBP_CustomizationMenu::OnMenuOpened()
        │
        ├─► Show widget
        ├─► Bind to MenuManager events
        ├─► Query player info (Level, XP, Credits)
        └─► Update UI elements
```

### Browsing Parts

```
Player clicks "Engine" category
    │
    ▼
WBP_CategorySelection::OnEngineCategoryClicked()
    │
    ▼
ACustomizationMenuManager::SelectCategory(Engine)
    │
    ├─► Set CurrentCategory = Engine
    ├─► Update FilterSettings.Category = Engine
    ├─► NavigateToState(PartBrowser)
    ├─► ClearPartSelection()
    └─► OnFilterChanged.Broadcast()
            │
            ▼
    WBP_PartBrowser::OnFilterChanged()
        │
        ├─► Call GetFilteredParts()
        │       │
        │       ▼
        │   ACustomizationMenuManager::GetFilteredParts()
        │       │
        │       ├─► Query CustomizationComponent::GetPartsInCategory(Engine)
        │       ├─► Apply unlocked filter
        │       ├─► Apply search filter
        │       ├─► Apply rarity filter
        │       ├─► Apply level filter
        │       └─► Sort parts
        │               │
        │               ▼
        │       Return TArray<FShipPartData>
        │
        ├─► Clear ScrollBox_PartList
        ├─► For each part in filtered list:
        │   │
        │   └─► Create WBP_PartListItem
        │       ├─► Set part data
        │       ├─► Format using UCustomizationUIHelper
        │       ├─► Set rarity color
        │       ├─► Set locked/unlocked state
        │       └─► Bind click event
        │
        └─► Update equipped part preview
```

### Selecting a Part for Comparison

```
Player clicks part in list
    │
    ▼
WBP_PartListItem::OnClicked()
    │
    ▼
ACustomizationMenuManager::SelectPart(PartData)
    │
    ├─► Set SelectedPart = PartData
    ├─► Set bHasSelectedPart = true
    ├─► NavigateToState(StatsComparison)
    └─► OnPartSelected.Broadcast(PartData)
            │
            ▼
    WBP_StatsComparison::OnPartSelected(PartData)
        │
        ├─► Call CompareSelectedPart()
        │       │
        │       ▼
        │   ACustomizationMenuManager::CompareSelectedPart()
        │       │
        │       ├─► Get currently equipped part
        │       ├─► Calculate stat difference
        │       ├─► Determine if upgrade
        │       └─► Return FPartComparisonData
        │
        ├─► Display current part (left column)
        ├─► Display new part (right column)
        ├─► Display stat differences with colors
        │   │
        │   └─► For each stat:
        │       ├─► Format using UCustomizationUIHelper::FormatStatDifference()
        │       └─► Color using GetStatDifferenceColor()
        │
        ├─► Display upgrade summary
        └─► Update action buttons (Equip/Unlock)
```

### Equipping a Part

```
Player clicks "Equip" button
    │
    ▼
WBP_StatsComparison::OnEquipButtonClicked()
    │
    ├─► Validate part is unlocked
    ├─► Validate level requirement
    └─► Call CustomizationComponent::EquipPart(Category, PartID)
            │
            ├─► Validate part
            ├─► Update CurrentLoadout.EquippedParts
            ├─► Recalculate stats
            ├─► Apply to FlightController
            ├─► Update visuals
            ├─► OnPartEquipped.Broadcast(Category, PartID)
            └─► OnLoadoutChanged.Broadcast()
                    │
                    ▼
            ACustomizationMenuManager::OnPartEquippedEvent(Category, PartID)
                │
                ├─► Clear selection
                └─► Navigate back to PartBrowser
                        │
                        ▼
                WBP_PartBrowser::RefreshPartList()
                    └─► Update equipped part indicator
```

### Unlocking a Part

```
Player clicks "Unlock" button
    │
    ▼
WBP_StatsComparison::OnUnlockButtonClicked()
    │
    ├─► Validate affordability
    ├─► Validate level requirement
    └─► Call CustomizationComponent::UnlockPart(PartID)
            │
            ├─► Check credits
            ├─► Deduct credits
            ├─► Add to UnlockedParts
            ├─► Save progress
            ├─► OnPartUnlocked.Broadcast(PartID)
            └─► OnCreditsChanged (implicitly via save)
                    │
                    ▼
            ACustomizationMenuManager::OnPartUnlockedEvent(PartID)
                │
                └─► OnFilterChanged.Broadcast()
                        │
                        ▼
                WBP_PartBrowser::OnFilterChanged()
                    └─► Refresh part list (newly unlocked part now available)
```

---

## Event Binding Architecture

### MenuManager Events → UI Widgets

The UI widgets bind to `ACustomizationMenuManager` events in their `Construct()` or `NativeConstruct()` functions:

```cpp
// WBP_CustomizationMenu.cpp (example)
void UWBPCustomizationMenu::NativeConstruct()
{
    Super::NativeConstruct();

    // Find MenuManager
    MenuManager = FindMenuManager();

    if (MenuManager)
    {
        // Bind to menu events
        MenuManager->OnMenuStateChanged.AddDynamic(this, &UWBPCustomizationMenu::OnMenuStateChanged);
        MenuManager->OnMenuVisibilityChanged.AddDynamic(this, &UWBPCustomizationMenu::OnMenuVisibilityChanged);
        MenuManager->OnPartSelected.AddDynamic(this, &UWBPCustomizationMenu::OnPartSelected);
        MenuManager->OnSkinSelected.AddDynamic(this, &UWBPCustomizationMenu::OnSkinSelected);
        MenuManager->OnFilterChanged.AddDynamic(this, &UWBPCustomizationMenu::OnFilterChanged);
    }
}
```

### CustomizationComponent Events → MenuManager

The MenuManager automatically binds to component events when `bAutoBindEvents = true`:

```cpp
void ACustomizationMenuManager::BindToComponentEvents()
{
    CustomizationComponent->OnPartEquipped.AddDynamic(this, &ACustomizationMenuManager::OnPartEquippedEvent);
    CustomizationComponent->OnSkinEquipped.AddDynamic(this, &ACustomizationMenuManager::OnSkinEquippedEvent);
    CustomizationComponent->OnPartUnlocked.AddDynamic(this, &ACustomizationMenuManager::OnPartUnlockedEvent);
    CustomizationComponent->OnPlayerLevelUp.AddDynamic(this, &ACustomizationMenuManager::OnLevelUpEvent);
}
```

### Event Flow Diagram

```
CustomizationComponent          MenuManager                UI Widgets
        │                            │                         │
        │  OnPartEquipped             │                         │
        ├─────────────────────────────►                         │
        │                            │                         │
        │                     OnPartEquippedEvent()            │
        │                            │                         │
        │                      ClearPartSelection()            │
        │                            │                         │
        │                      NavigateBack()                  │
        │                            │                         │
        │                            │  OnMenuStateChanged     │
        │                            ├─────────────────────────►
        │                            │                         │
        │                            │                  RefreshUI()
        │                            │                         │
```

---

## State Management

### Menu States

The menu uses an enum-based state machine:

```cpp
enum class ECustomizationMenuState : uint8
{
    Closed,              // Menu is closed
    CategorySelection,   // Main menu - choose category
    PartBrowser,        // Browse parts in selected category
    SkinBrowser,        // Browse skins
    LoadoutManager,     // Manage saved loadouts
    StatsComparison     // Compare selected part with equipped
};
```

### State Transitions

```
Closed
  │
  ├─► OpenMenu() ──────► CategorySelection
                              │
                              ├─► SelectCategory(Engine) ──► PartBrowser
                              │                                   │
                              │                                   ├─► SelectPart() ──► StatsComparison
                              │                                   │                         │
                              │                                   │                         ├─► EquipPart() ──► PartBrowser
                              │                                   │                         │
                              │                                   │                         └─► NavigateBack() ──► PartBrowser
                              │                                   │
                              │                                   └─► NavigateBack() ──► CategorySelection
                              │
                              ├─► SelectSkins() ────────────────► SkinBrowser
                              │                                       │
                              │                                       └─► NavigateBack() ──► CategorySelection
                              │
                              └─► SelectLoadouts() ──────────────► LoadoutManager
                                                                      │
                                                                      └─► NavigateBack() ──► CategorySelection
```

### State Persistence

- `CurrentMenuState`: Current active state
- `PreviousMenuState`: Previous state (for back navigation)
- Back navigation uses `NavigateBack()` which restores `PreviousMenuState`

---

## Performance Considerations

### 1. Widget Pooling

For `ScrollBox_PartList`, use widget pooling to avoid creating/destroying widgets:

```cpp
// Create pool of WBP_PartListItem widgets
// Reuse widgets when scrolling instead of destroying/creating
```

### 2. Lazy Loading

- Icons: Use async loading for part/skin icons
- Meshes: Only load 3D preview mesh when needed
- DataTables: Cache frequently accessed rows

### 3. Update Throttling

- Debounce search input (wait 300ms after last keystroke)
- Throttle filter updates (max 10 updates/second)
- Batch UI updates in single frame

### 4. Profiling Hooks

The MenuManager uses profiling stats (inherited from ShipCustomizationComponent):

```cpp
SCOPE_CYCLE_COUNTER(STAT_ShipCustomization_EquipPart);
SCOPE_CYCLE_COUNTER(STAT_ShipCustomization_CalculateStats);
SCOPE_CYCLE_COUNTER(STAT_ShipCustomization_UpdateVisuals);
```

### 5. VR Frame Rate Targets

- **90 FPS minimum** for VR headsets
- **120 FPS target** for high-end VR
- Use `stat fps` and `stat unit` to monitor performance

---

## VR-Specific Design

### 1. World-Space 3D Widgets

Place `ACustomizationMenuManager` in world, attach widget component:

```cpp
// In VR GameMode or PlayerController
ACustomizationMenuManager* MenuManager = GetWorld()->SpawnActor<ACustomizationMenuManager>(Location, Rotation);

// Attach 3D widget
UWidgetComponent* WidgetComp = MenuManager->FindComponentByClass<UWidgetComponent>();
WidgetComp->SetWidgetClass(WBP_CustomizationMenu::StaticClass());
WidgetComp->SetDrawSize(FVector2D(1920, 1080));
WidgetComp->SetPivot(FVector2D(0.5f, 0.5f));
```

### 2. VR Interaction

- Use **VR Motion Controller** line traces for interaction
- Support **hand tracking** for direct touch
- Implement **laser pointer** for distant UI elements
- Add **haptic feedback** on button clicks

### 3. Text Readability

- **Minimum font size**: 24pt for VR
- **High contrast**: White text on dark backgrounds
- **Clear hierarchy**: Bold headers, regular body text
- **Avoid small details**: Icons should be large (64x64 minimum)

### 4. Spatial Layout

```
Player Position
      │
      │ 1.5m distance
      ▼
┌─────────────────────────────────┐
│   Customization Menu (3D)       │  ← Eye level (1.6m height)
│   - Large buttons               │
│   - Clear text (24pt+)          │
│   - High contrast colors        │
└─────────────────────────────────┘
      │
      │ 1.0m to left
      ▼
┌──────────────────┐
│  Ship Preview    │  ← 3D model rotating
│  (3D Actor)      │
└──────────────────┘
```

### 5. Navigation

- **VR-friendly back button**: Large, always visible in top-left
- **Thumbstick navigation**: Support for gamepad-style scrolling
- **Gaze-based selection**: Optional eye-tracking support

---

## Implementation Guide

### Step 1: Setup MenuManager

```cpp
// In your VR GameMode or Level Blueprint
void AVRGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Spawn MenuManager
    FVector SpawnLocation = FVector(100.0f, 0.0f, 150.0f); // In front of player
    FRotator SpawnRotation = FRotator::ZeroRotator;

    MenuManager = GetWorld()->SpawnActor<ACustomizationMenuManager>(
        ACustomizationMenuManager::StaticClass(),
        SpawnLocation,
        SpawnRotation
    );

    // MenuManager will auto-find CustomizationComponent if bAutoFindComponent = true
    // MenuManager will auto-bind events if bAutoBindEvents = true
}
```

### Step 2: Create Main Menu Widget

```cpp
// WBP_CustomizationMenu.h
UCLASS()
class UWBPCustomizationMenu : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    // Reference to MenuManager
    UPROPERTY(BlueprintReadOnly, Category = "Customization")
    ACustomizationMenuManager* MenuManager;

    // Event handlers
    UFUNCTION()
    void OnMenuStateChanged(ECustomizationMenuState OldState, ECustomizationMenuState NewState);

    UFUNCTION()
    void OnMenuVisibilityChanged(bool bIsOpen);

    UFUNCTION()
    void OnFilterChanged();

    // Widget references
    UPROPERTY(meta = (BindWidget))
    UWidgetSwitcher* WidgetSwitcher_MenuStates;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_PlayerLevel;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* Text_Credits;

    // ... more widget bindings
};
```

### Step 3: Implement Part Browser

```cpp
// WBP_PartBrowser.cpp
void UWBPPartBrowser::RefreshPartList()
{
    if (!MenuManager)
        return;

    // Get filtered parts
    TArray<FShipPartData> Parts = MenuManager->GetFilteredParts();

    // Clear list
    ScrollBox_PartList->ClearChildren();

    // Add part items
    for (const FShipPartData& Part : Parts)
    {
        // Create widget
        UWBPPartListItem* ItemWidget = CreateWidget<UWBPPartListItem>(this, PartListItemClass);

        // Set data
        ItemWidget->SetPartData(Part);

        // Add to scroll box
        ScrollBox_PartList->AddChild(ItemWidget);
    }
}
```

### Step 4: Implement Stats Comparison

```cpp
// WBP_StatsComparison.cpp
void UWBPStatsComparison::UpdateComparison()
{
    if (!MenuManager || !MenuManager->bHasSelectedPart)
        return;

    // Get comparison data
    FPartComparisonData Comparison = MenuManager->CompareSelectedPart();

    // Display current part
    Text_CurrentPartName->SetText(Comparison.CurrentPart.DisplayName);
    Image_CurrentIcon->SetBrushFromTexture(
        UCustomizationUIHelper::LoadIconTexture(Comparison.CurrentPart.Icon)
    );

    // Display new part
    Text_NewPartName->SetText(Comparison.ComparedPart.DisplayName);
    Image_NewIcon->SetBrushFromTexture(
        UCustomizationUIHelper::LoadIconTexture(Comparison.ComparedPart.Icon)
    );

    // Display stat differences
    UpdateStatDifferences(Comparison.StatDifference);

    // Display upgrade summary
    FText Summary = UCustomizationUIHelper::GetUpgradeSummary(
        Comparison.StatsImproved,
        Comparison.StatsWorsened
    );
    Text_UpgradeSummary->SetText(Summary);

    // Update button states
    UpdateActionButtons(Comparison.ComparedPart);
}
```

---

## API Reference

### ACustomizationMenuManager

#### Key Functions

| Function | Description | Returns |
|----------|-------------|---------|
| `OpenMenu()` | Opens the customization menu | `bool` |
| `CloseMenu()` | Closes the customization menu | `bool` |
| `SelectCategory(Category)` | Browse parts in a category | `bool` |
| `GetFilteredParts()` | Get parts matching current filters | `TArray<FShipPartData>` |
| `SelectPart(PartData)` | Select part for comparison | `void` |
| `CompareSelectedPart()` | Get comparison data | `FPartComparisonData` |
| `CanAffordPart(PartData)` | Check if player can buy | `bool` |

#### Events

| Event | Parameters | When Fired |
|-------|------------|------------|
| `OnMenuStateChanged` | `OldState`, `NewState` | Menu state changes |
| `OnMenuVisibilityChanged` | `bIsOpen` | Menu opens/closes |
| `OnPartSelected` | `PartData` | Part selected |
| `OnSkinSelected` | `SkinData` | Skin selected |
| `OnFilterChanged` | None | Filter settings change |

### UCustomizationUIHelper

#### Formatting Functions

| Function | Description | Example |
|----------|-------------|---------|
| `FormatStatMultiplier(1.5f, 1)` | Format stat as multiplier | "1.5x" |
| `FormatCredits(1250)` | Format credits | "1,250 CR" |
| `FormatXP(5000)` | Format XP | "5,000 XP" |
| `FormatLevel(15)` | Format level | "Level 15" |
| `FormatStatDifference(0.3f)` | Format stat diff | "+0.3x" |

#### Color Functions

| Function | Description | Returns |
|----------|-------------|---------|
| `GetRarityColor(Rarity)` | Get rarity color | `FLinearColor` |
| `GetStatDifferenceColor(Diff)` | Green/red for +/- | `FLinearColor` |
| `GetPositiveColor()` | Green | `FLinearColor(0,1,0,1)` |
| `GetNegativeColor()` | Red | `FLinearColor(1,0,0,1)` |

#### Validation Functions

| Function | Description | Returns |
|----------|-------------|---------|
| `CanAffordPart(Part, Credits)` | Check affordability | `bool` |
| `MeetsLevelRequirement(Part, Level)` | Check level req | `bool` |
| `GetPurchaseStatus(Part, Credits, Level, IsUnlocked)` | Get status text | `FText` |

---

## Blueprint Integration Examples

### Example 1: Open Menu from Input

```
Event: InputAction_OpenMenu
    │
    ├─► Get All Actors of Class (ACustomizationMenuManager)
    │       │
    │       └─► [0] (first MenuManager)
    │               │
    │               └─► OpenMenu()
    │                       │
    │                       └─► Return Value (bool)
    │                               │
    │                               └─► Branch
    │                                   ├─► True: Show cursor, set input mode UI
    │                                   └─► False: Print error
```

### Example 2: Display Filtered Parts

```
Event: OnFilterChanged (from MenuManager)
    │
    ├─► MenuManager → GetFilteredParts()
    │       │
    │       └─► Return Array (FShipPartData)
    │               │
    │               └─► ForEach Loop
    │                       │
    │                       ├─► Create Widget (WBP_PartListItem)
    │                       │       │
    │                       │       └─► SetPartData(LoopBody)
    │                       │               │
    │                       │               ├─► Format using CustomizationUIHelper
    │                       │               └─► Add to ScrollBox
```

### Example 3: Equip Part with Validation

```
Event: OnEquipButtonClicked
    │
    ├─► Get Selected Part (from MenuManager)
    │       │
    │       └─► CustomizationComponent → IsPartUnlocked(PartID)
    │               │
    │               └─► Branch
    │                   ├─► True: EquipPart(Category, PartID)
    │                   │       │
    │                   │       └─► Success: Show "Equipped!" notification
    │                   │
    │                   └─► False: Show "Part is locked" error
```

---

## Testing Checklist

### Functionality Tests

- [ ] Menu opens/closes correctly
- [ ] Category selection works
- [ ] Part filtering by category
- [ ] Part filtering by rarity
- [ ] Part filtering by level
- [ ] Search functionality
- [ ] Part comparison shows correct stats
- [ ] Stat differences calculated correctly
- [ ] Colors display properly (green/red)
- [ ] Equip part updates stats
- [ ] Unlock part deducts credits
- [ ] Loadout save/load works
- [ ] Events fire correctly

### VR Tests

- [ ] 3D widget renders in world space
- [ ] VR controller can interact with buttons
- [ ] Text is readable at 1.5m distance
- [ ] No motion sickness (smooth UI)
- [ ] Haptic feedback works on clicks
- [ ] Menu maintains 90+ FPS
- [ ] Ship preview rotates smoothly

### Performance Tests

- [ ] Filtering 100+ parts: < 16ms
- [ ] Search across all parts: < 10ms
- [ ] Menu open/close: < 5ms
- [ ] Part selection: < 5ms
- [ ] Stat calculation: < 1ms
- [ ] Total frame time: < 11ms (90 FPS)

---

## Future Enhancements

### Phase 4 (Content Expansion)

- [ ] 3D ship preview with real-time part swapping
- [ ] Animated transitions between menu states
- [ ] Advanced sorting (by stat values)
- [ ] Favorites/bookmarking system
- [ ] Part comparison history (compare last 5 parts)
- [ ] Grid view vs list view toggle

### Phase 5 (Advanced Features)

- [ ] Loadout recommendations (AI-suggested builds)
- [ ] Community loadouts (multiplayer sharing)
- [ ] Achievement integration (unlock parts via achievements)
- [ ] Daily/weekly featured parts
- [ ] Customization challenges (build constraints)
- [ ] Part upgrade preview (show upgrade tree)

---

## Conclusion

This architecture provides a solid foundation for a production-quality ship customization menu system. The separation of concerns between data, business logic, and presentation ensures:

- **Maintainability**: Changes to UI don't affect core logic
- **Testability**: Each layer can be tested independently
- **Scalability**: Easy to add new features (skins, loadouts, etc.)
- **Performance**: Optimized for VR frame rates
- **Flexibility**: Works in both traditional and VR gameplay

All C++ classes are complete and ready for UMG widget integration in Phase 3.

---

**Document Version:** 1.0
**Last Updated:** 2025-01-07
**Next Review:** Phase 3 Widget Implementation
