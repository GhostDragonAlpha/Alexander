# Tutorial Course Implementation Guide

## Overview

Complete specification for "Flight Training Academy" - an interactive tutorial course that teaches players all flight mechanics step-by-step with guided instructions, objectives, and immediate feedback.

## Design Philosophy

- **No Time Pressure**: Players learn at their own pace
- **Progressive Complexity**: Each segment builds on previous skills
- **Clear Objectives**: Every segment has specific, measurable goals
- **Immediate Feedback**: UI shows progress and completion in real-time
- **Optional Skip**: Experienced players can skip directly to courses
- **Reward Learning**: Completion unlocks all Easy difficulty courses

## Tutorial Structure

### 11 Segments Total

Each segment teaches one specific skill or mechanic:

1. **Basic Movement** - Forward/backward thrust (W/S)
2. **Lateral Movement** - Strafe and vertical (A/D/Q/E)
3. **Rotation Controls** - Mouse pitch/yaw
4. **First Ring** - Ring navigation basics
5. **Speed Control** - Acceleration and velocity management
6. **Precise Maneuvering** - Precision mode and fine control
7. **Vertical Navigation** - Climbing and 3D movement
8. **Descending Turn** - Complex 3D maneuvers
9. **Flight Assist Modes** - Understanding assist systems
10. **Ring Sequence** - Combining all skills
11. **Graduation** - Course completion and rewards

### Total Course Metrics

- **Distance**: 8930.3 units
- **Duration**: 10-15 minutes (learner-paced)
- **Waypoints**: 12
- **Rings**: 7 (varied difficulty)
- **Difficulty**: Tutorial (easiest)
- **Par Time**: None (no time limit)

---

## Segment Details

### Segment 1: Basic Movement

**Location**: (0, 0, 300)
**Skill Taught**: Forward and backward thrust

**Instructions**:
1. "Welcome to Flight Training Academy!"
2. "Use W/S to control forward/backward thrust"
3. "Try moving forward to the green waypoint"

**Objectives**:
- Reach waypoint WP_Movement

**Success Criteria**: Player uses W or S key and reaches waypoint

**UI Hints**:
- "Press W to accelerate forward"
- "Press S to decelerate or reverse"

**Implementation**:
- Spawn green waypoint at (800, 0, 300)
- Detect player input (W/S keys)
- Check distance to waypoint < 100 units
- Advance when reached

---

### Segment 2: Lateral Movement

**Location**: (800, 0, 300)
**Skill Taught**: Strafing left/right and up/down

**Instructions**:
1. "Great! Now let's learn lateral movement"
2. "Use A/D to strafe left/right"
3. "Use Q/E to move up/down"
4. "Navigate to the waypoint using lateral thrusters"

**Objectives**:
- Reach waypoint WP_Lateral

**Success Criteria**: Player uses lateral controls (A/D/Q/E) to reach waypoint

**UI Hints**:
- "A/D: Strafe Left/Right"
- "Q/E: Move Down/Up"

**Implementation**:
- Spawn waypoint at (1600, 0, 300)
- Detect lateral input keys
- Optional: Spawn waypoint slightly offset to force lateral movement
- Check waypoint reach

---

### Segment 3: Rotation Controls

**Location**: (1600, 0, 300)
**Skill Taught**: Mouse-controlled rotation (pitch/yaw)

**Instructions**:
1. "Excellent! Now let's practice rotation"
2. "Move your mouse to control pitch and yaw"
3. "Rotate your ship to face the next waypoint"
4. "The ship HUD will show your target direction"

**Objectives**:
1. Face direction toward waypoint (within 15 degrees)
2. Reach waypoint WP_Rotation

**Success Criteria**: Player uses mouse to orient toward target and navigate

**UI Hints**:
- "Move mouse to rotate ship"
- "Green arrow shows target direction"

**Implementation**:
- Spawn waypoint at (2400, 0, 300)
- Add HUD direction arrow pointing to waypoint
- Calculate angle between ship forward vector and waypoint direction
- Check if angle < 15 degrees for 1 second
- Then check waypoint reach

---

### Segment 4: First Ring

**Location**: (2400, 0, 300)
**Skill Taught**: Ring navigation

**Instructions**:
1. "Time to try your first ring!"
2. "Rings are checkpoints in flight courses"
3. "Align with the ring and fly through it"
4. "The ring will turn green when you pass through"

**Objectives**:
- Pass through ring R_Tutorial_1

**Success Criteria**: Player flies through ring successfully

**UI Hints**:
- "Line up with the ring center"
- "Adjust speed for precise control"

**Implementation**:
- Spawn ring at (3200, 0, 300) with 700 unit diameter (large, forgiving)
- Ring material: Blue (default) -> Green (on pass)
- Detect overlap with ring trigger volume
- Play success sound and visual effect
- Advance to next segment

---

### Segment 5: Speed Control

**Location**: (3200, 0, 300)
**Skill Taught**: Acceleration and velocity management

**Instructions**:
1. "Perfect! Let's practice speed control"
2. "Hold W to accelerate to maximum speed"
3. "Watch your velocity indicator on the HUD"
4. "Reach the next waypoint at high speed"

**Objectives**:
1. Reach 500+ units/sec for 2 seconds
2. Reach waypoint WP_Speed

**Success Criteria**: Player maintains high speed and reaches waypoint

**UI Hints**:
- "Current speed shown on HUD"
- "Target: 500+ units/sec"

**Implementation**:
- HUD velocity meter highlights when >500 u/s
- Timer tracks duration above threshold
- Green checkmark appears when 2 seconds reached
- Spawn waypoint at (4000, 0, 300)

---

### Segment 6: Precise Maneuvering

**Location**: (4000, 0, 300)
**Skill Taught**: Precision mode (reduced thrust for fine control)

**Instructions**:
1. "Great speed control! Now let's practice precision"
2. "Hold Shift to enable precision mode (reduced thrust)"
3. "Navigate through the narrow ring carefully"

**Objectives**:
- Pass through narrow ring R_Tutorial_2

**Success Criteria**: Player uses precision mode to navigate narrow ring

**UI Hints**:
- "Hold Shift for precision mode"
- "Take your time, no rush!"

**Implementation**:
- Add precision mode input (Left Shift)
- When held: Thrust/rotation multiplied by 0.3
- Spawn narrow ring at (4600, 0, 300) with 400 unit diameter
- Detect precision mode usage and ring pass

---

### Segment 7: Vertical Navigation

**Location**: (4600, 0, 300)
**Skill Taught**: Climbing to elevated waypoints

**Instructions**:
1. "Excellent control! Time for vertical movement"
2. "The next waypoint is high above you"
3. "Use Q/E and mouse pitch to climb"
4. "Maintain forward velocity while ascending"

**Objectives**:
- Reach elevated waypoint WP_Vertical

**Success Criteria**: Player climbs to high-altitude waypoint

**UI Hints**:
- "Press E to ascend"
- "Mouse up to pitch nose up"

**Implementation**:
- Spawn waypoint at (4600, 0, 800) - 500 units above current position
- Add vertical velocity indicator on HUD
- Check altitude gain
- Check waypoint reach

---

### Segment 8: Descending Turn

**Location**: (5400, 0, 800)
**Skill Taught**: Complex 3D maneuvers combining multiple controls

**Instructions**:
1. "Nice climb! Now let's practice descending"
2. "The waypoint is below and to the side"
3. "Combine rotation, descent, and lateral movement"
4. "This is a 3D maneuver - use all controls together"

**Objectives**:
- Reach waypoint WP_DescendTurn

**Success Criteria**: Player performs complex maneuver using multiple controls

**UI Hints**:
- "Q: Descend | A/D: Turn | W: Thrust"
- "Combine all controls smoothly"

**Implementation**:
- Spawn waypoint at (6000, -400, 500) - descending and lateral offset
- Monitor multiple input types (WASDQE + mouse)
- Reward smooth, coordinated movement
- Check waypoint reach

---

### Segment 9: Flight Assist Modes

**Location**: (6000, -400, 500)
**Skill Taught**: Understanding and using flight assist modes

**Instructions**:
1. "Well done! Let's learn about Flight Assist"
2. "Press F to cycle through assist modes:"
3. "- AutoLevel: Ship auto-corrects orientation"
4. "- Stability: Damping reduces drift"
5. "- None: Full manual control"
6. "Try each mode and see the difference"

**Objectives**:
1. Cycle through all 3 assist modes (3 cycles)
2. Reach waypoint WP_AssistDemo

**Success Criteria**: Player tries each assist mode

**UI Hints**:
- "Press F to change assist mode"
- "Current mode shown on HUD"

**Implementation**:
- Detect F key press
- Cycle: AutoLevel -> Stability -> None -> AutoLevel
- Display current mode on HUD
- Count mode changes (need 3+ to try all)
- Spawn waypoint at (6600, -400, 500)

---

### Segment 10: Ring Sequence

**Location**: (6600, -400, 500)
**Skill Taught**: Combining all learned skills

**Instructions**:
1. "Now let's combine everything you've learned"
2. "Navigate through 3 rings in sequence"
3. "Use the skills you've practiced"
4. "Take your time - this is your final test!"

**Objectives**:
1. Pass through ring R_Final_1
2. Pass through ring R_Final_2
3. Pass through ring R_Final_3

**Success Criteria**: Player passes through all 3 rings in order

**UI Hints**:
- "Ring 1 of 3" (updates as rings passed)
- "Remember: precision over speed!"

**Ring Positions**:
- Ring 1: (7000, -200, 550) - Diameter 600, Rotation (0, 0, -10)
- Ring 2: (7400, 200, 650) - Diameter 600, Rotation (10, 0, 10)
- Ring 3: (7800, -100, 550) - Diameter 600, Rotation (0, 0, -15)

**Implementation**:
- Spawn 3 rings in sequence
- Track which rings passed (in order)
- Update UI counter "Ring X of 3"
- Advance only when all 3 passed

---

### Segment 11: Graduation

**Location**: (8200, 0, 500)
**Skill Taught**: None (celebration and course completion)

**Instructions**:
1. "Congratulations! You've completed flight training!"
2. "You've learned:"
3. "- Forward/backward thrust (W/S)"
4. "- Lateral movement (A/D, Q/E)"
5. "- Rotation controls (Mouse)"
6. "- Ring navigation"
7. "- Speed control"
8. "- Precision maneuvering"
9. "- Flight assist modes"
10. ""
11. "You're now ready to tackle real flight courses!"
12. "Fly to the finish line to complete training"

**Objectives**:
- Reach finish waypoint WP_Finish

**Success Criteria**: Player reaches graduation waypoint

**Completion Rewards**:
- 500 XP
- 150 Credits
- Unlock: All Easy difficulty courses

**Implementation**:
- Spawn large gold waypoint at (8200, 0, 500)
- Play celebration music
- Show completion screen with rewards
- Update player progression (unlock Easy courses)
- Return to course selection menu

---

## UI Components

### WBP_TutorialInstructions Widget

**Location**: Top center of screen
**Style**: Semi-transparent popup box with rounded corners

**Layout**:
```
+------------------------------------------------+
| FLIGHT TRAINING ACADEMY                        |
| Segment 1 of 11: Basic Movement                |
|                                                |
| [Instruction text, multiple lines]             |
|                                                |
| OBJECTIVES:                                    |
| [ ] Reach waypoint WP_Movement                 |
|                                                |
| [Dismiss] [Skip Tutorial]                      |
+------------------------------------------------+
```

**Features**:
- Auto-appears when entering new segment
- Pauses game while displayed
- Player clicks "Dismiss" to resume
- "Skip Tutorial" button (confirmation dialog)
- Progress indicator (Segment X of 11)
- Objective checklist with real-time updates

### WBP_TutorialHUD Widget

**Location**: Various positions on screen
**Elements**:
- **Velocity Meter**: Bottom right, shows current speed
- **Current Objective**: Top right, always visible
- **Direction Arrow**: Center screen, points to next objective
- **Flight Assist Mode**: Bottom left, shows current mode
- **Hint Text**: Bottom center, context-sensitive tips
- **Progress Bar**: Top center, shows overall tutorial progress (1-11)

### Tutorial Pause Menu

**Accessible**: Press ESC during tutorial
**Options**:
- Resume Tutorial
- Restart Current Segment
- Restart Tutorial from Beginning
- Skip Tutorial (requires confirmation)
- Return to Main Menu

---

## Implementation Details

### BP_TutorialManager Blueprint

**Location**: Spawned automatically when tutorial level loads

**Variables**:
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite)
TArray<FTutorialSegment> TutorialSegments;

UPROPERTY(BlueprintReadWrite)
int32 CurrentSegmentIndex = 0;

UPROPERTY(BlueprintReadWrite)
TArray<bool> ObjectiveCompletionStatus;

UPROPERTY()
UWBPTutorialInstructions* InstructionWidget;

UPROPERTY()
UWBPTutorialHUD* HUDWidget;
```

**Key Functions**:

1. **BeginPlay()**:
   - Initialize tutorial data
   - Create instruction and HUD widgets
   - Start Segment 1
   - Pause game and show instructions

2. **StartSegment(int32 SegmentIndex)**:
   - Load segment data
   - Spawn objectives (waypoints, rings)
   - Update instruction widget
   - Pause game for instruction display
   - Reset objective completion status

3. **Tick(float DeltaTime)**:
   - If game not paused:
     - Check all current segment objectives
     - Update objective completion status
     - Update HUD (direction arrow, hints, progress)
     - If all objectives complete: Advance to next segment

4. **CheckObjective(FObjectiveData Objective)**:
   - Switch based on objective type:
     - "reach_waypoint": Check distance < threshold
     - "pass_through_ring": Check overlap with ring trigger
     - "reach_speed": Check velocity >= target for duration
     - "face_direction": Check angle to target < tolerance
     - "cycle_assist_modes": Track mode changes
   - Return true if objective met

5. **AdvanceToNextSegment()**:
   - Increment CurrentSegmentIndex
   - If SegmentIndex < 11:
     - Despawn previous objectives
     - Call StartSegment(CurrentSegmentIndex)
   - Else:
     - Call CompleteTutorial()

6. **CompleteTutorial()**:
   - Show completion screen
   - Award 500 XP, 150 Credits
   - Unlock all Easy difficulty courses
   - Play celebration effects
   - Wait 5 seconds
   - Return to course selection menu

7. **SkipTutorial()**:
   - Show confirmation dialog
   - If confirmed:
     - Award basic rewards (no XP bonus)
     - Unlock Easy courses
     - Return to menu

### Objective Data Structures

```cpp
USTRUCT(BlueprintType)
struct FObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ObjectiveType; // "reach_waypoint", "pass_through_ring", etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TargetName; // Name of target actor/component

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetValue; // Speed threshold, distance, angle, etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration; // Time requirement (e.g., maintain speed for X seconds)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequired; // Must complete to advance

    UPROPERTY(BlueprintReadWrite)
    bool bCompleted; // Runtime completion status
};

USTRUCT(BlueprintType)
struct FTutorialSegment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SegmentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SegmentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> InstructionText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FObjectiveData> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> UIHints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector SpawnLocation;
};
```

---

## Level Design

### L_TutorialFlightAcademy Level

**Environment**: Indoor training hangar (enclosed space, safe)

**Lighting**: Bright, even lighting (no harsh shadows)

**Skybox**: Interior hangar ceiling with overhead lights

**Floor**: Marked with grid lines and safety zones

**Props**:
- Training markers (cones, arrows)
- Safety barriers along edges
- Holographic instruction displays
- Signage for each training zone

**Dimensions**:
- Length: ~8500 units
- Width: ~1000 units
- Height: ~1000 units
- Total area: Safe, enclosed training space

**Boundaries**: Invisible walls to prevent players from flying out of bounds

---

## Integration with Course Selection

### Unlock Logic

Tutorial completion unlocks:
- DA_BeginnerCanyon (Easy)
- Any other Easy difficulty courses

**PlayerProgressionComponent**:
```cpp
void OnTutorialComplete()
{
    AddXP(500);
    AddCredits(150);
    SetTutorialCompleted(true);
    UnlockDifficulty(EDifficultyLevel::Easy);
}
```

### First-Time Player Flow

1. Player starts game for first time
2. No courses unlocked
3. Only option: "Start Flight Training"
4. Complete tutorial -> Easy courses unlocked
5. Can now select from course menu

### Skip Tutorial Option

For experienced players or testing:
- Option in settings: "Skip Tutorial and Unlock Easy Courses"
- Confirmation dialog warns about missing training
- Awards basic unlocks but no XP/Credits

---

## Testing Checklist

### Segment Testing
- [ ] Each segment's instructions display correctly
- [ ] Objectives track progress accurately
- [ ] Objectives complete when criteria met
- [ ] Segments advance automatically on completion
- [ ] Cannot skip segments (must complete in order)
- [ ] UI hints display at appropriate times
- [ ] Direction arrow points to correct objective

### Input Testing
- [ ] W/S thrust controls work in Segment 1
- [ ] A/D/Q/E lateral controls work in Segment 2
- [ ] Mouse rotation works in Segment 3
- [ ] Ring pass detection works in Segments 4, 6, 10
- [ ] Speed detection works in Segment 5
- [ ] Precision mode (Shift) works in Segment 6
- [ ] Assist mode cycling (F) works in Segment 9
- [ ] All inputs work in final ring sequence

### UI Testing
- [ ] Instruction widget displays correctly
- [ ] Progress indicator updates (X of 11)
- [ ] Objective checklist updates in real-time
- [ ] HUD elements position correctly
- [ ] Velocity meter shows accurate speed
- [ ] Direction arrow points to targets
- [ ] Flight assist mode displays current mode

### Progression Testing
- [ ] Tutorial completion awards 500 XP, 150 Credits
- [ ] Easy courses unlock on completion
- [ ] Tutorial completion saves to player profile
- [ ] Can replay tutorial from course menu
- [ ] Skip tutorial button works with confirmation
- [ ] Skipping tutorial still unlocks Easy courses

### Performance Testing
- [ ] Tutorial runs smoothly (60+ FPS)
- [ ] No memory leaks from spawning/despawning objects
- [ ] Instruction widget doesn't cause hitches

---

## Localization Support

All instruction text should be externalized for localization:
- 11 segments × ~4 instruction lines = 44 strings
- UI hints: ~20 strings
- Objective descriptions: ~15 strings
- Button labels: 5 strings

**Total**: ~85 strings to localize

Create string table: `ST_TutorialInstructions`

---

## Accessibility Features

1. **Adjustable Text Size**: Scale instruction text (100%, 125%, 150%)
2. **Colorblind Mode**: Alternative colors for waypoints/rings
3. **Subtitles**: All audio instructions also shown as text
4. **Pause Anytime**: Press ESC to pause and review instructions
5. **No Time Limit**: Players can take as long as needed

---

## Conclusion

Complete specification for interactive tutorial course with:
- 11 progressive learning segments
- 12 waypoints, 7 rings
- Step-by-step skill building
- Real-time objective tracking
- No time pressure
- Rewards completion with XP, Credits, and Easy course unlocks

Estimated implementation time: **10-15 hours**

**Status**: Design Complete - Ready for Implementation
