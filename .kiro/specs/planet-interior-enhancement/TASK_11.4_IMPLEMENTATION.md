# Task 11.4 Implementation: Navigation Markers

## Overview

This document describes the implementation of the navigation marker system for the planet interior enhancement feature. The system provides orbital and atmospheric visibility markers with distance-based scaling and comprehensive filtering capabilities.

## Implementation Details

### Components Created

1. **NavigationMarkerComponent.h** - Header file defining the UNavigationMarkerComponent class
2. **NavigationMarkerComponent.cpp** - Implementation file with marker visibility and scaling logic
3. **NavigationMarkerSubsystem.h** - Header file defining the UNavigationMarkerSubsystem class
4. **NavigationMarkerSubsystem.cpp** - Implementation file with marker management and filtering
5. **LandingPad.h** - Updated to include navigation marker component
6. **LandingPad.cpp** - Updated to create and configure navigation markers

### Key Features Implemented

#### 1. Navigation Marker Component

**Purpose**: Provides visible navigation markers that scale and fade based on viewer distance and altitude.

**Marker Visibility Modes**:

- **Orbital**: Only visible above 100km altitude
- **Atmospheric**: Only visible between 10-100km altitude
- **Surface**: Only visible below 10km altitude
- **Always**: Visible at all altitudes (within distance range)

**Marker Types** (for filtering):

- Landing Pad
- Point of Interest
- Waypoint
- Hazard
- Resource
- Settlement
- Custom

**Key Properties**:

```cpp
// Identity
FString MarkerName;
EMarkerType MarkerType;
FLinearColor MarkerColor;
UTexture2D* MarkerIcon;

// Visibility
EMarkerVisibility VisibilityMode;
bool bIsEnabled;
float MinVisibilityDistance; // Default: 100m
float MaxVisibilityDistance; // Default: 100km

// Distance-Based Scaling
float BaseMarkerSize; // Default: 1m
float ReferenceDistance; // Default: 10km
float MinScaleFactor; // Default: 0.5
float MaxScaleFactor; // Default: 5.0

// Orbital Settings
float OrbitalAltitudeThreshold; // Default: 100km
float OrbitalScaleMultiplier; // Default: 3.0

// Atmospheric Settings
FVector2D AtmosphericAltitudeRange; // Default: 10-100km
float AtmosphericScaleMultiplier; // Default: 1.5
float AtmosphericFadeDistance; // Default: 5km
```

**Key Methods**:

```cpp
// Update marker based on viewer location and altitude
void UpdateMarker(const FVector& ViewerLocation, float ViewerAltitude);

// Check if marker should be visible
bool ShouldBeVisible(float ViewerAltitude) const;

// Calculate marker scale based on distance and altitude
float CalculateMarkerScale(float Distance, float ViewerAltitude) const;

// Calculate marker opacity for fading
float CalculateMarkerOpacity(float Distance, float ViewerAltitude) const;

// Enable/disable marker
void SetMarkerEnabled(bool bEnabled);

// Set marker color
void SetMarkerColor(const FLinearColor& NewColor);
```

**Events**:

- `OnMarkerBecameVisible` - Fired when marker becomes visible
- `OnMarkerBecameHidden` - Fired when marker becomes hidden

#### 2. Distance-Based Scaling System

The marker scale adapts based on distance and altitude:

**Distance Scaling**:

- Markers scale up when farther than reference distance
- Markers scale down when closer than reference distance
- Minimum scale factor prevents markers from becoming too small
- Maximum scale factor prevents markers from becoming too large

**Altitude Multipliers**:

- **Orbital View (>100km)**: 3x scale multiplier for visibility from space
- **Atmospheric View (10-100km)**: 1.5x scale multiplier for mid-range visibility
- **Surface View (<10km)**: 1x scale multiplier for normal size

**Scaling Formula**:

```cpp
float DistanceScale = Distance / ReferenceDistance;
float AltitudeMultiplier = GetAltitudeMultiplier(ViewerAltitude);
float FinalScale = Clamp(DistanceScale * AltitudeMultiplier, MinScale, MaxScale);
```

#### 3. Opacity Fading System

Markers fade in and out smoothly at visibility boundaries:

**Distance Fading**:

- Fade out at maximum visibility distance (5km fade zone)
- Fade in at minimum visibility distance (1km fade zone)

**Altitude Fading** (for Atmospheric markers):

- Fade at lower boundary (2km fade zone)
- Fade at upper boundary (10km fade zone)

**Opacity Formula**:

```cpp
float Opacity = 1.0f;

// Distance fade
if (Distance > MaxDistance - FadeDistance)
    Opacity *= 1.0 - ((Distance - (MaxDistance - FadeDistance)) / FadeDistance);

// Altitude fade (for atmospheric markers)
if (Altitude < LowerBoundary + FadeZone)
    Opacity *= (Altitude - LowerBoundary) / FadeZone;
```

#### 4. Navigation Marker Subsystem

**Purpose**: World subsystem for managing all navigation markers, providing filtering and batch operations.

**Marker Registration**:

```cpp
void RegisterMarker(UNavigationMarkerComponent* Marker);
void UnregisterMarker(UNavigationMarkerComponent* Marker);
```

**Marker Queries**:

```cpp
// Get all markers
TArray<UNavigationMarkerComponent*> GetAllMarkers() const;

// Get markers by type
TArray<UNavigationMarkerComponent*> GetMarkersByType(EMarkerType Type) const;

// Get markers in range
TArray<UNavigationMarkerComponent*> GetMarkersInRange(const FVector& Location, float MaxDistance) const;

// Find nearest marker
UNavigationMarkerComponent* FindNearestMarker(const FVector& Location, EMarkerType Type) const;
```

**Filtering System**:

```cpp
struct FMarkerFilterSettings
{
    bool bShowLandingPads = true;
    bool bShowPointsOfInterest = true;
    bool bShowWaypoints = true;
    bool bShowHazards = true;
    bool bShowResources = true;
    bool bShowSettlements = true;
    bool bShowCustom = true;
    int32 MaxVisibleMarkers = 50;
    bool bPrioritizeByDistance = true;
};

// Apply filter settings
void SetFilterSettings(const FMarkerFilterSettings& NewSettings);
void SetMarkerTypeEnabled(EMarkerType Type, bool bEnabled);
void ApplyFilters();
```

**Batch Operations**:

```cpp
// Enable/disable all markers
void SetAllMarkersEnabled(bool bEnabled);

// Set color for all markers of a type
void SetMarkerTypeColor(EMarkerType Type, const FLinearColor& Color);
```

**Statistics**:

```cpp
int32 GetMarkerCount() const;
int32 GetVisibleMarkerCount() const;
int32 GetMarkerCountByType(EMarkerType Type) const;
```

#### 5. Landing Pad Integration

Landing pads now automatically create and configure navigation markers:

**Automatic Marker Creation**:

- Navigation marker component added to landing pad
- Positioned 5m above pad center
- Configured as LandingPad type
- Always visible mode (within distance range)
- Visibility range: 100m to 100km

**Status-Based Coloring**:

- Green: Available
- Yellow: Reserved
- Red: Occupied
- Gray: Disabled

**Automatic Registration**:

- Marker registered with subsystem on BeginPlay
- Marker name set to pad name
- Color updates automatically with pad status

## Usage Guide

### Creating Navigation Markers in C++

```cpp
// Add marker component to actor
UNavigationMarkerComponent* Marker = CreateDefaultSubobject<UNavigationMarkerComponent>(TEXT("NavMarker"));
Marker->SetupAttachment(RootComponent);

// Configure marker
Marker->MarkerName = TEXT("My Waypoint");
Marker->MarkerType = EMarkerType::Waypoint;
Marker->VisibilityMode = EMarkerVisibility::Always;
Marker->MarkerColor = FLinearColor::Blue;
Marker->MinVisibilityDistance = 500.0f * 100.0f; // 500m
Marker->MaxVisibilityDistance = 50000.0f * 100.0f; // 50km

// Register with subsystem
UNavigationMarkerSubsystem* MarkerSubsystem = GetWorld()->GetSubsystem<UNavigationMarkerSubsystem>();
if (MarkerSubsystem)
{
    MarkerSubsystem->RegisterMarker(Marker);
}
```

### Creating Navigation Markers in Blueprint

1. **Add Component**:

   - Select actor in editor
   - Add Component → Navigation Marker Component
   - Position marker at desired location

2. **Configure Properties**:

   ```
   Marker Name: "Resource Node Alpha"
   Marker Type: Resource
   Visibility Mode: Surface
   Marker Color: Yellow
   Min Visibility Distance: 10000 (100m)
   Max Visibility Distance: 1000000 (10km)
   ```

3. **Marker automatically registers on BeginPlay**

### Using the Marker Subsystem

#### Finding Nearest Landing Pad

```cpp
UNavigationMarkerSubsystem* MarkerSubsystem = GetWorld()->GetSubsystem<UNavigationMarkerSubsystem>();
if (MarkerSubsystem)
{
    FVector ShipLocation = GetActorLocation();
    UNavigationMarkerComponent* NearestPad = MarkerSubsystem->FindNearestMarker(
        ShipLocation,
        EMarkerType::LandingPad
    );

    if (NearestPad)
    {
        FVector PadLocation = NearestPad->GetComponentLocation();
        NavigateTo(PadLocation);
    }
}
```

#### Filtering Markers

```cpp
// Hide all resource markers
MarkerSubsystem->SetMarkerTypeEnabled(EMarkerType::Resource, false);

// Show only landing pads and waypoints
FMarkerFilterSettings FilterSettings;
FilterSettings.bShowLandingPads = true;
FilterSettings.bShowWaypoints = true;
FilterSettings.bShowPointsOfInterest = false;
FilterSettings.bShowHazards = false;
FilterSettings.bShowResources = false;
FilterSettings.bShowSettlements = false;
FilterSettings.MaxVisibleMarkers = 20;
FilterSettings.bPrioritizeByDistance = true;

MarkerSubsystem->SetFilterSettings(FilterSettings);
```

#### Getting Markers in Range

```cpp
FVector SearchLocation = GetActorLocation();
float SearchRadius = 10000.0f * 100.0f; // 10km

TArray<UNavigationMarkerComponent*> NearbyMarkers = MarkerSubsystem->GetMarkersInRange(
    SearchLocation,
    SearchRadius
);

for (UNavigationMarkerComponent* Marker : NearbyMarkers)
{
    UE_LOG(LogTemp, Log, TEXT("Found marker: %s at distance %.0fm"),
        *Marker->MarkerName,
        Marker->GetDistanceToViewer() / 100.0f);
}
```

### Responding to Marker Events

```cpp
// Bind to marker visibility events
Marker->OnMarkerBecameVisible.AddDynamic(this, &AMyActor::OnMarkerVisible);
Marker->OnMarkerBecameHidden.AddDynamic(this, &AMyActor::OnMarkerHidden);

void AMyActor::OnMarkerVisible()
{
    UE_LOG(LogTemp, Log, TEXT("Marker became visible"));
    // Play notification sound, show UI element, etc.
}

void AMyActor::OnMarkerHidden()
{
    UE_LOG(LogTemp, Log, TEXT("Marker became hidden"));
    // Hide UI element, etc.
}
```

### Creating Custom Marker Types

For specialized markers, extend the component:

```cpp
UCLASS()
class UResourceMarkerComponent : public UNavigationMarkerComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Resource")
    EResourceType ResourceType;

    UPROPERTY(EditAnywhere, Category = "Resource")
    float ResourceAmount;

    virtual void UpdateMarkerVisuals() override
    {
        Super::UpdateMarkerVisuals();

        // Custom visual updates based on resource amount
        if (ResourceAmount < 100.0f)
        {
            SetMarkerColor(FLinearColor::Red); // Low resources
        }
        else if (ResourceAmount < 500.0f)
        {
            SetMarkerColor(FLinearColor::Yellow); // Medium resources
        }
        else
        {
            SetMarkerColor(FLinearColor::Green); // High resources
        }
    }
};
```

## Requirements Satisfied

### Requirement 9.4: Navigation Markers

✅ **"WHERE landing zones exist, THE Planet System SHALL provide navigation markers visible from orbit and atmosphere"**

- **Orbital Visibility**: Markers visible from >100km altitude with 3x scale multiplier
- **Atmospheric Visibility**: Markers visible in 10-100km range with 1.5x scale multiplier
- **Distance-Based Scaling**: Markers scale from 0.5x to 5.0x based on distance
- **Marker Filtering**: Comprehensive filtering system by type with max visible limit

## Technical Implementation Details

### Altitude Calculation

The system calculates altitude above planet surface:

```cpp
float CalculateAltitudeAboveSurface(const FVector& Location) const
{
    // Trace down to find ground
    FVector TraceStart = Location;
    FVector TraceEnd = Location - FVector(0, 0, 1000000.0f); // 10km down

    FHitResult HitResult;
    if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
    {
        return Location.Z - HitResult.Location.Z;
    }

    // No ground found - use absolute Z (for space)
    return Location.Z;
}
```

### Viewer Information

Markers automatically get viewer location from player controller:

```cpp
bool GetViewerInfo(FVector& OutLocation, float& OutAltitude) const
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return false;

    FVector CameraLocation;
    FRotator CameraRotation;
    PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

    OutLocation = CameraLocation;
    OutAltitude = CalculateAltitudeAboveSurface(CameraLocation);

    return true;
}
```

### Performance Optimization

**Tick Optimization**:

- Markers tick in `TG_PostPhysics` group
- Only update when enabled
- Visibility checks are fast (distance and altitude comparisons)

**Subsystem Optimization**:

- Marker limit prevents excessive visible markers
- Distance-based prioritization when limiting
- Efficient marker queries using TArray operations

**Performance Targets**:

- Marker update: < 0.1ms per marker
- Subsystem filtering: < 1ms for 100 markers
- Total overhead: < 5ms for 50 visible markers

### Debug Visualization

Enable debug visualization to see markers:

```cpp
Marker->bShowDebugVisualization = true;
```

**Debug Display**:

- Colored sphere at marker location (scaled)
- Text showing:
  - Marker name
  - Distance to viewer
  - Viewer altitude
  - Current scale factor
  - Current opacity

## Integration with Other Systems

### Landing Guidance UI Integration

The landing guidance UI can use markers to find target pads:

```cpp
// Find nearest landing pad marker
UNavigationMarkerSubsystem* MarkerSubsystem = GetWorld()->GetSubsystem<UNavigationMarkerSubsystem>();
UNavigationMarkerComponent* NearestPadMarker = MarkerSubsystem->FindNearestMarker(
    PlayerLocation,
    EMarkerType::LandingPad
);

if (NearestPadMarker)
{
    // Get the landing pad actor
    ALandingPad* LandingPad = Cast<ALandingPad>(NearestPadMarker->GetOwner());
    if (LandingPad)
    {
        LandingGuidanceWidget->SetTargetPad(LandingPad);
    }
}
```

### HUD Integration

Markers can be rendered in HUD/UI:

```cpp
// In HUD draw function
UNavigationMarkerSubsystem* MarkerSubsystem = GetWorld()->GetSubsystem<UNavigationMarkerSubsystem>();
TArray<UNavigationMarkerComponent*> VisibleMarkers = MarkerSubsystem->GetAllMarkers();

for (UNavigationMarkerComponent* Marker : VisibleMarkers)
{
    if (!Marker->bIsEnabled) continue;

    FVector MarkerLocation = Marker->GetComponentLocation();
    FVector2D ScreenPosition;

    if (ProjectWorldToScreen(MarkerLocation, ScreenPosition))
    {
        // Draw marker icon at screen position
        DrawMarkerIcon(ScreenPosition, Marker->MarkerIcon, Marker->MarkerColor, Marker->CurrentScale);

        // Draw marker name
        DrawText(Marker->MarkerName, ScreenPosition + FVector2D(0, 20), Marker->MarkerColor);

        // Draw distance
        float Distance = Marker->GetDistanceToViewer() / 100.0f; // Convert to meters
        FString DistanceText = FString::Printf(TEXT("%.0fm"), Distance);
        DrawText(DistanceText, ScreenPosition + FVector2D(0, 35), FLinearColor::White);
    }
}
```

### VR Integration

For VR, markers can be rendered as 3D widgets:

```cpp
// Create 3D widget for marker
UWidgetComponent* MarkerWidget = NewObject<UWidgetComponent>(Marker->GetOwner());
MarkerWidget->SetupAttachment(Marker);
MarkerWidget->SetWidgetClass(MarkerWidgetClass);
MarkerWidget->SetDrawSize(FVector2D(200, 100));
MarkerWidget->SetPivot(FVector2D(0.5f, 0.5f));

// Update widget based on marker state
MarkerWidget->SetVisibility(Marker->bIsEnabled);
MarkerWidget->SetWorldScale3D(FVector(Marker->CurrentScale));
```

## Testing Recommendations

### Unit Tests

1. **Visibility Calculation**:

   - Test orbital visibility (>100km)
   - Test atmospheric visibility (10-100km)
   - Test surface visibility (<10km)
   - Test always visible mode

2. **Scale Calculation**:

   - Test distance-based scaling
   - Test altitude multipliers
   - Test min/max clamping

3. **Opacity Calculation**:
   - Test distance fading
   - Test altitude fading
   - Test boundary transitions

### Integration Tests

1. **Subsystem Registration**:

   - Test marker registration
   - Test marker unregistration
   - Test duplicate registration handling

2. **Filtering System**:

   - Test type filtering
   - Test marker count limiting
   - Test distance prioritization

3. **Landing Pad Integration**:
   - Test automatic marker creation
   - Test status color updates
   - Test marker registration

### Visual Tests

1. **Orbital View**:

   - Verify markers visible from >100km
   - Verify 3x scale multiplier
   - Verify appropriate marker count

2. **Atmospheric View**:

   - Verify markers visible in 10-100km range
   - Verify 1.5x scale multiplier
   - Verify smooth fading at boundaries

3. **Surface View**:
   - Verify markers visible <10km
   - Verify normal scale
   - Verify distance-based scaling

### Performance Tests

1. **Marker Count**:

   - Test with 10 markers
   - Test with 50 markers
   - Test with 100 markers
   - Verify frame rate impact

2. **Update Frequency**:
   - Measure marker update time
   - Measure subsystem filtering time
   - Verify total overhead <5ms

## Future Enhancements

Potential improvements for future iterations:

1. **Advanced Filtering**:

   - Filter by distance range
   - Filter by marker name/tag
   - Custom filter predicates
   - Saved filter presets

2. **Marker Clustering**:

   - Group nearby markers
   - Show cluster count
   - Expand on approach

3. **Directional Indicators**:

   - Off-screen marker indicators
   - Distance and direction arrows
   - Compass integration

4. **Dynamic Icons**:

   - Animated marker icons
   - Status-based icon changes
   - Custom icon rendering

5. **Audio Integration**:

   - Proximity beeps
   - Marker identification audio
   - Directional audio cues

6. **Multiplayer Support**:
   - Player-placed markers
   - Shared waypoints
   - Team-specific markers

## Conclusion

Task 11.4 is complete with a comprehensive navigation marker system that provides:

- **Orbital Visibility Markers**: Visible from space with appropriate scaling
- **Atmospheric Markers**: Visible during atmospheric flight with smooth transitions
- **Distance-Based Scaling**: Markers scale appropriately based on distance and altitude
- **Marker Filtering System**: Comprehensive filtering by type with count limiting
- **Landing Pad Integration**: Automatic marker creation for all landing pads
- **Subsystem Management**: Centralized marker registration and batch operations

The implementation satisfies requirement 9.4 from the planet interior enhancement specification and provides a professional-quality navigation system comparable to Star Citizen's marker system.

## Quick Start Guide

### For Designers

1. Add NavigationMarkerComponent to any actor
2. Configure marker type, visibility mode, and colors
3. Marker automatically registers and updates
4. Use subsystem to filter and manage markers

### For Programmers

1. Include "NavigationMarkerComponent.h" and "NavigationMarkerSubsystem.h"
2. Create marker component with CreateDefaultSubobject<>()
3. Configure marker properties
4. Register with subsystem using RegisterMarker()
5. Query markers using subsystem methods

### For Level Designers

1. Landing pads automatically have markers
2. Add markers to points of interest
3. Configure visibility ranges for each marker
4. Test from orbital, atmospheric, and surface views
5. Adjust marker limits in subsystem settings
