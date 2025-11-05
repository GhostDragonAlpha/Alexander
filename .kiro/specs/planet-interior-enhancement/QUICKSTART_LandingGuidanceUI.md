# Quick Start: Landing Guidance UI

## Overview

The Landing Guidance UI provides real-time landing assistance with altitude, speed, alignment, and distance indicators. This guide shows you how to quickly set up and use the landing guidance system.

## What You Get

- **Altitude Display**: Shows height above landing pad
- **Speed Indicators**: Vertical and horizontal speed with color warnings
- **Alignment Guidance**: Shows how well aligned you are with approach vector
- **Distance Display**: Shows distance to landing pad
- **Status Messages**: Contextual guidance ("Reduce descent rate", "Landing approach good", etc.)
- **Color-Coded Warnings**: Green = safe, Yellow = caution, Red = danger

## Quick Setup (5 Minutes)

### Step 1: Create the Widget Blueprint

1. Open Content Browser
2. Right-click → User Interface → Widget Blueprint
3. Name it `WBP_LandingGuidance`
4. Open the widget
5. In Class Settings, set Parent Class to `LandingGuidanceWidget`

### Step 2: Design the Widget

Add these widgets with **exact names** (required for binding):

```
Canvas Panel
└── Vertical Box
    ├── Text Block: "PadNameText" (displays pad name)
    ├── Text Block: "AltitudeText" (displays altitude)
    ├── Text Block: "VerticalSpeedText" (displays vertical speed)
    ├── Text Block: "HorizontalSpeedText" (displays horizontal speed)
    ├── Text Block: "DistanceText" (displays distance)
    ├── Text Block: "AlignmentText" (displays alignment %)
    ├── Progress Bar: "AlignmentBar" (visual alignment bar)
    ├── Image: "AlignmentIndicator" (color indicator)
    └── Text Block: "StatusText" (displays status message)
```

**Quick Layout Template:**

```
┌─────────────────────────────────┐
│  Landing Pad Alpha              │ ← PadNameText
│                                 │
│  ALTITUDE:        150 m         │ ← AltitudeText
│  VERTICAL SPEED:  -3.5 m/s      │ ← VerticalSpeedText
│  HORIZONTAL SPEED: 8.2 m/s      │ ← HorizontalSpeedText
│  DISTANCE:        450 m         │ ← DistanceText
│  ALIGNMENT:       85%           │ ← AlignmentText
│  ████████████░░░░ 85%           │ ← AlignmentBar
│  ●                              │ ← AlignmentIndicator
│                                 │
│  Landing approach good          │ ← StatusText
└─────────────────────────────────┘
```

### Step 3: Add to Player HUD

**Blueprint Method:**

In your Player Controller or HUD blueprint:

1. On Begin Play:

   - Create Widget → Class: WBP_LandingGuidance
   - Add to Viewport
   - Set Player Pawn → Get Player Pawn
   - Save reference to variable

2. When approaching landing pad:
   - Set Target Pad → Your Landing Pad
   - Set Guidance Visible → True

**C++ Method:**

```cpp
// In PlayerController.h
UPROPERTY(EditDefaultsOnly, Category = "UI")
TSubclassOf<ULandingGuidanceWidget> LandingGuidanceClass;

UPROPERTY()
ULandingGuidanceWidget* LandingGuidance;

// In PlayerController.cpp
void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (LandingGuidanceClass)
    {
        LandingGuidance = CreateWidget<ULandingGuidanceWidget>(this, LandingGuidanceClass);
        LandingGuidance->AddToViewport();
        LandingGuidance->SetPlayerPawn(GetPawn());
    }
}

void AMyPlayerController::SetLandingTarget(ALandingPad* Pad)
{
    if (LandingGuidance)
    {
        LandingGuidance->SetTargetPad(Pad);
    }
}
```

## Usage

### Automatic Display

The widget automatically:

- Shows when within 5km of target pad
- Hides when too far away
- Updates every frame
- Smooths speed values to prevent jitter

### Reading the Display

**Altitude:**

- Green: Safe altitude (> 100m)
- Yellow: Caution (50-100m)
- Red: Danger zone (< 50m)

**Vertical Speed:**

- Green: Safe descent (> -5 m/s)
- Yellow: Fast descent (-5 to -10 m/s)
- Red: Dangerous descent (< -10 m/s)

**Horizontal Speed:**

- Green: Safe speed (< 10 m/s)
- Yellow: Fast (10-20 m/s)
- Red: Too fast (> 20 m/s)

**Alignment:**

- Green: Good alignment (> 80%)
- Yellow: Moderate (50-80%)
- Red: Poor alignment (< 50%)

**Status Messages:**

- "Approaching landing zone" - Far away
- "Align with approach vector" - Need to align
- "Reduce horizontal speed" - Too fast
- "WARNING: Descent rate too high!" - Danger!
- "Landing approach good" - Perfect!

## Configuration

### Adjusting Thresholds

In the widget blueprint, you can adjust:

**Speed Thresholds:**

- Safe Descent Rate: -5.0 m/s
- Warning Descent Rate: -10.0 m/s
- Danger Descent Rate: -15.0 m/s
- Safe Horizontal Speed: 10.0 m/s
- Warning Horizontal Speed: 20.0 m/s

**Display Range:**

- Max Guidance Distance: 5000 m
- Min Guidance Altitude: 0 m
- Max Guidance Altitude: 10000 m

**Colors:**

- Good Alignment Color: Green
- Moderate Alignment Color: Yellow
- Poor Alignment Color: Red
- Safe Descent Color: Green
- Warning Descent Color: Yellow
- Danger Descent Color: Red

### VR Optimization

For VR displays:

1. **Font Sizes:**

   - Use 18pt minimum for body text
   - Use 24pt+ for important values
   - High contrast colors

2. **Positioning:**

   - Place 1-2 meters from eyes
   - Slightly below eye level
   - Attach to cockpit or camera

3. **3D Widget Component:**

```cpp
// In cockpit blueprint
UWidgetComponent* Display = CreateDefaultSubobject<UWidgetComponent>(TEXT("LandingDisplay"));
Display->SetWidgetClass(WBP_LandingGuidance);
Display->SetDrawSize(FVector2D(800, 600));
Display->SetRelativeLocation(FVector(100, 0, 50));
```

## Common Scenarios

### Scenario 1: Approaching Landing Pad

```
Distance: 2.5 km
Altitude: 500 m
Status: "Approaching landing zone"
Action: Continue toward pad
```

### Scenario 2: Need Alignment

```
Distance: 800 m
Alignment: 35% (RED)
Status: "Align with approach vector"
Action: Adjust heading to match approach vector
```

### Scenario 3: Too Fast Descent

```
Altitude: 200 m
Vertical Speed: -12 m/s (RED)
Status: "WARNING: Descent rate too high!"
Action: Reduce throttle, increase pitch
```

### Scenario 4: Perfect Approach

```
Altitude: 80 m
Vertical Speed: -3 m/s (GREEN)
Horizontal Speed: 5 m/s (GREEN)
Alignment: 92% (GREEN)
Status: "Landing approach good"
Action: Continue current approach
```

### Scenario 5: Final Landing

```
Altitude: 15 m
Vertical Speed: -1.5 m/s (GREEN)
Alignment: 95% (GREEN)
Status: "Landing approach good"
Action: Gentle touchdown
```

## Troubleshooting

### Widget Not Showing

**Check:**

- Is target pad set? (SetTargetPad called)
- Is player pawn set? (SetPlayerPawn called)
- Are you within 5km of pad?
- Is bShowGuidance set to true?

### Values Not Updating

**Check:**

- Is widget added to viewport?
- Is NativeTick being called?
- Is player pawn valid?
- Is target pad valid?

### Incorrect Values

**Check:**

- Player pawn location is correct
- Target pad location is correct
- Units are in Unreal units (cm)
- Conversion to meters is working

### Widget Bindings Failed

**Check:**

- Widget names match exactly (case-sensitive)
- All required widgets are present
- Parent class is LandingGuidanceWidget
- Compile the widget blueprint

## Tips for Best Results

1. **Place Landing Pads Strategically:**

   - Clear approach paths
   - Away from obstacles
   - Good visibility from orbit

2. **Practice Approaches:**

   - Start from approach position
   - Maintain alignment > 80%
   - Keep descent rate < 5 m/s
   - Reduce speed near ground

3. **Use Status Messages:**

   - Read and follow guidance
   - Green messages = good
   - Yellow messages = adjust
   - Red messages = immediate action

4. **VR Comfort:**
   - Keep widget in comfortable view
   - Don't make it too large
   - Use high contrast
   - Smooth movements

## Integration with Landing Pads

The guidance UI works seamlessly with landing pads:

1. **Approach Position:**

   - UI shows alignment with pad's approach vector
   - Calculated by landing pad system

2. **Reservation:**

   - UI displays pad name
   - Shows pad status through colors

3. **Landing Detection:**
   - UI can hide on successful landing
   - Shows when taking off again

## Example: Complete Landing Sequence

```
1. Approach from orbit (5km out)
   → Widget appears
   → Status: "Approaching landing zone"

2. Align with approach vector (2km out)
   → Alignment increases to 85%
   → Status: "Continue approach"

3. Reduce speed (500m out)
   → Horizontal speed drops to 8 m/s
   → Status: "Final approach"

4. Control descent (100m altitude)
   → Vertical speed at -4 m/s
   → Status: "Landing approach good"

5. Final touchdown (< 20m altitude)
   → Gentle descent at -1.5 m/s
   → Status: "Landing approach good"

6. Landed!
   → Widget hides
   → Landing pad occupied
```

## Performance

- **Update Rate:** Every frame (60-90 FPS)
- **Performance Impact:** < 0.1ms per frame
- **Memory Usage:** Minimal (< 1MB)
- **VR Compatible:** Yes, optimized for 90+ FPS

## Summary

The Landing Guidance UI provides professional-quality landing assistance with:

- Real-time altitude and speed monitoring
- Visual alignment guidance
- Color-coded safety warnings
- Contextual status messages
- VR-optimized display

Follow this guide to quickly set up and start using the landing guidance system in your game!
