# Ship Customization Guide - Complete Documentation Package

## Overview

This package contains comprehensive documentation for creating and managing ship customization data assets in Unreal Engine for the Alexander project. Everything you need to create, balance, and deploy ship parts, skins, and physics configurations is included.

## Files Included

### 1. **BLUEPRINT_DATA_ASSET_CREATION_GUIDE.md** (Main Guide - 62 KB)

The comprehensive designer's guide with everything needed to create ship customization content.

**Contents:**
- Section 1: Creating Ship Parts Data Tables (30 steps) - Engine, Thruster, Hull, Wing, Weapon, Shield, Utility categories
- Section 2: Creating Ship Skins Data Tables (20 steps) - Visual customization
- Section 3: Creating Ship Physics Configs (25 steps) - Flight characteristic presets
- Section 4: Setting Up Ship Blueprint (20 steps) - Component integration
- Section 5: Testing in PIE (15 steps) - Validation and debugging
- Section 6: Common Issues & Solutions (10 problem/solution pairs)
- Section 7: Content Organization Best Practices
- Section 8: Part Balancing Guidelines (multiplier ranges, progression pacing)
- Section 9: Example Preset Parts (20 complete part examples with full stats)
- Section 10: Video Tutorial Script (10-minute tutorial breakdown)
- Appendices: CSV import format, initial content package, advanced topics, troubleshooting

**Key Sections for Different Users:**
- **Designers:** Start with Sections 1-4
- **QA/Testers:** Focus on Section 5-6
- **Balancers:** Read Sections 8-9
- **Leads:** Review Section 7 + Appendices

---

### 2. **RECOMMENDED_CONTENT_PACKAGE_SPEC.md** (16 KB)

Detailed specification for the launch content package - exactly what parts to create and why.

**Contents:**
- Executive summary (25 parts across 7 categories)
- Recommended parts breakdown with balance rationale
- 3 physics configuration set (Light Fighter, Balanced Scout, Heavy Freighter)
- 8 skin selection with rarity distribution
- 2-week implementation roadmap
- Success criteria and launch metrics
- Future expansion ideas (phases 3-5)
- Asset requirements checklist
- Cost/benefit analysis (~80 designer-hours investment)
- Known limitations and future improvements

**Use This For:**
- Project planning and timeline estimation
- Understanding recommended part balancing
- Determining asset creation priorities
- Managing scope and expectations

---

### 3. Example CSV Files (5 KB each)

Pre-formatted CSV files for bulk importing parts into data tables. Import these directly into Unreal Editor to populate data tables quickly.

**Files:**
- `EXAMPLE_SHIP_PARTS_ENGINES.csv` - 8 engine variants from basic to experimental
- `EXAMPLE_SHIP_PARTS_THRUSTERS.csv` - 8 thruster variants with varying agility
- `EXAMPLE_SHIP_PARTS_HULLS.csv` - 8 hull variants from light to adaptive
- `EXAMPLE_SHIP_PARTS_WEAPONS.csv` - 8 weapon variants from pulse to super-laser
- `EXAMPLE_SHIP_PARTS_SHIELDS.csv` - 8 shield variants from basic to fortress
- `EXAMPLE_SHIP_SKINS.csv` - 15 skin color variants with different rarities

**How to Use:**
1. In Unreal Editor, create a data table with appropriate row structure (FShipPartData, FShipSkinData)
2. Right-click table → Asset Actions → Import from CSV
3. Select the CSV file
4. Data populates automatically with all stats and properties

---

## Quick Start Guide

### For Brand New Designers (0-2 hours)

1. **Read:** BLUEPRINT_DATA_ASSET_CREATION_GUIDE.md - Quick Start Overview
2. **Read:** RECOMMENDED_CONTENT_PACKAGE_SPEC.md - Executive Summary
3. **Skim:** Section 1 of the main guide (30 steps for engines)
4. **Download:** EXAMPLE_SHIP_PARTS_ENGINES.csv
5. **Import:** CSV into new data table
6. **Result:** 8 engine parts ready to test

### For Experienced Level Designers (2-6 hours)

1. **Review:** Sections 1-4 of main guide
2. **Review:** Section 8 (Balancing Guidelines)
3. **Create:** One data table category (e.g., engines) from scratch
4. **Test:** In PIE mode using instructions in Section 5
5. **Create:** 2-3 more categories
6. **Balance:** Using spreadsheet to track relative power

### For Project Leads (1 hour)

1. **Read:** RECOMMENDED_CONTENT_PACKAGE_SPEC.md (full)
2. **Review:** Implementation Roadmap section
3. **Check:** Asset Requirements Checklist
4. **Note:** Cost/Benefit Analysis (~80 hours)
5. **Plan:** 2-week sprint for content creation + 1 week QA

---

## File Organization

Recommended folder structure in Unreal Editor:

```
Content/
├── Data/
│   ├── ShipParts/
│   │   ├── DT_ShipParts_Engines.uasset
│   │   ├── DT_ShipParts_Thrusters.uasset
│   │   ├── DT_ShipParts_Hulls.uasset
│   │   ├── DT_ShipParts_Wings.uasset
│   │   ├── DT_ShipParts_Cockpits.uasset
│   │   ├── DT_ShipParts_Weapons.uasset
│   │   ├── DT_ShipParts_Shields.uasset
│   │   └── DT_ShipParts_Utility.uasset
│   ├── ShipSkins/
│   │   ├── DT_ShipSkins_Default.uasset
│   │   ├── DT_ShipSkins_Premium.uasset
│   │   └── DT_ShipSkins_Limited.uasset
│   └── PhysicsConfigs/
│       ├── DA_LightFighter.uasset
│       ├── DA_HeavyFreighter.uasset
│       ├── DA_BalancedScout.uasset
│       └── DA_RacingShip.uasset
├── Materials/
│   └── ShipSkins/
│       ├── M_ShipSkin_White.uasset
│       ├── M_ShipSkin_Racing.uasset
│       └── ... (other skin materials)
├── Meshes/
│   └── ShipParts/
│       ├── Engine_Basic.uasset
│       ├── Engine_Advanced.uasset
│       └── ... (other part meshes)
└── Blueprints/
    └── Ships/
        └── BP_VRSpaceshipPlayer.uasset
```

---

## Key Concepts Explained

### Data Tables vs. Data Assets

**Data Tables (DT_*):**
- Store multiple rows of structured data
- Best for: Lists of similar items (all engines, all weapons)
- Format: FTableRowBase struct (FShipPartData, FShipSkinData)
- Edit: Double-click to open table editor
- Example: DT_ShipParts_Engines contains 8 engine variants

**Data Assets (DA_*):**
- Store single instances of configuration data
- Best for: Unique presets and configurations
- Format: Any UDataAsset-derived class (UShipPhysicsConfig)
- Edit: Double-click to open Details panel
- Example: DA_LightFighter contains flight characteristics

### ShipStats Structure

10 core statistics that can be modified by parts:

```
ThrustPower (multiplicative)        → Acceleration capability
MaxVelocity (multiplicative)        → Top speed limit
RotationSpeed (multiplicative)      → Turn rate
Acceleration (multiplicative)       → How quickly thrust builds
HullIntegrity (additive)            → Ship health
ShieldStrength (additive)           → Shield capacity
WeaponDamage (multiplicative)       → Weapon output
EnergyCapacity (additive)           → Energy reserves
EnergyRegenRate (additive)          → Energy recovery per second
Mass (additive)                     → Ship weight (affects inertia)
```

### Rarity System

| Rarity | Level Req | Cost Range | Multiplier Range | Use |
|--------|-----------|------------|------------------|-----|
| Common | 1-3 | 0-200 | 0.9-1.1 | Starter parts |
| Uncommon | 3-6 | 100-400 | 1.0-1.3 | Early upgrades |
| Rare | 6-12 | 400-1000 | 1.2-1.6 | Mid-game choices |
| Epic | 12-18 | 1000-3000 | 1.5-1.9 | Late-game power |
| Legendary | 18-20 | 3000-5000 | 1.8-2.0 | Endgame top tier |

### Part Categories

**7 Main Categories:**
1. **Engines** - Primary thrust (biggest impact on speed)
2. **Thrusters** - Rotation and maneuverability (affects turning)
3. **Hulls** - Durability and structural integrity (affects health)
4. **Weapons** - Offensive capability (affects damage output)
5. **Shields** - Defensive layer (protects hull)
6. **Wings** - Aerodynamics (affects top speed and handling)
7. **Utility** - Misc enhancements (miscellaneous bonuses)
8. **Cockpit** - Control interface (optional, affects responsiveness)

---

## Balancing Philosophy

### Core Principles

1. **No Single Best Build**
   - Each part should have trade-offs
   - Powerful parts should have drawbacks (heavy, expensive, slow, high-energy)
   - Every rarity should have at least 2 viable options

2. **Trade-Offs Create Interesting Choices**
   - Fast engines are light but fragile
   - Heavy armor reduces speed but increases durability
   - High-damage weapons consume lots of energy
   - Shields are expensive and heavy but protect well

3. **Progression Feels Meaningful**
   - Each level unlock should matter
   - Each new part should be visibly better (not just different)
   - Upgrades should be 10-30% better than previous tier
   - Spacing matters - unlock every 3-5 levels for good pacing

4. **Skill Rewards Mastery**
   - Overpowered parts should require skill to use
   - Glass cannons (high damage, low HP) reward skilled aim
   - Agile ships reward good piloting
   - Tank builds should still be beatable by skilled racers

### Balancing Checklist

Before shipping any part, verify:

- [ ] Stat multipliers are within reasonable range (0.5 to 2.0)
- [ ] Mass values make physical sense
- [ ] Level requirements don't have gaps (shouldn't jump from level 2 to level 8)
- [ ] Unlock costs scale with rarity (common < uncommon < rare < epic < legendary)
- [ ] Parts in same category aren't pure upgrades
- [ ] No single part enables impossible strategies
- [ ] Upgrade chains are clear and intuitive
- [ ] Specialist builds are possible (racer, tank, fighter, balanced)

---

## Implementation Timeline

### Week 1: Content Creation
- **Days 1-2:** Create engine parts (4 variants)
- **Day 3:** Create weapons (4 variants)
- **Day 4:** Create remaining categories (thrusters, hulls, shields, wings, utility)
- **Day 5:** Create physics configs and integrate into ship blueprint

### Week 2: Testing & Balancing
- **Days 1-2:** Test all parts load correctly, verify no crashes
- **Day 3:** Play-test each of 4 recommended builds
- **Day 4:** Balance adjustments based on testing
- **Day 5:** Final polish, documentation, sign-off

### Total Time: ~80 designer-hours

---

## Common Mistakes to Avoid

### Mistake 1: Multiplicative vs. Additive Confusion
**Wrong:** Set ThrustPower to 1500 (thinking it's 1.5x)
**Right:** Set ThrustPower to 1.5 (it's a 1.5x multiplier)
**Remember:** Multipliers between 0.5 and 2.0; additives in raw units

### Mistake 2: Overlapping Level Requirements
**Wrong:** Engine_Basic (level 1), Engine_Advanced (level 1), Engine_Racing (level 5)
**Right:** Engine_Basic (level 1), Engine_Advanced (level 5), Engine_Racing (level 10)
**Remember:** Space out unlocks every 3-5 levels for progression feel

### Mistake 3: Cost Not Scaling with Rarity
**Wrong:** Common part costs 5000, Uncommon costs 200
**Right:** Common costs 0-100, Uncommon costs 200-400, Rare costs 500-1000
**Remember:** Use formula: `Cost = 100 × RarityMult × (1 + LevelReq/20)`

### Mistake 4: Forgetting Weight Physics
**Wrong:** High-damage weapon that's also light, fast, and cheap
**Right:** High-damage weapon that's heavy (600+ kg), expensive, slow-turning
**Remember:** More power = more mass (or higher cost/level requirement)

### Mistake 5: Part Overload
**Wrong:** Creating 50 weapons "for variety"
**Right:** Creating 4-5 carefully balanced weapon types
**Remember:** Fewer well-balanced options > more confusing choices

### Mistake 6: No Upgrade Chains
**Wrong:** All parts are standalone, no progression path
**Right:** Engine_Basic → Engine_Advanced → Engine_Racing
**Remember:** Upgrade chains show clear progression to players

### Mistake 7: Forgetting to Test Integration
**Wrong:** Creating parts but not assigning to ship blueprint
**Right:** Creating parts, assigning to component, testing in PIE
**Remember:** Test in game mode to verify stats actually apply

---

## Troubleshooting Reference

### Data Table Won't Open
**Solution:** Check row structure is FShipPartData, regenerate VS project files, recompile C++

### Parts Don't Show in Game
**Solution:** Verify ShipPartsDataTable is assigned in component, check data table actually has rows, verify player level meets requirement

### Stats Don't Apply
**Solution:** Verify ApplyStatsToFlightController is called, check FlightController component exists, verify stat values aren't zero

### CSV Import Fails
**Solution:** Check CSV format matches expected columns, verify no special characters in values, ensure data types match (numbers, booleans as TRUE/FALSE)

**Full troubleshooting guide:** See Section 6 and Appendix D of main guide

---

## Advanced Topics

### Creating Synergy Bonuses
Group related parts by naming convention:
- Combat_Engine, Combat_Weapon, Combat_Thruster
- Racing_Engine, Racing_Hull, Racing_Wings
- Detect prefixes in code, apply +10% bonus if full set equipped

### Adding Limited-Time Skins
Create seasonal or event-specific skins with legendary rarity:
- Halloween event → Skin_Halloween (cost 2000, level 1, limited availability)
- Championship victory → Skin_Champion (cost 0, achievement-locked)
- Anniversary → Skin_Anniversary (cost 1500, 1-week window)

### Building Synergy Chains
Create upgrade progressions that encourage specific builds:
- "Fighter Fleet" - all combat-oriented parts
- "Racer Collection" - all speed-oriented parts
- "Tank Arsenal" - all defense-oriented parts

### Performance Optimization
- Use soft object pointers for large assets (meshes, materials)
- Split data tables once they exceed 200 rows
- Cache loaded data in component (already implemented)
- Profile with Unreal Insights if experiencing slowness

---

## Support & Resources

### Documentation Files Included
- ✓ BLUEPRINT_DATA_ASSET_CREATION_GUIDE.md - Main comprehensive guide
- ✓ RECOMMENDED_CONTENT_PACKAGE_SPEC.md - Content specification
- ✓ 6 Example CSV files - Ready-to-import part data
- ✓ This README file - Navigation and overview

### Source Code References
- `C:\Users\allen\Desktop\Alexander\Alexander\Source\Alexander\Public\ShipCustomizationComponent.h` - Component definition
- `C:\Users\allen\Desktop\Alexander\Alexander\Source\Alexander\Public\ShipPhysicsConfig.h` - Physics config class
- `C:\Users\allen\Desktop\Alexander\Alexander\Source\Alexander\Private\ShipCustomizationComponent.cpp` - Implementation

### Related Documentation
- Unreal Engine Data Tables Documentation
- Unreal Engine Data Assets Documentation
- Alexander Game Design Document (see your project folder)
- Alexander Code Architecture Guide

---

## FAQ

**Q: How many parts should I create?**
A: Start with 25 (4 engines, 3 thrusters, 3 hulls, 4 weapons, 3 shields, 2 wings, 2 utility). This provides good variety without overwhelming players.

**Q: Can I have more than one weapon equipped?**
A: By default, no - each category is mutually exclusive. To allow multiple weapons, modify the loadout system to use arrays instead of maps.

**Q: Should parts have meshes?**
A: Optional for launch. You can use material swaps on a single base mesh, or add unique meshes later for visual differentiation.

**Q: How do I balance costs?**
A: Use the formula: `Cost = 100 × RarityMult × (1 + LevelReq/20)`. Adjust the base cost (100) to tune economy difficulty.

**Q: Can players create their own parts?**
A: Not with this system - parts are designer-created data. To allow player creation, you'd need to build a UI editor for part customization.

**Q: How do I export/backup part data?**
A: Right-click data table → Asset Actions → Export to CSV. Regularly backup CSVs to version control.

**Q: What if I want to add new stats?**
A: Extend FShipStats struct in ShipCustomizationComponent.h, recompile, then data tables will show new fields.

**Q: How long does it take to create all this?**
A: ~80 designer-hours total (see implementation timeline). Experienced designers ~40-50 hours, new designers ~100-120 hours.

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | Nov 2025 | Initial comprehensive guide package |

---

## Contact & Feedback

For questions about this documentation or ship customization system, contact:
- **Design Lead:** [Your Name]
- **Technical Lead:** [Your Name]
- **Project Manager:** [Your Name]

Document Location: `C:\Users\allen\Desktop\Alexander\Alexander\`

---

## License & Attribution

These guides are created for the Alexander project. Adapt freely for your own projects.

**Based on:**
- Unreal Engine 5 best practices
- Shipped game design patterns
- Community game design documentation

---

**Last Updated:** November 7, 2025
**Document Status:** READY FOR PRODUCTION
**Recommended For:** Level Designers, Game Designers, Content Creators, Project Leads
