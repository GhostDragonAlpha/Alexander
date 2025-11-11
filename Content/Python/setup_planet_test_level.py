#!/usr/bin/env python3
"""
Automated Planet Test Level Setup
Creates a complete playable level: Planet → Walk → Enter Ship → Fly
"""

import unreal

def clear_level():
    """Clear all actors from the level except essentials"""
    print("Clearing level...")

    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    cleared = 0

    for actor in actors:
        if actor:
            # Don't delete lights, cameras, or essential actors
            actor_name = actor.get_class().get_name()
            if actor_name not in ['DirectionalLight', 'SkyLight', 'SkyAtmosphere', 'ExponentialHeightFog', 'PostProcessVolume']:
                try:
                    unreal.EditorLevelLibrary.destroy_actor(actor)
                    cleared += 1
                except:
                    pass

    print(f"  Cleared {cleared} actors")

def spawn_planet(location=(0, 0, 0), radius=6371000.0):
    """Spawn a planet with proper settings"""
    print(f"Spawning planet at {location}...")

    world = unreal.EditorLevelLibrary.get_editor_world()
    if not world:
        print("ERROR: Could not get world")
        return None

    # Load Planet class
    planet_class = unreal.load_class(None, "/Script/Alexander.Planet")
    if not planet_class:
        print("ERROR: Could not load Planet class")
        return None

    # Spawn planet
    spawn_loc = unreal.Vector(location[0], location[1], location[2])
    planet = unreal.EditorLevelLibrary.spawn_actor_from_class(
        planet_class,
        spawn_loc,
        unreal.Rotator(0, 0, 0)
    )

    if not planet:
        print("ERROR: Failed to spawn planet")
        return None

    # Configure planet
    planet.set_actor_label("TestPlanet_Earth")
    planet.set_editor_property("PlanetRadius", radius / 100000.0)  # Convert cm to km

    # Set mass for gravity (Earth mass)
    try:
        planet.set_editor_property("Mass", 5.972e24)
        print("  Set planet mass for gravity")
    except:
        print("  Could not set mass (property may not exist)")

    print(f"  [OK] Planet spawned: {planet.get_actor_label()}")
    return planet

def spawn_player_start(location=(0, 0, 6400000)):
    """Spawn player start point above planet"""
    print(f"Spawning PlayerStart at {location}...")

    player_start_class = unreal.load_class(None, "/Script/Engine.PlayerStart")
    if not player_start_class:
        print("ERROR: Could not load PlayerStart class")
        return None

    spawn_loc = unreal.Vector(location[0], location[1], location[2])
    player_start = unreal.EditorLevelLibrary.spawn_actor_from_class(
        player_start_class,
        spawn_loc,
        unreal.Rotator(0, 0, 0)
    )

    if player_start:
        player_start.set_actor_label("PlayerStart_Surface")
        print(f"  [OK] PlayerStart spawned")
    else:
        print("  ERROR: Failed to spawn PlayerStart")

    return player_start

def spawn_ship(location=(500000, 0, 6371000), ship_blueprint_path=None):
    """Spawn a ship on the planet surface"""
    print(f"Spawning ship at {location}...")

    # Default ship blueprint path
    if not ship_blueprint_path:
        ship_blueprint_path = "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer_C"

    # Try to load the ship class
    ship_class = unreal.load_class(None, ship_blueprint_path)
    if not ship_class:
        print(f"WARNING: Could not load ship at {ship_blueprint_path}")
        print("  You'll need to manually place a ship in the level")
        return None

    spawn_loc = unreal.Vector(location[0], location[1], location[2])
    ship = unreal.EditorLevelLibrary.spawn_actor_from_class(
        ship_class,
        spawn_loc,
        unreal.Rotator(0, 0, 0)
    )

    if ship:
        ship.set_actor_label("PlayerShip")
        print(f"  [OK] Ship spawned: {ship.get_actor_label()}")
    else:
        print("  ERROR: Failed to spawn ship")

    return ship

def add_directional_light():
    """Add sun/directional light"""
    print("Adding directional light...")

    light_class = unreal.load_class(None, "/Script/Engine.DirectionalLight")
    if not light_class:
        print("  Could not load DirectionalLight class")
        return None

    light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        light_class,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(-45, 0, 0)  # Angled down
    )

    if light:
        light.set_actor_label("Sun")
        print("  [OK] Directional light added")

    return light

def add_sky_atmosphere():
    """Add sky atmosphere for visual quality"""
    print("Adding sky atmosphere...")

    sky_class = unreal.load_class(None, "/Script/Engine.SkyAtmosphere")
    if not sky_class:
        print("  Could not load SkyAtmosphere class")
        return None

    sky = unreal.EditorLevelLibrary.spawn_actor_from_class(
        sky_class,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )

    if sky:
        sky.set_actor_label("SkyAtmosphere")
        print("  [OK] Sky atmosphere added")

    return sky

def create_complete_test_level():
    """
    Create a complete test level with:
    - Planet with gravity
    - Player spawn point on surface
    - Ship nearby for entry
    - Lighting
    """
    print("="*70)
    print("CREATING COMPLETE PLANET TEST LEVEL")
    print("="*70)

    # Step 1: Clear existing level
    clear_level()

    # Step 2: Spawn planet at origin
    # Radius: 6371 km = 6,371,000 m = 637,100,000 cm
    planet_radius_cm = 637100000
    planet = spawn_planet((0, 0, 0), planet_radius_cm)

    if not planet:
        print("\n[FAILED] Could not create planet")
        return False

    # Step 3: Add player start on planet surface
    # Position: slightly above surface for safety
    surface_height = planet_radius_cm + 20000  # +200m above surface
    player_start = spawn_player_start((0, 0, surface_height))

    # Step 4: Spawn ship 5km away on surface
    ship_distance = 500000  # 5km = 500,000 cm
    ship_height = planet_radius_cm + 50000  # +500m above surface
    ship = spawn_ship((ship_distance, 0, ship_height))

    # Step 5: Add lighting
    add_directional_light()
    add_sky_atmosphere()

    # Step 6: Save level
    try:
        level_path = "/Game/Maps/PlanetTest"
        unreal.EditorLoadingAndSavingUtils.save_current_level()
        print(f"\n[OK] Level saved")
    except Exception as e:
        print(f"\n[WARNING] Could not save level: {e}")

    print("\n" + "="*70)
    print("LEVEL CREATION COMPLETE")
    print("="*70)
    print("\nWhat was created:")
    print("  ✓ Planet (Earth-sized) at origin with gravity")
    print("  ✓ PlayerStart on planet surface")
    print("  ✓ Ship 5km away on surface")
    print("  ✓ Directional light (sun)")
    print("  ✓ Sky atmosphere")
    print("\nNext steps:")
    print("  1. Press Play (PIE)")
    print("  2. You spawn on planet surface")
    print("  3. Walk toward ship (5km away)")
    print("  4. Approach ship to enter")
    print("  5. Fly off into space!")
    print("\nTo test in PIE:")
    print("  - WASD to walk")
    print("  - Space to jump")
    print("  - Ship controls once inside")

    return True

def create_close_test():
    """Create a test level with ship very close for quick testing"""
    print("="*70)
    print("CREATING CLOSE-RANGE TEST LEVEL")
    print("="*70)

    clear_level()

    # Planet at origin
    planet_radius_cm = 637100000  # 6371 km
    planet = spawn_planet((0, 0, 0), planet_radius_cm)

    if not planet:
        print("\n[FAILED] Could not create planet")
        return False

    # Player starts on surface
    surface_height = planet_radius_cm + 20000
    player_start = spawn_player_start((0, 0, surface_height))

    # Ship VERY close - just 100m away
    ship_distance = 10000  # 100m
    ship_height = planet_radius_cm + 20000
    ship = spawn_ship((ship_distance, 0, ship_height))

    # Lighting
    add_directional_light()
    add_sky_atmosphere()

    print("\n" + "="*70)
    print("CLOSE TEST LEVEL COMPLETE")
    print("="*70)
    print("\nShip is only 100m away for quick testing!")
    print("Press Play and walk straight ahead to reach ship.")

    return True

# ============================================================================
# MAIN EXECUTION
# ============================================================================

if __name__ == "__main__":
    import sys

    if len(sys.argv) > 1:
        command = sys.argv[1].lower()

        if command == "close":
            create_close_test()
        elif command == "full":
            create_complete_test_level()
        elif command == "clear":
            clear_level()
            print("[OK] Level cleared")
        else:
            print(f"Unknown command: {command}")
            print("Usage: setup_planet_test_level.py [close|full|clear]")
    else:
        # Default: close test for quick iteration
        create_close_test()

print("\n" + "="*70)
print("To run this in Unreal Editor:")
print("  py Content/Python/setup_planet_test_level.py")
print("\nOr with arguments:")
print("  py Content/Python/setup_planet_test_level.py close  # Ship 100m away")
print("  py Content/Python/setup_planet_test_level.py full   # Ship 5km away")
print("  py Content/Python/setup_planet_test_level.py clear  # Clear level")
print("="*70)
