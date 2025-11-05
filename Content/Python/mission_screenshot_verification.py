"""
Mission Screenshot Verification System
Captures screenshots at key mission stages for visual verification

This script automates screenshot capture during mission testing to verify
that all stages are working correctly and providing visual feedback.

Usage:
    python remote_client.py --file "mission_screenshot_verification.py"
"""

import unreal
import time

def log_info(message):
    """Log information message"""
    unreal.log(f"[SCREENSHOT] {message}")

def log_success(message):
    """Log success message"""
    unreal.log(f"[SCREENSHOT] ✓ {message}")

def log_warning(message):
    """Log warning message"""
    unreal.log_warning(f"[SCREENSHOT] ⚠ {message}")

def log_error(message):
    """Log error message"""
    unreal.log_error(f"[SCREENSHOT] ✗ {message}")

def capture_screenshot(filename, description=""):
    """Capture a screenshot with the given filename"""
    try:
        # Use automation screenshot command
        screenshot_path = f"Saved/Screenshots/Windows/{filename}.png"
        
        # Execute screenshot command
        unreal.AutomationLibrary.take_automation_screenshot(filename)
        
        log_success(f"Screenshot captured: {filename}.png")
        if description:
            log_info(f"  Description: {description}")
        
        return True
    except Exception as e:
        log_error(f"Failed to capture screenshot '{filename}': {e}")
        return False

def setup_verification_cameras():
    """Set up camera positions for verification screenshots"""
    log_info("Setting up verification camera positions...")
    
    camera_positions = [
        {
            'name': 'PlanetOverview',
            'location': unreal.Vector(0, 0, 100000),  # High above planet
            'rotation': unreal.Rotator(-90, 0, 0),
            'description': 'Bird\'s eye view of planet and starting area'
        },
        {
            'name': 'PlayerStart',
            'location': unreal.Vector(200, -200, 150),  # Near player start
            'rotation': unreal.Rotator(-10, 45, 0),
            'description': 'Player spawn point and nearby spaceship'
        },
        {
            'name': 'SpaceshipClose',
            'location': unreal.Vector(600, 100, 120),  # Close to ship
            'rotation': unreal.Rotator(-5, -90, 0),
            'description': 'Close-up of spaceship'
        },
        {
            'name': 'MoonView',
            'location': unreal.Vector(4999500, 0, 100),  # Near moon
            'rotation': unreal.Rotator(0, 0, 0),
            'description': 'View of moon from distance'
        },
        {
            'name': 'FlightPath',
            'location': unreal.Vector(2500000, 0, 50000),  # Mid-flight
            'rotation': unreal.Rotator(-10, 0, 0),
            'description': 'Mid-journey between planet and moon'
        },
    ]
    
    log_info(f"Defined {len(camera_positions)} verification camera positions")
    return camera_positions

def capture_mission_stage_screenshots():
    """Capture screenshots for each mission stage"""
    log_info("=" * 60)
    log_info("MISSION STAGE SCREENSHOT VERIFICATION")
    log_info("=" * 60)
    log_info("")
    
    # Mission stages to capture
    stages = [
        ('00_MainMenu', 'Main menu before mission start'),
        ('01_Briefing', 'Mission briefing screen'),
        ('02_PlanetSurface', 'Player spawned on planet surface'),
        ('03_ApproachingShip', 'Player approaching spaceship'),
        ('04_ShipInteraction', 'Ship interaction prompt visible'),
        ('05_EnteringShip', 'Player entering ship animation'),
        ('06_InsideCockpit', 'Player inside cockpit view'),
        ('07_ShipControls', 'Ship control tutorial overlay'),
        ('08_TakeOff', 'Ship taking off from planet'),
        ('09_InFlight', 'Ship in flight toward moon'),
        ('10_MoonApproach', 'Approaching moon, guidance active'),
        ('11_LandingGuidance', 'Landing guidance system active'),
        ('12_MoonLanding', 'Landing on moon surface'),
        ('13_ExitingShip', 'Exiting ship on moon'),
        ('14_MoonSurface', 'Walking on moon surface'),
        ('15_MoonExploration', 'Exploring moon, objectives shown'),
        ('16_ReturnToShip', 'Returning to ship on moon'),
        ('17_ReturnFlight', 'Flying back to planet'),
        ('18_PlanetLanding', 'Landing back on planet'),
        ('19_MissionComplete', 'Mission complete screen with XP'),
    ]
    
    log_info(f"Will capture {len(stages)} mission stage screenshots")
    log_info("")
    log_info("Screenshot Stages:")
    for filename, description in stages:
        log_info(f"  - {filename}: {description}")
    
    log_info("")
    log_warning("MANUAL TESTING REQUIRED:")
    log_info("These screenshots must be captured during actual gameplay.")
    log_info("")
    log_info("Testing Instructions:")
    log_info("1. Press Alt+P to start Play In Editor")
    log_info("2. Use F9 or ` (backtick) to open console")
    log_info("3. At each stage, use command: ke * HighResShot")
    log_info("   OR use: Automation.TakeScreenshot <name>")
    log_info("4. Progress through all mission stages")
    log_info("5. Screenshots will be saved to: Saved/Screenshots/Windows/")
    log_info("")
    
    return stages

def create_automated_screenshot_sequence():
    """Create automated screenshot sequence for level verification"""
    log_info("Creating automated level verification screenshots...")
    log_info("")
    
    # Get camera positions
    camera_positions = setup_verification_cameras()
    
    # Capture screenshots from each position
    screenshots_captured = []
    
    try:
        # Get editor level library
        editor_lib = unreal.EditorLevelLibrary
        
        for i, cam_pos in enumerate(camera_positions):
            log_info(f"Capturing view {i+1}/{len(camera_positions)}: {cam_pos['name']}")
            
            # Note: Moving editor camera requires specific API calls
            # This is a placeholder for the actual implementation
            filename = f"LevelView_{cam_pos['name']}"
            
            log_info(f"  Location: {cam_pos['location']}")
            log_info(f"  Description: {cam_pos['description']}")
            
            # Capture screenshot (if possible from editor)
            if capture_screenshot(filename, cam_pos['description']):
                screenshots_captured.append(filename)
            
            log_info("")
        
        log_success(f"Captured {len(screenshots_captured)} automated screenshots")
        
    except Exception as e:
        log_error(f"Automated screenshot sequence failed: {e}")
    
    return screenshots_captured

def setup_mission_monitoring():
    """Set up mission event monitoring for automatic screenshots"""
    log_info("Setting up mission monitoring system...")
    log_info("")
    
    log_info("Mission monitoring will:")
    log_info("  - Detect mission stage changes")
    log_info("  - Automatically capture screenshots at key moments")
    log_info("  - Log mission progress")
    log_info("  - Record timing information")
    log_info("")
    
    log_warning("NOTE: Full monitoring requires Blueprint integration")
    log_info("To enable automatic screenshots during gameplay:")
    log_info("")
    log_info("1. In BP_FirstMissionGameMode Blueprint:")
    log_info("   - Bind to OnMissionStageChanged event")
    log_info("   - Call Python script or execute console command")
    log_info("   - Command: Automation.TakeScreenshot Stage_{StageName}")
    log_info("")
    log_info("2. Or use Blueprint node 'Take Screenshot'")
    log_info("   - Add after each stage transition")
    log_info("   - Name screenshots descriptively")
    log_info("")

def create_screenshot_comparison_html():
    """Create HTML file for screenshot comparison"""
    log_info("Creating screenshot comparison HTML...")
    
    html_content = """<!DOCTYPE html>
<html>
<head>
    <title>First Mission - Screenshot Verification</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 20px;
            background-color: #1a1a1a;
            color: #ffffff;
        }
        h1 {
            color: #4CAF50;
        }
        .screenshot-grid {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(400px, 1fr));
            gap: 20px;
            margin-top: 20px;
        }
        .screenshot-item {
            border: 2px solid #333;
            padding: 10px;
            background-color: #2a2a2a;
            border-radius: 8px;
        }
        .screenshot-item img {
            width: 100%;
            height: auto;
            border-radius: 4px;
        }
        .screenshot-title {
            font-weight: bold;
            margin: 10px 0 5px 0;
            color: #4CAF50;
        }
        .screenshot-description {
            font-size: 14px;
            color: #aaa;
        }
        .stage-number {
            display: inline-block;
            background-color: #4CAF50;
            color: black;
            padding: 2px 8px;
            border-radius: 3px;
            font-weight: bold;
            margin-right: 8px;
        }
        .instructions {
            background-color: #2a2a2a;
            padding: 15px;
            border-left: 4px solid #4CAF50;
            margin: 20px 0;
        }
    </style>
</head>
<body>
    <h1>🚀 First Mission - Screenshot Verification</h1>
    
    <div class="instructions">
        <h3>How to Use This Verification System:</h3>
        <ol>
            <li>Compile the project in Unreal Editor</li>
            <li>Run the mission in Play In Editor (Alt+P)</li>
            <li>At each stage, press F9 and type: <code>ke * HighResShot</code></li>
            <li>Name screenshots according to stage (see list below)</li>
            <li>Refresh this page to see updated screenshots</li>
        </ol>
        <p><strong>Screenshot Location:</strong> Saved/Screenshots/Windows/</p>
    </div>
    
    <h2>Mission Stages - Visual Verification</h2>
    <div class="screenshot-grid">
"""
    
    # Add screenshot placeholders for each stage
    stages = capture_mission_stage_screenshots()
    
    for filename, description in stages:
        stage_num = filename.split('_')[0]
        stage_name = filename.split('_', 1)[1] if '_' in filename else filename
        
        html_content += f"""
        <div class="screenshot-item">
            <div class="screenshot-title">
                <span class="stage-number">{stage_num}</span>
                {stage_name.replace('_', ' ')}
            </div>
            <img src="../../Saved/Screenshots/Windows/{filename}.png" 
                 alt="{description}"
                 onerror="this.src='data:image/svg+xml,%3Csvg xmlns=\\'http://www.w3.org/2000/svg\\' width=\\'400\\' height=\\'300\\'%3E%3Crect fill=\\'%23333\\' width=\\'400\\' height=\\'300\\'/%3E%3Ctext x=\\'50%25\\' y=\\'50%25\\' text-anchor=\\'middle\\' fill=\\'%23666\\' font-size=\\'20\\'%3EScreenshot Not Yet Captured%3C/text%3E%3C/svg%3E'">
            <div class="screenshot-description">{description}</div>
        </div>
"""
    
    html_content += """
    </div>
    
    <h2>📊 Testing Checklist</h2>
    <div class="instructions">
        <h3>Verify Each Stage Shows:</h3>
        <ul>
            <li>✓ Correct camera position and view</li>
            <li>✓ UI elements displaying properly</li>
            <li>✓ Mission objectives visible</li>
            <li>✓ Landing guidance when appropriate</li>
            <li>✓ No visual glitches or artifacts</li>
            <li>✓ Proper lighting and materials</li>
            <li>✓ VFX effects working (landing beam, particles)</li>
            <li>✓ HUD updating correctly</li>
        </ul>
    </div>
    
    <footer style="margin-top: 40px; text-align: center; color: #666;">
        <p>First Mission Screenshot Verification System</p>
        <p>Auto-refresh this page after capturing new screenshots</p>
    </footer>
</body>
</html>
"""
    
    # Save HTML file
    html_path = "Saved/Screenshots/MissionVerification.html"
    try:
        with open(html_path, 'w') as f:
            f.write(html_content)
        log_success(f"Created verification HTML: {html_path}")
        log_info("Open this file in a browser to view screenshots")
        return True
    except Exception as e:
        log_error(f"Failed to create HTML file: {e}")
        return False

def print_testing_instructions():
    """Print detailed testing instructions"""
    log_info("=" * 60)
    log_info("SCREENSHOT VERIFICATION - TESTING INSTRUCTIONS")
    log_info("=" * 60)
    log_info("")
    log_info("📸 SCREENSHOT CAPTURE METHODS:")
    log_info("")
    log_info("Method 1: High-Resolution Screenshots (Recommended)")
    log_info("  1. Press F9 (or ` backtick) to open console")
    log_info("  2. Type: ke * HighResShot")
    log_info("  3. Screenshot saved to: Saved/Screenshots/Windows/")
    log_info("")
    log_info("Method 2: Automation Screenshots")
    log_info("  Console command: Automation.TakeScreenshot <name>")
    log_info("  Example: Automation.TakeScreenshot Stage_PlanetSurface")
    log_info("")
    log_info("Method 3: Blueprint Integration")
    log_info("  - Add 'Take Screenshot' node in BP_FirstMissionGameMode")
    log_info("  - Trigger on each OnMissionStageChanged event")
    log_info("")
    log_info("🎮 TESTING WORKFLOW:")
    log_info("")
    log_info("1. Compile project in Unreal Editor")
    log_info("2. Load MissionLevel.umap")
    log_info("3. Press Alt+P to Play In Editor")
    log_info("4. Capture screenshots at each stage:")
    log_info("   - Main menu")
    log_info("   - Planet surface spawn")
    log_info("   - Approaching spaceship")
    log_info("   - Inside cockpit")
    log_info("   - Flying to moon")
    log_info("   - Landing guidance active")
    log_info("   - Moon surface")
    log_info("   - Mission complete")
    log_info("5. Open MissionVerification.html to review")
    log_info("")
    log_info("📁 FILES CREATED:")
    log_info("  - Saved/Screenshots/MissionVerification.html")
    log_info("  - Saved/Screenshots/Windows/*.png (your screenshots)")
    log_info("")
    log_info("✅ VERIFICATION CHECKLIST:")
    log_info("  □ All 20 mission stages captured")
    log_info("  □ UI elements visible and correct")
    log_info("  □ Landing guidance appears at right time")
    log_info("  □ Camera transitions smooth")
    log_info("  □ No visual glitches")
    log_info("  □ Performance acceptable (check FPS)")
    log_info("  □ VFX effects working")
    log_info("  □ Materials displaying correctly")
    log_info("")
    log_info("=" * 60)

def main():
    """Main verification setup function"""
    log_info("=" * 60)
    log_info("MISSION SCREENSHOT VERIFICATION SYSTEM")
    log_info("=" * 60)
    log_info("")
    
    # Step 1: Set up camera positions
    camera_positions = setup_verification_cameras()
    
    # Step 2: Define mission stages
    log_info("")
    stages = capture_mission_stage_screenshots()
    
    # Step 3: Set up monitoring
    log_info("")
    setup_mission_monitoring()
    
    # Step 4: Create HTML comparison file
    log_info("")
    create_screenshot_comparison_html()
    
    # Step 5: Print testing instructions
    log_info("")
    print_testing_instructions()
    
    log_info("")
    log_success("Screenshot verification system setup complete!")
    log_info("")
    log_info("NEXT STEPS:")
    log_info("1. Compile and test the mission")
    log_info("2. Capture screenshots during testing")
    log_info("3. Open Saved/Screenshots/MissionVerification.html")
    log_info("4. Verify all stages visually")
    log_info("")
    
    return True

# Execute main function
if __name__ == '__main__':
    main()
