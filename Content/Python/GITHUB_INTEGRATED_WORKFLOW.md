# GitHub-Integrated Development Workflow

**Project:** Alexander VR Space Farming Simulator
**Integration:** GitHub + Unreal Engine + Python Automation

---

## 🎯 Workflow Philosophy

**Local Development** → **Build & Test** → **Git Commit** → **GitHub Issue Tracking** → **Documentation Updates**

---

## 📂 Repository Structure

```
Alexander/
├── .git/                          # Git repository
├── Alexander/                     # Unreal project
│   ├── Content/
│   │   ├── Python/               # Python automation scripts
│   │   │   ├── *.py              # Script files
│   │   │   ├── *.md              # Documentation
│   │   │   └── *.txt             # Reference files
│   │   └── SpaceShip/            # Game content
│   ├── Saved/
│   │   └── Screenshots/          # Test screenshots
│   └── Config/                    # Project settings
├── .gitignore                     # Git ignore rules
└── README.md                      # Project overview
```

---

## 🌳 Branching Strategy

### Main Branches

1. **`main`** - Production-ready code
   - Always stable
   - Deploys work correctly
   - Lighting confirmed working

2. **`develop`** - Integration branch
   - Latest development work
   - Tested but may have minor issues
   - Merges into `main` when stable

### Feature Branches

Pattern: `feature/<issue-number>-<short-description>`

**Examples:**
- `feature/1-ai-vision-test-arena`
- `feature/2-spaceship-cockpit`
- `feature/3-dual-lighting-system`
- `feature/4-flight-physics`

### Fix Branches

Pattern: `fix/<issue-number>-<short-description>`

**Examples:**
- `fix/5-overlapping-lights-error`
- `fix/6-black-screenshots`
- `fix/7-missing-materials`

---

## 🔄 Complete Workflow

### Step 1: Create GitHub Issue

```markdown
**Title:** Add Dual Lighting System to Test Arena

**Description:**
Implement starlight (ambient) + dynamic star (point light) system for day/night coverage in AI Vision Test Arena.

**Requirements:**
- [ ] Static SkyLight for ambient starlight
- [ ] Point light for dynamic star
- [ ] Stay under 4-5 movable light limit
- [ ] Verify lighting builds successfully

**Acceptance Criteria:**
- Scene is properly lit
- Screenshots capture visible geometry
- No "too many overlapping lights" error

**Labels:** enhancement, phase-3, lighting
```

### Step 2: Create Feature Branch

```bash
# From develop branch
git checkout develop
git pull origin develop

# Create feature branch
git checkout -b feature/3-dual-lighting-system
```

### Step 3: Develop & Test (CRITICAL ORDER!)

```bash
# 1. Make changes via remote execution
python Alexander/Content/Python/remote_client.py --file "setup_dual_lighting.py"

# 2. ⚠️ TRIGGER BUILD (NEVER SKIP!)
python Alexander/Content/Python/remote_client.py --file "trigger_lighting_build.py"

# 3. ⚠️ RESTART EDITOR (Connection will drop during build)
./APPLY_CHANGES_NO_PAUSE.bat

# 4. Wait 30-60 seconds for editor to fully load

# 5. Test connection
python Alexander/Content/Python/remote_client.py --test

# 6. Test/capture screenshots (once reconnected)
python Alexander/Content/Python/remote_client.py --file "capture_arena_views.py"

# 7. Verify results
```

**Note:** Connection drops during builds are normal - restart is part of the workflow!

### Step 4: Document Changes

Update relevant documentation:
- `WORKFLOW_STATUS.md` - Mark task complete
- `CRITICAL_REMINDER_LIGHTING.txt` - Add any new troubleshooting
- Comments in Python scripts

### Step 5: Commit with Descriptive Message

```bash
# Check what changed
git status
git diff

# Stage changes
git add Alexander/Content/Python/setup_dual_lighting.py
git add Alexander/Content/Python/WORKFLOW_STATUS.md

# Commit with reference to issue
git commit -m "feat: Add dual lighting system (starlight + star)

- Implemented static SkyLight for ambient starlight (10.0 intensity)
- Added Star_Dynamic point light (500,000 intensity)
- Set SkyLight to STATIC to avoid movable light limit
- Kept total movable lights at 4 (under limit)
- Verified scene renders properly after build

Resolves #3"
```

#### Commit Message Format

```
<type>: <subject>

<body>

<footer>
```

**Types:**
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation only
- `refactor`: Code restructuring
- `test`: Test-related
- `chore`: Maintenance

**Examples:**
```
fix: Resolve "too many overlapping lights" error

- Removed 5 excess fill lights
- Set SkyLight to STATIC mobility
- Reduced movable lights from 10 to 4
- Verified error no longer appears

Fixes #5

---

docs: Add critical build-before-test workflow

- Created CRITICAL_WORKFLOW_ORDER.md
- Emphasized trigger build step
- Added troubleshooting checklist

Related to #6
```

### Step 6: Push to GitHub

```bash
# Push feature branch
git push origin feature/3-dual-lighting-system
```

### Step 7: Create Pull Request

**On GitHub:**

**Title:** `[Phase 3] Add Dual Lighting System`

**Description:**
```markdown
## Changes
- ✅ Implemented static SkyLight for ambient starlight
- ✅ Added Star_Dynamic point light (500,000 intensity)
- ✅ Fixed "too many overlapping lights" error
- ✅ Set SkyLight to STATIC mobility
- ✅ Verified lighting builds successfully

## Testing
- [x] Scene visible in viewport
- [x] Screenshots capture lit geometry
- [x] No lighting errors in console
- [x] Movable light count: 4 (under limit)

## Screenshots
![Cam1 Overview](link-to-screenshot)
![Cam5 Isometric](link-to-screenshot)

## Related Issues
Resolves #3
Closes #5

## Checklist
- [x] Code follows project conventions
- [x] Documentation updated
- [x] Build triggered and verified
- [x] Screenshots captured and verified
```

**Reviewers:** Assign yourself or team members
**Labels:** `phase-3`, `lighting`, `ready-for-review`

### Step 8: Merge & Close Issue

After PR approval:

```bash
# Merge into develop
git checkout develop
git merge feature/3-dual-lighting-system
git push origin develop

# Delete feature branch
git branch -d feature/3-dual-lighting-system
git push origin --delete feature/3-dual-lighting-system
```

**On GitHub:** Issue #3 automatically closes via "Resolves #3" in commit

---

## 📋 GitHub Issue Labels

### Type Labels
- `feature` - New functionality
- `bug` - Something broken
- `documentation` - Docs only
- `enhancement` - Improvement to existing feature

### Phase Labels
- `phase-1` - Visual Verification
- `phase-2` - Geometry Building
- `phase-3` - VR Interaction & Lighting
- `phase-4` - Flight Physics
- `phase-5` - AI Integration

### Priority Labels
- `priority-critical` - Blocking work
- `priority-high` - Important
- `priority-medium` - Normal
- `priority-low` - Nice to have

### Status Labels
- `status-in-progress` - Currently working
- `status-blocked` - Waiting on something
- `status-ready-for-review` - PR ready
- `status-testing` - In testing phase

---

## 🔍 Issue Tracking Examples

### Example 1: Feature Request

```markdown
**Title:** Implement 6DOF Flight Physics System

**Labels:** feature, phase-4, priority-high

**Description:**
Create physics system that reads FlightStick and Throttle positions to control spaceship movement.

**Requirements:**
- Read FlightStick position → Pitch/Roll/Yaw
- Read Throttle position → Forward/Backward thrust
- Apply forces to spaceship
- Zero gravity environment (space)
- Damping for smooth control

**Acceptance Criteria:**
- FlightStick controls rotation
- Throttle controls translation
- Movement feels natural in VR
- Physics stable at all framerates

**Related Files:**
- `Alexander/Content/Python/implement_flight_physics.py`
- `Alexander/Content/SpaceShip/BP_FlightController.uasset`
```

### Example 2: Bug Report

```markdown
**Title:** Screenshots Capturing Black Screen

**Labels:** bug, phase-3, priority-critical

**Description:**
All captured screenshots are completely black despite proper lighting setup.

**Steps to Reproduce:**
1. Setup dual lighting system
2. Run `capture_arena_views.py`
3. Check screenshots in `Saved/Screenshots/WindowsEditor/`
4. All images are black

**Expected Behavior:**
Screenshots should show lit scene with visible geometry

**Actual Behavior:**
Black images only

**Environment:**
- Unreal Engine 5.6
- Windows 11
- Python Remote Execution enabled

**Investigation:**
- [x] Checked lighting exists (10 lights present)
- [x] Checked materials assigned (DefaultMaterial)
- [ ] Check viewport show flags
- [ ] Check if VR mode active
- [ ] Trigger lighting build

**Root Cause:** (To be determined)
```

### Example 3: Documentation Task

```markdown
**Title:** Document Critical Build-Before-Test Workflow

**Labels:** documentation, priority-high

**Description:**
Create documentation emphasizing that lighting builds must be triggered after changes before testing.

**Tasks:**
- [ ] Create CRITICAL_WORKFLOW_ORDER.md
- [ ] Add build workflow examples
- [ ] Document build times
- [ ] Create troubleshooting section
- [ ] Add warning signs checklist

**Acceptance Criteria:**
- Clear step-by-step workflow
- Examples for common scenarios
- Easy to find and reference
```

---

## 🎯 Current Project Status (GitHub Issues)

### Open Issues

#### Phase 3: VR Interaction & Lighting
- [ ] #5 - Fix "too many overlapping lights" error (IN PROGRESS)
- [ ] #6 - Resolve black screenshot issue (BLOCKED - need build trigger)
- [ ] #7 - Exit VR mode programmatically

#### Phase 4: Flight Physics
- [ ] #8 - Implement 6DOF movement system
- [ ] #9 - Connect VR motion controllers to flight controls
- [ ] #10 - Tune physics damping for smooth flight

### Closed Issues

#### Phase 1-2: Arena Setup
- [x] #1 - Create AI Vision Test Arena with 4 zones
- [x] #2 - Add spaceship cockpit to Zone 4

#### Phase 3: Lighting
- [x] #3 - Implement dual lighting system (starlight + star)
- [x] #4 - Clean up duplicate actors (503→73)

---

## 📊 Milestones

### Milestone: Phase 3 Complete
**Due:** Current Sprint
**Progress:** 80% (4/5 issues closed)

**Remaining:**
- Verify lighting renders correctly
- Capture working screenshots

### Milestone: Phase 4 Complete
**Due:** Next Sprint
**Progress:** 0% (0/3 issues closed)

**Requirements:**
- Flight physics implemented
- VR controllers connected
- Smooth flight experience

---

## 🔄 Daily Workflow

### Morning Standup (Self-Check)

```bash
# 1. Pull latest from develop
git checkout develop
git pull origin develop

# 2. Check GitHub issues assigned to you
# 3. Create/checkout feature branch
# 4. Review WORKFLOW_STATUS.md
```

### During Development

```bash
# Make changes → BUILD → Test (never skip build!)
# Commit frequently with good messages
# Push to feature branch regularly
```

### End of Day

```bash
# 1. Commit all work
git add .
git commit -m "wip: [description of progress]"
git push origin feature/branch-name

# 2. Update WORKFLOW_STATUS.md
# 3. Comment on relevant GitHub issues with progress
```

---

## 📝 Documentation Integration

### When to Update Docs

**Always update when:**
- Adding new Python scripts
- Fixing critical bugs
- Changing workflow
- Discovering new troubleshooting steps

**Files to Update:**
- `WORKFLOW_STATUS.md` - Task progress
- `CRITICAL_WORKFLOW_ORDER.md` - Workflow changes
- `CRITICAL_REMINDER_LIGHTING.txt` - Lighting troubleshooting
- Script comments - Implementation details

### Doc Update Checklist

```bash
# Before committing
- [ ] Updated WORKFLOW_STATUS.md with task completion
- [ ] Added/updated comments in Python scripts
- [ ] Updated troubleshooting docs if discovered new issues
- [ ] Captured screenshots if visual changes
- [ ] Referenced GitHub issue numbers in commits
```

---

## 🚨 Critical Reminders

1. **ALWAYS trigger build after changes before testing**
2. **Commit with descriptive messages referencing issues**
3. **Update documentation alongside code changes**
4. **Create GitHub issues for all significant work**
5. **Use feature branches, never commit directly to develop/main**
6. **Keep WORKFLOW_STATUS.md current**

---

## 🎓 Benefits of This Workflow

✅ **Traceability:** Every change linked to an issue
✅ **Documentation:** Always up-to-date with code
✅ **Collaboration:** Clear status for team members
✅ **Rollback:** Easy to revert broken changes
✅ **Learning:** Issue history shows problem-solving process
✅ **Quality:** Build-test-commit cycle ensures working code

---

**Remember:** Good workflow prevents bugs, saves time, and makes collaboration smooth!
