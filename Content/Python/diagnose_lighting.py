"""
Diagnose Lighting Issues in AI Vision Test Arena
Check what lights exist and their settings
"""

import unreal

print("\n" + "="*70)
print("  🔍 LIGHTING DIAGNOSTIC")
print("="*70 + "\n")

# Get all actors
actors = unreal.EditorLevelLibrary.get_all_level_actors()

# Find all lights
lights_found = {
    'DirectionalLight': [],
    'PointLight': [],
    'SpotLight': [],
    'SkyLight': []
}

for actor in actors:
    if not actor:
        continue

    class_name = actor.get_class().get_name()
    label = actor.get_actor_label()

    for light_type in lights_found.keys():
        if light_type in class_name:
            lights_found[light_type].append((label, actor))

print("📊 LIGHTS IN SCENE:\n")

total_lights = 0
for light_type, light_list in lights_found.items():
    if light_list:
        print(f"{light_type}: {len(light_list)}")
        for label, actor in light_list:
            total_lights += 1
            comp = actor.root_component
            pos = actor.get_actor_location()

            print(f"  • {label}")
            print(f"    Position: ({pos.x:.0f}, {pos.y:.0f}, {pos.z:.0f})")

            # Try to get intensity
            try:
                intensity = comp.get_editor_property("intensity")
                print(f"    Intensity: {intensity}")
            except:
                print(f"    Intensity: (cannot read)")

            # Try to get mobility (static lights need to be built)
            try:
                mobility = comp.get_editor_property("mobility")
                print(f"    Mobility: {mobility}")
            except:
                pass

            print()

if total_lights == 0:
    print("⚠️  NO LIGHTS FOUND IN SCENE!\n")
else:
    print(f"Total lights: {total_lights}\n")

# Check lighting build status
print("="*70)
print("  💡 LIGHTING BUILD INFO")
print("="*70 + "\n")

print("⚠️  IMPORTANT: Unreal Engine lighting types:")
print("  • Static lights: Need to be BUILT (F7 or Build Lighting)")
print("  • Stationary lights: Can move, shadows need building")
print("  • Movable lights: Fully dynamic, no build needed\n")

print("🔧 SOLUTION: Set all lights to MOVABLE for instant lighting")
print("   (No build required, works immediately)\n")
