# PlayerProgression Integration Guide

## Overview
Connects the flight course gameplay to the existing PlayerProgression system, rewarding players with XP, credits, and skill improvements.

## System Architecture

**FlightProgressionManager Actor**
- Manages all progression rewards for flight course
- Interfaces with PlayerProgression component
- Tracks player performance and awards accordingly

**PlayerProgression Component** (from PlayerProgression.cpp)
- `AddExperience(int32 Amount)` - Awards XP, checks for level up
- `AddCredits(int32 Amount)` - Awards in-game currency
- `AddSkillXP(ESkillType Skill, int32 Amount)` - Improves specific skills

## Reward Structure

### Per-Objective Rewards
| Objective | XP | Credits | Flight Skill XP |
|-----------|-----|---------|-----------------|
| Waypoint Collection | 50 | 25 | 10 |
| Ring Passage | 30 | 15 | 5 |

### Completion Rewards
- **Course Completion**: 200 XP, 100 Credits, 50 Flight XP
- **Speed Bonus** (under 60s): 100 XP additional

### Total Possible Rewards
- **Perfect Run**: 570 XP, 285 Credits, 120 Flight XP
- **With Speed Bonus**: 670 XP, 285 Credits, 120 Flight XP

## Implementation Steps

### 1. FlightProgressionManager Blueprint

**Variables:**
```
PlayerProgressionRef: UPlayerProgression*
```

**Event BeginPlay:**
```blueprint
1. Get Player Pawn
2. Get PlayerProgression component from pawn
3. Store in PlayerProgressionRef
4. Validate reference is not null
```

**Functions:**

`OnWaypointCollected(int WaypointIndex)`
```blueprint
If PlayerProgressionRef != null:
    Call AddExperience(50)
    Call AddCredits(25)
    Call AddSkillXP(Flight, 10)
    Print "Waypoint +50 XP +25 Credits"
```

`OnRingPassed(int RingIndex)`
```blueprint
If PlayerProgressionRef != null:
    Call AddExperience(30)
    Call AddCredits(15)
    Call AddSkillXP(Flight, 5)
    Print "Ring +30 XP +15 Credits"
```

`OnCourseCompleted(float Time)`
```blueprint
If PlayerProgressionRef != null:
    Call AddExperience(200)
    Call AddCredits(100)
    Call AddSkillXP(Flight, 50)
    Print "Course Complete +200 XP"

    If Time < 60.0:
        Call AddExperience(100)
        Print "Speed Bonus +100 XP"
```

### 2. BP_FlightGameState Integration

**Add Variable:**
```
ProgressionManagerRef: BP_FlightProgressionManager*
```

**Event BeginPlay:**
```blueprint
Get all actors of class BP_FlightProgressionManager
Store first result in ProgressionManagerRef
```

**Update Existing Functions:**

`CollectWaypoint()`
```blueprint
WaypointsCollected += 1
Score += 100

If ProgressionManagerRef != null:
    Call ProgressionManagerRef.OnWaypointCollected(WaypointsCollected)

If WaypointsCollected == 5:
    Call CheckCourseCompletion()
```

`PassThroughRing()`
```blueprint
RingsPassed += 1
Score += 50

If ProgressionManagerRef != null:
    Call ProgressionManagerRef.OnRingPassed(RingsPassed)
```

`CheckCourseCompletion()`
```blueprint
If WaypointsCollected == 5 AND RingsPassed == 4:
    CourseComplete = True

    If ProgressionManagerRef != null:
        Call ProgressionManagerRef.OnCourseCompleted(TimeElapsed)
```

### 3. Waypoint/Ring Trigger Logic

Each waypoint and ring needs OnOverlapBegin event:

**Waypoint OnOverlapBegin:**
```blueprint
If Other Actor is Player Pawn:
    Get BP_FlightGameState from level
    Call GameState.CollectWaypoint()
    Destroy this actor (or disable collision)
```

**Ring OnOverlapBegin:**
```blueprint
If Other Actor is Player Pawn:
    Get BP_FlightGameState from level
    Call GameState.PassThroughRing()
    Play visual effect (optional)
```

## Testing Checklist

- [ ] PlayerProgression component accessible from player pawn
- [ ] FlightProgressionManager finds player successfully
- [ ] Waypoint collection awards correct XP/Credits
- [ ] Ring passage awards correct XP/Credits
- [ ] Course completion awards bonus
- [ ] Speed bonus triggers for fast completion
- [ ] Flight skill level increases
- [ ] HUD shows updated progression values
- [ ] Level up notification appears (if level threshold crossed)

## Next Steps

1. Implement Blueprint graphs for FlightProgressionManager
2. Add ProgressionManagerRef to BP_FlightGameState
3. Update waypoint/ring trigger logic
4. Test full progression loop in PIE
5. Tune reward values based on playtesting
6. Add visual feedback for progression rewards (XP popup, etc.)

## Status
- Architecture: DEFINED
- Actors Created: YES (FlightProgressionManager spawned)
- Implementation: PENDING (requires Blueprint visual scripting)
- Integration Points: DOCUMENTED
