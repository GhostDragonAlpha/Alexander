"""
Test Mission Systems - Verify First Mission Implementation
Tests all C++ systems and reports status

This script verifies that all First Mission systems are properly configured.
Run this AFTER create_mission_level.py and compilation.

Usage:
    python remote_client.py --file "test_mission_systems.py"
"""

import unreal

def log_info(message):
    """Log information message"""
    unreal.log(f"[MISSION TEST] {message}")

def log_success(message):
    """Log success message"""
    unreal.log(f"[MISSION TEST] ✓ {message}")

def log_warning(message):
    """Log warning message"""
    unreal.log_warning(f"[MISSION TEST] ⚠ {message}")

def log_error(message):
    """Log error message"""
    unreal.log_error(f"[MISSION TEST] ✗ {message}")

def test_cpp_classes():
    """Test that all C++ classes are available"""
    log_info("Testing C++ Classes...")
    log_info("-" * 60)
    
    all_passed = True
    
    classes_to_test = [
        ('FirstMissionGameMode', '/Script/Alexander.FirstMissionGameMode'),
        ('ShipEntryComponent', '/Script/Alexander.ShipEntryComponent'),
        ('SurfacePlayerCharacter', '/Script/Alexander.SurfacePlayerCharacter'),
        ('LandingGuidanceSystem', '/Script/Alexander.LandingGuidanceSystem'),
        ('SurfaceExplorationComponent', '/Script/Alexander.SurfaceExplorationComponent'),
    ]
    
    for class_name, class_path in classes_to_test:
        try:
            # Check if class exists
            if unreal.EditorAssetLibrary.does_asset_exist(class_path):
                log_success(f"{class_name} found")
            else:
                log_error(f"{class_name} NOT found at {class_path}")
                all_passed = False
        except Exception as e:
            log_error(f"Error checking {class_name}: {e}")
            all_passed = False
    
    log_info("-" * 60)
    return all_passed

def test_level_exists():
    """Test that MissionLevel exists"""
    log_info("Testing Level...")
    log_info("-" * 60)
    
    level_path = '/Game/Levels/MissionLevel'
    
    if unreal.EditorAssetLibrary.does_asset_exist(level_path):
        log_success("MissionLevel exists")
        
        # Try to load it
        try:
            unreal.EditorLevelLibrary.load_level(level_path)
            log_success("MissionLevel loaded successfully")
            return True
        except Exception as e:
            log_error(f"Failed to load MissionLevel: {e}")
            return False
    else:
        log_error("MissionLevel not found")
        log_info("Run: python remote_client.py --file \"create_mission_level.py\"")
        return False

def test_level_actors():
    """Test that required actors exist in level"""
    log_info("Testing Level Actors...")
    log_info("-" * 60)
    
    all_passed = True
    
    try:
        # Get all actors in level
        all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
        actor_labels = [actor.get_actor_label() for actor in all_actors]
        
        log_info(f"Found {len(all_actors)} actors in level")
        
        # Check for required actors
        required_actors = [
            ('PlayerStart', 'Player spawn point'),
            ('Sun', 'Directional light'),
        ]
        
        for actor_name, description in required_actors:
            if actor_name in actor_labels:
                log_success(f"{actor_name} found ({description})")
            else:
                log_warning(f"{actor_name} not found - {description} missing")
                all_passed = False
        
        # Check for optional actors
        optional_actors = [
            ('SkyAtmosphere', 'Atmospheric effects'),
            ('PostProcessVolume', 'Post processing'),
            ('Planet_Placeholder', 'Planet (placeholder)'),
            ('Moon_Placeholder', 'Moon (placeholder)'),
            ('Spaceship_Placeholder', 'Spaceship (placeholder)'),
            ('LandingZone_Planet', 'Planet landing zone'),
            ('LandingZone_Moon', 'Moon landing zone'),
        ]
        
        for actor_name, description in optional_actors:
            if actor_name in actor_labels:
                log_success(f"{actor_name} found ({description})")
            else:
                log_info(f"{actor_name} not found - {description}")
        
        # List all actors for debugging
        log_info("")
        log_info("All actors in level:")
        for label in sorted(actor_labels):
            log_info(f"  - {label}")
        
    except Exception as e:
        log_error(f"Failed to get level actors: {e}")
        all_passed = False
    
    log_info("-" * 60)
    return all_passed

def test_game_mode_configuration():
    """Test game mode settings"""
    log_info("Testing Game Mode Configuration...")
    log_info("-" * 60)
    
    try:
        world = unreal.EditorLevelLibrary.get_editor_world()
        if world:
            world_settings = world.get_world_settings()
            if world_settings:
                game_mode = world_settings.get_editor_property('default_game_mode')
                if game_mode:
                    log_success(f"Game Mode set: {game_mode.get_name()}")
                    
                    # Check if it's FirstMissionGameMode
                    if 'FirstMission' in game_mode.get_name():
                        log_success("Correct game mode (FirstMissionGameMode)")
                        return True
                    else:
                        log_warning("Game mode is not FirstMissionGameMode")
                        log_info("Manually set in Project Settings > Maps & Modes")
                        return False
                else:
                    log_warning("No default game mode set")
                    log_info("Set in Project Settings > Maps & Modes > Default GameMode")
                    return False
            else:
                log_error("Could not get world settings")
                return False
        else:
            log_error("Could not get editor world")
            return False
    except Exception as e:
        log_error(f"Failed to check game mode: {e}")
        return False

def test_input_actions():
    """Test if Enhanced Input Actions exist"""
    log_info("Testing Input Actions...")
    log_info("-" * 60)
    
    all_passed = True
    
    input_actions = [
        ('/Game/Input/Actions/IA_Move', 'Move action'),
        ('/Game/Input/Actions/IA_Look', 'Look action'),
        ('/Game/Input/Actions/IA_Jump', 'Jump action'),
        ('/Game/Input/Actions/IA_Sprint', 'Sprint action'),
        ('/Game/Input/Actions/IA_Crouch', 'Crouch action'),
        ('/Game/Input/Actions/IA_Interact', 'Interact action'),
        ('/Game/Input/Actions/IA_Throttle', 'Throttle action'),
        ('/Game/Input/Actions/IA_Pitch', 'Pitch action'),
        ('/Game/Input/Actions/IA_Yaw', 'Yaw action'),
        ('/Game/Input/Actions/IA_Roll', 'Roll action'),
        ('/Game/Input/Actions/IA_LandingGear', 'Landing gear action'),
        ('/Game/Input/Actions/IA_ExitShip', 'Exit ship action'),
    ]
    
    for action_path, description in input_actions:
        if unreal.EditorAssetLibrary.does_asset_exist(action_path):
            log_success(f"{description} exists")
        else:
            log_warning(f"{description} not found at {action_path}")
            all_passed = False
    
    if not all_passed:
        log_info("")
        log_info("Create Input Actions manually:")
        log_info("  Content Browser > Right-click > Input > Input Action")
        log_info("  See UNREAL_EDITOR_SETUP_GUIDE.md Phase 3")
    
    log_info("-" * 60)
    return all_passed

def test_blueprints():
    """Test if required Blueprints exist"""
    log_info("Testing Blueprint Classes...")
    log_info("-" * 60)
    
    all_passed = True
    
    blueprints = [
        ('/Game/FirstMission/Blueprints/BP_FirstMissionGameMode', 'Mission Game Mode'),
        ('/Game/FirstMission/Blueprints/BP_SurfacePlayer', 'Surface Player Character'),
    ]
    
    for bp_path, description in blueprints:
        if unreal.EditorAssetLibrary.does_asset_exist(bp_path):
            log_success(f"{description} exists")
        else:
            log_warning(f"{description} not found at {bp_path}")
            all_passed = False
    
    if not all_passed:
        log_info("")
        log_info("Create Blueprints manually:")
        log_info("  Content Browser > Right-click > Blueprint Class")
        log_info("  Parent Class: Search for C++ class name")
        log_info("  See UNREAL_EDITOR_SETUP_GUIDE.md Phase 4")
    
    log_info("-" * 60)
    return all_passed

def test_widgets():
    """Test if UMG widgets exist"""
    log_info("Testing UMG Widgets...")
    log_info("-" * 60)
    
    all_passed = True
    
    widgets = [
        ('/Game/FirstMission/Widgets/WBP_MissionHUD', 'Mission HUD'),
        ('/Game/FirstMission/Widgets/WBP_MainMenu', 'Main Menu'),
    ]
    
    for widget_path, description in widgets:
        if unreal.EditorAssetLibrary.does_asset_exist(widget_path):
            log_success(f"{description} exists")
        else:
            log_warning(f"{description} not found at {widget_path}")
            all_passed = False
    
    if not all_passed:
        log_info("")
        log_info("Create UMG Widgets manually:")
        log_info("  Content Browser > Right-click > User Interface > Widget Blueprint")
        log_info("  See UNREAL_EDITOR_SETUP_GUIDE.md Phase 8")
    
    log_info("-" * 60)
    return all_passed

def print_summary(results):
    """Print test summary"""
    log_info("")
    log_info("=" * 60)
    log_info("TEST SUMMARY")
    log_info("=" * 60)
    
    total_tests = len(results)
    passed_tests = sum(1 for result in results.values() if result)
    
    for test_name, result in results.items():
        status = "✓ PASS" if result else "✗ FAIL"
        log_info(f"{status}: {test_name}")
    
    log_info("")
    log_info(f"Total: {passed_tests}/{total_tests} tests passed")
    
    if passed_tests == total_tests:
        log_info("")
        log_success("ALL TESTS PASSED! 🎉")
        log_info("System is ready for mission testing!")
        log_info("")
        log_info("Next Steps:")
        log_info("1. Press Alt+P to Play In Editor")
        log_info("2. Test character movement")
        log_info("3. Test ship entry/exit")
        log_info("4. Test flight to moon")
    else:
        log_info("")
        log_warning(f"{total_tests - passed_tests} test(s) failed")
        log_info("")
        log_info("Fix Issues:")
        log_info("1. Review failed tests above")
        log_info("2. Follow manual setup steps in UNREAL_EDITOR_SETUP_GUIDE.md")
        log_info("3. Rerun this test script")
    
    log_info("=" * 60)

def run_quick_diagnostics():
    """Run quick diagnostics on current setup"""
    log_info("")
    log_info("Running Quick Diagnostics...")
    log_info("-" * 60)
    
    try:
        # Check Python plugin
        log_info("Python Editor Script Plugin: Enabled")
        
        # Check editor version
        log_info(f"Unreal Engine Version: {unreal.SystemLibrary.get_engine_version()}")
        
        # Check project
        log_info(f"Project: {unreal.SystemLibrary.get_project_directory()}")
        
        # Check current level
        current_level = unreal.EditorLevelLibrary.get_editor_world()
        if current_level:
            level_name = current_level.get_name()
            log_info(f"Current Level: {level_name}")
        
    except Exception as e:
        log_warning(f"Diagnostics error: {e}")
    
    log_info("-" * 60)

def main():
    """Main test function"""
    log_info("=" * 60)
    log_info("FIRST MISSION SYSTEM TESTS")
    log_info("=" * 60)
    log_info("")
    
    # Run diagnostics
    run_quick_diagnostics()
    
    # Run tests
    results = {}
    
    log_info("")
    results['C++ Classes'] = test_cpp_classes()
    
    log_info("")
    results['Mission Level'] = test_level_exists()
    
    log_info("")
    results['Level Actors'] = test_level_actors()
    
    log_info("")
    results['Game Mode Config'] = test_game_mode_configuration()
    
    log_info("")
    results['Input Actions'] = test_input_actions()
    
    log_info("")
    results['Blueprint Classes'] = test_blueprints()
    
    log_info("")
    results['UMG Widgets'] = test_widgets()
    
    # Print summary
    print_summary(results)
    
    return all(results.values())

# Execute main function
if __name__ == '__main__':
    success = main()
    if success:
        unreal.log("All tests passed!")
    else:
        unreal.log_warning("Some tests failed - review output above")
