# UnrealFrontend Automation Guide for Alexander Project

## Overview

This guide focuses on **command-line automation** using UnrealFrontend.exe for programmatic builds, cooking, and packaging that can be triggered by scripts or CI/CD systems.

---

## Key Automation Commands

### 1. Automated Build Command

```cmd
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealFrontend.exe" ^
  -project="C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" ^
  -Build ^
  -Configuration=Development ^
  -Platform=Win64 ^
  -unattended ^
  -nopause
```

**Parameters:**
- `-Build`: Compile the C++ code
- `-Configuration`: Development, DebugGame, Shipping, or Test
- `-Platform`: Win64, Linux, Android, etc.
- `-unattended`: Run without user interaction
- `-nopause`: Don't wait for key press on completion

### 2. Cook Content Command

```cmd
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealFrontend.exe" ^
  -project="C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" ^
  -Cook ^
  -TargetPlatform=Win64 ^
  -unattended ^
  -nopause
```

**What it does:**
- Converts assets to platform-specific formats
- Optimizes content for runtime
- Prepares game data for deployment

### 3. Package Game Command

```cmd
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealFrontend.exe" ^
  -project="C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" ^
  -Package ^
  -Configuration=Shipping ^
  -Platform=Win64 ^
  -StagingDirectory="C:\Users\allen\Desktop\Alexander\Builds\Shipping" ^
  -unattended ^
  -nopause
```

**Output:**
- Complete standalone game executable
- All required content files
- Ready-to-distribute build

### 4. Build + Cook + Package (Full Pipeline)

```cmd
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealFrontend.exe" ^
  -project="C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" ^
  -Build ^
  -Cook ^
  -Package ^
  -Configuration=Development ^
  -Platform=Win64 ^
  -StagingDirectory="C:\Users\allen\Desktop\Alexander\Builds\Development" ^
  -unattended ^
  -nopause
```

---

## Integration with Python Scripts

### Example: automated_build.py

```python
import subprocess
import os
import sys
from datetime import datetime

class UnrealFrontendAutomation:
    def __init__(self):
        self.ue_path = r"C:\Program Files\Epic Games\UE_5.6"
        self.frontend_exe = os.path.join(self.ue_path, "Engine", "Binaries", "Win64", "UnrealFrontend.exe")
        self.project_path = r"C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject"
        self.builds_dir = r"C:\Users\allen\Desktop\Alexander\Builds"
        
    def build_project(self, configuration="Development"):
        """Compile C++ code only"""
        print(f"[{datetime.now()}] Building Alexander ({configuration})...")
        
        cmd = [
            self.frontend_exe,
            f'-project="{self.project_path}"',
            '-Build',
            f'-Configuration={configuration}',
            '-Platform=Win64',
            '-unattended',
            '-nopause'
        ]
        
        result = subprocess.run(' '.join(cmd), shell=True, capture_output=True, text=True)
        
        if result.returncode == 0:
            print(f"✓ Build succeeded")
            return True
        else:
            print(f"✗ Build failed")
            print(result.stderr)
            return False
    
    def cook_content(self):
        """Cook game content for Win64"""
        print(f"[{datetime.now()}] Cooking content...")
        
        cmd = [
            self.frontend_exe,
            f'-project="{self.project_path}"',
            '-Cook',
            '-TargetPlatform=Win64',
            '-unattended',
            '-nopause'
        ]
        
        result = subprocess.run(' '.join(cmd), shell=True, capture_output=True, text=True)
        
        if result.returncode == 0:
            print(f"✓ Cook succeeded")
            return True
        else:
            print(f"✗ Cook failed")
            print(result.stderr)
            return False
    
    def package_game(self, configuration="Shipping", build_name=None):
        """Package complete game build"""
        if build_name is None:
            build_name = f"Alexander_{configuration}_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
        
        staging_dir = os.path.join(self.builds_dir, build_name)
        os.makedirs(staging_dir, exist_ok=True)
        
        print(f"[{datetime.now()}] Packaging {configuration} build to {staging_dir}...")
        
        cmd = [
            self.frontend_exe,
            f'-project="{self.project_path}"',
            '-Package',
            f'-Configuration={configuration}',
            '-Platform=Win64',
            f'-StagingDirectory="{staging_dir}"',
            '-unattended',
            '-nopause'
        ]
        
        result = subprocess.run(' '.join(cmd), shell=True, capture_output=True, text=True)
        
        if result.returncode == 0:
            print(f"✓ Package succeeded: {staging_dir}")
            return True
        else:
            print(f"✗ Package failed")
            print(result.stderr)
            return False
    
    def full_build_pipeline(self, configuration="Development"):
        """Build, cook, and package in one go"""
        build_name = f"Alexander_{configuration}_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
        staging_dir = os.path.join(self.builds_dir, build_name)
        os.makedirs(staging_dir, exist_ok=True)
        
        print(f"[{datetime.now()}] Starting full build pipeline ({configuration})...")
        
        cmd = [
            self.frontend_exe,
            f'-project="{self.project_path}"',
            '-Build',
            '-Cook',
            '-Package',
            f'-Configuration={configuration}',
            '-Platform=Win64',
            f'-StagingDirectory="{staging_dir}"',
            '-unattended',
            '-nopause'
        ]
        
        result = subprocess.run(' '.join(cmd), shell=True, capture_output=True, text=True)
        
        if result.returncode == 0:
            print(f"✓ Full pipeline succeeded: {staging_dir}")
            return True
        else:
            print(f"✗ Full pipeline failed")
            print(result.stderr)
            return False

# Usage examples
if __name__ == "__main__":
    automation = UnrealFrontendAutomation()
    
    # Quick development build (compile only)
    if len(sys.argv) > 1 and sys.argv[1] == "build":
        automation.build_project("Development")
    
    # Full shipping package
    elif len(sys.argv) > 1 and sys.argv[1] == "ship":
        automation.full_build_pipeline("Shipping")
    
    # Development package for testing
    elif len(sys.argv) > 1 and sys.argv[1] == "dev":
        automation.full_build_pipeline("Development")
    
    else:
        print("Usage:")
        print("  python automated_build.py build   # Compile only")
        print("  python automated_build.py dev     # Full development build")
        print("  python automated_build.py ship    # Full shipping build")
```

### Example: nightly_build.py

```python
import schedule
import time
from automated_build import UnrealFrontendAutomation

def nightly_build():
    """Run automated nightly build"""
    automation = UnrealFrontendAutomation()
    
    print("=== Starting Nightly Build ===")
    
    # Build and package development version
    success = automation.full_build_pipeline("Development")
    
    if success:
        print("✓ Nightly build completed successfully")
        # Could add: upload to server, send notification, etc.
    else:
        print("✗ Nightly build failed")
        # Could add: send alert email

# Schedule nightly build at 2 AM
schedule.every().day.at("02:00").do(nightly_build)

print("Nightly build scheduler started...")
while True:
    schedule.run_pending()
    time.sleep(60)
```

---

## Batch File Examples

### quick_build.bat

```batch
@echo off
echo Building Alexander (Development)...

"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealFrontend.exe" ^
  -project="C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" ^
  -Build ^
  -Configuration=Development ^
  -Platform=Win64 ^
  -unattended ^
  -nopause

if %ERRORLEVEL% EQU 0 (
    echo Build succeeded!
) else (
    echo Build failed!
    exit /b 1
)
```

### package_shipping.bat

```batch
@echo off
echo Creating Shipping Build...

set BUILD_DIR=C:\Users\allen\Desktop\Alexander\Builds\Shipping_%date:~-4,4%%date:~-10,2%%date:~-7,2%

"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealFrontend.exe" ^
  -project="C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" ^
  -Build ^
  -Cook ^
  -Package ^
  -Configuration=Shipping ^
  -Platform=Win64 ^
  -StagingDirectory="%BUILD_DIR%" ^
  -unattended ^
  -nopause

if %ERRORLEVEL% EQU 0 (
    echo Shipping build created: %BUILD_DIR%
    explorer "%BUILD_DIR%"
) else (
    echo Packaging failed!
    exit /b 1
)
```

---

## Advanced Automation Options

### Additional Command-Line Parameters

```cmd
# Incremental cooking (faster, only cook changed content)
-IterativeCooking

# Skip cooking, use existing cooked content
-SkipCook

# Generate debug symbols for shipping build
-DebugInfo

# Compress pak files
-Compressed

# Create pak file for content
-Pak

# Clean build (rebuild everything)
-Clean

# Build for multiple platforms
-Platform=Win64+Linux

# Specify custom build target
-Target=Alexander

# Enable logging to file
-LogFile="C:\Builds\alexander_build.log"

# Continue on errors (don't stop on first error)
-IgnoreJunkErrors
```

### Full Example with All Options

```cmd
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealFrontend.exe" ^
  -project="C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" ^
  -Build ^
  -Cook ^
  -Package ^
  -Configuration=Shipping ^
  -Platform=Win64 ^
  -StagingDirectory="C:\Builds\Shipping" ^
  -Pak ^
  -Compressed ^
  -IterativeCooking ^
  -LogFile="C:\Builds\build_log.txt" ^
  -unattended ^
  -nopause
```

---

## CI/CD Integration

### GitHub Actions Example

```yaml
name: Automated Build

on:
  push:
    branches: [ main ]
  schedule:
    - cron: '0 2 * * *'  # 2 AM daily

jobs:
  build:
    runs-on: windows-latest
    
    steps:
    - uses: actions/checkout@v2
    
    - name: Build Alexander
      run: |
        "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealFrontend.exe" `
          -project="${{ github.workspace }}\Alexander.uproject" `
          -Build `
          -Configuration=Development `
          -Platform=Win64 `
          -unattended `
          -nopause
    
    - name: Package Game
      if: success()
      run: |
        "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealFrontend.exe" `
          -project="${{ github.workspace }}\Alexander.uproject" `
          -Package `
          -Configuration=Shipping `
          -Platform=Win64 `
          -StagingDirectory="${{ github.workspace }}\Builds\Shipping" `
          -unattended `
          -nopause
    
    - name: Upload Build Artifact
      if: success()
      uses: actions/upload-artifact@v2
      with:
        name: Alexander-Build
        path: Builds/Shipping
```

---

## Integration with Existing Alexander Scripts

### Add to build_and_test.py

```python
# Add this function to your existing build_and_test.py

def build_with_frontend(configuration="Development"):
    """Use UnrealFrontend for automated build"""
    import subprocess
    
    frontend_exe = r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealFrontend.exe"
    project_path = r"C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject"
    
    cmd = [
        frontend_exe,
        f'-project="{project_path}"',
        '-Build',
        f'-Configuration={configuration}',
        '-Platform=Win64',
        '-unattended',
        '-nopause'
    ]
    
    result = subprocess.run(' '.join(cmd), shell=True)
    return result.returncode == 0
```

---

## Use Cases for Alexander Project

### 1. Nightly Automated Builds
```batch
REM Schedule with Windows Task Scheduler
python automated_build.py dev
```

### 2. Pre-Commit Build Verification
```batch
REM Run before git commit
python automated_build.py build
if %ERRORLEVEL% NEQ 0 exit /b 1
git commit -m "Your message"
```

### 3. Release Packaging
```batch
REM Create distribution package
python automated_build.py ship
```

### 4. Continuous Integration
- Trigger builds on git push
- Run automated tests after build
- Deploy to test server

---

## Performance Tips

1. **Incremental Builds**: Use `-IterativeCooking` for faster subsequent builds
2. **Parallel Building**: UnrealFrontend automatically uses multiple cores
3. **Skip Unnecessary Steps**: Use `-SkipCook` if content hasn't changed
4. **Clean Builds**: Use `-Clean` when switching configurations

---

## Monitoring and Logging

### Log Output to File

```cmd
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealFrontend.exe" ^
  -project="C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" ^
  -Build ^
  -Configuration=Development ^
  -Platform=Win64 ^
  -LogFile="C:\Builds\Logs\build_%date:~-4,4%%date:~-10,2%%date:~-7,2%.log" ^
  -unattended ^
  -nopause
```

### Parse Build Results

```python
def check_build_log(log_file):
    """Parse log file for errors"""
    with open(log_file, 'r') as f:
        content = f.read()
        
    if "BUILD SUCCESSFUL" in content:
        print("✓ Build completed successfully")
        return True
    elif "BUILD FAILED" in content:
        print("✗ Build failed")
        # Extract error messages
        errors = [line for line in content.split('\n') if 'error' in line.lower()]
        for error in errors[:10]:  # Show first 10 errors
            print(f"  - {error}")
        return False
```

---

## Summary

**UnrealFrontend.exe for automation provides:**

✅ Command-line build automation  
✅ Integration with Python scripts  
✅ CI/CD pipeline support  
✅ Scheduled/nightly builds  
✅ Multi-platform packaging  
✅ Complete build+cook+package workflows  

**Best used for:**
- Automated nightly builds
- CI/CD integration
- Release packaging
- Multi-platform builds
- Scheduled testing builds

**Not needed for:**
- Quick code-test iteration (use Visual Studio)
- Editor content work (use Unreal Editor)
- Blueprint-only changes (use hot reload)
