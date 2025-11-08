# Physics Configuration Creation Guide

**Phase 2 - Ship Physics Configuration System**

This guide provides step-by-step instructions for creating, testing, and tuning physics configuration data assets in Unreal Engine.

---

## Table of Contents

1. [Quick Start](#quick-start)
2. [Understanding Physics Configs](#understanding-physics-configs)
3. [Creating Configs in Unreal](#creating-configs-in-unreal)
4. [Testing Configs in PIE](#testing-configs-in-pie)
5. [Tuning Based on Feel](#tuning-based-on-feel)
6. [Common Issues and Solutions](#common-issues-and-solutions)
7. [Advanced Configuration](#advanced-configuration)

---

## Quick Start

### Automated Creation (Recommended)

1. **Open Unreal Editor**
2. **Run the generator script**:
   ```bash
   python generate_physics_assets.py
   ```

3. **In Unreal Editor Python Console**:
   ```python
   exec(open('generated_create_physics_assets.py').read())
   ```

4. **Verify assets created**:
   - Navigate to `Content/Data/PhysicsConfigs/`
   - You should see 10 data asset files (DA_LightFighter, DA_HeavyFreighter, etc.)

### Manual Creation (If Automation Fails)

See [Manual Asset Creation](#manual-asset-creation) section below.

---

## Understanding Physics Configs

### What is a Physics Config?

A `UShipPhysicsConfig` data asset defines all the physics parameters for a ship archetype. It includes:

- **Mass**: How heavy the ship is
- **Thrust Power**: How powerful the engines are
- **Max Velocity**: Top speed limit
- **Angular Thrust**: How fast it can rotate
- **Multipliers**: Fine-tuning for responsiveness
- **Stability Assist**: Auto-stabilization strength

### The 10 Ship Archetypes

| Archetype | Mass | Speed | Agility | Use Case |
|-----------|------|-------|---------|----------|
| **Light Fighter** | 500 kg | Very Fast | Excellent | Combat, Racing |
| **Heavy Freighter** | 3000 kg | Slow | Poor | Cargo, Beginners |
| **Balanced Scout** | 1000 kg | Medium | Good | All-Purpose |
| **Speed Demon** | 350 kg | Extreme | Moderate | Racing |
| **Juggernaut** | 4000 kg | Very Slow | Very Poor | Heavy Combat |
| **Interceptor** | 600 kg | Fast | Excellent | Fast Combat |
| **Stealth Scout** | 700 kg | Fast | Good | Reconnaissance |
| **Assault Bomber** | 2500 kg | Medium | Poor | Bombing Runs |
| **Civilian Shuttle** | 1500 kg | Medium | Moderate | Transport |
| **Support Ship** | 1800 kg | Medium | Moderate | Utility |

---

## Creating Configs in Unreal

### Method 1: Automated Python Script

**Prerequisites**:
- Unreal Editor 5.0+
- Python support enabled
- PhysicsConfigs.json exists

**Steps**:

1. Generate the Unreal Python script:
   ```bash
   python generate_physics_assets.py
   ```

2. Open Unreal Editor

3. Open Python Console:
   - Window > Developer Tools > Python Console

4. Execute the generated script:
   ```python
   exec(open('generated_create_physics_assets.py').read())
   ```

5. Wait for completion (should see "Created X/10 assets")

6. Verify in Content Browser:
   - Navigate to `/Game/Data/PhysicsConfigs/`
   - Should see all 10 DA_ assets

### Method 2: Manual Asset Creation

**For Each Configuration**:

1. **Create Data Asset**:
   - Right-click in `Content/Data/PhysicsConfigs/`
   - Select: Miscellaneous > Data Asset
   - Choose class: `ShipPhysicsConfig`
   - Name it (e.g., `DA_LightFighter`)

2. **Open the asset** (double-click)

3. **Set Config Info**:
   - Config Name: "Light Fighter" (display name)
   - Description: "Fast and agile combat ship..."

4. **Set Physics Parameters**:

   **Light Fighter Example**:
   ```
   Mass: 500.0
   Thrust Power: 150000.0
   Max Velocity: 1500.0
   Angular Thrust Power: 75000.0
   Fuel Capacity: 800.0
   Fuel Consumption Rate: 1.2
   ```

5. **Set Flight Characteristics**:
   ```
   Acceleration Multiplier: 1.5
   Rotation Speed Multiplier: 1.3
   Stability Assist Strength: 0.4
   ```

6. **Save the asset** (Ctrl+S)

7. **Repeat for all 10 configurations**

**Reference**: See `MANUAL_PHYSICS_ASSET_CREATION.md` for exact values for all configs.

---

## Testing Configs in PIE

### Setup Test Environment

1. **Create Test Level**:
   - Use existing VRTemplateMap or create new flight test level
   - Ensure ship actor is placeable

2. **Add Physics Config Component**:
   - Open your ship Blueprint (BP_PlayerShip)
   - Add `ShipCustomizationComponent` (if not present)
   - Set default physics config

### Quick PIE Test

1. **Set Test Config**:
   ```
   Event BeginPlay (BP_PlayerShip)
   -> Get Ship Customization Component
   -> Apply Physics Config (select config to test)
   ```

2. **Launch PIE** (Alt+P)

3. **Test Flight Characteristics**:
   - Press `W` to accelerate - Does it feel responsive?
   - Check max speed - Is it reaching expected velocity?
   - Test rotation - Does it turn at the right speed?
   - Release controls - Does stability assist kick in?

4. **Take Notes**:
   - Too fast/slow?
   - Too responsive/sluggish?
   - Stable or floaty?

### Automated Testing

Run the test script to measure actual performance:

```bash
python test_physics_configs.py
```

This generates `physics_config_test_report.md` with:
- Expected vs actual acceleration
- Time to max velocity
- Rotation speed measurements
- Pass/fail indicators

### Performance Validation

Use the utility actor to validate configs:

1. **Place PhysicsConfigUtility** in level

2. **In Blueprint or Python**:
   ```python
   import unreal

   # Load a config
   config = unreal.EditorAssetLibrary.load_asset("/Game/Data/PhysicsConfigs/DA_LightFighter")

   # Validate it
   validation = unreal.PhysicsConfigUtility.validate_physics_config(config)

   print(f"Valid: {validation.is_valid}")
   print(f"T/W Ratio: {validation.thrust_to_weight_ratio}")
   print(f"Warnings: {validation.warnings}")
   ```

---

## Tuning Based on Feel

### The Tuning Process

Physics configs are designed to be tweaked based on how they *feel* in gameplay, not just raw numbers.

#### Step 1: Identify the Problem

**Ship feels too...**

| Problem | Likely Cause | Solution |
|---------|--------------|----------|
| **Slow** | Low thrust or high mass | Increase ThrustPower or decrease Mass |
| **Sluggish** | Low acceleration multiplier | Increase AccelerationMultiplier |
| **Twitchy** | High acceleration/rotation multipliers | Decrease multipliers |
| **Floaty** | Low stability assist | Increase StabilityAssistStrength |
| **Uncontrollable** | Too high stability assist | Decrease StabilityAssistStrength |
| **Won't turn** | Low angular thrust | Increase AngularThrustPower |
| **Spins too fast** | High angular thrust + high rotation mult | Decrease one or both |

#### Step 2: Make Small Adjustments

**IMPORTANT**: Only change ONE parameter at a time by small amounts.

**Good adjustments**:
- Mass: ±50-100 kg
- Thrust Power: ±5000-10000 N
- Max Velocity: ±50-100 m/s
- Multipliers: ±0.1
- Stability Assist: ±0.05

**Bad adjustments**:
- Changing multiple parameters at once
- Doubling or halving values
- Not testing between changes

#### Step 3: Test Immediately

After EVERY change:
1. Save the asset
2. Launch PIE
3. Fly for at least 30 seconds
4. Take notes on what changed

#### Step 4: Iterate

Repeat until the ship feels right. This may take 5-20 iterations.

### Feel Guidelines

**Light Fighter should feel**:
- Instant response to input
- Fast acceleration
- Quick turns
- Slight drift when changing direction
- Requires constant pilot input

**Heavy Freighter should feel**:
- Slow to respond
- Takes time to build speed
- Wide, slow turns
- Lots of momentum
- Very stable, auto-corrects

**Balanced Scout should feel**:
- Predictable and controllable
- Moderate speed and agility
- Comfortable for long flights
- Not too fast, not too slow

---

## Common Issues and Solutions

### Issue: Config values not applying in PIE

**Symptoms**:
- Ship behaves the same regardless of config
- Changes to data asset have no effect

**Solutions**:
1. Verify config is being applied in BeginPlay
2. Check ShipCustomizationComponent exists
3. Ensure ApplyPhysicsConfig is being called
4. Print debug log to confirm config loaded
5. Try closing/reopening editor (asset cache issue)

### Issue: Ship immediately spins out of control

**Symptoms**:
- Ship starts spinning on PIE launch
- Uncontrollable rotation
- Can't stabilize

**Solutions**:
1. Check StabilityAssistStrength (should be > 0.3 for most ships)
2. Reduce RotationSpeedMultiplier
3. Reduce AngularThrustPower
4. Ensure flight controller is properly initialized

### Issue: Ship won't move at all

**Symptoms**:
- Pressing throttle does nothing
- Ship frozen in place

**Solutions**:
1. Verify ThrustPower > 0
2. Check if physics simulation enabled on root component
3. Ensure ship has primitive component with physics
4. Check for collision blocking movement
5. Verify input is being received (print throttle value)

### Issue: Ship reaches max velocity instantly

**Symptoms**:
- Pressing W immediately hits max speed
- No acceleration curve

**Solutions**:
1. Reduce AccelerationMultiplier
2. Increase Mass (more inertia)
3. Add thrust ramping in flight controller
4. Check for infinite thrust bug

### Issue: Ship drifts indefinitely

**Symptoms**:
- Ship never stops moving
- No friction or drag
- Keeps velocity forever

**Solutions**:
1. This is actually correct for space flight!
2. If you want drag, add it to flight controller
3. Increase StabilityAssistStrength for auto-stop behavior
4. Implement velocity dampening in Blueprint

### Issue: T/W ratio warnings

**Symptoms**:
- Validation shows T/W ratio out of range
- Ship performs poorly

**Solutions**:

**If T/W < 5**:
- Ship may struggle to overcome gravity
- Increase ThrustPower
- Decrease Mass
- This is OK for very heavy ships

**If T/W > 100**:
- Ship may be overpowered
- Decrease ThrustPower
- Increase Mass
- This is OK for racing ships

### Issue: Inconsistent behavior between PIE and packaged build

**Symptoms**:
- Works in PIE, broken in build
- Different physics in standalone

**Solutions**:
1. Ensure configs are packaged (check Project Settings > Packaging)
2. Verify config references are hard references, not soft
3. Test in Standalone Game mode (Alt+S) before packaging
4. Check for editor-only code affecting physics

---

## Advanced Configuration

### Custom Archetypes

To create your own ship archetype:

1. **Start with closest preset**
   - Identify which existing config is closest to your goal

2. **Duplicate the asset**
   - Right-click existing config > Duplicate
   - Rename to your archetype name

3. **Identify core characteristic**
   - Speed-focused? Increase MaxVelocity and ThrustPower
   - Agility-focused? Increase AngularThrust and rotation multiplier
   - Tank-focused? Increase Mass and StabilityAssist

4. **Balance opposites**
   - High speed → Low stability or high fuel consumption
   - High mass → Low max velocity or slower rotation
   - High agility → Lower thrust or higher fuel consumption

5. **Calculate T/W ratio**
   ```
   T/W = ThrustPower / (Mass * 9.81)
   ```
   - Aim for 10-30 for balanced ships
   - 30-60 for fast ships
   - 5-15 for heavy ships

### Dynamic Config Switching

Allow players to switch configs mid-flight:

```cpp
// In Blueprint or C++
UFUNCTION(BlueprintCallable)
void SwitchPhysicsConfig(UShipPhysicsConfig* NewConfig)
{
    if (ShipCustomizationComponent && NewConfig)
    {
        ShipCustomizationComponent->ApplyPhysicsConfig(NewConfig);

        // Optionally smooth the transition
        bTransitioningConfig = true;
        TransitionTimer = 0.0f;
        TransitionDuration = 2.0f; // 2 second blend
    }
}
```

### Config Modifiers

Apply percentage modifiers to configs at runtime:

```cpp
// Boost mode: 1.5x thrust for 5 seconds
UFUNCTION(BlueprintCallable)
void ActivateBoost()
{
    if (ShipCustomizationComponent)
    {
        UShipPhysicsConfig* Config = ShipCustomizationComponent->GetCurrentConfig();
        Config->ThrustPower *= 1.5f;

        // Reset after delay
        GetWorld()->GetTimerManager().SetTimer(
            BoostTimerHandle,
            this,
            &ThisClass::DeactivateBoost,
            5.0f,
            false
        );
    }
}
```

### Save/Load Custom Configs

Allow players to create and save custom tuning:

```cpp
// Save player's custom config
UFUNCTION(BlueprintCallable)
void SaveCustomConfig(const FString& ConfigName)
{
    UShipPhysicsConfig* CustomConfig = DuplicateObject<UShipPhysicsConfig>(
        CurrentConfig,
        GetTransientPackage()
    );

    CustomConfig->ConfigName = FText::FromString(ConfigName);

    // Save to player's save game
    // (Implement save game system)
}
```

---

## Appendix: Parameter Reference

### Mass (kg)

**What it does**: Determines inertia and momentum.

**Recommended ranges**:
- Light ships: 300-800 kg
- Medium ships: 800-2000 kg
- Heavy ships: 2000-5000 kg

**Affects**:
- Acceleration (heavier = slower)
- Turning radius (heavier = wider)
- Collision impact
- Fuel efficiency

---

### Thrust Power (Newtons)

**What it does**: Force applied for linear movement.

**Recommended ranges**:
- Low power: 80000-120000 N
- Medium power: 120000-180000 N
- High power: 180000-350000 N

**Formula**: Acceleration = ThrustPower / Mass

**Affects**:
- Forward/backward acceleration
- Ability to overcome gravity
- Fuel consumption

---

### Max Velocity (m/s)

**What it does**: Speed limit (safety cap).

**Recommended ranges**:
- Slow: 400-700 m/s
- Medium: 700-1200 m/s
- Fast: 1200-1800 m/s
- Extreme: 1800-3000 m/s

**Note**: This is a cap, not guaranteed speed. Ship must have enough thrust to reach it.

---

### Angular Thrust Power (Newtons)

**What it does**: Rotational force for pitch/yaw/roll.

**Recommended ranges**:
- Slow turning: 40000-60000 N
- Medium turning: 60000-80000 N
- Fast turning: 80000-120000 N

**Affects**:
- Rotation speed
- Combat maneuverability
- Precision control

---

### Acceleration Multiplier

**What it does**: Scales thrust responsiveness.

**Recommended ranges**:
- Sluggish: 0.4-0.7
- Normal: 0.8-1.2
- Responsive: 1.3-1.7
- Instant: 1.8-2.5

**Use**: Makes ship feel heavier or lighter than its actual mass.

---

### Rotation Speed Multiplier

**What it does**: Scales rotational input speed.

**Recommended ranges**:
- Slow: 0.4-0.7
- Normal: 0.8-1.1
- Fast: 1.2-1.5

**Use**: Fine-tune how quickly ship responds to rotation input.

---

### Stability Assist Strength

**What it does**: Auto-stabilization force (0 = none, 1 = max).

**Recommended ranges**:
- Manual flight: 0.0-0.3
- Balanced: 0.4-0.6
- Assisted: 0.7-0.9
- Auto-level: 0.9-1.0

**Use**:
- Higher = easier to fly (beginner-friendly)
- Lower = more skill required (expert)

---

## Conclusion

Physics configuration is an iterative process. Don't expect to get it perfect on the first try. Test, tune, and test again until each ship archetype feels distinct and fun to fly.

**Remember**:
1. Change one parameter at a time
2. Test immediately after each change
3. Take notes on what feels good/bad
4. Use the validation tools to check balance
5. Get player feedback when possible

For additional help, see:
- `SHIP_PHYSICS_PRESETS_EXAMPLES.md` - Detailed preset documentation
- `test_physics_configs.py` - Automated testing
- `PhysicsConfigUtility.h` - C++ validation utilities

---

**Good luck tuning your ships!**
