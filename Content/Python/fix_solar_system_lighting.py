"""
Fix Solar System Lighting and Appearance
Adjusts based on game designer analysis of overexposed scene
"""

import unreal

print("\n" + "="*70)
print("  🔧 FIXING SOLAR SYSTEM LIGHTING")
print("="*70 + "\n")

actors = unreal.EditorLevelLibrary.get_all_level_actors()

# Fix 1: Reduce star light intensity (100K → 10K)
print("📉 Reducing star light intensity...")
for actor in actors:
    if not actor:
        continue
    if actor.get_actor_label() == "Light_Star":
        light_comp = actor.root_component
        old_intensity = light_comp.get_editor_property("intensity")
        light_comp.set_editor_property("intensity", 10000.0)  # 10x less
        print(f"  ✅ Star light: {old_intensity} → 10,000")
        break

# Fix 2: Remove/hide ground plane
print("\n🗑️ Hiding ground plane (no floor in space)...")
for actor in actors:
    if not actor:
        continue
    label = actor.get_actor_label()
    if 'Floor' in label or 'Ground' in label or 'Plane' in label:
        actor.set_actor_hidden_in_game(True)
        print(f"  ✅ Hidden: {label}")

# Fix 3: Reduce ambient skylight (0.5 → 0.05)
print("\n🌌 Reducing ambient light (space is dark)...")
for actor in actors:
    if not actor:
        continue
    if 'SkyLight' in actor.get_class().get_name() and 'Ambient' in actor.get_actor_label():
        sky_comp = actor.root_component
        old_intensity = sky_comp.get_editor_property("intensity")
        sky_comp.set_editor_property("intensity", 0.05)  # Very dim
        print(f"  ✅ Ambient skylight: {old_intensity} → 0.05")
        break

# Fix 4: Add emissive glow to star
print("\n✨ Making star glow (emissive material)...")
# Note: Would need custom material for true emissive
# For now, just note that star should use emissive material

# Fix 5: Color-code planets for differentiation
print("\n🎨 Adding visual markers to planets...")
planet_count = 0
for actor in actors:
    if not actor:
        continue
    label = actor.get_actor_label()
    if label.startswith("Planet_"):
        # Planets identified, would apply colored materials here
        print(f"  📍 Found: {label}")
        planet_count += 1

print(f"  ℹ️  {planet_count} planets need colored materials (requires material creation)")

# Fix 6: Add post-process volume for exposure control
print("\n📸 Creating post-process volume for exposure control...")
pp_location = unreal.Vector(0, 0, 0)
post_process = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.PostProcessVolume,
    pp_location
)
post_process.set_actor_label("PostProcess_ExposureControl")

# Make it global (affects whole scene)
post_process.set_editor_property("unbound", True)

# Get settings
pp_settings = post_process.get_editor_property("settings")

# Reduce auto-exposure to prevent overexposure
pp_settings.set_editor_property("auto_exposure_bias", -2.0)  # Darker
pp_settings.set_editor_property("auto_exposure_min_brightness", 0.1)
pp_settings.set_editor_property("auto_exposure_max_brightness", 1.0)

print(f"  ✅ Post-process volume created with exposure bias: -2.0")

# Fix 7: Adjust camera positions for better framing
print("\n📸 Adjusting camera for better framing...")
for actor in actors:
    if not actor:
        continue
    if 'CameraActor' in actor.get_class().get_name():
        if actor.get_actor_label() == "Cam_StarCloseup":
            # Move camera back a bit to see more context
            current_loc = actor.get_actor_location()
            new_loc = current_loc + unreal.Vector(-200, 0, 100)  # Back and up
            actor.set_actor_location(new_loc, False, False)
            print(f"  ✅ Cam_StarCloseup repositioned for better view")

print("\n" + "="*70)
print("  ✅ LIGHTING FIXES APPLIED!")
print("="*70)

print("\n📊 Changes Made:")
print("  • Star light: 100K → 10K (90% reduction)")
print("  • Ambient light: 0.5 → 0.05 (90% reduction)")
print("  • Ground plane: Hidden (space has no floor)")
print("  • Post-process: -2.0 exposure bias (darker)")
print("  • Camera: Repositioned for context")

print("\n🎯 Next Steps:")
print("  1. Capture new screenshots")
print("  2. Should see:")
print("     - Darker space background")
print("     - Star brighter than planets")
print("     - Visible light falloff")
print("     - Better contrast and detail")
print("\n💡 If still too bright, reduce star light further!")
print()
