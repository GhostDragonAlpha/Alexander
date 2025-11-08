# Ship Customization Testing Guide

**Date:** 2025-11-07
**Version:** 1.0
**Purpose:** Automated testing for Ship Customization + N-Body Physics integration

---

## Overview

This testing suite provides comprehensive automated tests to verify the Ship Customization System's integration with the n-body physics engine. The tests ensure that:

- Equipment changes correctly modify ship stats
- Stats affect physics behavior (acceleration, gravity)
- Network replication works correctly in multiplayer
- Progression and unlocking systems function properly
- Physics configurations apply correctly
- Loadout save/load works as expected

---

## Test Suite Structure

### Files Created

```
Alexander/
├── test_framework.py                          # Base testing framework
├── test_ship_customization_stats.py           # Test 1: Stats modification
├── test_ship_physics_integration.py           # Test 2: Physics effects
├── test_ship_customization_network.py         # Test 3: Network replication
├── test_physics_config_data_assets.py         # Test 4: Config data assets
├── test_ship_loadout_system.py                # Test 5: Loadout save/load
├── test_ship_progression.py                   # Test 6: Progression system
├── test_gravity_with_customization.py         # Test 7: Gravity + mass
├── run_ship_customization_tests.py            # Test runner
└── SHIP_CUSTOMIZATION_TESTING_GUIDE.md        # This file
```

---

## Prerequisites

### 1. Server Requirements

The AutomationAPIServer must be running and exposed via HTTP. This requires:

- `AutomationAPIServer` component added to GameMode
- Server listening on port 8080 (or configured port)
- Ship customization endpoints implemented

**Required Endpoints:**

```
POST /spawn_ship                    - Spawn ship with customization component
GET  /get_ship_stats/{ship_id}      - Get ship's current stats
POST /equip_part                    - Equip a part on a ship
POST /unequip_part                  - Unequip a part from a ship
POST /apply_physics_config          - Apply physics config to ship
GET  /get_position/{ship_id}        - Get ship position
GET  /get_velocity/{ship_id}        - Get ship velocity
POST /set_input                     - Set thrust/rotation input
POST /save_loadout                  - Save current loadout
POST /load_loadout                  - Load saved loadout
POST /add_xp                        - Add XP to player
GET  /get_player_level/{ship_id}    - Get player level
GET  /get_credits/{ship_id}         - Get player credits
POST /unlock_part                   - Unlock a part
DELETE /destroy_ship/{ship_id}      - Destroy a ship
GET  /status                        - Server status
```

### 2. Python Dependencies

```bash
pip install requests
```

### 3. Ship Parts Data

Ship parts must be defined in Data Tables:

**Example Parts (from SHIP_CUSTOMIZATION_INTEGRATION_GUIDE.md):**

- `Engine_TurboII_02` - High-thrust engine (+0.3 ThrustPower, +150 kg Mass)
- `Hull_Reinforced_01` - Heavy armor (+200 kg Mass, +50 HP HullIntegrity)
- `Thrusters_Enhanced_01` - Enhanced thrusters (+0.2 Acceleration)
- `Wings_Aerodynamic_01` - Aerodynamic wings
- `Shield_Basic_01` - Basic shield

**Physics Configs:**

- `Fighter` - Low mass, high thrust
- `Cargo` - High mass, low thrust

---

## Running Tests

### Option 1: Run All Tests

```bash
python run_ship_customization_tests.py
```

This will:
1. Run all 7 tests in sequence
2. Generate `SHIP_CUSTOMIZATION_TEST_REPORT.json`
3. Generate `SHIP_CUSTOMIZATION_TEST_REPORT.md`
4. Print summary to console

### Option 2: Run Individual Tests

```bash
# Test 1: Stats modification
python test_ship_customization_stats.py

# Test 2: Physics integration
python test_ship_physics_integration.py

# Test 3: Network replication
python test_ship_customization_network.py

# Test 4: Physics config data assets
python test_physics_config_data_assets.py

# Test 5: Loadout save/load
python test_ship_loadout_system.py

# Test 6: Progression system
python test_ship_progression.py

# Test 7: Gravity with customization
python test_gravity_with_customization.py
```

### Option 3: Custom Server URL

```python
from run_ship_customization_tests import run_all_tests

report = run_all_tests(server_url="http://192.168.1.100:8080")
```

---

## Test Descriptions

### Test 1: Ship Customization Stats

**File:** `test_ship_customization_stats.py`

**Purpose:** Verify that equipping parts changes ship stats correctly.

**Test Steps:**
1. Spawn ship with ShipCustomizationComponent
2. Get initial stats (Mass, ThrustPower, MaxVelocity)
3. Equip Engine part (should increase ThrustPower)
4. Verify ThrustPower increased
5. Equip Heavy Hull part (should increase Mass)
6. Verify Mass increased
7. Verify stats replicated to physics system

**Expected Result:**
- ThrustPower increases after equipping engine
- Mass increases after equipping heavy hull
- Stats accessible via API

**Common Failures:**
- `Failed to get stats` - ShipCustomizationComponent not present
- `ThrustPower did not increase` - Part data not configured correctly
- `Mass did not increase` - Stat application not working

---

### Test 2: Ship Physics Integration

**File:** `test_ship_physics_integration.py`

**Purpose:** Verify that ship stats actually affect physics behavior.

**Test Steps:**
1. Spawn two identical ships
2. Ship A: Baseline (stock engine)
3. Ship B: Upgraded engine (high thrust)
4. Apply same thrust input to both
5. Measure acceleration over 5 seconds
6. Verify Ship B accelerates faster
7. Test Mass effect: Add heavy armor to Ship A
8. Verify heavier ship has different physics

**Expected Result:**
- Ship B accelerates faster than Ship A (ThrustPower effect confirmed)
- Heavier ship shows increased mass in stats

**Physics Formula Verified:**
```
Acceleration = ThrustForce / Mass
ThrustForce = ThrustPower * BaseThrust
```

**Common Failures:**
- `Ship B did not accelerate faster` - ThrustPower not affecting physics
- `Failed to measure acceleration` - Velocity API not working
- Measurement noise - Extend duration parameter

---

### Test 3: Ship Customization Network

**File:** `test_ship_customization_network.py`

**Purpose:** Test multiplayer replication of ship stats.

**Test Steps:**
1. Simulate server + 2 clients
2. Client 1 equips parts
3. Verify Client 2 sees stat changes
4. Verify server has authoritative stats
5. Test late-join: Client 3 connects after customization
6. Verify Client 3 receives correct stats on join

**Expected Result:**
- Server sees stat changes immediately
- Clients receive replicated stats within 1 second
- Late-joining clients get current state

**Network Architecture:**
- Server is authoritative (validates all changes)
- RPCs used for customization requests
- `CurrentLoadout` is replicated property

**Common Failures:**
- `Client stats don't match server` - Replication not configured
- `Late-join client has incorrect stats` - Initial replication missing

---

### Test 4: Physics Config Data Assets

**File:** `test_physics_config_data_assets.py`

**Purpose:** Test UShipPhysicsConfig data asset application.

**Test Steps:**
1. Load "Fighter" physics config
2. Apply config to ship
3. Verify Mass, ThrustPower, MaxVelocity match config
4. Load "Cargo" physics config
5. Verify stats update correctly
6. Equip parts on top of config
7. Verify parts modify config base

**Expected Result:**
- Configs apply different base stats
- Parts add to config base (not replace)
- Fighter config: Low mass, high thrust
- Cargo config: High mass, low thrust

**Common Failures:**
- `Fighter config not available` - Data assets not created
- `Parts did not modify config base` - Stat calculation logic incorrect

---

### Test 5: Ship Loadout System

**File:** `test_ship_loadout_system.py`

**Purpose:** Test save/load of ship configurations.

**Test Steps:**
1. Equip 5 different parts
2. Save loadout with name "TestLoadout1"
3. Unequip all parts
4. Load "TestLoadout1"
5. Verify all 5 parts re-equipped
6. Verify stats match saved state

**Expected Result:**
- Loadout saves successfully
- Loading restores exact stat configuration
- Multiple loadout slots work

**Save Format:**
```cpp
struct FShipLoadout {
    FText LoadoutName;
    TMap<EShipPartCategory, FName> EquippedParts;
    FName EquippedSkin;
    FShipStats TotalStats;
};
```

**Common Failures:**
- `Failed to save loadout` - SaveGame system not configured
- `Loaded stats don't match saved` - Stat calculation inconsistent

---

### Test 6: Ship Progression

**File:** `test_ship_progression.py`

**Purpose:** Test unlock system and progression.

**Test Steps:**
1. Get initial level and credits
2. Add 500 XP (should trigger level up)
3. Verify level increased (1 → 2)
4. Verify credits awarded on level up
5. Unlock part with credits
6. Verify part can be equipped
7. Test level requirement (can't equip high-level part)

**Expected Result:**
- Level up occurs at XP threshold (Level² * 100)
- Credits awarded on level up (Level * 100)
- Parts unlock with credits
- Level requirements enforced

**XP Curve:**
```
Level 1: 0 XP
Level 2: 400 XP (2² * 100)
Level 3: 900 XP (3² * 100)
Level 4: 1600 XP (4² * 100)
```

**Common Failures:**
- `Level did not increase` - Progression system not active
- `Could not unlock part` - Part doesn't exist in data table

---

### Test 7: Gravity with Customization

**File:** `test_gravity_with_customization.py`

**Purpose:** Test n-body gravity with variable ship mass.

**Test Steps:**
1. Spawn planet (large mass)
2. Spawn ship 100km away
3. Measure gravitational acceleration with default mass (1000 kg)
4. Equip heavy armor (increase mass to ~1200 kg)
5. Measure gravitational acceleration again
6. Verify F = G * m1 * m2 / r² holds true
7. Note that acceleration is mass-independent

**Expected Result:**
- Gravitational acceleration measured successfully
- Acceleration remains constant (independent of ship mass)
- Force increases proportionally to mass

**Physics Verification:**
```
Force:        F = G * m1 * m2 / r²
Acceleration: a = F / m = (G * m1 * m2 / r²) / m2 = G * m1 / r²

Therefore: Acceleration is independent of ship mass (m2)
But:       Force increases with ship mass
```

**Gravitational Constant:**
```
G = 6.67430 × 10⁻¹¹ m³/(kg·s²)
```

**Common Failures:**
- `Could not measure gravitational acceleration` - Time too short or distance too large
- Measurement noise - Use longer observation periods

---

## Expected Test Output

### Successful Test Run

```
================================================================================
                    SHIP CUSTOMIZATION TEST SUITE
================================================================================

================================================================================
            Running Test 1: Ship Customization Stats (1/7)
================================================================================

[1/7] Checking server status...
[OK] Server is running

[2/7] Spawning ship with customization component...
[OK] Ship spawned: Ship_1

[3/7] Getting initial ship stats...
  Initial Mass: 1000.0 kg
  Initial ThrustPower: 1.0
  Initial MaxVelocity: 1.0

[4/7] Equipping Engine upgrade...
[OK] Part equipped

[5/7] Verifying ThrustPower increased...
  New ThrustPower: 1.3
  Change: +0.3

[6/7] Equipping Heavy Hull...
[OK] Part equipped

[7/7] Verifying Mass increased...
  New Mass: 1200.0 kg
  Change: +200.0 kg

[PASS] test_ship_customization_stats
Duration: 8.45s
Message: Ship stats updated correctly: ThrustPower 1.00 -> 1.30, Mass 1000 -> 1200 kg

================================================================================
                              TEST SUMMARY
================================================================================

Total Tests:    7
Passed:         7
Failed:         0
Success Rate:   100.0%
Total Duration: 92.34s

Individual Test Results:
--------------------------------------------------------------------------------
  [PASS] test_ship_customization_stats                        8.45s
  [PASS] test_ship_physics_integration                       15.23s
  [PASS] test_ship_customization_network                     11.67s
  [PASS] test_physics_config_data_assets                      9.88s
  [PASS] test_ship_loadout_system                            13.45s
  [PASS] test_ship_progression                                7.92s
  [PASS] test_gravity_with_customization                     25.74s
--------------------------------------------------------------------------------

[OK] JSON report saved to: SHIP_CUSTOMIZATION_TEST_REPORT.json
[OK] Markdown report saved to: SHIP_CUSTOMIZATION_TEST_REPORT.md

================================================================================
                         TEST SUITE COMPLETE
================================================================================

✅ All tests passed!
```

---

## Troubleshooting

### Server Not Responding

**Error:** `Server not responding`

**Solutions:**
1. Check if Unreal Editor is running
2. Verify AutomationAPIServer is enabled
3. Check firewall settings for port 8080
4. Test with: `curl http://localhost:8080/status`

### Ship Spawn Failures

**Error:** `Failed to spawn ship`

**Solutions:**
1. Verify ship blueprint exists at path
2. Check AutomationGameMode is active
3. Ensure map has spawn points
4. Check Unreal Editor logs for spawn errors

### Stats Not Updating

**Error:** `ThrustPower did not increase`

**Solutions:**
1. Verify ShipCustomizationComponent is on ship
2. Check part data table contains the part
3. Verify `EquipPart()` is being called
4. Check `ApplyStatsToFlightController()` is called after equip

### Network Replication Issues

**Error:** `Client stats don't match server`

**Solutions:**
1. Verify `CurrentLoadout` has `UPROPERTY(Replicated)`
2. Implement `GetLifetimeReplicatedProps()`
3. Add `SetIsReplicatedComponent(true)` in constructor
4. Test in multiplayer mode (not standalone)

### Progression Not Working

**Error:** `Level did not increase`

**Solutions:**
1. Verify progression system is enabled
2. Check XP calculation: `Level² * 100`
3. Ensure `CheckLevelUp()` is called after adding XP
4. Check SaveGame system is configured

---

## Performance Expectations

### Test Duration

- **Test 1 (Stats):** ~8 seconds
- **Test 2 (Physics):** ~15 seconds (includes 5s acceleration measurements)
- **Test 3 (Network):** ~12 seconds
- **Test 4 (Config):** ~10 seconds
- **Test 5 (Loadout):** ~13 seconds
- **Test 6 (Progression):** ~8 seconds
- **Test 7 (Gravity):** ~26 seconds (includes 3s gravity measurements)

**Total:** ~90 seconds for full suite

### Server Load

- CPU: <5% during tests
- Memory: <100 MB additional
- Network: <10 KB/s bandwidth

---

## Integration with CI/CD

### GitHub Actions Example

```yaml
name: Ship Customization Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v2

      - name: Setup Python
        uses: actions/setup-python@v2
        with:
          python-version: '3.9'

      - name: Install dependencies
        run: pip install requests

      - name: Start Unreal Server
        run: |
          Start-Process -FilePath "C:\UnrealEngine\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" `
            -ArgumentList "${{ github.workspace }}\Alexander.uproject -server -log"
          Start-Sleep -Seconds 30

      - name: Run Tests
        run: python run_ship_customization_tests.py

      - name: Upload Test Report
        uses: actions/upload-artifact@v2
        with:
          name: test-report
          path: SHIP_CUSTOMIZATION_TEST_REPORT.md
```

---

## Extending the Test Suite

### Adding a New Test

1. Create new test file: `test_my_feature.py`

```python
from test_framework import ShipCustomizationTestFramework, TestResult
import time

def test_my_feature(server_url: str = "http://localhost:8080") -> TestResult:
    framework = ShipCustomizationTestFramework(server_url)
    start_time = time.time()

    try:
        # Your test logic here

        return TestResult(
            "test_my_feature",
            True,
            "Test passed",
            time.time() - start_time
        )
    except Exception as e:
        return TestResult(
            "test_my_feature",
            False,
            f"Exception: {str(e)}",
            time.time() - start_time
        )
```

2. Add to `run_ship_customization_tests.py`:

```python
from test_my_feature import test_my_feature

tests = [
    # ... existing tests ...
    ("Test 8: My Feature", test_my_feature),
]
```

### Adding New Framework Methods

Edit `test_framework.py`:

```python
def my_new_method(self, ship_id: str) -> bool:
    """My new test utility method"""
    response = self._post("/my_endpoint", {"ship_id": ship_id})
    return response and response.get("success", False)
```

---

## API Reference

### ShipCustomizationTestFramework

**Core Methods:**

```python
# Server communication
check_server_status() -> bool
spawn_ship_with_customization() -> Optional[str]
destroy_ship(ship_id: str) -> bool

# Customization
get_ship_stats(ship_id: str) -> Optional[ShipStats]
equip_part(ship_id: str, category: str, part_id: str) -> bool
unequip_part(ship_id: str, category: str) -> bool
apply_physics_config(ship_id: str, config_name: str) -> bool

# Physics measurements
get_ship_position(ship_id: str) -> Optional[Tuple[float, float, float]]
get_ship_velocity(ship_id: str) -> Optional[Tuple[float, float, float]]
measure_acceleration(ship_id: str, thrust_direction, duration) -> Optional[float]
measure_gravitational_force(ship_id: str, planet_id: str, duration) -> Optional[float]

# Network testing
get_replicated_stats(ship_id: str, client_index: int) -> Optional[ShipStats]
verify_stat_replication(ship_id: str, expected_stats, tolerance) -> bool

# Progression
add_xp(ship_id: str, amount: int) -> bool
unlock_part(ship_id: str, part_id: str) -> bool
get_player_level(ship_id: str) -> Optional[int]
get_credits(ship_id: str) -> Optional[int]

# Loadouts
save_loadout(ship_id: str, loadout_name: str) -> bool
load_loadout(ship_id: str, loadout_index: int) -> bool

# Reporting
log_test_result(result: TestResult)
generate_report(output_file: str)
print_summary()
```

---

## Best Practices

### Test Design

1. **Independent Tests:** Each test should be self-contained
2. **Cleanup:** Always destroy spawned ships
3. **Timing:** Allow time for physics/replication (0.5-1s delays)
4. **Tolerance:** Use floating-point tolerance (±0.01 for stats)
5. **Fallbacks:** Don't fail if optional features aren't implemented

### Debugging Tests

1. **Enable Verbose Logging:**
   ```python
   framework.verbose = True
   ```

2. **Run Single Test:**
   ```bash
   python test_ship_customization_stats.py
   ```

3. **Check Server Logs:**
   - Look in `Saved/Logs/Alexander.log`
   - Search for "ShipCustomization" or "Equip"

4. **Add Print Statements:**
   ```python
   print(f"[DEBUG] Ship ID: {ship_id}")
   print(f"[DEBUG] Stats: {stats}")
   ```

### Common Pitfalls

1. **Not waiting for replication:** Add `time.sleep(1)` after network operations
2. **Forgetting to cleanup:** Always destroy ships in `finally` block
3. **Hardcoded expectations:** Use relative comparisons (ratio) not absolute
4. **Race conditions:** Ensure server is fully initialized before tests

---

## Summary

This testing suite provides comprehensive coverage of the Ship Customization System's integration with n-body physics. All 7 tests verify different aspects:

1. ✅ Stats modification
2. ✅ Physics integration
3. ✅ Network replication
4. ✅ Config data assets
5. ✅ Loadout system
6. ✅ Progression system
7. ✅ Gravity effects

**Total Lines of Code:** ~2,500 lines
**Test Coverage:** Core customization + physics integration
**Execution Time:** ~90 seconds for full suite

For questions or issues, refer to:
- `SHIP_CUSTOMIZATION_INTEGRATION_GUIDE.md` - Integration architecture
- `Source/Alexander/Public/ShipCustomizationComponent.h` - Component API
- `UNIFIED_GRAVITY_IMPLEMENTATION_SUMMARY.md` - Physics system

---

**END OF GUIDE**
