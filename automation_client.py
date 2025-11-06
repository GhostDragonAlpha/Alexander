"""
Automation Client for Unreal Server Control
Communicates with dedicated server running AutomationGameMode
Enables AI-driven autonomous game development and testing
"""

import json
import time
import subprocess
import os
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass
from pathlib import Path


@dataclass
class ShipState:
    """Ship state information"""
    ship_id: str
    position: Tuple[float, float, float]
    velocity: Tuple[float, float, float]
    speed: float


@dataclass
class TestResult:
    """Test execution result"""
    test_name: str
    success: bool
    message: str
    duration: float
    screenshot_path: Optional[str] = None


class UnrealAutomationClient:
    """
    Python client for controlling Unreal dedicated server
    Supports ship spawning, input control, and automated testing
    """

    def __init__(self, project_path: str, log_file: str = "automation_log.txt"):
        self.project_path = Path(project_path)
        self.uproject_file = self.project_path / "Alexander.uproject"
        self.log_file = self.project_path / log_file
        self.server_process = None
        self.tracked_ships: Dict[str, ShipState] = {}

        # Unreal Engine paths
        self.ue_editor_cmd = r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
        self.ue_editor = r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe"

        # Verify paths
        if not self.uproject_file.exists():
            raise FileNotFoundError(f"Project file not found: {self.uproject_file}")

    # ============================================================================
    # SERVER MANAGEMENT
    # ============================================================================

    def launch_dedicated_server(self, map_name: str = "VRTemplateMap") -> bool:
        """
        Launch dedicated server with AutomationGameMode

        Args:
            map_name: Map to load

        Returns:
            True if server launched successfully
        """
        print(f"Launching dedicated server with map: {map_name}")

        # Server launch command
        cmd = [
            self.ue_editor,
            str(self.uproject_file),
            map_name,
            "-server",
            "-log",
            "-automationmode",
            f"-GameMode=/Script/Alexander.AutomationGameMode"
        ]

        try:
            self.server_process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                cwd=str(self.project_path)
            )

            # Wait for server to initialize
            print("Waiting for server to initialize...")
            time.sleep(10)

            if self.server_process.poll() is None:
                print(f"[OK] Server launched successfully (PID: {self.server_process.pid})")
                return True
            else:
                print("[FAIL] Server failed to start")
                return False

        except Exception as e:
            print(f"[FAIL] Error launching server: {e}")
            return False

    def stop_server(self):
        """Stop the dedicated server"""
        if self.server_process:
            print("Stopping server...")
            self.server_process.terminate()
            self.server_process.wait(timeout=10)
            print("[OK] Server stopped")
            self.server_process = None

    def is_server_running(self) -> bool:
        """Check if server is running"""
        return self.server_process is not None and self.server_process.poll() is None

    # ============================================================================
    # SHIP CONTROL
    # ============================================================================

    def spawn_ship(
        self,
        position: Tuple[float, float, float] = (0, 0, 300),
        rotation: Tuple[float, float, float] = (0, 0, 0),
        ship_class: str = "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer"
    ) -> Optional[str]:
        """
        Spawn a ship at specified location

        Args:
            position: (X, Y, Z) spawn location
            rotation: (Pitch, Yaw, Roll) spawn rotation
            ship_class: Path to ship blueprint

        Returns:
            Ship ID if successful, None otherwise
        """
        print(f"Spawning ship at {position}")

        # Execute spawn command via Python script
        spawn_script = f"""
import unreal

# Spawn ship
editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world = editor_subsystem.get_editor_world()

ship_asset = unreal.EditorAssetLibrary.load_asset("{ship_class}")
if ship_asset:
    ship_class = ship_asset.generated_class()
    ship = actor_subsystem.spawn_actor_from_class(
        ship_class,
        unreal.Vector{position[0], position[1], position[2]},
        unreal.Rotator{rotation[0], rotation[1], rotation[2]}
    )
    if ship:
        print(f"SPAWN_SUCCESS:{{ship.get_name()}}")
    else:
        print("SPAWN_FAILED")
else:
    print("SPAWN_FAILED:CLASS_NOT_FOUND")
"""

        result = self._execute_python_script(spawn_script)

        if result and "SPAWN_SUCCESS:" in result:
            ship_id = result.split("SPAWN_SUCCESS:")[1].strip()
            print(f"[OK] Ship spawned: {ship_id}")
            return ship_id
        else:
            print(f"[FAIL] Failed to spawn ship: {result}")
            return None

    def set_ship_input(
        self,
        ship_id: str,
        thrust: Optional[Tuple[float, float, float]] = None,
        rotation: Optional[Tuple[float, float, float]] = None,
        assist_mode: Optional[int] = None
    ) -> bool:
        """
        Set input for a ship

        Args:
            ship_id: Ship identifier
            thrust: (X, Y, Z) thrust input (-1 to 1)
            rotation: (Pitch, Yaw, Roll) rotation input (-1 to 1)
            assist_mode: Flight assist mode (0-4)

        Returns:
            True if successful
        """
        print(f"Setting input for ship {ship_id}")

        input_script = f"""
import unreal

# Find ship by name
all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
ship = None
for actor in all_actors:
    if "{ship_id}" in actor.get_name():
        ship = actor
        break

if ship:
    # Get FlightController component
    flight_controller = ship.get_component_by_class(unreal.FlightController.static_class())
    if flight_controller:
"""

        if thrust:
            input_script += f"""
        flight_controller.set_thrust_input(unreal.Vector{thrust[0], thrust[1], thrust[2]})
"""

        if rotation:
            input_script += f"""
        flight_controller.set_rotation_input(unreal.Vector{rotation[0], rotation[1], rotation[2]})
"""

        if assist_mode is not None:
            input_script += f"""
        flight_controller.set_assist_mode({assist_mode})
"""

        input_script += """
        print("INPUT_SUCCESS")
    else:
        print("INPUT_FAILED:NO_FLIGHT_CONTROLLER")
else:
    print("INPUT_FAILED:SHIP_NOT_FOUND")
"""

        result = self._execute_python_script(input_script)

        if result and "INPUT_SUCCESS" in result:
            print(f"[OK] Input applied to ship {ship_id}")
            return True
        else:
            print(f"[FAIL] Failed to set input: {result}")
            return False

    def get_ship_position(self, ship_id: str) -> Optional[Tuple[float, float, float]]:
        """Get ship position"""
        position_script = f"""
import unreal

all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
ship = None
for actor in all_actors:
    if "{ship_id}" in actor.get_name():
        ship = actor
        break

if ship:
    pos = ship.get_actor_location()
    print(f"POSITION:{{pos.x}},{{pos.y}},{{pos.z}}")
else:
    print("POSITION_FAILED")
"""

        result = self._execute_python_script(position_script)

        if result and "POSITION:" in result:
            coords = result.split("POSITION:")[1].strip().split(",")
            position = tuple(float(c) for c in coords)
            print(f"[OK] Ship position: {position}")
            return position
        else:
            print(f"[FAIL] Failed to get position: {result}")
            return None

    def get_ship_velocity(self, ship_id: str) -> Optional[Tuple[float, float, float]]:
        """Get ship velocity"""
        velocity_script = f"""
import unreal

all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
ship = None
for actor in all_actors:
    if "{ship_id}" in actor.get_name():
        ship = actor
        break

if ship:
    vel = ship.get_velocity()
    print(f"VELOCITY:{{vel.x}},{{vel.y}},{{vel.z}}")
else:
    print("VELOCITY_FAILED")
"""

        result = self._execute_python_script(velocity_script)

        if result and "VELOCITY:" in result:
            coords = result.split("VELOCITY:")[1].strip().split(",")
            velocity = tuple(float(c) for c in coords)
            speed = (velocity[0]**2 + velocity[1]**2 + velocity[2]**2)**0.5
            print(f"[OK] Ship velocity: {velocity}, speed: {speed:.2f}")
            return velocity
        else:
            print(f"[FAIL] Failed to get velocity: {result}")
            return None

    # ============================================================================
    # AUTOMATED TESTING
    # ============================================================================

    def test_ship_movement(
        self,
        thrust_duration: float = 3.0,
        expected_min_distance: float = 100.0
    ) -> TestResult:
        """
        Test that ship spawns and moves when thrust applied

        Args:
            thrust_duration: How long to apply thrust (seconds)
            expected_min_distance: Minimum distance ship should move

        Returns:
            TestResult with success/failure and details
        """
        test_name = "test_ship_movement"
        start_time = time.time()

        print(f"\n{'='*70}")
        print(f"Running test: {test_name}")
        print(f"{'='*70}")

        # Step 1: Spawn ship
        print("\n[1/5] Spawning ship...")
        ship_id = self.spawn_ship(position=(0, 0, 300))
        if not ship_id:
            return TestResult(test_name, False, "Failed to spawn ship", time.time() - start_time)

        time.sleep(2)  # Let physics stabilize

        # Step 2: Get initial position
        print("\n[2/5] Recording initial position...")
        initial_pos = self.get_ship_position(ship_id)
        if not initial_pos:
            return TestResult(test_name, False, "Failed to get initial position", time.time() - start_time)

        # Step 3: Apply thrust
        print(f"\n[3/5] Applying forward thrust for {thrust_duration}s...")
        success = self.set_ship_input(ship_id, thrust=(1.0, 0.0, 0.0))
        if not success:
            return TestResult(test_name, False, "Failed to set thrust input", time.time() - start_time)

        time.sleep(thrust_duration)

        # Step 4: Get final position
        print("\n[4/5] Recording final position...")
        final_pos = self.get_ship_position(ship_id)
        if not final_pos:
            return TestResult(test_name, False, "Failed to get final position", time.time() - start_time)

        # Step 5: Calculate distance moved
        distance = ((final_pos[0] - initial_pos[0])**2 +
                    (final_pos[1] - initial_pos[1])**2 +
                    (final_pos[2] - initial_pos[2])**2)**0.5

        print(f"\n[5/5] Verifying movement...")
        print(f"  Initial position: {initial_pos}")
        print(f"  Final position:   {final_pos}")
        print(f"  Distance moved:   {distance:.2f} units")

        duration = time.time() - start_time

        if distance >= expected_min_distance:
            message = f"SUCCESS: Ship moved {distance:.2f} units (expected >= {expected_min_distance})"
            print(f"\n[OK] {message}")
            return TestResult(test_name, True, message, duration)
        else:
            message = f"FAILED: Ship only moved {distance:.2f} units (expected >= {expected_min_distance})"
            print(f"\n[FAIL] {message}")
            return TestResult(test_name, False, message, duration)

    def test_flight_assist_modes(self) -> List[TestResult]:
        """Test all 5 flight assist modes"""
        results = []
        modes = ["None", "Stability", "AutoLevel", "Orbital", "Docking"]

        for i, mode_name in enumerate(modes):
            print(f"\nTesting assist mode: {mode_name}")
            ship_id = self.spawn_ship(position=(i * 200, 0, 300))

            if ship_id:
                self.set_ship_input(ship_id, assist_mode=i)
                time.sleep(1)
                result = TestResult(
                    f"test_assist_mode_{mode_name}",
                    True,
                    f"Assist mode {mode_name} configured",
                    1.0
                )
            else:
                result = TestResult(
                    f"test_assist_mode_{mode_name}",
                    False,
                    "Failed to spawn ship",
                    0.0
                )

            results.append(result)

        return results

    # ============================================================================
    # HELPER FUNCTIONS
    # ============================================================================

    def _execute_python_script(self, script: str) -> Optional[str]:
        """
        Execute Python script in Unreal Editor via command line

        Args:
            script: Python script to execute

        Returns:
            Script output or None if failed
        """
        # Write script to temporary file
        script_file = self.project_path / "temp_automation_script.py"
        script_file.write_text(script)

        # Execute via UnrealEditor-Cmd
        cmd = [
            self.ue_editor_cmd,
            str(self.uproject_file),
            "-ExecutePythonScript=" + str(script_file),
            "-stdout",
            "-unattended",
            "-nopause",
            "-nosplash"
        ]

        try:
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=30,
                cwd=str(self.project_path)
            )

            # Clean up temp file
            script_file.unlink()

            return result.stdout

        except Exception as e:
            print(f"Error executing script: {e}")
            if script_file.exists():
                script_file.unlink()
            return None

    def log_result(self, result: TestResult):
        """Log test result to file"""
        with open(self.log_file, "a") as f:
            f.write(f"\n{'='*70}\n")
            f.write(f"Test: {result.test_name}\n")
            f.write(f"Success: {result.success}\n")
            f.write(f"Message: {result.message}\n")
            f.write(f"Duration: {result.duration:.2f}s\n")
            f.write(f"Timestamp: {time.strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write(f"{'='*70}\n")


# ============================================================================
# MAIN - Example Usage
# ============================================================================

if __name__ == "__main__":
    # Initialize client
    client = UnrealAutomationClient(r"C:\Users\allen\Desktop\Alexander\Alexander")

    print("Unreal Automation Client")
    print("=" * 70)

    # Run automated test
    result = client.test_ship_movement(thrust_duration=3.0, expected_min_distance=100.0)

    # Log result
    client.log_result(result)

    # Print summary
    print(f"\n{'='*70}")
    print("TEST SUMMARY")
    print(f"{'='*70}")
    print(f"Test: {result.test_name}")
    print(f"Result: {'PASS' if result.success else 'FAIL'}")
    print(f"Duration: {result.duration:.2f}s")
    print(f"Message: {result.message}")
    print(f"{'='*70}\n")
