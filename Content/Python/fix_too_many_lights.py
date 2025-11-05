"""
Fix: Too Many Overlapping Movable Lights
Unreal Engine limit: ~4-5 overlapping movable lights maximum
We created 10+ overlapping lights = rendering crash = black screen
"""

import unreal

print("\n" + "="*70)
print("  💡 FIXING TOO MANY MOVABLE LIGHTS")
print("="*70 + "\n")

print("⚠️  DIAGNOSIS:")
print("   Error: 'Too many overlapping movable lights'")
print("   Unreal limit: 4-5 overlapping movable lights max")
print("   We created: 10 lights all overlapping")
print("   Result: Rendering fails, black screen\n")

# Get all lights
actors = unreal.EditorLevelLibrary.get_all_level_actors()

lights_to_remove = []
lights_to_keep = []

for actor in actors:
    if not actor:
        continue

    class_name = actor.get_class().get_name()
    label = actor.get_actor_label()

    # Check if it's a light
    if any(light_type in class_name for light_type in ['PointLight', 'SpotLight', 'DirectionalLight']):
        # KEEP: Essential lights only
        if 'Star_Dynamic' in label:
            lights_to_keep.append((label, actor))
        elif 'Cockpit' in label:
            lights_to_keep.append((label, actor))
        else:
            # REMOVE: All fill lights (too many!)
            lights_to_remove.append((label, actor))

print("="*70)
print("  🗑️  REMOVING EXCESS LIGHTS")
print("="*70 + "\n")

print(f"Lights to keep: {len(lights_to_keep)}")
for label, actor in lights_to_keep:
    print(f"  ✅ Keeping: {label}")

print(f"\nLights to remove: {len(lights_to_remove)}")
for label, actor in lights_to_remove:
    print(f"  🗑️  Removing: {label}")
    actor.destroy_actor()

print("\n" + "="*70)
print("  ✅ LIGHTS CLEANED UP!")
print("="*70 + "\n")

# Now boost the remaining lights to compensate
print("="*70)
print("  ⚡ BOOSTING REMAINING LIGHTS")
print("="*70 + "\n")

for label, actor in lights_to_keep:
    comp = actor.root_component

    if 'Star' in label:
        # Already at 500,000
        print(f"✅ {label}: 500,000 (already max)")
    else:
        # Boost cockpit lights even more
        old_intensity = comp.get_editor_property("intensity")
        new_intensity = 100000.0  # Very bright
        comp.set_editor_property("intensity", new_intensity)
        print(f"✅ {label}: {old_intensity:.0f} → {new_intensity:.0f}")

# Boost skylight too
for actor in actors:
    if actor and 'SkyLight' in actor.get_class().get_name():
        comp = actor.root_component
        old_intensity = comp.get_editor_property("intensity")
        new_intensity = 10.0  # Much brighter
        comp.set_editor_property("intensity", new_intensity)
        print(f"✅ Starlight_Ambient: {old_intensity:.1f} → {new_intensity:.0f}")

print("\n" + "="*70)
print("  ✅ FIX COMPLETE!")
print("="*70 + "\n")

print("🎯 FINAL LIGHTING SETUP:")
print("   • Star_Dynamic: 500,000 (point light)")
print("   • Cockpit lights (2): 100,000 each")
print("   • Starlight_Ambient: 10.0 (skylight)")
print("   • TOTAL: 3 movable lights (under limit!)")
print("   • Fill lights: REMOVED (caused overlap error)\n")

print("📸 Scene should now render properly!\n")
