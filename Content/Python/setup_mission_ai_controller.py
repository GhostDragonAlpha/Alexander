"""
Setup Mission AI Controller
Creates Blueprint wrapper and sets up automated mission playthrough
"""

import unreal

def setup_mission_ai_controller():
    """Create Blueprint wrapper for MissionAIController and set up level"""
    
    print("=" * 80)
    print("MISSION AI CONTROLLER SETUP")
    print("=" * 80)
    
    # Get necessary subsystems
    editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    
    # Step 1: Compile C++ code
    print("\n[1/5] Checking C++ compilation...")
    print("IMPORTANT: You must compile the C++ project in Unreal Editor first!")
    print("  1. Close Unreal Editor if open")
    print("  2. Right-click Alexander.uproject -> Generate Visual Studio project files")
    print("  3. Open Alexander.sln in Visual Studio")
    print("  4. Build Solution (Ctrl+Shift+B)")
    print("  5. Launch Unreal Editor")
    print("  6. Run this script again")
    
    # Check if MissionAIController class exists
    ai_controller_class = unreal.find_object(None, "/Script/Alexander.MissionAIController")
    if not ai_controller_class:
        print("\n❌ MissionAIController C++ class not found!")
        print("Please compile the C++ project first (see instructions above)")
        return False
    
    print("✓ MissionAIController C++ class found")
    
    # Step 2: Create Blueprint wrapper
    print("\n[2/5] Creating Blueprint wrapper...")
    
    bp_path = "/Game/AI/BP_MissionAIController"
    bp_package_path = "/Game/AI"
    
    # Check if Blueprint already exists
    existing_bp = unreal.load_asset(bp_path)
    if existing_bp:
        print(f"✓ Blueprint already exists: {bp_path}")
    else:
        # Create Blueprint
        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", ai_controller_class)
        
        # Create the asset
        new_bp = asset_tools.create_asset(
            "BP_MissionAIController",
            bp_package_path,
            unreal.Blueprint,
            factory
        )
        
        if new_bp:
            print(f"✓ Created Blueprint: {bp_path}")
            # Save the asset
            unreal.EditorAssetLibrary.save_loaded_asset(new_bp)
        else:
            print(f"❌ Failed to create Blueprint")
            return False
    
    # Step 3: Load or create mission level
    print("\n[3/5] Setting up mission level...")
    
    mission_level_path = "/Game/Levels/FirstMission"
    
    # Check if level exists
    if unreal.EditorAssetLibrary.does_asset_exist(mission_level_path):
        print(f"✓ Mission level exists: {mission_level_path}")
        
        # Load the level
        if editor_subsystem.load_level(mission_level_path + ".umap"):
            print("✓ Loaded mission level")
        else:
            print("❌ Failed to load mission level")
            return False
    else:
        print(f"⚠ Mission level not found: {mission_level_path}")
        print("You need to create the mission level first using create_mission_level.py")
        return False
    
    # Step 4: Add AI controller to level
    print("\n[4/5] Adding AI controller to level...")
    
    # Get current level
    world = unreal.EditorLevelLibraryHelpers.get_editor_world()
    if not world:
        print("❌ Could not get editor world")
        return False
    
    # Check if AI controller already exists in level
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    ai_controller_actors = [actor for actor in all_actors 
                           if actor.get_class().get_name() == "BP_MissionAIController_C"]
    
    if ai_controller_actors:
        print(f"✓ AI controller already in level: {len(ai_controller_actors)} found")
        ai_controller = ai_controller_actors[0]
    else:
        # Spawn AI controller
        bp_class = unreal.load_object(None, bp_path + "_C")
        if not bp_class:
            print(f"❌ Could not load Blueprint class: {bp_path}")
            return False
        
        spawn_location = unreal.Vector(0, 0, 200)
        spawn_rotation = unreal.Rotator(0, 0, 0)
        
        ai_controller = unreal.EditorLevelLibrary.spawn_actor_from_class(
            bp_class,
            spawn_location,
            spawn_rotation
        )
        
        if ai_controller:
            print(f"✓ Spawned AI controller in level")
        else:
            print(f"❌ Failed to spawn AI controller")
            return False
    
    # Step 5: Configure AI controller
    print("\n[5/5] Configuring AI controller...")
    
    # Set properties
    ai_controller.set_editor_property("b_capture_screenshots", True)
    ai_controller.set_editor_property("b_debug_mode", True)
    ai_controller.set_editor_property("speed_multiplier", 1.0)
    ai_controller.set_editor_property("stage_delay_time", 2.0)
    
    print("✓ Configured AI controller properties:")
    print("  - Capture Screenshots: Enabled")
    print("  - Debug Mode: Enabled")
    print("  - Speed Multiplier: 1.0x")
    print("  - Stage Delay: 2.0 seconds")
    
    # Save level
    if unreal.EditorLevelLibrary.save_current_level():
        print("\n✓ Saved level")
    else:
        print("\n⚠ Could not save level")
    
    print("\n" + "=" * 80)
    print("SETUP COMPLETE!")
    print("=" * 80)
    print("\nNext steps:")
    print("1. Find the Surface Player Character in the level")
    print("2. In its Details panel, change AI Controller Class to BP_MissionAIController")
    print("3. Play the level")
    print("4. Open the console (~) and type: py exec(\"\"\"")
    print("   import unreal")
    print("   world = unreal.EditorLevelLibraryHelpers.get_editor_world()")
    print("   all_actors = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.MissionAIController)")
    print("   if all_actors:")
    print("       ai = all_actors[0]")
    print("       ai.start_automated_mission()")
    print("   \"\"\")")
    print("\nOr create a Level Blueprint event to start the mission automatically on BeginPlay")
    print("=" * 80)
    
    return True

def create_start_mission_console_command():
    """Create a simple console command file to start the mission"""
    
    commands = """# Console Commands for Mission AI Controller
# Copy and paste these into the Unreal Editor console (~)

# Start automated mission
py import unreal; actors = unreal.GameplayStatics.get_all_actors_of_class(unreal.EditorLevelLibraryHelpers.get_editor_world(), unreal.MissionAIController); actors[0].start_automated_mission() if actors else None

# Pause mission
py import unreal; actors = unreal.GameplayStatics.get_all_actors_of_class(unreal.EditorLevelLibraryHelpers.get_editor_world(), unreal.MissionAIController); actors[0].set_automation_paused(True) if actors else None

# Resume mission
py import unreal; actors = unreal.GameplayStatics.get_all_actors_of_class(unreal.EditorLevelLibraryHelpers.get_editor_world(), unreal.MissionAIController); actors[0].set_automation_paused(False) if actors else None

# Stop mission
py import unreal; actors = unreal.GameplayStatics.get_all_actors_of_class(unreal.EditorLevelLibraryHelpers.get_editor_world(), unreal.MissionAIController); actors[0].stop_automated_mission() if actors else None
"""
    
    # Write to file
    import os
    output_path = os.path.join(os.path.dirname(__file__), "mission_ai_console_commands.txt")
    with open(output_path, 'w') as f:
        f.write(commands)
    
    print(f"\n✓ Created console commands file: {output_path}")

if __name__ == "__main__":
    success = setup_mission_ai_controller()
    
    if success:
        create_start_mission_console_command()
        print("\n✅ Setup completed successfully!")
    else:
        print("\n❌ Setup failed. Please follow the instructions above.")
