# Ship Customization System - Widget Blueprints Creation Guide

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Project Setup](#project-setup)
3. [Creating Base Materials and Assets](#creating-base-materials-and-assets)
4. [Widget Creation Workflow](#widget-creation-workflow)
5. [Step-by-Step Widget Creation](#step-by-step-widget-creation)
6. [Widget Integration](#widget-integration)
7. [Testing and Debugging](#testing-and-debugging)
8. [VR-Specific Setup](#vr-specific-setup)

---

## Prerequisites

### Required Unreal Engine Version
- Unreal Engine 5.0 or higher
- VR Template installed (optional, but recommended)

### Required Plugins
- UMG (User Interface)
- Enhanced Input (for VR interaction)
- Steam VR / Oculus VR (for VR testing)

### Skill Requirements
- Basic understanding of UMG system
- Familiarity with Blueprint visual scripting
- Understanding of data binding concepts

---

## Project Setup

### Step 1: Create Folder Structure

1. Open Content Browser in Unreal Editor
2. Navigate to `Content/` folder
3. Create the following folder structure:

```
Content/
├── UI/
│   ├── Widgets/
│   │   ├── Customization/
│   │   │   ├── Main/
│   │   │   ├── Panels/
│   │   │   ├── Components/
│   │   │   └── Dialogs/
│   │   └── Common/
│   ├── Materials/
│   │   └── UI/
│   ├── Textures/
│   │   ├── Icons/
│   │   │   ├── Parts/
│   │   │   └── UI/
│   │   └── Backgrounds/
│   ├── Fonts/
│   └── WidgetSpecs/
├── Data/
│   └── Customization/
│       ├── Parts/
│       └── Presets/
└── Blueprints/
    └── Customization/
```

### Step 2: Import Required Assets

#### Fonts

1. Download Roboto font family from Google Fonts
2. Import `.ttf` files to `Content/UI/Fonts/`
3. Create Font assets for each weight:
   - `Font_Roboto_Regular`
   - `Font_Roboto_SemiBold`
   - `Font_Roboto_Bold`

#### Icons and Textures

1. Create or import the following textures:
   - `T_Icon_Credits.png` (32×32, 64×64 for VR)
   - `T_Icon_Lock.png`
   - `T_Icon_Checkmark.png`
   - `T_Arrow_Up.png`
   - `T_Arrow_Down.png`
   - `T_Arrow_Neutral.png`
   - `T_Icon_Search.png`

2. Import settings:
   - Compression: `UserInterface2D (RGBA)`
   - MipGenSettings: `NoMipmaps`
   - sRGB: `True`

---

## Creating Base Materials and Assets

### Material 1: M_UI_BackgroundBlur

**Purpose:** Creates a blurred background effect for the main customization menu.

**Steps:**

1. Right-click in `Content/UI/Materials/UI/` → Material → Name: `M_UI_BackgroundBlur`
2. Open the material editor
3. Set Material Domain: `User Interface`
4. Add nodes:

```
Scene Texture:PostProcessInput0
    └─> Gaussian Blur (Kernel Size: 5, Strength: 5.0)
        └─> Lerp (Alpha: 0.6)
            ├─> A: Blurred texture
            ├─> B: Constant (0,0,0) [Black]
            └─> Final Color
```

5. Set Blend Mode: `Translucent`
6. Save and close

### Material 2: M_CircleGradient

**Purpose:** Creates the circular gradient for the level indicator.

**Steps:**

1. Create new Material: `M_CircleGradient`
2. Set Material Domain: `User Interface`
3. Add nodes:

```
Radial Gradient Exponential
    ├─> CenterPosition: (0.5, 0.5)
    ├─> Radius: 0.4
    └─> Density: 5.0
        └─> Multiply by Parameter (Color: Cyan)
            └─> Final Color

Radial Gradient Exponential (same settings)
    └─> OneMinus
        └─> Clamp (0-1)
            └─> Opacity
```

4. Create Material Parameter: `TintColor` (Vector)
5. Save and close

### Widget Style 1: WidgetStyle_ButtonPrimary

**Purpose:** Reusable button style for primary actions.

**Steps:**

1. Right-click in `Content/UI/Widgets/Common/` → User Interface → Widget Blueprint → Name: `WS_ButtonPrimary`
2. In Style settings:
   - Normal:
     - Tint: `#00BCD4`
     - Padding: `8,16,8,16`
   - Hovered:
     - Tint: `#00ACC1`
   - Pressed:
     - Tint: `#0097A7`
   - Disabled:
     - Tint: `#424242`

3. Text Style:
   - Font: `Font_Roboto_Bold`
   - Size: `16`
   - Color: `#FFFFFF`

### Widget Style 2: WidgetStyle_ButtonSecondary

Similar to primary, but with gray color scheme (`#616161`, `#757575`, `#424242`).

---

## Widget Creation Workflow

### Standard Workflow for Each Widget

1. **Create Blueprint**
   - Right-click in appropriate folder
   - User Interface → Widget Blueprint
   - Name according to specification (e.g., `WBP_CustomizationMenu`)

2. **Design Layout**
   - Add root panel (Canvas/Overlay)
   - Add child components according to hierarchy
   - Set anchors and alignment
   - Configure sizing (Desktop first, VR later)

3. **Apply Styles**
   - Set colors, fonts, padding
   - Apply widget styles where applicable
   - Configure border styles

4. **Create Variables**
   - Add necessary member variables
   - Set default values
   - Mark appropriate variables as Instance Editable or Blueprint Read Only

5. **Implement Bindings**
   - Create binding functions for dynamic properties
   - Set up two-way bindings where needed

6. **Create Functions**
   - Implement initialization functions
   - Create update/refresh functions
   - Add helper/utility functions

7. **Implement Events**
   - Add event handlers for buttons
   - Create event dispatchers for parent communication
   - Wire up child widget events

8. **Create Animations**
   - Add animations in Animation tab
   - Configure animation tracks
   - Set easing curves

9. **Test**
   - Test in Widget Blueprint editor
   - Test in PIE (Play In Editor)
   - Test in VR (if applicable)

10. **Optimize**
    - Check invalidation settings
    - Set up widget pooling if needed
    - Optimize bindings (avoid per-frame updates)

---

## Step-by-Step Widget Creation

## Widget 1: WBP_CustomizationMenu

### Phase 1: Create Blueprint and Basic Layout

1. **Create the Widget Blueprint**
   - Navigate to `Content/UI/Widgets/Customization/Main/`
   - Right-click → User Interface → Widget Blueprint
   - Name: `WBP_CustomizationMenu`
   - Open the blueprint

2. **Set up Root Panel**
   - Hierarchy panel: Select and delete default Canvas Panel
   - Palette panel: Drag `Canvas Panel` to hierarchy
   - Rename to `Root_Canvas`
   - Details panel:
     - Is Variable: `True`
     - Anchors: `Fill Screen (0,0,1,1)`
     - Offsets: `0,0,0,0`

3. **Add Background Blur**
   - Drag `Image` onto `Root_Canvas`
   - Rename: `Image_Background`
   - Details:
     - Anchors: `Fill (0,0,1,1)`
     - Offsets: `0,0,0,0`
     - Brush → Image: Select `M_UI_BackgroundBlur` material
     - Brush → Tint: `#000000`, Alpha: `0.6`
     - Z-Order: `0`

4. **Add Main Container**
   - Drag `Overlay` onto `Root_Canvas`
   - Rename: `Overlay_MainContainer`
   - Details:
     - Anchors: `Center (0.5, 0.5)`
     - Alignment: `0.5, 0.5`
     - Size X: `1728` (90% of 1920)
     - Size Y: `972` (90% of 1080)
     - Z-Order: `1`

5. **Add Main Border Panel**
   - Drag `Border` onto `Overlay_MainContainer`
   - Rename: `Border_MainPanel`
   - Details:
     - Anchors: `Fill (0,0,1,1)`
     - Brush Color: `#1A1A1A`
     - Brush → Draw As: `Box`
     - Brush → Tint: `#00BCD4`
     - Padding: `16,16,16,16`
     - Border:
       - Brush Image: `WhiteSquareTexture`
       - Margins: `0.5` (creates 2px border at default DPI)

6. **Add Content Container**
   - Drag `Vertical Box` onto `Border_MainPanel`
   - Rename: `VBox_Content`
   - No special settings needed

7. **Add Header Row**
   - Drag `Horizontal Box` onto `VBox_Content`
   - Rename: `HBox_Header`
   - Details:
     - Slot → Size: `Auto`
     - Slot → Padding: `0,0,0,16`

8. **Add Title Text**
   - Drag `Text` onto `HBox_Header`
   - Rename: `Text_Title`
   - Details:
     - Text: `SHIP CUSTOMIZATION`
     - Font → Font Family: `Font_Roboto_Bold`
     - Font → Size: `32`
     - Color: `#FFFFFF`
     - Shadow Offset: `2,2`
     - Shadow Color: `#000000`, Alpha: `0.5`
     - Slot → Horizontal Alignment: `Left`
     - Slot → Vertical Alignment: `Center`

9. **Add Spacer**
   - Drag `Spacer` onto `HBox_Header`
   - Slot → Size: `Fill`, Size Rule: `1.0`

10. **Add Close Button**
    - Drag `Button` onto `HBox_Header`
    - Rename: `Button_Close`
    - Details:
      - Is Variable: `True` (for event binding)
      - Style → Normal → Tint: `#424242`
      - Style → Hovered → Tint: `#F44336`
      - Style → Pressed → Tint: `#D32F2F`
      - Slot → Size → Width Override: `40`
      - Slot → Size → Height Override: `40`

11. **Add Close Button Text**
    - Drag `Text` onto `Button_Close`
    - Rename: `Text_CloseX`
    - Details:
      - Text: `✕`
      - Font → Size: `24`
      - Color: `#FFFFFF`
      - Justification → Horizontal: `Center`
      - Justification → Vertical: `Center`

### Phase 2: Add Main Content Panels

12. **Add Main Content Horizontal Box**
    - Drag `Horizontal Box` onto `VBox_Content`
    - Rename: `HBox_MainContent`
    - Details:
      - Slot → Size: `Fill`, Size Rule: `1.0`

13. **Add Part Selection Panel (Placeholder)**
    - Drag `Border` onto `HBox_MainContent`
    - Rename: `Border_PartSelectionPlaceholder`
    - Details:
      - Brush Color: `#2A2A2A`
      - Slot → Size: `Auto`
      - Slot → Padding: `0,0,8,0`

14. **Add Sizing Box for Part Selection**
    - Drag `Size Box` onto `Border_PartSelectionPlaceholder`
    - Details:
      - Width Override: `300`
      - Height Override: `0` (let it fill)

15. **Add Part Detail Panel (Placeholder)**
    - Drag `Border` onto `HBox_MainContent`
    - Rename: `Border_PartDetailPlaceholder`
    - Details:
      - Brush Color: `#2A2A2A`
      - Slot → Size: `Fill`, Size Rule: `1.0`
      - Slot → Padding: `0,0,8,0`

16. **Add Stats Comparison Panel (Placeholder)**
    - Drag `Border` onto `HBox_MainContent`
    - Rename: `Border_StatsComparisonPlaceholder`
    - Details:
      - Brush Color: `#2A2A2A`
      - Slot → Size: `Auto`

17. **Add Sizing Box for Stats Comparison**
    - Drag `Size Box` onto `Border_StatsComparisonPlaceholder`
    - Details:
      - Width Override: `300`

### Phase 3: Add Variables

18. **Switch to Graph tab**

19. **Add Variables** (Variables panel, click + button):
    - `SubsystemReference` (Type: `ShipCustomizationSubsystem`, Object Reference)
    - `bIsOpen` (Type: `Boolean`, Default: `false`)
    - `CurrentlySelectedPart` (Type: `ShipPartData`, Object Reference)
    - `PendingLoadout` (Type: `ShipLoadout`, Default: `None`)

### Phase 4: Create Initialization Function

20. **Create Function: InitializeMenu**
    - Functions panel → + → Name: `InitializeMenu`
    - Add Input: `InSubsystem` (Type: `ShipCustomizationSubsystem`, Object Reference)

21. **Function Graph:**

```
InitializeMenu
    ├─> Set SubsystemReference (value: InSubsystem)
    ├─> Branch (Condition: IsValid(InSubsystem))
    │   ├─> True:
    │   │   ├─> Call WBP_PartSelectionPanel.Initialize(InSubsystem)
    │   │   ├─> Call WBP_PartDetailPanel.Initialize(InSubsystem)
    │   │   ├─> Call WBP_StatsComparison.Initialize(InSubsystem)
    │   │   └─> Play Animation (Anim_Open)
    │   └─> False:
    │       └─> Print String ("Error: Invalid Subsystem Reference")
```

### Phase 5: Create Event Handlers

22. **Create Function: OnCloseButtonClicked**
    - Select `Button_Close` in Designer
    - Details panel → Events → On Clicked → Add
    - This creates event in Event Graph

23. **Event Graph:**

```
OnCloseButtonClicked
    ├─> Play Animation (Anim_Close, Mode: Forward)
    ├─> Delay (0.2 seconds) [Match animation duration]
    └─> Remove from Parent
```

24. **Create Custom Event: ShowMenu**
    - Event Graph → Right-click → Add Custom Event
    - Name: `ShowMenu`

```
ShowMenu
    ├─> Add to Viewport (Z-Order: 100)
    ├─> Set bIsOpen = True
    ├─> Play Animation (Anim_Open)
    └─> Set Input Mode UI Only
        └─> Set Show Mouse Cursor = True
```

25. **Create Custom Event: HideMenu**

```
HideMenu
    ├─> Play Animation (Anim_Close)
    ├─> Delay (0.2)
    ├─> Remove from Parent
    ├─> Set bIsOpen = False
    └─> Set Input Mode Game Only
```

### Phase 6: Create Animations

26. **Create Animation: Anim_Open**
    - Animations panel → + Animation → Name: `Anim_Open`
    - Length: `0.3` seconds

27. **Add Animation Tracks:**
    - Click `+ Track` → Select `Overlay_MainContainer`
    - Add track: `Render Opacity`
      - Keyframe at 0.0s: Value = `0.0`
      - Keyframe at 0.3s: Value = `1.0`
      - Select both keys → Right-click → Interpolation → Cubic (Ease Out)

    - Add track: `Render Transform → Scale`
      - Keyframe at 0.0s: X=`0.95`, Y=`0.95`
      - Keyframe at 0.3s: X=`1.0`, Y=`1.0`
      - Interpolation: Cubic (Ease Out)

28. **Create Animation: Anim_Close**
    - Similar to Anim_Open, but reversed
    - Length: `0.2` seconds
    - Opacity: 1.0 → 0.0
    - Scale: 1.0 → 0.95
    - Interpolation: Cubic (Ease In)

### Phase 7: Compile and Test

29. **Compile the Blueprint**
    - Click "Compile" button (top toolbar)
    - Fix any errors shown in Compiler Results

30. **Test in Designer**
    - Switch to Designer tab
    - Click "Preview" dropdown → Change viewport size
    - Check 1920×1080, 2560×1440, 3840×2160

31. **Initial PIE Test**
    - Save blueprint
    - Create test level or use existing
    - Create Blueprint Actor or Game Mode to spawn widget
    - Play in Editor and verify appearance

---

## Widget 2: WBP_PartSelectionPanel

### Phase 1: Create Blueprint and Layout

1. **Create Widget Blueprint**
   - Location: `Content/UI/Widgets/Customization/Panels/`
   - Name: `WBP_PartSelectionPanel`

2. **Root Container**
   - Root: `Border`
   - Rename: `Border_Container`
   - Details:
     - Brush Color: `#2A2A2A`
     - Border Brush: `WhiteSquareTexture`
     - Border Color: `#424242`
     - Border Margins: `0.5`
     - Padding: `12,12,12,12`

3. **Main Vertical Box**
   - Add `Vertical Box` to `Border_Container`
   - Rename: `VBox_Main`

4. **Header Text**
   - Add `Text` to `VBox_Main`
   - Rename: `Text_Header`
   - Text: `PART SELECTION`
   - Font: `Font_Roboto_SemiBold`, Size: `24`
   - Color: `#00BCD4`
   - Slot → Padding: `0,0,0,12`

5. **Category Tabs Grid**
   - Add `Uniform Grid Panel` to `VBox_Main`
   - Rename: `Grid_CategoryTabs`
   - Slot → Padding: `0,0,0,12`

### Phase 2: Add Category Tab Buttons

6. **Create First Button (Template)**
   - Add `Button` to `Grid_CategoryTabs`
   - Rename: `Button_Engine`
   - Is Variable: `True`
   - Style:
     - Normal → Tint: `#424242`
     - Hovered → Tint: `#616161`
     - Pressed → Tint: `#00BCD4`
   - Slot:
     - Column: `0`, Row: `0`
     - Horizontal Alignment: `Fill`
     - Vertical Alignment: `Fill`

7. **Add Button Text**
   - Add `Text` to `Button_Engine`
   - Text: `ENGINE`
   - Font: `Font_Roboto_Regular`, Size: `14`
   - Color: `#BDBDBD`
   - Alignment: Center/Center

8. **Size Box for Button**
   - Wrap button in `Size Box`
   - Width Override: `140`
   - Height Override: `36`

9. **Duplicate for All Categories**
   - Copy `Button_Engine` (with Size Box)
   - Paste and arrange in grid:
     - Row 0: Engine (Col 0), Weapon (Col 1)
     - Row 1: Shield (Col 0), Hull (Col 1)
     - Row 2: Thruster (Col 0), Wings (Col 1)
     - Row 3: Utility (Col 0), Skins (Col 1)
   - Rename each button appropriately
   - Update text for each

### Phase 3: Add Search Box

10. **Search Container**
    - Add `Horizontal Box` to `VBox_Main`
    - Rename: `HBox_SearchBox`
    - Slot → Padding: `0,0,0,8`

11. **Search Icon**
    - Add `Image` to `HBox_SearchBox`
    - Rename: `Image_SearchIcon`
    - Brush → Image: `T_Icon_Search`
    - Brush → Tint: `#9E9E9E`
    - Slot → Size: `Auto`
    - Slot → Padding: `4,0,4,0`
    - Desired Size: `16,16`

12. **Search Text Box**
    - Add `Editable Text Box` to `HBox_SearchBox`
    - Rename: `TextBox_Search`
    - Is Variable: `True`
    - Hint Text: `Search parts...`
    - Font: `Font_Roboto_Regular`, Size: `14`
    - Foreground Color: `#FFFFFF`
    - Background Color: `#1A1A1A`
    - Padding: `8,8,8,8`
    - Slot → Size: `Fill`

### Phase 4: Add Rarity Filter

13. **Rarity Filter Container**
    - Add `Horizontal Box` to `VBox_Main`
    - Rename: `HBox_RarityFilter`
    - Slot → Padding: `0,0,0,12`

14. **Rarity Label**
    - Add `Text` to `HBox_RarityFilter`
    - Text: `Rarity:`
    - Font: Size `14`
    - Color: `#9E9E9E`
    - Slot → Padding: `0,0,8,0`
    - Slot → Vertical Alignment: Center

15. **Rarity Combo Box**
    - Add `Combo Box (String)` to `HBox_RarityFilter`
    - Rename: `ComboBox_Rarity`
    - Is Variable: `True`
    - Default Options: `All`, `Common`, `Uncommon`, `Rare`, `Epic`, `Legendary`
    - Font: Size `14`
    - Slot → Size: `Fill`

### Phase 5: Add Parts List

16. **Scroll Box**
    - Add `Scroll Box` to `VBox_Main`
    - Rename: `ScrollBox_PartsList`
    - Is Variable: `True`
    - Orientation: Vertical
    - Scroll Bar Visibility: `Auto`
    - Slot → Size: `Fill`

17. **Parts Container**
    - Add `Vertical Box` to `ScrollBox_PartsList`
    - Rename: `VBox_PartsContainer`
    - Is Variable: `True`
    - (This will be populated dynamically with WBP_PartListEntry widgets)

### Phase 6: Variables and Functions

18. **Add Variables:**
    - `SubsystemReference` (ShipCustomizationSubsystem, Object Reference)
    - `CurrentCategory` (Byte/Enum: EShipPartCategory)
    - `FilteredParts` (Array of ShipPartData references)
    - `SearchText` (Text)
    - `SelectedRarity` (String)
    - `PartEntryWidgets` (Array of WBP_PartListEntry references) - for pooling

19. **Create Function: Initialize**

```blueprint
Initialize (Input: SubsystemReference)
    ├─> Set SubsystemReference
    ├─> Set CurrentCategory = Engine
    ├─> Call RefreshPartsList()
    └─> Bind Search Text Changed event
```

20. **Create Function: RefreshPartsList**

```blueprint
RefreshPartsList
    ├─> Clear VBox_PartsContainer (Remove all children)
    ├─> Get All Parts from Subsystem
    ├─> Filter by CurrentCategory
    ├─> Filter by SearchText (if not empty)
    ├─> Filter by SelectedRarity (if not "All")
    ├─> Store result in FilteredParts
    ├─> ForEach Part in FilteredParts:
    │   ├─> Create WBP_PartListEntry
    │   ├─> Call Entry.SetPartData(Part, IsEquipped, IsLocked)
    │   ├─> Bind Entry.OnPartClicked to OnPartEntryClicked
    │   └─> Add to VBox_PartsContainer
    └─> Return
```

21. **Create Function: OnCategoryTabClicked**
    - Input: `Category` (EShipPartCategory)

```blueprint
OnCategoryTabClicked
    ├─> Set CurrentCategory = Category
    ├─> Update all button styles (selected vs unselected)
    ├─> Play Animation (Anim_CategorySelect)
    └─> Call RefreshPartsList()
```

22. **Create Event: OnPartEntryClicked**
    - Input: `PartData` (ShipPartData reference)

```blueprint
OnPartEntryClicked
    ├─> Call Event Dispatcher: OnPartSelected(PartData)
    └─> Update visual selection (highlight selected entry)
```

23. **Create Event Dispatcher:**
    - Name: `OnPartSelected`
    - Parameters: `PartData` (ShipPartData reference)

### Phase 7: Bind Events

24. **Bind Category Buttons**
    - For each category button (Engine, Weapon, etc.):
    - Designer → Select button → Details → Events → On Clicked
    - Call `OnCategoryTabClicked` with appropriate category enum

25. **Bind Search Text**
    - Select `TextBox_Search`
    - Events → On Text Changed
    - Call:
    ```blueprint
    OnSearchTextChanged
        ├─> Set SearchText = New Text
        ├─> Delay (0.3 seconds) [Debounce]
        └─> Call RefreshPartsList()
    ```

26. **Bind Rarity Filter**
    - Select `ComboBox_Rarity`
    - Events → On Selection Changed
    - Call:
    ```blueprint
    OnRaritySelectionChanged
        ├─> Set SelectedRarity = Selected Option
        └─> Call RefreshPartsList()
    ```

### Phase 8: Animations

27. **Create Anim_CategorySelect**
    - Duration: `0.15s`
    - Add track for selected button: Tint = `#00BCD4`
    - Add track for deselected buttons: Tint = `#424242`

28. **Compile and Test**

---

## Widget 3: WBP_PartListEntry

### Phase 1: Create Reusable Entry Widget

1. **Create Widget Blueprint**
   - Location: `Content/UI/Widgets/Customization/Components/`
   - Name: `WBP_PartListEntry`

2. **Root Button**
   - Root: `Button`
   - Rename: `Button_Main`
   - Is Variable: `True`
   - Style:
     - Normal → Tint: Transparent
     - Hovered → Tint: `#424242` with 20% alpha
     - Pressed → Tint: `#00BCD4` with 30% alpha
   - Padding: `8,8,8,8`

3. **Container Border**
   - Add `Border` to `Button_Main`
   - Rename: `Border_Container`
   - Border Brush → Image: `WhiteSquareTexture`
   - Border Brush → Margins: `0.05, 0, 0, 0` (left border only)
   - Border Brush → Draw As: `Border`
   - Padding: `0`

4. **Content Horizontal Box**
   - Add `Horizontal Box` to `Border_Container`
   - Rename: `HBox_Content`

5. **Icon Container**
   - Add `Border` to `HBox_Content`
   - Rename: `Border_IconContainer`
   - Size: `48×48`
   - Brush Color: `#1A1A1A`
   - Border Color: `#424242`
   - Slot → Padding: `0,0,8,0`

6. **Part Icon**
   - Add `Image` to `Border_IconContainer`
   - Rename: `Image_PartIcon`
   - Is Variable: `True`
   - Stretch: `Uniform`

7. **Text Container**
   - Add `Vertical Box` to `HBox_Content`
   - Rename: `VBox_TextInfo`
   - Slot → Size: `Fill`
   - Slot → Vertical Alignment: Center

8. **Part Name Text**
   - Add `Text` to `VBox_TextInfo`
   - Rename: `Text_PartName`
   - Is Variable: `True`
   - Font: `Font_Roboto_SemiBold`, Size: `16`
   - Color: `#FFFFFF`

9. **Rarity Subtext**
   - Add `Text` to `VBox_TextInfo`
   - Rename: `Text_PartSubtext`
   - Is Variable: `True`
   - Font: `Font_Roboto_Regular`, Size: `12`

10. **Spacer**
    - Add `Spacer` to `HBox_Content`
    - Slot → Size: `Fill`

11. **Status Indicator Overlay**
    - Add `Overlay` to `HBox_Content`
    - Rename: `Overlay_StatusIndicator`
    - Size: `24×24`

12. **Equipped Checkmark**
    - Add `Image` to `Overlay_StatusIndicator`
    - Rename: `Image_EquippedCheckmark`
    - Is Variable: `True`
    - Brush → Image: `T_Icon_Checkmark`
    - Brush → Tint: `#4CAF50`
    - Visibility: `Hidden` (by default)

13. **Locked Icon**
    - Add `Image` to `Overlay_StatusIndicator`
    - Rename: `Image_LockedIcon`
    - Is Variable: `True`
    - Brush → Image: `T_Icon_Lock`
    - Brush → Tint: `#F44336`
    - Visibility: `Hidden` (by default)

### Phase 2: Create Entry Logic

14. **Add Variables:**
    - `PartData` (ShipPartData, Object Reference)
    - `bIsEquipped` (Boolean)
    - `bIsLocked` (Boolean)

15. **Create Function: SetPartData**
    - Inputs: `InPartData`, `InIsEquipped`, `InIsLocked`

```blueprint
SetPartData
    ├─> Set PartData = InPartData
    ├─> Set bIsEquipped = InIsEquipped
    ├─> Set bIsLocked = InIsLocked
    ├─> Branch (IsValid(InPartData))
    │   ├─> True:
    │   │   ├─> Set Image_PartIcon → Brush → Image = PartData.IconTexture
    │   │   ├─> Set Text_PartName → Text = PartData.PartName
    │   │   ├─> Set Text_PartSubtext → Text = PartData.Rarity (as text)
    │   │   ├─> Set Text_PartSubtext → Color = GetRarityColor()
    │   │   ├─> Set Border_Container → Border Color = GetRarityColor()
    │   │   ├─> Call UpdateStatus()
    │   │   └─> Return
    │   └─> False:
    │       └─> Print Warning ("Invalid Part Data")
```

16. **Create Function: UpdateStatus**

```blueprint
UpdateStatus
    ├─> Set Image_PartIcon → Tint = (bIsLocked ? Gray : White)
    ├─> Set Text_PartName → Color = (bIsLocked ? #9E9E9E : #FFFFFF)
    ├─> Set Image_EquippedCheckmark → Visibility = (bIsEquipped ? Visible : Hidden)
    └─> Set Image_LockedIcon → Visibility = (bIsLocked ? Visible : Hidden)
```

17. **Create Function: GetRarityColor** (Pure)
    - Returns: LinearColor

```blueprint
GetRarityColor
    ├─> Switch on Enum (PartData.Rarity)
    │   ├─> Common → Return #4CAF50
    │   ├─> Uncommon → Return #2196F3
    │   ├─> Rare → Return #9C27B0
    │   ├─> Epic → Return #FF9800
    │   ├─> Legendary → Return #F44336
    │   └─> Default → Return #FFFFFF
```

18. **Create Event Dispatcher:**
    - Name: `OnPartClicked`
    - Parameter: `PartData` (ShipPartData reference)

19. **Bind Button Click**
    - Select `Button_Main` → Events → On Clicked

```blueprint
OnButtonClicked
    ├─> Branch (IsValid(PartData))
    │   ├─> True:
    │   │   ├─> Call Event Dispatcher: OnPartClicked(PartData)
    │   │   └─> Play Animation (Anim_Click)
    │   └─> False:
    │       └─> (Do nothing)
```

### Phase 3: Animations

20. **Create Anim_Hover**
    - Duration: `0.1s`
    - Track: Button_Main → Render Transform → Scale: 1.0 → 1.02

21. **Create Anim_Unhover**
    - Duration: `0.1s`
    - Track: Button_Main → Render Transform → Scale: 1.02 → 1.0

22. **Create Anim_Click**
    - Duration: `0.1s`
    - Track: Button_Main → Render Transform → Scale: 1.0 → 0.98 → 1.0

23. **Bind Hover Events**
    - Button_Main → On Hovered → Play Animation (Anim_Hover)
    - Button_Main → On Unhovered → Play Animation (Anim_Unhover)

---

## Widget 4: WBP_PartDetailPanel

### Abbreviated Steps (Following Same Pattern)

1. Create widget in `Panels/` folder
2. Root: `Border` with `#2A2A2A` background
3. Add `Vertical Box` for main layout
4. Add header text "PART DETAILS"
5. Add `Image` for part icon (256×256 size box)
6. Add horizontal box for part name + rarity badge
7. Add description text box (scrollable, wrap enabled)
8. Add "STAT MODIFIERS:" section with dynamic vertical box
9. Add "REQUIREMENTS:" section
10. Add cost/status row
11. Add action buttons (Unlock, Equip)
12. Create variables: `SelectedPart`, `SubsystemRef`
13. Create function: `DisplayPart(PartData)`
14. Implement stat entry generation (dynamic text widgets)
15. Implement button enable/disable logic based on ownership
16. Create animations for part changes
17. Bind button events to event dispatchers

---

## Widget Integration

### Integrating Child Widgets into Parent

**Example: Adding WBP_PartSelectionPanel to WBP_CustomizationMenu**

1. **Replace Placeholder**
   - Open `WBP_CustomizationMenu`
   - Delete `Border_PartSelectionPlaceholder`

2. **Add Child Widget**
   - Drag `WBP_PartSelectionPanel` from Content Browser
   - Drop onto `HBox_MainContent` in first position
   - Rename: `Widget_PartSelection`
   - Is Variable: `True`
   - Slot → Size: `Auto`
   - Slot → Padding: `0,0,8,0`

3. **Bind Event Dispatcher**
   - Graph → Event Construct (or BeginPlay)

```blueprint
Construct
    ├─> Get Widget_PartSelection
    └─> Bind Event to OnPartSelected
        └─> Handler: OnPartSelected_Handler
```

4. **Create Handler Function:**

```blueprint
OnPartSelected_Handler (Input: PartData)
    ├─> Set CurrentlySelectedPart = PartData
    ├─> Call Widget_PartDetail.DisplayPart(PartData)
    └─> Call Widget_StatsComparison.UpdatePreview(PartData)
```

5. **Initialize in InitializeMenu:**

```blueprint
InitializeMenu
    ├─> [existing logic]
    ├─> Widget_PartSelection.Initialize(SubsystemReference)
    ├─> Widget_PartDetail.Initialize(SubsystemReference)
    └─> Widget_StatsComparison.Initialize(SubsystemReference)
```

---

## Testing and Debugging

### Unit Testing Individual Widgets

1. **Create Test Level**
   - Create new level: `TestLevel_CustomizationUI`
   - Add basic lighting

2. **Create Test Game Mode**
   - Blueprints/Customization → Blueprint Class → Game Mode Base
   - Name: `GM_CustomizationTest`

3. **Create Test HUD**
   - Blueprint Class → HUD
   - Name: `HUD_CustomizationTest`
   - Event BeginPlay:

```blueprint
BeginPlay
    ├─> Create Widget (WBP_CustomizationMenu)
    ├─> Add to Viewport
    └─> Set Input Mode UI Only
```

4. **Set World Settings**
   - Open `TestLevel_CustomizationUI`
   - World Settings → Game Mode → Select `GM_CustomizationTest`

5. **Create Mock Data**
   - Create Data Asset: `DA_MockShipPart_Common`
   - Fill with test data (name, icon, stats, etc.)
   - Create several variants for each rarity

6. **PIE Test Checklist**
   - [ ] Menu opens and displays correctly
   - [ ] All buttons are clickable
   - [ ] Category tabs switch properly
   - [ ] Search filters parts correctly
   - [ ] Rarity filter works
   - [ ] Part selection updates detail panel
   - [ ] Stats comparison shows correct values
   - [ ] Unlock dialog appears on unlock button
   - [ ] Purchase completes successfully
   - [ ] Equip changes loadout
   - [ ] Loadout save/load works
   - [ ] Animations play smoothly
   - [ ] No console errors or warnings

### Debugging Common Issues

**Issue: Widget not appearing**
- Check Z-Order (should be high, e.g., 100)
- Check visibility (should be Visible, not Hidden/Collapsed)
- Check parent widget is added to viewport
- Check anchors are set correctly

**Issue: Buttons not responding**
- Check Button → Events are bound
- Check widget has interaction enabled
- Check input mode is set to UI or Game and UI
- Check no other widget is blocking (Z-Order)

**Issue: Bindings not updating**
- Check variables are marked as "Expose on Spawn" if needed
- Check binding functions are marked as Pure
- Use "Invalidate" to force redraw
- Check tick rate (some bindings update per frame)

**Issue: Animations not playing**
- Check animation is not already playing
- Check animation length matches delay times
- Check "Play Mode" (Forward, Reverse, Ping-Pong)
- Check widget is visible before playing

**Issue: Text not displaying**
- Check font is imported correctly
- Check text color is not same as background
- Check text wrap is enabled for long text
- Check size box is not constraining to 0

---

## VR-Specific Setup

### Converting Desktop Widget to VR

1. **Create VR Variant**
   - Duplicate `WBP_CustomizationMenu`
   - Rename: `WBP_CustomizationMenu_VR`

2. **Scale All Elements**
   - Method 1: Global scale multiplier
     - Root panel → Render Transform → Scale: `2.0, 2.0`
   - Method 2: Individual adjustments
     - Font sizes: ×2
     - Button sizes: ×2
     - Padding/margins: ×2
     - Icons: Use higher res versions

3. **Adjust Interactive Elements**
   - All buttons: Minimum 80×80 pixels (2cm × 2cm)
   - Increase padding around clickable areas
   - Use larger hit boxes

4. **Widget Component Setup**
   - Add to VR Pawn Blueprint (e.g., left hand)
   - Add Component → Widget
   - Widget Class: `WBP_CustomizationMenu_VR`
   - Draw Size: `2000 × 1500` (large for readability)
   - Pivot: `0.5, 0.5` (center)
   - Geometry Mode: `Plane`
   - Blend Mode: `Transparent`

5. **Widget Interaction Component**
   - Add Component → Widget Interaction
   - Parent to: VR hand (right hand for pointer)
   - Interaction Source: `World`
   - Interaction Distance: `500` cm
   - Debug → Show Debug: `True` (for testing)

6. **Input Mapping**
   - Project Settings → Input
   - Add Action Mapping: `UI_Click`
     - Vive Controller: Trigger
     - Oculus Touch: Index Trigger
     - Mouse: Left Mouse Button
   - Add Action Mapping: `UI_Back`
     - Vive Controller: Grip
     - Oculus Touch: B Button

7. **VR Interaction Logic**
   - In VR Pawn Blueprint:

```blueprint
InputAction UI_Click (Pressed)
    ├─> Widget Interaction Component → Press Pointer Key (Left Mouse Button)
    └─> Play Haptic Feedback (Light pulse)

InputAction UI_Click (Released)
    └─> Widget Interaction Component → Release Pointer Key (Left Mouse Button)

InputAction UI_Back
    └─> Close Customization Menu
```

8. **VR Comfort Settings**
   - Enable Snap Turn (not smooth turn) when menu open
   - Dim background (vignette effect)
   - Lock menu distance from head (no depth changes)
   - Optional: Anchor to world space, not hand space

### VR Testing Checklist

- [ ] Widget renders at readable distance
- [ ] All buttons are large enough to target with pointer
- [ ] Laser pointer visibly hits buttons
- [ ] Haptic feedback confirms button presses
- [ ] Text is crisp and readable
- [ ] No jaggy edges on icons (use hi-res textures)
- [ ] Performance: Maintains 90fps (or 120fps for Index/Quest 2)
- [ ] Can comfortably read for 5+ minutes without strain
- [ ] Widget doesn't clip through geometry
- [ ] Menu can be closed easily from VR

---

## Performance Optimization

### Widget Invalidation

1. **Set Invalidation Mode**
   - For static widgets: `Volatility → Invalidation → Invalidate on Source Changed`
   - For dynamic widgets: `Invalidation → Invalidate on Paint`

2. **Disable Tick When Possible**
   - Most widgets don't need Tick
   - Graph → Class Settings → Tick → Disable

3. **Use Attribute Bindings Sparingly**
   - Avoid binding functions that run every frame
   - Cache values and manually invalidate when changed

### Widget Pooling (Advanced)

**For WBP_PartListEntry:**

1. **Create Pool Manager Function** (in WBP_PartSelectionPanel):

```blueprint
Variables:
    - PartEntryPool (Array of WBP_PartListEntry)
    - ActiveEntries (Array of WBP_PartListEntry)

Function: GetPooledEntry
    ├─> Branch (PartEntryPool.Length > 0)
    │   ├─> True:
    │   │   ├─> Pop entry from PartEntryPool
    │   │   ├─> Set Visibility = Visible
    │   │   └─> Return entry
    │   └─> False:
    │       ├─> Create Widget (WBP_PartListEntry)
    │       └─> Return new entry

Function: ReturnEntryToPool
    ├─> Set Visibility = Collapsed
    ├─> Remove from ActiveEntries
    └─> Add to PartEntryPool

Function: RefreshPartsList (Modified)
    ├─> ForEach ActiveEntry:
    │   └─> ReturnEntryToPool(ActiveEntry)
    ├─> Clear ActiveEntries
    ├─> ForEach Part in FilteredParts:
    │   ├─> Get entry = GetPooledEntry()
    │   ├─> entry.SetPartData(Part)
    │   ├─> Add entry to VBox_PartsContainer
    │   └─> Add entry to ActiveEntries
```

2. **Benefits:**
   - Reduces garbage collection
   - Faster list updates
   - Smoother scrolling

### Texture Memory Optimization

1. **Icon Texture Settings:**
   - Compression: `UserInterface2D`
   - Mip Gen Settings: `NoMipmaps`
   - LOD Group: `UI`
   - Never Stream: `True` (keeps in memory)
   - Max Texture Size: `256` (Desktop), `512` (VR)

2. **Lazy Load Icons:**
   - Only load textures when part becomes visible
   - Use Soft Object References
   - Load asynchronously

---

## Conclusion

This guide provides step-by-step instructions for creating all 8 UMG widgets for the Ship Customization System. Follow each phase carefully, compile frequently, and test each widget individually before integration.

### Next Steps

1. Create all widgets following this guide
2. Create mock data assets for testing
3. Integrate with Ship Customization Subsystem (from Phase 2)
4. Implement save/load functionality
5. Polish animations and transitions
6. Optimize for performance
7. Test in VR
8. Iterate based on player feedback

### Additional Resources

- Unreal Engine UMG Documentation: https://docs.unrealengine.com/en-US/InteractiveExperiences/UMG/
- VR Widget Interaction: https://docs.unrealengine.com/en-US/SharingAndReleasing/XRDevelopment/VR/
- Blueprint Best Practices: https://docs.unrealengine.com/en-US/ProgrammingAndScripting/Blueprints/BestPractices/

### Support

For issues or questions, refer to:
- `CUSTOMIZATION_WIDGETS_SPECIFICATION.md` (this companion document)
- Individual widget JSON specs in `Content/UI/WidgetSpecs/`
- Ship Customization System documentation (Phases 1-2)
