"""
Comprehensive Baseline Validation Script
Validates all autonomous baseline generation components

This script checks:
1. Mission data assets (11 expected files)
2. Directory structures
3. Generated content locations
4. Space station blueprints
5. VFX system setup
6. UI widget configuration
7. Map population status

Usage in Unreal Editor Python console:
    import validate_baseline
    validate_baseline.run_full_validation()

Or run specific checks:
    validate_baseline.validate_missions()
    validate_baseline.validate_stations()
    validate_baseline.validate_vfx()
    validate_baseline.validate_ui()
"""

import unreal
import os
import json
from datetime import datetime


class ValidationReport:
    """Stores validation results"""
    def __init__(self):
        self.timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        self.checks = []
        self.passed = 0
        self.failed = 0
        self.warnings = 0

    def add_check(self, category, check_name, status, message, remediation=""):
        """Add a validation check result"""
        self.checks.append({
            "category": category,
            "check": check_name,
            "status": status,  # "PASS", "FAIL", "WARNING"
            "message": message,
            "remediation": remediation
        })

        if status == "PASS":
            self.passed += 1
        elif status == "FAIL":
            self.failed += 1
        elif status == "WARNING":
            self.warnings += 1

    def print_report(self):
        """Print formatted validation report"""
        print("\n" + "=" * 100)
        print(f"BASELINE VALIDATION REPORT - {self.timestamp}".center(100))
        print("=" * 100)

        # Group checks by category
        categories = {}
        for check in self.checks:
            cat = check["category"]
            if cat not in categories:
                categories[cat] = []
            categories[cat].append(check)

        # Print each category
        for category, checks in categories.items():
            print(f"\n{category}")
            print("-" * 100)

            for check in checks:
                status_symbol = "✓" if check["status"] == "PASS" else ("⚠" if check["status"] == "WARNING" else "✗")
                status_color = "PASS" if check["status"] == "PASS" else ("WARN" if check["status"] == "WARNING" else "FAIL")

                print(f"  {status_symbol} [{status_color}] {check['check']}")
                print(f"      {check['message']}")

                if check["remediation"]:
                    print(f"      → Fix: {check['remediation']}")

        # Summary
        print("\n" + "=" * 100)
        print(f"SUMMARY: {self.passed} Passed | {self.failed} Failed | {self.warnings} Warnings")

        overall_status = "✓ VALIDATION PASSED" if self.failed == 0 else "✗ VALIDATION FAILED"
        print(overall_status.center(100))
        print("=" * 100 + "\n")

    def export_json(self, output_path):
        """Export report as JSON"""
        report_data = {
            "timestamp": self.timestamp,
            "summary": {
                "passed": self.passed,
                "failed": self.failed,
                "warnings": self.warnings,
                "total": len(self.checks)
            },
            "checks": self.checks
        }

        os.makedirs(os.path.dirname(output_path), exist_ok=True)
        with open(output_path, 'w', encoding='utf-8') as f:
            json.dump(report_data, f, indent=2)

        print(f"\nValidation report exported to: {output_path}")


def validate_missions(report):
    """Validate mission template data assets"""
    category = "MISSION TEMPLATES"

    expected_files = [
        "DA_Combat_Missions",
        "DA_Assassination_Missions",
        "DA_Bounty_Missions",
        "DA_Delivery_Missions",
        "DA_Escort_Missions",
        "DA_Exploration_Missions",
        "DA_Mining_Missions",
        "DA_Reconnaissance_Missions",
        "DA_Rescue_Missions",
        "DA_Salvage_Missions",
        "DA_Smuggling_Missions"
    ]

    mission_dir = "/Game/Data/Missions/DataAssets"

    # Check directory exists
    if not unreal.EditorAssetLibrary.does_directory_exist(mission_dir):
        report.add_check(
            category,
            "Mission Directory Exists",
            "FAIL",
            f"Directory not found: {mission_dir}",
            "Run: import load_mission_templates; load_mission_templates.import_all_mission_templates()"
        )
        return
    else:
        report.add_check(
            category,
            "Mission Directory Exists",
            "PASS",
            f"Directory found: {mission_dir}"
        )

    # List all assets in directory
    assets = unreal.EditorAssetLibrary.list_assets(mission_dir, recursive=False)
    asset_names = [os.path.basename(asset) for asset in assets]

    # Check each expected file
    total_missions = 0
    found_files = 0

    for expected in expected_files:
        found = any(expected in name for name in asset_names)

        if found:
            found_files += 1
            # Load asset and count missions
            matching_assets = [a for a in assets if expected in a]
            if matching_assets:
                asset = unreal.EditorAssetLibrary.load_asset(matching_assets[0])
                try:
                    templates = asset.get_editor_property("templates")
                    mission_count = len(templates) if templates else 0
                    total_missions += mission_count

                    report.add_check(
                        category,
                        f"{expected}",
                        "PASS",
                        f"Found with {mission_count} mission templates"
                    )
                except Exception as e:
                    report.add_check(
                        category,
                        f"{expected}",
                        "WARNING",
                        f"Found but could not read templates: {e}",
                        "Check data asset structure"
                    )
        else:
            report.add_check(
                category,
                f"{expected}",
                "FAIL",
                "Data asset not found",
                f"Import {expected.replace('DA_', '').replace('_Missions', '').lower()} missions"
            )

    # Summary check
    if found_files == len(expected_files):
        report.add_check(
            category,
            "All Mission Categories Present",
            "PASS",
            f"All 11 mission categories imported with {total_missions} total missions"
        )
    else:
        report.add_check(
            category,
            "All Mission Categories Present",
            "FAIL",
            f"Only {found_files}/{len(expected_files)} categories found",
            "Re-run mission import script"
        )


def validate_stations(report):
    """Validate space station blueprints"""
    category = "SPACE STATIONS"

    station_dir = "/Game/Blueprints/Stations/Generated"

    # Check directory exists
    if not unreal.EditorAssetLibrary.does_directory_exist(station_dir):
        report.add_check(
            category,
            "Station Directory Exists",
            "FAIL",
            f"Directory not found: {station_dir}",
            "Run: import autonomous_station_generator; autonomous_station_generator.generate_all_stations()"
        )
        return
    else:
        report.add_check(
            category,
            "Station Directory Exists",
            "PASS",
            f"Directory found: {station_dir}"
        )

    # List blueprints
    assets = unreal.EditorAssetLibrary.list_assets(station_dir, recursive=True)
    blueprint_assets = [a for a in assets if a.endswith("_C")]

    expected_stations = [
        "BP_Station_Trading",
        "BP_Station_Mining",
        "BP_Station_Military",
        "BP_Station_Research",
        "BP_Station_Diplomatic"
    ]

    found_count = 0
    for expected in expected_stations:
        found = any(expected in asset for asset in blueprint_assets)

        if found:
            found_count += 1
            report.add_check(
                category,
                f"{expected}",
                "PASS",
                "Blueprint found"
            )
        else:
            report.add_check(
                category,
                f"{expected}",
                "WARNING",
                "Blueprint not found - may need regeneration",
                f"Generate {expected} using autonomous_station_generator"
            )

    # Overall check
    if found_count >= 3:
        report.add_check(
            category,
            "Sufficient Stations Generated",
            "PASS",
            f"{found_count}/5 station blueprints available"
        )
    elif found_count > 0:
        report.add_check(
            category,
            "Sufficient Stations Generated",
            "WARNING",
            f"Only {found_count}/5 station blueprints - recommend generating all 5"
        )
    else:
        report.add_check(
            category,
            "Sufficient Stations Generated",
            "FAIL",
            "No station blueprints found",
            "Run station generator script"
        )


def validate_vfx(report):
    """Validate VFX system setup"""
    category = "VFX SYSTEMS"

    vfx_base_dir = "/Game/VFX"
    vfx_generated_dir = "/Game/VFX/Generated"

    # Check base directory
    if not unreal.EditorAssetLibrary.does_directory_exist(vfx_base_dir):
        report.add_check(
            category,
            "VFX Base Directory",
            "FAIL",
            f"Base directory not found: {vfx_base_dir}",
            "Run: import autonomous_vfx_generator; autonomous_vfx_generator.create_vfx_templates()"
        )
        return
    else:
        report.add_check(
            category,
            "VFX Base Directory",
            "PASS",
            "Base directory exists"
        )

    # Check generated directory
    if unreal.EditorAssetLibrary.does_directory_exist(vfx_generated_dir):
        report.add_check(
            category,
            "VFX Generated Directory",
            "PASS",
            "Generated directory exists"
        )
    else:
        report.add_check(
            category,
            "VFX Generated Directory",
            "WARNING",
            "Generated directory not found - VFX templates not created yet",
            "Create Niagara systems from VFX specifications"
        )

    # Check for VFX config file
    vfx_config_path = "C:/Users/allen/Desktop/Alexander/Alexander/Content/Data/VFX/vfx_requirements.json"
    if os.path.exists(vfx_config_path):
        report.add_check(
            category,
            "VFX Configuration File",
            "PASS",
            f"Configuration file exists: {vfx_config_path}"
        )

        # Read and validate config
        try:
            with open(vfx_config_path, 'r') as f:
                vfx_config = json.load(f)
                vfx_count = len(vfx_config.get("vfx_systems", []))

                if vfx_count >= 16:
                    report.add_check(
                        category,
                        "VFX System Definitions",
                        "PASS",
                        f"{vfx_count} VFX systems defined in configuration"
                    )
                else:
                    report.add_check(
                        category,
                        "VFX System Definitions",
                        "WARNING",
                        f"Only {vfx_count} VFX systems defined (expected 16+)",
                        "Review VFX requirements document"
                    )
        except Exception as e:
            report.add_check(
                category,
                "VFX Configuration Valid",
                "WARNING",
                f"Could not parse config file: {e}"
            )
    else:
        report.add_check(
            category,
            "VFX Configuration File",
            "WARNING",
            "Configuration file not found - VFX system not configured",
            "Run autonomous_vfx_generator.export_vfx_config()"
        )


def validate_ui(report):
    """Validate UI widget setup"""
    category = "UI WIDGETS"

    ui_base_dir = "/Game/UI"
    ui_generated_dir = "/Game/UI/Generated"

    # Check base directory
    if not unreal.EditorAssetLibrary.does_directory_exist(ui_base_dir):
        report.add_check(
            category,
            "UI Base Directory",
            "FAIL",
            f"Base directory not found: {ui_base_dir}",
            "Run: import autonomous_ui_generator; autonomous_ui_generator.create_ui_templates()"
        )
        return
    else:
        report.add_check(
            category,
            "UI Base Directory",
            "PASS",
            "Base directory exists"
        )

    # Check generated directory
    if unreal.EditorAssetLibrary.does_directory_exist(ui_generated_dir):
        report.add_check(
            category,
            "UI Generated Directory",
            "PASS",
            "Generated directory exists"
        )
    else:
        report.add_check(
            category,
            "UI Generated Directory",
            "WARNING",
            "Generated directory not found - UI widgets not created yet",
            "Create UMG widgets from UI specifications"
        )

    # Check for UI config file
    ui_config_path = "C:/Users/allen/Desktop/Alexander/Alexander/Content/Data/UI/ui_requirements.json"
    if os.path.exists(ui_config_path):
        report.add_check(
            category,
            "UI Configuration File",
            "PASS",
            f"Configuration file exists: {ui_config_path}"
        )

        # Read and validate config
        try:
            with open(ui_config_path, 'r') as f:
                ui_config = json.load(f)
                widget_count = len(ui_config.get("ui_widgets", []))

                if widget_count >= 18:
                    report.add_check(
                        category,
                        "UI Widget Definitions",
                        "PASS",
                        f"{widget_count} UI widgets defined in configuration"
                    )
                else:
                    report.add_check(
                        category,
                        "UI Widget Definitions",
                        "WARNING",
                        f"Only {widget_count} UI widgets defined (expected 18+)",
                        "Review UI requirements document"
                    )
        except Exception as e:
            report.add_check(
                category,
                "UI Configuration Valid",
                "WARNING",
                f"Could not parse config file: {e}"
            )
    else:
        report.add_check(
            category,
            "UI Configuration File",
            "WARNING",
            "Configuration file not found - UI system not configured",
            "Run autonomous_ui_generator.export_ui_config()"
        )


def validate_content_structure(report):
    """Validate overall content directory structure"""
    category = "CONTENT STRUCTURE"

    required_dirs = [
        "/Game/Data",
        "/Game/Data/Missions",
        "/Game/Data/Missions/DataAssets",
        "/Game/Blueprints",
        "/Game/Blueprints/Stations",
        "/Game/Maps"
    ]

    for directory in required_dirs:
        if unreal.EditorAssetLibrary.does_directory_exist(directory):
            report.add_check(
                category,
                f"Directory: {directory}",
                "PASS",
                "Directory exists"
            )
        else:
            report.add_check(
                category,
                f"Directory: {directory}",
                "FAIL",
                "Directory missing",
                f"Create directory: {directory}"
            )


def validate_json_templates(report):
    """Validate source JSON template files exist"""
    category = "SOURCE JSON FILES"

    templates_dir = "C:/Users/allen/Desktop/Alexander/Alexander/Content/Data/Missions/Templates"

    expected_files = [
        "mission_templates_combat.json",
        "mission_templates_assassination.json",
        "mission_templates_bounty.json",
        "mission_templates_delivery.json",
        "mission_templates_escort.json",
        "mission_templates_exploration.json",
        "mission_templates_mining.json",
        "mission_templates_reconnaissance.json",
        "mission_templates_rescue.json",
        "mission_templates_salvage.json",
        "mission_templates_smuggling.json"
    ]

    found_count = 0
    total_missions = 0

    for filename in expected_files:
        filepath = os.path.join(templates_dir, filename)

        if os.path.exists(filepath):
            found_count += 1

            # Count missions in file
            try:
                with open(filepath, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                    # Get mission count (handles different JSON structures)
                    mission_count = 0
                    if isinstance(data, dict):
                        for key, value in data.items():
                            if isinstance(value, list):
                                mission_count = len(value)
                                break
                    elif isinstance(data, list):
                        mission_count = len(data)

                    total_missions += mission_count

                    report.add_check(
                        category,
                        filename,
                        "PASS",
                        f"File exists with {mission_count} missions"
                    )
            except Exception as e:
                report.add_check(
                    category,
                    filename,
                    "WARNING",
                    f"File exists but could not parse: {e}"
                )
        else:
            report.add_check(
                category,
                filename,
                "FAIL",
                "Source JSON file missing",
                f"Restore file: {filepath}"
            )

    # Summary
    if found_count == len(expected_files):
        report.add_check(
            category,
            "All Source Files Present",
            "PASS",
            f"All 11 JSON files found with {total_missions} total mission definitions"
        )
    else:
        report.add_check(
            category,
            "All Source Files Present",
            "FAIL",
            f"Only {found_count}/11 JSON files found"
        )


def validate_maps(report):
    """Validate game maps"""
    category = "GAME MAPS"

    expected_maps = [
        "/Game/Maps/FlightTest",
        "/Game/Maps/SolarSystem"
    ]

    for map_path in expected_maps:
        # Check if map asset exists
        if unreal.EditorAssetLibrary.does_asset_exist(map_path):
            report.add_check(
                category,
                f"Map: {os.path.basename(map_path)}",
                "PASS",
                "Map exists"
            )
        else:
            report.add_check(
                category,
                f"Map: {os.path.basename(map_path)}",
                "WARNING",
                "Map not found",
                f"Create or verify map path: {map_path}"
            )


def run_full_validation():
    """Run complete baseline validation"""

    print("\n" + "=" * 100)
    print("STARTING COMPREHENSIVE BASELINE VALIDATION".center(100))
    print("=" * 100 + "\n")

    report = ValidationReport()

    # Run all validation checks
    print("Validating source JSON files...")
    validate_json_templates(report)

    print("Validating content structure...")
    validate_content_structure(report)

    print("Validating mission templates...")
    validate_missions(report)

    print("Validating space stations...")
    validate_stations(report)

    print("Validating VFX systems...")
    validate_vfx(report)

    print("Validating UI widgets...")
    validate_ui(report)

    print("Validating game maps...")
    validate_maps(report)

    # Print report
    report.print_report()

    # Export JSON report
    output_path = "C:/Users/allen/Desktop/Alexander/Alexander/Saved/ValidationReports/baseline_validation_" + \
                  datetime.now().strftime("%Y%m%d_%H%M%S") + ".json"
    report.export_json(output_path)

    return report


def quick_validation():
    """Quick validation - just check if major components exist"""
    print("\n" + "=" * 100)
    print("QUICK BASELINE VALIDATION".center(100))
    print("=" * 100 + "\n")

    checks = []

    # Check missions
    mission_dir = "/Game/Data/Missions/DataAssets"
    if unreal.EditorAssetLibrary.does_directory_exist(mission_dir):
        assets = unreal.EditorAssetLibrary.list_assets(mission_dir, recursive=False)
        print(f"✓ Missions: {len(assets)} data assets found")
        checks.append(("Missions", True))
    else:
        print(f"✗ Missions: Directory not found")
        checks.append(("Missions", False))

    # Check stations
    station_dir = "/Game/Blueprints/Stations/Generated"
    if unreal.EditorAssetLibrary.does_directory_exist(station_dir):
        print(f"✓ Stations: Directory exists")
        checks.append(("Stations", True))
    else:
        print(f"✗ Stations: Directory not found")
        checks.append(("Stations", False))

    # Check VFX
    vfx_dir = "/Game/VFX"
    if unreal.EditorAssetLibrary.does_directory_exist(vfx_dir):
        print(f"✓ VFX: Directory exists")
        checks.append(("VFX", True))
    else:
        print(f"✗ VFX: Directory not found")
        checks.append(("VFX", False))

    # Check UI
    ui_dir = "/Game/UI"
    if unreal.EditorAssetLibrary.does_directory_exist(ui_dir):
        print(f"✓ UI: Directory exists")
        checks.append(("UI", True))
    else:
        print(f"✗ UI: Directory not found")
        checks.append(("UI", False))

    # Summary
    passed = sum(1 for _, status in checks if status)
    total = len(checks)

    print(f"\n{'=' * 100}")
    print(f"Quick Check: {passed}/{total} components ready".center(100))
    print(f"{'=' * 100}\n")

    return passed == total


if __name__ == "__main__":
    print("\nBaseline Validation Script")
    print("=" * 100)
    print("\nAvailable Commands:")
    print("  Full Validation:")
    print("    import validate_baseline")
    print("    validate_baseline.run_full_validation()")
    print("")
    print("  Quick Validation:")
    print("    validate_baseline.quick_validation()")
    print("")
    print("  Individual Components:")
    print("    report = validate_baseline.ValidationReport()")
    print("    validate_baseline.validate_missions(report)")
    print("    validate_baseline.validate_stations(report)")
    print("    validate_baseline.validate_vfx(report)")
    print("    validate_baseline.validate_ui(report)")
    print("    report.print_report()")
    print("=" * 100)
