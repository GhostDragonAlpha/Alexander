#!/usr/bin/env python3
"""
Simple 3-Client Launcher
Launches 3 game clients on ports 8080, 8081, 8082 and waits for them to be ready
"""

import subprocess
import time
import requests

UNREAL_EDITOR = r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe"
PROJECT_FILE = r"C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject"
MAP_NAME = "VRTemplateMap"
GAME_MODE = "/Script/Alexander.AutomationGameMode"

PORTS = [8080, 8081, 8082]

def launch_client(port):
    """Launch a single game client"""
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
        "-log"
    ]

    process = subprocess.Popen(
        cmd,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
        creationflags=subprocess.CREATE_NEW_CONSOLE
    )

    print(f"Client on port {port} started (PID: {process.pid})")
    return process

def wait_for_client(port, timeout=180):
    """Wait for a client's HTTP API to be ready"""
    print(f"Waiting for port {port} to be ready...")
    start_time = time.time()

    while time.time() - start_time < timeout:
        try:
            response = requests.get(f"http://localhost:{port}/status", timeout=2)
            if response.status_code == 200:
                elapsed = int(time.time() - start_time)
                print(f"Port {port} is ready! (took {elapsed}s)")
                return True
        except:
            pass

        time.sleep(3)

    print(f"Port {port} timed out after {timeout}s")
    return False

def main():
    print("="*60)
    print("LAUNCHING 3 GAME CLIENTS")
    print("="*60)

    # Launch all 3 clients
    processes = []
    for port in PORTS:
        process = launch_client(port)
        processes.append(process)
        time.sleep(5)  # Stagger launches

    print("\nAll clients launched, waiting for HTTP APIs...")

    # Wait for all to be ready
    for port in PORTS:
        if not wait_for_client(port):
            print(f"ERROR: Port {port} failed to start")
            return False

    print("\n" + "="*60)
    print("ALL 3 CLIENTS READY!")
    print("="*60)
    print("\nPorts 8080, 8081, 8082 are all responding.")
    print("Ready for multi-client testing.")
    print("\nClients will keep running. Press Ctrl+C to exit.")

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nShutting down...")
        for process in processes:
            process.terminate()
        print("Done.")

    return True

if __name__ == "__main__":
    success = main()
    exit(0 if success else 1)
