# Baseline Daily Workflow Guide - Phase 8

**Project**: Alexander VR Space Simulation
**Purpose**: Daily routines and best practices for Phase 8 development
**Audience**: All team members working on Phase 8 content
**Last Updated**: November 8, 2025

---

## Table of Contents

1. [Morning Routine](#morning-routine)
2. [Development Workflow](#development-workflow)
3. [Testing Workflow](#testing-workflow)
4. [Performance Monitoring](#performance-monitoring)
5. [Git Best Practices](#git-best-practices)
6. [Daily Standup Format](#daily-standup-format)
7. [End-of-Day Checklist](#end-of-day-checklist)

---

## Morning Routine

**Duration**: 15-20 minutes
**Goal**: Start day with clean, up-to-date codebase and clear priorities

### Step 1: Sync with Repository (5 minutes)

```bash
# Navigate to project directory
cd /c/Users/allen/Desktop/Alexander/Alexander

# Check current branch
git branch --show-current

# If on feature branch, ensure it's up to date
git fetch origin
git status

# If behind remote, pull latest changes
git pull origin <your-branch>

# If main/master has updates, consider merging
git fetch origin master
git log HEAD..origin/master --oneline  # See what's new
# git merge origin/master  # Merge if needed
```

**Check for**:
- [ ] No merge conflicts
- [ ] Remote branch exists
- [ ] Local changes are committed or stashed

---

### Step 2: Review Project Status (5 minutes)

```bash
# Check build status
cat build_output.txt  # Review last build

# Check test results
cat TEST_REPORT.md  # Review test status

# View recent changes
git log --oneline -10  # Last 10 commits

# Check what files changed
git diff origin/master...HEAD --stat
```

**Review Documents**:
- [ ] `PHASE8_PROGRESS_TRACKER.md` - Overall progress
- [ ] `ITERATION_LOG.md` - Recent changes
- [ ] Team chat/Slack - Blockers or updates

---

### Step 3: Verify Development Environment (5 minutes)

```bash
# Test build (quick check)
"C:/Program Files/Epic Games/UE_5.6/Engine/Build/BatchFiles/Build.bat" AlexanderEditor Win64 Development "C:/Users/allen/Desktop/Alexander/Alexander/Alexander.uproject" -NoHotReload

# If build fails, check errors and fix before proceeding
```

**Environment Checks**:
- [ ] Unreal Engine 5.6 launches successfully
- [ ] Visual Studio 2022 opens without errors
- [ ] Python environment active (if using automation)
- [ ] VR headset connected (if testing VR features)

---

### Step 4: Plan Today's Tasks (5 minutes)

**Priority Framework**:

1. **Critical** (do first):
   - Blocking bugs
   - PR review requests
   - Build failures
   - Performance regressions

2. **High** (schedule today):
   - Feature development assigned to you
   - Testing new content
   - Integration work
   - Documentation updates

3. **Medium** (if time permits):
   - Code refactoring
   - Optimization tasks
   - Polish work
   - Asset creation

4. **Low** (backlog):
   - Nice-to-have features
   - Experimental work
   - Future planning

**Create Daily Task List**:
```
Today's Goals (November 8, 2025):
1. [ ] CRITICAL: Fix mission board loading crash
2. [ ] HIGH: Implement 5 new mission templates
3. [ ] HIGH: Test VFX performance in VR
4. [ ] MEDIUM: Optimize particle systems
5. [ ] LOW: Explore new UI designs

Blockers:
- None / [Describe blocker and who can help]

Questions for Team:
- [Any questions for standup?]
```

---

## Development Workflow

**Goal**: Efficient, organized development with minimal context switching

### Standard Development Cycle

```
Plan → Code → Build → Test → Profile → Review → Commit → Deploy
  ↑                                                           ↓
  └───────────────────────── Iterate ────────────────────────┘
```

---

### Step 1: Create Feature Branch (2 minutes)

```bash
# Always work on feature branches, never on master
git checkout -b feature/mission-templates-sabotage

# Branch naming convention:
# feature/[feature-name]
# bugfix/[bug-description]
# hotfix/[critical-fix]
# content/[content-type]
# optimize/[system-name]
```

**Good Branch Names**:
- `feature/mission-templates-sabotage`
- `bugfix/vfx-particle-crash`
- `content/station-trading-hub`
- `optimize/particle-systems`

**Bad Branch Names**:
- `my-branch`
- `test`
- `fix`
- `updates`

---

### Step 2: Develop Feature (varies)

**Best Practices**:

**A. Small, Focused Changes**
```
✓ One feature per branch
✓ One concept per commit
✗ Multiple unrelated changes
✗ Giant commits with everything
```

**B. Code Often, Commit Often**
```bash
# Save work frequently
git add <files>
git commit -m "Descriptive message"

# Commit every 30-60 minutes or logical checkpoint
```

**C. Use TODO Comments**
```cpp
// TODO(your-name): Optimize this loop for large arrays
// FIXME(your-name): Handle edge case when array is empty
// NOTE(your-name): This assumes positive values only
// HACK(your-name): Temporary fix until proper solution in v2
```

**D. Document as You Go**
```cpp
/**
 * Processes mission objectives and updates player progress.
 *
 * @param Objectives Array of mission objectives to process
 * @param PlayerProgress Current player progress data
 * @return True if all objectives complete, false otherwise
 *
 * Example:
 *   TArray<FObjective> Objectives = Mission->GetObjectives();
 *   FPlayerProgress Progress = Player->GetProgress();
 *   bool bComplete = ProcessObjectives(Objectives, Progress);
 */
bool ProcessObjectives(const TArray<FObjective>& Objectives, FPlayerProgress& PlayerProgress);
```

---

### Step 3: Build Project (5-10 minutes)

**Development Build** (fast iteration):
```bash
# From command line (faster than IDE)
cd /c/Users/allen/Desktop/Alexander/Alexander

"C:/Program Files/Epic Games/UE_5.6/Engine/Build/BatchFiles/Build.bat" AlexanderEditor Win64 Development "C:/Users/allen/Desktop/Alexander/Alexander/Alexander.uproject"

# Check for errors
# Fix any compilation errors before proceeding
```

**Or use Python script**:
```bash
python build_and_test.py --build-only
```

**Build Success Checklist**:
- [ ] No compilation errors
- [ ] No linker errors
- [ ] No missing include errors
- [ ] Build time reasonable (< 5 minutes for incremental)

**If Build Fails**:
```bash
# Clean build (if incremental build fails)
"C:/Program Files/Epic Games/UE_5.6/Engine/Build/BatchFiles/Clean.bat" AlexanderEditor Win64 Development "C:/Users/allen/Desktop/Alexander/Alexander/Alexander.uproject"

# Rebuild
"C:/Program Files/Epic Games/UE_5.6/Engine/Build/BatchFiles/Build.bat" AlexanderEditor Win64 Development "C:/Users/allen/Desktop/Alexander/Alexander/Alexander.uproject"

# If still fails, check:
# 1. Recent changes (git diff)
# 2. Missing files
# 3. Include paths
# 4. Ask team for help
```

---

### Step 4: Test Locally (15-30 minutes)

**Quick Smoke Test**:
```bash
# Launch editor
"C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor.exe" "C:/Users/allen/Desktop/Alexander/Alexander/Alexander.uproject"

# Manual checks:
1. Editor launches without crashes
2. Test level loads
3. Play In Editor (PIE) works
4. Your changes appear/work
5. No obvious regressions
```

**Feature-Specific Testing**:
```
Based on what you changed:

Mission Templates:
├── [ ] Mission appears in mission board
├── [ ] Variables substitute correctly
├── [ ] Objectives function properly
├── [ ] Rewards granted correctly
└── [ ] Mission completes successfully

VFX:
├── [ ] Effect spawns at correct location
├── [ ] Effect looks correct (colors, size)
├── [ ] Effect performs well (check FPS)
├── [ ] Effect despawns properly
└── [ ] No visual artifacts

Station:
├── [ ] Station appears in world
├── [ ] Docking works
├── [ ] Mission board functional
├── [ ] Trading interface opens
└── [ ] Interior navigation works

UI:
├── [ ] Widget appears on screen
├── [ ] Data updates correctly
├── [ ] Interactions work
├── [ ] Readable in VR
└── [ ] No layout issues
```

---

### Step 5: Profile Performance (10-15 minutes)

**Every significant change should be profiled**

```bash
# Run profiling script
python autonomous_profiling_test.py

# Or manual profiling in editor
# 1. Launch editor
# 2. Open test level
# 3. Run console commands:

stat FPS
stat Unit
stat GPU
stat SceneRendering

# Record baseline:
# Frame Time: ___ ms (target < 11.1ms for 90 FPS)
# Game Thread: ___ ms
# Render Thread: ___ ms
# GPU: ___ ms

# With your changes:
# Frame Time: ___ ms
# Delta: ___ ms

# If delta > 1ms, investigate and optimize
```

**Performance Targets**:
```
VR (90 FPS):
├── Frame Time: < 11.1ms
├── Game Thread: < 8ms
├── Render Thread: < 8ms
└── GPU: < 11ms

Desktop (60 FPS):
├── Frame Time: < 16.6ms
├── Game Thread: < 12ms
├── Render Thread: < 12ms
└── GPU: < 16ms
```

---

### Step 6: Code Review (Self-Review) (10 minutes)

**Before committing, review your own changes**:

```bash
# See all changes
git diff

# See specific file
git diff Source/Alexander/Private/MissionSystem.cpp

# Use a diff tool for better visualization
git difftool
```

**Self-Review Checklist**:
- [ ] Code follows project style guide
- [ ] No debug code left in (print statements, test code)
- [ ] No commented-out code (remove or explain why)
- [ ] Variable names are descriptive
- [ ] Functions have clear purpose
- [ ] No magic numbers (use constants)
- [ ] Error handling is present
- [ ] No obvious bugs or typos
- [ ] Comments explain "why", not "what"
- [ ] No performance red flags

**Code Quality Checks**:
```cpp
// Bad:
void Update()
{
    if (a > 5)  // What is 5? What is a?
    {
        b = c * 2;  // Why multiply by 2?
    }
}

// Good:
void UpdatePlayerHealth()
{
    const float CRITICAL_HEALTH_THRESHOLD = 5.0f;
    const float REGENERATION_MULTIPLIER = 2.0f;

    if (CurrentHealth > CRITICAL_HEALTH_THRESHOLD)
    {
        // Apply bonus regeneration when above critical health
        HealthRegenRate = BaseRegenRate * REGENERATION_MULTIPLIER;
    }
}
```

---

## Testing Workflow

**Goal**: Catch bugs early, ensure quality

### Manual Testing (Daily)

**Quick Test** (5 minutes):
```
1. Launch editor
2. PIE (Play In Editor)
3. Test your changes
4. Verify no crashes
5. Exit cleanly
```

**Full Test** (15-30 minutes):
```
1. Load each relevant test level
2. Test all paths (success, failure, edge cases)
3. Test with different data
4. Test error handling
5. Test in VR (if VR feature)
6. Check for memory leaks (stat Memory)
7. Document any issues found
```

---

### Automated Testing (Before Commit)

```bash
# Run all automated tests
python build_and_test.py

# Run specific test suite
python build_and_test.py --suite=Mission

# Check test results
cat TEST_REPORT.md
cat TEST_REPORT.json

# All tests should PASS before committing
```

**Test Report Review**:
```
✓ All tests passed: Proceed to commit
⚠ Some tests failed: Fix before committing
✗ Critical tests failed: Do not commit, fix immediately
```

---

### Regression Testing (Weekly)

**Ensure old features still work**:

```
Regression Test Areas:
├── [ ] Core gameplay (flight, combat, trading)
├── [ ] Mission system (accept, complete, rewards)
├── [ ] UI/HUD (all screens functional)
├── [ ] VR interactions (grab, teleport, UI)
├── [ ] Performance (no degradation)
└── [ ] Save/Load (data persists correctly)

If regression found:
1. Create bug ticket
2. Assign priority
3. Fix before next release
```

---

## Performance Monitoring

**Goal**: Maintain 90 FPS in VR throughout development

### Daily Performance Check (10 minutes)

**Baseline Test**:
```bash
# Run performance baseline script
python phase9_performance_baseline.py

# Review results
cat PHASE9_PERFORMANCE_OPTIMIZATION_PLAN.md

# Check for regressions:
# - Frame time increased?
# - Memory usage increased?
# - Draw calls increased?
```

**Performance Dashboard**:
```
Current Performance Metrics (Nov 8, 2025):
├── Frame Time: 9.2ms (target < 11.1ms) ✓
├── FPS: 108 (target > 90) ✓
├── Memory: 4.2GB / 8GB (52%) ✓
├── Draw Calls: 1,234 (target < 2000) ✓
├── Particles: 15,432 (target < 50,000) ✓
└── Audio Channels: 24 / 64 (38%) ✓

Trend: Stable / Improving / Degrading
Action: None / Monitor / Optimize
```

---

### Weekly Performance Review (30 minutes)

**Deep Dive Analysis**:

```bash
# Run comprehensive profiling
python autonomous_profiling_test.py --full

# Generate detailed report
# Review bottlenecks
# Identify optimization opportunities
# Create optimization tasks
```

**Profiling Areas**:
```
1. CPU Performance:
   ├── Game Thread
   ├── Render Thread
   ├── Physics Thread
   └── Audio Thread

2. GPU Performance:
   ├── Draw Calls
   ├── Shader Complexity
   ├── Texture Memory
   └── Post-Processing

3. Memory:
   ├── Heap Usage
   ├── Asset Memory
   ├── Streaming Pool
   └── Garbage Collection

4. I/O:
   ├── Asset Loading
   ├── Level Streaming
   ├── Save/Load Times
   └── Network (if applicable)
```

**Performance Optimization Priority**:
```
Priority 1 (Critical):
- Frame time > 11.1ms (below 90 FPS)
- Memory leaks
- Crashes

Priority 2 (High):
- Frame time 10-11ms (approaching limit)
- Hitching/stuttering
- Load times > 5 seconds

Priority 3 (Medium):
- Frame time 8-10ms (optimize for headroom)
- Memory usage > 75%
- Minor optimizations

Priority 4 (Low):
- Polish optimizations
- Future-proofing
- Nice-to-haves
```

---

## Git Best Practices

**Goal**: Clean commit history, easy collaboration

### Commit Message Format

**Template**:
```
<type>(<scope>): <short summary>

<detailed description if needed>

<footer: issue refs, breaking changes, etc.>
```

**Types**:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation only
- `style`: Code style changes (formatting, no logic change)
- `refactor`: Code refactoring (no functionality change)
- `perf`: Performance improvement
- `test`: Adding or updating tests
- `chore`: Maintenance tasks

**Examples**:

**Good Commit Messages**:
```
feat(missions): Add sabotage mission type

Implements sabotage missions with stealth mechanics.
Includes 3 template variants and integration with mission board.

Closes #123
```

```
fix(vfx): Prevent particle overflow crash

Fixed crash when spawning > 100k particles by adding
spawn rate limiter and distance culling.

Performance improved from 45 FPS to 90 FPS in stress test.
```

```
perf(rendering): Optimize station mesh LODs

Reduced polygon count by 40% for distant LODs.
Frame time improved by 1.2ms in station approach.
```

**Bad Commit Messages**:
```
✗ "Fixed stuff"
✗ "WIP"
✗ "Updates"
✗ "asdf"
✗ "Fixed bug" (which bug?)
```

---

### Commit Workflow

**Step 1: Stage Changes** (2 minutes)
```bash
# Review changes
git status

# Stage specific files
git add Source/Alexander/Private/MissionSystem.cpp
git add Content/Data/Missions/Templates/mission_templates_sabotage.json

# Or stage all (use with caution)
git add .

# Verify what's staged
git diff --staged
```

**Step 2: Commit** (2 minutes)
```bash
# Commit with message
git commit -m "feat(missions): Add sabotage mission type

Implements sabotage missions with stealth mechanics.
Includes 3 template variants and integration with mission board.

Closes #123"

# Or use editor for longer messages
git commit
# (Opens editor for multi-line message)
```

**Step 3: Push to Remote** (1 minute)
```bash
# First time pushing branch
git push -u origin feature/mission-templates-sabotage

# Subsequent pushes
git push
```

---

### Branch Management

**When to Create Branch**:
```
✓ Starting new feature
✓ Fixing a bug
✓ Experimenting with idea
✓ Creating content batch
```

**When to Merge Branch**:
```
✓ Feature complete and tested
✓ All tests pass
✓ Code reviewed
✓ No merge conflicts
✓ CI/CD passes (if applicable)
```

**Branch Cleanup**:
```bash
# After branch merged, delete local branch
git branch -d feature/mission-templates-sabotage

# Delete remote branch
git push origin --delete feature/mission-templates-sabotage

# Prune deleted remote branches
git remote prune origin
```

---

### Pull Request Best Practices

**Before Creating PR**:
```bash
# 1. Ensure branch is up to date
git fetch origin master
git rebase origin/master  # Or merge

# 2. Resolve any conflicts

# 3. Run all tests
python build_and_test.py

# 4. Verify build succeeds

# 5. Push final changes
git push
```

**Creating PR**:
```bash
# Using GitHub CLI
gh pr create --title "Add sabotage mission type" --body "$(cat <<'EOF'
## Summary
- Implemented sabotage mission type with stealth mechanics
- Added 3 mission template variants
- Integrated with mission board and faction system

## Test Plan
- [x] Mission templates load correctly
- [x] Stealth mechanics function properly
- [x] Mission board displays sabotage missions
- [x] Rewards granted correctly
- [x] Performance test passed (90 FPS maintained)

## Screenshots
[Attach screenshots if applicable]

## Notes
- Requires latest mission system changes from PR #122

Generated with Claude Code
EOF
)"
```

**PR Review Checklist** (for reviewers):
```
Code Quality:
├── [ ] Follows project conventions
├── [ ] Well-documented
├── [ ] No obvious bugs
├── [ ] Error handling present
└── [ ] No security issues

Functionality:
├── [ ] Feature works as described
├── [ ] Tests pass
├── [ ] No regressions
└── [ ] Edge cases handled

Performance:
├── [ ] No performance regressions
├── [ ] Efficient algorithms
└── [ ] Resource usage acceptable

Integration:
├── [ ] Integrates well with existing systems
├── [ ] No conflicts
└── [ ] Documentation updated
```

---

## Daily Standup Format

**Duration**: 15 minutes max (3 minutes per person max)
**Frequency**: Daily, same time

### Standup Template

**1. What did you accomplish yesterday?**
```
Example:
"Yesterday I completed 5 sabotage mission templates, fixed the
particle overflow crash, and started optimization work on station
LODs. All changes are in PR #145."
```

**2. What will you work on today?**
```
Example:
"Today I plan to finish station LOD optimization, test the new
mission templates in VR, and review PR #142 from Sarah."
```

**3. Any blockers?**
```
Example:
"Blocked on mission board integration - waiting for API changes
from John (PR #143). Can you review that today?"

Or:
"No blockers."
```

**4. Any questions or help needed?**
```
Example:
"Has anyone worked with the new Niagara GPU particles? I need
advice on optimization."

Or:
"All clear."
```

---

### Standup Best Practices

**Do**:
- Be concise and specific
- Mention PR numbers
- Flag blockers immediately
- Ask for help when needed
- Stay on topic

**Don't**:
- Go into excessive technical detail
- Solve problems during standup (take offline)
- Report every tiny task
- Miss standups without notification
- Wait until standup to flag critical blockers

**If Async Standup** (written):
```
Update for November 8, 2025

Yesterday:
- ✓ Completed 5 sabotage mission templates
- ✓ Fixed particle overflow crash (PR #145)
- ✓ Started station LOD optimization

Today:
- Finish station LOD optimization
- Test mission templates in VR
- Review PR #142

Blockers:
- Waiting for mission board API changes (PR #143)

Help Needed:
- Advice on Niagara GPU particle optimization

Status: On Track / At Risk / Blocked
```

---

## End-of-Day Checklist

**Duration**: 10-15 minutes
**Goal**: Clean shutdown, ready for tomorrow

### Step 1: Save All Work (2 minutes)

```bash
# Commit any uncommitted work
git status

# If work in progress, commit with WIP tag
git add .
git commit -m "WIP: Station LOD optimization (in progress)"
git push

# Or stash if not ready to commit
git stash save "WIP: Station LODs - testing needed"
```

---

### Step 2: Update Progress Tracking (5 minutes)

**Update Project Tracker**:
```markdown
# In PHASE8_PROGRESS_TRACKER.md or task management tool

Week 1 - Day 3 (November 8, 2025):

Completed:
- [x] 5 sabotage mission templates
- [x] Particle overflow crash fix
- [x] Station LOD optimization (80% complete)

In Progress:
- [ ] Station LOD optimization (testing needed)
- [ ] VR testing of mission templates

Blocked:
- Mission board integration (waiting on PR #143)

Tomorrow's Plan:
- Complete station LOD testing
- VR test mission templates
- Review PR #142
```

**Update Personal Notes**:
```
Things to Remember:
- Station LOD reduction: 40% polygon reduction working well
- Particle spawn rate limiter: 50k cap works for VR
- New mission templates need translation strings
- Talk to Sarah about VFX optimization techniques

Ideas to Explore:
- Automated mission template validation
- Procedural station generation
- Dynamic LOD based on VR performance
```

---

### Step 3: Run Final Build/Test (5 minutes)

```bash
# Quick build to ensure nothing broken
python build_and_test.py --quick

# If build succeeds, good to go
# If build fails, fix before leaving (or notify team)
```

---

### Step 4: Clean Up Environment (3 minutes)

```
Workspace Cleanup:
├── [ ] Close unnecessary applications
├── [ ] Close unused browser tabs
├── [ ] Save/close documents
├── [ ] Clear desktop clutter
└── [ ] Update tomorrow's task list

Code Cleanup:
├── [ ] Remove debug print statements
├── [ ] Delete temporary files
├── [ ] Close unused IDE tabs
└── [ ] Commit or stash changes
```

---

### Step 5: Prepare for Tomorrow (2 minutes)

```
Tomorrow's Setup:
├── [ ] Review tomorrow's tasks
├── [ ] Check calendar for meetings
├── [ ] Note any dependencies
├── [ ] Identify potential blockers
└── [ ] Set priorities
```

**Tomorrow's Task List**:
```
High Priority:
1. Complete station LOD testing
2. VR test mission templates
3. Review PR #142

Medium Priority:
4. Start VFX optimization
5. Update documentation

Low Priority:
6. Explore automated validation
7. Refactor mission loading code

Meetings:
- 10:00 AM - Team standup
- 2:00 PM - Design review (optional)
```

---

## Weekly Routines

### Monday: Week Planning

```
Week Planning (30 minutes):
├── Review last week's progress
├── Set this week's goals
├── Identify blockers
├── Coordinate with team
└── Update project tracker
```

### Friday: Week Review

```
Week Review (30 minutes):
├── Review completed tasks
├── Document lessons learned
├── Update documentation
├── Clean up branches
├── Backup important work
└── Plan next week
```

---

## Emergency Procedures

### Build Broken

```
1. Check if local issue or team-wide
   git pull origin master
   Clean build

2. If team-wide, check recent commits
   git log --oneline -10

3. Identify breaking commit
   git bisect start
   git bisect bad HEAD
   git bisect good <last-known-good-commit>

4. Notify team immediately
   "Build broken as of commit <hash>. Investigating..."

5. Fix or revert
   git revert <bad-commit>

6. Verify fix
   Build and test

7. Push fix
   git push
```

### Critical Bug in Production

```
1. Assess severity
   - Game-breaking? Hotfix immediately
   - Minor issue? Schedule for next patch

2. Create hotfix branch
   git checkout -b hotfix/critical-bug-description master

3. Fix bug (minimal changes)

4. Test thoroughly

5. Create PR with "HOTFIX" label

6. Fast-track review

7. Merge and deploy

8. Document post-mortem
```

### Lost Work

```
1. Check git reflog
   git reflog

2. Find lost commit
   git show <commit-hash>

3. Restore work
   git cherry-pick <commit-hash>
   # Or
   git reset --hard <commit-hash>

4. If not in git, check:
   - Editor auto-saves
   - Local backup (if enabled)
   - Recycle bin/trash

5. Prevention:
   - Commit frequently
   - Push to remote daily
   - Enable auto-save
```

---

## Tips for Productivity

### Time Management

**Pomodoro Technique**:
```
1. Work focused for 25 minutes
2. Take 5 minute break
3. Repeat 4 times
4. Take 15-30 minute break
5. Repeat cycle
```

**Deep Work Blocks**:
```
Morning: 9:00-12:00 (3 hours deep work)
- No interruptions
- Focus on hardest problems
- Maximum productivity

Afternoon: 1:00-5:00 (meetings, reviews, lighter tasks)
```

### Avoiding Context Switching

```
✓ Batch similar tasks together
✓ Disable notifications during focus time
✓ Complete one task before starting another
✓ Use "Do Not Disturb" mode

✗ Checking email every 5 minutes
✗ Jumping between unrelated tasks
✗ Multitasking complex problems
```

### Staying Motivated

```
Daily Wins:
- Celebrate small victories
- Track completed tasks
- Share progress with team
- Take breaks when stuck

Avoid Burnout:
- Don't work excessive hours
- Take regular breaks
- Ask for help when needed
- Maintain work-life balance
```

---

## Useful Commands Reference

### Git Commands

```bash
# Status and info
git status
git log --oneline -10
git diff
git branch --all

# Branching
git checkout -b feature/new-feature
git branch -d feature/old-feature

# Staging and committing
git add <files>
git commit -m "message"
git push

# Syncing
git fetch origin
git pull origin master
git merge origin/master

# Undoing changes
git reset HEAD <file>  # Unstage
git checkout -- <file>  # Discard changes
git revert <commit>    # Revert commit

# Stashing
git stash save "description"
git stash list
git stash pop
```

### Build Commands

```bash
# Development build
"C:/Program Files/Epic Games/UE_5.6/Engine/Build/BatchFiles/Build.bat" AlexanderEditor Win64 Development "C:/Users/allen/Desktop/Alexander/Alexander/Alexander.uproject"

# Clean build
"C:/Program Files/Epic Games/UE_5.6/Engine/Build/BatchFiles/Clean.bat" AlexanderEditor Win64 Development "C:/Users/allen/Desktop/Alexander/Alexander/Alexander.uproject"

# Shipping build
"C:/Program Files/Epic Games/UE_5.6/Engine/Build/BatchFiles/Build.bat" Alexander Win64 Shipping "C:/Users/allen/Desktop/Alexander/Alexander/Alexander.uproject"
```

### Test Commands

```bash
# Run all tests
python build_and_test.py

# Quick build and test
python build_and_test.py --quick

# Run profiling
python autonomous_profiling_test.py
```

### Unreal Console Commands

```
// Performance
stat FPS
stat Unit
stat GPU
stat SceneRendering
stat Particles
stat Niagara

// Debugging
show Collision
show Bounds
show NavMesh
stat Memory

// Profiling
stat StartFile
stat StopFile

// VR
vr.PixelDensity 1.0
vr.HeadTrackingBoundsRadius 2000
```

---

## Document Version Control

**Version**: 1.0
**Last Updated**: November 8, 2025
**Maintained By**: Development Team
**Changelog**:
- v1.0 (Nov 8, 2025): Initial version

**Feedback**: Please suggest improvements or report issues

**Related Documents**:
- `QUICK_START_TEMPLATES.md` - Ready-to-use templates
- `COMMON_TASKS_COOKBOOK.md` - Step-by-step task recipes
- `PHASE8_CONTENT_CREATION_GUIDE.md` - Master content guide
- `PHASE8_PROGRESS_TRACKER.md` - Progress tracking

---

**Remember**: Consistency over intensity. Daily routines compound into major results over time.
