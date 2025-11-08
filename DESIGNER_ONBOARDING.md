# Designer Onboarding Guide
## Ship Customization Content Pipeline

Welcome to the ship customization content creation team! This guide will get you up and running quickly.

---

## Day 1: Setup & First Content

### Morning: Environment Setup (30 minutes)

1. **Verify Python Installation**
   ```bash
   python --version
   # Should be Python 3.7+
   ```

2. **Test Tools**
   ```bash
   # Navigate to project directory
   cd C:\Users\allen\Desktop\Alexander\Alexander

   # Test content creator
   python ship_content_creator.py
   # Press 0 to exit
   ```

3. **Review Documentation**
   - Read `CONTENT_CREATION_WORKFLOW.md` (15 minutes)
   - Skim `ASSET_NAMING_CONVENTIONS.md` (5 minutes)

### Afternoon: Create Your First Content (2 hours)

#### Exercise 1: Create a Ship Part (30 minutes)

Follow along:

```bash
python ship_content_creator.py
```

1. Select option **1** (Create new ship part)
2. Enter these values:
   - Part ID: `engine_trainee_01`
   - Display Name: `Trainee Engine`
   - Description: `Your first engine!`
   - Category: **1** (Engine)
   - Rarity: **1** (Common)
   - Level Requirement: `1`
   - Unlock Cost: `0` (accept suggestion)
   - Unlocked by default: `y`
   - Customize stats: `n` (use auto-balanced)
   - Part Mesh Path: (leave empty, press Enter)
   - Icon Path: (leave empty, press Enter)
   - Parent Part ID: (leave empty, press Enter)
   - Can be upgraded: `y`
   - Upgrade Level: `0`
3. Save when prompted: `y`

✓ **Checkpoint**: You've created your first ship part!

#### Exercise 2: Use Quick Generator (15 minutes)

1. In the menu, select option **8** (Quick content generator)
2. Choose category: **2** (Thrusters)
3. Watch as it generates 5 thruster variants

✓ **Checkpoint**: You now have 6 parts total

#### Exercise 3: Create a Skin (15 minutes)

1. Select option **2** (Create new skin)
2. Enter values:
   - Skin ID: `skin_rookie_blue`
   - Display Name: `Rookie Blue`
   - Description: `A fresh pilot's first skin`
   - Rarity: **1** (Common)
   - Unlock Cost: `0`
   - Unlocked by default: `y`
   - Material Path: (leave empty)
   - Icon Path: (leave empty)
3. Save: `y`

✓ **Checkpoint**: You've created your first skin!

#### Exercise 4: Validate & Export (15 minutes)

1. Select option **5** (Validate all content)
   - Should show "All content is valid!"
2. Select option **6** (Export to Unreal format)
   - Note the output directory path
3. Select option **0** to exit

✓ **Checkpoint**: Content is ready for Unreal!

#### Exercise 5: View Your Work (15 minutes)

Open the generated files:

```bash
# View parts
notepad content_output\ship_parts.json

# View skins
notepad content_output\ship_skins.json
```

✓ **Day 1 Complete!** You've created and exported your first content.

---

## Day 2: Advanced Workflow

### Morning: Batch Creation & Organization (2 hours)

#### Exercise 6: Create a Complete Part Set

Goal: Create all parts for the "Hull" category

1. Run content creator
2. Use Quick Generator (option 8)
3. Select: **3** (Hull)
4. Result: 5 hull variants created
5. Export to Unreal

#### Exercise 7: Organize Content

```bash
# Create folder structure (would run in Unreal Python)
python organize_content.py --create-folders

# Generate naming conventions doc
python organize_content.py --generate-doc
```

Review the generated `ASSET_NAMING_CONVENTIONS.md`

### Afternoon: Import to Unreal (1 hour)

#### Exercise 8: Import Your Content

**Note**: This requires Unreal Editor to be running

```bash
# Import all content
python batch_import_content.py ./content_output
```

Watch for:
- ✓ Parts imported: X
- ✓ Skins imported: Y
- ✓ Data Tables created/updated

#### Exercise 9: Verify in Unreal

1. Open Unreal Editor
2. Navigate to `/Game/ShipCustomization/DataTables/`
3. Open `DT_ShipParts`
4. Find your `engine_trainee_01` part
5. Verify data is correct

✓ **Day 2 Complete!** You can now create and import content to Unreal.

---

## Day 3: Iteration & Balance

### Morning: Stat Balancing (2 hours)

#### Exercise 10: Understand Stat Balance

Review the stat multipliers:
- Common: 1.0x
- Uncommon: 1.25x
- Rare: 1.5x
- Epic: 1.75x
- Legendary: 2.0x

#### Exercise 11: Create Balanced Part Chains

Create a 3-tier upgrade chain for Weapons:

1. **Tier 1**: Common Laser (baseline)
   - Weapon Damage: 1.0
   - Mass: 600

2. **Tier 2**: Uncommon Laser (upgrade)
   - Weapon Damage: 1.25
   - Mass: 650
   - Parent: Tier 1

3. **Tier 3**: Rare Laser (advanced)
   - Weapon Damage: 1.5
   - Mass: 700
   - Parent: Tier 2

Use content creator to make each part, setting the ParentPartID correctly.

### Afternoon: Testing & Iteration (2 hours)

#### Exercise 12: In-Game Testing

1. Launch game in PIE (Play In Editor)
2. Open ship customization menu
3. Equip your created parts
4. Test flight performance
5. Note any balance issues

#### Exercise 13: Iterate on Design

1. Export current data from Unreal (if needed)
2. Modify stats in content creator
3. Re-import to Unreal
4. Test again
5. Repeat until satisfied

✓ **Day 3 Complete!** You understand the full iteration cycle.

---

## Week 1 Goals Checklist

By end of week 1, you should be able to:

- [ ] Create ship parts using interactive tool
- [ ] Create ship skins
- [ ] Use quick generator for batch creation
- [ ] Validate content before import
- [ ] Export to Unreal-ready JSON format
- [ ] Import content to Unreal Engine
- [ ] Verify data in Data Tables
- [ ] Understand stat balance principles
- [ ] Create upgrade chains
- [ ] Iterate on designs based on testing

---

## Common First-Week Questions

### "What stats should I use for X category?"

**Reference the base stats**:
- Engine: ThrustPower, Acceleration, Mass
- Thrusters: RotationSpeed, Mass
- Hull: HullIntegrity, Mass (high)
- Weapon: WeaponDamage, EnergyCapacity, Mass
- Shield: ShieldStrength, EnergyCapacity, Mass

Use the auto-balanced stats as a starting point!

### "How do I know if my part is balanced?"

**Compare to existing parts**:
1. Look at Common parts in same category
2. Your Common should be similar power level
3. Scale up with rarity
4. Test in-game for feel

### "I made a mistake, how do I fix it?"

**Two options**:

1. **Before import**:
   - Edit JSON file directly
   - Or load in content creator (option 7) and re-save

2. **After import**:
   - Edit Data Table in Unreal directly
   - Or re-export, fix, re-import

### "Can I copy someone else's part and modify it?"

**Yes!** Best practice:
1. Load their JSON file (option 7)
2. Find the part you want to copy
3. Modify the values
4. Change the Part ID to be unique
5. Save

---

## Best Practices for New Designers

### DO:

✓ Start with Common rarity items
✓ Use auto-balanced stats initially
✓ Test every part in-game
✓ Follow naming conventions
✓ Validate before importing
✓ Save frequently
✓ Ask questions!

### DON'T:

✗ Create Legendary items first
✗ Ignore stat balance
✗ Skip validation
✗ Use spaces in IDs
✗ Forget to set Mass
✗ Make items too powerful
✗ Work in isolation (collaborate!)

---

## Workflow Diagram

```
Design Concept
      ↓
Create in Tool (ship_content_creator.py)
      ↓
Validate (option 5)
      ↓
Export (option 6)
      ↓
Import to Unreal (batch_import_content.py)
      ↓
Verify in Data Tables
      ↓
Test in Game (PIE)
      ↓
Gather Feedback
      ↓
Iterate (back to Create in Tool)
      ↓
Polish & Publish
```

---

## Quick Reference Commands

```bash
# Create content
python ship_content_creator.py

# Import to Unreal
python batch_import_content.py ./content_output

# Organize assets
python organize_content.py

# Generate documentation
python organize_content.py --generate-doc
```

---

## Week 2+ Goals

After mastering the basics, you'll move on to:

- Creating physics configurations
- Linking visual assets (meshes, materials)
- Advanced stat balancing
- Creating themed content sets
- Collaborating with artists
- Balancing multiplayer considerations
- Creating seasonal/event content

---

## Getting Help

### Resources:

1. **Documentation**:
   - `CONTENT_CREATION_WORKFLOW.md` - Complete guide
   - `ASSET_NAMING_CONVENTIONS.md` - Naming rules
   - This file - Onboarding steps

2. **Examples**:
   - `examples/example_ship_parts.json`
   - `examples/example_ship_skins.json`
   - `examples/example_physics_config_balanced.json`

3. **Team**:
   - Technical Lead: For tool issues
   - Senior Designer: For balance questions
   - Artist: For asset creation
   - QA: For testing feedback

### When Stuck:

1. Check the FAQ in `CONTENT_CREATION_WORKFLOW.md`
2. Review example files
3. Try validation tool (often shows the issue)
4. Ask a teammate
5. Check Unreal logs (`Saved/Logs/`)

---

## Success Metrics

You're doing great if:

- ✓ You can create a complete part in under 5 minutes
- ✓ Your parts pass validation on first try
- ✓ You understand stat trade-offs
- ✓ You can explain your design decisions
- ✓ Your content works in-game without issues

---

## Next Steps

After completing onboarding:

1. **Week 2**: Create a complete themed set (e.g., "Racing Pack")
2. **Week 3**: Collaborate with artist on visual assets
3. **Week 4**: Create content for upcoming update
4. **Ongoing**: Contribute to content roadmap

---

## Welcome to the Team!

You now have everything you need to start creating amazing ship customization content. Remember:

- **Start simple** and build complexity
- **Test early and often**
- **Ask questions** - we're here to help
- **Have fun!** This is creative work

Good luck, and happy creating! 🚀

---

**Revision History**:
- v1.0 - Initial onboarding guide created
