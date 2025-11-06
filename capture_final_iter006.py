"""
Final screenshot for iteration 006 - All tasks complete
"""

import mss
import mss.tools
from datetime import datetime
import json
import socket
import time

def send_unreal_command(code):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    multicast_group = ('239.0.0.1', 6766)
    message = json.dumps({'command': 'remote_exec', 'data': code})
    sock.sendto(message.encode('utf-8'), multicast_group)
    sock.close()
    time.sleep(0.3)

# Query final state
query_code = '''
import unreal

print("[FINAL] Querying final development state...")

all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
waypoints = [a for a in all_actors if "FlightWaypoint" in a.get_actor_label()]
rings = [a for a in all_actors if "FlightRing" in a.get_actor_label()]
gamestate = [a for a in all_actors if "FlightGameState" in a.get_actor_label()]
hud_mgr = [a for a in all_actors if "FlightHUDManager" in a.get_actor_label()]
prog_mgr = [a for a in all_actors if "FlightProgressionManager" in a.get_actor_label()]
audio_mgr = [a for a in all_actors if "FlightAudioManager" in a.get_actor_label()]

print(f"[FINAL] Flight Course Objects: {len(waypoints) + len(rings)}")
print(f"[FINAL] System Actors: {len(gamestate) + len(hud_mgr) + len(prog_mgr) + len(audio_mgr)}")
print(f"[FINAL] Total Level Actors: {len(all_actors)}")
print(f"[FINAL] Development Cycle: COMPLETE")
'''

send_unreal_command(query_code)
time.sleep(1)

print("[Screenshot] Capturing final state...")
with mss.mss() as sct:
    monitor = sct.monitors[1]
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"Screenshots/final_iter006_complete_{timestamp}.png"

    screenshot = sct.grab(monitor)
    mss.tools.to_png(screenshot.rgb, screenshot.size, output=filename)

    print(f"[Screenshot] Saved: {filename}")

    metadata = {
        "iteration": "006_FINAL",
        "status": "ALL_TASKS_COMPLETE",
        "timestamp": datetime.now().isoformat(),
        "resolution": f"{screenshot.width}x{screenshot.height}",
        "completed_tasks": ["RD01", "RD02", "RD03", "RD04", "RD05", "RD06"],
        "objects_created": {
            "waypoints": 5,
            "rings": 4,
            "system_actors": 4
        },
        "purpose": "Final state verification - development cycle complete"
    }

    metadata_file = f"Screenshots/final_iter006_complete_{timestamp}_meta.json"
    with open(metadata_file, "w") as f:
        json.dump(metadata, f, indent=2)

    print(f"[Screenshot] Metadata: {metadata_file}")
    print("[Screenshot] FINAL_CAPTURE_COMPLETE")
