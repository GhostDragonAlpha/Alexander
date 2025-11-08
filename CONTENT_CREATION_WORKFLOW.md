# Ship Customization Content Creation Workflow

**Complete Guide for Designers**

---

## Table of Contents

1. [Overview](#overview)
2. [Quick Start](#quick-start)
3. [Tools Overview](#tools-overview)
4. [Workflow Steps](#workflow-steps)
5. [Creating Ship Parts](#creating-ship-parts)
6. [Creating Ship Skins](#creating-ship-skins)
7. [Creating Physics Configs](#creating-physics-configs)
8. [Content Organization](#content-organization)
9. [Importing to Unreal](#importing-to-unreal)
10. [Validation and Testing](#validation-and-testing)
11. [Best Practices](#best-practices)
12. [Troubleshooting](#troubleshooting)
13. [FAQ](#faq)

---

## Overview

This guide provides a complete workflow for creating ship customization content, from concept to in-game implementation. You'll learn how to use the provided tools to create ship parts, skins, and physics configurations efficiently.

### What You Can Create

- **Ship Parts**: Engines, thrusters, hulls, wings, cockpits, weapons, shields, utilities
- **Ship Skins**: Visual customization with different rarities
- **Physics Configs**: Flight handling presets

### Tools Provided

1. **ship_content_creator.py** - Interactive content creation tool
2. **batch_import_content.py** - Batch import JSON files to Unreal
3. **organize_content.py** - Organize assets into proper folder structure
4. **ShipContentEditorUtility** - C++ utility for advanced operations

---

## Quick Start

### 5-Minute Quickstart

```bash
# 1. Create content interactively
python ship_content_creator.py

# 2. Generate a complete set of parts (option 8 in menu)
# Select "Engine" category to create 5 engine variants

# 3. Export to Unreal format (option 6 in menu)

# 4. Import to Unreal
python batch_import_content.py ./content_output

# 5. Done! Your content is now in Unreal Engine
```

---

## Tools Overview

### 1. Ship Content Creator (Interactive)

**Purpose**: Menu-driven tool for creating content

**Features**:
- Interactive prompts with validation
- Auto-balanced stat generation
- Quick batch generation
- Real-time validation
- Export to Unreal format

**Usage**:
```bash
python ship_content_creator.py
```

**Menu Options**:
1. Create new ship part
2. Create new skin
3. Create physics config
4. View all content
5. Validate all content
6. Export to Unreal format
7. Load existing content
8. Quick content generator
9. Help & Documentation
0. Exit

### 2. Batch Import Content

**Purpose**: Automatically import JSON files to Unreal Engine Data Tables

**Features**:
- Auto-detects file types
- Creates Data Tables if missing
- Validates content during import
- Reports success/failures

**Usage**:
```bash
# Import single directory
python batch_import_content.py ./content_output

# Import with options
python batch_import_content.py ./content_output --no-recursive --no-validate
```

### 3. Content Organizer

**Purpose**: Create folder structure and organize assets

**Features**:
- Creates proper folder hierarchy
- Moves assets to correct locations
- Validates naming conventions
- Generates documentation

**Usage**:
```bash
# Create folders and organize
python organize_content.py

# Just create folders
python organize_content.py --create-folders

# Validate naming
python organize_content.py --validate

# Auto-fix naming issues
python organize_content.py --validate --auto-fix

# Generate naming conventions doc
python organize_content.py --generate-doc
```

### 4. C++ Editor Utility

**Purpose**: Advanced operations in Unreal Editor

**Features**:
- Import/export JSON
- Create Data Tables
- Validate content
- Generate balanced stats
- Organize assets

**Usage** (in Unreal Editor):
```cpp
// Load utility
UShipContentEditorUtility* Utility = NewObject<UShipContentEditorUtility>();

// Import parts
FContentImportResult Result = Utility->ImportPartsFromJSON(
    "C:/content/ship_parts.json",
    PartsDataTable
);

// Validate all content
FContentValidationResult ValidationResult = Utility->ValidateAllContent();
```

---

## Workflow Steps

### Complete Workflow (Concept to In-Game)

```
1. DESIGN
   ↓
   Create concept (stats, visuals, gameplay role)
   ↓

2. CREATE CONTENT
   ↓
   Use ship_content_creator.py to define parts/skins
   ↓

3. VALIDATE
   ↓
   Use validation tools to check for errors
   ↓

4. CREATE ASSETS (if needed)
   ↓
   Create meshes, materials, icons in DCC tools
   ↓

5. ORGANIZE
   ↓
   Use organize_content.py to set up folders
   ↓

6. IMPORT TO UNREAL
   ↓
   Use batch_import_content.py
   ↓

7. LINK ASSETS
   ↓
   Update Data Tables with asset references
   ↓

8. TEST IN-GAME
   ↓
   Playtest and iterate
   ↓

9. BALANCE
   ↓
   Adjust stats based on testing
   ↓

10. PUBLISH
    ↓
    Content ready for players!
```

---

## Creating Ship Parts

### Step-by-Step Guide

#### 1. Launch the Content Creator

```bash
python ship_content_creator.py
```

#### 2. Select Option 1: "Create new ship part"

#### 3. Fill in Required Information

**Part ID**:
- Format: `<category>_<name>_<variant>`
- Example: `engine_basic_01`
- Must be lowercase with underscores
- Must be unique

**Display Name**:
- User-friendly name shown in UI
- Example: "Basic Engine"

**Description**:
- Brief description of the part
- Example: "A standard propulsion system suitable for beginners"

**Category**:
Select from:
1. Engine - Main propulsion
2. Thrusters - Maneuvering
3. Hull - Structure and armor
4. Wings - Aerodynamics
5. Cockpit - Control systems
6. Weapon - Offensive systems
7. Shield - Defensive systems
8. Utility - Special equipment

**Rarity**:
Select from:
1. Common (1.0x stats)
2. Uncommon (1.25x stats)
3. Rare (1.5x stats)
4. Epic (1.75x stats)
5. Legendary (2.0x stats)

**Level Requirement**:
- Minimum player level to unlock
- Range: 1-50
- Suggested: Match rarity (Common=1, Uncommon=5, Rare=10, Epic=20, Legendary=30)

**Unlock Cost**:
- Credits required to unlock
- Auto-calculated based on rarity and level
- Can be customized

**Stats**:
- Auto-generated based on category and rarity
- Can be customized if needed
- See [Stat Balance Guide](#stat-balance-guide) for recommendations

#### 4. Review and Save

The tool will show a summary and ask if you want to save. Choose "Yes" to save immediately.

### Quick Batch Generation

For creating multiple related parts quickly:

1. Select Option 8: "Quick content generator"
2. Choose a category
3. Tool generates 5 variants (one per rarity)
4. Creates upgrade chain automatically
5. Auto-saves to file

**Example Output**:
```
engine_common_01     (Level 1, 0 credits)
engine_uncommon_02   (Level 2, 1000 credits)
engine_rare_03       (Level 3, 6000 credits)
engine_epic_04       (Level 4, 20000 credits)
engine_legendary_05  (Level 5, 50000 credits)
```

---

## Creating Ship Skins

### Step-by-Step Guide

#### 1. Select Option 2: "Create new skin"

#### 2. Fill in Information

**Skin ID**:
- Format: `skin_<theme>_<variant>`
- Example: `skin_racing_red`
- Must start with "skin_"
- Must be lowercase with underscores

**Display Name**:
- User-friendly name
- Example: "Racing Red"

**Description**:
- Brief description
- Example: "Bold red paint scheme for competitive pilots"

**Rarity**:
- Determines unlock cost
- Common to Legendary

**Unlock Cost**:
- Auto-calculated based on rarity
- Can be customized

**Asset References** (optional initially):
- Material Path: Path to material instance
- Icon Path: Path to icon texture

#### 3. Save

Skins are automatically saved to `./content_output/ship_skins.json`

### Skin Naming Themes

Suggested themes for organization:

- **Racing**: `skin_racing_red`, `skin_racing_blue`
- **Military**: `skin_military_camo`, `skin_military_stealth`
- **Corporate**: `skin_corp_executive`, `skin_corp_industrial`
- **Faction**: `skin_federation_navy`, `skin_rebel_fighter`
- **Special**: `skin_golden_ace`, `skin_neon_racer`

---

## Creating Physics Configs

### Step-by-Step Guide

#### 1. Select Option 3: "Create physics config"

#### 2. Choose Preset or Custom

**Presets Available**:

1. **Balanced** (All-rounder)
   - Thrust: 50000
   - Rotation: 20000
   - Max Speed: 1000
   - Mass: 10000

2. **Speed** (High speed, low armor)
   - Thrust: 70000
   - Rotation: 15000
   - Max Speed: 1500
   - Mass: 8000

3. **Tank** (High armor, low speed)
   - Thrust: 40000
   - Rotation: 10000
   - Max Speed: 600
   - Mass: 15000

4. **Agile** (High maneuverability)
   - Thrust: 45000
   - Rotation: 30000
   - Max Speed: 900
   - Mass: 7000

5. **Custom** (Define your own)

#### 3. Name and Save

Give your config a descriptive name:
- `Balanced`, `Speed`, `Tank`, `Agile`
- `Fighter`, `Bomber`, `Interceptor`
- `Cargo`, `Explorer`, `Racer`

---

## Content Organization

### Folder Structure

After using `organize_content.py`, your content will be organized as:

```
/Game/ShipCustomization/
├── DataTables/
│   ├── DT_ShipParts          # Ship parts data table
│   └── DT_ShipSkins          # Ship skins data table
│
├── DataAssets/
│   ├── PC_Balanced           # Physics config assets
│   ├── PC_Speed
│   └── PC_Tank
│
├── Materials/
│   ├── Masters/
│   │   ├── M_Ship_Hull       # Master materials
│   │   └── M_Ship_Engine
│   └── Instances/
│       ├── MI_Ship_Hull_Red  # Material instances
│       └── MI_Ship_Hull_Blue
│
├── Meshes/
│   ├── Engines/
│   │   ├── SM_Ship_Engine_Basic_01
│   │   └── SM_Ship_Engine_Advanced_01
│   ├── Thrusters/
│   ├── Hulls/
│   ├── Wings/
│   ├── Cockpits/
│   └── Weapons/
│
├── Icons/
│   ├── T_Icon_Engine_Basic01
│   └── T_Icon_Thruster_Basic01
│
└── Textures/
    ├── T_Ship_Hull_Diffuse
    └── T_Ship_Engine_Normal
```

### Naming Conventions

**Parts**:
- ID: `<category>_<name>_<variant>` (lowercase, underscores)
- Mesh: `SM_Ship_<Category>_<Name>_<Variant>` (PascalCase)

**Skins**:
- ID: `skin_<theme>_<variant>` (lowercase, underscores)
- Material: `MI_Ship_<Name>_<Variant>` (PascalCase)

**Icons**:
- `T_Icon_<Category>_<Name><Variant>` (PascalCase, no underscores in variant)

See `ASSET_NAMING_CONVENTIONS.md` (auto-generated) for complete guide.

---

## Importing to Unreal

### Automatic Import

```bash
# Import all JSON files from directory
python batch_import_content.py ./content_output

# Import with validation
python batch_import_content.py ./content_output --validate

# Import specific directory only (no subdirectories)
python batch_import_content.py ./content_output --no-recursive
```

### Import Process

The batch importer will:
1. ✓ Detect file types automatically
2. ✓ Create Data Tables if they don't exist
3. ✓ Import parts to `DT_ShipParts`
4. ✓ Import skins to `DT_ShipSkins`
5. ✓ Import configs to Data Assets
6. ✓ Validate content structure
7. ✓ Report success/failures

### Manual Import (Advanced)

Using the C++ Editor Utility in Unreal:

1. Open Unreal Editor
2. Load `BP_ShipContentEditorUtility` (or create from C++ class)
3. Set file paths
4. Call `ImportPartsFromJSON()` or `ImportSkinsFromJSON()`
5. Check logs for results

---

## Validation and Testing

### Pre-Import Validation

**Using Content Creator**:
```bash
python ship_content_creator.py
# Select option 5: "Validate all content"
```

**Using Batch Importer**:
```bash
python batch_import_content.py ./content_output --validate
```

### In-Editor Validation

**Using C++ Utility**:
```cpp
UShipContentEditorUtility* Utility = NewObject<UShipContentEditorUtility>();
FContentValidationResult Result = Utility->ValidateAllContent();

if (Result.bIsValid)
{
    UE_LOG(LogTemp, Log, TEXT("All content is valid!"));
}
else
{
    for (const FString& Error : Result.ValidationErrors)
    {
        UE_LOG(LogTemp, Error, TEXT("%s"), *Error);
    }
}
```

### Common Validation Errors

| Error | Cause | Solution |
|-------|-------|----------|
| "PartID is required" | Missing or empty Part ID | Add valid Part ID |
| "Mass must be positive" | Mass set to 0 or negative | Set mass > 0 |
| "Duplicate PartID" | Two parts with same ID | Make IDs unique |
| "Invalid stat values" | Stats out of range | Check stat ranges |
| "Broken asset reference" | Asset path doesn't exist | Update path or create asset |

### In-Game Testing

1. **Load Game**:
   - Start PIE (Play In Editor)
   - Open ship customization menu

2. **Test Part**:
   - Equip the new part
   - Check visual appearance
   - Test stat changes
   - Verify unlock requirements

3. **Balance Testing**:
   - Compare with existing parts
   - Test in different scenarios
   - Get player feedback
   - Iterate on stats

4. **Performance Testing**:
   - Check frame rate impact
   - Verify memory usage
   - Test with multiple parts equipped

---

## Best Practices

### Design Principles

1. **Start Simple**:
   - Begin with Common rarity items
   - Build upgrade chains gradually
   - Test thoroughly before adding more

2. **Balance Carefully**:
   - Use auto-generated stats as baseline
   - Make small, incremental changes
   - Test in actual gameplay

3. **Be Consistent**:
   - Follow naming conventions
   - Use similar stat ranges for same category
   - Maintain visual style

4. **Think Progression**:
   - Create clear upgrade paths
   - Make higher rarities feel meaningful
   - Don't make Common items useless

### Stat Balance Guide

**General Guidelines**:
- Common: 1.0x baseline
- Uncommon: 1.25x baseline (+25%)
- Rare: 1.5x baseline (+50%)
- Epic: 1.75x baseline (+75%)
- Legendary: 2.0x baseline (+100%)

**Trade-offs**:
- Higher thrust = higher mass
- Higher speed = lower armor
- Higher damage = lower energy capacity
- Higher shields = lower mobility

**Recommended Stat Ranges**:

| Stat | Common | Legendary | Notes |
|------|--------|-----------|-------|
| ThrustPower | 0.8-1.2 | 1.6-2.4 | Affects acceleration |
| MaxVelocity | 0.9-1.1 | 1.8-2.2 | Top speed multiplier |
| RotationSpeed | 0.8-1.2 | 1.6-2.4 | Turn rate |
| Mass | 500-2000 | 800-3000 | Affects handling |
| HullIntegrity | 50-150 | 100-300 | Health pool |
| ShieldStrength | 0-50 | 50-150 | Shield capacity |

### Content Creation Tips

1. **Use Quick Generator**:
   - Perfect for complete category sets
   - Auto-creates upgrade chains
   - Ensures consistent balance

2. **Iterate Often**:
   - Create → Test → Adjust → Repeat
   - Get feedback early
   - Don't be afraid to change

3. **Document Your Work**:
   - Keep notes on design intent
   - Track balance changes
   - Note player feedback

4. **Collaborate**:
   - Share JSON files with team
   - Use version control
   - Review each other's work

### Asset Creation Workflow

1. **Concept First**:
   - Create part definition in tool
   - Test stats and balance
   - Get approval before creating assets

2. **Create Placeholder**:
   - Use basic mesh/material initially
   - Test functionality
   - Iterate on design

3. **Create Final Assets**:
   - Model in DCC tool (Blender, Maya, etc.)
   - Export to Unreal
   - Use organize_content.py to organize

4. **Link Assets**:
   - Open Data Table in Unreal
   - Update asset paths
   - Verify references load correctly

---

## Troubleshooting

### Common Issues

#### "File not found" error

**Problem**: JSON file can't be found

**Solution**:
```bash
# Check file exists
ls ./content_output/ship_parts.json

# Use absolute path
python batch_import_content.py "C:/Projects/Alexander/content_output"
```

#### "Data Table is null" error

**Problem**: Data Table doesn't exist or wrong path

**Solution**:
1. Check Data Table exists at `/Game/ShipCustomization/DataTables/DT_ShipParts`
2. Create manually if missing
3. Update path in script configuration

#### Import succeeds but items don't appear

**Problem**: Content imported but not visible in-game

**Solution**:
1. Check Data Table in Content Browser
2. Verify rows were added
3. Check unlock requirements (level, cost)
4. Ensure `UnlockedByDefault` is true for testing

#### Stats don't apply in-game

**Problem**: Equipped part doesn't change stats

**Solution**:
1. Verify `ShipCustomizationComponent` is attached to ship
2. Check `ApplyStatsToFlightController()` is being called
3. Ensure Data Tables are assigned in component
4. Test with logging enabled

#### Validation fails with "Invalid stat values"

**Problem**: Stats are out of acceptable range

**Solution**:
1. Check stat values are positive
2. Ensure Mass > 0
3. Keep multipliers reasonable (0.1 - 5.0)
4. Use auto-balanced stats as reference

### Debug Mode

Enable verbose logging:

```bash
# Content Creator
python ship_content_creator.py --debug

# Batch Importer
python batch_import_content.py ./content_output --verbose

# Organizer
python organize_content.py --debug
```

### Getting Help

1. **Check Logs**:
   - Python scripts output detailed errors
   - Unreal logs in `Saved/Logs/`
   - Look for error messages

2. **Validate First**:
   - Always run validation before import
   - Fix errors one at a time
   - Test after each fix

3. **Use Examples**:
   - Reference existing parts in Data Tables
   - Copy structure from working items
   - Compare JSON formats

4. **Test Incrementally**:
   - Import one file at a time
   - Test each part individually
   - Isolate problematic items

---

## FAQ

### General Questions

**Q: Do I need to know programming to use these tools?**

A: No! The interactive content creator is menu-driven and requires no programming knowledge. Just follow the prompts.

**Q: Can I edit existing parts?**

A: Yes! Load the JSON file (option 7), modify the data, and re-export. Or edit directly in Unreal's Data Table editor.

**Q: What happens if I import the same file twice?**

A: It will update existing entries with matching IDs. Useful for iterating on designs.

**Q: Can I use these tools for other games?**

A: The structure is specific to this game, but you can adapt the JSON format and Python scripts for other projects.

### Content Creation

**Q: How many parts should I create?**

A: Recommended:
- Minimum: 3-5 per category per rarity (Common, Uncommon, Rare)
- Good: 5-10 per category across all rarities
- Excellent: 10+ with upgrade chains and variants

**Q: Should every part have an upgrade path?**

A: Not necessarily. Some parts can be sidegrades (different, not strictly better). Create diverse options.

**Q: How do I create a "unique" item?**

A: Set rarity to Legendary, give it distinctive stats (not just higher numbers), and make it feel special through visuals and lore.

**Q: Can parts affect multiple stats?**

A: Yes! Most parts should. For example, a heavy armor hull might increase HullIntegrity but also increase Mass.

### Technical Questions

**Q: What format should asset paths use?**

A: Unreal Engine content path format:
```
/Game/ShipCustomization/Meshes/SM_Ship_Engine_Basic_01
```

**Q: Can I use spaces in Part IDs?**

A: No. Use underscores and lowercase only:
```
Good: engine_basic_01
Bad: Engine Basic 01
```

**Q: How do I create custom stat combinations?**

A: Use the content creator and select "Customize stats" when prompted. You can set each stat individually.

**Q: Can I import from Excel/CSV?**

A: Convert to JSON first using a converter, or modify the batch importer to support CSV.

### Workflow Questions

**Q: What order should I create content?**

A: Recommended order:
1. Common parts (baseline)
2. Uncommon parts (upgrades)
3. Rare+ parts (special variants)
4. Skins (after parts are working)
5. Physics configs (for ship archetypes)

**Q: How do I test balance without implementing in-game?**

A: Create a spreadsheet with stat comparisons, calculate effective values, and compare to design goals.

**Q: Can multiple designers work on content simultaneously?**

A: Yes! Each designer can create their own JSON files, then merge them before importing to Unreal.

**Q: How do I version control content?**

A: Commit JSON files to Git. They're text-based and diff-friendly. Unreal Data Tables should also be in version control.

---

## Additional Resources

### Files Included

- `ship_content_creator.py` - Interactive content creation tool
- `batch_import_content.py` - Batch import to Unreal
- `organize_content.py` - Content organization
- `ShipContentEditorUtility.h/.cpp` - C++ editor utility
- `CONTENT_CREATION_WORKFLOW.md` - This guide
- `ASSET_NAMING_CONVENTIONS.md` - Auto-generated naming guide

### Example Content

Example JSON files are included in `./examples/`:
- `example_ship_parts.json` - Sample parts
- `example_ship_skins.json` - Sample skins
- `example_physics_config.json` - Sample configs

### Quick Reference

**Common Commands**:
```bash
# Create content
python ship_content_creator.py

# Import to Unreal
python batch_import_content.py ./content_output

# Organize assets
python organize_content.py

# Validate content
python ship_content_creator.py
# Select option 5

# Generate naming conventions doc
python organize_content.py --generate-doc
```

**Stat Multipliers**:
- Common: 1.0x
- Uncommon: 1.25x
- Rare: 1.5x
- Epic: 1.75x
- Legendary: 2.0x

**Unlock Cost Formula**:
```
Cost = BaseRarityCost × LevelRequirement

Base Costs:
- Common: 0
- Uncommon: 500
- Rare: 2000
- Epic: 5000
- Legendary: 10000
```

---

## Appendix: JSON Format Reference

### Ship Part Format

```json
{
  "parts": [
    {
      "PartID": "engine_basic_01",
      "DisplayName": "Basic Engine",
      "Description": "A standard propulsion system",
      "Category": "Engine",
      "Rarity": "Common",
      "LevelRequirement": 1,
      "UnlockCost": 0,
      "UnlockedByDefault": true,
      "StatModifiers": {
        "ThrustPower": 1.0,
        "MaxVelocity": 1.0,
        "RotationSpeed": 0.0,
        "Acceleration": 1.0,
        "HullIntegrity": 0.0,
        "ShieldStrength": 0.0,
        "WeaponDamage": 0.0,
        "EnergyCapacity": 0.0,
        "EnergyRegenRate": 0.0,
        "Mass": 1000.0
      },
      "PartMeshPath": "/Game/ShipCustomization/Meshes/Engines/SM_Ship_Engine_Basic_01",
      "IconPath": "/Game/ShipCustomization/Icons/T_Icon_Engine_Basic01",
      "ParentPartID": "",
      "CanBeUpgraded": true,
      "UpgradeLevel": 0
    }
  ]
}
```

### Ship Skin Format

```json
{
  "skins": [
    {
      "SkinID": "skin_default",
      "DisplayName": "Default",
      "Description": "Standard ship appearance",
      "Rarity": "Common",
      "SkinMaterialPath": "/Game/ShipCustomization/Materials/Instances/MI_Ship_Default",
      "IconPath": "/Game/ShipCustomization/Icons/T_Icon_Skin_Default",
      "UnlockCost": 0,
      "UnlockedByDefault": true
    }
  ]
}
```

### Physics Config Format

```json
{
  "ConfigName": "Balanced",
  "ThrustForce": 50000.0,
  "RotationTorque": 20000.0,
  "MaxSpeed": 1000.0,
  "Mass": 10000.0,
  "Drag": 0.1,
  "AngularDrag": 0.05
}
```

---

## Change Log

**v1.0** - Initial release
- Interactive content creator
- Batch import tool
- Content organizer
- C++ editor utility
- Complete documentation

---

## Support

For issues, questions, or suggestions:
1. Check this documentation first
2. Review FAQ section
3. Check troubleshooting guide
4. Contact technical lead

---

**Happy Content Creating!** 🚀
