"""
Create a flight course with waypoints in the FlightTest level
This is REAL game development - adding actual gameplay content
"""

import socket
import json

def send_unreal_command(code):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    multicast_group = ('239.0.0.1', 6766)
    message = json.dumps({'command': 'remote_exec', 'data': code})
    sock.sendto(message.encode('utf-8'), multicast_group)
    sock.close()

# REAL DEVELOPMENT: Create a flight course
flight_course_code = '''
import unreal

# Get current world
world = unreal.EditorLevelLibrary.get_editor_world()

# Create waypoint locations for a flight course
waypoint_positions = [
    (1000, 0, 500),     # Waypoint 1
    (2000, 1000, 800),  # Waypoint 2
    (3000, 0, 1200),    # Waypoint 3
    (4000, -1000, 800), # Waypoint 4
    (5000, 0, 500),     # Waypoint 5 (finish)
]

print(\"CREATING FLIGHT COURSE:\")
print(f\"Adding {len(waypoint_positions)} waypoints\")

for i, (x, y, z) in enumerate(waypoint_positions, 1):
    # Spawn a sphere to mark the waypoint
    location = unreal.Vector(x, y, z)
    rotation = unreal.Rotator(0, 0, 0)

    # Create a static mesh actor for the waypoint
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        location,
        rotation
    )

    if actor:
        # Name it
        actor.set_actor_label(f\"Waypoint_{i}\")

        # Set the mesh to a sphere
        static_mesh_comp = actor.static_mesh_component

        # Load sphere mesh
        sphere_mesh = unreal.load_asset(\"/Engine/BasicShapes/Sphere\")
        if sphere_mesh:
            static_mesh_comp.set_static_mesh(sphere_mesh)

            # Scale it up so it's visible
            actor.set_actor_scale3d(unreal.Vector(3.0, 3.0, 3.0))

            print(f\"Created Waypoint {i} at ({x}, {y}, {z})\")
        else:
            print(f\"Could not load sphere mesh for waypoint {i}\")
    else:
        print(f\"Failed to spawn waypoint {i}\")

print(\"FLIGHT_COURSE_CREATED\")
print(\"Course ready for flight testing!\")
'''

print("[REAL DEV] Creating actual flight course with waypoints...")
send_unreal_command(flight_course_code)
print("[REAL DEV] Flight course creation command sent")
print("[REAL DEV] This adds 5 waypoints forming a flight path")
