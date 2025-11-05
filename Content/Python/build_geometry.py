"""
Build Actual Cockpit Geometry
Add 3D meshes and components to make the cockpit visible and interactive
"""

import unreal

print("\n" + "="*70)
print("  🎨 BUILDING COCKPIT GEOMETRY")
print("="*70 + "\n")

# Get subsystems
editor_asset_lib = unreal.EditorAssetLibrary()
editor_utility = unreal.EditorUtilityLibrary()

def add_geometry_to_blueprint(bp_path, component_descriptions):
    """Add static mesh components to a blueprint"""
    print(f"\n🔧 Adding geometry to {bp_path.split('/')[-1]}...")
    
    if not editor_asset_lib.does_asset_exist(bp_path):
        print(f"  ⚠ Blueprint not found: {bp_path}")
        return False
    
    try:
        # Load the blueprint
        bp_asset = editor_asset_lib.load_blueprint_class(bp_path)
        
        if not bp_asset:
            print(f"  ⚠ Could not load blueprint")
            return False
        
        print(f"  ✅ Loaded blueprint successfully")
        
        # Note: Adding components to blueprints via Python is complex
        # For now, we'll create a detailed guide for manual setup
        # Or we can spawn them directly in the level with meshes
        
        return True
        
    except Exception as e:
        print(f"  ❌ Error: {e}")
        return False

def create_cockpit_scene():
    """Create a complete cockpit scene with visible geometry"""
    print("\n🎬 Creating cockpit scene with geometry...\n")
    
    editor_level_lib = unreal.EditorLevelLibrary()
    
    # Clear existing actors
    print("Cleaning up existing actors...")
    all_actors = editor_level_lib.get_all_level_actors()
    for actor in all_actors:
        if actor and "BP_" in str(actor.get_actor_label()):
            editor_level_lib.destroy_actor(actor)
    
    print("✅ Scene cleared\n")
    
    # Cockpit Base - Large platform
    print("Creating cockpit base...")
    base = editor_level_lib.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(0, 0, 0)
    )
    if base:
        base.set_actor_label("Cockpit_Base")
        # Set it to use a cube mesh
        mesh_comp = base.static_mesh_component
        cube_mesh = unreal.EditorAssetLibrary.load_asset('/Engine/BasicShapes/Cube')
        if cube_mesh:
            mesh_comp.set_static_mesh(cube_mesh)
            base.set_actor_scale3d(unreal.Vector(4, 3, 0.1))  # Wide, flat platform
        print("  ✅ Cockpit base created")
    
    # Seat
    print("Creating pilot seat...")
    seat = editor_level_lib.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(0, -50, 50)
    )
    if seat:
        seat.set_actor_label("Pilot_Seat")
        mesh_comp = seat.static_mesh_component
        cube_mesh = unreal.EditorAssetLibrary.load_asset('/Engine/BasicShapes/Cube')
        if cube_mesh:
            mesh_comp.set_static_mesh(cube_mesh)
            seat.set_actor_scale3d(unreal.Vector(0.8, 0.8, 0.8))
        print("  ✅ Pilot seat created")
    
    # Flight Stick (Left hand)
    print("Creating flight stick...")
    stick = editor_level_lib.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(-40, 20, 70)
    )
    if stick:
        stick.set_actor_label("FlightStick")
        mesh_comp = stick.static_mesh_component
        cylinder_mesh = unreal.EditorAssetLibrary.load_asset('/Engine/BasicShapes/Cylinder')
        if cylinder_mesh:
            mesh_comp.set_static_mesh(cylinder_mesh)
            stick.set_actor_scale3d(unreal.Vector(0.1, 0.1, 0.5))  # Thin, tall stick
            stick.set_actor_rotation(unreal.Rotator(15, 0, 0), False)  # Slight tilt (teleport_physics=False)
        print("  ✅ Flight stick created")
    
    # Throttle (Right hand)
    print("Creating throttle...")
    throttle = editor_level_lib.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(40, 20, 70)
    )
    if throttle:
        throttle.set_actor_label("Throttle")
        mesh_comp = throttle.static_mesh_component
        cube_mesh = unreal.EditorAssetLibrary.load_asset('/Engine/BasicShapes/Cube')
        if cube_mesh:
            mesh_comp.set_static_mesh(cube_mesh)
            throttle.set_actor_scale3d(unreal.Vector(0.3, 0.2, 0.8))
        print("  ✅ Throttle created")
    
    # Button Panel (Center)
    print("Creating button panel...")
    panel = editor_level_lib.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(0, 70, 90)
    )
    if panel:
        panel.set_actor_label("ButtonPanel")
        mesh_comp = panel.static_mesh_component
        cube_mesh = unreal.EditorAssetLibrary.load_asset('/Engine/BasicShapes/Cube')
        if cube_mesh:
            mesh_comp.set_static_mesh(cube_mesh)
            panel.set_actor_scale3d(unreal.Vector(2, 0.1, 1))  # Wide, thin panel
            panel.set_actor_rotation(unreal.Rotator(-30, 0, 0), False)  # Angled toward player (teleport_physics=False)
        print("  ✅ Button panel created")
    
    # HUD Displays (Left and Right)
    print("Creating HUD displays...")
    for side, x_pos in [("Left", -80), ("Right", 80)]:
        display = editor_level_lib.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(x_pos, 60, 120)
        )
        if display:
            display.set_actor_label(f"HUD_{side}")
            mesh_comp = display.static_mesh_component
            cube_mesh = unreal.EditorAssetLibrary.load_asset('/Engine/BasicShapes/Cube')
            if cube_mesh:
                mesh_comp.set_static_mesh(cube_mesh)
                display.set_actor_scale3d(unreal.Vector(0.8, 0.1, 0.6))
                display.set_actor_rotation(unreal.Rotator(0, -45 if side == "Left" else 45, 0), False)  # Angled displays (teleport_physics=False)
            print(f"  ✅ {side} HUD display created")
    
    # VR Camera Position (where player's head will be)
    print("Creating VR camera reference...")
    camera_ref = editor_level_lib.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(0, -20, 120)
    )
    if camera_ref:
        camera_ref.set_actor_label("VR_CameraPosition")
        mesh_comp = camera_ref.static_mesh_component
        sphere_mesh = unreal.EditorAssetLibrary.load_asset('/Engine/BasicShapes/Sphere')
        if sphere_mesh:
            mesh_comp.set_static_mesh(sphere_mesh)
            camera_ref.set_actor_scale3d(unreal.Vector(0.3, 0.3, 0.3))  # Small reference sphere
        print("  ✅ VR camera reference created")
    
    print("\n" + "="*70)
    print("  ✅ COCKPIT GEOMETRY COMPLETE!")
    print("="*70)
    print("\n📊 Created:")
    print("  • Cockpit base platform")
    print("  • Pilot seat")
    print("  • Flight stick (left)")
    print("  • Throttle control (right)")
    print("  • Button panel (center)")
    print("  • 2 HUD displays (left/right)")
    print("  • VR camera reference")
    print("\n💡 Check the viewport to see the cockpit layout!")
    print("💡 All components use basic shapes - ready for art pass\n")

# Run the cockpit creation
create_cockpit_scene()
