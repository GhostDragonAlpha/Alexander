"""
Clear Level - Delete All Actors for Clean Slate
Removes all actors from the current level except essential ones
"""

import unreal

print("\n" + "=" * 70)
print("  CLEARING LEVEL FOR CLEAN SLATE")
print("=" * 70 + "\n")

# Get all actors
all_actors = unreal.EditorLevelLibrary.get_all_level_actors()

print(f"📊 Found {len(all_actors)} total actors in level\n")

# Essential actors to keep (don't delete these)
essential_types = [
    'WorldSettings',
    'DefaultPhysicsVolume',
    'LevelBounds',
    'NavigationDataChunk'
]

# Count what we're deleting
to_delete = []
to_keep = []

for actor in all_actors:
    if not actor:
        continue

    actor_class = actor.get_class().get_name()
    actor_label = actor.get_actor_label()

    # Keep essential actors
    if any(essential in actor_class for essential in essential_types):
        to_keep.append(f"{actor_label} ({actor_class})")
        continue

    to_delete.append((actor, actor_label, actor_class))

print(f"🗑️  Will delete: {len(to_delete)} actors")
print(f"✅ Will keep: {len(to_keep)} essential actors\n")

# Show what we're keeping
if to_keep:
    print("Keeping essential actors:")
    for item in to_keep[:5]:
        print(f"  - {item}")
    if len(to_keep) > 5:
        print(f"  ... and {len(to_keep) - 5} more")
    print()

# Delete non-essential actors
deleted_count = 0
failed_count = 0

print("Deleting actors...")
for actor, label, class_name in to_delete:
    try:
        actor.destroy_actor()
        deleted_count += 1
        if deleted_count <= 10:
            print(f"  ✓ Deleted: {label} ({class_name})")
        elif deleted_count == 11:
            print(f"  ... deleting remaining actors ...")
    except Exception as e:
        failed_count += 1
        if failed_count <= 3:
            print(f"  ✗ Failed to delete: {label} - {e}")

print()
print("=" * 70)
print(f"  CLEANUP COMPLETE")
print("=" * 70)
print(f"✅ Deleted: {deleted_count} actors")
if failed_count > 0:
    print(f"⚠️  Failed: {failed_count} actors")
print(f"✅ Kept: {len(to_keep)} essential actors")
print()
print("💡 Next: Run APPLY_CHANGES_NO_PAUSE.bat to restart")
print("   The test arena will rebuild cleanly on startup")
print("=" * 70 + "\n")
