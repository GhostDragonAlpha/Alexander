# Unreal Insights Guide - Ship Customization Profiling

## Overview
Unreal Insights is a powerful profiling and analysis tool that provides detailed frame-by-frame performance data. This guide walks through using it to profile the ship customization system.

## Prerequisites

- Unreal Engine 5.0+ installed
- Project compiled in Development or DebugGame configuration
- Ship customization component implemented with profiling macros
- UnrealInsights.exe executable (comes with UE5)

## Step-by-Step Guide

### Step 1: Enable Tracing in Your Project

**For Editor-based profiling:**

1. Open your project in Unreal Editor
2. Navigate to: **Window → Developer Tools → Output Log**
3. In the output log console, type:
   ```
   Trace.Start
   ```
4. This starts memory, CPU, and frame tracing
5. Continue with gameplay
6. Stop tracing with:
   ```
   Trace.Stop
   ```

**For Packaged Game Profiling:**

Package your game for Development or DebugGame, then launch with:
```bash
YourGame.exe -trace=cpu,frame,stats,memory
```

### Step 2: Generate a Trace File

**Option A: Using Console Command (Editor)**

1. Open **Window → Developer Tools → Output Log**
2. Execute:
   ```
   Trace.Start
   ```
3. Run gameplay for 30-60 seconds (equip parts, test customization)
4. Execute:
   ```
   Trace.Stop
   ```
5. The trace file is automatically saved to: `Saved/Traces/`

**Option B: Command Line (Packaged Game)**

```bash
YourGame.exe -trace=cpu,frame,stats -tracefile=output.utrace
```

The trace file will be saved to the project root or specified location.

### Step 3: Open UnrealInsights

1. Navigate to your Unreal Engine installation directory
2. Open: `Engine/Binaries/Win64/UnrealInsights.exe`
3. Or from the Editor: **Window → Developer Tools → Unreal Insights**

### Step 4: Load Your Trace File

1. In UnrealInsights, select **Open Trace File**
2. Navigate to your trace file (usually in `Saved/Traces/`)
3. Click **Open**
4. UnrealInsights will load and analyze the trace

### Step 5: Navigate to Ship Customization Stats

**Method 1: Using the Timers View**

1. In UnrealInsights, open the **Timers** tab
2. Search for "ShipCustomization" in the filter box
3. You'll see all ship customization stat counters:
   - `ShipCustomization_CalculateStats`
   - `ShipCustomization_EquipPart`
   - `ShipCustomization_ApplyStats`
   - `ShipCustomization_UpdateVisuals`
   - etc.

**Method 2: Using the CPU Profiler**

1. Open the **CPU Profiler** tab
2. Select **Call Stack** or **Thread Timeline**
3. Look for functions from `ShipCustomizationComponent`
4. Functions with `PROFILE_SHIP_CUSTOMIZATION_SCOPE()` will appear here

### Step 6: Identify Performance Hotspots

**Frame Timeline View:**

1. Open **Frames** tab
2. Look for frames that exceed 11.1ms (red bars)
3. Click on a frame to see breakdown
4. Look for spikes in:
   - `ShipCustomization_CalculateStats`
   - `ShipCustomization_ApplyStats`
   - `ShipCustomization_UpdateVisuals`

**Analyze Timing:**

1. In the **Timers** tab, select a stat
2. View statistics:
   - **Average** - typical execution time
   - **Min** - best case
   - **Max** - worst case
   - **Count** - how many times called

Example expected values:
```
ShipCustomization_CalculateStats:
  Average: 0.045 ms ✓
  Max: 0.065 ms ✓
  Count: 5400 (per frame)

ShipCustomization_ApplyStats:
  Average: 0.180 ms ✓
  Max: 0.200 ms ✓
  Count: 150 (10 ships × 15 updates)
```

### Step 7: Compare Multiple Frames

**Before Optimization:**

1. Record a baseline trace (before optimization)
2. Note hotspot functions and their timings
3. Save this trace as reference

**After Optimization:**

1. Record a new trace
2. Compare side-by-side
3. Look for improvements in:
   - Max frame time
   - Average CPU time per function
   - Number of allocations

### Step 8: Memory Analysis

**In Unreal Insights:**

1. Open the **Memory** tab
2. Look for allocations from:
   - `ShipCustomizationComponent`
   - `FShipLoadout`
   - `FPlayerProgressionData`
3. Check for growth patterns
4. Identify spikes (potential memory leaks)

Expected memory allocation:
```
ShipCustomization Memory:
  Per ship: 40-45 KB
  Per equipped part: 2-3 KB
  Total for 100 ships: 4.2-4.7 MB
```

### Step 9: Export Data for Analysis

**Export as CSV:**

1. In any stat view, right-click on a stat
2. Select **Export**
3. Choose CSV format
4. Save file for spreadsheet analysis

**Export Screenshots:**

1. View interesting profiles
2. Take screenshots (Ctrl+PrintScreen)
3. Use for reporting

## Common Issues and Solutions

### Issue: "No trace data found for ShipCustomization"
**Solution:**
- Verify `ENABLE_SHIP_CUSTOMIZATION_PROFILING` is enabled (should be on by default in Development)
- Ensure profiling macros were added to the code
- Recompile the project
- Make sure ships are being spawned and customized during the trace

### Issue: "Frame time looks normal but ship systems show high times"
**Solution:**
- Check CPU clock throttling (power settings)
- Verify other systems aren't interfering
- Isolate by disabling other game systems
- Check for hitching from garbage collection or level streaming

### Issue: "Can't find UnrealInsights.exe"
**Solution:**
```bash
# Look in:
C:\Program Files\Epic Games\UE_5.X\Engine\Binaries\Win64\UnrealInsights.exe

# Or enable from Editor:
Window → Developer Tools → Unreal Insights
```

## Performance Tuning Workflow

### 1. Profile Baseline
- Start a clean trace
- Spawn 10 ships with customization
- Equip parts 1000 times
- Stop trace
- Note timings

### 2. Identify Bottlenecks
- Open trace in UnrealInsights
- Find the slowest operation
- Note the average and max times
- Check against budget

### 3. Optimize
- Make code changes
- Recompile
- Re-profile
- Compare traces

### 4. Verify Improvement
- Open both traces
- Compare average times
- Verify frame time improved
- Check for memory regression

## Example Optimization Scenario

**Before:**
```
ShipCustomization_EquipPart: 0.45 ms (budget: 0.15 ms) ✗ EXCEEDS
  └─ CalculateTotalStats: 0.25 ms (too slow)
  └─ ApplyStatsToFlightController: 0.15 ms (ok)
  └─ UpdateShipVisuals: 0.05 ms (ok)
```

**Optimization: Cache stat calculations**
- Only recalculate when parts change
- Use cached value on ApplyStats

**After:**
```
ShipCustomization_EquipPart: 0.10 ms ✓ PASS
  └─ CalculateTotalStats: 0.08 ms (cached hit, fast)
  └─ ApplyStatsToFlightController: 0.02 ms (uses cache)
  └─ UpdateShipVisuals: 0.00 ms (batched)
```

## Advanced Features

### Using the CPU Profiler

1. Open **CPU Profiler** tab
2. Sort by **Total Time** to find hotspots
3. Click on a function to expand call stack
4. See who's calling your expensive functions
5. Identify unexpected callers

### Memory Tracking

1. Enable **Memory** tab
2. Filter for `ShipCustomization*` allocations
3. Sort by **Total Size**
4. Look for growing allocations
5. Check for unreleased memory

### Frame Comparison

1. Take snapshot of a problematic frame
2. Take snapshot of good frame
3. Compare side-by-side
4. Identify differences

## Tips for Best Results

1. **Run 60+ seconds** - Ensures stable average measurements
2. **Minimize engine overhead** - Close unnecessary windows
3. **Test with realistic scenarios** - Multiple ships, various customization
4. **Compare consistent scenarios** - Same test setup each time
5. **Use release builds for final profiling** - Better represents shipped performance
6. **Profile on target hardware** - VR headset, quest 3, etc.

## Performance Budget Checklist

After profiling with UnrealInsights:

- [ ] Average frame time < 11.1 ms
- [ ] 95th percentile frame time < 11.1 ms
- [ ] CalculateTotalStats < 0.05 ms average
- [ ] EquipPart < 0.15 ms average
- [ ] ApplyStats < 0.2 ms average
- [ ] UpdateVisuals < 0.25 ms average
- [ ] Peak memory < 306 MB
- [ ] No memory leaks detected
- [ ] No unexplained spikes in frame time
- [ ] Consistent performance across multiple frames

## Automated Tracing Script

For batch profiling, create a script:

```python
import subprocess
import os
from pathlib import Path

project_path = Path("C:/Projects/Alexander")
engine_path = Path("C:/Program Files/Epic Games/UE_5.4")

# Launch with tracing enabled
cmd = [
    str(engine_path / "Engine/Binaries/Win64/UnrealEditor.exe"),
    str(project_path / "Alexander.uproject"),
    "-game",
    "-trace=cpu,frame,stats,memory",
]

subprocess.run(cmd)
```

Then analyze the generated trace with UnrealInsights.

## Resources

- [Unreal Engine Insights Documentation](https://docs.unrealengine.com/latest/INT/developing-with-unreal-insights/index.html)
- [Performance Profiling Best Practices](https://docs.unrealengine.com/latest/INT/performance-and-profiling/index.html)
- [Frame Pacing for VR](https://docs.unrealengine.com/latest/INT/developing-games/vr/index.html)

---

**Last Updated**: November 7, 2025
**Status**: ACTIVE
**Next Update**: After first performance profiling session
