# Ship Customization Widgets - Quick Reference

## Widget Overview

| Widget Name | Purpose | Size (Desktop) | Size (VR) | Complexity |
|-------------|---------|----------------|-----------|------------|
| **WBP_CustomizationMenu** | Main container | 1728×972 | 3456×1944 | High |
| **WBP_PartSelectionPanel** | Part browser | 300×Fill | 600×Fill | Medium |
| **WBP_PartDetailPanel** | Part details | Fill×Fill | Fill×Fill | Medium |
| **WBP_PartListEntry** | List item (reusable) | Fill×64 | Fill×128 | Low |
| **WBP_StatsComparison** | Stats before/after | 300×Fill | 600×Fill | Medium |
| **WBP_LoadoutPanel** | Loadout manager | 300×Fill | 600×Fill | Medium |
| **WBP_ProgressionDisplay** | Level/XP/Credits HUD | Auto×Auto | Auto×Auto | Low |
| **WBP_UnlockDialog** | Purchase modal | 500×600 | 1000×1200 | Low |

---

## Color Quick Reference

### Rarity Colors
```cpp
Common:    #4CAF50  // Green
Uncommon:  #2196F3  // Blue
Rare:      #9C27B0  // Purple
Epic:      #FF9800  // Orange
Legendary: #F44336  // Red
```

### UI Colors
```cpp
// Backgrounds
PrimaryBG:    #1A1A1A
SecondaryBG:  #2A2A2A
Borders:      #424242

// Accents
Accent:       #00BCD4  // Cyan
Success:      #4CAF50  // Green
Warning:      #FFC107  // Amber
Error:        #F44336  // Red

// Text
TextPrimary:  #FFFFFF
TextSecondary:#BDBDBD
TextDisabled: #9E9E9E
```

---

## Font Sizes Quick Reference

| Element | Desktop | VR |
|---------|---------|-----|
| Headers | 32pt | 64pt |
| Subheaders | 24pt | 48pt |
| Body Text | 16pt | 32pt |
| Small Text | 12pt | 24pt |
| Part Names | 28pt | 56pt |
| Buttons | 16pt | 32pt |

---

## Button Sizes Quick Reference

| Type | Desktop | VR |
|------|---------|-----|
| Standard Button | 120×40 | 240×80 |
| Action Button | 140×44 | 280×88 |
| Large Button | Fill×48 | Fill×96 |
| Close Button | 40×40 | 80×80 |
| Category Tab | 140×36 | 280×72 |
| Slot Button | 80×32 | 160×64 |

---

## Common Materials

```
M_UI_BackgroundBlur
├── Domain: User Interface
├── Blend Mode: Translucent
├── Input: Scene Texture (PostProcessInput0)
├── Effect: Gaussian Blur (Strength: 5)
└── Opacity: 0.6

M_CircleGradient
├── Domain: User Interface
├── Type: Radial Gradient Exponential
├── Center: (0.5, 0.5)
├── Radius: 0.4
└── Parameter: TintColor (Vector)
```

---

## Required Textures

| Texture | Size | Format | Usage |
|---------|------|--------|-------|
| T_Icon_Credits | 32×32 / 64×64 | RGBA | Currency icon |
| T_Icon_Lock | 32×32 / 64×64 | RGBA | Locked status |
| T_Icon_Checkmark | 32×32 / 64×64 | RGBA | Equipped status |
| T_Arrow_Up | 16×16 / 32×32 | RGBA | Stat increase |
| T_Arrow_Down | 16×16 / 32×32 | RGBA | Stat decrease |
| T_Arrow_Neutral | 16×16 / 32×32 | RGBA | No change |
| T_Icon_Search | 16×16 / 32×32 | RGBA | Search box |

All textures: Compression = UserInterface2D, MipGen = NoMipmaps, sRGB = True

---

## Animation Timing Quick Reference

| Animation | Duration | Easing |
|-----------|----------|--------|
| Menu Open | 0.3s | Ease Out Cubic |
| Menu Close | 0.2s | Ease In Cubic |
| Button Hover | 0.1s | Linear |
| Button Click | 0.05s | Linear |
| Part Changed | 0.3s | Ease In Out |
| Level Up | 1.0s | Ease Out Elastic |
| XP Gained | 0.5s | Ease Out Cubic |
| Credits Gained | 0.6s | Ease Out Cubic |
| Credits Spent | 0.4s | Ease In Cubic |
| Purchase Success | 0.6s | Ease Out Back |
| Insufficient Funds | 0.4s | Linear (Shake) |
| Stats Updated | 0.4s | Ease In Out |
| Loadout Load | 0.4s | Ease Out Back |

---

## Event Dispatcher Signatures

```cpp
// WBP_PartSelectionPanel
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartSelected, UShipPartData*, PartData);

// WBP_PartListEntry
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPartEntryClicked, UShipPartData*, PartData);

// WBP_PartDetailPanel
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnlockRequested, UShipPartData*, PartData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipRequested, UShipPartData*, PartData);

// WBP_StatsComparison
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnApplyChangesClicked);

// WBP_LoadoutPanel
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadoutLoaded, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadoutSaved, int32, SlotIndex);

// WBP_UnlockDialog
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnlockConfirmed, UShipPartData*, PartData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUnlockCancelled);
```

---

## Common Functions

### GetRarityColor (Pure Function)
```cpp
Input: EShipPartRarity Rarity
Output: FLinearColor

Switch on Rarity:
  Common    -> return #4CAF50
  Uncommon  -> return #2196F3
  Rare      -> return #9C27B0
  Epic      -> return #FF9800
  Legendary -> return #F44336
  Default   -> return #FFFFFF
```

### UpdateProgression
```cpp
Inputs: int32 Level, int32 XP, int32 NextLevelXP

If Level > PlayerLevel:
  PlayLevelUpAnimation()
Set PlayerLevel = Level
Set CurrentXP = XP
Update progress bars
```

### CanAfford (Pure Function)
```cpp
Output: bool

return (PlayerCredits >= UnlockCost);
```

---

## VR Input Mapping

```cpp
// Action Mappings
UI_Click
├── Vive Controller: Trigger
├── Oculus Touch: Index Trigger
└── Mouse: Left Mouse Button

UI_Back
├── Vive Controller: Grip
├── Oculus Touch: B Button
└── Keyboard: Escape

// Widget Interaction
Source: World
Distance: 500 cm
Debug: Show Debug = True (testing only)
```

---

## Widget Component Settings

```cpp
// For VR Hand Attachment
DrawSize: 2000 × 1500
Pivot: 0.5, 0.5
GeometryMode: Plane
BlendMode: Transparent
TwoSided: False
BackgroundColor: Transparent
```

---

## Binding Examples

### Text Binding
```cpp
// Simple property
Text_PartName.Text = PartData->PartName

// Formatted
Text_XPValue.Text = FString::Printf(TEXT("%d/%d"), CurrentXP, NextLevelXP)

// With number formatting
Text_CreditsValue.Text = FText::AsNumber(Credits, FNumberFormattingOptions::DefaultWithGrouping())
```

### Color Binding
```cpp
// Simple
Border_RarityBadge.BrushColor = GetRarityColor()

// Conditional
Text_AfterValue.ColorAndOpacity = CanAfford() ? Green : Red
```

### Visibility Binding
```cpp
// Boolean to visibility
Image_LockedIcon.Visibility = bIsLocked ? ESlateVisibility::Visible : ESlateVisibility::Hidden

// Function-based
Button_Unlock.Visibility = GetUnlockButtonVisibility()
```

---

## Performance Targets

| Metric | Desktop | VR |
|--------|---------|-----|
| Frame Rate | 60 FPS | 90 FPS (120 FPS for Index/Quest 2) |
| Widget Draw Calls | < 100 | < 50 |
| Texture Memory | < 100 MB | < 200 MB |
| Update Frequency | 60 Hz | 30 Hz (stats comparison) |
| List Pooling | 20-50 widgets | 20-50 widgets |

---

## Testing Commands

```cpp
// PIE Testing
- Resolution: 1920×1080, 2560×1440, 3840×2160
- Performance: stat fps, stat unit
- Memory: stat memory
- Texture streaming: stat streaming

// VR Testing
- VR Preview in editor
- Standalone VR game
- Check frame timing: stat vr
- Verify interaction distance
- Test haptic feedback
```

---

## Common Issues and Solutions

| Issue | Solution |
|-------|----------|
| Widget not appearing | Check Z-Order, Visibility, Anchors |
| Buttons not responding | Check input mode, blocking widgets |
| Bindings not updating | Check Pure functions, Invalidation mode |
| Animations stuttering | Reduce animation count, check tick rate |
| Text not showing | Check font, color vs background, size box |
| VR widget too small | Check draw size, scale multipliers |
| VR buttons hard to click | Increase hit boxes, check laser distance |
| Memory leak | Check widget pooling, clear references |

---

## File Locations

### Documentation
```
/CUSTOMIZATION_WIDGETS_SPECIFICATION.md  (51 KB)
/widget_blueprints_guide.md             (37 KB)
/PHASE3_WIDGET_DOCUMENTATION_COMPLETE.md (18 KB)
/WIDGET_QUICK_REFERENCE.md              (This file)
```

### JSON Specs
```
/Content/UI/WidgetSpecs/
├── WBP_CustomizationMenu_spec.json     (7 KB)
├── WBP_PartSelectionPanel_spec.json    (8 KB)
├── WBP_PartDetailPanel_spec.json       (11 KB)
├── WBP_PartListEntry_spec.json         (9 KB)
├── WBP_StatsComparison_spec.json       (12 KB)
├── WBP_LoadoutPanel_spec.json          (13 KB)
├── WBP_ProgressionDisplay_spec.json    (11 KB)
└── WBP_UnlockDialog_spec.json          (15 KB)
```

---

## Implementation Order

1. **Assets First**
   - Import fonts
   - Create/import icons
   - Create materials

2. **Bottom-Up Widget Creation**
   - WBP_PartListEntry (no dependencies)
   - WBP_PartSelectionPanel (uses PartListEntry)
   - WBP_PartDetailPanel
   - WBP_StatsComparison
   - WBP_LoadoutPanel
   - WBP_ProgressionDisplay
   - WBP_UnlockDialog
   - WBP_CustomizationMenu (uses all others)

3. **Integration**
   - Create test data
   - Wire up subsystem
   - Implement save/load
   - Add input bindings

4. **VR Setup**
   - Create VR variants
   - Add widget components
   - Configure interaction
   - Test in VR headset

5. **Polish**
   - Add all animations
   - Optimize performance
   - Add audio/haptics
   - Final testing

---

## Quick Start Checklist

Desktop Implementation:
- [ ] Import Roboto fonts
- [ ] Create UI icon textures
- [ ] Create M_UI_BackgroundBlur material
- [ ] Create M_CircleGradient material
- [ ] Create WBP_PartListEntry
- [ ] Create WBP_PartSelectionPanel
- [ ] Create WBP_PartDetailPanel
- [ ] Create WBP_StatsComparison
- [ ] Create WBP_CustomizationMenu
- [ ] Create test ShipPartData assets
- [ ] Wire up to ShipCustomizationSubsystem
- [ ] Test in PIE

VR Addition:
- [ ] Scale all elements 2x
- [ ] Add Widget Component to VR pawn
- [ ] Add Widget Interaction Component
- [ ] Configure VR input mapping
- [ ] Test pointer interaction
- [ ] Add haptic feedback
- [ ] Verify 90+ FPS
- [ ] Test in VR headset

---

## Support Resources

- **Main Spec:** CUSTOMIZATION_WIDGETS_SPECIFICATION.md
- **Guide:** widget_blueprints_guide.md
- **Complete Summary:** PHASE3_WIDGET_DOCUMENTATION_COMPLETE.md
- **Individual Specs:** Content/UI/WidgetSpecs/*.json

**Estimated Implementation Time:** 30-46 hours (4-6 days)

**Last Updated:** November 7, 2025
