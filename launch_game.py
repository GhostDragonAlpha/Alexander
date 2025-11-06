#!/usr/bin/env python3
"""
Universal Game Launcher with Mode Switching

Usage:
  python launch_game.py --mode=automated   # 3 clients, VR disabled, multi-client testing
  python launch_game.py --mode=manual      # 1 client, VR enabled, manual testing
  python launch_game.py --mode=vr          # Alias for manual
"""

import subprocess
import sys
import time
import requests

UNREAL_EDITOR = r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe"
PROJECT_FILE = r"C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject"
MAP_NAME = "VRTemplateMap"
GAME_MODE = "/Script/Alexander.AutomationGameMode"

def launch_automated_mode():
    """Launch 3 clients for automated testing (VR DISABLED)"""
    print("="*60)
    print("AUTOMATED TESTING MODE")
    print("="*60)
    print("Launching 3 game clients with VR DISABLED")
    print("Ports: 8080, 8081, 8082")
    print("Ready for multi-client consensus testing")
    print("="*60 + "\n")

    PORTS = [8080, 8081, 8082]
    processes = []

    for port in PORTS:
        print(f"Launching client on port {port}...")
        cmd = [
            UNREAL_EDITOR,
            PROJECT_FILE,
            f"{MAP_NAME}?game={GAME_MODE}",
            "-game",
            "-windowed",
            "-ResX=800",
            "-ResY=600",
            f"-HTTPPort={port}",
            "-nohmd",  # DISABLE VR for multi-client testing
            "-log"
        ]

        process = subprocess.Popen(
            cmd,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            creationflags=subprocess.CREATE_NEW_CONSOLE
        )

        processes.append({"port": port, "process": process})
        print(f"  Client {port} started (PID: {process.pid})")
        time.sleep(5)  # Stagger launches

    print("\nAll clients launched. Waiting for HTTP APIs...")

    # Wait for all to be ready
    for client in processes:
        port = client["port"]
        print(f"Waiting for port {port}...", end="", flush=True)

        start_time = time.time()
        while time.time() - start_time < 180:
            try:
                response = requests.get(f"http://localhost:{port}/status", timeout=2)
                if response.status_code == 200:
                    elapsed = int(time.time() - start_time)
                    print(f" READY! (took {elapsed}s)")
                    break
            except:
                pass
            time.sleep(3)
        else:
            print(f" TIMEOUT after 180s")

    print("\n" + "="*60)
    print("AUTOMATED MODE READY")
    print("="*60)
    print("All 3 clients are running.")
    print("HTTP APIs: localhost:8080, localhost:8081, localhost:8082")
    print("\nRun test scripts:")
    print("  python test_position_reporting.py")
    print("  python launch_multi_client_consensus_test.py")
    print("\nPress Ctrl+C to shutdown all clients.")
    print("="*60 + "\n")

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\n\nShutting down all clients...")
        for client in processes:
            client["process"].terminate()
        print("Done.")

def launch_manual_mode():
    """Launch single client for manual VR testing (VR ENABLED)"""
    print("="*60)
    print("MANUAL VR MODE")
    print("="*60)
    print("Launching single game client with VR ENABLED")
    print("Port: 8080")
    print("Put on your VR headset and enjoy!")
    print("="*60 + "\n")

    cmd = [
        UNREAL_EDITOR,
        PROJECT_FILE,
        f"{MAP_NAME}?game={GAME_MODE}",
        "-game",
        "-windowed",
        "-ResX=1280",
        "-ResY=720",
        "-HTTPPort=8080",
        # NO -nohmd flag = VR ENABLED
        "-log"
    ]

    print("Starting VR client...")
    process = subprocess.Popen(
        cmd,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
        creationflags=subprocess.CREATE_NEW_CONSOLE
    )

    print(f"VR client started (PID: {process.pid})")
    print("\nWaiting for HTTP API on port 8080...", end="", flush=True)

    start_time = time.time()
    while time.time() - start_time < 180:
        try:
            response = requests.get("http://localhost:8080/status", timeout=2)
            if response.status_code == 200:
                elapsed = int(time.time() - start_time)
                print(f" READY! (took {elapsed}s)")
                break
        except:
            pass
        time.sleep(3)
    else:
        print(" TIMEOUT after 180s")

    print("\n" + "="*60)
    print("MANUAL VR MODE READY")
    print("="*60)
    print("VR client is running on port 8080")
    print("HTTP API: http://localhost:8080")
    print("\nPut on your VR headset and play!")
    print("Press Ctrl+C to shutdown.")
    print("="*60 + "\n")

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\n\nShutting down VR client...")
        process.terminate()
        print("Done.")

def print_usage():
    """Print usage instructions"""
    print("="*60)
    print("GAME LAUNCHER - MODE SWITCHER")
    print("="*60)
    print("\nUsage:")
    print("  python launch_game.py --mode=automated")
    print("    → Launch 3 clients with VR DISABLED for automated testing")
    print()
    print("  python launch_game.py --mode=manual")
    print("    → Launch 1 client with VR ENABLED for manual testing")
    print()
    print("  python launch_game.py --mode=vr")
    print("    → Alias for manual mode")
    print()
    print("Examples:")
    print("  # Automated multi-client testing:")
    print("  python launch_game.py --mode=automated")
    print("  python test_position_reporting.py")
    print()
    print("  # Manual VR gameplay:")
    print("  python launch_game.py --mode=manual")
    print("  # Put on headset and play!")
    print()
    print("="*60)

def main():
    """Main entry point"""
    if len(sys.argv) < 2:
        print("ERROR: Missing --mode argument\n")
        print_usage()
        return 1

    mode_arg = sys.argv[1]

    if not mode_arg.startswith("--mode="):
        print(f"ERROR: Invalid argument '{mode_arg}'\n")
        print_usage()
        return 1

    mode = mode_arg.split("=")[1].lower()

    if mode == "automated":
        launch_automated_mode()
    elif mode in ["manual", "vr"]:
        launch_manual_mode()
    else:
        print(f"ERROR: Unknown mode '{mode}'\n")
        print_usage()
        return 1

    return 0

if __name__ == "__main__":
    exit(main())
