"""
Ship Content Importer
Converts ship parts and skins JSON data into Unreal Engine Data Table CSV format
"""

import json
import csv
import os
from pathlib import Path

# Category enum mapping (matches EShipPartCategory in C++)
CATEGORY_MAP = {
    "Engine": 0,
    "Thrusters": 1,
    "Hull": 2,
    "Wings": 3,
    "Cockpit": 4,
    "Weapon": 5,
    "Shield": 6,
    "Utility": 7
}

# Rarity enum mapping (matches EShipPartRarity in C++)
RARITY_MAP = {
    "Common": 0,
    "Uncommon": 1,
    "Rare": 2,
    "Epic": 3,
    "Legendary": 4
}

def convert_ship_parts_to_csv(json_path, csv_path):
    """Convert ShipParts.json to Unreal DataTable CSV format"""

    print(f"Reading ship parts from: {json_path}")
    with open(json_path, 'r') as f:
        data = json.load(f)

    parts = data.get('Parts', [])
    print(f"Found {len(parts)} ship parts")

    # Define CSV columns matching FShipPartData structure
    fieldnames = [
        'Name',  # Row name (PartID)
        'PartID',
        'DisplayName',
        'Description',
        'Category',
        'Rarity',
        'LevelRequirement',
        'UnlockCost',
        'bUnlockedByDefault',
        'StatModifiers.ThrustPower',
        'StatModifiers.MaxVelocity',
        'StatModifiers.RotationSpeed',
        'StatModifiers.Acceleration',
        'StatModifiers.HullIntegrity',
        'StatModifiers.ShieldStrength',
        'StatModifiers.WeaponDamage',
        'StatModifiers.EnergyCapacity',
        'StatModifiers.EnergyRegenRate',
        'StatModifiers.Mass',
        'PartMesh',
        'Icon',
        'ParentPartID',
        'bCanBeUpgraded',
        'UpgradeLevel'
    ]

    print(f"Writing CSV to: {csv_path}")
    with open(csv_path, 'w', newline='', encoding='utf-8') as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()

        for part in parts:
            stats = part.get('StatModifiers', {})

            row = {
                'Name': part['PartID'],
                'PartID': part['PartID'],
                'DisplayName': part['DisplayName'],
                'Description': part['Description'],
                'Category': CATEGORY_MAP.get(part['Category'], 0),
                'Rarity': RARITY_MAP.get(part['Rarity'], 0),
                'LevelRequirement': part.get('LevelRequirement', 1),
                'UnlockCost': part.get('UnlockCost', 0),
                'bUnlockedByDefault': str(part.get('bUnlockedByDefault', False)).lower(),
                'StatModifiers.ThrustPower': stats.get('ThrustPower', 0.0),
                'StatModifiers.MaxVelocity': stats.get('MaxVelocity', 0.0),
                'StatModifiers.RotationSpeed': stats.get('RotationSpeed', 0.0),
                'StatModifiers.Acceleration': stats.get('Acceleration', 0.0),
                'StatModifiers.HullIntegrity': stats.get('HullIntegrity', 0.0),
                'StatModifiers.ShieldStrength': stats.get('ShieldStrength', 0.0),
                'StatModifiers.WeaponDamage': stats.get('WeaponDamage', 0.0),
                'StatModifiers.EnergyCapacity': stats.get('EnergyCapacity', 0.0),
                'StatModifiers.EnergyRegenRate': stats.get('EnergyRegenRate', 0.0),
                'StatModifiers.Mass': stats.get('Mass', 0.0),
                'PartMesh': '',  # Placeholder - assets not created yet
                'Icon': '',  # Placeholder
                'ParentPartID': '',  # No parent parts in this version
                'bCanBeUpgraded': 'true',
                'UpgradeLevel': 0
            }

            writer.writerow(row)

    print(f"[OK] Successfully converted {len(parts)} ship parts to CSV")
    return len(parts)

def convert_ship_skins_to_csv(json_path, csv_path):
    """Convert ShipSkins.json to Unreal DataTable CSV format"""

    print(f"Reading ship skins from: {json_path}")
    with open(json_path, 'r') as f:
        data = json.load(f)

    skins = data.get('Skins', [])
    print(f"Found {len(skins)} ship skins")

    # Define CSV columns matching FShipSkinData structure
    fieldnames = [
        'Name',  # Row name (SkinID)
        'SkinID',
        'DisplayName',
        'Description',
        'Rarity',
        'SkinMaterial',
        'Icon',
        'UnlockCost',
        'bUnlockedByDefault'
    ]

    print(f"Writing CSV to: {csv_path}")
    with open(csv_path, 'w', newline='', encoding='utf-8') as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()

        for skin in skins:
            row = {
                'Name': skin['SkinID'],
                'SkinID': skin['SkinID'],
                'DisplayName': skin['DisplayName'],
                'Description': skin['Description'],
                'Rarity': RARITY_MAP.get(skin['Rarity'], 0),
                'SkinMaterial': skin.get('MaterialPath', ''),
                'Icon': '',  # Placeholder
                'UnlockCost': skin.get('UnlockCost', 0),
                'bUnlockedByDefault': str(skin.get('bUnlockedByDefault', False)).lower()
            }

            writer.writerow(row)

    print(f"[OK] Successfully converted {len(skins)} ship skins to CSV")
    return len(skins)

def generate_import_script():
    """Generate batch script for importing into Unreal"""

    script_content = """@echo off
REM Ship Content Import Script
REM This script documents the manual import process for Unreal Engine

echo ========================================
echo Ship Content Import Instructions
echo ========================================
echo.
echo 1. Open Unreal Editor
echo.
echo 2. Create Ship Parts Data Table:
echo    - Right-click in Content Browser
echo    - Miscellaneous ^> Data Table
echo    - Select row structure: ShipPartData
echo    - Name: DT_ShipParts
echo    - Save to: Content/Data/
echo.
echo 3. Import Ship Parts CSV:
echo    - Open DT_ShipParts
echo    - Click "Import" button
echo    - Select: Content/Data/ShipParts.csv
echo    - Verify all rows imported correctly
echo.
echo 4. Create Ship Skins Data Table:
echo    - Right-click in Content Browser
echo    - Miscellaneous ^> Data Table
echo    - Select row structure: ShipSkinData
echo    - Name: DT_ShipSkins
echo    - Save to: Content/Data/
echo.
echo 5. Import Ship Skins CSV:
echo    - Open DT_ShipSkins
echo    - Click "Import" button
echo    - Select: Content/Data/ShipSkins.csv
echo    - Verify all rows imported correctly
echo.
echo 6. Link Data Tables to ShipCustomizationComponent:
echo    - Open your ship Blueprint
echo    - Find ShipCustomizationComponent
echo    - Set ShipPartsDataTable to DT_ShipParts
echo    - Set ShipSkinsDataTable to DT_ShipSkins
echo.
echo ========================================
echo Import process documented above
echo ========================================
pause
"""

    with open('import_ship_content.bat', 'w') as f:
        f.write(script_content)

    print("[OK] Generated import_ship_content.bat")

def main():
    """Main import process"""

    print("=" * 60)
    print("Ship Content Importer")
    print("=" * 60)
    print()

    # Get project directory
    project_dir = Path(__file__).parent
    content_data_dir = project_dir / 'Content' / 'Data'

    # Ensure output directory exists
    content_data_dir.mkdir(parents=True, exist_ok=True)

    # Convert ship parts
    parts_json = content_data_dir / 'ShipParts.json'
    parts_csv = content_data_dir / 'ShipParts.csv'

    if parts_json.exists():
        parts_count = convert_ship_parts_to_csv(str(parts_json), str(parts_csv))
    else:
        print(f"ERROR: ShipParts.json not found at {parts_json}")
        parts_count = 0

    print()

    # Convert ship skins
    skins_json = content_data_dir / 'ShipSkins.json'
    skins_csv = content_data_dir / 'ShipSkins.csv'

    if skins_json.exists():
        skins_count = convert_ship_skins_to_csv(str(skins_json), str(skins_csv))
    else:
        print(f"ERROR: ShipSkins.json not found at {skins_json}")
        skins_count = 0

    print()

    # Generate import script
    generate_import_script()

    print()
    print("=" * 60)
    print("IMPORT SUMMARY")
    print("=" * 60)
    print(f"Ship Parts: {parts_count} parts across 8 categories")
    print(f"Ship Skins: {skins_count} skins")
    print()
    print("Generated Files:")
    print(f"  - {parts_csv}")
    print(f"  - {skins_csv}")
    print(f"  - import_ship_content.bat")
    print()
    print("Next Steps:")
    print("  1. Run import_ship_content.bat for import instructions")
    print("  2. Follow manual import process in Unreal Editor")
    print("  3. Verify data tables populated correctly")
    print("  4. Link data tables to ShipCustomizationComponent")
    print("=" * 60)

if __name__ == '__main__':
    main()
