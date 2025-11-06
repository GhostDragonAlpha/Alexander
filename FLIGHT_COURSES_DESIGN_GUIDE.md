# Flight Courses Implementation Guide

## Overview

This document provides complete specifications for implementing 3 additional flight courses with progressive difficulty. Each course has been designed with unique themes, challenges, and reward structures.

## Course Summary

| Course | Difficulty | Distance | Waypoints | Rings | Obstacles | Theme |
|--------|-----------|----------|-----------|-------|-----------|-------|
| Beginner's Canyon | Easy | 4601.8 units | 5 | 4 | 2 | Desert Canyon |
| Industrial Nexus | Medium | 8299.9 units | 6 | 5 | 5 | Space Station |
| Asteroid Gauntlet | Hard | 12356.0 units | 7 | 6 | 7 | Asteroid Field |

## Course 1: Beginner's Canyon (Easy)

### Design Philosophy
Perfect introduction to flight mechanics. Wide rings, gentle turns, minimal obstacles. Focus on teaching basic 6DOF movement and waypoint navigation.

### Waypoint Layout
```
Start (0, 0, 300)
  -> WP2 (1200, 500, 400)      [Distance: 1304 units, Climb: 100]
  -> WP3 (2200, 0, 500)         [Distance: 1118 units, Climb: 100]
  -> WP4 (3000, -600, 400)      [Distance: 1029 units, Descend: -100]
  -> Finish (4000, 0, 300)      [Distance: 1166 units, Descend: -100]
```

### Ring Specifications
All rings are 800 units diameter (beginner-friendly size).

**Ring 1**: Between Start and WP2
- Position: (600, 250, 350)
- Rotation: (0, 0, -15) - Slight bank
- Purpose: First challenge, easy to hit

**Ring 2**: Between WP2 and WP3
- Position: (1700, 250, 450)
- Rotation: (0, 0, 10) - Opposite bank
- Purpose: Teaches roll correction

**Ring 3**: Between WP3 and WP4
- Position: (2600, -300, 450)
- Rotation: (0, 0, -10)
- Purpose: Turning challenge

**Ring 4**: Before Finish
- Position: (3500, -300, 350)
- Rotation: (0, 0, 5)
- Purpose: Final accuracy test

### Obstacles
**Canyon Walls** (2 total):
- North Wall: (2000, 1500, 400) - Scale (50, 20, 15)
- South Wall: (2000, -1500, 400) - Scale (50, 20, 15)
- Purpose: Define flight corridor, prevent shortcuts

### Environment
- **Theme**: Desert canyon at sunset
- **Sky**: Orange/red sunset gradient
- **Lighting**: Warm directional light (sunset angle)
- **Fog**: Light atmospheric fog (0.02 density)
- **Particles**: Dust motes in air
- **Ground**: Sand/rock texture
- **Props**: Desert vegetation, rock formations

### Physics Configuration
```cpp
MaxSafeVelocity: 700.0 units/sec (slow, controllable)
ThrottleSmoothing: 0.15 (very smooth)
RotationSmoothing: 0.20 (gentle turns)
DefaultAssistMode: AutoLevel (beginner-friendly)
CollisionAvoidance: Enabled
ThrustLimiting: Enabled
```

### Rewards
- **Completion XP**: 150
- **Completion Credits**: 50
- **Time Bonus**: 1.0x multiplier
- **First Completion**: +100 XP bonus
- **Par Time**: 60 seconds
- **Recommended Time**: 75 seconds

### Implementation Steps
1. Create new level "L_BeginnerCanyon"
2. Place terrain (canyon floor and walls)
3. Spawn 5 waypoints at specified positions
4. Spawn 4 rings with 800 unit diameter
5. Add 2 canyon wall static meshes
6. Configure sunset lighting and atmosphere
7. Set physics config to Easy preset
8. Create CourseData asset with rewards

---

## Course 2: Industrial Nexus (Medium)

### Design Philosophy
Navigate through abandoned space station superstructure. Medium-sized rings, more complex path with vertical and horizontal turns, multiple obstacle types. Tests spatial awareness and precision.

### Waypoint Layout
```
Start (0, 0, 500)
  -> WP2 (1500, 800, 700)       [Distance: 1717 units, Climb: 200]
  -> WP3 (2500, -600, 1100)     [Distance: 1760 units, Climb: 400]
  -> WP4 (3800, 400, 800)       [Distance: 1509 units, Descend: -300]
  -> WP5 (5000, -800, 600)      [Distance: 1529 units, Descend: -200]
  -> Finish (6200, 0, 500)      [Distance: 1529 units, Descend: -100]
```

### Ring Specifications
Rings are 550-600 units diameter (moderate difficulty).

**Ring 1**: Early ascent
- Position: (750, 400, 600)
- Diameter: 600 units
- Rotation: (0, 0, -20)
- Purpose: Climbing turn

**Ring 2**: High altitude turn
- Position: (2000, 100, 900)
- Diameter: 550 units
- Rotation: (15, 0, -10) - Pitched and banked
- Purpose: 3D orientation challenge

**Ring 3**: Peak altitude
- Position: (3150, -100, 950)
- Diameter: 600 units
- Rotation: (-10, 0, 15)
- Purpose: Direction change at apex

**Ring 4**: Descending turn
- Position: (4400, -200, 700)
- Diameter: 550 units
- Rotation: (0, 0, -25)
- Purpose: Steep bank while descending

**Ring 5**: Final approach
- Position: (5600, -400, 550)
- Diameter: 600 units
- Rotation: (10, 0, 10)
- Purpose: Setup for finish

### Obstacles
**Station Beams** (3 total):
- Beam 1: (1500, 0, 900) - Scale (5, 5, 40) - Rotation (0, 45, 0)
- Beam 2: (2500, 500, 800) - Scale (5, 5, 35) - Rotation (0, -30, 0)
- Beam 3: (3500, -500, 700) - Scale (5, 5, 30) - Rotation (0, 60, 0)
- Purpose: Structural hazards requiring vertical navigation

**Debris Field**:
- Position: (4200, 200, 650)
- Count: 8 floating objects
- Spread: 300 unit radius
- Purpose: Dodge challenge

**Station Platform**:
- Position: (5000, -400, 400)
- Scale: (30, 30, 2) - Large flat surface
- Purpose: Low-altitude hazard

### Environment
- **Theme**: Abandoned industrial space station
- **Lighting**: Harsh white overhead lights (some flickering)
- **Fog**: Minimal (0.01 density)
- **Particles**: Sparks from damaged systems
- **Hazards**: Electric arcs between beams, steam vents
- **Ambience**: Mechanical creaking sounds
- **Skybox**: Dark space with distant nebula

### Physics Configuration
```cpp
MaxSafeVelocity: 1000.0 units/sec (standard speed)
ThrottleSmoothing: 0.1 (balanced)
RotationSmoothing: 0.15 (responsive)
DefaultAssistMode: Stability (balanced assist)
CollisionAvoidance: Enabled
ThrustLimiting: Enabled
```

### Rewards
- **Completion XP**: 300
- **Completion Credits**: 100
- **Time Bonus**: 1.5x multiplier
- **First Completion**: +200 XP bonus
- **Par Time**: 45 seconds
- **Recommended Time**: 55 seconds

### Implementation Steps
1. Create new level "L_IndustrialNexus"
2. Place station superstructure meshes
3. Spawn 6 waypoints at specified positions
4. Spawn 5 rings with varied diameters
5. Add 3 station beam obstacles with rotations
6. Add debris field (8 objects)
7. Add platform obstacle
8. Configure harsh lighting and particle effects
9. Set physics config to Medium preset
10. Create CourseData asset with rewards

---

## Course 3: Asteroid Gauntlet (Hard)

### Design Philosophy
High-speed run through dense asteroid field. Smallest rings, sharp turns, many obstacles requiring constant evasion. Expert pilots only - no assists. Tests reflexes, precision, and spatial navigation under pressure.

### Waypoint Layout
```
Start (0, 0, 800)
  -> WP2 (1800, 1200, 1200)     [Distance: 2190 units, Climb: 400]
  -> WP3 (3200, -800, 1600)     [Distance: 2519 units, Climb: 400]
  -> WP4 (4800, 600, 1100)      [Distance: 1936 units, Descend: -500]
  -> WP5 (6000, -1000, 1400)    [Distance: 1961 units, Climb: 300]
  -> WP6 (7500, 400, 900)       [Distance: 2031 units, Descend: -500]
  -> Finish (8800, 0, 800)      [Distance: 1414 units, Descend: -100]
```

### Ring Specifications
Rings are 400-450 units diameter (expert difficulty).

**Ring 1**: Early acceleration
- Position: (900, 600, 1000)
- Diameter: 450 units
- Rotation: (0, 0, -30)
- Purpose: Speed test

**Ring 2**: Climbing through field
- Position: (2500, 200, 1400)
- Diameter: 400 units (smallest)
- Rotation: (20, 0, -15)
- Purpose: Precision at speed

**Ring 3**: High altitude weave
- Position: (4000, -100, 1350)
- Diameter: 420 units
- Rotation: (-15, 0, 25)
- Purpose: Complex orientation

**Ring 4**: Asteroid cluster
- Position: (5400, -200, 1250)
- Diameter: 400 units
- Rotation: (10, 0, -20)
- Purpose: Threading between asteroids

**Ring 5**: Dense field
- Position: (6750, -300, 1150)
- Diameter: 450 units
- Rotation: (-20, 0, 15)
- Purpose: Evasion challenge

**Ring 6**: Final sprint
- Position: (8150, 200, 850)
- Diameter: 420 units
- Rotation: (0, 0, -25)
- Purpose: Last accuracy test

### Obstacles
**Large Asteroids** (4 total):
- Asteroid 1: (1500, 800, 1100) - Diameter 300 - Rotation speed 0.5 rad/sec
- Asteroid 2: (2800, -400, 1500) - Diameter 350 - Rotation speed 0.3 rad/sec
- Asteroid 3: (5200, -600, 1300) - Diameter 320 - Rotation speed 0.6 rad/sec
- Asteroid 4: (7800, -200, 900) - Diameter 340 - Rotation speed 0.4 rad/sec
- Purpose: Major hazards requiring wide berth

**Asteroid Clusters** (2 areas):
- Cluster 1: (4000, 200, 1200) - 12 medium asteroids - 400 unit spread
- Cluster 2: (6500, 100, 1000) - 15 medium asteroids - 450 unit spread
- Purpose: Weaving challenge

**Asteroid Field**:
- Position: (4500, 0, 1300)
- Count: 25 small asteroids
- Spread: 800 unit radius
- Density: High
- Purpose: Dense navigation test

### Environment
- **Theme**: Dense asteroid belt
- **Lighting**: Harsh directional starlight
- **Fog**: Minimal (0.005 density)
- **Particles**: Rock dust clouds
- **Hazards**: Rotating asteroids, debris clouds
- **Ambience**: Low rumbling from impacts
- **Skybox**: Deep space with stellar background

### Physics Configuration
```cpp
MaxSafeVelocity: 1500.0 units/sec (high speed)
ThrottleSmoothing: 0.06 (very responsive)
RotationSmoothing: 0.1 (quick turns)
DefaultAssistMode: None (no assistance)
CollisionAvoidance: Disabled (player skill required)
ThrustLimiting: Disabled (full performance)
```

### Rewards
- **Completion XP**: 500
- **Completion Credits**: 200
- **Time Bonus**: 2.0x multiplier
- **First Completion**: +500 XP bonus
- **Difficulty Bonus**: +300 XP
- **Par Time**: 30 seconds
- **Recommended Time**: 40 seconds

### Implementation Steps
1. Create new level "L_AsteroidGauntlet"
2. Place asteroid field backdrop
3. Spawn 7 waypoints at specified positions
4. Spawn 6 rings with 400-450 unit diameter
5. Add 4 large rotating asteroids with collision
6. Add 2 asteroid clusters (27 total asteroids)
7. Add dense asteroid field (25 asteroids)
8. Configure starlight lighting
9. Set physics config to Hard preset
10. Create CourseData asset with rewards
11. Enable asteroid rotation blueprints

---

## Technical Implementation

### CourseData Asset Structure

Create a new Blueprint class `BP_FlightCourseData` inheriting from `UDataAsset`:

```cpp
UCLASS(BlueprintType)
class UFlightCourseData : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Course Info")
    FString CourseName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Course Info")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Course Info")
    EDifficultyLevel Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Course Info")
    TSoftObjectPtr<UWorld> CourseLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float ParTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float RecommendedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
    int32 CompletionXP;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
    int32 CompletionCredits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
    float TimeBonusMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
    int32 FirstCompletionBonus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    UFlightPhysicsConfig* PhysicsConfig;
};
```

### Waypoint Actor Blueprint

Use existing `BP_FlightWaypoint` with checkpoint type property:
- Start: Green material, larger scale
- Checkpoint: Blue material, standard scale
- Finish: Gold material, larger scale with particle burst

### Ring Actor Blueprint

Use existing `BP_FlightRing` with configurable diameter:
- Set diameter via construction script
- Apply rotation in editor
- Energy field material with emissive pulse

### Obstacle Actors

Create modular obstacle blueprints:
- `BP_CanyonWall`: Static mesh with collision
- `BP_StationBeam`: Metal beam with electric arc particles
- `BP_DebrisField`: Spawner that places multiple debris objects
- `BP_Asteroid`: Rotating mesh with collision, configurable size
- `BP_AsteroidCluster`: Spawner for multiple asteroids

### Course Manager

Create `BP_CourseManager` actor:
- Loads CourseData asset
- Tracks player progress through waypoints
- Calculates time and awards rewards
- Handles course reset and respawn

## Difficulty Progression

| Aspect | Easy | Medium | Hard |
|--------|------|--------|------|
| Course Length | 4517 units | 8744 units | 14051 units |
| Ring Diameter | 800 units | 550-600 units | 400-450 units |
| Max Turn Angle | ~60 degrees | ~100 degrees | ~130 degrees |
| Obstacles | 2 (static) | 5 (mixed) | 7 (moving) |
| Max Velocity | 700 u/s | 1000 u/s | 1500 u/s |
| Assist Mode | AutoLevel | Stability | None |
| XP Reward | 150 | 300 | 500 |

## Testing Checklist

### Per-Course Testing
- [ ] All waypoints spawn at correct positions
- [ ] All rings spawn with correct diameter and rotation
- [ ] All obstacles spawn without overlapping rings
- [ ] Course completion triggers rewards correctly
- [ ] Time tracking works accurately
- [ ] Checkpoint system saves progress
- [ ] Reset functionality returns player to start
- [ ] Physics config loads correctly for difficulty
- [ ] Environmental effects render properly
- [ ] Lighting creates intended atmosphere

### Progression Testing
- [ ] Courses unlock in correct order
- [ ] Easy course is completable by new players (>80% success rate)
- [ ] Medium course provides good challenge (50-70% success rate)
- [ ] Hard course is challenging for experts (20-40% first-try success)
- [ ] Par times are achievable but challenging
- [ ] Reward progression feels balanced
- [ ] Physics differences are noticeable between difficulties

## Next Steps (CE02)

With course designs complete, next task is to create course selection menu system to allow players to choose and load these courses.

## Conclusion

Three distinct flight courses designed with progressive difficulty:
1. **Beginner's Canyon**: Wide, gentle, forgiving - perfect for learning
2. **Industrial Nexus**: Medium challenge with 3D navigation and obstacles
3. **Asteroid Gauntlet**: Expert-level speed run through dense hazards

**Total Content**: 18 waypoints, 15 rings, 14 obstacles across 3 themed environments.

**Implementation Ready**: All positions, rotations, scales, and configurations specified.

**Status**: Design Complete - Ready for level creation (CE02)
