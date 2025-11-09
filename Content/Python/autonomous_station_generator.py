"""
Autonomous Space Station Generator for Unreal Engine
Creates baseline space stations using CSG primitives and Blueprint construction

Usage in Unreal Editor Python console:
    import autonomous_station_generator
    autonomous_station_generator.generate_all_stations()
"""

import unreal
import math

# Output directory for generated blueprints
OUTPUT_DIR = "/Game/Blueprints/Stations/Generated"
MATERIALS_DIR = "/Game/Materials"

# Station configurations
STATION_CONFIGS = {
    "TradingHub": {
        "size": 500.0,  # meters diameter
        "complexity": "high",
        "modules": ["docking_bay", "trading_floor", "corridors", "shops", "offices"],
        "exterior_sections": 5,
        "interior_rooms": 20,
        "description": "Main trading hub - central commerce location"
    },
    "ResearchFacility": {
        "size": 250.0,
        "complexity": "medium",
        "modules": ["lab", "observation", "corridors", "data_center"],
        "exterior_sections": 3,
        "interior_rooms": 10,
        "description": "Scientific research facility"
    },
    "MilitaryOutpost": {
        "size": 300.0,
        "complexity": "medium",
        "modules": ["command", "hangar", "armory", "barracks"],
        "exterior_sections": 4,
        "interior_rooms": 10,
        "description": "Military defense outpost"
    },
    "MiningPlatform": {
        "size": 150.0,
        "complexity": "low",
        "modules": ["processing", "storage", "control_room"],
        "exterior_sections": 2,
        "interior_rooms": 6,
        "description": "Ore processing platform"
    },
    "TradingOutpost": {
        "size": 200.0,
        "complexity": "low",
        "modules": ["market", "docking", "storage"],
        "exterior_sections": 3,
        "interior_rooms": 8,
        "description": "Small trading outpost"
    }
}


def create_basic_material(material_name, base_color, metallic=0.8, roughness=0.3):
    """Create a basic material instance"""
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    # Create material instance
    material_path = f"{MATERIALS_DIR}/{material_name}"

    # Check if already exists
    if unreal.EditorAssetLibrary.does_asset_exist(material_path):
        return unreal.load_asset(material_path)

    # For baseline, just reference Unreal's built-in materials
    # We'll use material instances of built-in materials
    return None  # Will use default material for now


def create_primitive_mesh(parent_actor, mesh_type, scale, location, rotation=(0, 0, 0), material=None):
    """
    Add a primitive static mesh component to an actor
    mesh_type: 'cube', 'sphere', 'cylinder', 'cone'
    """

    # Map to Unreal's basic shapes
    mesh_paths = {
        'cube': '/Engine/BasicShapes/Cube.Cube',
        'sphere': '/Engine/BasicShapes/Sphere.Sphere',
        'cylinder': '/Engine/BasicShapes/Cylinder.Cylinder',
        'cone': '/Engine/BasicShapes/Cone.Cone'
    }

    if mesh_type not in mesh_paths:
        unreal.log_warning(f"Unknown mesh type: {mesh_type}")
        return None

    # Create static mesh component
    mesh_component = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor.static_class(),
        location
    )

    if mesh_component:
        # Load the static mesh
        static_mesh = unreal.load_asset(mesh_paths[mesh_type])

        if static_mesh:
            mesh_component.static_mesh_component.set_static_mesh(static_mesh)
            mesh_component.static_mesh_component.set_world_scale3d(unreal.Vector(scale[0], scale[1], scale[2]))
            mesh_component.static_mesh_component.set_relative_rotation(unreal.Rotator(rotation[0], rotation[1], rotation[2]))

    return mesh_component


def create_station_core(station_name, base_size):
    """Create the central core of a space station"""
    unreal.log(f"Creating core for {station_name}...")

    # Main sphere core
    core_scale = (base_size / 100.0, base_size / 100.0, base_size / 100.0)
    core = create_primitive_mesh(
        None,
        'sphere',
        core_scale,
        unreal.Vector(0, 0, 0)
    )

    return core


def create_docking_bay_module(base_location, size_scale=1.0):
    """Create a docking bay module using primitives"""
    # Main bay structure (long box)
    bay_scale = (size_scale * 5, size_scale * 3, size_scale * 2)

    bay = create_primitive_mesh(
        None,
        'cube',
        bay_scale,
        base_location
    )

    return bay


def create_corridor_module(start_location, end_location, width=2.0):
    """Create a corridor connecting two points"""
    # Calculate direction and distance
    direction = unreal.Vector(
        end_location.x - start_location.x,
        end_location.y - start_location.y,
        end_location.z - start_location.z
    )

    distance = math.sqrt(direction.x**2 + direction.y**2 + direction.z**2)
    mid_point = unreal.Vector(
        (start_location.x + end_location.x) / 2,
        (start_location.y + end_location.y) / 2,
        (start_location.z + start_location.z) / 2
    )

    # Create corridor as elongated cylinder
    corridor_scale = (width, width, distance / 100.0)

    corridor = create_primitive_mesh(
        None,
        'cylinder',
        corridor_scale,
        mid_point,
        rotation=(0, 90, 0)  # Rotate to align with direction
    )

    return corridor


def create_room_module(base_location, room_type, size_scale=1.0):
    """Create an interior room module"""
    room_configs = {
        "small": (size_scale * 3, size_scale * 3, size_scale * 2),
        "medium": (size_scale * 5, size_scale * 5, size_scale * 3),
        "large": (size_scale * 8, size_scale * 8, size_scale * 4),
        "trading_floor": (size_scale * 15, size_scale * 15, size_scale * 5),
        "hangar": (size_scale * 20, size_scale * 10, size_scale * 8)
    }

    scale = room_configs.get(room_type, room_configs["medium"])

    room = create_primitive_mesh(
        None,
        'cube',
        scale,
        base_location
    )

    return room


def generate_station_layout(station_name, config):
    """Generate complete station layout"""
    unreal.log(f"Generating layout for {station_name}...")

    base_size = config["size"]
    modules = config["modules"]

    created_actors = []

    # Create central core
    core = create_station_core(station_name, base_size)
    if core:
        created_actors.append(core)

    # Create exterior sections in a ring around core
    num_sections = config["exterior_sections"]
    radius = base_size * 1.5

    for i in range(num_sections):
        angle = (2 * math.pi * i) / num_sections
        x = radius * math.cos(angle)
        y = radius * math.sin(angle)

        # Create section module (cylinder or cube based on station type)
        location = unreal.Vector(x, y, 0)

        if "docking" in modules or "hangar" in modules:
            section = create_docking_bay_module(location, base_size / 100.0)
        else:
            section = create_room_module(location, "large", base_size / 100.0)

        if section:
            created_actors.append(section)

    # Create interior rooms (stacked vertically in core)
    num_rooms = config["interior_rooms"]
    room_height = base_size / num_rooms

    for i in range(num_rooms):
        z_offset = (i - num_rooms/2) * room_height

        # Vary room positions slightly
        x_offset = (i % 3 - 1) * base_size * 0.3
        y_offset = ((i // 3) % 3 - 1) * base_size * 0.3

        location = unreal.Vector(x_offset, y_offset, z_offset)
        room = create_room_module(location, "medium", base_size / 200.0)

        if room:
            created_actors.append(room)

    return created_actors


def create_station_blueprint(station_name, config):
    """Create a Blueprint containing all station components"""
    unreal.log(f"Creating Blueprint for {station_name}...")

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    # Create blueprint path
    blueprint_name = f"BP_{station_name}_Station"
    blueprint_path = f"{OUTPUT_DIR}/{blueprint_name}"

    # Check if already exists
    if unreal.EditorAssetLibrary.does_asset_exist(blueprint_path):
        unreal.log_warning(f"Blueprint already exists: {blueprint_path}")
        unreal.EditorAssetLibrary.delete_asset(blueprint_path)

    # Create output directory if needed
    if not unreal.EditorAssetLibrary.does_directory_exist(OUTPUT_DIR):
        unreal.EditorAssetLibrary.make_directory(OUTPUT_DIR)

    # For now, we'll generate the station components directly in the level
    # Blueprint creation via Python is complex - we'll use actor grouping instead

    unreal.log(f"Generating {station_name} components in current level...")
    actors = generate_station_layout(station_name, config)

    unreal.log(f"✓ Created {len(actors)} components for {station_name}")

    return actors


def generate_all_stations():
    """Generate all baseline space stations"""
    unreal.log("=" * 80)
    unreal.log("AUTONOMOUS STATION GENERATION - Starting")
    unreal.log("=" * 80)

    created_stations = {}

    for station_name, config in STATION_CONFIGS.items():
        unreal.log(f"\nGenerating {station_name}...")
        unreal.log(f"  Size: {config['size']}m")
        unreal.log(f"  Complexity: {config['complexity']}")
        unreal.log(f"  Modules: {', '.join(config['modules'])}")

        try:
            actors = create_station_blueprint(station_name, config)
            created_stations[station_name] = actors

            unreal.log(f"✓ {station_name} complete")
        except Exception as e:
            unreal.log_error(f"✗ Failed to create {station_name}: {e}")

    unreal.log("\n" + "=" * 80)
    unreal.log(f"AUTONOMOUS STATION GENERATION - Complete")
    unreal.log(f"Created {len(created_stations)} stations")
    unreal.log("=" * 80)

    return created_stations


def generate_single_station(station_name):
    """Generate a single station by name"""
    if station_name not in STATION_CONFIGS:
        unreal.log_error(f"Unknown station: {station_name}")
        unreal.log(f"Available: {', '.join(STATION_CONFIGS.keys())}")
        return None

    config = STATION_CONFIGS[station_name]
    return create_station_blueprint(station_name, config)


# Quick reference
def list_stations():
    """List all available station configurations"""
    unreal.log("Available Stations:")
    for name, config in STATION_CONFIGS.items():
        unreal.log(f"  - {name}: {config['size']}m, {config['description']}")


if __name__ == "__main__":
    unreal.log("\nAutonomous Station Generator")
    unreal.log("Usage:")
    unreal.log("  import autonomous_station_generator")
    unreal.log("  autonomous_station_generator.list_stations()")
    unreal.log("  autonomous_station_generator.generate_single_station('TradingHub')")
    unreal.log("  autonomous_station_generator.generate_all_stations()")
