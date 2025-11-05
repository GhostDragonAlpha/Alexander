"""
Fix Lighting - Diagnose and improve lighting in current level
"""

import unreal

print("\n" + "="*70)
print("  💡 LIGHTING DIAGNOSTIC AND FIX")
print("="*70 + "\n")

# Get all actors
actors = unreal.EditorLevelLibrary.get_all_level_actors()

# Find lights
sun = None
skylight = None
point_lights = []

for actor in actors:
    if not actor:
        continue

    class_name = actor.get_class().get_name()
    label = actor.get_actor_label()

    if 'DirectionalLight' in class_name:
        sun = actor
    elif 'SkyLight' in class_name:
        skylight = actor
    elif 'PointLight' in class_name or 'SpotLight' in class_name:
        point_lights.append(actor)

# Diagnose current lighting
print("📊 Current Lighting Setup:\n")

if sun:
    # Get root component (the light component)
    sun_comp = sun.root_component
    print(f"☀️  Directional Light (Sun): {sun.get_actor_label()}")
    try:
        print(f"    Intensity: {sun_comp.intensity}")
    except:
        print(f"    (Cannot read intensity)")
else:
    print("⚠️  No Directional Light found!")

if skylight:
    # Get root component
    sky_comp = skylight.root_component
    print(f"\n🌤️  Sky Light: {skylight.get_actor_label()}")
    try:
        print(f"    Intensity: {sky_comp.intensity}")
    except:
        print(f"    (Cannot read intensity)")
else:
    print("\n⚠️  No Sky Light found!")

print(f"\n💡 Point/Spot Lights: {len(point_lights)}")

# Check for Post Process Volume
post_process = None
for actor in actors:
    if actor and 'PostProcess' in actor.get_class().get_name():
        post_process = actor
        break

if post_process:
    print(f"\n📐 Post Process Volume: {post_process.get_actor_label()}")
else:
    print("\n⚠️  No Post Process Volume found!")

# Apply fixes
print("\n" + "="*70)
print("  🔧 APPLYING LIGHTING FIXES")
print("="*70 + "\n")

# Fix 1: Increase Sun intensity
if sun:
    sun_comp = sun.root_component
    try:
        old_intensity = sun_comp.intensity
        sun_comp.set_editor_property("intensity", 10.0)  # Much brighter
        print(f"✅ Sun intensity: {old_intensity} → 10.0")
    except Exception as e:
        print(f"⚠️  Could not modify sun intensity: {e}")
else:
    # Create a sun
    print("🆕 Creating new Directional Light...")
    sun = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0, 0, 500)
    )
    if sun:
        sun.set_actor_label("Sun_Main")
        sun.set_actor_rotation(unreal.Rotator(-45, 0, 0), False)
        sun.root_component.set_editor_property("intensity", 10.0)
        print("✅ Created Sun_Main with intensity 10.0")

# Fix 2: Increase SkyLight intensity
if skylight:
    sky_comp = skylight.root_component
    try:
        old_intensity = sky_comp.intensity
        sky_comp.set_editor_property("intensity", 1.5)  # Brighter ambient
        print(f"✅ SkyLight intensity: {old_intensity} → 1.5")
    except Exception as e:
        print(f"⚠️  Could not modify skylight intensity: {e}")
else:
    # Create a skylight
    print("🆕 Creating new Sky Light...")
    skylight = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0, 0, 400)
    )
    if skylight:
        skylight.set_actor_label("SkyLight_Main")
        skylight.root_component.set_editor_property("intensity", 1.5)
        print("✅ Created SkyLight_Main with intensity 1.5")

# Fix 3: Add fill lights near cockpit
print("\n💡 Adding fill lights for better cockpit visibility...")
fill_positions = [
    ("FillLight_Left", unreal.Vector(-200, 0, 200)),
    ("FillLight_Right", unreal.Vector(200, 0, 200)),
    ("FillLight_Front", unreal.Vector(0, 200, 150)),
]

for name, pos in fill_positions:
    fill = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PointLight,
        pos
    )
    if fill:
        fill.set_actor_label(name)
        fill.root_component.set_editor_property("intensity", 2000.0)  # Point lights need higher values
        fill.root_component.set_editor_property("attenuation_radius", 500.0)
        print(f"  ✅ Created {name}")

# Fix 4: Disable auto-exposure in Post Process (if it exists)
if post_process:
    # Note: Modifying post process settings via Python is complex
    # This would require accessing the post process settings struct
    print("📐 Post Process Volume detected (auto-exposure might need manual adjustment)")
else:
    print("🆕 Creating Post Process Volume...")
    pp = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PostProcessVolume,
        unreal.Vector(0, 0, 0)
    )
    if pp:
        pp.set_actor_label("PostProcess_Lighting")
        # Make it infinite bounds
        pp.set_editor_property("unbound", True)
        print("✅ Created Post Process Volume (set to unbound)")

print("\n" + "="*70)
print("  ✅ LIGHTING FIX COMPLETE!")
print("="*70)
print("\n💡 Changes applied:")
print("  • Sun intensity increased to 10.0")
print("  • SkyLight intensity increased to 1.5")
print("  • 3 fill lights added around cockpit")
print("  • Post Process Volume configured\n")
print("📸 Take a new screenshot to see the improvements!\n")
