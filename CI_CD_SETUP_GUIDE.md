# Alexander CI/CD Pipeline Setup Guide

**Last Updated:** November 4, 2025  
**Version:** 1.0  
**Platform:** GitHub Actions

---

## Table of Contents

1. [Overview](#overview)
2. [Prerequisites](#prerequisites)
3. [Pipeline Architecture](#pipeline-architecture)
4. [Setup Instructions](#setup-instructions)
5. [Pipeline Jobs](#pipeline-jobs)
6. [Workflow Triggers](#workflow-triggers)
7. [Test Reports](#test-reports)
8. [Performance Monitoring](#performance-monitoring)
9. [Troubleshooting](#troubleshooting)
10. [Customization](#customization)
11. [Best Practices](#best-practices)

---

## Overview

The Alexander CI/CD pipeline provides automated testing, validation, and quality checks for every code change. It ensures that all systems remain functional and performant throughout development.

### Key Features

✓ **Automated Testing** - 229 tests executed on every push/PR  
✓ **Performance Regression Detection** - Monitors test execution times  
✓ **Code Quality Checks** - Python linting and style validation  
✓ **Documentation Validation** - Ensures critical docs are present  
✓ **Security Scanning** - Checks for exposed secrets/credentials  
✓ **Daily Scheduled Runs** - Continuous validation even without changes  
✓ **Artifact Retention** - Test reports stored for 30 days  

### Pipeline Status

The pipeline consists of **7 jobs** running across **~90 minutes**:

1. Unit Tests (30 min)
2. End-to-End Tests (60 min)
3. Performance Check (20 min)
4. Code Quality (15 min)
5. Documentation Check (10 min)
6. Build Summary (10 min)
7. Security Scan (15 min) - main branch only

---

## Prerequisites

### Required

- **GitHub Repository** - Project must be hosted on GitHub
- **Python 3.10+** - For test execution
- **Git** - Version control system
- **Windows Runner** - Tests designed for Windows environment

### Optional

- **Slack/Discord Webhook** - For build notifications
- **Branch Protection Rules** - Enforce CI checks before merge
- **GitHub Actions Secrets** - For sensitive configuration

---

## Pipeline Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     Trigger Events                          │
│  (Push to main/develop, PR, Schedule, Manual)              │
└───────────────────┬─────────────────────────────────────────┘
                    │
                    ▼
        ┌───────────────────────┐
        │   Unit Tests (26)     │
        │  ~2.5s execution      │
        └───────────┬───────────┘
                    │
                    ▼
        ┌───────────────────────┐
        │   E2E Tests (203)     │
        │  Full mission flow    │
        └───────────┬───────────┘
                    │
                    ▼
        ┌───────────────────────┐
        │ Performance Analysis  │
        │  Regression detection │
        └───────────┬───────────┘
                    │
        ┌───────────┴───────────┬───────────────┬────────────┐
        │                       │               │            │
        ▼                       ▼               ▼            ▼
┌───────────────┐   ┌──────────────────┐  ┌──────────┐  ┌────────┐
│ Code Quality  │   │ Documentation    │  │ Security │  │Summary │
│   Checks      │   │    Validation    │  │   Scan   │  │ Report │
└───────────────┘   └──────────────────┘  └──────────┘  └────────┘
```

---

## Setup Instructions

### Step 1: Verify File Structure

Ensure your repository has:

```
Alexander/
├── .github/
│   └── workflows/
│       └── ci.yml                    # Created ✓
├── run_comprehensive_tests.py         # Unit tests
├── run_end_to_end_tests.py           # E2E tests
├── TEST_VALIDATION_DOCUMENTATION.md   # Test docs
├── VISUAL_QUALITY_VALIDATION.md       # Quality docs
├── PROJECT_TEST_STATUS.md             # Status report
└── CI_CD_SETUP_GUIDE.md              # This file
```

### Step 2: Initialize Git Repository (if needed)

```bash
git init
git add .
git commit -m "Initial commit with CI/CD pipeline"
```

### Step 3: Push to GitHub

```bash
# Create repository on GitHub, then:
git remote add origin https://github.com/YOUR_USERNAME/Alexander.git
git branch -M main
git push -u origin main
```

### Step 4: Verify Workflow File

1. Go to your GitHub repository
2. Navigate to **Actions** tab
3. You should see "Alexander CI/CD Pipeline" workflow
4. If not visible, check `.github/workflows/ci.yml` exists

### Step 5: Enable GitHub Actions (if needed)

1. Go to **Settings** → **Actions** → **General**
2. Ensure "Allow all actions and reusable workflows" is selected
3. Click **Save**

### Step 6: Configure Branch Protection (Optional)

For `main` and `develop` branches:

1. Go to **Settings** → **Branches**
2. Add rule for `main`:
   - ✓ Require status checks to pass before merging
   - ✓ Require branches to be up to date before merging
   - Select checks:
     - Unit Tests
     - E2E Tests
     - Documentation Validation
3. Repeat for `develop` branch

---

## Pipeline Jobs

### Job 1: Unit Tests

**Duration:** ~30 minutes  
**Purpose:** Validate individual system functionality

- Runs `run_comprehensive_tests.py`
- Executes 26 unit tests across 6 categories
- Generates `TEST_REPORT.json` and `TEST_REPORT.md`
- Uploads reports as artifacts
- **Fails if:** Any test fails or execution time > 3.6s (20% over baseline)

### Job 2: End-to-End Tests

**Duration:** ~60 minutes  
**Purpose:** Validate complete game flow

- Requires Unit Tests to pass first
- Runs `run_end_to_end_tests.py`
- Executes 203 tests across 9 mission phases
- Generates `END_TO_END_TEST_REPORT.json` and `END_TO_END_TEST_REPORT.md`
- **Fails if:** Any test fails

### Job 3: Performance Regression Check

**Duration:** ~20 minutes  
**Purpose:** Detect performance degradation

- Analyzes test execution times
- Compares against baseline (3.0s for unit tests)
- Warns if tests exceed 20% threshold
- Reviews E2E test phase results
- **Warns but doesn't fail** on performance issues

### Job 4: Code Quality Check

**Duration:** ~15 minutes  
**Purpose:** Maintain code standards

- Installs: pylint, flake8, black
- Scans all Python files (excludes venv, Intermediate, Saved)
- Checks style compliance (120 char line length)
- **Non-blocking** - provides warnings only

### Job 5: Documentation Validation

**Duration:** ~10 minutes  
**Purpose:** Ensure documentation completeness

Required files checked:
- `README.md`
- `PROJECT_TEST_STATUS.md`
- `TEST_VALIDATION_DOCUMENTATION.md`
- `VISUAL_QUALITY_VALIDATION.md`

**Fails if:** Any required documentation is missing

### Job 6: Build Summary

**Duration:** ~10 minutes  
**Purpose:** Aggregate and report results

- Downloads all test artifacts
- Generates comprehensive build summary
- Reports status of all jobs
- **Fails if:** Any critical job (Unit, E2E, Docs) fails

### Job 7: Security Scan

**Duration:** ~15 minutes  
**Purpose:** Detect security vulnerabilities

- **Only runs on:** Pushes to `main` branch
- Scans for sensitive patterns:
  - Hardcoded passwords
  - API keys
  - Secrets/tokens
- **Non-blocking** - provides warnings only

---

## Workflow Triggers

### Automatic Triggers

1. **Push to main/develop**
   ```yaml
   on:
     push:
       branches: [ main, develop ]
   ```
   - Triggers on every commit
   - Full pipeline execution

2. **Pull Requests**
   ```yaml
   on:
     pull_request:
       branches: [ main, develop ]
   ```
   - Runs on PR creation/update
   - Validates before merge

3. **Scheduled Runs**
   ```yaml
   on:
     schedule:
       - cron: '0 2 * * *'  # 2 AM UTC daily
   ```
   - Daily validation
   - Catches environment drift

### Manual Trigger

```yaml
on:
  workflow_dispatch:
```

**To trigger manually:**
1. Go to **Actions** tab
2. Select "Alexander CI/CD Pipeline"
3. Click **Run workflow**
4. Select branch
5. Click **Run workflow**

---

## Test Reports

### Accessing Reports

After pipeline execution:

1. Go to **Actions** tab
2. Select the workflow run
3. Scroll to **Artifacts** section
4. Download desired reports:
   - `unit-test-reports` (TEST_REPORT.json/md)
   - `e2e-test-reports` (END_TO_END_TEST_REPORT.json/md)
   - `build-summary` (All artifacts)

### Report Retention

- **Duration:** 30 days
- **Size Limit:** 2GB per artifact (GitHub default)
- **Total Limit:** 500MB per repository (free tier)

### Report Contents

**Unit Test Report:**
```json
{
  "metadata": {
    "total_tests": 26,
    "timestamp": "2025-11-04T22:00:00Z",
    "total_time_seconds": 2.5
  },
  "summary": {
    "passed": 26,
    "failed": 0,
    "skipped": 0,
    "pass_rate": 100.0
  },
  "categories": { /* ... */ }
}
```

**E2E Test Report:**
```json
{
  "summary": {
    "total_phases": 9,
    "total_tests": 203,
    "total_passed": 203,
    "total_failed": 0
  },
  "phases": [ /* ... */ ]
}
```

---

## Performance Monitoring

### Baseline Metrics

| Metric | Baseline | Threshold | Action |
|--------|----------|-----------|--------|
| Unit Tests | 3.0s | 3.6s (+20%) | Warning |
| E2E Tests | N/A | Test failures | Fail |
| Memory Usage | 4GB | 4.8GB (+20%) | Warning |
| Tick Budget | 5ms | 6ms (+20%) | Warning |

### Regression Detection

The pipeline monitors:

1. **Test Execution Time**
   - Compares against baseline
   - Warns if > 20% slower
   - Trend analysis over time

2. **Test Pass Rate**
   - Must maintain 100%
   - Immediate failure on any test failure

3. **System Performance**
   - VR frame rate (90 FPS target)
   - Memory usage (4GB budget)
   - Tick time (5ms budget)

### Viewing Performance Trends

1. Go to **Actions** tab
2. Select multiple workflow runs
3. Compare execution times
4. Download artifacts for detailed analysis

---

## Troubleshooting

### Common Issues

#### Issue 1: Workflow Not Triggering

**Symptoms:** No pipeline runs after push

**Solutions:**
- Verify `.github/workflows/ci.yml` exists
- Check GitHub Actions are enabled (Settings → Actions)
- Ensure branch name matches trigger (main/develop)
- Check for syntax errors in YAML

#### Issue 2: Tests Failing Locally Work in CI

**Symptoms:** Local tests pass, CI fails

**Solutions:**
- Check Python version matches (3.10)
- Verify all dependencies installed
- Check for platform-specific issues
- Review test isolation

#### Issue 3: Performance Warnings

**Symptoms:** Tests exceed time thresholds

**Solutions:**
- Review test execution logs
- Identify slow tests
- Optimize test logic
- Adjust baseline if justified

#### Issue 4: Artifact Upload Fails

**Symptoms:** Reports not available

**Solutions:**
- Check report files are generated
- Verify file paths in workflow
- Ensure runner has write permissions
- Check artifact size < 2GB

#### Issue 5: Security Scan False Positives

**Symptoms:** Warnings for non-sensitive data

**Solutions:**
- Review patterns in workflow
- Add comments to clarify false positives
- Adjust regex patterns if needed
- Use GitHub Secrets for real credentials

### Debug Mode

Enable debug logging:

1. Go to **Settings** → **Secrets and variables** → **Actions**
2. Add repository variable:
   - Name: `ACTIONS_RUNNER_DEBUG`
   - Value: `true`
3. Add another variable:
   - Name: `ACTIONS_STEP_DEBUG`
   - Value: `true`
4. Re-run workflow

### Getting Help

1. Check workflow logs in Actions tab
2. Review individual job logs
3. Download artifacts for detailed analysis
4. Check GitHub Actions documentation
5. Review this guide's troubleshooting section

---

## Customization

### Adjusting Test Timeouts

Edit `.github/workflows/ci.yml`:

```yaml
jobs:
  unit-tests:
    timeout-minutes: 30  # Change this value
```

### Adding Notification

**Slack Integration:**

```yaml
- name: Notify Slack
  if: always()
  uses: 8398a7/action-slack@v3
  with:
    status: ${{ job.status }}
    webhook_url: ${{ secrets.SLACK_WEBHOOK }}
```

**Discord Integration:**

```yaml
- name: Notify Discord
  if: always()
  uses: sarisia/actions-status-discord@v1
  with:
    webhook: ${{ secrets.DISCORD_WEBHOOK }}
```

### Adding More Tests

1. Update test scripts (`run_comprehensive_tests.py`, `run_end_to_end_tests.py`)
2. Tests automatically included in next run
3. No workflow changes needed

### Changing Schedule

Edit cron expression:

```yaml
schedule:
  - cron: '0 2 * * *'  # Daily at 2 AM UTC
  # - cron: '0 */4 * * *'  # Every 4 hours
  # - cron: '0 0 * * 0'  # Weekly on Sunday
```

### Custom Performance Baselines

Edit performance-check job:

```yaml
$baselineTime = 3.0  # Adjust baseline
$threshold = $baselineTime * 1.2  # Adjust threshold percentage
```

---

## Best Practices

### For Developers

1. **Run Tests Locally First**
   ```bash
   python run_comprehensive_tests.py
   python run_end_to_end_tests.py
   ```

2. **Small, Focused Commits**
   - Easier to debug CI failures
   - Faster feedback loops

3. **Review CI Logs**
   - Check warnings even if build passes
   - Monitor performance trends

4. **Keep Tests Fast**
   - Unit tests should run in seconds
   - Mock external dependencies
   - Use test fixtures

5. **Update Documentation**
   - Keep docs in sync with code
   - CI will fail if docs are missing

### For Teams

1. **Branch Protection**
   - Require CI checks before merge
   - Require code reviews
   - Keep main branch stable

2. **Regular Monitoring**
   - Review daily scheduled run results
   - Address performance degradation early
   - Keep artifact storage under limits

3. **Performance Budgets**
   - Set and enforce performance targets
   - Monitor trends over time
   - Adjust baselines when justified

4. **Test Coverage**
   - Add tests for new features
   - Maintain 100% pass rate
   - Review test quality regularly

5. **Security**
   - Never commit secrets
   - Use GitHub Secrets for credentials
   - Review security scan results

---

## Quick Reference

### Run Pipeline Manually

```bash
# Via GitHub UI
Actions → Alexander CI/CD Pipeline → Run workflow

# Via GitHub CLI
gh workflow run ci.yml
```

### Check Pipeline Status

```bash
# Via GitHub CLI
gh run list --workflow=ci.yml
gh run view <run-id>
```

### Download Test Reports

```bash
# Via GitHub CLI
gh run download <run-id>
```

### View Logs

```bash
# Via GitHub CLI
gh run view <run-id> --log
```

---

## Summary

The Alexander CI/CD pipeline provides comprehensive automated testing and validation:

- **229 automated tests** executed on every change
- **7 validation jobs** covering testing, quality, docs, security
- **Performance monitoring** with regression detection
- **30-day artifact retention** for test reports
- **Daily scheduled runs** for continuous validation

**Next Steps:**

1. ✓ Pipeline is configured and ready to use
2. Push code to trigger first run
3. Monitor results in Actions tab
4. Review and address any issues
5. Configure branch protection rules (recommended)

For additional help, refer to:
- `TEST_VALIDATION_DOCUMENTATION.md` - Test framework details
- `PROJECT_TEST_STATUS.md` - Current test status
- GitHub Actions documentation - Platform details
