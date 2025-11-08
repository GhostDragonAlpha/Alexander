# Ship Customization System - Development Roadmap

**Project:** Alexander VR Space Flight Game
**Version:** 1.0
**Last Updated:** November 7, 2025

---

## Overview

This document outlines the complete development roadmap for the Ship Customization System, from initial C++ backend implementation through advanced multiplayer features. The roadmap is divided into 5 distinct phases, with Phase 1 already complete.

---

## Phase Summary

| Phase | Status | Focus | Duration | Deliverables |
|-------|--------|-------|----------|--------------|
| **Phase 1** | ✅ COMPLETE | Core C++ Backend + Physics Integration | 2 weeks | Component system, n-body physics integration, save system |
| **Phase 2** | 📋 NEXT | Blueprint Assets + Data Content | 2-3 weeks | Data tables, presets, materials, initial content package |
| **Phase 3** | 🔜 PLANNED | UI/HUD System + Customization Menu | 3-4 weeks | In-game UI, loadout menu, part preview, stats display |
| **Phase 4** | 🔜 PLANNED | Progression + Unlocks + Achievements | 2-3 weeks | XP system, unlock flow, achievement integration, economy tuning |
| **Phase 5** | 💡 FUTURE | Advanced Features | 4-6 weeks | Procedural parts, crafting, marketplace, synergy bonuses |

**Total Estimated Development Time**: 13-18 weeks (3-4.5 months)

---

## Phase 1: Core C++ Backend + Physics Integration

### Status: ✅ COMPLETE

### Timeline
- **Started**: October 2025
- **Completed**: November 2025
- **Duration**: 2 weeks

### Objectives
- ✅ Create C++ component architecture
- ✅ Integrate with n-body physics system (OrbitalBody)
- ✅ Implement data-driven part/skin system
- ✅ Build save/load system
- ✅ Add network replication foundation
- ✅ Create physics configuration assets

### Deliverables Completed

#### 1. UShipCustomizationComponent
**File**: `Source/Alexander/Public/ShipCustomizationComponent.h`

**Features**:
- Component-based architecture (attach to any ship)
- Data table integration (FShipPartData, FShipSkinData)
- Loadout management (equip, unequip, save, load)
- Stats calculation system (10 ship stats)
- Progression tracking (level, XP, credits, unlocks)
- Event system (delegates for loadout changes, unlocks, level-ups)
- SaveGame persistence

**Stats System**:
- `ThrustPower` (multiplier) - affects acceleration
- `MaxVelocity` (multiplier) - affects top speed
- `RotationSpeed` (multiplier) - affects turn rate
- `Acceleration` (multiplier) - affects responsiveness
- `HullIntegrity` (additive) - ship health
- `ShieldStrength` (additive) - shield capacity
- `WeaponDamage` (multiplier) - weapon output
- `EnergyCapacity` (additive) - energy reserves
- `EnergyRegenRate` (additive) - energy recovery
- `Mass` (additive) - affects inertia and gravity

**Key Methods**:
- `EquipPart(Category, PartID)` - equip a part
- `UnlockPart(PartID)` - unlock with credits
- `CalculateTotalStats()` - sum all part stats
- `ApplyStatsToFlightController()` - update physics
- `SaveCustomizationData()` / `LoadCustomizationData()` - persistence

#### 2. Physics Integration

**Integration Points**:

**AOrbitalBody** (n-body physics):
- `Mass` - affects gravitational forces (F = G * m1 * m2 / r²)
- `ThrustPower` - affects ship acceleration
- `StandardGravitationalParameter` - μ = G * Mass for orbital mechanics

**UFlightController** (flight assistance):
- `MaxSafeVelocity` - speed limit near planets

**ASpaceship** (compatibility):
- `MaxThrust` - legacy property sync
- Network RPC: `ServerApplyCustomization()` - authoritative stats application

**Impact**: Ship customization directly affects orbital mechanics, gravity calculations, and flight performance.

#### 3. Data Structures

**FShipPartData** - Part definition:
- Metadata (PartID, DisplayName, Description, Icon)
- Categorization (Category, Rarity)
- Requirements (LevelRequirement, UnlockCost, bUnlockedByDefault)
- Stats (StatModifiers)
- Visuals (PartMesh, Icon)
- Progression (ParentPartID, bCanBeUpgraded, UpgradeLevel)

**FShipSkinData** - Skin definition:
- Metadata (SkinID, DisplayName, Description)
- Visuals (SkinMaterial, Icon)
- Requirements (Rarity, UnlockCost, bUnlockedByDefault)

**FShipLoadout** - Complete ship configuration:
- EquippedParts (Map<Category, PartID>)
- EquippedSkin (FName)
- TotalStats (cached)

**FPlayerProgressionData** - Player state:
- PlayerLevel, TotalXP, Credits
- UnlockedParts, UnlockedSkins
- SavedLoadouts, ActiveLoadoutIndex

#### 4. UShipPhysicsConfig (Data Asset)

**Purpose**: Preset physics configurations for ship types

**Properties**:
- Mass, ThrustPower, MaxVelocity, AngularThrustPower
- AccelerationMultiplier, RotationSpeedMultiplier
- StabilityAssistStrength
- FuelCapacity, FuelConsumptionRate (future)

**Example Presets**:
- Light Fighter (low mass, high thrust, fast)
- Heavy Freighter (high mass, moderate thrust, slow)
- Balanced Scout (medium stats)

#### 5. Network Replication

**Replicated Properties**:
- `CurrentLoadout` (FShipLoadout) - synced to all clients
- `AOrbitalBody::Mass`, `ThrustPower` - physics state

**NOT Replicated**:
- `ProgressionData` (FPlayerProgressionData) - client-only, saved to disk

**Network Strategy**:
- Server authoritative for physics state
- Client prediction for immediate feedback
- RPC: `ServerApplyCustomization()` for client requests
- Bandwidth optimization: ~100 bytes per loadout update

#### 6. Documentation

**Created**:
- ✅ SHIP_CUSTOMIZATION_ARCHITECTURE.md (this document's companion)
- ✅ SHIP_CUSTOMIZATION_API_REFERENCE.md (complete API documentation)
- ✅ SHIP_CUSTOMIZATION_INTEGRATION_PATTERNS.md (integration guide)
- ✅ BUILD_STATUS_REPORT.md (build verification)

### Technical Achievements

1. **Seamless Physics Integration**: Ship stats directly modify n-body orbital mechanics
2. **Data-Driven Design**: All content in data tables, no hardcoded parts
3. **Network-Ready**: Multiplayer foundation with authoritative server model
4. **Modular Architecture**: Component can be added to any ship
5. **Save System**: Automatic persistence of progression and loadouts

### Known Limitations (Phase 1)

- No Blueprint content yet (parts, skins, materials)
- No UI/HUD for customization
- Part meshes don't swap (visual placeholder)
- No progression tuning (XP curve, costs)
- No achievement integration

---

## Phase 2: Blueprint Assets + Data Content

### Status: 📋 NEXT (Starting Soon)

### Timeline
- **Planned Start**: November 2025
- **Estimated Duration**: 2-3 weeks
- **Dependencies**: Phase 1 complete ✅

### Objectives
- Create ship parts data tables (8 categories)
- Create ship skins data tables
- Create physics configuration presets
- Build materials library for skins
- Create part meshes (or source from art team)
- Test all parts in PIE mode
- Balance part stats and costs

### Deliverables

#### 1. Ship Parts Data Tables (25 parts minimum)

**Recommended Content Package**:

| Category | Count | Examples |
|----------|-------|----------|
| Engines | 4 | Basic Ion, Advanced Ion, Plasma Drive, Quantum Drive |
| Thrusters | 3 | Standard RCS, High-Agility RCS, Precision RCS |
| Hulls | 3 | Light Frame, Standard Hull, Reinforced Armor |
| Weapons | 4 | Pulse Laser, Beam Laser, Missile Launcher, Plasma Cannon |
| Shields | 3 | Basic Shield, Advanced Shield, Fortress Shield |
| Wings | 2 | Standard Wings, Racing Wings |
| Utility | 2 | Cargo Expansion, Sensor Array |
| Cockpit | 0 | (Future - Phase 5) |

**File Structure**:
```
Content/Data/ShipParts/
├── DT_ShipParts_Engines.uasset
├── DT_ShipParts_Thrusters.uasset
├── DT_ShipParts_Hulls.uasset
├── DT_ShipParts_Weapons.uasset
├── DT_ShipParts_Shields.uasset
├── DT_ShipParts_Wings.uasset
└── DT_ShipParts_Utility.uasset
```

**Example Part Data**:
```
Engine_Advanced:
  DisplayName: "Advanced Ion Engine"
  Category: Engine
  Rarity: Rare
  LevelRequirement: 5
  UnlockCost: 500
  bUnlockedByDefault: false
  StatModifiers:
    ThrustPower: 0.5  (adds 50% thrust)
    Mass: 200.0       (adds 200 kg)
    EnergyConsumption: 5.0  (drains 5 energy/sec)
```

#### 2. Ship Skins Data Table (8-10 skins)

**Recommended Skins**:
- Default (unlocked by default)
- Racing Red (unlocked by default)
- Military Gray (Uncommon, 200 credits)
- Stealth Black (Rare, 500 credits)
- Championship Gold (Epic, 1000 credits)
- Legendary Chrome (Legendary, 3000 credits)
- Pirate Skull (Special, achievement unlock)
- Corporate Blue (Special, achievement unlock)

**File Structure**:
```
Content/Data/ShipSkins/
└── DT_ShipSkins.uasset

Content/Materials/ShipSkins/
├── M_ShipSkin_Default.uasset
├── M_ShipSkin_Racing_Red.uasset
├── M_ShipSkin_Military_Gray.uasset
└── ...
```

#### 3. Physics Configuration Presets (3 minimum)

**Recommended Presets**:

**Light Fighter**:
```
ConfigName: "Light Fighter"
Mass: 800 kg
ThrustPower: 120000 N
MaxVelocity: 1200 m/s
AccelerationMultiplier: 1.3
RotationSpeedMultiplier: 1.5
StabilityAssistStrength: 0.3
```

**Balanced Scout**:
```
ConfigName: "Balanced Scout"
Mass: 1000 kg
ThrustPower: 100000 N
MaxVelocity: 1000 m/s
AccelerationMultiplier: 1.0
RotationSpeedMultiplier: 1.0
StabilityAssistStrength: 0.5
```

**Heavy Freighter**:
```
ConfigName: "Heavy Freighter"
Mass: 3000 kg
ThrustPower: 150000 N
MaxVelocity: 600 m/s
AccelerationMultiplier: 0.7
RotationSpeedMultiplier: 0.6
StabilityAssistStrength: 0.8
```

**File Structure**:
```
Content/Data/PhysicsConfigs/
├── DA_LightFighter.uasset
├── DA_BalancedScout.uasset
└── DA_HeavyFreighter.uasset
```

#### 4. Materials Library

**Skin Materials**:
- Base material with color/roughness/metallic parameters
- Material instances for each skin variant
- Emissive materials for special skins
- Iridescent/holographic materials for legendary skins

**File Structure**:
```
Content/Materials/ShipSkins/
├── M_ShipSkin_Master.uasset (master material)
└── Instances/
    ├── MI_ShipSkin_Default.uasset
    ├── MI_ShipSkin_Racing_Red.uasset
    └── ...
```

#### 5. Part Meshes (Optional for Phase 2)

**Options**:
- Use placeholder meshes (cubes, cylinders)
- Source from art team (if available)
- Purchase from marketplace
- Defer to Phase 5 (use materials only for now)

**Recommended Approach**: Use materials only for Phase 2, defer meshes to Phase 5.

#### 6. Testing & Balancing

**Tasks**:
- [ ] Create test level with ship
- [ ] Verify all parts load from data tables
- [ ] Test equipping parts in each category
- [ ] Verify stats apply to physics correctly
- [ ] Test save/load persistence
- [ ] Balance part costs and stats
- [ ] Create balancing spreadsheet (Excel/Google Sheets)

**Balancing Guidelines**:

| Stat | Common | Uncommon | Rare | Epic | Legendary |
|------|--------|----------|------|------|-----------|
| Thrust | 0.9-1.1x | 1.0-1.3x | 1.2-1.6x | 1.5-1.9x | 1.8-2.0x |
| Speed | 0.9-1.1x | 1.0-1.3x | 1.2-1.6x | 1.5-1.9x | 1.8-2.0x |
| Mass | 800-1200kg | 1000-1500kg | 1200-1800kg | 1500-2500kg | 2000-3000kg |
| Cost | 0-200 | 100-400 | 400-1000 | 1000-3000 | 3000-5000 |
| Level | 1-3 | 3-6 | 6-12 | 12-18 | 18-20 |

### Success Criteria
- [ ] 25+ parts created across 7 categories
- [ ] 8+ skins created with materials
- [ ] 3+ physics presets created
- [ ] All parts load correctly in-game
- [ ] Stats apply to ship physics correctly
- [ ] Save/load works with all parts
- [ ] No crashes or errors in PIE mode
- [ ] Balancing spreadsheet created and reviewed

### Risks & Mitigation
- **Risk**: Art assets not ready → **Mitigation**: Use placeholder materials/meshes
- **Risk**: Balancing takes longer than expected → **Mitigation**: Start with conservative values, iterate in Phase 4
- **Risk**: Data table bugs → **Mitigation**: Create CSV import templates, test early

---

## Phase 3: UI/HUD System + Customization Menu

### Status: 🔜 PLANNED

### Timeline
- **Planned Start**: December 2025
- **Estimated Duration**: 3-4 weeks
- **Dependencies**: Phase 2 complete

### Objectives
- Create customization menu UI
- Build part selection interface
- Add stats comparison display
- Create loadout management UI
- Add progression display (level, XP, credits)
- Implement unlock flow UI
- Add VR-specific UI considerations

### Deliverables

#### 1. Customization Menu (UMG Widget)

**Layout**:
```
┌─────────────────────────────────────────────────────┐
│ SHIP CUSTOMIZATION                    [X] Close     │
├──────────┬──────────────────────────────────────────┤
│          │                                          │
│ PARTS    │  ┌────────────────────────────────────┐ │
│ ├─Engine │  │  PART PREVIEW (3D or Icon)         │ │
│ │        │  │                                    │ │
│ ├─Weapon │  └────────────────────────────────────┘ │
│ │        │                                          │
│ ├─Shield │  Part Name: Advanced Ion Engine          │
│ │        │  Rarity: [RARE]                          │
│ ├─Hull   │  Level Req: 5                            │
│ │        │  Cost: 500 credits                       │
│ ├─Thruster│                                          │
│ │        │  Stats:                                  │
│ ├─Wings  │  + Thrust Power: +50%                    │
│ │        │  + Mass: +200 kg                         │
│ └─Utility│                                          │
│          │  [EQUIP] or [UNLOCK FOR 500]             │
│ SKINS    │                                          │
│ ├─🎨 Red  │  ┌────────────────────────────────────┐ │
│ ├─🎨 Blue │  │  TOTAL STATS                       │ │
│ └─🎨 Gold │  │  Thrust: 1.7x  Speed: 1.3x         │ │
│          │  │  Mass: 1500 kg  Hull: 150 HP       │ │
│ LOADOUTS │  └────────────────────────────────────┘ │
│ ├─Racing │                                          │
│ ├─Combat │  Level: 7  XP: 3500/5000                 │
│ └─Tank   │  Credits: 1250                           │
└──────────┴──────────────────────────────────────────┘
```

**Features**:
- Part category tabs (Engine, Weapon, Shield, etc.)
- Part list with icons and rarity colors
- Part preview (3D model or 2D icon)
- Stats comparison (current vs. new)
- Unlock button (if locked)
- Equip button (if unlocked)
- Total stats display (live updates)
- Loadout save/load interface
- Progression display (level, XP, credits)

#### 2. Part Selection Interface

**Components**:
- **Part List Widget** (scrollable list)
  - Part icon
  - Part name
  - Rarity badge
  - Lock/unlock indicator
  - Equipped indicator (checkmark)
- **Part Detail Widget** (right panel)
  - Large icon or 3D preview
  - Full description
  - Stat modifiers (colored text: green = positive, red = negative)
  - Requirements (level, cost)
  - Equip/unlock button

#### 3. Stats Comparison Display

**Before/After Comparison**:
```
CURRENT STATS          NEW STATS              CHANGE
─────────────          ─────────              ──────
Thrust: 1.5x           Thrust: 2.0x           +0.5x ↑
Speed:  1.2x           Speed:  1.2x            0.0x ─
Mass:   1300 kg        Mass:   1500 kg       +200 kg ↑
Hull:   120 HP         Hull:   120 HP          0 HP ─
```

**Implementation**:
- Calculate stats for current loadout
- Calculate stats for loadout with new part
- Display difference with color coding (green = better, red = worse)

#### 4. Loadout Management UI

**Features**:
- Save loadout (name input)
- Load loadout (dropdown list)
- Delete loadout (confirmation dialog)
- Loadout preview (stats summary)

**Layout**:
```
MY LOADOUTS
├── Racing Build     (Thrust: 1.8x, Speed: 1.5x)  [Load] [Delete]
├── Combat Build     (Weapon: 1.9x, Shield: 200)  [Load] [Delete]
└── Tank Build       (Hull: 250 HP, Mass: 3000kg) [Load] [Delete]

[+ SAVE CURRENT LOADOUT]
```

#### 5. Progression Display

**Components**:
- **Level Display**: Large number with circular progress bar
- **XP Bar**: Progress toward next level
- **Credits Display**: Currency balance with icon
- **Unlock Notifications**: Toast messages when parts unlock

**Layout**:
```
┌──────────────────────────────────┐
│  LEVEL 7                 🔓 12   │
│  ████████░░░░ 3500/5000 XP       │
│  💰 1250 Credits                  │
└──────────────────────────────────┘
```

#### 6. Unlock Flow UI

**Flow**:
1. User clicks locked part
2. Show unlock dialog:
   ```
   ┌─────────────────────────────┐
   │ Unlock "Advanced Engine"?   │
   │                             │
   │ Cost: 500 credits           │
   │ Your balance: 1250 credits  │
   │                             │
   │ [UNLOCK]  [CANCEL]          │
   └─────────────────────────────┘
   ```
3. On confirm:
   - Deduct credits
   - Unlock part
   - Show success animation
   - Enable equip button

#### 7. VR Considerations

**VR-Specific Features**:
- **3D Widget in World Space**: Menu floats in front of player
- **VR Controller Interaction**: Ray-cast pointing and clicking
- **Hand-Tracked Gestures**: Grab and drag parts (optional)
- **Haptic Feedback**: Vibration on equip/unlock
- **Spatial Audio**: UI sounds positioned in 3D space

**Layout Adjustments for VR**:
- Larger fonts (readable at arm's length)
- Bigger buttons (easier to click with ray-cast)
- Less dense information (avoid clutter)
- 3D part preview (rotate with motion controller)

### Success Criteria
- [ ] Customization menu functional and bug-free
- [ ] All parts display correctly with icons
- [ ] Stats comparison shows accurate data
- [ ] Unlock flow works end-to-end
- [ ] Loadout save/load works correctly
- [ ] VR interaction feels natural
- [ ] UI performance is smooth (60 FPS)
- [ ] No visual glitches or layout issues

### Risks & Mitigation
- **Risk**: VR UI difficult to navigate → **Mitigation**: Iterate with VR playtests, adjust layout
- **Risk**: Stats comparison confusing → **Mitigation**: Use clear color coding and arrows
- **Risk**: Performance issues with 3D previews → **Mitigation**: Use static images as fallback

---

## Phase 4: Progression + Unlocks + Achievements

### Status: 🔜 PLANNED

### Timeline
- **Planned Start**: January 2026
- **Estimated Duration**: 2-3 weeks
- **Dependencies**: Phase 3 complete

### Objectives
- Tune XP curve and level progression
- Balance part unlock costs
- Integrate with achievement system
- Create unlock events and rewards
- Add mission/race XP rewards
- Tune economy (credit sources and sinks)
- Add progression feedback (notifications, sounds)

### Deliverables

#### 1. XP Curve Tuning

**Current Formula**: `XP = Level² * 100`

**Goals**:
- Levels 1-5: Fast progression (tutorial phase)
- Levels 6-12: Moderate progression (main gameplay)
- Levels 13-20: Slow progression (endgame)

**Tuned Formula** (proposed):
```
XP = (Level^2.2) * 80 + (Level * 50)

Level 1→2:  120 XP    (1 mission)
Level 2→3:  280 XP    (2 missions)
Level 5→6:  1100 XP   (5 missions)
Level 10→11: 4200 XP  (20 missions)
Level 19→20: 15000 XP (75 missions)
```

**Testing**:
- Playtest 1-20 level progression
- Adjust multipliers based on feedback
- Ensure 20-30 hours to reach max level

#### 2. Part Unlock Cost Balancing

**Cost Formula** (proposed):
```
Cost = 100 × RarityMultiplier × (1 + LevelReq/20)

Rarity Multipliers:
- Common: 0 (free)
- Uncommon: 2
- Rare: 5
- Epic: 10
- Legendary: 15
```

**Example Costs**:
```
Common Engine (Level 1):   0 credits
Uncommon Weapon (Level 3): 230 credits
Rare Shield (Level 8):     700 credits
Epic Engine (Level 12):    1600 credits
Legendary Weapon (Level 18): 2850 credits
```

**Credit Sources**:
- Race completion: 100-500 credits (based on position)
- Mission completion: 200-1000 credits (based on difficulty)
- Enemy destruction: 50-250 credits (based on enemy type)
- Level up bonus: Level × 100 credits
- Daily login bonus: 100 credits

**Credit Sinks**:
- Part unlocks: 0-5000 credits
- Skin unlocks: 200-3000 credits
- Ship repairs: 50-500 credits (future)
- Fuel costs: 10-100 credits (future)

#### 3. Achievement Integration

**Achievement → Part Unlock Mapping**:

| Achievement | Part Unlocked | Type |
|-------------|---------------|------|
| First Blood (destroy 1 enemy) | Weapon_Laser_Tier2 | Uncommon |
| Race Winner (win 1 race) | Engine_Racing_Advanced | Rare |
| Speed Demon (reach 2000 m/s) | Wings_Racing | Rare |
| Tank Commander (survive 10 hits) | Hull_Reinforced | Rare |
| Ace Pilot (destroy 50 enemies) | Weapon_Plasma_Cannon | Epic |
| Champion (win 10 races) | Skin_Championship_Gold | Epic |
| Legendary (reach level 20) | Engine_Quantum_Drive | Legendary |

**Implementation**:
```cpp
void UAchievementManager::OnAchievementUnlocked(FName AchievementID)
{
    FName PartToUnlock = GetPartForAchievement(AchievementID);
    if (PartToUnlock != NAME_None)
    {
        // Unlock part for free
        CustomizationComp->ProgressionData.UnlockedParts.Add(PartToUnlock);
        CustomizationComp->OnPartUnlocked.Broadcast(PartToUnlock);
        CustomizationComp->SaveCustomizationData();

        // Show notification
        ShowUnlockNotification(PartToUnlock);
    }
}
```

#### 4. Mission/Race XP Rewards

**Race Rewards**:
```cpp
void ARaceGameMode::OnRaceCompleted(APlayerController* Player, int32 Position)
{
    int32 XPReward = 0;
    int32 CreditsReward = 0;

    switch (Position)
    {
        case 1: XPReward = 1000; CreditsReward = 500; break;
        case 2: XPReward = 750;  CreditsReward = 350; break;
        case 3: XPReward = 500;  CreditsReward = 200; break;
        default: XPReward = 250; CreditsReward = 100; break;
    }

    UShipCustomizationComponent* CustomComp = GetCustomizationComponent(Player);
    CustomComp->AddXP(XPReward);
    CustomComp->AddCredits(CreditsReward);

    ShowRewardScreen(XPReward, CreditsReward);
}
```

**Mission Rewards**:
```cpp
void AMissionGameMode::OnMissionCompleted(APlayerController* Player, EMissionDifficulty Difficulty)
{
    int32 XPReward = 0;
    int32 CreditsReward = 0;

    switch (Difficulty)
    {
        case EMissionDifficulty::Easy:   XPReward = 300;  CreditsReward = 200; break;
        case EMissionDifficulty::Medium: XPReward = 600;  CreditsReward = 400; break;
        case EMissionDifficulty::Hard:   XPReward = 1200; CreditsReward = 800; break;
        case EMissionDifficulty::Extreme: XPReward = 2000; CreditsReward = 1500; break;
    }

    UShipCustomizationComponent* CustomComp = GetCustomizationComponent(Player);
    CustomComp->AddXP(XPReward);
    CustomComp->AddCredits(CreditsReward);
}
```

#### 5. Progression Feedback

**Visual Feedback**:
- **Level Up Screen**: Full-screen overlay with new level, rewards, unlocks
- **XP Bar Animation**: Smooth fill animation when XP awarded
- **Unlock Notification**: Toast message "NEW PART UNLOCKED: Advanced Engine!"
- **Credit Counter Animation**: Number count-up animation

**Audio Feedback**:
- Level up fanfare sound
- Part unlock "ding" sound
- Credit earned "cash register" sound
- Achievement unlocked celebratory sound

**Haptic Feedback** (VR):
- Vibration pulse on level up
- Short vibration on part unlock
- Gentle pulse on credit earned

#### 6. Economy Tuning

**Goals**:
- Player can afford 1-2 unlocks per level
- Grinding required for legendary parts
- Economy encourages varied gameplay (races + missions)

**Tuning Spreadsheet**:
```
Level | XP Required | Credits from Level Up | Cumulative Credits | Affordable Parts
------|-------------|----------------------|-------------------|------------------
1     | 0           | 100                  | 100               | Common parts
3     | 280         | 300                  | 680               | Uncommon parts
5     | 1100        | 500                  | 1980              | Rare parts
10    | 4200        | 1000                 | 7200              | Epic parts
20    | 15000       | 2000                 | 25000             | Legendary parts
```

### Success Criteria
- [ ] XP curve feels satisfying (not too fast or slow)
- [ ] Part costs feel fair (achievable but not trivial)
- [ ] Achievement unlocks feel rewarding
- [ ] Economy is balanced (no exploits)
- [ ] Progression feedback is clear and satisfying
- [ ] Players can reach max level in 20-30 hours

### Risks & Mitigation
- **Risk**: Economy too generous → **Mitigation**: Playtesting, adjust costs/rewards
- **Risk**: XP curve too slow → **Mitigation**: Multiple tuning passes, telemetry
- **Risk**: Achievements don't unlock → **Mitigation**: Thorough testing, logging

---

## Phase 5: Advanced Features

### Status: 💡 FUTURE (Not Started)

### Timeline
- **Planned Start**: February 2026
- **Estimated Duration**: 4-6 weeks
- **Dependencies**: Phase 4 complete

### Objectives
- Procedural part generation
- Crafting system
- Marketplace system
- Synergy bonuses
- Part upgrade system
- Visual part mesh swapping
- Advanced VR interactions

### Deliverables

#### 1. Procedural Part Generation

**Feature**: Generate random parts with unique stats

**Use Cases**:
- Loot drops from enemy kills
- Reward from rare missions
- Random shop inventory
- Daily/weekly challenges

**Implementation**:
```cpp
FShipPartData GenerateRandomPart(EShipPartCategory Category, EShipPartRarity Rarity)
{
    FShipPartData Part;
    Part.PartID = FName(*FString::Printf(TEXT("Procedural_%s_%d"), ...));
    Part.Category = Category;
    Part.Rarity = Rarity;

    // Generate random stats within rarity bounds
    float PowerMultiplier = GetRarityMultiplier(Rarity);
    Part.StatModifiers.ThrustPower = FMath::FRandRange(0.8f, 1.2f) * PowerMultiplier;
    Part.StatModifiers.Mass = FMath::FRandRange(800.0f, 1200.0f);

    return Part;
}
```

**Quality Tiers**:
- **Trash**: Below-average stats
- **Normal**: Average stats
- **Quality**: Above-average stats
- **Masterwork**: Near-max stats
- **Perfect**: Maximum possible stats

#### 2. Crafting System

**Feature**: Combine parts to create upgraded parts

**Recipes**:
```
Basic Engine + Advanced Engine → Superior Engine
Rare Hull + Rare Shield → Epic Defensive Package
3× Common Parts (same category) → 1× Uncommon Part
```

**Implementation**:
- Crafting UI (ingredient slots + output preview)
- Recipe database (data table)
- Crafting costs (credits + materials)
- Success chance (based on recipe difficulty)

#### 3. Marketplace System

**Feature**: Buy/sell parts with other players

**Types**:
- **NPC Marketplace**: Fixed prices, rotating inventory
- **Player Marketplace**: Auction house, player-set prices
- **Black Market**: Rare/illegal parts, high costs

**Features**:
- Listing system (sell parts for credits)
- Bidding system (auction house)
- Transaction history
- Price tracking (market trends)

#### 4. Synergy Bonuses

**Feature**: Equipping related parts grants bonus stats

**Synergy Sets**:
- **Combat Set** (3+ parts): +15% weapon damage, +50 shield
- **Racing Set** (3+ parts): +10% speed, -10% mass
- **Tank Set** (3+ parts): +20% hull, +30% shield, -20% speed
- **Stealth Set** (3+ parts): -50% radar signature, +10% energy

**Detection**:
- Name prefix matching (`Combat_Engine`, `Combat_Weapon`, `Combat_Shield`)
- Explicit set definitions in data table
- Visual indicator in UI when set bonus active

#### 5. Part Upgrade System

**Feature**: Upgrade parts in-place to higher levels

**Upgrade Tree**:
```
Engine_Basic (Level 0)
  ↓ Upgrade (500 credits)
Engine_Basic_Lvl1 (Level 1)
  ↓ Upgrade (1000 credits)
Engine_Basic_Lvl2 (Level 2)
  ↓ Upgrade (2000 credits)
Engine_Basic_Lvl3 (Level 3 - MAX)
```

**Implementation**:
- Part data includes `UpgradeLevel`, `bCanBeUpgraded`, `ParentPartID`
- Upgrade UI shows next level stats and cost
- Upgrade function unlocks and equips next level part

#### 6. Visual Part Mesh Swapping

**Feature**: Ship visually changes based on equipped parts

**Implementation**:
- Attachment sockets on ship mesh (engine_socket, weapon_socket_L, weapon_socket_R)
- Part meshes attach to sockets
- Material overrides for color coordination
- LOD management for performance

**Challenges**:
- Art pipeline (need meshes for every part)
- Performance (many static mesh components)
- Collision (hitboxes for parts)

#### 7. Advanced VR Interactions

**Features**:
- **Physical Part Manipulation**: Grab parts with VR hands, drag to ship
- **Holographic Part Preview**: 3D hologram of part in cockpit
- **Voice Commands**: "Equip advanced engine" (speech recognition)
- **Hand Gestures**: Swipe to scroll parts, pinch to select

**Implementation**:
- VR hand tracking integration
- Physics-based UI (grabbable widgets)
- Spatial audio feedback
- Haptic feedback on interactions

### Success Criteria
- [ ] Procedural parts feel unique and valuable
- [ ] Crafting system is intuitive and rewarding
- [ ] Marketplace is functional and fair
- [ ] Synergy bonuses create interesting builds
- [ ] Part upgrades feel meaningful
- [ ] Visual mesh swapping works smoothly
- [ ] VR interactions feel natural and immersive

### Risks & Mitigation
- **Risk**: Procedural parts unbalanced → **Mitigation**: Strict stat bounds, testing
- **Risk**: Marketplace exploits → **Mitigation**: Transaction validation, logging
- **Risk**: Visual meshes too expensive (art time) → **Mitigation**: Start with materials only
- **Risk**: VR interactions janky → **Mitigation**: Extensive VR playtesting, iteration

---

## Long-Term Vision (Beyond Phase 5)

### Phase 6: Live Service Features (Future)
- Seasonal events (limited-time parts/skins)
- Battle pass system (progression rewards)
- Daily/weekly challenges
- Leaderboards integration (ranked customization)
- Community marketplace (player-created skins/parts)
- Mod support (community content)

### Phase 7: Esports Integration (Future)
- Tournament-ready loadout validation
- Competitive balancing patches
- Spectator mode enhancements
- Loadout statistics and analytics
- Meta tracking (most-used parts)

---

## Development Resources

### Team Requirements

**Phase 2** (Content Creation):
- 1× Game Designer (balancing, data entry)
- 1× Technical Artist (materials, shaders)
- 0.5× Programmer (bug fixes, tooling)

**Phase 3** (UI Development):
- 1× UI/UX Designer (wireframes, layouts)
- 1× UI Programmer (UMG implementation)
- 1× VR Specialist (VR interaction design)

**Phase 4** (Progression):
- 1× Game Designer (economy balancing)
- 0.5× Programmer (achievement integration)

**Phase 5** (Advanced):
- 1× Gameplay Programmer (crafting, marketplace)
- 1× Technical Artist (mesh swapping)
- 1× VR Specialist (advanced interactions)

### Testing Requirements

**Phase 2**: 20 hours playtesting (content validation)
**Phase 3**: 40 hours playtesting (UI/UX iteration)
**Phase 4**: 30 hours playtesting (economy balancing)
**Phase 5**: 60 hours playtesting (feature validation)

**Total**: 150 hours QA testing

---

## Conclusion

The Ship Customization System roadmap spans 5 phases over 3-4.5 months of development. Phase 1 (Core Backend) is complete and provides a solid foundation. The next critical milestone is **Phase 2** (Blueprint Assets + Data Content), which will provide the first playable content for players to experience customization.

**Recommended Next Steps**:
1. Finalize Phase 2 content plan (parts list, balancing spreadsheet)
2. Assign resources (designer, artist)
3. Create prototype data tables (5-10 parts for testing)
4. Begin material creation for skins
5. Start Phase 3 UI wireframes in parallel (for faster transition)

**Success depends on**:
- Maintaining data-driven design (easy iteration)
- Continuous playtesting (balancing feedback)
- Clear communication (design → programming → art)
- Iterative development (prototype → test → refine)

---

**Document Status**: COMPLETE
**Review Status**: READY FOR PLANNING REVIEW
**Recommended For**: Project Managers, Technical Directors, Design Leads
