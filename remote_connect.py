"""
Connect to running Unreal Editor via remote execution
"""
import socket
import json

# Try to connect to Unreal's Python remote execution port (default 9998)
HOST = 'localhost'
PORT = 9998

try:
    # Create socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(2.0)

    # Connect
    print(f"Attempting to connect to Unreal Editor at {HOST}:{PORT}...")
    sock.connect((HOST, PORT))
    print(f"✓ Connected!")

    # Send a test command
    command = {
        'command': 'remote_exec',
        'data': 'print("Hello from remote connection!")'
    }

    sock.sendall(json.dumps(command).encode('utf-8') + b'\n')

    # Receive response
    response = sock.recv(4096)
    print(f"Response: {response.decode('utf-8')}")

    sock.close()

except ConnectionRefusedError:
    print(f"✗ Connection refused. Remote execution may not be enabled.")
    print("  Enable it in: Edit > Project Settings > Plugins > Python > Enable Remote Execution")
except Exception as e:
    print(f"✗ Error: {e}")
