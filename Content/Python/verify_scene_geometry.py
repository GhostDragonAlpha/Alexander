"""
Verify Scene Geometry - Check what visible objects exist
"""

import unreal

print("\n" + "="*70)
print("  🔍 SCENE GEOMETRY VERIFICATION")
print("="*70 + "\n")

# Get all actors
actors = unreal.EditorLevelLibrary.get_all_level_actors()

# Categorize actors
static_meshes = []
lights = []
cameras = []
other_actors = []

for actor in actors:
    if not actor:
        continue

    class_name = actor.get_class().get_name()
    label = actor.get_actor_label()

    if 'StaticMeshActor' in class_name:
        static_meshes.append((label, actor))
    elif any(light_type in class_name for light_type in ['Light', 'SkyLight']):
        lights.append((label, actor))
    elif 'CameraActor' in class_name:
        cameras.append((label, actor))
    else:
        other_actors.append((label, actor))

print(f"📊 SCENE INVENTORY:")
print(f"   Static Meshes (visible geometry): {len(static_meshes)}")
print(f"   Lights: {len(lights)}")
print(f"   Cameras: {len(cameras)}")
print(f"   Other: {len(other_actors)}\n")

# ===========================================================================
# LIST ALL STATIC MESHES (VISIBLE GEOMETRY)
# ===========================================================================
print("="*70)
print("  🎭 STATIC MESHES (VISIBLE GEOMETRY)")
print("="*70 + "\n")

if not static_meshes:
    print("⚠️  NO STATIC MESHES FOUND!")
    print("   This means there's NOTHING VISIBLE in the scene to light!\n")
else:
    print(f"Found {len(static_meshes)} static mesh actors:\n")

    for label, actor in static_meshes:
        pos = actor.get_actor_location()
        scale = actor.get_actor_scale3d()

        # Check if it has a mesh assigned
        mesh_comp = actor.static_mesh_component
        mesh = mesh_comp.get_editor_property("static_mesh")

        print(f"  • {label}")
        print(f"    Position: ({pos.x:.0f}, {pos.y:.0f}, {pos.z:.0f})")
        print(f"    Scale: ({scale.x:.1f}, {scale.y:.1f}, {scale.z:.1f})")

        if mesh:
            mesh_name = mesh.get_name()
            print(f"    Mesh: {mesh_name}")
        else:
            print(f"    Mesh: ⚠️  NONE (empty actor!)")

        # Check visibility
        is_visible = mesh_comp.get_editor_property("visible")
        print(f"    Visible: {is_visible}\n")

# ===========================================================================
# LIST ALL LIGHTS
# ===========================================================================
print("="*70)
print("  💡 LIGHTS")
print("="*70 + "\n")

for label, actor in lights:
    pos = actor.get_actor_location()
    comp = actor.root_component

    try:
        intensity = comp.get_editor_property("intensity")
        mobility = comp.get_editor_property("mobility")
        visible = comp.get_editor_property("visible")

        print(f"  • {label}")
        print(f"    Position: ({pos.x:.0f}, {pos.y:.0f}, {pos.z:.0f})")
        print(f"    Intensity: {intensity:.0f}")
        print(f"    Mobility: {mobility}")
        print(f"    Visible: {visible}\n")
    except:
        print(f"  • {label} (cannot read properties)\n")

# ===========================================================================
# DIAGNOSE THE DARKNESS ISSUE
# ===========================================================================
print("="*70)
print("  🔬 DARKNESS DIAGNOSIS")
print("="*70 + "\n")

print("🔍 CHECKING POSSIBLE ISSUES:\n")

# Check 1: Do we have geometry?
if len(static_meshes) == 0:
    print("❌ ISSUE 1: NO GEOMETRY!")
    print("   The scene has no static meshes to light.")
    print("   Solution: Add floor, walls, or other geometry\n")
else:
    print(f"✅ Check 1: {len(static_meshes)} static meshes present\n")

# Check 2: Do we have lights?
if len(lights) == 0:
    print("❌ ISSUE 2: NO LIGHTS!")
    print("   The scene has no light sources.\n")
else:
    print(f"✅ Check 2: {len(lights)} lights present\n")

# Check 3: Are lights movable?
non_movable_lights = 0
for label, actor in lights:
    comp = actor.root_component
    try:
        mobility = comp.get_editor_property("mobility")
        if mobility != unreal.ComponentMobility.MOVABLE:
            non_movable_lights += 1
    except:
        pass

if non_movable_lights > 0:
    print(f"⚠️  Warning: {non_movable_lights} lights are not MOVABLE")
    print("   They may need lighting to be built (F7)\n")
else:
    print("✅ Check 3: All lights are MOVABLE (dynamic)\n")

# Check 4: Scene bounds
if static_meshes:
    # Calculate scene bounding box
    min_x = min_y = min_z = float('inf')
    max_x = max_y = max_z = float('-inf')

    for label, actor in static_meshes:
        pos = actor.get_actor_location()
        min_x = min(min_x, pos.x)
        min_y = min(min_y, pos.y)
        min_z = min(min_z, pos.z)
        max_x = max(max_x, pos.x)
        max_y = max(max_y, pos.y)
        max_z = max(max_z, pos.z)

    print(f"📦 Scene Bounding Box:")
    print(f"   X: {min_x:.0f} to {max_x:.0f}")
    print(f"   Y: {min_y:.0f} to {max_y:.0f}")
    print(f"   Z: {min_z:.0f} to {max_z:.0f}\n")

print("="*70)
print("  🎯 RECOMMENDATION")
print("="*70 + "\n")

if len(static_meshes) == 0:
    print("⚠️  CRITICAL: Scene has no visible geometry!")
    print("   Cameras are pointing at empty space.")
    print("   Need to add arena floor, walls, and objects.\n")
else:
    print("📝 Scene has geometry and lights.")
    print("   If still dark, possible causes:")
    print("   1. Post-process auto-exposure crushing blacks")
    print("   2. Viewport show flags disabled")
    print("   3. Materials are black/unlit")
    print("   4. Camera exposure settings\n")
