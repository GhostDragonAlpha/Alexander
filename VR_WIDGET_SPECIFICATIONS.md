# VR Widget Specifications

## Overview

This document provides detailed specifications for creating VR-optimized widgets for the ship customization menu in Alexander. All specifications are designed to ensure readability, usability, and comfort in virtual reality environments.

## Table of Contents

1. [Font Size Guidelines](#font-size-guidelines)
2. [Button Sizing](#button-sizing)
3. [3D Widget Positioning](#3d-widget-positioning)
4. [Depth and Layering](#depth-and-layering)
5. [Layout Adjustments](#layout-adjustments)
6. [Color and Contrast](#color-and-contrast)
7. [Anti-aliasing and Readability](#anti-aliasing-and-readability)
8. [Widget Templates](#widget-templates)

---

## Font Size Guidelines

### Minimum Font Sizes

**Effective Minimum at Arm's Length (60cm)**: 18pt

Font size requirements vary based on viewing distance and VR headset resolution.

### Font Size Calculation

```
Physical Font Height (mm) = (Font Size in Points × 25.4) / 72
Visual Angle (degrees) = 2 × arctan(Physical Height / (2 × Distance))
```

### Recommended Font Sizes by Use Case

| Element Type | Minimum Size | Recommended Size | Maximum Size |
|--------------|--------------|------------------|--------------|
| Body Text | 18pt | 24pt | 32pt |
| Headings | 24pt | 32pt | 48pt |
| Subheadings | 20pt | 28pt | 36pt |
| Button Labels | 20pt | 26pt | 32pt |
| Tooltips | 16pt | 20pt | 24pt |
| Fine Print | 14pt | 16pt | 18pt |
| Menu Titles | 32pt | 42pt | 64pt |

### Distance-Based Font Scaling

```cpp
// Calculate appropriate font size based on distance
float CalculateVRFontSize(float ViewingDistanceCm, float DesiredVisualAngleDegrees = 0.3f)
{
    // Convert to radians
    float VisualAngleRad = FMath::DegreesToRadians(DesiredVisualAngleDegrees);

    // Calculate physical height in mm
    float PhysicalHeightMm = 2.0f * (ViewingDistanceCm * 10.0f) * FMath::Tan(VisualAngleRad / 2.0f);

    // Convert to points (1 point = 1/72 inch = 0.353 mm)
    float FontSizePoints = (PhysicalHeightMm * 72.0f) / 25.4f;

    // Round to nearest even number for clean rendering
    return FMath::RoundToInt(FontSizePoints / 2.0f) * 2.0f;
}

// Example usage:
float FontSize60cm = CalculateVRFontSize(60.0f, 0.3f); // Returns ~24pt
float FontSize80cm = CalculateVRFontSize(80.0f, 0.3f); // Returns ~32pt
```

### Font Family Recommendations

1. **Primary Font**: Roboto Medium / Inter Medium
   - Excellent legibility in VR
   - Wide character support
   - Available as SDF font

2. **Monospace Font**: Roboto Mono / Source Code Pro
   - For numerical data, coordinates, stats
   - Clear distinction between similar characters (0/O, 1/I/l)

3. **Display Font**: Orbitron / Exo 2
   - For sci-fi themed headers
   - Use sparingly for visual interest

### Font Rendering Settings

```cpp
// UMG Text Block settings for VR
TextBlock->SetFont(FSlateFontInfo(
    FontObject,
    24,                                    // Size
    EFontHinting::Default,                 // Hinting
    EFontLoadingPolicy::PreLoad            // Loading
));

// Enable SDF rendering for crispness
TextBlock->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));

// Shadow for depth and readability
FSlateFontInfo FontInfo = TextBlock->GetFont();
FontInfo.OutlineSettings.OutlineSize = 2;
FontInfo.OutlineSettings.bSeparateFillAlpha = true;
TextBlock->SetFont(FontInfo);
```

---

## Button Sizing

### Minimum Interactive Element Size

**Critical Requirement**: 2cm × 2cm minimum for all interactive elements

This ensures comfortable interaction without precision issues in VR.

### Button Size Guidelines by Type

| Button Type | Minimum Size (cm) | Recommended Size (cm) | Pixel Size (at 40cm wide menu, 1920px) |
|-------------|-------------------|------------------------|----------------------------------------|
| Primary Action | 2.5 × 1.5 | 3.0 × 2.0 | 120 × 96 |
| Secondary Action | 2.0 × 1.2 | 2.5 × 1.5 | 96 × 72 |
| Icon Button | 2.0 × 2.0 | 2.5 × 2.5 | 96 × 96 |
| Toggle Switch | 3.0 × 1.5 | 3.5 × 2.0 | 144 × 96 |
| Slider | 4.0 × 1.5 | 5.0 × 2.0 | 192 × 96 |
| Color Picker | 2.5 × 2.5 | 3.0 × 3.0 | 120 × 120 |

### Button Size Calculator

```cpp
// Calculate button size in pixels from physical size
struct FVRButtonSizeHelper
{
    static FVector2D CalculateButtonPixelSize(
        float ButtonWidthCm,
        float ButtonHeightCm,
        float MenuWidthCm,
        float MenuHeightCm,
        FVector2D MenuResolution)
    {
        float PixelsPerCmX = MenuResolution.X / MenuWidthCm;
        float PixelsPerCmY = MenuResolution.Y / MenuHeightCm;

        return FVector2D(
            ButtonWidthCm * PixelsPerCmX,
            ButtonHeightCm * PixelsPerCmY
        );
    }

    // Example: 2.5cm × 2cm button on 40cm × 30cm menu at 1920×1080
    // Returns: 120×72 pixels
};
```

### Button Spacing and Padding

```cpp
// Minimum spacing between interactive elements
const float MinButtonSpacingCm = 0.5f;  // 0.5cm gap
const float MinButtonPaddingCm = 0.3f;  // Internal padding

// Example layout calculation
struct FVRButtonLayout
{
    FVector2D ButtonSize = FVector2D(2.5f, 2.0f);    // cm
    float ButtonSpacing = 0.5f;                       // cm
    float RowSpacing = 0.7f;                          // cm

    FVector2D CalculateNextButtonPosition(int32 Column, int32 Row)
    {
        return FVector2D(
            Column * (ButtonSize.X + ButtonSpacing),
            Row * (ButtonSize.Y + RowSpacing)
        );
    }
};
```

### Button Visual Feedback

All buttons should provide clear visual feedback for VR interaction states:

```cpp
// Button states and visual styles
enum class EVRButtonState : uint8
{
    Normal,      // Default appearance
    Hovered,     // Laser pointer is over button
    Pressed,     // Button is being pressed
    Disabled     // Button is not interactive
};

// Visual specifications for each state
struct FVRButtonVisualSpec
{
    // Normal state
    FLinearColor NormalColor = FLinearColor(0.15f, 0.15f, 0.2f, 1.0f);       // Dark blue-gray
    FLinearColor NormalBorderColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);   // Light blue
    float NormalBorderThickness = 2.0f;                                       // pixels

    // Hovered state - HIGH CONTRAST for visibility
    FLinearColor HoveredColor = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);        // Bright blue
    FLinearColor HoveredBorderColor = FLinearColor(0.0f, 0.8f, 1.0f, 1.0f);  // Cyan
    float HoveredBorderThickness = 3.0f;                                      // pixels
    float HoveredGlow = 0.3f;                                                 // Emissive strength

    // Pressed state
    FLinearColor PressedColor = FLinearColor(0.3f, 0.5f, 0.7f, 1.0f);        // Brighter blue
    FLinearColor PressedBorderColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f);  // Gold
    float PressedBorderThickness = 4.0f;                                      // pixels

    // Disabled state
    FLinearColor DisabledColor = FLinearColor(0.1f, 0.1f, 0.1f, 0.5f);       // Dark gray, transparent
    FLinearColor DisabledTextColor = FLinearColor(0.3f, 0.3f, 0.3f, 0.7f);   // Gray
};
```

### Icon Sizing

```cpp
// Icon sizes relative to button size
struct FVRIconSpec
{
    // Icon should be 60-70% of button size
    static FVector2D CalculateIconSize(FVector2D ButtonSize)
    {
        return ButtonSize * 0.65f;
    }

    // Example: 2.5cm button = 1.6cm icon
    // In pixels (96px button) = 62px icon
};
```

---

## 3D Widget Positioning

### Optimal Viewing Distance

**Primary Range**: 50-80cm from player's eyes
**Ideal Distance**: 60cm (arm's length)

### Positioning Guidelines

```cpp
// Ship customization menu positioning
struct FVRMenuPositioning
{
    // Main customization panel
    FVector MainPanelPosition = FVector(60.0f, 0.0f, 0.0f);           // 60cm forward
    FRotator MainPanelRotation = FRotator(0.0f, 180.0f, 0.0f);        // Face player

    // Side panels (part browser, color picker)
    FVector LeftPanelPosition = FVector(50.0f, -30.0f, 5.0f);         // Angled left
    FRotator LeftPanelRotation = FRotator(0.0f, 150.0f, 0.0f);        // 30° angle

    FVector RightPanelPosition = FVector(50.0f, 30.0f, 5.0f);         // Angled right
    FRotator RightPanelRotation = FRotator(0.0f, 210.0f, 0.0f);       // 30° angle

    // Floating tooltips
    FVector TooltipOffset = FVector(10.0f, 0.0f, 15.0f);              // Above and forward
};
```

### Comfortable Field of View

```cpp
// Keep interactive elements within comfortable FOV
struct FVRFieldOfView
{
    // Horizontal FOV: ±60° from center (120° total)
    // Vertical FOV: ±45° from center (90° total)

    static bool IsWithinComfortableView(FVector WidgetPosition, FVector PlayerPosition, FRotator PlayerRotation)
    {
        FVector Direction = (WidgetPosition - PlayerPosition).GetSafeNormal();
        FVector Forward = PlayerRotation.Vector();

        float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Direction, Forward)));

        // Within 60° horizontally
        return AngleDegrees <= 60.0f;
    }
};
```

### Multi-Panel Layout

```cpp
// Curved panel arrangement for wide UI
void ArrangePanelsInArc(TArray<UWidgetComponent*> Panels, float Radius = 60.0f, float ArcAngle = 90.0f)
{
    int32 NumPanels = Panels.Num();
    float AngleStep = ArcAngle / FMath::Max(1, NumPanels - 1);
    float StartAngle = -ArcAngle / 2.0f;

    for (int32 i = 0; i < NumPanels; i++)
    {
        float Angle = FMath::DegreesToRadians(StartAngle + (i * AngleStep));

        // Position along arc
        FVector Position(
            Radius * FMath::Cos(Angle),
            Radius * FMath::Sin(Angle),
            0.0f
        );

        // Rotate to face player
        FRotator Rotation(0.0f, 180.0f - FMath::RadiansToDegrees(Angle), 0.0f);

        Panels[i]->SetRelativeLocation(Position);
        Panels[i]->SetRelativeRotation(Rotation);
    }
}
```

### Dynamic Repositioning

```cpp
// Reposition menu to stay in front of player
void RepositionMenuToPlayer(UWidgetComponent* Menu, FVector PlayerLocation, FRotator PlayerRotation, float Distance = 60.0f)
{
    // Calculate position in front of player
    FVector Forward = PlayerRotation.Vector();
    FVector NewPosition = PlayerLocation + (Forward * Distance);

    // Smooth transition
    FVector CurrentPosition = Menu->GetComponentLocation();
    FVector InterpolatedPosition = FMath::VInterpTo(CurrentPosition, NewPosition, GetWorld()->GetDeltaSeconds(), 2.0f);

    Menu->SetWorldLocation(InterpolatedPosition);

    // Face player
    FRotator LookAtRotation = (PlayerLocation - InterpolatedPosition).Rotation();
    Menu->SetWorldRotation(FRotator(0.0f, LookAtRotation.Yaw, 0.0f));
}
```

---

## Depth and Layering

### Z-Depth Organization

Organize UI elements by depth to avoid visual confusion:

```cpp
// Depth layers (relative to player)
struct FVRDepthLayers
{
    const float Background = 80.0f;          // Background panels, reference images
    const float MainContent = 60.0f;         // Primary interaction surface
    const float SecondaryContent = 55.0f;    // Secondary panels, options
    const float Overlay = 45.0f;             // Popups, confirmations
    const float Tooltip = 40.0f;             // Tooltips, help text
};
```

### Depth Cues for UI Elements

```cpp
// Visual depth indicators
struct FVRDepthVisualCues
{
    // Parallax scrolling for depth perception
    static void ApplyParallaxToLayer(UWidgetComponent* Widget, float DepthFactor)
    {
        // Closer objects move faster when player moves head
        // DepthFactor: 0.0 = no parallax, 1.0 = full parallax
    }

    // Shadow casting for depth
    static void EnableDepthShadow(UWidgetComponent* Widget, bool bEnabled)
    {
        Widget->SetCastShadow(bEnabled);
    }

    // Blur for background elements
    static void ApplyDepthBlur(UWidgetComponent* Widget, float BlurAmount)
    {
        // Post-process blur for background layers
    }
};
```

### Popup and Modal Positioning

```cpp
// Modal dialog positioning
void ShowModalDialog(UWidgetComponent* Dialog, FVector PlayerPosition)
{
    // Position 45cm in front of player
    FVector DialogPosition = PlayerPosition + (GetPlayerForward() * 45.0f);
    Dialog->SetWorldLocation(DialogPosition);

    // Slightly larger to emphasize importance
    float ModalScale = 0.025f; // Slightly larger than base UI
    Dialog->SetWorldScale3D(FVector(ModalScale));

    // Dim background panels
    DimBackgroundPanels(0.3f); // 30% brightness
}
```

---

## Layout Adjustments

### Grid-Based Layout for VR

```cpp
// VR-optimized grid system
struct FVRGridLayout
{
    float CellSizeCm = 2.5f;               // Base cell size (matches min button size)
    float GutterSizeCm = 0.5f;             // Spacing between cells
    int32 Columns = 8;                      // Number of columns
    int32 Rows = 6;                         // Number of rows

    FVector2D GetCellPosition(int32 Column, int32 Row)
    {
        return FVector2D(
            Column * (CellSizeCm + GutterSizeCm),
            Row * (CellSizeCm + GutterSizeCm)
        );
    }

    FVector2D GetCellSize(int32 ColumnSpan = 1, int32 RowSpan = 1)
    {
        return FVector2D(
            (ColumnSpan * CellSizeCm) + ((ColumnSpan - 1) * GutterSizeCm),
            (RowSpan * CellSizeCm) + ((RowSpan - 1) * GutterSizeCm)
        );
    }
};
```

### Responsive Layout for Different Distances

```cpp
// Adjust layout based on viewing distance
void AdjustLayoutForDistance(float ViewingDistanceCm)
{
    // Scale factor: elements should appear similar visual size regardless of distance
    float BaseDistance = 60.0f;
    float ScaleFactor = ViewingDistanceCm / BaseDistance;

    // Adjust font sizes
    float BaseFontSize = 24.0f;
    float AdjustedFontSize = BaseFontSize * ScaleFactor;

    // Adjust button sizes
    FVector2D BaseButtonSize(2.5f, 2.0f);
    FVector2D AdjustedButtonSize = BaseButtonSize * ScaleFactor;

    // Adjust spacing
    float BaseSpacing = 0.5f;
    float AdjustedSpacing = BaseSpacing * ScaleFactor;
}
```

### Scrolling and Pagination

```cpp
// VR-friendly scrolling (avoid traditional scroll bars)
struct FVRScrollingSpec
{
    // Use discrete pages instead of continuous scrolling
    void NextPage()
    {
        // Animate to next page with smooth transition
        // Show page indicator (1/5, 2/5, etc.)
    }

    // Or use infinite scroll with momentum
    void ScrollWithVelocity(float Velocity)
    {
        // Flick gesture to scroll
        // Decelerate naturally with friction
    }

    // Page indicators should be large and clear
    float PageDotSize = 1.0f;              // cm
    float PageDotSpacing = 0.5f;           // cm
    FLinearColor ActivePageColor = FLinearColor::White;
    FLinearColor InactivePageColor = FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);
};
```

---

## Color and Contrast

### WCAG AA Minimum Compliance

All text must meet WCAG AA contrast requirements (minimum 4.5:1 for normal text, 3:1 for large text).

### Recommended Color Schemes

```cpp
// Dark theme (recommended for VR to reduce eye strain)
struct FVRDarkTheme
{
    // Background colors
    FLinearColor BackgroundPrimary = FLinearColor(0.08f, 0.08f, 0.12f, 1.0f);    // Very dark blue-gray
    FLinearColor BackgroundSecondary = FLinearColor(0.12f, 0.12f, 0.16f, 1.0f);  // Slightly lighter

    // Text colors
    FLinearColor TextPrimary = FLinearColor(0.95f, 0.95f, 0.97f, 1.0f);          // Near-white
    FLinearColor TextSecondary = FLinearColor(0.7f, 0.7f, 0.75f, 1.0f);          // Light gray
    FLinearColor TextDisabled = FLinearColor(0.4f, 0.4f, 0.45f, 1.0f);           // Medium gray

    // Accent colors
    FLinearColor AccentPrimary = FLinearColor(0.0f, 0.6f, 1.0f, 1.0f);           // Bright blue
    FLinearColor AccentSecondary = FLinearColor(0.0f, 0.8f, 0.6f, 1.0f);         // Cyan-green
    FLinearColor AccentWarning = FLinearColor(1.0f, 0.7f, 0.0f, 1.0f);           // Amber
    FLinearColor AccentError = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f);             // Red

    // Interactive element colors
    FLinearColor ButtonNormal = FLinearColor(0.15f, 0.15f, 0.2f, 1.0f);
    FLinearColor ButtonHover = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);
    FLinearColor ButtonPressed = FLinearColor(0.3f, 0.5f, 0.7f, 1.0f);
};
```

### Contrast Calculation

```cpp
// Calculate contrast ratio between two colors
float CalculateContrastRatio(FLinearColor Color1, FLinearColor Color2)
{
    auto GetRelativeLuminance = [](FLinearColor Color) -> float
    {
        // Convert to sRGB
        auto ToSRGB = [](float Channel) -> float
        {
            return Channel <= 0.03928f ? Channel / 12.92f : FMath::Pow((Channel + 0.055f) / 1.055f, 2.4f);
        };

        float R = ToSRGB(Color.R);
        float G = ToSRGB(Color.G);
        float B = ToSRGB(Color.B);

        return 0.2126f * R + 0.7152f * G + 0.0722f * B;
    };

    float L1 = GetRelativeLuminance(Color1);
    float L2 = GetRelativeLuminance(Color2);

    float Lighter = FMath::Max(L1, L2);
    float Darker = FMath::Min(L1, L2);

    return (Lighter + 0.05f) / (Darker + 0.05f);
}

// Verify WCAG AA compliance
bool IsWCAGAACompliant(FLinearColor TextColor, FLinearColor BackgroundColor, bool bLargeText = false)
{
    float ContrastRatio = CalculateContrastRatio(TextColor, BackgroundColor);
    float MinimumRatio = bLargeText ? 3.0f : 4.5f;

    return ContrastRatio >= MinimumRatio;
}
```

### Color Blindness Considerations

```cpp
// Avoid relying solely on color for important information
// Use additional visual indicators:

struct FVRAccessibleDesign
{
    // Use icons + color
    void ShowSuccessState(UWidget* Widget)
    {
        Widget->SetColor(FLinearColor::Green);
        Widget->SetIcon(CheckmarkIcon);           // Icon as backup
        Widget->SetText(FText::FromString("OK")); // Text as backup
    }

    // Use patterns + color
    void ApplyColorBlindFriendlyPattern()
    {
        // Red: Diagonal lines
        // Green: Dots
        // Blue: Horizontal lines
    }

    // Ensure sufficient contrast even for color blind users
    // Use high contrast between foreground and background
};
```

---

## Anti-aliasing and Readability

### MSAA Settings

```cpp
// Project Settings > Engine > Rendering
// Anti-Aliasing Method: MSAA
// MSAA Sample Count: 4x (minimum for VR)

// For best quality in VR:
r.MSAACount = 4
r.PostProcessAAQuality = 6
```

### Signed Distance Field (SDF) Fonts

```cpp
// Enable SDF for fonts in project settings
// Much sharper text rendering in VR at any distance

// Import font with SDF settings
FSlateFontInfo CreateSDFFont(UFont* FontObject, int32 Size)
{
    FSlateFontInfo FontInfo(FontObject, Size);
    FontInfo.FontFallback = EFontFallback::FF_Max; // Use fallback for missing characters

    // SDF settings (set in font asset)
    // Enable "Is Signed Distance Field Font" in font import settings

    return FontInfo;
}
```

### Text Rendering Optimization

```cpp
// UMG Text rendering settings for VR
struct FVRTextRenderingSettings
{
    // Enable subpixel rendering
    static void ConfigureTextBlock(UTextBlock* TextBlock)
    {
        // Use SDF font
        TextBlock->SetFont(GetSDFFont());

        // Add subtle outline for clarity
        FSlateFontInfo Font = TextBlock->GetFont();
        Font.OutlineSettings.OutlineSize = 1;
        Font.OutlineSettings.OutlineColor = FLinearColor::Black;
        TextBlock->SetFont(Font);

        // Shadow for depth (optional)
        TextBlock->SetShadowOffset(FVector2D(1.0f, 1.0f));
        TextBlock->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f));

        // Justification for readability
        TextBlock->SetJustification(ETextJustify::Left); // Left-aligned easier to read
    }
};
```

### Material Settings for UI

```cpp
// Widget material settings for crisp rendering
void ConfigureWidgetMaterial(UMaterialInstanceDynamic* Material)
{
    // Disable mip-mapping for UI textures (prevents blur)
    // Set in texture asset: Mip Gen Settings = NoMipmaps

    // Use unlit material for consistent appearance
    // Blend Mode: Translucent or Masked

    // Disable temporal AA for widgets (can cause ghosting)
    Material->SetScalarParameterValue("DisableTemporalAA", 1.0f);
}
```

### Supersampling for Critical Text

```cpp
// Render important text at higher resolution
void CreateHighResolutionText(UWidgetComponent* Widget)
{
    // Render at 2x resolution, then downscale
    FVector2D BaseDrawSize(1920, 1080);
    FVector2D SupersampledSize = BaseDrawSize * 2.0f;

    Widget->SetDrawSize(SupersampledSize);

    // Compensate with scale
    FVector CurrentScale = Widget->GetRelativeScale3D();
    Widget->SetRelativeScale3D(CurrentScale * 0.5f);
}
```

---

## Widget Templates

### Standard Button Template

```cpp
// Standard VR button widget specifications
USTRUCT(BlueprintType)
struct FVRStandardButton
{
    GENERATED_BODY()

    // Physical dimensions
    FVector2D PhysicalSizeCm = FVector2D(2.5f, 2.0f);

    // Visual style
    FVRButtonVisualSpec VisualStyle;

    // Text
    FText Label = FText::FromString("Button");
    int32 FontSize = 24;
    FLinearColor TextColor = FLinearColor::White;

    // Optional icon
    UTexture2D* Icon = nullptr;
    FVector2D IconSizeCm = FVector2D(1.5f, 1.5f);

    // Interaction
    bool bEnabled = true;
    FName ActionName = NAME_None;
};
```

### Color Picker Template

```cpp
// VR-optimized color picker
struct FVRColorPickerSpec
{
    // Physical size: 15cm × 15cm panel
    FVector2D PhysicalSizeCm = FVector2D(15.0f, 15.0f);

    // Color swatches: 3cm × 3cm each
    FVector2D SwatchSizeCm = FVector2D(3.0f, 3.0f);

    // Grid: 5×5 color swatches
    int32 SwatchColumns = 5;
    int32 SwatchRows = 5;

    // Custom color slider
    FVector2D SliderSizeCm = FVector2D(12.0f, 2.0f);

    // Preview panel
    FVector2D PreviewSizeCm = FVector2D(5.0f, 5.0f);
};
```

### Part Browser Template

```cpp
// Ship part browser widget
struct FVRPartBrowserSpec
{
    // Panel size: 30cm × 40cm
    FVector2D PanelSizeCm = FVector2D(30.0f, 40.0f);

    // Part thumbnail: 6cm × 6cm
    FVector2D ThumbnailSizeCm = FVector2D(6.0f, 6.0f);

    // Grid: 4 columns × 5 rows (20 parts visible)
    int32 Columns = 4;
    int32 Rows = 5;

    // Spacing between thumbnails
    float ThumbnailSpacingCm = 0.8f;

    // Part name label: 20pt font
    int32 PartNameFontSize = 20;

    // Category tabs: 3cm × 2cm each
    FVector2D CategoryTabSizeCm = FVector2D(3.0f, 2.0f);
};
```

### Confirmation Dialog Template

```cpp
// Modal confirmation dialog
struct FVRConfirmationDialogSpec
{
    // Dialog size: 25cm × 15cm
    FVector2D DialogSizeCm = FVector2D(25.0f, 15.0f);

    // Title font: 32pt bold
    int32 TitleFontSize = 32;

    // Message font: 24pt regular
    int32 MessageFontSize = 24;

    // Buttons: 6cm × 2.5cm
    FVector2D ButtonSizeCm = FVector2D(6.0f, 2.5f);

    // Button spacing: 2cm apart
    float ButtonSpacingCm = 2.0f;

    // Icon size: 5cm × 5cm
    FVector2D IconSizeCm = FVector2D(5.0f, 5.0f);
};
```

### Tooltip Template

```cpp
// Floating tooltip widget
struct FVRTooltipSpec
{
    // Dynamic size based on content (max 20cm wide)
    float MaxWidthCm = 20.0f;

    // Font size: 18pt
    int32 FontSize = 18;

    // Padding: 0.5cm all sides
    float PaddingCm = 0.5f;

    // Background opacity: 90%
    float BackgroundOpacity = 0.9f;

    // Offset from pointer: 10cm up, 5cm forward
    FVector OffsetFromPointer = FVector(5.0f, 0.0f, 10.0f);

    // Fade in/out duration: 0.2s
    float FadeDuration = 0.2f;
};
```

---

## Implementation Example

### Complete VR Widget Setup

```cpp
// Example: Ship customization menu widget component setup
void AShipCustomizationMenu::SetupVRWidget()
{
    // Create widget component
    MenuWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("CustomizationMenuWidget"));
    MenuWidget->SetupAttachment(RootComponent);

    // Configure for VR
    MenuWidget->SetWidgetSpace(EWidgetSpace::World);
    MenuWidget->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    MenuWidget->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

    // Set physical size (40cm × 30cm menu)
    float MenuWidthCm = 40.0f;
    float MenuHeightCm = 30.0f;
    MenuWidget->SetDrawSize(FVector2D(1920, 1440)); // 1920×1440 resolution

    // Calculate scale (1 unit = 1cm in UE5)
    float ScaleX = MenuWidthCm / 1920.0f;
    float ScaleY = MenuHeightCm / 1440.0f;
    MenuWidget->SetRelativeScale3D(FVector(ScaleX, ScaleY, 1.0f));

    // Position at comfortable viewing distance
    MenuWidget->SetRelativeLocation(FVector(60.0f, 0.0f, 0.0f));  // 60cm forward
    MenuWidget->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f)); // Face player

    // Set widget class
    MenuWidget->SetWidgetClass(CustomizationMenuWidgetClass);

    // Enable interaction
    MenuWidget->SetWidgetSpace(EWidgetSpace::World);
}
```

---

## Testing and Validation

### VR Widget Checklist

Use this checklist to validate all VR widgets:

- [ ] Font size minimum 18pt at viewing distance
- [ ] All interactive elements minimum 2cm × 2cm
- [ ] Text contrast ratio meets WCAG AA (4.5:1 minimum)
- [ ] Widget positioned 50-80cm from player
- [ ] Widget within ±60° horizontal FOV
- [ ] SDF fonts enabled for sharp rendering
- [ ] MSAA 4x enabled in project settings
- [ ] Hover state clearly visible
- [ ] Press state clearly visible
- [ ] Disabled state clearly distinguishable
- [ ] No text smaller than 14pt
- [ ] Button spacing minimum 0.5cm
- [ ] Color not sole indicator of state
- [ ] Icons clear and recognizable at distance
- [ ] Tooltip text readable and non-blocking

### Measurement Tools

```cpp
// Helper functions for validating widget specifications
class FVRWidgetValidator
{
public:
    // Validate font size
    static bool ValidateFontSize(int32 FontSize, float ViewingDistanceCm)
    {
        float MinFontSize = (ViewingDistanceCm / 60.0f) * 18.0f;
        return FontSize >= MinFontSize;
    }

    // Validate button size
    static bool ValidateButtonSize(FVector2D ButtonSizeCm)
    {
        return ButtonSizeCm.X >= 2.0f && ButtonSizeCm.Y >= 2.0f;
    }

    // Validate viewing distance
    static bool ValidateViewingDistance(float DistanceCm)
    {
        return DistanceCm >= 50.0f && DistanceCm <= 80.0f;
    }

    // Validate contrast ratio
    static bool ValidateContrast(FLinearColor TextColor, FLinearColor BackgroundColor, bool bLargeText)
    {
        return IsWCAGAACompliant(TextColor, BackgroundColor, bLargeText);
    }
};
```

---

## Additional Resources

### Tools and Utilities

- **UE5 VR Widget Previewer**: Preview widgets in VR without play mode
- **Contrast Checker**: Online tool for WCAG compliance
- **Font Size Calculator**: Calculate optimal font size for distance

### Documentation References

- [Unreal Engine UMG Documentation](https://docs.unrealengine.com/5.3/en-US/umg-ui-designer-for-unreal-engine/)
- [VR Best Practices Guide](https://docs.unrealengine.com/5.3/en-US/vr-best-practices-in-unreal-engine/)
- [WCAG 2.1 Guidelines](https://www.w3.org/WAI/WCAG21/quickref/)

---

**Last Updated**: 2025-11-07
**Engine Version**: Unreal Engine 5.6
**Author**: Claude (Subagent 3 - Ship Customization System Phase 3)
