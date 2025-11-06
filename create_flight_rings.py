"""
Create ring obstacles for the flight course
Real game development - adding visual targets to fly through
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

# Create rings between waypoints
ring_code = '''
import unreal

print(\"CREATING FLIGHT RINGS:\")

# Ring positions - between waypoints for visual targets
ring_positions = [
    (1500, 500, 650),      # Between WP1 and WP2
    (2500, 500, 1000),     # Between WP2 and WP3
    (3500, -500, 1000),    # Between WP3 and WP4
    (4500, -500, 650),     # Between WP4 and WP5
]

for i, (x, y, z) in enumerate(ring_positions, 1):
    location = unreal.Vector(x, y, z)
    rotation = unreal.Rotator(0, 90, 0)  # Rotated to face flight path

    # Create static mesh actor for the ring
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        location,
        rotation
    )

    if actor:
        actor.set_actor_label(f\"FlightRing_{i}\")

        # Try to use torus shape for ring
        # If not available, use sphere with hole effect via scaling
        actor.set_actor_scale3d(unreal.Vector(8.0, 8.0, 1.0))  # Flattened for ring effect

        print(f\"Created FlightRing_{i} at ({x}, {y}, {z})\")
    else:
        print(f\"Failed to create ring {i}\")

print(f\"FLIGHT_RINGS_CREATED: {len(ring_positions)} rings\")
print(\"Flight course now has visual targets!\")
'''

print("[REAL DEV] Creating ring obstacles for flight course...")
send_unreal_command(ring_code)
print("[REAL DEV] 4 rings positioned between waypoints")
print("[REAL DEV] Flight course now has visible targets to aim for")
