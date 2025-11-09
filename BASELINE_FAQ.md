# Alexander Autonomous Baseline System - FAQ

Frequently asked questions about the Alexander autonomous baseline system for rapid prototype development.

---

## General Questions

### What is the autonomous baseline?

The autonomous baseline is an automated system that generates a fully functional prototype of the Alexander space simulation game using CSG primitives (cubes, spheres, cylinders) instead of final art assets. It creates:

- 3 space station environments with primitive geometry
- 10+ sample missions with objectives and rewards
- VFX systems (engine trails, weapon effects, environmental particles)
- Complete UI/HUD system with VR support
- Performance profiling integration

**Purpose**: Validate gameplay mechanics, test systems integration, and establish performance baselines before investing in high-quality assets.

**Time to create**: 15-30 minutes for complete baseline generation.

---

### Why use primitives instead of real assets?

**Speed**: Generate complete environments in minutes vs. weeks/months for art pipeline.

**Iteration**: Quickly test different station layouts, mission flows, and gameplay mechanics.

**Performance baseline**: Establish target frame rates with minimal geometry before adding complex assets.

**Validation**: Prove gameplay concepts work before committing artist resources.

**Placeholder identification**: Clearly see what needs final art (everything is obviously placeholder).

**Example**: Test station docking mechanics with cube-based stations. Once validated, replace cubes with detailed 3D models while keeping the same collision and gameplay logic.

---

### How long does it take to create the baseline?

**Typical execution times**:
- Mission generation: 2-5 minutes (10+ missions)
- Station creation: 5-10 minutes (3 stations with interiors)
- VFX setup: 3-7 minutes (engine trails, weapons, environment)
- UI generation: 2-5 minutes (HUD, menus, VR interfaces)
- Total: 15-30 minutes for complete baseline

**Factors affecting time**:
- Computer speed (SSD vs. HDD makes significant difference)
- Unreal Editor responsiveness
- Number of missions generated
- Complexity of station layouts

**Recommendation**: Run overnight or during lunch break for first-time generation.

---

### What quality level can I expect?

**Functional quality**: Production-ready gameplay systems.
- All systems work and are fully integrated
- Mission logic is complete and tested
- UI responds to all player inputs
- VFX perform at target frame rates

**Visual quality**: Prototype/greybox level.
- Primitive geometry (cubes, spheres, cylinders)
- Basic materials (solid colors, simple textures)
- Particle effects using basic shapes
- Functional but not visually polished UI

**Performance quality**: Optimized for target platforms.
- VR: 90 FPS on recommended hardware
- Desktop: 60 FPS on minimum spec
- All systems use object pooling and LOD

**Analogy**: Like a movie animatic - all the action and timing is there, but using stick figures and placeholder effects.

---

### Can I skip steps?

**Not recommended for first baseline**: The system is designed as an integrated whole. Skipping steps may cause:
- Missing dependencies (UI needs mission data)
- Incomplete testing (can't validate without all systems)
- Performance unknowns (can't establish baseline)

**After initial baseline**: You can regenerate individual components:
```python
# Regenerate only missions
python phase9_baseline_simple.py --component missions

# Regenerate only VFX
python phase9_baseline_simple.py --component vfx

# Regenerate only UI
python phase9_baseline_simple.py --component ui
```

**Safe to skip**:
- VR components if no VR hardware available
- Specific mission types you don't need
- Individual VFX effects you won't use

**Never skip**:
- Station generation (needed for mission contexts)
- Core mission system (foundation for all gameplay)
- Performance profiling (critical for optimization)

---

### What if I don't have VR?

**The baseline works fine without VR hardware.**

**What you get**:
- Full desktop gameplay with mouse/keyboard or gamepad
- 2D UI overlays work perfectly
- All missions and station interactions functional
- VFX visible on standard monitor

**What you miss**:
- VR-specific UI (3D spatial widgets)
- Hand controller interactions
- Room-scale movement testing
- VR performance profiling

**Configuration**: Edit `phase9_baseline_simple.py`:
```python
ENABLE_VR = False  # Set to False if no VR hardware
```

**Later VR addition**: VR components can be added later without regenerating entire baseline:
```python
python phase9_baseline_simple.py --component vr_ui --enable-vr
```

---

## Technical Questions

### What Unreal version is required?

**Required version**: Unreal Engine 5.6

**Why 5.6?**
- Uses UE 5.6 Niagara improvements
- Requires UE 5.6 Enhanced Input system
- Depends on UE 5.6 VR template updates
- Performance tools added in UE 5.6

**Will it work on older versions?**
- UE 5.5: Mostly compatible, some VFX may need adjustments
- UE 5.4 or earlier: Not recommended, missing critical features
- UE 4.27: Not compatible, requires significant rework

**Checking your version**:
1. Open Unreal Editor
2. Help → About Unreal Editor
3. Look for "Version: 5.6.0" or similar

**Upgrading**: If on older version, download UE 5.6 from Epic Games Launcher.

---

### What Python version?

**Required**: Python 3.8 or higher

**Recommended**: Python 3.10 or 3.11 for best performance

**Checking your version**:
```bash
python --version
# Should show: Python 3.8.x or higher
```

**Required packages** (install via `pip install -r requirements.txt`):
- `unreal` (Unreal Python API)
- `numpy` (for math operations)
- `json` (built-in, for data handling)

**Virtual environment recommended**:
```bash
python -m venv alexander_env
alexander_env\Scripts\activate  # Windows
pip install -r requirements.txt
```

**Troubleshooting**:
- If `python` command not found, try `python3`
- Ensure Python is in system PATH
- Use 64-bit Python (32-bit not supported by Unreal)

---

### Do I need Blender/Maya? (Not for baseline)

**No 3D modeling software required for baseline.**

The autonomous baseline generates all geometry using Unreal's built-in CSG (Constructive Solid Geometry) tools. Everything is created programmatically.

**When you'll need 3D software**:
- Replacing primitive stations with detailed models
- Creating custom spaceship meshes
- Building character models
- Modeling environment props

**Recommended for later phases**:
- Blender (free, open source)
- Maya (industry standard, subscription)
- 3ds Max (alternative to Maya)
- Houdini (for procedural generation)

**Baseline creates**:
- Station geometry: CSG cubes and cylinders
- Docking ports: CSG primitives
- Placeholder ships: Basic shapes
- Environment: Procedural generation

---

### Can I use this commercially?

**Yes, with proper licensing.**

**Alexander project code**:
- Check project LICENSE file for specific terms
- Typically MIT or similar permissive license
- Commercial use generally allowed

**Unreal Engine**:
- Free to use until $1M in gross revenue
- 5% royalty on revenue above $1M
- See Unreal Engine EULA for complete terms

**Third-party dependencies**:
- Check individual licenses for any plugins used
- Most are compatible with commercial use
- Some may require attribution

**Best practice**:
1. Read Alexander LICENSE file
2. Review Unreal Engine EULA
3. Check licenses of any additional plugins
4. Consult legal counsel for commercial projects

**Assets you create**: You own all content you create using the baseline system.

---

### How do I update the baseline later?

**Updating baseline components**:

**1. Regenerate specific components**:
```python
# Update just missions without touching stations/VFX
python phase9_baseline_simple.py --component missions --force-update

# Update VFX systems
python phase9_baseline_simple.py --component vfx --force-update
```

**2. Incremental updates**:
```python
# Add new mission types to existing set
python phase9_baseline_simple.py --add-missions transport,defense

# Add new VFX effects
python phase9_baseline_simple.py --add-vfx explosion,nebula
```

**3. Full regeneration** (nuclear option):
```bash
# Backup first!
git commit -am "Backup before baseline regeneration"

# Delete generated content
python phase9_baseline_simple.py --clean

# Regenerate everything
python phase9_baseline_simple.py --full-regenerate
```

**Best practices**:
- Always commit to Git before updates
- Test updates in separate branch
- Update one component at a time
- Profile performance after updates
- Document what changed in commit messages

**Version compatibility**: If updating to newer Alexander version:
1. Check CHANGELOG.md for breaking changes
2. Backup current project
3. Run migration script if provided
4. Regenerate baseline with new version

---

## Workflow Questions

### Should I create VFX or UI first?

**Recommended order**: Stations → Missions → VFX → UI → Testing

**Reasoning**:

**1. Stations first**:
- Provides context for missions
- Establishes spatial layout
- Defines docking points and zones

**2. Missions second**:
- Defines what VFX are needed
- Determines UI information requirements
- Establishes gameplay flow

**3. VFX third**:
- Depends on mission objectives (combat vs. transport)
- Needs station locations for environmental effects
- Should be performance-tested before UI added

**4. UI last**:
- Displays mission data created earlier
- Shows VFX intensity and status
- Integrates all previous systems

**Parallel work possible**:
- VFX and UI can be done simultaneously by different team members
- Stations and missions can overlap if mission templates are generic

**The script does this automatically**:
```python
# phase9_baseline_simple.py executes in optimal order:
1. Generate_stations()
2. Create_mission_templates()
3. Setup_VFX_systems()
4. Build_UI_components()
5. Run_integration_tests()
```

---

### Can I work on multiple maps simultaneously?

**Yes, but with caution.**

**Safe approach**:
1. Work on different map types (station vs. space)
2. Use separate Unreal levels
3. Commit each map completion to Git before starting next
4. Test each map independently

**Risks of parallel map work**:
- Shared assets may conflict (materials, blueprints)
- Performance budget unclear until all maps tested together
- Mission system dependencies across maps
- Harder to track which map has which features

**Recommended workflow**:
```
Week 1: Station Map Alpha (complete + test + commit)
Week 2: Station Map Beta (complete + test + commit)
Week 3: Open Space Map (complete + test + commit)
Week 4: Integration testing of all maps
```

**Team workflow** (multiple people):
- Person A: Station interiors
- Person B: Open space areas
- Person C: UI that works in both
- Daily: Share asset library updates
- Weekly: Integration build and test

**Script support**:
```python
# Generate specific maps only
python phase9_baseline_simple.py --maps station_alpha,station_beta

# Or generate all then work on specific ones
python phase9_baseline_simple.py --full
# Then manually iterate on individual maps in Unreal Editor
```

---

### How often should I profile?

**Profiling frequency depends on phase**:

**Baseline generation**: Profile once after complete baseline is generated.
```python
python phase9_baseline_simple.py --profile-after-generation
```

**Active development**: Profile after every major change.
- Added new mission type: Profile
- Created new VFX system: Profile
- Modified station layout significantly: Profile
- Changed UI complexity: Profile

**Daily during intensive development**: Start/end of day profiles.
```bash
# Morning: Establish baseline
python autonomous_profiling_test.py --quick

# Evening: Check what changed during day
python autonomous_profiling_test.py --compare-to-baseline
```

**Before commits**: Always profile before committing major changes.
```bash
# Pre-commit workflow
python autonomous_profiling_test.py
# Review results
git add .
git commit -m "Added combat missions - profiling shows 75 FPS average"
```

**Continuous integration**: Automated profiling on every push.
```yaml
# .github/workflows/profile.yml
on: [push]
jobs:
  profile:
    runs-on: windows-latest
    steps:
      - run: python autonomous_profiling_test.py
      - upload: profiling_results.json
```

**Red flags requiring immediate profiling**:
- Visual hitching or stuttering
- Frame rate drops in VR
- Increased load times
- Memory warnings
- Any "something feels slow" observations

---

### When should I commit to Git?

**Commit frequently with meaningful messages.**

**Minimum commit points**:
1. After initial baseline generation
2. After each major component completion (stations, missions, VFX, UI)
3. Before any risky changes
4. After successful testing
5. End of each work session

**Recommended commit workflow**:

**After baseline generation**:
```bash
git add .
git commit -m "Initial autonomous baseline generation - 3 stations, 10 missions, VFX, UI"
```

**After component completion**:
```bash
git add Content/Maps/StationAlpha*
git commit -m "Complete Station Alpha - 5 docking ports, 3 interior zones"
```

**Before risky changes**:
```bash
git commit -am "Pre-refactor checkpoint - current system working at 80 FPS"
git branch feature/mission-refactor  # Work on branch
```

**After testing**:
```bash
git commit -am "Combat missions tested - all objectives functional, 72 FPS average"
```

**Don't commit**:
- Unreal intermediate files (Intermediate/, Saved/)
- Build artifacts (Binaries/ not meant for version control)
- Local settings (.vscode/, .vs/)
- Large binary files without Git LFS

**Good commit messages**:
```
GOOD: "Add mining missions with resource collection VFX - 68 FPS in VR"
BAD:  "Updated stuff"

GOOD: "Fix station docking collision - ships no longer clip through ports"
BAD:  "Fixed bug"

GOOD: "Optimize particle pooling - improved VFX frame time from 12ms to 4ms"
BAD:  "Performance improvements"
```

---

### How do I share with team members?

**Git-based workflow** (recommended):

**1. Setup repository**:
```bash
# Initial setup (project lead)
git init
git lfs install
git lfs track "*.uasset"
git lfs track "*.umap"
git add .gitattributes
git commit -am "Initial commit with Git LFS"
git remote add origin https://github.com/yourteam/alexander.git
git push -u origin master
```

**2. Team members clone**:
```bash
git clone https://github.com/yourteam/alexander.git
cd alexander
git lfs pull  # Download large binary files
```

**3. Daily workflow**:
```bash
# Start of day: Get latest
git pull origin master

# Work on your area
# ... make changes ...

# End of day: Share your work
git add .
git commit -m "Added station Beta interior zones"
git push origin master
```

**Branch workflow for parallel work**:
```bash
# Person A: Works on stations
git checkout -b feature/stations-alpha-beta

# Person B: Works on missions
git checkout -b feature/combat-missions

# Person C: Works on UI
git checkout -b feature/vr-ui

# Periodically: Merge to master
git checkout master
git merge feature/stations-alpha-beta
```

**File sharing** (alternative for small teams):
- Use cloud storage (Dropbox, Google Drive, OneDrive)
- Share entire project folder
- Risk of version conflicts
- Harder to track changes
- Not recommended for teams > 2 people

**Perforce** (enterprise option):
- Industry standard for game development
- Better handling of large binary files
- More complex setup than Git
- Recommended for teams > 5 people

---

## Performance Questions

### What FPS should I expect with baseline?

**Target frame rates with baseline (primitive geometry only)**:

**VR mode**:
- Meta Quest 2/3: 90 FPS (72 FPS acceptable)
- Valve Index: 120 FPS (90 FPS acceptable)
- PSVR2: 90 FPS minimum

**Desktop mode**:
- High-end PC: 120+ FPS
- Mid-range PC: 90-120 FPS
- Minimum spec: 60 FPS

**Typical baseline performance**:
- Empty space: 200+ FPS
- Single station view: 120-150 FPS
- Station interior: 90-120 FPS
- Heavy VFX scene: 75-90 FPS (VR)

**Performance budget breakdown**:
- Rendering: 8-10ms per frame (VR)
- Game logic: 2-3ms per frame
- VFX: 2-4ms per frame
- UI: 1-2ms per frame
- Physics: 1-2ms per frame
- Total: ~11ms = 90 FPS (VR target)

**If below targets**:
1. Profile to identify bottleneck
2. Check for debug builds (should use Development or Shipping)
3. Verify VR settings (dynamic resolution enabled?)
4. Review VFX particle counts
5. Check UI update frequency

**Headroom for final assets**:
Baseline should run 20-30% faster than target to allow for:
- Detailed meshes replacing primitives
- High-res textures
- Complex materials
- Additional gameplay systems

---

### What happens if I add too many particles?

**Symptoms of particle overload**:
- Frame rate drops when VFX play
- Stuttering during combat or engine trails
- VR judder or reprojection
- Increased GPU temperature
- Profiler shows VFX consuming >5ms per frame

**Automatic protections in baseline**:
```cpp
// Particle systems use pooling and limits
MaxParticles = 1000;  // Per system
MaxActiveSystems = 20;  // Simultaneous effects
CullingDistance = 5000;  // Hide distant particles
```

**If you exceed limits**:
1. Older particles despawn early
2. New emitters refuse to spawn (warning logged)
3. Distant effects automatically cull
4. LOD system reduces particle density

**Manual mitigation**:
```python
# Reduce particle counts in VFX definitions
engine_trail_particles = 500  # Down from 1000
weapon_hit_particles = 200    # Down from 500

# Reduce active system count
max_simultaneous_explosions = 5  # Down from 10
```

**Performance monitoring**:
```cpp
// Check particle performance in profiler
Stat Particles  // Console command
// Look for:
// - Total particle count < 50,000
// - Particle update time < 4ms
// - GPU particle time < 3ms
```

**Best practices**:
- Use texture sheets instead of many particles
- Set aggressive culling distances
- Pool and reuse particle systems
- Reduce emission rates for background effects
- Test on minimum-spec hardware early

---

### How do I know if a system is too slow?

**Profiling indicators**:

**Frame time budget**:
- VR: Each frame must complete in <11ms (90 FPS)
- Desktop: Each frame should complete in <16ms (60 FPS)

**Warning signs**:
```
GameThread time > 8ms: Gameplay logic too heavy
RenderThread time > 8ms: Too much draw calls or complex shaders
GPU time > 9ms: Too many particles, complex materials, or geometry
```

**Specific system budgets**:
```
Mission system: <1ms per frame
VFX manager: <3ms per frame
UI updates: <1ms per frame
Physics: <2ms per frame
AI/NPCs: <2ms per frame
```

**Using the profiler**:
```bash
# Run automated profiling
python autonomous_profiling_test.py

# Check END_TO_END_TEST_REPORT.json
{
  "systems": {
    "MissionSystem": {
      "avg_frame_time_ms": 0.8,  // GOOD
      "max_frame_time_ms": 2.1   // OK
    },
    "VFXManager": {
      "avg_frame_time_ms": 4.2,  // WARNING: Approaching 5ms budget
      "max_frame_time_ms": 8.7   // BAD: Spikes above budget
    }
  }
}
```

**Console commands in Unreal**:
```
stat unit          // Shows frame, game, render, GPU times
stat fps           // Shows current FPS
stat particles     // Particle system performance
stat game          // Gameplay system performance
```

**Optimization triggers**:
- Any system consistently >50% of budget: Investigate
- Any system occasionally >100% of budget: Optimize now
- Total frame time >90% of budget: Stop adding features, optimize

---

### Can I optimize before replacing assets?

**Yes, and you should!**

**Benefits of early optimization**:
1. Establish performance baselines
2. Identify algorithmic inefficiencies
3. Prove optimization techniques work
4. Learn profiling tools
5. Build performance culture

**What to optimize in baseline phase**:

**Game logic**:
- Mission system update frequency
- UI update loops
- Physics simulation complexity
- AI behavior tree efficiency

**VFX**:
- Particle emission rates
- Pooling and reuse strategies
- Culling distances
- LOD transitions

**UI**:
- Widget update frequency
- Data binding efficiency
- Material complexity
- Draw call batching

**What NOT to optimize yet**:
- Rendering primitives (will change with real assets)
- Texture sizes (placeholders now)
- Mesh LODs (primitives don't need them)
- Lighting (will change with final art)

**Example optimization workflow**:
```python
# 1. Profile baseline
python autonomous_profiling_test.py
# Result: VFX at 4.2ms average

# 2. Optimize VFX pooling
# Edit Source/Alexander/Private/VFXManager.cpp
# Increase pool sizes, improve reuse

# 3. Profile again
python autonomous_profiling_test.py
# Result: VFX at 2.1ms average (50% improvement!)

# 4. Commit optimization
git commit -am "Optimize VFX pooling - reduced frame time from 4.2ms to 2.1ms"
```

**Performance headroom goal**:
With baseline optimizations, target 50% faster than required:
- VR target: 90 FPS → Optimize baseline to 135 FPS
- Desktop target: 60 FPS → Optimize baseline to 90 FPS

This headroom accommodates final asset complexity.

---

### What's the performance budget for each system?

**Total frame budget** (VR @ 90 FPS):
- Total available: 11.1ms per frame
- Reserve for variance: 2.1ms
- Usable budget: 9.0ms

**System-by-system budget**:

**Rendering: 4.0ms**
- Geometry: 1.5ms
- Materials: 1.0ms
- Lighting: 0.5ms
- Post-processing: 1.0ms

**Game logic: 2.5ms**
- Mission system: 0.8ms
- Physics: 0.8ms
- AI/NPCs: 0.6ms
- Player input: 0.3ms

**VFX: 1.5ms**
- Particle simulation: 0.8ms
- Particle rendering: 0.5ms
- VFX pooling/management: 0.2ms

**UI: 0.8ms**
- Widget updates: 0.4ms
- UI rendering: 0.3ms
- Data binding: 0.1ms

**Audio: 0.2ms**
- Sound cue evaluation
- 3D audio spatialization

**Total: 9.0ms** (leaves 2.1ms headroom)

**Desktop budgets** (60 FPS = 16.6ms):
- Scale all budgets by 1.85x
- Example: VFX budget becomes 2.8ms

**Monitoring budgets**:
```cpp
// Unreal console commands
stat unit          // Total frame breakdown
stat game          // Game logic systems
stat particles     // VFX performance
stat slate         // UI performance
```

**Budget violations**:
```python
# Example profiling output showing violation
{
  "VFXManager": {
    "avg_ms": 2.8,  // OVER BUDGET: 1.5ms target
    "max_ms": 5.2,  // SPIKE: Nearly 2x budget
    "recommendation": "Reduce particle emission rates or increase culling"
  }
}
```

---

## Content Questions

### How many missions are included?

**Baseline mission count**: 10-15 missions across multiple types.

**Mission type breakdown**:

**Transport missions** (3-4 missions):
- Deliver cargo between stations
- Time-sensitive deliveries
- Multi-stop routes
- Fragile cargo handling

**Combat missions** (2-3 missions):
- Defend station from attackers
- Clear pirate ambush
- Escort vulnerable ships

**Mining/harvesting missions** (2-3 missions):
- Collect asteroid resources
- Harvest gas clouds
- Return resources to station

**Exploration missions** (2-3 missions):
- Scout unknown sectors
- Scan anomalies
- Discover hidden stations

**Hybrid missions** (1-2 missions):
- Rescue + combat
- Delivery + exploration
- Mining + defense

**Tutorial missions** (1-2 missions):
- Learn basic flight
- Learn docking
- Learn combat basics

**Mission difficulty progression**:
- Tier 1 (Easy): 4 missions
- Tier 2 (Medium): 4 missions
- Tier 3 (Hard): 3 missions
- Tier 4 (Expert): 1-2 missions

**Mission rewards**:
- Credits: 500 - 5000 per mission
- Experience: 100 - 1000 XP
- Reputation: Increases with mission difficulty
- Unlocks: Some missions unlock new areas or missions

**Generated content location**:
```
Content/Data/Missions/
  ├── Transport/
  ├── Combat/
  ├── Mining/
  ├── Exploration/
  └── Tutorial/
```

---

### Can I add custom missions?

**Yes! Multiple ways to add missions:**

**1. Scripted generation** (easiest):
```python
# Create custom mission template
from mission_generator import MissionTemplate

custom_mission = MissionTemplate(
    name="Rescue Scientists",
    type="rescue",
    objectives=[
        {"type": "navigate", "target": "DistressBeacon"},
        {"type": "dock", "target": "DamagedStation"},
        {"type": "defend", "duration": 120, "enemies": 5},
        {"type": "transport", "passengers": 3, "destination": "SafeStation"}
    ],
    rewards={"credits": 2500, "xp": 500, "reputation": 100}
)

# Add to mission pool
python phase9_baseline_simple.py --add-custom-mission rescue_scientists.json
```

**2. Data asset creation** (more control):
```cpp
// Create in Unreal Editor
// Content Browser → Right-click → Miscellaneous → Data Asset
// Choose UMissionTemplateDataAsset

// Fill in properties:
MissionName = "Pirate Ambush"
MissionType = EMissionType::Combat
ObjectiveChain = [...]
Rewards = FMissionRewards(Credits=3000, XP=750)
```

**3. Blueprint extension** (most flexible):
```
// Create Blueprint class inheriting from BP_MissionBase
// Implement custom logic in Blueprint graphs
// Add to mission pool via MissionBoardComponent
```

**Mission system integration**:
All custom missions automatically integrate with:
- Mission board UI
- Objective tracking
- Reward distribution
- Progression system
- Save/load system

**Testing custom missions**:
```python
# Test specific mission
python test_content_integration.py --mission "Rescue Scientists"

# Profile mission performance
python autonomous_profiling_test.py --test-mission "Rescue Scientists"
```

**Mission templates location**:
```
Content/Data/Missions/Templates/
  └── [YourCustomMission].uasset
```

---

### How do I modify station layouts?

**Three approaches**:

**1. Regenerate with different parameters**:
```python
# Edit phase9_baseline_simple.py
STATION_CONFIGS = {
    "Alpha": {
        "size": "large",        # small/medium/large/huge
        "docking_ports": 8,     # Changed from 5
        "interior_zones": 5,    # Changed from 3
        "style": "industrial"   # industrial/commercial/military
    }
}

# Regenerate
python phase9_baseline_simple.py --component stations --force-update
```

**2. Manual editing in Unreal**:
```
1. Open station map in Unreal Editor
2. Select station CSG components
3. Use transform tools to reposition
4. Add/remove CSG primitives as needed
5. Update collision volumes
6. Test docking points still work
7. Save map
```

**3. Procedural modification script**:
```python
import unreal

# Load station asset
station = unreal.EditorAssetLibrary.load_asset("/Game/Maps/StationAlpha")

# Modify programmatically
station_actor = unreal.EditorLevelLibrary.get_actor_reference("StationAlpha_Main")

# Add new docking port
new_port = add_docking_port(station_actor, location=(1000, 0, 500))

# Save changes
unreal.EditorAssetLibrary.save_asset("/Game/Maps/StationAlpha")
```

**Key components to update when modifying**:
- Collision volumes (for docking)
- Navigation mesh (for interior walking)
- Light sources (for visibility)
- Trigger volumes (for zone detection)
- Spawn points (for NPCs/players)

**Testing modified stations**:
```python
# Test docking still works
python test_content_integration.py --test station_docking

# Check for collision issues
python test_content_integration.py --test navigation
```

**Backup before modifying**:
```bash
git commit -am "Checkpoint before modifying Station Alpha layout"
git branch backup/station-alpha-original
```

---

### Can I change VFX colors?

**Yes, easily changed!**

**Method 1: Niagara parameter adjustment**:
```cpp
// In Unreal Editor
1. Open Niagara system: Content/VFX/EngineTrail
2. Find "Color" module
3. Adjust color gradient or solid color
4. Preview changes in real-time
5. Save asset

// Color options:
- Gradient: Smooth transitions
- Curve: Custom color animation
- Parameter: Expose for Blueprint control
```

**Method 2: Material instance parameters**:
```cpp
// Content/VFX/Materials/M_ParticleBase
1. Create Material Instance
2. Expose BaseColor parameter
3. Set color in instance
4. Apply to particle system
```

**Method 3: Scripted color changes**:
```python
# Batch update all engine trails to blue
import unreal

vfx_systems = unreal.EditorAssetLibrary.find_asset_data("/Game/VFX/")
for vfx in vfx_systems:
    system = vfx.get_asset()
    if "EngineTrail" in system.get_name():
        # Set color parameter
        system.set_vector_parameter("TrailColor", (0.2, 0.5, 1.0, 1.0))  # Blue
        unreal.EditorAssetLibrary.save_asset(vfx.object_path)
```

**Method 4: Blueprint runtime control**:
```cpp
// BP_Spaceship.cpp or Blueprint
// Expose color to designer

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
FLinearColor EngineTrailColor = FLinearColor::Blue;

// Apply at runtime
EngineTrailComponent->SetColorParameter("TrailColor", EngineTrailColor);
```

**Team/faction color coding**:
```cpp
// Auto-assign colors based on faction
switch (ShipFaction) {
    case EFaction::Player:
        TrailColor = FLinearColor::Green;
        break;
    case EFaction::Ally:
        TrailColor = FLinearColor::Blue;
        break;
    case EFaction::Enemy:
        TrailColor = FLinearColor::Red;
        break;
    case EFaction::Neutral:
        TrailColor = FLinearColor::Yellow;
        break;
}
```

**Performance consideration**:
- Changing colors doesn't impact performance
- Dynamic color changes have negligible cost
- Use material instances to avoid material bloat

---

### How do I add new UI widgets?

**Step-by-step process**:

**1. Create widget Blueprint**:
```
Unreal Editor:
1. Content Browser → Right-click → User Interface → Widget Blueprint
2. Name: WBP_YourWidgetName
3. Open in Widget Designer
```

**2. Design widget layout**:
```
1. Drag UI elements from Palette:
   - Text blocks for labels
   - Progress bars for status
   - Buttons for interaction
   - Images for icons

2. Arrange in layout:
   - Canvas Panel: Absolute positioning
   - Vertical/Horizontal Box: Auto-layout
   - Grid Panel: Table-like layout
   - Overlay: Layered elements
```

**3. Add data binding**:
```cpp
// Widget Blueprint → Graph
// Create binding functions

UFUNCTION(BlueprintCallable)
FText GetShieldStatus() {
    if (PlayerShip) {
        float shield_pct = PlayerShip->GetShieldPercent();
        return FText::Format(
            LOCTEXT("ShieldStatus", "Shields: {0}%"),
            FText::AsPercent(shield_pct)
        );
    }
    return FText::GetEmpty();
}

// Bind in Designer:
// Text block → Text → Bind → GetShieldStatus
```

**4. Integrate with UI system**:
```cpp
// In HUD or UI manager Blueprint/C++
void AMyHUD::BeginPlay() {
    Super::BeginPlay();

    // Create widget
    UYourWidget* NewWidget = CreateWidget<UYourWidget>(
        GetWorld(),
        YourWidgetClass
    );

    // Add to viewport
    NewWidget->AddToViewport(ZOrder);
}
```

**5. VR adaptation** (if needed):
```cpp
// Create 3D widget component version
UPROPERTY(EditAnywhere)
UWidgetComponent* VR_WidgetComponent;

// In BeginPlay:
VR_WidgetComponent->SetWidgetClass(YourWidgetClass);
VR_WidgetComponent->SetDrawSize(FVector2D(1920, 1080));
VR_WidgetComponent->SetWorldLocation(CockpitLocation);
```

**Scripted widget generation**:
```python
# Add to baseline generation
def create_custom_widget(widget_name, elements):
    widget_blueprint = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name=f"WBP_{widget_name}",
        package_path="/Game/UI/Custom",
        asset_class=unreal.WidgetBlueprint,
        factory=unreal.WidgetBlueprintFactory()
    )

    # Add elements programmatically
    for element in elements:
        add_widget_element(widget_blueprint, element)

    return widget_blueprint

# Usage:
create_custom_widget("CargoStatus", [
    {"type": "text", "text": "Cargo Hold"},
    {"type": "progressbar", "binding": "CargoCapacity"},
    {"type": "button", "text": "Jettison", "onclick": "JettisonCargo"}
])
```

**Widget locations**:
```
Content/UI/
  ├── HUD/              (Always visible UI)
  ├── Menus/            (Pause, settings, etc.)
  ├── MissionBoard/     (Mission selection)
  ├── VR/               (VR-specific widgets)
  └── Custom/           (Your additions)
```

---

## Troubleshooting Questions

### Script won't run - what do I check?

**Troubleshooting checklist**:

**1. Python environment**:
```bash
# Check Python version
python --version
# Should be 3.8+

# Check required packages
pip list | grep unreal
# Should show unreal package

# Reinstall if missing
pip install -r requirements.txt
```

**2. Unreal Python API enabled**:
```
Unreal Editor:
1. Edit → Plugins
2. Search "Python"
3. Ensure "Python Editor Script Plugin" is enabled
4. Restart editor if you just enabled it
```

**3. Script permissions**:
```bash
# Windows: Check file isn't read-only
# Right-click script → Properties → Uncheck "Read-only"

# Check file exists
dir phase9_baseline_simple.py
```

**4. Working directory**:
```bash
# Ensure you're in project root
cd c:\Users\allen\Desktop\Alexander\Alexander

# Verify with:
dir Alexander.uproject
# Should show the .uproject file
```

**5. Unreal Editor state**:
```
# Script requires editor to be:
- Running (not closed)
- Not cooking or building
- Not in PIE (Play In Editor) mode
- Project fully loaded

# Check console for errors
Window → Developer Tools → Output Log
```

**6. Common error messages**:

```python
# Error: "No module named 'unreal'"
# Fix: Install Unreal Python API
pip install unreal

# Error: "Permission denied"
# Fix: Close Unreal Editor, run script, then reopen

# Error: "Cannot find project file"
# Fix: Check UPROJECT_PATH in script points to correct location

# Error: "Timeout waiting for Unreal"
# Fix: Increase timeout in script or ensure editor isn't frozen
```

**7. Enable debug logging**:
```python
# Add to top of script
import logging
logging.basicConfig(level=logging.DEBUG)

# Run script again to see detailed output
python phase9_baseline_simple.py
```

**8. Test with minimal script**:
```python
# Create test_unreal.py
import unreal

print("Unreal Python API working!")
print(f"Unreal version: {unreal.SystemLibrary.get_engine_version()}")

# Run:
python test_unreal.py
# Should print version number
```

---

### No missions showing - why?

**Diagnosis steps**:

**1. Check mission generation completed**:
```bash
# Look for generated mission files
dir Content\Data\Missions\*.json
# Should show multiple mission JSON files

# Check log output
# Look in phase9_baseline_simple.py output for:
# "Generated 10 missions successfully"
```

**2. Verify mission board component**:
```cpp
// In Unreal Editor
1. Open station map
2. Search for "MissionBoard" in World Outliner
3. Select MissionBoardComponent
4. Check Details panel:
   - MissionDataPath: Should point to Content/Data/Missions/
   - Auto-load missions: Should be checked
   - Debug mode: Enable temporarily
```

**3. Check mission data loading**:
```cpp
// Blueprint or C++ debugging
void UMissionBoardComponent::BeginPlay() {
    Super::BeginPlay();

    // Add debug logging
    UE_LOG(LogTemp, Warning, TEXT("Mission board initializing..."));

    LoadMissionsFromPath(MissionDataPath);

    UE_LOG(LogTemp, Warning, TEXT("Loaded %d missions"), AvailableMissions.Num());

    // If 0, missions didn't load!
}
```

**4. UI display issues**:
```
1. Open mission board UI widget (WBP_MissionBoard)
2. Check if ListView is bound to mission array
3. Enable widget reflector: Ctrl+Shift+W
4. Check if list items are being created but invisible
```

**5. Common causes**:

**Missions not generated**:
```bash
# Regenerate missions only
python phase9_baseline_simple.py --component missions --force-update
```

**Wrong file path**:
```cpp
// MissionBoardComponent.cpp
// Check path is absolute, not relative
MissionDataPath = "/Game/Data/Missions";  // NOT relative path
```

**Data table not loaded**:
```cpp
// Check data table reference is valid
if (!MissionDataTable) {
    UE_LOG(LogTemp, Error, TEXT("Mission data table is null!"));
    return;
}
```

**Filtering too aggressive**:
```cpp
// Check mission filters
MinPlayerLevel = 1;  // Not 100!
RequiredReputation = 0;  // Not requiring high reputation
ShowCompletedMissions = true;  // Show all for debugging
```

**6. Manual verification**:
```cpp
// Console command in Unreal
ListAllMissions
// Should output all loaded missions

// Or Blueprint command
Get Mission Board Component → Get Available Missions → Length
// Should be > 0
```

---

### FPS is low - where do I start?

**Systematic performance investigation**:

**Step 1: Measure baseline**:
```bash
# Run automated profiling
python autonomous_profiling_test.py

# Check END_TO_END_TEST_REPORT.json
# Identify which systems are slow
```

**Step 2: Identify bottleneck type**:
```cpp
// In Unreal Editor console
stat unit

// Look at the numbers:
// Frame: Total frame time (this is your FPS)
// Game: Game thread (CPU - logic)
// Draw: Render thread (CPU - draw call submission)
// GPU: GPU time (GPU - actual rendering)

// The highest number is your bottleneck!
```

**Step 3: If Game thread is slowest** (logic bottleneck):
```cpp
stat game  // Show game thread breakdown

// Common culprits:
// - Too many tick functions
// - Complex Blueprint logic in tick
// - Unoptimized AI
// - Heavy physics simulation

// Solutions:
// 1. Reduce tick frequency
// 2. Move logic to timers instead of tick
// 3. Optimize Blueprint nodes
// 4. Reduce physics complexity
```

**Step 4: If Draw thread is slowest** (draw call bottleneck):
```cpp
stat rhi  // Show rendering hardware interface stats

// Common culprits:
// - Too many draw calls (many small objects)
// - Complex materials
// - Too many dynamic lights

// Solutions:
// 1. Merge static meshes
// 2. Use instancing for repeated objects
// 3. Simplify materials
// 4. Use static lighting where possible
```

**Step 5: If GPU is slowest** (rendering bottleneck):
```cpp
stat gpu  // Show GPU breakdown

// Common culprits:
// - Too many particles
// - Expensive post-processing
// - High-resolution textures
// - Complex shaders

// Solutions:
// 1. Reduce particle counts
// 2. Disable or reduce post-processing
// 3. Lower texture quality temporarily
// 4. Simplify material shaders
```

**Quick wins for baseline**:

```cpp
// 1. Disable expensive features temporarily
r.VolumetricFog 0
r.Bloom 0
r.MotionBlur 0
r.DepthOfField 0

// 2. Reduce particle limits
fx.MaxParticleSystems 20  // Down from 50

// 3. Increase culling aggression
r.ViewDistanceScale 0.7  // Cull distant objects sooner

// 4. Lower resolution (temporarily)
r.ScreenPercentage 80  // Render at 80% resolution
```

**Profiling specific systems**:
```python
# Profile just VFX
python autonomous_profiling_test.py --system vfx

# Profile just missions
python autonomous_profiling_test.py --system missions

# Profile just UI
python autonomous_profiling_test.py --system ui
```

**If still unclear**:
```cpp
// Capture detailed profile trace
1. Console: stat startfile
2. Play for 30 seconds
3. Console: stat stopfile
4. Analyze .ue4stats file in Unreal Frontend
```

---

### UI not updating - common causes?

**Troubleshooting UI update issues**:

**Cause 1: Binding not configured**:
```cpp
// Check widget Blueprint
// Text/Image → Details → Binding
// Should show function name (green "Bind" button)

// If not bound:
1. Click "Bind" → Create Binding
2. Write binding function
3. Compile and save
```

**Cause 2: Binding function returns stale data**:
```cpp
// WRONG: Caching data in binding
UFUNCTION(BlueprintPure)
FText GetShieldText() {
    static FText CachedText = FText::FromString("100%");  // NEVER UPDATES!
    return CachedText;
}

// RIGHT: Get fresh data every call
UFUNCTION(BlueprintPure)
FText GetShieldText() {
    if (PlayerShip) {
        float ShieldPct = PlayerShip->GetShieldPercentage();
        return FText::Format(LOCTEXT("Shield", "{0}%"), FText::AsNumber(ShieldPct));
    }
    return FText::GetEmpty();
}
```

**Cause 3: Widget not ticking**:
```cpp
// Widget Blueprint → Graph → Event Construct
// Check if:
SetCanTick(true);  // Widget needs to tick for bindings to update

// Or for performance, manual refresh:
void AMyHUD::UpdateUI() {
    if (ShieldWidget) {
        ShieldWidget->RefreshAllBindings();  // Force update
    }
}
```

**Cause 4: Data source not broadcasting changes**:
```cpp
// Data source needs to notify of changes
UPROPERTY(BlueprintAssignable)
FOnShieldChangedSignature OnShieldChanged;

// When shield changes:
void ASpaceship::TakeDamage(float Damage) {
    CurrentShield -= Damage;
    OnShieldChanged.Broadcast(CurrentShield);  // Notify listeners!
}

// Widget subscribes:
void UShieldWidget::NativeConstruct() {
    Super::NativeConstruct();

    PlayerShip->OnShieldChanged.AddDynamic(this, &UShieldWidget::HandleShieldChanged);
}
```

**Cause 5: Widget not in viewport**:
```cpp
// Check widget is actually added
void AMyHUD::BeginPlay() {
    Super::BeginPlay();

    MyWidget = CreateWidget<UMyWidget>(GetWorld(), WidgetClass);

    // MUST call AddToViewport!
    MyWidget->AddToViewport();

    // Verify it's there:
    UE_LOG(LogTemp, Warning, TEXT("Widget in viewport: %d"), MyWidget->IsInViewport());
}
```

**Cause 6: Z-order / occlusion**:
```cpp
// Widget might be behind something else
MyWidget->AddToViewport(10);  // Higher number = in front

// Or another widget is blocking it
// Check widget hierarchy in UMG designer
```

**Cause 7: VR-specific issues**:
```cpp
// Widget component not facing player
VRWidget->SetWorldRotation(FRotator(0, 180, 0));  // Face player

// Widget component not set to screen space for VR
VRWidget->SetWidgetSpace(EWidgetSpace::World);  // For 3D widgets in VR

// Widget size too small in VR
VRWidget->SetDrawSize(FVector2D(1920, 1080));  // Large enough to read
```

**Debugging tools**:
```cpp
// Widget Reflector: Ctrl+Shift+W
// Shows all active widgets and their hierarchy
// Can inspect which widgets are updating

// Console command:
WidgetReflector  // Opens reflector

// Slow motion to watch updates:
slomo 0.1  // Slow to 10% speed to see if updates happen
```

**Quick test**:
```cpp
// Add obvious changing value to widget
UFUNCTION(BlueprintPure)
FText GetTestTime() {
    return FText::FromString(FString::Printf(TEXT("Time: %.2f"), GetWorld()->GetTimeSeconds()));
}

// If this updates, binding works
// If this doesn't update, widget/binding broken
```

---

### VFX not visible - checklist?

**Systematic VFX troubleshooting**:

**1. VFX system exists and spawns**:
```cpp
// Check VFX spawning code
UNiagaraComponent* VFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
    GetWorld(),
    EngineTrailSystem,  // Check this is not nullptr!
    SpawnLocation,
    SpawnRotation
);

// Add logging:
if (!VFX) {
    UE_LOG(LogTemp, Error, TEXT("Failed to spawn VFX!"));
} else {
    UE_LOG(LogTemp, Warning, TEXT("VFX spawned at %s"), *SpawnLocation.ToString());
}
```

**2. VFX within view distance**:
```cpp
// Check culling distance
// Select Niagara system in Content Browser
// Details → System → Max Distance
// Should be large enough: 5000+ for engine trails

// Or disable culling temporarily:
VFXComponent->SetMaxDistance(999999.f);
```

**3. VFX emitting particles**:
```cpp
// Open Niagara system in editor
// Check Emitter → Spawn Rate
// Should be > 0

// Check particle lifetime
// Emitter → Lifetime: Should be > 0 seconds

// Enable debug visualization:
fx.Niagara.Debug.Enabled 1  // Console command
```

**4. Particles have size**:
```cpp
// Niagara system → Particle Update → Scale Sprite Size
// Particle Size should be > 0
// Typical range: 10-100 for visible particles

// Check scale multipliers:
// Ensure no multiplier is 0
```

**5. Material renders correctly**:
```cpp
// Check particle material:
// Niagara system → Sprite Renderer → Material
// Should reference valid material

// Material should:
// - Have Emissive color (particles usually emissive)
// - Blend mode: Translucent or Additive
// - Shading model: Unlit (for particles)

// Test with default material:
M_DefaultLitParticle
```

**6. VFX layer/visibility**:
```cpp
// Check VFX isn't on hidden layer
// World Outliner → Layers
// Ensure VFX layer is visible (eye icon)

// Check VFX actor visibility:
VFXComponent->SetVisibility(true);
VFXComponent->SetHiddenInGame(false);
```

**7. Camera can see VFX**:
```cpp
// Are you looking in the right direction?
// Add marker at VFX location:
DrawDebugSphere(
    GetWorld(),
    VFXLocation,
    100.f,
    12,
    FColor::Red,
    false,
    5.f
);
// If you see red sphere, camera can see location
```

**8. VFX pooling not starved**:
```cpp
// Check pool availability
int32 Available = VFXManager->GetAvailablePoolSize();
if (Available == 0) {
    UE_LOG(LogTemp, Warning, TEXT("VFX pool exhausted!"));
    // Increase pool size or reduce concurrent effects
}
```

**9. Performance culling**:
```cpp
// Check if VFX disabled due to performance
r.Niagara.AllowCulling 0  // Disable performance culling temporarily

// Check significance manager:
fx.Niagara.Significance.Enabled 0  // Disable significance culling
```

**10. VR-specific issues**:
```cpp
// Check stereo rendering:
vr.InstancedStereo 1  // Ensure instanced stereo enabled

// Check VFX renders in both eyes:
// Ensure VFX isn't in UI layer (only renders in one eye)

// Increase VFX scale for VR visibility:
VFXComponent->SetWorldScale3D(FVector(2.0f));  // 2x larger for VR
```

**Quick visibility test**:
```cpp
// Spawn known-good VFX
UNiagaraComponent* TestVFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
    GetWorld(),
    LoadObject<UNiagaraSystem>(nullptr, TEXT("/Niagara/Systems/NS_DefaultParticle")),
    PlayerLocation + PlayerForward * 500.f  // Right in front of player
);

// If you see this, Niagara works
// If not, Niagara plugin or project issue
```

**Console commands for debugging**:
```cpp
fx.Niagara.Debug.Enabled 1           // Show particle bounds
fx.Niagara.Debug.Verbosity 2         // Detailed logging
stat Particles                        // Show particle counts
r.Niagara.ShowAllEmitters 1          // Force all emitters active
```

---

## Summary

This FAQ covers the most common questions and issues developers encounter when working with the Alexander autonomous baseline system. For deeper technical details, refer to:

- **KNOWLEDGE_BASE.md**: Conceptual understanding and best practices
- **GLOSSARY.md**: Term definitions and reference
- **BUILD_INSTRUCTIONS_PHASE9.md**: Step-by-step setup guide
- **PHASE9_STATUS_REPORT.md**: Current implementation status

For issues not covered here, check:
- Unreal Engine documentation: docs.unrealengine.com
- Project GitHub issues: [repository URL]/issues
- Community Discord: [invite link]

**Getting help**:
1. Search this FAQ first
2. Check relevant documentation file
3. Search project GitHub issues
4. Create new issue with:
   - What you were trying to do
   - What happened instead
   - Error messages (full text)
   - Profiling data if performance-related
   - Steps to reproduce

Happy prototyping!
