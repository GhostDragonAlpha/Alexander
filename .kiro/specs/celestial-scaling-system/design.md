# Celestial Scaling System - Design Document

## Overview

The Celestial Scaling System implements a player-centric universe architecture where the VR player remains at world origin while celestial bodies dynamically scale and reposition around them. This design is proven in Chimera-VR and solves the fundamental VR precision problem in space simulation.

### Core Design Principles

1. **Player at Origin**: VR player never moves from (0,0,0) in world space
2. **Universe Moves**: All celestial bodies translate relative to player movement
3. **Synchronized Scaling**: Visual scale and gravitational force use identical distance calculations
4. **Performance First**: Optimized for 90+ FPS in VR with 50+ celestial bodies
5. **Unreal Engine Integration**: Leverages UE5's actor component system and physics engine

## Architecture

### System Components

```
CelestialScalingSystem (Game Mode Component)
├── PlayerOriginManager (Manages player position and universe translation)
├── CelestialBodyRegistry (Tracks all scalable celestial bodies)
├── ScalingCalculator (Computes scale factors and transformations)
├── GravitySimulator (Calculates and applies gravitational forces)
└── AstronomicalConstants (Static data class with physical constants)

CelestialBodyComponent (Actor Component)
├── ScaleData (Current scale factor, base size, distance)
├── PhysicalProperties (Mass, radius, gravitational influence)
└── PerformanceSettings (LOD thresholds, update frequency)
```

### Data Flow

```
Player Movement Input
    ↓
PlayerOriginManager.TranslateUniverse()
    ↓
CelestialBodyRegistry.UpdateAllPositions()
    ↓
ScalingCalculator.ComputeScaleFactors()
    ↓
├→ CelestialBodyComponent.ApplyScale() → Visual Update
└→ GravitySimulator.CalculateForces() → Physics Update
```

## Components and Interfaces

### 1. AstronomicalConstants (Static Class)

**Purpose**: Centralized repository for all physical constants and celestial body data.

**Implementation**: C++ static class with Blueprint-accessible functions

```cpp
class ALEXANDER_API UAstronomicalConstants : public UBlueprintFunctionLibrary
{
public:
    // Fundamental Constants
    static constexpr double GRAVITATIONAL_CONSTANT = 6.67430e-11; // m³/kg/s²
    static constexpr double SPEED_OF_LIGHT = 299792458.0; // m/s
    static constexpr double AU_TO_METERS = 149597870700.0; // 1 AU in meters

    // Scale System Constants
    static constexpr double MIN_SCALE_FACTOR = 0.0001;
    static constexpr double MAX_SCALE_FACTOR = 10000.0;
    static constexpr double SCALE_SMOOTHING_RATE = 0.95; // Per frame
    static constexpr double VR_PRECISION_RADIUS = 1000.0; // meters

    // Celestial Body Data
    struct FCelestialBodyData
    {
        FString Name;
        double Mass; // kg
        double Radius; // meters
        double SemiMajorAxis; // meters
        double Eccentricity;
        double Inclination; // radians
        FLinearColor DebugColor;
    };

    UFUNCTION(BlueprintCallable, Category = "Astronomy")
    static FCelestialBodyData GetPlanetData(FName PlanetName);

    UFUNCTION(BlueprintCallable, Category = "Astronomy")
    static double CalculateOrbitalVelocity(double SemiMajorAxis, double CentralBodyMass);
};
```

**Data Storage**: Hardcoded constants with data table support for modding

### 2. PlayerOriginManager (Actor Component)

**Purpose**: Maintains player at world origin and translates universe accordingly.

**Responsibilities**:

- Track player's "virtual" position in solar system
- Translate all celestial bodies when player moves
- Detect precision threshold violations
- Trigger universe recentering if needed

```cpp
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UPlayerOriginManager : public UActorComponent
{
    GENERATED_BODY()

public:
    // Virtual position in solar system (high precision)
    UPROPERTY(BlueprintReadOnly, Category = "Position")
    FVector VirtualPosition; // Using double precision internally

    // Movement tracking
    UPROPERTY(BlueprintReadOnly, Category = "Position")
    FVector VirtualVelocity;

    UFUNCTION(BlueprintCallable, Category = "Origin")
    void MovePlayer(FVector DeltaMovement);

    UFUNCTION(BlueprintCallable, Category = "Origin")
    void TranslateUniverse(FVector Translation);

    UFUNCTION(BlueprintCallable, Category = "Origin")
    bool ShouldRecenterUniverse() const;

    UFUNCTION(BlueprintCallable, Category = "Origin")
    void RecenterUniverse();

private:
    void UpdateCelestialBodyPositions(FVector Translation);
    double CalculatePrecisionError() const;
};
```

**Update Frequency**: Every frame (Tick) for position, every physics tick for forces

### 3. CelestialBodyComponent (Actor Component)

**Purpose**: Attached to each celestial body actor to enable scaling and physics.

**Responsibilities**:

- Store physical properties (mass, radius)
- Apply scale transformations
- Provide data to gravity simulator
- Manage LOD transitions

```cpp
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UCelestialBodyComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Physical Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    double Mass; // kg

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    double Radius; // meters (actual size)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bApplyGravity = true;

    // Scaling State
    UPROPERTY(BlueprintReadOnly, Category = "Scaling")
    double CurrentScaleFactor = 1.0;

    UPROPERTY(BlueprintReadOnly, Category = "Scaling")
    double TargetScaleFactor = 1.0;

    UPROPERTY(BlueprintReadOnly, Category = "Scaling")
    double DistanceToPlayer = 0.0;

    // Performance
    UPROPERTY(EditAnywhere, Category = "Performance")
    float MaxUpdateDistance = 1000000000.0; // 1 million km

    UPROPERTY(EditAnywhere, Category = "Performance")
    bool bUseScaleSmoothing = true;

    UFUNCTION(BlueprintCallable, Category = "Scaling")
    void UpdateScale(double PlayerDistance);

    UFUNCTION(BlueprintCallable, Category = "Scaling")
    void ApplyScaleImmediate(double ScaleFactor);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    FVector CalculateGravitationalForce(FVector PlayerPosition, double PlayerMass) const;

private:
    void SmoothScaleTransition(float DeltaTime);
    void UpdateLODSystem();
};
```

**Attachment**: Added to planet/moon/star actors in editor or at runtime

### 4. ScalingCalculator (Subsystem)

**Purpose**: Centralized calculation engine for all scaling mathematics.

**Responsibilities**:

- Compute scale factors using inverse-square law
- Apply smoothing algorithms
- Clamp values to safe ranges
- Optimize calculations with caching

```cpp
UCLASS()
class ALEXANDER_API UScalingCalculator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Scaling")
    double CalculateScaleFactor(double Distance, double BaseRadius) const;

    UFUNCTION(BlueprintCallable, Category = "Scaling")
    double ApplyInverseSquareLaw(double Distance, double ReferenceDistance) const;

    UFUNCTION(BlueprintCallable, Category = "Scaling")
    double SmoothScaleTransition(double CurrentScale, double TargetScale, float DeltaTime) const;

    UFUNCTION(BlueprintCallable, Category = "Scaling")
    bool ShouldUpdateScale(double CurrentDistance, double LastDistance, double Threshold) const;

private:
    // Cache for optimization
    TMap<int32, double> ScaleFactorCache;
    double CacheValidityThreshold = 1000.0; // meters

    double ClampScaleFactor(double ScaleFactor) const;
};
```

**Implementation**: Game Instance Subsystem for global access

### 5. GravitySimulator (Subsystem)

**Purpose**: Calculate and apply gravitational forces synchronized with scaling.

**Responsibilities**:

- Sum gravitational forces from all celestial bodies
- Apply forces to player spacecraft
- Synchronize with visual scaling
- Prevent physics instabilities

```cpp
UCLASS()
class ALEXANDER_API UGravitySimulator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Gravity")
    FVector CalculateTotalGravitationalForce(AActor* PlayerShip) const;

    UFUNCTION(BlueprintCallable, Category = "Gravity")
    FVector CalculateGravityFromBody(
        UCelestialBodyComponent* Body,
        FVector PlayerPosition,
        double PlayerMass) const;

    UFUNCTION(BlueprintCallable, Category = "Gravity")
    void ApplyGravityToShip(AActor* PlayerShip, float DeltaTime);

    UPROPERTY(EditAnywhere, Category = "Safety")
    double MaxGForce = 100.0; // Clamp to prevent physics explosion

    UPROPERTY(EditAnywhere, Category = "Performance")
    double GravityInfluenceRadius = 1000000000.0; // 1 million km

private:
    FVector ClampGravitationalForce(FVector Force, double PlayerMass) const;
    bool IsWithinInfluenceRadius(double Distance) const;
};
```

**Update Frequency**: Every physics tick (FixedUpdate)

### 6. CelestialBodyRegistry (Subsystem)

**Purpose**: Track and manage all celestial bodies in the scene.

**Responsibilities**:

- Register/unregister celestial bodies
- Provide fast lookup by name or ID
- Batch update operations
- Performance monitoring

```cpp
UCLASS()
class ALEXANDER_API UCelestialBodyRegistry : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Registry")
    void RegisterCelestialBody(UCelestialBodyComponent* Body);

    UFUNCTION(BlueprintCallable, Category = "Registry")
    void UnregisterCelestialBody(UCelestialBodyComponent* Body);

    UFUNCTION(BlueprintCallable, Category = "Registry")
    TArray<UCelestialBodyComponent*> GetAllCelestialBodies() const;

    UFUNCTION(BlueprintCallable, Category = "Registry")
    UCelestialBodyComponent* FindBodyByName(FName BodyName) const;

    UFUNCTION(BlueprintCallable, Category = "Registry")
    void UpdateAllBodyPositions(FVector Translation);

    UFUNCTION(BlueprintCallable, Category = "Registry")
    void UpdateAllBodyScales(FVector PlayerPosition);

private:
    UPROPERTY()
    TArray<UCelestialBodyComponent*> RegisteredBodies;

    TMap<FName, UCelestialBodyComponent*> BodyLookup;

    void SortBodiesByDistance(FVector PlayerPosition);
};
```

**Lifecycle**: Initialized at game start, persists through level transitions

## Data Models

### Scale Data Structure

```cpp
USTRUCT(BlueprintType)
struct FScaleData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    double CurrentScale = 1.0;

    UPROPERTY(BlueprintReadWrite)
    double TargetScale = 1.0;

    UPROPERTY(BlueprintReadWrite)
    double BaseRadius = 1000000.0; // meters

    UPROPERTY(BlueprintReadWrite)
    double DistanceToPlayer = 0.0;

    UPROPERTY(BlueprintReadWrite)
    double LastUpdateDistance = 0.0;

    UPROPERTY(BlueprintReadWrite)
    float TimeSinceLastUpdate = 0.0f;
};
```

### Gravitational Force Data

```cpp
USTRUCT(BlueprintType)
struct FGravitationalForceData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FVector ForceVector = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    double ForceMagnitude = 0.0;

    UPROPERTY(BlueprintReadWrite)
    FName SourceBodyName;

    UPROPERTY(BlueprintReadWrite)
    double Distance = 0.0;

    UPROPERTY(BlueprintReadWrite)
    bool bIsValid = false;
};
```

### Virtual Position Data (High Precision)

```cpp
// Custom high-precision vector for astronomical distances
USTRUCT(BlueprintType)
struct FVirtualPosition
{
    GENERATED_BODY()

    // Store as double precision (Unreal's FVector is float)
    UPROPERTY(BlueprintReadWrite)
    double X = 0.0;

    UPROPERTY(BlueprintReadWrite)
    double Y = 0.0;

    UPROPERTY(BlueprintReadWrite)
    double Z = 0.0;

    FVector ToFVector() const { return FVector(X, Y, Z); }
    void FromFVector(const FVector& V) { X = V.X; Y = V.Y; Z = V.Z; }

    double Distance() const { return FMath::Sqrt(X*X + Y*Y + Z*Z); }
    FVirtualPosition operator+(const FVirtualPosition& Other) const;
    FVirtualPosition operator-(const FVirtualPosition& Other) const;
};
```

## Error Handling

### Precision Error Detection

```cpp
// In PlayerOriginManager
bool UPlayerOriginManager::ShouldRecenterUniverse() const
{
    // Check if any celestial body is too far from origin
    for (auto* Body : Registry->GetAllCelestialBodies())
    {
        FVector BodyPos = Body->GetOwner()->GetActorLocation();
        if (BodyPos.Size() > UAstronomicalConstants::VR_PRECISION_RADIUS)
        {
            UE_LOG(LogCelestialScaling, Warning,
                TEXT("Precision threshold exceeded: %f meters"), BodyPos.Size());
            return true;
        }
    }
    return false;
}
```

### Scale Factor Clamping

```cpp
// In ScalingCalculator
double UScalingCalculator::ClampScaleFactor(double ScaleFactor) const
{
    if (ScaleFactor < UAstronomicalConstants::MIN_SCALE_FACTOR)
    {
        UE_LOG(LogCelestialScaling, Warning,
            TEXT("Scale factor too small: %f, clamping to %f"),
            ScaleFactor, UAstronomicalConstants::MIN_SCALE_FACTOR);
        return UAstronomicalConstants::MIN_SCALE_FACTOR;
    }

    if (ScaleFactor > UAstronomicalConstants::MAX_SCALE_FACTOR)
    {
        UE_LOG(LogCelestialScaling, Warning,
            TEXT("Scale factor too large: %f, clamping to %f"),
            ScaleFactor, UAstronomicalConstants::MAX_SCALE_FACTOR);
        return UAstronomicalConstants::MAX_SCALE_FACTOR;
    }

    return ScaleFactor;
}
```

### Gravitational Force Safety

```cpp
// In GravitySimulator
FVector UGravitySimulator::ClampGravitationalForce(FVector Force, double PlayerMass) const
{
    double ForceMagnitude = Force.Size();
    double Acceleration = ForceMagnitude / PlayerMass;
    double GForce = Acceleration / 9.81; // Convert to G-force

    if (GForce > MaxGForce)
    {
        UE_LOG(LogCelestialScaling, Error,
            TEXT("Gravitational force exceeds safety limit: %.2f G, clamping to %.2f G"),
            GForce, MaxGForce);

        double ClampedMagnitude = MaxGForce * 9.81 * PlayerMass;
        return Force.GetSafeNormal() * ClampedMagnitude;
    }

    return Force;
}
```

### Null Reference Protection

```cpp
// In CelestialBodyRegistry
void UCelestialBodyRegistry::UpdateAllBodyScales(FVector PlayerPosition)
{
    // Remove null entries
    RegisteredBodies.RemoveAll([](UCelestialBodyComponent* Body) {
        return Body == nullptr || !IsValid(Body) || !IsValid(Body->GetOwner());
    });

    // Update valid bodies
    for (UCelestialBodyComponent* Body : RegisteredBodies)
    {
        if (ensure(Body))
        {
            double Distance = FVector::Dist(PlayerPosition, Body->GetOwner()->GetActorLocation());
            Body->UpdateScale(Distance);
        }
    }
}
```

## Testing Strategy

### Unit Tests

1. **AstronomicalConstants Validation**

   - Verify all constants are within expected ranges
   - Test data retrieval functions
   - Validate orbital calculations

2. **ScalingCalculator Tests**

   - Test inverse-square law calculations
   - Verify clamping behavior
   - Test smoothing algorithms
   - Validate cache invalidation

3. **GravitySimulator Tests**
   - Test force calculations against known values
   - Verify force clamping
   - Test multi-body scenarios
   - Validate synchronization with scaling

### Integration Tests

1. **Player Movement Test**

   - Move player 1 million km
   - Verify all celestial bodies translate correctly
   - Check VR tracking precision maintained
   - Validate no floating-point errors

2. **Scaling Synchronization Test**

   - Approach planet from 1 million km to surface
   - Verify visual scale matches gravitational force
   - Check smooth transitions
   - Validate LOD system integration

3. **Multi-Body Gravity Test**
   - Position player between multiple celestial bodies
   - Verify force vectors sum correctly
   - Check for physics instabilities
   - Validate performance under load

### Performance Tests

1. **50 Celestial Bodies Test**

   - Spawn 50 planets/moons
   - Measure frame time for scaling updates
   - Verify 90+ FPS maintained
   - Profile CPU/GPU usage

2. **Rapid Movement Test**

   - Move player at 10,000 m/s
   - Verify smooth scaling transitions
   - Check for frame hitches
   - Validate no motion sickness triggers

3. **Universe Recentering Test**
   - Trigger recentering at precision threshold
   - Measure recentering time
   - Verify no visual artifacts
   - Check physics continuity

### VR Comfort Tests

1. **Tracking Precision Test**

   - Measure hand tracking accuracy at origin
   - Verify sub-millimeter precision
   - Test across different VR headsets
   - Validate no jitter or drift

2. **Motion Sickness Test**
   - Test planetary approach/departure
   - Verify smooth scaling transitions
   - Check frame rate consistency
   - Validate no unexpected accelerations

### Automated Test Framework

```cpp
// Example automated test
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCelestialScalingTest,
    "Alexander.CelestialScaling.InverseSquareLaw",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FCelestialScalingTest::RunTest(const FString& Parameters)
{
    // Setup
    UScalingCalculator* Calculator = NewObject<UScalingCalculator>();
    double ReferenceDistance = 1000000.0; // 1000 km

    // Test: 2x closer = 4x scale
    double Distance1 = ReferenceDistance;
    double Distance2 = ReferenceDistance / 2.0;

    double Scale1 = Calculator->ApplyInverseSquareLaw(Distance1, ReferenceDistance);
    double Scale2 = Calculator->ApplyInverseSquareLaw(Distance2, ReferenceDistance);

    double ExpectedRatio = 4.0;
    double ActualRatio = Scale2 / Scale1;

    TestEqual(TEXT("Inverse square law ratio"), ActualRatio, ExpectedRatio, 0.001);

    return true;
}
```

## Performance Considerations

### Optimization Strategies

1. **Distance-Based Updates**

   - Only update celestial bodies within influence radius
   - Skip calculations for bodies outside render distance
   - Use spatial partitioning for large systems

2. **Calculation Caching**

   - Cache scale factors when distance change is minimal
   - Reuse gravitational force calculations
   - Invalidate cache only when necessary

3. **LOD Integration**

   - Reduce mesh complexity for distant bodies
   - Disable physics for very distant bodies
   - Use imposters for background stars

4. **Parallel Processing**
   - Calculate scale factors in parallel for multiple bodies
   - Use job system for gravitational force calculations
   - Batch update operations

### Memory Management

- Pre-allocate arrays for registered bodies
- Use object pooling for temporary calculations
- Minimize Blueprint-C++ boundary crossings
- Use native types (double) for precision calculations

### VR-Specific Optimizations

- Maintain 90+ FPS minimum (11.1ms frame budget)
- Prioritize player-visible bodies for updates
- Use async loading for distant celestial bodies
- Implement predictive scaling for fast movement

## Integration Points

### Existing Alexander Systems

1. **VR Template Integration**

   - PlayerOriginManager attaches to VR Pawn
   - Hand tracking remains at origin
   - Motion controllers work normally

2. **Spaceship Flight System**

   - Ship movement triggers universe translation
   - Gravity forces applied to ship physics
   - Autopilot uses virtual positions

3. **Planet Landing System**

   - Scaling transitions to 1:1 at surface
   - Terrain LOD coordinates with scaling
   - Landing detection uses actual distances

4. **AI Vision System**
   - Screenshot capture unaffected
   - Scene analysis includes scale data
   - AI suggestions account for scaling

### External Dependencies

- Unreal Engine 5.6 physics system
- OpenXR for VR tracking
- Blueprint visual scripting for game logic
- Python integration for AI backend

## Migration from Chimera-VR

### Code Adaptation

1. **C# to C++ Translation**

   - Convert Unity MonoBehaviour to UActorComponent
   - Replace Unity's Vector3 with Unreal's FVector
   - Adapt Update() to Tick() and FixedUpdate() to physics tick

2. **Naming Conventions**

   - Prefix classes with U (UObject) or A (Actor)
   - Use Unreal's UPROPERTY and UFUNCTION macros
   - Follow Unreal coding standards

3. **Physics System Differences**
   - Unity's Rigidbody → Unreal's UPrimitiveComponent
   - AddForce() → AddForce() (similar API)
   - FixedUpdate → Substepping or custom physics tick

### Data Migration

- Export Chimera-VR astronomical constants
- Convert Unity prefabs to Unreal Blueprints
- Adapt test scenarios to Unreal's automation framework

## Diagrams

### System Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                     Game Mode / Level                        │
│  ┌────────────────────────────────────────────────────────┐ │
│  │              PlayerOriginManager                        │ │
│  │  - VirtualPosition (double precision)                  │ │
│  │  - TranslateUniverse()                                 │ │
│  └────────────────┬───────────────────────────────────────┘ │
│                   │                                          │
│  ┌────────────────▼───────────────────────────────────────┐ │
│  │          CelestialBodyRegistry                         │ │
│  │  - RegisteredBodies[]                                  │ │
│  │  - UpdateAllBodyPositions()                            │ │
│  └────────────────┬───────────────────────────────────────┘ │
│                   │                                          │
│         ┌─────────┴─────────┐                               │
│         │                   │                               │
│  ┌──────▼──────┐    ┌──────▼──────┐                        │
│  │  Scaling    │    │   Gravity   │                        │
│  │ Calculator  │    │  Simulator  │                        │
│  │             │    │             │                        │
│  └──────┬──────┘    └──────┬──────┘                        │
│         │                   │                               │
│         └─────────┬─────────┘                               │
│                   │                                          │
│  ┌────────────────▼───────────────────────────────────────┐ │
│  │         CelestialBodyComponent (on each planet)        │ │
│  │  - Mass, Radius                                        │ │
│  │  - CurrentScaleFactor                                  │ │
│  │  - UpdateScale()                                       │ │
│  │  - CalculateGravitationalForce()                      │ │
│  └────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

### Scaling Update Flow

```
Frame Start
    │
    ├─→ Player Input (Movement)
    │       │
    │       ▼
    │   PlayerOriginManager.MovePlayer()
    │       │
    │       ├─→ Update VirtualPosition
    │       │
    │       └─→ TranslateUniverse(-DeltaMovement)
    │               │
    │               ▼
    │       CelestialBodyRegistry.UpdateAllBodyPositions()
    │               │
    │               ▼
    │       For each CelestialBody:
    │           Actor->AddActorWorldOffset(-DeltaMovement)
    │
    ├─→ Tick Update (Visual)
    │       │
    │       ▼
    │   CelestialBodyRegistry.UpdateAllBodyScales()
    │       │
    │       ▼
    │   For each CelestialBody:
    │       │
    │       ├─→ Calculate Distance to Player
    │       │
    │       ├─→ ScalingCalculator.CalculateScaleFactor()
    │       │       │
    │       │       └─→ Apply Inverse-Square Law
    │       │
    │       ├─→ SmoothScaleTransition()
    │       │
    │       └─→ Actor->SetActorScale3D()
    │
    └─→ Physics Tick
            │
            ▼
        GravitySimulator.ApplyGravityToShip()
            │
            ▼
        For each CelestialBody:
            │
            ├─→ CalculateGravityFromBody()
            │       │
            │       └─→ Use same distance as scaling
            │
            ├─→ Sum all force vectors
            │
            └─→ Ship->AddForce(TotalGravity)
```

## Design Decisions and Rationales

### Decision 1: Player at Origin vs. Universe at Origin

**Chosen**: Player at Origin

**Rationale**: VR tracking systems have sub-millimeter precision only near world origin. Moving the player away from origin causes floating-point precision loss, leading to jitter and tracking errors. By keeping the player at origin and moving the universe, we maintain perfect VR precision regardless of virtual position in the solar system.

**Trade-off**: Requires translating all celestial bodies every frame, but this is computationally cheap compared to the alternative of dealing with precision errors.

### Decision 2: Inverse-Square Scaling Law

**Chosen**: Scale factor proportional to 1/distance²

**Rationale**: This matches how gravitational force scales with distance, ensuring perfect synchronization between visual appearance and physical forces. When a planet appears 4x larger (because you're 2x closer), the gravitational pull is also 4x stronger.

**Trade-off**: Non-linear scaling can be counterintuitive, but it's physically accurate and prevents visual/physics mismatches.

### Decision 3: Subsystem vs. Actor Components

**Chosen**: Hybrid approach - Subsystems for global logic, Components for per-body data

**Rationale**: Subsystems (ScalingCalculator, GravitySimulator, Registry) provide global access and persist across levels. Components (CelestialBodyComponent) attach to individual actors and store per-body state. This separation of concerns improves maintainability and performance.

**Trade-off**: Slightly more complex architecture, but better organized and more performant.

### Decision 4: Double Precision for Calculations

**Chosen**: Use double precision for all distance and scale calculations

**Rationale**: Astronomical distances require precision beyond float's ~7 significant digits. Double precision provides ~15 significant digits, sufficient for meter-level accuracy across solar system scales.

**Trade-off**: Requires custom FVirtualPosition struct since Unreal's FVector uses floats. Conversion overhead is minimal.

### Decision 5: Frame-Rate Independent Smoothing

**Chosen**: Exponential smoothing with delta time compensation

**Rationale**: Ensures consistent visual behavior regardless of frame rate. Critical for VR where frame rate variations can cause motion sickness.

**Trade-off**: Slightly more complex math, but essential for VR comfort.

## Future Enhancements

1. **Multi-Star Systems**: Support for binary/trinary star systems with complex gravitational interactions
2. **Relativistic Effects**: Time dilation and length contraction at high velocities
3. **N-Body Simulation**: Dynamic orbital mechanics instead of fixed orbits
4. **Procedural Scaling**: Generate LOD meshes procedurally based on scale factor
5. **Network Replication**: Synchronize scaling across multiplayer clients
