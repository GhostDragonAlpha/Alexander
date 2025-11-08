"""
Ship Content Validation Script
Validates ship parts and skins data for balance, uniqueness, and consistency
"""

import json
from pathlib import Path
from collections import defaultdict
import sys

class ContentValidator:
    def __init__(self):
        self.errors = []
        self.warnings = []
        self.info = []

    def error(self, message):
        """Add error message"""
        self.errors.append(f"[ERROR] {message}")

    def warning(self, message):
        """Add warning message"""
        self.warnings.append(f"[WARNING] {message}")

    def info_msg(self, message):
        """Add info message"""
        self.info.append(f"[INFO] {message}")

    def print_results(self):
        """Print all validation results"""
        print()
        print("=" * 70)
        print("VALIDATION RESULTS")
        print("=" * 70)
        print()

        if self.info:
            print("Information:")
            for msg in self.info:
                print(f"  {msg}")
            print()

        if self.warnings:
            print("Warnings:")
            for msg in self.warnings:
                print(f"  {msg}")
            print()

        if self.errors:
            print("Errors:")
            for msg in self.errors:
                print(f"  {msg}")
            print()
        else:
            print("[OK] No errors found!")
            print()

        print("=" * 70)
        print(f"Summary: {len(self.errors)} errors, {len(self.warnings)} warnings")
        print("=" * 70)

        return len(self.errors) == 0

def validate_ship_parts(json_path):
    """Validate ship parts data"""

    validator = ContentValidator()

    print(f"Loading ship parts from: {json_path}")

    try:
        with open(json_path, 'r') as f:
            data = json.load(f)
    except Exception as e:
        validator.error(f"Failed to load JSON: {e}")
        return validator

    parts = data.get('Parts', [])
    validator.info_msg(f"Found {len(parts)} ship parts")

    # Track IDs and categories
    part_ids = set()
    category_counts = defaultdict(int)
    rarity_counts = defaultdict(int)

    # Rarity cost ranges (expected)
    rarity_cost_ranges = {
        'Common': (0, 0),
        'Uncommon': (1000, 5000),
        'Rare': (5000, 10000),
        'Epic': (10000, 20000),
        'Legendary': (20000, 30000)
    }

    # Category groups for balance checking
    category_parts = defaultdict(list)

    for i, part in enumerate(parts):
        part_id = part.get('PartID', f'MISSING_ID_{i}')

        # Check unique ID
        if part_id in part_ids:
            validator.error(f"Duplicate PartID: {part_id}")
        else:
            part_ids.add(part_id)

        # Check required fields
        required_fields = ['PartID', 'DisplayName', 'Description', 'Category', 'Rarity']
        for field in required_fields:
            if field not in part:
                validator.error(f"{part_id}: Missing required field '{field}'")

        # Track categories and rarities
        category = part.get('Category', 'Unknown')
        rarity = part.get('Rarity', 'Unknown')
        category_counts[category] += 1
        rarity_counts[rarity] += 1

        # Group by category for balance checking
        category_parts[category].append(part)

        # Validate cost matches rarity
        cost = part.get('UnlockCost', 0)
        if rarity in rarity_cost_ranges:
            min_cost, max_cost = rarity_cost_ranges[rarity]
            if not (min_cost <= cost <= max_cost):
                validator.warning(
                    f"{part_id}: Cost {cost} outside expected range "
                    f"[{min_cost}-{max_cost}] for rarity '{rarity}'"
                )

        # Validate level requirements increase with rarity
        level_req = part.get('LevelRequirement', 1)
        expected_min_level = {'Common': 1, 'Uncommon': 3, 'Rare': 5, 'Epic': 8, 'Legendary': 12}
        if rarity in expected_min_level:
            if level_req < expected_min_level[rarity]:
                validator.warning(
                    f"{part_id}: Level requirement {level_req} is low for "
                    f"rarity '{rarity}' (expected >={expected_min_level[rarity]})"
                )

        # Check stat modifiers exist
        if 'StatModifiers' not in part:
            validator.error(f"{part_id}: Missing StatModifiers")
        else:
            stats = part['StatModifiers']
            # Check for empty stats (all zeros)
            if all(v == 0.0 for v in stats.values()):
                validator.warning(f"{part_id}: All stat modifiers are zero")

    # Report category distribution
    validator.info_msg("Parts per category:")
    for category, count in sorted(category_counts.items()):
        validator.info_msg(f"  {category}: {count} parts")

    # Check minimum parts per category
    for category, count in category_counts.items():
        if count < 3:
            validator.warning(f"Category '{category}' has only {count} parts (recommended: 3+)")

    # Report rarity distribution
    validator.info_msg("Parts per rarity:")
    for rarity, count in sorted(rarity_counts.items()):
        validator.info_msg(f"  {rarity}: {count} parts")

    # Validate balance within categories
    validator.info_msg("Checking balance within categories...")
    for category, parts_list in category_parts.items():
        if len(parts_list) < 2:
            continue  # Skip balance check for single-part categories

        # Check if any part is strictly better than all others
        for i, part_a in enumerate(parts_list):
            stats_a = part_a.get('StatModifiers', {})

            for j, part_b in enumerate(parts_list):
                if i == j:
                    continue

                stats_b = part_b.get('StatModifiers', {})

                # Compare all relevant stats
                better_count = 0
                worse_count = 0
                equal_count = 0

                for stat_name in ['ThrustPower', 'MaxVelocity', 'RotationSpeed', 'Acceleration']:
                    val_a = stats_a.get(stat_name, 0.0)
                    val_b = stats_b.get(stat_name, 0.0)

                    if val_a > val_b:
                        better_count += 1
                    elif val_a < val_b:
                        worse_count += 1
                    else:
                        equal_count += 1

                # If strictly better in all stats, flag as balance issue
                if better_count > 0 and worse_count == 0 and equal_count < 3:
                    validator.warning(
                        f"Balance: {part_a['PartID']} may be strictly better than "
                        f"{part_b['PartID']} in category '{category}'"
                    )

    return validator

def validate_ship_skins(json_path):
    """Validate ship skins data"""

    validator = ContentValidator()

    print(f"Loading ship skins from: {json_path}")

    try:
        with open(json_path, 'r') as f:
            data = json.load(f)
    except Exception as e:
        validator.error(f"Failed to load JSON: {e}")
        return validator

    skins = data.get('Skins', [])
    validator.info_msg(f"Found {len(skins)} ship skins")

    # Track IDs
    skin_ids = set()
    rarity_counts = defaultdict(int)

    # Check for at least one unlocked-by-default skin
    has_default = False

    for i, skin in enumerate(skins):
        skin_id = skin.get('SkinID', f'MISSING_ID_{i}')

        # Check unique ID
        if skin_id in skin_ids:
            validator.error(f"Duplicate SkinID: {skin_id}")
        else:
            skin_ids.add(skin_id)

        # Check required fields
        required_fields = ['SkinID', 'DisplayName', 'Description', 'Rarity']
        for field in required_fields:
            if field not in skin:
                validator.error(f"{skin_id}: Missing required field '{field}'")

        # Track rarity
        rarity = skin.get('Rarity', 'Unknown')
        rarity_counts[rarity] += 1

        # Check for default skin
        if skin.get('bUnlockedByDefault', False):
            has_default = True

        # Validate cost increases with rarity
        cost = skin.get('UnlockCost', 0)
        level = skin.get('LevelRequirement', 1)

    if not has_default:
        validator.error("No default skin (bUnlockedByDefault=true) found")

    # Report rarity distribution
    validator.info_msg("Skins per rarity:")
    for rarity, count in sorted(rarity_counts.items()):
        validator.info_msg(f"  {rarity}: {count} skins")

    # Check for reasonable skin count
    if len(skins) < 5:
        validator.warning(f"Only {len(skins)} skins available (recommended: 8+)")

    return validator

def main():
    """Main validation process"""

    print("=" * 70)
    print("Ship Content Validation")
    print("=" * 70)
    print()

    project_dir = Path(__file__).parent
    content_data_dir = project_dir / 'Content' / 'Data'

    # Validate ship parts
    parts_json = content_data_dir / 'ShipParts.json'
    if parts_json.exists():
        print(f"Validating Ship Parts...")
        print("-" * 70)
        parts_validator = validate_ship_parts(parts_json)
        parts_valid = parts_validator.print_results()
    else:
        print(f"ERROR: ShipParts.json not found at {parts_json}")
        parts_valid = False

    print()

    # Validate ship skins
    skins_json = content_data_dir / 'ShipSkins.json'
    if skins_json.exists():
        print(f"Validating Ship Skins...")
        print("-" * 70)
        skins_validator = validate_ship_skins(skins_json)
        skins_valid = skins_validator.print_results()
    else:
        print(f"ERROR: ShipSkins.json not found at {skins_json}")
        skins_valid = False

    # Overall result
    print()
    if parts_valid and skins_valid:
        print("[OK] All validation checks passed!")
        return 0
    else:
        print("[FAIL] Validation failed - please fix errors above")
        return 1

if __name__ == '__main__':
    sys.exit(main())
