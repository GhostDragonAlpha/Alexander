# Autonomous Playtesting System - Troubleshooting Guide

**Version**: 1.0.0  
**Last Updated**: 2025-11-11

## Table of Contents
1. [Quick Troubleshooting Steps](#quick-troubleshooting-steps)
2. [Common Issues and Solutions](#common-issues-and-solutions)
3. [Performance Problems](#performance-problems)
4. [Log File Interpretation](#log-file-interpretation)
5. [Debugging Techniques](#debugging-techniques)
6. [System Recovery Procedures](#system-recovery-procedures)
7. [When to Restart vs Reset](#when-to-restart-vs-reset)
8. [Getting Help](#getting-help)

## Quick Troubleshooting Steps

### The "5-Minute Fix" Checklist

If the system isn't working, run through these steps first:

1. **Check if ports are available**:
   ```bash
   netstat -ano | findstr "8080"
   netstat -ano | findstr "8081"
   netstat -ano | findstr "8082"
   ```

2. **Verify Python environment**:
   ```bash
   python --version
   pip list | findstr "flask requests opencv"
   ```

3. **Test game launch manually**:
   ```bash
   python launch_game_desktop.py --resolution 1280x720 --quality low --timeout 60
   ```

4. **Check configuration files exist**:
   ```bash
   dir playtesting_config.json
   dir automation_config\screenshot_config.json
   ```

5. **Review recent logs**:
   ```bash
   type automation_config\logs\playtesting_*.log | more
   ```

### System Status Check

Run this diagnostic script to check system health:

```python
#!/usr/bin/env python3
"""
Quick system diagnostic script
"""
import sys
import subprocess
import requests
import os
from pathlib import Path

def check_port(port, name):
    """Check if port is in use"""
    try:
        response = requests.get(f"http://localhost:{port}/status", timeout=2)
        print(f"✓ Port {port} ({name}): Responding")
        return True
    except:
        print(f"✗ Port {port} ({name}): Not responding")
        return False

def check_file(path, description):
    """Check if file exists"""
    if Path(path).exists():
        print(f"✓ {description}: Found")
        return True
    else:
        print(f"✗ {description}: Missing")
        return False

def check_python_module(module, description):
    """Check if Python module is installed"""
    try:
        __import__(module)
        print(f"✓ {description}: Installed")
        return True
    except ImportError:
        print(f"✗ {description}: Not installed")
        return False

def main():
    print("=" * 60)
    print("PLAYTESTING SYSTEM DIAGNOSTIC")
    print("=" * 60)
    
    all_good = True
    
    # Check ports
    print("\n--- Port Checks ---")
    all_good &= check_port(8080, "Game API")
    all_good &= check_port(8081, "Analysis API")
    all_good &= check_port(8082, "Dashboard")
    
    # Check configuration files
    print("\n--- Configuration Files ---")
    all_good &= check_file("playtesting_config.json", "Main config")
    all_good &= check_file("automation_config/screenshot_config.json", "Screenshot config")
    all_good &= check_file("visual_analysis_config.json", "Analysis config")
    
    # Check Python modules
    print("\n--- Python Modules ---")
    all_good &= check_python_module("flask", "Flask")
    all_good &= check_python_module("requests", "Requests")
    all_good &= check_python_module("cv2", "OpenCV")
    all_good &= check_python_module("PIL", "Pillow")
    all_good &= check_python_module("mss", "MSS")
    all_good &= check_python_module("numpy", "NumPy")
    all_good &= check_python_module("psutil", "psutil")
    
    # Check directories
    print("\n--- Directories ---")
    all_good &= check_file("automation_config", "Config directory")
    all_good &= check_file("playtesting_data", "Data directory")
    all_good &= check_file("screenshots_visual_playtest", "Screenshot directory")
    
    # Check Unreal Engine
    print("\n--- Unreal Engine ---")
    unreal_path = Path("C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor.exe")
    if unreal_path.exists():
        print("✓ Unreal Engine: Found")
    else:
        print("✗ Unreal Engine: Not found at expected location")
        all_good = False
    
    # Check project
    if Path("Alexander.uproject").exists():
        print("✓ Project file: Found")
    else:
        print("✗ Project file: Missing")
        all_good = False
    
    print("\n" + "=" * 60)
    if all_good:
        print("✓ SYSTEM STATUS: HEALTHY")
        print("All checks passed. System should work correctly.")
    else:
        print("✗ SYSTEM STATUS: ISSUES DETECTED")
        print("Check the failures above and follow the troubleshooting guide.")
    print("=" * 60)

if __name__ == "__main__":
    main()
```

## Common Issues and Solutions

### Issue 1: "ModuleNotFoundError: No module named 'flask'"

**Symptoms**:
```
Traceback (most recent call last):
  File "continuous_playtesting.py", line 22, in <module>
    from flask import Flask
ModuleNotFoundError: No module named 'flask'
```

**Solutions**:

1. **Install missing module**:
   ```bash
   pip install flask==2.3.2
   ```

2. **Install all dependencies**:
   ```bash
   pip install -r requirements.txt
   ```

3. **Check virtual environment**:
   ```bash
   # If using venv
   .\venv\Scripts\activate
   pip list
   ```

4. **Verify Python path**:
   ```bash
   python -c "import sys; print(sys.path)"
   ```

**Prevention**:
- Always use virtual environments
- Keep requirements.txt updated
- Document all dependencies

---

### Issue 2: "Cannot connect to AutomationAPIServer (port 8080)"

**Symptoms**:
```
[FAIL] Cannot connect to AutomationAPIServer (port 8080)
[INFO] Make sure:
  1. Standalone game is running (-game flag)
  2. Automation GameMode is set as default
  3. AutomationAPIServer started on port 8080
```

**Solutions**:

1. **Check if game is running**:
   ```bash
   tasklist | findstr "UnrealEditor"
   ```

2. **Verify HTTP API is enabled**:
   - Open `Config/DefaultEngine.ini`
   - Add:
     ```ini
     [/Script/Alexander.AutomationAPIServer]
     ListenPort=8080
     bServerEnabled=True
     ```

3. **Check game logs**:
   ```bash
   type automation_config/logs/launcher.log
   ```

4. **Test API manually**:
   ```bash
   curl http://localhost:8080/status
   # Or in browser: http://localhost:8080/status
   ```

5. **Check firewall**:
   ```bash
   # Windows Firewall
   netsh advfirewall firewall add rule name="Unreal HTTP API" dir=in action=allow protocol=TCP localport=8080
   ```

**Common Causes**:
- Game crashed during startup
- Wrong GameMode set
- HTTP API component not enabled
- Port blocked by firewall

---

### Issue 3: "Game failed to start properly"

**Symptoms**:
```
[ERROR] Game failed to start properly
[ERROR] Failed to launch game: [Error details]
```

**Solutions**:

1. **Check Unreal Engine path**:
   ```json
   {
     "integration": {
       "unreal_editor_path": "C:\\Program Files\\Epic Games\\UE_5.6\\Engine\\Binaries\\Win64\\UnrealEditor.exe"
     }
   }
   ```

2. **Verify project file**:
   ```bash
   dir Alexander.uproject
   ```

3. **Check map exists**:
   ```bash
   dir Content\VRTemplate\Maps\VRTemplateMap.umap
   ```

4. **Test manual launch**:
   ```bash
   "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" Alexander.uproject VRTemplateMap -game
   ```

5. **Check for compilation errors**:
   - Open project in Unreal Editor
   - Check Output Log for errors
   - Ensure AutomationGameMode compiles

**Log File Locations**:
- Launcher logs: `automation_config/logs/launcher.log`
- Game logs: `Saved/Logs/Alexander.log`
- Crash logs: `Saved/Crashes/`

---

### Issue 4: "Screenshot capture failed"

**Symptoms**:
```
[ERROR] Error capturing screen: [Error details]
[WARNING] Failed to capture screenshot
```

**Solutions**:

1. **Check screen resolution**:
   ```bash
   # Verify resolution in config
   "desktop_launcher": {
     "default_resolution": "1280x720"
   }
   ```

2. **Test MSS capture**:
   ```python
   import mss
   with mss.mss() as sct:
       screenshot = sct.grab(sct.monitors[1])
       print(f"Captured: {screenshot.width}x{screenshot.height}")
   ```

3. **Check display settings**:
   - Ensure display scaling is 100%
   - Verify primary monitor is available
   - Check for virtual displays

4. **Run as administrator**:
   - Right-click on terminal/command prompt
   - Select "Run as administrator"
   - Then run the playtesting script

5. **Check capture region**:
   ```json
   {
     "screenshot_capture": {
       "capture_region": null  # Use null for full screen
     }
   }
   ```

**Common Causes**:
- Permission issues
- Wrong display/monitor
- Resolution mismatch
- MSS library issues

---

### Issue 5: "Analysis server not responding"

**Symptoms**:
```
[ERROR] Analysis server returned status 500
[ERROR] Screenshot analysis failed: [Error details]
```

**Solutions**:

1. **Check if analysis server is running**:
   ```bash
   netstat -ano | findstr "8081"
   ```

2. **Test analysis API**:
   ```bash
   curl http://localhost:8081/health
   ```

3. **Check analysis server logs**:
   ```bash
   type automation_config/logs/analysis_server.log
   ```

4. **Verify analysis modules**:
   ```bash
   curl http://localhost:8081/config
   ```

5. **Restart analysis server**:
   ```bash
   # Kill existing process
   taskkill /F /IM python.exe /FI "WINDOWTITLE eq *analysis_server*"
   
   # Restart
   python analysis_server.py
   ```

**Common Causes**:
- Missing analysis modules
- Corrupted model files
- Memory issues
- Port conflicts

---

### Issue 6: "Low FPS detected" warnings

**Symptoms**:
```
[WARNING] Low FPS detected: 25.3
[WARNING] Performance regression: FPS dropped from 90 to 25
```

**Solutions**:

1. **Check game quality settings**:
   ```json
   {
     "desktop_launcher": {
       "default_quality": "low"  # or "medium"
     }
   }
   ```

2. **Reduce resolution**:
   ```json
   {
     "desktop_launcher": {
       "default_resolution": "1024x768"
     }
   }
   ```

3. **Close other applications**:
   - Close browsers, editors, etc.
   - Check Task Manager for resource usage

4. **Update graphics drivers**:
   - Update NVIDIA/AMD drivers
   - Ensure GPU is being used (not integrated graphics)

5. **Check for performance leaks**:
   ```bash
   # Monitor memory usage
   python -c "import psutil; print(f'Memory: {psutil.virtual_memory().percent}%')"
   ```

**Investigation Steps**:
1. Check which system is bottleneck:
   - CPU usage > 90%: CPU bottleneck
   - GPU usage < 50%: CPU bottleneck
   - GPU usage > 90%: GPU bottleneck

2. Use Unreal profiling:
   ```bash
   # In game console
   stat unit
   stat fps
   stat game
   ```

---

### Issue 7: "Disk full" errors

**Symptoms**:
```
[ERROR] No space left on device
[WARNING] Failed to save screenshot: [Errno 28] No space left on device
```

**Solutions**:

1. **Check disk space**:
   ```bash
   wmic logicaldisk get size,freespace,caption
   ```

2. **Clean up old screenshots**:
   ```bash
   # Manual cleanup
   del screenshots_visual_playtest\screenshot_*.jpg
   
   # Or use data manager
   python data_manager.py --action cleanup
   ```

3. **Reduce retention period**:
   ```json
   {
     "data_management": {
       "data_retention_days": 7
     }
   }
   ```

4. **Increase capture interval**:
   ```json
   {
     "screenshot_capture": {
       "capture_interval": 10.0
     }
   }
   ```

5. **Reduce screenshot quality**:
   ```json
   {
     "screenshot_capture": {
       "jpeg_quality": 70
     }
   }
   ```

**Storage Planning**:
- 3s interval, 85% quality: ~20 MB/hour
- 5s interval, 80% quality: ~12 MB/hour
- 10s interval, 70% quality: ~5 MB/hour

---

### Issue 8: "GitHub ticket creation failed"

**Symptoms**:
```
[ERROR] Error creating GitHub ticket: [Error details]
[WARNING] Failed to create GitHub issue
```

**Solutions**:

1. **Check GitHub token**:
   ```bash
   # Set environment variable
   setx GITHUB_TOKEN "your_token_here"
   
   # Or in config:
   {
     "integration": {
       "github_token": "your_token_here"
     }
   }
   ```

2. **Verify repository access**:
   - Token must have `repo` scope
   - Repository must exist
   - User must have write access

3. **Test GitHub API**:
   ```bash
   curl -H "Authorization: token YOUR_TOKEN" https://api.github.com/user
   ```

4. **Check network connectivity**:
   ```bash
   ping api.github.com
   ```

5. **Review GitHub API limits**:
   - Free tier: 60 requests/hour (unauthenticated)
   - Token: 5000 requests/hour

**Configuration Example**:
```json
{
  "integration": {
    "github_token": "${GITHUB_TOKEN}",
    "github_repo": "yourusername/alexander",
    "create_github_tickets": true
  }
}
```

---

### Issue 9: "Session timeout reached"

**Symptoms**:
```
[INFO] Session timeout reached (60 minutes)
[INFO] Stopping playtesting system
```

**Solutions**:

1. **Increase timeout**:
   ```json
   {
     "continuous_playtesting": {
       "session_timeout_minutes": 240
     }
   }
   ```

2. **Disable timeout (0 = unlimited)**:
   ```json
   {
     "continuous_playtesting": {
       "session_timeout_minutes": 0
     }
   }
   ```

3. **Check if timeout is appropriate**:
   - Overnight testing: 480-720 minutes (8-12 hours)
   - CI/CD testing: 30-60 minutes
   - Development testing: 0-30 minutes

4. **Monitor actual test duration**:
   ```bash
   type playtesting_data/sessions/latest/session_summary.json
   ```

---

### Issue 10: "Too many consecutive failures"

**Symptoms**:
```
[ERROR] Too many consecutive failures (3), stopping
[ERROR] Test iteration failed
```

**Solutions**:

1. **Increase failure tolerance**:
   ```json
   {
     "continuous_playtesting": {
       "max_consecutive_failures": 5
     }
   }
   ```

2. **Investigate root cause**:
   ```bash
   # Check recent test results
   type playtesting_data/sessions/latest/test_results.json
   
   # Check issues
   type playtesting_data/sessions/latest/issue_report.json
   ```

3. **Check game stability**:
   - Can you launch game manually?
   - Does game crash without playtesting?
   - Check for recent code changes

4. **Review failure patterns**:
   - Same test failing repeatedly?
   - Random failures?
   - Performance-related failures?

**Common Causes**:
- Game crashes
- HTTP API instability
- Performance degradation
- Memory leaks

---

## Performance Problems

### High CPU Usage

**Symptoms**:
- System becomes unresponsive
- High CPU usage in Task Manager
- Screenshot capture delays

**Solutions**:

1. **Reduce capture frequency**:
   ```json
   {
     "screenshot_capture": {
       "capture_interval": 10.0
     }
   }
   ```

2. **Disable intensive analysis**:
   ```json
   {
     "analysis_options": {
       "object_detection": false,
       "text_recognition": false
     }
   }
   ```

3. **Lower game quality**:
   ```json
   {
     "desktop_launcher": {
       "default_quality": "low"
     }
   }
   ```

4. **Close other applications**:
   - Close browsers, IDEs, etc.
   - Check startup programs

5. **Upgrade hardware**:
   - More CPU cores
   - Faster CPU
   - More RAM

---

### High Memory Usage

**Symptoms**:
- Memory usage > 90%
- System swapping to disk
- Out of memory errors

**Solutions**:

1. **Reduce screenshot retention**:
   ```json
   {
     "screenshot_capture": {
       "max_screenshots": 100
     }
   }
   ```

2. **Increase cleanup frequency**:
   ```json
   {
     "data_management": {
       "cleanup_interval_hours": 6
     }
   }
   ```

3. **Reduce image quality**:
   ```json
   {
     "screenshot_capture": {
       "jpeg_quality": 70
     }
   }
   ```

4. **Disable memory-intensive features**:
   ```json
   {
     "analysis_options": {
       "object_detection": false
     }
   }
   ```

5. **Add more RAM**:
   - 16GB minimum
   - 32GB recommended
   - 64GB for intensive testing

---

### Slow Screenshot Capture

**Symptoms**:
- Capture interval > configured value
- Missing screenshots
- High latency

**Solutions**:

1. **Reduce resolution**:
   ```json
   {
     "desktop_launcher": {
       "default_resolution": "1024x768"
     }
   }
   ```

2. **Use region capture**:
   ```json
   {
     "screenshot_capture": {
       "capture_region": [0, 0, 800, 600]
     }
   }
   ```

3. **Disable compression**:
   ```json
   {
     "screenshot_capture": {
       "enable_compression": false
     }
   }
   ```

4. **Use faster storage**:
   - SSD instead of HDD
   - NVMe SSD for best performance

5. **Optimize MSS settings**:
   ```python
   # In capture_screenshot.py
   self.sct = mss.mss()
   self.sct.compression_level = 1  # Lower = faster
   ```

---

## Log File Interpretation

### Log File Locations

- **Main logs**: `automation_config/logs/playtesting_*.log`
- **Launcher logs**: `automation_config/logs/launcher.log`
- **Screenshot logs**: `automation_config/logs/screenshot_capture.log`
- **Issue logs**: `automation_config/logs/issue_reporter.log`
- **Data manager logs**: `automation_config/logs/data_manager.log`
- **Analysis logs**: `automation_config/logs/analysis_server.log`

### Log Format

```
2024-01-15 14:30:25 - INFO - Game launched successfully (PID: 12345)
2024-01-15 14:30:45 - WARNING - High memory usage detected: 4500 MB
2024-01-15 14:31:00 - ERROR - Screenshot capture failed: [Errno 28] No space left on device
```

**Format**: `YYYY-MM-DD HH:MM:SS - LEVEL - Message`

### Common Log Patterns

**Normal Operation**:
```
[INFO] Game launched successfully (PID: 12345)
[INFO] HTTP API ready! (took 15s)
[INFO] Screenshot capture started (interval: 3.0s)
[INFO] Performance collector started
[INFO] Starting iteration 1
[INFO] Screenshot captured and processed successfully
[INFO] Test completed: first_mission - passed
```

**Warnings** (non-critical issues):
```
[WARNING] Low FPS detected: 28.5
[WARNING] High memory usage detected: 4200 MB
[WARNING] Screenshot capture took longer than expected: 5.2s
```

**Errors** (critical issues):
```
[ERROR] Game failed to start properly
[ERROR] Screenshot capture failed: [Errno 28] No space left on device
[ERROR] HTTP API not responding after 180s
```

### Log Analysis Commands

**Find errors**:
```bash
findstr "ERROR" automation_config/logs/playtesting_*.log
```

**Find warnings**:
```bash
findstr "WARNING" automation_config/logs/playtesting_*.log
```

**Track specific issue**:
```bash
findstr "screenshot" automation_config/logs/playtesting_*.log
```

**Monitor in real-time**:
```bash
type automation_config/logs/playtesting_*.log /t
```

**Extract timestamps**:
```bash
# Find when errors occurred
findstr "ERROR" automation_config/logs/playtesting_*.log | findstr "2024-01-15"
```

---

## Debugging Techniques

### Enable Debug Logging

1. **Update configuration**:
   ```json
   {
     "logging": {
       "log_level": "DEBUG"
     }
   }
   ```

2. **Run with verbose output**:
   ```bash
   python continuous_playtesting.py --verbose
   ```

3. **Enable component-specific debugging**:
   ```python
   # In individual modules
   import logging
   logging.basicConfig(level=logging.DEBUG)
   ```

### Debug Specific Components

**Test Game Launcher**:
```bash
python launch_game_desktop.py --resolution 800x600 --quality low --timeout 60 --verbose
```

**Test Screenshot Capture**:
```bash
python capture_screenshot.py --interval 1 --duration 10 --verbose
```

**Test Analysis Server**:
```bash
python analysis_server.py --debug
```

**Test Issue Reporter**:
```bash
python issue_reporter.py --action detect --verbose
```

### Use Python Debugger

```bash
# Start with debugger
python -m pdb continuous_playtesting.py

# Inside debugger:
# (Pdb) break continuous_playtesting.py:100  # Set breakpoint
# (Pdb) continue  # Run to breakpoint
# (Pdb) print(self.config)  # Inspect variables
# (Pdb) step  # Step through code
```

### Add Debug Prints

```python
# Add to problematic areas
print(f"DEBUG: Current value: {variable}")
print(f"DEBUG: Config: {self.config}")
print(f"DEBUG: Game state: {game_state}")
```

### Network Debugging

**Test API connectivity**:
```bash
# Test game API
curl -v http://localhost:8080/status

# Test analysis API
curl -v http://localhost:8081/health

# Test dashboard API
curl -v http://localhost:8082/api/status
```

**Monitor network traffic**:
```bash
# Use Wireshark or similar tool
# Filter: tcp.port == 8080 or tcp.port == 8081 or tcp.port == 8082
```

---

## System Recovery Procedures

### Soft Reset (Keep Data)

1. **Stop all components**:
   ```bash
   # Press Ctrl+C in each terminal
   # Or kill processes
   taskkill /F /IM python.exe
   taskkill /F /IM UnrealEditor.exe
   ```

2. **Clear temporary files**:
   ```bash
   del temp_screenshots\*.png
   del automation_config\game_process.json
   ```

3. **Restart system**:
   ```bash
   python continuous_playtesting.py
   ```

### Medium Reset (Clear Recent Data)

1. **Stop all components**:
   ```bash
   taskkill /F /IM python.exe
   taskkill /F /IM UnrealEditor.exe
   ```

2. **Clear recent data**:
   ```bash
   # Keep configuration, clear data
   rmdir /s /q playtesting_data\sessions\*
   rmdir /s /q screenshots_visual_playtest\*
   del playtesting_data\issues.db
   ```

3. **Reset configuration**:
   ```bash
   # Restore default config
   copy playtesting_config.json.backup playtesting_config.json
   ```

4. **Restart system**:
   ```bash
   python continuous_playtesting.py
   ```

### Hard Reset (Complete Reset)

1. **Stop all components**:
   ```bash
   taskkill /F /IM python.exe
   taskkill /F /IM UnrealEditor.exe
   ```

2. **Clear all data**:
   ```bash
   rmdir /s /q playtesting_data
   rmdir /s /q screenshots_visual_playtest
   rmdir /s /q automation_config\logs
   del playtesting_data\issues.db
   ```

3. **Reset configuration**:
   ```bash
   del playtesting_config.json
   del automation_config\screenshot_config.json
   del visual_analysis_config.json
   ```

4. **Recreate directories**:
   ```bash
   mkdir playtesting_data
   mkdir playtesting_data\sessions
   mkdir playtesting_data\performance
   mkdir playtesting_data\issues
   mkdir playtesting_data\archive
   mkdir playtesting_data\backups
   mkdir playtesting_data\exports
   mkdir screenshots_visual_playtest
   mkdir automation_config\logs
   ```

5. **Restore default configs**:
   ```bash
   # Copy from backup or recreate
   copy docs\default_playtesting_config.json playtesting_config.json
   ```

6. **Restart system**:
   ```bash
   python continuous_playtesting.py
   ```

---

## When to Restart vs Reset

### Restart (Soft Reset)
**When to use**:
- Temporary glitches
- Memory leaks
- Performance degradation
- HTTP API hiccups

**Procedure**:
1. Stop system (Ctrl+C)
2. Wait 10 seconds
3. Start system again

**Keeps**: All data, configuration, logs

### Reset Configuration
**When to use**:
- Configuration problems
- Settings corruption
- Want to test different config

**Procedure**:
1. Backup current config
2. Restore default or previous config
3. Restart system

**Keeps**: All data, logs
**Resets**: Configuration only

### Reset Data (Medium Reset)
**When to use**:
- Corrupted data
- Database issues
- Storage full
- Want to start fresh test series

**Procedure**:
1. Stop system
2. Clear data directories
3. Keep configuration
4. Restart system

**Keeps**: Configuration
**Resets**: All test data, screenshots, issues

### Full Reset (Hard Reset)
**When to use**:
- Major corruption
- Fresh installation
- Unrecoverable errors
- Starting from scratch

**Procedure**:
1. Stop system
2. Clear everything
3. Reinstall if needed
4. Reconfigure
5. Restart system

**Resets**: Everything (data, config, logs)

---

## Getting Help

### Before Asking for Help

1. **Check documentation**:
   - Review relevant markdown files
   - Check API reference
   - Review configuration guide

2. **Search logs**:
   ```bash
   findstr "ERROR" automation_config/logs/*.log
   ```

3. **Run diagnostics**:
   ```bash
   python diagnostic_script.py
   ```

4. **Check recent changes**:
   - What changed since it last worked?
   - New code? New config? New dependencies?

### Information to Provide

When asking for help, include:

1. **System information**:
   - Windows version
   - Python version
   - Unreal Engine version
   - Hardware specs

2. **Error details**:
   - Full error message
   - Log excerpts
   - Screenshots of errors

3. **Configuration**:
   - Relevant config sections
   - Any custom modifications
   - Environment variables

4. **Steps to reproduce**:
   - What command did you run?
   - What happened before the error?
   - Can you reproduce it consistently?

### Support Channels

1. **GitHub Issues**: Bug reports, feature requests
2. **Documentation**: Check existing docs first
3. **Community Forums**: User discussions
4. **Discord/Slack**: Real-time chat

### Creating a Bug Report

Use this template:

```markdown
**System Information**
- OS: [e.g., Windows 11]
- Python: [e.g., 3.9.7]
- Unreal Engine: [e.g., 5.6.0]
- Playtesting System Version: [e.g., 1.0.0]

**Configuration**
```json
{
  "relevant_config": "here"
}
```

**Error Description**
[Clear description of the problem]

**Steps to Reproduce**
1. Run command: `python continuous_playtesting.py`
2. Wait for 5 minutes
3. Error occurs

**Expected Behavior**
[What should happen]

**Actual Behavior**
[What actually happens]

**Logs**
```
[Relevant log excerpts here]
```

**Screenshots**
[If applicable]

**Additional Context**
[Any other relevant information]
```

---

**Next Steps**: See [PLAYTESTING_QUICKSTART.md](PLAYTESTING_QUICKSTART.md) for a condensed quick start guide.