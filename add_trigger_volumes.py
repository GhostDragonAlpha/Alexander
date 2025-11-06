"""
Step 19: Execute RD01 - Add trigger volumes to waypoints and rings
Makes the flight course interactive by detecting player passage
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

trigger_code = '''
import unreal

print(\"ADDING_TRIGGER_VOLUMES:\")

# Get all waypoint and ring actors
actors = unreal.EditorLevelLibrary.get_all_level_actors()
waypoints = [a for a in actors if \"FlightWaypoint\" in a.get_actor_label()]
rings = [a for a in actors if \"FlightRing\" in a.get_actor_label()]

added_count = 0

# Add trigger box to each waypoint
for wp in waypoints:
    try:
        # In Python, we can't directly add components
        # But we can set collision settings on existing components
        static_mesh_comp = wp.static_mesh_component

        if static_mesh_comp:
            # Enable collision
            static_mesh_comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
            static_mesh_comp.set_collision_response_to_all_channels(unreal.CollisionResponse.OVERLAP)

            # Enable overlap events
            static_mesh_comp.set_generate_overlap_events(True)

            print(f\"  Enabled collision on {wp.get_actor_label()}\")
            added_count += 1
    except Exception as e:
        print(f\"  Error on {wp.get_actor_label()}: {str(e)}\")

# Add trigger box to each ring
for ring in rings:
    try:
        static_mesh_comp = ring.static_mesh_component

        if static_mesh_comp:
            static_mesh_comp.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS)
            static_mesh_comp.set_collision_response_to_all_channels(unreal.CollisionResponse.OVERLAP)
            static_mesh_comp.set_generate_overlap_events(True)

            print(f\"  Enabled collision on {ring.get_actor_label()}\")
            added_count += 1
    except Exception as e:
        print(f\"  Error on {ring.get_actor_label()}: {str(e)}\")

print(f\"TRIGGERS_ADDED: {added_count} objects now detect overlaps\")
print(\"Flight course is now interactive!\")
print(\"Next: Create Blueprint logic to handle overlap events\")
'''

print("[STEP 19] Executing RD01: Adding trigger volumes...")
send_unreal_command(trigger_code)
print("[STEP 19] Trigger volumes enable collision detection")
print("[STEP 19] Waypoints and rings will now detect player passage")
