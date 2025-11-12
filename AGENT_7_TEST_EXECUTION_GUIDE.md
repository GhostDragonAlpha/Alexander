# AGENT 7: TEST EXECUTION GUIDE

## Overview

This guide provides step-by-step instructions for executing the Alexander project's automated test suite once compilation succeeds.

---

## Prerequisites

### Required:
1. ✅ **Successful Compilation:** Project must build without errors
2. ✅ **Unreal Engine 5.6:** Installed at default location
3. ✅ **UnrealEditor-Cmd.exe:** Available at `C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\`

### Recommended:
- 16GB+ RAM (for headless test execution)
- SSD storage (for faster test execution)
- No other UE instances running (to avoid conflicts)

---

## Quick Start

### 1. Verify Compilation Success

```bash
# Check that build succeeded
cd C:\Users\allen\Desktop\Alexander\Alexander
# Verify no compilation errors
cat compilation_errors_report.json
```

**Expected:** `"success": true, "total_errors": 0`

### 2. Run All Automated Tests

```bash
# Execute all tests
run_automated_tests.bat
```

**Expected Duration:** 10-30 minutes
**Output:** `TestReports\index.html`

### 3. View Results

- Open `TestReports\index.html` in browser
- Review `TestReports\AutomationReport.json` for details
- Check `test_execution.log` for console output

---

## Test Execution Methods

### Method 1: Batch Script (Recommended)

**Run All Tests:**
```bash
run_automated_tests.bat
```

**Run Specific Suite:**
```bash
run_specific_test_suite.bat Alexander.PlanetaryFarming
run_specific_test_suite.bat Alexander.Persistence
run_specific_test_suite.bat Alexander.PlanetInterior
```

### Method 2: Direct UnrealEditor-Cmd

**All Tests:**
```bash
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" ^
  "C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" ^
  -ExecCmds="Automation RunTests Alexander" ^
  -TestExit="Automation Test Queue Empty" ^
  -ReportOutputPath="C:\Users\allen\Desktop\Alexander\Alexander\TestReports" ^
  -Log -NoSplash -Unattended -NullRHI
```

**Specific Suite:**
```bash
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" ^
  "C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" ^
  -ExecCmds="Automation RunTests Alexander.PlanetaryFarming" ^
  -TestExit="Automation Test Queue Empty" ^
  -ReportOutputPath="TestReports" ^
  -Log -NoSplash -Unattended -NullRHI
```

### Method 3: Unreal Editor UI

1. Open project in Unreal Editor
2. Window → Test Automation (Ctrl+Shift+T)
3. Select "Alexander" in test tree
4. Click "Start Tests"

---

## Test Categories

### 1. Planetary Farming Tests (7 tests)
**Category:** `Alexander.PlanetaryFarming`
**Duration:** ~5 minutes
**Coverage:**
- Land Claim Manager (7 assertions)
- Crop Growth System (12 assertions)
- Farming System (10 assertions)
- Base Building Manager (15 assertions)
- Production Chain Manager (12 assertions)
- Integration Tests (5 assertions)
- Master Test Suite (6 assertions)

**Run:**
```bash
run_specific_test_suite.bat Alexander.PlanetaryFarming
```

### 2. Persistence Tests (5 tests)
**Category:** `Alexander.Persistence`
**Duration:** ~3 minutes
**Coverage:**
- Basic Save/Load Operations (12 assertions)
- Auto-Save Functionality (5 assertions)
- Backup Management (8 assertions)
- Origin-Centered Physics (10 assertions)
- Integration Workflow (10 assertions)

**Run:**
```bash
run_specific_test_suite.bat Alexander.Persistence
```

### 3. Planet Interior Tests (24 tests)
**Category:** `Alexander.PlanetInterior`
**Duration:** ~8 minutes
**Coverage:**
- Noise Generation (4 tests, 15+ assertions)
- Biome Blending (3 tests, 10+ assertions)
- LOD Management (3 tests, 8+ assertions)
- Crop Growth Simulation (6 tests, 25+ assertions)
- Performance Monitoring (8 tests, 30+ assertions)

**Run:**
```bash
run_specific_test_suite.bat Alexander.PlanetInterior
```

---

## Manual Tests (Not Automated)

These tests are not integrated with UE Automation Framework and require manual invocation.

### Farming System Tests
**File:** `Source\Alexander\Tests\FarmingSystemTest.cpp`
**Tests:** 6 methods
**Invocation:** Add to game code or test harness:
```cpp
FarmingSystemTest::RunAllTests();
```

### Resource Gathering Tests
**File:** `Source\Alexander\Tests\ResourceGatheringSystemTest.cpp`
**Tests:** 7 methods
**Invocation:**
```cpp
FResourceGatheringSystemTest::RunAllTests();
```

### AI Dialogue Tests
**File:** `Source\Alexander\Tests\AIDialogueSystemTest.cpp`
**Tests:** 4 methods
**Invocation:**
```cpp
AIDialogueSystemTest::RunAllTests();
```

---

## Test Execution Best Practices

### Before Running Tests:
1. ✅ Ensure project compiles successfully
2. ✅ Close all Unreal Editor instances
3. ✅ Clear previous test reports: `del /s /q TestReports\*`
4. ✅ Check available disk space (500MB+ recommended)
5. ✅ Ensure no background processes using project files

### During Test Execution:
- ⏳ Wait for completion (10-30 minutes)
- 📊 Monitor CPU/RAM usage
- 💾 Don't modify project files
- 🚫 Don't start other UE instances

### After Test Execution:
1. 📋 Review test report: `TestReports\index.html`
2. 🔍 Check for failures
3. 📝 Analyze failed tests
4. 🐛 Report bugs found
5. 📊 Compare with previous results

---

## Understanding Test Results

### Test Report Structure:
```
TestReports/
├── index.html              # HTML report (open in browser)
├── AutomationReport.json   # Detailed JSON data
└── Screenshots/            # Test failure screenshots
```

### Result Categories:

**✅ Success:**
- Test passed all assertions
- No errors or warnings
- Expected behavior confirmed

**❌ Failed:**
- One or more assertions failed
- Check logs for details
- May indicate bugs or regressions

**⚠️ Warning:**
- Test completed but with warnings
- Non-critical issues detected
- Review recommended

**⏭️ Skipped:**
- Test not executed (disabled or filtered)
- No result available

### Common Exit Codes:

| Exit Code | Meaning | Action |
|-----------|---------|--------|
| 0 | All tests passed | ✅ Success |
| 1 | Some tests failed | Review failures |
| 2 | Test execution error | Check logs |
| 6 | Build/compile error | Fix compilation |
| 137 | Out of memory | Increase RAM |

---

## Troubleshooting

### Problem: "UnrealEditor-Cmd.exe not found"
**Solution:**
1. Verify UE 5.6 installation
2. Update path in batch scripts
3. Check for typos in path

### Problem: "Project file not found"
**Solution:**
1. Verify working directory
2. Check project path is correct
3. Ensure .uproject file exists

### Problem: Tests hang or timeout
**Solution:**
1. Increase timeout in script
2. Close other applications
3. Run specific suites instead of all tests
4. Check for infinite loops in test code

### Problem: Tests crash
**Solution:**
1. Check compilation succeeded
2. Review crash logs: `Saved\Logs\`
3. Run in debugger mode
4. Report crash with logs

### Problem: "No tests found"
**Solution:**
1. Verify test files compiled
2. Check test naming follows pattern
3. Ensure tests use IMPLEMENT_SIMPLE_AUTOMATION_TEST
4. Rebuild project

---

## Coverage Analysis

### Generate Coverage Report:
```bash
# Run tests with coverage tracking
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" ^
  "Alexander.uproject" ^
  -ExecCmds="Automation RunTests Alexander" ^
  -ReportExportPath="Coverage.json" ^
  -Log -NoSplash -Unattended -NullRHI
```

### Analyze Coverage:
```python
# Parse coverage data
import json

with open('Coverage.json', 'r') as f:
    coverage = json.load(f)

# Calculate coverage percentages
total_tests = len(coverage['tests'])
passed_tests = sum(1 for t in coverage['tests'] if t['result'] == 'Passed')
coverage_pct = (passed_tests / total_tests) * 100

print(f"Coverage: {coverage_pct:.2f}%")
```

---

## Performance Benchmarking

### Run Performance Tests Only:
```bash
run_specific_test_suite.bat Alexander.PlanetInterior.Performance
```

### VR Performance Validation:
```bash
# Ensure VR tests run with headset connected
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" ^
  "Alexander.uproject" ^
  -ExecCmds="Automation RunTests Alexander.PlanetInterior.Performance.VR" ^
  -vr -Log -NoSplash -Unattended
```

### Performance Thresholds:
- **VR FPS:** ≥90 FPS average
- **VR Frame Time:** ≤11.1ms
- **VR Consistency:** ≤2ms variance
- **Memory Usage:** ≤8GB
- **Streaming Bandwidth:** ≤500 MB/s

---

## Continuous Integration Setup

### Jenkins Configuration:
```groovy
pipeline {
    agent any
    stages {
        stage('Build') {
            steps {
                bat 'ubt.bat'
            }
        }
        stage('Test') {
            steps {
                bat 'run_automated_tests.bat'
            }
        }
        stage('Report') {
            steps {
                publishHTML([
                    reportName: 'Test Report',
                    reportDir: 'TestReports',
                    reportFiles: 'index.html'
                ])
            }
        }
    }
}
```

### GitHub Actions:
```yaml
name: Run Tests
on: [push, pull_request]
jobs:
  test:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v2
      - name: Build Project
        run: .\ubt.bat
      - name: Run Tests
        run: .\run_automated_tests.bat
      - name: Upload Results
        uses: actions/upload-artifact@v2
        with:
          name: test-reports
          path: TestReports/
```

---

## Test Maintenance

### Adding New Tests:
1. Create test file in `Source\Alexander\Tests\`
2. Use IMPLEMENT_SIMPLE_AUTOMATION_TEST macro
3. Follow naming convention: `F[TestName]Test`
4. Add to appropriate category: `Alexander.[Category].[TestName]`
5. Rebuild project
6. Run tests to verify

### Disabling Tests:
```cpp
// Add "_DISABLED" to test name
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMyTest_DISABLED, "Alexander.Category.MyTest",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::Disabled)
```

### Updating Tests:
1. Modify test code
2. Rebuild project
3. Run specific test to verify
4. Run full suite to check for regressions

---

## Expected Test Results

### Baseline (Clean Build):
- **Total Tests:** 36 automated
- **Expected Pass Rate:** ≥95%
- **Expected Failures:** ≤2 tests
- **Execution Time:** 10-15 minutes
- **Memory Peak:** ~6GB

### Performance Tests:
- **VR FPS:** 90-120 FPS
- **Frame Time:** 8-11ms
- **Load Time:** ≤30 seconds
- **Streaming:** ≤400 MB/s

### Integration Tests:
- **Save/Load:** ≤1 second
- **World Generation:** ≤5 seconds
- **System Initialization:** ≤3 seconds

---

## Contact & Support

**Test Infrastructure Issues:**
- Agent 7 - Test Infrastructure & Execution
- Reference: `AGENT_7_TEST_INFRASTRUCTURE_REPORT.md`

**Build/Compilation Issues:**
- Agent 1 - Compilation & Build System
- Reference: `compilation_errors_report.json`

**Test Failures:**
- Review test logs: `TestReports\AutomationReport.json`
- Check implementation in test source files
- Report bugs with full stack trace

---

**Last Updated:** November 12, 2025
**Document Version:** 1.0
**Project:** Alexander (Unreal Engine 5.6)
**Status:** Ready for Test Execution (Pending Compilation Success)
