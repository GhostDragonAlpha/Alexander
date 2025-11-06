"""
Remote Python Execution Client for Unreal Editor
Sends commands to RUNNING editor via UDP multicast
"""

import socket
import sys
import time
import argparse

# Unreal Python Remote Execution defaults
MULTICAST_GROUP = "239.0.0.1"
MULTICAST_PORT = 6766

def send_command(command_text):
    """Send Python command to running Unreal Editor"""
    import json

    print(f"\n[REMOTE] Sending command to editor...")
    print(f"[REMOTE] Target: {MULTICAST_GROUP}:{MULTICAST_PORT}")

    try:
        # Wrap command in JSON format expected by Unreal Python Remote Execution
        command_json = {
            "magic": "ue_py",
            "version": 1,
            "type": "command",
            "source": "remote",
            "command": command_text,
            "unattended": True
        }

        # Create UDP socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 2)

        # Send JSON-wrapped command
        message = json.dumps(command_json).encode('utf-8')
        sock.sendto(message, (MULTICAST_GROUP, MULTICAST_PORT))

        print(f"[REMOTE] [OK] Sent {len(message)} bytes (JSON wrapped)")
        sock.close()
        return True

    except Exception as e:
        print(f"[REMOTE] [FAIL] Error: {e}")
        return False

def send_file(filepath):
    """Send Python file contents to running editor"""

    print(f"\n[REMOTE] Reading file: {filepath}")

    try:
        with open(filepath, 'r') as f:
            code = f.read()

        print(f"[REMOTE] File loaded ({len(code)} bytes)")
        return send_command(code)

    except Exception as e:
        print(f"[REMOTE] [FAIL] Error reading file: {e}")
        return False

def test_connection():
    """Test if editor is responding"""

    test_code = """
import unreal
unreal.log("Remote execution test successful!")
print("REMOTE TEST: Connection OK")
"""
    return send_command(test_code)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Send Python commands to running Unreal Editor')
    parser.add_argument('--file', help='Python file to execute')
    parser.add_argument('--command', help='Direct Python command to execute')
    parser.add_argument('--test', action='store_true', help='Test connection')

    args = parser.parse_args()

    if args.test:
        print("=" * 70)
        print("TESTING REMOTE CONNECTION")
        print("=" * 70)
        if test_connection():
            print("\n[OK] Connection test sent")
            print("  Check Unreal Editor's Output Log for confirmation")
        else:
            print("\n[FAIL] Connection test failed")
            print("  Make sure:")
            print("    1. Unreal Editor is running")
            print("    2. Python Remote Execution is enabled")
            print("       (Edit → Project Settings → Python → Remote Execution)")

    elif args.file:
        print("=" * 70)
        print(f"EXECUTING FILE: {args.file}")
        print("=" * 70)
        if send_file(args.file):
            print("\n[OK] File sent to editor")
            print("  Check Unreal Editor's Output Log for results")
        else:
            print("\n[FAIL] Failed to send file")

    elif args.command:
        print("=" * 70)
        print("EXECUTING COMMAND")
        print("=" * 70)
        if send_command(args.command):
            print("\n[OK] Command sent to editor")
        else:
            print("\n[FAIL] Failed to send command")

    else:
        parser.print_help()
