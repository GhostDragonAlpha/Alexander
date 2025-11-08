#!/usr/bin/env python3
"""
Interactive Ship Content Creator
Menu-driven tool for designers to create ship customization content
"""

import json
import os
import sys
from typing import Dict, List, Any, Optional
from datetime import datetime

# Color codes for console output
class Colors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'


class ShipContentCreator:
    """Interactive tool for creating ship customization content"""

    # Stat balance recommendations by rarity
    STAT_MULTIPLIERS = {
        "Common": 1.0,
        "Uncommon": 1.25,
        "Rare": 1.5,
        "Epic": 1.75,
        "Legendary": 2.0
    }

    # Base stats by category
    BASE_STATS = {
        "Engine": {
            "ThrustPower": 1.0,
            "Acceleration": 1.0,
            "Mass": 1000.0
        },
        "Thrusters": {
            "RotationSpeed": 1.0,
            "Mass": 500.0
        },
        "Hull": {
            "HullIntegrity": 100.0,
            "Mass": 2000.0
        },
        "Wings": {
            "MaxVelocity": 1.0,
            "RotationSpeed": 0.5,
            "Mass": 800.0
        },
        "Cockpit": {
            "Mass": 600.0
        },
        "Weapon": {
            "WeaponDamage": 1.0,
            "Mass": 600.0
        },
        "Shield": {
            "ShieldStrength": 50.0,
            "EnergyCapacity": 100.0,
            "Mass": 800.0
        },
        "Utility": {
            "Mass": 500.0
        }
    }

    def __init__(self):
        self.parts_library = []
        self.skins_library = []
        self.physics_configs = []
        self.output_dir = "./content_output"

        # Create output directory
        os.makedirs(self.output_dir, exist_ok=True)

    def run(self):
        """Main menu loop"""
        while True:
            self.clear_screen()
            self.print_header("Ship Content Creator - Main Menu")
            print(f"{Colors.OKCYAN}Current Libraries:{Colors.ENDC}")
            print(f"  Parts: {Colors.OKGREEN}{len(self.parts_library)}{Colors.ENDC}")
            print(f"  Skins: {Colors.OKGREEN}{len(self.skins_library)}{Colors.ENDC}")
            print(f"  Physics Configs: {Colors.OKGREEN}{len(self.physics_configs)}{Colors.ENDC}")
            print()

            print(f"{Colors.BOLD}Main Menu:{Colors.ENDC}")
            print("  1. Create new ship part")
            print("  2. Create new skin")
            print("  3. Create physics config")
            print("  4. View all content")
            print("  5. Validate all content")
            print("  6. Export to Unreal format")
            print("  7. Load existing content")
            print("  8. Quick content generator")
            print("  9. Help & Documentation")
            print("  0. Exit")
            print()

            choice = input(f"{Colors.OKCYAN}Select option: {Colors.ENDC}")

            if choice == "1":
                self.create_ship_part()
            elif choice == "2":
                self.create_ship_skin()
            elif choice == "3":
                self.create_physics_config()
            elif choice == "4":
                self.view_all_content()
            elif choice == "5":
                self.validate_all_content()
            elif choice == "6":
                self.export_to_unreal()
            elif choice == "7":
                self.load_existing_content()
            elif choice == "8":
                self.quick_content_generator()
            elif choice == "9":
                self.show_help()
            elif choice == "0":
                print(f"\n{Colors.OKGREEN}Goodbye!{Colors.ENDC}\n")
                break
            else:
                self.print_error("Invalid option")
                self.wait_for_enter()

    def create_ship_part(self):
        """Interactive ship part creation"""
        self.clear_screen()
        self.print_header("Create New Ship Part")

        part = {}

        # Part ID
        part["PartID"] = self.get_input(
            "Part ID (e.g., engine_basic_01): ",
            validator=self.validate_part_id
        )

        # Display Name
        part["DisplayName"] = self.get_input(
            "Display Name (e.g., Basic Engine): ",
            required=True
        )

        # Description
        part["Description"] = self.get_input(
            "Description: ",
            default="A standard ship component"
        )

        # Category
        categories = ["Engine", "Thrusters", "Hull", "Wings", "Cockpit", "Weapon", "Shield", "Utility"]
        part["Category"] = self.select_from_list("Category", categories)

        # Rarity
        rarities = ["Common", "Uncommon", "Rare", "Epic", "Legendary"]
        part["Rarity"] = self.select_from_list("Rarity", rarities)

        # Level Requirement
        part["LevelRequirement"] = int(self.get_input(
            "Level Requirement: ",
            default="1",
            validator=lambda x: x.isdigit() and int(x) >= 1
        ))

        # Unlock Cost
        suggested_cost = self.calculate_unlock_cost(part["Rarity"], part["LevelRequirement"])
        part["UnlockCost"] = int(self.get_input(
            f"Unlock Cost (suggested: {suggested_cost}): ",
            default=str(suggested_cost),
            validator=lambda x: x.isdigit() and int(x) >= 0
        ))

        # Unlocked by default?
        part["UnlockedByDefault"] = self.get_yes_no("Unlocked by default?", default=False)

        # Generate balanced stats
        print(f"\n{Colors.OKCYAN}Generating balanced stats...{Colors.ENDC}")
        part["StatModifiers"] = self.create_balanced_stats(part["Category"], part["Rarity"])

        # Allow customization
        if self.get_yes_no("Customize stats?", default=False):
            part["StatModifiers"] = self.customize_stats(part["StatModifiers"])

        # Asset paths
        print(f"\n{Colors.OKCYAN}Asset References:{Colors.ENDC}")
        part["PartMeshPath"] = self.get_input(
            "Part Mesh Path (optional): ",
            default="",
            required=False
        )
        part["IconPath"] = self.get_input(
            "Icon Path (optional): ",
            default="",
            required=False
        )

        # Upgrade chain
        part["ParentPartID"] = self.get_input(
            "Parent Part ID (for upgrades, optional): ",
            default="",
            required=False
        )
        part["CanBeUpgraded"] = self.get_yes_no("Can be upgraded?", default=True)
        part["UpgradeLevel"] = int(self.get_input(
            "Upgrade Level (0 = base): ",
            default="0",
            validator=lambda x: x.isdigit() and 0 <= int(x) <= 5
        ))

        # Add to library
        self.parts_library.append(part)

        # Show summary
        print(f"\n{Colors.OKGREEN}✓ Part created successfully!{Colors.ENDC}")
        self.print_part_summary(part)

        # Save immediately?
        if self.get_yes_no("\nSave to file now?", default=True):
            self.save_parts_to_file()

        self.wait_for_enter()

    def create_ship_skin(self):
        """Interactive ship skin creation"""
        self.clear_screen()
        self.print_header("Create New Ship Skin")

        skin = {}

        # Skin ID
        skin["SkinID"] = self.get_input(
            "Skin ID (e.g., skin_racing_red): ",
            validator=self.validate_skin_id
        )

        # Display Name
        skin["DisplayName"] = self.get_input(
            "Display Name (e.g., Racing Red): ",
            required=True
        )

        # Description
        skin["Description"] = self.get_input(
            "Description: ",
            default="A unique ship appearance"
        )

        # Rarity
        rarities = ["Common", "Uncommon", "Rare", "Epic", "Legendary"]
        skin["Rarity"] = self.select_from_list("Rarity", rarities)

        # Unlock Cost
        suggested_cost = self.calculate_skin_unlock_cost(skin["Rarity"])
        skin["UnlockCost"] = int(self.get_input(
            f"Unlock Cost (suggested: {suggested_cost}): ",
            default=str(suggested_cost),
            validator=lambda x: x.isdigit() and int(x) >= 0
        ))

        # Unlocked by default?
        skin["UnlockedByDefault"] = self.get_yes_no("Unlocked by default?", default=False)

        # Asset paths
        print(f"\n{Colors.OKCYAN}Asset References:{Colors.ENDC}")
        skin["SkinMaterialPath"] = self.get_input(
            "Skin Material Path (optional): ",
            default="",
            required=False
        )
        skin["IconPath"] = self.get_input(
            "Icon Path (optional): ",
            default="",
            required=False
        )

        # Add to library
        self.skins_library.append(skin)

        # Show summary
        print(f"\n{Colors.OKGREEN}✓ Skin created successfully!{Colors.ENDC}")
        self.print_skin_summary(skin)

        # Save immediately?
        if self.get_yes_no("\nSave to file now?", default=True):
            self.save_skins_to_file()

        self.wait_for_enter()

    def create_physics_config(self):
        """Interactive physics config creation"""
        self.clear_screen()
        self.print_header("Create Physics Configuration")

        config = {}

        # Config Name
        config["ConfigName"] = self.get_input(
            "Config Name (e.g., Balanced, Speed, Tank): ",
            required=True
        )

        print(f"\n{Colors.OKCYAN}Select preset or customize:{Colors.ENDC}")
        print("  1. Balanced (all-rounder)")
        print("  2. Speed (high speed, low armor)")
        print("  3. Tank (high armor, low speed)")
        print("  4. Agile (high maneuverability)")
        print("  5. Custom")

        preset = input(f"{Colors.OKCYAN}Select preset: {Colors.ENDC}")

        if preset == "1":
            config.update(self.get_balanced_preset())
        elif preset == "2":
            config.update(self.get_speed_preset())
        elif preset == "3":
            config.update(self.get_tank_preset())
        elif preset == "4":
            config.update(self.get_agile_preset())
        else:
            config.update(self.get_custom_physics_config())

        # Add to library
        self.physics_configs.append(config)

        # Show summary
        print(f"\n{Colors.OKGREEN}✓ Physics config created successfully!{Colors.ENDC}")
        self.print_physics_config_summary(config)

        # Save immediately?
        if self.get_yes_no("\nSave to file now?", default=True):
            self.save_physics_configs_to_file()

        self.wait_for_enter()

    def quick_content_generator(self):
        """Quick generator for multiple items"""
        self.clear_screen()
        self.print_header("Quick Content Generator")

        print("Generate a complete set of parts for a category:")
        print()

        categories = ["Engine", "Thrusters", "Hull", "Wings", "Weapon", "Shield"]
        category = self.select_from_list("Category", categories)

        # Generate 5 variants (one per rarity)
        rarities = ["Common", "Uncommon", "Rare", "Epic", "Legendary"]

        print(f"\n{Colors.OKCYAN}Generating {len(rarities)} {category} parts...{Colors.ENDC}")

        for i, rarity in enumerate(rarities, 1):
            part = {
                "PartID": f"{category.lower()}_{rarity.lower()}_{i:02d}",
                "DisplayName": f"{rarity} {category}",
                "Description": f"A {rarity.lower()} quality {category.lower()}",
                "Category": category,
                "Rarity": rarity,
                "LevelRequirement": i,
                "UnlockCost": self.calculate_unlock_cost(rarity, i),
                "UnlockedByDefault": rarity == "Common",
                "StatModifiers": self.create_balanced_stats(category, rarity),
                "PartMeshPath": "",
                "IconPath": "",
                "ParentPartID": "" if i == 1 else f"{category.lower()}_{rarities[i-2].lower()}_{i-1:02d}",
                "CanBeUpgraded": i < len(rarities),
                "UpgradeLevel": i - 1
            }
            self.parts_library.append(part)
            print(f"{Colors.OKGREEN}  ✓ Created: {part['PartID']}{Colors.ENDC}")

        print(f"\n{Colors.OKGREEN}Generated {len(rarities)} parts successfully!{Colors.ENDC}")

        # Auto-save
        self.save_parts_to_file()

        self.wait_for_enter()

    def view_all_content(self):
        """View all created content"""
        self.clear_screen()
        self.print_header("All Content")

        # Parts
        if self.parts_library:
            print(f"\n{Colors.BOLD}Parts ({len(self.parts_library)}):{Colors.ENDC}")
            for part in self.parts_library:
                print(f"  • {part['PartID']} - {part['DisplayName']} ({part['Rarity']} {part['Category']})")

        # Skins
        if self.skins_library:
            print(f"\n{Colors.BOLD}Skins ({len(self.skins_library)}):{Colors.ENDC}")
            for skin in self.skins_library:
                print(f"  • {skin['SkinID']} - {skin['DisplayName']} ({skin['Rarity']})")

        # Physics Configs
        if self.physics_configs:
            print(f"\n{Colors.BOLD}Physics Configs ({len(self.physics_configs)}):{Colors.ENDC}")
            for config in self.physics_configs:
                print(f"  • {config['ConfigName']}")

        if not (self.parts_library or self.skins_library or self.physics_configs):
            print(f"{Colors.WARNING}No content created yet{Colors.ENDC}")

        self.wait_for_enter()

    def validate_all_content(self):
        """Validate all content"""
        self.clear_screen()
        self.print_header("Content Validation")

        errors = []
        warnings = []

        # Validate parts
        for part in self.parts_library:
            part_errors = self.validate_part(part)
            if part_errors:
                errors.extend([f"Part '{part['PartID']}': {e}" for e in part_errors])

        # Validate skins
        for skin in self.skins_library:
            skin_errors = self.validate_skin(skin)
            if skin_errors:
                errors.extend([f"Skin '{skin['SkinID']}': {e}" for e in skin_errors])

        # Print results
        if errors:
            print(f"{Colors.FAIL}Found {len(errors)} errors:{Colors.ENDC}")
            for error in errors:
                print(f"{Colors.FAIL}  ✗ {error}{Colors.ENDC}")
        else:
            print(f"{Colors.OKGREEN}✓ All content is valid!{Colors.ENDC}")

        if warnings:
            print(f"\n{Colors.WARNING}Warnings:{Colors.ENDC}")
            for warning in warnings:
                print(f"{Colors.WARNING}  ⚠ {warning}{Colors.ENDC}")

        self.wait_for_enter()

    def export_to_unreal(self):
        """Export content to Unreal-ready JSON format"""
        self.clear_screen()
        self.print_header("Export to Unreal Format")

        # Save all to files
        parts_saved = self.save_parts_to_file()
        skins_saved = self.save_skins_to_file()
        configs_saved = self.save_physics_configs_to_file()

        if parts_saved or skins_saved or configs_saved:
            print(f"\n{Colors.OKGREEN}✓ All content exported successfully!{Colors.ENDC}")
            print(f"\nOutput directory: {Colors.OKCYAN}{os.path.abspath(self.output_dir)}{Colors.ENDC}")
            print(f"\nNext steps:")
            print(f"  1. Copy files to your Unreal project directory")
            print(f"  2. Run: python batch_import_content.py {self.output_dir}")
            print(f"  3. Content will be imported to data tables automatically")
        else:
            print(f"{Colors.WARNING}No content to export{Colors.ENDC}")

        self.wait_for_enter()

    def load_existing_content(self):
        """Load existing JSON files"""
        self.clear_screen()
        self.print_header("Load Existing Content")

        filepath = self.get_input("Path to JSON file: ", required=True)

        if not os.path.exists(filepath):
            self.print_error(f"File not found: {filepath}")
            self.wait_for_enter()
            return

        try:
            with open(filepath, 'r', encoding='utf-8') as f:
                data = json.load(f)

            loaded_count = 0

            if 'parts' in data:
                self.parts_library.extend(data['parts'])
                loaded_count += len(data['parts'])
                print(f"{Colors.OKGREEN}Loaded {len(data['parts'])} parts{Colors.ENDC}")

            if 'skins' in data:
                self.skins_library.extend(data['skins'])
                loaded_count += len(data['skins'])
                print(f"{Colors.OKGREEN}Loaded {len(data['skins'])} skins{Colors.ENDC}")

            if 'configs' in data:
                self.physics_configs.extend(data['configs'])
                loaded_count += len(data['configs'])
                print(f"{Colors.OKGREEN}Loaded {len(data['configs'])} configs{Colors.ENDC}")

            if loaded_count > 0:
                print(f"\n{Colors.OKGREEN}✓ Loaded {loaded_count} items successfully{Colors.ENDC}")
            else:
                print(f"{Colors.WARNING}No content found in file{Colors.ENDC}")

        except Exception as e:
            self.print_error(f"Failed to load file: {e}")

        self.wait_for_enter()

    def show_help(self):
        """Show help documentation"""
        self.clear_screen()
        self.print_header("Help & Documentation")

        help_text = """
Ship Content Creator - Quick Start Guide

1. Creating Parts:
   - Use option 1 to create individual parts
   - Use option 8 for quick batch generation
   - Parts have stats that affect gameplay

2. Creating Skins:
   - Skins are cosmetic only
   - Don't affect gameplay stats
   - Can have different rarities

3. Physics Configs:
   - Define ship handling presets
   - Can be applied to different ship types
   - Affects core flight physics

4. Workflow:
   - Create content using the menu
   - Content is saved automatically
   - Export to Unreal format (option 6)
   - Use batch_import_content.py to import

5. Validation:
   - Use option 5 to check for errors
   - Fix any errors before exporting
   - Warnings are suggestions, not errors

6. Stat Balance:
   - Stats auto-scale with rarity
   - Common = 1.0x, Legendary = 2.0x
   - Customize stats for unique items

7. Asset References:
   - Leave paths empty during creation
   - Fill in after creating assets in Unreal
   - Or use organize_content.py to auto-organize

Tips:
- Use descriptive IDs and names
- Follow naming conventions
- Test balance in-game
- Start with Common items
- Build upgrade chains
"""

        print(help_text)
        self.wait_for_enter()

    # ============================================================================
    # HELPER FUNCTIONS
    # ============================================================================

    def create_balanced_stats(self, category: str, rarity: str) -> Dict[str, float]:
        """Create balanced stats based on category and rarity"""
        base_stats = self.BASE_STATS.get(category, {"Mass": 500.0})
        multiplier = self.STAT_MULTIPLIERS.get(rarity, 1.0)

        stats = {
            "ThrustPower": 0.0,
            "MaxVelocity": 0.0,
            "RotationSpeed": 0.0,
            "Acceleration": 0.0,
            "HullIntegrity": 0.0,
            "ShieldStrength": 0.0,
            "WeaponDamage": 0.0,
            "EnergyCapacity": 0.0,
            "EnergyRegenRate": 0.0,
            "Mass": 0.0
        }

        # Apply base stats with multiplier
        for key, value in base_stats.items():
            if key in stats:
                stats[key] = value * multiplier

        return stats

    def customize_stats(self, stats: Dict[str, float]) -> Dict[str, float]:
        """Allow user to customize stats"""
        print(f"\n{Colors.OKCYAN}Customize Stats (press Enter to keep current value):{Colors.ENDC}")

        for key, value in stats.items():
            if value > 0:  # Only show non-zero stats
                new_value = self.get_input(
                    f"{key} [{value:.2f}]: ",
                    default=str(value),
                    required=False,
                    validator=lambda x: x == "" or (x.replace('.', '').replace('-', '').isdigit())
                )
                if new_value:
                    stats[key] = float(new_value)

        return stats

    def calculate_unlock_cost(self, rarity: str, level: int) -> int:
        """Calculate suggested unlock cost"""
        base_costs = {
            "Common": 0,
            "Uncommon": 500,
            "Rare": 2000,
            "Epic": 5000,
            "Legendary": 10000
        }
        return base_costs.get(rarity, 0) * level

    def calculate_skin_unlock_cost(self, rarity: str) -> int:
        """Calculate suggested skin unlock cost"""
        costs = {
            "Common": 0,
            "Uncommon": 1000,
            "Rare": 3000,
            "Epic": 7000,
            "Legendary": 15000
        }
        return costs.get(rarity, 0)

    def validate_part(self, part: Dict) -> List[str]:
        """Validate a ship part"""
        errors = []

        if not part.get("PartID"):
            errors.append("PartID is required")

        if not part.get("DisplayName"):
            errors.append("DisplayName is required")

        if part.get("UnlockCost", 0) < 0:
            errors.append("UnlockCost cannot be negative")

        if part.get("LevelRequirement", 0) < 1:
            errors.append("LevelRequirement must be at least 1")

        # Validate stats
        stats = part.get("StatModifiers", {})
        if stats.get("Mass", 0) <= 0:
            errors.append("Mass must be positive")

        return errors

    def validate_skin(self, skin: Dict) -> List[str]:
        """Validate a ship skin"""
        errors = []

        if not skin.get("SkinID"):
            errors.append("SkinID is required")

        if not skin.get("DisplayName"):
            errors.append("DisplayName is required")

        if skin.get("UnlockCost", 0) < 0:
            errors.append("UnlockCost cannot be negative")

        return errors

    def validate_part_id(self, part_id: str) -> bool:
        """Validate part ID format"""
        return bool(part_id and '_' in part_id and part_id.islower().replace('_', '').isalnum())

    def validate_skin_id(self, skin_id: str) -> bool:
        """Validate skin ID format"""
        return bool(skin_id and skin_id.startswith("skin_") and skin_id.islower().replace('_', '').isalnum())

    def save_parts_to_file(self) -> bool:
        """Save parts library to JSON"""
        if not self.parts_library:
            return False

        filepath = os.path.join(self.output_dir, "ship_parts.json")
        try:
            with open(filepath, 'w', encoding='utf-8') as f:
                json.dump({"parts": self.parts_library}, f, indent=2)
            print(f"{Colors.OKGREEN}✓ Saved parts to: {filepath}{Colors.ENDC}")
            return True
        except Exception as e:
            self.print_error(f"Failed to save parts: {e}")
            return False

    def save_skins_to_file(self) -> bool:
        """Save skins library to JSON"""
        if not self.skins_library:
            return False

        filepath = os.path.join(self.output_dir, "ship_skins.json")
        try:
            with open(filepath, 'w', encoding='utf-8') as f:
                json.dump({"skins": self.skins_library}, f, indent=2)
            print(f"{Colors.OKGREEN}✓ Saved skins to: {filepath}{Colors.ENDC}")
            return True
        except Exception as e:
            self.print_error(f"Failed to save skins: {e}")
            return False

    def save_physics_configs_to_file(self) -> bool:
        """Save physics configs to JSON"""
        if not self.physics_configs:
            return False

        for config in self.physics_configs:
            filepath = os.path.join(self.output_dir, f"physics_config_{config['ConfigName'].lower()}.json")
            try:
                with open(filepath, 'w', encoding='utf-8') as f:
                    json.dump(config, f, indent=2)
                print(f"{Colors.OKGREEN}✓ Saved config to: {filepath}{Colors.ENDC}")
            except Exception as e:
                self.print_error(f"Failed to save config: {e}")
                return False

        return True

    def get_balanced_preset(self) -> Dict:
        """Get balanced physics preset"""
        return {
            "ThrustForce": 50000.0,
            "RotationTorque": 20000.0,
            "MaxSpeed": 1000.0,
            "Mass": 10000.0,
            "Drag": 0.1,
            "AngularDrag": 0.05
        }

    def get_speed_preset(self) -> Dict:
        """Get speed physics preset"""
        return {
            "ThrustForce": 70000.0,
            "RotationTorque": 15000.0,
            "MaxSpeed": 1500.0,
            "Mass": 8000.0,
            "Drag": 0.05,
            "AngularDrag": 0.08
        }

    def get_tank_preset(self) -> Dict:
        """Get tank physics preset"""
        return {
            "ThrustForce": 40000.0,
            "RotationTorque": 10000.0,
            "MaxSpeed": 600.0,
            "Mass": 15000.0,
            "Drag": 0.15,
            "AngularDrag": 0.1
        }

    def get_agile_preset(self) -> Dict:
        """Get agile physics preset"""
        return {
            "ThrustForce": 45000.0,
            "RotationTorque": 30000.0,
            "MaxSpeed": 900.0,
            "Mass": 7000.0,
            "Drag": 0.08,
            "AngularDrag": 0.03
        }

    def get_custom_physics_config(self) -> Dict:
        """Get custom physics config from user"""
        config = {}

        print(f"\n{Colors.OKCYAN}Enter custom values:{Colors.ENDC}")

        config["ThrustForce"] = float(self.get_input(
            "Thrust Force [50000]: ",
            default="50000",
            validator=lambda x: x.replace('.', '').isdigit()
        ))

        config["RotationTorque"] = float(self.get_input(
            "Rotation Torque [20000]: ",
            default="20000",
            validator=lambda x: x.replace('.', '').isdigit()
        ))

        config["MaxSpeed"] = float(self.get_input(
            "Max Speed [1000]: ",
            default="1000",
            validator=lambda x: x.replace('.', '').isdigit()
        ))

        config["Mass"] = float(self.get_input(
            "Mass [10000]: ",
            default="10000",
            validator=lambda x: x.replace('.', '').isdigit()
        ))

        config["Drag"] = float(self.get_input(
            "Drag [0.1]: ",
            default="0.1",
            validator=lambda x: x.replace('.', '').isdigit()
        ))

        config["AngularDrag"] = float(self.get_input(
            "Angular Drag [0.05]: ",
            default="0.05",
            validator=lambda x: x.replace('.', '').isdigit()
        ))

        return config

    # UI Helper Functions
    def clear_screen(self):
        """Clear console screen"""
        os.system('cls' if os.name == 'nt' else 'clear')

    def print_header(self, text: str):
        """Print colored header"""
        print(f"\n{Colors.HEADER}{Colors.BOLD}{'='*80}{Colors.ENDC}")
        print(f"{Colors.HEADER}{Colors.BOLD}{text.center(80)}{Colors.ENDC}")
        print(f"{Colors.HEADER}{Colors.BOLD}{'='*80}{Colors.ENDC}\n")

    def print_success(self, text: str):
        print(f"{Colors.OKGREEN}✓ {text}{Colors.ENDC}")

    def print_error(self, text: str):
        print(f"{Colors.FAIL}✗ {text}{Colors.ENDC}")

    def print_part_summary(self, part: Dict):
        """Print part summary"""
        print(f"\n{Colors.BOLD}Part Summary:{Colors.ENDC}")
        print(f"  ID: {part['PartID']}")
        print(f"  Name: {part['DisplayName']}")
        print(f"  Category: {part['Category']}")
        print(f"  Rarity: {part['Rarity']}")
        print(f"  Level Req: {part['LevelRequirement']}")
        print(f"  Unlock Cost: {part['UnlockCost']}")

        print(f"\n{Colors.BOLD}Stats:{Colors.ENDC}")
        for key, value in part['StatModifiers'].items():
            if value > 0:
                print(f"  {key}: {value:.2f}")

    def print_skin_summary(self, skin: Dict):
        """Print skin summary"""
        print(f"\n{Colors.BOLD}Skin Summary:{Colors.ENDC}")
        print(f"  ID: {skin['SkinID']}")
        print(f"  Name: {skin['DisplayName']}")
        print(f"  Rarity: {skin['Rarity']}")
        print(f"  Unlock Cost: {skin['UnlockCost']}")

    def print_physics_config_summary(self, config: Dict):
        """Print physics config summary"""
        print(f"\n{Colors.BOLD}Physics Config Summary:{Colors.ENDC}")
        for key, value in config.items():
            print(f"  {key}: {value}")

    def get_input(self, prompt: str, default: str = "", required: bool = True, validator = None) -> str:
        """Get validated input from user"""
        while True:
            value = input(f"{Colors.OKCYAN}{prompt}{Colors.ENDC}").strip()

            if not value and default:
                return default

            if not value and not required:
                return ""

            if not value and required:
                self.print_error("This field is required")
                continue

            if validator and not validator(value):
                self.print_error("Invalid input format")
                continue

            return value

    def get_yes_no(self, prompt: str, default: bool = False) -> bool:
        """Get yes/no input"""
        default_str = "Y/n" if default else "y/N"
        response = input(f"{Colors.OKCYAN}{prompt} [{default_str}]: {Colors.ENDC}").strip().lower()

        if not response:
            return default

        return response in ['y', 'yes']

    def select_from_list(self, prompt: str, options: List[str]) -> str:
        """Select from a list of options"""
        print(f"\n{Colors.OKCYAN}{prompt}:{Colors.ENDC}")
        for i, option in enumerate(options, 1):
            print(f"  {i}. {option}")

        while True:
            choice = input(f"{Colors.OKCYAN}Select option (1-{len(options)}): {Colors.ENDC}").strip()

            if choice.isdigit() and 1 <= int(choice) <= len(options):
                return options[int(choice) - 1]

            self.print_error("Invalid selection")

    def wait_for_enter(self):
        """Wait for user to press Enter"""
        input(f"\n{Colors.OKCYAN}Press Enter to continue...{Colors.ENDC}")


def main():
    """Main entry point"""
    creator = ShipContentCreator()
    creator.run()


if __name__ == "__main__":
    main()
