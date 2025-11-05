"""
List All Actors in Current Level
Shows what's in the scene and identifies duplicates
"""

import unreal

print("\n" + "=" * 70)
print("LISTING ALL ACTORS IN CURRENT LEVEL")
print("=" * 70 + "\n")

# Get all actors
all_actors = unreal.EditorLevelLibrary.get_all_level_actors()

# Categorize actors
actor_dict = {}
for actor in all_actors:
    if actor:
        actor_label = actor.get_actor_label()
        actor_class = actor.get_class().get_name()

        if actor_label not in actor_dict:
            actor_dict[actor_label] = []

        actor_dict[actor_label].append({
            'class': actor_class,
            'location': actor.get_actor_location()
        })

# Print results
print(f"Total actors: {len(all_actors)}\n")
print(f"Unique actor labels: {len(actor_dict)}\n")

# Find duplicates
duplicates = {label: actors for label, actors in actor_dict.items() if len(actors) > 1}

if duplicates:
    print("=" * 70)
    print("⚠️  DUPLICATE ACTORS FOUND")
    print("=" * 70 + "\n")

    for label, actors in duplicates.items():
        print(f"'{label}' - {len(actors)} instances:")
        for i, actor_info in enumerate(actors, 1):
            loc = actor_info['location']
            print(f"  [{i}] {actor_info['class']} at ({loc.x:.1f}, {loc.y:.1f}, {loc.z:.1f})")
        print()

# List by category
print("=" * 70)
print("ACTORS BY CATEGORY")
print("=" * 70 + "\n")

categories = {
    'Cameras': [],
    'Planets': [],
    'Cockpit': [],
    'Test': [],
    'Lights': [],
    'Other': []
}

for label in sorted(actor_dict.keys()):
    count = len(actor_dict[label])
    entry = f"{label} ({count})" if count > 1 else label

    if 'Cam' in label:
        categories['Cameras'].append(entry)
    elif any(planet in label for planet in ['Sun', 'Mercury', 'Venus', 'Earth', 'Mars', 'Jupiter', 'Saturn', 'Uranus', 'Neptune']):
        categories['Planets'].append(entry)
    elif any(word in label for word in ['Cockpit', 'Flight', 'Throttle', 'Button', 'Panel', 'Seat', 'Stick']):
        categories['Cockpit'].append(entry)
    elif any(word in label for word in ['Zone', 'Grid', 'Reference', 'Label']):
        categories['Test'].append(entry)
    elif 'Light' in label or 'Sun_' in label or 'Sky' in label:
        categories['Lights'].append(entry)
    else:
        categories['Other'].append(entry)

for category, items in categories.items():
    if items:
        print(f"📂 {category}: ({len(items)})")
        for item in items:
            print(f"   - {item}")
        print()

print("=" * 70)
print(f"Total: {len(all_actors)} actors")
print("=" * 70)
