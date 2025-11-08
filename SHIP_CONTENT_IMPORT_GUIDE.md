# Ship Content Import Guide

Complete instructions for importing ship parts and skins into Unreal Engine.

---

## Table of Contents
1. [Quick Start](#quick-start)
2. [Detailed Import Steps](#detailed-import-steps)
3. [Verification](#verification)
4. [Integration](#integration)
5. [Troubleshooting](#troubleshooting)

---

## Quick Start

**Files to Import**:
- `Content/Data/ShipParts.csv` (32 ship parts)
- `Content/Data/ShipSkins.csv` (10 ship skins)

**Expected Result**: Two Data Tables ready to use in your ship customization system.

---

## Detailed Import Steps

### Part 1: Create Ship Parts Data Table

1. **Open Unreal Editor** for the Alexander project

2. **Navigate to Content Browser**
   - Go to `Content/Data/` folder
   - If the folder doesn't exist, create it: Right-click → New Folder → "Data"

3. **Create Data Table**
   - Right-click in `Content/Data/`
   - Select: **Miscellaneous → Data Table**
   - In the "Pick Structure" dialog:
     - Search for: `ShipPartData`
     - Select: **FShipPartData** (should appear as struct from ShipCustomizationComponent)
     - Click "Select"

4. **Name the Data Table**
   - Name: `DT_ShipParts`
   - Press Enter to confirm

5. **Import CSV Data**
   - Double-click `DT_ShipParts` to open it
   - In the toolbar, click **"Import"** button (or File → Import)
   - Browse to: `Content/Data/ShipParts.csv`
   - Click "Open"
   - Unreal will parse the CSV and populate the data table

6. **Verify Import**
   - You should see **32 rows** in the data table
   - Check that categories show correctly:
     - Engine (5 parts)
     - Thrusters (4 parts)
     - Hull (4 parts)
     - Shield (4 parts)
     - Weapon (5 parts)
     - Utility (4 parts)
     - Wings (3 parts)
     - Cockpit (3 parts)
   - Spot-check a few rows for correct data (e.g., "Phantom Drive X-7" should be Legendary)

7. **Save Data Table**
   - File → Save Selected
   - Or Ctrl+S

---

### Part 2: Create Ship Skins Data Table

1. **Create Data Table** (same location: `Content/Data/`)
   - Right-click in `Content/Data/`
   - Select: **Miscellaneous → Data Table**
   - In the "Pick Structure" dialog:
     - Search for: `ShipSkinData`
     - Select: **FShipSkinData**
     - Click "Select"

2. **Name the Data Table**
   - Name: `DT_ShipSkins`
   - Press Enter to confirm

3. **Import CSV Data**
   - Double-click `DT_ShipSkins` to open it
   - Click **"Import"** button
   - Browse to: `Content/Data/ShipSkins.csv`
   - Click "Open"

4. **Verify Import**
   - You should see **10 rows** in the data table
   - Check that all skins are present:
     - Factory Default (Common, free)
     - Military Gray (Common)
     - Racing Red (Uncommon)
     - Stealth Black (Rare)
     - Corporate Blue (Rare)
     - Pirate Gold (Epic)
     - Explorer Green (Epic)
     - Royal Purple (Legendary)
     - Chrome Mirror (Legendary)
     - Neon Circuit (Legendary)

5. **Save Data Table**
   - File → Save Selected
   - Or Ctrl+S

---

### Part 3: Link Data Tables to Ship Customization Component

#### Option A: Link in Blueprint Class Default

1. **Open Ship Blueprint**
   - Find your ship Blueprint (e.g., `BP_PlayerShip`, `BP_FlightPawn`, etc.)
   - Double-click to open Blueprint Editor

2. **Find Ship Customization Component**
   - In the Components panel (left side), look for:
     - `ShipCustomizationComponent` or
     - `ShipCustomization`
   - Click on it to select

3. **Set Data Table References**
   - In the Details panel (right side), find the "Data" category
   - Set the following properties:
     - **Ship Parts Data Table**: `DT_ShipParts`
     - **Ship Skins Data Table**: `DT_ShipSkins`
   - To set each:
     - Click the dropdown next to the property
     - Search for the data table name
     - Select it from the list

4. **Compile and Save**
   - Click "Compile" button (top toolbar)
   - Click "Save" button
   - Close Blueprint Editor

#### Option B: Link via Game Mode or Level Blueprint

If your ship is spawned dynamically, you may need to set these references at runtime:

1. **Open Level Blueprint or Game Mode**
   - Level Blueprint: Blueprints → Open Level Blueprint
   - Game Mode: Content Browser → Find your Game Mode BP

2. **Get Ship Reference**
   - Get a reference to your spawned ship actor
   - Example: `Get Player Pawn` → `Cast to BP_PlayerShip`

3. **Get Customization Component**
   - From ship reference: `Get Component By Class`
   - Class: `ShipCustomizationComponent`

4. **Set Data Tables**
   - From component reference, drag out and search:
     - `Set Ship Parts Data Table`
     - `Set Ship Skins Data Table`
   - Connect the appropriate data table assets

5. **Compile and Save**

---

## Verification

### Test 1: Data Table Integrity

1. Open `DT_ShipParts`
2. Click on random rows and verify:
   - **Display Name** is readable (e.g., "Phantom Drive X-7")
   - **Category** shows correct enum (e.g., "Engine")
   - **Rarity** shows correct enum (e.g., "Legendary")
   - **Stat Modifiers** have non-zero values (except for "None" parts)
   - **UnlockCost** matches rarity (higher rarity = higher cost)

3. Open `DT_ShipSkins`
4. Verify:
   - All skin IDs are unique
   - Default skin exists (Factory Default)
   - Material paths are set (even if placeholder)

### Test 2: Runtime Access

1. **Create Test Blueprint** (optional, for debugging)
   - Create new Blueprint: Actor or GameMode
   - In Event Graph:
     - `Event BeginPlay`
     - Get Data Table Row: `DT_ShipParts`, Row Name: `Engine_Legendary_Phantom`
     - `Print String`: Show the part's Display Name

2. **Run in Editor**
   - Press Play (PIE)
   - You should see "Phantom Drive X-7" printed on screen
   - This confirms data tables are accessible at runtime

### Test 3: Component Integration

1. **Place Ship in Level**
   - Drag your ship Blueprint into the level
   - Select it

2. **Check Component Properties**
   - In Details panel, find `ShipCustomizationComponent`
   - Expand the "Data" category
   - Verify both data tables are set

3. **Play and Debug**
   - Start PIE
   - Open Blueprints debugger
   - Check that `ProgressionData` initializes
   - Check that `UnlockedParts` contains default parts

---

## Integration

### Accessing Parts in Blueprints

**Get All Parts in Category**:
```
ShipCustomizationComponent → Get Parts In Category
  Category: Engine
  Return: Array of FShipPartData
```

**Get Equipped Part**:
```
ShipCustomizationComponent → Get Equipped Part
  Category: Engine
  Return: FShipPartData (with "Found" bool)
```

**Equip a Part**:
```
ShipCustomizationComponent → Equip Part
  Category: Engine
  Part ID: Engine_Legendary_Phantom
  Return: bool (success/fail)
```

**Unlock a Part**:
```
ShipCustomizationComponent → Unlock Part
  Part ID: Engine_Speed_Racer
  Return: bool (success/fail - checks credits & level)
```

### Accessing Parts in C++

```cpp
// Get ship customization component
UShipCustomizationComponent* CustomComp = ShipActor->FindComponentByClass<UShipCustomizationComponent>();

// Get all parts in category
TArray<FShipPartData> EngineParts = CustomComp->GetPartsInCategory(EShipPartCategory::Engine);

// Equip a part
bool bSuccess = CustomComp->EquipPart(EShipPartCategory::Engine, FName("Engine_Legendary_Phantom"));

// Get total stats
FShipStats TotalStats = CustomComp->GetTotalStats();
```

### Recommended Integration Points

1. **Main Menu → Customization Screen**
   - Display all unlocked parts per category
   - Show player's credits and level
   - Allow equipping/unequipping parts
   - Preview stats changes

2. **Gameplay → Post-Race Rewards**
   - Grant credits based on performance
   - Grant XP for completion
   - Check for level-ups
   - Notify about newly unlockable parts

3. **Save/Load System**
   - Call `SaveCustomizationData()` when player exits customization screen
   - Call `LoadCustomizationData()` on game start
   - Integrate with your existing save game system

---

## Troubleshooting

### Issue: Data Table shows "Unknown Struct"

**Cause**: Unreal can't find `FShipPartData` or `FShipSkinData` structs

**Solution**:
1. Verify `ShipCustomizationComponent.h` is compiled
2. Close Unreal Editor
3. Rebuild project (Visual Studio → Build Solution)
4. Reopen Unreal Editor
5. Re-create data tables

---

### Issue: CSV import fails or shows empty rows

**Cause**: CSV format mismatch with struct definition

**Solution**:
1. Check CSV column names match struct property names exactly
2. Verify category/rarity values are numeric (0, 1, 2, etc.)
3. Ensure `bUnlockedByDefault` is lowercase "true" or "false"
4. Check for extra commas or line breaks in CSV

**Quick Fix**:
- Re-run `python import_ship_content.py` to regenerate CSV files

---

### Issue: Parts don't show up in Blueprint dropdown

**Cause**: Data table not set on component

**Solution**:
1. Select ship Blueprint
2. Find ShipCustomizationComponent
3. Set both data tables in Details panel
4. Compile and save Blueprint

---

### Issue: Stats don't apply to ship

**Cause**: `ApplyStatsToFlightController()` not called

**Solution**:
1. After equipping parts, call: `ShipCustomizationComponent → Apply Stats To Flight Controller`
2. This should happen automatically in `BeginPlay()`, but verify in your Blueprint

**Debug**:
- Print `GetTotalStats()` to verify stats are calculated
- Check if FlightController reference is valid
- Ensure FlightController component exists on ship

---

### Issue: Unlocking parts costs no credits

**Cause**: Unlock logic not fully implemented

**Solution**:
1. Verify `ProgressionData.Credits` is set correctly
2. Call `AddCredits(Amount)` when player earns credits
3. Check `UnlockPart()` logic in C++ implementation
4. Ensure credits are deducted: `ProgressionData.Credits -= UnlockCost`

---

### Issue: Parts appear locked even after level-up

**Cause**: Level requirement check failing

**Solution**:
1. Verify `ProgressionData.PlayerLevel` is updated
2. Call `AddXP(Amount)` and `CheckLevelUp()` after gaining XP
3. Check level requirement thresholds in CSV data

---

## Next Steps After Import

### Phase 2A: UI Implementation
- [ ] Create customization menu widget
- [ ] Display available parts per category
- [ ] Show part details (stats, cost, requirements)
- [ ] Implement equip/unequip functionality
- [ ] Show total stats preview

### Phase 2B: Progression Integration
- [ ] Award credits on course completion
- [ ] Award XP based on performance
- [ ] Implement level-up system
- [ ] Create unlock notifications
- [ ] Add part unlock celebration effects

### Phase 2C: Visual Implementation
- [ ] Create placeholder meshes for parts (or use existing ship mesh)
- [ ] Create material instances for skins
- [ ] Implement skin application to ship mesh
- [ ] Add visual effects for part equipping

### Phase 2D: Balance Testing
- [ ] Playtest all 32 parts
- [ ] Compare lap times across builds
- [ ] Adjust costs based on progression curve
- [ ] Fine-tune stat values for balance

---

## Content Summary

**Total Ship Parts**: 32

| Category | Count | Rarities |
|----------|-------|----------|
| Engine | 5 | Common, Uncommon, Rare, Epic, Legendary |
| Thrusters | 4 | Common, Uncommon, Rare, Epic |
| Hull | 4 | Common, Uncommon, Rare, Epic |
| Shield | 4 | Common, Uncommon, Rare, Epic |
| Weapon | 5 | Common, Uncommon, Rare, Epic, Legendary |
| Utility | 4 | Common, Uncommon, Rare, Epic |
| Wings | 3 | Common, Uncommon, Rare |
| Cockpit | 3 | Common, Uncommon, Rare |

**Total Ship Skins**: 10

| Rarity | Count | Examples |
|--------|-------|----------|
| Common | 2 | Factory Default, Military Gray |
| Uncommon | 1 | Racing Red |
| Rare | 2 | Stealth Black, Corporate Blue |
| Epic | 2 | Pirate Gold, Explorer Green |
| Legendary | 3 | Royal Purple, Chrome Mirror, Neon Circuit |

**Total Content Value**: 276,500 credits (if player unlocks everything)

---

## Support

If you encounter issues not covered in this guide:

1. **Re-run validation**: `python validate_ship_content.py`
2. **Check console logs**: Look for errors related to data tables or structs
3. **Verify struct definitions**: Ensure ShipCustomizationComponent is compiled
4. **Regenerate CSV files**: Run `python import_ship_content.py` again

---

**Last Updated**: Phase 2 Content Generation
**Author**: Ship Customization System
**Version**: 1.0
