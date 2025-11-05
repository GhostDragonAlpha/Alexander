# Quick Start: Navigation Markers

## Overview

The Navigation Marker system provides visible markers for navigation that scale and fade based on viewer distance and altitude. Markers are visible from orbital, atmospheric, and surface views with automatic scaling and filtering.

## Basic Usage

### Adding a Marker to an Actor (Blueprint)

1. **Select Actor** in the level
2. **Add Component** → Search for "Navigation Marker Component"
3. **Configure Properties**:
   ```
   Marker Name: "My Waypoint"
   Marker Type: Waypoint
   Visibility Mode: Always
   Marker Color: Blue
   Min Visibility Distance: 10000 (100m)
   Max Visibility Distance: 10000000 (100km)
   ```

### Adding a Marker in C++

```cpp
#include "NavigationMarkerComponent.h"
#include "NavigationMarkerSubsystem.h"

// In constructor
UNavigationMarkerComponent* Marker = CreateDefaultSubobject<UNavigationMarkerComponent>(TEXT("NavMarker"));
Marker->SetupAttachment(RootComponent);
Marker->MarkerName = TEXT("Resource Node");
Marker->MarkerType = EMarkerType::Resource;
Marker->VisibilityMode = EMarkerVisibility::Surface;
Marker->MarkerColor = FLinearColor::Yellow;

// In BeginPlay
UNavigationMarkerSubsystem* MarkerSubsystem = GetWorld()->GetSubsystem<UNavigationMarkerSubsystem>();
if (MarkerSubsystem)
{
    MarkerSubsystem->RegisterMarker(Marker);
}
```

## Marker Types

- **Landing Pad**: Designated landing zones
- **Point of Interest**: Notable locations
- **Waypoint**: Navigation points
- **Hazard**: Dangerous areas
- **Resource**: Resource nodes
- **Settlement**: Populated areas
- **Custom**: User-defined markers

## Visibility Modes

### Orbital (>100km altitude)

- Visible only from space
- 3x scale multiplier
- Best for planet-wide landmarks

### Atmospheric (10-100km altitude)

- Visible during atmospheric flight
- 1.5x scale multiplier
- Best for approach guidance

### Surface (<10km altitude)

- Visible near ground
- 1x scale multiplier
- Best for local navigation

### Always

- Visible at all altitudes (within distance range)
- Scale adapts to altitude
- Best for general navigation

## Using the Marker Subsystem

### Finding Nearest Marker

```cpp
UNavigationMarkerSubsystem* MarkerSubsystem = GetWorld()->GetSubsystem<UNavigationMarkerSubsystem>();

// Find nearest landing pad
UNavigationMarkerComponent* NearestPad = MarkerSubsystem->FindNearestMarker(
    PlayerLocation,
    EMarkerType::LandingPad
);

if (NearestPad)
{
    FVector PadLocation = NearestPad->GetComponentLocation();
    NavigateTo(PadLocation);
}
```

### Filtering Markers

```cpp
// Hide resource markers
MarkerSubsystem->SetMarkerTypeEnabled(EMarkerType::Resource, false);

// Show only landing pads
FMarkerFilterSettings FilterSettings;
FilterSettings.bShowLandingPads = true;
FilterSettings.bShowWaypoints = false;
FilterSettings.bShowPointsOfInterest = false;
FilterSettings.bShowHazards = false;
FilterSettings.bShowResources = false;
FilterSettings.bShowSettlements = false;
FilterSettings.MaxVisibleMarkers = 20;

MarkerSubsystem->SetFilterSettings(FilterSettings);
```

### Getting Markers in Range

```cpp
FVector SearchLocation = GetActorLocation();
float SearchRadius = 10000.0f * 100.0f; // 10km in cm

TArray<UNavigationMarkerComponent*> NearbyMarkers = MarkerSubsystem->GetMarkersInRange(
    SearchLocation,
    SearchRadius
);

for (UNavigationMarkerComponent* Marker : NearbyMarkers)
{
    UE_LOG(LogTemp, Log, TEXT("Found: %s"), *Marker->MarkerName);
}
```

## Landing Pad Integration

Landing pads automatically create navigation markers:

```cpp
// Landing pads have built-in markers
ALandingPad* LandingPad = SpawnLandingPad();

// Marker is automatically created and registered
// Color updates with pad status:
// - Green: Available
// - Yellow: Reserved
// - Red: Occupied
// - Gray: Disabled
```

## Configuration Tips

### Orbital Markers (Space Stations, Planets)

```
Visibility Mode: Orbital
Min Distance: 100km
Max Distance: 1000km
Orbital Scale Multiplier: 5.0
```

### Atmospheric Markers (Landing Zones)

```
Visibility Mode: Atmospheric
Min Distance: 1km
Max Distance: 100km
Atmospheric Scale Multiplier: 2.0
```

### Surface Markers (Resources, POIs)

```
Visibility Mode: Surface
Min Distance: 100m
Max Distance: 10km
Base Marker Size: 100cm
```

### Always Visible (Waypoints)

```
Visibility Mode: Always
Min Distance: 100m
Max Distance: 50km
Reference Distance: 10km
```

## Debug Visualization

Enable debug visualization to see marker information:

```cpp
Marker->bShowDebugVisualization = true;
```

Shows:

- Colored sphere at marker location
- Marker name
- Distance to viewer
- Viewer altitude
- Current scale factor
- Current opacity

## Common Patterns

### Creating a Waypoint System

```cpp
void ANavigationController::AddWaypoint(FVector Location, FString Name)
{
    AActor* WaypointActor = GetWorld()->SpawnActor<AActor>(Location, FRotator::ZeroRotator);

    UNavigationMarkerComponent* Marker = NewObject<UNavigationMarkerComponent>(WaypointActor);
    Marker->RegisterComponent();
    Marker->AttachToComponent(WaypointActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

    Marker->MarkerName = Name;
    Marker->MarkerType = EMarkerType::Waypoint;
    Marker->VisibilityMode = EMarkerVisibility::Always;
    Marker->MarkerColor = FLinearColor::Blue;

    UNavigationMarkerSubsystem* MarkerSubsystem = GetWorld()->GetSubsystem<UNavigationMarkerSubsystem>();
    MarkerSubsystem->RegisterMarker(Marker);
}
```

### Highlighting Nearest Objective

```cpp
void APlayerController::HighlightNearestObjective()
{
    UNavigationMarkerSubsystem* MarkerSubsystem = GetWorld()->GetSubsystem<UNavigationMarkerSubsystem>();

    FVector PlayerLocation = GetPawn()->GetActorLocation();
    UNavigationMarkerComponent* NearestMarker = MarkerSubsystem->FindNearestMarker(
        PlayerLocation,
        EMarkerType::Custom // Find any type
    );

    if (NearestMarker)
    {
        // Highlight by changing color
        NearestMarker->SetMarkerColor(FLinearColor::White);

        // Show distance
        float Distance = NearestMarker->GetDistanceToViewer() / 100.0f;
        ShowMessage(FString::Printf(TEXT("Objective: %s (%.0fm)"), *NearestMarker->MarkerName, Distance));
    }
}
```

### Dynamic Marker Updates

```cpp
void AResourceNode::UpdateMarkerBasedOnResources()
{
    if (NavigationMarker)
    {
        if (ResourceAmount < 100.0f)
        {
            NavigationMarker->SetMarkerColor(FLinearColor::Red); // Low
        }
        else if (ResourceAmount < 500.0f)
        {
            NavigationMarker->SetMarkerColor(FLinearColor::Yellow); // Medium
        }
        else
        {
            NavigationMarker->SetMarkerColor(FLinearColor::Green); // High
        }
    }
}
```

## Performance Considerations

- **Marker Limit**: Set `MaxVisibleMarkers` to prevent too many markers
- **Distance Prioritization**: Enable `bPrioritizeByDistance` to show closest markers first
- **Visibility Modes**: Use specific modes (Orbital/Atmospheric/Surface) instead of Always when possible
- **Update Frequency**: Markers update every frame but checks are fast (<0.1ms per marker)

## Troubleshooting

### Markers Not Visible

1. Check `bIsEnabled` is true
2. Verify viewer is within `MinVisibilityDistance` and `MaxVisibilityDistance`
3. Check `VisibilityMode` matches current altitude
4. Verify marker type is not filtered out in subsystem

### Markers Too Small/Large

1. Adjust `BaseMarkerSize`
2. Modify `MinScaleFactor` and `MaxScaleFactor`
3. Change `ReferenceDistance`
4. Adjust altitude multipliers

### Markers Flickering

1. Increase fade distances
2. Smooth altitude transitions
3. Check for rapid enable/disable calls

## Best Practices

1. **Use Appropriate Types**: Choose marker types that match their purpose
2. **Set Visibility Modes**: Use specific modes for better performance
3. **Configure Distance Ranges**: Set appropriate min/max distances for each marker
4. **Use Filtering**: Enable filtering to prevent marker clutter
5. **Test at All Altitudes**: Verify markers work from orbital, atmospheric, and surface views
6. **Color Coding**: Use consistent colors for marker types
7. **Limit Marker Count**: Set reasonable `MaxVisibleMarkers` limit

## Example: Complete Landing Pad Marker Setup

```cpp
// Landing pad automatically creates marker in constructor
ALandingPad::ALandingPad()
{
    // ... other components ...

    NavigationMarker = CreateDefaultSubobject<UNavigationMarkerComponent>(TEXT("NavigationMarker"));
    NavigationMarker->SetupAttachment(RootComponent);
    NavigationMarker->SetRelativeLocation(FVector(0, 0, 500.0f)); // 5m above pad

    // Configure for landing pad
    NavigationMarker->MarkerType = EMarkerType::LandingPad;
    NavigationMarker->VisibilityMode = EMarkerVisibility::Always;
    NavigationMarker->MarkerColor = FLinearColor::Green;

    // Visibility range: 100m to 100km
    NavigationMarker->MinVisibilityDistance = 100.0f * 100.0f;
    NavigationMarker->MaxVisibilityDistance = 100000.0f * 100.0f;

    // Altitude thresholds
    NavigationMarker->OrbitalAltitudeThreshold = 100000.0f * 100.0f; // 100km
    NavigationMarker->AtmosphericAltitudeRange = FVector2D(10000.0f * 100.0f, 100000.0f * 100.0f); // 10-100km
}

// Register in BeginPlay
void ALandingPad::BeginPlay()
{
    Super::BeginPlay();

    if (NavigationMarker)
    {
        NavigationMarker->MarkerName = PadName;

        UNavigationMarkerSubsystem* MarkerSubsystem = GetWorld()->GetSubsystem<UNavigationMarkerSubsystem>();
        if (MarkerSubsystem)
        {
            MarkerSubsystem->RegisterMarker(NavigationMarker);
        }
    }
}

// Update color based on status
void ALandingPad::UpdateVisuals()
{
    FLinearColor StatusColor = GetStatusColor();

    if (NavigationMarker)
    {
        NavigationMarker->SetMarkerColor(StatusColor);
    }
}
```

## Summary

The Navigation Marker system provides:

- ✅ Orbital visibility markers (>100km)
- ✅ Atmospheric markers (10-100km)
- ✅ Distance-based scaling (0.5x to 5.0x)
- ✅ Marker filtering by type
- ✅ Automatic landing pad integration
- ✅ Subsystem for centralized management

For more details, see `TASK_11.4_IMPLEMENTATION.md`.
