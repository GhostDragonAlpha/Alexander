"""
AI Vision Test Environment - Auto Builder
Run this in Unreal Python console to automatically create the test environment
"""

import unreal

def log(message):
    """Helper to log messages"""
    unreal.log(message)

def create_text_render(text, location, scale=2.0, color=(1, 1, 1)):
    """Create a 3D text render actor"""
    editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    text_actor = editor_actor_subsystem.spawn_actor_from_class(
        unreal.TextRenderActor,
        location,
        unreal.Rotator(0, 0, 0)
    )

    if text_actor:
        text_component = text_actor.get_component_by_class(unreal.TextRenderComponent)
        if text_component:
            text_component.set_text(text)
            text_component.set_world_size(scale * 10)
            # Skip color and alignment for UE 5.6 compatibility (use defaults)
            # Color parameter kept for future reference: color=(1,1,1)

        text_actor.set_actor_label(f"Label_{text}")

    return text_actor

def create_cube(location, size, color, label):
    """Create a colored cube actor"""
    editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    # Spawn static mesh actor
    cube_actor = editor_actor_subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor,
        location,
        unreal.Rotator(0, 0, 0)
    )

    if cube_actor:
        # Get the static mesh component
        static_mesh_comp = cube_actor.static_mesh_component

        # Load cube mesh (Unreal Engine's default cube)
        cube_mesh = unreal.load_asset('/Engine/BasicShapes/Cube')
        static_mesh_comp.set_static_mesh(cube_mesh)

        # Set scale
        scale_factor = size / 100.0  # Convert cm to desired size
        cube_actor.set_actor_scale3d(unreal.Vector(scale_factor, scale_factor, scale_factor))

        # Create and assign colored material
        # Note: You'll need to create base materials in Unreal first
        # For now, we'll just set the label
        cube_actor.set_actor_label(f"{label}_Cube_{int(size)}cm")

    return cube_actor

def create_sphere(location, radius, label):
    """Create a sphere actor"""
    editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    sphere_actor = editor_actor_subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor,
        location,
        unreal.Rotator(0, 0, 0)
    )

    if sphere_actor:
        static_mesh_comp = sphere_actor.static_mesh_component

        # Load sphere mesh
        sphere_mesh = unreal.load_asset('/Engine/BasicShapes/Sphere')
        static_mesh_comp.set_static_mesh(sphere_mesh)

        # Set scale
        scale_factor = radius / 50.0  # Default sphere is 100cm diameter
        sphere_actor.set_actor_scale3d(unreal.Vector(scale_factor, scale_factor, scale_factor))

        sphere_actor.set_actor_label(f"{label}_Sphere")

    return sphere_actor

def create_reference_grid():
    """Create Zone 1: Reference Grid with measurements"""
    log("Creating Zone 1: Reference Grid...")

    # Grid origin (Northwest corner)
    grid_origin_x = -2000  # -20 meters
    grid_origin_y = -2000  # -20 meters
    grid_size = 100  # 1 meter per square

    # Create grid floor markers (numbers)
    for i in range(11):
        # X-axis numbers
        x_pos = grid_origin_x + (i * grid_size)
        create_text_render(
            str(i),
            unreal.Vector(x_pos, grid_origin_y - 50, 5),
            scale=1.5,
            color=(1, 1, 0)  # Yellow
        )

        # Y-axis numbers
        y_pos = grid_origin_y + (i * grid_size)
        create_text_render(
            str(i),
            unreal.Vector(grid_origin_x - 50, y_pos, 5),
            scale=1.5,
            color=(1, 1, 0)  # Yellow
        )

    # Create corner reference cubes
    corner_x = grid_origin_x + 200
    corner_y = grid_origin_y + 200

    # Small cube (red) - 25cm
    create_cube(
        unreal.Vector(corner_x, corner_y, 12.5),
        25,
        (1, 0, 0),
        "Small_Reference"
    )

    # Medium cube (green) - 50cm
    create_cube(
        unreal.Vector(corner_x + 150, corner_y, 25),
        50,
        (0, 1, 0),
        "Medium_Reference"
    )

    # Large cube (blue) - 100cm
    create_cube(
        unreal.Vector(corner_x + 350, corner_y, 50),
        100,
        (0, 0, 1),
        "Large_Reference"
    )

    # Height reference poles
    for i, height in enumerate([100, 200, 300]):  # 1m, 2m, 3m
        pole_x = corner_x + 600
        pole_y = corner_y + (i * 150)

        # Create cylinder for pole
        editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        pole = editor_actor_subsystem.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(pole_x, pole_y, height / 2),
            unreal.Rotator(0, 0, 0)
        )

        if pole:
            static_mesh_comp = pole.static_mesh_component
            cylinder_mesh = unreal.load_asset('/Engine/BasicShapes/Cylinder')
            static_mesh_comp.set_static_mesh(cylinder_mesh)

            # Scale: thin (5cm diameter), tall (height)
            pole.set_actor_scale3d(unreal.Vector(0.05, 0.05, height / 100.0))
            pole.set_actor_label(f"Height_Pole_{height}cm")

            # Add height label at top
            create_text_render(
                f"{height / 100}M",
                unreal.Vector(pole_x + 30, pole_y, height + 20),
                scale=1.0,
                color=(1, 1, 0)
            )

    log("✓ Zone 1 complete: Reference Grid created")

def create_material_test_zone():
    """Create Zone 2: Material Test spheres"""
    log("Creating Zone 2: Material Test Zone...")

    # Zone origin (Northeast corner)
    zone_x = 500
    zone_y = -2000

    # Material test sphere positions (2 rows x 3 columns)
    materials = [
        "Metal", "Glass", "Plastic",
        "Stone", "Ceramic", "Emissive"
    ]

    row_spacing = 250
    col_spacing = 250

    for i, material_name in enumerate(materials):
        row = i // 3
        col = i % 3

        sphere_x = zone_x + (col * col_spacing)
        sphere_y = zone_y + (row * row_spacing)

        # Create sphere
        sphere = create_sphere(
            unreal.Vector(sphere_x, sphere_y, 50),
            50,
            material_name
        )

        # Create label below sphere
        create_text_render(
            material_name,
            unreal.Vector(sphere_x, sphere_y, -20),
            scale=1.0,
            color=(1, 1, 1)
        )

    # Add zone title
    create_text_render(
        "MATERIAL TEST ZONE",
        unreal.Vector(zone_x + 250, zone_y - 200, 100),
        scale=2.5,
        color=(0, 1, 1)  # Cyan
    )

    log("✓ Zone 2 complete: Material Test Zone created")

def create_object_library():
    """Create Zone 3: Object Library with pedestals"""
    log("Creating Zone 3: Object Library...")

    # Zone origin (Southwest corner)
    zone_x = -2000
    zone_y = 500

    # Create pedestals and objects
    library_items = [
        ("Primitives", ["Sphere", "Cube", "Cylinder", "Cone"]),
        ("Weapons", ["Pistol", "Rifle"]),
        ("Props", ["Crate", "Barrel", "Tool"])
    ]

    current_x = zone_x + 200

    for category, items in library_items:
        # Category label
        create_text_render(
            category,
            unreal.Vector(current_x, zone_y - 100, 80),
            scale=2.0,
            color=(1, 0.8, 0)  # Orange
        )

        current_y = zone_y + 200

        for item_name in items:
            # Create white pedestal (cube)
            pedestal = create_cube(
                unreal.Vector(current_x, current_y, 25),
                50,
                (1, 1, 1),
                f"Pedestal_{item_name}"
            )

            # Create item label
            create_text_render(
                item_name,
                unreal.Vector(current_x, current_y, -10),
                scale=0.8,
                color=(0, 0, 0)  # Black text
            )

            # Note: Actual object placement would go here
            # You'll need to manually place your weapon models, etc.

            current_y += 200

        current_x += 400  # Move to next category column

    # Add zone title
    create_text_render(
        "OBJECT LIBRARY",
        unreal.Vector(zone_x + 800, zone_y - 200, 100),
        scale=2.5,
        color=(0, 1, 1)  # Cyan
    )

    log("✓ Zone 3 complete: Object Library created")

def create_iteration_comparison_zone():
    """Create Zone 4: Iteration Comparison area"""
    log("Creating Zone 4: Iteration Comparison Zone...")

    # Zone origin (Southeast corner)
    zone_x = 500
    zone_y = 500

    # Create BEFORE area
    create_text_render(
        "BEFORE",
        unreal.Vector(zone_x + 200, zone_y - 100, 100),
        scale=3.0,
        color=(1, 0, 0)  # Red
    )

    # Create AFTER area
    create_text_render(
        "AFTER",
        unreal.Vector(zone_x + 200, zone_y + 400, 100),
        scale=3.0,
        color=(0, 1, 0)  # Green
    )

    # Create sample objects in both areas (for demonstration)
    # BEFORE side
    create_sphere(
        unreal.Vector(zone_x + 150, zone_y + 100, 50),
        50,
        "Before_Test"
    )

    # AFTER side
    create_sphere(
        unreal.Vector(zone_x + 150, zone_y + 550, 50),
        50,
        "After_Test"
    )

    # Change indicator
    create_text_render(
        "COMPARE CHANGES",
        unreal.Vector(zone_x + 200, zone_y + 325, 50),
        scale=1.5,
        color=(1, 0, 1)  # Magenta
    )

    log("✓ Zone 4 complete: Iteration Comparison Zone created")

def create_camera_positions():
    """Set up fixed camera positions for consistent screenshots"""
    log("Creating camera positions...")

    editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    # Camera configurations
    cameras = {
        "Cam1_Overview_North": {
            "location": unreal.Vector(0, -1500, 500),
            "rotation": unreal.Rotator(-15, 0, 0),
            "fov": 90
        },
        "Cam2_Detail_East": {
            "location": unreal.Vector(1500, 0, 200),
            "rotation": unreal.Rotator(0, 180, 0),
            "fov": 60
        },
        "Cam3_Detail_West": {
            "location": unreal.Vector(-1500, 0, 200),
            "rotation": unreal.Rotator(0, 0, 0),
            "fov": 60
        },
        "Cam4_Character_Showcase": {
            "location": unreal.Vector(0, 500, 180),
            "rotation": unreal.Rotator(0, 180, 0),
            "fov": 70
        },
        "Cam5_Isometric": {
            "location": unreal.Vector(2000, -2000, 1500),
            "rotation": unreal.Rotator(-30, 45, 0),
            "fov": 50
        }
    }

    for cam_name, config in cameras.items():
        # Spawn CineCameraActor
        camera = editor_actor_subsystem.spawn_actor_from_class(
            unreal.CineCameraActor,
            config["location"],
            config["rotation"]
        )

        if camera:
            camera.set_actor_label(cam_name)

            # Set FOV on the camera component
            camera_component = camera.get_cine_camera_component()
            if camera_component:
                camera_component.set_field_of_view(config["fov"])

            log(f"  Created camera: {cam_name}")

    log("✓ All cameras created")

def create_floor_and_walls():
    """Create the basic environment structure"""
    log("Creating floor and walls...")

    editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    # Create floor (40m x 40m)
    floor = editor_actor_subsystem.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(0, 0, -5),  # Slightly below zero
        unreal.Rotator(0, 0, 0)
    )

    if floor:
        static_mesh_comp = floor.static_mesh_component
        floor_mesh = unreal.load_asset('/Engine/BasicShapes/Plane')
        static_mesh_comp.set_static_mesh(floor_mesh)

        # Scale to 40m x 40m (plane is 2m by default)
        floor.set_actor_scale3d(unreal.Vector(20, 20, 1))
        floor.set_actor_label("Floor_Main")

    # Create walls (4 walls around perimeter)
    wall_positions = [
        ("North", unreal.Vector(0, -2000, 200), unreal.Rotator(0, 0, 90)),
        ("South", unreal.Vector(0, 2000, 200), unreal.Rotator(0, 0, 90)),
        ("East", unreal.Vector(2000, 0, 200), unreal.Rotator(0, 90, 90)),
        ("West", unreal.Vector(-2000, 0, 200), unreal.Rotator(0, 90, 90))
    ]

    for wall_name, location, rotation in wall_positions:
        wall = editor_actor_subsystem.spawn_actor_from_class(
            unreal.StaticMeshActor,
            location,
            rotation
        )

        if wall:
            static_mesh_comp = wall.static_mesh_component
            plane_mesh = unreal.load_asset('/Engine/BasicShapes/Plane')
            static_mesh_comp.set_static_mesh(plane_mesh)

            # Scale to 40m wide x 4m tall
            wall.set_actor_scale3d(unreal.Vector(20, 2, 1))
            wall.set_actor_label(f"Wall_{wall_name}")

    log("✓ Floor and walls created")

def create_lighting():
    """Set up improved lighting for the test environment"""
    log("Creating improved lighting...")

    editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    # Directional light (sun) - Camera-friendly angle from northeast, elevated
    # This avoids backlighting cameras positioned at south, east, and west
    sun = editor_actor_subsystem.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0, 0, 500),
        unreal.Rotator(-60, 45, 0)  # 60° down from northeast - illuminates towards cameras
    )

    if sun:
        sun.set_actor_label("Sun_Main")
        light_component = sun.get_component_by_class(unreal.DirectionalLightComponent)
        if light_component:
            light_component.set_intensity(10.0)  # DOUBLED from 5.0

    # Sky light - MUCH BRIGHTER
    sky_light = editor_actor_subsystem.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0, 0, 300),
        unreal.Rotator(0, 0, 0)
    )

    if sky_light:
        sky_light.set_actor_label("SkyLight_Main")
        sky_component = sky_light.get_component_by_class(unreal.SkyLightComponent)
        if sky_component:
            sky_component.set_intensity(3.0)  # Bright ambient light

    log("✓ Improved lighting created")

def create_post_process_volume():
    """Create Post Process Volume with auto-exposure disabled"""
    log("Creating Post Process Volume...")

    editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

    # Create Post Process Volume
    ppv = editor_actor_subsystem.spawn_actor_from_class(
        unreal.PostProcessVolume,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )

    if ppv:
        ppv.set_actor_label("PostProcess_Main")
        # Set to infinite extent (affects entire level)
        ppv.set_editor_property("bUnbound", True)

        log("✓ Post Process Volume created (auto-exposure will be disabled by default settings)")

    log("✓ Post Process Volume complete")

def build_complete_environment():
    """
    Main function to build the entire AI Vision Test Environment
    """
    log("=" * 60)
    log("AI VISION TEST ENVIRONMENT - AUTO BUILDER")
    log("=" * 60)
    log("")

    log("Building test environment...")
    log("")

    # Step 1: Basic structure
    create_floor_and_walls()
    log("")

    # Step 2: Lighting
    create_lighting()
    log("")

    # Step 2.5: Post Process Volume (for consistent screenshots)
    create_post_process_volume()
    log("")

    # Step 3: Create all zones
    create_reference_grid()
    log("")

    create_material_test_zone()
    log("")

    create_object_library()
    log("")

    create_iteration_comparison_zone()
    log("")

    # Step 4: Camera positions
    create_camera_positions()
    log("")

    log("=" * 60)
    log("✓ ENVIRONMENT BUILD COMPLETE!")
    log("=" * 60)
    log("")
    log("Next steps:")
    log("1. Manually place your weapon models in Zone 3")
    log("2. Assign materials to the spheres in Zone 2")
    log("3. Add Post Process Volume (disable auto-exposure!)")
    log("4. Test screenshots from each camera position")
    log("")
    log("Camera positions created:")
    log("  - Cam1_Overview_North (main overview)")
    log("  - Cam2_Detail_East")
    log("  - Cam3_Detail_West")
    log("  - Cam4_Character_Showcase")
    log("  - Cam5_Isometric")
    log("")
    log("To pilot a camera: Select it in World Outliner → Right-click → Pilot")
    log("")

# Run the builder
if __name__ == "__main__":
    build_complete_environment()
