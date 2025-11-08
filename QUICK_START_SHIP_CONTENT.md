# Quick Start: Ship Parts Content System

**Status**: Ready for Import
**Total Content**: 32 Parts + 10 Skins

---

## 1. Run Import Script (30 seconds)

```bash
python import_ship_content.py
```

**Output**: Creates CSV files ready for Unreal

---

## 2. Import into Unreal (5 minutes)

### Create Ship Parts Table
1. Content Browser → `Content/Data/`
2. Right-click → Miscellaneous → Data Table
3. Row Structure: `FShipPartData`
4. Name: `DT_ShipParts`
5. Open → Import → Select `Content/Data/ShipParts.csv`
6. Verify: **32 rows** imported

### Create Ship Skins Table
1. Right-click → Miscellaneous → Data Table
2. Row Structure: `FShipSkinData`
3. Name: `DT_ShipSkins`
4. Open → Import → Select `Content/Data/ShipSkins.csv`
5. Verify: **10 rows** imported

---

## 3. Link to Ship Blueprint (2 minutes)

1. Open your ship Blueprint (e.g., `BP_PlayerShip`)
2. Select `ShipCustomizationComponent`
3. Details Panel → Data section:
   - **Ship Parts Data Table**: `DT_ShipParts`
   - **Ship Skins Data Table**: `DT_ShipSkins`
4. Compile & Save

---

## 4. Test in PIE (1 minute)

**Blueprint Test**:
```
Event BeginPlay
  → Get Component By Class (ShipCustomizationComponent)
  → Get Parts In Category (Engine)
  → Print String (Array Length)
```

**Expected Output**: "5" (5 engine parts found)

---

## Content Summary

### Parts by Category (32 total)
- Engine: 5 parts
- Thrusters: 4 parts
- Hull: 4 parts
- Shield: 4 parts
- Weapon: 5 parts
- Utility: 4 parts
- Wings: 3 parts
- Cockpit: 3 parts

### Skins by Rarity (10 total)
- Common: 2 (Factory Default, Military Gray)
- Uncommon: 1 (Racing Red)
- Rare: 2 (Stealth Black, Corporate Blue)
- Epic: 2 (Pirate Gold, Explorer Green)
- Legendary: 3 (Royal Purple, Chrome Mirror, Neon Circuit)

---

## Sample Builds

### Speed Demon (Racing)
- Engine: Phantom Drive X-7 (1.6× velocity)
- Thrusters: Eclipse Racing (1.8× rotation)
- Hull: Carbon Nanoweave (120 mass)
- Shield: None
- **Result**: Fastest possible, fragile

### Fortress (Combat)
- Engine: Titan Mk2 (1.6× thrust)
- Hull: Dreadnought (300 HP)
- Shield: Fortress Array (250 shield HP)
- Weapon: Railgun Mk5 (2.0× damage)
- **Result**: 550 total HP, slow

### Balanced Pro
- Engine: Quantum Balance (1.25× all stats)
- Thrusters: Mongoose Agility (1.5× rotation)
- Hull: Reinforced Titanium (180 HP)
- Shield: Mk1 Projector (100 shield HP)
- **Result**: Competitive, affordable (28,500 credits)

---

## Validation

**Run Validator**:
```bash
python validate_ship_content.py
```

**Expected Result**:
- 0 errors
- 13 warnings (acceptable, intentional balance choices)

---

## Documentation

| File | Purpose |
|------|---------|
| `SHIP_PARTS_BALANCING.md` | Balancing philosophy, formulas (12 pages) |
| `SHIP_CONTENT_IMPORT_GUIDE.md` | Step-by-step import instructions (10 pages) |
| `SHIP_PARTS_STAT_COMPARISON.md` | Detailed stat analysis, builds (15 pages) |
| `PHASE2_CONTENT_GENERATION_SUMMARY.md` | Complete phase summary (8 pages) |

---

## Troubleshooting

**Issue**: Data table shows "Unknown Struct"
- **Fix**: Rebuild project in Visual Studio, reopen Unreal

**Issue**: CSV import fails
- **Fix**: Re-run `python import_ship_content.py`

**Issue**: Parts don't affect ship stats
- **Fix**: Call `Apply Stats To Flight Controller` after equipping

---

## Next Steps

### Phase 3: UI Implementation
- [ ] Create customization menu widget
- [ ] Display parts per category
- [ ] Show stat preview
- [ ] Implement equip/unequip buttons

### Phase 4: Gameplay Integration
- [ ] Award credits on course completion
- [ ] Award XP based on performance
- [ ] Implement level-up system
- [ ] Create unlock notifications

---

**Total Time to Import**: ~8 minutes
**Ready for**: Phase 3 UI Development

---

**Created**: 2025-11-07
**Version**: 1.0 - Quick Reference
