#!/usr/bin/env python3
"""
Batch Content Import Script
Automatically imports ship customization content from JSON files to Unreal Engine Data Tables
"""

import json
import os
import sys
from pathlib import Path
from typing import Dict, List, Any, Tuple
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
    UNDERLINE = '\033[4m'

def print_header(text: str):
    """Print colored header"""
    print(f"\n{Colors.HEADER}{Colors.BOLD}{'='*80}{Colors.ENDC}")
    print(f"{Colors.HEADER}{Colors.BOLD}{text.center(80)}{Colors.ENDC}")
    print(f"{Colors.HEADER}{Colors.BOLD}{'='*80}{Colors.ENDC}\n")

def print_success(text: str):
    """Print success message"""
    print(f"{Colors.OKGREEN}✓ {text}{Colors.ENDC}")

def print_error(text: str):
    """Print error message"""
    print(f"{Colors.FAIL}✗ {text}{Colors.ENDC}")

def print_warning(text: str):
    """Print warning message"""
    print(f"{Colors.WARNING}⚠ {text}{Colors.ENDC}")

def print_info(text: str):
    """Print info message"""
    print(f"{Colors.OKCYAN}ℹ {text}{Colors.ENDC}")


class ContentImporter:
    """Handles batch import of ship customization content"""

    def __init__(self):
        self.editor_utility = None
        self.parts_data_table = None
        self.skins_data_table = None
        self.import_stats = {
            'parts_imported': 0,
            'skins_imported': 0,
            'configs_created': 0,
            'errors': [],
            'warnings': []
        }

    def initialize(self):
        """Initialize Unreal Engine editor utilities"""
        print_header("Initializing Content Importer")

        try:
            # Load editor utility blueprint class
            utility_class = unreal.load_class(None, "/Game/EditorUtilities/BP_ShipContentEditorUtility.BP_ShipContentEditorUtility_C")
            if not utility_class:
                # Try C++ class directly
                utility_class = unreal.ShipContentEditorUtility

            self.editor_utility = unreal.new_object(utility_class)
            print_success("Loaded ShipContentEditorUtility")

            # Load or create data tables
            self.parts_data_table = self._load_or_create_data_table(
                "/Game/ShipCustomization/DataTables/DT_ShipParts",
                "ShipPartData"
            )

            self.skins_data_table = self._load_or_create_data_table(
                "/Game/ShipCustomization/DataTables/DT_ShipSkins",
                "ShipSkinData"
            )

            print_success("Initialization complete")
            return True

        except Exception as e:
            print_error(f"Initialization failed: {e}")
            return False

    def _load_or_create_data_table(self, asset_path: str, row_struct_name: str):
        """Load existing data table or create new one"""
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        editor_asset_lib = unreal.EditorAssetLibrary()

        # Check if exists
        if editor_asset_lib.does_asset_exist(asset_path):
            print_info(f"Loading existing data table: {asset_path}")
            return editor_asset_lib.load_asset(asset_path)

        # Create new data table
        print_info(f"Creating new data table: {asset_path}")

        # Create folder if needed
        folder_path = os.path.dirname(asset_path)
        if not editor_asset_lib.does_directory_exist(folder_path):
            editor_asset_lib.make_directory(folder_path)

        # Create data table
        factory = unreal.DataTableFactory()

        # Set row struct based on type
        if "Parts" in asset_path:
            factory.struct = unreal.load_object(None, "/Script/Alexander.ShipPartData")
        elif "Skins" in asset_path:
            factory.struct = unreal.load_object(None, "/Script/Alexander.ShipSkinData")

        asset_name = os.path.basename(asset_path)
        package_path = os.path.dirname(asset_path)

        data_table = asset_tools.create_asset(asset_name, package_path, unreal.DataTable, factory)

        if data_table:
            print_success(f"Created data table: {asset_path}")
            return data_table
        else:
            print_error(f"Failed to create data table: {asset_path}")
            return None

    def import_json_file(self, json_path: str) -> bool:
        """Import a single JSON file"""
        print_info(f"Importing: {json_path}")

        if not os.path.exists(json_path):
            print_error(f"File not found: {json_path}")
            self.import_stats['errors'].append(f"File not found: {json_path}")
            return False

        try:
            with open(json_path, 'r', encoding='utf-8') as f:
                data = json.load(f)

            # Detect file type
            if 'parts' in data:
                return self._import_parts(data['parts'], json_path)
            elif 'skins' in data:
                return self._import_skins(data['skins'], json_path)
            elif 'ConfigName' in data:
                return self._import_physics_config(data, json_path)
            else:
                print_warning(f"Unknown JSON format: {json_path}")
                self.import_stats['warnings'].append(f"Unknown format: {json_path}")
                return False

        except Exception as e:
            print_error(f"Failed to import {json_path}: {e}")
            self.import_stats['errors'].append(f"{json_path}: {e}")
            return False

    def _import_parts(self, parts: List[Dict], source_file: str) -> bool:
        """Import ship parts"""
        print_info(f"Importing {len(parts)} ship parts...")

        if not self.parts_data_table:
            print_error("Parts data table not available")
            return False

        imported = 0
        failed = 0

        for part_data in parts:
            try:
                # Use editor utility to import
                if self.editor_utility:
                    result = self.editor_utility.import_parts_from_json(source_file, self.parts_data_table)
                    if result.b_success:
                        imported += result.items_imported
                        failed += result.items_failed
                else:
                    # Fallback: direct import
                    part_id = part_data.get('PartID', '')
                    if self._add_part_to_table(part_data):
                        imported += 1
                        print_success(f"  Imported part: {part_id}")
                    else:
                        failed += 1
                        print_error(f"  Failed to import part: {part_id}")

            except Exception as e:
                failed += 1
                print_error(f"  Error importing part: {e}")

        self.import_stats['parts_imported'] += imported
        print_success(f"Parts: {imported} imported, {failed} failed")
        return imported > 0

    def _import_skins(self, skins: List[Dict], source_file: str) -> bool:
        """Import ship skins"""
        print_info(f"Importing {len(skins)} ship skins...")

        if not self.skins_data_table:
            print_error("Skins data table not available")
            return False

        imported = 0
        failed = 0

        for skin_data in skins:
            try:
                if self.editor_utility:
                    result = self.editor_utility.import_skins_from_json(source_file, self.skins_data_table)
                    if result.b_success:
                        imported += result.items_imported
                        failed += result.items_failed
                else:
                    # Fallback: direct import
                    skin_id = skin_data.get('SkinID', '')
                    if self._add_skin_to_table(skin_data):
                        imported += 1
                        print_success(f"  Imported skin: {skin_id}")
                    else:
                        failed += 1
                        print_error(f"  Failed to import skin: {skin_id}")

            except Exception as e:
                failed += 1
                print_error(f"  Error importing skin: {e}")

        self.import_stats['skins_imported'] += imported
        print_success(f"Skins: {imported} imported, {failed} failed")
        return imported > 0

    def _import_physics_config(self, config_data: Dict, source_file: str) -> bool:
        """Import physics configuration"""
        config_name = config_data.get('ConfigName', 'Unknown')
        print_info(f"Importing physics config: {config_name}")

        try:
            if self.editor_utility:
                asset_path = f"/Game/ShipCustomization/DataAssets/PC_{config_name}"
                if self.editor_utility.create_physics_config_asset(source_file, asset_path):
                    self.import_stats['configs_created'] += 1
                    print_success(f"  Created physics config: {config_name}")
                    return True

            print_warning(f"  Physics config import not fully implemented")
            return False

        except Exception as e:
            print_error(f"  Failed to import physics config: {e}")
            return False

    def _add_part_to_table(self, part_data: Dict) -> bool:
        """Add part directly to data table (fallback method)"""
        # This is a simplified version - the C++ utility has more robust handling
        try:
            # Convert dict to Unreal struct
            part_id = unreal.Name(part_data.get('PartID', ''))

            # Note: Direct row addition requires proper struct initialization
            # This is a placeholder - actual implementation depends on UE Python API
            print_warning("Direct part addition not fully implemented - use C++ utility")
            return False

        except Exception as e:
            print_error(f"Failed to add part to table: {e}")
            return False

    def _add_skin_to_table(self, skin_data: Dict) -> bool:
        """Add skin directly to data table (fallback method)"""
        try:
            skin_id = unreal.Name(skin_data.get('SkinID', ''))
            print_warning("Direct skin addition not fully implemented - use C++ utility")
            return False

        except Exception as e:
            print_error(f"Failed to add skin to table: {e}")
            return False

    def import_directory(self, directory_path: str, recursive: bool = True) -> bool:
        """Import all JSON files in a directory"""
        print_header(f"Batch Importing from: {directory_path}")

        if not os.path.isdir(directory_path):
            print_error(f"Directory not found: {directory_path}")
            return False

        # Find all JSON files
        json_files = []
        if recursive:
            for root, dirs, files in os.walk(directory_path):
                for file in files:
                    if file.endswith('.json'):
                        json_files.append(os.path.join(root, file))
        else:
            json_files = [
                os.path.join(directory_path, f)
                for f in os.listdir(directory_path)
                if f.endswith('.json')
            ]

        print_info(f"Found {len(json_files)} JSON files")

        # Import each file
        success_count = 0
        for json_file in json_files:
            if self.import_json_file(json_file):
                success_count += 1

        print_success(f"Imported {success_count}/{len(json_files)} files successfully")
        return success_count > 0

    def validate_imported_content(self) -> bool:
        """Validate all imported content"""
        print_header("Validating Imported Content")

        if not self.editor_utility:
            print_warning("Editor utility not available - skipping validation")
            return False

        try:
            # Validate parts
            if self.parts_data_table:
                print_info("Validating parts...")
                parts_result = self.editor_utility.validate_all_parts(self.parts_data_table)
                print_info(f"Parts: {parts_result.valid_items}/{parts_result.total_items} valid")

                for error in parts_result.validation_errors:
                    print_error(f"  {error}")

            # Validate skins
            if self.skins_data_table:
                print_info("Validating skins...")
                skins_result = self.editor_utility.validate_all_skins(self.skins_data_table)
                print_info(f"Skins: {skins_result.valid_items}/{skins_result.total_items} valid")

                for error in skins_result.validation_errors:
                    print_error(f"  {error}")

            return True

        except Exception as e:
            print_error(f"Validation failed: {e}")
            return False

    def print_summary(self):
        """Print import summary"""
        print_header("Import Summary")

        print(f"Parts Imported:       {Colors.OKGREEN}{self.import_stats['parts_imported']}{Colors.ENDC}")
        print(f"Skins Imported:       {Colors.OKGREEN}{self.import_stats['skins_imported']}{Colors.ENDC}")
        print(f"Configs Created:      {Colors.OKGREEN}{self.import_stats['configs_created']}{Colors.ENDC}")

        if self.import_stats['errors']:
            print(f"\nErrors ({len(self.import_stats['errors'])}):")
            for error in self.import_stats['errors']:
                print_error(f"  {error}")

        if self.import_stats['warnings']:
            print(f"\nWarnings ({len(self.import_stats['warnings'])}):")
            for warning in self.import_stats['warnings']:
                print_warning(f"  {warning}")

        print()


def main():
    """Main entry point"""
    print_header("Ship Customization Content Batch Importer")

    # Parse command line arguments
    if len(sys.argv) < 2:
        print_error("Usage: python batch_import_content.py <directory_path> [--no-recursive] [--no-validate]")
        print_info("Example: python batch_import_content.py ./content/ship_parts")
        sys.exit(1)

    directory_path = sys.argv[1]
    recursive = "--no-recursive" not in sys.argv
    validate = "--no-validate" not in sys.argv

    # Create importer
    importer = ContentImporter()

    # Initialize
    if not importer.initialize():
        print_error("Failed to initialize importer")
        sys.exit(1)

    # Import content
    if not importer.import_directory(directory_path, recursive):
        print_warning("Import completed with errors")

    # Validate if requested
    if validate:
        importer.validate_imported_content()

    # Print summary
    importer.print_summary()

    print_success("Batch import complete!")


if __name__ == "__main__":
    main()
