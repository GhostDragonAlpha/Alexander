# SolarSystem Level - Testing & Validation Checklist

## Overview
Comprehensive testing guide for validating the celestial scaling system implementation in the SolarSystem test level.

---

## Pre-Testing Setup

### Verify C++ Compilation
- [ ] Project compiles without errors
- [ ] All celestial scaling modules load successfully
- [ ] No warnings related to celestial scaling code
- [ ] Game launches to main menu

### Verify Blueprint Creation
- [ ] BP_CelestialBodyBase exists and compiles
- [ ] BP_Sun exists and compiles
- [ ] BP_Earth exists and compiles
- [ ] BP_Moon exists and compiles
- [ ] BP_SolarSystemGameMode exists and compiles
- [ ] BP_VRSpaceshipPlayer exists and compiles
- [ ] WBP_Chronometer exists and compiles
- [ ] WBP_DebugOverlay exists and compiles
- [ ] WBP_TimeWarpWarning exists and compiles

### Verify Level Setup
- [ ] SolarSystem.umap opens without errors
- [ ] Game Mode Override set to BP_SolarSystemGameMode
- [ ] All celestial bodies placed at correct coordinates
- [ ] PlayerStart placed 10 km from Moon
- [ ] Lighting configured (DirectionalLight + skybox)
- [ ] PostProcessVolume configured for space

---

## Phase 1: Subsystem Initialization Tests

### Test 1.1: Game Mode Initialization
**Purpose:** Verify all subsystems initialize correctly

**Steps:**
1. Open SolarSystem.umap
2. Click Play (Alt+P)
3. Check Output Log

**Expected Results:**
```
LogTemp: === Solar System Game Mode Initializing ===
LogTemp: CelestialBodyRegistry initialized
LogTemp: GravitySimulator initialized: Mode=MultiBody, MaxG=50.000000
LogTemp: RelativitySystem initialized: Enabled=True
LogTemp: ScalingCalculator initialized
LogTemp: SafetySystem initialized
LogTemp: Player registered with RelativitySystem
LogTemp: Debug overlay created
LogTemp: Chronometer widget created
LogTemp: === Solar System Game Mode Ready ===
LogTemp: Registered Bodies: 3
```

**Pass Criteria:**
- [ ] All subsystems initialize without errors
- [ ] Registered body count matches number of celestial bodies in level
- [ ] No null pointer warnings
- [ ] Initialization completes in < 1 second

---

### Test 1.2: Celestial Body Registration
**Purpose:** Verify all celestial bodies auto-register with CelestialBodyRegistry

**Steps:**
1. Play level
2. Open console (`~`)
3. Type: `DumpRegisteredBodies` (if console command exists)
4. Or check Output Log for registration messages

**Expected Results:**
```
LogCelestialScaling: Registered body: Sun (Mass: 1.98892e+30 kg, Radius: 696000.000000 km)
LogCelestialScaling: Registered body: Earth (Mass: 5.97237e+24 kg, Radius: 6371.000000 km)
LogCelestialScaling: Registered body: Moon (Mass: 7.34200e+22 kg, Radius: 1737.400000 km)
```

**Pass Criteria:**
- [ ] All bodies register on BeginPlay
- [ ] Body properties match AstronomicalConstants values
- [ ] No duplicate registrations
- [ ] Bodies unregister on EndPlay/destroy

---

### Test 1.3: Player Spawn and VR Initialization
**Purpose:** Verify VR pawn spawns correctly with all components

**Steps:**
1. Ensure VR headset is connected (or test in desktop mode)
2. Play level
3. Check player location
4. Verify components in World Outliner

**Expected Results:**
- Player spawns at (1,051,000,000, 0, 0) cm = 10 km from Moon
- VROrigin component exists at (0, 0, 0) relative to pawn
- VRCamera component exists and is active
- LeftHand and RightHand motion controllers exist
- OriginManager component exists and is initialized
- TimeComponent exists and is registered

**Pass Criteria:**
- [ ] Player spawns at correct location
- [ ] VR components present (even if HMD not connected)
- [ ] OriginManager has valid virtual position
- [ ] TimeComponent shows initial age (25.0 years)
- [ ] No component initialization errors

---

## Phase 2: Celestial Scaling Tests

### Test 2.1: Initial Scale Rendering
**Purpose:** Verify bodies render at correct initial scales

**Steps:**
1. Play level (spawns 10 km from Moon)
2. Observe visual appearance
3. Use free camera to view each body

**Expected Visual Results:**
- **Moon (10 km away):** Large sphere filling significant portion of view
  - Apparent angular size: ~20 degrees
  - Clearly visible surface features
- **Earth (510 km away):** Medium-sized sphere
  - Apparent angular size: ~1.4 degrees
  - Visible as blue planet
- **Sun (10,510 km away):** Bright disk in distance
  - Apparent angular size: ~7.6 degrees
  - Very bright emissive glow

**Pass Criteria:**
- [ ] All bodies visible from spawn point
- [ ] Relative sizes appear correct
- [ ] No z-fighting or rendering artifacts
- [ ] Bodies don't flicker or disappear

---

### Test 2.2: Dynamic Scaling on Approach
**Purpose:** Verify scale updates smoothly as player approaches body

**Steps:**
1. Play level
2. Use console command or VR controls to move toward Moon
3. Monitor scale changes
4. Stop at 1 km from Moon surface
5. Observe Moon size
6. Continue to 100 m from surface

**Expected Behavior:**
- Moon gradually increases in visual size
- At 1 km: Moon fills most of field of view
- At 100 m: Moon surface dominates view, can see terrain detail
- Scale transitions are smooth (no popping)
- Frame rate remains stable (>60 FPS desktop, >90 FPS VR)

**Pass Criteria:**
- [ ] Scaling is smooth and continuous
- [ ] No sudden jumps in scale
- [ ] Far clip plane doesn't cut off body
- [ ] Performance remains acceptable
- [ ] Moon doesn't scale too small at close range

**Debug Info to Check (if WBP_DebugOverlay visible):**
- Distance to Moon decreases smoothly
- ScaleFactor increases appropriately
- CurrentScale updates every frame

---

### Test 2.3: Multi-Body Scaling Consistency
**Purpose:** Verify all bodies scale correctly relative to each other

**Steps:**
1. Teleport to Earth-Moon midpoint: `teleport 1025000000 0 0`
2. Observe both Earth and Moon
3. Verify relative sizes match real-world ratios

**Expected Results:**
- Earth (Radius: 6,371 km) should appear ~3.7× larger than Moon (Radius: 1,737 km)
- Both bodies visible simultaneously
- Scaling ratio remains constant regardless of player position

**Pass Criteria:**
- [ ] Earth visually larger than Moon
- [ ] Size ratio approximately correct
- [ ] Both bodies render without culling issues
- [ ] No rendering priority conflicts

---

### Test 2.4: Extreme Distance Scaling
**Purpose:** Verify scaling works at very large distances

**Steps:**
1. Teleport far from system: `teleport 3000000000 0 0` (30,000 km from Sun)
2. Look back toward solar system
3. Observe Sun, Earth, Moon appearances

**Expected Results:**
- Sun still visible as bright star/disk
- Earth barely visible (small dot if visible at all)
- Moon not visible (too small)
- Stars/skybox visible in background

**Pass Criteria:**
- [ ] Sun remains visible at extreme distance
- [ ] Smaller bodies correctly become invisible when too far
- [ ] No rendering errors at extreme ranges
- [ ] Far clip plane set appropriately (>10M km)

---

## Phase 3: Origin Management Tests

### Test 3.1: Virtual Position Tracking
**Purpose:** Verify player virtual position updates correctly

**Steps:**
1. Play level
2. Enable WBP_DebugOverlay (should show by default if bShowDebugOverlay=true)
3. Use VR controllers or console to move player
4. Monitor virtual position display

**Expected Debug Display:**
```
Virtual Position:
  Sector: (0, 0, 0)
  Local: (10510.000, 0.000, 0.000) km
  Distance from Origin: 10510.000 km
```

**Pass Criteria:**
- [ ] Virtual position updates in real-time
- [ ] Local offset increases as player moves
- [ ] Sector coordinates remain (0,0,0) until threshold exceeded
- [ ] Distance calculation is accurate

---

### Test 3.2: Universe Recentering
**Purpose:** Verify universe recenters when player exceeds threshold

**Steps:**
1. Note starting position (10,510 km from origin)
2. Move player toward origin (Sun) by 1,000 km
3. When virtual distance < threshold, expect NO recentering
4. Continue moving until distance from any sector boundary > 10 km
5. Recentering should NOT occur yet
6. Move >10 km from spawn in any direction
7. Verify universe recenters

**Expected Behavior:**
- VROrigin (player) stays at (0, 0, 0) in local space
- All celestial bodies translate relative to player
- Virtual position updates to reflect new sector
- No visible teleporting or stuttering
- VR tracking remains smooth

**Pass Criteria:**
- [ ] Recentering occurs at correct threshold (10 km from local origin)
- [ ] Player local position remains (0, 0, 0)
- [ ] All world actors move smoothly
- [ ] No frame drops during recentering
- [ ] VR tracking unaffected
- [ ] Virtual position correctly updated

**Debug Output Expected:**
```
LogPlayerOrigin: Universe recentering triggered
LogPlayerOrigin: Old virtual position: Sector(0,0,0) + (10500.0, 5000.0, 0.0)
LogPlayerOrigin: New virtual position: Sector(0,0,0) + (0.0, 0.0, 0.0)
LogPlayerOrigin: Translated 3 celestial bodies
```

---

### Test 3.3: Precision Maintenance at High Speeds
**Purpose:** Verify no precision loss during fast movement

**Steps:**
1. Possess spaceship or use high-speed movement
2. Accelerate to 1000 m/s
3. Monitor position tracking
4. Look for jitter or drift

**Pass Criteria:**
- [ ] No visible jitter in VR view
- [ ] Position precision maintained (sub-meter)
- [ ] Recentering happens smoothly during movement
- [ ] No accumulating drift over time

---

## Phase 4: Gravity Simulation Tests

### Test 4.1: Single Body Gravity
**Purpose:** Verify gravity from single body affects player/spaceship

**Steps:**
1. Play level (near Moon)
2. Disable spaceship thrust (if applicable)
3. Observe player movement
4. Monitor velocity and acceleration

**Expected Behavior:**
- Player/spaceship accelerates toward Moon
- Acceleration approximately: a = GM/r²
  - Moon: G=6.67430e-20, M=7.342e22, r=10 km
  - Expected: ~0.0049 m/s² (~0.0005 G)
- Acceleration increases as distance decreases

**Pass Criteria:**
- [ ] Gravity pulls player toward Moon
- [ ] Acceleration magnitude reasonable
- [ ] Direction points toward Moon center
- [ ] Gravity strengthens when closer

**Debug Info (WBP_DebugOverlay):**
```
Gravity Sources:
  Moon: 0.0049 m/s² (Distance: 10.000 km)
  Earth: 0.0002 m/s² (Distance: 510.000 km)
  Sun: < 0.0001 m/s² (Distance: 10510.000 km)
Total G-Force: 0.0005 G
```

---

### Test 4.2: Multi-Body Gravity
**Purpose:** Verify multiple gravity sources combine correctly

**Steps:**
1. Teleport to Earth-Moon midpoint: `teleport 1025000000 0 0`
2. Observe gravity from both Earth and Moon
3. Verify forces combine as vectors

**Expected Behavior:**
- Gravity from both Earth and Moon affect player
- Earth's gravity stronger (larger mass)
- Net force points more toward Earth
- Total force = vector sum of both

**Calculation:**
- Distance to Earth: ~250 km
- Distance to Moon: ~250 km
- Earth gravity: ~0.095 m/s²
- Moon gravity: ~0.012 m/s²
- Net force: Mostly toward Earth

**Pass Criteria:**
- [ ] Both gravity sources contribute
- [ ] Earth has stronger pull
- [ ] Net direction toward Earth
- [ ] Total G-force reasonable (<1 G)

---

### Test 4.3: G-Force Limiting
**Purpose:** Verify extreme gravity doesn't kill player

**Steps:**
1. Teleport very close to Sun: `teleport 100000000 0 0` (1000 km from Sun)
2. Observe gravity forces
3. Verify safety system limits force

**Expected Behavior:**
- Calculated gravity would be enormous
- SafetySystem clamps to MaxGForce (50 G)
- Player survives (no instant death)
- Warning message displays

**Pass Criteria:**
- [ ] G-force clamped to 50 G maximum
- [ ] Warning displayed to player
- [ ] No crash or physics explosion
- [ ] Player can escape if thrusting away

**Debug Output Expected:**
```
LogCelestialScaling: Warning: G-force exceeds safe limit! Calculated: 2834.5 G, Clamped to: 50.0 G
```

---

### Test 4.4: Gravity Mode Switching
**Purpose:** Verify gravity simulation modes work correctly

**Test A: SingleBody Mode**
1. Set GravityMode to SingleBody (closest body only)
2. Position between Earth and Moon
3. Verify only one body provides gravity

**Test B: MultiBody Mode**
1. Set GravityMode to MultiBody (sphere of influence)
2. Verify multiple bodies within SOI contribute
3. Verify distant bodies ignored

**Test C: NBody Mode**
1. Set GravityMode to NBody (all bodies always)
2. Verify all bodies contribute
3. Check performance impact

**Pass Criteria:**
- [ ] SingleBody: Only closest body affects player
- [ ] MultiBody: Bodies within SOI contribute, others don't
- [ ] NBody: All bodies contribute regardless of distance
- [ ] Mode switching works at runtime
- [ ] Performance acceptable in all modes

---

## Phase 5: Time Relativity Tests

### Test 5.1: Time Display at Normal Speed
**Purpose:** Verify chronometer displays correct times

**Steps:**
1. Play level
2. Wait 60 seconds real time
3. Observe WBP_Chronometer displays

**Expected Results:**
- Real Time: +60 seconds (matches system clock)
- Universal Time: +60 seconds (no time warp)
- Biological Age: +60 seconds (~0.000001 years)
- Time Debt: 0.000 years (no time warp used)

**Pass Criteria:**
- [ ] All three clocks update every frame
- [ ] Real and Universal time match at 1× speed
- [ ] Biological age increases correctly
- [ ] No drift between clocks

---

### Test 5.2: Velocity Time Dilation
**Purpose:** Verify special relativity effects on time

**Steps:**
1. Accelerate spaceship to high velocity (>1000 km/s, >0.003c)
2. Maintain speed for 60 seconds real time
3. Observe time dilation effect
4. Monitor chronometer

**Expected Behavior:**
- At 1000 km/s (~0.0033c):
  - Time dilation factor: ~0.999995
  - Very minimal effect (<0.001% slower)
- At 30,000 km/s (~0.1c):
  - Time dilation factor: ~0.995
  - Universal time: ~59.7 seconds
  - Biological age: ~59.7 seconds
  - Time debt starts accumulating

**Pass Criteria:**
- [ ] Time dilation occurs at relativistic speeds
- [ ] Formula correct: t' = t × √(1 - v²/c²)
- [ ] Biological age slows relative to universal time
- [ ] Time debt calculated correctly
- [ ] Effect is subtle at low speeds, dramatic at high speeds

**Debug Info:**
```
Velocity: 30000.0 km/s (0.100c)
Time Dilation: 0.995
Time Debt: 0.000014 years
```

---

### Test 5.3: Gravitational Time Dilation
**Purpose:** Verify general relativity effects on time

**Steps:**
1. Teleport to very close to Sun: `teleport 100000000 0 0` (1000 km from surface)
2. Observe time dilation effect
3. Wait 60 seconds real time
4. Monitor chronometer

**Expected Behavior:**
- Sun: Mass = 1.98892e30 kg, Radius = 696,000 km
- At 697,000 km from center (1000 km above surface):
  - Gravitational time dilation: t' ≈ 0.999998 × t
  - Minimal effect at this distance
- Closer to event horizon (if possible): More dramatic

**Pass Criteria:**
- [ ] Gravitational time dilation occurs near massive bodies
- [ ] Formula correct: t' = t × √(1 - 2GM/(rc²))
- [ ] Stronger effect closer to body
- [ ] Combines with velocity time dilation

---

### Test 5.4: Time Warp Warning System
**Purpose:** Verify time warp activation and warnings

**Steps:**
1. Attempt to activate time warp at 100× speed
2. Verify warning widget appears
3. Review predicted consequences
4. Confirm or cancel

**Expected Widget Display (WBP_TimeWarpWarning):**
```
⚠ TIME WARP WARNING ⚠

You are about to activate 100× time warp.

Current Status:
  Real Time: 12:30:45
  Universal Time: 12:30:45
  Your Age: 25.0342 years

Predicted After 10 Minutes (Real Time):
  Universal Time: +16.67 hours
  Your Age: +16.67 hours
  Time Debt: 0.0019 years

Severity: HIGH

Consequences:
  - Your friends/crew will age 16 hours
  - Orbital positions will change significantly
  - Missions may expire
  - Trade prices may shift

[Cancel] [Activate Time Warp]
```

**Pass Criteria:**
- [ ] Warning displays before activation
- [ ] Predictions are accurate
- [ ] Severity calculated correctly
- [ ] Cancel button works
- [ ] Activate button applies time warp

---

### Test 5.5: Time Debt Accumulation
**Purpose:** Verify time debt tracks correctly

**Steps:**
1. Activate 10× time warp
2. Wait 60 seconds real time (= 600 seconds universal time)
3. Deactivate time warp
4. Check time debt

**Expected Results:**
- Real time: +60 seconds
- Universal time: +600 seconds
- Biological age: +60 seconds (player ages at real time rate)
- Time debt: +(600-60)/31,557,600 = 0.0000171 years

**Pass Criteria:**
- [ ] Time debt calculated: (Universal - Biological) age
- [ ] Time debt persists after warp ends
- [ ] Time debt displayed on chronometer
- [ ] Time debt can accumulate over multiple warps

---

## Phase 6: Spaceship Integration Tests

### Test 6.1: Spaceship Possession
**Purpose:** Verify VR pawn can possess and control spaceship

**Steps:**
1. Play level
2. Verify spaceship exists in level (or spawns automatically)
3. Check possession in World Outliner
4. Test spaceship controls

**Expected Behavior:**
- BP_VRSpaceshipPlayer finds/spawns spaceship on BeginPlay
- Spaceship possessed successfully
- VR pawn and spaceship share same virtual position
- VR pawn stays at (0,0,0), spaceship moves with universe translation

**Pass Criteria:**
- [ ] Spaceship possession succeeds
- [ ] VR controls move spaceship
- [ ] Origin managers synchronized
- [ ] No double-translation during recentering

---

### Test 6.2: Spaceship Movement and Virtual Position
**Purpose:** Verify spaceship movement updates virtual position

**Steps:**
1. Note starting virtual position (10,510 km from Sun)
2. Thrust forward 1000 km
3. Monitor virtual position update
4. Verify celestial bodies remain at correct relative positions

**Pass Criteria:**
- [ ] Virtual position updates with spaceship movement
- [ ] Celestial bodies appear in correct relative positions
- [ ] No desync between spaceship and VR pawn
- [ ] Smooth movement, no stuttering

---

### Test 6.3: Networked Spaceship (Multiplayer)
**Purpose:** Verify networked spaceship replication

**Setup:**
1. Launch dedicated server
2. Connect two clients
3. Each player possesses a spaceship

**Tests:**
- [ ] Server sees both players' virtual positions
- [ ] Client 1 sees Client 2's spaceship at correct location
- [ ] Universe recentering synchronized across clients
- [ ] No desync or rubber-banding
- [ ] Network bandwidth reasonable (<50 kbps per player)

**Expected Network Traffic (per player):**
- Virtual position: 20 Hz × 16 bytes = 320 bytes/sec = 2.5 kbps
- Spaceship state: 20 Hz × 64 bytes = 1280 bytes/sec = 10 kbps
- Total: ~15 kbps per player

---

## Phase 7: VR-Specific Tests

### Test 7.1: VR Headset Tracking
**Purpose:** Verify VR headset tracking works correctly

**Prerequisites:** VR headset connected (Oculus, Vive, etc.)

**Steps:**
1. Click "VR Preview" button in editor
2. Put on headset
3. Look around cockpit
4. Move head in all directions

**Pass Criteria:**
- [ ] Head tracking smooth and responsive
- [ ] No latency or lag
- [ ] No drift over time
- [ ] VROrigin stays at (0,0,0)
- [ ] Camera rotation matches head rotation 1:1

---

### Test 7.2: VR Motion Controllers
**Purpose:** Verify VR controller tracking and interactions

**Steps:**
1. Start VR Preview
2. Observe both motion controllers
3. Move controllers around cockpit
4. Test grabbing virtual controls

**Pass Criteria:**
- [ ] Both controllers visible
- [ ] Controller tracking accurate (<1 cm error)
- [ ] Controller models render correctly
- [ ] Interaction raycasts work
- [ ] Haptic feedback functional (if implemented)

---

### Test 7.3: VR Comfort and Performance
**Purpose:** Verify VR comfort and no motion sickness triggers

**Steps:**
1. Play in VR for 10+ minutes
2. Perform various movements
3. Approach celestial bodies
4. Observe universe recentering
5. Monitor frame rate

**Pass Criteria:**
- [ ] Frame rate stable >90 FPS (>120 FPS ideal)
- [ ] No sudden camera movements
- [ ] Universe recentering doesn't cause disorientation
- [ ] Scaling changes don't cause nausea
- [ ] Cockpit provides fixed reference frame
- [ ] No VR comfort issues reported

**Performance Targets:**
- Desktop VR (Oculus Rift S, Vive): >90 FPS sustained
- High-end VR (Index, Reverb G2): >120 FPS sustained
- Frame time: <11.1 ms (90 FPS) or <8.3 ms (120 FPS)

---

## Phase 8: UI Widget Tests

### Test 8.1: Chronometer Widget Visibility
**Purpose:** Verify chronometer displays correctly

**Steps:**
1. Play level
2. Locate WBP_Chronometer on screen
3. Verify all elements visible

**Expected Display:**
```
⏰ CHRONOMETER

Real Time:      12:30:45
Universal Time: 12:30:45
Biological Age: 25.0342 years

Time Debt: 0.0000 years
Dilation: 1.000×
```

**Pass Criteria:**
- [ ] Widget visible on screen
- [ ] All text elements present
- [ ] Font size readable
- [ ] Updates every frame
- [ ] No UI overlap issues

---

### Test 8.2: Debug Overlay Functionality
**Purpose:** Verify debug overlay displays system status

**Steps:**
1. Ensure bShowDebugOverlay = true in game mode
2. Play level
3. Review debug information

**Expected Display:**
```
CELESTIAL SCALING DEBUG

Virtual Position:
  Sector: (0, 0, 0)
  Local: (10510.000, 0.000, 0.000) km

Registered Bodies: 3
  Sun: 10510.000 km, Scale: 0.015×
  Earth: 510.000 km, Scale: 0.125×
  Moon: 10.000 km, Scale: 1.740×

Gravity:
  Mode: MultiBody
  Sources: 2
  Total: 0.0051 m/s² (0.0005 G)

Performance:
  FPS: 120
  Frame Time: 8.3 ms
  Celestial Ticks: 3
```

**Pass Criteria:**
- [ ] All information accurate
- [ ] Updates in real-time
- [ ] Readable formatting
- [ ] No performance impact from debug rendering

---

### Test 8.3: Time Warp Warning Widget
**Purpose:** Verify time warp warning appears and functions

**Steps:**
1. Trigger time warp >10× (via console or UI)
2. Observe warning widget
3. Test Cancel button
4. Re-trigger and test Activate button

**Pass Criteria:**
- [ ] Warning appears for warp >10×
- [ ] No warning for warp <10×
- [ ] Predictions accurate
- [ ] Cancel prevents time warp
- [ ] Activate applies time warp correctly
- [ ] Modal (blocks other input)

---

## Phase 9: Performance and Optimization Tests

### Test 9.1: Frame Rate Under Load
**Purpose:** Verify acceptable performance with all systems active

**Test Scenarios:**
1. Idle near Moon: Target >120 FPS
2. Moving at high speed: Target >90 FPS
3. During universe recentering: Target >90 FPS (no drops)
4. With multiple bodies visible: Target >90 FPS

**Profiling Commands:**
```
stat fps
stat unit
stat scenerendering
stat game
```

**Pass Criteria:**
- [ ] Desktop: >60 FPS minimum, >90 FPS average
- [ ] VR: >90 FPS minimum, >120 FPS ideal
- [ ] Frame time: <16.6 ms (60 FPS) or <11.1 ms (90 FPS)
- [ ] No frame spikes >50 ms
- [ ] Smooth gameplay throughout

---

### Test 9.2: Memory Usage
**Purpose:** Verify reasonable memory consumption

**Steps:**
1. Launch level
2. Check memory stats: `stat memory`
3. Play for 10 minutes
4. Check for memory leaks

**Pass Criteria:**
- [ ] Total memory <4 GB (desktop) or <2 GB (VR optimized)
- [ ] No continuous memory increase (leaks)
- [ ] Stable memory after initialization
- [ ] Texture streaming working

---

### Test 9.3: Network Bandwidth (Multiplayer)
**Purpose:** Verify acceptable network usage

**Steps:**
1. Run server + 2 clients
2. Monitor network traffic: `stat net`
3. Perform various actions
4. Check bandwidth per player

**Pass Criteria:**
- [ ] Bandwidth <50 kbps per player average
- [ ] No bandwidth spikes >200 kbps
- [ ] Update rate 20 Hz
- [ ] Latency compensation working

---

## Phase 10: Edge Cases and Stress Tests

### Test 10.1: Extreme Distance Test
**Purpose:** Test system at billions of kilometers

**Steps:**
1. Teleport extremely far: `teleport 100000000000 0 0` (1,000,000 km from Sun)
2. Verify no crashes
3. Check precision maintained
4. Observe rendering

**Pass Criteria:**
- [ ] No crash or freeze
- [ ] Position precision maintained
- [ ] Celestial bodies render or cull appropriately
- [ ] Can return to normal distances

---

### Test 10.2: Rapid Universe Recentering
**Purpose:** Test recentering stability under stress

**Steps:**
1. Move rapidly back and forth across recentering threshold
2. Force recentering every frame (if possible)
3. Monitor stability

**Pass Criteria:**
- [ ] No crash or freeze
- [ ] No visual artifacts
- [ ] VR tracking remains stable
- [ ] Performance acceptable

---

### Test 10.3: Celestial Body Spawn/Despawn
**Purpose:** Test dynamic body addition/removal

**Steps:**
1. Spawn new celestial body at runtime
2. Verify registration
3. Destroy celestial body
4. Verify cleanup

**Pass Criteria:**
- [ ] New body registers automatically
- [ ] Scaling applies to new body
- [ ] Destroyed body unregisters
- [ ] No memory leaks
- [ ] No null pointer errors

---

### Test 10.4: Time Warp Extreme Values
**Purpose:** Test time warp at extreme speeds

**Test A: Very Slow Time (0.1×)**
1. Set time warp to 0.1×
2. Verify all systems work
3. Check for divide-by-zero errors

**Test B: Very Fast Time (10,000×)**
1. Set time warp to 10,000×
2. Verify safety warnings
3. Check for overflow errors
4. Monitor time debt accumulation

**Pass Criteria:**
- [ ] System handles 0.01× to 100,000× range
- [ ] No crashes or errors
- [ ] Time calculations remain accurate
- [ ] Safety limits apply

---

## Phase 11: Integration Tests

### Test 11.1: Full Gameplay Loop
**Purpose:** Test complete player experience

**Scenario:**
1. Spawn near Moon
2. Approach Moon to 1 km
3. Orbit Moon
4. Travel to Earth (510 km journey)
5. Approach Earth to 100 km
6. Activate 10× time warp
7. Return to Moon

**Pass Criteria:**
- [ ] All systems work together seamlessly
- [ ] No conflicts between subsystems
- [ ] Smooth player experience
- [ ] No crashes during full loop
- [ ] Performance acceptable throughout

---

### Test 11.2: Save/Load System
**Purpose:** Verify game state persists correctly

**Steps:**
1. Play to specific location
2. Accumulate time debt
3. Save game
4. Exit and reload
5. Verify state restored

**Pass Criteria:**
- [ ] Virtual position restored
- [ ] Time debt restored
- [ ] Biological age restored
- [ ] Celestial body states restored
- [ ] No data loss

---

## Phase 12: Regression Tests

### Test 12.1: Existing Systems Compatibility
**Purpose:** Verify celestial scaling doesn't break existing features

**Tests:**
- [ ] Spaceship flight mechanics still work
- [ ] Landing zones functional
- [ ] Colony building system unaffected
- [ ] Combat systems functional
- [ ] UI systems render correctly
- [ ] Mission system compatible

---

## Final Validation Checklist

Before marking implementation complete:

### Critical Features
- [ ] All celestial bodies render at correct scales
- [ ] Dynamic scaling works smoothly
- [ ] Origin management prevents precision loss
- [ ] Gravity simulation functional
- [ ] Time relativity system working
- [ ] VR tracking precise and comfortable
- [ ] UI widgets display correctly
- [ ] Performance meets targets (>90 FPS VR)

### Quality Metrics
- [ ] Zero crashes during 30-minute test session
- [ ] No memory leaks detected
- [ ] Network bandwidth acceptable (<50 kbps/player)
- [ ] VR comfort rating: No motion sickness
- [ ] Code compiles without warnings
- [ ] All Blueprints compile successfully

### Documentation
- [ ] All specification documents created
- [ ] Testing results documented
- [ ] Known issues logged
- [ ] Performance benchmarks recorded

---

## Issue Reporting Template

If issues found during testing:

```markdown
**Issue ID:** [Sequential number]
**Severity:** [Critical / High / Medium / Low]
**Component:** [Which subsystem]
**Test:** [Which test revealed the issue]

**Description:**
[Clear description of the problem]

**Steps to Reproduce:**
1. [Step 1]
2. [Step 2]
3. [Step 3]

**Expected Behavior:**
[What should happen]

**Actual Behavior:**
[What actually happens]

**Environment:**
- Editor Version: UE 5.6
- Platform: Windows/VR
- Build Config: Development/Shipping
- VR Headset: [If applicable]

**Logs/Screenshots:**
[Attach relevant information]

**Possible Fix:**
[If known]
```

---

**Created:** 2025-01-05
**Version:** 1.0
**Status:** Ready for testing
