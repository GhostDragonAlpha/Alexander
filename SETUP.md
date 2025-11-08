# Alexander - Development Setup Guide

**Project**: Alexander VR Space Simulation
**Engine**: Unreal Engine 5.6
**Last Updated**: November 8, 2025

---

## Quick Start

### Prerequisites

1. **Unreal Engine 5.6**
   - Install from Epic Games Launcher
   - Default location: `C:\Program Files\Epic Games\UE_5.6\`

2. **Visual Studio 2022**
   - Workloads: Game Development with C++
   - Individual Components: Windows 10/11 SDK

3. **Python 3.8+**
   - Required for automation scripts
   - Install dependencies: `pip install -r requirements.txt`

4. **Git**
   - For version control
   - Configure LFS for large assets (if needed)

### Initial Setup

```bash
# 1. Clone repository
git clone <repository-url>
cd Alexander

# 2. Install Python dependencies
pip install -r requirements.txt

# 3. Generate Visual Studio project files
# Right-click Alexander.uproject → "Generate Visual Studio project files"

# 4. Open solution in Visual Studio
# Open Alexander.sln

# 5. Build the project
# Build → Build Solution (Ctrl+Shift+B)
```

---

## Project Structure

```
Alexander/
├── Source/
│   ├── Alexander/          # Main game module
│   │   ├── Private/        # Implementation (.cpp)
│   │   ├── Public/         # Headers (.h)
│   │   └── Alexander.Build.cs
│   └── AlexanderEditor/    # Editor-only code
│
├── Content/
│   ├── Blueprints/         # Blueprint assets
│   ├── Maps/               # Game levels
│   ├── Materials/          # Materials & textures
│   ├── Models/             # Static/skeletal meshes
│   ├── Data/               # Data assets (missions, dialogues)
│   ├── UI/                 # Widget blueprints
│   └── Python/             # Python automation scripts
│
├── Config/                 # Engine/project configuration
├── Saved/                  # Logs, profiling data, temp files
│   ├── Logs/               # Game and editor logs
│   └── Profiling/          # Performance profiling reports
│
├── Binaries/               # Compiled executables
├── Intermediate/           # Build intermediates
└── Plugins/                # Third-party plugins
```

---

## Building the Project

### Command Line Build

```bash
# Development build (fast iteration)
python automated_build.py

# Full rebuild
python automated_build.py --rebuild

# Shipping build
python build_and_test.py --shipping
```

### Visual Studio Build

1. Set build configuration: `Development Editor`
2. Set platform: `Win64`
3. Build → Build Solution

**Build Configurations:**
- **DebugGame**: Full debugging, slow
- **Development**: Balanced (default for iteration)
- **Shipping**: Optimized, no debug symbols

---

## Running the Game

### From Editor

1. Open `Alexander.uproject` in Unreal Editor
2. Click Play button (Alt+P)
3. Default map: Content/Maps/SolarSystem

### Standalone Build

```bash
# Launch game directly
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" Alexander.uproject -game

# Launch with specific map
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" Alexander.uproject /Game/SolarSystem -game
```

### VR Mode

```bash
# Launch in VR
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" Alexander.uproject -game -vr
```

---

## Common Development Tasks

### Adding New C++ Class

1. In Unreal Editor: Tools → New C++ Class
2. Choose parent class (e.g., Actor, Component)
3. Name the class
4. Editor will generate .h and .cpp files
5. Compile in Visual Studio

**Or manually:**
```cpp
// In Source/Alexander/Public/MyNewClass.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyNewClass.generated.h"

UCLASS()
class ALEXANDER_API AMyNewClass : public AActor
{
    GENERATED_BODY()
public:
    AMyNewClass();
    virtual void Tick(float DeltaTime) override;
};
```

### Hot Reload

- **Live Coding** (Ctrl+Alt+F11): Compile C++ changes without restarting editor
- Faster than full recompile, but can be unstable
- Prefer full rebuilds for major changes

### Debugging

**Attach to running editor:**
1. Visual Studio → Debug → Attach to Process
2. Find `UnrealEditor.exe`
3. Click Attach

**Launch with debugger:**
1. Set `AlexanderEditor` as startup project
2. Press F5 to launch with debugger attached

**Logging:**
```cpp
UE_LOG(LogTemp, Warning, TEXT("Debug message: %s"), *MyString);
```

View logs: Window → Developer Tools → Output Log

---

## Performance Profiling

See [PROFILING.md](PROFILING.md) for complete profiling guide.

**Quick profiling:**
```bash
python run_automated_profiling.py
```

---

## Automation Scripts

| Script | Purpose |
|--------|---------|
| `run_automated_profiling.py` | Automated performance profiling |
| `automated_build.py` | Build project from command line |
| `build_and_test.py` | Build + run tests |
| `nightly_build.py` | CI/CD nightly builds |

---

## Troubleshooting

### Build Errors

**Error: "Cannot open include file"**
- Solution: Regenerate project files (right-click .uproject)

**Error: "Unresolved external symbol"**
- Solution: Check .Build.cs dependencies, rebuild

**Error: "Module not found"**
- Solution: Add module to .uproject Modules array

### Editor Crashes

**Check logs:**
```bash
# Latest log
cat Saved/Logs/Alexander.log

# Crash report
cat Saved/Crashes/...
```

**Common fixes:**
1. Delete `Intermediate/` and `Binaries/` folders
2. Regenerate project files
3. Rebuild solution

### Python Script Issues

**UTF-8 encoding errors:**
- Scripts have UTF-8 wrappers for Windows
- Ensure `encoding='utf-8'` in file operations

**Module not found:**
```bash
pip install -r requirements.txt
```

---

## Git Workflow

### Branches

- `master` - Main branch (stable)
- `feature/*` - Feature branches
- `bugfix/*` - Bug fix branches

### Committing Changes

```bash
# Stage changes
git add Source/ Content/

# Commit
git commit -m "Add ship customization system"

# Push
git push origin feature/my-feature
```

**Important:**
- Don't commit `Binaries/`, `Intermediate/`, `Saved/` folders
- `.gitignore` handles this automatically
- Large assets should use Git LFS

---

## Key Systems Architecture

### Core Systems

1. **FlightController** - 6DOF flight dynamics
2. **OrbitalMechanics** - N-body orbital simulation
3. **PhysicsConsensus** - Multiplayer physics validation
4. **StarSystemManager** - Celestial bodies management
5. **TerrainStreamingManager** - LOD and streaming

### VR Integration

- **VRPlayerController** - VR-specific controls
- **VRCockpitComponent** - Interactive cockpit
- **VRHandInteraction** - Hand tracking and interaction

### Gameplay Systems

- **MissionSystem** - Dynamic mission generation
- **DialogueSystem** - AI-driven conversations
- **ProgressionSystem** - Player advancement
- **TutorialSystem** - Onboarding and guidance

---

## Performance Targets

### VR Requirements

| Metric | Target | Minimum |
|--------|--------|---------|
| FPS | 90+ | 75 |
| Frame Time (CPU) | <8ms | <11ms |
| Frame Time (GPU) | <11ms | <16ms |
| Memory | <8GB | <12GB |

### Optimization Guidelines

- Profile first (don't guess)
- Critical systems: <8ms average
- Use LOD for distant objects
- Implement spatial partitioning
- Cache expensive calculations

---

## Additional Resources

- [PROFILING.md](PROFILING.md) - Performance profiling guide
- [PHASE9_QUICK_START.md](PHASE9_QUICK_START.md) - Quick profiling reference
- [PROJECT_STATUS_NOVEMBER_2025.md](PROJECT_STATUS_NOVEMBER_2025.md) - Project status
- [Unreal Engine Documentation](https://docs.unrealengine.com/5.6/)

---

## Support

**Issues:**
- Check existing documentation
- Review `Saved/Logs/` for errors
- Search git commit history for relevant changes

**Build Issues:**
1. Delete `Intermediate/` and `Binaries/`
2. Regenerate project files
3. Clean and rebuild solution

**Performance Issues:**
1. Run `python run_automated_profiling.py`
2. Identify bottlenecks
3. See PROFILING.md for optimization techniques

---

**Last Updated**: November 8, 2025
**Unreal Engine**: 5.6
**Build Status**: ✅ Passing
