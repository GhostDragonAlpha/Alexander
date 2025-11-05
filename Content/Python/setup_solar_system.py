"""
Solar System Setup Script
Run this ONCE to populate an empty level with the orbital hierarchy.
This is NOT an automatic startup script - run it manually when you want to rebuild the level.

FIXED: Proper C++ class loading (NOT Blueprint loading)
"""

import unreal

# Get editor utilities using UE5.5+ subsystems
editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

def spawn_actor(actor_class_path, location, name):
    """
    Spawn an actor at the specified location with a given name.

    IMPORTANT: For C++ classes like Sun, Planet, Moon, Asteroid:
    - DO NOT use EditorAssetLibrary.load_blueprint_class() - that's ONLY for Blueprints!
    - Use unreal.load_class() for C++ classes

    Example paths:
    - C++ class: "/Script/Alexander.Sun"
    - Blueprint: "/Game/Blueprints/BP_Sun.BP_Sun_C"
    """
    actor_class = None

    # Check if this is a C++ class path (starts with /Script/)
    if actor_class_path.startswith("/Script/"):
        # This is a C++ class - use load_class
        try:
            actor_class = unreal.load_class(None, actor_class_path)
            if actor_class:
                unreal.log(f"Loaded C++ class: {actor_class_path}")
        except Exception as e:
            unreal.log_error(f"Failed to load C++ class '{actor_class_path}': {e}")
    else:
        # This might be a Blueprint - try to load as asset
        try:
            editor_asset_subsystem = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
            blueprint_asset = editor_asset_subsystem.load_asset(actor_class_path)
            if blueprint_asset and hasattr(blueprint_asset, 'generated_class'):
                actor_class = blueprint_asset.generated_class()
                unreal.log(f"Loaded Blueprint class: {actor_class_path}")
        except Exception as e:
            unreal.log_error(f"Failed to load Blueprint '{actor_class_path}': {e}")

    if actor_class:
        # Spawn the actor using subsystem
        actor = editor_actor_subsystem.spawn_actor_from_class(
            actor_class,
            location,
            unreal.Rotator(0, 0, 0)
        )
        if actor:
            actor.set_actor_label(name)
            unreal.log(f"Created: {name}")
            return actor
        else:
            unreal.log_error(f"Failed to spawn actor: {name}")
    else:
        unreal.log_error(f"Failed to load class: {actor_class_path}")

    return None

def setup_solar_system():
    """Create a basic solar system hierarchy."""
    unreal.log("=" * 60)
    unreal.log("SETTING UP SOLAR SYSTEM ORBITAL HIERARCHY")
    unreal.log("=" * 60)

    # 1. Spawn the Sun at origin (root of the system)
    sun = spawn_actor(
        "/Script/Alexander.Sun",
        unreal.Vector(0, 0, 0),
        "Sun_Main"
    )

    if not sun:
        unreal.log_error("Failed to create Sun - aborting setup")
        return

    # Configure Sun properties
    sun.set_editor_property("sun_scale", 2.0)
    sun.set_editor_property("sun_color", unreal.LinearColor(1.0, 0.8, 0.3, 1.0))
    sun.set_editor_property("light_intensity", 500.0)
    sun.set_editor_property("emissive_intensity", 500.0)
    unreal.log("  - Sun configured as Root")

    # 2. Spawn Planets orbiting the Sun
    planet_configs = [
        {"name": "Planet_Inner", "distance": 5000.0, "color": unreal.LinearColor(0.8, 0.4, 0.2, 1.0), "scale": 0.4, "speed": 15.0},
        {"name": "Planet_Middle", "distance": 10000.0, "color": unreal.LinearColor(0.3, 0.5, 0.9, 1.0), "scale": 0.6, "speed": 10.0},
        {"name": "Planet_Outer", "distance": 15000.0, "color": unreal.LinearColor(0.9, 0.7, 0.5, 1.0), "scale": 0.5, "speed": 7.0},
    ]

    planets = []
    for i, config in enumerate(planet_configs):
        # Spawn planet at its orbital distance
        planet = spawn_actor(
            "/Script/Alexander.Planet",
            unreal.Vector(config["distance"], 0, 0),
            config["name"]
        )

        if planet:
            # Configure planet properties
            planet.set_editor_property("orbit_target", sun)
            planet.set_editor_property("orbit_radius", config["distance"])
            planet.set_editor_property("orbit_speed", config["speed"])
            planet.set_editor_property("planet_scale", config["scale"])
            planet.set_editor_property("planet_color", config["color"])
            planet.set_editor_property("orbit_mode", unreal.OrbitMode.ORBIT)
            planets.append(planet)
            unreal.log(f"  - {config['name']} orbiting Sun at {config['distance']}cm")

    # 3. Spawn a Moon orbiting the middle planet
    if len(planets) >= 2:
        middle_planet = planets[1]
        moon = spawn_actor(
            "/Script/Alexander.Moon",
            unreal.Vector(10000.0 + 3000.0, 0, 0),  # Planet distance + moon orbit
            "Moon_01"
        )

        if moon:
            moon.set_editor_property("orbit_target", middle_planet)
            moon.set_editor_property("orbit_radius", 3000.0)
            moon.set_editor_property("orbit_speed", 25.0)
            moon.set_editor_property("moon_scale", 0.15)
            moon.set_editor_property("moon_color", unreal.LinearColor(0.7, 0.7, 0.7, 1.0))
            moon.set_editor_property("orbit_mode", unreal.OrbitMode.ORBIT)
            moon.set_editor_property("orbit_inclination", 5.0)
            unreal.log(f"  - Moon_01 orbiting Planet_Middle")

    # 4. Spawn some asteroids in various configurations
    asteroid_configs = [
        {"name": "Asteroid_01", "target_idx": 0, "distance": 2000.0, "scale": 0.08},
        {"name": "Asteroid_02", "target_idx": 2, "distance": 2500.0, "scale": 0.1},
        {"name": "Asteroid_Belt_01", "orbit_sun": True, "distance": 12000.0, "scale": 0.05},
    ]

    for config in asteroid_configs:
        if config.get("orbit_sun"):
            # Asteroid orbiting the Sun
            asteroid = spawn_actor(
                "/Script/Alexander.Asteroid",
                unreal.Vector(config["distance"], 0, 500.0),
                config["name"]
            )
            if asteroid:
                asteroid.set_editor_property("orbit_target", sun)
                asteroid.set_editor_property("orbit_radius", config["distance"])
                asteroid.set_editor_property("orbit_speed", 20.0)
                asteroid.set_editor_property("asteroid_scale", config["scale"])
                asteroid.set_editor_property("orbit_mode", unreal.OrbitMode.ORBIT)
                asteroid.set_editor_property("orbit_inclination", 15.0)
                unreal.log(f"  - {config['name']} orbiting Sun")
        else:
            # Asteroid orbiting a planet
            target_planet = planets[config["target_idx"]] if config["target_idx"] < len(planets) else None
            if target_planet:
                planet_dist = planet_configs[config["target_idx"]]["distance"]
                asteroid = spawn_actor(
                    "/Script/Alexander.Asteroid",
                    unreal.Vector(planet_dist + config["distance"], 0, 200.0),
                    config["name"]
                )
                if asteroid:
                    asteroid.set_editor_property("orbit_target", target_planet)
                    asteroid.set_editor_property("orbit_radius", config["distance"])
                    asteroid.set_editor_property("orbit_speed", 30.0)
                    asteroid.set_editor_property("asteroid_scale", config["scale"])
                    asteroid.set_editor_property("orbit_mode", unreal.OrbitMode.ORBIT)
                    unreal.log(f"  - {config['name']} orbiting planet")

    unreal.log("=" * 60)
    unreal.log("SOLAR SYSTEM SETUP COMPLETE")
    unreal.log("=" * 60)
    unreal.log("Hierarchy created:")
    unreal.log("  Sun (Root)")
    unreal.log("    ├─ Planet_Inner")
    unreal.log("    │    └─ Asteroid_01")
    unreal.log("    ├─ Planet_Middle")
    unreal.log("    │    └─ Moon_01")
    unreal.log("    ├─ Planet_Outer")
    unreal.log("    │    └─ Asteroid_02")
    unreal.log("    └─ Asteroid_Belt_01")
    unreal.log("")
    unreal.log("Press PLAY to see orbital mechanics in action!")
    unreal.log("=" * 60)

# Run the setup
if __name__ == "__main__":
    setup_solar_system()
