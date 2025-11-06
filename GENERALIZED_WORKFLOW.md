# GENERALIZED AUTONOMOUS DEVELOPMENT WORKFLOW

## The Universal Development Loop

```
OBSERVE → DECIDE → ACT → VERIFY → ITERATE
```

---

## The Permanent TODO List (With Specific Commands)

### 1. OBSERVE CURRENT STATE

**What:** Gather information about current state

**Commands:**
```bash
# CRITICAL FIRST STEP: Maximize Unreal Editor window
python maximize_unreal.py

# Take screenshot of editor
python take_screenshot.py

# Check recent log for errors
tail -50 Saved/Logs/Alexander.log

# Check for compilation errors
grep "error C" Saved/Logs/Alexander.log

# Check git status
git status

# List recent file changes
ls -lt Source/Alexander/Private/ | head -10
ls -lt Content/ | head -10
```

**Output:** Screenshot + log analysis + file status

---

### 2. ANALYZE & PRIORITIZE

**What:** Decide what's most important to work on RIGHT NOW

**Decision Tree:**
```bash
# Check for crashes
grep "Crash" Saved/Logs/Alexander.log
→ If found: Fix crash (highest priority)

# Check for build failures
grep "error C" Saved/Logs/Alexander.log
→ If found: Fix compilation errors

# Check screenshot for missing core features
→ If spaceship not visible: Add spaceship
→ If controls not working: Fix controls
→ If performance poor: Optimize

# Check for incomplete features
→ Implement next feature from design doc
```

**Output:** ONE specific task chosen

---

### 3. PLAN THE SOLUTION

**What:** Think through implementation before coding

**Commands:**
```bash
# Find relevant files
find Source -name "*Spaceship*"
find Content -name "*BP_*Spaceship*"

# Check existing implementation
grep -r "FlightController" Source/
grep -r "VRController" Source/

# Read current code
cat Source/Alexander/Private/Spaceship.cpp
cat Source/Alexander/Public/Spaceship.h
```

**Output:** Plan written in 2-3 sentences

---

### 4. IMPLEMENT THE CHANGE

**What:** Write code, modify assets, or adjust settings

**For C++ Code:**
```bash
# Edit files
Edit Source/Alexander/Private/Spaceship.cpp
Edit Source/Alexander/Public/Spaceship.h

# Or use sed for simple changes
sed -i 's/OldValue/NewValue/' Source/Alexander/Private/Spaceship.cpp
```

**For Blueprints/Assets:**
```python
# Use Unreal Python API
unreal.EditorAssetLibrary.load_asset("/Game/Path/To/Asset")
unreal.EditorLevelLibrary.spawn_actor_from_class(...)
```

**For Settings:**
```bash
# Edit config files
Edit Config/DefaultEngine.ini
Edit Config/DefaultGame.ini
```

**Output:** Modified files

---

### 5. BUILD & COMPILE

**What:** Compile C++ changes

**Commands:**
```bash
# Build the project
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" Alexander Win64 Development "C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" -WaitMutex

# Check build output
tail -100 Saved/Logs/Alexander.log | grep "error"

# If errors, identify which files failed
grep "error C" Saved/Logs/Alexander.log | head -20
```

**Output:** Compiled binaries OR error messages

---

### 6. VISUAL VERIFICATION

**What:** Take screenshot to see the change

**Commands:**
```bash
# Maximize editor window first
python maximize_unreal.py

# Capture current editor state
python take_screenshot.py

# Read the screenshot
Read current_screen.png

# Compare to previous
diff previous_screen.png current_screen.png (visually)
```

**Output:** Screenshot showing change

---

### 7. FUNCTIONAL TESTING

**What:** Test that feature actually works

**In-Editor Testing:**
```bash
# For running editor (PID 83192):
# Manually click "Play" in editor
# OR send command via Python:
python -c "import unreal; unreal.EditorLevelLibrary.editor_play_mode()"
```

**Test via Python:**
```python
# Execute test script
python test_spaceship_flight.py

# Or via Unreal Python
unreal-cmd.exe -ExecutePythonScript=test_flight.py
```

**Output:** Pass/fail result

---

### 8. CHECK FOR SIDE EFFECTS

**What:** Verify nothing else broke

**Commands:**
```bash
# Check for new errors
tail -100 Saved/Logs/Alexander.log | grep -i "error\|warning"

# Check for crashes
grep "Crash\|Assert\|Fatal" Saved/Logs/Alexander.log

# Run quick smoke tests
python quick_smoke_test.py

# Verify build still succeeds
Build.bat (if previously passed)
```

**Output:** Clean log OR list of new issues

---

### 9. POLISH & REFINE

**What:** Make it better if needed

**Commands:**
```bash
# Is feature complete? Check requirements
grep "TODO" Source/Alexander/Private/Spaceship.cpp

# Performance check
grep "FPS" Saved/Logs/Alexander.log | tail -20

# Visual quality check
python take_screenshot.py
# (Manually review screenshot)
```

**Decision:**
- Good enough? → Move to step 10
- Needs work? → Go back to step 4

---

### 10. DOCUMENT & COMMIT

**What:** Record what changed and save

**Commands:**
```bash
# Document change
echo "Added spaceship flight controls - 6DOF working" >> CHANGELOG.md

# Save level in editor (via Python)
python -c "import unreal; unreal.EditorLevelLibrary.save_current_level()"

# Git commit (optional)
git add Source/Alexander/Private/Spaceship.cpp
git add Source/Alexander/Public/Spaceship.h
git commit -m "Implement 6DOF flight controls for spaceship"

# Update TODO list
TodoWrite - mark current task as completed
```

**Output:** Changes saved and documented

---

### 11. IDENTIFY NEXT TASK

**What:** Determine what to work on next

**Commands:**
```bash
# Review TODO list
cat TODO.md

# Check screenshot for obvious next steps
Read current_screen.png

# Check project status
python take_screenshot.py
tail -50 Saved/Logs/Alexander.log

# Ask: What would make the biggest impact next?
```

**Output:** Next task identified

**→ LOOP BACK TO STEP 1**

---

## Quick Reference: Essential Commands

### Observation
```bash
python take_screenshot.py                              # Visual state
tail -50 Saved/Logs/Alexander.log                      # Recent logs
grep "error" Saved/Logs/Alexander.log                  # Find errors
git status                                              # Code changes
```

### File Operations
```bash
find Source -name "*.cpp"                              # Find C++ files
find Content -name "*.uasset"                          # Find assets
grep -r "ClassName" Source/                            # Search code
cat Source/Path/File.cpp                               # Read file
```

### Building
```bash
Build.bat Alexander Win64 Development Alexander.uproject -WaitMutex
```

### Testing
```bash
python test_script.py                                  # Run tests
UnrealEditor-Cmd.exe -ExecutePythonScript=test.py      # In-engine test
```

### Editor Control (Python)
```python
import unreal

# Load level
unreal.EditorLevelLibrary.load_level("/Game/LevelName")

# Spawn actor
unreal.EditorLevelLibrary.spawn_actor_from_class(ActorClass, Location, Rotation)

# Save level
unreal.EditorLevelLibrary.save_current_level()

# Take screenshot
unreal.AutomationLibrary.take_high_res_screenshot(1920, 1080, "name")

# Get actors
actors = unreal.EditorLevelLibrary.get_all_level_actors()
```

---

## The Meta-Rules

### ALWAYS
1. **Observe before acting** → `python take_screenshot.py` FIRST
2. **One thing at a time** → Change ONE file, verify, then next
3. **Verify everything** → Screenshot + log check after EVERY change
4. **Simplest solution first** → Minimum code to solve problem
5. **Fix breaks immediately** → `grep "error" log` → fix → rebuild

### NEVER
1. **Skip verification** → Always screenshot after change
2. **Ignore logs** → Always `tail log` after action
3. **Assume it works** → Always test it
4. **Make multiple changes at once** → ONE file → verify → next file
5. **Leave broken builds** → If `Build.bat` fails, fix before continuing

---

## Decision Framework: What to Work On Next?

```bash
# Priority 1: Is something broken?
if grep -q "Crash" Saved/Logs/Alexander.log; then
    FIX_CRASH
fi

# Priority 2: Does build fail?
if ! Build.bat; then
    FIX_COMPILATION
fi

# Priority 3: Is core feature missing?
if ! grep -q "BP_VRSpaceshipPlayer" current_screen.png; then
    ADD_SPACESHIP
fi

# Priority 4: Does feature not work?
if ! python test_flight.py; then
    FIX_FUNCTIONALITY
fi

# Priority 5: Is UX poor?
IMPROVE_USABILITY

# Priority 6: Does it look bad?
POLISH_VISUALS

# Priority 7: Is code messy?
REFACTOR
```

---

## Iteration Principles

### Fast Loop (5-10 minutes)
```bash
python take_screenshot.py              # 1. Observe
# Make small change to ONE file       # 2. Act
Build.bat (if C++)                     # 3. Build
python take_screenshot.py              # 4. Verify
tail -20 Saved/Logs/Alexander.log      # 5. Check
# Next iteration
```

### Medium Loop (30-60 minutes)
```bash
python take_screenshot.py              # Before
# Implement complete feature
Build.bat
python test_feature.py
python take_screenshot.py              # After
git commit -m "Feature complete"
```

### Slow Loop (Few hours)
```bash
# Review all changes
git log --oneline -20
git diff HEAD~10

# Check overall state
python take_screenshot.py
tail -100 Saved/Logs/Alexander.log

# Plan next major work
cat ROADMAP.md
```

---

## THE COMPLETE LOOP IN COMMANDS

```bash
# 1. OBSERVE
python take_screenshot.py
tail -50 Saved/Logs/Alexander.log

# 2. DECIDE
# (Human/AI judgment: What needs work?)

# 3. PLAN
find Source -name "*FeatureName*"
grep -r "RelatedCode" Source/

# 4. IMPLEMENT
# Edit file(s) with changes

# 5. BUILD (if C++ changed)
Build.bat Alexander Win64 Development Alexander.uproject -WaitMutex

# 6. VERIFY VISUALLY
python take_screenshot.py

# 7. TEST FUNCTIONALLY
python test_feature.py

# 8. CHECK SIDE EFFECTS
tail -100 Saved/Logs/Alexander.log | grep -i "error"

# 9. POLISH
# (Is it good enough? If no, return to step 4)

# 10. DOCUMENT
git commit -m "Description of change"

# 11. NEXT TASK
# → LOOP BACK TO STEP 1
```

---

**REPEAT FOREVER**
