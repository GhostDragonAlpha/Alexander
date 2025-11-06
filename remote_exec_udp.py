"""
Execute Python command in Unreal via UDP multicast
"""
import socket
import json
import sys

def execute_unreal_command(python_code):
    """Send Python code to Unreal via UDP multicast"""
    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    # Unreal's multicast group from settings
    multicast_group = ('239.0.0.1', 6766)

    # Create command message
    message = json.dumps({
        'command': 'remote_exec',
        'data': python_code
    })

    # Send to multicast group
    sock.sendto(message.encode('utf-8'), multicast_group)
    sock.close()

    return True

if __name__ == "__main__":
    # Test command
    code = """
import unreal
world = unreal.EditorLevelLibrary.get_editor_world()
if world:
    print(f"Current Level: {world.get_name()}")
else:
    print("No level loaded")
"""

    print("Sending command to Unreal...")
    execute_unreal_command(code)
    print("Command sent via UDP multicast to 239.0.0.1:6766")
