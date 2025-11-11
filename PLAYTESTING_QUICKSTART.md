# Autonomous Playtesting System - Quick Start Guide

**Version**: 1.0.0  
**Last Updated**: 2025-11-11

## Table of Contents
1. [Prerequisites Checklist](#prerequisites-checklist)
2. [5-Minute Setup](#5-minute-setup)
3. [Essential Commands](#essential-commands)
4. [Key Configuration Options](#key-configuration-options)
5. [Common Workflows](#common-workflows)
6. [Troubleshooting Quick Fixes](#troubleshooting-quick-fixes)

## Prerequisites Checklist

Before starting, ensure you have:

- [ ] **Windows 10/11** (64-bit)
- [ ] **Python 3.8+** installed and in PATH
- [ ] **Unreal Engine 5.6+** installed
- [ ] **Tesseract OCR** installed
- [ ] **Alexander.uproject** exists
- [ ] **AutomationGameMode** available in project

**Quick Verify**:
```bash
python --version  # Should show 3.8+
tesseract --version  # Should show tesseract version
dir Alexander.uproject  # Should show project file
```

## 5-Minute Setup

### Step 1: Install Python Dependencies (2 minutes)

```bash
pip install flask==2.3.2 flask-cors==4.0.0 requests==2.31.0
pip install psutil==5.9.5 opencv-python==4.8.0.76
pip install pillow==10.0.0 mss==9.0.1 numpy==1.24.3
```

### Step 2: Create Configuration File (1 minute)

Create `playtesting_config.json` in project root:

```json
{
  "desktop_launcher": {
    "default_resolution": "1280x720",
    "default_quality": "medium"
  },
  "screenshot_capture": {
    "capture_interval": 3.0,
    "jpeg_quality": 85
  },
  "test_scenarios": {
    "default_timeout": 300
  },
  "performance_monitoring": {
    "fps_warning_threshold": 30
  }
}
```

### Step 3: Create Required Directories (1 minute)

```bash
mkdir automation_config\logs
mkdir playtesting_data
mkdir screenshots_visual_playtest
```

### Step 4: Test Game Launch (1 minute)

```bash
python launch_game_desktop.py --resolution 1280x720 --quality medium --timeout 60
```

**Success indicators**:
- Game window opens
- "Game ready!" message appears
- Can access http://localhost:8080/status

## Essential Commands

### Starting the System

**Basic start**:
```bash
python continuous_playtesting.py
```

**With iteration limit**:
```bash
python continuous_playtesting.py --iterations 10
```

**Manual mode** (game only, no automation):
```bash
python continuous_playtesting.py --manual
```

**Custom configuration**:
```bash
python continuous_playtesting.py --config my_config.json
```

### Monitoring Commands

**Check system status**:
```bash
# Game API
curl http://localhost:8080/status

# Analysis API
curl http://localhost:8081/health

# Dashboard
start http://localhost:8082
```

**View logs in real-time**:
```bash
type automation_config\logs\playtesting_*.log /t
```

**Check recent screenshots**:
```bash
dir screenshots_visual_playtest\*.jpg /o-d
```

### Stopping the System

**Graceful shutdown**:
- Press `Ctrl+C` in the terminal
- Wait for cleanup to complete

**Force stop**:
```bash
taskkill /F /IM python.exe
taskkill /F /IM UnrealEditor.exe
```

## Key Configuration Options

### Most Important Settings

**Game Settings** (`playtesting_config.json`):
```json
{
  "desktop_launcher": {
    "default_resolution": "1280x720",  // Game window size
    "default_quality": "medium",         // Graphics quality
    "max_restarts": 3,                   // Auto-restart on crash
    "timeout_seconds": 0                 // 0 = no timeout
  }
}
```

**Screenshot Settings**:
```json
{
  "screenshot_capture": {
    "capture_interval": 3.0,   // Seconds between screenshots
    "jpeg_quality": 85,        // Image quality (1-100)
    "max_screenshots": 1000    // Keep only latest N
  }
}
```

**Performance Settings**:
```json
{
  "performance_monitoring": {
    "fps_warning_threshold": 30,           // Alert below this FPS
    "memory_warning_threshold_mb": 4096    // Alert above this memory
  }
}
```

**Data Retention**:
```json
{
  "data_management": {
    "data_retention_days": 30   // Keep data for N days
  }
}
```

### Quick Configuration Examples

**Fast Testing** (quick feedback):
```json
{
  "screenshot_capture": {
    "capture_interval": 5.0,
    "jpeg_quality": 75
  },
  "test_scenarios": {
    "default_timeout": 120
  }
}
```

**High Quality Testing** (detailed analysis):
```json
{
  "desktop_launcher": {
    "default_resolution": "1920x1080",
    "default_quality": "high"
  },
  "screenshot_capture": {
    "capture_interval": 2.0,
    "jpeg_quality": 95
  }
}
```

**Overnight Testing** (long duration):
```json
{
  "desktop_launcher": {
    "timeout_seconds": 28800,
    "max_restarts": 10
  },
  "screenshot_capture": {
    "capture_interval": 10.0
  },
  "data_management": {
    "data_retention_days": 7
  }
}
```

## Common Workflows

### Workflow 1: Quick Smoke Test (5 minutes)

**Goal**: Verify system works, catch obvious issues

```bash
# 1. Start system with limited iterations
python continuous_playtesting.py --iterations 5

# 2. Monitor dashboard
start http://localhost:8082

# 3. Check results
# Look for:
# - Success rate > 90%
# - No critical issues
# - FPS stable
```

**Expected Results**:
- 5 test iterations complete
- Dashboard shows green status
- Screenshots captured successfully

### Workflow 2: Performance Baseline (30 minutes)

**Goal**: Establish performance baseline

```bash
# 1. Start performance monitoring
python continuous_playtesting.py --iterations 60

# 2. Let it run for 30 minutes

# 3. Check performance report
type playtesting_data\sessions\latest\performance_report.json

# 4. Review key metrics:
# - Average FPS (target: 90+ for VR)
# - Memory usage (target: < 4GB)
# - Frame time stability
```

### Workflow 3: Overnight Stress Test (8+ hours)

**Goal**: Find memory leaks, stability issues

```bash
# 1. Configure for long run
# Edit playtesting_config.json:
# "timeout_seconds": 28800 (8 hours)
# "max_restarts": 10

# 2. Start system
python continuous_playtesting.py

# 3. Leave running overnight

# 4. Next morning, check:
# - Session summary
# - Issue reports
# - Performance trends
# - Memory growth
```

### Workflow 4: CI/CD Integration

**Goal**: Automated testing on every commit

```bash
# In CI/CD pipeline:
# 1. Install dependencies
pip install -r requirements.txt

# 2. Start services
python analysis_server.py &
python playtesting_dashboard.py &

# 3. Run tests
python continuous_playtesting.py --iterations 50 --config playtesting_config_ci.json

# 4. Generate reports
python generate_dashboard.py --input playtesting_data/sessions/latest --output test_dashboard.html

# 5. Upload artifacts
# [Upload playtesting_data/, test_dashboard.html]
```

### Workflow 5: Manual Testing with Automation

**Goal**: Human testing with automated data collection

```bash
# 1. Start in manual mode
python continuous_playtesting.py --manual

# 2. Play the game manually

# 3. System automatically:
# - Captures screenshots
# - Collects performance data
# - Detects issues

# 4. Review results in dashboard
start http://localhost:8082
```

## Troubleshooting Quick Fixes

### Problem: Game won't launch

**Quick fix**:
```bash
# Test manual launch first
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" Alexander.uproject VRTemplateMap -game

# If that works, check config paths
# If that fails, fix Unreal/game issues first
```

### Problem: Screenshots not capturing

**Quick fix**:
```bash
# 1. Test MSS directly
python -c "import mss; sct=mss.mss(); sct.grab(sct.monitors[1])"

# 2. Check capture interval
# Should be at least 1.0 seconds

# 3. Verify game window is visible
# Not minimized, not overlapped
```

### Problem: Low FPS warnings

**Quick fix**:
```bash
# 1. Reduce game quality
python launch_game_desktop.py --quality low

# 2. Or increase threshold
# Edit playtesting_config.json:
# "fps_warning_threshold": 25
```

### Problem: Disk space full

**Quick fix**:
```bash
# 1. Clean up old screenshots
del screenshots_visual_playtest\screenshot_*.jpg

# 2. Run cleanup
python data_manager.py --action cleanup

# 3. Reduce retention
# Edit playtesting_config.json:
# "data_retention_days": 7
```

### Problem: Ports already in use

**Quick fix**:
```bash
# Find what's using the port
netstat -ano | findstr "8080"

# Kill the process
taskkill /PID <PID> /F

# Or use different port
# Edit playtesting_config.json:
# "listen_port": 8083
```

### Problem: Analysis server errors

**Quick fix**:
```bash
# Restart analysis server
taskkill /F /IM python.exe /FI "WINDOWTITLE eq *analysis_server*"
python analysis_server.py

# Check if modules are installed
python -c "import cv2; import numpy; import PIL"
```

### Problem: Too many false positives

**Quick fix**:
```bash
# Adjust issue detection sensitivity
# Edit playtesting_config.json:

# For performance:
"fps_warning_threshold": 25  # Lower = fewer warnings

# For visual analysis:
"confidence_threshold": 0.8  # Higher = fewer detections

# For issue classification:
"severity_threshold": "high"  # Only report high severity
```

## Quick Reference

### File Locations
- **Config**: `playtesting_config.json`
- **Logs**: `automation_config/logs/`
- **Screenshots**: `screenshots_visual_playtest/`
- **Data**: `playtesting_data/`
- **Dashboard**: http://localhost:8082

### Default Ports
- **8080**: Game HTTP API
- **8081**: Analysis API
- **8082**: Dashboard

### Key Metrics
- **Good FPS**: 90+ (VR), 60+ (Desktop)
- **Good Memory**: < 4GB
- **Good Pass Rate**: > 95%
- **Max Restarts**: 3 (default)

### Common Commands
```bash
# Start testing
python continuous_playtesting.py

# Start with limits
python continuous_playtesting.py --iterations 10

# Manual mode
python continuous_playtesting.py --manual

# Check status
curl http://localhost:8080/status

# View dashboard
start http://localhost:8082

# Stop everything
taskkill /F /IM python.exe && taskkill /F /IM UnrealEditor.exe
```

### Emergency Procedures

**System frozen**:
```bash
taskkill /F /IM python.exe
taskkill /F /IM UnrealEditor.exe
del automation_config\game_process.json
```

**Disk full**:
```bash
python data_manager.py --action cleanup
# Or manually:
del screenshots_visual_playtest\screenshot_*.jpg
```

**Corrupted data**:
```bash
rmdir /s /q playtesting_data\sessions\*
rmdir /s /q screenshots_visual_playtest\*
```

---

**Next Steps**: For detailed information, see:
- [PLAYTESTING_SETUP_GUIDE.md](PLAYTESTING_SETUP_GUIDE.md) - Full setup instructions
- [PLAYTESTING_USER_MANUAL.md](PLAYTESTING_USER_MANUAL.md) - Comprehensive usage guide
- [PLAYTESTING_TROUBLESHOOTING.md](PLAYTESTING_TROUBLESHOOTING.md) - Detailed troubleshooting