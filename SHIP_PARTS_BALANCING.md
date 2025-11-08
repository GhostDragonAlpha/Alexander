# Ship Parts Balancing Guide

## Table of Contents
- [Balancing Philosophy](#balancing-philosophy)
- [Stat System Overview](#stat-system-overview)
- [Category Breakdown](#category-breakdown)
- [Rarity Tiers](#rarity-tiers)
- [Progression Curve](#progression-curve)
- [Cost Calculation](#cost-calculation)
- [Balance Testing Recommendations](#balance-testing-recommendations)

---

## Balancing Philosophy

### Core Principles

1. **Tradeoffs Over Power Creep**: Higher rarity parts should offer **specialization**, not strict upgrades
2. **Meaningful Choices**: Players should make decisions based on playstyle, not obvious "best" options
3. **Build Diversity**: Multiple viable builds for different scenarios (racing, combat, exploration)
4. **Progression Feel**: Players should feel rewarded for unlocking new parts without invalidating previous choices

### Design Goals

- **No Strictly Superior Parts**: Every part should have at least one meaningful tradeoff
- **Synergy Opportunities**: Parts should work together to create specialized builds
- **Physics Integration**: Stats should interact meaningfully with flight physics presets
- **Accessibility**: Common/Uncommon parts viable for core gameplay; Rare+ for optimization

---

## Stat System Overview

### Stat Categories

| Stat Name | Effect | Additive/Multiplicative | Typical Range |
|-----------|--------|------------------------|---------------|
| `ThrustPower` | Forward thrust magnitude | Multiplier | 0.8 - 1.6 |
| `MaxVelocity` | Top speed limit | Multiplier | 0.8 - 1.6 |
| `RotationSpeed` | Angular velocity (pitch/yaw/roll) | Multiplier | 0.8 - 1.8 |
| `Acceleration` | Rate of velocity change | Multiplier | 0.7 - 1.5 |
| `HullIntegrity` | Health pool | Additive | 100 - 300 |
| `ShieldStrength` | Shield capacity | Additive | 0 - 250 |
| `WeaponDamage` | Weapon output | Multiplier | 0 - 3.0 |
| `EnergyCapacity` | Energy pool size | Additive | 0 - 100 |
| `EnergyRegenRate` | Energy regen per second | Additive | -8 to +12 |
| `Mass` | Ship mass (affects handling) | Additive | 0 - 500 |

### Stat Interaction with Physics

```
Effective Thrust = BaseThrustPower × ThrustPower × (1 / Mass)
Effective Max Speed = BaseMaxVelocity × MaxVelocity
Effective Rotation = BaseRotationSpeed × RotationSpeed × (1 / Mass)
```

**Key Insight**: Mass affects both thrust and rotation effectiveness, creating natural tradeoffs.

---

## Category Breakdown

### Engine (5 parts)

**Primary Stats**: ThrustPower, MaxVelocity, Acceleration
**Secondary Stats**: Mass, EnergyRegenRate

| Part | Role | Tradeoff |
|------|------|----------|
| **Mk1 Ion Drive** | Baseline | Balanced, no specialization |
| **Velocity V3** | Speed Racer | +40% max velocity, -15% thrust, lower mass, energy drain |
| **Titan Mk2** | Heavy Hauler | +60% thrust, -20% max velocity, heavy mass |
| **Quantum Balance** | All-Rounder | +25% all stats, moderate mass, slight energy bonus |
| **Phantom Drive X-7** | Elite Speed | +50% thrust, +60% velocity, low mass, expensive |

**Balancing Notes**:
- Speed build: Phantom + Light Hull + Racing Thrusters = Glass cannon
- Tank build: Titan + Heavy Hull + Precision Thrusters = Slow but unstoppable
- Balanced build: Quantum + Reinforced Hull = Jack of all trades

### Thrusters (4 parts)

**Primary Stats**: RotationSpeed, Acceleration
**Secondary Stats**: Mass, EnergyRegenRate

| Part | Role | Tradeoff |
|------|------|----------|
| **Standard RCS** | Baseline | No bonuses, light |
| **Mongoose Agility** | Racing | +50% rotation, energy drain, very light |
| **Precision Matrix** | Industrial | +30% accel, slower rotation, adds stability |
| **Eclipse Racing** | Elite Agility | +80% rotation, highest energy drain |

**Balancing Notes**:
- High rotation speed pairs well with low-mass builds
- Precision thrusters benefit heavy ships that need stability
- Energy drain can be offset by high-capacity power cores

### Hull (4 parts)

**Primary Stats**: HullIntegrity, Mass
**Secondary Stats**: Acceleration (negative correlation with mass)

| Part | Role | Tradeoff |
|------|------|----------|
| **Aluminum Composite** | Lightweight | Minimal armor (100 HP), very light (200 mass) |
| **Reinforced Titanium** | Medium Armor | +80% armor, +75% mass |
| **Dreadnought Plating** | Heavy Tank | +200% armor, +150% mass, -30% accel |
| **Carbon Nanoweave** | Racing Shell | +20% armor, -40% mass, +15% accel, fragile |

**Balancing Notes**:
- Mass heavily impacts maneuverability
- Combat builds want 250+ HP total
- Racing builds want <250 total mass

### Shield (4 parts)

**Primary Stats**: ShieldStrength, EnergyCapacity, EnergyRegenRate
**Secondary Stats**: Mass

| Part | Role | Tradeoff |
|------|------|----------|
| **No Shield** | Pure Performance | No protection, zero mass/energy cost |
| **Mk1 Projector** | Basic Shield | 100 shield HP, moderate energy use |
| **Fortress Array** | Tank Shield | 250 shield HP, high capacity, slow regen |
| **Adaptive Regenerator** | Quick Recovery | 150 shield HP, very fast regen, lower capacity |

**Balancing Notes**:
- Shields add mass and drain energy
- Fortress Shield for sustained combat
- Adaptive Shield for hit-and-run tactics
- No shield viable for racing/exploration

### Weapon (5 parts)

**Primary Stats**: WeaponDamage
**Secondary Stats**: Mass, EnergyRegenRate

| Part | Role | Tradeoff |
|------|------|----------|
| **No Weapons** | Peaceful | Zero damage, zero cost |
| **Pulse Laser** | Standard | 1.0× damage, moderate energy drain |
| **Railgun Mk5** | Heavy Hitter | 2.0× damage, heavy, slow fire rate (implied) |
| **Plasma Accelerator** | High DPS | 3.0× damage, extreme energy drain |
| **Seeker Missiles** | Tracking | 2.5× damage, ammo-based (future mechanic) |

**Balancing Notes**:
- Weapons compete with shields for energy budget
- Plasma weapon requires high energy regen build
- Unarmed builds maximize performance

### Utility (4 parts)

**Primary Stats**: Varies by utility type
**Secondary Stats**: Mass, EnergyCapacity

| Part | Role | Tradeoff |
|------|------|----------|
| **Standard Scanner** | Basic Detection | Baseline, minimal cost |
| **Long-Range Sensors** | Exploration | Extended range, heavier, energy cost |
| **Stealth ECM** | Combat Utility | Stealth capability (future), high energy drain |
| **Auto-Repair Nanobots** | Survivability | Hull regen (future), +50 HP, heavy |

**Balancing Notes**:
- Utility parts are role-specific
- Exploration builds use Long-Range Sensors
- Combat builds may use ECM or Auto-Repair

### Wings & Cockpit (3 parts each)

**Primary Stats**: RotationSpeed, Mass
**Effect**: Primarily cosmetic with minor stat adjustments

---

## Rarity Tiers

### Common (White)
- **Purpose**: Starting equipment, always available
- **Cost**: Free (0 credits)
- **Level Requirement**: 1
- **Balance**: Baseline performance, no specialization

**Examples**: Mk1 Ion Drive, Standard RCS, No Shield

### Uncommon (Green)
- **Purpose**: First upgrades, introduce specialization
- **Cost**: 1,500 - 5,000 credits
- **Level Requirement**: 3-6
- **Balance**: +10-40% in primary stat, minor tradeoffs

**Examples**: Velocity V3 Engine, Mongoose Agility Thrusters, Basic Shield

### Rare (Blue)
- **Purpose**: Meaningful specialization, clear tradeoffs
- **Cost**: 5,000 - 10,000 credits
- **Level Requirement**: 5-9
- **Balance**: +30-60% in primary stat, significant tradeoffs

**Examples**: Titan Mk2 Engine, Dreadnought Hull, Fortress Shield

### Epic (Purple)
- **Purpose**: High-performance specialized builds
- **Cost**: 10,000 - 20,000 credits
- **Level Requirement**: 8-13
- **Balance**: +40-80% in primary stat, extreme specialization

**Examples**: Quantum Balance Drive, Eclipse Racing Thrusters, Plasma Accelerator

### Legendary (Gold)
- **Purpose**: Aspirational endgame items
- **Cost**: 20,000 - 25,000 credits
- **Level Requirement**: 12-18
- **Balance**: Best-in-class for specific role

**Examples**: Phantom Drive X-7, Seeker Missiles

---

## Progression Curve

### Credits Earnings (Estimated)

| Activity | Credits Earned |
|----------|---------------|
| Tutorial Course Complete | 500 |
| Easy Course Complete | 1,000 |
| Medium Course Complete | 2,000 |
| Hard Course Complete | 3,500 |
| First Place Bonus | +50% |
| Perfect Run Bonus | +25% |

### Unlock Timeline (Typical Player)

| Level | Cumulative Credits | Affordable Rarity | Example Parts |
|-------|-------------------|-------------------|---------------|
| 1 | 0 | Common | All starters |
| 3 | ~3,000 | Uncommon | 1-2 uncommon parts |
| 5 | ~8,000 | Uncommon/Rare | First rare part |
| 8 | ~18,000 | Rare/Epic | 2-3 rare parts + 1 epic |
| 12 | ~35,000 | Epic/Legendary | Multiple epic parts |
| 15+ | ~60,000+ | Legendary | Full legendary build |

### Recommended Purchase Order

**Phase 1 (Level 1-5)**: Unlock versatility
- Shield (Basic) - Survivability
- Engine (Speed or Power) - Performance boost
- Thrusters (Agility) - Handling improvement

**Phase 2 (Level 5-10)**: Specialize
- Hull upgrade matching playstyle
- Epic engine or thrusters
- Weapon or utility based on needs

**Phase 3 (Level 10+)**: Optimize
- Legendary parts for main build
- Alternative parts for different courses
- Cosmetic skins

---

## Cost Calculation

### Formula

```
Base Cost = Rarity Multiplier × Category Weight × Stat Budget

Rarity Multipliers:
- Common: 0 (free starter items)
- Uncommon: 1.0
- Rare: 2.0
- Epic: 4.0
- Legendary: 8.0

Category Weights:
- Engine: 1000 (core performance)
- Thrusters: 900
- Hull: 800
- Shield: 850
- Weapon: 950
- Utility: 750
- Wings/Cockpit: 600 (minor impact)

Stat Budget = Sum of normalized stat bonuses
```

### Example Calculation

**Phantom Drive X-7** (Legendary Engine):
```
Stat Budget =
  ThrustPower: +0.5 → 50 points
  MaxVelocity: +0.6 → 60 points
  Acceleration: +0.4 → 40 points
  Mass: -20 (low) → +30 points (negative mass is good)
  Energy: +30 capacity + 3 regen → 20 points

Total: 200 points

Base Cost = 8.0 × 1000 × (200/100) = 16,000
Adjusted for aspirational value: 25,000 credits
```

---

## Balance Testing Recommendations

### Test Scenarios

1. **Racing Performance Test**
   - Lightweight build vs balanced build
   - Measure lap times on 3 courses (easy/medium/hard)
   - Target: <10% variance in lap times for different builds

2. **Combat Durability Test**
   - Tank build vs agile build vs balanced
   - Sustained damage over 60 seconds
   - Target: No build >2× effective HP of others

3. **Energy Budget Test**
   - Shield + Weapon + High-thrust engine
   - Energy should deplete in 30-60 seconds of max use
   - Energy regen should restore in 20-40 seconds idle

4. **Mass Impact Test**
   - Lightest possible build (~300 mass)
   - Heaviest possible build (~1200 mass)
   - Rotation speed should differ by 2-3×

### Metrics to Track

| Metric | Ideal Range | Red Flag |
|--------|-------------|----------|
| Lap Time Variance | <15% | >25% |
| Cost/Performance Ratio | Linear with rarity | Exponential |
| Build Diversity (ranked matches) | 5+ viable builds | <3 dominant builds |
| Legendary Unlock Rate | 20% at level 15 | 80% at level 10 |

### Iteration Process

1. **Playtest** with 3-5 different builds
2. **Measure** performance in racing/combat scenarios
3. **Identify** dominant strategies or useless parts
4. **Adjust** stats by 10-20% increments
5. **Re-test** and iterate

---

## Integration with Physics Presets

### Easy Mode Synergy
- **Best Parts**: Balanced builds (Quantum Engine, Standard thrusters)
- **Why**: Smoothing/assists mask stat differences
- **Avoid**: Extreme specialization (wasted on assists)

### Medium Mode Synergy
- **Best Parts**: Specialized racing or combat builds
- **Why**: Player skill can leverage stat advantages
- **Avoid**: Heavy tank builds (less forgiving than Easy)

### Hard Mode Synergy
- **Best Parts**: Extreme min-max builds (Phantom + Carbon Hull)
- **Why**: No safety nets, pure performance matters
- **Avoid**: Suboptimal compromises

---

## Future Considerations

### Upgrade System
- **Concept**: Spend credits to upgrade existing parts (+10% stats per level, max 3 levels)
- **Balance**: Upgrades cheaper than buying new parts, but limited ceiling

### Set Bonuses
- **Concept**: Equip 3+ parts from same "manufacturer" for set bonus
- **Example**: "Velocity Industries" - 3 parts = +10% all speed stats
- **Balance**: Encourage thematic builds without mandating them

### Dynamic Pricing
- **Concept**: Part costs vary based on player performance/usage
- **Example**: If player wins 5 races with Phantom Drive, cost increases 20%
- **Balance**: Encourages build diversity, prevents meta-stagnation

---

## Conclusion

This balancing system prioritizes:
1. ✅ **Player choice** over optimal paths
2. ✅ **Tradeoffs** over power creep
3. ✅ **Progression satisfaction** over grind
4. ✅ **Build diversity** over meta dominance

Regular playtesting and iteration will be essential to maintain balance as more content is added.

---

**Last Updated**: Phase 2 Initial Content Creation
**Next Review**: After first playtest session
**Maintained By**: Ship Customization Team
