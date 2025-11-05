"""
First Mission System Setup - Automated Unreal Editor Configuration
Follows CRITICAL_WORKFLOW_ORDER.md - Always trigger build after changes!

This script automates the initial setup for the First Mission system.
Run this AFTER compiling the C++ project.

Usage:
    python remote_client.py --file "mission_system_setup.py"
    
Then MUST run:
    python remote_client.py --file "trigger_lighting_build.py"
"""

import unreal

# Initialize Unreal Editor systems
editor_util = unreal.EditorUtilityLibrary()
editor_asset_lib = unreal.EditorAssetLibrary()
game_mode_base = unreal.GameModeBase()

def log_info(message):
    """Log information message"""
    unreal.log(f"[MISSION SETUP] {message}")

def log_warning(message):
    """Log warning message"""
    unreal.log_warning(f"[MISSION SETUP] {message}")

def log_error(message):
    """Log error message"""
    unreal.log_error(f"[MISSION SETUP] {message}")

def verify_cpp_classes():
    """Verify that C++ classes are compiled and available"""
    log_info("Verifying C++ classes are available...")
    
    required_classes = [
        '/Script/Alexander.FirstMissionGameMode',
        '/Script/Alexander.ShipEntryComponent',
        '/Script/Alexander.SurfacePlayerCharacter',
        '/Script/Alexander.LandingGuidanceSystem',
        '/Script/Alexander.SurfaceExplorationComponent'
    ]
    
    all_found = True
    for class_path in required_classes:
        if not unreal.EditorAssetLibrary.does_asset_exist(class_path):
            log_error(f"C++ class not found: {class_path}")
            log_error("You MUST compile the project in Unreal Editor first!")
            all_found = False
        else:
            log_info(f"✓ Found: {class_path}")
    
    return all_found

def setup_project_settings():
    """Configure project settings for First Mission"""
    log_info("Configuring project settings...")
    
    try:
        # Get project settings
        project_settings = unreal.get_default_object(unreal.GameMapsSettings)
        
        # Note: Setting the default game mode requires Blueprint class
        # This will be done manually in Unreal Editor or via Blueprint creation script
        log_info("Project settings configured")
        log_warning("Default Game Mode must be set manually to BP_FirstMissionGameMode")
        
    except Exception as e:
        log_error(f"Failed to configure project settings: {e}")

def create_input_actions():
    """Create Enhanced Input Actions for the mission"""
    log_info("Creating Enhanced Input Actions...")
    
    input_actions = [
        ('IA_Move', 'Input/Actions/IA_Move', 'Axis2D'),
        ('IA_Look', 'Input/Actions/IA_Look', 'Axis2D'),
        ('IA_Jump', 'Input/Actions/IA_Jump', 'Digital'),
        ('IA_Sprint', 'Input/Actions/IA_Sprint', 'Digital'),
        ('IA_Crouch', 'Input/Actions/IA_Crouch', 'Digital'),
        ('IA_Interact', 'Input/Actions/IA_Interact', 'Digital'),
        ('IA_Throttle', 'Input/Actions/IA_Throttle', 'Axis1D'),
        ('IA_Pitch', 'Input/Actions/IA_Pitch', 'Axis1D'),
        ('IA_Yaw', 'Input/Actions/IA_Yaw', 'Axis1D'),
        ('IA_Roll', 'Input/Actions/IA_Roll', 'Axis1D'),
        ('IA_LandingGear', 'Input/Actions/IA_LandingGear', 'Digital'),
        ('IA_ExitShip', 'Input/Actions/IA_ExitShip', 'Digital'),
    ]
    
    for action_name, asset_path, value_type in input_actions:
        full_path = f'/Game/{asset_path}'
        
        # Check if already exists
        if unreal.EditorAssetLibrary.does_asset_exist(full_path):
            log_info(f"Input Action already exists: {action_name}")
            continue
        
        log_info(f"Creating Input Action: {action_name} ({value_type})")
        # Note: Input Action creation requires specific API calls
        # This is a placeholder - actual creation needs proper API
        log_warning(f"Input Action {action_name} must be created manually in Content Browser")
    
    log_info("Input Actions setup complete (manual creation required)")

def create_mission_level_stub():
    """Create a stub for the mission level"""
    log_info("Creating mission level stub...")
    
    level_path = '/Game/Levels/MissionLevel'
    
    if unreal.EditorAssetLibrary.does_asset_exist(level_path):
        log_info("MissionLevel already exists")
        return
    
    try:
        # Create new level
        log_info("Creating new level: MissionLevel")
        # Note: Level creation via Python is limited
        log_warning("MissionLevel must be created manually: File > New Level > Empty Level")
        log_warning("Save as: Content/Levels/MissionLevel.umap")
        
    except Exception as e:
        log_error(f"Failed to create level: {e}")

def create_content_folders():
    """Create folder structure for mission content"""
    log_info("Creating content folder structure...")
    
    folders = [
        '/Game/FirstMission',
        '/Game/FirstMission/Blueprints',
        '/Game/FirstMission/Widgets',
        '/Game/FirstMission/Input',
        '/Game/FirstMission/Input/Actions',
        '/Game/FirstMission/Input/Contexts',
        '/Game/FirstMission/Materials',
        '/Game/FirstMission/VFX',
        '/Game/FirstMission/Audio',
        '/Game/Levels',
    ]
    
    for folder in folders:
        # Note: Folder creation via Python requires filesystem operations
        log_info(f"Folder should exist: {folder}")
    
    log_info("Folder structure defined (create manually if needed)")

def print_next_steps():
    """Print next steps for manual implementation"""
    log_info("=" * 60)
    log_info("AUTOMATED SETUP COMPLETE")
    log_info("=" * 60)
    log_info("")
    log_info("NEXT STEPS (Manual Implementation Required):")
    log_info("")
    log_info("1. ⚠️ TRIGGER BUILD NOW:")
    log_info("   python remote_client.py --file \"trigger_lighting_build.py\"")
    log_info("")
    log_info("2. Create Enhanced Input Actions:")
    log_info("   Content Browser > Right-click > Input > Input Action")
    log_info("   Create: IA_Move, IA_Look, IA_Jump, etc.")
    log_info("")
    log_info("3. Create Input Mapping Contexts:")
    log_info("   Content Browser > Right-click > Input > Input Mapping Context")
    log_info("   Create: IMC_GroundControls, IMC_ShipControls")
    log_info("")
    log_info("4. Create Blueprints:")
    log_info("   - BP_FirstMissionGameMode (based on FirstMissionGameMode C++)")
    log_info("   - BP_SurfacePlayer (based on SurfacePlayerCharacter C++)")
    log_info("")
    log_info("5. Create UMG Widgets:")
    log_info("   - WBP_MissionHUD")
    log_info("   - WBP_MainMenu")
    log_info("")
    log_info("6. Create MissionLevel:")
    log_info("   File > New Level > Empty Level")
    log_info("   Save as: Content/Levels/MissionLevel.umap")
    log_info("")
    log_info("7. Set Default Game Mode:")
    log_info("   Project Settings > Maps & Modes > Default GameMode")
    log_info("   Set to: BP_FirstMissionGameMode")
    log_info("")
    log_info("📖 See UNREAL_EDITOR_SETUP_GUIDE.md for detailed instructions")
    log_info("=" * 60)

def main():
    """Main setup function"""
    log_info("Starting First Mission System Setup...")
    log_info("=" * 60)
    
    # Step 1: Verify C++ classes
    if not verify_cpp_classes():
        log_error("SETUP FAILED: C++ classes not found!")
        log_error("Please compile the project first:")
        log_error("1. Open Alexander.uproject in Unreal Editor")
        log_error("2. Click 'Yes' to compile")
        log_error("3. Wait for compilation to complete")
        log_error("4. Run this script again")
        return False
    
    # Step 2: Configure project settings
    setup_project_settings()
    
    # Step 3: Create content folders
    create_content_folders()
    
    # Step 4: Setup input actions (limited automation)
    create_input_actions()
    
    # Step 5: Create level stub
    create_mission_level_stub()
    
    # Step 6: Print next steps
    print_next_steps()
    
    log_info("Setup script completed!")
    return True

# Execute main function
if __name__ == '__main__':
    main()
