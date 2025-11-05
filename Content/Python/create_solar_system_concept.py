"""
Create Scaled-Down Solar System Concept Level
Tests lighting interactions on simple spheres representing celestial bodies
"""

import unreal

print("\n" + "="*70)
print("  🌟 CREATING SOLAR SYSTEM CONCEPT LEVEL")
print("="*70 + "\n")

# Create new level or clear current one
print("Creating concept level...")

# Central position for the solar system
CENTER = unreal.Vector(0, 0, 100)

# Scale reference: 1 unit = 1 meter in Unreal
# Let's make a mini solar system that fits in ~2000 units diameter

print("\n📍 Creating Central Star (Sun)...")
# Star at center - bright yellow sphere
star_location = CENTER
star = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.StaticMeshActor,
    star_location
)
star.set_actor_label("Star_Sun")
star_mesh = star.static_mesh_component

# Load sphere mesh
sphere = unreal.EditorAssetLibrary.load_asset('/Engine/BasicShapes/Sphere')
star_mesh.set_static_mesh(sphere)

# Make it glow-colored (will add actual light next)
star.set_actor_scale3d(unreal.Vector(2.0, 2.0, 2.0))  # 200cm diameter star

print(f"  ✅ Star created at {star_location}")

# Add point light at star location
print("\n💡 Adding Star Point Light...")
star_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.PointLight,
    star_location
)
star_light.set_actor_label("Light_Star")
light_comp = star_light.root_component

# Bright light to illuminate the system
light_comp.set_editor_property("intensity", 100000.0)
light_comp.set_editor_property("attenuation_radius", 5000.0)
# Note: Light color defaults to white, which works fine
light_comp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)

print(f"  ✅ Star light: 100,000 intensity, 5000 unit radius")

# Create planets at different orbital distances
planets = [
    {"name": "Planet_Inner", "distance": 300, "size": 0.3, "color": "rocky"},
    {"name": "Planet_Mid", "distance": 600, "size": 0.8, "color": "gas"},
    {"name": "Planet_Outer", "distance": 900, "size": 0.6, "color": "ice"},
]

print(f"\n🪐 Creating {len(planets)} planets...")

for i, planet_data in enumerate(planets):
    # Position planet along X axis at different distances
    planet_location = CENTER + unreal.Vector(planet_data["distance"], 0, 0)

    planet = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        planet_location
    )
    planet.set_actor_label(planet_data["name"])
    planet_mesh = planet.static_mesh_component
    planet_mesh.set_static_mesh(sphere)

    # Scale planet
    scale = planet_data["size"]
    planet.set_actor_scale3d(unreal.Vector(scale, scale, scale))

    print(f"  ✅ {planet_data['name']}: {planet_data['distance']} units from star, {int(scale*100)}cm diameter")

# Add a tiny spaceship for scale reference
print("\n🚀 Adding Spaceship for scale reference...")
ship_location = CENTER + unreal.Vector(150, 150, 50)
ship = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.StaticMeshActor,
    ship_location
)
ship.set_actor_label("Spaceship_Reference")
ship_mesh = ship.static_mesh_component

# Use cube as simple spaceship shape
cube = unreal.EditorAssetLibrary.load_asset('/Engine/BasicShapes/Cube')
ship_mesh.set_static_mesh(cube)
ship.set_actor_scale3d(unreal.Vector(0.5, 0.2, 0.1))  # Elongated ship shape

print(f"  ✅ Spaceship placed at {ship_location}")

# Add cameras to view the system
print("\n📸 Creating viewing cameras...")

# Camera 1: Side view of solar system
cam1_loc = CENTER + unreal.Vector(0, -1500, 100)
cam1_rot = unreal.Rotator(0, 90, 0)  # Looking at center
cam1 = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.CameraActor,
    cam1_loc
)
cam1.set_actor_label("Cam_SideView")
cam1.set_actor_rotation(cam1_rot, False)
print(f"  ✅ Cam_SideView: Side perspective")

# Camera 2: Top-down view
cam2_loc = CENTER + unreal.Vector(0, 0, 1500)
cam2_rot = unreal.Rotator(-90, 0, 0)  # Looking down
cam2 = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.CameraActor,
    cam2_loc
)
cam2.set_actor_label("Cam_TopDown")
cam2.set_actor_rotation(cam2_rot, False)
print(f"  ✅ Cam_TopDown: Overhead view")

# Camera 3: Close-up of star
cam3_loc = CENTER + unreal.Vector(-400, 0, 0)
cam3_rot = unreal.Rotator(0, 0, 0)  # Looking at star
cam3 = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.CameraActor,
    cam3_loc
)
cam3.set_actor_label("Cam_StarCloseup")
cam3.set_actor_rotation(cam3_rot, False)
print(f"  ✅ Cam_StarCloseup: Close to star")

# Add ambient skylight for baseline illumination
print("\n🌌 Adding ambient skylight...")
skylight = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.SkyLight,
    CENTER + unreal.Vector(0, 0, 500)
)
skylight.set_actor_label("SkyLight_Ambient")
sky_comp = skylight.root_component
sky_comp.set_editor_property("intensity", 0.5)  # Very dim, just to see shadows
sky_comp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
print(f"  ✅ Ambient skylight: 0.5 intensity (movable)")

print("\n" + "="*70)
print("  ✅ SOLAR SYSTEM CONCEPT COMPLETE!")
print("="*70)

print("\n📊 Summary:")
print(f"  • 1 Star (200cm) with 100K point light")
print(f"  • {len(planets)} Planets (30-80cm) at varying distances")
print(f"  • 1 Spaceship reference")
print(f"  • 3 Cameras (side, top, closeup)")
print(f"  • Ambient skylight (0.5)")

print("\n🎯 Next Steps:")
print("  1. Capture screenshots from all 3 cameras")
print("  2. Verify:")
print("     - Star illuminates nearby planets brighter")
print("     - Planets cast shadows")
print("     - Lighting falloff visible with distance")
print("     - Spaceship shows scale")
print("\n💡 This tests lighting before building full-scale system!")
print()
