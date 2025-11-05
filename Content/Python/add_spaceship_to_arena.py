"""
Add Spaceship Cockpit to AI Vision Test Arena
Combines cockpit geometry + flight physics for testing with AI vision system
"""

import unreal

print("\n" + "="*70)
print("  🚀 ADDING SPACESHIP TO TEST ARENA")
print("="*70 + "\n")

# Position in Zone 4 (Iteration Comparison Zone) - perfect for testing!
COCKPIT_POSITION = unreal.Vector(800, 0, 150)  # Zone 4, elevated

print(f"📍 Cockpit will be positioned at: {COCKPIT_POSITION}")
print("   (Zone 4 - Iteration Comparison Zone)")
print("   Visible from Cam1, Cam2, Cam5\n")

# ===========================================================================
# 1. BUILD COCKPIT GEOMETRY
# ===========================================================================
print("="*70)
print("  🔨 BUILDING COCKPIT GEOMETRY")
print("="*70 + "\n")

cockpit_actors = []

# Cockpit Base Platform
print("Creating cockpit base...")
base = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.StaticMeshActor,
    COCKPIT_POSITION
)
if base:
    base.set_actor_label("Spaceship_Base")
    mesh_comp = base.static_mesh_component
    cube_mesh = unreal.EditorAssetLibrary.load_asset('/Engine/BasicShapes/Cube')
    if cube_mesh:
        mesh_comp.set_static_mesh(cube_mesh)
        base.set_actor_scale3d(unreal.Vector(3, 2, 0.1))  # Platform
    cockpit_actors.append(base)
    print("  ✅ Spaceship base created")

# Pilot Seat
print("Creating pilot seat...")
seat_pos = COCKPIT_POSITION + unreal.Vector(0, -30, 40)
seat = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.StaticMeshActor,
    seat_pos
)
if seat:
    seat.set_actor_label("Pilot_Seat")
    mesh_comp = seat.static_mesh_component
    cube_mesh = unreal.EditorAssetLibrary.load_asset('/Engine/BasicShapes/Cube')
    if cube_mesh:
        mesh_comp.set_static_mesh(cube_mesh)
        seat.set_actor_scale3d(unreal.Vector(0.7, 0.7, 0.7))
    cockpit_actors.append(seat)
    print("  ✅ Pilot seat created")

# Flight Stick (Left - PHYSICS ENABLED)
print("Creating flight stick (physics-enabled)...")
stick_pos = COCKPIT_POSITION + unreal.Vector(-30, 15, 60)
stick = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.StaticMeshActor,
    stick_pos
)
if stick:
    stick.set_actor_label("FlightStick")
    mesh_comp = stick.static_mesh_component
    cylinder_mesh = unreal.EditorAssetLibrary.load_asset('/Engine/BasicShapes/Cylinder')
    if cylinder_mesh:
        mesh_comp.set_static_mesh(cylinder_mesh)
        stick.set_actor_scale3d(unreal.Vector(0.08, 0.08, 0.4))
        stick.set_actor_rotation(unreal.Rotator(15, 0, 0), False)

        # PHASE 3: Enable physics for VR interaction
        mesh_comp.set_simulate_physics(True)
        mesh_comp.set_enable_gravity(False)  # Space!
        mesh_comp.set_linear_damping(3.0)
        mesh_comp.set_angular_damping(3.0)

    cockpit_actors.append(stick)
    print("  ✅ Flight stick created (physics + VR ready)")

# Throttle (Right - PHYSICS ENABLED)
print("Creating throttle (physics-enabled)...")
throttle_pos = COCKPIT_POSITION + unreal.Vector(30, 15, 60)
throttle = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.StaticMeshActor,
    throttle_pos
)
if throttle:
    throttle.set_actor_label("Throttle")
    mesh_comp = throttle.static_mesh_component
    cube_mesh = unreal.EditorAssetLibrary.load_asset('/Engine/BasicShapes/Cube')
    if cube_mesh:
        mesh_comp.set_static_mesh(cube_mesh)
        throttle.set_actor_scale3d(unreal.Vector(0.25, 0.15, 0.7))

        # PHASE 3: Enable physics for VR interaction
        mesh_comp.set_simulate_physics(True)
        mesh_comp.set_enable_gravity(False)
        mesh_comp.set_linear_damping(3.0)
        mesh_comp.set_angular_damping(3.0)

    cockpit_actors.append(throttle)
    print("  ✅ Throttle created (physics + VR ready)")

# Button Panel (Center, angled)
print("Creating button panel...")
panel_pos = COCKPIT_POSITION + unreal.Vector(0, 60, 80)
panel = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.StaticMeshActor,
    panel_pos
)
if panel:
    panel.set_actor_label("ButtonPanel")
    mesh_comp = panel.static_mesh_component
    cube_mesh = unreal.EditorAssetLibrary.load_asset('/Engine/BasicShapes/Cube')
    if cube_mesh:
        mesh_comp.set_static_mesh(cube_mesh)
        panel.set_actor_scale3d(unreal.Vector(1.5, 0.1, 0.8))
        panel.set_actor_rotation(unreal.Rotator(-25, 0, 0), False)
    cockpit_actors.append(panel)
    print("  ✅ Button panel created")

# VR Camera Reference (player head position)
print("Creating VR camera reference...")
camera_pos = COCKPIT_POSITION + unreal.Vector(0, -10, 110)
camera_ref = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.StaticMeshActor,
    camera_pos
)
if camera_ref:
    camera_ref.set_actor_label("VR_CameraPosition")
    mesh_comp = camera_ref.static_mesh_component
    sphere_mesh = unreal.EditorAssetLibrary.load_asset('/Engine/BasicShapes/Sphere')
    if sphere_mesh:
        mesh_comp.set_static_mesh(sphere_mesh)
        camera_ref.set_actor_scale3d(unreal.Vector(0.25, 0.25, 0.25))
    cockpit_actors.append(camera_ref)
    print("  ✅ VR camera reference created")

# ===========================================================================
# 2. ADD COCKPIT LIGHTING
# ===========================================================================
print("\n"  + "="*70)
print("  💡 ADDING COCKPIT LIGHTING")
print("="*70 + "\n")

light_positions = [
    ("Cockpit_Light_Front", COCKPIT_POSITION + unreal.Vector(0, 150, 200)),
    ("Cockpit_Light_Above", COCKPIT_POSITION + unreal.Vector(0, 0, 300)),
]

for name, pos in light_positions:
    light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PointLight,
        pos
    )
    if light:
        light.set_actor_label(name)
        light.root_component.set_editor_property("intensity", 3000.0)
        light.root_component.set_editor_property("attenuation_radius", 600.0)
        print(f"  ✅ Created {name}")

# ===========================================================================
# 3. CREATE FLIGHT PHYSICS PLACEHOLDER
# ===========================================================================
print("\n" + "="*70)
print("  🎯 PHASE 4: FLIGHT PHYSICS PREPARATION")
print("="*70 + "\n")

print("📝 Flight Physics Integration:")
print("  ✅ Physics enabled on FlightStick and Throttle")
print("  ✅ Zero gravity (space environment)")
print("  ✅ Damping configured for smooth control")
print("  📋 Next: Implement flight physics system that reads:")
print("     - FlightStick position → Pitch/Roll/Yaw")
print("     - Throttle position → Forward/Backward thrust")
print("     - Apply forces to spaceship")
print("     - Integrate with VR motion controllers\n")

# ===========================================================================
# SUMMARY
# ===========================================================================
print("="*70)
print("  ✅ SPACESHIP ADDED TO TEST ARENA!")
print("="*70 + "\n")

print(f"📊 Created {len(cockpit_actors)} cockpit components")
print(f"📍 Position: Zone 4 (Iteration Comparison)")
print(f"💡 Added 2 dedicated lights")
print(f"🎮 Physics-enabled controls ready for VR")
print(f"\n📸 Visible from cameras:")
print(f"  • Cam1_Overview_North - Full arena view")
print(f"  • Cam2_Detail_East - Side view of cockpit")
print(f"  • Cam5_Isometric - Angled view")
print(f"\n🎯 Next Steps:")
print(f"  1. Capture screenshots from test cameras")
print(f"  2. Implement flight physics (Phase 4)")
print(f"  3. Use AI to test and validate controls")
print(f"  4. Iterate based on AI feedback\n")
