# UI Theme Specification

## Overview

This document defines the complete visual theme for the ship customization system UI, including colors, typography, spacing, button states, and styling guidelines.

---

## Color Palette

### Primary Colors

**Primary (Blue):**
```
Hex: #1E88E5
RGB: (30, 136, 229)
HSL: (207°, 79%, 51%)
Use: Primary actions, selected states, links, progress indicators
```

**Secondary (Green):**
```
Hex: #43A047
RGB: (67, 160, 71)
HSL: (122°, 41%, 45%)
Use: Success states, confirmations, stat increases, equip actions
```

**Accent (Cyan):**
```
Hex: #00BCD4
RGB: (0, 188, 212)
HSL: (187°, 100%, 42%)
Use: Highlights, hover states, interactive elements
```

### Neutral Colors

**Background Dark:**
```
Hex: #212121
RGB: (33, 33, 33)
HSL: (0°, 0%, 13%)
Use: Main background, dialog backgrounds, dark sections
```

**Surface:**
```
Hex: #424242
RGB: (66, 66, 66)
HSL: (0°, 0%, 26%)
Use: Card backgrounds, panel backgrounds, raised surfaces
```

**Surface Light:**
```
Hex: #616161
RGB: (97, 97, 97)
HSL: (0°, 0%, 38%)
Use: Hover states, lighter panels, dividers
```

**Border:**
```
Hex: #757575
RGB: (117, 117, 117)
HSL: (0°, 0%, 46%)
Use: Borders, separators, outlines
```

### Text Colors

**Text Primary:**
```
Hex: #FFFFFF
RGB: (255, 255, 255)
HSL: (0°, 0%, 100%)
Opacity: 100%
Use: Main headings, important text, labels
```

**Text Secondary:**
```
Hex: #E0E0E0
RGB: (224, 224, 224)
HSL: (0°, 0%, 88%)
Opacity: 90%
Use: Body text, descriptions, secondary information
```

**Text Disabled:**
```
Hex: #9E9E9E
RGB: (158, 158, 158)
HSL: (0°, 0%, 62%)
Opacity: 60%
Use: Disabled elements, placeholders, hints
```

**Text Hint:**
```
Hex: #BDBDBD
RGB: (189, 189, 189)
HSL: (0°, 0%, 74%)
Opacity: 70%
Use: Input placeholders, tooltips, helper text
```

### Semantic Colors

**Error (Red):**
```
Hex: #E53935
RGB: (229, 57, 53)
HSL: (1°, 77%, 55%)
Use: Errors, delete actions, critical warnings, stat decreases
```

**Warning (Yellow):**
```
Hex: #FDD835
RGB: (253, 216, 53)
HSL: (49°, 98%, 60%)
Use: Warnings, cautions, currency, level-up effects
```

**Info (Light Blue):**
```
Hex: #29B6F6
RGB: (41, 182, 246)
HSL: (199°, 92%, 56%)
Use: Information messages, tips, tooltips
```

**Success (Light Green):**
```
Hex: #66BB6A
RGB: (102, 187, 106)
HSL: (123°, 39%, 57%)
Use: Success confirmations, completed states, achievements
```

### Rarity Colors

**Common (Gray):**
```
Hex: #9E9E9E
RGB: (158, 158, 158)
HSL: (0°, 0%, 62%)
Pattern: Solid fill
Border: 2px solid
Glow: None
```

**Uncommon (Green):**
```
Hex: #4CAF50
RGB: (76, 175, 80)
HSL: (122°, 39%, 49%)
Pattern: Horizontal lines (2px spacing)
Border: 2px solid
Glow: 2px blur, 30% opacity
```

**Rare (Blue):**
```
Hex: #2196F3
RGB: (33, 150, 243)
HSL: (207°, 90%, 54%)
Pattern: Dots (4px grid)
Border: 3px solid
Glow: 4px blur, 50% opacity
```

**Epic (Purple):**
```
Hex: #9C27B0
RGB: (156, 39, 176)
HSL: (291°, 64%, 42%)
Pattern: Diagonal stripes (45°, 3px width, 6px spacing)
Border: 3px solid
Glow: 6px blur, 60% opacity
```

**Legendary (Orange):**
```
Hex: #FF9800
RGB: (255, 152, 0)
HSL: (36°, 100%, 50%)
Pattern: Stars (8px star icons, 12px grid)
Border: 4px solid
Glow: 8px blur, 80% opacity
Animation: Pulsing glow (2s cycle)
```

**Mythic (Gold - Future Use):**
```
Hex: #FFD700
RGB: (255, 215, 0)
HSL: (51°, 100%, 50%)
Pattern: Animated shimmer
Border: 4px solid, gradient border
Glow: 12px blur, 100% opacity, animated
```

### Special Effect Colors

**XP Gain:**
```
Primary: #43A047 (green)
Secondary: #66BB6A (light green)
Gradient: 135° linear
```

**Credits Gain:**
```
Primary: #FDD835 (yellow/gold)
Secondary: #FFB300 (darker gold)
Gradient: 180° linear
```

**Damage Indicator:**
```
Primary: #E53935 (red)
Flash: #FFFFFF (white)
Fade: 1.0 → 0.0 over 500ms
```

**Shield/Protection:**
```
Primary: #29B6F6 (light blue)
Secondary: #039BE5 (blue)
Gradient: Radial from center
```

---

## Typography

### Font Families

**Primary Font:**
```
Family: Roboto
Weights: Regular (400), Medium (500), Bold (700)
Source: Google Fonts
Fallback: "Segoe UI", "Helvetica Neue", Arial, sans-serif
```

**Monospace Font (Stats/Numbers):**
```
Family: Roboto Mono
Weight: Regular (400), Medium (500)
Source: Google Fonts
Fallback: "Courier New", Courier, monospace
Use: Numerical stats, credits, XP values, timers
```

### Type Scale (Desktop)

**Display Large:**
```
Font: Roboto Bold
Size: 48pt
Line Height: 56pt
Letter Spacing: -0.5pt
Use: Hero headings, splash screens
```

**Display:**
```
Font: Roboto Bold
Size: 36pt
Line Height: 44pt
Letter Spacing: 0pt
Use: Page titles, major headings
```

**Heading 1:**
```
Font: Roboto Bold
Size: 24pt
Line Height: 32pt
Letter Spacing: 0pt
Use: Section headings, panel titles
```

**Heading 2:**
```
Font: Roboto Bold
Size: 20pt
Line Height: 28pt
Letter Spacing: 0.15pt
Use: Sub-section headings, card titles
```

**Heading 3:**
```
Font: Roboto Medium
Size: 16pt
Line Height: 24pt
Letter Spacing: 0.15pt
Use: List headings, group labels
```

**Body Large:**
```
Font: Roboto Regular
Size: 16pt
Line Height: 24pt
Letter Spacing: 0.5pt
Use: Important body text, primary descriptions
```

**Body:**
```
Font: Roboto Regular
Size: 14pt
Line Height: 20pt
Letter Spacing: 0.25pt
Use: Standard body text, descriptions
```

**Body Small:**
```
Font: Roboto Regular
Size: 12pt
Line Height: 16pt
Letter Spacing: 0.4pt
Use: Secondary text, captions, metadata
```

**Label:**
```
Font: Roboto Medium
Size: 14pt
Line Height: 20pt
Letter Spacing: 0.1pt
Use: Form labels, input labels, tags
```

**Button:**
```
Font: Roboto Bold
Size: 16pt
Line Height: 24pt
Letter Spacing: 1.25pt
Transform: Uppercase
Use: All buttons
```

**Caption:**
```
Font: Roboto Regular
Size: 10pt
Line Height: 14pt
Letter Spacing: 0.4pt
Use: Hints, tooltips, footnotes
```

### Type Scale (VR)

**Adjustments for VR:**
- Add +4pt to all font sizes
- Increase line height by +4pt
- Increase letter spacing by +0.2pt
- Minimum font size: 18pt (for readability at distance)

**VR Examples:**
```
Heading 1 VR: 28pt / 36pt line height
Body VR: 18pt / 24pt line height
Button VR: 20pt / 28pt line height
Caption VR: 14pt / 18pt line height
```

### Text Rendering

**Anti-Aliasing:**
```
Method: Subpixel anti-aliasing (ClearType)
Hinting: Slight hinting for small sizes (<14pt)
Smoothing: Enabled
```

**Drop Shadows:**
```
Headings:
  - Offset: 2px, 2px
  - Blur: 4px
  - Color: #000000
  - Opacity: 50%

Body Text (on dark backgrounds):
  - Offset: 1px, 1px
  - Blur: 2px
  - Color: #000000
  - Opacity: 30%

VR Text (all):
  - Offset: 3px, 3px
  - Blur: 6px
  - Color: #000000
  - Opacity: 70%
  - Reason: Depth separation and readability
```

---

## Spacing System

### Base Unit

**Spacing Unit:** 8px (0.5rem)

All spacing should be multiples of 8px for consistency:
- 4px (0.5 unit) - Tight spacing
- 8px (1 unit) - Default spacing
- 16px (2 units) - Medium spacing
- 24px (3 units) - Large spacing
- 32px (4 units) - Extra large spacing
- 48px (6 units) - Section spacing
- 64px (8 units) - Page spacing

### Layout Spacing

**Container Padding:**
```
Small containers: 16px
Medium containers: 24px
Large containers: 32px
Full-page panels: 48px
```

**Grid Gaps:**
```
Tight grid: 8px
Normal grid: 16px
Loose grid: 24px
Section grid: 32px
```

**Stack Spacing:**
```
Tight stack: 4px (labels + values)
Normal stack: 8px (form fields)
Loose stack: 16px (sections)
```

**Inline Spacing:**
```
Icon + Text: 8px
Button groups: 12px
Breadcrumbs: 8px
Tags: 4px
```

### Component Spacing

**Buttons:**
```
Padding (horizontal): 24px
Padding (vertical): 12px
Icon + Text gap: 8px
Button group gap: 12px
```

**Input Fields:**
```
Padding (horizontal): 16px
Padding (vertical): 12px
Label margin-bottom: 8px
Helper text margin-top: 4px
Field-to-field gap: 16px
```

**Cards:**
```
Padding: 24px
Header margin-bottom: 16px
Content gap: 12px
Footer margin-top: 16px
```

**Dialogs:**
```
Padding: 32px
Section gaps: 24px
Button section margin-top: 32px
```

---

## Button Styles

### Primary Button

**Normal State:**
```
Background: #1E88E5 (Primary Blue)
Text Color: #FFFFFF
Border: None
Border Radius: 8px
Shadow: 0px 2px 4px rgba(0,0,0,0.3)
Padding: 12px 24px
Font: Roboto Bold, 16pt, Uppercase
```

**Hover State:**
```
Background: #2196F3 (Lighter Blue)
Text Color: #FFFFFF
Shadow: 0px 4px 8px rgba(0,0,0,0.4)
Scale: 1.05
Glow: 0px 0px 8px rgba(30,136,229,0.5)
Transition: 100ms ease-out
```

**Pressed State:**
```
Background: #1565C0 (Darker Blue)
Text Color: #FFFFFF
Shadow: 0px 1px 2px rgba(0,0,0,0.2)
Scale: 0.95
Transition: 50ms ease-in
```

**Disabled State:**
```
Background: #424242 (Surface)
Text Color: #9E9E9E (Disabled Text)
Border: None
Shadow: None
Cursor: not-allowed
Opacity: 0.5
```

**Focus State (Keyboard):**
```
Border: 2px solid #FFFFFF
Outline: 2px solid #1E88E5 (4px offset)
```

### Secondary Button

**Normal State:**
```
Background: #424242 (Surface)
Text Color: #FFFFFF
Border: 2px solid #616161 (Surface Light)
Border Radius: 8px
Shadow: None
Padding: 12px 24px
```

**Hover State:**
```
Background: #616161 (Surface Light)
Text Color: #FFFFFF
Border: 2px solid #757575 (Border)
Scale: 1.03
```

**Pressed State:**
```
Background: #212121 (Background Dark)
Text Color: #E0E0E0
Border: 2px solid #616161
Scale: 0.97
```

### Success Button

**Normal State:**
```
Background: #43A047 (Secondary Green)
Text Color: #FFFFFF
Border: None
Border Radius: 8px
Shadow: 0px 2px 4px rgba(0,0,0,0.3)
```

**Hover State:**
```
Background: #66BB6A (Success Light)
Glow: 0px 0px 8px rgba(67,160,71,0.5)
```

### Danger Button

**Normal State:**
```
Background: #E53935 (Error Red)
Text Color: #FFFFFF
Border: None
Border Radius: 8px
Shadow: 0px 2px 4px rgba(0,0,0,0.3)
```

**Hover State:**
```
Background: #EF5350 (Lighter Red)
Glow: 0px 0px 8px rgba(229,57,53,0.5)
```

### Icon Button

**Normal State:**
```
Background: Transparent
Icon Color: #E0E0E0
Border: None
Border Radius: 50% (circular)
Size: 40px × 40px
Icon Size: 24px × 24px
```

**Hover State:**
```
Background: rgba(255,255,255,0.1)
Icon Color: #FFFFFF
```

**Pressed State:**
```
Background: rgba(255,255,255,0.2)
Icon Color: #FFFFFF
Scale: 0.9
```

### Text Button (Link Style)

**Normal State:**
```
Background: Transparent
Text Color: #29B6F6 (Info)
Border: None
Padding: 8px 12px
Underline: None
```

**Hover State:**
```
Text Color: #4FC3F7 (Lighter Info)
Underline: 1px solid
```

**Pressed State:**
```
Text Color: #0288D1 (Darker Info)
```

---

## Input Styles

### Text Input

**Normal State:**
```
Background: #212121 (Background Dark)
Text Color: #FFFFFF
Border: 2px solid #616161 (Surface Light)
Border Radius: 4px
Padding: 12px 16px
Font: Roboto Regular, 14pt
Placeholder Color: #9E9E9E (60% opacity)
```

**Focus State:**
```
Border: 2px solid #1E88E5 (Primary)
Background: #2A2A2A (Slightly lighter)
Glow: 0px 0px 4px rgba(30,136,229,0.3)
```

**Error State:**
```
Border: 2px solid #E53935 (Error)
Background: rgba(229,57,53,0.05)
Error message: #E53935, 12pt, margin-top 4px
```

**Disabled State:**
```
Background: #424242 (Surface)
Border: 2px solid #757575 (Border)
Text Color: #9E9E9E (Disabled)
Cursor: not-allowed
```

### Dropdown / Select

**Normal State:**
```
Same as text input
Dropdown icon: Chevron down, 16px, right 12px
Icon color: #E0E0E0
```

**Open State:**
```
Border: 2px solid #1E88E5
Dropdown panel:
  - Background: #424242
  - Border: 2px solid #1E88E5
  - Shadow: 0px 8px 16px rgba(0,0,0,0.5)
  - Max height: 300px (scrollable)
```

**Option Hover:**
```
Background: #616161 (Surface Light)
Text Color: #FFFFFF
```

**Option Selected:**
```
Background: #1E88E5 (Primary)
Text Color: #FFFFFF
Check icon: Right side, 16px
```

### Checkbox

**Normal State:**
```
Box size: 20px × 20px
Background: #212121
Border: 2px solid #616161
Border Radius: 4px
Check icon: None
```

**Checked State:**
```
Background: #1E88E5 (Primary)
Border: 2px solid #1E88E5
Check icon: White checkmark, 14px
Animation: Scale 0.0 → 1.2 → 1.0 (200ms)
```

**Hover State:**
```
Border: 2px solid #1E88E5
Background: rgba(30,136,229,0.1)
```

### Radio Button

**Normal State:**
```
Circle size: 20px × 20px
Background: #212121
Border: 2px solid #616161
Border Radius: 50% (full circle)
Dot: None
```

**Selected State:**
```
Border: 2px solid #1E88E5
Dot: 10px circle, #1E88E5, centered
Animation: Scale 0.0 → 1.0 (150ms, ease-out)
```

### Slider

**Track:**
```
Height: 4px
Background: #616161 (Surface Light)
Border Radius: 2px
Fill (left of thumb): #1E88E5 (Primary)
```

**Thumb:**
```
Size: 20px × 20px
Background: #1E88E5 (Primary)
Border: 2px solid #FFFFFF
Border Radius: 50%
Shadow: 0px 2px 4px rgba(0,0,0,0.3)
```

**Thumb Hover:**
```
Scale: 1.2
Shadow: 0px 4px 8px rgba(0,0,0,0.4)
```

**Thumb Pressed:**
```
Scale: 1.1
Shadow: 0px 2px 4px rgba(0,0,0,0.3)
```

---

## Card Styles

### Standard Card

```
Background: #424242 (Surface)
Border: 1px solid #616161 (Surface Light)
Border Radius: 12px
Padding: 24px
Shadow: 0px 2px 8px rgba(0,0,0,0.2)
```

**Hover State:**
```
Border: 1px solid #1E88E5 (Primary)
Shadow: 0px 4px 16px rgba(0,0,0,0.3)
Transform: TranslateY(-2px)
Transition: 150ms ease-out
```

### Elevated Card

```
Background: #424242 (Surface)
Border: None
Border Radius: 12px
Padding: 24px
Shadow: 0px 4px 16px rgba(0,0,0,0.4)
```

### Outlined Card

```
Background: Transparent
Border: 2px solid #616161 (Surface Light)
Border Radius: 12px
Padding: 24px
Shadow: None
```

### Part Card (Rarity-Themed)

```
Base:
  - Background: #424242
  - Border Radius: 12px
  - Padding: 16px

Rarity Border:
  - Left edge: 4px solid {RarityColor}
  - Top edge: 1px solid {RarityColor} (30% opacity)

Rarity Glow (Rare+):
  - Box shadow: 0px 0px {GlowSize}px {RarityColor}
  - Opacity: Based on rarity

Content:
  - Icon: 64px × 64px, centered
  - Name: Roboto Bold, 14pt
  - Rarity badge: Top-right corner
  - Stats: Bottom section, 12pt
```

---

## Borders and Dividers

### Border Styles

**Thin Border:**
```
Width: 1px
Color: #616161 (Surface Light)
Style: Solid
```

**Medium Border:**
```
Width: 2px
Color: #757575 (Border)
Style: Solid
```

**Thick Border:**
```
Width: 3px
Color: #757575 (Border)
Style: Solid
```

**Rarity Border:**
```
Width: Based on rarity (2-4px)
Color: Rarity color
Style: Solid
Optional: Gradient for Legendary/Mythic
```

### Dividers

**Horizontal Divider:**
```
Height: 1px
Background: #616161 (Surface Light)
Margin: 16px 0
Width: 100%
```

**Vertical Divider:**
```
Width: 1px
Background: #616161 (Surface Light)
Margin: 0 16px
Height: 100%
```

**Section Divider:**
```
Height: 2px
Background: Linear gradient (#616161 → transparent → #616161)
Margin: 24px 0
```

---

## Shadows and Depth

### Shadow Levels

**Level 0 (No Shadow):**
```
Box Shadow: None
Use: Flat elements, overlays
```

**Level 1 (Subtle):**
```
Box Shadow: 0px 1px 3px rgba(0,0,0,0.12), 0px 1px 2px rgba(0,0,0,0.24)
Use: Buttons, chips, small cards
```

**Level 2 (Default):**
```
Box Shadow: 0px 2px 8px rgba(0,0,0,0.2), 0px 2px 4px rgba(0,0,0,0.3)
Use: Cards, panels, raised elements
```

**Level 3 (Elevated):**
```
Box Shadow: 0px 4px 16px rgba(0,0,0,0.3), 0px 4px 8px rgba(0,0,0,0.4)
Use: Modals, dialogs, dropdowns
```

**Level 4 (Floating):**
```
Box Shadow: 0px 8px 32px rgba(0,0,0,0.4), 0px 8px 16px rgba(0,0,0,0.5)
Use: Tooltips, popovers, context menus
```

**Level 5 (Maximum):**
```
Box Shadow: 0px 16px 48px rgba(0,0,0,0.5), 0px 16px 24px rgba(0,0,0,0.6)
Use: Full-screen overlays, splash screens
```

### Glow Effects

**Standard Glow:**
```
Box Shadow: 0px 0px 8px {Color}
Opacity: 50%
Use: Hover states, highlights
```

**Intense Glow:**
```
Box Shadow: 0px 0px 16px {Color}
Opacity: 80%
Use: Active states, selections
```

**Pulsing Glow (Animation):**
```
Box Shadow: 0px 0px {8-16}px {Color}
Animation: Pulse between 8px and 16px blur (2s cycle)
Opacity: 50-80%
Use: Legendary items, important notifications
```

---

## Border Radius

### Radius Scale

```
None: 0px - No rounding
XS: 2px - Minimal rounding (tags, badges)
SM: 4px - Small elements (inputs, checkboxes)
MD: 8px - Default (buttons, cards)
LG: 12px - Large panels (dialogs, modals)
XL: 16px - Extra large (hero sections)
Full: 50% or 9999px - Circular (avatars, icon buttons)
```

### Component Radii

```
Buttons: 8px (MD)
Cards: 12px (LG)
Inputs: 4px (SM)
Dialogs: 16px (XL)
Chips/Tags: 16px (pill shape)
Images: 8px (MD)
Avatars: 50% (Full circle)
Progress bars: 6px (between SM and MD)
```

---

## Icons

### Icon Sizes

```
XS: 16px × 16px - Inline with text
SM: 20px × 20px - Small buttons, tags
MD: 24px × 24px - Default buttons, list items
LG: 32px × 32px - Large buttons, cards
XL: 48px × 48px - Hero sections, headers
XXL: 64px × 64px - Part icons, feature displays
XXXL: 128px × 128px - Dialog headers, showcases
```

### Icon Styles

**Outlined:**
```
Stroke width: 2px
Fill: None
Use: Default state, secondary actions
```

**Filled:**
```
Stroke: None
Fill: Solid color
Use: Active state, primary actions
```

**Two-Tone:**
```
Primary fill: Full color
Secondary fill: 30% opacity
Use: Special features, rarity indicators
```

### Icon Colors

```
Default: #E0E0E0 (Text Secondary)
Hover: #FFFFFF (Text Primary)
Active: #1E88E5 (Primary)
Disabled: #9E9E9E (Text Disabled)
Success: #43A047 (Secondary)
Error: #E53935 (Error)
Warning: #FDD835 (Warning)
```

---

## Accessibility

### Color Blind Support

**Rarity Patterns:**
- Common: Solid (no pattern)
- Uncommon: Horizontal lines (|||)
- Rare: Dots (•••)
- Epic: Diagonal stripes (///)
- Legendary: Stars (★★★)

**High Contrast Mode:**
```
Background: #000000 (Pure black)
Text: #FFFFFF (Pure white)
Borders: 4px instead of 2px
Contrast Ratio: Minimum 7:1 (AAA standard)
```

### Focus Indicators

**Keyboard Focus:**
```
Outline: 2px solid #FFFFFF
Outline Offset: 2px
Box Shadow: 0px 0px 0px 4px #1E88E5
Border Radius: Inherit from element
```

**Focus Visible Only:**
- Show focus indicators only for keyboard navigation
- Hide focus indicators on mouse/touch interaction

### Touch Targets (VR/Mobile)

**Minimum Size:**
```
Desktop: 32px × 32px
Mobile: 44px × 44px
VR: 60px × 60px
```

**Spacing:**
```
Minimum gap between targets: 8px
Recommended gap: 12px
VR gap: 16px
```

---

## Responsive Breakpoints

### Breakpoint Values

```
XS (Mobile): 0px - 599px
SM (Tablet): 600px - 959px
MD (Desktop): 960px - 1279px
LG (Large Desktop): 1280px - 1919px
XL (Extra Large): 1920px+
```

### Scaling Rules

**Typography:**
```
XS: Base size - 2pt
SM: Base size - 1pt
MD: Base size
LG: Base size + 1pt
XL: Base size + 2pt
```

**Spacing:**
```
XS: Reduce by 25%
SM: Reduce by 15%
MD: Standard
LG: Increase by 15%
XL: Increase by 25%
```

**Layout:**
```
XS: Single column, full width
SM: Single column, max-width 600px
MD: Two columns, max-width 960px
LG: Three columns, max-width 1280px
XL: Three columns, max-width 1920px
```

---

## VR-Specific Adjustments

### World-Space UI

**Distance Scaling:**
```
1.0m: Scale 1.0 (base)
1.5m: Scale 1.2
2.0m: Scale 1.5
3.0m: Scale 2.0
```

**Readability:**
```
Minimum font size: 18pt
Preferred font size: 20-24pt
Line height: +4pt from desktop
Letter spacing: +0.2pt
```

### VR Color Adjustments

**Brightness:**
```
Increase overall brightness by 10%
Increase contrast by 15%
Reason: Lenses reduce perceived brightness
```

**Saturation:**
```
Increase saturation by 5%
Reason: Maintain color vibrancy in headset
```

### VR Glow Effects

**Reduced Glow:**
```
Max blur: 8px (vs 16px on desktop)
Max opacity: 60% (vs 80% on desktop)
Reason: Prevent eye strain and discomfort
```

---

## Material Specifications

### UI Material: M_UI_Base

**Properties:**
```
Blend Mode: Translucent
Shading Model: Unlit
Two Sided: False
```

**Parameters:**
```
BaseColor: Color (Default: #FFFFFF)
Opacity: Scalar (0.0 - 1.0, Default: 1.0)
EmissiveColor: Color (Default: #000000)
EmissiveIntensity: Scalar (0.0 - 10.0, Default: 0.0)
```

### UI Material: M_UI_Glow

**Properties:**
```
Blend Mode: Additive
Shading Model: Unlit
Two Sided: False
```

**Parameters:**
```
GlowColor: Color (Default: #1E88E5)
GlowIntensity: Scalar (0.0 - 5.0, Default: 1.0)
GlowSize: Scalar (0.0 - 32.0, Default: 8.0)
PulseSpeed: Scalar (0.0 - 5.0, Default: 1.0)
```

### UI Material: M_ProgressBar

**Parameters:**
```
FillColor: Color (Default: #43A047)
BackgroundColor: Color (Default: #212121)
BorderColor: Color (Default: #616161)
FillPercent: Scalar (0.0 - 1.0, Default: 0.0)
GlowPosition: Scalar (0.0 - 1.0, Default: 0.5)
GlowIntensity: Scalar (0.0 - 2.0, Default: 0.0)
```

---

## Theme Data Asset Structure

### DA_UITheme (Data Asset)

```cpp
UCLASS()
class UDA_UITheme : public UDataAsset
{
    GENERATED_BODY()

public:
    // Colors
    UPROPERTY(EditAnywhere, Category="Colors|Primary")
    FLinearColor PrimaryColor;

    UPROPERTY(EditAnywhere, Category="Colors|Primary")
    FLinearColor SecondaryColor;

    UPROPERTY(EditAnywhere, Category="Colors|Neutral")
    FLinearColor BackgroundDark;

    UPROPERTY(EditAnywhere, Category="Colors|Neutral")
    FLinearColor Surface;

    // Typography
    UPROPERTY(EditAnywhere, Category="Typography")
    UFont* PrimaryFont;

    UPROPERTY(EditAnywhere, Category="Typography")
    int32 HeadingSize;

    UPROPERTY(EditAnywhere, Category="Typography")
    int32 BodySize;

    // Spacing
    UPROPERTY(EditAnywhere, Category="Spacing")
    float SpacingUnit;

    // Rarity colors
    UPROPERTY(EditAnywhere, Category="Colors|Rarity")
    TMap<EItemRarity, FLinearColor> RarityColors;

    // Materials
    UPROPERTY(EditAnywhere, Category="Materials")
    UMaterialInterface* UIBaseMaterial;

    UPROPERTY(EditAnywhere, Category="Materials")
    UMaterialInterface* UIGlowMaterial;

    // Sounds
    UPROPERTY(EditAnywhere, Category="Audio")
    USoundBase* ButtonClickSound;

    UPROPERTY(EditAnywhere, Category="Audio")
    USoundBase* UnlockSuccessSound;
};
```

---

**Document Version:** 1.0
**Last Updated:** 2025-11-07
**Author:** Ship Customization System - Phase 3, Subagent 4
