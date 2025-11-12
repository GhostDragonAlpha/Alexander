# Alexander Verification Test Execution Guide

## Quick Start

1. **Compile the game** (if needed):
   ```bash
   "C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealBuildTool.exe" -projectfiles -project="Alexander.uproject" -game -engine
   ```

2. **Run verification tests**:
   ```bash
   python run_verification_tests.py
   ```

3. **Review results**:
   - Check `verification_report.json` for detailed results
   - AI will analyze and provide recommendations

## Test Scenarios

### Test 1: Game Launch
- Verifies game starts without crashing
- Checks main menu renders correctly
- Expected: Game process remains running after 30 seconds

### Test 2: Save/Load System
- Loads test_farm_save.sav
- Verifies farm plots and crops are present
- Expected: All 20 crop types loaded correctly

### Test 3: Orbital Mechanics
- Loads test_mining_save.sav
- Verifies asteroid field is present
- Tests orbital navigation
- Expected: Can navigate to and land on planet

### Test 4: Farming System
- Verifies crop growth stages
- Tests automated farming infrastructure
- Expected: All crops growing correctly

### Test 5: Mining System
- Verifies resource deposits
- Tests mining laser functionality
- Expected: Resources can be mined and collected

### Test 6: Crafting System
- Verifies refining recipes
- Tests crafting blueprints
- Expected: Items can be crafted successfully

### Test 7: Trading System
- Verifies market functionality
- Tests trade missions
- Expected: Trades execute correctly, credits update

### Test 8: Performance
- Monitors FPS throughout tests
- Tracks memory usage
- Expected: 90+ FPS, <8GB memory

## Expected Results

**All tests should PASS with:**
- 223/223 unit tests passing
- 25/25 integration tests passing
- 90+ FPS average
- <8GB memory peak
- No crashes during 1-hour stress test

## Test Save Files

### test_farm_save.sav
**Location:** `Saved/SaveGames/test_farm_save.sav`

**Setup:**
- Player on planet surface
- 4 farm plots (small, medium, large, huge)
- All 20 crop types planted at different growth stages
- Hydroponics bays and greenhouses built
- Automated irrigation and drones active

**Verification Points:**
- [ ] All 20 crop types present
- [ ] Farm plots correctly sized
- [ ] Automated systems functioning
- [ ] Resources being produced

### test_mining_save.sav
**Location:** `Saved/SaveGames/test_mining_save.sav`

**Setup:**
- Player ship near asteroid field
- 10 asteroids with different resource deposits
- Mining lasers equipped (all tiers)
- Cargo hold with mixed resources

**Verification Points:**
- [ ] Asteroid field rendered correctly
- [ ] Resource deposits visible
- [ ] Mining lasers functional
- [ ] Resources collectable

### test_trading_save.sav
**Location:** `Saved/SaveGames/test_trading_save.sav`

**Setup:**
- Player at space station
- Market with 50+ commodities
- Faction representative present
- Player-owned market stall configured
- 100,000 credits

**Verification Points:**
- [ ] Market UI functional
- [ ] All commodities listed
- [ ] Trade transactions work
- [ ] Credits update correctly

## Configuration

### Game Path
Default: `C:/Users/allen/Desktop/Alexander/Alexander/Binaries/Win64/Alexander-Win64-Shipping.exe`

### Test Saves Directory
Default: `C:/Users/allen/Desktop/Alexander/Alexander/Saved/SaveGames`

### Performance Targets
- **FPS Target:** 90+ average
- **Memory Target:** <8GB peak
- **Load Time Target:** <10 seconds per save

## Troubleshooting

**If tests fail:**
1. Check `verification_report.json` for specific errors
2. Review error messages and stack traces
3. Run individual system tests to isolate issues
4. Check game logs for additional information
5. Verify all plugins are enabled
6. Ensure UE5.6 is properly installed

**Common issues:**
- Compilation errors: Check includes and UE5.6 API
- Runtime crashes: Look for null pointers in new code
- Performance issues: Profile with Unreal Insights
- Integration failures: Verify HTTP API is running on port 8080

## Manual Testing Checklist

If automated tests pass, manually verify:

- [ ] Can launch from all three save files
- [ ] Farming: Plant, water, harvest crops
- [ ] Mining: Mine asteroids, collect resources
- [ ] Crafting: Refine resources, craft items
- [ ] Trading: Buy/sell commodities, complete missions
- [ ] Orbital mechanics: Launch, orbit, land
- [ ] Persistence: Save/load maintains state
- [ ] Performance: Smooth gameplay, no stuttering

## Continuous Integration

To integrate with CI/CD:

```bash
# Run tests and generate report
python run_verification_tests.py

# Check exit code
if [ $? -eq 0 ]; then
    echo "Tests PASSED"
else
    echo "Tests FAILED"
    exit 1
fi
```

## Support

For issues or questions:
1. Check this documentation
2. Review `verification_report.json`
3. Consult AI assistant for analysis
4. Check game logs in `Saved/Logs/`

---

**Ready to test? Run:** `python run_verification_tests.py`