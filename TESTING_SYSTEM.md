# The Alexander Testing Method

**Official automated testing methodology for AI-assisted game development**

---

## Overview

The Alexander Testing Method is a pragmatic, AI-friendly approach to automated game testing that combines visual verification with functional API testing. This system provides three complementary approaches for verifying the Alexander VR space game:

1. **Editor Capture** - Build + screenshot of editor UI
2. **Runtime Capture** - Build + gameplay screenshots
3. **Unit Tests** - API-based functional verification

All scripts are designed for easy AI agent execution and analysis.

---

## What Makes This "The Alexander Method"?

The Alexander Testing Method is designed specifically for AI-assisted game development:

**Core Principles**:
1. **Visual First** - System screenshots capture actual state, no complex rendering APIs
2. **Simplicity** - Three simple scripts, no framework dependencies
3. **AI-Friendly Output** - Structured reports, clear pass/fail, easy parsing
4. **Self-Contained** - Build, test, capture, report - all in one script
5. **Fast Feedback** - Complete test cycle in ~6 minutes

**Why It Works**:
- AI agents can see exactly what's on screen
- Logs are automatically collected and summarized
- No manual intervention required
- Tests run consistently, regardless of complexity
- Perfect for iterative AI-driven development

**Contrast to Traditional Testing**:
- Traditional: Complex test frameworks, manual verification
- Alexander Method: Simple scripts, automated screenshots, structured output

---

## Quick Start

### Prerequisites

```bash
pip install pillow pywin32 requests
```

### Run All Tests

```bash
# 1. Editor capture
python build_and_capture.py

# 2. Unit tests (requires editor running)
python run_unit_tests.py

# 3. Runtime capture
python build_and_capture_runtime.py
```

---

## 1. Editor Capture

**Script**: `build_and_capture.py`

**Purpose**: Verify build success and editor state

**What it does**:
1. Builds the project
2. Starts Unreal Editor (maximized)
3. Waits for full load (60s)
4. Takes screenshot of editor window
5. Collects log files
6. Shuts down editor
7. Creates summary report

**Output** (`capture_output/`):
- `editor_screenshot.png` - Full editor UI
- `build_output.txt` - Complete build log
- `editor_log_[timestamp].log` - Editor runtime log
- `log_summary.txt` - Last 50 lines of log
- `summary.md` - AI-friendly report

**When to use**:
- After making code changes
- To verify editor loads correctly
- To check for UI errors
- Before committing changes

**Example**:
```bash
python build_and_capture.py
```

**Expected duration**: ~2 minutes

---

## 2. Unit Tests

**Script**: `run_unit_tests.py`

**Purpose**: Verify core functionality through API

**What it does**:
1. Tests API connectivity
2. Starts PIE
3. Tests player pawn
4. Tests ship spawning
5. Tests input control
6. Tests thrust control
7. Stops PIE
8. Generates test report

**Output** (`test_results/`):
- `test_report_[timestamp].txt` - Detailed results

**Test phases**:
1. **API Health** - Server online, correct port
2. **PIE Control** - Can start/stop PIE
3. **Player Pawn** - Can get pawn, valid location
4. **Ship Spawning** - Can spawn, list, get position
5. **Input Control** - Can send movement/turn commands
6. **Cleanup** - Can stop PIE

**When to use**:
- After API changes
- To verify functionality
- Before major refactors
- As smoke tests

**Example**:
```bash
# Start editor first, then:
python run_unit_tests.py
```

**Expected duration**: ~30 seconds

---

## 3. Runtime Capture

**Script**: `build_and_capture_runtime.py`

**Purpose**: Verify game runs correctly in standalone mode

**What it does**:
1. Builds the project
2. Starts game (`-game` flag)
3. Waits for load (30s)
4. Takes screenshots every 5s for 30s
5. Collects log files
6. Shuts down game
7. Creates summary report

**Output** (`capture_output_runtime/`):
- `gameplay_01_[timestamp].png` - Screenshot at t=0s
- `gameplay_02_[timestamp].png` - Screenshot at t=5s
- `gameplay_03_[timestamp].png` - Screenshot at t=10s
- ... (6 screenshots total)
- `build_output.txt` - Complete build log
- `game_log_[timestamp].log` - Game runtime log
- `log_summary.txt` - Last 100 lines of log
- `summary.md` - AI-friendly report

**When to use**:
- To verify runtime performance
- To check gameplay visuals
- Before releasing builds
- For visual regression testing

**Example**:
```bash
python build_and_capture_runtime.py
```

**Expected duration**: ~3 minutes

---

## AI Agent Workflow

### Typical Development Cycle

```
1. AI makes code changes
   ↓
2. Run editor capture
   python build_and_capture.py
   ↓
3. Review: capture_output/summary.md
   ↓
4. If build failed:
   - Fix errors in build_output.txt
   - Go to step 2
   ↓
5. If build succeeded:
   Run unit tests
   python run_unit_tests.py
   ↓
6. Review: test_results/test_report_[timestamp].txt
   ↓
7. If tests failed:
   - Fix failing tests
   - Go to step 5
   ↓
8. If tests passed:
   Run runtime capture
   python build_and_capture_runtime.py
   ↓
9. Review: capture_output_runtime/summary.md
   ↓
10. If visuals good:
    ✅ Changes complete!
```

---

## For AI Agents: What to Look For

### Editor Capture

**Review**:
- `build_output.txt` → No compilation errors
- `editor_screenshot.png` → Editor loaded, no error dialogs
- `log_summary.txt` → No ERROR or WARNING lines

**Red flags**:
- Compilation errors in build output
- Error dialogs visible in screenshot
- "LogTemp: Error:" in log summary
- Missing content in Content Browser

---

### Unit Tests

**Review**:
- `test_report_[timestamp].txt` → All tests PASS

**Expected**:
```
Summary:
  Total Tests: 13
  Passed: 13
  Failed: 0
  Success Rate: 100.0%
```

**Red flags**:
- Any test marked "FAIL" or "ERROR"
- Success rate < 100%
- API connectivity failures
- PIE start/stop failures

---

### Runtime Capture

**Review**:
- All gameplay screenshots → Visuals rendering
- `log_summary.txt` → No errors during gameplay

**Look for**:
- Black screens (rendering failure)
- Missing geometry (asset loading failure)
- Crash messages in log
- Performance warnings

---

## Configuration

### Timing Adjustments

**Editor load time** (`build_and_capture.py`):
```python
EDITOR_LOAD_TIME = 60  # Increase if editor loads slowly
CAPTURE_DELAY = 5      # Wait before screenshot
```

**Game duration** (`build_and_capture_runtime.py`):
```python
GAME_LOAD_TIME = 30         # Wait for game to load
GAMEPLAY_DURATION = 30      # How long to run game
SCREENSHOT_INTERVAL = 5     # Seconds between screenshots
```

**Test timeouts** (`run_unit_tests.py`):
```python
TIMEOUT = 10  # API request timeout in seconds
```

---

## Troubleshooting

### "PIL or pywin32 not installed"

```bash
pip install pillow pywin32
```

### "API server not responding"

Make sure Unreal Editor is running before unit tests:
```bash
start "" "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject"

# Wait ~60 seconds, then:
python run_unit_tests.py
```

### "Build failed"

Check `capture_output/build_output.txt` for errors. Common issues:
- Syntax errors in C++ code
- Missing includes
- Unresolved references

### "Game window not found"

Increase wait time in script:
```python
GAME_LOAD_TIME = 60  # Instead of 30
```

---

## Advanced Usage

### Custom Test Suites

Add tests to `run_unit_tests.py`:

```python
def test_custom_feature():
    """Test: Custom feature works"""
    result = api_post("/my_endpoint", {"param": "value"})
    return result.get("success")

# Add to main():
runner.test("Custom feature", test_custom_feature)
```

### Multiple Screenshots

Modify `build_and_capture.py` to take multiple screenshots:

```python
# After maximizing window:
for i in range(5):
    time.sleep(10)
    screenshot_file = OUTPUT_DIR / f"editor_{i:02d}.png"
    capture_window_screenshot(hwnd, screenshot_file)
```

### Automated Comparison

Compare screenshots to baseline:

```python
from PIL import Image, ImageChops

def compare_screenshots(baseline, current):
    img1 = Image.open(baseline)
    img2 = Image.open(current)
    diff = ImageChops.difference(img1, img2)

    # Get histogram to quantify difference
    histogram = diff.histogram()
    total_pixels = sum(histogram)
    changed_pixels = sum(histogram[256:])  # Non-zero differences

    return (changed_pixels / total_pixels) * 100
```

---

## Output Directory Structure

```
Alexander/
├── build_and_capture.py
├── build_and_capture_runtime.py
├── run_unit_tests.py
├── capture_output/
│   ├── editor_screenshot.png
│   ├── build_output.txt
│   ├── editor_log_[timestamp].log
│   ├── log_summary.txt
│   └── summary.md
├── capture_output_runtime/
│   ├── gameplay_01_[timestamp].png
│   ├── gameplay_02_[timestamp].png
│   ├── ...
│   ├── build_output.txt
│   ├── game_log_[timestamp].log
│   ├── log_summary.txt
│   └── summary.md
└── test_results/
    └── test_report_[timestamp].txt
```

---

## Best Practices

### For AI Agents

1. **Always build first** - Don't skip build verification
2. **Review logs** - Check log_summary.txt for errors
3. **Check screenshots** - Visual verification is critical
4. **Run unit tests** - Functional verification matters
5. **Document findings** - Note issues in reports

### For Developers

1. **Run after major changes** - Verify nothing broke
2. **Check before commits** - Don't commit broken code
3. **Compare baselines** - Track visual regressions
4. **Add new tests** - Test new features
5. **Update docs** - Keep this file current

---

## Integration with CI/CD

### GitHub Actions Example

```yaml
name: Alexander Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: pip install pillow pywin32 requests
      - name: Build and test
        run: python build_and_capture.py
      - name: Upload artifacts
        uses: actions/upload-artifact@v2
        with:
          name: test-results
          path: |
            capture_output/
            test_results/
```

---

## Summary

| Script | Duration | Output | Use Case |
|--------|----------|--------|----------|
| `build_and_capture.py` | ~2 min | Editor screenshot + logs | Verify editor loads |
| `run_unit_tests.py` | ~30 sec | Test report | Verify functionality |
| `build_and_capture_runtime.py` | ~3 min | Gameplay screenshots | Verify runtime |

**Total test suite duration**: ~6 minutes

**Recommended frequency**: After every code change

---

## Quick Reference

```bash
# Editor + build verification
python build_and_capture.py

# Functional testing (editor must be running)
python run_unit_tests.py

# Runtime + gameplay verification
python build_and_capture_runtime.py
```

---

## The Alexander Method Philosophy

This testing approach represents a fundamental shift in how AI agents verify game development:

**Traditional Approach**:
- Complex test frameworks
- Manual screenshot capture
- Separate build/test/verify steps
- Requires human interpretation

**The Alexander Method**:
- Three simple scripts
- Automatic screenshot capture
- Integrated build/test/verify
- AI-parseable structured output

**Key Innovation**: Treating the entire game state (visual + functional + logs) as a single testable artifact that AI agents can easily verify.

**Result**: AI agents can make changes, run tests, and verify results without human intervention - enabling true autonomous development cycles.

---

**Status**: ✅ The Alexander Testing Method - Complete
**Last Updated**: 2025-11-10
**Version**: 1.0
**Methodology**: Official Alexander Method for AI-assisted game development
