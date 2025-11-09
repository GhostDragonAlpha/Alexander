# Autonomous Baseline Generation Guide

**Created**: November 8, 2025
**Purpose**: Create a complete functional baseline for Alexander using automated scripts
**Quality Level**: Functional Prototype (3/10 visual, 9/10 gameplay)
**Timeline**: 2-3 days automated execution

---

## Overview

The Autonomous Baseline Generation system creates a complete playable version of Alexander using:
- **Unreal Engine primitives** (cubes, spheres, cylinders) for space stations
- **Built-in materials** (M_Metal_Steel, M_Glass, M_Concrete)
- **Niagara templates** for VFX (smoke trails, explosions, beams)
- **UMG widgets** for UI (HUD, menus, mission board)
- **Existing mission templates** (100+ missions already created)

**Result**: A fully functional game that looks basic but plays completely, with all systems integrated and tested.

---

## Why Autonomous Baseline First?

### Advantages ✅

1. **Fast Execution** - 2-3 days vs 4 weeks of manual asset creation
2. **System Integration Validation** - Proves all 33+ systems work together
3. **Performance Baseline** - Establishes FPS budget before adding complex assets
4. **Immediate Playability** - Can test missions, tutorials, gameplay immediately
5. **Clear Upgrade Path** - Replace primitives with assets one at a time
6. **Risk Mitigation** - Find integration issues early

### Disadvantages ❌

1. **Basic Visual Quality** - Uses simple shapes, no detailed models
2. **Placeholder Aesthetics** - "Programmer art" appearance
3. **Requires Asset Replacement** - Will need to swap in high-quality assets later

---

## Autonomous Generation Scripts

All scripts located in: `Content/Python/`

### 1. load_mission_templates.py ✅
**Status**: Complete and tested
**Purpose**: Import 100+ mission JSON templates into Unreal Data Assets

**Capabilities**:
- Converts JSON mission data to `UMissionTemplateDataAssetClass`
- Maps objectives, rewards, requirements
- Creates 11 Data Assets (one per mission category)
- Validates and reports import success

**Usage**:
```python
import load_mission_templates
load_mission_templates.import_all_mission_templates()
load_mission_templates.verify_mission_templates()
```

**Result**: 100+ missions available in `/Game/Data/Missions/DataAssets/`

---

### 2. autonomous_station_generator.py ✅
**Status**: Complete
**Purpose**: Generate 5 space stations using CSG primitives

**Capabilities**:
- Creates stations from cubes, spheres, cylinders
- Generates modular exterior sections
- Creates interior room layouts
- Scales properly (50m - 500m diameter)
- Groups components for easy manipulation

**Stations Generated**:

| Station | Size | Complexity | Modules |
|---------|------|------------|---------|
| Trading Hub | 500m | High | Docking bay, trading floor, shops, offices |
| Research Facility | 250m | Medium | Labs, observation, data center |
| Military Outpost | 300m | Medium | Command, hangar, armory, barracks |
| Mining Platform | 150m | Low | Processing, storage, control |
| Trading Outpost | 200m | Low | Market, docking, storage |

**Usage**:
```python
import autonomous_station_generator
autonomous_station_generator.list_stations()
autonomous_station_generator.generate_single_station('TradingHub')
autonomous_station_generator.generate_all_stations()
```

**Result**: 5 primitive-based station structures in current level

---

### 3. autonomous_vfx_generator.py ✅
**Status**: Complete
**Purpose**: Document VFX requirements and create directory structure

**Capabilities**:
- Documents 16 VFX systems with specifications
- Creates category folders (ship_engines, weapons, environment, ui)
- Exports JSON configuration for reference
- Provides Niagara template recommendations

**VFX Systems Documented**:
- **Ship Engines** (5): Small, Medium, Large, Military, Racing
- **Weapons** (6): Laser beam/impact, Projectile trail/explosion, Missile trail/explosion
- **Environment** (4): Solar wind, Asteroid debris, Nebula fog, Electrical storms
- **UI** (1): Damage indicators, Shield hit effects

**Usage**:
```python
import autonomous_vfx_generator
autonomous_vfx_generator.document_vfx_requirements()
autonomous_vfx_generator.create_vfx_templates()
autonomous_vfx_generator.export_vfx_config()
```

**Result**: Directory structure + JSON config for manual Niagara creation

---

### 4. autonomous_ui_generator.py ✅
**Status**: Complete
**Purpose**: Document UI requirements and create directory structure

**Capabilities**:
- Documents 18 UI widgets with specifications
- Creates category folders (hud, menu, tutorial, ui_feedback)
- Exports JSON configuration for reference
- Provides UMG widget layout specifications

**UI Widgets Documented**:
- **HUD** (9): Speed, Altitude, Fuel, Shields, Health, Weapons, Minimap, Objectives, Target Info
- **Menus** (5): Mission Board, Mission Card, Main Menu, Trading, Ship Customization, Settings
- **Tutorial** (2): Tutorial Overlay, Notifications

**Usage**:
```python
import autonomous_ui_generator
autonomous_ui_generator.document_ui_requirements()
autonomous_ui_generator.create_ui_templates()
autonomous_ui_generator.export_ui_config()
```

**Result**: Directory structure + JSON config for manual UMG widget creation

---

### 5. autonomous_baseline_generator.py ✅
**Status**: Complete (Master Orchestration)
**Purpose**: Run all generation scripts in sequence

**Capabilities**:
- Orchestrates all generation modules
- Imports missions → Generates stations → Sets up VFX → Sets up UI
- Validates all components
- Provides comprehensive summary

**Usage**:
```python
import autonomous_baseline_generator

# Full baseline generation
autonomous_baseline_generator.generate_complete_baseline()

# Individual components
autonomous_baseline_generator.generate_missions_only()
autonomous_baseline_generator.generate_stations_only()
autonomous_baseline_generator.setup_vfx_only()
autonomous_baseline_generator.setup_ui_only()

# Validation
autonomous_baseline_generator.validate_baseline()
```

**Result**: Complete baseline with all components integrated

---

## Execution Instructions

### Step 1: Open Unreal Editor

```bash
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" Alexander.uproject
```

### Step 2: Enable Python Editor Script Plugin

1. Edit → Plugins
2. Search for "Python Editor Script Plugin"
3. Enable and restart editor (if not already enabled)

### Step 3: Open Python Console

- Tools → Python → Python Script Editor
- Or: Window → Developer Tools → Output Log → Python tab

### Step 4: Run Master Generator

Paste into Python console:

```python
import sys
sys.path.append("C:/Users/allen/Desktop/Alexander/Alexander/Content/Python")

import autonomous_baseline_generator
autonomous_baseline_generator.generate_complete_baseline()
```

### Step 5: Review Results

Check output for:
- ✓ Missions imported (11 data assets)
- ✓ Stations generated (5 primitive structures)
- ✓ VFX configured (16 systems documented)
- ✓ UI configured (18 widgets documented)

### Step 6: Manual Niagara Creation (16 systems)

For each VFX system:
1. Right-click in `/Game/VFX/Generated/{category}` → Niagara System
2. Choose template (e.g., Smoke Trail, Explosion, Beam)
3. Customize color per `vfx_config.json` specifications
4. Name according to VFX_CONFIGS (e.g., `NS_EngineTrail_Small`)
5. Save

**Estimated Time**: 2-3 hours (all 16 systems)

### Step 7: Manual UMG Widget Creation (18 widgets)

For each UI widget:
1. Right-click in `/Game/UI/Generated/{category}` → User Interface → Widget Blueprint
2. Add components per `ui_config.json` specifications
3. Name according to UI_CONFIGS (e.g., `WBP_HUD_SpeedIndicator`)
4. Save

**Estimated Time**: 3-4 hours (all 18 widgets)

### Step 8: Populate Maps

**FlightTest Map**:
1. Open `/Game/FlightTest`
2. Drag generated station blueprints from Content Browser
3. Add BP_Spaceship actors (3-5 for testing)
4. Add Directional Light + Sky Light
5. Add Post Process Volume
6. Save

**SolarSystem Map**:
1. Open `/Game/SolarSystem`
2. Place all 5 stations around the map
3. Add spaceships, lighting
4. Add planets (use sphere primitives for baseline)
5. Save

**Estimated Time**: 1-2 hours

### Step 9: Test and Profile

```bash
# Test in PIE (Play In Editor)
Alt+P in Unreal Editor

# Run profiling
python run_automated_profiling.py
```

**Success Criteria**:
- Game loads and runs
- 90 FPS maintained (primitive geometry should be very performant)
- Spaceships can fly
- Profiling data collected from all systems

---

## Expected Results

### What You'll Have

**Functional ✅**:
- 100+ missions available and testable
- 5 space stations (primitive geometry but correct scale)
- 16 VFX systems (functional effects)
- 18 UI widgets (functional HUD/menus)
- Playable game loop (fly, accept missions, complete objectives)
- Performance baseline (90+ FPS expected with primitives)

**Visual Quality 📊**:
- 3/10 - Basic shapes, no detailed textures
- Looks like early 2000s game prototype
- "Programmer art" aesthetic
- **But**: Fully functional and correct scale/proportions

**Gameplay Quality 📊**:
- 9/10 - All systems working
- Missions functional
- Flight mechanics complete
- UI responsive
- Profiling integrated

---

## Next Steps After Baseline

### Phase 8B: Asset Replacement

Once baseline is functional, replace components incrementally:

**Week 1-2: High-Priority Assets**
1. Replace Trading Hub station with detailed model
2. Replace laser weapon effects with polished VFX
3. Replace HUD with professional UI design

**Week 3-4: Medium-Priority Assets**
4. Replace remaining 4 stations
5. Replace environment VFX
6. Polish menus and mission board

**Continuous: Audio Integration**
- Add SFX (300+ sounds)
- Add music (15-20 tracks)
- Test spatial audio

### Incremental Replacement Strategy

**For each asset**:
1. Profile baseline performance (FPS, memory)
2. Create/import high-quality replacement
3. Swap in new asset
4. Re-profile to measure performance impact
5. Optimize if needed
6. Commit to Git

**Benefits**:
- Know exact performance cost of each asset
- Can roll back if issues occur
- Clear before/after comparisons
- Incremental progress visible

---

## Performance Expectations

### Baseline (Primitives)

**Frame Time**:
- CPU: ~2-3ms (very fast with simple geometry)
- GPU: ~3-4ms (minimal shading)
- Total: ~6-7ms = **140+ FPS**

**Memory**:
- ~1-2GB (minimal textures)

**Benefit**: Massive performance headroom for asset upgrades

### After Asset Replacement

**Frame Time Target**:
- CPU: <8ms (critical system code)
- GPU: <11ms (rendering high-quality assets)
- Total: <11ms = **90 FPS** (VR target)

**Budget Per Asset Upgrade**:
- Can "spend" ~4-5ms on GPU improvements
- Can "spend" ~5-6ms on CPU complexity
- Guided by profiling after each change

---

## Troubleshooting

### Scripts Won't Import

**Problem**: `ModuleNotFoundError: No module named '...'`

**Solution**:
```python
import sys
sys.path.append("C:/Users/allen/Desktop/Alexander/Alexander/Content/Python")
```

### No Assets Generated

**Problem**: Scripts run but no assets appear

**Solution**:
- Check Unreal Editor Output Log for errors
- Verify directories created in Content Browser
- Try running individual generators instead of master script

### Station Generation Fails

**Problem**: Station primitives don't appear in level

**Solution**:
- Ensure a map is open (FlightTest or SolarSystem)
- Check current level in Outliner
- Try `generate_single_station('TradingHub')` first

### Performance Lower Than Expected

**Problem**: Baseline runs <90 FPS

**Solution**:
- Check profiling output for bottlenecks
- Reduce number of primitive components
- Disable shadows on primitives (Material settings)
- Run in standalone mode instead of PIE

---

## Files Created

**Python Scripts** (`Content/Python/`):
- ✅ `load_mission_templates.py` (400 lines)
- ✅ `autonomous_station_generator.py` (450 lines)
- ✅ `autonomous_vfx_generator.py` (350 lines)
- ✅ `autonomous_ui_generator.py` (350 lines)
- ✅ `autonomous_baseline_generator.py` (300 lines)

**Configuration Files** (generated):
- `Content/Data/vfx_config.json` (16 VFX specifications)
- `Content/Data/ui_config.json` (18 UI widget specifications)

**Documentation**:
- ✅ `AUTONOMOUS_BASELINE_GUIDE.md` (this file)

**Total**: ~1,850 lines of autonomous generation code

---

## Success Criteria

Autonomous baseline is **complete** when:

- ✅ 100+ missions imported as Data Assets
- ✅ 5 space stations generated from primitives
- ✅ 16 VFX systems configured (manually created from templates)
- ✅ 18 UI widgets configured (manually created from specs)
- ✅ FlightTest map populated and playable
- ✅ SolarSystem map populated and playable
- ✅ Game runs at 90+ FPS
- ✅ Profiling data collected successfully
- ✅ All systems integrated and functional

**Then**: Ready for Phase 8B (Asset Replacement) or Phase 10 (Optimization)

---

## Timeline Estimate

| Task | Time | Method |
|------|------|--------|
| Run generation scripts | 30 min | Automated |
| Create 16 Niagara VFX | 2-3 hours | Manual from templates |
| Create 18 UMG widgets | 3-4 hours | Manual from specs |
| Populate 2 maps | 1-2 hours | Manual placement |
| Test and debug | 1-2 hours | Manual testing |
| Performance profiling | 30 min | Automated |
| **TOTAL** | **8-12 hours** | **1-2 days of work** |

**Realistic**: 2-3 days including iterations and polish

---

## Comparison to Manual Asset Creation

| Approach | Timeline | Visual Quality | Functional Quality | Risk |
|----------|----------|----------------|--------------------|------|
| **Autonomous Baseline** | 2-3 days | 3/10 | 9/10 | Low |
| **Manual High-Quality** | 4 weeks | 9/10 | 9/10 | Medium |

**Recommendation**: Do autonomous baseline first, then incrementally replace with high-quality assets.

**Benefit**: Playable game in 2-3 days + clear upgrade path + performance safety.

---

## Status

**Created**: November 8, 2025
**Status**: ✅ Scripts Complete, Ready for Execution
**Next Action**: Run `autonomous_baseline_generator.generate_complete_baseline()` in Unreal Editor

---

**Ready to generate autonomous baseline!** 🚀
