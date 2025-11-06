"""
Task T003B: Check VRTemplateMap for playable pawn and flight setup
Simple level inspection
"""

import unreal

print("=" * 70)
print("TASK T003B: CHECKING VRTEMPLATEMAP")
print("=" * 70)

# Get current world (should be VRTemplateMap)
world = unreal.EditorLevelLibrary.get_editor_world()
print(f"Current World: {world.get_name()}")
print("")

# Get all actors
all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
print(f"Total Actors in Level: {len(all_actors)}")
print("")

# Find Player Start
print("PLAYER START:")
player_starts = [a for a in all_actors if a.get_class().get_name() == "PlayerStart"]
if player_starts:
    for ps in player_starts:
        loc = ps.get_actor_location()
        print(f"  Found: {ps.get_name()} at ({loc.x:.0f}, {loc.y:.0f}, {loc.z:.0f})")
else:
    print("  NO PLAYER START FOUND!")
print("")

# Find Pawns
print("PAWNS/SHIPS:")
pawns = [a for a in all_actors if "pawn" in a.get_class().get_name().lower()]
if pawns:
    for p in pawns:
        print(f"  Found: {p.get_class().get_name()} - {p.get_name()}")
else:
    print("  No pawns found.")
print("")

# Find anything with "ship" or "flight" in name
print("SHIP/FLIGHT ACTORS:")
flight_actors = [a for a in all_actors if "ship" in a.get_name().lower() or "flight" in a.get_name().lower()]
if flight_actors:
    for fa in flight_actors:
        print(f"  Found: {fa.get_class().get_name()} - {fa.get_name()}")
else:
    print("  No ship/flight actors found.")
print("")

# Check for Game Mode
print("GAME MODE:")
game_mode = world.get_world_settings().get_default_game_mode()
if game_mode:
    print(f"  {game_mode.get_name()}")
else:
    print("  No game mode set.")
print("")

print("=" * 70)
print("SUMMARY:")
print(f"  Player Start: {'YES' if player_starts else 'NO'}")
print(f"  Pawns: {len(pawns)} found")
print(f"  Ship/Flight Actors: {len(flight_actors)} found")
print("=" * 70)
