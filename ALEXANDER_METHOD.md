# The Alexander Testing Method

**Official methodology for AI-assisted game development**

---

## What Is It?

The Alexander Testing Method is a pragmatic approach to automated game testing that combines:
- **Visual verification** (system screenshots)
- **Functional testing** (API-based unit tests)
- **Build validation** (integrated compilation checks)

All designed for easy AI agent execution and analysis.

---

## Three Simple Scripts

### 1. Editor Capture
```bash
python build_and_capture.py
```
- Builds project
- Opens editor (maximized)
- Takes screenshot
- Collects logs
- Creates report

**Use when**: Verifying editor loads correctly after code changes

---

### 2. Unit Tests
```bash
python run_unit_tests.py
```
- Tests API connectivity
- Verifies PIE control
- Tests ship spawning
- Tests input/thrust
- Generates pass/fail report

**Use when**: Verifying functionality through API

---

### 3. Runtime Capture
```bash
python build_and_capture_runtime.py
```
- Builds project
- Runs game in standalone mode
- Captures 6 gameplay screenshots
- Collects runtime logs
- Creates report

**Use when**: Verifying game runs correctly

---

## Why "The Alexander Method"?

**Problem**: Traditional testing is complex and requires human verification

**Solution**: Simple scripts that AI agents can run and interpret autonomously

**Key Innovation**: Treating the entire game state (visual + functional + logs) as a single testable artifact

---

## Quick Start

### Install Dependencies
```bash
pip install pillow pywin32 requests
```

### Run Complete Test Suite
```bash
# 1. Editor + build verification
python build_and_capture.py

# 2. Functional testing
python run_unit_tests.py

# 3. Runtime verification
python build_and_capture_runtime.py
```

**Total time**: ~6 minutes

---

## Core Principles

1. **Visual First** - System screenshots capture actual state
2. **Simplicity** - Three simple scripts, no frameworks
3. **AI-Friendly** - Structured reports, clear pass/fail
4. **Self-Contained** - Build, test, capture, report in one script
5. **Fast Feedback** - Complete cycle in ~6 minutes

---

## Output Structure

```
Alexander/
├── capture_output/           # Editor screenshots + logs
│   ├── editor_screenshot.png
│   ├── build_output.txt
│   └── summary.md
│
├── test_results/             # Unit test reports
│   └── test_report_[timestamp].txt
│
└── capture_output_runtime/   # Gameplay screenshots + logs
    ├── gameplay_01_*.png
    ├── gameplay_02_*.png
    └── summary.md
```

---

## For AI Agents

### What to Check

**Editor Capture**:
- ✅ Build succeeded (no errors in build_output.txt)
- ✅ Editor loaded (screenshot shows UI, no error dialogs)
- ✅ No errors in log_summary.txt

**Unit Tests**:
- ✅ All 13 tests passed
- ✅ 100% success rate

**Runtime Capture**:
- ✅ All screenshots show rendering
- ✅ No crashes in logs
- ✅ Game runs smoothly

---

## Philosophy

**Traditional Approach**:
```
Code → Complex Framework → Manual Verification → Human Interpretation
```

**Alexander Method**:
```
Code → Simple Script → Automatic Capture → AI-Parseable Output
```

**Result**: True autonomous development cycles

---

## Documentation

**Complete Guide**: [TESTING_SYSTEM.md](TESTING_SYSTEM.md)

**Detailed topics**:
- Configuration and timing adjustments
- Adding custom tests
- Troubleshooting common issues
- Advanced usage patterns
- CI/CD integration

---

## Example AI Agent Workflow

```
1. AI makes code changes to C++ files
   ↓
2. AI runs: python build_and_capture.py
   ↓
3. AI reviews: capture_output/summary.md
   ↓
4. If build failed:
   - AI reads build_output.txt
   - AI fixes errors
   - Go to step 2
   ↓
5. If build succeeded:
   AI runs: python run_unit_tests.py
   ↓
6. AI reviews: test_results/test_report_*.txt
   ↓
7. If tests passed:
   AI runs: python build_and_capture_runtime.py
   ↓
8. AI reviews: capture_output_runtime/summary.md
   ↓
9. All green? ✅ Changes complete!
```

**No human intervention required**

---

## Comparison to Other Methods

| Feature | Traditional | Alexander Method |
|---------|-------------|------------------|
| Setup complexity | High | Low (3 scripts) |
| Dependencies | Many frameworks | PIL, pywin32, requests |
| Visual verification | Manual | Automatic screenshots |
| AI-friendly output | No | Yes (structured markdown) |
| Build integration | Separate | Built-in |
| Time to run | Varies | ~6 minutes |
| Human intervention | Required | None |

---

## Success Metrics

**After implementing the Alexander Method on this project**:

- ✅ Build verification automated
- ✅ Editor state captured automatically
- ✅ 13 functional tests via API
- ✅ Runtime gameplay captured
- ✅ Complete test cycle in 6 minutes
- ✅ AI agent can iterate autonomously

---

## Get Started

```bash
# Clone and install
pip install pillow pywin32 requests

# Run first test
python build_and_capture.py

# Review results
cat capture_output/summary.md
```

---

**Status**: ✅ Production Ready
**Version**: 1.0
**Methodology**: The Alexander Testing Method
**Use Case**: AI-assisted game development

**Full Documentation**: [TESTING_SYSTEM.md](TESTING_SYSTEM.md)
