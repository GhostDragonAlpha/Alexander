# Ship Parts Stat Comparison

Visual comparison of all ship parts by category, showing stat tradeoffs and build options.

---

## Engine Comparison

| Part Name | Rarity | Cost | Thrust | Max Vel | Accel | Mass | Energy | Best For |
|-----------|--------|------|--------|---------|-------|------|--------|----------|
| **Mk1 Ion Drive** | Common | 0 | 1.0× | 1.0× | 1.0× | 100 | 0/0 | Starter/Balanced |
| **Velocity V3** | Uncommon | 2,500 | 0.85× | **1.4×** | 1.2× | 75 | 0/-2 | Speed Racing |
| **Titan Mk2** | Rare | 5,000 | **1.6×** | 0.8× | **1.5×** | 180 | 0/-3 | Heavy Hauling |
| **Quantum Balance** | Epic | 10,000 | 1.25× | 1.25× | 1.25× | 110 | +20/+1 | All-Round Pro |
| **Phantom Drive X-7** | Legendary | 25,000 | **1.5×** | **1.6×** | **1.4×** | 80 | +30/+3 | Elite Speed |

### Build Recommendations

**Speed Demon Build**:
- Engine: Phantom Drive X-7 (max velocity)
- Hull: Carbon Nanoweave (low mass)
- Thrusters: Eclipse Racing (max agility)
- **Result**: Fastest ship, fragile

**Tank Build**:
- Engine: Titan Mk2 (high thrust for mass)
- Hull: Dreadnought Plating (max armor)
- Shield: Fortress Array (max shields)
- **Result**: Slowest, most durable

**Balanced Pro Build**:
- Engine: Quantum Balance (all-round)
- Hull: Reinforced Titanium (moderate armor)
- Thrusters: Precision Matrix (stability)
- **Result**: Jack of all trades

---

## Thrusters Comparison

| Part Name | Rarity | Cost | Rotation | Accel | Mass | Energy | Best For |
|-----------|--------|------|----------|-------|------|--------|----------|
| **Standard RCS** | Common | 0 | 1.0× | 1.0× | 50 | 0/0 | Baseline |
| **Mongoose Agility** | Uncommon | 3,000 | **1.5×** | 1.1× | 35 | 0/-1.5 | Racing Turns |
| **Precision Matrix** | Rare | 6,000 | 0.9× | 1.3× | 90 | +10/+0.5 | Heavy Ships |
| **Eclipse Racing** | Epic | 12,000 | **1.8×** | 1.2× | 40 | +15/-2 | Elite Racing |

### Rotation Speed Impact

**Mass Penalty**: Effective rotation = RotationSpeed × (1 / TotalMass)

Example with 500 total mass:
- Standard RCS: 1.0 × (1/500) = **0.002 rad/s**
- Mongoose: 1.5 × (1/500) = **0.003 rad/s** (+50% turn rate)
- Eclipse Racing: 1.8 × (1/500) = **0.0036 rad/s** (+80% turn rate)

**Key Insight**: Light ships benefit most from high-rotation thrusters.

---

## Hull Comparison

| Part Name | Rarity | Cost | Hull HP | Mass | Accel Mod | Max Vel Mod | Best For |
|-----------|--------|------|---------|------|-----------|-------------|----------|
| **Aluminum Composite** | Common | 0 | 100 | 200 | 1.0× | - | Racing |
| **Reinforced Titanium** | Uncommon | 3,500 | 180 | 350 | 0.85× | - | General Use |
| **Dreadnought Plating** | Rare | 7,000 | **300** | 500 | 0.7× | 0.9× thrust | Combat Tank |
| **Carbon Nanoweave** | Epic | 11,000 | 120 | **120** | **1.15×** | **1.1×** | Ultra-Light Racing |

### Hull Mass vs Protection

```
Armor Efficiency = Hull HP / Mass

Aluminum:      100 / 200 = 0.50 HP/mass
Reinforced:    180 / 350 = 0.51 HP/mass  (slightly better)
Dreadnought:   300 / 500 = 0.60 HP/mass  (best armor efficiency)
Carbon:        120 / 120 = 1.00 HP/mass  (best mass efficiency)
```

**Tradeoff**: Carbon Nanoweave has terrible armor efficiency, but exceptional mass efficiency.

---

## Shield Comparison

| Part Name | Rarity | Cost | Shield HP | Mass | Energy | Regen Rate | Best For |
|-----------|--------|------|-----------|------|--------|------------|----------|
| **No Shield** | Common | 0 | 0 | 0 | 0/0 | - | Pure Performance |
| **Mk1 Projector** | Uncommon | 4,000 | 100 | 80 | +50/+5 | Moderate | Basic Protection |
| **Fortress Array** | Rare | 8,000 | **250** | 150 | +100/+3 | Slow | Sustained Combat |
| **Adaptive Regenerator** | Epic | 13,000 | 150 | 100 | +80/**+12** | Fast | Hit-and-Run |

### Shield Strategies

**Total Effective HP** = Hull HP + Shield HP

| Build Example | Hull | Shield | Total HP | Regen |
|---------------|------|--------|----------|-------|
| **Racing (No Shield)** | 120 (Carbon) | 0 | **120** | 0/s |
| **Balanced** | 180 (Titanium) | 100 (Mk1) | **280** | 5/s |
| **Tank** | 300 (Dreadnought) | 250 (Fortress) | **550** | 3/s |
| **Agile Tank** | 180 (Titanium) | 150 (Adaptive) | **330** | **12/s** |

**Key Insight**: Adaptive Regenerator recovers shields in 12.5 seconds, while Fortress takes 83 seconds.

---

## Weapon Comparison

| Part Name | Rarity | Cost | Damage | Mass | Energy | Best For |
|-----------|--------|------|--------|------|--------|----------|
| **No Weapons** | Common | 0 | 0× | 0 | 0/0 | Racing |
| **Pulse Laser** | Uncommon | 5,000 | 1.0× | 60 | +40/-4 | General Combat |
| **Railgun Mk5** | Rare | 9,000 | **2.0×** | 140 | +30/-2 | Heavy Hitter |
| **Plasma Accelerator** | Epic | 15,000 | **3.0×** | 110 | +60/-8 | Max DPS |
| **Seeker Missiles** | Legendary | 20,000 | 2.5× | 95 | +50/-3 | Tracking |

### Energy Budget Analysis

**Energy Pool** = Base (100) + Engine + Weapon + Shield + Utility

**Energy Drain** = Weapon Drain (only when firing)

Example Combat Build:
```
Engine: Quantum Balance (+20 energy, +1 regen)
Weapon: Plasma Accelerator (+60 energy, -8 drain when firing)
Shield: Adaptive Regenerator (+80 energy, +12 regen)
Utility: Standard Scanner (+20 energy, 0 drain)

Total Energy Pool: 100 + 20 + 60 + 80 + 20 = 280
Total Regen (idle): +1 +12 = +13/s
Total Drain (firing): -8/s

Time to deplete firing continuously: 280 / 8 = 35 seconds
Time to recharge: 280 / 13 = 21.5 seconds
```

**Key Insight**: Plasma weapon requires high energy capacity to sustain fire.

---

## Utility Comparison

| Part Name | Rarity | Cost | Mass | Energy | Effect |
|-----------|--------|------|------|--------|--------|
| **Standard Scanner** | Common | 0 | 30 | +20/0 | Baseline detection |
| **Long-Range Sensors** | Uncommon | 4,500 | 55 | +40/-1 | Extended range |
| **Stealth ECM** | Rare | 10,000 | 70 | +60/-2.5 | Stealth capability |
| **Auto-Repair Nanobots** | Epic | 16,000 | 85 | +80/-3 | +50 HP, hull regen |

### Utility Usage

**Exploration Build**:
- Long-Range Sensors (find distant objectives)
- Light hull (travel fast)
- No weapons (save mass/energy)

**Combat Build**:
- Auto-Repair Nanobots (survivability)
- Heavy hull (tank damage)
- Stealth ECM (avoid detection)

---

## Wings Comparison

| Part Name | Rarity | Cost | Mass | Rotation | Max Vel | Accel | Hull | Best For |
|-----------|--------|------|------|----------|---------|-------|------|----------|
| **Standard Wing** | Common | 0 | 80 | 1.0× | - | 1.0× | - | Baseline |
| **Delta Wing** | Uncommon | 3,500 | 60 | 1.2× | 1.15× | 1.05× | - | Racing |
| **Cargo Wings** | Rare | 6,500 | 140 | 0.85× | - | 1.1× | +30 | Hauling |

**Note**: Wings primarily cosmetic in space, but affect atmospheric flight.

---

## Cockpit Comparison

| Part Name | Rarity | Cost | Mass | Rotation | Energy |
|-----------|--------|------|------|----------|--------|
| **Standard Cockpit** | Common | 0 | 120 | - | +30 |
| **Fighter Cockpit** | Uncommon | 5,500 | 100 | 1.1× | +50 |
| **Explorer Dome** | Rare | 7,500 | 90 | - | +70/+1 |

**Note**: Cockpits primarily cosmetic, minor stat bonuses.

---

## Sample Build Comparisons

### Build 1: "Speed Demon" (Racing Optimized)

| Slot | Part | Reasoning |
|------|------|-----------|
| Engine | Phantom Drive X-7 | Max velocity (+1.6×) |
| Thrusters | Eclipse Racing | Max rotation (+1.8×) |
| Hull | Carbon Nanoweave | Minimal mass (120) |
| Shield | None | Save mass/energy |
| Weapon | None | Save mass |
| Utility | Standard Scanner | Minimal mass |
| Wings | Delta Wing | Speed boost |
| Cockpit | Fighter Cockpit | Rotation bonus |

**Total Stats**:
- **Mass**: ~570 (very light)
- **Max Velocity**: 1.6 × 1.1 × 1.15 = **2.024× base** (fastest possible)
- **Rotation**: 1.8 × 1.1 = **1.98× base** (extremely agile)
- **Hull HP**: 120 (fragile)
- **Energy**: 200 (sufficient)

**Cost**: 71,500 credits

---

### Build 2: "Fortress" (Combat Tank)

| Slot | Part | Reasoning |
|------|------|-----------|
| Engine | Titan Mk2 | High thrust for mass |
| Thrusters | Precision Matrix | Stability for heavy ship |
| Hull | Dreadnought Plating | Max armor (300 HP) |
| Shield | Fortress Array | Max shields (250 HP) |
| Weapon | Railgun Mk5 | Heavy damage |
| Utility | Auto-Repair Nanobots | Hull regen (+50 HP) |
| Wings | Cargo Wings | Extra hull |
| Cockpit | Standard | Baseline |

**Total Stats**:
- **Mass**: ~1,455 (very heavy)
- **Hull HP**: 300 + 50 + 30 = **380 HP**
- **Shield HP**: **250 HP**
- **Total Effective HP**: **630 HP** (5× racing build)
- **Max Velocity**: 0.8× base (slow)
- **Rotation**: 0.9× base (sluggish)

**Cost**: 58,500 credits

---

### Build 3: "Balanced Pro" (All-Rounder)

| Slot | Part | Reasoning |
|------|------|-----------|
| Engine | Quantum Balance | Balanced stats |
| Thrusters | Mongoose Agility | Good rotation |
| Hull | Reinforced Titanium | Moderate armor |
| Shield | Mk1 Projector | Basic shields |
| Weapon | Pulse Laser | General combat |
| Utility | Long-Range Sensors | Utility |
| Wings | Standard | Baseline |
| Cockpit | Explorer Dome | Energy bonus |

**Total Stats**:
- **Mass**: ~890 (moderate)
- **Hull HP**: 180
- **Shield HP**: 100
- **Total Effective HP**: **280 HP**
- **Max Velocity**: 1.25× base
- **Rotation**: 1.5× base
- **Energy**: 360 (good capacity)

**Cost**: 28,500 credits (most affordable competitive build)

---

## Build Efficiency Analysis

| Build Type | Total Cost | Credits/HP | Credits/Speed | Best Value? |
|------------|-----------|------------|---------------|-------------|
| Speed Demon | 71,500 | 595.8 | 35,338 | No (expensive) |
| Fortress | 58,500 | 92.9 | N/A (too slow) | **Yes (combat)** |
| Balanced Pro | 28,500 | 101.8 | 22,800 | **Yes (racing)** |

**Recommendation**: Balanced Pro offers best value for new players. Save for Legendary parts later.

---

## Progression Timeline

### Early Game (Level 1-5, ~8,000 credits)

**Priority Unlocks**:
1. Velocity V3 Engine (2,500) - Speed boost
2. Mongoose Agility Thrusters (3,000) - Handling
3. Mk1 Shield (4,000) - Survivability

**Total**: 9,500 credits (slightly over budget, choose 2/3)

---

### Mid Game (Level 5-10, ~20,000 credits)

**Priority Unlocks**:
1. Quantum Balance Engine (10,000) - Major upgrade
2. Reinforced Titanium Hull (3,500) - Durability
3. Precision Matrix Thrusters (6,000) - Heavy ship handling

**Total**: 19,500 credits

---

### Late Game (Level 10-15, ~50,000 credits)

**Priority Unlocks**:
1. Phantom Drive X-7 (25,000) - Best engine
2. Eclipse Racing Thrusters (12,000) - Best thrusters
3. Carbon Nanoweave Hull (11,000) - Racing hull
4. Adaptive Regenerator Shield (13,000) - Fast regen

**Total**: 61,000 credits (selective unlocking required)

---

### Endgame (Level 15+, ~100,000 credits)

**Complete Collection**:
- All Legendary parts unlocked
- Multiple specialized builds
- All skins for customization

---

## Meta Build Predictions

Based on stat analysis, the following builds will likely dominate:

### Racing Meta
**"Phantom Speed"**:
- Phantom Drive + Eclipse Thrusters + Carbon Hull
- **Why**: Maximum velocity + minimum mass = fastest lap times

### Combat Meta
**"Adaptive Tank"**:
- Quantum Balance + Precision Thrusters + Dreadnought Hull + Adaptive Shield
- **Why**: High survivability with fast shield regen

### All-Rounder Meta
**"Quantum Mongoose"**:
- Quantum Balance + Mongoose Thrusters + Reinforced Hull + Mk1 Shield
- **Why**: Competitive in both racing and combat at affordable cost

---

## Balance Concerns

### Potential Issues

1. **Phantom Drive might be too dominant**: +60% velocity with only 80 mass
   - **Mitigation**: High cost (25,000) limits early access
   - **Monitor**: Lap time variance in playtesting

2. **Carbon Hull strictly better for racing**: Low mass with acceleration bonus
   - **Mitigation**: Fragile (120 HP only), risky for collisions
   - **Monitor**: Win rates with Carbon vs Titanium hull

3. **Adaptive Shield might invalidate Fortress**: Better regen, lower mass
   - **Mitigation**: Lower capacity (150 vs 250), can't sustain prolonged fire
   - **Monitor**: Combat survival rates

### Recommended Adjustments (Post-Playtest)

If balance issues emerge:

**Option 1**: Adjust costs
- Increase Phantom Drive to 35,000
- Decrease Fortress Shield to 6,000

**Option 2**: Adjust stats
- Reduce Phantom velocity to 1.5×
- Increase Carbon Hull mass to 140

**Option 3**: Add tradeoffs
- Phantom Drive: -5 energy regen (requires power management)
- Carbon Hull: Reduce HP to 100

---

**Last Updated**: Phase 2 Content Generation
**Next Review**: After first balance playtest
