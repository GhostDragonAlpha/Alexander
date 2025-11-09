# UMG Widget Creation Guide - Alexander Project
## Comprehensive Step-by-Step Manual for Creating 18 UI Widgets

---

## Table of Contents
1. [Overview](#overview)
2. [Prerequisites](#prerequisites)
3. [Directory Structure Setup](#directory-structure-setup)
4. [Widget Creation by Category](#widget-creation-by-category)
   - [HUD Components (9 widgets)](#hud-components)
   - [Menu Systems (6 widgets)](#menu-systems)
   - [Tutorial System (1 widget)](#tutorial-system)
   - [UI Feedback (2 widgets)](#ui-feedback)
5. [UMG Best Practices for VR](#umg-best-practices-for-vr)
6. [Data Binding Setup](#data-binding-setup)
7. [Troubleshooting](#troubleshooting)
8. [Validation Checklist](#validation-checklist)

---

## Overview

**Total Widgets**: 18
**Total Estimated Time**: 12-16 hours
**Priority Order**: HUD → Tutorial → Menu → Feedback

### Widget Categories Summary
- **HUD Components**: 9 widgets (4-6 hours)
- **Menu Systems**: 6 widgets (5-7 hours)
- **Tutorial System**: 1 widget (30-45 minutes)
- **UI Feedback**: 2 widgets (45-60 minutes)

### Required Skills
- UMG Widget Blueprint creation
- Canvas Panel/layout management
- Data binding concepts
- Material/texture basics
- VR UI considerations

---

## Prerequisites

### 1. Enable Required Plugins
**Path**: Edit → Plugins
- UMG (Unreal Motion Graphics) - Should be enabled by default
- Widget Component
- Media Framework (for video playback in menus)

### 2. Create Master Materials
Before creating widgets, prepare these materials:

#### Create Material: M_ProgressBar_Master
1. **Content Browser** → Right-click → Material → Material
2. Name: `M_ProgressBar_Master`
3. Add parameters:
   - `Color` (Vector Parameter) - Default: (1, 1, 1)
   - `Progress` (Scalar Parameter) - Default: 0.5
   - `BackgroundColor` (Vector Parameter) - Default: (0.2, 0.2, 0.2)
4. Save to: `/Game/UI/Materials/`

#### Create Material: M_UI_Background
1. Create Material: `M_UI_Background`
2. Add parameters:
   - `Opacity` (Scalar Parameter) - Default: 0.8
   - `Color` (Vector Parameter) - Default: (0.1, 0.1, 0.15)
   - `BorderThickness` (Scalar Parameter) - Default: 2.0
3. Save to: `/Game/UI/Materials/`

### 3. Import Icon Assets
Create placeholder icons or import real assets to:
- `/Game/UI/Icons/`
- Required icons: fuel_icon, weapon_icon, credits_icon, notification_icon, mission_type_icon

---

## Directory Structure Setup

### Create Folder Structure
**Content Browser Path**: Right-click on "Content" folder → New Folder

```
/Game/UI/
├── Generated/
│   ├── hud/
│   ├── menu/
│   ├── tutorial/
│   └── ui_feedback/
├── Materials/
├── Icons/
└── Textures/
```

**Time**: 5 minutes

---

## Widget Creation by Category

---

## HUD Components

### Widget 1: HUD_SpeedIndicator
**Category**: HUD | **Time**: 30-40 minutes | **Priority**: HIGH

#### Step 1: Create Widget Blueprint
1. **Content Browser** → Navigate to `/Game/UI/Generated/hud/`
2. Right-click → **User Interface** → **Widget Blueprint**
3. Name: `WBP_HUD_SpeedIndicator`
4. Double-click to open in UMG Designer

#### Step 2: Set Canvas Panel Properties
1. In **Hierarchy** panel, select root **Canvas Panel**
2. **Details Panel** → **Slot (Canvas Panel Slot)**:
   - Anchors: Bottom-Left (click anchor preset, select bottom-left)
   - Position: X=20, Y=-80
   - Size: X=200, Y=50
   - Alignment: X=0, Y=1

#### Step 3: Add Background Border
1. **Palette** panel → Drag **Border** onto Canvas Panel
2. Rename to: `Background_Border`
3. **Details Panel**:
   - **Slot (Canvas Panel Slot)**:
     - Anchors: Fill (0,0,1,1)
     - Offsets: Left=0, Top=0, Right=0, Bottom=0
   - **Appearance**:
     - Brush Color: R=0.1, G=0.1, B=0.15, A=0.8
     - Padding: Left=10, Top=5, Right=10, Bottom=5

#### Step 4: Add Horizontal Box Container
1. Drag **Horizontal Box** onto Background_Border
2. Rename to: `Content_HBox`
3. **Details Panel**:
   - **Slot (Border Slot)**:
     - Horizontal Alignment: Fill
     - Vertical Alignment: Center

#### Step 5: Add Speed Label Text
1. Drag **Text** onto Content_HBox
2. Rename to: `Label_Speed`
3. **Details Panel**:
   - **Content** → Text: `Speed:`
   - **Appearance**:
     - Color and Opacity: R=1, G=1, B=1, A=1
     - Font → Size: 18
     - Font → Typeface: Bold
   - **Slot (Horizontal Box Slot)**:
     - Size: Auto
     - Padding: Right=10

#### Step 6: Add Progress Bar
1. Drag **Progress Bar** onto Content_HBox
2. Rename to: `ProgressBar_Speed`
3. **Details Panel**:
   - **Appearance**:
     - Percent: 0.5 (default, will be bound)
     - Fill Color and Opacity: R=0.3, G=0.8, B=1.0, A=1
     - Background Image: Set to material M_ProgressBar_Master (if created)
   - **Slot (Horizontal Box Slot)**:
     - Size: Fill
     - Horizontal Alignment: Fill
     - Vertical Alignment: Center
     - Padding: Right=10

#### Step 7: Add Speed Value Text
1. Drag **Text** onto Content_HBox
2. Rename to: `Text_SpeedValue`
3. **Details Panel**:
   - **Content** → Text: `0 m/s`
   - **Appearance**:
     - Color and Opacity: R=0.3, G=0.8, B=1.0, A=1
     - Font → Size: 20
     - Font → Typeface: Bold
   - **Slot (Horizontal Box Slot)**:
     - Size: Auto

#### Step 8: Setup Data Binding (Graph View)
1. Switch to **Graph** tab (top-right of UMG editor)
2. Create function: `UpdateSpeed`
   - Add input parameter: `NewSpeed` (Float)
3. In function:
   - Set `ProgressBar_Speed` → Set Percent (NewSpeed / MaxSpeed)
   - Set `Text_SpeedValue` → Set Text (Format Text: "{0} m/s", NewSpeed)
4. Compile and Save

#### Step 9: Save and Test
1. **File** → Save
2. Test in viewport by changing ProgressBar percent value manually

**Estimated Time**: 30-40 minutes

---

### Widget 2: HUD_AltitudeDisplay
**Category**: HUD | **Time**: 15-20 minutes | **Priority**: HIGH

#### Step 1: Create Widget Blueprint
1. Navigate to `/Game/UI/Generated/hud/`
2. Right-click → **User Interface** → **Widget Blueprint**
3. Name: `WBP_HUD_AltitudeDisplay`

#### Step 2: Set Canvas Panel
1. Root Canvas Panel:
   - Anchors: Bottom-Left
   - Position: X=20, Y=-130
   - Size: X=200, Y=30

#### Step 3: Add Background Border
1. Add **Border** → Name: `Background_Border`
2. **Details**:
   - Anchors: Fill
   - Brush Color: R=0.1, G=0.1, B=0.15, A=0.8
   - Padding: 10,5,10,5

#### Step 4: Add Horizontal Box
1. Add **Horizontal Box** onto Border
2. Name: `Content_HBox`
3. Fill horizontally, center vertically

#### Step 5: Add Label Text
1. Add **Text** to HBox → Name: `Label_Altitude`
2. Text: `Altitude:`
3. Font Size: 16, Color: White
4. Slot: Auto size, Right padding: 10

#### Step 6: Add Value Text
1. Add **Text** to HBox → Name: `Text_AltitudeValue`
2. Text: `0.0 km`
3. Font Size: 18, Bold, Color: R=1, G=0.9, B=0.3
4. Slot: Auto size

#### Step 7: Binding Setup (Graph)
1. Create function: `UpdateAltitude(Float NewAltitude)`
2. Format text as: `{altitude} km`
3. Compile and Save

**Estimated Time**: 15-20 minutes

---

### Widget 3: HUD_FuelGauge
**Category**: HUD | **Time**: 25-30 minutes | **Priority**: HIGH

#### Step 1: Create Widget Blueprint
1. Path: `/Game/UI/Generated/hud/`
2. Name: `WBP_HUD_FuelGauge`

#### Step 2: Canvas Setup
1. Anchors: Top-Right
2. Position: X=-170, Y=20
3. Size: X=150, Y=40

#### Step 3: Add Background Border
1. **Border** → `Background_Border`
2. Anchors: Fill
3. Color: R=0.1, G=0.1, B=0.15, A=0.9

#### Step 4: Add Horizontal Box
1. **Horizontal Box** → `Content_HBox`

#### Step 5: Add Fuel Icon
1. **Image** → `Icon_Fuel`
2. **Details**:
   - Brush → Image: Select fuel icon texture
   - Size: 24x24
   - Tint: R=1, G=0.6, B=0.2 (orange)
3. **Slot**: Size=Auto, Padding Right=8

#### Step 6: Add Progress Bar
1. **Progress Bar** → `ProgressBar_Fuel`
2. **Details**:
   - Fill Color: R=1, G=0.6, B=0.2 (orange)
   - Percent: 1.0 (full by default)
3. **Slot**: Fill, Vertical Center, Padding Right=8

#### Step 7: Add Percentage Text
1. **Text** → `Text_FuelPercent`
2. Text: `100%`
3. Font Size: 16, Bold
4. Color: R=1, G=0.6, B=0.2

#### Step 8: Add Color Warning Logic (Graph)
1. Create function: `UpdateFuel(Float FuelLevel)`
2. Add Branch:
   - If FuelLevel < 25%: Set color to Red
   - Else: Set color to Orange
3. Update progress bar and text
4. Compile and Save

**Estimated Time**: 25-30 minutes

---

### Widget 4: HUD_ShieldBar
**Category**: HUD | **Time**: 20-25 minutes | **Priority**: HIGH

#### Step 1: Create Widget Blueprint
1. Path: `/Game/UI/Generated/hud/`
2. Name: `WBP_HUD_ShieldBar`

#### Step 2: Canvas Setup
1. Anchors: Top-Left
2. Position: X=20, Y=20
3. Size: X=200, Y=30

#### Step 3: Add Border Background
1. **Border** → Fill anchors
2. Color: R=0.1, G=0.1, B=0.15, A=0.85

#### Step 4: Add Horizontal Box
1. **Horizontal Box** inside Border

#### Step 5: Add Progress Bar
1. **Progress Bar** → `ProgressBar_Shield`
2. **Details**:
   - Fill Color: R=0.2, G=0.6, B=1.0, A=1 (blue)
   - Fill Type: Left to Right
   - Percent: 1.0
3. **Slot**: Fill, Right Padding=10

#### Step 6: Add Percentage Text
1. **Text** → `Text_ShieldPercent`
2. Text: `100%`
3. Font: Size=16, Bold
4. Color: R=0.4, G=0.8, B=1.0
5. **Slot**: Auto size

#### Step 7: Add Shield Recharge Animation (Graph)
1. Add Animation track in Animations panel
2. Name: `Anim_ShieldRecharge`
3. Animate Fill Color pulsing when recharging
4. Create function: `UpdateShield(Float ShieldLevel, Bool IsRecharging)`
5. Compile and Save

**Estimated Time**: 20-25 minutes

---

### Widget 5: HUD_HealthBar
**Category**: HUD | **Time**: 20-25 minutes | **Priority**: HIGH

#### Step 1: Create Widget Blueprint
1. Path: `/Game/UI/Generated/hud/`
2. Name: `WBP_HUD_HealthBar`

#### Step 2: Canvas Setup
1. Anchors: Top-Left
2. Position: X=20, Y=55 (below shield bar)
3. Size: X=200, Y=30

#### Step 3: Add Border Background
1. **Border** → Fill
2. Color: R=0.1, G=0.1, B=0.15, A=0.85

#### Step 4: Add Horizontal Box
1. **Horizontal Box**

#### Step 5: Add Progress Bar
1. **Progress Bar** → `ProgressBar_Health`
2. **Details**:
   - Fill Color: R=0.8, G=0.2, B=0.2, A=1 (red)
   - Percent: 1.0
3. **Slot**: Fill, Right Padding=10

#### Step 6: Add Text
1. **Text** → `Text_HealthPercent`
2. Text: `100%`
3. Font: Size=16, Bold
4. Color: R=1, G=0.4, B=0.4

#### Step 7: Add Damage Flash Animation
1. Create animation: `Anim_DamageFlash`
2. Flash border red when damaged
3. Create function: `UpdateHealth(Float HealthLevel)`
4. Add warning color at <25% health
5. Compile and Save

**Estimated Time**: 20-25 minutes

---

### Widget 6: HUD_WeaponStatus
**Category**: HUD | **Time**: 35-40 minutes | **Priority**: MEDIUM

#### Step 1: Create Widget Blueprint
1. Path: `/Game/UI/Generated/hud/`
2. Name: `WBP_HUD_WeaponStatus`

#### Step 2: Canvas Setup
1. Anchors: Bottom-Right
2. Position: X=-270, Y=-100
3. Size: X=250, Y=80

#### Step 3: Add Border Background
1. **Border** → Fill, Color: R=0.1, G=0.1, B=0.15, A=0.9

#### Step 4: Add Vertical Box
1. **Vertical Box** → `Content_VBox`

#### Step 5: Add Top Horizontal Box (Weapon Info)
1. **Horizontal Box** → `HBox_WeaponInfo`
2. Add to VBox

#### Step 6: Add Weapon Icon
1. **Image** → `Icon_Weapon`
2. Size: 32x32
3. **Slot**: Auto, Right Padding=10

#### Step 7: Add Weapon Name
1. **Text** → `Label_Weapon`
2. Text: `Weapon:`
3. Font: Size=14
4. **Slot**: Auto, Right Padding=5

#### Step 8: Add Weapon Name Value
1. **Text** → `Text_WeaponName`
2. Text: `Pulse Cannon`
3. Font: Size=16, Bold
4. Color: R=1, G=0.8, B=0.3

#### Step 9: Add Heat Progress Bar (Bottom HBox)
1. **Horizontal Box** → `HBox_Heat`
2. **Progress Bar** → `ProgressBar_Heat`
3. Fill Color: R=1, G=0.5, B=0.2 (orange when heating)
4. **Slot**: Fill

#### Step 10: Add Ammo Counter (Bottom HBox)
1. **Horizontal Box** → `HBox_Ammo`
2. **Text** → `Text_Ammo`
3. Text: `Ammo: 30/30`
4. Font: Size=14, Bold

#### Step 11: Binding (Graph)
1. Function: `UpdateWeapon(String WeaponName, Float HeatLevel, Int32 CurrentAmmo, Int32 MaxAmmo)`
2. Update all displays
3. Change heat color when >75%
4. Compile and Save

**Estimated Time**: 35-40 minutes

---

### Widget 7: HUD_Minimap
**Category**: HUD | **Time**: 30-35 minutes | **Priority**: MEDIUM

#### Step 1: Create Widget Blueprint
1. Path: `/Game/UI/Generated/hud/`
2. Name: `WBP_HUD_Minimap`

#### Step 2: Canvas Setup
1. Anchors: Top-Right
2. Position: X=-170, Y=70
3. Size: X=150, Y=150

#### Step 3: Add Border (Radar Frame)
1. **Border** → `Border_RadarFrame`
2. **Details**:
   - Anchors: Fill
   - Brush: Use radar border material
   - Brush Color: R=0.2, G=0.8, B=1.0, A=0.8

#### Step 4: Add Image for Render Target
1. **Image** → `Image_MinimapRT`
2. **Details**:
   - Anchors: Fill (inside border)
   - Offsets: 5,5,5,5 (margin from border)
   - Brush → Image: Select or create Render Target texture
   - Note: Render target must be created separately as RT_Minimap

#### Step 5: Add Corner Markers (Optional)
1. Create 4 small **Image** widgets at corners
2. Each 8x8 pixels, white color
3. Gives radar targeting feel

#### Step 6: Binding (Graph)
1. Function: `UpdateMinimap()`
2. Set Image brush to updated render target
3. Note: Actual minimap rendering happens in C++ or Blueprint
4. Compile and Save

**Estimated Time**: 30-35 minutes

**Additional Setup Required**:
- Create Render Target: RT_Minimap (150x150)
- Setup minimap camera in world to render to RT_Minimap

---

### Widget 8: HUD_ObjectiveTracker
**Category**: HUD | **Time**: 40-45 minutes | **Priority**: HIGH

#### Step 1: Create Widget Blueprint
1. Path: `/Game/UI/Generated/hud/`
2. Name: `WBP_HUD_ObjectiveTracker`

#### Step 2: Canvas Setup
1. Anchors: Right (Middle-Right)
2. Position: X=-320, Y=0
3. Size: X=300, Y=200
4. Alignment: X=1, Y=0.5

#### Step 3: Add Border Background
1. **Border** → Fill
2. Color: R=0.05, G=0.05, B=0.1, A=0.85
3. Padding: 15,10,15,10

#### Step 4: Add Vertical Box
1. **Vertical Box** → `Content_VBox`

#### Step 5: Add Header Text
1. **Text** → `Header_Objectives`
2. Text: `OBJECTIVES`
3. Font: Size=20, Bold
4. Color: R=1, G=0.9, B=0.3
5. **Slot**: Top padding=0, Bottom padding=10

#### Step 6: Add Separator Line
1. **Image** → `Separator_Line`
2. **Details**:
   - Brush Color: R=1, G=0.9, B=0.3, A=0.5
   - Size: Height=2 (fixed)
3. **Slot**: Fill width, Bottom padding=10

#### Step 7: Add Scroll Box
1. **Scroll Box** → `ScrollBox_Objectives`
2. **Details**:
   - Orientation: Vertical
   - Scroll Bar Visibility: Auto
3. **Slot**: Fill

#### Step 8: Create Objective Entry Widget (Sub-Widget)
1. Create new Widget Blueprint: `WBP_ObjectiveEntry`
2. **Horizontal Box** root
3. Add **CheckBox** (for completion status)
4. Add **Text** (objective description)
5. Save to `/Game/UI/Generated/hud/`

#### Step 9: Populate Objectives (Graph)
1. In WBP_HUD_ObjectiveTracker Graph
2. Create function: `AddObjective(String ObjectiveText, Bool IsComplete)`
3. Create WBP_ObjectiveEntry widget
4. Add to ScrollBox_Objectives
5. Create function: `ClearObjectives()`
6. Compile and Save

**Estimated Time**: 40-45 minutes

---

### Widget 9: HUD_TargetInfo
**Category**: HUD | **Time**: 30-35 minutes | **Priority**: MEDIUM

#### Step 1: Create Widget Blueprint
1. Path: `/Game/UI/Generated/hud/`
2. Name: `WBP_HUD_TargetInfo`

#### Step 2: Canvas Setup
1. Anchors: Top-Center
2. Position: X=-125, Y=20
3. Size: X=250, Y=100
4. Alignment: X=0.5, Y=0

#### Step 3: Add Overlay Container
1. **Overlay** → Root container
2. Allows layering of elements

#### Step 4: Add Background Border
1. **Border** → `Background_Border`
2. Add to Overlay slot 0
3. Color: R=0.1, G=0.05, B=0.05, A=0.9 (reddish tint)
4. Padding: 10,10,10,10

#### Step 5: Add Vertical Box
1. **Vertical Box** → `Content_VBox`
2. Add to Overlay slot 1

#### Step 6: Add Target Name
1. **Text** → `Text_TargetName`
2. Text: `[No Target]`
3. Font: Size=18, Bold
4. Color: R=1, G=0.4, B=0.4
5. Justification: Center
6. **Slot**: Top padding=5, Bottom padding=5

#### Step 7: Add Distance Text
1. **Text** → `Text_Distance`
2. Text: `Distance: -- m`
3. Font: Size=14
4. Color: White
5. Justification: Center
6. **Slot**: Bottom padding=8

#### Step 8: Add Health Progress Bar
1. **Progress Bar** → `ProgressBar_TargetHealth`
2. Fill Color: R=0.8, G=0.2, B=0.2
3. Height: 20
4. **Slot**: Fill width

#### Step 9: Add Health Percentage Overlay
1. **Text** → `Text_HealthPercent`
2. Text: `100%`
3. Font: Size=12, Bold
4. Add to Overlay (above progress bar)
5. Position centered over progress bar

#### Step 10: Visibility Control (Graph)
1. Create function: `SetTargetInfo(String TargetName, Float Distance, Float HealthPercent)`
2. Create function: `ShowTargetInfo(Bool bShow)`
3. Set Visibility based on whether target exists
4. Compile and Save

**Estimated Time**: 30-35 minutes

**Total HUD Time**: 4-6 hours

---

## Menu Systems

### Widget 10: UI_MissionBoard
**Category**: Menu | **Time**: 50-60 minutes | **Priority**: HIGH

#### Step 1: Create Widget Blueprint
1. Path: `/Game/UI/Generated/menu/`
2. Name: `WBP_UI_MissionBoard`

#### Step 2: Canvas Setup
1. Root Canvas Panel
2. Fill entire screen (Anchors: Fill, Offsets: 0,0,0,0)

#### Step 3: Add Background Overlay
1. **Overlay** → `Overlay_Root`
2. Add **Image** → `Background_Dim`
3. Color: R=0, G=0, B=0, A=0.7 (screen dimming)
4. Anchors: Fill

#### Step 4: Add Panel Border
1. **Border** → `Border_Panel`
2. **Details**:
   - Anchors: Center
   - Alignment: X=0.5, Y=0.5
   - Position: X=-400, Y=-300
   - Size: X=800, Y=600
   - Brush Color: R=0.15, G=0.15, B=0.2, A=0.95
   - Padding: 20,20,20,20

#### Step 5: Add Vertical Box Container
1. **Vertical Box** → `Content_VBox`
2. Inside Border_Panel

#### Step 6: Add Title Section
1. **Text** → `Title_MissionBoard`
2. Text: `AVAILABLE MISSIONS`
3. Font: Size=32, Bold
4. Color: R=1, G=0.9, B=0.3
5. Justification: Center
6. **Slot**: Bottom padding=20

#### Step 7: Add Mission List Scroll Box
1. **Scroll Box** → `ScrollBox_Missions`
2. **Details**:
   - Orientation: Vertical
   - Scroll Bar Thickness: 12
3. **Slot**: Fill, minimum height=400

#### Step 8: Add Bottom Button Row
1. **Horizontal Box** → `HBox_Buttons`
2. Justification: Center
3. **Slot**: Top padding=20

#### Step 9: Add Accept Mission Button
1. **Button** → `Button_Accept`
2. **Details**:
   - Size: X=180, Y=50
   - Is Enabled: False (until mission selected)
3. Add **Text** child: `Accept Mission`
4. Font: Size=18, Bold
5. **Slot**: Right padding=20

#### Step 10: Add Close Button
1. **Button** → `Button_Close`
2. Size: X=120, Y=50
3. Add **Text** child: `Close`
4. Font: Size=18

#### Step 11: Setup Button Events (Graph)
1. Button_Accept → OnClicked → Event `OnAcceptMission`
2. Button_Close → OnClicked → Event `OnCloseMissionBoard`
3. Create function: `PopulateMissions(Array<MissionData>)`
4. Create function: `OnMissionSelected(MissionCard)`
5. Compile and Save

**Estimated Time**: 50-60 minutes

---

### Widget 11: UI_MissionCard
**Category**: Menu | **Time**: 40-45 minutes | **Priority**: HIGH

#### Step 1: Create Widget Blueprint
1. Path: `/Game/UI/Generated/menu/`
2. Name: `WBP_UI_MissionCard`
3. This is used inside MissionBoard scroll box

#### Step 2: Root Container Setup
1. **Button** → Root (makes entire card clickable)
2. Size: X=350, Y=150

#### Step 3: Add Card Border
1. **Border** → Child of Button
2. Anchors: Fill
3. Color: R=0.2, G=0.2, B=0.25, A=1
4. Padding: 15,15,15,15

#### Step 4: Add Horizontal Box Main Layout
1. **Horizontal Box** → `HBox_Main`

#### Step 5: Add Mission Icon (Left Side)
1. **Image** → `Icon_MissionType`
2. Size: 64x64
3. **Slot**: Size=Auto, Right padding=15

#### Step 6: Add Right Side Vertical Box
1. **Vertical Box** → `VBox_Info`
2. **Slot**: Fill

#### Step 7: Add Mission Title
1. **Text** → `Text_MissionTitle`
2. Text: `Mission Title Here`
3. Font: Size=20, Bold
4. Color: R=1, G=0.9, B=0.3
5. **Slot**: Bottom padding=8

#### Step 8: Add Mission Description
1. **Text** → `Text_Description`
2. Text: `Mission description goes here...`
3. Font: Size=14
4. Color: R=0.8, G=0.8, B=0.8
5. Auto Wrap Text: True
6. **Slot**: Bottom padding=10

#### Step 9: Add Reward Row
1. **Horizontal Box** → `HBox_Reward`
2. Add **Text**: `Reward:`
3. Add **Image**: Credits icon (16x16)
4. Add **Text** → `Text_RewardAmount`: `5000`
5. Style with gold color
6. **Slot**: Bottom padding=5

#### Step 10: Add Difficulty Row
1. **Horizontal Box** → `HBox_Difficulty`
2. Add **Text** → `Text_Difficulty`: `Difficulty: Medium`
3. Color-code: Easy=Green, Medium=Yellow, Hard=Red

#### Step 11: Add Hover Effect (Graph)
1. Button → OnHovered → Event
2. Set Border color to lighter shade
3. Button → OnUnhovered → Event
4. Restore original color
5. Create function: `SetMissionData(MissionData)`
6. Compile and Save

**Estimated Time**: 40-45 minutes

---

### Widget 12: UI_MainMenu
**Category**: Menu | **Time**: 60-70 minutes | **Priority**: HIGH

#### Step 1: Create Widget Blueprint
1. Path: `/Game/UI/Generated/menu/`
2. Name: `WBP_UI_MainMenu`

#### Step 2: Canvas Setup
1. Root Canvas Panel
2. Fill screen (1920x1080 reference)

#### Step 3: Add Background Image
1. **Image** → `Background_Space`
2. Anchors: Fill
3. Brush: Space scene texture/material
4. Draw As: Image (stretched)

#### Step 4: Add Title Overlay
1. **Overlay** → `Overlay_Title`
2. Anchors: Top-Center
3. Position: Y=150
4. Size: X=800, Y=200

#### Step 5: Add Game Title
1. **Text** → `Title_Game`
2. Text: `ALEXANDER`
3. Font: Size=72, Bold
4. Color: R=1, G=0.9, B=0.3
5. Shadow Offset: X=3, Y=3
6. Shadow Color: Black, Opacity=0.8
7. Justification: Center

#### Step 6: Add Subtitle
1. **Text** → `Subtitle_Game`
2. Text: `Space Exploration & Farming`
3. Font: Size=24
4. Color: R=0.8, G=0.8, B=0.9
5. Justification: Center
6. Position below title

#### Step 7: Add Menu Vertical Box
1. **Vertical Box** → `VBox_Menu`
2. Anchors: Center
3. Alignment: X=0.5, Y=0.5
4. Position: Y=50 (slightly below center)
5. Size: X=400, Y=Auto

#### Step 8: Add New Game Button
1. **Button** → `Button_NewGame`
2. Size: X=400, Y=70
3. Add **Text** child: `NEW GAME`
4. Font: Size=24, Bold
5. **Slot**: Bottom padding=15

#### Step 9: Add Continue Button
1. **Button** → `Button_Continue`
2. Size: X=400, Y=70
3. Add **Text** child: `CONTINUE`
4. Font: Size=24, Bold
5. Is Enabled: Bind to "Has Save Game" boolean
6. **Slot**: Bottom padding=15

#### Step 10: Add Settings Button
1. **Button** → `Button_Settings`
2. Size: X=400, Y=70
3. Add **Text** child: `SETTINGS`
4. Font: Size=24, Bold
5. **Slot**: Bottom padding=15

#### Step 11: Add Exit Button
1. **Button** → `Button_Exit`
2. Size: X=400, Y=70
3. Add **Text** child: `EXIT`
4. Font: Size=24, Bold

#### Step 12: Add Version Text
1. **Text** → `Text_Version`
2. Text: `v0.1.0 Alpha`
3. Font: Size=12
4. Color: R=0.5, G=0.5, B=0.5
5. Anchors: Bottom-Right
6. Position: X=-20, Y=-20

#### Step 13: Setup Button Events (Graph)
1. Button_NewGame → OnClicked → Start new game
2. Button_Continue → OnClicked → Load save
3. Button_Settings → OnClicked → Open settings menu
4. Button_Exit → OnClicked → Quit application
5. Add button hover animations
6. Compile and Save

**Estimated Time**: 60-70 minutes

---

### Widget 13: UI_TradingInterface
**Category**: Menu | **Time**: 70-80 minutes | **Priority**: MEDIUM

#### Step 1: Create Widget Blueprint
1. Path: `/Game/UI/Generated/menu/`
2. Name: `WBP_UI_TradingInterface`

#### Step 2: Canvas Setup
1. Root Canvas Panel, Fill screen

#### Step 3: Add Background Dim
1. **Image** → Semi-transparent black (A=0.8)
2. Anchors: Fill

#### Step 4: Add Main Panel Border
1. **Border** → `Border_MainPanel`
2. Anchors: Center
3. Size: X=1200, Y=700
4. Color: R=0.1, G=0.1, B=0.15, A=0.95
5. Padding: 20

#### Step 5: Add Horizontal Box (3 Columns)
1. **Horizontal Box** → `HBox_ThreeColumns`

#### Step 6: Column 1 - Player Inventory (Left)
1. **Vertical Box** → `VBox_PlayerInventory`
2. **Slot**: Fill (weight=1)
3. Add **Text** header: `YOUR INVENTORY`
4. Add **Scroll Box** → `ScrollBox_PlayerItems`
5. Add **Text** footer: `Total Value: 0 credits`

#### Step 7: Column 2 - Station Inventory (Middle)
1. **Vertical Box** → `VBox_StationInventory`
2. **Slot**: Fill (weight=1), Left/Right padding=20
3. Add **Text** header: `STATION INVENTORY`
4. Add **Scroll Box** → `ScrollBox_StationItems`
5. Add **Text** footer: `Credits: 10000`

#### Step 8: Column 3 - Transaction Summary (Right)
1. **Vertical Box** → `VBox_Transaction`
2. **Slot**: Fixed width=300
3. Add **Text** header: `TRANSACTION`
4. Add **Scroll Box** → `ScrollBox_Cart`
5. Add **Text**: `Total: 0 credits`
6. Add **Button**: `Complete Trade` (Size: 260x50)
7. Add **Button**: `Cancel` (Size: 260x40)

#### Step 9: Create Item Entry Widget
1. New Widget: `WBP_TradingItemEntry`
2. Horizontal Box root
3. **Image**: Item icon
4. **Text**: Item name
5. **Text**: Price
6. **Button**: Buy/Sell
7. Save to `/Game/UI/Generated/menu/`

#### Step 10: Setup Trading Logic (Graph)
1. Function: `PopulatePlayerInventory(Array<ItemData>)`
2. Function: `PopulateStationInventory(Array<ItemData>)`
3. Function: `AddToCart(ItemData, Bool IsBuying)`
4. Function: `CalculateTotal()`
5. Function: `CompleteTrade()`
6. Compile and Save

**Estimated Time**: 70-80 minutes

---

### Widget 14: UI_ShipCustomization
**Category**: Menu | **Time**: 60-70 minutes | **Priority**: MEDIUM

#### Step 1: Create Widget Blueprint
1. Path: `/Game/UI/Generated/menu/`
2. Name: `WBP_UI_ShipCustomization`

#### Step 2: Canvas Setup
1. Root Canvas Panel, Fill screen

#### Step 3: Add Overlay Root
1. **Overlay** → For layering viewport and UI

#### Step 4: Add 3D Viewport Background (Slot 0)
1. **Image** → `Viewport_ShipPreview`
2. Anchors: Fill
3. Brush: Render Target for ship preview
4. Note: Requires Scene Capture Component 2D setup

#### Step 5: Add UI Overlay (Slot 1)
1. **Canvas Panel** → `Canvas_UI`

#### Step 6: Add Parts List Panel (Left)
1. **Border** → `Border_PartsList`
2. Anchors: Left (Top-Left to Bottom-Left)
3. Size: X=350, Y=Fill
4. Color: R=0.05, G=0.05, B=0.1, A=0.9
5. Padding: 15

#### Step 7: Add Parts Vertical Box
1. **Vertical Box** → `VBox_Parts`
2. Add **Text** header: `SHIP PARTS`
3. Add **Scroll Box** → `ScrollBox_Parts`
4. Categories: Hull, Engines, Weapons, Shields, etc.

#### Step 8: Add Stats Panel (Right)
1. **Border** → `Border_Stats`
2. Anchors: Right
3. Size: X=300, Y=Auto
4. Position: Top-Right
5. Color: R=0.05, G=0.05, B=0.1, A=0.9

#### Step 9: Add Stats Display
1. **Vertical Box** → `VBox_Stats`
2. Add **Text** header: `SHIP STATISTICS`
3. Add stat rows:
   - Speed: [value]
   - Armor: [value]
   - Firepower: [value]
   - Shield: [value]
   - Cargo: [value]

#### Step 10: Add Bottom Control Bar
1. **Horizontal Box** → `HBox_Controls`
2. Anchors: Bottom-Center
3. Size: X=600, Y=60
4. Add **Button**: `Reset View`
5. Add **Button**: `Apply Changes`
6. Add **Button**: `Close`

#### Step 11: Setup Part Selection (Graph)
1. Function: `LoadShipConfiguration(ShipConfig)`
2. Function: `SelectPart(PartType, PartID)`
3. Function: `UpdateStatsDisplay()`
4. Function: `ApplyCustomization()`
5. Compile and Save

**Estimated Time**: 60-70 minutes

---

### Widget 15: UI_SettingsMenu
**Category**: Menu | **Time**: 50-60 minutes | **Priority**: LOW

#### Step 1: Create Widget Blueprint
1. Path: `/Game/UI/Generated/menu/`
2. Name: `WBP_UI_SettingsMenu`

#### Step 2: Canvas Setup
1. Root Canvas Panel

#### Step 3: Add Center Border Panel
1. **Border** → `Border_Panel`
2. Anchors: Center
3. Size: X=600, Y=500
4. Color: R=0.1, G=0.1, B=0.15, A=0.95
5. Padding: 20

#### Step 4: Add Vertical Box Container
1. **Vertical Box** → `VBox_Settings`

#### Step 5: Add Header
1. **Text** → `Header_Settings`
2. Text: `SETTINGS`
3. Font: Size=28, Bold
4. Justification: Center
5. **Slot**: Bottom padding=20

#### Step 6: Add Master Volume Slider
1. **Horizontal Box** → `HBox_MasterVolume`
2. Add **Text**: `Master Volume`
3. Add **Slider** → `Slider_MasterVolume`
   - Min Value: 0
   - Max Value: 1
   - Value: 0.8
4. Add **Text** → `Text_MasterVolumeValue`: `80%`
5. **Slot**: Bottom padding=15

#### Step 7: Add Graphics Quality Slider
1. **Horizontal Box** → `HBox_Graphics`
2. Add **Text**: `Graphics Quality`
3. Add **Slider** → `Slider_GraphicsQuality`
   - Min Value: 0
   - Max Value: 3
   - Step Size: 1
   - Value: 2 (High)
4. Add **Text** → `Text_QualityLevel`: `High`
5. **Slot**: Bottom padding=15

#### Step 8: Add VSync Checkbox
1. **Horizontal Box** → `HBox_VSync`
2. Add **CheckBox** → `CheckBox_VSync`
3. Add **Text**: `Enable VSync`
4. **Slot**: Bottom padding=10

#### Step 9: Add Motion Blur Checkbox
1. **Horizontal Box** → `HBox_MotionBlur`
2. Add **CheckBox** → `CheckBox_MotionBlur`
3. Add **Text**: `Motion Blur`
4. **Slot**: Bottom padding=20

#### Step 10: Add Button Row
1. **Horizontal Box** → `HBox_Buttons`
2. Justification: Center
3. Add **Button** → `Button_Apply`: `APPLY` (180x50)
4. Add **Button** → `Button_Cancel`: `CANCEL` (180x50)
5. Right padding between: 20

#### Step 11: Setup Events (Graph)
1. Slider_MasterVolume → OnValueChanged → Update volume
2. Slider_GraphicsQuality → OnValueChanged → Update quality text
3. Button_Apply → OnClicked → Save and apply settings
4. Button_Cancel → OnClicked → Discard and close
5. Compile and Save

**Estimated Time**: 50-60 minutes

**Total Menu Time**: 5-7 hours

---

## Tutorial System

### Widget 16: UI_TutorialOverlay
**Category**: Tutorial | **Time**: 30-45 minutes | **Priority**: HIGH

#### Step 1: Create Widget Blueprint
1. Path: `/Game/UI/Generated/tutorial/`
2. Name: `WBP_UI_TutorialOverlay`

#### Step 2: Canvas Setup
1. Root Canvas Panel
2. This will be positioned dynamically

#### Step 3: Add Border Background
1. **Border** → `Border_Tutorial`
2. Anchors: Custom (set in code)
3. Size: X=400, Y=200
4. Color: R=0.1, G=0.3, B=0.5, A=0.9 (tutorial blue)
5. Padding: 15

#### Step 4: Add Vertical Box Container
1. **Vertical Box** → `VBox_Content`

#### Step 5: Add Tutorial Text
1. **Text** → `Text_TutorialInstruction`
2. Text: `Tutorial instruction text goes here...`
3. Font: Size=16
4. Color: White
5. Auto Wrap Text: True
6. **Slot**: Bottom padding=10

#### Step 6: Add Input Icon Row
1. **Horizontal Box** → `HBox_Input`
2. Justification: Center
3. Add **Image** → `Image_InputIcon` (Size: 64x64)
4. Shows keyboard/gamepad/VR controller icon
5. **Slot**: Bottom padding=10

#### Step 7: Add Progress Bar
1. **Progress Bar** → `ProgressBar_Tutorial`
2. Fill Color: R=0.3, G=0.8, B=1.0
3. Height: 15
4. Shows completion progress

#### Step 8: Add Close Button (Optional)
1. **Button** → `Button_Skip`
2. Size: X=100, Y=30
3. Add **Text**: `Skip`
4. Anchors: Top-Right of border
5. Position: X=-5, Y=5

#### Step 9: Add Animated Arrow (Optional)
1. **Image** → `Image_Arrow`
2. Points to relevant UI element or world location
3. Rotate and position dynamically

#### Step 10: Setup Tutorial Logic (Graph)
1. Function: `ShowTutorial(String InstructionText, Texture2D InputIcon, Float Progress)`
2. Function: `HideTutorial()`
3. Create Animation: `Anim_FadeIn`
4. Create Animation: `Anim_FadeOut`
5. Create Animation: `Anim_Pulse` (for attention)
6. Compile and Save

**Estimated Time**: 30-45 minutes

**Total Tutorial Time**: 30-45 minutes

---

## UI Feedback

### Widget 17: UI_Notification
**Category**: UI Feedback | **Time**: 25-30 minutes | **Priority**: MEDIUM

#### Step 1: Create Widget Blueprint
1. Path: `/Game/UI/Generated/ui_feedback/`
2. Name: `WBP_UI_Notification`

#### Step 2: Canvas Setup
1. Root Canvas Panel
2. Anchors: Top-Center
3. Position: X=-175, Y=120
4. Size: X=350, Y=80

#### Step 3: Add Border Background
1. **Border** → `Border_Notification`
2. Anchors: Fill
3. Color: R=0.15, G=0.15, B=0.2, A=0.95
4. Padding: 15,10,15,10

#### Step 4: Add Horizontal Box
1. **Horizontal Box** → `HBox_Content`

#### Step 5: Add Notification Icon
1. **Image** → `Icon_Notification`
2. Size: 32x32
3. Brush: Set dynamically (success, warning, error, info)
4. **Slot**: Size=Auto, Right padding=15

#### Step 6: Add Notification Text
1. **Text** → `Text_Notification`
2. Text: `Notification message here`
3. Font: Size=16
4. Color: White
5. Auto Wrap Text: True
6. **Slot**: Fill

#### Step 7: Add Close Button
1. **Button** → `Button_Close`
2. Size: 24x24
3. Add **Text**: `X`
4. Anchors: Top-Right
5. **Slot**: Size=Auto

#### Step 8: Setup Animations (Animations Panel)
1. Create `Anim_SlideIn`:
   - Duration: 0.3s
   - Move from Y=-100 to Y=120
   - Ease Out
2. Create `Anim_SlideOut`:
   - Duration: 0.3s
   - Move to Y=-100
   - Ease In
3. Create `Anim_FadeOut`:
   - Duration: 0.5s
   - Opacity 1 to 0

#### Step 9: Setup Notification Types (Graph)
1. Enum: NotificationType (Success, Warning, Error, Info)
2. Function: `ShowNotification(String Message, NotificationType Type, Float Duration)`
3. Set icon and color based on type:
   - Success: Green
   - Warning: Yellow
   - Error: Red
   - Info: Blue
4. Auto-hide after Duration seconds
5. Button_Close → OnClicked → Hide immediately
6. Compile and Save

**Estimated Time**: 25-30 minutes

---

### Widget 18: UI_DialogueBox (Bonus - Not in original spec but useful)
**Category**: UI Feedback | **Time**: 35-40 minutes | **Priority**: LOW

Note: This widget is implied by the dialogue system but not explicitly listed. Including for completeness.

#### Step 1: Create Widget Blueprint
1. Path: `/Game/UI/Generated/ui_feedback/`
2. Name: `WBP_UI_DialogueBox`

#### Step 2: Canvas Setup
1. Root Canvas Panel
2. Anchors: Bottom-Center
3. Size: X=900, Y=200

#### Step 3: Add Border Background
1. **Border** → Fill
2. Color: R=0.05, G=0.05, B=0.1, A=0.9
3. Padding: 20

#### Step 4: Add Horizontal Box
1. **Horizontal Box** → `HBox_Main`

#### Step 5: Add Speaker Portrait
1. **Image** → `Image_SpeakerPortrait`
2. Size: 120x120
3. **Slot**: Size=Auto, Right padding=20

#### Step 6: Add Vertical Box for Text
1. **Vertical Box** → `VBox_Dialogue`
2. **Slot**: Fill

#### Step 7: Add Speaker Name
1. **Text** → `Text_SpeakerName`
2. Text: `Speaker Name`
3. Font: Size=20, Bold
4. Color: R=1, G=0.9, B=0.3
5. **Slot**: Bottom padding=10

#### Step 8: Add Dialogue Text
1. **Text** → `Text_Dialogue`
2. Text: `Dialogue text appears here...`
3. Font: Size=16
4. Auto Wrap Text: True
5. **Slot**: Fill

#### Step 9: Add Choice Buttons Container
1. **Vertical Box** → `VBox_Choices`
2. Initially hidden
3. Contains player dialogue choices

#### Step 10: Setup Dialogue System (Graph)
1. Function: `ShowDialogue(String Speaker, Texture2D Portrait, String DialogueText)`
2. Function: `ShowChoices(Array<String> Choices)`
3. Function: `HideDialogue()`
4. Typewriter text effect (optional)
5. Compile and Save

**Estimated Time**: 35-40 minutes

**Total UI Feedback Time**: 60-70 minutes (including bonus widget)

---

## UMG Best Practices for VR

### General VR UI Guidelines

#### 1. Widget World Space Setup
All VR widgets should be placed in world space using Widget Component:

```cpp
// In C++ or Blueprint
UWidgetComponent* WidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
WidgetComp->SetWidgetSpace(EWidgetSpace::World);
WidgetComp->SetDrawSize(FVector2D(1000, 600)); // Physical size in world units
WidgetComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
```

#### 2. Size and Distance
- **Recommended Widget Size**: 1000-2000 UE units wide
- **Viewing Distance**: 100-300 cm from player
- **Text Size**: Minimum 24pt for readability in VR
- **Button Size**: Minimum 80x80 units for comfortable interaction

#### 3. VR-Specific Widget Properties

**For HUD Widgets**:
- Attach to VR Camera/HMD
- Use **Screen Space** widgets sparingly (causes discomfort)
- Prefer **World Space** attached to wrist/dashboard

**For Menu Widgets**:
- Place in world at comfortable reading distance (150-200 cm)
- Angle slightly toward player (10-15 degrees)
- Use curved panels for immersive menus

#### 4. Color and Contrast
- High contrast ratios (minimum 4.5:1)
- Avoid pure white (use R=0.9, G=0.9, B=0.9)
- Avoid pure black (use R=0.1, G=0.1, B=0.1)
- No red text on green backgrounds (colorblind friendly)

#### 5. Animation and Motion
- Avoid rapid animations (can cause nausea)
- Limit parallax effects
- Smooth transitions (0.3-0.5 second duration)
- No camera shake when UI appears

#### 6. Interaction Design
- Use **Widget Interaction Component** for pointer-based interaction
- Visual feedback on hover (color change, scale, glow)
- Audio feedback on click
- Clear selected state
- Support both motion controllers and gamepad

#### 7. VR HUD Positioning
```
Recommended HUD Layout for VR:

     [Target Info]          [Minimap]
                             [Fuel]
                             [Shield]
                             [Health]

[Speed]                      [Weapon]
[Altitude]
[Objectives]
```

#### 8. Performance Optimization
- Use **Widget Pooling** for frequently created/destroyed widgets
- Invalidation boxes to reduce redraw overhead
- Limit tick functions (use Event-Based updates)
- Atlas textures for icons
- Mip-mapping for widget textures

#### 9. VR-Specific Considerations
- **Stereoscopic Rendering**: Widgets render for both eyes
- **IPD Variance**: Test with different interpupillary distances
- **Field of View**: Keep important info within 60-degree cone
- **Depth Perception**: Use subtle shadows and outlines

---

## Data Binding Setup

### Creating Bindable Properties

#### Method 1: Function Binding (Recommended for Dynamic Data)

1. In **Graph** view of widget
2. Create function matching data type (e.g., `GetSpeedText` returns Text)
3. In **Designer** view, select widget property
4. Click **Bind** dropdown → **Create Binding**
5. Select your function

Example:
```
Function: GetSpeedText
Return: Text
Body: Return Text::FromString(FString::Printf(TEXT("%d m/s"), CurrentSpeed))
```

#### Method 2: Property Binding (Recommended for Variables)

1. Create variable in widget (e.g., `CurrentSpeed` Float)
2. Make variable **Instance Editable** and **Expose on Spawn**
3. In Designer, bind widget property to variable
4. Update variable from parent blueprint/C++

#### Method 3: Event-Driven Updates (Best Performance)

1. Create **Event Dispatcher** in player character
2. Create public function in widget (e.g., `UpdateSpeed(Float NewSpeed)`)
3. Bind event dispatcher to widget function
4. Call dispatcher when data changes

Example Event Dispatcher Setup:
```
Character Blueprint:
- Event Dispatcher: OnSpeedChanged(Float NewSpeed)
- When speed changes → Call OnSpeedChanged

Widget Blueprint:
- Event Construct → Bind OnSpeedChanged to UpdateSpeed
- Function UpdateSpeed(Float) → Update UI elements
```

### Binding Best Practices

1. **Use Event-Driven over Tick**: Don't update UI every frame
2. **Cache References**: Store widget references instead of searching
3. **Batch Updates**: Update multiple properties in single function call
4. **Validate Data**: Check for null/invalid before binding
5. **Unbind on Destroy**: Prevent memory leaks

---

## Troubleshooting

### Common Issues and Solutions

#### Issue 1: Widget Not Appearing in Viewport
**Symptoms**: Widget blueprint created but not visible in game

**Solutions**:
1. Check Z-Order (right-click in Hierarchy → Bring to Front)
2. Verify parent widget is visible
3. Check widget Visibility property (not Collapsed/Hidden)
4. Ensure widget is added to viewport in code:
   ```cpp
   UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
   Widget->AddToViewport();
   ```

#### Issue 2: Text Not Displaying Correctly
**Symptoms**: Text appears as boxes, wrong size, or clipped

**Solutions**:
1. Check font is imported correctly
2. Verify text size fits container
3. Enable **Auto Wrap Text** for long strings
4. Check **Size to Content** if text should expand container
5. Verify canvas panel has sufficient size

#### Issue 3: Buttons Not Clickable
**Symptoms**: Button hover/click events not firing

**Solutions**:
1. Verify button **Is Enabled** is true
2. Check **Visibility** is not "Not Hit-Testable"
3. Ensure no overlapping widget blocking input
4. Check **Event Graph** has OnClicked event bound
5. Verify PlayerController has UI input enabled:
   ```cpp
   PlayerController->bShowMouseCursor = true;
   PlayerController->bEnableClickEvents = true;
   ```

#### Issue 4: Progress Bar Not Updating
**Symptoms**: Progress bar stays at default value

**Solutions**:
1. Check binding is correctly set up
2. Verify Percent property is being updated (0.0 to 1.0 range)
3. Check Fill Type is set correctly
4. Ensure material supports progress (if using custom material)

#### Issue 5: VR Widget Not Visible
**Symptoms**: Widget component not showing in VR

**Solutions**:
1. Verify **Widget Space** is set to World
2. Check **Draw Size** is appropriate (e.g., 1000x600)
3. Ensure widget is within view distance
4. Check **Pivot** is set correctly (typically center)
5. Verify **Blend Mode** is appropriate (typically Masked or Transparent)

#### Issue 6: Performance Issues / Lag
**Symptoms**: UI causes frame drops

**Solutions**:
1. Reduce **Tick** functions in widgets
2. Use **Invalidation Boxes** to cache rendered sections
3. Limit **Auto Wrap Text** on large text blocks
4. Optimize material complexity
5. Use **Widget Pooling** for repeated elements
6. Check for expensive bindings (avoid per-frame calculations)

#### Issue 7: Blurry Text in VR
**Symptoms**: Text difficult to read in VR headset

**Solutions**:
1. Increase font size (minimum 24pt)
2. Use **Clear Type** fonts
3. Increase widget component **Draw Size**
4. Enable **Mip Maps** on widget texture
5. Position widget at optimal viewing distance
6. Use high-contrast colors

#### Issue 8: Widget Stretching/Distortion
**Symptoms**: Widget appears stretched or squished

**Solutions**:
1. Check **Anchors** are set correctly
2. Verify **Alignment** matches intended layout
3. Use **Size to Content** for auto-sizing
4. Check parent container (Canvas/Box) size
5. Verify **Aspect Ratio** is maintained

#### Issue 9: Scroll Box Not Scrolling
**Symptoms**: Scroll box content not scrollable

**Solutions**:
1. Ensure content exceeds scroll box size
2. Check **Scroll Bar Visibility** is not Hidden
3. Verify **Orientation** is set correctly
4. Check for overlapping input-blocking widgets
5. Ensure **Consume Mouse Wheel** is enabled

#### Issue 10: Animation Not Playing
**Symptoms**: Widget animation doesn't play when triggered

**Solutions**:
1. Verify animation is not 0 duration
2. Check animation is being called (add log/breakpoint)
3. Ensure widget is visible when animation plays
4. Check animation **Play Mode** (Forward, Reverse, PingPong)
5. Verify keyframes are set on correct properties

---

## Validation Checklist

### Pre-Creation Validation

- [ ] All required plugins enabled (UMG, Widget Component)
- [ ] Directory structure created (`/Game/UI/Generated/hud/`, `/menu/`, `/tutorial/`, `/ui_feedback/`)
- [ ] Master materials created (M_ProgressBar_Master, M_UI_Background)
- [ ] Icon assets imported to `/Game/UI/Icons/`
- [ ] Background textures prepared

### Per-Widget Validation

For each widget created, verify:

#### Structure
- [ ] Widget Blueprint named correctly (WBP_ prefix)
- [ ] Saved to correct category folder
- [ ] Root container is correct type (Canvas Panel, Overlay, etc.)
- [ ] Component hierarchy matches specification
- [ ] All components are named descriptively

#### Layout
- [ ] Anchors set appropriately
- [ ] Position values correct
- [ ] Size values correct
- [ ] Alignment values correct
- [ ] Padding/margins applied
- [ ] Z-order correct (no unwanted overlaps)

#### Appearance
- [ ] Colors match specification
- [ ] Fonts are correct size and style
- [ ] Icons/images assigned
- [ ] Opacity/transparency correct
- [ ] Border styles applied
- [ ] Background materials assigned

#### Functionality
- [ ] All buttons have click events
- [ ] Data bindings are set up
- [ ] Progress bars update correctly
- [ ] Text formatting is correct
- [ ] Animations created (if specified)
- [ ] Visibility logic implemented

#### VR Compatibility
- [ ] Text size minimum 24pt
- [ ] High contrast colors used
- [ ] Widget component configured for VR
- [ ] Appropriate draw size set
- [ ] Comfortable viewing distance
- [ ] No rapid animations

### Post-Creation Validation

#### Testing in Editor
- [ ] Widget displays in preview window
- [ ] All elements visible and positioned correctly
- [ ] Hover states work on buttons
- [ ] Bindings update when test values changed
- [ ] Animations play smoothly
- [ ] No console errors/warnings

#### Testing in PIE (Play in Editor)
- [ ] Widget appears in game viewport
- [ ] Data updates from game logic
- [ ] Buttons respond to clicks
- [ ] Scroll boxes scroll properly
- [ ] Performance is acceptable (no frame drops)
- [ ] No visual glitches

#### Testing in VR (if applicable)
- [ ] Widget visible in VR headset
- [ ] Text is readable
- [ ] Comfortable viewing distance
- [ ] Interaction with motion controllers works
- [ ] No discomfort/nausea from UI
- [ ] Performance maintains target framerate

#### Integration Testing
- [ ] Widget integrates with game systems (HUD with ship, menus with game mode, etc.)
- [ ] Data flows correctly from C++ or Blueprints
- [ ] Events fire to game logic
- [ ] Widget appears/disappears when expected
- [ ] Multiple widgets don't conflict

### Final Validation

#### Complete Widget Set
- [ ] All 18 widgets created
- [ ] All widgets tested individually
- [ ] Widgets tested together (no conflicts)
- [ ] Consistent styling across all widgets
- [ ] Performance acceptable with all widgets active

#### Documentation
- [ ] All widgets documented with comments in blueprints
- [ ] Variable names are clear and descriptive
- [ ] Function names follow conventions
- [ ] Any special setup noted

#### Version Control
- [ ] All widget blueprints saved
- [ ] Assets checked into version control
- [ ] No temporary or test widgets left in project
- [ ] Content Browser organized

---

## Widget Creation Priority Order

Based on gameplay importance and dependencies:

### Phase 1: Critical HUD (Day 1-2) - 3-4 hours
1. **HUD_SpeedIndicator** - Essential for flight
2. **HUD_AltitudeDisplay** - Essential for landing
3. **HUD_FuelGauge** - Essential for resource management
4. **HUD_ShieldBar** - Essential for combat
5. **HUD_HealthBar** - Essential for survival

### Phase 2: Gameplay HUD (Day 2-3) - 2-3 hours
6. **HUD_ObjectiveTracker** - Important for missions
7. **HUD_TargetInfo** - Important for combat/docking
8. **HUD_WeaponStatus** - Important for combat
9. **HUD_Minimap** - Nice to have for navigation

### Phase 3: Tutorial System (Day 3) - 30-45 minutes
10. **UI_TutorialOverlay** - Important for onboarding

### Phase 4: Core Menus (Day 4-5) - 3-4 hours
11. **UI_MainMenu** - Essential for game start
12. **UI_MissionBoard** - Core gameplay loop
13. **UI_MissionCard** - Dependency of MissionBoard
14. **UI_SettingsMenu** - Essential for configuration

### Phase 5: Advanced Menus (Day 6-7) - 3-4 hours
15. **UI_TradingInterface** - Major gameplay system
16. **UI_ShipCustomization** - Major gameplay system

### Phase 6: Feedback Systems (Day 7) - 1 hour
17. **UI_Notification** - Important for player communication

---

## Estimated Time Summary

### Time Breakdown by Category
| Category | Widgets | Estimated Time |
|----------|---------|----------------|
| HUD Components | 9 | 4-6 hours |
| Menu Systems | 6 | 5-7 hours |
| Tutorial System | 1 | 30-45 minutes |
| UI Feedback | 2 | 45-60 minutes |
| **TOTAL** | **18** | **10-14 hours** |

### Time Breakdown by Priority
| Priority | Widgets | Estimated Time |
|----------|---------|----------------|
| HIGH | 11 | 7-9 hours |
| MEDIUM | 5 | 3-4 hours |
| LOW | 2 | 1-2 hours |
| **TOTAL** | **18** | **11-15 hours** |

### Realistic Schedule
- **Beginner UMG User**: 16-20 hours (with learning curve)
- **Intermediate UMG User**: 12-16 hours
- **Advanced UMG User**: 10-12 hours
- **Expert UMG User**: 8-10 hours

### Daily Schedule (Intermediate User)
- **Day 1** (3 hours): Phase 1 - Critical HUD (Widgets 1-5)
- **Day 2** (3 hours): Phase 2 - Gameplay HUD (Widgets 6-9)
- **Day 3** (2 hours): Phase 3 - Tutorial + Start Menus (Widget 10-11)
- **Day 4** (3 hours): Phase 4 - Core Menus (Widgets 12-14)
- **Day 5** (3 hours): Phase 5 - Advanced Menus (Widgets 15-16)
- **Day 6** (2 hours): Phase 6 - Feedback + Testing (Widget 17 + validation)

**Total**: 16 hours over 6 days

---

## Additional Resources

### Useful Unreal Engine Documentation
- UMG Best Practices: https://docs.unrealengine.com/en-US/umg-best-practices/
- Widget Blueprints: https://docs.unrealengine.com/en-US/widget-blueprints/
- VR UI Guidelines: https://docs.unrealengine.com/en-US/vr-ui-best-practices/

### Recommended Fonts for VR
- Roboto (good readability)
- Montserrat (modern, clean)
- Orbitron (sci-fi themed)
- Exo 2 (futuristic)

### Color Palette Recommendations
```
Primary UI: R=0.1, G=0.1, B=0.15 (Dark Blue-Grey)
Accent: R=1.0, G=0.9, B=0.3 (Gold)
Success: R=0.2, G=0.8, B=0.3 (Green)
Warning: R=1.0, G=0.6, B=0.2 (Orange)
Danger: R=0.8, G=0.2, B=0.2 (Red)
Info: R=0.3, G=0.7, B=1.0 (Blue)
```

---

## Quick Reference: Menu Paths

### Creating Widget Blueprint
1. Content Browser → Right-click
2. User Interface → Widget Blueprint
3. Name with WBP_ prefix
4. Save to appropriate category folder

### Adding Components
1. Open Widget Blueprint
2. Palette panel (left side)
3. Drag component to Hierarchy or Designer view
4. Configure in Details panel (right side)

### Setting up Bindings
1. Designer view → Select widget property
2. Click Bind dropdown
3. Create Binding → Create function
4. Or select existing function/property

### Creating Animations
1. Animations panel (bottom)
2. Click +Animation button
3. Add tracks for properties to animate
4. Set keyframes
5. Configure duration and loop settings

### Testing Widget
1. Click Compile button
2. Use Preview window in Designer
3. Or Play in Editor (PIE) to test in game
4. File → Save

---

## Conclusion

This guide provides comprehensive instructions for manually creating all 18 UMG widgets for the Alexander autonomous baseline. Each widget includes detailed step-by-step instructions, property values, and best practices.

**Remember**:
- Start with high-priority HUD elements
- Test frequently during creation
- Follow VR best practices for comfort
- Use consistent naming conventions
- Document any deviations or customizations

For questions or issues not covered in the Troubleshooting section, consult Unreal Engine documentation or community forums.

**Good luck with your UI creation!**
