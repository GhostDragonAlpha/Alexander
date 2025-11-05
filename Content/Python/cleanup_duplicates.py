"""
Clean Up Duplicate Actors in Solar System Level
Keeps only the required number of each actor type
"""

import unreal
from collections import defaultdict

print("\n" + "="*70)
print("  🧹 CLEANING UP SOLAR SYSTEM LEVEL")
print("="*70 + "\n")

# Define the expected solar system actors and their counts
EXPECTED_ACTORS = {
    # Stars
    'Star': 1,
    'Sun': 1,

    # Planets
    'Mercury': 1,
    'Venus': 1,
    'Earth': 1,
    'Mars': 1,
    'Jupiter': 1,
    'Saturn': 1,
    'Uranus': 1,
    'Neptune': 1,

    # Lighting
    'Sun_PointLight': 1,
    'SunLight': 1,

    # Effects
    'SunStorms_Effect': 1,
}

# Get all actors
actors = unreal.EditorLevelLibrary.get_all_level_actors()

print(f"📊 Total actors in level: {len(actors)}\n")

# Group actors by label
actor_groups = defaultdict(list)
essential_types = ['WorldSettings', 'DefaultPhysicsVolume', 'LevelBounds', 'NavigationDataChunk',
                   'PlayerStart', 'SkyLight', 'ExponentialHeightFog', 'PostProcessVolume']

for actor in actors:
    if not actor:
        continue

    # Skip essential actors
    actor_class_name = actor.get_class().get_name()
    if any(essential in actor_class_name for essential in essential_types):
        continue

    label = actor.get_actor_label()
    actor_groups[label].append(actor)

# Find duplicates and unexpected actors
duplicates_to_remove = []
cleanup_report = []

print("🔍 Analyzing actors against expected solar system configuration:\n")

for label, actor_list in sorted(actor_groups.items()):
    count = len(actor_list)
    expected_count = EXPECTED_ACTORS.get(label, 0)

    if expected_count == 0:
        # Unexpected actor - remove all
        cleanup_report.append(f"❌ {label}: {count}x found (UNEXPECTED - removing all)")
        duplicates_to_remove.extend(actor_list)
    elif count > expected_count:
        # Too many - keep only expected count
        cleanup_report.append(f"⚠️  {label}: {count}x found (expected {expected_count}, removing {count - expected_count})")
        duplicates_to_remove.extend(actor_list[expected_count:])
    elif count == expected_count:
        # Correct count
        cleanup_report.append(f"✅ {label}: {count}x (correct)")
    else:
        # Missing actors
        cleanup_report.append(f"ℹ️  {label}: {count}x found (expected {expected_count}, missing {expected_count - count})")

# Check for missing expected actors
print("Expected actors status:")
for label, expected in sorted(EXPECTED_ACTORS.items()):
    if label not in actor_groups:
        cleanup_report.append(f"⚠️  {label}: 0x found (expected {expected}, MISSING)")

print()
for line in cleanup_report:
    print(f"  {line}")

print(f"\n📊 Total actors to remove: {len(duplicates_to_remove)}\n")

# Confirm and delete
print("="*70)
print("  🗑️  REMOVING DUPLICATES")
print("="*70 + "\n")

deleted_count = 0
failed_count = 0

for actor in duplicates_to_remove:
    try:
        actor.destroy_actor()
        deleted_count += 1
        if deleted_count % 50 == 0:
            print(f"  Deleted {deleted_count}/{len(duplicates_to_remove)}...")
    except Exception as e:
        failed_count += 1

print(f"\n✅ Cleanup complete!")
print(f"   Deleted: {deleted_count} actors")
if failed_count > 0:
    print(f"   Failed: {failed_count} actors")

# Final manifest check
print("\n" + "="*70)
print("  📋 FINAL SOLAR SYSTEM MANIFEST")
print("="*70 + "\n")

remaining_actors = unreal.EditorLevelLibrary.get_all_level_actors()
print(f"Total actors in level: {len(remaining_actors)}\n")

# Recount actors after cleanup
final_groups = defaultdict(list)
for actor in remaining_actors:
    if not actor:
        continue
    actor_class_name = actor.get_class().get_name()
    if any(essential in actor_class_name for essential in essential_types):
        continue
    label = actor.get_actor_label()
    final_groups[label].append(actor)

print("Solar System Components:")
for label, expected in sorted(EXPECTED_ACTORS.items()):
    actual = len(final_groups.get(label, []))
    status = "✅" if actual == expected else ("⚠️" if actual > 0 else "❌")
    print(f"  {status} {label}: {actual}/{expected}")

print("\n" + "="*70)
print(f"  ✅ MANIFEST CHECK COMPLETE")
print("="*70)
print(f"\n📊 Before cleanup: {len(actors)} actors")
print(f"📊 After cleanup:  {len(remaining_actors)} actors")
print(f"📊 Removed:        {len(actors) - len(remaining_actors)} actors\n")
