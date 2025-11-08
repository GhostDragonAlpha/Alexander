# Minimal Profiling Test Setup for FlightTest Map

## Objective
Create the MINIMAL content needed in the FlightTest map to validate the profiling system with real game data before Phase 8 full content creation. This is NOT about building a complete game experience—only enough to generate meaningful profiling data.

## 1. Systems Being Profiled

Based on `PerformanceProfilerSubsystem.h` and grep analysis, the following systems are actively profiled:

| System | Component | Profile Name | Purpose |
|--------|-----------|--------------|---------|
| **Spaceship** | Pawn | `Spaceship_Tick` | Main ship update loop |
| **FlightController** | Component | `FlightController` | Input handling and flight assistance |
| **OrbitalMechanics** | Component | `OrbitalMechanics` | Orbital calculations (if in orbit) |
| **PhysicsConsensus** | System | `PhysicsConsensus_Predict` / `PhysicsConsensus_Validate` | Physics prediction and validation |
| **StarSystemManager** | System | `StarSystemManager` | Star system updates |
| **Frame Time** | Engine | *implicit* | Frame time tracking (FPS) |
| **Memory Usage** | Engine | *implicit* | Current and peak memory |

**Profiler Features Tracked:**
- Average, min, max execution time per system (ms)
- Sample count and total time
- Start/end timestamps
- Current FPS
- Current and peak memory usage (MB)
- Top 10 bottlenecks

---

## 2. Minimal Actor Requirements

To profile these systems with real data, you need:

### 2.1 Essential Actors
- **1x ASpaceship** - The player spaceship (required to trigger all flight-related profiling)
- **1x Minimal Environment** - A basic static mesh or placeholder (so physics/rendering have something to do)

### 2.2 Optional but Recommended
- **1x Asteroid or Obstacle** - To test collision detection profiling
- **1x Star Body** - To test orbital mechanics if you want full profiling

### 2.3 NOT Needed for Minimal Test
- Multiple complex meshes
- Full mission content
- NPCs or dialogue systems
- VFX or weather systems
- Harvesting equipment
- Full UI systems

---

## 3. Step-by-Step Setup in Unreal Editor

### Step 1: Open FlightTest Map
1. In Unreal Editor, open `Content/FlightTest.umap`
2. Note: This map should already exist from previous phases

### Step 2: Place Spaceship Actor
1. In the **Place Actors** panel (Window > Place Actors)
2. Search for `Spaceship` in the search box
3. If using a Blueprint: Search for `BP_Spaceship`
4. If no Blueprint exists, create one:
   - Right-click in Content Browser > Blueprint Class
   - Parent: `ASpaceship`
   - Compile and save as `BP_Spaceship`
5. **Place the spaceship** in the level at coordinates (0, 0, 0) or any visible location
6. **Configure:**
   - Set the spaceship as the player controller's pawn (optional, or handle in GameMode)
   - Verify it has a FlightController component (should be auto-added)
   - Verify it has OrbitalMechanics component if orbital mechanics should be tested

### Step 3: Spawn a Player Controller
If this map doesn't have a GameMode or PlayerController:
1. Create a new GameMode Blueprint or use the default
2. Set it in Project Settings > Project > Maps & Modes > Default GameMode
3. Or set it in the level's World Settings

### Step 4: Add Static Geometry (Minimal)
**Option A - Simple Placeholder:**
1. Place > Cube from the Place Actors panel
2. Scale it to something visible (e.g., 1000, 1000, 100 units)
3. Position it offset from the spaceship

**Option B - Terrain:**
1. Create a basic landscape or use a large static plane mesh

**Why:** Gives the engine something to render/physics something to collide with

### Step 5: (Optional) Add One Asteroid
1. Search for `Asteroid` in Place Actors
2. Place one near the spaceship (but not touching)
3. This will trigger asteroid ticking and physics

### Step 6: Configure Test Settings
In the level's World Settings:
1. **Enable Gravity** as needed
2. **Set Time Scale** to 1.0 for normal speed (or slower for easy observation)
3. **Player Start** - Make sure one exists or the spaceship will be the spawn point

### Step 7: Add a Profiling Test Actor (Recommended)
1. Create a new Blueprint Class > Actor
2. Add the `ProfilingTestActor` component OR create a simple test actor that:
   - Runs for 30-60 seconds
   - Logs profiling results
   - Exports to JSON
3. Place it in the level
4. Configure test duration in Details panel

---

## 4. What Profiling Data You Should See

### 4.1 Expected Output (Console Logs)
```
[LogTemp] Spaceship_Tick: Avg=2.5ms, Min=1.2ms, Max=5.8ms, Samples=120
[LogTemp] FlightController: Avg=0.8ms, Min=0.2ms, Max=1.5ms, Samples=120
[LogTemp] OrbitalMechanics: Avg=0.3ms, Min=0.1ms, Max=0.9ms, Samples=120
[LogTemp] PhysicsConsensus_Predict: Avg=1.2ms, Min=0.5ms, Max=3.2ms, Samples=120
[LogTemp] StarSystemManager: Avg=0.1ms, Min=0.05ms, Max=0.4ms, Samples=120
```

### 4.2 Expected JSON Export
File: `Saved/Profiling/ProfileReport_YYYYMMDD_HHMMSS.json`
```json
{
  "Profiles": [
    {
      "SystemName": "Spaceship_Tick",
      "AverageTimeMs": 2.5,
      "MinTimeMs": 1.2,
      "MaxTimeMs": 5.8,
      "SampleCount": 120,
      "TotalTimeMs": 300.0
    },
    ...
  ],
  "FrameTimeMs": 16.67,
  "CurrentFPS": 60.0,
  "CurrentMemoryMB": 1024,
  "PeakMemoryMB": 1256
}
```

### 4.3 Top Bottlenecks
The system automatically identifies the slowest systems:
- Expected for minimal test: Spaceship_Tick (usually ~2-3ms at 60 FPS)
- Physics/Flight systems should be in the 0.1-1.0ms range each

---

## 5. Estimated Time to Complete Setup

| Task | Time | Notes |
|------|------|-------|
| Open/review FlightTest map | 2 min | Should already exist |
| Place Spaceship actor | 5 min | Or 10 min if creating Blueprint first |
| Add static geometry | 5 min | Just one cube or plane |
| (Optional) Add asteroid | 2 min | If testing collision |
| (Optional) Add profiling test actor | 5 min | For automated testing |
| Test run and verify profiling works | 10 min | Run for 30-60 seconds |
| **TOTAL** | **15-30 min** | Depends on reusing existing assets |

---

## 6. Verification Checklist

- [ ] FlightTest map opens without errors
- [ ] Spaceship spawns at or near (0, 0, 0)
- [ ] Spaceship has FlightController component (check in Details)
- [ ] No red errors in the Output Log about missing components
- [ ] Static geometry is visible in viewport
- [ ] When you press Play, the spaceship doesn't fall infinitely
- [ ] Console shows profiling output after ~30 seconds
- [ ] JSON export file is created in `Saved/Profiling/`
- [ ] All system names appear in profiling output

---

## 7. Alternative: Programmatic Actor Creation

Instead of manually placing actors, you can create them programmatically via:

### 7.1 Blueprint Level Script
In the FlightTest map's Level Blueprint:
1. **Event BeginPlay** → **Construct Object from Class** (ASpaceship)
2. Connect to **Spawn Actor from Class** with the result
3. Set transform and rotation
4. Repeat for static geometry and other actors

**Advantages:**
- Reproducible
- No manual positioning needed
- Can be version-controlled more easily

**Disadvantages:**
- Requires Blueprint knowledge
- Harder to iterate visually
- May need to adjust spawn logic

### 7.2 C++ GameMode
Create a custom GameMode class:
```cpp
// In BeginPlay or PostInitializeComponents
void AMinimalTestGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Spawn spaceship
    FVector SpawnLocation(0, 0, 500); // Above ground
    ASpaceship* Spaceship = GetWorld()->SpawnActor<ASpaceship>(SpawnLocation, FRotator::ZeroRotator);

    // Spawn environment cube
    FVector CubeLocation(5000, 0, 100);
    AActor* Cube = GetWorld()->SpawnActor<AActor>(CubeLocation, FRotator::ZeroRotator);
    // (Would need to attach a static mesh component)

    // Spawn profiling test actor
    AProfilingTestActor* Profiler = GetWorld()->SpawnActor<AProfilingTestActor>();
}
```

**Advantages:**
- Full control via code
- Can set all properties programmatically
- Works headless for CI/CD testing

**Disadvantages:**
- Requires C++ compilation
- Less visual feedback during iteration

### 7.3 Python Script (Via Unreal's Python API)
Can be run in Editor or as a standalone script (see `Content/Python/` directory)

---

## 8. What NOT to Do

- **Don't add complex meshes** - Use primitives or simple cubes
- **Don't populate with many actors** - Start with 1-2, only add more if profiling seems too fast
- **Don't enable complex systems** (Weather, VFX, Audio) - Only if specifically testing them
- **Don't create custom materials** - Use default materials
- **Don't place NPCs or missions** - Not needed for profiling validation
- **Don't set up networking** - Keep it single-player for now

---

## 9. Next Steps After Minimal Setup

Once you verify profiling works with this minimal setup:

1. **Run profiling test** - 30-60 second gameplay session
2. **Export results** - JSON report will be in `Saved/Profiling/`
3. **Analyze bottlenecks** - Use JSON to identify slowest systems
4. **Document baseline** - Save the profiling output as your baseline
5. **Begin Phase 8 content** - Now add real mission content with confidence
6. **Retest profiling** - Run same tests with real content to measure impact

---

## 10. Troubleshooting

### Profiling Data Not Appearing
- Check that `PerformanceProfilerSubsystem` is enabled in project settings
- Verify `bProfilingEnabled` is true in the subsystem
- Look for errors in Output Log referencing the profiler

### Spaceship Doesn't Spawn
- Check that the Spaceship or BP_Spaceship class is compiled
- Verify the class is in the correct module (Alexander)
- Check that no compilation errors exist

### No Physics/Movement Happening
- Ensure gravity is enabled (World Settings)
- Check that FlightController component is attached
- Verify the player controller is set to possess the spaceship

### Performance is Already High CPU Usage
- With just one spaceship, expect 2-5ms per frame for Spaceship_Tick
- If higher, check for infinite loops or missing tick rate settings
- If lower, gameplay may be too simple (which is fine for validation)

---

## 11. Key Files Referenced

- `Source/Alexander/Public/PerformanceProfilerSubsystem.h` - Profiler interface
- `Source/Alexander/Private/PerformanceProfilerSubsystem.cpp` - Implementation
- `Source/Alexander/Public/Spaceship.h` - Spaceship class
- `Source/Alexander/Public/FlightController.h` - Flight input/assistance
- `Source/Alexander/Public/OrbitalMechanics.h` - Orbital calculations
- `Content/FlightTest.umap` - The test map
- `Saved/Profiling/` - Output directory for profiling reports

---

## 12. Success Criteria

Your minimal profiling test is successful when:

1. **Map loads** without errors
2. **Spaceship exists** and doesn't fall infinitely
3. **Console logs** show profiling output after running for 30+ seconds
4. **JSON export** contains data for at least 3 systems (Spaceship_Tick, FlightController, OrbitalMechanics)
5. **Frame time** is stable (60 FPS or your target framerate)
6. **Memory** doesn't grow unbounded during the test
7. **You can see patterns** in the profiling data (which systems are slowest)

Once these are met, you have a working profiling test setup ready for Phase 8 content creation.

---

## Summary

This minimal setup requires:
- **1 Spaceship actor**
- **1 static geometry (cube/plane)**
- **30 minutes setup time**
- **Generates valid profiling data** for all core flight systems
- **Foundation for Phase 8** content testing

Start here, verify it works, then expand with real game content while continuously profiling.
