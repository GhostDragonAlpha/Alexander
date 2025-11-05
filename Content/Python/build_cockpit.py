"""
VR Spaceship Cockpit - Development Script
Run this in Unreal Console to start building the cockpit
"""

import unreal

print("=" * 70)
print("  STARTING VR SPACESHIP COCKPIT DEVELOPMENT")
print("=" * 70)

# Get subsystems
editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
editor_asset_lib = unreal.EditorAssetLibrary()
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

# Define paths
cockpit_path = "/Game/SpaceShip/Cockpit/"
blueprints_path = "/Game/SpaceShip/Blueprints/"
materials_path = "/Game/SpaceShip/Materials/"

print("\n[1/6] Creating folder structure...")
try:
    # Create directories if they don't exist
    folders = [
        "/Game/SpaceShip",
        "/Game/SpaceShip/Cockpit",
        "/Game/SpaceShip/Blueprints",
        "/Game/SpaceShip/Materials",
        "/Game/SpaceShip/Meshes",
        "/Game/SpaceShip/Audio",
        "/Game/Farming",
        "/Game/Planets",
        "/Game/AI"
    ]
    
    for folder in folders:
        if not editor_asset_lib.does_directory_exist(folder):
            editor_asset_lib.make_directory(folder)
            print(f"  ✓ Created: {folder}")
        else:
            print(f"  • Exists: {folder}")
            
    print("✓ Folder structure ready")
except Exception as e:
    print(f"✗ Error creating folders: {e}")

print("\n[2/6] Creating VR Pawn Blueprint...")
try:
    # Create VR Player Pawn
    pawn_name = "BP_VRSpaceshipPawn"
    pawn_package_path = blueprints_path + pawn_name
    
    if not editor_asset_lib.does_asset_exist(pawn_package_path):
        # Create blueprint based on Pawn
        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", unreal.Pawn)
        
        asset = asset_tools.create_asset(
            pawn_name,
            blueprints_path,
            unreal.Blueprint,
            factory
        )
        
        if asset:
            print(f"  ✓ Created: {pawn_name}")
            
            # Get the blueprint's generated class
            bp_class = asset.generated_class()
            
            # We'll add components via the full editor for now
            print(f"  • Blueprint ready for component setup")
        else:
            print(f"  ✗ Failed to create {pawn_name}")
    else:
        print(f"  • Already exists: {pawn_name}")
        
    print("✓ VR Pawn blueprint created")
except Exception as e:
    print(f"✗ Error creating pawn: {e}")

print("\n[3/6] Creating Cockpit Mesh Actor...")
try:
    cockpit_name = "BP_CockpitStructure"
    cockpit_package_path = blueprints_path + cockpit_name
    
    if not editor_asset_lib.does_asset_exist(cockpit_package_path):
        factory = unreal.BlueprintFactory()
        factory.set_editor_property("parent_class", unreal.Actor)
        
        asset = asset_tools.create_asset(
            cockpit_name,
            blueprints_path,
            unreal.Blueprint,
            factory
        )
        
        if asset:
            print(f"  ✓ Created: {cockpit_name}")
        else:
            print(f"  ✗ Failed to create {cockpit_name}")
    else:
        print(f"  • Already exists: {cockpit_name}")
        
    print("✓ Cockpit structure blueprint created")
except Exception as e:
    print(f"✗ Error creating cockpit: {e}")

print("\n[4/6] Creating Interactive Controls...")
try:
    controls = [
        "BP_Joystick",
        "BP_Throttle",
        "BP_Button",
        "BP_Switch",
        "BP_HolographicDisplay"
    ]
    
    for control_name in controls:
        control_path = blueprints_path + control_name
        
        if not editor_asset_lib.does_asset_exist(control_path):
            factory = unreal.BlueprintFactory()
            factory.set_editor_property("parent_class", unreal.Actor)
            
            asset = asset_tools.create_asset(
                control_name,
                blueprints_path,
                unreal.Blueprint,
                factory
            )
            
            if asset:
                print(f"  ✓ Created: {control_name}")
            else:
                print(f"  ✗ Failed to create {control_name}")
        else:
            print(f"  • Exists: {control_name}")
            
    print("✓ Control blueprints created")
except Exception as e:
    print(f"✗ Error creating controls: {e}")

print("\n[5/6] Creating Material Instances...")
try:
    # Find a base material to use
    base_materials = editor_asset_lib.list_assets("/Engine/BasicShapes")
    
    print("  • Materials will be created in editor")
    print("  • Use M_Metal_Steel or similar as base")
    print("✓ Material setup ready")
except Exception as e:
    print(f"✗ Error with materials: {e}")

print("\n[6/6] Setting up level...")
try:
    world = editor_subsystem.get_editor_world()
    
    if world:
        # Spawn VR Pawn in level
        pawn_class = unreal.EditorAssetLibrary.load_blueprint_class(blueprints_path + "BP_VRSpaceshipPawn")
        
        if pawn_class:
            location = unreal.Vector(0, 0, 120)
            rotation = unreal.Rotator(0, 0, 0)
            
            spawned = unreal.EditorLevelLibrary.spawn_actor_from_class(
                pawn_class,
                location,
                rotation
            )
            
            if spawned:
                print(f"  ✓ Spawned VR Pawn at {location}")
            else:
                print("  • VR Pawn ready to place manually")
        else:
            print("  • VR Pawn blueprint needs compilation first")
            
        # Add some initial lighting
        lights = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.DirectionalLight)
        if len(lights) == 0:
            light_location = unreal.Vector(0, 0, 500)
            light_rotation = unreal.Rotator(-45, 0, 0)
            
            light = unreal.EditorLevelLibrary.spawn_actor_from_class(
                unreal.DirectionalLight,
                light_location,
                light_rotation
            )
            
            if light:
                print("  ✓ Added directional light")
        else:
            print(f"  • Lighting exists ({len(lights)} lights)")
            
        print("✓ Level setup complete")
    else:
        print("  ⚠ No level loaded - open a level first")
        
except Exception as e:
    print(f"✗ Error setting up level: {e}")

print("\n" + "=" * 70)
print("  🚀 VR SPACESHIP COCKPIT FOUNDATION COMPLETE!")
print("=" * 70)
print("\nNext steps:")
print("  1. Open BP_VRSpaceshipPawn in blueprint editor")
print("  2. Add components:")
print("     - VROrigin (Scene)")
print("     - Camera (attached to VROrigin)")
print("     - MotionController_L and MotionController_R")
print("     - Grab Sphere components for hand collision")
print("  3. Open BP_Joystick and add:")
print("     - Static Mesh component")
print("     - Physics Handle component")
print("     - Grab detection sphere")
print("  4. Set up VR input mappings in Project Settings")
print("\nReady to start adding flight physics! 🎮")
