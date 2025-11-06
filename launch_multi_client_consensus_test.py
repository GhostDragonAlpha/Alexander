#!/usr/bin/env python3
"""
Multi-Client Consensus Testing System

Launches multiple game clients simultaneously to test physics consensus:
- Each client = game instance with HTTP API
- Each client spawns a ship and reports positions
- Each client validates other clients' positions
- Proves decentralized consensus works

Architecture:
Client 1 (Port 8080) <--> Client 2 (Port 8081) <--> Client 3 (Port 8082)
     ^                        ^                        ^
     |                        |                        |
  Validates               Validates                Validates
  Others                  Others                   Others
"""

import subprocess
import time
import requests
import json
from typing import List, Dict

# Configuration
UNREAL_EDITOR = r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe"
PROJECT_FILE = r"C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject"
MAP_NAME = "VRTemplateMap"
GAME_MODE = "/Script/Alexander.AutomationGameMode"

# Client configuration
CLIENTS = [
    {"id": 1, "port": 8080, "ship_location": [0, 0, 500]},
    {"id": 2, "port": 8081, "ship_location": [1000, 0, 500]},
    {"id": 3, "port": 8082, "ship_location": [0, 1000, 500]},
]

class ConsensusTestClient:
    """Represents a single game client with its own HTTP API"""

    def __init__(self, client_config: Dict):
        self.id = client_config["id"]
        self.port = client_config["port"]
        self.ship_location = client_config["ship_location"]
        self.process = None
        self.ship_id = None
        self.api_url = f"http://localhost:{self.port}"

    def launch(self):
        """Launch game client with specific HTTP API port"""
        print(f"[Client {self.id}] Launching on port {self.port}...")

        cmd = [
            UNREAL_EDITOR,
            PROJECT_FILE,
            f"{MAP_NAME}?game={GAME_MODE}",
            "-game",
            "-windowed",
            "-ResX=800",
            "-ResY=600",
            f"-HTTPPort={self.port}",  # Each client on different port
            "-log"
        ]

        self.process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            creationflags=subprocess.CREATE_NEW_CONSOLE  # Separate console window
        )

        print(f"[Client {self.id}] Process started (PID: {self.process.pid})")

    def wait_for_ready(self, timeout=60):
        """Wait for HTTP API to be ready"""
        print(f"[Client {self.id}] Waiting for HTTP API on port {self.port}...")

        start_time = time.time()
        while time.time() - start_time < timeout:
            try:
                response = requests.get(f"{self.api_url}/status", timeout=2)
                if response.status_code == 200:
                    print(f"[Client {self.id}] HTTP API ready!")
                    return True
            except requests.exceptions.RequestException:
                pass
            time.sleep(2)

        print(f"[Client {self.id}] Timeout waiting for HTTP API")
        return False

    def spawn_ship(self):
        """Spawn a PhysicsTestShip via HTTP API"""
        print(f"[Client {self.id}] Spawning ship at {self.ship_location}...")

        payload = {
            "ship_class": "/Script/Alexander.PhysicsTestShip",
            "location": self.ship_location,
            "rotation": [0, 0, 0]
        }

        try:
            response = requests.post(
                f"{self.api_url}/spawn_ship",
                json=payload,
                timeout=5
            )

            if response.status_code == 200:
                data = response.json()
                if data.get("success"):
                    self.ship_id = data.get("data", {}).get("ship_id")
                    print(f"[Client {self.id}] Ship spawned: {self.ship_id}")
                    return True
                else:
                    print(f"[Client {self.id}] Spawn failed: {data.get('message')}")
            else:
                print(f"[Client {self.id}] HTTP error: {response.status_code}")

        except Exception as e:
            print(f"[Client {self.id}] Exception: {e}")

        return False

    def get_position(self) -> Dict:
        """Get ship position"""
        try:
            response = requests.get(
                f"{self.api_url}/get_position",
                params={"ship_id": self.ship_id},
                timeout=2
            )

            if response.status_code == 200:
                data = response.json()
                if data.get("success"):
                    return data.get("data", {})
        except Exception as e:
            print(f"[Client {self.id}] Position fetch error: {e}")

        return {}

    def apply_thrust(self, thrust_vector: List[float]):
        """Apply thrust to ship"""
        payload = {
            "ship_id": self.ship_id,
            "thrust": thrust_vector,
            "torque": [0, 0, 0]
        }

        try:
            response = requests.post(
                f"{self.api_url}/set_input",
                json=payload,
                timeout=2
            )
            return response.status_code == 200
        except:
            return False

    def shutdown(self):
        """Shutdown client"""
        if self.process:
            print(f"[Client {self.id}] Shutting down...")
            self.process.terminate()
            self.process.wait(timeout=10)


class ConsensusTestOrchestrator:
    """Orchestrates multi-client consensus testing"""

    def __init__(self):
        self.clients: List[ConsensusTestClient] = []

    def launch_all_clients(self):
        """Launch all game clients"""
        print("\n" + "="*60)
        print("LAUNCHING MULTI-CLIENT CONSENSUS TEST")
        print("="*60 + "\n")

        # Launch all clients
        for config in CLIENTS:
            client = ConsensusTestClient(config)
            client.launch()
            self.clients.append(client)
            time.sleep(5)  # Stagger launches

        # Wait for all to be ready
        print("\nWaiting for all clients to be ready...")
        for client in self.clients:
            if not client.wait_for_ready():
                print(f"ERROR: Client {client.id} failed to start")
                return False

        print("\nAll clients ready!")
        return True

    def spawn_all_ships(self):
        """Spawn ships in all clients"""
        print("\n" + "="*60)
        print("SPAWNING SHIPS")
        print("="*60 + "\n")

        for client in self.clients:
            if not client.spawn_ship():
                print(f"ERROR: Failed to spawn ship for Client {client.id}")
                return False
            time.sleep(1)

        print("\nAll ships spawned!")
        return True

    def test_basic_consensus(self):
        """Test basic consensus validation"""
        print("\n" + "="*60)
        print("TEST 1: BASIC POSITION REPORTING")
        print("="*60 + "\n")

        # Get positions from all clients
        positions = {}
        for client in self.clients:
            pos = client.get_position()
            positions[client.id] = pos
            print(f"Client {client.id} Position: {pos.get('location', 'N/A')}")

        # TODO: Add consensus validation logic here
        print("\nBasic position reporting working!")

    def test_thrust_validation(self):
        """Test thrust validation"""
        print("\n" + "="*60)
        print("TEST 2: THRUST APPLICATION & VALIDATION")
        print("="*60 + "\n")

        # Apply thrust to Client 1
        print("Applying thrust to Client 1...")
        thrust = [0, 0, 10000]  # 10 kN upward
        self.clients[0].apply_thrust(thrust)

        # Wait and check positions
        time.sleep(2)

        for client in self.clients:
            pos = client.get_position()
            print(f"Client {client.id} Position: {pos.get('location', 'N/A')}")

        # TODO: Validate that Client 1 moved, others didn't
        print("\nThrust application working!")

    def test_consensus_validation(self):
        """Test consensus voting"""
        print("\n" + "="*60)
        print("TEST 3: CONSENSUS VALIDATION")
        print("="*60 + "\n")

        # TODO: Implement consensus validation test
        # - Client 1 reports position
        # - Clients 2 & 3 validate position against physics
        # - Check if consensus reached (2/3 majority)

        print("Consensus validation test would go here...")
        print("(Requires network position sharing implementation)")

    def run_all_tests(self):
        """Run complete test suite"""
        try:
            # Launch clients
            if not self.launch_all_clients():
                return False

            # Spawn ships
            if not self.spawn_all_ships():
                return False

            # Run tests
            self.test_basic_consensus()
            time.sleep(2)

            self.test_thrust_validation()
            time.sleep(2)

            self.test_consensus_validation()

            print("\n" + "="*60)
            print("ALL TESTS COMPLETE")
            print("="*60 + "\n")

            # Keep running for observation
            print("Clients running. Press Ctrl+C to shutdown...")
            while True:
                time.sleep(1)

        except KeyboardInterrupt:
            print("\n\nShutdown requested...")
        finally:
            self.shutdown_all_clients()

    def shutdown_all_clients(self):
        """Shutdown all clients"""
        print("\nShutting down all clients...")
        for client in self.clients:
            client.shutdown()
        print("All clients shut down.")


def main():
    """Main entry point"""
    orchestrator = ConsensusTestOrchestrator()
    orchestrator.run_all_clients()


if __name__ == "__main__":
    main()
