"""
Integration test script for AI Vision System
Run this inside Unreal Editor console with: py exec(open('test_ai_vision_integration.py').read())
"""

import unreal
import json
import http.client
import time

def test_ai_backend_connection():
    """Test connection to AI backend server"""
    print("=" * 60)
    print("TEST 1: AI Backend Connection")
    print("=" * 60)
    
    try:
        conn = http.client.HTTPConnection("localhost", 8000, timeout=5)
        conn.request("GET", "/api/health")
        response = conn.getresponse()
        data = response.read().decode()
        
        if response.status == 200:
            print("✅ AI Backend is ONLINE")
            print(f"   Response: {data}")
            return True
        else:
            print(f"❌ AI Backend returned status: {response.status}")
            return False
    except Exception as e:
        print(f"❌ Cannot connect to AI Backend: {e}")
        print("   Make sure the server is running at http://localhost:8000")
        return False
    finally:
        conn.close()

def test_python_environment():
    """Test Python environment in Unreal"""
    print("\n" + "=" * 60)
    print("TEST 2: Unreal Python Environment")
    print("=" * 60)
    
    try:
        # Test basic Unreal Python API
        editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        world = editor_subsystem.get_editor_world()
        
        if world:
            print(f"✅ World loaded: {world.get_name()}")
        else:
            print("⚠️  No world loaded - open a level first")
            return False
            
        # Test asset registry
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        print(f"✅ Asset Registry accessible")
        
        # Test level editor subsystem
        level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        current_level = level_editor.get_current_level()
        print(f"✅ Current Level: {current_level.get_name() if current_level else 'None'}")
        
        return True
        
    except Exception as e:
        print(f"❌ Python environment error: {e}")
        return False

def test_screenshot_capture_actor():
    """Check if AIScreenshotCapture actor exists in project"""
    print("\n" + "=" * 60)
    print("TEST 3: Screenshot Capture Actor")
    print("=" * 60)
    
    try:
        # Try to find the AIScreenshotCapture class
        screenshot_class = unreal.load_class(None, "/Script/AIVisionSystem.AIScreenshotCapture")
        
        if screenshot_class:
            print(f"✅ AIScreenshotCapture class found: {screenshot_class.get_name()}")
            
            # Check if any instances exist in the level
            editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
            world = editor_subsystem.get_editor_world()
            
            if world:
                all_actors = unreal.GameplayStatics.get_all_actors_of_class(world, screenshot_class)
                if len(all_actors) > 0:
                    print(f"✅ Found {len(all_actors)} AIScreenshotCapture actor(s) in level")
                    for actor in all_actors:
                        print(f"   - {actor.get_name()}")
                    return True
                else:
                    print("⚠️  No AIScreenshotCapture actors in level")
                    print("   To add one: Content Browser → Plugins → AIVisionSystem")
                    print("   Drag AIScreenshotCapture into your level")
                    return False
            else:
                print("⚠️  No world loaded")
                return False
        else:
            print("❌ AIScreenshotCapture class not found")
            print("   Make sure the plugin is compiled and loaded")
            return False
            
    except Exception as e:
        print(f"⚠️  Could not load AIScreenshotCapture class: {e}")
        print("   This is normal if plugin isn't loaded yet")
        return False

def test_ai_executor_loaded():
    """Check if AI executor is loaded"""
    print("\n" + "=" * 60)
    print("TEST 4: AI Executor System")
    print("=" * 60)
    
    try:
        # Check if ai_executor functions exist
        if 'start_ai_dev' in dir():
            print("✅ AI Executor already loaded")
            return True
        else:
            print("⚠️  AI Executor not loaded yet")
            print("   Run: py exec(open('Content/Python/ai_executor.py').read())")
            return False
    except:
        print("⚠️  AI Executor not loaded yet")
        return False

def spawn_test_screenshot_actor():
    """Spawn a test AIScreenshotCapture actor"""
    print("\n" + "=" * 60)
    print("AUTO-SPAWN: Creating AIScreenshotCapture Actor")
    print("=" * 60)
    
    try:
        screenshot_class = unreal.load_class(None, "/Script/AIVisionSystem.AIScreenshotCapture")
        
        if not screenshot_class:
            print("❌ Cannot load AIScreenshotCapture class")
            return False
            
        editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        world = editor_subsystem.get_editor_world()
        
        if not world:
            print("❌ No world loaded")
            return False
            
        # Spawn actor at origin
        location = unreal.Vector(0, 0, 200)
        rotation = unreal.Rotator(0, 0, 0)
        
        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            screenshot_class,
            location,
            rotation
        )
        
        if actor:
            print(f"✅ Spawned AIScreenshotCapture: {actor.get_name()}")
            print(f"   Location: {location}")
            
            # Select the actor
            unreal.EditorLevelLibrary.set_selected_level_actors([actor])
            print("✅ Actor selected in viewport")
            
            return True
        else:
            print("❌ Failed to spawn actor")
            return False
            
    except Exception as e:
        print(f"❌ Error spawning actor: {e}")
        return False

def run_full_integration_test():
    """Run complete integration test"""
    print("\n")
    print("*" * 60)
    print("    AI VISION SYSTEM - INTEGRATION TEST")
    print("*" * 60)
    print()
    
    results = []
    
    # Test 1: Backend connection
    results.append(("AI Backend Connection", test_ai_backend_connection()))
    
    # Test 2: Python environment
    results.append(("Python Environment", test_python_environment()))
    
    # Test 3: Screenshot actor
    actor_exists = test_screenshot_capture_actor()
    results.append(("Screenshot Actor", actor_exists))
    
    # If no actor exists, try to spawn one
    if not actor_exists:
        spawn_result = spawn_test_screenshot_actor()
        if spawn_result:
            results.append(("Auto-Spawn Actor", True))
    
    # Test 4: AI executor
    results.append(("AI Executor", test_ai_executor_loaded()))
    
    # Summary
    print("\n" + "=" * 60)
    print("TEST SUMMARY")
    print("=" * 60)
    
    passed = sum(1 for _, result in results if result)
    total = len(results)
    
    for test_name, result in results:
        status = "✅ PASS" if result else "❌ FAIL"
        print(f"{status} - {test_name}")
    
    print(f"\nResults: {passed}/{total} tests passed")
    
    if passed == total:
        print("\n🎉 ALL TESTS PASSED! System is fully operational!")
        print("\nNext steps:")
        print("1. Select the AIScreenshotCapture actor in viewport")
        print("2. In Details panel, set 'Development Goal'")
        print("3. Check 'Debug Mode' = true")
        print("4. Click 'Start Auto Capture'")
        print("5. Watch the Output Log for AI analysis!")
    else:
        print("\n⚠️  Some tests failed. Check messages above.")
        
    return passed == total

# Auto-run test when script is executed
if __name__ == "__main__":
    run_full_integration_test()
else:
    # When imported, make function available
    print("Integration test loaded. Run: py run_full_integration_test()")
