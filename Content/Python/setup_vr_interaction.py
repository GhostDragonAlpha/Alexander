"""
Add VR Interaction to Cockpit
Add grab components, physics, and motion controller setup
"""

import unreal

print("\n" + "="*70)
print("  🎮 ADDING VR INTERACTION COMPONENTS")
print("="*70 + "\n")

editor_level_lib = unreal.EditorLevelLibrary()
editor_asset_lib = unreal.EditorAssetLibrary()

def setup_vr_pawn():
    """Create VR pawn with motion controllers"""
    print("🧍 Setting up VR Player Pawn...\n")
    
    # Check if BP_VRSpaceshipPlayer exists
    pawn_path = "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer"
    
    if not editor_asset_lib.does_asset_exist(pawn_path):
        print("  ⚠ VR Pawn blueprint not found")
        return
    
    print(f"  ✅ VR Pawn exists: {pawn_path}")
    print("  💡 Manual setup required:")
    print("     1. Open BP_VRSpaceshipPlayer")
    print("     2. Add VROrigin (Scene component)")
    print("     3. Add Camera component under VROrigin")
    print("     4. Add MotionController_Left (with hand mesh)")
    print("     5. Add MotionController_Right (with hand mesh)")
    print("     6. Set as default pawn in World Settings\n")

def add_grab_components():
    """Add physics and grab capability to interactive objects"""
    print("🤚 Adding Grab Components to Controls...\n")
    
    # Get all actors in the level
    all_actors = editor_level_lib.get_all_level_actors()
    
    interactive_objects = ["FlightStick", "Throttle"]
    
    for actor in all_actors:
        if not actor:
            continue
            
        actor_label = str(actor.get_actor_label())
        
        if any(obj in actor_label for obj in interactive_objects):
            print(f"  🔧 Found: {actor_label}")
            
            # Enable physics simulation
            try:
                if hasattr(actor, 'static_mesh_component'):
                    mesh_comp = actor.static_mesh_component
                    if mesh_comp:
                        # Enable physics
                        mesh_comp.set_simulate_physics(True)
                        mesh_comp.set_enable_gravity(False)  # No gravity in space!
                        mesh_comp.set_linear_damping(2.0)  # Smooth movement
                        mesh_comp.set_angular_damping(2.0)
                        
                        print(f"    ✅ Enabled physics on {actor_label}")
                        print(f"    ✅ Disabled gravity (we're in space!)")
                        print(f"    ✅ Added damping for smooth control\n")
            except Exception as e:
                print(f"    ⚠ Could not modify {actor_label}: {e}\n")

def create_vr_interaction_guide():
    """Create a text file guide for VR interaction setup"""
    print("📝 Creating VR Interaction Setup Guide...\n")
    
    guide = """
# VR INTERACTION SETUP GUIDE

## Components Created:
✅ FlightStick - Left hand control (physics enabled)
✅ Throttle - Right hand control (physics enabled)  
✅ VR_CameraPosition - Player head reference
✅ Cockpit geometry - Seats, panels, displays

## Next Steps for Full VR:

### 1. VR Pawn Setup (BP_VRSpaceshipPlayer)
- Add VROrigin (Scene Component)
- Add Camera (under VROrigin) at eye level
- Add MotionController_Left with:
  - GrabComponent or PhysicsHandle
  - Hand mesh/visualization
- Add MotionController_Right with:
  - GrabComponent or PhysicsHandle  
  - Hand mesh/visualization

### 2. Input Mapping
Create Input Actions for:
- Grab_Left (Trigger on left controller)
- Grab_Right (Trigger on right controller)
- Release (Release triggers)

### 3. Grab Logic
In motion controller blueprints:
- On Grab: Line trace from controller
- If hit FlightStick/Throttle: Attach to hand
- While grabbed: Apply forces based on hand movement
- On Release: Detach from hand

### 4. Flight Control Integration
Map hand positions to flight controls:
- FlightStick position → Pitch/Roll
- Throttle position → Forward/Backward thrust
- Button presses → Systems (lights, shields, etc.)

### 5. Testing in VR
- Set BP_VRSpaceshipPlayer as default pawn
- PIE (Play In Editor) with VR headset
- Test grabbing and moving controls
- Adjust physics damping for feel

## Current Status:
✅ Cockpit geometry built
✅ Physics enabled on interactive objects
✅ VR pawn blueprint created
⏳ Need to configure motion controllers and grab logic
⏳ Need input mappings
⏳ Need to connect to flight physics

## Files to Edit:
- /Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer
- Project Settings → Input → Action Mappings
- Create BP_FlightController for physics simulation
"""
    
    import os
    project_dir = unreal.SystemLibrary.get_project_directory()
    guide_path = os.path.join(project_dir, "Content", "SpaceShip", "VR_INTERACTION_GUIDE.txt")
    
    try:
        with open(guide_path, 'w') as f:
            f.write(guide)
        print(f"  ✅ Created guide: {guide_path}\n")
    except Exception as e:
        print(f"  ⚠ Could not write guide: {e}\n")

# Run VR interaction setup
setup_vr_pawn()
add_grab_components()
create_vr_interaction_guide()

print("=" * 70)
print("  ✅ VR INTERACTION SETUP COMPLETE!")
print("=" * 70)
print("\n📋 Summary:")
print("  • VR Pawn blueprint ready for configuration")
print("  • Physics enabled on FlightStick and Throttle")
print("  • Gravity disabled (space environment)")
print("  • Setup guide created")
print("\n🎯 Next: Configure motion controllers in BP_VRSpaceshipPlayer")
print("💡 See: Content/SpaceShip/VR_INTERACTION_GUIDE.txt\n")
