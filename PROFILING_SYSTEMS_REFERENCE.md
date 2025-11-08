# Profiling Systems Reference

Detailed breakdown of every system that generates profiling data, what triggers it, and what it measures.

---

## System Profiling Map

```
PerformanceProfilerSubsystem (World Subsystem)
├── Frame Time Tracking
│   ├── AverageFrameTimeMs
│   ├── CurrentFPS
│   └── Updates every frame (implicit)
│
├── Memory Tracking
│   ├── CurrentMemoryUsageMB
│   ├── PeakMemoryUsageMB
│   └── Sampled each frame
│
└── System Profiles (Named)
    ├── Spaceship_Tick
    │   ├── Component: Spaceship (APawn)
    │   ├── Called from: Spaceship::Tick()
    │   ├── Measures: Main spaceship update loop
    │   └── Expected: 2-5ms at 60 FPS
    │
    ├── FlightController
    │   ├── Component: FlightController
    │   ├── Called from: FlightController::TickComponent()
    │   ├── Measures: Input processing, flight assistance
    │   └── Expected: 0.5-1.5ms at 60 FPS
    │
    ├── OrbitalMechanics
    │   ├── Component: OrbitalMechanics
    │   ├── Called from: OrbitalMechanics::TickComponent()
    │   ├── Measures: Orbital calculation (if enabled)
    │   └── Expected: 0.1-1.0ms at 60 FPS
    │
    ├── PhysicsConsensus_Predict
    │   ├── Component: PhysicsConsensus
    │   ├── Called from: PhysicsConsensus::PredictNextState()
    │   ├── Measures: Physics prediction for next frame
    │   └── Expected: 0.5-2.0ms per prediction
    │
    ├── PhysicsConsensus_Validate
    │   ├── Component: PhysicsConsensus
    │   ├── Called from: PhysicsConsensus::ValidatePhysicsState()
    │   ├── Measures: Physics state validation
    │   └── Expected: 0.3-1.0ms per validation
    │
    └── StarSystemManager
        ├── Component: StarSystemManager
        ├── Called from: StarSystemManager::Tick()
        ├── Measures: Star system updates
        └── Expected: 0.05-0.5ms (rarely called in minimal test)
```

---

## 1. Spaceship_Tick

**What it measures:** The main update loop of the spaceship pawn.

**File:** `Source/Alexander/Private/Spaceship.cpp:235`

**Triggered:** Every frame when spaceship exists and is ticking

**Components involved:**
- Base pawn tick
- FlightController tick (nested)
- OrbitalMechanics tick (nested)
- Physics updates
- Network replication

**Expected timing:**
- Minimal test (1 spaceship): 2-5ms per frame
- With physics: +1-2ms
- With network replication: +0.5ms
- Total impact on frame time: ~15% at 60 FPS target (16.67ms per frame)

**Profile data captured:**
```
SystemName: "Spaceship_Tick"
AverageTimeMs: 2.5
MinTimeMs: 1.2
MaxTimeMs: 5.8
SampleCount: 120  (one per frame at 60 FPS for 2 seconds)
TotalTimeMs: 300.0  (2.5ms * 120)
```

**What impacts this:**
- Player input complexity
- Physics enabled/disabled
- Network update frequency
- Number of replicated properties
- Collision checks

---

## 2. FlightController

**What it measures:** Input handling and flight assistance calculation.

**File:** `Source/Alexander/Private/FlightController.cpp:75`

**Triggered:** Every frame during TickComponent

**Components involved:**
- Input axis reading (throttle, pitch, yaw, roll)
- Input smoothing calculations
- Flight assist mode logic
- Collision avoidance (if enabled)
- Thrust limiting (if enabled)
- Velocity calculations

**Expected timing:**
- Simple input only: 0.2-0.5ms
- With flight assistance: +0.2ms
- With collision avoidance: +0.1-0.2ms
- Total: typically 0.5-1.5ms per frame

**Profile data captured:**
```
SystemName: "FlightController"
AverageTimeMs: 0.8
MinTimeMs: 0.2
MaxTimeMs: 1.5
SampleCount: 120
```

**What impacts this:**
- Input sensitivity/smoothing settings
- Flight assist mode active
- Collision avoidance enabled
- Number of nearby obstacles
- Physics query complexity

---

## 3. OrbitalMechanics

**What it measures:** Orbital physics calculations (semi-major axis, true anomaly, etc.)

**File:** `Source/Alexander/Private/OrbitalMechanics.cpp:67`

**Triggered:** Every frame during TickComponent (if orbital mechanics active)

**Components involved:**
- Kepler element calculations
- State vector transformations
- Orbital period calculations
- Velocity vector computations
- Gravitational parameter lookups

**Expected timing:**
- Orbital math: 0.1-0.5ms
- With gravity calculation: +0.2-0.5ms
- Total: typically 0.3-1.0ms

**Profile data captured:**
```
SystemName: "OrbitalMechanics"
AverageTimeMs: 0.35
MinTimeMs: 0.1
MaxTimeMs: 0.9
SampleCount: 120
```

**What impacts this:**
- Orbital elements complexity
- Number of bodies influencing gravity
- Calculation frequency
- Precision requirements

---

## 4. PhysicsConsensus_Predict

**What it measures:** Physics prediction for the next frame state.

**File:** `Source/Alexander/Private/PhysicsConsensus.cpp:35` (approx)

**Triggered:** Per physics prediction (not necessarily every frame)

**Components involved:**
- Position prediction based on velocity
- Velocity update based on forces
- Rotation prediction
- Angular velocity updates
- Constraint resolution

**Expected timing:**
- Simple prediction: 0.5-1.0ms
- With constraints: +0.5-1.0ms
- Total: typically 0.5-2.0ms per prediction call

**Profile data captured:**
```
SystemName: "PhysicsConsensus_Predict"
AverageTimeMs: 1.0
MinTimeMs: 0.5
MaxTimeMs: 2.0
SampleCount: 60  (may not run every frame)
```

**What impacts this:**
- Number of physics objects
- Constraint complexity
- Solver iteration count
- Mass and inertia calculations

---

## 5. PhysicsConsensus_Validate

**What it measures:** Validation of predicted physics state against actual physics.

**File:** `Source/Alexander/Private/PhysicsConsensus.cpp:86` (approx)

**Triggered:** Per physics validation cycle

**Components involved:**
- Collision detection
- Constraint checking
- Velocity verification
- Position correction
- Error accumulation

**Expected timing:**
- Simple validation: 0.3-0.5ms
- With many collisions: +0.5-1.0ms
- Total: typically 0.3-1.0ms

**Profile data captured:**
```
SystemName: "PhysicsConsensus_Validate"
AverageTimeMs: 0.5
MinTimeMs: 0.3
MaxTimeMs: 1.2
SampleCount: 60
```

**What impacts this:**
- Number of colliding objects
- Collision complexity
- Validation iterations
- Physics body count

---

## 6. StarSystemManager

**What it measures:** Updates to star system state and celestial body positions.

**File:** `Source/Alexander/Private/StarSystemManager.cpp:38`

**Triggered:** When StarSystemManager ticks (may be less frequent than other systems)

**Components involved:**
- Celestial body position updates
- Orbital element progression
- Time advancement
- System state changes
- Network updates (if multiplayer)

**Expected timing:**
- Single system update: 0.05-0.2ms
- With multiple bodies: +0.05-0.1ms per body
- Total: typically 0.05-0.5ms (or not called in minimal test)

**Profile data captured:**
```
SystemName: "StarSystemManager"
AverageTimeMs: 0.1
MinTimeMs: 0.05
MaxTimeMs: 0.4
SampleCount: 12  (may be called less frequently)
```

**What impacts this:**
- Number of celestial bodies
- Update frequency
- Orbital accuracy requirements
- Network replication frequency

---

## 7. Frame Time & Memory (Implicit Tracking)

**What it measures:** Overall frame time and memory usage across all systems.

**Accessed via:**
```cpp
Profiler->GetAverageFrameTimeMs();      // Average ms per frame
Profiler->GetCurrentFPS();               // Calculated from frame time
Profiler->GetCurrentMemoryUsageMB();    // Current memory usage
Profiler->GetPeakMemoryUsageMB();       // Peak memory since init
```

**Expected values:**
- Frame time at 60 FPS: ~16.67ms
- FPS: ~60.0
- Memory: 800-1200 MB (depends on system)
- Memory growth: <1% per minute (should be flat)

**Data structure:**
```cpp
struct FProfileData
{
    FName SystemName;
    double AverageTimeMs;
    double MinTimeMs;
    double MaxTimeMs;
    int32 SampleCount;
    double TotalTimeMs;
    FDateTime StartTime;
    FDateTime EndTime;
};
```

---

## Profiling Hooks

All these systems are profiled using the PROFILE_SCOPE macro:

```cpp
// In each system's update function:
void ASomeActor::Tick(float DeltaTime)
{
    // Get profiler subsystem
    UPerformanceProfilerSubsystem* Profiler = GetWorld()->GetSubsystem<UPerformanceProfilerSubsystem>();

    // Scope-based profiling
    PROFILE_SCOPE(Profiler, FName("SystemName"));

    // ... system code here ...
    // Automatically records time when scope exits
}
```

The macro measures elapsed time between creation and destruction of the FProfileScope object.

---

## Expected Output Timeline

### At T=0s (Level Load)
```
[LogTemp] PerformanceProfilerSubsystem initialized
```

### At T=5s (Sampling running)
```
[Output Log] Spaceship_Tick: 120 samples, Avg=2.5ms
[Output Log] FlightController: 120 samples, Avg=0.8ms
```

### At T=30s (Test duration complete)
```
[LogTemp] ======================================================================
[LogTemp] PROFILING RESULTS
[LogTemp] ======================================================================
[LogTemp] Spaceship_Tick
[LogTemp]   Average: 2.5 ms
[LogTemp]   Min: 1.2 ms
[LogTemp]   Max: 5.8 ms
[LogTemp]   Samples: 1800 (30 sec @ 60 FPS)
[LogTemp]   Total: 4500.0 ms
[LogTemp]
[LogTemp] FlightController
[LogTemp]   Average: 0.8 ms
[LogTemp]   ...
```

### At T=30.5s (Export complete)
```
[LogTemp] PerformanceProfilerSubsystem deinitialized - Report saved to:
         C:\...\Saved\Profiling\ProfileReport_20251108_120000.json
```

---

## JSON Export Format

The profiler exports to JSON when the world is destroyed or manually via:
```cpp
Profiler->ExportToJSON(FilePath);
```

**Sample output:**
```json
{
  "Profiles": [
    {
      "SystemName": "Spaceship_Tick",
      "AverageTimeMs": 2.5,
      "MinTimeMs": 1.2,
      "MaxTimeMs": 5.8,
      "SampleCount": 1800,
      "TotalTimeMs": 4500.0,
      "StartTime": "2025-11-08T12:00:00Z",
      "EndTime": "2025-11-08T12:00:30Z"
    },
    {
      "SystemName": "FlightController",
      "AverageTimeMs": 0.8,
      "MinTimeMs": 0.2,
      "MaxTimeMs": 1.5,
      "SampleCount": 1800,
      "TotalTimeMs": 1440.0
    },
    ...
  ],
  "FrameTimeMs": 16.67,
  "CurrentFPS": 60.0,
  "CurrentMemoryMB": 1024,
  "PeakMemoryMB": 1256,
  "SessionDuration": 30.0
}
```

---

## How to Read Profiling Results

**Good signs:**
- Spaceship_Tick: 2-5ms (acceptable physics overhead)
- FlightController: 0.5-1.5ms (fast input processing)
- FPS: Stable at 60 (no spikes)
- Memory: Flat (no leaks)

**Bad signs:**
- Spaceship_Tick: >10ms (physics bottleneck)
- Memory growing continuously (leak)
- FPS dropping over time (accumulating overhead)
- One system taking >50% of frame time (optimize that system)

---

## Quick Reference Table

| System | Expected Time | Frequency | When Active |
|--------|---------------|-----------|-------------|
| Spaceship_Tick | 2-5ms | Every frame | Spaceship spawned |
| FlightController | 0.5-1.5ms | Every frame | Spaceship spawned |
| OrbitalMechanics | 0.3-1.0ms | Every frame | Orbital mode active |
| PhysicsConsensus_Predict | 0.5-2.0ms | Per prediction | Physics active |
| PhysicsConsensus_Validate | 0.3-1.0ms | Per validation | Physics active |
| StarSystemManager | 0.05-0.5ms | Per update | Star system updates |
| Frame Time | ~16.67ms @ 60FPS | Every frame | Always |
| Memory | 800-1200MB | Per frame sample | Always |

---

## Extending the Profiler

To add your own system to profiling:

```cpp
// In your system's update function:
void MySystem::Update()
{
    UPerformanceProfilerSubsystem* Profiler = GetWorld()->GetSubsystem<UPerformanceProfilerSubsystem>();

    // Option 1: Scope-based (automatic)
    {
        PROFILE_SCOPE(Profiler, FName("MySystem"));
        // ... code here ...
    }

    // Option 2: Manual
    Profiler->StartProfiling(FName("MySystem"));
    // ... code here ...
    Profiler->StopProfiling(FName("MySystem"));

    // Option 3: Record specific measurement
    Profiler->RecordSample(FName("MySystem"), TimeInMs);
}
```

Then the profiler will automatically track timing, min/max, samples, etc.

---

## Summary

The profiling system captures execution time for:
- 6 main systems (Spaceship, Flight, Orbital, Physics×2, Stars)
- Overall frame time and FPS
- Memory usage (current and peak)

Minimal test should generate meaningful data from the main 3-4 systems within 30-60 seconds of gameplay.
