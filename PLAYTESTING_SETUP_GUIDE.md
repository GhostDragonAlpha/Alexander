# Autonomous Playtesting System - Installation & Setup Guide

**Version**: 1.0.0  
**Last Updated**: 2025-11-11

## Table of Contents
1. [Prerequisites](#prerequisites)
2. [System Requirements](#system-requirements)
3. [Installation Steps](#installation-steps)
4. [Configuration Setup](#configuration-setup)
5. [Environment Variables](#environment-variables)
6. [First-Time Launch](#first-time-launch)
7. [Verification](#verification)
8. [Troubleshooting](#troubleshooting)

## Prerequisites

### Required Software
- **Windows 10/11** (64-bit)
- **Python 3.8+** (64-bit)
- **Unreal Engine 5.6+** (with Python support)
- **Tesseract OCR** (for text recognition)
- **Git** (for version control)

### Required Python Packages
```bash
# Core dependencies
flask==2.3.2
flask-cors==4.0.0
requests==2.31.0
psutil==5.9.5
opencv-python==4.8.0.76
pillow==10.0.0
mss==9.0.1
numpy==1.24.3
sqlite3  # Built-in Python module
```

### Unreal Engine Requirements
- **Engine Version**: Unreal Engine 5.6 or compatible
- **Project**: Alexander.uproject
- **Game Mode**: AutomationGameMode
- **Plugins**: HTTP API server component
- **Build Configuration**: Development or Debug

### Optional but Recommended
- **Visual Studio 2022** (for debugging)
- **GitHub CLI** (for issue automation)
- **Process Monitor** (for troubleshooting)

## System Requirements

### Minimum Requirements
- **CPU**: Intel Core i5 or AMD Ryzen 5 (4 cores)
- **RAM**: 16 GB
- **Storage**: 50 GB free space
- **GPU**: DirectX 11 compatible
- **Display**: 1920x1080 resolution

### Recommended Requirements
- **CPU**: Intel Core i7 or AMD Ryzen 7 (8+ cores)
- **RAM**: 32 GB
- **Storage**: 100 GB SSD free space
- **GPU**: NVIDIA GTX 1060 / RTX 2060 or better
- **Display**: 1920x1080 or higher

### Network Requirements
- Localhost connectivity (127.0.0.1)
- No external network access required
- Firewall must allow local port access (8080-8082)

## Installation Steps

### Step 1: Install Python 3.8+

1. **Download Python**:
   ```bash
   # Visit: https://www.python.org/downloads/
   # Download Python 3.8 or newer (64-bit version)
   ```

2. **Install Python**:
   - Run the installer
   - **IMPORTANT**: Check "Add Python to PATH"
   - Check "Install pip"
   - Check "Install for all users" (optional but recommended)

3. **Verify Installation**:
   ```bash
   python --version
   # Should show: Python 3.8.x or higher
   
   pip --version
   # Should show pip version
   ```

### Step 2: Install Tesseract OCR

1. **Download Tesseract**:
   ```bash
   # Visit: https://github.com/UB-Mannheim/tesseract/wiki
   # Download the latest installer for Windows
   ```

2. **Install Tesseract**:
   - Run the installer
   - Note the installation path (default: C:\Program Files\Tesseract-OCR)
   - Add to system PATH environment variable

3. **Verify Installation**:
   ```bash
   tesseract --version
   # Should show tesseract version
   ```

### Step 3: Install Python Dependencies

1. **Navigate to Project Directory**:
   ```bash
   cd C:\Users\allen\Desktop\Alexander\Alexander
   ```

2. **Create Virtual Environment (Recommended)**:
   ```bash
   python -m venv venv
   .\venv\Scripts\activate
   ```

3. **Install Dependencies**:
   ```bash
   pip install flask==2.3.2 flask-cors==4.0.0 requests==2.31.0
   pip install psutil==5.9.5 opencv-python==4.8.0.76
   pip install pillow==10.0.0 mss==9.0.1 numpy==1.24.3
   ```

4. **Verify Installation**:
   ```bash
   pip list
   # Should show all installed packages
   ```

### Step 4: Verify Unreal Engine Installation

1. **Check Unreal Engine Path**:
   ```bash
   # Default path should be:
   # C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe
   ```

2. **Verify Project File**:
   ```bash
   # Check that Alexander.uproject exists:
   dir Alexander.uproject
   ```

3. **Verify AutomationGameMode**:
   - Open Unreal Editor
   - Check that AutomationGameMode exists in Source/Alexander

### Step 5: Create Required Directories

Run the following commands to create necessary directories:

```bash
# Create automation configuration directory
mkdir automation_config
mkdir automation_config\logs

# Create playtesting data directories
mkdir playtesting_data
mkdir playtesting_data\sessions
mkdir playtesting_data\performance
mkdir playtesting_data\issues
mkdir playtesting_data\archive
mkdir playtesting_data\backups
mkdir playtesting_data\exports

# Create screenshot directory
mkdir screenshots_visual_playtest

# Create temporary directories
mkdir temp_screenshots
```

### Step 6: Configure Unreal Engine Project

1. **Set Default Game Mode**:
   - Open `Config/DefaultGame.ini`
   - Add or verify:
     ```ini
     [/Script/EngineSettings.GameMapsSettings]
     GameDefaultMap=/Game/VRTemplate/Maps/VRTemplateMap
     GlobalDefaultGameMode=/Script/Alexander.AutomationGameMode
     ```

2. **Enable HTTP API**:
   - Open `Config/DefaultEngine.ini`
   - Add:
     ```ini
     [/Script/Alexander.AutomationAPIServer]
     ListenPort=8080
     bServerEnabled=True
     ```

3. **Configure Project for Automation**:
   - Ensure AutomationGameMode is set as default
   - Verify HTTP API server component is enabled
   - Check that required modules are built

## Configuration Setup

### Step 1: Create Main Configuration File

Create `playtesting_config.json` in the project root:

```json
{
  "desktop_launcher": {
    "default_resolution": "1280x720",
    "default_quality": "medium",
    "default_port": 8080,
    "max_restarts": 3,
    "timeout_seconds": 0,
    "enable_console": true,
    "enable_debug": true,
    "process_check_interval": 10,
    "http_api_timeout": 180
  },
  "screenshot_capture": {
    "capture_interval": 3.0,
    "jpeg_quality": 85,
    "max_screenshots": 1000,
    "auto_cleanup": true,
    "enable_compression": true,
    "metadata_embedding": true,
    "enable_ai_transmission": true,
    "server_endpoint": "/receive_screenshot",
    "resize_to": null,
    "capture_region": null
  },
  "test_scenarios": {
    "default_timeout": 300,
    "max_attempts": 3,
    "required_screenshots": 5,
    "scenario_configs": {
      "first_mission": {
        "timeout": 180,
        "success_criteria": {
          "mission_completed": true,
          "min_distance_traveled": 1000.0,
          "max_time": 120
        },
        "failure_criteria": {
          "ship_destroyed": true,
          "timeout": true,
          "stuck_duration": 30
        }
      },
      "trading_system": {
        "timeout": 240,
        "success_criteria": {
          "trade_completed": true,
          "credits_earned": 100,
          "items_traded": 1
        },
        "failure_criteria": {
          "insufficient_credits": true,
          "trade_rejected": true,
          "timeout": true
        }
      },
      "combat_scenario": {
        "timeout": 300,
        "randomize": true,
        "success_criteria": {
          "enemies_defeated": 3,
          "player_survived": true,
          "combat_duration": 60
        },
        "failure_criteria": {
          "player_destroyed": true,
          "ammo_depleted": true,
          "timeout": true
        }
      },
      "free_exploration": {
        "timeout": 600,
        "randomize": true,
        "success_criteria": {
          "min_distance_explored": 5000.0,
          "discoveries_made": 3,
          "waypoints_visited": 5
        },
        "failure_criteria": {
          "ship_destroyed": true,
          "fuel_depleted": true,
          "timeout": true
        }
      }
    }
  },
  "game_state_extractor": {
    "state_update_interval": 0.1,
    "nearby_actor_detection_radius": 5000.0,
    "track_performance": true,
    "extract_player_state": true,
    "extract_world_state": true,
    "extract_performance_metrics": true,
    "extract_inventory_state": true,
    "extract_mission_state": true,
    "extract_nearby_actors": true,
    "extract_ui_state": true
  },
  "performance_monitoring": {
    "track_fps": true,
    "track_memory": true,
    "track_hardware": true,
    "fps_warning_threshold": 30,
    "memory_warning_threshold_mb": 4096,
    "log_performance_interval": 60
  },
  "http_api_server": {
    "listen_port": 8080,
    "max_requests_per_second": 60,
    "enable_verbose_logging": true,
    "enable_rate_limiting": true,
    "cors_enabled": true,
    "allowed_origins": ["*"]
  },
  "logging": {
    "log_level": "INFO",
    "log_to_file": true,
    "log_to_console": true,
    "max_log_file_size_mb": 100,
    "max_log_files": 10,
    "log_directory": "automation_config/logs"
  },
  "integration": {
    "game_mode": "/Script/Alexander.AutomationGameMode",
    "default_map": "VRTemplateMap",
    "unreal_editor_path": "C:\\Program Files\\Epic Games\\UE_5.6\\Engine\\Binaries\\Win64\\UnrealEditor.exe",
    "project_file": "C:\\Users\\allen\\Desktop\\Alexander\\Alexander\\Alexander.uproject"
  },
  "data_management": {
    "data_retention_days": 30,
    "compress_old_data": true,
    "cleanup_interval_hours": 24
  }
}
```

### Step 2: Create Screenshot Configuration

Create `automation_config/screenshot_config.json`:

```json
{
  "capture_interval": 3.0,
  "jpeg_quality": 85,
  "max_screenshots": 1000,
  "auto_cleanup": true,
  "capture_region": null,
  "resize_to": null,
  "enable_compression": true,
  "metadata_embedding": true,
  "server_endpoint": "/screenshot"
}
```

### Step 3: Configure Analysis Settings

Create `visual_analysis_config.json`:

```json
{
  "analysis_server": {
    "host": "localhost",
    "port": 8081,
    "max_file_size": 10485760,
    "timeout": 2.0
  },
  "visual_analysis": {
    "enable_ui_detection": true,
    "enable_artifact_detection": true,
    "confidence_threshold": 0.7
  },
  "text_recognition": {
    "enable_ocr": true,
    "languages": ["eng"],
    "confidence_threshold": 0.6
  },
  "object_detection": {
    "enable_detection": true,
    "model_path": "models/object_detector.tflite",
    "confidence_threshold": 0.5
  },
  "issue_classification": {
    "enable_classification": true,
    "severity_threshold": "medium",
    "min_frequency_for_pattern": 3
  }
}
```

## Environment Variables

### Required Environment Variables

Set these system environment variables:

```batch
# Unreal Engine Path
setx UE_PATH "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64"

# Project Path
setx ALEXANDER_PROJECT "C:\Users\allen\Desktop\Alexander\Alexander"

# Python Path (if using virtual environment)
setx PYTHONPATH "C:\Users\allen\Desktop\Alexander\Alexander\venv\Lib\site-packages"

# Enable Automation Mode
setx UE_AUTOMATION_MODE "1"
```

### Optional Environment Variables

```batch
# Logging Level
setx PLAYTESTING_LOG_LEVEL "INFO"

# Disable GPU for analysis (if needed)
setx CUDA_VISIBLE_DEVICES "-1"

# Custom configuration path
setx PLAYTESTING_CONFIG "C:\Users\allen\Desktop\Alexander\Alexander\playtesting_config.json"
```

### Setting Environment Variables

1. **Open Command Prompt as Administrator**
2. **Run the commands above** (setx commands)
3. **Restart your computer** for changes to take effect
4. **Verify settings**:
   ```bash
   echo %UE_PATH%
   echo %ALEXANDER_PROJECT%
   ```

## First-Time Launch

### Step 1: Test Individual Components

1. **Test Game Launcher**:
   ```bash
   python launch_game_desktop.py --help
   # Should show help information
   ```

2. **Test Screenshot Capture**:
   ```bash
   python capture_screenshot.py --interval 3 --duration 10
   # Should capture screenshots for 10 seconds
   ```

3. **Test Analysis Server**:
   ```bash
   python analysis_server.py
   # Should start on port 8081
   ```

4. **Test Issue Reporter**:
   ```bash
   python issue_reporter.py --action stats
   # Should show issue statistics
   ```

### Step 2: Run Full System Test

1. **Start Analysis Server** (in separate terminal):
   ```bash
   python analysis_server.py
   ```

2. **Start Playtesting Dashboard** (in separate terminal):
   ```bash
   python playtesting_dashboard.py
   ```

3. **Run Continuous Playtesting** (in main terminal):
   ```bash
   python continuous_playtesting.py --config playtesting_config.json --iterations 5
   ```

### Step 3: Monitor the System

1. **Check Dashboard**: Open browser to `http://localhost:8082`
2. **Check Game API**: `http://localhost:8080/status`
3. **Check Analysis API**: `http://localhost:8081/health`
4. **Monitor Logs**: Check `automation_config/logs/`

## Verification

### Component Verification Checklist

- [ ] Python 3.8+ installed and in PATH
- [ ] All Python dependencies installed
- [ ] Tesseract OCR installed and in PATH
- [ ] Unreal Engine 5.6+ installed
- [ ] Alexander.uproject exists and opens
- [ ] AutomationGameMode is available
- [ ] Required directories created
- [ ] Configuration files created
- [ ] Environment variables set
- [ ] Test launch successful
- [ ] Screenshot capture working
- [ ] Analysis server responding
- [ ] Dashboard accessible

### Test Verification Commands

```bash
# Test Python environment
python -c "import flask, requests, cv2, PIL, mss, numpy, psutil; print('All imports successful')"

# Test Tesseract
tesseract --version

# Test network ports
netstat -ano | findstr "8080"
netstat -ano | findstr "8081"
netstat -ano | findstr "8082"

# Test file permissions
icacls automation_config
icacls playtesting_data
icacls screenshots_visual_playtest
```

### Expected Results

**Successful Launch Indicators**:
- Game window opens in 1280x720 resolution
- HTTP API responds on port 8080
- Screenshots appear in `screenshots_visual_playtest/`
- Analysis server responds on port 8081
- Dashboard accessible on port 8082
- No error messages in logs
- Performance metrics being collected

## Troubleshooting

### Common Issues and Solutions

#### Issue 1: Python Module Import Errors

**Symptoms**:
```
ModuleNotFoundError: No module named 'flask'
```

**Solution**:
```bash
# Activate virtual environment
.\venv\Scripts\activate

# Reinstall dependencies
pip install -r requirements.txt
```

#### Issue 2: Tesseract Not Found

**Symptoms**:
```
TesseractNotFoundError: tesseract is not installed or it's not in your PATH
```

**Solution**:
1. Reinstall Tesseract from official source
2. Add to PATH: `C:\Program Files\Tesseract-OCR`
3. Restart computer
4. Verify: `tesseract --version`

#### Issue 3: Port Already in Use

**Symptoms**:
```
OSError: [Errno 10048] Address already in use
```

**Solution**:
```bash
# Find process using port
netstat -ano | findstr "8080"

# Kill the process
taskkill /PID <PID> /F

# Or use different port in configuration
```

#### Issue 4: Game Fails to Launch

**Symptoms**:
```
Failed to launch game: [Error details]
```

**Solution**:
1. Check Unreal Engine path in configuration
2. Verify Alexander.uproject exists
3. Check that AutomationGameMode is compiled
4. Review logs in `automation_config/logs/`
5. Try manual launch first

#### Issue 5: HTTP API Not Responding

**Symptoms**:
```
Cannot connect to AutomationAPIServer
```

**Solution**:
1. Verify game is running
2. Check port 8080 is open: `netstat -ano | findstr "8080"`
3. Ensure AutomationGameMode is set
4. Check game logs for HTTP server startup
5. Verify firewall allows localhost connections

### Log File Locations

- **Game Launcher**: `automation_config/launcher.log`
- **Screenshot Capture**: `automation_config/screenshot_capture.log`
- **Issue Reporter**: `automation_config/issue_reporter.log`
- **Data Manager**: `automation_config/data_manager.log`
- **Playtesting**: `automation_config/logs/playtesting_*.log`

### Getting Help

1. **Check Documentation**: Review all markdown files in project root
2. **Check Logs**: Examine relevant log files for error details
3. **Test Components**: Run individual components to isolate issues
4. **Verify Configuration**: Ensure all paths and settings are correct
5. **Check System Resources**: Ensure sufficient disk space and memory

### Emergency Procedures

**If System Hangs**:
```bash
# Kill all Python processes
taskkill /F /IM python.exe

# Kill Unreal processes
taskkill /F /IM UnrealEditor.exe

# Clean up PID files
del automation_config\game_process.json
```

**If Disk Full**:
```bash
# Run cleanup manually
python data_manager.py --action cleanup

# Or manually delete old screenshots
del screenshots_visual_playtest\screenshot_*.jpg
```

---

**Next Steps**: See [PLAYTESTING_USER_MANUAL.md](PLAYTESTING_USER_MANUAL.md) for usage instructions.