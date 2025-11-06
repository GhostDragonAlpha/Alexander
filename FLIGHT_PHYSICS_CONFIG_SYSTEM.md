# Flight Physics Configuration System - Implementation Guide

## Overview

This system enables runtime configuration of all flight physics parameters without C++ recompilation. Designers can create multiple difficulty presets and switch between them dynamically.

## Architecture

### Component 1: UFlightPhysicsConfig (Data Asset)
**File**: FlightPhysicsConfig.h
**Type**: UDataAsset subclass
**Purpose**: Store all configurable physics parameters

### Component 2: Configuration Loading (FlightController)
**Files**: FlightController.h/.cpp (modifications)
**Functions**: LoadPhysicsConfig(), ApplyPhysicsConfig()
**Purpose**: Apply configuration at runtime

### Component 3: Preset Assets
**Location**: /Game/Config/FlightPhysics/
**Files**:
- DA_FlightPhysics_Easy.uasset
- DA_FlightPhysics_Medium.uasset
- DA_FlightPhysics_Hard.uasset

## Implementation Steps

### Step 1: Create UFlightPhysicsConfig Class

**Create file**: Source/Alexander/Public/FlightPhysicsConfig.h

```cpp
// FlightPhysicsConfig.h
// Data asset for configurable flight physics parameters

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FlightController.h"
#include "FlightPhysicsConfig.generated.h"

/**
 * Flight Physics Configuration Data Asset
 * Allows runtime configuration of flight parameters without code recompilation
 */
UCLASS(BlueprintType)
class ALEXANDER_API UFlightPhysicsConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    // ============================================================================
    // VELOCITY CONFIGURATION
    // ============================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velocity")
    float MaxSafeVelocity = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velocity")
    float ThrustReductionFactor = 0.1f;

    // ============================================================================
    // INPUT SMOOTHING
    // ============================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Smoothing")
    float ThrottleSmoothing = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Smoothing")
    float RotationSmoothing = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Smoothing")
    float TranslationSmoothing = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Smoothing")
    float Deadzone = 0.05f;

    // ============================================================================
    // FLIGHT ASSISTANCE
    // ============================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight Assist")
    EFlightAssistMode DefaultAssistMode = EFlightAssistMode::Stability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight Assist")
    bool bCollisionAvoidance = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight Assist")
    bool bThrustLimiting = true;

    // ============================================================================
    // ASSIST MODE PARAMETERS
    // ============================================================================

    // Stability mode damping factor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assist Parameters")
    float StabilityDamping = 0.95f;

    // Stability mode rotation correction multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assist Parameters")
    float StabilityCorrection = 0.1f;

    // AutoLevel correction strength
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assist Parameters")
    float AutoLevelStrength = 0.5f;

    // AutoLevel thrust reduction during maneuvers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assist Parameters")
    float AutoLevelThrustReduction = 0.7f;

    // Orbital assist prograde boost
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assist Parameters")
    float OrbitalProgradeBoost = 1.2f;

    // Docking mode thrust scale
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assist Parameters")
    float DockingThrustScale = 0.3f;

    // Docking mode rotation scale
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assist Parameters")
    float DockingRotationScale = 0.5f;

    // ============================================================================
    // COLLISION DETECTION
    // ============================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float CollisionDetectionRange = 500.0f;

    // ============================================================================
    // METADATA
    // ============================================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
    FString ConfigName = "Default";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
    FString Description = "Default flight physics configuration";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
    int32 DifficultyLevel = 2; // 1=Easy, 2=Medium, 3=Hard
};

```

**Create file**: Source/Alexander/Private/FlightPhysicsConfig.cpp

```cpp
#include "FlightPhysicsConfig.h"

// Default constructor sets baseline values
// No additional implementation needed - all UPROPERTY defaults handle initialization
```

### Step 2: Modify FlightController

**Add to FlightController.h** (Public section):

```cpp
// FlightController.h additions
// Add to UFlightController class

public:
    // Physics configuration asset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Config")
    UFlightPhysicsConfig* PhysicsConfig;

    // Load configuration at runtime
    UFUNCTION(BlueprintCallable, Category = "Physics Config")
    void LoadPhysicsConfig(UFlightPhysicsConfig* NewConfig);

    // Apply configuration to current settings
    UFUNCTION(BlueprintCallable, Category = "Physics Config")
    void ApplyPhysicsConfig();



```

**Add to FlightController.cpp**:

```cpp
 additions
// Add to UFlightController implementation

void UFlightController::LoadPhysicsConfig(UFlightPhysicsConfig* NewConfig)
{
    if (!NewConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("LoadPhysicsConfig: Invalid config asset"));
        return;
    }

    PhysicsConfig = NewConfig;
    ApplyPhysicsConfig();

    UE_LOG(LogTemp, Log, TEXT("Loaded physics config: %s"), *NewConfig->ConfigName);
}

void UFlightController::ApplyPhysicsConfig()
{
    if (!PhysicsConfig)
    {
        UE_LOG(LogTemp, Warning, TEXT("ApplyPhysicsConfig: No config loaded"));
        return;
    }

    // Apply velocity settings
    MaxSafeVelocity = PhysicsConfig->MaxSafeVelocity;

    // Apply input smoothing
    SmoothingConfig.ThrottleSmoothing = PhysicsConfig->ThrottleSmoothing;
    SmoothingConfig.RotationSmoothing = PhysicsConfig->RotationSmoothing;
    SmoothingConfig.TranslationSmoothing = PhysicsConfig->TranslationSmoothing;
    SmoothingConfig.Deadzone = PhysicsConfig->Deadzone;

    // Apply assist settings
    AssistMode = PhysicsConfig->DefaultAssistMode;
    bCollisionAvoidance = PhysicsConfig->bCollisionAvoidance;
    bThrustLimiting = PhysicsConfig->bThrustLimiting;

    // Apply collision detection range
    if (CollisionDetectionBox)
    {
        FVector NewExtent(
            PhysicsConfig->CollisionDetectionRange,
            PhysicsConfig->CollisionDetectionRange,
            PhysicsConfig->CollisionDetectionRange
        );
        CollisionDetectionBox->SetBoxExtent(NewExtent);
    }

    UE_LOG(LogTemp, Log, TEXT("Applied physics config: MaxVel=%f, Assist=%s"),
        MaxSafeVelocity,
        *UEnum::GetValueAsString(AssistMode));
}

// Also need to update assist functions to use config values
// Replace hardcoded values with PhysicsConfig->ParameterName

```

### Step 3: Update Assist Functions

Modify assist mode functions to use config values instead of hardcoded constants:

**Before**:
```cpp
void UFlightController::ApplyStabilityAssistance(FVector& OutThrust, FVector& OutRotation, float DeltaTime)
{
    float DampingFactor = 0.95f;  // Hardcoded
    OutThrust *= DampingFactor;
    // ...
}
```

**After**:
```cpp
void UFlightController::ApplyStabilityAssistance(FVector& OutThrust, FVector& OutRotation, float DeltaTime)
{
    if (!PhysicsConfig) return;

    float DampingFactor = PhysicsConfig->StabilityDamping;  // From config
    OutThrust *= DampingFactor;
    // ...
}
```

Apply same pattern to all assist functions for their respective parameters.

### Step 4: Compile C++ Code

```bash
# In Unreal Editor
# 1. Close Unreal Editor
# 2. Right-click Alexander.uproject -> Generate Visual Studio project files
# 3. Open Alexander.sln in Visual Studio
# 4. Build Solution (Ctrl+Shift+B)
# 5. Reopen Unreal Editor
```

### Step 5: Create Preset Data Assets

In Unreal Editor:

1. Content Browser -> /Game/Config/FlightPhysics/ (create folders if needed)
2. Right-click -> Miscellaneous -> Data Asset
3. Select UFlightPhysicsConfig as parent class
4. Name: DA_FlightPhysics_Easy
5. Open asset, configure parameters:

**Easy Mode Settings**:

### Easy Mode Preset

```
ConfigName: Easy Mode
Description: Beginner-friendly flight with strong assistance
DifficultyLevel: 1
MaxSafeVelocity: 700.0
ThrustReductionFactor: 0.1
ThrottleSmoothing: 0.15
RotationSmoothing: 0.2
TranslationSmoothing: 0.15
Deadzone: 0.05
DefaultAssistMode: AutoLevel
bCollisionAvoidance: True
bThrustLimiting: True
StabilityDamping: 0.97
StabilityCorrection: 0.15
AutoLevelStrength: 0.7
AutoLevelThrustReduction: 0.7
OrbitalProgradeBoost: 1.2
DockingThrustScale: 0.3
DockingRotationScale: 0.5
CollisionDetectionRange: 600.0
```

### Medium Mode Preset

```
ConfigName: Medium Mode
Description: Balanced flight for intermediate players
DifficultyLevel: 2
MaxSafeVelocity: 1000.0
ThrustReductionFactor: 0.1
ThrottleSmoothing: 0.1
RotationSmoothing: 0.15
TranslationSmoothing: 0.1
Deadzone: 0.05
DefaultAssistMode: Stability
bCollisionAvoidance: True
bThrustLimiting: True
StabilityDamping: 0.95
StabilityCorrection: 0.1
AutoLevelStrength: 0.5
AutoLevelThrustReduction: 0.7
OrbitalProgradeBoost: 1.2
DockingThrustScale: 0.3
DockingRotationScale: 0.5
CollisionDetectionRange: 500.0
```

### Hard Mode Preset

```
ConfigName: Hard Mode
Description: Expert flight with minimal assistance
DifficultyLevel: 3
MaxSafeVelocity: 1500.0
ThrustReductionFactor: 0.05
ThrottleSmoothing: 0.06
RotationSmoothing: 0.1
TranslationSmoothing: 0.06
Deadzone: 0.03
DefaultAssistMode: None
bCollisionAvoidance: False
bThrustLimiting: False
StabilityDamping: 0.92
StabilityCorrection: 0.05
AutoLevelStrength: 0.3
AutoLevelThrustReduction: 0.8
OrbitalProgradeBoost: 1.1
DockingThrustScale: 0.3
DockingRotationScale: 0.5
CollisionDetectionRange: 400.0
```


### Step 6: Test Configuration Loading

**Blueprint Test** (in Level Blueprint or GameMode):

```blueprint
Event BeginPlay:
    Get Player Pawn
    Get Component by Class: FlightController

    Load Asset: DA_FlightPhysics_Easy
    Call LoadPhysicsConfig (pass loaded asset)

    Print String: "Loaded Easy mode physics"
```

**C++ Test**:
```cpp
// In GameMode or PlayerController BeginPlay
UFlightController* FlightComp = PlayerPawn->FindComponentByClass<UFlightController>();
if (FlightComp)
{
    UFlightPhysicsConfig* EasyConfig = LoadObject<UFlightPhysicsConfig>(
        nullptr,
        TEXT("/Game/Config/FlightPhysics/DA_FlightPhysics_Easy.DA_FlightPhysics_Easy")
    );
    FlightComp->LoadPhysicsConfig(EasyConfig);
}
```

## Integration with Flight Course

### Option 1: Difficulty Selection Menu

Create UI widget for difficulty selection:

```blueprint
WBP_DifficultySelect:
    Button_Easy:
        OnClicked -> Load DA_FlightPhysics_Easy -> Apply to FlightController

    Button_Medium:
        OnClicked -> Load DA_FlightPhysics_Medium -> Apply to FlightController

    Button_Hard:
        OnClicked -> Load DA_FlightPhysics_Hard -> Apply to FlightController
```

### Option 2: Game Mode Configuration

```cpp
// In FlightCourseGameMode.h
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Course Settings")
UFlightPhysicsConfig* CoursePhysicsConfig;

// In BeginPlay
void AFlightCourseGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Apply physics config to all players
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        APawn* PlayerPawn = PC->GetPawn();
        if (PlayerPawn)
        {
            UFlightController* FlightComp = PlayerPawn->FindComponentByClass<UFlightController>();
            if (FlightComp && CoursePhysicsConfig)
            {
                FlightComp->LoadPhysicsConfig(CoursePhysicsConfig);
            }
        }
    }
}
```

### Option 3: BP_FlightGameState Integration

Add difficulty tier to GameState:

```blueprint
BP_FlightGameState:
    Variables:
        CurrentDifficulty: Integer (1=Easy, 2=Medium, 3=Hard)
        PhysicsConfigMap: Map<Integer, UFlightPhysicsConfig*>

    Function SetDifficulty(NewDifficulty):
        CurrentDifficulty = NewDifficulty
        Config = PhysicsConfigMap[NewDifficulty]
        Get all FlightControllers in level
        For each: Call LoadPhysicsConfig(Config)
```

## Testing Procedure

### Test 1: Configuration Loading
- [ ] Create all 3 preset assets
- [ ] Load Easy preset in PIE
- [ ] Verify MaxSafeVelocity = 700
- [ ] Verify AssistMode = AutoLevel
- [ ] Repeat for Medium and Hard

### Test 2: Runtime Switching
- [ ] Start PIE with Medium config
- [ ] Switch to Easy during gameplay
- [ ] Verify controls feel more stable/slower
- [ ] Switch to Hard
- [ ] Verify controls feel more responsive/faster

### Test 3: Parameter Validation
- [ ] Modify Easy preset (e.g., MaxVelocity = 500)
- [ ] Reload in PIE
- [ ] Verify change took effect
- [ ] Reset to original value

### Test 4: Flight Course Integration
- [ ] Complete course on Easy (should be achievable)
- [ ] Complete course on Medium (moderate challenge)
- [ ] Attempt course on Hard (difficult, requires skill)
- [ ] Verify completion times match targets

## Performance Considerations

**Configuration Loading**: ~0.1ms one-time cost
**Runtime Parameter Access**: Negligible (simple property reads)
**Memory**: ~200 bytes per config asset
**No Runtime Overhead**: Same performance as hardcoded values

## Troubleshooting

### Issue: Config not loading
**Check**:
- PhysicsConfig pointer is not null
- Data asset path is correct
- ApplyPhysicsConfig() is being called

### Issue: Parameters not applying
**Check**:
- All assist functions updated to use PhysicsConfig->
- Not mixing hardcoded and config values
- Config loaded before gameplay starts

### Issue: Crash on load
**Check**:
- All UPROPERTY() macros present
- GENERATED_BODY() macro in class
- Header properly included in .cpp

## Benefits

1. **No Recompilation**: Designers tune parameters without programmer
2. **Rapid Iteration**: Test changes in seconds, not minutes
3. **Multiple Presets**: Easy/Medium/Hard without code duplication
4. **Runtime Switching**: Change difficulty mid-gameplay
5. **Data-Driven**: All values in assets, version controlled
6. **Blueprint Accessible**: Non-programmers can create configs
7. **Maintainable**: Single source of truth for each preset

## Next Steps (GT03-GT06)

- GT03: Analyze course waypoint/ring placement optimization
- GT04: Implement difficulty tier selection UI
- GT05: Add respawn/reset mechanics for failed attempts
- GT06: Create debug visualization for tuning

## Status

**Design**: COMPLETE
**C++ Code**: Specified (needs implementation)
**Preset Assets**: Defined (needs creation)
**Integration**: Documented
**Testing**: Procedures defined
