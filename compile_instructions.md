# Compilation Instructions for Alexander Project

## Overview
Complete step-by-step guide to compiling the Alexander Unreal Engine 5.6 project with ship customization system.

---

## Prerequisites

### System Requirements
- **Windows 10/11** (64-bit)
- **8GB RAM minimum** (16GB recommended)
- **50GB free disk space** (SSD strongly recommended)
- **GPU**: NVIDIA GTX 960+ or equivalent

### Software Requirements
- **Unreal Engine 5.6** - Download from Epic Games Launcher
- **Visual Studio 2022** - Community Edition or higher
  - Workload: Desktop development with C++
  - Components: Windows 10 SDK, CMake tools for Windows
- **Git** - For version control (optional but recommended)

### Visual Studio 2022 Setup
1. Install Visual Studio 2022 Community Edition
2. Run Visual Studio Installer
3. Click "Modify" on existing installation
4. Add following workloads:
   - Desktop development with C++
5. Add individual components:
   - Windows 10 SDK (latest)
   - CMake tools for Windows
6. Click "Modify" to complete installation

---

## Project Structure

```
Alexander/
├── Source/
│   └── Alexander/
│       ├── Public/          # Header files (.h)
│       ├── Private/         # Implementation files (.cpp)
│       └── Alexander.Build.cs
├── Binaries/                # Compiled executables
├── Intermediate/            # Intermediate build files
├── Saved/                   # Runtime generated files
├── Content/                 # Game assets
├── Alexander.uproject       # Project file
└── Build_Verification_*.md  # Verification documents
```

---

## Step 1: Pre-Compilation Verification

Before attempting compilation, verify the codebase:

### Run Verification Scripts
```bash
# Navigate to project directory
cd "c:\Users\allen\Desktop\Alexander\Alexander"

# Check include statements
python verify_includes.py

# Verify Build.cs configuration
python verify_build_cs.py

# Check replication setup
python verify_replication.py
```

### Review Reports
- Check `BUILD_STATUS_REPORT.md` for any issues
- Fix errors before proceeding to compilation
- Address warnings if they seem critical

### Checklist
- [ ] Run verify_includes.py - no errors
- [ ] Run verify_build_cs.py - no errors
- [ ] Run verify_replication.py - no errors
- [ ] BUILD_STATUS_REPORT.md reviewed

---

## Step 2: Generate Visual Studio Project Files

### Method A: Using Unreal Engine Editor

1. **Open Unreal Engine Project Launcher**
   - Start Epic Games Launcher
   - Navigate to Library
   - Select Unreal Engine 5.6

2. **Browse to Alexander.uproject**
   - In launcher, click "Browse"
   - Navigate to: `c:\Users\allen\Desktop\Alexander\Alexander`
   - Select `Alexander.uproject`

3. **Generate Project Files**
   - Right-click on Alexander.uproject
   - Select "Generate Visual Studio project files"
   - Wait for completion

### Method B: Using Command Line

```bash
# Open Command Prompt or PowerShell
# Navigate to project root
cd "c:\Users\allen\Desktop\Alexander\Alexander"

# Generate Visual Studio project files
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\GenerateProjectFiles.bat" Alexander.uproject -AllowUE4Projects
```

### Verification
- [ ] `Alexander.sln` file created in project root
- [ ] `Intermediate` directory expanded
- [ ] `Visual Studio project files generated

---

## Step 3: Build from Visual Studio

### Opening the Solution

1. **Open Alexander.sln**
   - Location: `c:\Users\allen\Desktop\Alexander\Alexander\Alexander.sln`
   - Right-click → Open with → Visual Studio 2022
   - Or open Visual Studio first, then File → Open → Alexander.sln

2. **Visual Studio Workspace Setup**
   - Solution: Alexander
   - Startup Project: Alexander (should show in bold)
   - Configuration: Select "Development Editor" from dropdown
   - Platform: Win64

### Clean Solution

1. **In Visual Studio**
   - Menu: Build → Clean Solution
   - Wait for output window to show "Done"

### Build Solution

1. **Build Configuration**
   - Ensure Configuration is "Development Editor"
   - Ensure Platform is "Win64"

2. **Compile**
   - Menu: Build → Build Solution
   - Or press: Ctrl + Shift + B
   - Wait for compilation to complete

3. **Monitor Build Progress**
   - Watch Output window for progress
   - Common stages:
     - Compiling: ShipCustomizationComponent.cpp
     - Compiling: ShipPhysicsConfig.cpp
     - Compiling: OrbitalBody.cpp
     - Linking...

### Build Completed Successfully When
- Output shows: "Build succeeded" at bottom
- No red error messages in Error List
- Binaries generated in `Binaries\Win64\UE4Editor-Alexander.dll`

---

## Step 4: Successful Build Verification

### Check Binaries
```
c:\Users\allen\Desktop\Alexander\Alexander\Binaries\Win64\
├── UE4Editor-Alexander.dll       (Main module)
├── UE4Editor-Alexander.lib       (Import library)
└── UE4Editor-Alexander.pdb       (Debug symbols)
```

### Verify Files Exist
- [ ] `UE4Editor-Alexander.dll` present
- [ ] File size > 1 MB (indicates compiled code)
- [ ] Modified date = current date/time
- [ ] `.pdb` debug file present (for debugging)

### Verify Intermediate Files
```
c:\Users\allen\Desktop\Alexander\Alexander\Intermediate\
└── Build\Win64\UE4Editor\Inc\Alexander\
    ├── ShipCustomizationComponent.generated.h
    ├── ShipPhysicsConfig.generated.h
    ├── OrbitalBody.generated.h
    └── (other generated files)
```

---

## Step 5: Test Compilation

### Launch Editor
1. In Visual Studio, press F5 or Debug → Start Debugging
2. Unreal Engine editor should launch
3. Wait for project to load (may take 1-2 minutes)

### Verify in Editor
1. Check Output Log for errors
   - Window → Output Log
   - Should not see C++ compilation errors
2. Navigate to Content Browser
3. Verify classes are accessible:
   - ShipCustomizationComponent
   - ShipPhysicsConfig
   - OrbitalBody
   - FlightController

### Run PIE Test
1. Click Play button (or Alt + P)
2. Game should launch in viewport
3. Check console for errors
4. Look for warning about missing data tables (expected if not set up)
5. Stop Play session (Esc)

---

## Command Line Compilation

### Build from Command Prompt

```batch
REM Navigate to project root
cd "c:\Users\allen\Desktop\Alexander\Alexander"

REM Generate project files
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\GenerateProjectFiles.bat" Alexander.uproject -AllowUE4Projects

REM Build using UnrealBuildTool
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" Alexander Development Win64 -Project="c:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" -TargetType=Editor
```

### Build Using MSBuild

```batch
REM Navigate to project root
cd "c:\Users\allen\Desktop\Alexander\Alexander"

REM Build with MSBuild
msbuild Alexander.sln /p:Configuration=Development /p:Platform=x64 /t:Build
```

---

## Common Compilation Errors

### Error: "Unresolved External Symbol"

**Cause**: Missing module dependency or function implementation

**Solutions**:
1. Check Alexander.Build.cs for required modules
2. Verify all .cpp files are in Source directory
3. Search for function declaration in .h files
4. Ensure function is implemented in corresponding .cpp

**Example**:
```
Error: unresolved external symbol "UFlightController::GetMaxVelocity"
Fix: Add FlightController.cpp to Source/Alexander/Private
```

### Error: "C4251 - DLL Interface Requires..." (Warning)

**Cause**: Template member variables in exported classes

**Solutions**:
1. Use pointers instead of embedded objects:
   ```cpp
   // Bad
   TArray<FShipPartData> Parts;

   // Good
   TArray<FShipPartData>* Parts;
   ```

2. Use forward declares with pointers:
   ```cpp
   class UFlightController*  FlightCtrl;  // OK
   class UFlightController   FlightCtrl;  // Error
   ```

3. Mark with ALEXANDER_API:
   ```cpp
   UPROPERTY()
   class ALEXANDER_API UFlightController* FlightCtrl;
   ```

### Error: "File Not Found - ShipCustomizationComponent.generated.h"

**Cause**: Include directive not processed by Unreal Header Tool (UHT)

**Solutions**:
1. Verify UCLASS/USTRUCT/UENUM macros are correct
2. Check for syntax errors in macro parameters
3. Ensure .generated.h include is at END of .h file
4. Regenerate Visual Studio files:
   ```bash
   GenerateProjectFiles.bat Alexander.uproject -AllowUE4Projects
   ```

### Error: "Ambiguous Symbol - Multiple Definitions"

**Cause**: Circular #include or duplicate definitions

**Solutions**:
1. Use forward declarations instead of includes
2. Move includes to .cpp files
3. Check for header guards: `#pragma once`
4. Use cyclic dependency checker:
   ```bash
   python verify_includes.py
   ```

### Error: "Missing Required Module"

**Example**: `Missing module 'CoreUObject' required by 'Alexander'`

**Solution**:
1. Add to Alexander.Build.cs PublicDependencyModuleNames
2. Regenerate project files
3. Rebuild solution

### Error: "Macro Mismatch or Invalid"

**Cause**: Wrong UPROPERTY/UFUNCTION parameters

**Solutions**:
1. Check macro syntax is correct
2. Ensure GENERATED_BODY() present
3. Verify no typos in macro keywords
4. Run `verify_build_cs.py` to check macros

---

## Compilation Performance Tips

### Faster Builds
1. **Use Visual Studio 2022** (faster than older versions)
2. **Use SSD** for source and intermediate files
3. **Enable parallel compilation**:
   - Tools → Options → Projects and Solutions → VC++ Project Settings
   - Enable parallel builds
4. **Use precompiled headers (PCH)**:
   - Enabled by default in Alexander.Build.cs
5. **Disable unnecessary modules**:
   - Remove unused modules from Alexander.Build.cs

### Monitor Build Performance
1. Visual Studio output shows compile time per file
2. Times > 10 seconds indicate heavy template processing
3. Check Diagnostic Tools for CPU/memory usage

---

## Troubleshooting Build Issues

### Step 1: Clean Everything
```batch
cd "c:\Users\allen\Desktop\Alexander\Alexander"
rmdir /s /q Intermediate
rmdir /s /q Binaries
rmdir /s /q Saved
del Alexander.sln
del *.VC.db
```

### Step 2: Regenerate Files
```batch
GenerateProjectFiles.bat Alexander.uproject -AllowUE4Projects
```

### Step 3: Run Verification Scripts
```bash
python verify_includes.py
python verify_build_cs.py
python verify_replication.py
```

### Step 4: Rebuild from Visual Studio
- Open Alexander.sln
- Build → Clean Solution
- Build → Build Solution

### Step 5: Check Logs
- Output window in Visual Studio shows detailed errors
- Save output to file for analysis
- Look for first error (subsequent errors often cascade)

---

## Building for Shipping

### Create Shipping Build
```batch
REM In Command Prompt
msbuild Alexander.sln /p:Configuration=Shipping /p:Platform=x64
```

### Shipping Build Differences
- Optimizations enabled
- Debug symbols stripped
- ~1/3 size of Development builds
- Cannot use debugger

### Distribution
1. Create Shipping build
2. Package all files in `Binaries\Win64\Shipping\`
3. Include required engine DLLs
4. Create installer if needed

---

## Useful Build Commands

| Command | Purpose |
|---------|---------|
| Ctrl + Shift + B | Build solution |
| Ctrl + Alt + Delete (Editor) | Stop running editor |
| F5 | Start debugging |
| F11 | Toggle breakpoint |
| Ctrl + L | Show output window |
| Build → Rebuild Solution | Clean + Build |
| Build → Clean Solution | Remove binaries |

---

## Build Verification Checklist

- [ ] Visual Studio 2022 installed with C++ workload
- [ ] Unreal Engine 5.6 installed
- [ ] Project files generated (Alexander.sln exists)
- [ ] All verification scripts run successfully
- [ ] No C++ compilation errors
- [ ] Binaries generated in Binaries/Win64/
- [ ] Editor launches successfully
- [ ] Ship customization classes accessible in editor
- [ ] PIE test runs without C++ errors

---

## Next Steps

After successful compilation:

1. **Set Up Game Data**
   - Create ship parts data table
   - Create ship skins data table
   - Create physics presets

2. **Test Implementation**
   - Create test character
   - Attach ShipCustomizationComponent
   - Test equip/unequip in PIE

3. **Integrate with Game**
   - Add to player character blueprint
   - Connect UI to ship customization
   - Test multiplayer replication

4. **Profiling and Optimization**
   - Use Unreal Insights for profiling
   - Optimize hot paths
   - Test on target hardware

---

## Support and Resources

- **Unreal Engine Documentation**: https://docs.unrealengine.com
- **Build Verification Checklist**: `BUILD_VERIFICATION_CHECKLIST.md`
- **Report Format**: `BUILD_STATUS_REPORT.md`
- **Verification Scripts**: `verify_*.py` scripts in project root

---

## Version History

| Date | Version | Changes |
|------|---------|---------|
| 2025-11-07 | 1.0 | Initial compilation guide |

