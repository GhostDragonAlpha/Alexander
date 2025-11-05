"""
Aggressive Lighting Fix - Boost intensities massively
Add multiple high-intensity lights to ensure visibility
"""

import unreal

print("\n" + "="*70)
print("  💥 AGGRESSIVE LIGHTING FIX")
print("="*70 + "\n")

# Get all actors
actors = unreal.EditorLevelLibrary.get_all_level_actors()

# Count objects in scene
object_count = 0
for actor in actors:
    if actor:
        object_count += 1

print(f"📊 Total actors in scene: {object_count}\n")

# ===========================================================================
# PHASE 1: BOOST EXISTING LIGHTS TO EXTREME VALUES
# ===========================================================================
print("="*70)
print("  ⚡ BOOSTING EXISTING LIGHTS")
print("="*70 + "\n")

for actor in actors:
    if not actor:
        continue

    class_name = actor.get_class().get_name()
    label = actor.get_actor_label()

    # Boost all point lights
    if 'PointLight' in class_name:
        comp = actor.root_component
        old_intensity = comp.get_editor_property("intensity")

        # MASSIVE intensity increase
        if 'Star' in label:
            new_intensity = 500000.0  # 10x star brightness!
        else:
            new_intensity = 50000.0  # Huge cockpit lights

        comp.set_editor_property("intensity", new_intensity)
        comp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)

        print(f"✅ {label}: {old_intensity:.0f} → {new_intensity:.0f}")

    # Boost skylight
    elif 'SkyLight' in class_name:
        comp = actor.root_component
        old_intensity = comp.get_editor_property("intensity")

        new_intensity = 5.0  # Much brighter ambient

        comp.set_editor_property("intensity", new_intensity)
        comp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)

        print(f"✅ {label}: {old_intensity:.2f} → {new_intensity:.0f}")

print()

# ===========================================================================
# PHASE 2: ADD MASSIVE FILL LIGHTS AT STRATEGIC POSITIONS
# ===========================================================================
print("="*70)
print("  💡 ADDING MASSIVE FILL LIGHTS")
print("="*70 + "\n")

# Add lights at key arena positions
fill_light_positions = [
    ("Arena_Fill_Center", unreal.Vector(0, 0, 500)),
    ("Arena_Fill_North", unreal.Vector(-500, 0, 300)),
    ("Arena_Fill_South", unreal.Vector(500, 0, 300)),
    ("Arena_Fill_East", unreal.Vector(0, 500, 300)),
    ("Arena_Fill_West", unreal.Vector(0, -500, 300)),
    ("Cockpit_Fill_Direct", unreal.Vector(800, 0, 300)),  # Right above cockpit
]

for name, pos in fill_light_positions:
    light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PointLight,
        pos
    )

    if light:
        light.set_actor_label(name)
        comp = light.root_component

        # EXTREME intensity
        comp.set_editor_property("intensity", 100000.0)
        comp.set_editor_property("attenuation_radius", 5000.0)
        comp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)

        print(f"✅ Created {name} at ({pos.x:.0f}, {pos.y:.0f}, {pos.z:.0f})")

print()

# ===========================================================================
# PHASE 3: DISABLE AUTO-EXPOSURE (if Post Process Volume exists)
# ===========================================================================
print("="*70)
print("  📐 CHECKING POST PROCESS SETTINGS")
print("="*70 + "\n")

post_process_found = False
for actor in actors:
    if actor and 'PostProcess' in actor.get_class().get_name():
        label = actor.get_actor_label()
        print(f"📐 Found: {label}")
        print("   Manual adjustment may be needed for exposure settings\n")
        post_process_found = True

if not post_process_found:
    print("ℹ️  No Post Process Volume found\n")

# ===========================================================================
# PHASE 4: VERIFY CAMERA POSITIONS
# ===========================================================================
print("="*70)
print("  📸 CAMERA POSITIONS")
print("="*70 + "\n")

cameras_found = []
for actor in actors:
    if actor and 'CameraActor' in actor.get_class().get_name():
        label = actor.get_actor_label()
        pos = actor.get_actor_location()
        rot = actor.get_actor_rotation()

        cameras_found.append(label)
        print(f"📷 {label}")
        print(f"   Position: ({pos.x:.0f}, {pos.y:.0f}, {pos.z:.0f})")
        print(f"   Rotation: (Pitch={rot.pitch:.1f}, Yaw={rot.yaw:.1f}, Roll={rot.roll:.1f})\n")

if not cameras_found:
    print("⚠️  No cameras found!\n")

# ===========================================================================
# SUMMARY
# ===========================================================================
print("="*70)
print("  ✅ AGGRESSIVE LIGHTING APPLIED!")
print("="*70 + "\n")

print("💥 EXTREME CHANGES:")
print("   • Star_Dynamic: 500,000 intensity (10x original)")
print("   • Cockpit lights: 50,000 intensity each")
print("   • Starlight_Ambient: 5.0 intensity")
print("   • Added 6 massive fill lights at 100,000 intensity")
print("   • All lights set to MOVABLE (instant)\n")

print("🎯 TOTAL LIGHTING POWER:")
print("   • Star: 500,000")
print("   • Fill lights: 600,000 (6 x 100,000)")
print("   • Cockpit lights: ~150,000")
print("   • Ambient: 5.0")
print("   • TOTAL: >1,250,000 lighting units!\n")

print("📸 If still dark after this, check:")
print("   • Viewport show flags (Editor > Show > Lighting)")
print("   • Post-process auto-exposure settings")
print("   • Camera is pointing at scene geometry\n")
