# UI Widgets Specifications

Complete specifications for all SolarSystem test level UI widgets.

---

## WBP_Chronometer - Three-Clock Time Display

**Blueprint Location:** `Content/UI/WBP_Chronometer`
**Parent Class:** `UChronometerWidget` (C++ class from Source/Alexander/Public/ChronometerWidget.h)

### Purpose
Display three synchronized time scales with time dilation visualization.

### Widget Hierarchy
```
Canvas Panel (Root)
├── BackgroundPanel (Image - semi-transparent)
│   └── ContentBox (Vertical Box)
│       ├── HeaderText (Text Block) - "CHRONOMETER"
│       ├── ClockContainer (Horizontal Box)
│       │   ├── RealTimePanel (Vertical Box)
│       │   │   ├── RealTimeLabel (Text) - "REAL TIME"
│       │   │   └── RealTimeText (Text) - "00:15:34"
│       │   ├── UniversalTimePanel (Vertical Box)
│       │   │   ├── UniversalTimeLabel (Text) - "UNIVERSAL TIME"
│       │   │   └── UniversalTimeText (Text) - "Day 1, 02:47:18"
│       │   └── BiologicalTimePanel (Vertical Box)
│       │       ├── BiologicalTimeLabel (Text) - "BIOLOGICAL AGE"
│       │       └── BiologicalTimeText (Text) - "25.14 years"
│       ├── TimeDebtPanel (Horizontal Box)
│       │   ├── TimeDebtLabel (Text) - "TIME DEBT:"
│       │   └── TimeDebtText (Text) - "3 days, 4 hours"
│       ├── TimeDilationPanel (Vertical Box)
│       │   ├── TimeDilationLabel (Text) - "TIME DILATION"
│       │   └── TimeDilationBar (Progress Bar)
│       └── PhysicsDataPanel (Horizontal Box)
│           ├── VelocityText (Text) - "Velocity: 1,234 km/s"
│           └── AltitudeText (Text) - "Altitude: 10,542 km"
```

### Widget Properties (Bind to C++)

**Text Blocks to Bind:**
```cpp
UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
UTextBlock* RealTimeText;

UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
UTextBlock* UniversalTimeText;

UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
UTextBlock* BiologicalTimeText;

UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
UTextBlock* TimeDebtText;

UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
UTextBlock* VelocityText;

UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
UTextBlock* AltitudeText;

UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
UProgressBar* TimeDilationBar;
```

**Configuration Properties:**
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite)
bool bAutoUpdate = true;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
float UpdateFrequency = 0.5f; // 2 Hz

UPROPERTY(EditAnywhere, BlueprintReadWrite)
bool bShowPhysicalData = true;
```

### Event Graph

**Event Construct:**
```
1. Call parent Construct (C++ initialization)
2. Get RelativitySystem subsystem
3. Store reference
4. If bAutoUpdate:
   - Set Timer by Function Name: "UpdateChronometer"
   - Time: UpdateFrequency
   - Looping: true
```

**Function: UpdateChronometer (Event Tick or Timer)**
```
Called from C++ or Blueprint timer

1. Get RelativitySystem reference
2. Get local player controller
3. If valid:
   - Get Real Time: GetWorld()->GetTimeSeconds()
   - Format: HH:MM:SS
   - Set RealTimeText

   - Get Universal Time: RelativitySystem->GetUniversalTime()
   - Format: "Day X, HH:MM:SS"
   - Set UniversalTimeText

   - Get Biological Age: RelativitySystem->GetPlayerBiologicalAge(PlayerController)
   - Format: "XX.XX years"
   - Set BiologicalTimeText

   - Get Time Debt: RelativitySystem->GetTimeDebt(PlayerController)
   - Format: "X days, X hours, X minutes"
   - Set TimeDebtText
   - Color: Green if <1 day, Yellow if <7 days, Red if >7 days

   - Get Time Dilation: RelativitySystem->GetCombinedTimeDilation(PlayerPawn)
   - Set TimeDilationBar percent: Dilation (0.0 to 1.0)
   - Color: Green if >0.9, Yellow if >0.7, Orange if >0.5, Red if <0.5

4. If bShowPhysicalData:
   - Get Player Pawn velocity
   - Convert to km/s: Velocity / 100000
   - Set VelocityText: "Velocity: XXX km/s"

   - Get nearest celestial body distance
   - Set AltitudeText: "Altitude: XXX km"
```

### Styling

**Colors:**
```
Background: (0.05, 0.05, 0.05, 0.8) - Dark semi-transparent
Text (Normal): (0.9, 0.9, 0.9, 1.0) - Light gray
Text (Warning): (1.0, 0.8, 0.0, 1.0) - Yellow
Text (Critical): (1.0, 0.2, 0.0, 1.0) - Red
Labels: (0.6, 0.6, 0.6, 1.0) - Medium gray

Time Dilation Bar:
- Green: (0.0, 1.0, 0.0) for 0.9-1.0
- Yellow: (1.0, 1.0, 0.0) for 0.7-0.9
- Orange: (1.0, 0.5, 0.0) for 0.5-0.7
- Red: (1.0, 0.0, 0.0) for <0.5
```

**Fonts:**
```
Header: Roboto Bold, 18pt
Labels: Roboto Regular, 12pt
Values: Roboto Mono, 16pt (monospace for numbers)
```

**Layout:**
```
Position: Top-center of screen
Anchor: (0.5, 0.0) - Top center
Alignment: (0.5, 0.0)
Offset: (0, 20) - 20 pixels from top
Size: (600, 200) pixels
```

---

## WBP_DebugOverlay - Developer Information Display

**Blueprint Location:** `Content/UI/WBP_DebugOverlay`
**Parent Class:** `UserWidget`

### Purpose
Show real-time debug information for celestial scaling system.

### Widget Hierarchy
```
Canvas Panel (Root)
└── DebugPanel (Vertical Box) - Anchored top-left
    ├── TitleText (Text) - "=== CELESTIAL SCALING DEBUG ==="
    ├── Spacer (Vertical Spacer) - 10px
    ├── VirtualPositionSection (Vertical Box)
    │   ├── VirtualPosLabel (Text) - "Virtual Position:"
    │   ├── SectorText (Text) - "  Sector: (X, Y, Z)"
    │   ├── LocalOffsetText (Text) - "  Local: (X, Y, Z) cm"
    │   └── DistanceFromOriginText (Text) - "  Distance: XXX km"
    ├── CelestialBodiesSection (Vertical Box)
    │   ├── BodiesLabel (Text) - "Celestial Bodies:"
    │   ├── RegisteredCountText (Text) - "  Registered: XX"
    │   └── NearbyBodiesList (Vertical Box) - Dynamic list
    ├── GravitySection (Vertical Box)
    │   ├── GravityLabel (Text) - "Gravity System:"
    │   ├── GravityModeText (Text) - "  Mode: MultiBody"
    │   ├── TotalForceText (Text) - "  Total Force: XX.XX N"
    │   └── GForceText (Text) - "  G-Force: X.XX G"
    ├── ScalingSection (Vertical Box)
    │   ├── ScalingLabel (Text) - "Scaling System:"
    │   ├── NearestBodyText (Text) - "  Nearest: Earth"
    │   ├── DistanceText (Text) - "  Distance: XXX km"
    │   └── ScaleFactorText (Text) - "  Scale: X.XXx"
    ├── NetworkSection (Vertical Box)
    │   ├── NetworkLabel (Text) - "Network:"
    │   ├── PingText (Text) - "  Ping: XX ms"
    │   └── PlayersText (Text) - "  Players: X"
    └── PerformanceSection (Vertical Box)
        ├── PerformanceLabel (Text) - "Performance:"
        ├── FPSText (Text) - "  FPS: XX"
        └── FrameTimeText (Text) - "  Frame: X.X ms"
```

### Blueprint Variables

```cpp
UPROPERTY(BlueprintReadWrite)
UPlayerOriginManager* OriginManager;

UPROPERTY(BlueprintReadWrite)
UCelestialBodyRegistry* BodyRegistry;

UPROPERTY(BlueprintReadWrite)
UGravitySimulator* GravitySimulator;

UPROPERTY(BlueprintReadWrite)
float UpdateInterval = 0.1f; // 10 Hz
```

### Event Graph

**Event Construct:**
```
1. Get Game Mode → Cast to BP_SolarSystemGameMode
2. Get subsystem references:
   - OriginManager: From player pawn
   - BodyRegistry: From world subsystem
   - GravitySimulator: From game instance subsystem
3. Set Timer: UpdateDebugInfo, every UpdateInterval
```

**Function: UpdateDebugInfo**
```
1. Update Virtual Position:
   - Get from OriginManager->GetVirtualPosition()
   - Set SectorText: Format sector coordinates
   - Set LocalOffsetText: Format local offset
   - Calculate and set DistanceFromOriginText

2. Update Celestial Bodies:
   - Get BodyRegistry->GetAllCelestialBodies()
   - Set RegisteredCountText: Count
   - Get bodies sorted by distance
   - Update NearbyBodiesList (top 5):
     - For each body:
       - Create text: "[Name]: [Distance] km, Scale: [Factor]x"

3. Update Gravity:
   - Get GravitySimulator->GetSimulationMode()
   - Set GravityModeText
   - Get total force on player
   - Set TotalForceText
   - Calculate G-force: Force / (Mass * 9.81)
   - Set GForceText
   - Color: Green if <5G, Yellow if <20G, Red if >20G

4. Update Scaling:
   - Get nearest body from BodyRegistry
   - Set NearestBodyText: Body name
   - Set DistanceText: Distance in km
   - Get CelestialBodyComponent->GetCurrentScaleFactor()
   - Set ScaleFactorText

5. Update Network (if multiplayer):
   - Get PlayerController->PlayerState->Ping
   - Set PingText
   - Get Game State->PlayerArray.Num()
   - Set PlayersText

6. Update Performance:
   - Get FPS: 1.0 / DeltaTime
   - Set FPSText
   - Color: Green if >90, Yellow if >60, Red if <60
   - Set FrameTimeText: DeltaTime * 1000 (ms)
```

### Styling

**Colors:**
```
Background: (0.0, 0.0, 0.0, 0.7) - Very dark, semi-transparent
Text (Normal): (0.0, 1.0, 0.0, 1.0) - Terminal green
Text (Warning): (1.0, 1.0, 0.0, 1.0) - Yellow
Text (Error): (1.0, 0.0, 0.0, 1.0) - Red
Section Labels: (0.0, 1.0, 1.0, 1.0) - Cyan
```

**Fonts:**
```
All text: Courier New (monospace), 11pt
Labels: Bold
```

**Layout:**
```
Position: Top-left corner
Anchor: (0.0, 0.0)
Padding: 10 pixels
Size: Auto-size based on content
Max Width: 400 pixels
```

### Toggle Functionality

**Console Command or Key Binding:**
```
Input Action: ToggleDebugOverlay (F3 key)

In PlayerController or Game Mode:
1. Get WBP_DebugOverlay reference
2. Toggle visibility:
   - If visible: SetVisibility(Collapsed)
   - If hidden: SetVisibility(Visible)
```

---

## WBP_TimeWarpWarning - Time Warp Confirmation Dialog

**Blueprint Location:** `Content/UI/WBP_TimeWarpWarning`
**Parent Class:** `UTimeWarpWarningWidget` (C++ class)

### Purpose
Warn player about time warp consequences before activation.

### Widget Hierarchy
```
Canvas Panel (Root)
└── ModalBackground (Image - Full screen, dark overlay)
    └── WarningDialog (Border - Centered)
        └── DialogContent (Vertical Box)
            ├── HeaderPanel (Horizontal Box)
            │   ├── WarningIcon (Image) - ⚠️ icon
            │   └── HeaderText (Text) - "TIME WARP WARNING"
            ├── Spacer (10px)
            ├── WarpFactorText (Text) - "Warp Factor: XXXx"
            ├── DurationText (Text) - "Estimated Duration: X minutes"
            ├── Spacer (20px)
            ├── PredictionsLabel (Text) - "PREDICTIONS:"
            ├── PredictionsList (Vertical Box)
            │   ├── UniverseTimeText (Text) - "Universe will advance: X hours"
            │   ├── PlayerTimeText (Text) - "You will experience: X minutes"
            │   ├── TimeDebtText (Text) - "Time Debt Increase: +X hours"
            │   └── SeverityText (Text) - "Severity: [Level]"
            ├── Spacer (20px)
            ├── ConsequencesLabel (Text) - "CONSEQUENCES:"
            ├── ConsequencesList (Vertical Box)
            │   ├── NPCAgingText (Text) - "• NPCs will age X hours"
            │   ├── MissionsText (Text) - "• Time-sensitive missions may fail"
            │   ├── OrbitalsText (Text) - "• Planetary orbits will shift"
            │   └── WarningText (Text) - "⚠️ CANNOT BE UNDONE"
            ├── Spacer (30px)
            └── ButtonPanel (Horizontal Box)
                ├── CancelButton (Button) - "CANCEL"
                ├── Spacer (Horizontal)
                └── ConfirmButton (Button) - "I UNDERSTAND - PROCEED"
```

### Widget Properties (Bind to C++)

```cpp
UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
UTextBlock* WarpFactorText;

UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
UTextBlock* DurationText;

UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
UTextBlock* UniverseTimeText;

UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
UTextBlock* PlayerTimeText;

UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
UTextBlock* TimeDebtText;

UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
UTextBlock* SeverityText;

UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
UButton* ConfirmButton;

UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
UButton* CancelButton;

// Configuration
UPROPERTY(EditAnywhere, BlueprintReadWrite)
float WarpFactor = 1.0f;

UPROPERTY(EditAnywhere, BlueprintReadWrite)
float EstimatedDuration = 60.0f; // seconds
```

### Event Graph

**Function: ShowTimeWarpWarning**
```
Parameters:
- InWarpFactor (Float)
- InEstimatedDuration (Float)

1. Store WarpFactor and EstimatedDuration
2. Calculate predictions:
   - Universe time advance: EstimatedDuration * WarpFactor
   - Player time: EstimatedDuration * GetCurrentTimeDilation()
   - Time debt increase: Universe time - Player time

3. Update UI text:
   - Set WarpFactorText: Format with commas if large
   - Set DurationText: Format as readable time
   - Set UniverseTimeText: Format predictions
   - Set PlayerTimeText
   - Set TimeDebtText

4. Determine severity:
   - Low: < 1 hour time debt
   - Moderate: 1-6 hours
   - High: 6-24 hours
   - Extreme: >24 hours
   - Set SeverityText with color

5. Add to viewport (modal):
   - Z-Order: 1000 (top)
   - Focus on ConfirmButton

6. Pause game (optional):
   - SetGamePaused(true)
```

**Event: ConfirmButton Clicked**
```
1. Get Game Mode or PlayerController
2. Call: SetTimeWarpFactor(WarpFactor)
3. Remove from parent (close dialog)
4. Unpause game
5. Log: "Time warp activated: [WarpFactor]x"
```

**Event: CancelButton Clicked**
```
1. Remove from parent (close dialog)
2. Unpause game
3. Log: "Time warp cancelled"
```

### Styling

**Modal Background:**
```
Color: (0.0, 0.0, 0.0, 0.8) - Very dark overlay
Render Opacity: 0.8
```

**Dialog Border:**
```
Background: (0.1, 0.1, 0.1, 1.0) - Dark gray
Border Color: Based on severity:
  - Low: (0.0, 1.0, 0.0) Green
  - Moderate: (1.0, 1.0, 0.0) Yellow
  - High: (1.0, 0.5, 0.0) Orange
  - Extreme: (1.0, 0.0, 0.0) Red
Border Width: 3px
Padding: 30px
Size: (600, 500) pixels
```

**Buttons:**
```
ConfirmButton:
- Background: (1.0, 0.2, 0.0) Red
- Text: White, Bold, 14pt
- Hovered: Brighter red
- Size: (300, 50)

CancelButton:
- Background: (0.3, 0.3, 0.3) Gray
- Text: White, Regular, 14pt
- Hovered: Lighter gray
- Size: (150, 50)
```

**Text Colors:**
```
Header: White
Warp Info: Cyan
Predictions: Yellow
Consequences: Orange/Red
Warning (Cannot Undo): Bright Red, Blinking
```

---

## Widget Integration in Game

### In BP_SolarSystemGameMode BeginPlay:

```
1. Create WBP_Chronometer:
   - Create Widget
   - Add to Viewport
   - Z-Order: 50

2. Create WBP_DebugOverlay:
   - Create Widget
   - Add to Viewport
   - Z-Order: 100
   - Initially Hidden (Visibility: Collapsed)

3. Store references for later access
```

### In BP_VRSpaceshipPlayer:

```
Can also create widgets here and attach to player HUD
```

### Time Warp Activation:

```
In Input Event or Function:
1. Get desired warp factor from input (e.g., thumbstick)
2. If factor > threshold:
   - Create WBP_TimeWarpWarning
   - Call ShowTimeWarpWarning(Factor, EstimatedDuration)
   - Wait for user response
3. Else:
   - Activate warp immediately
```

---

## Testing Checklist

### WBP_Chronometer:
- [ ] All three clocks display
- [ ] Times update correctly
- [ ] Time debt calculation accurate
- [ ] Dilation bar shows correct values
- [ ] Colors change based on severity
- [ ] Physics data updates (velocity, altitude)
- [ ] Readable in VR

### WBP_DebugOverlay:
- [ ] Virtual position displays accurately
- [ ] Celestial bodies list updates
- [ ] Gravity data shows correctly
- [ ] Scaling info matches actual scaling
- [ ] Performance metrics accurate
- [ ] Toggle on/off works
- [ ] Text is monospace and aligned

### WBP_TimeWarpWarning:
- [ ] Predictions calculate correctly
- [ ] Severity levels display with right colors
- [ ] Confirm activates time warp
- [ ] Cancel closes without activating
- [ ] Modal overlay blocks interaction
- [ ] Game pauses while open
- [ ] Warning text is prominent

---

**Created:** 2025-01-05
**Version:** 1.0
