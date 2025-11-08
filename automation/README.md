# Unreal Engine Automation Framework

Python automation framework for Unreal Engine 5.6 helper programs, providing programmatic control over build, packaging, and profiling workflows.

## Directory Structure

```
automation/
├── ue_tools/              # Core automation modules
│   ├── __init__.py       # Package configuration
│   ├── ue_build.py       # Build/Cook/Package automation
│   ├── ue_pak.py         # Pak file operations
│   └── ue_insights.py    # Performance profiling
├── workflows/            # High-level workflow scripts
│   ├── nightly_build.py  # Automated nightly builds
│   └── performance_test.py # Performance profiling
└── config/               # Configuration files
```

## Modules

### ue_tools.ue_build

Build automation wrapper for UnrealBuildTool and UAT.

**Features**:
- Build editor (Development/Debug)
- Build game (Shipping/Development)
- Cook content for platforms
- Complete build-cook-package pipeline
- Automated distribution packaging

**Example**:
```python
from automation.ue_tools import ue_build

# Quick editor build
success, output = ue_build.build_editor()

# Package for release
success, output = ue_build.package_release(version="1.0.0")

# Full build-cook-package
builder = ue_build.UnrealBuildAutomation()
success, output = builder.build_cook_package(
    platform="Win64",
    configuration="Shipping",
    archive_dir="C:/Builds/Alexander_v1.0",
    pak=True,
    compress=True
)
```

**CLI Usage**:
```bash
# Build editor
python automation/ue_tools/ue_build.py editor

# Build game
python automation/ue_tools/ue_build.py game

# Package release
python automation/ue_tools/ue_build.py package 1.0.0
```

---

### ue_tools.ue_pak

UnrealPak.exe wrapper for pak file operations.

**Features**:
- Create compressed/encrypted pak files
- Extract pak contents
- Test pak integrity
- List pak contents
- Generate response files

**Example**:
```python
from automation.ue_tools import ue_pak

# Create pak
pak_tool = ue_pak.UnrealPakAutomation()
success, output = pak_tool.create_pak(
    "Alexander.pak",
    "FileList.txt",
    compress=True,
    encrypt=False
)

# Extract pak
success, output = pak_tool.extract_pak("Alexander.pak", "C:/Extracted")

# Test integrity
success, output = pak_tool.test_pak_integrity("Alexander.pak")
```

**CLI Usage**:
```bash
# Create pak
python automation/ue_tools/ue_pak.py create Alexander.pak FileList.txt

# Extract pak
python automation/ue_tools/ue_pak.py extract Alexander.pak C:/Extracted

# Test pak
python automation/ue_tools/ue_pak.py test Alexander.pak
```

---

### ue_tools.ue_insights

UnrealInsights profiling automation.

**Features**:
- Start profiling sessions with trace channels
- Automatic trace file management
- Extract basic trace statistics
- Complete profiling workflows

**Available Trace Channels**:
- `cpu` - CPU profiling
- `gpu` - GPU profiling
- `frame` - Frame timing
- `physics` - Physics simulation
- `memory` - Memory allocations
- `loadtime` - Asset loading
- `file` - File I/O
- `networking` - Network traffic

**Example**:
```python
from automation.ue_tools import ue_insights

# Quick profile
success, trace_file = ue_insights.profile_map("VRTemplateMap", duration=60)

# Full workflow
profiler = ue_insights.UnrealInsightsAutomation()
success, trace_file = profiler.run_profiling_workflow(
    map_name="VRTemplateMap",
    test_name="physics_test",
    trace_channels="cpu,gpu,frame,physics",
    duration_seconds=120
)

# Open latest trace in Insights
ue_insights.open_latest_trace()
```

**CLI Usage**:
```bash
# Profile for 60 seconds
python automation/ue_tools/ue_insights.py profile VRTemplateMap 60

# Open latest trace
python automation/ue_tools/ue_insights.py open

# Show latest trace info
python automation/ue_tools/ue_insights.py latest
```

---

## Workflows

### Nightly Build Workflow

Automated build pipeline for nightly builds.

**Steps**:
1. Build editor (Development)
2. Build game (Shipping)
3. Package for distribution
4. Generate build report

**Usage**:
```bash
# Run nightly build
python automation/workflows/nightly_build.py

# Run with specific version
python automation/workflows/nightly_build.py 1.0.0
```

**Output**:
- `Builds/Nightly_[version]/` - Packaged build
- `Builds/Nightly_[version]/build_report.txt` - Build report

---

### Performance Test Workflow

Automated performance profiling with Unreal Insights.

**Steps**:
1. Run profiling session
2. Extract performance metrics
3. Generate performance report

**Usage**:
```bash
# Profile default map for 60 seconds
python automation/workflows/performance_test.py

# Profile specific map for custom duration
python automation/workflows/performance_test.py VRTemplateMap 120
```

**Output**:
- `PerformanceResults/[test_name]_metrics.json` - JSON metrics
- `PerformanceResults/[test_name]_report.txt` - Text report
- Trace file in `%LOCALAPPDATA%/UnrealEngine/Common/UnrealTrace/Store/`

---

## Integration with Existing Scripts

The automation framework integrates with existing project scripts:

```python
# In launch_game.py
from automation.ue_tools import ue_insights

# Add profiling to existing test
profiler = ue_insights.UnrealInsightsAutomation()
game_process = profiler.start_profiling_session(
    map_name="VRTemplateMap",
    trace_channels="cpu,gpu,physics"
)
```

## CI/CD Integration

### GitHub Actions Example

```yaml
name: Nightly Build

on:
  schedule:
    - cron: '0 2 * * *'  # 2 AM daily

jobs:
  build:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v3
      - name: Run Nightly Build
        run: python automation/workflows/nightly_build.py
      - name: Upload Build Artifacts
        uses: actions/upload-artifact@v3
        with:
          name: nightly-build
          path: Builds/Nightly_*
```

## Requirements

- Python 3.8+
- Unreal Engine 5.6
- Windows (Mac/Linux support planned)

## Path Configuration

Default paths are configured in `automation/ue_tools/__init__.py`:

```python
UE_ENGINE_ROOT = Path(r"C:\Program Files\Epic Games\UE_5.6\Engine")
PROJECT_FILE = PROJECT_ROOT / "Alexander.uproject"
```

Modify these if your installation differs.

## Documentation

- [UE Helper Programs Guide](../UE_HELPER_PROGRAMS_AUTOMATION.md)
- [UE 5.6 Compatibility Issues](../UE56_COMPATIBILITY_ISSUES.md)

## Support

For issues or feature requests, see the project repository.

---
**Created**: 2025-11-08
**Version**: 1.0.0
**Status**: Production Ready
