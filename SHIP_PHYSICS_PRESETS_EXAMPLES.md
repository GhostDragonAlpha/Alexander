# Ship Physics Presets Examples

## Overview

This document provides complete, ready-to-use physics preset configurations for different ship types. Copy these values directly into your `UShipPhysicsConfig` Data Assets for instant, balanced ship archetypes.

## Table of Contents

1. [Light Fighter](#light-fighter)
2. [Heavy Freighter](#heavy-freighter)
3. [Balanced Scout](#balanced-scout)
4. [Racing Ship](#racing-ship)
5. [Tank Ship](#tank-ship)
6. [Additional Archetypes](#additional-archetypes)
7. [Customization Guide](#customization-guide)
8. [Physics Parameter Reference](#physics-parameter-reference)

---

## Light Fighter

### Overview
Fast, agile combat ship designed for dogfighting and quick maneuvers. Low mass provides excellent handling but makes it vulnerable to impacts and gravity wells.

### Flight Characteristics
- **Speed**: Very Fast
- **Agility**: Excellent
- **Stability**: Moderate
- **Best For**: Combat, racing, precision flying
- **Weaknesses**: Low health, affected by gravity, requires skill

### Data Asset Configuration

**Asset Name**: `DA_LightFighter`

```
Config Name: Light Fighter
Description: Fast and agile combat ship with excellent maneuverability

=== PHYSICS PARAMETERS ===
Mass: 500.0 kg
Thrust Power: 150000.0 N
Max Velocity: 1500.0 m/s
Angular Thrust Power: 75000.0 N
Fuel Capacity: 800.0
Fuel Consumption Rate: 1.2

=== FLIGHT CHARACTERISTICS ===
Acceleration Multiplier: 1.5
Rotation Speed Multiplier: 1.3
Stability Assist Strength: 0.4
```

### Recommended Ship Stats (Parts)
Combine with these stat multipliers for optimal performance:

- Thrust Power: 1.2 - 1.5x
- Max Velocity: 1.3 - 1.8x
- Rotation Speed: 1.2 - 1.6x
- Acceleration: 1.3 - 1.7x
- Hull Integrity: 80 - 120
- Mass: 400 - 600 kg total

### Gameplay Feel
Piloting a Light Fighter feels like flying a jet fighter - responsive, fast, and requiring constant attention. Players can execute tight turns, rapid direction changes, and high-speed combat maneuvers. Best suited for experienced players who value mobility over durability.

---

## Heavy Freighter

### Overview
Massive cargo ship with powerful engines to move heavy loads. High mass provides momentum and stability but reduces agility. Ideal for long-haul transport missions.

### Flight Characteristics
- **Speed**: Slow to Moderate
- **Agility**: Poor
- **Stability**: Excellent
- **Best For**: Cargo missions, stable flight, beginners
- **Weaknesses**: Slow turning, long acceleration times

### Data Asset Configuration

**Asset Name**: `DA_HeavyFreighter`

```
Config Name: Heavy Freighter
Description: Massive cargo hauler with powerful thrust and high stability

=== PHYSICS PARAMETERS ===
Mass: 3000.0 kg
Thrust Power: 200000.0 N
Max Velocity: 600.0 m/s
Angular Thrust Power: 60000.0 N
Fuel Capacity: 2000.0
Fuel Consumption Rate: 2.5

=== FLIGHT CHARACTERISTICS ===
Acceleration Multiplier: 0.6
Rotation Speed Multiplier: 0.5
Stability Assist Strength: 0.8
```

### Recommended Ship Stats (Parts)
- Thrust Power: 1.5 - 2.0x (needs power to move mass)
- Max Velocity: 0.8 - 1.2x
- Rotation Speed: 0.6 - 0.9x
- Acceleration: 0.7 - 1.0x
- Hull Integrity: 200 - 300 (high durability)
- Mass: 2500 - 3500 kg total

### Gameplay Feel
Flying a Heavy Freighter is like piloting a semi-truck in space. Every movement requires planning ahead due to high inertia. Excellent for players who prefer methodical, strategic flight over twitch reflexes. Very forgiving due to high stability assist.

---

## Balanced Scout

### Overview
All-around ship with no extreme characteristics. Medium mass, moderate speed, balanced handling. The ideal starter ship and general-purpose vessel.

### Flight Characteristics
- **Speed**: Moderate
- **Agility**: Good
- **Stability**: Good
- **Best For**: Exploration, all-purpose missions, learning
- **Weaknesses**: No particular strengths, average at everything

### Data Asset Configuration

**Asset Name**: `DA_BalancedScout`

```
Config Name: Balanced Scout
Description: Well-rounded ship suitable for all mission types

=== PHYSICS PARAMETERS ===
Mass: 1000.0 kg
Thrust Power: 120000.0 N
Max Velocity: 1000.0 m/s
Angular Thrust Power: 60000.0 N
Fuel Capacity: 1200.0
Fuel Consumption Rate: 1.5

=== FLIGHT CHARACTERISTICS ===
Acceleration Multiplier: 1.0
Rotation Speed Multiplier: 1.0
Stability Assist Strength: 0.5
```

### Recommended Ship Stats (Parts)
- Thrust Power: 1.0 - 1.3x
- Max Velocity: 1.0 - 1.3x
- Rotation Speed: 1.0 - 1.2x
- Acceleration: 1.0 - 1.2x
- Hull Integrity: 100 - 150
- Mass: 900 - 1200 kg total

### Gameplay Feel
The Balanced Scout is the "Honda Civic" of spaceships - reliable, predictable, and comfortable. Perfect for new players learning flight mechanics or experienced players who want a no-nonsense ship. Handles well in all situations without excelling at any one task.

---

## Racing Ship

### Overview
Pure speed demon built for time trials and racing. Minimum mass, maximum velocity, stripped-down everything else. Requires expert piloting skills.

### Flight Characteristics
- **Speed**: Extremely Fast
- **Agility**: Moderate (fast but wide turns)
- **Stability**: Low (manual control required)
- **Best For**: Racing, time trials, speed runs
- **Weaknesses**: Very fragile, difficult to control, minimal assists

### Data Asset Configuration

**Asset Name**: `DA_RacingShip`

```
Config Name: Speed Demon
Description: Maximum velocity racing ship with minimal flight assistance

=== PHYSICS PARAMETERS ===
Mass: 350.0 kg
Thrust Power: 180000.0 N
Max Velocity: 2000.0 m/s
Angular Thrust Power: 50000.0 N
Fuel Capacity: 600.0
Fuel Consumption Rate: 2.0

=== FLIGHT CHARACTERISTICS ===
Acceleration Multiplier: 2.0
Rotation Speed Multiplier: 0.9
Stability Assist Strength: 0.2
```

### Recommended Ship Stats (Parts)
- Thrust Power: 1.8 - 2.5x (maximum acceleration)
- Max Velocity: 2.0 - 3.0x (boost top speed)
- Rotation Speed: 0.8 - 1.1x (turning less important)
- Acceleration: 2.0 - 2.5x (instant response)
- Hull Integrity: 50 - 80 (minimal armor)
- Mass: 300 - 400 kg total (keep it light!)

### Gameplay Feel
The Racing Ship is like strapping a rocket to a paper airplane. Insane top speed and acceleration, but turning at high velocity requires planning ahead. Minimal flight assists mean the pilot is responsible for everything - one mistake at max speed can be catastrophic. For adrenaline junkies only.

---

## Tank Ship

### Overview
Heavy assault vessel designed to take and dish out punishment. Maximum mass, maximum thrust, minimum speed. Built for head-on combat and breaking through defenses.

### Flight Characteristics
- **Speed**: Very Slow
- **Agility**: Very Poor
- **Stability**: Maximum
- **Best For**: Heavy combat, assault missions, new players
- **Weaknesses**: Cannot evade, easy target, fuel hungry

### Data Asset Configuration

**Asset Name**: `DA_TankShip`

```
Config Name: Juggernaut
Description: Heavily armored assault ship with devastating firepower

=== PHYSICS PARAMETERS ===
Mass: 4000.0 kg
Thrust Power: 300000.0 N
Max Velocity: 400.0 m/s
Angular Thrust Power: 120000.0 N
Fuel Capacity: 2500.0
Fuel Consumption Rate: 3.5

=== FLIGHT CHARACTERISTICS ===
Acceleration Multiplier: 0.4
Rotation Speed Multiplier: 0.4
Stability Assist Strength: 0.9
```

### Recommended Ship Stats (Parts)
- Thrust Power: 2.0 - 3.0x (need massive power)
- Max Velocity: 0.5 - 0.8x (speed isn't the goal)
- Rotation Speed: 0.4 - 0.7x (sluggish turning)
- Acceleration: 0.5 - 0.8x
- Hull Integrity: 300 - 500 (maximum armor)
- Shield Strength: 200 - 400
- Weapon Damage: 2.0 - 3.0x
- Mass: 3500 - 4500 kg total

### Gameplay Feel
Flying the Tank Ship is like piloting a space battleship. Every movement is deliberate and slow, but you're nearly invincible. Best for players who prefer overwhelming firepower and durability over mobility. Requires teammates or strategic positioning to be effective.

---

## Additional Archetypes

### Interceptor (Fast Combat)

**Quick Reference**:
- Mass: 600 kg
- Thrust: 140000 N
- Max Velocity: 1400 m/s
- Angular Thrust: 70000 N
- Accel Multiplier: 1.4
- Rotation Multiplier: 1.4
- Stability Assist: 0.3

**Role**: Fast-response combat ship, pursuit missions
**Feel**: Between Light Fighter and Racing Ship

---

### Stealth Scout (Low Signature)

**Quick Reference**:
- Mass: 700 kg
- Thrust: 100000 N
- Max Velocity: 1200 m/s
- Angular Thrust: 55000 N
- Accel Multiplier: 1.1
- Rotation Multiplier: 1.2
- Stability Assist: 0.6

**Role**: Reconnaissance, stealth missions, exploration
**Feel**: Quiet, methodical, precise

---

### Assault Bomber (Heavy Weapons Platform)

**Quick Reference**:
- Mass: 2500 kg
- Thrust: 180000 N
- Max Velocity: 700 m/s
- Angular Thrust: 80000 N
- Accel Multiplier: 0.7
- Rotation Multiplier: 0.6
- Stability Assist: 0.7

**Role**: Anti-capital ship, bombing runs
**Feel**: Powerful but ponderous

---

### Civilian Shuttle (Transport)

**Quick Reference**:
- Mass: 1500 kg
- Thrust: 110000 N
- Max Velocity: 800 m/s
- Angular Thrust: 50000 N
- Accel Multiplier: 0.8
- Rotation Multiplier: 0.8
- Stability Assist: 0.9

**Role**: Passenger transport, VIP missions
**Feel**: Safe, stable, comfortable

---

### Support Ship (Repair/Supply)

**Quick Reference**:
- Mass: 1800 kg
- Thrust: 130000 N
- Max Velocity: 900 m/s
- Angular Thrust: 65000 N
- Accel Multiplier: 0.9
- Rotation Multiplier: 0.9
- Stability Assist: 0.8

**Role**: Team support, repair, cargo
**Feel**: Reliable utility workhorse

---

## Customization Guide

### Creating Custom Presets

**Step 1: Choose a Base**
Start with one of the presets above that's closest to your desired feel.

**Step 2: Identify the Core Trait**
What's the ONE thing this ship does best?
- Speed? Increase Max Velocity and Thrust
- Agility? Increase Angular Thrust and Rotation Multiplier
- Durability? Increase Mass (and compensate with Thrust)
- Stability? Increase Stability Assist and decrease multipliers

**Step 3: Adjust ONE Parameter at a Time**
Change one value, test in-game, iterate. Never adjust everything at once.

**Step 4: Balance Weaknesses**
Every strength needs a weakness:
- Fast ships should be fragile (low hull integrity stats)
- Heavy ships should be slow (low velocity, rotation)
- Agile ships should sacrifice speed or armor

### Balancing Formula

For balanced gameplay, ensure:
```
Thrust-to-Weight Ratio = Thrust Power / (Mass * 9.81)
Ideal Range: 15 - 40

Example (Light Fighter):
150000 / (500 * 9.81) = 30.58 ✓ Good

Example (Heavy Freighter):
200000 / (3000 * 9.81) = 6.79 ✓ Good for heavy ship
```

### Testing Checklist

Test each preset for:
- [ ] Can complete 180-degree turn in < 3 seconds (fighters)
- [ ] Can complete 180-degree turn in < 6 seconds (freighters)
- [ ] Acceleration feels responsive (< 2 seconds to 50% max speed)
- [ ] Max velocity is reached within reasonable time (< 10 seconds)
- [ ] Ship doesn't feel "floaty" or "stuck"
- [ ] Stability assist helps but doesn't remove skill requirement
- [ ] Fun to fly for at least 10 minutes straight

---

## Physics Parameter Reference

### Mass (kg)
**What it does**: Determines inertia and momentum. Higher mass = more momentum, harder to change direction.

**Range**: 350 - 4000 kg
- 350-700: Light, agile ships
- 700-1500: Medium ships
- 1500-3000: Heavy ships
- 3000+: Super-heavy ships

**Affects**:
- Acceleration (heavier = slower)
- Turning radius (heavier = wider)
- Gravity pull (heavier = stronger)
- Collision impact

---

### Thrust Power (Newtons)
**What it does**: Force applied for forward/backward movement.

**Range**: 100000 - 300000 N
- 100000-140000: Low thrust (slow acceleration)
- 140000-180000: Medium thrust
- 180000-250000: High thrust (fast acceleration)
- 250000+: Extreme thrust (instant response)

**Formula**: Acceleration (m/s²) = Thrust / Mass

**Affects**:
- Forward/backward acceleration
- Ability to overcome gravity
- Fuel consumption (higher = more fuel used)

---

### Max Velocity (m/s)
**What it does**: Speed limit for the ship. Safety cutoff to prevent excessive speed.

**Range**: 400 - 2000 m/s
- 400-700: Slow (freighters, tanks)
- 700-1200: Medium (scouts, balanced)
- 1200-1600: Fast (fighters)
- 1600-2000+: Extreme (racers)

**Affects**:
- Top speed in any direction
- Time to cross distances
- Difficulty of control at high speeds

---

### Angular Thrust Power (Newtons)
**What it does**: Rotational force for pitch/yaw/roll.

**Range**: 40000 - 120000 N
- 40000-60000: Slow turning
- 60000-80000: Medium turning
- 80000-100000: Fast turning
- 100000+: Very fast turning

**Affects**:
- Pitch speed (nose up/down)
- Yaw speed (nose left/right)
- Roll speed (barrel roll)
- Maneuverability in combat

---

### Acceleration Multiplier
**What it does**: Scales how quickly thrust input translates to actual thrust force.

**Range**: 0.4 - 2.0
- 0.4-0.7: Gradual, floaty (freighters)
- 0.8-1.2: Normal, responsive
- 1.3-1.7: Quick, snappy (fighters)
- 1.8-2.0+: Instant, twitchy (racers)

**Affects**:
- Input responsiveness
- "Weight" feeling of ship
- How quickly ship responds to throttle changes

---

### Rotation Speed Multiplier
**What it does**: Scales rotational input speed.

**Range**: 0.4 - 1.5
- 0.4-0.6: Slow, deliberate turns
- 0.7-1.0: Normal turning
- 1.1-1.3: Fast, agile turns
- 1.4-1.5+: Very fast, difficult to control

**Affects**:
- How quickly ship rotates from input
- Combat maneuverability
- Difficulty of precision aiming

---

### Stability Assist Strength
**What it does**: Auto-stabilization force that counteracts unwanted rotation.

**Range**: 0.0 - 1.0
- 0.0-0.3: Minimal assist (manual flight)
- 0.4-0.6: Moderate assist (balanced)
- 0.7-0.9: Strong assist (beginner-friendly)
- 1.0: Maximum assist (auto-leveling)

**Affects**:
- How much ship auto-corrects rotation
- Difficulty of flight
- Feeling of "weightlessness" vs "grounded"

**Recommendation**: Higher for heavy ships, lower for agile ships

---

### Fuel Capacity & Consumption
**What it does**: (Future feature) Limits flight time before refueling needed.

**Capacity Range**: 600 - 2500
**Consumption Range**: 1.0 - 3.5 per second

**Balance**:
- Light ships: Low capacity, low consumption (600 / 1.2 = 500 seconds flight)
- Heavy ships: High capacity, high consumption (2500 / 3.5 = 714 seconds flight)

---

## Comparison Table

Quick reference for all five main presets:

| Parameter | Light Fighter | Heavy Freighter | Balanced Scout | Racing Ship | Tank Ship |
|-----------|--------------|-----------------|----------------|-------------|-----------|
| **Mass** | 500 kg | 3000 kg | 1000 kg | 350 kg | 4000 kg |
| **Thrust** | 150000 N | 200000 N | 120000 N | 180000 N | 300000 N |
| **Max Vel** | 1500 m/s | 600 m/s | 1000 m/s | 2000 m/s | 400 m/s |
| **Angular** | 75000 N | 60000 N | 60000 N | 50000 N | 120000 N |
| **Accel Mult** | 1.5 | 0.6 | 1.0 | 2.0 | 0.4 |
| **Rot Mult** | 1.3 | 0.5 | 1.0 | 0.9 | 0.4 |
| **Stability** | 0.4 | 0.8 | 0.5 | 0.2 | 0.9 |
| **T/W Ratio** | 30.6 | 6.8 | 12.2 | 52.4 | 7.6 |
| **Fuel Cap** | 800 | 2000 | 1200 | 600 | 2500 |
| **Fuel Rate** | 1.2/s | 2.5/s | 1.5/s | 2.0/s | 3.5/s |

---

## Usage in Blueprints

### Example: Apply Preset on BeginPlay

```
Event BeginPlay (BP_PlayerShip)
-> Get Ship Customization Component (Self)
-> Apply Physics Config (DA_LightFighter)
```

### Example: Preset Selector UI

```
On Dropdown Selection Changed:
-> Switch on String (Dropdown.GetSelectedOption)
   -> "Light Fighter": Apply Physics Config (DA_LightFighter)
   -> "Heavy Freighter": Apply Physics Config (DA_HeavyFreighter)
   -> "Balanced Scout": Apply Physics Config (DA_BalancedScout)
   -> "Racing Ship": Apply Physics Config (DA_RacingShip)
   -> "Tank Ship": Apply Physics Config (DA_TankShip)
```

---

**End of Physics Presets Guide**

For more information on implementing these presets in Blueprints, see [SHIP_CUSTOMIZATION_BLUEPRINT_GUIDE.md](SHIP_CUSTOMIZATION_BLUEPRINT_GUIDE.md).
