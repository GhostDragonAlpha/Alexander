# Alexander - Performance Profiling Guide

**Project**: Alexander VR Space Simulation
**Last Updated**: November 8, 2025
**Status**: Production Ready

---

## Quick Start (Recommended Method)

### Automated Profiling

**Zero manual steps - fully automated:**

```bash
python run_automated_profiling.py
```

**What it does:**
1. Launches game with FlightTest map
2. Profiles for 3 minutes automatically
3. Closes game and exports profiling data
4. Analyzes results and displays bottlenecks

**Time**: ~5 minutes total (including startup/shutdown)

**Output**: Console summary + JSON report in `Saved/Profiling/ProfileReport_YYYYMMDD_HHMMSS.json`

---

## Understanding Profiling Results

### Output Format

```
SYSTEM PERFORMANCE:
================================================================================
!! [CRITICAL] OrbitalMechanics    Avg:  12.543ms  Max:    25.123ms  Samples: 10800
!  [HIGH    ] Spaceship_Tick      Avg:   6.234ms  Max:    15.432ms  Samples: 54000
   [OK      ] FlightController    Avg:   1.234ms  Max:     3.456ms  Samples: 54000

BOTTLENECK ANALYSIS:
!! CRITICAL (>8ms): 1 systems
     OrbitalMechanics: 12.543ms

!  HIGH (5-8ms): 1 systems
     Spaceship_Tick: 6.234ms

   OK (<2ms): 1 systems
```

### Performance Thresholds

Based on 90 FPS VR target (11.1ms frame budget):

| Status | Avg Time | Symbol | Action Required |
|--------|----------|--------|-----------------|
| **CRITICAL** | >8ms | !! | Must optimize immediately |
| **HIGH** | 5-8ms | ! | Should optimize soon |
| **MEDIUM** | 2-5ms | ~ | Monitor, optimize if easy |
| **OK** | <2ms | ✓ | Acceptable performance |

**Frame Budget Breakdown:**
- 90 FPS = 11.1ms total frame time
- CPU budget: ~8ms (leaves 3ms+ for GPU)
- GPU budget: ~11ms (rendering)
- Per-system budget: <2ms ideal, <8ms maximum

---

## Currently Instrumented Systems

The profiling infrastructure tracks these systems:

1. **Spaceship::Tick** - Main spaceship actor update
2. **FlightController** - 6DOF flight dynamics
3. **OrbitalMechanics** - N-body orbital calculations
4. **StarSystemManager** - Celestial body management
5. **PhysicsConsensus** - Multiplayer physics validation (if active)

**Future additions planned (Phase 10):**
- VFXManager - Visual effects
- AudioSystemManager - Sound system
- TerrainStreamingManager - LOD and streaming
- AdvancedWeatherSystem - Weather simulation
- NetworkReplicationManager - Network sync

---

## Profiling Infrastructure

### How It Works

The profiling system uses RAII-based scoped macros:

```cpp
// In any system's critical function
void UMySystem::ExpensiveFunction()
{
    PROFILE_SCOPE("MySystem_ExpensiveFunction");

    // Your code here
    // Profiler automatically measures execution time
}
```

**Implementation:**
- `PerformanceProfilerSubsystem` - GameInstance subsystem
- `PROFILE_SCOPE` macro - RAII timing wrapper
- Automatic JSON export on game shutdown
- No performance overhead in Shipping builds

### Adding Profiling to Your Code

**Step 1: Add macro to function**
```cpp
void UFlightController::Tick(float DeltaTime)
{
    PROFILE_SCOPE("FlightController");  // <-- Add this line

    // Existing code...
}
```

**Step 2: Rebuild**
```bash
python automated_build.py
```

**Step 3: Profile**
```bash
python run_automated_profiling.py
```

**Step 4: Check results**
Your new system will appear in profiling output automatically.

---

## Configuration Options

### Customizing Automated Profiling

Edit `run_automated_profiling.py`:

```python
# Line 21: Change map to profile
MAP_TO_LOAD = "/Game/FlightTest"  # Primary testing map, or "/Game/SolarSystem" (Phase 8 content)

# Line 23: Change profiling duration
PROFILE_DURATION = 180  # seconds (default: 3 minutes)

# Line 19-20: Editor/project paths (usually don't need to change)
EDITOR = r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe"
PROJECT = r"C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject"
```

**Recommended durations:**
- Quick check: 60 seconds
- Normal profiling: 180 seconds (3 minutes)
- Stress test: 600 seconds (10 minutes)

---

## Manual Profiling (Alternative Methods)

### Method 1: Play-In-Editor (PIE)

**Use when:**
- You want to manually control the session
- Testing specific gameplay scenarios
- Debugging performance issues interactively

**Steps:**
1. Open `Alexander.uproject` in Unreal Editor
2. Load your test map (e.g., FlightTest - primary testing map)
3. Click Play (Alt+P) or Play in Standalone Game
4. Perform gameplay actions you want to profile
5. Close PIE (Escape or Stop)
6. Check `Saved/Profiling/` for JSON export

**Advantages:**
- Full control over gameplay scenario
- Can test specific edge cases
- Visual debugging

**Disadvantages:**
- Requires manual intervention
- Editor overhead affects measurements
- Not reproducible for CI/CD

### Method 2: Unreal Insights

**For deep CPU/GPU analysis:**

1. Enable Insights trace:
   ```bash
   UnrealEditor.exe Alexander.uproject -game -trace=cpu,frame,log
   ```

2. Launch Unreal Insights:
   ```bash
   "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealInsights.exe"
   ```

3. Load trace file from `Saved/Profiling/Traces/`

**Use for:**
- Frame-by-frame analysis
- GPU profiling
- Memory allocation tracking
- Thread contention analysis

### Method 3: Stat Commands

**In-game console commands:**

```
stat FPS          # Show FPS
stat Unit         # Frame time breakdown (Game/Draw/GPU)
stat SceneRendering  # Rendering stats
stat Memory       # Memory usage
stat Slate        # UI performance
```

**To enable console:**
- Press ` (backtick) in PIE
- Type command
- Press Enter

---

## Analyzing Results

### Reading JSON Reports

Profiling data is exported to: `Saved/Profiling/ProfileReport_YYYYMMDD_HHMMSS.json`

**Structure:**
```json
{
  "TotalFrames": 10800,
  "AverageFrameTimeMs": 16.667,
  "AverageFPS": 60.00,
  "PeakMemoryMB": 3800,
  "CurrentMemoryMB": 3250,
  "ExportTime": "2025.11.08-16.00.00",
  "Profiles": [
    {
      "SystemName": "OrbitalMechanics",
      "AverageTimeMs": 12.543,
      "MinTimeMs": 5.123,
      "MaxTimeMs": 25.123,
      "TotalTimeMs": 135464.4,
      "SampleCount": 10800,
      "StartTime": "2025.11.08-16.00.00",
      "EndTime": "2025.11.08-16.03.00"
    }
  ]
}
```

### Identifying Bottlenecks

**Priority 1: Critical (>8ms average)**
- Blocks VR frame rate target
- Must optimize immediately
- Use aggressive optimization techniques

**Priority 2: High (5-8ms average)**
- May cause frame drops under load
- Optimize soon
- Consider caching, LOD, spatial partitioning

**Priority 3: Medium (2-5ms average)**
- Monitor for regressions
- Low-hanging fruit optimizations
- Consider if total frame time is tight

**Priority 4: OK (<2ms average)**
- Acceptable performance
- No immediate action needed

**Also check:**
- **Max time** - Spikes indicate inconsistent performance
- **Sample count** - Verify system is actually running
- **Frame variance** - High variance = unstable frame times

---

## Optimization Techniques

### When System is >8ms (Critical)

**1. Caching**
```cpp
// BAD: Recalculate every frame
float Gravity = CalculateGravityField(Location);  // Expensive!

// GOOD: Cache and update periodically
if (FrameCount % 10 == 0)  // Update every 10 frames
{
    CachedGravity = CalculateGravityField(Location);
}
float Gravity = CachedGravity;  // Use cached value
```

**2. Spatial Partitioning**
```cpp
// BAD: Check all objects
for (AActor* Actor : AllActors)  // O(N²)
{
    if (IsNearby(Actor)) { /* ... */ }
}

// GOOD: Use spatial hash or octree
TArray<AActor*> NearbyActors = SpatialGrid->QueryRadius(Location, Radius);  // O(1) or O(log N)
for (AActor* Actor : NearbyActors) { /* ... */ }
```

**3. LOD-Based Update Frequency**
```cpp
// Update distant objects less frequently
int UpdateInterval = FMath::Max(1, Distance / 1000.0f);  // Further = less frequent
if (FrameCount % UpdateInterval == 0)
{
    UpdateOrbitalPosition();
}
```

**4. Multi-threading**
```cpp
// Move heavy calculations to worker thread
AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
{
    // Heavy calculation here
    ExpensiveCalculation();
});
```

**5. GPU Compute Shaders**
```cpp
// For massively parallel tasks (e.g., n-body physics)
// Implement as compute shader
// Update thousands of objects in parallel on GPU
```

### General Optimization Checklist

- [ ] Profile first (don't guess)
- [ ] Optimize hottest code path first
- [ ] Cache expensive calculations
- [ ] Use LOD for distant objects
- [ ] Implement spatial queries (octree/grid)
- [ ] Reduce per-frame allocations
- [ ] Use object pooling
- [ ] Multi-thread when possible
- [ ] Consider GPU compute for parallel tasks
- [ ] Re-profile after changes

---

## CI/CD Integration

### Nightly Performance Tests

```bash
# In your build pipeline
python run_automated_profiling.py

# Check for regressions
if [ $? -ne 0 ]; then
  echo "Performance regression detected!"
  exit 1
fi
```

### Performance Budget Enforcement

```python
# Parse profiling JSON
import json

with open('Saved/Profiling/ProfileReport_latest.json') as f:
    data = json.load(f)

# Check budgets
for profile in data['Profiles']:
    avg_time = profile['AverageTimeMs']
    system = profile['SystemName']

    if avg_time > 8.0:
        print(f"FAIL: {system} exceeded budget: {avg_time}ms")
        exit(1)

print("PASS: All systems within budget")
```

### Regression Detection

```bash
# Compare current run to baseline
python compare_profiling.py baseline.json current.json

# Alert if any system regressed >10%
```

---

## Troubleshooting

### No Profiling Samples Collected

**Symptoms:**
- JSON file is empty or has 0 profiles
- Script reports "No profiling samples collected"

**Possible causes:**
1. Map doesn't have profiled actors/systems
2. Game crashed during profiling
3. Profiler subsystem didn't initialize

**Solutions:**
- Map is already set to `/Game/FlightTest` (primary testing map)
- Try increasing duration to 300 seconds (line 23)
- Check `Saved/Logs/Alexander.log` for errors
- Verify profiled systems exist in the map
- Ensure actors are spawned in the map during gameplay

### Game Exits Early

**Symptoms:**
- Script reports game exited after <30 seconds
- No profiling data collected

**Solutions:**
- Check crash logs in `Saved/Logs/`
- Try simpler map (e.g., empty test level)
- Verify game runs manually first
- Check system requirements (RAM, GPU)

### Only Test Systems in Results

**Symptoms:**
- Results show "TestSystem1", "TestSystem2", "TestSystem3"
- No real gameplay systems profiled

**Causes:**
- Map loaded has no real actors
- Test actors spawned for testing

**Solutions:**
- Use FlightTest map (primary testing map with content)
- Or use SolarSystem map (Phase 8 content, currently under development)
- Increase profiling duration
- Check that map has spaceships or profiled actors

### High Frame Time Variance

**Symptoms:**
- Max time >> Average time
- Unstable frame rates

**Solutions:**
- Implement time-slicing for heavy operations
- Add frame budgets (stop if >Xms spent)
- Spread work across multiple frames
- Use async tasks for non-critical work

---

## Best Practices

### When to Profile

- **After major feature additions**
- **Before optimization passes**
- **During performance regression checks**
- **Before release milestones**
- **When experiencing frame drops**

### Profiling Workflow

```
1. Run baseline profiling
   ↓
2. Identify bottlenecks (>8ms)
   ↓
3. Implement optimizations
   ↓
4. Rebuild and re-profile
   ↓
5. Compare before/after
   ↓
6. Commit if improved
```

### Performance Budget Strategy

1. **Total frame budget**: 11.1ms (90 FPS)
2. **Reserve for GPU**: 3-4ms
3. **CPU budget**: 7-8ms
4. **Per-system budgets**:
   - Critical gameplay: 2ms each
   - Background systems: 0.5ms each
   - Total: <8ms

### Documentation

- Document all optimizations made
- Keep before/after profiling data
- Note optimization techniques used
- Track performance over time

---

## Key Files & Locations

### Scripts
- `run_automated_profiling.py` - Main profiling automation (use this!)
- `test_performance_profiling.py` - Unit tests
- `integrate_profiling.py` - Integration helpers

### Source Code
- `Source/Alexander/Public/PerformanceProfilerSubsystem.h` - Profiler subsystem
- `Source/Alexander/Private/PerformanceProfilerSubsystem.cpp` - Implementation

### Documentation
- `PROFILING.md` - This document
- `PHASE9_QUICK_START.md` - Quick reference
- `PHASE9_FINAL_SUMMARY.md` - Phase 9 summary
- `AUTOMATED_PROFILING_COMPLETE.md` - Detailed automation guide

### Profiling Data
- `Saved/Profiling/ProfileReport_*.json` - Profiling results
- `Saved/Logs/Alexander.log` - Game logs

---

## Additional Resources

- [SETUP.md](SETUP.md) - Development environment setup
- [Unreal Engine Performance Guidelines](https://docs.unrealengine.com/5.6/en-US/performance-guidelines/)
- [Unreal Insights Documentation](https://docs.unrealengine.com/5.6/en-US/unreal-insights/)
- [VR Performance Best Practices](https://docs.unrealengine.com/5.6/en-US/vr-performance/)

---

## Summary

**For most use cases:**
```bash
python run_automated_profiling.py
```

**Check results, optimize critical systems (>8ms), re-profile, repeat.**

That's it! The automated profiling system handles everything else.

---

**Last Updated**: November 8, 2025
**Status**: Production Ready
**Profiling Systems**: 5 (expandable)
**Automation**: Fully automated, zero manual steps
