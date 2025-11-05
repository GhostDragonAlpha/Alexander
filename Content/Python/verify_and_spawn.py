"""
Verify C++ classes are loaded and spawn the solar system
"""

import unreal
import sys
import os

def verify_classes():
    """Check if all C++ orbital classes are available"""
    unreal.log("=" * 70)
    unreal.log("VERIFYING C++ ORBITAL CLASSES")
    unreal.log("=" * 70)

    classes_to_check = [
        "/Script/Alexander.OrbitalBody",
        "/Script/Alexander.Sun",
        "/Script/Alexander.Planet",
        "/Script/Alexander.Moon",
        "/Script/Alexander.Asteroid"
    ]

    all_loaded = True
    for class_path in classes_to_check:
        try:
            actor_class = unreal.load_class(None, class_path)
            if actor_class:
                unreal.log(f"✅ {class_path} - LOADED")
            else:
                unreal.log_error(f"❌ {class_path} - FAILED TO LOAD")
                all_loaded = False
        except Exception as e:
            unreal.log_error(f"❌ {class_path} - ERROR: {e}")
            all_loaded = False

    unreal.log("=" * 70)

    if all_loaded:
        unreal.log("✅ ALL C++ CLASSES LOADED SUCCESSFULLY")
        unreal.log("=" * 70)
        return True
    else:
        unreal.log_error("❌ SOME CLASSES FAILED TO LOAD")
        unreal.log("=" * 70)
        return False

def spawn_solar_system_with_classes():
    """Spawn the solar system using the compiled C++ classes"""

    if not verify_classes():
        unreal.log_error("Cannot spawn solar system - classes not loaded")
        return False

    unreal.log("\n" + "=" * 70)
    unreal.log("SPAWNING SOLAR SYSTEM WITH C++ CLASSES")
    unreal.log("=" * 70)

    editor_level_library = unreal.EditorLevelLibrary

    # Check if actors already exist
    all_actors = editor_level_library.get_all_level_actors()
    sun_exists = any(actor.get_actor_label() == "Sun_Main" for actor in all_actors)

    if sun_exists:
        unreal.log("⚠️ Solar system already exists in level")
        unreal.log("=" * 70)
        return True

    # 1. Spawn the Sun at origin
    sun_class = unreal.load_class(None, "/Script/Alexander.Sun")
    sun = editor_level_library.spawn_actor_from_class(
        sun_class,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )

    if not sun:
        unreal.log_error("Failed to spawn Sun")
        return False

    sun.set_actor_label("Sun_Main")
    unreal.log("✅ Spawned Sun at origin")

    # Configure Sun properties
    sun.set_editor_property("orbit_mode", unreal.OrbitMode.ROOT)
    sun.set_editor_property("mass", 1.989e30)
    sun.set_editor_property("sun_scale", 2.0)
    sun.set_editor_property("sun_color", unreal.LinearColor(1.0, 0.8, 0.3, 1.0))
    sun.set_editor_property("light_intensity", 500.0)
    sun.set_editor_property("emissive_intensity", 500.0)

    # 2. Spawn Planets
    planet_class = unreal.load_class(None, "/Script/Alexander.Planet")
    planet_configs = [
        {"name": "Planet_Inner", "distance": 5000.0, "color": unreal.LinearColor(0.8, 0.4, 0.2, 1.0), "scale": 0.4, "speed": 15.0},
        {"name": "Planet_Middle", "distance": 10000.0, "color": unreal.LinearColor(0.3, 0.5, 0.9, 1.0), "scale": 0.6, "speed": 10.0},
        {"name": "Planet_Outer", "distance": 15000.0, "color": unreal.LinearColor(0.9, 0.7, 0.5, 1.0), "scale": 0.5, "speed": 7.0},
    ]

    planets = []
    for config in planet_configs:
        planet = editor_level_library.spawn_actor_from_class(
            planet_class,
            unreal.Vector(config["distance"], 0, 0),
            unreal.Rotator(0, 0, 0)
        )

        if planet:
            planet.set_actor_label(config["name"])
            planet.set_editor_property("orbit_target", sun)
            planet.set_editor_property("orbit_radius", config["distance"])
            planet.set_editor_property("orbit_speed", config["speed"])
            planet.set_editor_property("planet_scale", config["scale"])
            planet.set_editor_property("planet_color", config["color"])
            planet.set_editor_property("orbit_mode", unreal.OrbitMode.ORBIT)
            planet.set_editor_property("mass", 5.972e24)
            planets.append(planet)
            unreal.log(f"✅ Spawned {config['name']}")

    # 3. Spawn a Moon orbiting the middle planet
    if len(planets) >= 2:
        moon_class = unreal.load_class(None, "/Script/Alexander.Moon")
        middle_planet = planets[1]

        moon = editor_level_library.spawn_actor_from_class(
            moon_class,
            unreal.Vector(13000.0, 0, 0),  # Planet distance + moon orbit
            unreal.Rotator(0, 0, 0)
        )

        if moon:
            moon.set_actor_label("Moon_01")
            moon.set_editor_property("orbit_target", middle_planet)
            moon.set_editor_property("orbit_radius", 3000.0)
            moon.set_editor_property("orbit_speed", 25.0)
            moon.set_editor_property("moon_scale", 0.15)
            moon.set_editor_property("moon_color", unreal.LinearColor(0.7, 0.7, 0.7, 1.0))
            moon.set_editor_property("orbit_mode", unreal.OrbitMode.ORBIT)
            moon.set_editor_property("orbit_inclination", 5.0)
            moon.set_editor_property("mass", 7.342e22)
            unreal.log("✅ Spawned Moon_01")

    # 4. Spawn some asteroids
    asteroid_class = unreal.load_class(None, "/Script/Alexander.Asteroid")
    asteroid_configs = [
        {"name": "Asteroid_01", "target": planets[0], "distance": 2000.0, "scale": 0.08},
        {"name": "Asteroid_02", "target": planets[2], "distance": 2500.0, "scale": 0.1},
        {"name": "Asteroid_Belt_01", "target": sun, "distance": 12000.0, "scale": 0.05},
    ]

    for config in asteroid_configs:
        target = config["target"]
        target_pos = target.get_actor_location()

        asteroid = editor_level_library.spawn_actor_from_class(
            asteroid_class,
            unreal.Vector(target_pos.x + config["distance"], 0, 500.0),
            unreal.Rotator(0, 0, 0)
        )

        if asteroid:
            asteroid.set_actor_label(config["name"])
            asteroid.set_editor_property("orbit_target", target)
            asteroid.set_editor_property("orbit_radius", config["distance"])
            asteroid.set_editor_property("orbit_speed", 20.0)
            asteroid.set_editor_property("asteroid_scale", config["scale"])
            asteroid.set_editor_property("orbit_mode", unreal.OrbitMode.ORBIT)
            asteroid.set_editor_property("mass", 1.0e18)
            if config["name"] == "Asteroid_Belt_01":
                asteroid.set_editor_property("orbit_inclination", 15.0)
            unreal.log(f"✅ Spawned {config['name']}")

    unreal.log("=" * 70)
    unreal.log("✅ SOLAR SYSTEM SPAWNED SUCCESSFULLY")
    unreal.log("=" * 70)
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
    unreal.log("=" * 70)

    return True

if __name__ == "__main__":
    spawn_solar_system_with_classes()
