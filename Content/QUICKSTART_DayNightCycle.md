# Quick Start: Day-Night Cycle System

## Overview

The Day-Night Cycle system provides realistic time-of-day simulation with automatic sun movement, sky color transitions, star visibility, and twilight effects.

## Basic Setup

### 1. Automatic Setup (Recommended)

The day-night cycle is automatically created when you add a Planet actor to your level:

1. Add a Planet actor to your level
2. Assign a Planet Configuration data asset
3. The day-night cycle will initialize automatically using the planet's rotation period

### 2. Manual Configuration

To customize the day-night cycle behavior:

1. Select your Planet actor
2. Find the "Day Night Cycle Component" in the Details panel
3. Expand "Day-Night Cycle" category
4. Adjust the following settings:

```
Cycle Config:
  - Day Length In Seconds: 1200.0 (20 minutes real-time)
  - Start Time Of Day: 12.0 (noon)
  - Auto Progress: true
  - Time Speed: 1.0 (normal speed)
  - Axial Tilt: 23.5 (degrees)
  - Rotation Axis: (0, 0, 1)
```

### 3. Sky Color Customization

Customize sky colors for different times of day:

1. Expand "Sky Colors" category
2. Configure four color presets:
   - **Night Colors**: Dark sky, dim lighting
   - **Dawn Colors**: Orange/pink horizon, blue zenith
   - **Day Colors**: Bright blue sky
   - **Dusk Colors**: Red/orange horizon, darkening zenith

Each preset includes:

- Zenith Color (top of sky)
- Horizon Color (horizon line)
- Sun Color (light color)
- Sun Intensity (brightness)

## Lighting Setup

### Required Lights

The day-night cycle requires two lights in your level:

#### 1. Directional Light (Sun)

1. Add a Directional Light to your level
2. Name it "Sun" or similar
3. The day-night cycle will automatically find and control it
4. Or manually assign it in the component's "Lighting" section

**Settings**:

- Mobility: Movable (required for rotation)
- Intensity: Will be controlled by day-night cycle
- Light Color: Will be controlled by day-night cycle

#### 2. Sky Light

1. Add a Sky Light to your level
2. Set Source Type to "SLS Captured Scene"
3. The day-night cycle will automatically find and control it
4. Or manually assign it in the component's "Lighting" section

**Settings**:

- Mobility: Movable (required for updates)
- Real Time Capture: true (for dynamic updates)
- Recapture Scene: Will be triggered by day-night cycle

## Testing the System

### Fast Time Testing

To quickly test the day-night cycle:

1. Select your Planet actor
2. Set "Time Speed" to 60.0 (60x faster)
3. Set "Day Length In Seconds" to 120.0 (2 minutes)
4. Play the level and watch a full day cycle in 2 minutes

### Manual Time Control

To manually control time for testing:

**Blueprint**:

```
// Set specific time
DayNightCycleComponent->SetTimeOfDay(18.0) // 6 PM

// Advance time
DayNightCycleComponent->AdvanceTime(2.0) // +2 hours

// Pause auto progression
DayNightCycleComponent->CycleConfig.bAutoProgress = false
```

**Console Commands** (if exposed):

```
SetTimeOfDay 6.0    // Set to 6 AM
SetTimeOfDay 18.0   // Set to 6 PM
SetTimeOfDay 0.0    // Set to midnight
```

## Common Use Cases

### 1. Specific Time of Day

To start at a specific time:

```cpp
// In BeginPlay or similar
DayNightCycleComponent->SetTimeOfDay(6.0f); // Start at dawn
```

### 2. Pause Time

To pause time progression:

```cpp
DayNightCycleComponent->CycleConfig.bAutoProgress = false;
```

### 3. Speed Up Time

To make time pass faster:

```cpp
DayNightCycleComponent->CycleConfig.TimeSpeed = 10.0f; // 10x speed
```

### 4. Listen for Time Changes

To react to time changes:

```cpp
// Bind to time of day changed event
DayNightCycleComponent->OnTimeOfDayChanged.AddDynamic(
    this, &AMyActor::OnTimeChanged);

void AMyActor::OnTimeChanged(ETimeOfDay NewTime)
{
    if (NewTime == ETimeOfDay::Night)
    {
        // Turn on lights, activate nocturnal creatures, etc.
    }
}
```

### 5. Check Current Time

To query current time:

```cpp
// Get current time in hours (0-24)
float CurrentTime = DayNightCycleComponent->CurrentTimeOfDay;

// Check if daytime
bool bIsDay = DayNightCycleComponent->IsDaytime();

// Check if twilight
bool bIsTwilight = DayNightCycleComponent->IsTwilight();

// Get time category
ETimeOfDay TimeCategory = DayNightCycleComponent->GetTimeOfDayCategory();
```

## Visual Effects

### Star Visibility

Stars automatically fade in/out based on sun position:

```cpp
// Get star visibility (0-1)
float StarVisibility = DayNightCycleComponent->StarVisibility;

// Use in material or particle system
// 0.0 = invisible (daytime)
// 1.0 = fully visible (night)
```

### Twilight Effects

Twilight effects are strongest at sunrise/sunset:

```cpp
// Get twilight intensity (0-1)
float TwilightIntensity = DayNightCycleComponent->TwilightIntensity;

// Get twilight color
FLinearColor TwilightColor = DayNightCycleComponent->GetTwilightColor();
```

### Sky Colors

Access current sky colors for materials or effects:

```cpp
// Get current colors
FLinearColor ZenithColor = DayNightCycleComponent->GetZenithColor();
FLinearColor HorizonColor = DayNightCycleComponent->GetHorizonColor();
FLinearColor SunColor = DayNightCycleComponent->GetSunColor();
float SunIntensity = DayNightCycleComponent->GetSunIntensity();
```

## Integration with Other Systems

### Atmosphere System

The day-night cycle works with the atmosphere component:

- Atmosphere provides base colors
- Day-night cycle modulates lighting
- Combined effect creates realistic sky

### Weather System

Weather can affect day-night cycle visuals:

- Overcast weather reduces sun intensity
- Rain/fog affects visibility
- Storms darken the sky

### Material System

Materials can react to time of day:

- Emissive materials glow at night
- Wetness increases at dawn (dew)
- Surface properties change with temperature

## Troubleshooting

### Sun Not Moving

**Problem**: Sun stays in one position

**Solutions**:

1. Check that "Auto Progress" is enabled
2. Verify "Time Speed" is greater than 0
3. Ensure Directional Light mobility is set to "Movable"
4. Check that the light is assigned to the component

### Sky Not Changing Color

**Problem**: Sky color stays constant

**Solutions**:

1. Verify Sky Light is assigned
2. Check Sky Light mobility is "Movable"
3. Enable "Real Time Capture" on Sky Light
4. Verify sky color presets are configured

### Stars Always Visible

**Problem**: Stars visible during daytime

**Solutions**:

1. Check sun elevation calculation
2. Verify time of day is progressing
3. Check star visibility calculation in materials

### Abrupt Color Changes

**Problem**: Sky colors change suddenly

**Solutions**:

1. Adjust color preset values for smoother transitions
2. Check elevation thresholds in color interpolation
3. Increase time between color presets

## Performance Tips

1. **Sky Light Recapture**: Happens every frame but is optimized by Unreal
2. **Time Speed**: Higher speeds don't affect performance
3. **Color Calculations**: Very lightweight, no performance impact
4. **Light Updates**: Minimal overhead, safe for VR

## Advanced Configuration

### Custom Rotation Axis

For planets with unusual rotation:

```cpp
// Rotate around different axis
FVector CustomAxis = FVector(1, 0, 0); // Rotate around X axis
DayNightCycleComponent->CycleConfig.RotationAxis = CustomAxis;
```

### Seasonal Variations

Axial tilt creates seasonal effects:

```cpp
// Earth-like seasons
DayNightCycleComponent->CycleConfig.AxialTilt = 23.5f;

// No seasons (upright rotation)
DayNightCycleComponent->CycleConfig.AxialTilt = 0.0f;

// Extreme seasons
DayNightCycleComponent->CycleConfig.AxialTilt = 45.0f;
```

### Multiple Planets

Each planet can have its own day-night cycle:

```cpp
// Planet A: Fast rotation (6 hour days)
PlanetA->DayNightCycleComponent->CycleConfig.DayLengthInSeconds = 360.0f;

// Planet B: Slow rotation (48 hour days)
PlanetB->DayNightCycleComponent->CycleConfig.DayLengthInSeconds = 2880.0f;
```

## Example: Complete Setup

```cpp
void AMyGameMode::SetupDayNightCycle()
{
    // Find planet
    APlanet* Planet = FindPlanetInLevel();
    if (!Planet) return;

    // Configure cycle
    FDayNightCycleConfig Config;
    Config.DayLengthInSeconds = 1200.0f; // 20 minute days
    Config.StartTimeOfDay = 6.0f;        // Start at dawn
    Config.bAutoProgress = true;
    Config.TimeSpeed = 1.0f;
    Config.AxialTilt = 23.5f;

    Planet->DayNightCycleComponent->CycleConfig = Config;

    // Customize colors
    FSkyColorConfig DawnColors;
    DawnColors.ZenithColor = FLinearColor(0.3f, 0.4f, 0.7f);
    DawnColors.HorizonColor = FLinearColor(1.0f, 0.6f, 0.4f);
    DawnColors.SunColor = FLinearColor(1.0f, 0.8f, 0.6f);
    DawnColors.SunIntensity = 5.0f;

    Planet->DayNightCycleComponent->DawnColors = DawnColors;

    // Listen for events
    Planet->DayNightCycleComponent->OnTimeOfDayChanged.AddDynamic(
        this, &AMyGameMode::OnTimeChanged);
}
```

## Next Steps

- Integrate with weather system for dynamic sky conditions
- Add moon phases and position
- Create time-based gameplay mechanics
- Implement circadian rhythms for NPCs
- Add time-lapse photography mode

## Related Documentation

- `AI_VISION_SYSTEM.md` - AI vision integration
- `README_AtmosphericRendering.md` - Atmosphere system
- `README_PlanetSystem.md` - Planet system overview
- `TASK_6.4_IMPLEMENTATION.md` - Technical implementation details
