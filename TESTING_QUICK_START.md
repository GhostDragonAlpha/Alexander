# Ship Customization Testing - Quick Start

## Prerequisites

1. **Server Running:** AutomationAPIServer on port 8080
2. **Python:** Python 3.x installed
3. **Dependencies:** `pip install requests`

## Run Tests

### All Tests (Recommended)
```bash
python run_ship_customization_tests.py
```

### Individual Tests
```bash
python test_ship_customization_stats.py
python test_ship_physics_integration.py
python test_ship_customization_network.py
python test_physics_config_data_assets.py
python test_ship_loadout_system.py
python test_ship_progression.py
python test_gravity_with_customization.py
```

## View Results

After running tests, check:
- **Console:** Immediate pass/fail status
- **SHIP_CUSTOMIZATION_TEST_REPORT.json:** Machine-readable results
- **SHIP_CUSTOMIZATION_TEST_REPORT.md:** Human-readable report

## Expected Output

```
================================================================================
                    SHIP CUSTOMIZATION TEST SUITE
================================================================================

Total Tests:    7
Passed:         7
Failed:         0
Success Rate:   100.0%
Total Duration: 92.34s

✅ All tests passed!
```

## If Tests Fail

1. Check server is running: `curl http://localhost:8080/status`
2. Verify endpoints exist (see SHIP_CUSTOMIZATION_TESTING_GUIDE.md)
3. Check Unreal logs: `Saved/Logs/Alexander.log`
4. Review test report for specific failures

## Documentation

- **SHIP_CUSTOMIZATION_TESTING_GUIDE.md** - Complete testing guide
- **SHIP_CUSTOMIZATION_TESTING_DELIVERABLES.md** - What was created
- **EXAMPLE_TEST_OUTPUT.txt** - Example successful run
- **test_framework.py** - API reference (docstrings)

## Test Overview

1. **Stats** - Equipment changes stats
2. **Physics** - Stats affect acceleration
3. **Network** - Multiplayer replication
4. **Configs** - Physics presets work
5. **Loadouts** - Save/load works
6. **Progression** - XP/levels/unlocks
7. **Gravity** - Mass affects n-body physics

## Need Help?

See troubleshooting section in SHIP_CUSTOMIZATION_TESTING_GUIDE.md
