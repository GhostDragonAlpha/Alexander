# Progression Widgets Specification

## Overview

This document specifies the UI widgets for the ship customization progression system, including progression displays, notification toasts, and unlock dialogs.

---

## 1. WBP_ProgressionDisplay

**Purpose:** Main HUD element displaying player progression, XP, and currency.

**Location:** Top-right corner of screen (VR: positioned in world space at comfortable viewing angle)

### Widget Hierarchy

```
WBP_ProgressionDisplay (User Widget)
├── Canvas Panel (Root)
│   ├── CircularLevelIndicator (Overlay)
│   │   ├── BackgroundCircle (Image - circular border)
│   │   ├── FillCircle (Image - animated radial fill)
│   │   └── LevelText (Text Block - "LEVEL 12")
│   ├── XPProgressContainer (Vertical Box)
│   │   ├── XPBar (Progress Bar)
│   │   │   ├── FillStyle: Gradient (Blue to Cyan)
│   │   │   └── AnimatedFill: True
│   │   ├── XPText (Text Block - "1,450 / 2,000 XP")
│   │   └── NextMilestone (Text Block - "Next: Level 13")
│   └── CreditsDisplay (Horizontal Box)
│       ├── CreditIcon (Image - currency icon)
│       └── CreditsText (Text Block - "12,450 CR")
```

### Visual Specifications

**Circular Level Indicator:**
- Diameter: 80px (VR: 120px)
- Border width: 4px
- Fill animation: Clockwise radial fill from top
- Background color: #424242 (50% opacity)
- Fill color: Gradient (#1E88E5 → #64B5F6)
- Level text: Roboto Bold, 18pt (VR: 22pt), Center aligned
- Glow effect on level-up: #1E88E5, 8px blur, 2s duration

**XP Progress Bar:**
- Width: 200px (VR: 280px)
- Height: 12px (VR: 16px)
- Border radius: 6px
- Background: #212121
- Fill gradient: #43A047 → #66BB6A
- Border: 2px solid #424242
- Animation: Smooth lerp fill, 1000ms duration
- Count-up animation on XP gain

**XP Text:**
- Font: Roboto Regular, 12pt (VR: 16pt)
- Color: #FFFFFF (90% opacity)
- Format: "{current} / {total} XP"
- Animate on value change: Flash #43A047 for 300ms

**Next Milestone:**
- Font: Roboto Medium, 10pt (VR: 14pt)
- Color: #64B5F6
- Format: "Next: Level {level}"

**Credits Display:**
- Icon size: 24px × 24px (VR: 32px × 32px)
- Icon: Gold coin symbol
- Text font: Roboto Medium, 14pt (VR: 18pt)
- Text color: #FDD835
- Padding: 8px between icon and text
- Animate on value change: Scale pulse (1.0 → 1.15 → 1.0), 400ms

### Blueprint Properties

```cpp
UCLASS()
class UWBPProgressionDisplay : public UUserWidget
{
    GENERATED_BODY()

public:
    // Bindable properties
    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UImage* BackgroundCircle;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UImage* FillCircle;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UTextBlock* LevelText;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UProgressBar* XPBar;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UTextBlock* XPText;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UTextBlock* NextMilestone;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UImage* CreditIcon;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UTextBlock* CreditsText;

    // Public methods
    UFUNCTION(BlueprintCallable, Category="Progression")
    void UpdateLevel(int32 NewLevel);

    UFUNCTION(BlueprintCallable, Category="Progression")
    void UpdateXP(int32 CurrentXP, int32 TotalXP, bool bAnimate = true);

    UFUNCTION(BlueprintCallable, Category="Progression")
    void UpdateCredits(int32 NewCredits, bool bAnimate = true);

    UFUNCTION(BlueprintCallable, Category="Progression")
    void PlayLevelUpAnimation();

private:
    // Animation curves
    UPROPERTY(EditDefaultsOnly, Category="Animation")
    UCurveFloat* XPFillCurve;

    UPROPERTY(EditDefaultsOnly, Category="Animation")
    UCurveFloat* LevelUpScaleCurve;

    // Cached values for interpolation
    float CachedXPProgress;
    int32 CachedCredits;
};
```

### Animation Specifications

**Level-Up Animation Sequence:**
1. Circular fill flashes white (100ms)
2. Scale pulse: 1.0 → 1.3 → 1.0 (600ms, bounce easing)
3. Particle burst effect (gold sparkles, 1.5s)
4. Glow effect fades in and out (2s total)
5. Level text counts up with number increment effect

**XP Gain Animation:**
1. XP bar fills smoothly using lerp (1000ms)
2. XP text counts up simultaneously
3. If XP bar fills completely, trigger level-up animation
4. Progress bar fill uses ease-out cubic curve

**Credits Change Animation:**
1. Scale pulse on icon (1.0 → 1.15 → 1.0, 400ms)
2. Text counts up/down to new value (500ms)
3. Flash color based on change:
   - Gain: #43A047 (green)
   - Loss: #E53935 (red)

---

## 2. WBP_NotificationToast

**Purpose:** Slide-in notification system for unlocks, level-ups, and achievements.

**Location:** Top-center of screen, slides down from above

### Widget Hierarchy

```
WBP_NotificationToast (User Widget)
├── Canvas Panel (Root)
│   └── NotificationContainer (Border)
│       ├── BackgroundImage (Image - gradient background)
│       ├── ContentBox (Horizontal Box)
│       │   ├── IconContainer (Size Box - 64×64)
│       │   │   └── NotificationIcon (Image)
│       │   ├── Spacer (8px)
│       │   └── TextContainer (Vertical Box)
│       │       ├── TitleText (Text Block - "PART UNLOCKED!")
│       │       └── DescriptionText (Text Block - "Plasma Cannon Mk II")
│       └── RarityBorder (Border - colored based on rarity)
```

### Visual Specifications

**Container:**
- Width: 400px (VR: 500px)
- Height: Auto (min 80px)
- Background: Linear gradient (#424242 → #212121)
- Border radius: 12px
- Shadow: 0px 4px 16px rgba(0,0,0,0.6)
- Padding: 16px

**Icon:**
- Size: 64×64px (VR: 80×80px)
- Background: Circular, color based on notification type
  - Unlock: #1E88E5 (blue)
  - Level-up: #FDD835 (yellow)
  - Achievement: #9C27B0 (purple)
- Border: 3px solid #FFFFFF

**Title Text:**
- Font: Roboto Bold, 16pt (VR: 20pt)
- Color: #FFFFFF
- All caps
- Drop shadow: 2px offset, 50% black

**Description Text:**
- Font: Roboto Regular, 14pt (VR: 18pt)
- Color: #E0E0E0
- Margin-top: 4px

**Rarity Border:**
- Border width: 3px (left edge only)
- Colors based on rarity:
  - Common: #9E9E9E (gray)
  - Uncommon: #4CAF50 (green)
  - Rare: #2196F3 (blue)
  - Epic: #9C27B0 (purple)
  - Legendary: #FF9800 (orange)

### Blueprint Properties

```cpp
UCLASS()
class UWBPNotificationToast : public UUserWidget
{
    GENERATED_BODY()

public:
    // Bindable properties
    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UBorder* NotificationContainer;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UImage* NotificationIcon;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UTextBlock* TitleText;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UTextBlock* DescriptionText;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UBorder* RarityBorder;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Toast")
    float DisplayDuration = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Toast")
    float SlideInDuration = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Toast")
    float SlideOutDuration = 0.3f;

    // Public methods
    UFUNCTION(BlueprintCallable, Category="Toast")
    void ShowUnlockNotification(const FString& ItemName, EItemRarity Rarity, UTexture2D* Icon);

    UFUNCTION(BlueprintCallable, Category="Toast")
    void ShowLevelUpNotification(int32 NewLevel);

    UFUNCTION(BlueprintCallable, Category="Toast")
    void ShowAchievementNotification(const FString& AchievementName, UTexture2D* Icon);

    UFUNCTION(BlueprintCallable, Category="Toast")
    void PlaySlideInAnimation();

    UFUNCTION(BlueprintCallable, Category="Toast")
    void PlaySlideOutAnimation();

protected:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnSlideInComplete();

    UFUNCTION()
    void OnSlideOutComplete();

private:
    FTimerHandle AutoDismissTimer;

    UPROPERTY(Transient)
    UWidgetAnimation* SlideInAnim;

    UPROPERTY(Transient)
    UWidgetAnimation* SlideOutAnim;
};
```

### Animation Specifications

**Slide-In Animation (400ms):**
1. Start position: Y = -100px (above screen)
2. End position: Y = 20px (visible)
3. Easing: Ease-out cubic
4. Opacity: 0.0 → 1.0 (first 200ms)
5. Scale: 0.95 → 1.0 (subtle)

**Slide-Out Animation (300ms):**
1. Start position: Current Y position
2. End position: Y = -100px
3. Easing: Ease-in cubic
4. Opacity: 1.0 → 0.0
5. Scale: 1.0 → 0.95

**Auto-Dismiss Timeline:**
1. Slide in (400ms)
2. Display (4000ms default, configurable)
3. Slide out (300ms)
4. Remove from viewport

**Icon Entrance:**
- Delay: 200ms after slide-in starts
- Scale: 0.0 → 1.2 → 1.0 (bounce effect, 500ms)
- Rotation: -15° → 0° (subtle tilt correction)

---

## 3. WBP_UnlockDialog

**Purpose:** Modal dialog for confirming part purchases with cost and balance display.

**Location:** Center screen, modal overlay

### Widget Hierarchy

```
WBP_UnlockDialog (User Widget)
├── Canvas Panel (Root)
│   ├── ModalOverlay (Image - 80% black, blocks input)
│   └── DialogContainer (Border)
│       ├── DialogBackground (Image - gradient)
│       ├── ContentPanel (Vertical Box)
│       │   ├── HeaderSection (Horizontal Box)
│       │   │   ├── PartIcon (Image - 128×128)
│       │   │   └── HeaderText (Vertical Box)
│       │   │       ├── PartName (Text Block - "Plasma Cannon Mk II")
│       │   │       └── RarityBadge (Border - "RARE")
│       │   ├── Spacer (16px)
│       │   ├── PreviewSection (Size Box)
│       │   │   └── PartPreview3D (Widget Component or static image)
│       │   ├── Spacer (16px)
│       │   ├── CostSection (Vertical Box)
│       │   │   ├── CostLabel (Text Block - "Cost:")
│       │   │   ├── CostAmount (Horizontal Box)
│       │   │   │   ├── CreditIcon (Image)
│       │   │   │   └── CostText (Text Block - "1,500 CR")
│       │   │   ├── Spacer (8px)
│       │   │   ├── BalanceLabel (Text Block - "Your Balance:")
│       │   │   ├── BalanceAmount (Horizontal Box)
│       │   │   │   ├── CreditIcon (Image)
│       │   │   │   └── BalanceText (Text Block - "12,450 CR")
│       │   │   ├── Spacer (8px)
│       │   │   └── AfterPurchase (Horizontal Box)
│       │   │       ├── AfterLabel (Text Block - "After Purchase:")
│       │   │       └── AfterAmount (Text Block - "10,950 CR")
│       │   ├── WarningSection (Border - conditional visibility)
│       │   │   └── WarningText (Text Block - "INSUFFICIENT FUNDS")
│       │   ├── LevelRequirement (Horizontal Box - conditional)
│       │   │   ├── LockIcon (Image)
│       │   │   └── RequirementText (Text Block - "Requires Level 15")
│       │   └── ButtonSection (Horizontal Box)
│       │       ├── CancelButton (Button)
│       │       │   └── CancelText (Text Block - "CANCEL")
│       │       └── ConfirmButton (Button)
│       │           └── ConfirmText (Text Block - "UNLOCK")
```

### Visual Specifications

**Modal Overlay:**
- Full screen coverage
- Color: #000000, 80% opacity
- Blur effect: 4px radius (if performance allows)
- Blocks all input except to dialog

**Dialog Container:**
- Width: 600px (VR: 800px)
- Height: Auto (min 400px)
- Background: Linear gradient (#424242 → #212121, top to bottom)
- Border: 2px solid #1E88E5
- Border radius: 16px
- Shadow: 0px 8px 32px rgba(0,0,0,0.8)
- Padding: 32px

**Part Icon:**
- Size: 128×128px (VR: 160×160px)
- Background: Circular gradient based on rarity
- Border: 4px solid, color matches rarity
- Glow effect: 8px blur, rarity color

**Part Name:**
- Font: Roboto Bold, 24pt (VR: 28pt)
- Color: #FFFFFF
- Drop shadow: 2px offset

**Rarity Badge:**
- Background: Rarity color, 50% opacity
- Border: 2px solid rarity color
- Border radius: 4px
- Padding: 4px 12px
- Font: Roboto Bold, 12pt (VR: 16pt)
- Text: All caps

**Cost Section:**
- Label font: Roboto Regular, 14pt (VR: 18pt), #B0B0B0
- Amount font: Roboto Bold, 20pt (VR: 24pt), #FDD835
- Icon size: 32×32px (VR: 40×40px)
- Spacing: 8px between elements

**Balance Display:**
- Same styling as cost
- Color: #FFFFFF if sufficient, #E53935 if insufficient

**After Purchase:**
- Font: Roboto Medium, 16pt (VR: 20pt)
- Color: #43A047 if sufficient, #E53935 if insufficient

**Warning Section:**
- Background: #E53935, 20% opacity
- Border: 2px solid #E53935
- Border radius: 8px
- Padding: 12px
- Font: Roboto Bold, 14pt (VR: 18pt)
- Color: #FFFFFF
- Icon: Red warning triangle

**Level Requirement:**
- Background: #FDD835, 20% opacity
- Border: 2px solid #FDD835
- Display when level too low
- Lock icon: 24×24px
- Font: Roboto Medium, 14pt (VR: 18pt)

**Buttons:**
- Width: 160px (VR: 200px)
- Height: 48px (VR: 60px)
- Border radius: 8px
- Font: Roboto Bold, 16pt (VR: 20pt)
- Spacing: 16px between buttons

**Cancel Button:**
- Normal: Background #424242, Text #FFFFFF
- Hover: Background #616161, Text #FFFFFF
- Pressed: Background #212121, Text #FFFFFF
- Border: 2px solid #616161

**Confirm Button:**
- Normal: Background #1E88E5, Text #FFFFFF
- Hover: Background #2196F3, Text #FFFFFF
- Pressed: Background #1565C0, Text #FFFFFF
- Disabled: Background #424242, Text #757575
- Border: 2px solid #1E88E5 (or #757575 if disabled)
- Glow on hover: #1E88E5, 8px blur

### Blueprint Properties

```cpp
UCLASS()
class UWBPUnlockDialog : public UUserWidget
{
    GENERATED_BODY()

public:
    // Bindable properties
    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UImage* ModalOverlay;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UBorder* DialogContainer;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UImage* PartIcon;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UTextBlock* PartName;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UBorder* RarityBadge;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UTextBlock* CostText;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UTextBlock* BalanceText;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UTextBlock* AfterAmount;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UBorder* WarningSection;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UTextBlock* WarningText;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UBorder* LevelRequirement;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UTextBlock* RequirementText;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UButton* CancelButton;

    UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
    UButton* ConfirmButton;

    // Public methods
    UFUNCTION(BlueprintCallable, Category="Unlock Dialog")
    void ShowDialog(const FShipPartDefinition& PartData, int32 CurrentCredits, int32 CurrentLevel);

    UFUNCTION(BlueprintCallable, Category="Unlock Dialog")
    void CloseDialog();

    UFUNCTION(BlueprintCallable, Category="Unlock Dialog")
    void PlaySuccessAnimation();

    // Event delegates
    UPROPERTY(BlueprintAssignable, Category="Unlock Dialog")
    FOnUnlockConfirmed OnUnlockConfirmed;

    UPROPERTY(BlueprintAssignable, Category="Unlock Dialog")
    FOnUnlockCancelled OnUnlockCancelled;

protected:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnConfirmClicked();

    UFUNCTION()
    void OnCancelClicked();

    void UpdateAffordability();

private:
    FShipPartDefinition CurrentPartData;
    int32 PlayerCredits;
    int32 PlayerLevel;
    bool bCanAfford;
    bool bMeetsLevelRequirement;

    UPROPERTY(Transient)
    UWidgetAnimation* OpenDialogAnim;

    UPROPERTY(Transient)
    UWidgetAnimation* CloseDialogAnim;

    UPROPERTY(Transient)
    UWidgetAnimation* SuccessAnim;
};
```

### Animation Specifications

**Dialog Open Animation (300ms):**
1. Modal overlay fades in: 0.0 → 0.8 opacity (200ms)
2. Dialog scales in: 0.8 → 1.05 → 1.0 (300ms, bounce easing)
3. Dialog fades in: 0.0 → 1.0 (200ms)
4. Content elements stagger in from bottom (50ms delay each):
   - Part icon (0ms)
   - Part name (50ms)
   - Cost section (100ms)
   - Buttons (150ms)

**Dialog Close Animation (200ms):**
1. Dialog scales out: 1.0 → 0.95 (200ms, ease-in)
2. Dialog fades out: 1.0 → 0.0 (200ms)
3. Modal overlay fades out: 0.8 → 0.0 (200ms)
4. Remove from viewport on complete

**Success Animation (2000ms):**
1. Confirm button:
   - Background flashes #43A047 (300ms)
   - Check mark icon appears (scale 0.0 → 1.0, 200ms)
2. Part icon:
   - Glows bright (500ms)
   - Scale pulse: 1.0 → 1.2 → 1.0 (600ms)
   - Particle burst (gold sparkles)
3. After 1500ms, auto-close dialog

**Insufficient Funds Animation:**
- Warning section pulses: opacity 0.2 → 0.4 → 0.2 (1000ms, repeating)
- Balance text shakes horizontally (200ms, ±5px)
- Confirm button disabled with red tint

**Button Interactions:**
- Hover: Scale 1.0 → 1.05 (100ms), glow appears
- Press: Scale 1.05 → 0.95 (50ms), glow intensifies
- Release: Scale 0.95 → 1.0 (150ms, bounce)

---

## 4. Supporting Widgets

### WBP_ProgressionStats

Detailed stats panel showing progression breakdown.

**Components:**
- Total XP earned
- Matches played
- Parts unlocked (X / Y)
- Current level
- XP to next level
- XP gain rate (avg per match)

**Location:** Customization menu, "Progression" tab

### WBP_LevelMilestone

Visual timeline of level milestones and unlocks.

**Components:**
- Horizontal timeline (levels 1-50)
- Unlock markers at specific levels
- Current level indicator (animated)
- Next unlock preview
- "What's ahead" section

---

## VR Considerations

### Input Handling
- All interactive elements support VR motion controller raycasting
- Buttons have minimum size of 60×60px in VR
- Hover states activate on raycast intersection
- Click requires trigger press while hovering

### Positioning
- WBP_ProgressionDisplay: Attached to player's left wrist (rotation follows wrist angle)
- WBP_NotificationToast: World-space widget positioned 2 meters from player's head, follows camera rotation
- WBP_UnlockDialog: World-space widget at 1.5 meters distance, center of view

### Motion Comfort
- Avoid rapid sliding animations (use fade instead)
- Animations run at max 60fps, never faster
- Scale changes limited to ±30% to prevent disorientation
- Particle effects limited in VR mode

### Text Readability
- All font sizes increased by 4pt in VR
- Minimum font size: 14pt (VR: 18pt)
- High contrast ratios (4.5:1 minimum)
- Drop shadows on all text for depth separation

---

## Performance Guidelines

### Widget Pooling
- Pool notification toasts (max 5 instances)
- Reuse unlock dialogs instead of creating new ones
- Destroy progression display only on level unload

### Optimization
- Use material instances for animated fills (GPU-based)
- Batch text updates (max 60 updates/second)
- Invalidate layout only when necessary
- Collapse invisible widgets

### Memory Budget
- Total widget memory: <10MB
- Texture atlas for icons: 2048×2048
- Compressed textures for backgrounds
- Shared materials across all widgets

---

## Accessibility Features

### Color Blind Support
- Rarity indicators use both color AND patterns:
  - Common: Solid gray
  - Uncommon: Green with horizontal lines
  - Rare: Blue with dots
  - Epic: Purple with diagonal stripes
  - Legendary: Orange with stars

### High Contrast Mode
- Optional high contrast theme
- Increased border widths (2px → 4px)
- Text outlines instead of shadows
- Background darkened to #000000

### Screen Reader Support
- All text elements have aria-labels
- Button states announced ("Confirm - Enabled/Disabled")
- Progress percentages announced

---

## Testing Checklist

- [ ] WBP_ProgressionDisplay updates correctly on level-up
- [ ] XP bar animates smoothly from 0 to 100%
- [ ] Credits update with proper animation
- [ ] Toast notifications appear and dismiss automatically
- [ ] Multiple toasts stack correctly
- [ ] Unlock dialog shows correct affordability state
- [ ] Unlock dialog prevents purchase when insufficient funds
- [ ] Success animation plays on successful unlock
- [ ] All widgets scale correctly in VR
- [ ] Performance maintains 90fps in VR with all widgets active
- [ ] Accessibility features function correctly
- [ ] Touch/controller input works for all interactive elements

---

**Document Version:** 1.0
**Last Updated:** 2025-11-07
**Author:** Ship Customization System - Phase 3, Subagent 4
