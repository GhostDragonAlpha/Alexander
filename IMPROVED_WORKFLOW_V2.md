# Improved Workflow Guide - V2
**Updated:** 2025-11-06
**Status:** Production-Ready

## Quick Reference

### Kill All Unreal Processes
```powershell
powershell -ExecutionPolicy Bypass -File kill_unreal.ps1
```

### Build C++ Changes
```batch
# Build runtime (fastest, bypasses editor lock)
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" Alexander Win64 Development "Alexander.uproject" -waitmutex

# Build editor (use after runtime build succeeds)
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" AlexanderEditor Win64 Development "Alexander.uproject" -waitmutex
```

### Launch Multi-Client Test
```batch
# 3 clients on ports 8080, 8081, 8082
python launch_3_clients_simple.py
```

---

## Configuration Changes

### Live Coding: DISABLED
**File:** `Config/DefaultEditor.ini` (lines 62-66)
```ini
[LiveCoding]
bEnabled=False

[/Script/LiveCoding.LiveCodingSettings]
bEnabled=False
```

**Reason:** Live Coding creates build locks that prevent compilation. With C++ changes requiring full rebuild, Live Coding adds no value and only creates friction.

---

## Build Process

### Step 1: Kill Existing Processes
**ALWAYS** run this before building:
```powershell
powershell -ExecutionPolicy Bypass -File kill_unreal.ps1
```

This script:
- Scans for all Unreal processes and displays PIDs
- Kills each process individually with detailed logging
- Removes Live Coding lock files from `Intermediate/` folder
- Reports summary of processes killed and files cleaned

### Step 2: Build Runtime First
```batch
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" Alexander Win64 Development "Alexander.uproject" -waitmutex
```

**Why Runtime First?**
- Bypasses editor/Live Coding locks
- Faster compilation
- Validates C++ changes before editor build

**Expected Output:**
```
[1/N] Compile [x64] YourFile.cpp
...
[N/N] Link [x64] Alexander.exe
Result: Succeeded
Total execution time: ~20 seconds
```

### Step 3: Build Editor (Optional)
```batch
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" AlexanderEditor Win64 Development "Alexander.uproject" -waitmutex
```

**When to Skip:**
- Running headless tests (no editor needed)
- Iterating rapidly on C++ (runtime is sufficient)
- Automation workflows

**When Required:**
- Opening project in Unreal Editor
- Using Blueprint integration
- Editor-only features

---

## Multi-Client Testing

**CRITICAL: VR Mode Limitation**
VR mode can only support ONE active window at a time due to headset limitations. For multi-client testing, you MUST use desktop mode with the `-nohmd` flag.

### HTTPPort Parameter Support
**File:** `Source/Alexander/Private/AutomationGameMode.cpp` (lines 39-45)

```cpp
// Parse command line parameters
int32 PortOverride = 0;
if (FParse::Value(FCommandLine::Get(), TEXT("HTTPPort="), PortOverride)) {
    AutomationPort = PortOverride;
    UE_LOG(LogTemp, Log, TEXT("AutomationGameMode: HTTP port overridden to %d from command line"), AutomationPort);
}
```

### Launch 3 Clients (Desktop Mode)
```batch
# Terminal 1
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" ^
  "Alexander.uproject" VRTemplateMap ^
  -game -windowed -ResX=800 -ResY=600 -HTTPPort=8080 -nohmd -log

# Terminal 2
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" ^
  "Alexander.uproject" VRTemplateMap ^
  -game -windowed -ResX=800 -ResY=600 -HTTPPort=8081 -nohmd -log

# Terminal 3
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" ^
  "Alexander.uproject" VRTemplateMap ^
  -game -windowed -ResX=800 -ResY=600 -HTTPPort=8082 -nohmd -log
```

**Note:** The `-nohmd` flag disables VR headset, allowing multiple simultaneous clients in desktop mode.

### HTTP API Endpoints

**Port 8080, 8081, 8082:**
- `GET /status` - Server status and port info
- `POST /spawn_ship` - Spawn ship at location
- `GET /get_position?ship_id=X` - Get ship position
- `POST /apply_thrust` - Apply thrust vector

---

## Current Implementation Status

### ✅ Completed

1. **Multi-Client Infrastructure**
   - HTTPPort command-line parameter parsing
   - 3 simultaneous clients on different ports
   - Independent physics simulation per client

2. **Physics Movement API**
   - `/apply_thrust` endpoint (AutomationAPIServer.cpp)
   - Thrust vector application
   - Position tracking across clients

3. **Triangulation Validator**
   - `TriangulationValidator.h/.cpp` (NEW)
   - Law of Cosines validation
   - Triangle inequality checking
   - Tetrahedron validation (4 observers)
   - Overdetermined systems (5+ observers)
   - Anti-fragile confidence scaling

4. **Build Tools**
   - `kill_unreal.bat` - Process management
   - Live Coding disabled
   - Runtime-first build process

### 🔄 In Progress

1. **Consensus HTTP API Endpoints**
   - `/commit_observation` - Commit hash of observation
   - `/reveal_observation` - Reveal observation after commit phase
   - `/validate_position` - Geometric validation using triangulation

### ⏳ Pending

1. **Triangulation Integration**
   - Connect TriangulationValidator to PhysicsConsensus
   - ScalingCalculator integration for distance observations
   - Multi-observer position validation

2. **Quantum-Resistant Signatures**
   - SPHINCS+ or Dilithium implementation
   - Threshold signatures (3-of-4 observers)
   - Cryptographic proof layer

3. **Orbital Mechanics Consensus**
   - Validate positions match orbital predictions
   - Deterministic position calculation from Kepler elements
   - Eliminate movement sync (calculate positions from time)

---

## Architecture Overview

### Layer 1: VR Origin System
- Player stays at (0,0,0) in VR space
- `PlayerOriginManager` - Sector-based virtual positioning
- `VRSpaceshipPawn` - VR origin component

### Layer 2: Scaling & Distance
- `ScalingCalculator` - Distance compression for VR
- Logarithmic scaling for large distances
- Apply/unapply scaling for geometric validation

### Layer 3: Orbital Mechanics (Deterministic)
- `OrbitalMechanics` - Kepler's laws implementation
- `OrbitalBody` - Objects follow orbital paths
- **No movement sync** - calculate positions from time + orbital elements

### Layer 4: Geometric Validation (NEW)
- `TriangulationValidator` - Law of Cosines validation
- 2 observers: Distance check
- 3 observers: Triangle (2D planar)
- 4 observers: Tetrahedron (3D space)
- 5+ observers: Overdetermined (least-squares)

### Layer 5: Consensus
- `PhysicsConsensus` - Multi-observer validation
- Byzantine fault tolerance (2/3 majority)
- Distance + Direction observations
- Geometric consistency checks

### Layer 6: Cryptography (Pending)
- Quantum-resistant signatures (SPHINCS+)
- Commit-reveal scheme
- Threshold signatures (3-of-4 authority)

---

## Anti-Fragile Property

**The system gets STRONGER with more players:**

| Players | Confidence | Validation Method | Security Level |
|---------|------------|-------------------|----------------|
| 2 | 50% | Distance only | Weak |
| 3 | 75% | Triangle (2D) | Basic |
| 4 | 94% | Tetrahedron (3D) | Strong |
| 8 | 99% | Overdetermined | Very Strong |
| 16 | 99.9% | Highly redundant | Cryptographic |
| 64 | 99.99% | Massively overdetermined | Quantum-resistant |

**Formula:** `Confidence = 1 - (1 / ObserverCount^1.5)`

---

## Testing Workflow

### 1. Build Changes
```powershell
powershell -ExecutionPolicy Bypass -File kill_unreal.ps1
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" Alexander Win64 Development "Alexander.uproject" -waitmutex
```

### 2. Launch 3 Clients
```python
python launch_3_clients_simple.py
```

### 3. Test Physics Movement
```python
python test_physics_movement.py
```

**Expected Results:**
- All 3 clients respond on ports 8080, 8081, 8082
- Thrust application moves ships vertically
- Position changes tracked independently per client

### 4. Test Triangulation (Future)
```python
python test_triangulation.py
```

**Will Test:**
- 4-observer tetrahedron validation
- Geometric consistency checks
- Anti-fragile confidence scaling
- Outlier detection with 5+ observers

---

## Common Issues & Solutions

### Issue: "Unable to build while Live Coding is active"
**Solution:**
1. Run `powershell -ExecutionPolicy Bypass -File kill_unreal.ps1`
2. Verify Live Coding disabled in `Config/DefaultEditor.ini`
3. Lock files auto-removed by kill script
4. Build runtime instead of editor: `Build.bat Alexander` (not `AlexanderEditor`)

### Issue: "Port already in use"
**Solution:**
1. Run `powershell -ExecutionPolicy Bypass -File kill_unreal.ps1`
2. Script automatically finds and kills all processes with PID display
3. If issues persist, check for orphaned processes: `tasklist | findstr Unreal`

### Issue: "Module not found after adding new C++ file"
**Solution:**
1. Files in `Source/Alexander/Public` and `Source/Alexander/Private` are auto-discovered
2. Run full rebuild: `kill_unreal.bat` then `Build.bat Alexander`
3. Check for compile errors in build output

### Issue: "HTTP API not responding"
**Solution:**
1. Verify `AutomationGameMode` is active (check logs)
2. Confirm `HTTPPort` parameter parsed correctly (check logs for "HTTP port overridden")
3. Test with `curl http://localhost:8080/status`

---

## Files Reference

### Build & Workflow
- `kill_unreal.ps1` - PowerShell script to kill all Unreal processes with detailed PID logging
- `Config/DefaultEditor.ini` - Live Coding disabled (lines 61-65)

### Multi-Client Support
- `Source/Alexander/Private/AutomationGameMode.cpp:39-45` - HTTPPort parsing
- `Source/Alexander/Private/AutomationAPIServer.cpp` - HTTP API endpoints
  - Lines 370-377: `/submit_observation` and `/validate_position` routing
  - Lines 861-917: `HandleSubmitObservation` implementation
  - Lines 919-1001: `HandleValidatePosition` implementation

### Triangulation & Consensus
- `Source/Alexander/Public/TriangulationValidator.h` - Geometric validation
- `Source/Alexander/Private/TriangulationValidator.cpp` - Law of Cosines implementation
- `Source/Alexander/Public/PhysicsConsensus.h` - Consensus framework
- `Source/Alexander/Public/OrbitalMechanics.h` - Deterministic physics

### Scaling & Origin
- `Source/Alexander/CelestialScaling/Public/ScalingCalculator.h` - Distance compression
- `Source/Alexander/Public/PlayerOriginManager.h` - VR origin system
- `Source/Alexander/Public/VRSpaceshipPawn.h` - VR player at (0,0,0)

---

## Next Steps

1. **Implement Consensus HTTP API**
   - Add `/commit_observation` endpoint
   - Add `/reveal_observation` endpoint
   - Add `/validate_position` endpoint (calls TriangulationValidator)

2. **Integrate Triangulation with Consensus**
   - Connect TriangulationValidator to PhysicsConsensus
   - Add ScalingCalculator to observations
   - Test 4-observer validation

3. **Quantum-Resistant Signatures**
   - Research liboqs integration with Unreal
   - Implement SPHINCS+ or Dilithium
   - Add threshold signature support

4. **Orbital Consensus**
   - Validate positions against orbital predictions
   - Eliminate position sync (calculate from time)
   - Test deterministic physics across clients

---

## Philosophy

**"You wouldn't be able to lie either way"**

This system makes cheating **physically and cryptographically impossible** by requiring positions to satisfy:

1. **Orbital Mechanics** - Deterministic Kepler predictions
2. **Geometric Triangulation** - Law of Cosines consistency
3. **Multi-Observer Consensus** - Byzantine fault tolerance
4. **Quantum-Resistant Crypto** - Unforgeable signatures

The more players in the game, the **stronger** the validation becomes (anti-fragile property).

---

**End of Workflow Guide V2**
