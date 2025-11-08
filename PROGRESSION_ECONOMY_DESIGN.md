# Progression & Economy Design Document
## VR Flight Game - Ship Customization System

**Version:** 1.0
**Last Updated:** 2025-11-07
**Status:** Design Complete, Ready for Implementation

---

## Table of Contents

1. [Design Philosophy](#design-philosophy)
2. [Progression System](#progression-system)
3. [Economy System](#economy-system)
4. [Achievement System](#achievement-system)
5. [Balancing Guidelines](#balancing-guidelines)
6. [Tuning Guide](#tuning-guide)
7. [Implementation Notes](#implementation-notes)

---

## Design Philosophy

### Core Principles

1. **Respect Player Time**
   - No artificial grind walls
   - Steady progression throughout 1-50
   - Multiple paths to earn rewards
   - Catch-up mechanics for new players

2. **Meaningful Choices**
   - Every unlock should feel impactful
   - Parts provide real gameplay benefits
   - Players choose specialization paths
   - Loadout system supports experimentation

3. **Long-Term Engagement**
   - 30-40 hours to unlock most content
   - 60-80 hours to unlock everything
   - Post-50 prestige system (optional)
   - Regular content drops extend progression

4. **Fair Monetization (Future)**
   - All content earnable through gameplay
   - Optional time-savers only (XP/Credit boosters)
   - No pay-to-win mechanics
   - Cosmetics can be premium

---

## Progression System

### Level Curve (1-50)

**Formula:** `XP = BaseXP * (Level^Exponent) + Smoothing * (Level - 1)`

**Parameters:**
- BaseXP: 100
- Exponent: 1.15
- Smoothing: 50

**Key Milestones:**

| Level | Total XP | XP This Level | Description |
|-------|----------|---------------|-------------|
| 1     | 0        | -             | Starting point |
| 5     | 499      | 174           | Basic customization unlocked |
| 10    | 1,654    | 293           | Racing specialization |
| 15    | 3,740    | 511           | Combat specialization |
| 20    | 7,196    | 826           | Part upgrades unlocked |
| 25    | 12,589   | 1,266         | Advanced prototypes |
| 30    | 20,646   | 1,865         | Advanced weaponry |
| 35    | 32,260   | 2,659         | Quantum tech tier |
| 40    | 48,511   | 3,680         | Legendary equipment |
| 45    | 70,692   | 5,010         | Exotic weapons |
| 50    | 100,340  | 6,611         | Ultimate endgame |

**Curve Characteristics:**
- Early game (1-10): Fast progression, frequent unlocks
- Mid game (10-30): Steady pace, player choice matters
- Late game (30-50): Longer levels, but bigger rewards
- Smooth exponential - no sudden walls

### XP Rewards

#### Race Completion
```
Base: 150 XP
+ Medal Bonus:
  - Gold: +100 XP
  - Silver: +50 XP
  - Bronze: +25 XP
× Difficulty Multiplier:
  - Tutorial: 0.5x
  - Easy: 1.0x
  - Medium: 1.5x
  - Hard: 2.0x
  - Expert: 3.0x

Example: Gold medal on Hard = (150 + 100) × 2.0 = 500 XP
```

#### Other Activities
- Asteroid Destroyed: 10-50 XP (by size)
- Mission Complete: 100-500 XP (by type)
- Discovery: 50-150 XP
- Achievement: 100-2,500 XP (by rarity)
- Daily Login: 50 XP
- First Win of Day: 200 XP
- Perfect Run: 300 XP

### Unlock Schedule

**Level 1-10: Foundation**
- Basic parts (engines, thrusters, hull, wings)
- First skins
- Shield system unlock
- 2 loadout slots

**Level 10-25: Specialization**
- Racing vs Combat paths diverge
- Advanced defensive/offensive parts
- Customization deepens
- 4 loadout slots

**Level 25-40: Mastery**
- Prototype and legendary parts
- Advanced stat tracking
- Quantum technology tier
- 5 loadout slots

**Level 40-50: Endgame**
- Ultimate equipment
- Exotic weapons
- Champion skins
- All features unlocked

### Level-Up Rewards

**Every Level:**
- 500 credits
- Satisfaction of progress

**Every 5 Levels:**
- 2,500 bonus credits
- 500 bonus XP
- Major milestone feeling

**Every 10 Levels:**
- 5,000 bonus credits
- Guaranteed rare+ part unlock
- Achievement progression

**Special Milestones:**
- Level 25: 10,000 credits + exclusive gold skin
- Level 50: 25,000 credits + ultimate bundle

---

## Economy System

### Credit Earn Rates

#### Race Completion
```
Base: 250 credits
+ Medal Bonus:
  - Gold: +200 cr
  - Silver: +100 cr
  - Bronze: +50 cr
× Difficulty Multiplier:
  - Tutorial: 0.5x
  - Easy: 1.0x
  - Medium: 1.5x
  - Hard: 2.0x
  - Expert: 3.0x
× First Time Bonus: 1.5x (first completion only)

Example: Gold on Hard (first time) = (250 + 200) × 2.0 × 1.5 = 1,350 cr
Example: Same race (repeat) = (250 + 200) × 2.0 = 900 cr
```

#### Expected Earnings by Skill Level

| Skill Level | Credits/Hour | Description |
|-------------|--------------|-------------|
| Casual      | 2,000 cr/hr  | Bronze/Silver medals, Easy/Medium |
| Average     | 4,000 cr/hr  | Silver/Gold medals, Medium |
| Skilled     | 8,000 cr/hr  | Gold medals, Hard |
| Expert      | 15,000 cr/hr | Gold medals, Expert, speedruns |

### Part Costs

#### Base Costs by Rarity

| Rarity    | Base Cost |
|-----------|-----------|
| Common    | 500 cr    |
| Uncommon  | 1,500 cr  |
| Rare      | 4,000 cr  |
| Epic      | 10,000 cr |
| Legendary | 25,000 cr |

#### Category Multipliers

| Category  | Multiplier | Reason |
|-----------|------------|--------|
| Engine    | 1.5x       | Core performance |
| Thrusters | 1.2x       | Important for handling |
| Hull      | 1.3x       | Survivability crucial |
| Wings     | 1.1x       | Cosmetic + minor stats |
| Cockpit   | 1.0x       | Mostly cosmetic |
| Weapon    | 1.4x       | Combat power |
| Shield    | 1.3x       | Defensive power |
| Utility   | 1.0x       | Situational |

#### Stat Impact Multiplier

Parts with higher total stat bonuses cost more:

```
Final Cost = BaseCost × CategoryMultiplier × (1 + TotalStatBonus × 0.1)

Example: Epic Engine (+60% total stats)
  = 10,000 × 1.5 × (1 + 0.6 × 0.1)
  = 10,000 × 1.5 × 1.06
  = 15,900 credits
```

### Skin Costs

#### Base Costs by Rarity

| Rarity    | Base Cost |
|-----------|-----------|
| Common    | 300 cr    |
| Uncommon  | 800 cr    |
| Rare      | 2,000 cr  |
| Epic      | 5,000 cr  |
| Legendary | 12,000 cr |

**Note:** Skins are cheaper than parts (cosmetic only)

#### Theme Multipliers (Future)

| Theme            | Multiplier |
|------------------|------------|
| Solid Color      | 0.8x       |
| Pattern          | 1.0x       |
| Animated         | 1.5x       |
| Holographic      | 2.0x       |
| Particle Effects | 2.5x       |

### Upgrade Costs

Parts can be upgraded 5 times (Level 1-5):

| Upgrade Level | Cost Multiplier | Example (6,000 cr part) |
|---------------|-----------------|-------------------------|
| Level 1       | 0.5x            | 3,000 cr                |
| Level 2       | 0.75x           | 4,500 cr                |
| Level 3       | 1.0x            | 6,000 cr                |
| Level 4       | 1.5x            | 9,000 cr                |
| Level 5       | 2.0x            | 12,000 cr               |
| **Total**     | **5.75x**       | **34,500 cr**           |

**Fully upgrading a part costs 5.75x its base cost** - expensive but powerful!

### Loadout Slot Costs

| Slot # | Cost      | Description |
|--------|-----------|-------------|
| 1      | FREE      | Everyone gets one |
| 2      | 5,000 cr  | Unlocked at Level 10 |
| 3      | 10,000 cr | Unlocked at Level 15 |
| 4      | 20,000 cr | Unlocked at Level 25 |
| 5      | 35,000 cr | Unlocked at Level 35 |
| 6      | 50,000 cr | Unlocked at Level 45 |

**Total:** 120,000 credits to unlock all slots

### Total Content Value

| Category       | Total Value  | Description |
|----------------|--------------|-------------|
| All Parts      | 450,000 cr   | ~60 parts across all rarities |
| All Skins      | 150,000 cr   | ~40 skins |
| All Upgrades   | 600,000 cr   | Upgrading favorite parts |
| Loadout Slots  | 120,000 cr   | Slots 2-6 |
| **GRAND TOTAL**| **1,320,000 cr** | Everything in the game |

### Progression Pace Targets

| Milestone              | Target Hours | Credits Needed |
|------------------------|--------------|----------------|
| First Customization    | 0.25 hr      | 500 cr         |
| Full Basic Loadout     | 2 hr         | 3,000 cr       |
| First Rare Item        | 5 hr         | 20,000 cr      |
| Competitive Loadout    | 15 hr        | 60,000 cr      |
| Most Content (80%)     | 40 hr        | ~300,000 cr    |
| Everything Maxed       | 80 hr        | 1,320,000 cr   |

**Average player at Level 50:** ~50 hours, ~150,000 credits earned
- Can afford competitive builds
- Has unlocked most content
- Still has goals to pursue

---

## Achievement System

### Total Achievements: 30

**Distribution by Rarity:**
- Common: 2 (easy intro achievements)
- Uncommon: 8 (regular milestones)
- Rare: 10 (significant accomplishments)
- Epic: 7 (major feats)
- Legendary: 6 (ultimate achievements)

### Total Rewards

- **Total XP:** 23,850 XP (~15% of XP to reach Level 50)
- **Total Credits:** 128,800 credits (~10% of total content value)
- **Unique Unlocks:** 20 exclusive skins/items

### Achievement Categories

1. **Customization (10 achievements)**
   - First customization
   - Loadout mastery
   - Build archetypes (speed, tank, glass cannon)
   - Experimentation rewards

2. **Progression (12 achievements)**
   - Part/skin collection
   - Rarity milestones
   - Level milestones
   - Upgrade achievements

3. **Economy (5 achievements)**
   - Credit accumulation
   - Wealth tiers
   - Spending patterns

4. **Hidden (3 achievements)**
   - Secret builds
   - Easter eggs
   - Meta achievements

### High-Value Achievements

| Achievement         | Reward        | Description |
|---------------------|---------------|-------------|
| Millionaire         | 25,000 cr     | Earn 1M total credits |
| Master Pilot (L50)  | 25,000 cr     | Reach max level |
| Completionist       | 10,000 cr     | Unlock all parts |
| Ultimate Ship       | 10,000 cr     | Max upgrade full loadout |
| Legend Collector    | 15,000 cr     | Unlock all legendary parts |

---

## Balancing Guidelines

### Golden Rules

1. **Never Hard-Gate Content**
   - Level requirements guide, don't block
   - Credits are always earnable
   - Multiple paths to any goal

2. **Respect Different Play Styles**
   - Casual players progress slower but steadily
   - Skilled players earn more, progress faster
   - No punishment for experimentation

3. **Maintain Value Perception**
   - Legendary parts should feel legendary
   - Upgrade costs should be significant
   - Free-to-play players can compete

4. **Avoid Grind Walls**
   - No single item costs >2 hours of grinding
   - Level-appropriate unlocks always available
   - Bonus systems help catch-up

### Warning Signs

**Too Grindy:**
- Players stuck at same level for 5+ hours
- Common parts take 1+ hour to afford
- Frequent complaints about credit earn rates

**Too Easy:**
- Players unlock everything by Level 30
- Max credits with nothing to buy
- No sense of accomplishment

**Poor Pacing:**
- Long gaps with no unlocks
- Multiple unlocks at once, then nothing
- Uneven difficulty spikes

---

## Tuning Guide

### Adjusting XP Curve

**To make leveling faster:**
```json
"levelCurve": {
  "baseXP": 80,        // Reduce from 100
  "exponent": 1.12,    // Reduce from 1.15
  "smoothingFactor": 40
}
```

**To make leveling slower:**
```json
"levelCurve": {
  "baseXP": 120,       // Increase from 100
  "exponent": 1.18,    // Increase from 1.15
  "smoothingFactor": 60
}
```

### Adjusting Credit Earn Rates

**To increase earnings (+25%):**
- Multiply all `creditEarnRates` values by 1.25
- Or increase race base from 250 to 312

**To decrease earnings (-25%):**
- Multiply all `creditEarnRates` values by 0.75
- Or decrease race base from 250 to 188

### Adjusting Part Costs

**To make parts cheaper (-20%):**
```json
"baseCostsByRarity": {
  "Common": 400,      // Was 500
  "Uncommon": 1200,   // Was 1500
  "Rare": 3200,       // Was 4000
  "Epic": 8000,       // Was 10000
  "Legendary": 20000  // Was 25000
}
```

**To make parts more expensive (+20%):**
```json
"baseCostsByRarity": {
  "Common": 600,
  "Uncommon": 1800,
  "Rare": 4800,
  "Epic": 12000,
  "Legendary": 30000
}
```

### Quick Balance Checks

**Run the test script:**
```bash
python test_ship_progression_balance.py
```

**Check these metrics:**
1. **Level 25 reached in:** Should be 12-20 hours
2. **Level 50 reached in:** Should be 40-60 hours
3. **Credits at L50:** Should be 100,000-200,000
4. **Content unlockable:** Should be 40-60% by L50

---

## Implementation Notes

### Integration with Existing Systems

1. **ShipCustomizationComponent**
   - Already has progression data structure
   - `AddXP()` and `AddCredits()` methods exist
   - Level-up events trigger unlock checks

2. **ProgressionCalculator**
   - Static utility class
   - All formulas centralized
   - Easy to call from Blueprint or C++

3. **Data-Driven Design**
   - All values in JSON files
   - Hot-reload during development
   - Easy balancing without recompiling

### Testing Strategy

1. **Unit Tests**
   - Test XP calculations
   - Test cost formulas
   - Test unlock validation

2. **Simulation Tests**
   - Run `test_ship_progression_balance.py`
   - Simulate 1-50 progression
   - Validate earn rates

3. **Playtesting**
   - Track actual player progression
   - Monitor credit balances
   - Adjust based on feedback

### Performance Considerations

- XP lookup table pre-calculated (O(1) lookup)
- Cost calculations cached per part
- Unlock schedule loaded once at startup
- No runtime JSON parsing

### Future Enhancements

1. **Prestige System**
   - Reset to Level 1 with bonuses
   - Exclusive prestige skins
   - +10% XP/Credit earning
   - Prestige-only parts

2. **Seasonal Content**
   - Limited-time unlocks
   - Seasonal achievements
   - Event-specific rewards

3. **Daily/Weekly Challenges**
   - Bonus credit objectives
   - Rotating challenges
   - Streak rewards

4. **Social Features**
   - Show off loadouts
   - Leaderboards by build
   - Trade/gift system (future)

---

## Monetization Considerations

### Current Design: Premium-Friendly, Not Required

**Can Add Later:**

1. **Premium Currency (Gems)**
   - Optional parallel currency
   - Purchase skins directly
   - Accelerate unlocks (not exclusive content)

2. **Battle Pass**
   - Free tier: Credits + XP
   - Premium tier: Exclusive skins + boosters
   - Never gates gameplay content

3. **Cosmetic Store**
   - Premium skins (can't earn through gameplay)
   - Ship nameplates
   - Victory animations
   - Trail effects

4. **Time Savers (NOT Pay-to-Win)**
   - XP Boost: +50% for 24 hours ($2.99)
   - Credit Boost: +50% for 24 hours ($2.99)
   - Part unlock (cosmetic only)

**Never Monetize:**
- Gameplay-affecting parts
- Competitive advantages
- Required content
- "Loot box" random unlocks

---

## Conclusion

This progression and economy system is designed to:

- Keep players engaged for 40-80 hours
- Respect player time and skill
- Provide clear goals and milestones
- Support fair monetization (if desired)
- Be easily tunable based on data

**All values are in JSON files for easy iteration without code changes.**

**Use `test_ship_progression_balance.py` to validate any changes.**

---

**Document Version:** 1.0
**Author:** AI System Design
**Next Review:** After first playtest session
