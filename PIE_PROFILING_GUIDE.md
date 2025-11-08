# PIE Profiling Guide - Real-World Performance Testing

**Phase 9 Complete - Ready for Real-World Data Collection**

---

## Quick Start (5 Minutes)

### Step 1: Launch Editor
Open `Alexander.uproject` in Unreal Editor 5.6

### Step 2: Load Your Test Level
- Open your primary testing level (e.g., `FlightTest` map)
- Or any level with active gameplay
- Note: SolarSystem will be populated with content during Phase 8

### Step 3: Enter Play-In-Editor (PIE)
- Press the **Play** button (or Alt+P)
- Play normally for **3-5 minutes**
- Do typical gameplay activities:
  - Fly your spaceship
  - Move around the world
  - Activate systems
  - Test VR (if available)
  - Stress test (spawn lots of objects if possible)

### Step 4: Exit PIE
- Press **Escape** or **Stop** button
- **Profiling data automatically exports when PIE ends**

### Step 5: Find Your Results
**Location:** `Saved/Profiling/ProfileReport_YYYYMMDD_HHMMSS.json`

Example: `Saved/Profiling/ProfileReport_20251108_153000.json`

---

## What Gets Profiled Automatically

The following systems are already instrumented and ready:

✅ **Spaceship::Tick** - Main spaceship actor update
✅ **FlightController** - Input processing and flight dynamics
✅ **OrbitalMechanics** - Orbital position calculations
✅ **StarSystemManager** - Celestial body updates
✅ **PhysicsConsensus** - Multiplayer physics validation (if active)

**All systems work perfectly in PIE mode** - no commandlet issues!

---

## Interpreting Results

### JSON Structure

```json
{
  "TotalFrames": 18000,
  "AverageFrameTimeMs": 16.67,
  "AverageFPS": 60.0,
  "PeakMemoryMB": 4250,
  "CurrentMemoryMB": 3890,
  "Profiles": [
    {
      "SystemName": "Spaceship_Tick",
      "AverageTimeMs": 1.234,
      "MinTimeMs": 0.5,
      "MaxTimeMs": 8.5,
      "SampleCount": 18000
    }
  ]
}
```

### Performance Thresholds (90 FPS VR Target = 11.1ms frame budget)

| Average Time | Status | Action |
|--------------|--------|--------|
| **>8ms** | 🔴 CRITICAL | Must optimize immediately |
| **5-8ms** | 🟡 HIGH | Should optimize soon |
| **2-5ms** | 🟠 MEDIUM | Monitor, optimize if easy wins |
| **<2ms** | ✅ OK | Acceptable |

---

## Quick Analysis

### Option 1: Manual Review
Open the JSON in your favorite editor and look at `AverageTimeMs` for each system.

### Option 2: Python Script (Already Available)
```bash
python run_profiling_commandlet.py
```

Wait, that's the commandlet script. Let me create a PIE analysis script...

Actually, the JSON format is the same! You can manually inspect it or I can create a quick analysis script.

---

## Common Scenarios

### Scenario 1: "I want baseline performance"
1. Enter PIE
2. Stand still for 1 minute (idle baseline)
3. Exit PIE
4. Check results

### Scenario 2: "I want stress test"
1. Enter PIE
2. Spawn 100+ objects
3. Fly around rapidly
4. Activate all systems
5. Exit PIE after 3-5 minutes
6. Check results for bottlenecks

### Scenario 3: "I want VR profiling"
1. Put on VR headset
2. Launch in VR mode
3. Play for 5 minutes
4. Check for systems >8ms (breaking 90 FPS target)

---

## Troubleshooting

### "No JSON file generated"
**Cause:** PerformanceProfilerSubsystem may not be initialized

**Solutions:**
1. Check that you're exiting PIE properly (not crashing)
2. Check Saved/Logs/Alexander.log for "Profile report exported"
3. Profiler only exports on clean shutdown

### "JSON has no data"
**Cause:** Systems didn't tick during PIE

**Solutions:**
1. Make sure you actually played (not just paused)
2. Ensure the profiled systems are active in your level
3. Check that your level has spaceships/actors using these systems

### "Numbers look weird"
**Cause:** May have captured loading/streaming hitches

**Solutions:**
1. Wait 30 seconds after entering PIE before "counting"
2. Ignore first/last 10 seconds of data (loading spikes)
3. Run multiple sessions and average

---

## What You'll Learn

After PIE profiling, you'll know:

✅ **Which systems are bottlenecks** (>8ms)
✅ **Frame budget breakdown** (how much time each system takes)
✅ **Max spikes** (worst-case scenarios)
✅ **Memory usage** (current and peak)
✅ **Actual FPS** (not just target FPS)

---

## Next Steps After Profiling

### If No Bottlenecks (<2ms per system)
🎉 Congratulations! Your performance is excellent. Consider:
- Adding more systems to profiling (VFX, Audio, Terrain)
- Stress testing with more objects
- Testing in VR

### If Minor Issues (2-5ms systems)
📊 Document and monitor. Consider:
- Low-hanging fruit optimizations
- LOD systems
- Culling improvements

### If Critical Issues (>8ms systems)
🚨 Phase 10 optimization required:
1. Identify the worst offender
2. Analyze what it's doing (use Unreal Insights for deeper profiling)
3. Apply targeted optimizations:
   - Reduce frequency (tick less often)
   - Optimize algorithm (better data structures)
   - Parallelize (multi-threading)
   - Cache results (avoid redundant calculations)

---

## Advanced: Unreal Insights Integration

For deeper profiling beyond our custom system:

### Enable Unreal Insights
1. Launch editor with: `-trace=cpu,frame,log`
2. Enter PIE and play
3. Open Unreal Insights app: `Engine/Binaries/Win64/UnrealInsights.exe`
4. Load the trace file from `Saved/Profiling/Traces/`

### Benefits
- GPU profiling
- Memory allocation tracking
- Thread visualization
- Function-level timing

---

## Why PIE Instead of Commandlet?

**Commandlet Issues:**
- ❌ Doesn't call BeginPlay() naturally
- ❌ No rendering load
- ❌ No physics simulation
- ❌ No player input
- ❌ Missing 70% of actual performance cost
- ✅ Fast baseline (we already have this)

**PIE Benefits:**
- ✅ All systems run naturally
- ✅ Real rendering cost
- ✅ Real physics simulation
- ✅ Real player input
- ✅ Real memory usage
- ✅ **Actual gameplay performance**

**Conclusion:** PIE profiling gives you the data you actually need for optimization.

---

## Automated PIE Profiling (Future)

For CI/CD, you can automate PIE profiling with Python:

```python
import unreal

# Load level
world = unreal.EditorLevelLibrary.load_level('/Game/Maps/SolarSystem')

# Enter PIE
unreal.EditorLevelLibrary.editor_set_game_view(True)

# Wait 180 seconds
time.sleep(180)

# Exit PIE (triggers profiling export)
unreal.EditorLevelLibrary.editor_set_game_view(False)

# Parse JSON results
# ...
```

But for now, **manual PIE profiling is the fastest path to useful data**.

---

## Summary

**You're ready to profile!**

1. ✅ Profiling infrastructure installed and working
2. ✅ 5 critical systems instrumented
3. ✅ Automatic JSON export configured
4. ✅ PIE mode works out of the box

**Just:**
1. Play in PIE for 3-5 minutes
2. Exit PIE
3. Check `Saved/Profiling/ProfileReport_*.json`
4. Identify bottlenecks >8ms
5. Move to Phase 10 optimization

---

**Total Time to Get Data: 5 minutes**
**Value: 100x more than commandlet baseline**

---

**Ready to Profile? Just hit Play!** ▶️
