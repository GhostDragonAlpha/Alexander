# TEST THIS IN UNREAL PYTHON CONSOLE
# Open console with ` (backtick) key, then paste this command:
# exec(open(r'C:\Users\allen\Desktop\Alexander\Alexander\Content\Python\TEST_VISUAL_VERIFICATION.py').read())

import unreal

print("\n" + "="*70)
print("  🧪 TESTING VISUAL VERIFICATION SYSTEM")
print("="*70 + "\n")

# Test 1: Can we create a blueprint?
print("TEST 1: Creating test blueprint...")
try:
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("ParentClass", unreal.Actor)
    
    test_bp = asset_tools.create_asset(
        asset_name="BP_TEST",
        package_path="/Game/SpaceShip/Blueprints",
        asset_class=unreal.Blueprint,
        factory=factory
    )
    
    if test_bp:
        print("✅ TEST 1 PASSED: Can create blueprints!")
    else:
        print("❌ TEST 1 FAILED: Blueprint creation returned None")
except Exception as e:
    print(f"❌ TEST 1 FAILED: {e}")

# Test 2: Can we place an actor in the scene?
print("\nTEST 2: Placing actor in scene...")
try:
    editor_level_lib = unreal.EditorLevelLibrary()
    
    # Try to spawn a cube
    cube = editor_level_lib.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(0, 0, 100)
    )
    
    if cube:
        print(f"✅ TEST 2 PASSED: Spawned actor at (0, 0, 100)")
    else:
        print("❌ TEST 2 FAILED: Spawn returned None")
except Exception as e:
    print(f"❌ TEST 2 FAILED: {e}")

# Test 3: Can we take a screenshot?
print("\nTEST 3: Taking screenshot...")
try:
    unreal.AutomationLibrary.take_high_res_screenshot(1920, 1080, "test_screenshot.png")
    print("✅ TEST 3 PASSED: Screenshot command executed")
    print("   Check: ProjectDir/Saved/Screenshots/Windows/")
except Exception as e:
    print(f"❌ TEST 3 FAILED: {e}")

print("\n" + "="*70)
print("  🧪 TEST COMPLETE")
print("="*70 + "\n")
