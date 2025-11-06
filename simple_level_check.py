import unreal

print("CHECKING VRTEMPLATEMAP CONTENTS")
print("=" * 50)

try:
    # Use new API
    editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    all_actors = editor_actor_subsystem.get_all_level_actors()

    print(f"Total actors: {len(all_actors)}")
    print("")

    # Count by type
    actor_types = {}
    for actor in all_actors:
        class_name = actor.get_class().get_name()
        actor_types[class_name] = actor_types.get(class_name, 0) + 1

    print("ACTOR TYPES:")
    for actor_type, count in sorted(actor_types.items()):
        print(f"  {actor_type}: {count}")

    print("")
    print("DONE")

except Exception as e:
    print(f"ERROR: {e}")
