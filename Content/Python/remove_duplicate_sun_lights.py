"""
Remove Duplicate Sun and SkyLight Actors
Unreal only supports ONE directional light as primary sun
"""

import unreal

print("\n" + "="*70)
print("  🧹 REMOVING DUPLICATE SUN/SKYLIGHT ACTORS")
print("="*70 + "\n")

# Get all actors
actors = unreal.EditorLevelLibrary.get_all_level_actors()

# Find directional lights and skylights
directional_lights = []
skylights = []

for actor in actors:
    if not actor:
        continue

    class_name = actor.get_class().get_name()
    label = actor.get_actor_label()

    if class_name == 'DirectionalLight':
        directional_lights.append((label, actor))
    elif class_name == 'SkyLight':
        skylights.append((label, actor))

print(f"Found {len(directional_lights)} DirectionalLight actors")
print(f"Found {len(skylights)} SkyLight actors\n")

# Keep only the FIRST of each, delete the rest
deleted_count = 0

# Handle DirectionalLights
if len(directional_lights) > 1:
    print(f"⚠️  Multiple DirectionalLights found! Keeping first, removing {len(directional_lights)-1} duplicates")
    for i, (label, actor) in enumerate(directional_lights):
        if i == 0:
            print(f"  ✅ Keeping: {label}")
        else:
            print(f"  ❌ Deleting: {label}")
            actor.destroy_actor()
            deleted_count += 1
    print()

# Handle SkyLights
if len(skylights) > 1:
    print(f"⚠️  Multiple SkyLights found! Keeping first, removing {len(skylights)-1} duplicates")
    for i, (label, actor) in enumerate(skylights):
        if i == 0:
            print(f"  ✅ Keeping: {label}")
        else:
            print(f"  ❌ Deleting: {label}")
            actor.destroy_actor()
            deleted_count += 1
    print()

print("="*70)
print(f"  ✅ CLEANUP COMPLETE - Removed {deleted_count} duplicate lights")
print("="*70)
print()
print("📊 Result: 1 DirectionalLight + 1 SkyLight (as it should be)")
print("⚠️  Warning about competing directional lights should be GONE")
print()
