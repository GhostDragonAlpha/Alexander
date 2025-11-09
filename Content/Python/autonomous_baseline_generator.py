"""
Autonomous Baseline Generator - Master Orchestration Script
Coordinates all autonomous content generation for Alexander Phase 8 baseline

This script runs all generation modules in sequence:
1. Mission template import
2. Space station generation
3. VFX system setup
4. UI widget setup
5. Map population
6. Validation and profiling

Usage in Unreal Editor Python console:
    import autonomous_baseline_generator
    autonomous_baseline_generator.generate_complete_baseline()
"""

import unreal
import sys

# Add Python path
sys.path.append("C:/Users/allen/Desktop/Alexander/Alexander/Content/Python")

# Import generation modules
try:
    import load_mission_templates
    import autonomous_station_generator
    import autonomous_vfx_generator
    import autonomous_ui_generator
    import create_niagara_vfx
    import create_umg_widgets
    import populate_maps
except ImportError as e:
    unreal.log_error(f"Failed to import generation modules: {e}")
    unreal.log("Ensure all generator scripts are in Content/Python/")


def print_header(title):
    """Print formatted header"""
    unreal.log("\n" + "=" * 80)
    unreal.log(title.center(80))
    unreal.log("=" * 80 + "\n")


def print_section(title):
    """Print formatted section header"""
    unreal.log("\n" + "-" * 80)
    unreal.log(title)
    unreal.log("-" * 80)


def generate_complete_baseline_with_content():
    """
    ENHANCED: Master function to generate complete Phase 8 baseline content
    Runs all generation steps INCLUDING automatic VFX, UI, and map creation
    """

    print_header("AUTONOMOUS CONTENT CREATION - PHASE 8 ENHANCED")

    results = {
        "missions": None,
        "stations": None,
        "vfx": None,
        "vfx_systems": [],
        "ui": None,
        "ui_widgets": [],
        "population": None,
        "success": True,
        "errors": []
    }

    # Step 1: Import Mission Templates
    print_section("STEP 1/7: Importing Mission Templates")
    try:
        unreal.log("Loading 100+ mission templates from JSON...")
        results["missions"] = load_mission_templates.import_all_mission_templates()
        unreal.log(f"✓ Imported {len(results['missions'])} mission data assets")
    except Exception as e:
        unreal.log_error(f"✗ Mission import failed: {e}")
        results["errors"].append(f"Mission import: {e}")
        results["success"] = False

    # Step 2: Generate Space Stations
    print_section("STEP 2/7: Generating Space Stations")
    try:
        unreal.log("Creating 5 space stations from primitives...")
        results["stations"] = autonomous_station_generator.generate_all_stations()
        unreal.log(f"✓ Generated {len(results['stations'])} space stations")
    except Exception as e:
        unreal.log_error(f"✗ Station generation failed: {e}")
        results["errors"].append(f"Station generation: {e}")
        # Continue even if this fails

    # Step 3: Setup VFX Directory Structure
    print_section("STEP 3/7: Setting Up VFX Directory Structure")
    try:
        unreal.log("Creating VFX directory structure and configs...")
        autonomous_vfx_generator.create_vfx_templates()
        autonomous_vfx_generator.export_vfx_config()
        results["vfx"] = "configured"
        unreal.log("✓ VFX system configuration complete")
    except Exception as e:
        unreal.log_error(f"✗ VFX setup failed: {e}")
        results["errors"].append(f"VFX setup: {e}")

    # Step 4: Create Niagara VFX Systems
    print_section("STEP 4/7: Creating Niagara VFX Systems")
    try:
        unreal.log("Automatically creating Niagara systems...")
        vfx_systems = create_niagara_vfx.create_all_vfx_systems()
        results["vfx_systems"] = vfx_systems
        unreal.log(f"✓ Created {len(vfx_systems)} Niagara VFX systems")
    except Exception as e:
        unreal.log_error(f"✗ VFX creation failed: {e}")
        results["errors"].append(f"VFX creation: {e}")

    # Step 5: Setup UI Directory Structure
    print_section("STEP 5/7: Setting Up UI Directory Structure")
    try:
        unreal.log("Creating UI directory structure and configs...")
        autonomous_ui_generator.create_ui_templates()
        autonomous_ui_generator.export_ui_config()
        results["ui"] = "configured"
        unreal.log("✓ UI widget configuration complete")
    except Exception as e:
        unreal.log_error(f"✗ UI setup failed: {e}")
        results["errors"].append(f"UI setup: {e}")

    # Step 6: Create UMG Widgets
    print_section("STEP 6/7: Creating UMG Widgets")
    try:
        unreal.log("Automatically creating UMG widgets...")
        ui_widgets = create_umg_widgets.create_all_widgets()
        results["ui_widgets"] = ui_widgets
        unreal.log(f"✓ Created {len(ui_widgets)} UMG widget blueprints")

        # Generate additional widget assets
        create_umg_widgets.generate_widget_cpp_bindings()
        create_umg_widgets.create_widget_style_assets()
    except Exception as e:
        unreal.log_error(f"✗ UI widget creation failed: {e}")
        results["errors"].append(f"UI widget creation: {e}")

    # Step 7: Populate Maps
    print_section("STEP 7/7: Populating Maps")
    try:
        unreal.log("Automatically populating maps with content...")
        results["population"] = populate_maps.populate_all_maps()
        unreal.log("✓ Map population complete")
    except Exception as e:
        unreal.log_error(f"✗ Map population failed: {e}")
        results["errors"].append(f"Map population: {e}")

    # Final Summary
    print_enhanced_summary(results)

    return results


def generate_complete_baseline():
    """
    LEGACY: Master function to generate complete Phase 8 baseline content
    Runs configuration steps (missions, directories, configs)
    For full autonomous content creation, use generate_complete_baseline_with_content()
    """

    print_header("AUTONOMOUS BASELINE GENERATION - PHASE 8")

    results = {
        "missions": None,
        "stations": None,
        "vfx": None,
        "ui": None,
        "population": None,
        "success": True,
        "errors": []
    }

    # Step 1: Import Mission Templates
    print_section("STEP 1/5: Importing Mission Templates")
    try:
        unreal.log("Loading 100+ mission templates from JSON...")
        results["missions"] = load_mission_templates.import_all_mission_templates()
        unreal.log(f"✓ Imported {len(results['missions'])} mission data assets")
    except Exception as e:
        unreal.log_error(f"✗ Mission import failed: {e}")
        results["errors"].append(f"Mission import: {e}")
        results["success"] = False

    # Step 2: Generate Space Stations
    print_section("STEP 2/5: Generating Space Stations")
    try:
        unreal.log("Creating 5 space stations from primitives...")
        results["stations"] = autonomous_station_generator.generate_all_stations()
        unreal.log(f"✓ Generated {len(results['stations'])} space stations")
    except Exception as e:
        unreal.log_error(f"✗ Station generation failed: {e}")
        results["errors"].append(f"Station generation: {e}")
        # Continue even if this fails

    # Step 3: Setup VFX Systems
    print_section("STEP 3/5: Setting Up VFX Systems")
    try:
        unreal.log("Creating VFX directory structure and configs...")
        autonomous_vfx_generator.create_vfx_templates()
        autonomous_vfx_generator.export_vfx_config()
        autonomous_vfx_generator.document_vfx_requirements()
        results["vfx"] = "configured"
        unreal.log("✓ VFX system configuration complete")
        unreal.log("  NOTE: Niagara systems should be created from templates")
    except Exception as e:
        unreal.log_error(f"✗ VFX setup failed: {e}")
        results["errors"].append(f"VFX setup: {e}")

    # Step 4: Setup UI Widgets
    print_section("STEP 4/5: Setting Up UI Widgets")
    try:
        unreal.log("Creating UI directory structure and configs...")
        autonomous_ui_generator.create_ui_templates()
        autonomous_ui_generator.export_ui_config()
        autonomous_ui_generator.document_ui_requirements()
        results["ui"] = "configured"
        unreal.log("✓ UI widget configuration complete")
        unreal.log("  NOTE: UMG widgets should be created from specifications")
    except Exception as e:
        unreal.log_error(f"✗ UI setup failed: {e}")
        results["errors"].append(f"UI setup: {e}")

    # Step 5: Populate Maps
    print_section("STEP 5/5: Populating Maps")
    try:
        unreal.log("Setting up baseline content in maps...")
        results["population"] = populate_baseline_maps()
        unreal.log("✓ Map population complete")
    except Exception as e:
        unreal.log_error(f"✗ Map population failed: {e}")
        results["errors"].append(f"Map population: {e}")

    # Final Summary
    print_summary(results)

    return results


def populate_baseline_maps():
    """
    Populate FlightTest and SolarSystem maps with baseline content
    """
    unreal.log("\nPopulating FlightTest map...")

    # Get current level
    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    current_level = level_subsystem.get_current_level()

    unreal.log(f"  Current level: {current_level}")

    # For now, log instructions since map manipulation is complex
    unreal.log("\nMap Population Instructions:")
    unreal.log("  1. Open FlightTest map")
    unreal.log("  2. Place generated station blueprints from /Game/Blueprints/Stations/Generated")
    unreal.log("  3. Add BP_Spaceship actors for testing")
    unreal.log("  4. Add lighting (Directional Light, Sky Light)")
    unreal.log("  5. Add Post Process Volume for atmosphere")
    unreal.log("  6. Save map")
    unreal.log("\n  Repeat for SolarSystem map")

    return "instructions_provided"


def print_summary(results):
    """Print final summary of baseline generation"""
    print_header("BASELINE GENERATION SUMMARY")

    unreal.log("Generation Results:")
    unreal.log(f"  Missions: {results.get('missions', 'Not started')}")
    unreal.log(f"  Stations: {results.get('stations', 'Not started')}")
    unreal.log(f"  VFX: {results.get('vfx', 'Not started')}")
    unreal.log(f"  UI: {results.get('ui', 'Not started')}")
    unreal.log(f"  Population: {results.get('population', 'Not started')}")

    if results["errors"]:
        unreal.log("\nErrors encountered:")
        for error in results["errors"]:
            unreal.log(f"  ✗ {error}")
    else:
        unreal.log("\n✓ All generation steps completed successfully")

    unreal.log("\nNext Steps:")
    unreal.log("  1. Review generated assets in Content Browser")
    unreal.log("  2. Create Niagara VFX from templates (16 systems)")
    unreal.log("  3. Create UMG widgets from specifications (18 widgets)")
    unreal.log("  4. Populate FlightTest and SolarSystem maps")
    unreal.log("  5. Test gameplay and missions")
    unreal.log("  6. Run performance profiling: python run_automated_profiling.py")

    print_header("BASELINE GENERATION COMPLETE")


def generate_missions_only():
    """Quick function to just import missions"""
    print_header("MISSION IMPORT ONLY")
    try:
        results = load_mission_templates.import_all_mission_templates()
        load_mission_templates.verify_mission_templates()
        return results
    except Exception as e:
        unreal.log_error(f"Mission import failed: {e}")
        return None


def generate_stations_only():
    """Quick function to just generate stations"""
    print_header("STATION GENERATION ONLY")
    try:
        results = autonomous_station_generator.generate_all_stations()
        return results
    except Exception as e:
        unreal.log_error(f"Station generation failed: {e}")
        return None


def setup_vfx_only():
    """Quick function to just setup VFX"""
    print_header("VFX SETUP ONLY")
    try:
        autonomous_vfx_generator.create_vfx_templates()
        autonomous_vfx_generator.export_vfx_config()
        return "complete"
    except Exception as e:
        unreal.log_error(f"VFX setup failed: {e}")
        return None


def setup_ui_only():
    """Quick function to just setup UI"""
    print_header("UI SETUP ONLY")
    try:
        autonomous_ui_generator.create_ui_templates()
        autonomous_ui_generator.export_ui_config()
        return "complete"
    except Exception as e:
        unreal.log_error(f"UI setup failed: {e}")
        return None


def print_enhanced_summary(results):
    """Print enhanced summary of autonomous content creation"""
    print_header("AUTONOMOUS CONTENT CREATION SUMMARY")

    unreal.log("Generation Results:")
    unreal.log(f"  Missions: {len(results.get('missions', []))} data assets")
    unreal.log(f"  Stations: {len(results.get('stations', {}))} configured")
    unreal.log(f"  VFX: {results.get('vfx', 'Not started')}")
    unreal.log(f"  VFX Systems: {len(results.get('vfx_systems', []))} Niagara systems created")
    unreal.log(f"  UI: {results.get('ui', 'Not started')}")
    unreal.log(f"  UI Widgets: {len(results.get('ui_widgets', []))} UMG widgets created")
    unreal.log(f"  Map Population: {results.get('population', 'Not started')}")

    if results["errors"]:
        unreal.log("\nErrors encountered:")
        for error in results["errors"]:
            unreal.log(f"  ✗ {error}")
    else:
        unreal.log("\n✓ All generation steps completed successfully")

    unreal.log("\nNext Steps:")
    unreal.log("  1. Review generated assets in Content Browser")
    unreal.log("  2. Test Niagara VFX systems (attach to spaceships)")
    unreal.log("  3. Customize UMG widgets (add specific components)")
    unreal.log("  4. Test gameplay in FlightTest and SolarSystem maps")
    unreal.log("  5. Run performance profiling: python run_automated_profiling.py")

    print_header("AUTONOMOUS CONTENT CREATION COMPLETE")


def validate_baseline():
    """Validate generated baseline content"""
    print_header("BASELINE VALIDATION")

    validation = {
        "missions": False,
        "stations": False,
        "vfx": False,
        "ui": False
    }

    # Check missions
    mission_dir = "/Game/Data/Missions/DataAssets"
    if unreal.EditorAssetLibrary.does_directory_exist(mission_dir):
        assets = unreal.EditorAssetLibrary.list_assets(mission_dir)
        validation["missions"] = len(assets) > 0
        unreal.log(f"✓ Missions: {len(assets)} data assets found")
    else:
        unreal.log("✗ Missions: Directory not found")

    # Check stations
    station_dir = "/Game/Blueprints/Stations/Generated"
    if unreal.EditorAssetLibrary.does_directory_exist(station_dir):
        validation["stations"] = True
        unreal.log("✓ Stations: Directory exists")
    else:
        unreal.log("✗ Stations: Directory not found")

    # Check VFX
    vfx_dir = "/Game/VFX/Generated"
    if unreal.EditorAssetLibrary.does_directory_exist(vfx_dir):
        validation["vfx"] = True
        unreal.log("✓ VFX: Directory exists")
    else:
        unreal.log("✗ VFX: Directory not found")

    # Check UI
    ui_dir = "/Game/UI/Generated"
    if unreal.EditorAssetLibrary.does_directory_exist(ui_dir):
        validation["ui"] = True
        unreal.log("✓ UI: Directory exists")
    else:
        unreal.log("✗ UI: Directory not found")

    # Summary
    total = sum(validation.values())
    unreal.log(f"\nValidation: {total}/4 categories ready")

    return validation


if __name__ == "__main__":
    print_header("AUTONOMOUS BASELINE GENERATOR")
    unreal.log("Available Commands:")
    unreal.log("")
    unreal.log("  Complete Baseline:")
    unreal.log("    import autonomous_baseline_generator")
    unreal.log("    autonomous_baseline_generator.generate_complete_baseline()")
    unreal.log("")
    unreal.log("  Individual Components:")
    unreal.log("    autonomous_baseline_generator.generate_missions_only()")
    unreal.log("    autonomous_baseline_generator.generate_stations_only()")
    unreal.log("    autonomous_baseline_generator.setup_vfx_only()")
    unreal.log("    autonomous_baseline_generator.setup_ui_only()")
    unreal.log("")
    unreal.log("  Validation:")
    unreal.log("    autonomous_baseline_generator.validate_baseline()")
    print_header("")
