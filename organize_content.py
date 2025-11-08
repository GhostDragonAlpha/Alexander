#!/usr/bin/env python3
"""
Content Organization Script
Creates proper folder structure and organizes ship customization assets
"""

import os
import sys
from typing import Dict, List, Tuple
import unreal

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

def print_header(text: str):
    print(f"\n{Colors.HEADER}{Colors.BOLD}{'='*80}{Colors.ENDC}")
    print(f"{Colors.HEADER}{Colors.BOLD}{text.center(80)}{Colors.ENDC}")
    print(f"{Colors.HEADER}{Colors.BOLD}{'='*80}{Colors.ENDC}\n")

def print_success(text: str):
    print(f"{Colors.OKGREEN}✓ {text}{Colors.ENDC}")

def print_error(text: str):
    print(f"{Colors.FAIL}✗ {text}{Colors.ENDC}")

def print_info(text: str):
    print(f"{Colors.OKCYAN}ℹ {text}{Colors.ENDC}")


class ContentOrganizer:
    """Organizes ship customization content into proper folder structure"""

    # Folder structure definition
    FOLDER_STRUCTURE = {
        "/Game/ShipCustomization": [
            "DataTables",
            "DataAssets",
            "Materials",
            "Materials/Masters",
            "Materials/Instances",
            "Meshes",
            "Meshes/Engines",
            "Meshes/Thrusters",
            "Meshes/Hulls",
            "Meshes/Wings",
            "Meshes/Cockpits",
            "Meshes/Weapons",
            "Icons",
            "Textures",
            "Blueprints",
            "PhysicsConfigs"
        ]
    }

    # Asset type to folder mapping
    ASSET_TYPE_FOLDERS = {
        "DataTable": "/Game/ShipCustomization/DataTables",
        "DataAsset": "/Game/ShipCustomization/DataAssets",
        "Material": "/Game/ShipCustomization/Materials/Masters",
        "MaterialInstanceConstant": "/Game/ShipCustomization/Materials/Instances",
        "StaticMesh": "/Game/ShipCustomization/Meshes",
        "Texture2D": "/Game/ShipCustomization/Textures",
        "Blueprint": "/Game/ShipCustomization/Blueprints"
    }

    # Naming conventions
    NAMING_CONVENTIONS = {
        "parts_datatable": "DT_ShipParts",
        "skins_datatable": "DT_ShipSkins",
        "mesh_prefix": "SM_Ship_",
        "material_master_prefix": "M_Ship_",
        "material_instance_prefix": "MI_Ship_",
        "texture_prefix": "T_Ship_",
        "icon_prefix": "T_Icon_",
        "blueprint_prefix": "BP_Ship_",
        "physics_config_prefix": "PC_"
    }

    def __init__(self):
        self.editor_asset_lib = unreal.EditorAssetLibrary()
        self.asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        self.stats = {
            'folders_created': 0,
            'assets_moved': 0,
            'assets_renamed': 0,
            'errors': []
        }

    def create_folder_structure(self) -> bool:
        """Create the complete folder structure"""
        print_header("Creating Folder Structure")

        try:
            for base_path, subfolders in self.FOLDER_STRUCTURE.items():
                # Create base folder
                if not self.editor_asset_lib.does_directory_exist(base_path):
                    if self.editor_asset_lib.make_directory(base_path):
                        print_success(f"Created: {base_path}")
                        self.stats['folders_created'] += 1
                    else:
                        print_error(f"Failed to create: {base_path}")
                        self.stats['errors'].append(f"Failed to create: {base_path}")

                # Create subfolders
                for subfolder in subfolders:
                    folder_path = f"{base_path}/{subfolder}"
                    if not self.editor_asset_lib.does_directory_exist(folder_path):
                        if self.editor_asset_lib.make_directory(folder_path):
                            print_success(f"Created: {folder_path}")
                            self.stats['folders_created'] += 1
                        else:
                            print_error(f"Failed to create: {folder_path}")
                            self.stats['errors'].append(f"Failed to create: {folder_path}")

            print_success(f"Created {self.stats['folders_created']} folders")
            return True

        except Exception as e:
            print_error(f"Failed to create folder structure: {e}")
            return False

    def organize_assets_by_type(self, search_path: str = "/Game") -> bool:
        """Organize assets into correct folders based on type"""
        print_header("Organizing Assets by Type")

        try:
            # Get all assets
            all_assets = self.editor_asset_lib.list_assets(search_path, recursive=True)
            ship_assets = [
                asset for asset in all_assets
                if "ship" in asset.lower() or "customization" in asset.lower()
            ]

            print_info(f"Found {len(ship_assets)} ship-related assets")

            for asset_path in ship_assets:
                self._organize_single_asset(asset_path)

            print_success(f"Moved {self.stats['assets_moved']} assets")
            return True

        except Exception as e:
            print_error(f"Failed to organize assets: {e}")
            return False

    def _organize_single_asset(self, asset_path: str):
        """Organize a single asset"""
        try:
            # Load asset
            asset = self.editor_asset_lib.load_asset(asset_path)
            if not asset:
                return

            # Get asset class
            asset_class = asset.get_class().get_name()

            # Determine target folder
            target_folder = self.ASSET_TYPE_FOLDERS.get(asset_class)
            if not target_folder:
                return

            # Check if asset needs specialized subfolder (for meshes)
            if asset_class == "StaticMesh":
                target_folder = self._get_mesh_subfolder(asset_path)

            # Get current folder
            current_folder = os.path.dirname(asset_path)

            # Move if needed
            if current_folder != target_folder:
                asset_name = os.path.basename(asset_path)
                new_path = f"{target_folder}/{asset_name}"

                if self.editor_asset_lib.rename_asset(asset_path, new_path):
                    print_success(f"Moved: {asset_name} -> {target_folder}")
                    self.stats['assets_moved'] += 1
                else:
                    print_error(f"Failed to move: {asset_name}")
                    self.stats['errors'].append(f"Failed to move: {asset_path}")

        except Exception as e:
            print_error(f"Error organizing {asset_path}: {e}")

    def _get_mesh_subfolder(self, asset_path: str) -> str:
        """Determine subfolder for mesh based on name"""
        asset_name = os.path.basename(asset_path).lower()

        if "engine" in asset_name:
            return "/Game/ShipCustomization/Meshes/Engines"
        elif "thruster" in asset_name:
            return "/Game/ShipCustomization/Meshes/Thrusters"
        elif "hull" in asset_name:
            return "/Game/ShipCustomization/Meshes/Hulls"
        elif "wing" in asset_name:
            return "/Game/ShipCustomization/Meshes/Wings"
        elif "cockpit" in asset_name:
            return "/Game/ShipCustomization/Meshes/Cockpits"
        elif "weapon" in asset_name:
            return "/Game/ShipCustomization/Meshes/Weapons"
        else:
            return "/Game/ShipCustomization/Meshes"

    def validate_naming_conventions(self, auto_fix: bool = False) -> Tuple[List[str], List[str]]:
        """Validate and optionally fix naming conventions"""
        print_header("Validating Naming Conventions")

        violations = []
        fixed = []

        try:
            # Get all ship customization assets
            assets = self.editor_asset_lib.list_assets("/Game/ShipCustomization", recursive=True)

            for asset_path in assets:
                asset_name = os.path.basename(asset_path)
                asset = self.editor_asset_lib.load_asset(asset_path)
                if not asset:
                    continue

                asset_class = asset.get_class().get_name()

                # Check naming based on type
                expected_prefix = self._get_expected_prefix(asset_class, asset_path)
                if expected_prefix and not asset_name.startswith(expected_prefix):
                    violation = f"{asset_name} should start with {expected_prefix}"
                    violations.append(violation)
                    print_error(f"Naming violation: {violation}")

                    # Auto-fix if requested
                    if auto_fix:
                        new_name = f"{expected_prefix}{asset_name}"
                        new_path = os.path.join(os.path.dirname(asset_path), new_name)

                        if self.editor_asset_lib.rename_asset(asset_path, new_path):
                            fixed.append(f"{asset_name} -> {new_name}")
                            print_success(f"Fixed: {asset_name} -> {new_name}")
                            self.stats['assets_renamed'] += 1

            print_info(f"Found {len(violations)} naming violations")
            if auto_fix:
                print_success(f"Fixed {len(fixed)} violations")

            return violations, fixed

        except Exception as e:
            print_error(f"Failed to validate naming: {e}")
            return violations, fixed

    def _get_expected_prefix(self, asset_class: str, asset_path: str) -> str:
        """Get expected naming prefix for asset type"""
        if asset_class == "StaticMesh":
            return self.NAMING_CONVENTIONS["mesh_prefix"]
        elif asset_class == "Material":
            return self.NAMING_CONVENTIONS["material_master_prefix"]
        elif asset_class == "MaterialInstanceConstant":
            return self.NAMING_CONVENTIONS["material_instance_prefix"]
        elif asset_class == "Texture2D":
            if "Icon" in asset_path:
                return self.NAMING_CONVENTIONS["icon_prefix"]
            else:
                return self.NAMING_CONVENTIONS["texture_prefix"]
        elif "Blueprint" in asset_class:
            return self.NAMING_CONVENTIONS["blueprint_prefix"]

        return ""

    def generate_naming_conventions_document(self, output_path: str = None) -> str:
        """Generate naming conventions documentation"""
        print_header("Generating Naming Conventions Document")

        doc = """# Ship Customization Asset Naming Conventions

## Overview
This document defines the naming conventions for all ship customization assets.
Following these conventions ensures consistency and makes assets easy to find.

## Prefixes by Asset Type

### Static Meshes
- **Prefix:** `SM_Ship_`
- **Format:** `SM_Ship_<Category>_<Name>_<Variant>`
- **Examples:**
  - `SM_Ship_Engine_Basic_01`
  - `SM_Ship_Thruster_Advanced_02`
  - `SM_Ship_Hull_Heavy_01`

### Materials
- **Master Materials:** `M_Ship_<Name>`
- **Material Instances:** `MI_Ship_<Name>_<Variant>`
- **Examples:**
  - `M_Ship_Hull` (master)
  - `MI_Ship_Hull_Red` (instance)
  - `MI_Ship_Hull_Racing` (instance)

### Textures
- **General Textures:** `T_Ship_<Name>_<Type>`
- **Icons:** `T_Icon_<Category>_<Name>`
- **Examples:**
  - `T_Ship_Hull_Diffuse`
  - `T_Ship_Engine_Normal`
  - `T_Icon_Engine_Basic01`

### Data Tables
- **Format:** `DT_<ContentType>`
- **Examples:**
  - `DT_ShipParts`
  - `DT_ShipSkins`

### Data Assets
- **Format:** `DA_<Type>_<Name>`
- **Examples:**
  - `DA_PhysicsConfig_Balanced`
  - `DA_PhysicsConfig_Speed`

### Blueprints
- **Prefix:** `BP_Ship_`
- **Format:** `BP_Ship_<Type>_<Name>`
- **Examples:**
  - `BP_Ship_Engine_Basic`
  - `BP_Ship_Weapon_Laser`

### Physics Configs
- **Prefix:** `PC_`
- **Format:** `PC_<Name>`
- **Examples:**
  - `PC_Balanced`
  - `PC_Speed`
  - `PC_Tank`

## Part ID Conventions

Ship parts use FName identifiers in data tables:

- **Format:** `<category>_<name>_<variant>`
- **Examples:**
  - `engine_basic_01`
  - `thruster_advanced_02`
  - `hull_heavy_mk1`

## Skin ID Conventions

Ship skins use FName identifiers:

- **Format:** `skin_<theme>_<variant>`
- **Examples:**
  - `skin_default`
  - `skin_racing_red`
  - `skin_military_camo`

## Folder Structure

```
/Game/ShipCustomization/
├── DataTables/         # Data tables for parts and skins
├── DataAssets/         # Physics configs and other data assets
├── Materials/
│   ├── Masters/        # Master materials
│   └── Instances/      # Material instances
├── Meshes/
│   ├── Engines/        # Engine meshes
│   ├── Thrusters/      # Thruster meshes
│   ├── Hulls/          # Hull meshes
│   ├── Wings/          # Wing meshes
│   ├── Cockpits/       # Cockpit meshes
│   └── Weapons/        # Weapon meshes
├── Icons/              # UI icons
├── Textures/           # Textures (diffuse, normal, etc.)
├── Blueprints/         # Blueprint assets
└── PhysicsConfigs/     # Physics configuration assets
```

## Best Practices

1. **Be Descriptive:** Names should clearly indicate what the asset is
2. **Use Variants:** Use numeric suffixes for variations (01, 02, etc.)
3. **Avoid Abbreviations:** Use full words unless commonly understood
4. **Use PascalCase:** For asset names after the prefix
5. **Use snake_case:** For IDs in data tables
6. **Be Consistent:** Follow the same pattern across similar assets

## Validation

Use the `organize_content.py` script to validate naming conventions:

```bash
python organize_content.py --validate
python organize_content.py --validate --auto-fix
```

## Examples

### Good Naming
- ✓ `SM_Ship_Engine_Quantum_01`
- ✓ `MI_Ship_Hull_Racing_Red`
- ✓ `T_Icon_Engine_Quantum01`
- ✓ `engine_quantum_01`

### Bad Naming
- ✗ `EngMesh1` (no prefix, unclear)
- ✗ `red_hull` (missing prefix)
- ✗ `SM_Engine` (missing Ship identifier)
- ✗ `MyTestAsset` (unclear purpose)
"""

        # Save to file if path provided
        if output_path:
            try:
                with open(output_path, 'w', encoding='utf-8') as f:
                    f.write(doc)
                print_success(f"Saved naming conventions to: {output_path}")
            except Exception as e:
                print_error(f"Failed to save document: {e}")

        return doc

    def print_summary(self):
        """Print organization summary"""
        print_header("Organization Summary")

        print(f"Folders Created:  {Colors.OKGREEN}{self.stats['folders_created']}{Colors.ENDC}")
        print(f"Assets Moved:     {Colors.OKGREEN}{self.stats['assets_moved']}{Colors.ENDC}")
        print(f"Assets Renamed:   {Colors.OKGREEN}{self.stats['assets_renamed']}{Colors.ENDC}")

        if self.stats['errors']:
            print(f"\nErrors ({len(self.stats['errors'])}):")
            for error in self.stats['errors']:
                print_error(f"  {error}")

        print()


def main():
    """Main entry point"""
    print_header("Ship Customization Content Organizer")

    # Parse command line arguments
    create_folders = "--create-folders" in sys.argv or len(sys.argv) == 1
    organize_assets = "--organize" in sys.argv or len(sys.argv) == 1
    validate_naming = "--validate" in sys.argv
    auto_fix = "--auto-fix" in sys.argv
    generate_doc = "--generate-doc" in sys.argv

    # Create organizer
    organizer = ContentOrganizer()

    # Create folder structure
    if create_folders:
        organizer.create_folder_structure()

    # Organize assets
    if organize_assets:
        organizer.organize_assets_by_type()

    # Validate naming conventions
    if validate_naming:
        organizer.validate_naming_conventions(auto_fix=auto_fix)

    # Generate documentation
    if generate_doc:
        doc_path = os.path.join(os.getcwd(), "ASSET_NAMING_CONVENTIONS.md")
        organizer.generate_naming_conventions_document(doc_path)

    # Print summary
    organizer.print_summary()

    print_success("Organization complete!")


if __name__ == "__main__":
    main()
