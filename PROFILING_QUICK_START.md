# Profiling Test Quick Start (5-Minute Version)

TL;DR - Minimal profiling test setup in 5 steps.

---

## Step-by-Step (Manual, Editor-Based)

### 1. Open FlightTest Map (1 min)
```
File > Open > Content/FlightTest.umap
```

### 2. Place Spaceship (1 min)
- **Place Actors** panel > Search "Spaceship" or "BP_Spaceship"
- Click to place in level at (0, 0, 0)
- If BP doesn't exist, see Troubleshooting below

### 3. Add a Cube (1 min)
- **Place Actors** panel > Search "Cube"
- Place at (5000, 0, 100)
- Scale to 100x100x10 in Details panel

### 4. Set Up Spaceship Control (1 min)
- In **World Settings** (Window > World Settings)
- Ensure you have a **Player Start** or the spaceship is default pawn
- Set **Default Pawn Class** to your spaceship blueprint if needed

### 5. Play and Check Profiling (1 min)
- Click **Play**
- Wait 30-60 seconds
- Check **Output Log** for lines like:
  ```
  [Spaceship_Tick] Avg=2.5ms ...
  [FlightController] Avg=0.8ms ...
  ```
- Stop play
- Check `Saved/Profiling/ProfileReport_*.json`

---

## Expected Output

If working, you'll see in the Output Log:

```
[LogTemp] PerformanceProfilerSubsystem initialized
[LogTemp] Spaceship_Tick: Avg=2.5ms, Min=1.2ms, Max=5.8ms, Samples=120
[LogTemp] FlightController: Avg=0.8ms, Min=0.2ms, Max=1.5ms, Samples=120
[LogTemp] OrbitalMechanics: Avg=0.3ms, Min=0.1ms, Max=0.9ms, Samples=120
```

And a JSON file in: `Saved/Profiling/ProfileReport_YYYYMMDD_HHMMSS.json`

---

## Troubleshooting (30 seconds)

| Problem | Solution |
|---------|----------|
| **No BP_Spaceship found** | Right-click Content Browser > New Blueprint Class > Parent: Spaceship > Save as BP_Spaceship |
| **Spaceship falls infinitely** | Make sure gravity is enabled OR add a collision plane |
| **No profiling output** | Check that PerformanceProfilerSubsystem is in the world |
| **Can't move spaceship** | Make sure FlightController component is attached (check Details) |

---

## What You Just Created

- **Minimal test map** with 1 spaceship + 1 cube
- **Real profiling data** from FlightController, OrbitalMechanics, Spaceship systems
- **Baseline metrics** before Phase 8 content creation
- **Validation** that the profiling system works

---

## What's NOT Included (On Purpose)

- Complex meshes or terrain
- NPCs or missions
- Weather or VFX
- Full level design
- Multiplayer networking

This is ONLY to validate profiling, not to build the game.

---

## Next Steps

1. **Confirm profiling works** - JSON file exists with real data
2. **Save baseline** - Note the FPS and average frame times
3. **Begin Phase 8** - Now add real mission content while profiling
4. **Compare metrics** - Rerun profiling as you add content to measure impact

---

## Alternative: Programmatic Setup (2 minutes)

If you prefer code over clicking:

1. Copy `TestProfilingGameMode.cpp` from `PROFILING_SETUP_CODE_EXAMPLES.md`
2. Compile the project
3. In Project Settings > Maps & Modes > set default GameMode to `TestProfilingGameMode`
4. Open FlightTest and click Play

Done. Actors spawn automatically.

---

## File Reference

- **Main Guide**: `MINIMAL_PROFILING_TEST_SETUP.md`
- **Code Examples**: `PROFILING_SETUP_CODE_EXAMPLES.md`
- **This File**: `PROFILING_QUICK_START.md`
- **Profiler System**: `Source/Alexander/Public/PerformanceProfilerSubsystem.h`
- **Test Map**: `Content/FlightTest.umap`
- **Output**: `Saved/Profiling/*.json`

---

## Success Indicators

- [ ] Map opens without errors
- [ ] Spaceship visible and doesn't fall infinitely
- [ ] Output Log shows profiling data after 30 seconds
- [ ] JSON file created in Saved/Profiling/
- [ ] Can see FPS and system profiling times
- [ ] Frame rate is stable (60 FPS or target rate)

If all checked, you're ready for Phase 8 content creation with profiling validation.

---

**Time to Success: 5 minutes**

Now go build with confidence!
