# Baseline Testing Guide

**Project:** Alexander - Space Simulation Game
**Phase:** 8 - Baseline Content Generation
**Purpose:** Comprehensive testing procedures for autonomous baseline content
**Last Updated:** November 8, 2025

---

## Table of Contents

1. [Pre-Testing Validation](#pre-testing-validation)
2. [Mission System Testing](#mission-system-testing)
3. [Flight Mechanics Testing](#flight-mechanics-testing)
4. [VFX System Testing](#vfx-system-testing)
5. [UI Widget Testing](#ui-widget-testing)
6. [Performance Testing](#performance-testing)
7. [Integration Testing](#integration-testing)
8. [Test Reporting](#test-reporting)

---

## Pre-Testing Validation

Before starting gameplay testing, validate that all baseline content is properly generated.

### Step 1: Run Validation Script

```python
# In Unreal Editor Python console:
import validate_baseline
report = validate_baseline.run_full_validation()
```

**Expected Results:**
- All 11 mission categories imported (100+ missions total)
- 5 space station blueprints generated
- VFX directory structure created
- UI directory structure created
- All content directories exist

### Step 2: Review Validation Report

Check the validation report in: `Saved/ValidationReports/baseline_validation_[timestamp].json`

**Pass Criteria:**
- 0 critical failures
- All mission data assets present
- Station blueprints available
- Directory structure complete

**If validation fails:** Follow remediation steps in validation report before proceeding.

---

## Mission System Testing

### Test 1: Mission Board Display

**Objective:** Verify mission board displays available missions

**Steps:**
1. Launch game in PIE (Play In Editor)
2. Navigate to space station
3. Interact with mission board terminal
4. Observe mission list

**Expected Results:**
- Mission board UI displays
- At least 5-10 missions visible
- Missions show title, description, reward
- Missions show difficulty indicator
- Filtering works (by type, difficulty)

**Pass Criteria:**
- UI responsive and readable
- Mission data populated correctly
- No visual artifacts or overlapping text

**Failure Actions:**
- Check mission data asset loading in logs
- Verify MissionBoardComponent is attached to actor
- Verify UI widget bindings

---

### Test 2: Accept Mission

**Objective:** Accept a mission and verify it's added to active missions

**Steps:**
1. Open mission board
2. Select a "Normal" difficulty mission
3. Click "Accept Mission" button
4. Close mission board
5. Open player's mission log (Journal/Quest UI)

**Expected Results:**
- Mission accepted confirmation message
- Mission appears in active missions list
- Mission objectives displayed
- Mission waypoint/marker appears in world (if applicable)
- Mission tracking UI updates

**Pass Criteria:**
- Mission state changes to "Active"
- Player can see all objectives
- UI updates correctly

**Failure Actions:**
- Check MissionSystem logs for accept() calls
- Verify PlayerState has MissionComponent
- Check mission objective serialization

---

### Test 3: Complete Mission Objective

**Objective:** Complete a mission objective and verify tracking

**Test Mission Types:**

#### Delivery Mission
1. Accept delivery mission
2. Pick up cargo from source station
3. Fly to destination station
4. Deliver cargo

**Expected:**
- Objective 1: "Pick up cargo" - completes when near source
- Objective 2: "Deliver to [destination]" - completes when delivered
- Progress bar updates

#### Combat Mission
1. Accept combat/bounty mission
2. Locate target enemy ship
3. Engage and destroy target
4. Return to station

**Expected:**
- Target marked on radar/HUD
- Combat triggers mission event
- Objective completes on target destruction
- Kill count increments

#### Mining Mission
1. Accept mining mission
2. Locate asteroid field
3. Mine required resources (e.g., 100 Iron Ore)
4. Return resources to station

**Expected:**
- Resource counter displays (0/100)
- Counter increments as resources mined
- Objective completes when quota reached

#### Exploration Mission
1. Accept exploration mission
2. Navigate to unexplored sector
3. Scan objects/locations as required
4. Return to station

**Expected:**
- Exploration targets marked
- Scan progress indicator
- Objective completes on scan completion

**Pass Criteria (All Mission Types):**
- Objective tracking accurate
- Progress updates in real-time
- UI reflects current status
- Completion triggers properly

---

### Test 4: Mission Completion and Rewards

**Objective:** Complete entire mission and receive rewards

**Steps:**
1. Complete all objectives of an active mission
2. Return to quest-giving station (if required)
3. Turn in mission
4. Check rewards received

**Expected Results:**
- Mission completion notification
- Credits added to player balance
- Experience points awarded (level up check)
- Items added to inventory (if applicable)
- Faction reputation updated
- Mission removed from active list
- Mission added to completed list

**Pass Criteria:**
- All rewards received correctly
- Amounts match mission template
- UI shows reward notification
- Player stats updated

**Failure Actions:**
- Check reward calculation logic
- Verify currency/inventory systems
- Check mission completion event binding

---

### Test 5: Mission Failure Conditions

**Objective:** Test mission failure scenarios

**Test Cases:**

#### Time Limit Expiration
1. Accept mission with time limit
2. Wait for timer to expire
3. Observe result

**Expected:**
- Timer displayed in mission UI
- Warning when <1 minute remaining
- Mission fails on expiration
- Failure notification shown
- Mission removed from active list

#### Failed Objectives
1. Accept escort mission
2. Allow escorted ship to be destroyed
3. Observe result

**Expected:**
- Mission fails immediately
- Failure reason displayed
- No rewards given
- Mission marked as failed

**Pass Criteria:**
- Failure conditions trigger correctly
- Player informed of failure reason
- Mission state updates properly

---

### Test 6: Procedural Mission Generation

**Objective:** Verify missions are generated with variable content

**Steps:**
1. Accept and complete a repeatable mission
2. Wait for cooldown period
3. Check mission board again
4. Accept same mission type
5. Compare details

**Expected Results:**
- Mission title contains different variable values
- Target names randomized
- Locations different
- Rewards vary slightly
- Each instance feels unique

**Pass Criteria:**
- At least 3/5 repeated missions have different variables
- Variables make sense (no placeholder text)
- Generation doesn't break mission logic

---

## Flight Mechanics Testing

### Test 7: Basic Flight with Mission Context

**Objective:** Test flight mechanics during mission gameplay

**Steps:**
1. Accept a delivery or exploration mission
2. Engage flight systems
3. Navigate to mission waypoint
4. Test all flight controls

**Flight Maneuvers to Test:**
- Forward thrust (W key)
- Reverse thrust (S key)
- Strafe left/right (A/D keys)
- Pitch up/down (Mouse Y)
- Yaw left/right (Mouse X / Q/E keys)
- Roll (Z/C keys)
- Boost (Shift)
- Brake (Space)

**Expected Results:**
- All controls responsive
- Ship physics feel realistic
- Mission waypoint tracking works during flight
- HUD updates with velocity, heading
- No control conflicts with UI

**Pass Criteria:**
- Smooth flight control
- 60+ FPS during flight
- Waypoint system functional
- No input lag

---

### Test 8: Combat Flight Mechanics

**Objective:** Test flight during combat missions

**Steps:**
1. Accept combat mission
2. Locate enemy target
3. Engage in dogfight
4. Test combat maneuvers

**Combat Maneuvers to Test:**
- Jousting (head-on pass)
- Pursuit curve
- Barrel roll evasion
- Boost-slide maneuver
- Circle strafe
- Vertical loop

**Expected Results:**
- Ship handles predictably under fire
- Weapon targeting stable
- Enemy ships maneuver believably
- No physics glitches during high-speed combat
- Damage feedback visual/audio

**Pass Criteria:**
- Engaging combat experience
- Controls responsive under stress
- No collision detection issues
- Combat feels balanced

---

### Test 9: Orbital Mechanics Integration

**Objective:** Verify flight works with orbital mechanics system

**Steps:**
1. Accept mission near planetary body
2. Approach planet/moon
3. Enter orbit
4. Navigate in orbital space

**Expected Results:**
- Gravity well affects ship velocity
- Orbital trajectory visualization
- Can establish stable orbit
- Can escape orbit with sufficient thrust
- Waypoints account for orbital motion

**Pass Criteria:**
- Orbital mechanics feel realistic
- Player can understand orbital motion
- Mission objectives achievable in orbital space

---

## VFX System Testing

### Test 10: Engine VFX During Flight

**Objective:** Verify engine particle effects display correctly

**Steps:**
1. Enter ship
2. Engage engines
3. Test thrust variations

**VFX to Verify:**
- Main engine thrust plume
- Maneuvering thruster bursts
- Boost afterburner effect
- Idle engine glow

**Expected Results:**
- VFX appear when thrusting
- VFX intensity scales with thrust amount
- Color/style matches ship design
- No particles clipping through ship geometry
- Performance remains stable (60+ FPS)

**Pass Criteria:**
- All engine VFX functional
- Visual quality high
- No performance degradation

---

### Test 11: Weapon VFX

**Objective:** Test combat visual effects

**Steps:**
1. Accept combat mission
2. Engage enemy
3. Fire weapons
4. Observe effects

**VFX to Verify:**
- Muzzle flash
- Projectile trail
- Impact sparks/explosion
- Shield hit effect
- Weapon charge-up glow

**Expected Results:**
- VFX synchronized with weapon fire
- Impacts appear at collision point
- Effect scale appropriate
- Color coding matches weapon type
- No VFX pooling issues (effects appear/disappear correctly)

**Pass Criteria:**
- Combat feels visually impactful
- All weapon types have VFX
- No missing or broken effects

---

### Test 12: Environmental VFX

**Objective:** Test space environment particle effects

**VFX to Verify:**
- Nebula clouds/particles
- Asteroid field dust
- Station lights/running lights
- Planetary atmospheric glow
- Starfield background

**Expected Results:**
- Environment feels alive
- VFX add to immersion
- No excessive particles causing FPS drop
- LOD system works (distant effects simplified)

**Pass Criteria:**
- Beautiful, immersive visuals
- Performance budget maintained
- No visual bugs (flickering, z-fighting)

---

### Test 13: VFX Triggering from Code

**Objective:** Verify VFX systems can be triggered programmatically

**Steps:**
1. Open VFXManager.cpp
2. Verify PROFILE_SCOPE integration
3. Trigger VFX from mission events

**Test Cases:**
- Mission waypoint reached (sparkle effect)
- Mission completed (celebration effect)
- Player damaged (spark shower)
- Entering hazard zone (warning particles)

**Expected Results:**
- VFX trigger on correct game events
- No delay between event and VFX
- VFX cleanup properly (no leaks)

**Pass Criteria:**
- Event-driven VFX work correctly
- VFXManager API responsive
- No performance issues

---

## UI Widget Testing

### Test 14: HUD Display During Flight

**Objective:** Verify heads-up display shows all necessary information

**HUD Elements to Test:**
- Velocity indicator
- Altitude (from nearest body)
- Shield/hull integrity bars
- Weapon status/ammo
- Radar/sensor display
- Target reticle
- Mission objective tracker
- Waypoint direction indicator

**Expected Results:**
- All elements visible and readable
- Updates in real-time
- No overlapping elements
- Scales properly at different resolutions
- VR-friendly layout (if applicable)

**Pass Criteria:**
- HUD provides all needed information
- Clean, professional appearance
- High readability

---

### Test 15: Mission UI Widgets

**Objective:** Test all mission-related UI screens

**Widgets to Test:**
1. **Mission Board UI**
   - List view of available missions
   - Mission details panel
   - Accept/Decline buttons
   - Filtering and sorting

2. **Mission Journal UI**
   - Active missions list
   - Mission objectives with checkboxes
   - Progress bars
   - Abandon mission option

3. **Mission Complete UI**
   - Completion notification
   - Rewards display
   - Continue button

**Expected Results:**
- Navigation smooth and intuitive
- Text readable, properly formatted
- Buttons respond to clicks
- Animations smooth (if any)
- Gamepad/keyboard navigation works

**Pass Criteria:**
- All UI functions work
- No visual glitches
- UX feels polished

---

### Test 16: Station UI

**Objective:** Test space station interaction menus

**Station UI Screens:**
1. **Docking Bay UI**
   - Ship selection
   - Repair/refuel options
   - Undock button

2. **Trading UI**
   - Buy/sell commodities
   - Inventory display
   - Price information

3. **Ship Outfitting UI**
   - Weapon/module shop
   - Install/uninstall equipment
   - Ship statistics preview

4. **Station Services**
   - Mission board
   - Faction representative
   - Shipyard

**Expected Results:**
- All screens accessible from station menu
- Transitions smooth
- Data persists between screens
- Back/cancel buttons work

**Pass Criteria:**
- Full station functionality available via UI
- Professional appearance
- No broken links or dead ends

---

### Test 17: UI Performance and Responsiveness

**Objective:** Verify UI doesn't cause performance issues

**Steps:**
1. Open mission board with 50+ missions
2. Scroll through list rapidly
3. Monitor frame rate
4. Open/close UI repeatedly

**Expected Results:**
- Frame rate stays above 60 FPS
- No stuttering when opening UI
- Scrolling smooth
- Textures load quickly

**Pass Criteria:**
- UI has minimal performance impact (<1ms frame time)
- No memory leaks on repeated open/close
- Responsive feel

---

## Performance Testing

### Test 18: Baseline Performance Profiling

**Objective:** Establish performance baseline metrics

**Setup:**
1. Build Development configuration
2. Run PIE (Play In Editor)
3. Let profiler run during gameplay

**Test Scenario:**
1. Start at station
2. Accept mission
3. Fly to waypoint (2-3 minutes flight)
4. Complete mission objective
5. Return to station
6. Complete mission

**Duration:** 5-10 minutes of gameplay

**Metrics to Capture:**
- Average FPS
- Frame time (milliseconds)
- Memory usage (current/peak)
- System-level timings:
  - Spaceship::Tick
  - FlightController::TickComponent
  - OrbitalMechanics::TickComponent
  - StarSystemManager::TickComponent
  - VFXManager (if instrumented)
  - UIManager (if instrumented)

**Expected Results:**
- Average FPS: 60+ (90+ target for VR)
- Frame time: <16.67ms (11.11ms for 90fps VR)
- No systems exceeding 8ms average
- Memory usage stable (no leaks)

**Pass Criteria:**
- Performance meets or exceeds targets
- No critical bottlenecks identified
- Smooth gameplay experience

**Report Location:** `Saved/Profiling/ProfileReport_[timestamp].json`

---

### Test 19: Stress Testing

**Objective:** Test performance under heavy load

**Stress Scenarios:**

#### Scenario 1: Many Active Missions
1. Accept 10 missions simultaneously
2. Navigate with all waypoints active
3. Monitor performance

**Expected:** Frame rate stays above 60 FPS

#### Scenario 2: Large Combat Engagement
1. Accept combat mission with multiple enemies
2. Engage 5+ enemy ships simultaneously
3. Use weapons, VFX, explosions
4. Monitor performance

**Expected:** Frame rate stays above 45 FPS (acceptable during combat)

#### Scenario 3: Complex Environment
1. Navigate to dense asteroid field
2. Near large planetary body
3. Space station visible
4. Multiple ships in area
5. Monitor performance

**Expected:** Frame rate stays above 60 FPS

**Pass Criteria:**
- Performance degradation graceful
- No hard crashes
- Recovery after stress scenario

---

### Test 20: Memory Leak Testing

**Objective:** Ensure no memory leaks in mission system

**Steps:**
1. Start PIE
2. Note initial memory usage
3. Accept mission
4. Complete mission
5. Note memory after completion
6. Repeat steps 3-5 for 10 missions
7. Compare initial vs final memory

**Expected Results:**
- Memory increases slightly during missions
- Memory returns to near-baseline after mission completion
- No runaway growth over multiple missions

**Pass Criteria:**
- Memory growth <50MB over 10 missions
- No sustained upward trend
- Clean mission cleanup

---

## Integration Testing

### Test 21: End-to-End Mission Flow

**Objective:** Complete full gameplay loop

**Full Test Scenario:**
1. Start new game
2. Complete tutorial (if implemented)
3. Dock at first station
4. Open mission board
5. Accept delivery mission
6. Undock from station
7. Navigate to pickup location
8. Pick up cargo
9. Navigate to delivery location
10. Deliver cargo
11. Return to quest-giving station
12. Complete mission and receive rewards
13. Use credits to buy ship upgrade
14. Accept combat mission
15. Complete combat mission
16. Verify progression (level up, faction rep)

**Duration:** 20-30 minutes

**Expected Results:**
- Entire loop works seamlessly
- No breaking bugs
- Player can understand what to do
- Progression feels rewarding

**Pass Criteria:**
- 0 critical bugs encountered
- Smooth gameplay experience
- Clear player objectives at each step

---

### Test 22: Save/Load Integration

**Objective:** Verify missions persist across save/load

**Steps:**
1. Accept 3 missions (different types)
2. Complete objectives of 1 mission
3. Save game
4. Exit to main menu
5. Load saved game
6. Check mission states

**Expected Results:**
- All 3 missions still active
- Progress preserved (completed objectives remain complete)
- Mission timers continue accurately
- Waypoints restored

**Pass Criteria:**
- Perfect mission state restoration
- No data loss
- Timers accurate

---

### Test 23: Multiplayer Considerations

**Objective:** Test mission system with multiplayer in mind (if applicable)

**Test Cases:**
1. **Shared Missions**
   - Two players accept same mission
   - Both can complete objectives
   - Both receive rewards

2. **Mission Ownership**
   - Player 1 accepts mission
   - Player 2 sees Player 1's mission waypoint
   - Only Player 1 can complete

3. **Mission Replication**
   - Mission state syncs across clients
   - Objective completion replicates
   - Rewards replicate correctly

**Expected Results:**
- No conflicts in mission system
- Multiplayer experience smooth
- Fair reward distribution

**Pass Criteria:**
- Multiplayer-ready architecture
- No duplication bugs
- Proper replication

---

### Test 24: Cross-System Integration

**Objective:** Verify all systems work together

**Integration Points:**
1. **Missions + Flight**
   - Waypoints guide flight
   - Objectives trigger during flight

2. **Missions + Combat**
   - Combat triggers mission events
   - Kill counts tracked correctly

3. **Missions + Economy**
   - Rewards add to currency
   - Purchased items affect mission capability

4. **Missions + Progression**
   - XP awards level player
   - Level gates mission access
   - Faction rep unlocks missions

5. **Missions + UI**
   - All mission data visible in UI
   - UI updates reflect mission state

**Expected Results:**
- No system conflicts
- Data flows correctly between systems
- Integrated experience feels cohesive

**Pass Criteria:**
- All integrations functional
- No data synchronization issues
- Seamless player experience

---

## Test Reporting

### Test Report Template

After each testing session, generate a report:

```
BASELINE TEST REPORT
Date: [YYYY-MM-DD]
Tester: [Name]
Build: [Version/Commit Hash]
Test Duration: [HH:MM]

SUMMARY:
- Tests Passed: X/Y
- Critical Issues: X
- Non-Critical Issues: X
- Performance: PASS/FAIL

TESTS EXECUTED:
[x] Test 1: Mission Board Display - PASS
[x] Test 2: Accept Mission - PASS
[ ] Test 3: Complete Objective - NOT TESTED
[x] Test 4: Mission Completion - FAIL (see issues)

ISSUES FOUND:

CRITICAL (Blocking):
1. [Issue Title]
   - Description: [Details]
   - Steps to Reproduce: [Steps]
   - Expected: [What should happen]
   - Actual: [What happened]
   - Impact: [How this affects gameplay]

NON-CRITICAL (Minor):
1. [Issue Title]
   - Description: [Details]
   - Severity: Low/Medium
   - Workaround: [If applicable]

PERFORMANCE METRICS:
- Average FPS: [X]
- Frame Time: [X ms]
- Memory Usage: [X MB]
- Bottlenecks: [System names >8ms]

RECOMMENDATIONS:
1. [Action item]
2. [Action item]

NEXT STEPS:
1. [What to test next]
2. [Blockers to resolve]
```

### Performance Report Template

```
BASELINE PERFORMANCE REPORT
Date: [YYYY-MM-DD]
Configuration: [Development/Shipping]
Test Scenario: [Description]

FRAME PERFORMANCE:
- Average FPS: [X]
- Min FPS: [X]
- Max FPS: [X]
- 1% Low: [X]
- Frame Time Avg: [X ms]

SYSTEM TIMINGS (Average):
- Spaceship::Tick: [X ms]
- FlightController: [X ms]
- OrbitalMechanics: [X ms]
- StarSystemManager: [X ms]
- VFXManager: [X ms]
- UIManager: [X ms]

MEMORY:
- Initial: [X MB]
- Peak: [X MB]
- Final: [X MB]
- Delta: [+/- X MB]

CRITICAL BOTTLENECKS (>8ms):
1. [System Name]: [X ms] - [Recommended action]

OPTIMIZATION RECOMMENDATIONS:
1. [System/Area]: [Specific optimization]
2. [System/Area]: [Specific optimization]

COMPARISON TO TARGETS:
- Target FPS: 90 (VR)
- Actual FPS: [X]
- Status: [PASS/FAIL]
- Frame Budget: 11.11ms
- Actual Avg: [X ms]
- Status: [PASS/FAIL]
```

---

## Automated Testing (Future)

### Automated Test Cases to Implement

1. **Mission Import Validation**
   - Script: `validate_baseline.py`
   - Frequency: On every build
   - PASS criteria: 11/11 mission files imported

2. **Performance Regression Testing**
   - Script: `performance_baseline.py`
   - Frequency: Daily/Weekly
   - PASS criteria: FPS within 5% of baseline

3. **Mission Completion Smoke Test**
   - Script: `mission_smoke_test.py`
   - Frequency: On every build
   - PASS criteria: Can accept and complete one mission

4. **UI Screenshot Comparison**
   - Script: `ui_visual_regression.py`
   - Frequency: On UI changes
   - PASS criteria: No unintended visual changes

---

## Troubleshooting Guide

### Common Issues and Solutions

#### Issue: Missions not appearing on mission board
**Solutions:**
1. Check mission import: `validate_baseline.run_full_validation()`
2. Verify MissionBoardComponent attached to actor
3. Check mission filtering logic (level requirements)
4. Ensure mission board UI widget properly bound

#### Issue: Mission objectives not completing
**Solutions:**
1. Add logging to objective completion code
2. Verify objective type matches player action
3. Check mission event bindings
4. Ensure MissionComponent ticking

#### Issue: Low FPS during flight
**Solutions:**
1. Run profiler: Check `Saved/Profiling/` for report
2. Check VFX particle counts
3. Verify LOD settings on ships/stations
4. Check draw call count (stat SceneRendering)

#### Issue: UI not responding to clicks
**Solutions:**
1. Check UI widget visibility in hierarchy
2. Verify input mode set to UI
3. Check event bindings in widget blueprint
4. Ensure no blocking widgets in front

#### Issue: VFX not appearing
**Solutions:**
1. Check VFXManager logs
2. Verify Niagara system assets exist
3. Check particle system bounds (culling)
4. Ensure VFX spawn location valid

---

## Testing Checklist Quick Reference

**Before Each Test Session:**
- [ ] Run validation: `validate_baseline.run_full_validation()`
- [ ] Check latest build successful
- [ ] Clear previous profiling data
- [ ] Note current git commit hash

**During Test Session:**
- [ ] Document each test performed
- [ ] Screenshot any visual bugs
- [ ] Note reproduction steps for issues
- [ ] Monitor performance metrics

**After Test Session:**
- [ ] Generate test report
- [ ] Export profiling data
- [ ] File bug reports for critical issues
- [ ] Update test documentation

---

## Success Criteria Summary

**Baseline Ready for Production:**
- [ ] All 24 test cases pass
- [ ] 0 critical bugs
- [ ] Average FPS > 60 (>90 for VR)
- [ ] Memory usage stable
- [ ] All mission types functional
- [ ] All UI screens working
- [ ] VFX system operational
- [ ] Integration testing complete
- [ ] Documentation complete

---

**Document Version:** 1.0
**Created:** November 8, 2025
**Author:** Autonomous Testing Agent
**Status:** Ready for Use
