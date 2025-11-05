"""
Quick Scene Check - See what's in the current level
"""

import unreal

print("\n" + "=" * 70)
print("  QUICK SCENE CHECK")
print("=" * 70 + "\n")

# Get current level
try:
    current_level = unreal.EditorLevelLibrary.get_editor_world()
    level_name = current_level.get_name() if current_level else "Unknown"
    print(f"📍 Current Level: {level_name}\n")
except:
    print("📍 Current Level: Unknown\n")

# Get all actors
all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
print(f"Total Actors: {len(all_actors)}\n")

# Categorize
categories = {
    'Planets': [],
    'Cockpit': [],
    'Test_Cameras': [],
    'Test_Objects': [],
    'Lights': []
}

for actor in all_actors:
    if not actor:
        continue

    label = actor.get_actor_label()

    # Categorize
    if any(name in label for name in ['Sun', 'Mercury', 'Venus', 'Earth', 'Mars', 'Jupiter', 'Saturn', 'Uranus', 'Neptune', 'Moon']):
        categories['Planets'].append(label)
    elif any(word in label for word in ['Cockpit', 'Flight', 'Throttle', 'Seat', 'Stick', 'Panel', 'Button']):
        categories['Cockpit'].append(label)
    elif 'Cam' in label and any(n in label for n in ['Cam1', 'Cam2', 'Cam3', 'Cam4', 'Cam5']):
        categories['Test_Cameras'].append(label)
    elif any(word in label for word in ['Zone', 'Grid', 'Reference', 'Sphere', 'Cube']):
        categories['Test_Objects'].append(label)
    elif 'Light' in label or 'Sun_' in label or 'Sky' in label:
        categories['Lights'].append(label)

# Print summary
for category, items in categories.items():
    if items:
        print(f"📂 {category}: {len(items)} items")
        if len(items) <= 10:
            for item in items[:10]:
                print(f"   - {item}")
        else:
            for item in items[:5]:
                print(f"   - {item}")
            print(f"   ... and {len(items)-5} more")
        print()

# Issue detection
issues = []
if categories['Planets'] and categories['Test_Cameras']:
    issues.append("⚠️  ISSUE: Planets AND test cameras in same level (mixed content!)")

if categories['Cockpit'] and categories['Test_Cameras']:
    issues.append("⚠️  ISSUE: Cockpit AND test cameras in same level (mixed content!)")

if issues:
    print("=" * 70)
    print("DETECTED ISSUES:")
    print("=" * 70)
    for issue in issues:
        print(issue)
    print("\nRECOMMENDATION: Clean up duplicates or start fresh level")
else:
    print("✅ No obvious content mixing detected")

print("\n" + "=" * 70 + "\n")
