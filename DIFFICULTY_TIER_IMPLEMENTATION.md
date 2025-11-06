# Difficulty Tier System Implementation

## Overview

Complete difficulty tier system with physics configurations, course variants, reward scaling, and UI integration.

## Architecture

### Component 1: Difficulty Enum
```cpp
// In GameTypes.h or similar
UENUM(BlueprintType)
enum class ECourseDifficulty : uint8
{
    Easy UMETA(DisplayName = "Easy"),
    Medium UMETA(DisplayName = "Medium"),
    Hard UMETA(DisplayName = "Hard")
};
```

### Component 2: Course Difficulty Configuration
```cpp
// In FlightCourseConfig.h
UCLASS(BlueprintType)
class UCourseDifficultyConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECourseDifficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UFlightPhysicsConfig* PhysicsConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RingScale = 1.0f; // 1.5 for Easy, 0.7 for Hard

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetCompletionTime = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RewardMultiplier = 1.0f; // 1.0 Easy, 1.5 Medium, 2.0 Hard

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShowRacingLine = true; // true for Easy, false for Hard

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShowDirectionalArrows = true;
};
```

### Component 3: GameState Integration
```blueprint
BP_FlightGameState (additions):
    Variables:
        CurrentDifficulty: ECourseDifficulty = Medium
        DifficultyConfig: UCourseDifficultyConfig*
        BaseXPReward: int = 570
        BaseCreditsReward: int = 285

    Function SetDifficulty(NewDifficulty):
        CurrentDifficulty = NewDifficulty
        Load appropriate DifficultyConfig asset
        Apply PhysicsConfig to player
        Scale rings based on RingScale
        Update HUD with difficulty info

    Function OnCourseCompleted():
        XP = BaseXPReward * DifficultyConfig.RewardMultiplier
        Credits = BaseCreditsReward * DifficultyConfig.RewardMultiplier
        Award rewards to player
```

## Difficulty Configurations

### Easy Mode
```
Difficulty: Easy
PhysicsConfig: DA_FlightPhysics_Easy
RingScale: 1.5x (600 unit diameter)
TargetTime: 60 seconds
RewardMultiplier: 1.0x
ShowRacingLine: true
ShowArrows: true
AdditionalWaypoints: 0
```

### Medium Mode
```
Difficulty: Medium
PhysicsConfig: DA_FlightPhysics_Medium
RingScale: 1.0x (400 unit diameter)
TargetTime: 45 seconds
RewardMultiplier: 1.5x
ShowRacingLine: false
ShowArrows: true
AdditionalWaypoints: 0
```

### Hard Mode
```
Difficulty: Hard
PhysicsConfig: DA_FlightPhysics_Hard
RingScale: 0.7x (280 unit diameter)
TargetTime: 30 seconds
RewardMultiplier: 2.0x
ShowRacingLine: false
ShowArrows: false
AdditionalWaypoints: 2 (total 7 waypoints)
```

## UI Implementation

### Difficulty Selection Widget (WBP_DifficultySelect)

**Layout**:
```
Canvas Panel
├─ Background Image
├─ Title Text: "SELECT DIFFICULTY"
├─ Button_Easy
│   ├─ Icon (Green)
│   ├─ Label: "EASY"
│   ├─ Description: "Forgiving controls, larger targets"
│   └─ Reward: "570 XP, 285 Credits"
├─ Button_Medium (Default highlighted)
│   ├─ Icon (Yellow)
│   ├─ Label: "MEDIUM"
│   ├─ Description: "Balanced challenge"
│   └─ Reward: "855 XP, 428 Credits"
├─ Button_Hard
│   ├─ Icon (Red)
│   ├─ Label: "HARD"
│   ├─ Description: "Expert pilots only"
│   └─ Reward: "1140 XP, 570 Credits"
└─ Button_Start: "BEGIN COURSE"
```

**Blueprint Logic**:
```blueprint
Variable: SelectedDifficulty = Medium

Button_Easy OnClicked:
    SelectedDifficulty = Easy
    Update button highlights
    Show Easy preview

Button_Medium OnClicked:
    SelectedDifficulty = Medium
    Update button highlights
    Show Medium preview

Button_Hard OnClicked:
    SelectedDifficulty = Hard
    Update button highlights
    Show Hard preview

Button_Start OnClicked:
    Get BP_FlightGameState
    Call SetDifficulty(SelectedDifficulty)
    Close widget
    Start course gameplay
```

### In-Game HUD Updates

**WBP_FlightHUD additions**:
```
Text_Difficulty: Shows current difficulty
Text_TargetTime: Shows target completion time
Progress_TimeRemaining: Optional timer bar
```

## Reward Scaling

### XP/Credits by Difficulty
| Difficulty | Base XP | Multiplier | Final XP | Base Credits | Final Credits |
|------------|---------|------------|----------|--------------|---------------|
| Easy       | 570     | 1.0x       | 570      | 285          | 285           |
| Medium     | 570     | 1.5x       | 855      | 285          | 428           |
| Hard       | 570     | 2.0x       | 1140     | 285          | 570           |

**With Speed Bonus** (under target time):
| Difficulty | Speed Bonus | Total XP |
|------------|-------------|----------|
| Easy       | +100        | 670      |
| Medium     | +150        | 1005     |
| Hard       | +200        | 1340     |

### Flight Skill XP Scaling
| Difficulty | Base Flight XP | Multiplier | Final |
|------------|----------------|------------|-------|
| Easy       | 120            | 1.0x       | 120   |
| Medium     | 120            | 1.5x       | 180   |
| Hard       | 120            | 2.0x       | 240   |

## Dynamic Course Adjustments

### Ring Scaling Script
```python
import unreal

def scale_rings_for_difficulty(difficulty_scale):
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    rings = [a for a in all_actors if "FlightRing" in a.get_actor_label()]

    for ring in rings:
        current_scale = ring.get_actor_scale3d()
        new_scale = unreal.Vector(
            current_scale.x * difficulty_scale,
            current_scale.y * difficulty_scale,
            current_scale.z
        )
        ring.set_actor_scale3d(new_scale)

# Usage:
# Easy: scale_rings_for_difficulty(1.5)
# Medium: scale_rings_for_difficulty(1.0)
# Hard: scale_rings_for_difficulty(0.7)
```

### Visual Guides Toggle
```blueprint
Function SetVisualGuidesEnabled(bool bEnabled):
    RacingLineActor = Find actor by tag "RacingLine"
    RacingLineActor.SetActorHiddenInGame(not bEnabled)

    DirectionalArrows = Find actors by tag "DirectionalArrow"
    For each arrow:
        arrow.SetActorHiddenInGame(not bEnabled)
```

## Testing Checklist

- [ ] Easy mode: Achievable by beginners (>80% success rate)
- [ ] Medium mode: Moderate challenge (50-70% success rate)
- [ ] Hard mode: Difficult for experts (20-40% success rate first try)
- [ ] Rewards scale correctly
- [ ] UI displays proper difficulty info
- [ ] Physics config applies correctly
- [ ] Ring scaling works in-game
- [ ] Visual guides toggle properly

## Implementation Steps

1. Create ECourseDifficulty enum
2. Create UCourseDifficultyConfig class
3. Create 3 difficulty preset assets
4. Implement WBP_DifficultySelect UI
5. Add SetDifficulty() to BP_FlightGameState
6. Integrate with course start logic
7. Test all difficulty tiers
8. Tune based on playtest data

## Status
**Design**: COMPLETE
**Implementation**: Documented (requires manual work)
**Presets**: Defined (3 tiers)
**UI**: Specified (needs UMG design)
