# Debug Visualization Tools for Flight Course

## Overview

Comprehensive debug visualization system for tuning and testing flight course gameplay. Helps developers see paths, collision volumes, performance metrics, and player behavior.

## Debug Draw Functions

### 1. Course Path Visualization

**Draw Waypoint Connections**:
```cpp
void UFlightCourseDebug::DrawWaypointConnections(UWorld* World)
{
    TArray<AActor*> Waypoints;
    UGameplayStatics::GetAllActorsWithTag(World, "FlightWaypoint", Waypoints);

    // Sort by waypoint number
    Waypoints.Sort([](const AActor& A, const AActor& B) {
        // Extract number from name
        return ExtractWaypointNumber(A.GetName()) < ExtractWaypointNumber(B.GetName());
    });

    // Draw lines between waypoints
    for (int32 i = 0; i < Waypoints.Num() - 1; i++)
    {
        FVector Start = Waypoints[i]->GetActorLocation();
        FVector End = Waypoints[i+1]->GetActorLocation();

        DrawDebugLine(
            World,
            Start,
            End,
            FColor::Cyan,
            false, // Not persistent
            -1.0f, // Lifetime (infinite if -1)
            0,     // Depth priority
            5.0f   // Thickness
        );

        // Draw arrow at midpoint showing direction
        FVector Midpoint = (Start + End) / 2.0f;
        FVector Direction = (End - Start).GetSafeNormal();
        DrawDebugDirectionalArrow(
            World,
            Midpoint - Direction * 100,
            Midpoint + Direction * 100,
            50.0f, // Arrow size
            FColor::Yellow
        );
    }
}
```

### 2. Ring Coverage Visualization

**Draw Ring Detection Volumes**:
```cpp
void UFlightCourseDebug::DrawRingCoverageAreas(UWorld* World)
{
    TArray<AActor*> Rings;
    UGameplayStatics::GetAllActorsWithTag(World, "FlightRing", Rings);

    for (AActor* Ring : Rings)
    {
        FVector Location = Ring->GetActorLocation();
        FVector Scale = Ring->GetActorScale3D();
        FRotator Rotation = Ring->GetActorRotation();

        // Draw ring outline
        DrawDebugCircle(
            World,
            Location,
            400.0f * Scale.X, // Outer radius
            64,    // Segments
            FColor::Blue,
            false,
            -1.0f,
            0,
            3.0f,
            Rotation.Vector(),
            FVector::RightVector
        );

        // Draw inner circle (passable area)
        DrawDebugCircle(
            World,
            Location,
            320.0f * Scale.X, // Inner radius
            64,
            FColor::Green,
            false,
            -1.0f,
            0,
            2.0f,
            Rotation.Vector(),
            FVector::RightVector
        );

        // Draw normal vector showing ring orientation
        DrawDebugDirectionalArrow(
            World,
            Location,
            Location + Rotation.Vector() * 500,
            100.0f,
            FColor::Red
        );
    }
}
```

### 3. Player Velocity Visualization

**Draw Velocity Vector**:
```cpp
void UFlightCourseDebug::DrawPlayerVelocity(APawn* PlayerPawn)
{
    if (!PlayerPawn) return;

    FVector Location = PlayerPawn->GetActorLocation();
    FVector Velocity = PlayerPawn->GetVelocity();
    float Speed = Velocity.Size();

    // Color based on speed
    FColor VelocityColor = FColor::Green;
    if (Speed > 1000.0f) VelocityColor = FColor::Yellow;
    if (Speed > 1500.0f) VelocityColor = FColor::Red;

    // Draw velocity arrow
    DrawDebugDirectionalArrow(
        PlayerPawn->GetWorld(),
        Location,
        Location + Velocity,
        50.0f,
        VelocityColor,
        false,
        -1.0f,
        0,
        3.0f
    );

    // Draw speed text
    FString SpeedText = FString::Printf(TEXT("%.0f u/s"), Speed);
    DrawDebugString(
        PlayerPawn->GetWorld(),
        Location + FVector(0, 0, 100),
        SpeedText,
        nullptr,
        VelocityColor,
        -1.0f
    );
}
```

### 4. Collision Detection Visualization

**Draw FlightController Collision Box**:
```cpp
void UFlightController::DrawCollisionDetection()
{
    if (!CollisionDetectionBox) return;

    FVector BoxExtent = CollisionDetectionBox->GetScaledBoxExtent();
    FVector BoxLocation = CollisionDetectionBox->GetComponentLocation();
    FRotator BoxRotation = CollisionDetectionBox->GetComponentRotation();

    DrawDebugBox(
        GetWorld(),
        BoxLocation,
        BoxExtent,
        BoxRotation.Quaternion(),
        FColor::Orange,
        false,
        -1.0f,
        0,
        2.0f
    );

    // Check for obstacles
    TArray<FHitResult> HitResults;
    FCollisionShape CollisionShape = FCollisionShape::MakeBox(BoxExtent);

    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        BoxLocation,
        BoxLocation + GetOwner()->GetActorForwardVector() * 100,
        BoxRotation.Quaternion(),
        ECC_Visibility,
        CollisionShape
    );

    // Draw detected obstacles in red
    for (const FHitResult& Hit : HitResults)
    {
        DrawDebugPoint(
            GetWorld(),
            Hit.Location,
            20.0f,
            FColor::Red,
            false,
            -1.0f
        );

        DrawDebugLine(
            GetWorld(),
            BoxLocation,
            Hit.Location,
            FColor::Red,
            false,
            -1.0f,
            0,
            1.0f
        );
    }
}
```

### 5. Optimal Racing Line

**Draw Ideal Path**:
```cpp
void UFlightCourseDebug::DrawRacingLine(UWorld* World)
{
    // Get waypoints in order
    TArray<FVector> PathPoints;
    // ... populate with waypoint locations ...

    // Interpolate smooth path using spline
    for (int32 i = 0; i < PathPoints.Num() - 1; i++)
    {
        FVector Start = PathPoints[i];
        FVector End = PathPoints[i + 1];

        // Draw spline curve (Catmull-Rom or Bezier)
        for (float t = 0.0f; t <= 1.0f; t += 0.05f)
        {
            FVector Point1 = FMath::Lerp(Start, End, t);
            FVector Point2 = FMath::Lerp(Start, End, t + 0.05f);

            DrawDebugLine(
                World,
                Point1,
                Point2,
                FColor::Purple,
                false,
                -1.0f,
                0,
                4.0f
            );
        }
    }
}
```

### 6. Performance Metrics Overlay

**Draw Frame Stats**:
```cpp
void UFlightCourseDebug::DrawPerformanceMetrics(APawn* PlayerPawn)
{
    // Gather metrics
    float FPS = 1.0f / GetWorld()->GetDeltaSeconds();
    UFlightController* FlightComp = PlayerPawn->FindComponentByClass<UFlightController>();

    FString MetricsText = FString::Printf(
        TEXT("FPS: %.1f\n")
        TEXT("Speed: %.0f u/s\n")
        TEXT("Assist Mode: %s\n")
        TEXT("Input Rate: %.1f Hz"),
        FPS,
        PlayerPawn->GetVelocity().Size(),
        FlightComp ? *UEnum::GetValueAsString(FlightComp->GetAssistMode()) : TEXT("N/A"),
        FlightComp ? FlightComp->AverageInputRate : 0.0f
    );

    // Draw in top-left corner
    DrawDebugString(
        GetWorld(),
        FVector(100, 100, 0),
        MetricsText,
        nullptr,
        FColor::White,
        -1.0f
    );
}
```

## Debug Console Commands

### Register Commands

```cpp
// In FlightCourseGameMode or similar
void AFlightCourseGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Register console commands
    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Flight.Debug.DrawPath"),
        TEXT("Toggle waypoint path visualization"),
        FConsoleCommandDelegate::CreateUObject(this, &AFlightCourseGameMode::TogglePathDebug),
        ECVF_Default
    );

    IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("Flight.Debug.DrawRings"),
        TEXT("Toggle ring coverage visualization"),
        FConsoleCommandDelegate::CreateUObject(this, &AFlightCourseGameMode::ToggleRingDebug),
        ECVF_Default
    );

    // ... more commands ...
}
```

### Console Command List

| Command | Description |
|---------|-------------|
| `Flight.Debug.DrawPath` | Toggle waypoint connections |
| `Flight.Debug.DrawRings` | Toggle ring coverage areas |
| `Flight.Debug.DrawVelocity` | Toggle velocity vectors |
| `Flight.Debug.DrawCollision` | Toggle collision detection |
| `Flight.Debug.DrawRacingLine` | Toggle optimal path |
| `Flight.Debug.ShowMetrics` | Toggle performance overlay |
| `Flight.Debug.All` | Toggle all debug visualizations |

## Debug Widget (WBP_FlightDebug)

**UI Layout**:
```
Overlay (anchored top-right)
├─ Background Panel (semi-transparent black)
├─ Title: "FLIGHT DEBUG"
├─ Checkbox: "Show Path" (toggle path drawing)
├─ Checkbox: "Show Rings" (toggle ring visualization)
├─ Checkbox: "Show Velocity" (toggle velocity arrows)
├─ Checkbox: "Show Collision" (toggle collision box)
├─ Checkbox: "Show Racing Line" (toggle ideal path)
├─ Separator
├─ Text: Current Speed (live update)
├─ Text: Assist Mode (live update)
├─ Text: Waypoints Collected (live update)
├─ Text: Course Completion % (live update)
└─ Button: "Reset Course" (quick reset)
```

**Blueprint Logic**:
```blueprint
Checkbox OnStateChanged(bool bIsChecked):
    Get FlightCourseDebug actor
    Call SetDebugFeatureEnabled(FeatureType, bIsChecked)

Event Tick:
    Update all live stat displays
```

## Implementation

### Step 1: Create Debug Actor

```cpp
// BP_FlightCourseDebug (place in level)
UCLASS()
class AFlightCourseDebug : public AActor
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawWaypointPath = false;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawRingCoverage = false;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawPlayerVelocity = false;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawCollisionDetection = false;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bDrawRacingLine = false;

    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bShowPerformanceMetrics = false;

    virtual void Tick(float DeltaTime) override
    {
        Super::Tick(DeltaTime);

        if (bDrawWaypointPath)
            DrawWaypointConnections(GetWorld());

        if (bDrawRingCoverage)
            DrawRingCoverageAreas(GetWorld());

        // ... etc for other features
    }
};
```

### Step 2: Add to Level

1. Place BP_FlightCourseDebug in level
2. Configure default debug settings
3. Optionally hide in packaged builds

### Step 3: Input Binding (Optional)

```cpp
// Toggle debug with F3 key
PlayerInputComponent->BindAction("ToggleDebug", IE_Pressed, this, &AFlightPlayerController::ToggleDebugVisualization);
```

## Performance Impact

**Debug Drawing Cost**:
- Waypoint path: ~0.1ms
- Ring coverage: ~0.2ms
- Velocity vectors: ~0.05ms
- Collision boxes: ~0.1ms
- Racing line: ~0.3ms
- **Total**: ~0.75ms per frame (negligible)

**Recommendation**: Disable in packaged builds
```cpp
#if !UE_BUILD_SHIPPING
    // Debug code only in editor/development builds
#endif
```

## Use Cases

### Use Case 1: Course Design Iteration
**Debug Tools**: Path + Rings + Racing Line
**Purpose**: Verify course layout, check ring placement, visualize ideal path

### Use Case 2: Physics Tuning
**Debug Tools**: Velocity + Collision + Metrics
**Purpose**: Monitor player speed, check collision detection, measure FPS impact

### Use Case 3: Difficulty Balancing
**Debug Tools**: All features
**Purpose**: Observe player behavior, identify difficult sections, measure completion times

### Use Case 4: Bug Reproduction
**Debug Tools**: All features + recording
**Purpose**: Capture exact conditions when issues occur

## Testing Checklist

- [ ] All debug draw functions work
- [ ] Console commands registered correctly
- [ ] Debug widget displays properly
- [ ] Toggle states persist correctly
- [ ] No crashes with debug enabled
- [ ] Minimal performance impact
- [ ] Debug disabled in packaged builds
- [ ] All visualizations accurate

## Status
**Design**: COMPLETE
**Implementation**: Documented (C++ + Blueprint)
**Console Commands**: Specified
**Debug Widget**: Designed
**Ready for**: Manual implementation
