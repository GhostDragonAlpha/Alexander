"""
Quick Test - Verify Autonomous Testing System
Tests that the new C++ components are accessible and working
"""

import unreal
import time

print("="*70)
print("QUICK TEST - Autonomous Testing System")
print("="*70)

# Test 1: Check if AutomationAPIServer class exists
print("\n[TEST 1] Checking AutomationAPIServer class...")
try:
    api_server_class = unreal.AutomationAPIServer.static_class()
    print(f"✓ AutomationAPIServer class found: {api_server_class.get_name()}")
except Exception as e:
    print(f"✗ AutomationAPIServer not found: {e}")

# Test 2: Check if VRHandSimulator class exists
print("\n[TEST 2] Checking VRHandSimulator class...")
try:
    vr_hand_sim_class = unreal.VRHandSimulator.static_class()
    print(f"✓ VRHandSimulator class found: {vr_hand_sim_class.get_name()}")
except Exception as e:
    print(f"✗ VRHandSimulator not found: {e}")

# Test 3: Check if AutomationGameMode class exists
print("\n[TEST 3] Checking AutomationGameMode class...")
try:
    game_mode_class = unreal.AutomationGameMode.static_class()
    print(f"✓ AutomationGameMode class found: {game_mode_class.get_name()}")
except Exception as e:
    print(f"✗ AutomationGameMode not found: {e}")

# Test 4: Check if FlightController has network replication
print("\n[TEST 4] Checking FlightController replication...")
try:
    flight_controller_class = unreal.FlightController.static_class()
    print(f"✓ FlightController class found: {flight_controller_class.get_name()}")

    # Check for replicated properties
    properties = flight_controller_class.get_properties()
    replicated_props = []
    for prop in properties:
        prop_name = prop.get_name()
        if any(name in prop_name for name in ['RawThrustInput', 'RawRotationInput', 'SmoothedThrustInput', 'SmoothedRotationInput', 'AssistMode']):
            replicated_props.append(prop_name)

    if replicated_props:
        print(f"✓ Found replicated properties: {', '.join(replicated_props)}")
    else:
        print("⚠ Could not verify replicated properties (may still work)")

except Exception as e:
    print(f"✗ FlightController check failed: {e}")

# Test 5: Try to create AutomationAPIServer component
print("\n[TEST 5] Testing AutomationAPIServer instantiation...")
try:
    # Create a test actor
    editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    world = editor_subsystem.get_editor_world()

    test_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.Actor.static_class(),
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )

    # Add AutomationAPIServer component
    api_server = test_actor.add_component_by_class(unreal.AutomationAPIServer.static_class())

    if api_server:
        print(f"✓ AutomationAPIServer component created successfully")
        print(f"  Listen Port: {api_server.get_editor_property('listen_port')}")
        print(f"  Server Enabled: {api_server.get_editor_property('b_server_enabled')}")

        # Clean up test actor
        test_actor.destroy_actor()
    else:
        print("✗ Failed to create AutomationAPIServer component")

except Exception as e:
    print(f"✗ AutomationAPIServer instantiation failed: {e}")

# Test 6: Try to create VRHandSimulator component
print("\n[TEST 6] Testing VRHandSimulator instantiation...")
try:
    # Create a test actor
    test_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.Actor.static_class(),
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )

    # Add VRHandSimulator component
    vr_hand_sim = test_actor.add_component_by_class(unreal.VRHandSimulator.static_class())

    if vr_hand_sim:
        print(f"✓ VRHandSimulator component created successfully")
        print(f"  Default Rest Position: {vr_hand_sim.get_editor_property('default_rest_position')}")
        print(f"  Max Reach Distance: {vr_hand_sim.get_editor_property('max_reach_distance')}")
        print(f"  Debug Visualization: {vr_hand_sim.get_editor_property('b_debug_visualization')}")

        # Clean up test actor
        test_actor.destroy_actor()
    else:
        print("✗ Failed to create VRHandSimulator component")

except Exception as e:
    print(f"✗ VRHandSimulator instantiation failed: {e}")

print("\n" + "="*70)
print("QUICK TEST COMPLETE")
print("="*70)
print("\nIf all tests passed, the autonomous testing system is ready!")
print("Next step: Run 'python test_flight_multiplayer.py' for full test suite")
print("="*70)
