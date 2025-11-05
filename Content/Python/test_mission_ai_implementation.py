"""
Test Mission AI Controller Implementation
Validates compilation, setup, and provides testing instructions
"""

import unreal
import os

def test_mission_ai_implementation():
    """
    Comprehensive test suite for Mission AI Controller
    Run this after compiling C++ code
    """
    
    print("=" * 80)
    print("MISSION AI CONTROLLER - TEST SUITE")
    print("=" * 80)
    print()
    
    results = {
        "total_tests": 0,
        "passed": 0,
        "failed": 0,
        "warnings": 0,
        "tests": []
    }
    
    # Test 1: Check C++ class compilation
    print("[TEST 1/10] Checking C++ Class Compilation...")
    results["total_tests"] += 1
    
    ai_controller_class = unreal.find_object(None, "/Script/Alexander.MissionAIController")
    if ai_controller_class:
        print("✅ PASS: MissionAIController C++ class found")
        results["passed"] += 1
        results["tests"].append(("C++ Compilation", "PASS", "Class compiled successfully"))
    else:
        print("❌ FAIL: MissionAIController C++ class not found")
        print("   Action: Compile the C++ project first")
        results["failed"] += 1
        results["tests"].append(("C++ Compilation", "FAIL", "Class not found - needs compilation"))
        return results  # Cannot continue without compiled class
    
    # Test 2: Check Blueprint wrapper
    print("\n[TEST 2/10] Checking Blueprint Wrapper...")
    results["total_tests"] += 1
    
    bp_path = "/Game/AI/BP_MissionAIController"
    bp_asset = unreal.load_asset(bp_path)
    if bp_asset:
        print("✅ PASS: Blueprint wrapper exists")
        results["passed"] += 1
        results["tests"].append(("Blueprint Wrapper", "PASS", "BP_MissionAIController found"))
    else:
        print("⚠ WARNING: Blueprint wrapper not found")
        print("   Action: Run setup_mission_ai_controller.py")
        results["warnings"] += 1
        results["tests"].append(("Blueprint Wrapper", "WARNING", "Blueprint not created yet"))
    
    # Test 3: Check FirstMissionGameMode
    print("\n[TEST 3/10] Checking FirstMissionGameMode...")
    results["total_tests"] += 1
    
    gamemode_class = unreal.find_object(None, "/Script/Alexander.FirstMissionGameMode")
    if gamemode_class:
        print("✅ PASS: FirstMissionGameMode class found")
        results["passed"] += 1
        results["tests"].append(("GameMode Class", "PASS", "FirstMissionGameMode compiled"))
    else:
        print("❌ FAIL: FirstMissionGameMode not found")
        results["failed"] += 1
        results["tests"].append(("GameMode Class", "FAIL", "Required dependency missing"))
    
    # Test 4: Check SurfacePlayerCharacter
    print("\n[TEST 4/10] Checking SurfacePlayerCharacter...")
    results["total_tests"] += 1
    
    character_class = unreal.find_object(None, "/Script/Alexander.SurfacePlayerCharacter")
    if character_class:
        print("✅ PASS: SurfacePlayerCharacter class found")
        results["passed"] += 1
        results["tests"].append(("Character Class", "PASS", "SurfacePlayerCharacter compiled"))
    else:
        print("❌ FAIL: SurfacePlayerCharacter not found")
        results["failed"] += 1
        results["tests"].append(("Character Class", "FAIL", "Required dependency missing"))
    
    # Test 5: Check Spaceship class
    print("\n[TEST 5/10] Checking Spaceship Class...")
    results["total_tests"] += 1
    
    spaceship_class = unreal.find_object(None, "/Script/Alexander.Spaceship")
    if spaceship_class:
        print("✅ PASS: Spaceship class found")
        results["passed"] += 1
        results["tests"].append(("Spaceship Class", "PASS", "Spaceship compiled"))
    else:
        print("⚠ WARNING: Spaceship class not found")
        print("   Note: May need to be created or may use different name")
        results["warnings"] += 1
        results["tests"].append(("Spaceship Class", "WARNING", "Spaceship class not found"))
    
    # Test 6: Check ShipEntryComponent
    print("\n[TEST 6/10] Checking ShipEntryComponent...")
    results["total_tests"] += 1
    
    entry_comp_class = unreal.find_object(None, "/Script/Alexander.ShipEntryComponent")
    if entry_comp_class:
        print("✅ PASS: ShipEntryComponent class found")
        results["passed"] += 1
        results["tests"].append(("ShipEntry Component", "PASS", "Component compiled"))
    else:
        print("❌ FAIL: ShipEntryComponent not found")
        results["failed"] += 1
        results["tests"].append(("ShipEntry Component", "FAIL", "Required component missing"))
    
    # Test 7: Check mission level
    print("\n[TEST 7/10] Checking Mission Level...")
    results["total_tests"] += 1
    
    level_path = "/Game/Levels/FirstMission"
    if unreal.EditorAssetLibrary.does_asset_exist(level_path):
        print("✅ PASS: Mission level exists")
        results["passed"] += 1
        results["tests"].append(("Mission Level", "PASS", "FirstMission level found"))
    else:
        print("⚠ WARNING: Mission level not found")
        print("   Action: Run create_mission_level.py")
        results["warnings"] += 1
        results["tests"].append(("Mission Level", "WARNING", "Level needs to be created"))
    
    # Test 8: Check LandingGuidanceSystem
    print("\n[TEST 8/10] Checking LandingGuidanceSystem...")
    results["total_tests"] += 1
    
    landing_class = unreal.find_object(None, "/Script/Alexander.LandingGuidanceSystem")
    if landing_class:
        print("✅ PASS: LandingGuidanceSystem class found")
        results["passed"] += 1
        results["tests"].append(("Landing System", "PASS", "LandingGuidanceSystem compiled"))
    else:
        print("⚠ WARNING: LandingGuidanceSystem not found")
        results["warnings"] += 1
        results["tests"].append(("Landing System", "WARNING", "Landing guidance missing"))
    
    # Test 9: Check Planet class
    print("\n[TEST 9/10] Checking Planet Class...")
    results["total_tests"] += 1
    
    planet_class = unreal.find_object(None, "/Script/Alexander.Planet")
    if planet_class:
        print("✅ PASS: Planet class found")
        results["passed"] += 1
        results["tests"].append(("Planet Class", "PASS", "Planet compiled"))
    else:
        print("⚠ WARNING: Planet class not found")
        results["warnings"] += 1
        results["tests"].append(("Planet Class", "WARNING", "Planet class missing"))
    
    # Test 10: Check AI controller properties
    print("\n[TEST 10/10] Validating AI Controller Properties...")
    results["total_tests"] += 1
    
    try:
        # Try to get class default object
        cdo = unreal.get_default_object(ai_controller_class)
        
        # Check if expected properties exist
        expected_props = ['b_capture_screenshots', 'b_debug_mode', 'speed_multiplier', 'stage_delay_time']
        props_found = []
        
        for prop in expected_props:
            if hasattr(cdo, prop):
                props_found.append(prop)
        
        if len(props_found) >= 3:
            print(f"✅ PASS: AI Controller has {len(props_found)}/{len(expected_props)} expected properties")
            results["passed"] += 1
            results["tests"].append(("AI Properties", "PASS", f"{len(props_found)} properties found"))
        else:
            print(f"⚠ WARNING: Only {len(props_found)}/{len(expected_props)} properties found")
            results["warnings"] += 1
            results["tests"].append(("AI Properties", "WARNING", "Some properties missing"))
    except Exception as e:
        print(f"⚠ WARNING: Could not validate properties: {str(e)}")
        results["warnings"] += 1
        results["tests"].append(("AI Properties", "WARNING", "Could not validate"))
    
    return results

def print_test_summary(results):
    """Print test results summary"""
    
    print("\n" + "=" * 80)
    print("TEST SUMMARY")
    print("=" * 80)
    print(f"Total Tests: {results['total_tests']}")
    print(f"✅ Passed:   {results['passed']}")
    print(f"❌ Failed:   {results['failed']}")
    print(f"⚠  Warnings: {results['warnings']}")
    print()
    
    # Calculate percentage
    if results['total_tests'] > 0:
        pass_rate = (results['passed'] / results['total_tests']) * 100
        print(f"Pass Rate: {pass_rate:.1f}%")
    
    print("\n" + "=" * 80)
    print("DETAILED RESULTS")
    print("=" * 80)
    
    for test_name, status, message in results['tests']:
        status_icon = "✅" if status == "PASS" else "❌" if status == "FAIL" else "⚠"
        print(f"{status_icon} {test_name}: {message}")
    
    print("\n" + "=" * 80)
    
    # Determine readiness
    if results['failed'] == 0:
        print("🎉 READY FOR TESTING!")
        print("=" * 80)
        print("\nNext Steps:")
        print("1. Open mission level")
        print("2. Play in editor (PIE)")
        print("3. Press ~ for console")
        print("4. Paste command:")
        print("   py import unreal; actors = unreal.GameplayStatics.get_all_actors_of_class(")
        print("      unreal.EditorLevelLibraryHelpers.get_editor_world(), unreal.MissionAIController);")
        print("      actors[0].start_automated_mission() if actors else None")
        print("\n5. Watch the AI play through the mission!")
    else:
        print("⚠ NOT READY - Please fix failed tests first")
        print("=" * 80)
        print("\nRequired Actions:")
        for test_name, status, message in results['tests']:
            if status == "FAIL":
                print(f"❌ {test_name}: {message}")
    
    print()

def generate_test_report_html(results):
    """Generate HTML test report"""
    
    html_content = f"""<!DOCTYPE html>
<html>
<head>
    <title>Mission AI Controller - Test Report</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }}
        .container {{ max-width: 1200px; margin: 0 auto; background: white; padding: 30px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }}
        h1 {{ color: #333; border-bottom: 3px solid #4CAF50; padding-bottom: 10px; }}
        .summary {{ display: flex; gap: 20px; margin: 20px 0; }}
        .stat-box {{ flex: 1; padding: 20px; border-radius: 5px; text-align: center; }}
        .stat-box.total {{ background: #2196F3; color: white; }}
        .stat-box.passed {{ background: #4CAF50; color: white; }}
        .stat-box.failed {{ background: #f44336; color: white; }}
        .stat-box.warnings {{ background: #FF9800; color: white; }}
        .stat-number {{ font-size: 48px; font-weight: bold; }}
        .stat-label {{ font-size: 14px; text-transform: uppercase; }}
        .test-list {{ margin: 30px 0; }}
        .test-item {{ padding: 15px; margin: 10px 0; border-radius: 5px; border-left: 4px solid #ddd; }}
        .test-item.pass {{ background: #E8F5E9; border-left-color: #4CAF50; }}
        .test-item.fail {{ background: #FFEBEE; border-left-color: #f44336; }}
        .test-item.warning {{ background: #FFF3E0; border-left-color: #FF9800; }}
        .test-name {{ font-weight: bold; font-size: 16px; }}
        .test-message {{ color: #666; margin-top: 5px; }}
        .timestamp {{ color: #999; font-size: 12px; margin-top: 20px; }}
    </style>
</head>
<body>
    <div class="container">
        <h1>Mission AI Controller - Test Report</h1>
        
        <div class="summary">
            <div class="stat-box total">
                <div class="stat-number">{results['total_tests']}</div>
                <div class="stat-label">Total Tests</div>
            </div>
            <div class="stat-box passed">
                <div class="stat-number">{results['passed']}</div>
                <div class="stat-label">Passed</div>
            </div>
            <div class="stat-box failed">
                <div class="stat-number">{results['failed']}</div>
                <div class="stat-label">Failed</div>
            </div>
            <div class="stat-box warnings">
                <div class="stat-number">{results['warnings']}</div>
                <div class="stat-label">Warnings</div>
            </div>
        </div>
        
        <div class="test-list">
            <h2>Test Results</h2>
"""
    
    for test_name, status, message in results['tests']:
        status_class = status.lower()
        status_icon = "✅" if status == "PASS" else "❌" if status == "FAIL" else "⚠"
        html_content += f"""
            <div class="test-item {status_class}">
                <div class="test-name">{status_icon} {test_name}</div>
                <div class="test-message">{message}</div>
            </div>
"""
    
    import datetime
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
    html_content += f"""
        </div>
        
        <div class="timestamp">
            Report generated: {timestamp}
        </div>
    </div>
</body>
</html>
"""
    
    # Save report
    import os
    report_path = os.path.join(os.path.dirname(__file__), "..", "..", "Saved", "mission_ai_test_report.html")
    os.makedirs(os.path.dirname(report_path), exist_ok=True)
    
    with open(report_path, 'w') as f:
        f.write(html_content)
    
    print(f"\n📄 HTML report saved to: {report_path}")
    return report_path

if __name__ == "__main__":
    print("Running Mission AI Controller test suite...\n")
    
    results = test_mission_ai_implementation()
    print_test_summary(results)
    
    # Generate HTML report
    try:
        report_path = generate_test_report_html(results)
        print(f"✅ Test report generated successfully")
    except Exception as e:
        print(f"⚠ Could not generate HTML report: {str(e)}")
    
    print("\n" + "=" * 80)
    print("TEST SUITE COMPLETE")
    print("=" * 80)
