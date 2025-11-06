# BP_SolarSystemGameMode - Game Mode Specification

## Overview
Custom Game Mode for Solar System testing that initializes all celestial scaling subsystems and configures gameplay parameters.

---

## Blueprint Setup

**Blueprint Location:** `Content/Blueprints/GameModes/BP_SolarSystemGameMode`

**Parent Class:** `AGameModeBase` (or `AFirstMissionGameMode` if you want mission integration)

---

## Class Defaults Configuration

### Game Mode Settings
```
Default Pawn Class: BP_VRSpaceshipPlayer
HUD Class: None (will add UI widgets manually)
Player Controller Class: PlayerController (default)
Game State Class: GameStateBase (default)
Player State Class: PlayerState (default)
Spectator Class: SpectatorPawn (default)
```

---

## Variables

### Configuration Variables
```cpp
// Celestial Scaling Configuration
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial Scaling")
bool bEnableCelestialScaling = true;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial Scaling")
bool bEnableOriginRecentering = true;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial Scaling")
float UniverseRecenterThreshold = 1000000.0; // 10 km in cm

// Gravity Simulation Configuration
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
TEnumAsByte<EGravitySimulationMode> GravityMode = EGravitySimulationMode::MultiBody;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
bool bEnableGravity = true;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
float MaxGForce = 50.0;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
float GravityUpdateFrequency = 60.0; // Hz

// Relativity System Configuration
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relativity")
bool bEnableRelativisticEffects = true;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relativity")
bool bEnableTimeDebt = true;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relativity")
float TimeWarpWarningThreshold = 10.0; // Warn when warp > 10x

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relativity")
float RelativityScaleFactor = 1.0; // 1.0 = realistic, >1.0 = dramatic

// Debug Configuration
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
bool bShowDebugOverlay = true;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
bool bEnableDebugVisualization = false;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
bool bLogSubsystemInitialization = true;
```

### Runtime Variables (Not Editable)
```cpp
// Subsystem References
UPROPERTY(BlueprintReadOnly, Category = "Subsystems")
UCelestialBodyRegistry* CelestialBodyRegistry;

UPROPERTY(BlueprintReadOnly, Category = "Subsystems")
UGravitySimulator* GravitySimulator;

UPROPERTY(BlueprintReadOnly, Category = "Subsystems")
URelativitySystem* RelativitySystem;

UPROPERTY(BlueprintReadOnly, Category = "Subsystems")
UScalingCalculator* ScalingCalculator;

UPROPERTY(BlueprintReadOnly, Category = "Subsystems")
UCelestialScalingSafetySystem* SafetySystem;

// Player References
UPROPERTY(BlueprintReadOnly, Category = "Players")
AVRSpaceshipPawn* LocalPlayerPawn;

UPROPERTY(BlueprintReadOnly, Category = "Players")
APlayerController* LocalPlayerController;
```

---

## Blueprint Graph Functions

### Event BeginPlay
```
Purpose: Initialize all subsystems and configure celestial scaling system

Blueprint Graph:
1. Call Parent BeginPlay

2. Log Initialization Start (if bLogSubsystemInitialization):
   - Print String: "=== Solar System Game Mode Initializing ==="

3. Get Game Instance:
   - Node: Get Game Instance
   - Cast to Game Instance

4. Initialize CelestialBodyRegistry (WorldSubsystem):
   - Node: Get World Subsystem
   - Class: UCelestialBodyRegistry
   - Store in variable: CelestialBodyRegistry
   - Log: "CelestialBodyRegistry initialized"

5. Initialize GravitySimulator (GameInstanceSubsystem):
   - From Game Instance → Get Subsystem
   - Class: UGravitySimulator
   - Store in variable: GravitySimulator
   - Configure:
     - Call: SetSimulationMode(GravityMode)
     - Call: SetMaxGForce(MaxGForce)
     - Call: SetGravityEnabled(bEnableGravity)
     - Call: SetUpdateFrequency(GravityUpdateFrequency)
   - Log: "GravitySimulator initialized: Mode=[GravityMode], MaxG=[MaxGForce]"

6. Initialize RelativitySystem (GameInstanceSubsystem):
   - From Game Instance → Get Subsystem
   - Class: URelativitySystem
   - Store in variable: RelativitySystem
   - Configure:
     - Call: SetRelativisticEffectsEnabled(bEnableRelativisticEffects)
     - Call: SetTimeDebtEnabled(bEnableTimeDebt)
     - Call: SetRelativityScaleFactor(RelativityScaleFactor)
   - Log: "RelativitySystem initialized: Enabled=[bEnableRelativisticEffects]"

7. Initialize ScalingCalculator (GameInstanceSubsystem):
   - From Game Instance → Get Subsystem
   - Class: UScalingCalculator
   - Store in variable: ScalingCalculator
   - Log: "ScalingCalculator initialized"

8. Initialize SafetySystem (GameInstanceSubsystem):
   - From Game Instance → Get Subsystem
   - Class: UCelestialScalingSafetySystem
   - Store in variable: SafetySystem
   - Log: "SafetySystem initialized"

9. Register Local Player with RelativitySystem:
   - Delay: 0.5 seconds (wait for player to spawn)
   - Get Player Controller: Index 0
   - Store in: LocalPlayerController
   - Get Controlled Pawn → Cast to AVRSpaceshipPawn
   - Store in: LocalPlayerPawn
   - If valid:
     - Call: RelativitySystem->RegisterPlayer(LocalPlayerController, 25.0) // Starting age
     - Log: "Player registered with RelativitySystem"

10. Configure Debug Visualization:
    - If bEnableDebugVisualization:
      - Call: GravitySimulator->SetDebugVisualizationEnabled(true)
      - Call: CelestialBodyRegistry->SetDebugDrawEnabled(true)
      - Log: "Debug visualization enabled"

11. Create and Add Debug UI (if bShowDebugOverlay):
    - Create Widget: WBP_DebugOverlay
    - Add to Viewport
    - Set Z-Order: 100 (top layer)
    - Log: "Debug overlay created"

12. Create and Add Chronometer UI:
    - Create Widget: WBP_Chronometer
    - Add to Viewport
    - Set Z-Order: 99
    - Log: "Chronometer widget created"

13. Log Initialization Complete:
    - Print String: "=== Solar System Game Mode Ready ==="
    - Print String: "Registered Bodies: [Count from CelestialBodyRegistry]"
```

### Event Tick (Optional - for debugging)
```
Purpose: Monitor system status

Blueprint Graph (only if bShowDebugOverlay):
1. Get CelestialBodyRegistry
2. Get registered body count
3. Get player position from LocalPlayerPawn->OriginManager
4. Get current gravity forces from GravitySimulator
5. Update debug display (can be done in widget instead)
```

### Function: OnPlayerSpawned (Custom Event)
```
Parameters:
- NewPlayer (APlayerController): The player that spawned

Purpose: Configure newly spawned player

Blueprint Graph:
1. Get Controlled Pawn
2. Cast to AVRSpaceshipPawn
3. If valid:
   - Get OriginManager component
   - Configure:
     - SetPrecisionThreshold(UniverseRecenterThreshold)
     - SetAutoRecenterEnabled(bEnableOriginRecentering)
   - Get PlayerTimeComponent
   - If valid:
     - Register with RelativitySystem
   - Log: "Player configured: [Player Name]"
```

### Function: ToggleDebugVisualization (Blueprint Callable)
```
Purpose: Toggle debug visualization at runtime (bind to console command)

Blueprint Graph:
1. Toggle bEnableDebugVisualization (NOT operation)
2. Call: GravitySimulator->SetDebugVisualizationEnabled(bEnableDebugVisualization)
3. Call: CelestialBodyRegistry->SetDebugDrawEnabled(bEnableDebugVisualization)
4. Log: "Debug visualization: [On/Off]"
```

### Function: SetTimeWarpFactor (Blueprint Callable)
```
Parameters:
- WarpFactor (Float): The time warp multiplier

Purpose: Activate time warp with warning check

Blueprint Graph:
1. If WarpFactor > TimeWarpWarningThreshold:
   - Create Widget: WBP_TimeWarpWarning
   - Set Warp Factor on widget
   - Add to Viewport (modal)
   - Wait for user confirmation
   - If confirmed:
     - Call: RelativitySystem->ApplyTimeWarpFactor(WarpFactor)
     - Log: "Time warp activated: [WarpFactor]x"
   - If cancelled:
     - Log: "Time warp cancelled by user"
2. Else (below threshold):
   - Call: RelativitySystem->ApplyTimeWarpFactor(WarpFactor)
   - Log: "Time warp activated: [WarpFactor]x"
```

### Function: GetSystemStatus (Blueprint Pure)
```
Return Type: String

Purpose: Get formatted system status for UI

Blueprint Graph:
1. Build string:
   - "Bodies Registered: [Count]"
   - "Gravity Mode: [GravityMode]"
   - "Relativity: [Enabled/Disabled]"
   - "Time Warp: [Current Factor]x"
2. Return formatted string
```

---

## Console Commands (Optional - Advanced)

You can bind Blueprint functions to console commands:

```cpp
// In C++ GameMode class (if extending to C++)
UFUNCTION(Exec, Category = "Solar System")
void ToggleDebug();

UFUNCTION(Exec, Category = "Solar System")
void SetTimeWarp(float Factor);

UFUNCTION(Exec, Category = "Solar System")
void ListCelestialBodies();

UFUNCTION(Exec, Category = "Solar System")
void TeleportToBody(FString BodyID);
```

Then in Blueprint, implement these functions to call the subsystems.

---

## Network Replication (Multiplayer)

If supporting multiplayer:

```cpp
// Replication Settings
Replicates: True
Replicate Movement: False

// Override Replication Function
GetLifetimeReplicatedProps:
- Replicate: bEnableGravity
- Replicate: GravityMode
- Replicate: bEnableRelativisticEffects
- Replicate: TimeWarpWarningThreshold
```

### Server RPCs
```cpp
// Activate Time Warp (Server-authoritative)
UFUNCTION(Server, Reliable, WithValidation)
void ServerSetTimeWarp(float Factor);

// Implementation:
1. Validate Factor (0.1 to 10000.0 range)
2. Call RelativitySystem->ApplyTimeWarpFactor(Factor)
3. Multicast to all clients: ClientUpdateTimeWarp(Factor)
```

---

## Default Values Recommended

For initial testing:

```
bEnableCelestialScaling: true
bEnableOriginRecentering: true
UniverseRecenterThreshold: 1000000.0 (10 km)
GravityMode: MultiBody
bEnableGravity: true
MaxGForce: 50.0
GravityUpdateFrequency: 60.0
bEnableRelativisticEffects: true
bEnableTimeDebt: true
TimeWarpWarningThreshold: 10.0
RelativityScaleFactor: 1.0 (realistic)
bShowDebugOverlay: true (for testing)
bEnableDebugVisualization: false (performance)
bLogSubsystemInitialization: true (for debugging)
```

---

## World Settings Configuration

Once BP_SolarSystemGameMode is created:

1. Open SolarSystem.umap
2. Open World Settings panel (Window → World Settings)
3. Game Mode section:
   - Game Mode Override: BP_SolarSystemGameMode
   - Default Pawn Class: Should auto-populate to BP_VRSpaceshipPlayer
4. Save level

---

## Testing Checklist

After creating BP_SolarSystemGameMode:

- [ ] Blueprint compiles without errors
- [ ] Set as Game Mode Override in World Settings
- [ ] BeginPlay executes (check Output Log)
- [ ] All subsystems initialize successfully
- [ ] CelestialBodyRegistry shows registered bodies count
- [ ] GravitySimulator activates
- [ ] RelativitySystem tracks player time
- [ ] Debug overlay displays (if enabled)
- [ ] Chronometer shows three time clocks
- [ ] Player spawns correctly

---

## Common Issues & Solutions

**Issue: Subsystems not found**
- Solution: Ensure C++ code is compiled
- Verify module dependencies in Alexander.Build.cs
- Check subsystem classes are UGameInstanceSubsystem or UWorldSubsystem

**Issue: Player not registered with RelativitySystem**
- Solution: Check delay (0.5s) is sufficient for player spawn
- Verify LocalPlayerPawn cast succeeds
- Ensure PlayerTimeComponent exists on pawn

**Issue: Chronometer doesn't update**
- Solution: Verify widget is added to viewport
- Check widget references RelativitySystem correctly
- Ensure bAutoUpdate is true in widget

**Issue: Gravity doesn't work**
- Solution: Verify bEnableGravity = true
- Check GravityMode is not Disabled
- Ensure celestial bodies have CelestialBodyComponent
- Verify MaxGForce is reasonable (not 0)

**Issue: Debug overlay missing**
- Solution: Verify WBP_DebugOverlay exists
- Check bShowDebugOverlay = true
- Ensure widget compiles without errors

---

## Performance Tuning

If experiencing performance issues:

```
GravityUpdateFrequency: 60 → 30 Hz (halve update rate)
GravityMode: NBody → MultiBody (simplified calculations)
bEnableDebugVisualization: true → false (disable debug draw)
MaxGForce: 50 → 20 (reduce force calculations)
```

For very distant bodies, consider:
- Increasing influence radius thresholds
- Using SingleBody mode when only one body is relevant
- Reducing update frequency for far bodies

---

## Advanced: Mission Integration

If using AFirstMissionGameMode as parent:

```cpp
// Override mission-specific functions
BeginPlay:
1. Call Parent BeginPlay (initializes mission system)
2. Initialize celestial scaling (as above)
3. Register mission objectives that use time debt

OnMissionStageChanged:
- Adjust time warp availability based on mission stage
- Lock/unlock certain celestial bodies
- Modify gravity simulation for tutorial phases
```

---

**Created:** 2025-01-05
**Version:** 1.0
