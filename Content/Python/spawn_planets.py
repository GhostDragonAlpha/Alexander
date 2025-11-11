#!/usr/bin/env python3
"""
Spawn Planets for Ship Flight Testing
Creates a simple solar system with planets to fly around
"""

import unreal

def spawn_planet(name, location, radius=6371.0, color=(0.5, 0.5, 0.8)):
    """
    Spawn a planet at the specified location

    Args:
        name: Planet name
        location: FVector location (x, y, z) in cm
        radius: Planet radius in km (default: Earth-sized)
        color: RGB color tuple (0-1)

    Returns:
        APlanet actor reference
    """
    print(f"Spawning planet: {name} at {location}")

    # Get the editor world
    world = unreal.EditorLevelLibrary.get_editor_world()
    if not world:
        print("ERROR: Could not get editor world")
        return None

    # Load the Planet class
    planet_class = unreal.load_class(None, "/Script/Alexander.Planet")
    if not planet_class:
        print("ERROR: Could not load Planet class")
        return None

    # Spawn the planet
    spawn_location = unreal.Vector(location[0], location[1], location[2])
    spawn_rotation = unreal.Rotator(0, 0, 0)

    planet = unreal.EditorLevelLibrary.spawn_actor_from_class(
        planet_class,
        spawn_location,
        spawn_rotation
    )

    if not planet:
        print(f"ERROR: Failed to spawn planet {name}")
        return None

    # Set planet properties
    planet.set_actor_label(name)
    planet.set_editor_property("PlanetRadius", radius)

    # Try to set a basic material color (if the mesh exists)
    try:
        mesh_component = planet.get_editor_property("PlanetMesh")
        if mesh_component:
            # Create a simple dynamic material
            material = unreal.MaterialFactoryNew().factory_create_new(
                unreal.Material, None, f"M_{name}", unreal.ObjectFlags.RF_STANDALONE, None
            )

            # Note: Setting material parameters requires the material to have proper parameters
            # For now, just print that we created it
            print(f"  Created material for {name}")
    except Exception as e:
        print(f"  Could not set material: {e}")

    print(f"  [OK] Planet '{name}' spawned at {spawn_location}")
    return planet

def create_simple_solar_system():
    """
    Create a simple solar system with a few planets for testing ship flight

    Scale: 1 km = 100,000 cm in Unreal (100m scale)
    Distances are scaled down massively for gameplay
    """
    print("="*70)
    print("CREATING SIMPLE SOLAR SYSTEM FOR SHIP FLIGHT TESTING")
    print("="*70)

    # Central star/sun (very far away, just for reference)
    # spawn_planet("Sun", (0, 0, 0), radius=695700.0, color=(1.0, 0.9, 0.3))

    # Earth-like planet at origin (home planet)
    earth = spawn_planet("Earth", (0, 0, 0), radius=6371.0, color=(0.3, 0.5, 0.8))

    # Moon orbiting Earth (scaled distance for gameplay)
    # Real distance is 384,400 km, but we'll make it 50km for fun
    moon = spawn_planet("Moon", (5000000, 0, 0), radius=1737.0, color=(0.7, 0.7, 0.7))

    # Mars-like planet (scaled distance)
    mars = spawn_planet("Mars", (15000000, 8000000, 0), radius=3389.0, color=(0.8, 0.3, 0.2))

    # Gas giant (Jupiter-like, far distance)
    jupiter = spawn_planet("Jupiter", (30000000, -10000000, 5000000), radius=69911.0, color=(0.8, 0.6, 0.4))

    # Ice planet (Neptune-like)
    neptune = spawn_planet("Neptune", (-25000000, 20000000, -8000000), radius=24622.0, color=(0.3, 0.4, 0.9))

    # Add some smaller bodies/asteroids (tiny planets)
    spawn_planet("Asteroid_A", (8000000, 5000000, 2000000), radius=50.0, color=(0.5, 0.5, 0.5))
    spawn_planet("Asteroid_B", (12000000, -6000000, -3000000), radius=30.0, color=(0.4, 0.4, 0.4))
    spawn_planet("Station_Alpha", (2000000, 2000000, 1000000), radius=100.0, color=(0.6, 0.7, 0.8))

    print("\n" + "="*70)
    print("SOLAR SYSTEM CREATION COMPLETE")
    print("="*70)
    print("\nPlanets created:")
    print("  - Earth (home planet at origin)")
    print("  - Moon (50km away)")
    print("  - Mars (150km away)")
    print("  - Jupiter (300km away)")
    print("  - Neptune (250km away)")
    print("  - 3 small asteroids/stations")
    print("\nYou can now spawn your ship and fly between planets!")
    print("\nQuick start:")
    print("  1. Run PIE (Play in Editor)")
    print("  2. Spawn ship at (0, 0, 50000) - 5km above Earth")
    print("  3. Fly to Moon, Mars, or explore!")

def create_close_test_environment():
    """
    Create planets very close together for quick testing
    All planets within 10km radius for easy testing
    """
    print("="*70)
    print("CREATING CLOSE TEST ENVIRONMENT")
    print("="*70)

    # Central planet
    spawn_planet("CenterPlanet", (0, 0, 0), radius=6371.0, color=(0.3, 0.5, 0.8))

    # Ring of planets around it (close for testing)
    distance = 1000000  # 10km in cm

    spawn_planet("North", (0, distance, 0), radius=3000.0, color=(0.8, 0.3, 0.3))
    spawn_planet("South", (0, -distance, 0), radius=3000.0, color=(0.3, 0.8, 0.3))
    spawn_planet("East", (distance, 0, 0), radius=3000.0, color=(0.3, 0.3, 0.8))
    spawn_planet("West", (-distance, 0, 0), radius=3000.0, color=(0.8, 0.8, 0.3))
    spawn_planet("Above", (0, 0, distance), radius=3000.0, color=(0.8, 0.3, 0.8))
    spawn_planet("Below", (0, 0, -distance), radius=3000.0, color=(0.3, 0.8, 0.8))

    print("\n" + "="*70)
    print("CLOSE TEST ENVIRONMENT COMPLETE")
    print("="*70)
    print("\nCreated 7 planets in a cube formation")
    print("All planets are 10km from center")
    print("Perfect for quick flight testing!")

def clear_all_planets():
    """Remove all planets from the level"""
    print("Clearing all planets...")

    # Get all actors of type Planet
    planet_class = unreal.load_class(None, "/Script/Alexander.Planet")
    if not planet_class:
        print("ERROR: Could not load Planet class")
        return

    planets = unreal.EditorLevelLibrary.get_all_level_actors()
    removed_count = 0

    for actor in planets:
        if actor and isinstance(actor, unreal.Planet):
            actor_name = actor.get_actor_label()
            unreal.EditorLevelLibrary.destroy_actor(actor)
            print(f"  Removed: {actor_name}")
            removed_count += 1

    print(f"\n[OK] Removed {removed_count} planet(s)")

# ============================================================================
# MAIN EXECUTION
# ============================================================================

if __name__ == "__main__":
    import sys

    # Parse command line arguments
    if len(sys.argv) > 1:
        command = sys.argv[1].lower()

        if command == "clear":
            clear_all_planets()
        elif command == "close":
            clear_all_planets()
            create_close_test_environment()
        elif command == "solar":
            clear_all_planets()
            create_simple_solar_system()
        elif command == "single":
            # Just spawn one planet for basic testing
            clear_all_planets()
            spawn_planet("TestPlanet", (0, 0, 0), radius=6371.0, color=(0.5, 0.7, 0.9))
            print("\n[OK] Single test planet created at origin")
        else:
            print(f"Unknown command: {command}")
            print("Usage: spawn_planets.py [clear|close|solar|single]")
    else:
        # Default: create close test environment
        clear_all_planets()
        create_close_test_environment()

print("\n" + "="*70)
print("PLANET SPAWNING COMPLETE")
print("="*70)
print("\nTo run this script in Unreal Editor:")
print("  1. Open Unreal Editor")
print("  2. Window → Developer Tools → Output Log")
print("  3. In the 'Cmd' tab, type:")
print("     py Content/Python/spawn_planets.py")
print("\nOr with arguments:")
print("  py Content/Python/spawn_planets.py close   # Close test setup")
print("  py Content/Python/spawn_planets.py solar   # Full solar system")
print("  py Content/Python/spawn_planets.py single  # One planet only")
print("  py Content/Python/spawn_planets.py clear   # Remove all planets")
