# Phase 9: Performance Profiling - Quick Start

**Status**: Production Ready
**Last Updated**: November 8, 2025

---

## Run Profiling (Zero Manual Steps)

```bash
python run_automated_profiling.py
```

**That's it!** The script will:
1. Launch game with SolarSystem map
2. Profile for 3 minutes automatically
3. Close game and analyze results
4. Display bottlenecks and recommendations

**Time**: ~5 minutes total (including startup/shutdown)

---

## Understanding Results

### Output Format

```
SYSTEM PERFORMANCE:
!! [CRITICAL] OrbitalMechanics    Avg:  12.543ms  Max:  25.123ms  [Must optimize immediately]
!  [HIGH    ] Spaceship_Tick      Avg:   6.234ms  Max:  15.432ms  [Should optimize soon]
   [OK      ] FlightController    Avg:   1.234ms  Max:   3.456ms  [Acceptable]
```

### Performance Thresholds

Based on 90 FPS VR target (11.1ms frame budget):

| Status | Avg Time | Symbol | Action Required |
|--------|----------|--------|-----------------|
| **CRITICAL** | >8ms | !! | Must optimize immediately |
| **HIGH** | 5-8ms | ! | Should optimize soon |
| **MEDIUM** | 2-5ms | ~ | Monitor, optimize if easy |
| **OK** | <2ms | ✓ | Acceptable performance |

---

## Configuration Options

Edit `run_automated_profiling.py` to customize:

```python
# Line 21: Change map
MAP_TO_LOAD = "/Game/SolarSystem"  # or "/Game/FlightTest"

# Line 23: Change duration
PROFILE_DURATION = 180  # seconds (default: 3 minutes)
```

---

## What Gets Profiled

Currently instrumented systems:
- **Spaceship::Tick** - Main actor update
- **FlightController** - Flight dynamics
- **OrbitalMechanics** - Orbital calculations
- **StarSystemManager** - Celestial management
- **PhysicsConsensus** - Multiplayer validation (if active)

---

## Troubleshooting

### No profiling samples collected

**Try:**
1. Change map to `/Game/FlightTest` (line 21)
2. Increase duration to 300 seconds (line 23)
3. Check `Saved/Logs/Alexander.log` for errors

### Game exits early

**Check:**
1. Crash logs in `Saved/Logs/`
2. Map loads properly (try simpler map)
3. Game runs manually first

---

## Next Steps After Profiling

### If Critical Bottlenecks Found (>8ms)

1. **Identify root cause** - What's taking the time?
2. **Apply optimization techniques:**
   - Caching computed values
   - Spatial partitioning
   - LOD-based update frequency
   - Multi-threading heavy calculations
3. **Re-profile** to verify improvement

### Optimization Workflow

```bash
# 1. Run baseline profiling
python run_automated_profiling.py

# 2. Make optimizations to code

# 3. Build game
python build_and_test.py

# 4. Re-profile to measure improvement
python run_automated_profiling.py

# 5. Compare before/after results
```

---

## Integration with CI/CD

### Nightly Performance Tests

```bash
# Add to build pipeline
python run_automated_profiling.py

# Check for regressions
if [ $? -ne 0 ]; then
  echo "Performance regression detected!"
  exit 1
fi
```

### Performance Budget Enforcement

Monitor profiling JSON files over time:
- Alert if any system's AverageTimeMs increases >10%
- Track frame budget trends
- Detect memory leaks

---

## Key Documentation

| Document | Purpose |
|----------|---------|
| **AUTOMATED_PROFILING_COMPLETE.md** | Complete profiling guide |
| **PHASE9_FINAL_SUMMARY.md** | Phase 9 summary & pivot explanation |
| **PHASE9_QUICK_START.md** | This document |
| **PIE_PROFILING_GUIDE.md** | Manual profiling (backup method) |

---

## Success Criteria

Phase 9 complete when:
- [x] Automated profiling works (zero manual steps)
- [x] Profiling data collected from 5+ systems
- [x] Bottleneck identification automated
- [ ] All critical systems <8ms average
- [ ] Frame time <11ms (90fps target)
- [ ] No frame drops during gameplay

---

## Production Files

**Use these:**
- `run_automated_profiling.py` - Main profiling automation
- `build_and_test.py` - Build & test suite
- `automated_build.py` - Build automation
- `nightly_build.py` - CI/CD builds

**Ignore these (deprecated):**
- `run_profiling_commandlet.py`
- `automated_profiling_v2.py`
- `autonomous_profiling_*.py`
- Other `run_profiling_*.py` variants

---

**Created**: November 8, 2025
**Author**: Claude (Autonomous Agent)
**Status**: Production Ready

Run `python run_automated_profiling.py` to get started!
