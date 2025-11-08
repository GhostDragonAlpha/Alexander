# Unreal Engine 5.6 Helper Programs - Automation Integration Guide

## Overview
The UE 5.6 Engine\Binaries\Win64 folder contains numerous standalone helper programs that can be integrated into automated workflows for build, test, profiling, and deployment pipelines.

## Location
`C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\`

## Core Automation Tools

### 1. UnrealEditor.exe
**Purpose**: Main editor executable for automation testing

**Command-Line Interface**:
```bash
UnrealEditor.exe [ProjectPath] [MapName] [Options]
```

**Key Options**:
- `-game` - Run as game (not editor)
- `-server` - Run as dedicated server
- `-log` - Show log window
- `-nullrhi` - No rendering (headless mode)
- `-windowed -ResX=800 -ResY=600` - Windowed mode with resolution
- `-HTTPPort=8080` - Enable web control API
- `-unattended` - No user interaction prompts
- `-nopause` - Don't pause on exit
- `-nosplash` - Skip splash screen
- `-stdout` - Output to stdout for capturing

**Current Usage**:
- [launch_game.py](launch_game.py) - Already integrated
- [test_earth_moon_orbit.py](test_earth_moon_orbit.py) - Testing with nullrhi mode
- [AutomationGameMode](Source/Alexander/Public/AutomationGameMode.h) - HTTP control enabled

**Automation Integration**: ✅ ALREADY INTEGRATED

---

### 2. UnrealPak.exe
**Purpose**: Package file (.pak) creation, extraction, and management

**Command-Line Interface**:
```bash
# Create .pak file
UnrealPak.exe [PakFileName.pak] -create=[ResponseFile.txt] [Options]

# Extract .pak file
UnrealPak.exe [PakFileName.pak] -extract [TargetPath] [Options]

# Test .pak integrity
UnrealPak.exe [PakFileName.pak] -test
```

**Key Options**:
- `-compress` - Compress files in pak
- `-encrypt` - Encrypt pak with AES key
- `-order=[OrderFile.txt]` - Optimize file order for streaming
- `-sign` - Sign the pak file
- `-diff` - Create diff patch pak
- `-list` - List pak contents

**Use Cases**:
1. **Automated Packaging**: Create distribution builds
2. **Content Validation**: Verify pak integrity in CI/CD
3. **Patch Generation**: Create incremental update paks
4. **Asset Analysis**: List and extract for debugging

**Automation Script Example**:
```python
# ue_pak_automation.py
import subprocess
import os

class UnrealPakAutomation:
    def __init__(self):
        self.pak_exe = r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealPak.exe"

    def create_pak(self, pak_name, response_file, compress=True, encrypt=False):
        """Create a .pak file from response file listing"""
        cmd = [self.pak_exe, pak_name, f"-create={response_file}"]
        if compress:
            cmd.append("-compress")
        if encrypt:
            cmd.append("-encrypt")

        result = subprocess.run(cmd, capture_output=True, text=True)
        return result.returncode == 0, result.stdout

    def extract_pak(self, pak_file, target_path):
        """Extract .pak file contents"""
        cmd = [self.pak_exe, pak_file, "-extract", target_path]
        result = subprocess.run(cmd, capture_output=True, text=True)
        return result.returncode == 0

    def test_pak_integrity(self, pak_file):
        """Verify pak file integrity"""
        cmd = [self.pak_exe, pak_file, "-test"]
        result = subprocess.run(cmd, capture_output=True, text=True)
        return result.returncode == 0, result.stdout
```

**Automation Integration**: 🔄 TO BE IMPLEMENTED

---

### 3. UnrealInsights.exe
**Purpose**: Performance profiling and trace analysis tool

**Command-Line Interface**:
```bash
# Launch Insights (GUI-based, limited CLI)
UnrealInsights.exe [TraceFile.utrace]
```

**Integration Strategy**:
- **Trace Capture**: Use `-trace` flags in UnrealEditor.exe
- **Automated Analysis**: Parse .utrace files programmatically
- **Performance Regression**: Compare traces between builds

**Trace Capture Commands**:
```bash
# Enable CPU profiling
UnrealEditor.exe [Project] -trace=cpu,frame,log

# Enable GPU profiling
UnrealEditor.exe [Project] -trace=gpu

# Full profiling session
UnrealEditor.exe [Project] -trace=cpu,gpu,frame,loadtime,file,bookmark
```

**Automation Script Example**:
```python
# ue_insights_automation.py
import subprocess
import os
from pathlib import Path

class UnrealInsightsAutomation:
    def __init__(self):
        self.editor_exe = r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe"
        self.insights_exe = r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealInsights.exe"
        self.trace_dir = Path.home() / "AppData/Local/UnrealEngine/Common/UnrealTrace/Store"

    def start_profiling_session(self, project_path, map_name, trace_channels="cpu,gpu,frame"):
        """Start game with profiling enabled"""
        cmd = [
            self.editor_exe,
            project_path,
            map_name,
            "-game",
            f"-trace={trace_channels}",
            "-windowed",
            "-ResX=1280",
            "-ResY=720"
        ]
        return subprocess.Popen(cmd)

    def get_latest_trace(self):
        """Find most recent trace file"""
        traces = list(self.trace_dir.glob("*.utrace"))
        if traces:
            return max(traces, key=lambda p: p.stat().st_mtime)
        return None

    def analyze_trace(self, trace_file):
        """Open trace in Insights (manual analysis)"""
        subprocess.Popen([self.insights_exe, str(trace_file)])
```

**Automation Integration**: 🔄 TO BE IMPLEMENTED

---

### 4. BuildPatchTool.exe
**Purpose**: Content patching and chunked build distribution

**Command-Line Interface**:
```bash
BuildPatchTool.exe -mode=[ModeName] [Options]
```

**Modes**:
- `PatchGeneration` - Create binary patches
- `Enumeration` - List files in build
- `Compactify` - Optimize patch data storage
- `Verification` - Verify build integrity

**Key Options**:
- `-BuildRoot=[Path]` - Source build directory
- `-CloudDir=[Path]` - Patch storage location
- `-BuildVersion=[Version]` - Version string
- `-AppLaunch=[Executable]` - App to launch after patch
- `-AppArgs=[Arguments]` - Arguments for launched app

**Use Cases**:
1. **Automated Patching**: Generate patches between builds
2. **CI/CD Integration**: Create update packages in pipeline
3. **Version Management**: Track and distribute incremental updates
4. **Build Verification**: Validate deployed builds

**Automation Integration**: 🔄 TO BE IMPLEMENTED

---

### 5. UnrealBuildTool.exe
**Purpose**: C++ compilation and project file generation

**Command-Line Interface**:
```bash
UnrealBuildTool.exe [Target] [Platform] [Configuration] [Options]

# Examples:
UnrealBuildTool.exe AlexanderEditor Win64 Development -Project="Alexander.uproject"
UnrealBuildTool.exe Alexander Win64 Shipping -Project="Alexander.uproject"
```

**Key Options**:
- `-WaitMutex` - Wait if another build is in progress
- `-Clean` - Clean before building
- `-SkipBuild` - Generate project files only
- `-Verbose` - Detailed output
- `-NoHotReload` - Disable hot reload
- `-2019` / `-2022` - Visual Studio version

**Current Usage**:
```bash
# Already used in build automation
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" AlexanderEditor Win64 Development -Project="Alexander.uproject" -waitmutex
```

**Automation Integration**: ✅ ALREADY USED (via Build.bat wrapper)

---

### 6. UnrealAutomationTool.exe (UAT)
**Purpose**: High-level automation for builds, cooks, and packaging

**Command-Line Interface**:
```bash
UnrealAutomationTool.exe [Command] [Options]
```

**Common Commands**:
- `BuildCookRun` - Complete build, cook, and package pipeline
- `Cook` - Cook content for target platform
- `Package` - Package project for distribution
- `Stage` - Stage files for deployment
- `Archive` - Create archive build
- `RunUnreal` - Run editor/game with automation

**BuildCookRun Example**:
```bash
UnrealAutomationTool.exe BuildCookRun ^
  -project="C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" ^
  -platform=Win64 ^
  -configuration=Shipping ^
  -cook ^
  -package ^
  -stage ^
  -archive -archivedirectory="C:\Builds\Alexander_v1.0" ^
  -build ^
  -pak ^
  -compressed
```

**Automation Script Example**:
```python
# ue_build_automation.py
import subprocess
from pathlib import Path

class UnrealBuildAutomation:
    def __init__(self, project_path):
        self.project_path = Path(project_path)
        self.uat_exe = r"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\RunUAT.bat"

    def build_cook_package(self, platform="Win64", configuration="Shipping",
                          archive_dir=None, pak=True, compress=True):
        """Complete build, cook, and package workflow"""
        cmd = [
            self.uat_exe,
            "BuildCookRun",
            f"-project={self.project_path}",
            f"-platform={platform}",
            f"-configuration={configuration}",
            "-cook",
            "-package",
            "-stage",
            "-build",
            "-unattended",  # No user prompts
            "-utf8output"   # Better log encoding
        ]

        if pak:
            cmd.append("-pak")
        if compress:
            cmd.append("-compressed")
        if archive_dir:
            cmd.extend(["-archive", f"-archivedirectory={archive_dir}"])

        result = subprocess.run(cmd, capture_output=True, text=True)
        return result.returncode == 0, result.stdout

    def cook_only(self, platform="Win64", maps=None):
        """Cook content without building"""
        cmd = [
            self.uat_exe,
            "Cook",
            f"-project={self.project_path}",
            f"-platform={platform}",
            "-unattended"
        ]

        if maps:
            cmd.append(f"-map={'+'.join(maps)}")

        result = subprocess.run(cmd, capture_output=True, text=True)
        return result.returncode == 0, result.stdout
```

**Automation Integration**: 🔄 TO BE IMPLEMENTED

---

### 7. CrashReportClient.exe
**Purpose**: Automated crash report submission

**Use Case**: Capture and submit crash data from automated test runs

**Integration**: Typically runs automatically on crashes, can be monitored for CI/CD failure detection

**Automation Integration**: ⚪ PASSIVE (auto-triggered)

---

### 8. ShaderCompileWorker.exe
**Purpose**: Distributed shader compilation worker

**Use Case**: Speeds up shader compilation in large projects

**Integration**: Spawned automatically by editor/cook processes, no direct CLI control needed

**Automation Integration**: ⚪ PASSIVE (auto-spawned)

---

### 9. UnrealLightmass.exe
**Purpose**: Static lighting build computation

**Use Case**: Offline lighting builds for automated level processing

**Integration**: Triggered by editor's lighting build system

**Automation Integration**: ⚪ PASSIVE (via editor)

---

### 10. LiveCodingConsole.exe
**Purpose**: Hot-reload C++ changes without full recompile

**Use Case**: Development workflow acceleration

**Integration**: Editor-integrated, minimal automation value

**Automation Integration**: ❌ NOT APPLICABLE for automation

---

## Proposed Automation Framework

### Directory Structure
```
Alexander/
├── automation/
│   ├── ue_tools/
│   │   ├── __init__.py
│   │   ├── ue_pak.py           # UnrealPak automation
│   │   ├── ue_insights.py      # Performance profiling
│   │   ├── ue_build.py         # Build/Cook/Package
│   │   ├── ue_patch.py         # BuildPatchTool
│   │   └── ue_editor.py        # Editor automation (existing)
│   ├── workflows/
│   │   ├── nightly_build.py    # Automated nightly builds
│   │   ├── performance_test.py # Profiling workflows
│   │   ├── packaging.py        # Release packaging
│   │   └── patch_generation.py # Update patch creation
│   └── config/
│       ├── build_config.json
│       ├── packaging_config.json
│       └── profiling_config.json
```

### Integration with Existing Scripts
```python
# Enhanced launch_game.py with profiling
from automation.ue_tools import ue_insights

# Start profiling session
profiler = ue_insights.UnrealInsightsAutomation()
game_process = profiler.start_profiling_session(
    "Alexander.uproject",
    "VRTemplateMap",
    trace_channels="cpu,gpu,frame,physics"
)

# Run tests
# ... existing test code ...

# Analyze results
trace_file = profiler.get_latest_trace()
profiler.analyze_trace(trace_file)
```

## Priority Implementation Order

### Phase 1: Build Automation (HIGH PRIORITY)
1. ✅ **UnrealEditor.exe** - Already integrated
2. 🔄 **UnrealBuildTool** - Wrap existing Build.bat calls
3. 🔄 **UAT BuildCookRun** - Complete packaging pipeline

### Phase 2: Performance Testing (MEDIUM PRIORITY)
4. 🔄 **UnrealInsights.exe** - Automated profiling sessions
5. 🔄 **Trace Analysis** - Parse .utrace files for regression detection

### Phase 3: Distribution (MEDIUM PRIORITY)
6. 🔄 **UnrealPak.exe** - Content packaging
7. 🔄 **BuildPatchTool.exe** - Incremental updates

### Phase 4: Advanced (LOW PRIORITY)
8. ⚪ **Crash Analysis** - Automated crash report monitoring
9. ⚪ **Distributed Builds** - XGE/FastBuild integration

## Next Steps

1. **Create Python module structure**: `automation/ue_tools/`
2. **Implement ue_build.py**: Wrap UAT for build automation
3. **Implement ue_insights.py**: Profiling automation
4. **Implement ue_pak.py**: Packaging automation
5. **Create workflow scripts**: Nightly builds, performance tests
6. **Integrate with CI/CD**: GitHub Actions / Jenkins / TeamCity

## Command Reference Sheet

### Build Commands
```bash
# Editor build
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" AlexanderEditor Win64 Development "Alexander.uproject" -waitmutex

# Game build
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" Alexander Win64 Shipping "Alexander.uproject" -waitmutex

# Full package
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="Alexander.uproject" -platform=Win64 -configuration=Shipping -cook -pak -stage -archive
```

### Profiling Commands
```bash
# CPU profiling
UnrealEditor.exe Alexander.uproject -game -trace=cpu,frame,log

# Full profiling
UnrealEditor.exe Alexander.uproject -game -trace=cpu,gpu,frame,loadtime,physics,bookmark -windowed -ResX=1280 -ResY=720
```

### Packaging Commands
```bash
# Create .pak
UnrealPak.exe Alexander.pak -create=FileList.txt -compress

# Test .pak
UnrealPak.exe Alexander.pak -test

# Extract .pak
UnrealPak.exe Alexander.pak -extract C:\Extracted
```

---
**Created**: 2025-11-08
**Status**: DOCUMENTED - Implementation in progress
**Priority**: HIGH for build automation, MEDIUM for profiling, LOW for advanced features
