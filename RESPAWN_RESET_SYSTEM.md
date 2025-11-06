# Respawn and Reset Mechanics Implementation

## Overview

Complete respawn system allowing players to retry course segments without restarting entire level. Includes checkpoint system, quick respawn, and course reset.

## System Components

### 1. Checkpoint System

**Checkpoint Actor** (BP_FlightCheckpoint):
```blueprint
Variables:
    CheckpointID: int (1 for WP3)
    SpawnLocation: Vector
    SpawnRotation: Rotator
    bActivated: bool = false

Components:
    TriggerVolume: BoxComponent (large, easy to pass through)
    VisualMesh: StaticMesh (optional checkpoint marker)
    ParticleEffect: Checkpoint activation effect

Event OnOverlapBegin(Other Actor):
    If Other is Player Pawn AND not bActivated:
        bActivated = true
        Get BP_FlightGameState
        Call GameState.SetLastCheckpoint(CheckpointID, SpawnLocation, SpawnRotation)
        Play activation sound
        Show checkpoint UI notification
```

**Checkpoint Placement**:
- Checkpoint 1: At WP3 (3000, 0, 1200) - Halfway point
- Optional Checkpoint 2: At WP4 for Hard mode

### 2. Player Respawn System

**BP_FlightPlayerPawn additions**:
```blueprint
Variables:
    LastCheckpointLocation: Vector = StartLocation
    LastCheckpointRotation: Rotator = StartRotation
    RespawnInvulnerabilityTime: float = 2.0 seconds

Function Respawn():
    // Disable player input briefly
    DisableInput()

    // Fade out
    PlayerController.PlayerCameraManager.StartCameraFade(0, 1, 0.3, Black, true, true)

    // Wait for fade
    Delay 0.3 seconds

    // Reset position
    SetActorLocation(LastCheckpointLocation)
    SetActorRotation(LastCheckpointRotation)

    // Reset velocity
    GetMovementComponent().Velocity = Zero Vector

    // Fade in
    PlayerController.PlayerCameraManager.StartCameraFade(1, 0, 0.5, Black, false, false)

    // Re-enable input
    Delay 0.5 seconds
    EnableInput()

    // Grant brief invulnerability (if collision damage enabled)
    bInvulnerable = true
    Delay RespawnInvulnerabilityTime
    bInvulnerable = false

    // Log respawn
    UE_LOG("Player respawned at checkpoint")
```

**Input Binding**:
```cpp
// In PlayerController or Pawn
void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    PlayerInputComponent->BindAction("Respawn", IE_Pressed, this, &AFlightPlayerPawn::RequestRespawn);
    PlayerInputComponent->BindAction("ResetCourse", IE_Pressed, this, &AFlightPlayerPawn::RequestCourseReset);
}

void AFlightPlayerPawn::RequestRespawn()
{
    Respawn();
}

void AFlightPlayerPawn::RequestCourseReset()
{
    // Show confirmation dialog first
    ShowResetConfirmationDialog();
}
```

### 3. Course Reset System

**BP_FlightGameState additions**:
```blueprint
Function ResetCourse():
    // Reset progress
    WaypointsCollected = 0
    RingsPassed = 0
    TimeElapsed = 0.0
    Score = 0
    CourseComplete = false

    // Reset checkpoints
    LastCheckpointLocation = CourseStartLocation
    LastCheckpointRotation = CourseStartRotation

    // Reset all waypoints (re-enable if collected)
    All Waypoint Actors.SetActorHiddenInGame(false)
    All Waypoint Actors.SetCollisionEnabled(true)

    // Reset all rings
    All Ring Actors.ResetState()

    // Reset player position
    Player Pawn.SetActorLocation(CourseStartLocation)
    Player Pawn.SetActorRotation(CourseStartRotation)
    Player Pawn.Velocity = Zero

    // Update HUD
    FlightHUDManager.RefreshDisplay()

    // Log reset
    UE_LOG("Course reset to start")
```

### 4. UI Integration

**Reset Confirmation Dialog** (WBP_ResetConfirmation):
```
Overlay
├─ Background (darkened, blurred)
├─ Dialog Box
│   ├─ Title: "RESET COURSE?"
│   ├─ Message: "All progress will be lost"
│   ├─ Button_Confirm: "YES, RESET"
│   │   OnClicked -> BP_FlightGameState.ResetCourse() -> Close Dialog
│   └─ Button_Cancel: "CANCEL"
│       OnClicked -> Close Dialog
```

**Respawn Notification** (WBP_RespawnNotification):
```
Canvas Panel (Centered)
├─ Text_Message: "RESPAWNING..."
└─ Progress Bar (brief animation)

// Auto-hide after respawn completes
```

**Checkpoint Notification** (WBP_CheckpointActivated):
```
Canvas Panel (Top-center)
├─ Icon_Checkpoint (green checkmark)
├─ Text_Message: "CHECKPOINT REACHED"
└─ Text_Subtext: "Press R to respawn here"

// Fade out after 3 seconds
```

## Respawn Strategies

### Strategy 1: Instant Respawn (Practice Mode)
- No penalty
- Unlimited respawns
- Fastest iteration for learning
- Best for beginners

### Strategy 2: Limited Attempts (Challenge Mode)
```blueprint
BP_FlightGameState additions:
    Variables:
        MaxRespawnAttempts: int = 3
        RemainingRespawns: int = 3
        bUnlimitedRespawns: bool = true // Toggle for mode

    Function Respawn():
        If not bUnlimitedRespawns:
            If RemainingRespawns <= 0:
                ShowGameOverScreen()
                return false

            RemainingRespawns--
            UpdateRespawnCounterUI()

        // Proceed with respawn
        return true
```

### Strategy 3: Time Penalty (Competitive Mode)
```blueprint
Function Respawn():
    // Add time penalty
    TimeElapsed += TimePenalty // e.g., +10 seconds

    // Show penalty notification
    Show "TIME PENALTY: +10s"

    // Proceed with respawn
```

## Fail Conditions

### Optional: Out of Bounds Detection
```blueprint
BP_CourseBoundary (trigger volume around course):
    Event OnOverlapBegin(Other Actor):
        If Other is Player Pawn:
            Delay 2 seconds // Grace period
            If still outside bounds:
                Player.Respawn()
                Show "OUT OF BOUNDS"
```

### Optional: Collision Damage
```cpp
void AFlightPlayerPawn::TakeDamage(...)
{
    Health -= Damage;

    if (Health <= 0)
    {
        // Respawn instead of destroying pawn
        Respawn();
    }
}
```

## Input Mapping

### Project Settings -> Input

**Respawn Action**:
- Name: "Respawn"
- Key: R
- Description: "Quick respawn at last checkpoint"

**Reset Course Action**:
- Name: "ResetCourse"
- Key: Escape (or Backspace)
- Description: "Reset entire course"

**Pause Menu** (contains Reset option):
- Key: Escape
- Shows menu with "Resume", "Reset Course", "Change Difficulty", "Quit"

## Testing Checklist

- [ ] Checkpoint triggers correctly at WP3
- [ ] Respawn places player at correct location
- [ ] Respawn resets velocity to zero
- [ ] Course reset clears all progress
- [ ] Course reset re-enables waypoints/rings
- [ ] UI notifications display correctly
- [ ] No input allowed during respawn fade
- [ ] Multiple respawns work correctly
- [ ] Limited attempts mode functions (if implemented)
- [ ] Time penalty applies correctly (if implemented)

## Edge Cases

### Player Respawns During Respawn
**Solution**: Disable respawn input during respawn animation
```blueprint
Variable: bCurrentlyRespawning = false

Function Respawn():
    If bCurrentlyRespawning:
        return // Ignore

    bCurrentlyRespawning = true
    // ... respawn logic ...
    bCurrentlyRespawning = false
```

### Course Reset During Active Gameplay
**Solution**: Require confirmation dialog
```blueprint
Function RequestCourseReset():
    If CourseActive:
        ShowResetConfirmationDialog()
    Else:
        ResetCourse() // Direct reset if not started
```

### Checkpoint Missed
**Solution**: Make trigger volumes large and forgiving
- Checkpoint trigger: 1000 unit radius sphere
- Positioned directly in optimal path

## Performance

**Respawn Cost**: <1ms (teleport + fade)
**Reset Cost**: ~5ms (iterate all course objects)
**Memory**: Negligible (few vector/rotator variables)
**No Runtime Overhead**: Only on respawn/reset events

## Implementation Priority

**High**:
1. Basic respawn at last checkpoint
2. Course reset functionality
3. Input bindings

**Medium**:
4. Checkpoint system with notification
5. Fade in/out effects
6. Reset confirmation dialog

**Low**:
7. Limited attempts mode
8. Time penalty system
9. Out of bounds detection

## Status
**Design**: COMPLETE
**Core System**: Documented
**Optional Features**: Specified
**Ready for**: Implementation
