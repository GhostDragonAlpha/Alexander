"""
Capture screenshot for iteration 002 verification
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

# Maximize Unreal first
maximize_code = '''
import unreal
print("[SCREENSHOT] Preparing viewport...")
'''
send_unreal_command(maximize_code)

print("[Screenshot] Capturing Unreal Editor viewport...")
time.sleep(1)

with mss.mss() as sct:
    monitor = sct.monitors[1]  # Primary monitor
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"Screenshots/iteration_002_gamestate_{timestamp}.png"

    screenshot = sct.grab(monitor)
    mss.tools.to_png(screenshot.rgb, screenshot.size, output=filename)

    print(f"[Screenshot] Saved: {filename}")

    # Save metadata
    metadata = {
        "iteration": "002",
        "task": "RD02_GameState_Created",
        "timestamp": datetime.now().isoformat(),
        "resolution": f"{screenshot.width}x{screenshot.height}",
        "objects_created": ["BP_FlightGameState", "FlightGameState actor"],
        "purpose": "Verify GameState tracking system creation"
    }

    metadata_file = f"Screenshots/iteration_002_gamestate_{timestamp}_meta.json"
    with open(metadata_file, "w") as f:
        json.dump(metadata, f, indent=2)

    print(f"[Screenshot] Metadata: {metadata_file}")
    print("[Screenshot] CAPTURE_COMPLETE")
