"""
Dual Lighting System for AI Vision Test Arena
- Ambient Starlight: SkyLight for faint glow under stars
- Directional Sun: Bright daylight for rotating planets
Covers both day and night lighting scenarios
"""

import unreal

print("\n" + "="*70)
print("  💫 DUAL LIGHTING SYSTEM SETUP")
print("="*70 + "\n")

# Get all actors
actors = unreal.EditorLevelLibrary.get_all_level_actors()

# Find existing lights
existing_sun = None
existing_skylight = None
other_lights = []

for actor in actors:
    if not actor:
        continue

    class_name = actor.get_class().get_name()
    label = actor.get_actor_label()

    if 'DirectionalLight' in class_name:
        existing_sun = actor
    elif 'SkyLight' in class_name:
        existing_skylight = actor
    elif 'PointLight' in class_name or 'SpotLight' in class_name:
        if 'Cockpit' not in label:  # Keep cockpit lights
            other_lights.append(actor)

print("📊 Current Lighting Analysis:")
print(f"  • Directional Lights (Sun): {1 if existing_sun else 0}")
print(f"  • Sky Lights (Starlight): {1 if existing_skylight else 0}")
print(f"  • Other Lights: {len(other_lights)}\n")

# ===========================================================================
# PHASE 1: CLEAN UP OLD LIGHTS (except cockpit)
# ===========================================================================
print("="*70)
print("  🧹 CLEANING OLD LIGHTING")
print("="*70 + "\n")

removed_count = 0

# Remove old sun if exists
if existing_sun:
    label = existing_sun.get_actor_label()
    existing_sun.destroy_actor()
    print(f"  🗑️  Removed old sun: {label}")
    removed_count += 1

# Remove old skylight if exists
if existing_skylight:
    label = existing_skylight.get_actor_label()
    existing_skylight.destroy_actor()
    print(f"  🗑️  Removed old skylight: {label}")
    removed_count += 1

# Remove other non-cockpit lights
for light in other_lights:
    label = light.get_actor_label()
    light.destroy_actor()
    print(f"  🗑️  Removed: {label}")
    removed_count += 1

if removed_count > 0:
    print(f"\n✅ Removed {removed_count} old lights\n")
else:
    print("  ℹ️  No old lights to remove\n")

# ===========================================================================
# PHASE 2: CREATE AMBIENT STARLIGHT (SkyLight)
# ===========================================================================
print("="*70)
print("  💫 CREATING AMBIENT STARLIGHT")
print("="*70 + "\n")

skylight = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.SkyLight,
    unreal.Vector(0, 0, 500)
)

if skylight:
    skylight.set_actor_label("Starlight_Ambient")
    sky_comp = skylight.root_component

    # Starlight settings: Very subtle ambient glow
    sky_comp.set_editor_property("intensity", 0.3)  # Faint starlight glow
    sky_comp.set_editor_property("source_type", unreal.SkyLightSourceType.SLS_CAPTURED_SCENE)

    print("✅ Starlight_Ambient created")
    print("   • Type: Sky Light (ambient)")
    print("   • Intensity: 0.3 (faint starlight glow)")
    print("   • Purpose: Provides base visibility in space\n")

# ===========================================================================
# PHASE 3: CREATE DYNAMIC STAR (Point Light)
# ===========================================================================
print("="*70)
print("  ⭐ CREATING DYNAMIC STAR")
print("="*70 + "\n")

# Position star at a distance (simulates distant star in space)
star_position = unreal.Vector(2000, 0, 1000)  # Far away, elevated

star = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.PointLight,
    star_position
)

if star:
    star.set_actor_label("Star_Dynamic")
    star_comp = star.root_component

    # Star settings: Very bright point light with huge radius
    star_comp.set_editor_property("intensity", 50000.0)  # Very bright (distant star)
    star_comp.set_editor_property("attenuation_radius", 10000.0)  # Huge radius (star range)
    star_comp.set_editor_property("use_inverse_squared_falloff", False)  # Linear falloff for space

    print("✅ Star_Dynamic created")
    print("   • Type: Point Light (radiates from source)")
    print("   • Position: 2000 units away (distant star)")
    print("   • Intensity: 50,000 (star brightness)")
    print("   • Radius: 10,000 units (far reach)")
    print("   • Benefit: No code needed - auto-lights from all angles!")
    print("   • Purpose: Simulates distant star in space\n")

# ===========================================================================
# PHASE 4: VERIFY COCKPIT LIGHTS INTACT
# ===========================================================================
print("="*70)
print("  💡 VERIFYING COCKPIT LIGHTS")
print("="*70 + "\n")

# Re-scan for cockpit lights
actors = unreal.EditorLevelLibrary.get_all_level_actors()
cockpit_lights = []

for actor in actors:
    if not actor:
        continue

    label = actor.get_actor_label()
    if 'Cockpit_Light' in label or 'FillLight' in label:
        cockpit_lights.append(label)

print(f"✅ Cockpit lights preserved: {len(cockpit_lights)}")
for light_name in cockpit_lights:
    print(f"   • {light_name}")

# ===========================================================================
# PHASE 5: LIGHTING SYSTEM SUMMARY
# ===========================================================================
print("\n" + "="*70)
print("  ✅ DUAL LIGHTING SYSTEM COMPLETE!")
print("="*70 + "\n")

print("🌌 LIGHTING ARCHITECTURE:")
print("\n1. AMBIENT STARLIGHT (Starlight_Ambient)")
print("   • Sky Light at intensity 0.3")
print("   • Provides faint glow for everything in space")
print("   • Always active - represents distant starfield")
print("   • Covers: Base visibility in all scenarios\n")

print("2. DYNAMIC STAR (Star_Dynamic)")
print("   • Point Light at intensity 50,000")
print("   • Positioned 2000 units away (distant star)")
print("   • 10,000 unit radius (massive reach)")
print("   • NO CODE NEEDED - radiates naturally from point!")
print("   • Simulates nearby star in solar system")
print("   • Covers: Planetary lighting, dynamic shadows\n")

print("3. COCKPIT LIGHTS (Preserved)")
print(f"   • {len(cockpit_lights)} dedicated cockpit lights")
print("   • Local illumination for controls and HUD")
print("   • Always visible to pilot\n")

print("🎮 LIGHTING SCENARIOS:")
print("   • Deep space: Starlight (0.3) + cockpit lights")
print("   • Near star: Starlight + Star_Dynamic (50k) - bright!")
print("   • Planet shadow: Starlight + partial star light")
print("   • Cockpit: Always locally lit\n")

print("⭐ DYNAMIC STAR BENEFITS:")
print("   • NO rotation code needed (point light works all angles)")
print("   • Realistic space lighting (point source)")
print("   • Natural shadows and falloff")
print("   • Player-agnostic (lights everything equally)")
print("   • Can move star to simulate orbital position\n")

print("📸 Ready to capture new screenshots with proper lighting!\n")
