"""
AUTONOMOUS DEVELOPMENT SYSTEM WITH VALIDATION
Automatically runs validation on Unreal startup
"""
import unreal
import sys
import os

print("=" * 80)
print("AUTONOMOUS DEVELOPMENT SYSTEM WITH VALIDATION")
print("=" * 80)

# Add Python path
project_root = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
python_dir = os.path.join(project_root, "Content", "Python")
if python_dir not in sys.path:
    sys.path.append(python_dir)

# Check if we should run baseline generation
# This is controlled by a command-line flag or environment variable
run_baseline = any('ExecutePythonScript' in arg for arg in sys.argv) or os.environ.get('RUN_BASELINE_GENERATION') == '1'

if run_baseline:
    print("Running ENHANCED autonomous content creation...")
    print("This will create: Missions, Stations, VFX, UI Widgets, and populate maps")
    try:
        import autonomous_baseline_generator
        results = autonomous_baseline_generator.generate_complete_baseline_with_content()
        print("AUTONOMOUS CONTENT CREATION COMPLETED")
        print(f"Results Summary:")
        print(f"  Missions: {len(results.get('missions', []))}")
        print(f"  VFX Systems: {len(results.get('vfx_systems', []))}")
        print(f"  UI Widgets: {len(results.get('ui_widgets', []))}")
        print(f"  Errors: {len(results.get('errors', []))}")
    except Exception as e:
        import traceback
        print(f"AUTONOMOUS CONTENT CREATION ERROR: {e}")
        print(traceback.format_exc())
else:
    print("Skipping autonomous content creation (not requested)")

# ALWAYS check and add PlayerStart actors if missing (CRITICAL FOR PLAYABILITY)
print("\nChecking for PlayerStart actors in maps...")
try:
    import populate_maps

    # Check if we have a world (only works when editor is fully loaded)
    import unreal
    world = unreal.EditorLevelLibrary.get_editor_world()
    if world:
        # Get current level name
        current_level = world.get_name()
        print(f"Current level: {current_level}")

        # Check for PlayerStart in current level
        player_starts = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.PlayerStart)

        if len(player_starts) == 0:
            print(f"No PlayerStart found in {current_level}, adding one now...")

            # Add PlayerStart based on the level
            if "FlightTest" in current_level:
                populate_maps.add_player_start(world, location=unreal.Vector(-500, 0, 200), rotation=unreal.Rotator(0, 0, 0))
                unreal.EditorLevelLibrary.save_current_level()
                print("PlayerStart added to FlightTest and saved!")
            elif "SolarSystem" in current_level:
                populate_maps.add_player_start(world, location=unreal.Vector(200000, 50000, 10000), rotation=unreal.Rotator(0, -90, 0))
                unreal.EditorLevelLibrary.save_current_level()
                print("PlayerStart added to SolarSystem and saved!")
        else:
            print(f"PlayerStart already exists in {current_level} ({len(player_starts)} found)")
    else:
        print("World not yet loaded, will add PlayerStart when level opens")

except Exception as e:
    print(f"Could not check/add PlayerStart: {e}")

print("=" * 80)
