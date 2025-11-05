"""
AI Visual Verification System
I create → place → screenshot → see → adjust
"""

import unreal
import json
import urllib.request
import urllib.error
import base64
import time

print("\n" + "="*70)
print("  🤖 AI VISUAL VERIFICATION SYSTEM")
print("="*70 + "\n")

# Get subsystems
editor_asset_lib = unreal.EditorAssetLibrary()
editor_level_lib = unreal.EditorLevelLibrary()
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

AI_BACKEND_URL = "http://localhost:8000/analyze-and-develop"

# ============================================================================
# STEP 1: CREATE ASSETS
# ============================================================================
def create_cockpit_assets():
    """Create all the blueprints and assets"""
    print("📦 STEP 1: Creating Assets...\n")
    
    # Ensure folders exist
    folders = [
        "/Game/SpaceShip/Blueprints",
        "/Game/SpaceShip/Materials",
        "/Game/SpaceShip/UI"
    ]
    
    for folder in folders:
        if not editor_asset_lib.does_directory_exist(folder):
            editor_asset_lib.make_directory(folder)
    
    # Create blueprints
    blueprints = [
        ("BP_VRSpaceshipPlayer", unreal.Pawn),
        ("BP_SpaceshipCockpit", unreal.Actor),
        ("BP_FlightStick", unreal.Actor),
        ("BP_ThrottleControl", unreal.Actor),
        ("BP_ButtonPanel", unreal.Actor),
    ]
    
    created = []
    for bp_name, parent_class in blueprints:
        bp_path = f"/Game/SpaceShip/Blueprints/{bp_name}"
        
        if not editor_asset_lib.does_asset_exist(bp_path):
            factory = unreal.BlueprintFactory()
            factory.set_editor_property("ParentClass", parent_class)
            
            try:
                new_asset = asset_tools.create_asset(
                    asset_name=bp_name,
                    package_path="/Game/SpaceShip/Blueprints",
                    asset_class=unreal.Blueprint,
                    factory=factory
                )
                
                if new_asset:
                    print(f"  ✅ Created: {bp_name}")
                    editor_asset_lib.save_loaded_asset(new_asset)
                    created.append(bp_name)
            except Exception as e:
                print(f"  ❌ Error: {bp_name} - {e}")
        else:
            print(f"  ✓ Exists: {bp_name}")
            created.append(bp_name)
    
    return created

# ============================================================================
# STEP 2: PLACE IN SCENE
# ============================================================================
def place_in_scene():
    """Place blueprints in the level for visual inspection"""
    print("\n🎬 STEP 2: Placing in Scene...\n")
    
    # Get current level
    level = editor_level_lib.get_editor_world()
    
    # Blueprints to place with positions
    placements = [
        ("/Game/SpaceShip/Blueprints/BP_SpaceshipCockpit", unreal.Vector(0, 0, 100)),
        ("/Game/SpaceShip/Blueprints/BP_FlightStick", unreal.Vector(-50, -30, 120)),
        ("/Game/SpaceShip/Blueprints/BP_ThrottleControl", unreal.Vector(50, -30, 120)),
        ("/Game/SpaceShip/Blueprints/BP_ButtonPanel", unreal.Vector(0, 50, 130)),
    ]
    
    spawned_actors = []
    
    for bp_path, location in placements:
        if editor_asset_lib.does_asset_exist(bp_path):
            # Load the blueprint
            bp_asset = editor_asset_lib.load_asset(bp_path)
            
            if bp_asset:
                # Spawn actor in level
                actor = editor_level_lib.spawn_actor_from_object(
                    bp_asset,
                    location,
                    unreal.Rotator(0, 0, 0)
                )
                
                if actor:
                    print(f"  ✅ Placed: {bp_path.split('/')[-1]} at {location}")
                    spawned_actors.append(actor)
                else:
                    print(f"  ⚠ Could not spawn: {bp_path.split('/')[-1]}")
    
    return spawned_actors

# ============================================================================
# STEP 3: TAKE SCREENSHOT
# ============================================================================
def take_screenshot():
    """Capture screenshot using Unreal's built-in screenshot system"""
    print("\n📸 STEP 3: Taking Screenshot...\n")

    # Use Unreal's built-in screenshot system
    try:
        # Note: AIVisionSystemLibrary requires the C++ plugin to be properly compiled
        # For now, use the reliable built-in screenshot system

        import os

        # Use Unreal's high-resolution screenshot
        unreal.AutomationLibrary.take_high_res_screenshot(
            1920, 1080, "ai_verification.png"
        )

        # Get project directory and build screenshot path
        project_dir = unreal.SystemLibrary.get_project_directory()
        screenshot_path = os.path.join(project_dir, "Saved", "Screenshots", "Windows", "ai_verification.png")

        print(f"  ✅ Screenshot saved: {screenshot_path}")
        print(f"  ℹ️  Note: For AI Vision System plugin integration, ensure AIVisionSystemLibrary C++ module is compiled")

        return screenshot_path

    except Exception as e:
        print(f"  ❌ Screenshot error: {e}")
        import traceback
        traceback.print_exc()
        return None

# ============================================================================
# STEP 4: SEND TO AI FOR ANALYSIS
# ============================================================================
def analyze_screenshot(screenshot_path):
    """Send screenshot to AI backend for analysis"""
    print("\n🔍 STEP 4: AI Analyzing Screenshot...\n")
    
    if not screenshot_path or not os.path.exists(screenshot_path):
        print("  ❌ No screenshot to analyze")
        return None
    
    try:
        # Read and encode screenshot
        with open(screenshot_path, 'rb') as f:
            image_data = base64.b64encode(f.read()).decode('utf-8')
        
        # Prepare request
        data = {
            "image": image_data,
            "context": "VR spaceship cockpit initial placement. Verify that cockpit, joystick, throttle, and button panel are visible and positioned correctly.",
            "goal": "Build VR spaceship cockpit with interactive controls"
        }
        
        # Send to backend
        req = urllib.request.Request(
            AI_BACKEND_URL,
            data=json.dumps(data).encode('utf-8'),
            headers={'Content-Type': 'application/json'}
        )
        
        with urllib.request.urlopen(req, timeout=30) as response:
            result = json.loads(response.read().decode('utf-8'))
            
            print("  ✅ AI Analysis Complete!")
            print(f"\n  💭 AI Says: {result.get('analysis', 'No analysis')}\n")
            
            # Get suggested actions
            actions = result.get('actions', [])
            if actions:
                print("  📝 Suggested Actions:")
                for action in actions:
                    print(f"    • {action}")
            
            return result
            
    except urllib.error.URLError as e:
        print(f"  ⚠ Backend not responding: {e}")
        print("  💡 Make sure AI backend is running on port 8000")
        return None
    except Exception as e:
        print(f"  ❌ Analysis error: {e}")
        return None

# ============================================================================
# STEP 5: EXECUTE AI SUGGESTIONS
# ============================================================================
def execute_suggestions(analysis_result):
    """Execute code suggestions from AI"""
    print("\n⚙️  STEP 5: Executing AI Suggestions...\n")
    
    if not analysis_result:
        print("  ⚠ No suggestions to execute")
        return
    
    code = analysis_result.get('code', '')
    if code:
        print("  🔧 Executing AI-generated code...")
        try:
            exec(code, globals())
            print("  ✅ Code executed successfully")
        except Exception as e:
            print(f"  ❌ Execution error: {e}")
    else:
        print("  ℹ️  No code to execute")

# ============================================================================
# MAIN VISUAL VERIFICATION LOOP
# ============================================================================
def run_visual_verification():
    """Main loop: Create → Place → Screenshot → Analyze → Adjust"""
    
    print("="*70)
    print("  🚀 STARTING VISUAL VERIFICATION LOOP")
    print("="*70 + "\n")
    
    # Step 1: Create assets
    created_assets = create_cockpit_assets()
    print(f"\n  ✅ Created {len(created_assets)} assets")
    
    # Step 2: Place in scene
    spawned_actors = place_in_scene()
    print(f"\n  ✅ Placed {len(spawned_actors)} actors in scene")
    
    # Wait for editor to update
    print("\n  ⏳ Waiting for editor to refresh...")
    time.sleep(2)
    
    # Step 3: Take screenshot
    screenshot_path = take_screenshot()
    
    if screenshot_path:
        # Step 4: Analyze
        analysis = analyze_screenshot(screenshot_path)
        
        if analysis:
            # Step 5: Execute suggestions
            execute_suggestions(analysis)
    
    print("\n" + "="*70)
    print("  ✅ VISUAL VERIFICATION COMPLETE!")
    print("="*70 + "\n")
    
    print("💡 Check the Content Browser → SpaceShip folder")
    print("💡 Check the viewport to see placed objects\n")

# ============================================================================
# RUN IT!
# ============================================================================
if __name__ == "__main__":
    import os
    run_visual_verification()
