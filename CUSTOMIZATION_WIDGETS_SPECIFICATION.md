# Ship Customization System - UMG Widget Specifications

## Overview

This document provides complete specifications for all UMG widgets required for the VR Flight Ship Customization System. All widgets are designed with both desktop and VR interaction in mind, with specific adaptations for VR usability.

---

## Global Design Standards

### Rarity Color Palette

```
Common:    #4CAF50 (Green)      - RGB(76, 175, 80)
Uncommon:  #2196F3 (Blue)       - RGB(33, 150, 243)
Rare:      #9C27B0 (Purple)     - RGB(156, 39, 176)
Epic:      #FF9800 (Orange)     - RGB(255, 152, 0)
Legendary: #F44336 (Red)        - RGB(244, 67, 54)
```

### Typography Standards

#### Desktop
- **Headers**: 32pt, Bold, White
- **Subheaders**: 24pt, SemiBold, #E0E0E0
- **Body Text**: 16pt, Regular, #BDBDBD
- **Small Text**: 12pt, Regular, #9E9E9E

#### VR (200% Scale)
- **Headers**: 64pt, Bold, White
- **Subheaders**: 48pt, SemiBold, #E0E0E0
- **Body Text**: 32pt, Regular, #BDBDBD
- **Small Text**: 24pt, Regular, #9E9E9E

### Interactive Element Standards

#### Desktop
- **Buttons**: Minimum 120px × 40px
- **Hit Targets**: Minimum 32px × 32px
- **Spacing**: 8px grid system

#### VR
- **Buttons**: Minimum 200px × 80px (2cm × 2cm at 100 DPI)
- **Hit Targets**: Minimum 80px × 80px
- **Spacing**: 16px grid system

### Color Palette

```
Primary Background:    #1A1A1A (Very Dark Gray)
Secondary Background:  #2A2A2A (Dark Gray)
Panel Borders:         #424242 (Medium Gray)
Accent Color:          #00BCD4 (Cyan)
Success:               #4CAF50 (Green)
Warning:               #FFC107 (Amber)
Error:                 #F44336 (Red)
Text Primary:          #FFFFFF (White)
Text Secondary:        #BDBDBD (Light Gray)
```

---

## Widget 1: WBP_CustomizationMenu

### Purpose
Main container widget for the entire customization system. Orchestrates all child panels and manages overall UI state.

### Layout Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│  [X]                    SHIP CUSTOMIZATION                          │
├─────────────┬─────────────────────────────┬─────────────────────────┤
│             │                             │                         │
│   PART      │      PART DETAIL            │    STATS COMPARISON     │
│ SELECTION   │                             │                         │
│             │   ┌─────────────┐           │  Current → New          │
│ [Tabs]      │   │   [IMAGE]   │           │                         │
│             │   └─────────────┘           │  Speed:   100 → 120 ↑   │
│ ┌─────────┐ │                             │  Armor:   80  → 75  ↓   │
│ │ Engine  │ │   Part Name (Rarity)        │  Shields: 50  → 50  -   │
│ └─────────┘ │                             │                         │
│ [Weapon]    │   Description text goes     │  Total Loadout:         │
│ [Shield]    │   here with detailed info   │  ───────────────        │
│ [Hull]      │   about this part.          │  Overall: +15% Speed    │
│ [Thruster]  │                             │           -5% Armor     │
│ [Wings]     │   STATS:                    │                         │
│ [Utility]   │   • Speed: +20%             │  [APPLY CHANGES]        │
│ [Skins]     │   • Handling: +10%          │                         │
│             │                             │                         │
│ [Search]    │   Cost: 500 CR              │                         │
│             │   [EQUIP] [UNLOCK]          │                         │
│ [Parts      │                             │                         │
│  List]      │                             │                         │
│             │                             │                         │
└─────────────┴─────────────────────────────┴─────────────────────────┘
```

### Component Hierarchy

```
WBP_CustomizationMenu (User Widget)
├── Canvas Panel (Root)
│   ├── Image_Background (Blur Effect)
│   ├── Overlay_MainContainer
│   │   ├── Border_MainPanel
│   │   │   ├── Vertical Box_Content
│   │   │   │   ├── Horizontal Box_Header
│   │   │   │   │   ├── Text_Title ("SHIP CUSTOMIZATION")
│   │   │   │   │   ├── Spacer
│   │   │   │   │   └── Button_Close (X)
│   │   │   │   └── Horizontal Box_MainContent
│   │   │   │       ├── WBP_PartSelectionPanel (Widget)
│   │   │   │       ├── WBP_PartDetailPanel (Widget)
│   │   │   │       └── WBP_StatsComparison (Widget)
│   │   └── WBP_UnlockDialog (Hidden by default)
```

### Style Specifications

**Canvas Panel:**
- Size: Fullscreen (0,0 to 1920,1080 anchored)
- Z-Order: 100

**Image_Background:**
- Material: M_UI_BackgroundBlur (Gaussian blur, strength 5)
- Color: #000000 with 60% opacity
- Anchors: Fill (0,0,1,1)

**Border_MainPanel:**
- Size: 90% of screen (5% margin on all sides)
- Anchors: Center (0.5, 0.5)
- Background: #1A1A1A
- Border Color: #00BCD4
- Border Width: 2px
- Padding: 16px

**Text_Title:**
- Font: Roboto Bold, 32pt (Desktop) / 64pt (VR)
- Color: #FFFFFF
- Shadow: 2px offset, 50% opacity

**Button_Close:**
- Size: 40px × 40px (Desktop) / 80px × 80px (VR)
- Normal: #424242
- Hovered: #F44336
- Pressed: #D32F2F

### Bindings Required

| Property | Binding | Type |
|----------|---------|------|
| Text_Title | PlayerLevel | FText (Format: "SHIP CUSTOMIZATION - Level {0}") |
| Visibility (UnlockDialog) | bShowingUnlockDialog | ESlateVisibility |

### Event Handlers

```cpp
// Button_Close clicked
UFUNCTION()
void OnCloseButtonClicked();

// Child widget events
UFUNCTION()
void OnPartSelected(UShipPartData* PartData);

UFUNCTION()
void OnEquipPartRequested(UShipPartData* PartData);

UFUNCTION()
void OnUnlockPartRequested(UShipPartData* PartData);

UFUNCTION()
void OnApplyChangesClicked();

// Initialize
UFUNCTION(BlueprintCallable, Category = "Customization")
void InitializeMenu(UShipCustomizationSubsystem* Subsystem);
```

### Animations

**Anim_Open:**
- Duration: 0.3s
- Fade from 0 to 1 opacity
- Scale from 0.95 to 1.0
- Easing: Ease Out Cubic

**Anim_Close:**
- Duration: 0.2s
- Fade from 1 to 0 opacity
- Scale from 1.0 to 0.95
- Easing: Ease In Cubic

### VR Adaptations

- Widget Component attached to VR hand (left hand)
- Interaction mode: World Space Widget
- Draw size: 200cm × 150cm
- Distance from hand: 50cm
- All font sizes doubled
- Button hit boxes doubled
- Scrolling: Laser pointer or direct touch

---

## Widget 2: WBP_PartSelectionPanel

### Purpose
Left panel for browsing ship parts by category with filtering options.

### Layout Diagram

```
┌─────────────────────┐
│   PART SELECTION    │
├─────────────────────┤
│ [Engine] [Weapon]   │
│ [Shield] [Hull]     │
│ [Thruster] [Wings]  │
│ [Utility] [Skins]   │
├─────────────────────┤
│ 🔍 [Search...]      │
├─────────────────────┤
│ Rarity: [All ▼]     │
├─────────────────────┤
│ ┌─────────────────┐ │
│ │ ☑ Part Name 1  │ │
│ │ ☐ Part Name 2  │ │
│ │ 🔒 Part Name 3  │ │
│ │ ☐ Part Name 4  │ │
│ │                │ │
│ │   (scrollable) │ │
│ │                │ │
│ └─────────────────┘ │
└─────────────────────┘
```

### Component Hierarchy

```
WBP_PartSelectionPanel (User Widget)
├── Border_Container
│   ├── Vertical Box_Main
│   │   ├── Text_Header ("PART SELECTION")
│   │   ├── Uniform Grid Panel_CategoryTabs
│   │   │   ├── Button_Engine (0,0)
│   │   │   ├── Button_Weapon (0,1)
│   │   │   ├── Button_Shield (1,0)
│   │   │   ├── Button_Hull (1,1)
│   │   │   ├── Button_Thruster (2,0)
│   │   │   ├── Button_Wings (2,1)
│   │   │   ├── Button_Utility (3,0)
│   │   │   └── Button_Skins (3,1)
│   │   ├── Horizontal Box_SearchBox
│   │   │   ├── Image_SearchIcon
│   │   │   └── Editable TextBox_Search
│   │   ├── Horizontal Box_RarityFilter
│   │   │   ├── Text_RarityLabel ("Rarity:")
│   │   │   └── ComboBox_Rarity
│   │   └── Scroll Box_PartsList
│   │       └── Vertical Box_PartsContainer (dynamic)
│   │           └── WBP_PartListEntry[] (generated)
```

### Style Specifications

**Border_Container:**
- Background: #2A2A2A
- Border: #424242, 1px
- Padding: 12px
- Size: 300px width (Desktop) / 600px (VR)

**Text_Header:**
- Font: Roboto SemiBold, 24pt / 48pt (VR)
- Color: #00BCD4
- Margin: 0,0,0,12

**Button_CategoryTab (all):**
- Size: 140px × 36px (Desktop) / 280px × 72px (VR)
- Normal: #424242
- Hovered: #616161
- Pressed/Selected: #00BCD4
- Text Color Normal: #BDBDBD
- Text Color Selected: #FFFFFF
- Font: 14pt / 28pt (VR)

**Editable TextBox_Search:**
- Height: 32px (Desktop) / 64px (VR)
- Background: #1A1A1A
- Border: #616161
- Text Color: #FFFFFF
- Hint Text: "Search parts..."
- Font: 14pt / 28pt (VR)

**ComboBox_Rarity:**
- Options: "All", "Common", "Uncommon", "Rare", "Epic", "Legendary"
- Height: 32px (Desktop) / 64px (VR)
- Background: #1A1A1A
- Font: 14pt / 28pt (VR)

**Scroll Box_PartsList:**
- Always Show Scrollbar: False (Desktop) / True (VR)
- Scrollbar Thickness: 8px (Desktop) / 16px (VR)
- Orientation: Vertical

### Bindings Required

| Property | Binding | Type |
|----------|---------|------|
| PartsList Count | FilteredParts.Num() | int32 |
| SearchBox Text | SearchText | FText |
| Selected Category | CurrentCategory | EShipPartCategory |

### Event Handlers

```cpp
// Category tab clicks
UFUNCTION()
void OnCategoryTabClicked(EShipPartCategory Category);

// Search text changed
UFUNCTION()
void OnSearchTextChanged(const FText& Text);

// Rarity filter changed
UFUNCTION()
void OnRarityFilterChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

// Part list entry clicked
UFUNCTION()
void OnPartEntryClicked(UShipPartData* PartData);

// Refresh list
UFUNCTION(BlueprintCallable)
void RefreshPartsList();

// Initialize
UFUNCTION(BlueprintCallable)
void Initialize(UShipCustomizationSubsystem* Subsystem);
```

### Animations

**Anim_CategorySelect:**
- Duration: 0.15s
- Selected tab: Color tint to #00BCD4
- Deselected tabs: Color tint to #424242

**Anim_FilterApplied:**
- Duration: 0.2s
- Fade out old list (0.1s)
- Fade in new list (0.1s)

### VR Adaptations

- Category buttons arranged in 2×4 grid for easier targeting
- Search box can be opened as virtual keyboard overlay
- Rarity dropdown shows as radial menu on VR hand
- Part list entries have 80px minimum height
- Touch scrolling with momentum

---

## Widget 3: WBP_PartDetailPanel

### Purpose
Center panel showing detailed information about the currently selected ship part.

### Layout Diagram

```
┌─────────────────────────────┐
│      PART DETAILS           │
├─────────────────────────────┤
│                             │
│   ┌─────────────────────┐   │
│   │                     │   │
│   │   [PART IMAGE]      │   │
│   │                     │   │
│   └─────────────────────┘   │
│                             │
│  Quantum Drive MK-V [EPIC]  │
│                             │
│  Advanced propulsion system │
│  utilizing quantum field    │
│  manipulation for superior  │
│  speed and efficiency.      │
│                             │
│  STAT MODIFIERS:            │
│  ───────────────────        │
│  • Speed: +25%              │
│  • Fuel Efficiency: +15%    │
│  • Heat Generation: +10%    │
│                             │
│  REQUIREMENTS:              │
│  • Player Level: 10         │
│  • Hull Type: Medium+       │
│                             │
│  Cost: 1500 CR              │
│  Status: LOCKED             │
│                             │
│  [ UNLOCK ]  [ EQUIP ]      │
│                             │
└─────────────────────────────┘
```

### Component Hierarchy

```
WBP_PartDetailPanel (User Widget)
├── Border_Container
│   ├── Vertical Box_Main
│   │   ├── Text_Header ("PART DETAILS")
│   │   ├── Spacer (Height: 16)
│   │   ├── Border_ImageContainer
│   │   │   └── Image_PartIcon
│   │   ├── Spacer (Height: 12)
│   │   ├── Horizontal Box_PartName
│   │   │   ├── Text_PartName
│   │   │   ├── Spacer
│   │   │   └── Border_RarityBadge
│   │   │       └── Text_Rarity
│   │   ├── Spacer (Height: 12)
│   │   ├── Border_DescriptionBox
│   │   │   └── Text_Description (Wrap)
│   │   ├── Spacer (Height: 16)
│   │   ├── Text_StatsHeader ("STAT MODIFIERS:")
│   │   ├── Scroll Box_StatsList
│   │   │   └── Vertical Box_StatsContainer
│   │   │       └── Text_StatEntry[] (dynamic)
│   │   ├── Spacer (Height: 16)
│   │   ├── Text_RequirementsHeader ("REQUIREMENTS:")
│   │   ├── Vertical Box_Requirements
│   │   │   └── Text_Requirement[] (dynamic)
│   │   ├── Spacer
│   │   ├── Horizontal Box_CostStatus
│   │   │   ├── Image_CreditIcon
│   │   │   ├── Text_Cost
│   │   │   ├── Spacer
│   │   │   └── Text_Status
│   │   └── Horizontal Box_ActionButtons
│   │       ├── Button_Unlock
│   │       │   └── Text ("UNLOCK")
│   │       ├── Spacer (Width: 8)
│   │       └── Button_Equip
│   │           └── Text ("EQUIP")
```

### Style Specifications

**Border_Container:**
- Background: #2A2A2A
- Border: #424242, 1px
- Padding: 16px
- Size: Flexible (fill center area)

**Image_PartIcon:**
- Size: 256px × 256px (Desktop) / 512px × 512px (VR)
- Border: #424242, 2px
- Background: #1A1A1A
- Content: Part texture/material preview

**Text_PartName:**
- Font: Roboto Bold, 28pt / 56pt (VR)
- Color: #FFFFFF
- Shadow: 2px, 50%

**Border_RarityBadge:**
- Padding: 4,8,4,8
- Background: Rarity color (from palette)
- Corner Radius: 4px

**Text_Rarity:**
- Font: Roboto SemiBold, 14pt / 28pt (VR)
- Color: #FFFFFF

**Border_DescriptionBox:**
- Background: #1A1A1A
- Padding: 12px
- Max Height: 120px (Desktop) / 240px (VR)

**Text_Description:**
- Font: Roboto Regular, 14pt / 28pt (VR)
- Color: #BDBDBD
- Wrap: True

**Text_StatsHeader / Text_RequirementsHeader:**
- Font: Roboto SemiBold, 16pt / 32pt (VR)
- Color: #00BCD4

**Text_StatEntry:**
- Font: Roboto Regular, 14pt / 28pt (VR)
- Color: #4CAF50 (positive) / #F44336 (negative) / #BDBDBD (neutral)
- Format: "• {StatName}: {Value}"

**Text_Requirement:**
- Font: Roboto Regular, 14pt / 28pt (VR)
- Color: #4CAF50 (met) / #F44336 (not met)
- Format: "• {RequirementText}"

**Text_Cost:**
- Font: Roboto Bold, 18pt / 36pt (VR)
- Color: #FFC107

**Text_Status:**
- Font: Roboto SemiBold, 14pt / 28pt (VR)
- Color: #4CAF50 (unlocked) / #F44336 (locked) / #00BCD4 (equipped)

**Button_Unlock / Button_Equip:**
- Size: 140px × 44px (Desktop) / 280px × 88px (VR)
- Normal: #00BCD4
- Hovered: #00ACC1
- Pressed: #0097A7
- Disabled: #424242
- Text Color: #FFFFFF
- Font: 16pt / 32pt (VR)

### Bindings Required

| Property | Binding | Type |
|----------|---------|------|
| Image_PartIcon | SelectedPart->IconTexture | UTexture2D* |
| Text_PartName | SelectedPart->PartName | FText |
| Text_Rarity | SelectedPart->Rarity | FText |
| Border_RarityBadge Color | GetRarityColor() | FSlateColor |
| Text_Description | SelectedPart->Description | FText |
| Text_Cost | SelectedPart->UnlockCost | FText (Format: "{0} CR") |
| Text_Status | GetPartStatus() | FText |
| Button_Unlock Enabled | CanUnlockPart() | bool |
| Button_Equip Enabled | CanEquipPart() | bool |
| Button_Unlock Visibility | GetUnlockButtonVisibility() | ESlateVisibility |
| Button_Equip Visibility | GetEquipButtonVisibility() | ESlateVisibility |

### Event Handlers

```cpp
// Button clicks
UFUNCTION()
void OnUnlockButtonClicked();

UFUNCTION()
void OnEquipButtonClicked();

// Update display
UFUNCTION(BlueprintCallable)
void DisplayPart(UShipPartData* PartData);

UFUNCTION(BlueprintCallable)
void ClearDisplay();

// Helpers
UFUNCTION(BlueprintPure)
FLinearColor GetRarityColor() const;

UFUNCTION(BlueprintPure)
FText GetPartStatus() const;

UFUNCTION(BlueprintPure)
bool CanUnlockPart() const;

UFUNCTION(BlueprintPure)
bool CanEquipPart() const;

UFUNCTION(BlueprintPure)
ESlateVisibility GetUnlockButtonVisibility() const;

UFUNCTION(BlueprintPure)
ESlateVisibility GetEquipButtonVisibility() const;
```

### Animations

**Anim_PartChanged:**
- Duration: 0.3s
- Fade out content (0.15s)
- Update bindings (instant)
- Fade in content (0.15s)

**Anim_UnlockSuccess:**
- Duration: 0.5s
- Flash rarity badge (pulse scale 1.0 → 1.2 → 1.0)
- Update status text with color change

**Anim_EquipSuccess:**
- Duration: 0.4s
- Flash action buttons (green tint)
- Update status text

### VR Adaptations

- Part icon shows 3D preview instead of 2D texture
- Can rotate 3D preview with hand gestures
- Stat list scrollable with touch
- Larger button hit boxes
- Haptic feedback on button press

---

## Widget 4: WBP_PartListEntry

### Purpose
Reusable widget representing a single part in the parts list. Shows basic info and status.

### Layout Diagram

```
┌─────────────────────────────┐
│ [Icon] Part Name       [☑]  │  ← Equipped
└─────────────────────────────┘

┌─────────────────────────────┐
│ [Icon] Part Name       [ ]  │  ← Not Equipped
└─────────────────────────────┘

┌─────────────────────────────┐
│ [Icon] Part Name       [🔒] │  ← Locked
└─────────────────────────────┘
```

### Component Hierarchy

```
WBP_PartListEntry (User Widget)
├── Button_Main (Entire entry is clickable)
│   ├── Border_Container
│   │   ├── Horizontal Box_Content
│   │   │   ├── Border_IconContainer
│   │   │   │   └── Image_PartIcon
│   │   │   ├── Vertical Box_TextInfo
│   │   │   │   ├── Text_PartName
│   │   │   │   └── Text_PartSubtext (Rarity)
│   │   │   ├── Spacer
│   │   │   └── Overlay_StatusIndicator
│   │   │       ├── Image_EquippedCheckmark
│   │   │       └── Image_LockedIcon
```

### Style Specifications

**Button_Main:**
- Size: Fill width × 64px (Desktop) / Fill × 128px (VR)
- Normal: Transparent
- Hovered: #424242 (20% opacity)
- Pressed: #00BCD4 (30% opacity)
- Padding: 8px

**Border_Container:**
- Border Color: Rarity color (see palette)
- Border Width: 2px (left side only)
- Padding: 0

**Border_IconContainer:**
- Size: 48px × 48px (Desktop) / 96px × 96px (VR)
- Background: #1A1A1A
- Border: #424242, 1px

**Image_PartIcon:**
- Size: Fill
- Tint: White (unlocked) / #616161 (locked)

**Text_PartName:**
- Font: Roboto SemiBold, 16pt / 32pt (VR)
- Color: #FFFFFF (unlocked) / #9E9E9E (locked)

**Text_PartSubtext:**
- Font: Roboto Regular, 12pt / 24pt (VR)
- Color: Rarity color

**Image_EquippedCheckmark:**
- Size: 24px × 24px (Desktop) / 48px × 48px (VR)
- Tint: #4CAF50
- Visibility: Visible if equipped, else Hidden

**Image_LockedIcon:**
- Size: 24px × 24px (Desktop) / 48px × 48px (VR)
- Tint: #F44336
- Visibility: Visible if locked, else Hidden

### Bindings Required

| Property | Binding | Type |
|----------|---------|------|
| Image_PartIcon | PartData->IconTexture | UTexture2D* |
| Text_PartName | PartData->PartName | FText |
| Text_PartSubtext | PartData->Rarity | FText |
| Border_Container.BorderColor | GetRarityColor() | FSlateColor |
| Image_EquippedCheckmark.Visibility | bIsEquipped | ESlateVisibility |
| Image_LockedIcon.Visibility | bIsLocked | ESlateVisibility |
| Image_PartIcon.Tint | GetIconTint() | FSlateColor |
| Text_PartName.Color | GetNameColor() | FSlateColor |

### Event Handlers

```cpp
// Button clicked
UFUNCTION()
void OnButtonClicked();

// Initialize entry
UFUNCTION(BlueprintCallable)
void SetPartData(UShipPartData* InPartData, bool bInIsEquipped, bool bInIsLocked);

// Update status
UFUNCTION(BlueprintCallable)
void UpdateStatus(bool bInIsEquipped, bool bInIsLocked);

// Helpers
UFUNCTION(BlueprintPure)
FLinearColor GetRarityColor() const;

UFUNCTION(BlueprintPure)
FSlateColor GetIconTint() const;

UFUNCTION(BlueprintPure)
FSlateColor GetNameColor() const;

// Events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartEntryClicked, UShipPartData*, PartData);
UPROPERTY(BlueprintAssignable)
FOnPartEntryClicked OnPartClicked;
```

### Animations

**Anim_Hover:**
- Duration: 0.1s
- Scale: 1.0 → 1.02
- Background opacity: 0 → 20%

**Anim_Unhover:**
- Duration: 0.1s
- Scale: 1.02 → 1.0
- Background opacity: 20% → 0%

**Anim_Click:**
- Duration: 0.05s
- Scale: 1.0 → 0.98 → 1.0

**Anim_StatusChanged:**
- Duration: 0.3s
- Flash border color
- Fade status icon

### VR Adaptations

- Full height 128px for easier selection
- Hover state triggered by laser pointer proximity
- Haptic pulse on hover
- Selected state has distinct visual feedback (glow)

---

## Widget 5: WBP_StatsComparison

### Purpose
Right panel showing before/after comparison of ship stats when equipping different parts.

### Layout Diagram

```
┌─────────────────────────────┐
│    STATS COMPARISON         │
├─────────────────────────────┤
│                             │
│  CURRENT LOADOUT            │
│  ────────────────           │
│  Speed:        100          │
│  Armor:         80          │
│  Shields:       50          │
│  Handling:      75          │
│  Firepower:     60          │
│                             │
│  WITH CHANGES               │
│  ────────────────           │
│  Speed:        120  ↑ +20   │
│  Armor:         75  ↓ -5    │
│  Shields:       50  - 0     │
│  Handling:      80  ↑ +5    │
│  Firepower:     75  ↑ +15   │
│                             │
│  TOTAL CHANGES              │
│  ────────────────           │
│  + Improved Speed (+20%)    │
│  + Improved Handling (+7%)  │
│  + Improved Firepower (+25%)│
│  - Reduced Armor (-6%)      │
│                             │
│  [  APPLY CHANGES  ]        │
│                             │
└─────────────────────────────┘
```

### Component Hierarchy

```
WBP_StatsComparison (User Widget)
├── Border_Container
│   ├── Vertical Box_Main
│   │   ├── Text_Header ("STATS COMPARISON")
│   │   ├── Spacer (Height: 16)
│   │   ├── Text_CurrentHeader ("CURRENT LOADOUT")
│   │   ├── Vertical Box_CurrentStats
│   │   │   ├── HorizontalBox_Speed
│   │   │   │   ├── Text_StatLabel ("Speed:")
│   │   │   │   ├── Spacer
│   │   │   │   └── Text_StatValue
│   │   │   ├── HorizontalBox_Armor
│   │   │   ├── HorizontalBox_Shields
│   │   │   ├── HorizontalBox_Handling
│   │   │   └── HorizontalBox_Firepower
│   │   ├── Spacer (Height: 16)
│   │   ├── Text_NewHeader ("WITH CHANGES")
│   │   ├── Vertical Box_NewStats
│   │   │   ├── HorizontalBox_Speed
│   │   │   │   ├── Text_StatLabel ("Speed:")
│   │   │   │   ├── Spacer
│   │   │   │   ├── Text_NewValue
│   │   │   │   ├── Image_Arrow (↑/↓/-)
│   │   │   │   └── Text_Difference
│   │   │   ├── HorizontalBox_Armor
│   │   │   ├── HorizontalBox_Shields
│   │   │   ├── HorizontalBox_Handling
│   │   │   └── HorizontalBox_Firepower
│   │   ├── Spacer (Height: 16)
│   │   ├── Text_SummaryHeader ("TOTAL CHANGES")
│   │   ├── Scroll Box_Summary
│   │   │   └── Vertical Box_SummaryList
│   │   │       └── Text_SummaryEntry[] (dynamic)
│   │   ├── Spacer
│   │   └── Button_ApplyChanges
│   │       └── Text ("APPLY CHANGES")
```

### Style Specifications

**Border_Container:**
- Background: #2A2A2A
- Border: #424242, 1px
- Padding: 16px
- Size: 300px width (Desktop) / 600px (VR)

**Text_Header:**
- Font: Roboto SemiBold, 24pt / 48pt (VR)
- Color: #00BCD4

**Text_CurrentHeader / Text_NewHeader / Text_SummaryHeader:**
- Font: Roboto SemiBold, 16pt / 32pt (VR)
- Color: #FFFFFF
- Underline: 1px, #616161

**Text_StatLabel:**
- Font: Roboto Regular, 14pt / 28pt (VR)
- Color: #BDBDBD
- Width: 120px

**Text_StatValue / Text_NewValue:**
- Font: Roboto Bold, 16pt / 32pt (VR)
- Color: #FFFFFF
- Alignment: Right

**Image_Arrow:**
- Size: 16px × 16px (Desktop) / 32px × 32px (VR)
- Texture: Arrow up (↑) / Arrow down (↓) / Dash (-)
- Tint: #4CAF50 (up) / #F44336 (down) / #9E9E9E (neutral)

**Text_Difference:**
- Font: Roboto SemiBold, 14pt / 28pt (VR)
- Color: #4CAF50 (positive) / #F44336 (negative) / #9E9E9E (zero)
- Format: "+{value}" or "-{value}"

**Text_SummaryEntry:**
- Font: Roboto Regular, 14pt / 28pt (VR)
- Color: #4CAF50 (improvement) / #F44336 (penalty)
- Format: "+ Improved {Stat} (+{percent}%)" or "- Reduced {Stat} (-{percent}%)"

**Button_ApplyChanges:**
- Size: Fill × 48px (Desktop) / Fill × 96px (VR)
- Normal: #4CAF50
- Hovered: #45A049
- Pressed: #388E3C
- Disabled: #424242
- Text Color: #FFFFFF
- Font: 18pt / 36pt (VR), Bold

### Bindings Required

| Property | Binding | Type |
|----------|---------|------|
| Current Stats Values | CurrentLoadoutStats | FShipStats |
| New Stats Values | PreviewLoadoutStats | FShipStats |
| Difference Values | Calculated (New - Current) | FShipStats |
| Arrow Images | GetArrowDirection(StatType) | UTexture2D* |
| Summary List | GenerateSummaryList() | TArray<FText> |
| Button_ApplyChanges Enabled | bHasPendingChanges | bool |

### Event Handlers

```cpp
// Apply button
UFUNCTION()
void OnApplyChangesClicked();

// Update displays
UFUNCTION(BlueprintCallable)
void UpdateComparison(const FShipStats& Current, const FShipStats& Preview);

UFUNCTION(BlueprintCallable)
void RefreshDisplay();

UFUNCTION(BlueprintCallable)
void ClearComparison();

// Helpers
UFUNCTION(BlueprintPure)
FLinearColor GetDifferenceColor(float Difference) const;

UFUNCTION(BlueprintPure)
UTexture2D* GetArrowTexture(float Difference) const;

UFUNCTION(BlueprintPure)
TArray<FText> GenerateSummaryList() const;

// Event dispatchers
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnApplyChangesClicked);
UPROPERTY(BlueprintAssignable)
FOnApplyChangesClicked OnApplyChanges;
```

### Animations

**Anim_StatsUpdated:**
- Duration: 0.4s
- Number counter animation (interpolate from old to new value)
- Arrow fade in with scale

**Anim_PositiveChange:**
- Duration: 0.5s
- Green glow pulse on improved stats

**Anim_NegativeChange:**
- Duration: 0.5s
- Red glow pulse on reduced stats

**Anim_ApplySuccess:**
- Duration: 0.6s
- Button flash green
- Entire panel briefly highlighted

### VR Adaptations

- Stat bars can be visualized as 3D progress bars
- Color coding more prominent for easier readability
- Summary list uses larger icons
- Apply button larger and centered

---

## Widget 6: WBP_LoadoutPanel

### Purpose
Manages saved loadout presets, allowing quick switching between configurations.

### Layout Diagram

```
┌─────────────────────────────┐
│    LOADOUT MANAGEMENT       │
├─────────────────────────────┤
│                             │
│  SAVED LOADOUTS             │
│                             │
│  ┌───────────────────────┐  │
│  │ Slot 1: Speed Build   │  │
│  │ [LOAD] [DELETE]       │  │
│  └───────────────────────┘  │
│                             │
│  ┌───────────────────────┐  │
│  │ Slot 2: Tank Build    │  │
│  │ [LOAD] [DELETE]       │  │
│  └───────────────────────┘  │
│                             │
│  ┌───────────────────────┐  │
│  │ Slot 3: (Empty)       │  │
│  │ [SAVE]                │  │
│  └───────────────────────┘  │
│                             │
│  ┌───────────────────────┐  │
│  │ Slot 4: (Empty)       │  │
│  │ [SAVE]                │  │
│  └───────────────────────┘  │
│                             │
│  ┌───────────────────────┐  │
│  │ Slot 5: (Empty)       │  │
│  │ [SAVE]                │  │
│  └───────────────────────┘  │
│                             │
│  [ SAVE CURRENT LOADOUT ]   │
│                             │
└─────────────────────────────┘
```

### Component Hierarchy

```
WBP_LoadoutPanel (User Widget)
├── Border_Container
│   ├── Vertical Box_Main
│   │   ├── Text_Header ("LOADOUT MANAGEMENT")
│   │   ├── Spacer (Height: 12)
│   │   ├── Text_SubHeader ("SAVED LOADOUTS")
│   │   ├── Scroll Box_LoadoutsList
│   │   │   └── Vertical Box_LoadoutsContainer
│   │   │       ├── WBP_LoadoutSlot (Slot 1)
│   │   │       ├── WBP_LoadoutSlot (Slot 2)
│   │   │       ├── WBP_LoadoutSlot (Slot 3)
│   │   │       ├── WBP_LoadoutSlot (Slot 4)
│   │   │       └── WBP_LoadoutSlot (Slot 5)
│   │   ├── Spacer (Height: 16)
│   │   └── Button_SaveCurrent
│   │       └── Text ("SAVE CURRENT LOADOUT")
```

### Sub-Widget: WBP_LoadoutSlot

```
WBP_LoadoutSlot (User Widget)
├── Border_SlotContainer
│   ├── Vertical Box_Main
│   │   ├── Horizontal Box_NameRow
│   │   │   ├── Text_SlotNumber ("Slot 1:")
│   │   │   └── Editable Text_LoadoutName
│   │   ├── Text_LoadoutSummary (e.g., "Speed: 120, Armor: 75...")
│   │   └── Horizontal Box_Actions
│   │       ├── Button_Load
│   │       │   └── Text ("LOAD")
│   │       ├── Spacer (Width: 8)
│   │       ├── Button_Save
│   │       │   └── Text ("SAVE")
│   │       └── Button_Delete
│   │           └── Text ("DELETE")
```

### Style Specifications

**Border_Container:**
- Background: #2A2A2A
- Border: #424242, 1px
- Padding: 16px
- Size: 300px width (Desktop) / 600px (VR)

**Text_Header:**
- Font: Roboto SemiBold, 24pt / 48pt (VR)
- Color: #00BCD4

**Border_SlotContainer:**
- Background: #1A1A1A
- Border: #616161, 1px
- Padding: 12px
- Margin: 0,0,0,8

**Text_SlotNumber:**
- Font: Roboto SemiBold, 14pt / 28pt (VR)
- Color: #9E9E9E
- Width: 60px

**Editable Text_LoadoutName:**
- Font: Roboto Regular, 16pt / 32pt (VR)
- Color: #FFFFFF (has data) / #616161 (empty)
- Hint Text: "(Empty)"
- Max Length: 20 characters

**Text_LoadoutSummary:**
- Font: Roboto Regular, 12pt / 24pt (VR)
- Color: #BDBDBD
- Format: "Speed: {0}, Armor: {1}, Shields: {2}"

**Button_Load:**
- Size: 80px × 32px (Desktop) / 160px × 64px (VR)
- Normal: #00BCD4
- Hovered: #00ACC1
- Pressed: #0097A7
- Disabled: #424242

**Button_Save:**
- Size: 80px × 32px (Desktop) / 160px × 64px (VR)
- Normal: #4CAF50
- Hovered: #45A049
- Pressed: #388E3C
- Disabled: #424242

**Button_Delete:**
- Size: 80px × 32px (Desktop) / 160px × 64px (VR)
- Normal: #F44336
- Hovered: #E53935
- Pressed: #D32F2F
- Disabled: #424242

**Button_SaveCurrent:**
- Size: Fill × 48px (Desktop) / Fill × 96px (VR)
- Normal: #4CAF50
- Hovered: #45A049
- Font: 16pt / 32pt (VR), Bold

### Bindings Required

| Property | Binding | Type |
|----------|---------|------|
| LoadoutName | SavedLoadouts[Index].Name | FText |
| LoadoutSummary | SavedLoadouts[Index].GetSummary() | FText |
| Button_Load Visibility | HasData ? Visible : Collapsed | ESlateVisibility |
| Button_Save Visibility | !HasData ? Visible : Collapsed | ESlateVisibility |
| Button_Delete Visibility | HasData ? Visible : Collapsed | ESlateVisibility |

### Event Handlers

```cpp
// Slot actions
UFUNCTION()
void OnLoadButtonClicked(int32 SlotIndex);

UFUNCTION()
void OnSaveButtonClicked(int32 SlotIndex);

UFUNCTION()
void OnDeleteButtonClicked(int32 SlotIndex);

UFUNCTION()
void OnLoadoutNameChanged(const FText& NewName, int32 SlotIndex);

// Main button
UFUNCTION()
void OnSaveCurrentClicked();

// Refresh
UFUNCTION(BlueprintCallable)
void RefreshLoadoutsList();

// Initialize
UFUNCTION(BlueprintCallable)
void Initialize(UShipCustomizationSubsystem* Subsystem);

// Event dispatchers
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadoutLoaded, int32, SlotIndex);
UPROPERTY(BlueprintAssignable)
FOnLoadoutLoaded OnLoadoutLoaded;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadoutSaved, int32, SlotIndex);
UPROPERTY(BlueprintAssignable)
FOnLoadoutSaved OnLoadoutSaved;
```

### Animations

**Anim_LoadSuccess:**
- Duration: 0.4s
- Flash loaded slot green
- Scale pulse 1.0 → 1.05 → 1.0

**Anim_SaveSuccess:**
- Duration: 0.4s
- Flash saved slot green
- Fade in loadout summary

**Anim_DeleteConfirm:**
- Duration: 0.3s
- Flash slot red
- Fade out content
- Replace with empty state

### VR Adaptations

- Loadout slots taller for easier interaction
- Virtual keyboard for renaming
- Confirmation dialog for delete action
- Haptic feedback on save/load

---

## Widget 7: WBP_ProgressionDisplay

### Purpose
HUD element showing player level, XP progress, and credits balance.

### Layout Diagram

```
┌─────────────────────────────┐
│     ╭─────╮                 │
│     │ 15  │   XP: 2500/5000 │
│     ╰─────╯   ████████▓▓▓▓  │
│               Credits: 12,450│
└─────────────────────────────┘
```

### Component Hierarchy

```
WBP_ProgressionDisplay (User Widget)
├── Overlay_Root
│   ├── Border_Background
│   │   ├── Horizontal Box_Main
│   │   │   ├── Overlay_LevelCircle
│   │   │   │   ├── Image_LevelBackground (Circle)
│   │   │   │   ├── Circular Progress Bar_LevelProgress
│   │   │   │   └── Text_Level
│   │   │   ├── Vertical Box_ProgressInfo
│   │   │   │   ├── Horizontal Box_XPRow
│   │   │   │   │   ├── Text_XPLabel ("XP:")
│   │   │   │   │   └── Text_XPValue ("2500/5000")
│   │   │   │   ├── Progress Bar_XP
│   │   │   │   └── Horizontal Box_CreditsRow
│   │   │   │       ├── Image_CreditIcon
│   │   │   │       ├── Text_CreditsLabel ("Credits:")
│   │   │   │       └── Text_CreditsValue
```

### Style Specifications

**Border_Background:**
- Background: #1A1A1A with 80% opacity
- Border: #00BCD4, 1px
- Padding: 12px
- Corner Radius: 8px

**Image_LevelBackground:**
- Size: 64px × 64px (Desktop) / 128px × 128px (VR)
- Tint: #00BCD4
- Material: M_CircleGradient

**Circular Progress Bar_LevelProgress:**
- Size: 64px × 64px (Desktop) / 128px × 128px (VR)
- Fill Color: #4CAF50
- Background Color: #424242
- Thickness: 4px

**Text_Level:**
- Font: Roboto Bold, 24pt / 48pt (VR)
- Color: #FFFFFF
- Alignment: Center

**Text_XPLabel:**
- Font: Roboto SemiBold, 14pt / 28pt (VR)
- Color: #9E9E9E

**Text_XPValue:**
- Font: Roboto Regular, 14pt / 28pt (VR)
- Color: #FFFFFF

**Progress Bar_XP:**
- Size: 200px × 8px (Desktop) / 400px × 16px (VR)
- Fill Color: #00BCD4
- Background Color: #424242
- Corner Radius: 4px

**Image_CreditIcon:**
- Size: 20px × 20px (Desktop) / 40px × 40px (VR)
- Texture: T_Icon_Credits

**Text_CreditsValue:**
- Font: Roboto Bold, 16pt / 32pt (VR)
- Color: #FFC107
- Format: "{0:N0}" (with thousands separator)

### Bindings Required

| Property | Binding | Type |
|----------|---------|------|
| Text_Level | PlayerLevel | FText |
| Circular Progress Bar Fill | XP / NextLevelXP | float (0-1) |
| Text_XPValue | Format("{0}/{1}", XP, NextLevelXP) | FText |
| Progress Bar_XP Percent | XP / NextLevelXP | float (0-1) |
| Text_CreditsValue | Credits | FText |

### Event Handlers

```cpp
// Update displays
UFUNCTION(BlueprintCallable)
void UpdateProgression(int32 Level, int32 CurrentXP, int32 NextLevelXP);

UFUNCTION(BlueprintCallable)
void UpdateCredits(int32 NewCredits);

UFUNCTION(BlueprintCallable)
void PlayLevelUpAnimation();

UFUNCTION(BlueprintCallable)
void PlayCreditsGainedAnimation(int32 AmountGained);

UFUNCTION(BlueprintCallable)
void PlayCreditsSpentAnimation(int32 AmountSpent);
```

### Animations

**Anim_LevelUp:**
- Duration: 1.0s
- Level circle: Scale pulse (1.0 → 1.3 → 1.0)
- Level circle: Glow effect (expand and fade)
- XP bar: Fill to 100%, then reset to new value
- Play particle effect

**Anim_XPGained:**
- Duration: 0.5s
- XP bar: Smooth fill interpolation
- Flash green

**Anim_CreditsGained:**
- Duration: 0.6s
- Number counter: Interpolate from old to new
- Flash green
- +{amount} floats up and fades

**Anim_CreditsSpent:**
- Duration: 0.4s
- Number counter: Interpolate from old to new
- Flash red briefly
- -{amount} floats up and fades

### VR Adaptations

- Can be attached to wrist (watch-style)
- Follows player gaze when in menu
- Larger icons and text
- Animated glow effects more prominent

---

## Widget 8: WBP_UnlockDialog

### Purpose
Modal confirmation dialog for purchasing/unlocking ship parts.

### Layout Diagram

```
┌─────────────────────────────┐
│  UNLOCK PART?               │
├─────────────────────────────┤
│                             │
│   ┌─────────────┐           │
│   │   [IMAGE]   │           │
│   └─────────────┘           │
│                             │
│  Quantum Drive MK-V [EPIC]  │
│                             │
│  This will unlock the part  │
│  for use in customization.  │
│                             │
│  Cost: 1500 CR              │
│  Your Balance: 2450 CR      │
│  After Purchase: 950 CR     │
│                             │
│  [  CANCEL  ]  [ CONFIRM ]  │
│                             │
└─────────────────────────────┘
```

### Component Hierarchy

```
WBP_UnlockDialog (User Widget)
├── Overlay_Root
│   ├── Border_ModalBackground (Full screen, semi-transparent)
│   └── Border_DialogBox
│       ├── Vertical Box_Main
│       │   ├── Text_Header ("UNLOCK PART?")
│       │   ├── Spacer (Height: 16)
│       │   ├── Border_ImageContainer
│       │   │   └── Image_PartIcon
│       │   ├── Spacer (Height: 12)
│       │   ├── Horizontal Box_PartName
│       │   │   ├── Text_PartName
│       │   │   └── Border_RarityBadge
│       │   │       └── Text_Rarity
│       │   ├── Spacer (Height: 16)
│       │   ├── Text_ConfirmationMessage
│       │   ├── Spacer (Height: 16)
│       │   ├── Vertical Box_CostInfo
│       │   │   ├── Horizontal Box_Cost
│       │   │   │   ├── Text_CostLabel ("Cost:")
│       │   │   │   ├── Spacer
│       │   │   │   └── Text_CostValue
│       │   │   ├── Horizontal Box_Balance
│       │   │   │   ├── Text_BalanceLabel ("Your Balance:")
│       │   │   │   ├── Spacer
│       │   │   │   └── Text_BalanceValue
│       │   │   └── Horizontal Box_AfterPurchase
│       │   │       ├── Text_AfterLabel ("After Purchase:")
│       │   │       ├── Spacer
│       │   │       └── Text_AfterValue
│       │   ├── Spacer (Height: 24)
│       │   └── Horizontal Box_Buttons
│       │       ├── Button_Cancel
│       │       │   └── Text ("CANCEL")
│       │       ├── Spacer (Width: 16)
│       │       └── Button_Confirm
│       │           └── Text ("CONFIRM")
```

### Style Specifications

**Border_ModalBackground:**
- Size: Fullscreen
- Background: #000000 with 70% opacity
- Z-Order: 200

**Border_DialogBox:**
- Size: 500px × 600px (Desktop) / 1000px × 1200px (VR)
- Anchors: Center
- Background: #1A1A1A
- Border: #00BCD4, 2px
- Border Glow: #00BCD4 with 50% opacity, 8px blur
- Padding: 24px
- Corner Radius: 8px

**Text_Header:**
- Font: Roboto Bold, 28pt / 56pt (VR)
- Color: #00BCD4
- Alignment: Center

**Image_PartIcon:**
- Size: 160px × 160px (Desktop) / 320px × 320px (VR)
- Border: #424242, 2px

**Text_PartName:**
- Font: Roboto Bold, 24pt / 48pt (VR)
- Color: #FFFFFF

**Border_RarityBadge:**
- Background: Rarity color
- Padding: 4,8,4,8
- Corner Radius: 4px

**Text_ConfirmationMessage:**
- Font: Roboto Regular, 14pt / 28pt (VR)
- Color: #BDBDBD
- Alignment: Center
- Wrap: True

**Text_CostLabel / Text_BalanceLabel / Text_AfterLabel:**
- Font: Roboto SemiBold, 16pt / 32pt (VR)
- Color: #9E9E9E

**Text_CostValue:**
- Font: Roboto Bold, 16pt / 32pt (VR)
- Color: #FFC107

**Text_BalanceValue:**
- Font: Roboto Bold, 16pt / 32pt (VR)
- Color: #FFFFFF

**Text_AfterValue:**
- Font: Roboto Bold, 16pt / 32pt (VR)
- Color: #4CAF50 (if sufficient) / #F44336 (if insufficient)

**Button_Cancel:**
- Size: 140px × 48px (Desktop) / 280px × 96px (VR)
- Normal: #616161
- Hovered: #757575
- Pressed: #424242
- Text Color: #FFFFFF
- Font: 16pt / 32pt (VR), Bold

**Button_Confirm:**
- Size: 140px × 48px (Desktop) / 280px × 96px (VR)
- Normal: #4CAF50
- Hovered: #45A049
- Pressed: #388E3C
- Disabled: #424242
- Text Color: #FFFFFF
- Font: 16pt / 32pt (VR), Bold

### Bindings Required

| Property | Binding | Type |
|----------|---------|------|
| Image_PartIcon | PartToUnlock->IconTexture | UTexture2D* |
| Text_PartName | PartToUnlock->PartName | FText |
| Text_Rarity | PartToUnlock->Rarity | FText |
| Border_RarityBadge Color | GetRarityColor() | FSlateColor |
| Text_CostValue | PartToUnlock->UnlockCost | FText |
| Text_BalanceValue | PlayerCredits | FText |
| Text_AfterValue | PlayerCredits - UnlockCost | FText |
| Text_AfterValue Color | GetAffordabilityColor() | FSlateColor |
| Button_Confirm Enabled | CanAfford() | bool |

### Event Handlers

```cpp
// Button clicks
UFUNCTION()
void OnCancelClicked();

UFUNCTION()
void OnConfirmClicked();

// Show/hide dialog
UFUNCTION(BlueprintCallable)
void ShowDialog(UShipPartData* PartData, int32 PlayerCredits);

UFUNCTION(BlueprintCallable)
void HideDialog();

// Helpers
UFUNCTION(BlueprintPure)
bool CanAfford() const;

UFUNCTION(BlueprintPure)
FLinearColor GetAffordabilityColor() const;

UFUNCTION(BlueprintPure)
FLinearColor GetRarityColor() const;

// Event dispatchers
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnlockConfirmed, UShipPartData*, PartData);
UPROPERTY(BlueprintAssignable)
FOnUnlockConfirmed OnUnlockConfirmed;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUnlockCancelled);
UPROPERTY(BlueprintAssignable)
FOnUnlockCancelled OnUnlockCancelled;
```

### Animations

**Anim_Show:**
- Duration: 0.3s
- Modal background: Fade from 0 to 70% opacity
- Dialog box: Scale from 0.8 to 1.0
- Dialog box: Fade from 0 to 1 opacity
- Easing: Ease Out Back (slight overshoot)

**Anim_Hide:**
- Duration: 0.2s
- Modal background: Fade from 70% to 0 opacity
- Dialog box: Scale from 1.0 to 0.8
- Dialog box: Fade from 1 to 0 opacity
- Easing: Ease In Cubic

**Anim_InsufficientFunds:**
- Duration: 0.4s
- After value text: Shake (horizontal oscillation)
- Confirm button: Flash red
- Play error sound

**Anim_PurchaseSuccess:**
- Duration: 0.6s
- Dialog box: Flash green border
- Part icon: Scale pulse
- Play success sound
- Auto-close after animation

### VR Adaptations

- Dialog appears in center of view
- Can be dismissed by looking away (gaze-based)
- Larger buttons for easier targeting
- Haptic feedback on button press
- Voice confirmation option ("Yes" / "No")

---

## Additional Implementation Notes

### Widget Communication Flow

```
WBP_CustomizationMenu (Parent)
    ├─> WBP_PartSelectionPanel
    │   └─> Fires: OnPartSelected
    │       └─> Parent updates: WBP_PartDetailPanel
    │           └─> Parent updates: WBP_StatsComparison
    │
    ├─> WBP_PartDetailPanel
    │   ├─> Fires: OnEquipPartRequested
    │   │   └─> Parent updates all panels
    │   └─> Fires: OnUnlockPartRequested
    │       └─> Parent shows: WBP_UnlockDialog
    │
    ├─> WBP_StatsComparison
    │   └─> Fires: OnApplyChangesClicked
    │       └─> Parent applies loadout changes
    │
    └─> WBP_UnlockDialog
        ├─> Fires: OnUnlockConfirmed
        │   └─> Parent processes unlock
        └─> Fires: OnUnlockCancelled
            └─> Parent hides dialog
```

### Performance Optimization

1. **Widget Pooling**: Reuse WBP_PartListEntry instances instead of destroying/creating
2. **Lazy Loading**: Only load part icons when visible in scroll box
3. **Update Throttling**: Limit stats comparison updates to 30 FPS
4. **Texture Streaming**: Use texture streaming for part icons (256x256 base, 512x512 VR)
5. **Animation Budgeting**: Max 3 simultaneous animations per widget

### Accessibility Features

1. **Screen Reader Support**: All interactive elements have accessible text
2. **Keyboard Navigation**: Tab order properly set for all buttons
3. **Color Blind Mode**: Alternative rarity indicators (icons + patterns)
4. **Scalable UI**: All sizes based on multipliers (1x, 1.5x, 2x options)
5. **Audio Cues**: Sound feedback for all button presses and state changes

### Localization Considerations

1. **Text Wrapping**: All text blocks set to auto-wrap
2. **Dynamic Sizing**: Buttons expand to fit localized text
3. **String Tables**: All displayed text uses string table keys
4. **RTL Support**: Layout can flip for right-to-left languages
5. **Font Fallbacks**: Support for non-Latin character sets

---

## Summary

This specification covers 8 comprehensive UMG widgets for the Ship Customization System:

1. **WBP_CustomizationMenu** - Main container
2. **WBP_PartSelectionPanel** - Part browser
3. **WBP_PartDetailPanel** - Part information
4. **WBP_PartListEntry** - Reusable list item
5. **WBP_StatsComparison** - Before/after stats
6. **WBP_LoadoutPanel** - Loadout management
7. **WBP_ProgressionDisplay** - HUD progression element
8. **WBP_UnlockDialog** - Purchase confirmation

All widgets include:
- Complete layout diagrams
- Component hierarchies
- Style specifications for desktop and VR
- Binding requirements
- Event handlers
- Animation specifications
- VR-specific adaptations

The design follows modern UI/UX best practices with special attention to VR usability, accessibility, and performance.
