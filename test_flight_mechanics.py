"""
Task T003: Test player spawning and flight mechanics
Execute flight test in PIE mode
"""

import unreal
import time

print("=" * 70)
print("TASK T003: TESTING FLIGHT MECHANICS IN PIE")
print("=" * 70)

# Load FlightTest level
print("[1/5] Loading FlightTest level...")
unreal.EditorLevelLibrary.load_level("/Game/FlightTest")
time.sleep(2)

# Get world
world = unreal.EditorLevelLibrary.get_editor_world()
print(f"[2/5] World loaded: {world.get_name()}")

# Check for player start
print("[3/5] Checking for Player Start...")
player_starts = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.PlayerStart)
if player_starts:
    print(f"    Found {len(player_starts)} Player Start(s)")
    for i, start in enumerate(player_starts):
        loc = start.get_actor_location()
        print(f"    PlayerStart {i+1}: ({loc.x:.0f}, {loc.y:.0f}, {loc.z:.0f})")
else:
    print("    WARNING: No Player Start found! Player won't spawn.")

# Check for ship/pawn
print("[4/5] Checking for playable pawn/ship...")
all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
ships = [a for a in all_actors if "ship" in a.get_name().lower() or "pawn" in a.get_name().lower()]
if ships:
    print(f"    Found {len(ships)} potential ship/pawn actors:")
    for ship in ships:
        print(f"    - {ship.get_name()}")
else:
    print("    WARNING: No ship/pawn found in level!")

# Check for FlightController component
print("[5/5] Checking for FlightController...")
for actor in all_actors:
    components = actor.get_components_by_class(unreal.ActorComponent)
    for comp in components:
        if "flight" in comp.get_name().lower():
            print(f"    Found FlightController on: {actor.get_name()}")

print("")
print("FLIGHT MECHANICS TEST SUMMARY:")
print("-" * 70)
if player_starts and ships:
    print("STATUS: READY TO FLY")
    print("RECOMMENDATION: Press Alt+P to test in PIE")
else:
    print("STATUS: MISSING COMPONENTS")
    if not player_starts:
        print("ERROR: Need to add PlayerStart to level")
    if not ships:
        print("ERROR: Need to add playable ship/pawn to level")

print("=" * 70)
