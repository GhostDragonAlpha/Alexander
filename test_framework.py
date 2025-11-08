"""
Ship Customization Testing Framework
Base classes and utilities for testing ship customization + n-body physics integration
"""

import json
import time
import requests
from typing import Dict, List, Optional, Tuple, Any
from dataclasses import dataclass
from pathlib import Path


@dataclass
class ShipStats:
    """Ship statistics from customization"""
    mass: float
    thrust_power: float
    max_velocity: float
    rotation_speed: float
    acceleration: float
    hull_integrity: float
    shield_strength: float
    weapon_damage: float
    energy_capacity: float
    energy_regen_rate: float


@dataclass
class TestResult:
    """Result of a test execution"""
    test_name: str
    success: bool
    message: str
    duration: float
    details: Optional[Dict[str, Any]] = None


class ShipCustomizationTestFramework:
    """
    Testing framework for Ship Customization System
    Communicates with AutomationAPIServer via HTTP
    """

    def __init__(self, server_url: str = "http://localhost:8080"):
        self.server_url = server_url
        self.tracked_ships: Dict[str, str] = {}  # test_id -> ship_id
        self.test_results: List[TestResult] = []

    # ============================================================================
    # SERVER COMMUNICATION
    # ============================================================================

    def _post(self, endpoint: str, data: Dict[str, Any]) -> Optional[Dict[str, Any]]:
        """Send POST request to server"""
        try:
            response = requests.post(
                f"{self.server_url}{endpoint}",
                json=data,
                timeout=10
            )
            if response.status_code == 200:
                return response.json()
            else:
                print(f"[ERROR] POST {endpoint} failed: {response.status_code}")
                return None
        except Exception as e:
            print(f"[ERROR] POST {endpoint} exception: {e}")
            return None

    def _get(self, endpoint: str) -> Optional[Dict[str, Any]]:
        """Send GET request to server"""
        try:
            response = requests.get(
                f"{self.server_url}{endpoint}",
                timeout=10
            )
            if response.status_code == 200:
                return response.json()
            else:
                print(f"[ERROR] GET {endpoint} failed: {response.status_code}")
                return None
        except Exception as e:
            print(f"[ERROR] GET {endpoint} exception: {e}")
            return None

    def check_server_status(self) -> bool:
        """Check if server is running and responsive"""
        try:
            response = self._get("/status")
            if response and response.get("success"):
                print("[OK] Server is running")
                return True
            else:
                print("[FAIL] Server not responding")
                return False
        except Exception as e:
            print(f"[FAIL] Server connection failed: {e}")
            return False

    # ============================================================================
    # SHIP SPAWNING & MANAGEMENT
    # ============================================================================

    def spawn_ship_with_customization(
        self,
        position: Tuple[float, float, float] = (0, 0, 30000),
        rotation: Tuple[float, float, float] = (0, 0, 0),
        ship_class: str = "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer"
    ) -> Optional[str]:
        """
        Spawn a ship with ShipCustomizationComponent

        Returns:
            Ship ID if successful, None otherwise
        """
        print(f"Spawning ship at {position}...")

        response = self._post("/spawn_ship", {
            "position": {"x": position[0], "y": position[1], "z": position[2]},
            "rotation": {"pitch": rotation[0], "yaw": rotation[1], "roll": rotation[2]},
            "ship_class": ship_class
        })

        if response and response.get("success"):
            ship_id = response.get("data", {}).get("ship_id")
            print(f"[OK] Ship spawned: {ship_id}")
            return ship_id
        else:
            print("[FAIL] Failed to spawn ship")
            return None

    def destroy_ship(self, ship_id: str) -> bool:
        """Destroy a ship"""
        response = self._get(f"/destroy_ship/{ship_id}")
        return response and response.get("success", False)

    # ============================================================================
    # CUSTOMIZATION OPERATIONS
    # ============================================================================

    def get_ship_stats(self, ship_id: str) -> Optional[ShipStats]:
        """
        Query ship's current stats from ShipCustomizationComponent

        Returns:
            ShipStats object or None if failed
        """
        response = self._get(f"/get_ship_stats/{ship_id}")

        if response and response.get("success"):
            data = response.get("data", {})
            return ShipStats(
                mass=data.get("mass", 1000.0),
                thrust_power=data.get("thrust_power", 1.0),
                max_velocity=data.get("max_velocity", 1.0),
                rotation_speed=data.get("rotation_speed", 1.0),
                acceleration=data.get("acceleration", 1.0),
                hull_integrity=data.get("hull_integrity", 100.0),
                shield_strength=data.get("shield_strength", 0.0),
                weapon_damage=data.get("weapon_damage", 1.0),
                energy_capacity=data.get("energy_capacity", 100.0),
                energy_regen_rate=data.get("energy_regen_rate", 10.0)
            )
        else:
            print(f"[FAIL] Failed to get stats for ship {ship_id}")
            return None

    def equip_part(self, ship_id: str, category: str, part_id: str) -> bool:
        """
        Equip a part on a ship

        Args:
            ship_id: Ship identifier
            category: Part category (Engine, Hull, Thrusters, etc.)
            part_id: Part identifier

        Returns:
            True if successful
        """
        print(f"Equipping {category} part '{part_id}' on ship {ship_id}...")

        response = self._post("/equip_part", {
            "ship_id": ship_id,
            "category": category,
            "part_id": part_id
        })

        if response and response.get("success"):
            print(f"[OK] Part equipped")
            return True
        else:
            print(f"[FAIL] Failed to equip part")
            return False

    def unequip_part(self, ship_id: str, category: str) -> bool:
        """Unequip a part from a ship"""
        response = self._post("/unequip_part", {
            "ship_id": ship_id,
            "category": category
        })
        return response and response.get("success", False)

    def apply_physics_config(self, ship_id: str, config_name: str) -> bool:
        """Apply a UShipPhysicsConfig data asset to a ship"""
        response = self._post("/apply_physics_config", {
            "ship_id": ship_id,
            "config_name": config_name
        })
        return response and response.get("success", False)

    # ============================================================================
    # PHYSICS MEASUREMENTS
    # ============================================================================

    def get_ship_position(self, ship_id: str) -> Optional[Tuple[float, float, float]]:
        """Get ship's current position"""
        response = self._get(f"/get_position/{ship_id}")

        if response and response.get("success"):
            pos = response.get("data", {})
            return (pos.get("x", 0), pos.get("y", 0), pos.get("z", 0))
        return None

    def get_ship_velocity(self, ship_id: str) -> Optional[Tuple[float, float, float]]:
        """Get ship's current velocity"""
        response = self._get(f"/get_velocity/{ship_id}")

        if response and response.get("success"):
            vel = response.get("data", {})
            return (vel.get("x", 0), vel.get("y", 0), vel.get("z", 0))
        return None

    def measure_acceleration(
        self,
        ship_id: str,
        thrust_direction: Tuple[float, float, float] = (1.0, 0.0, 0.0),
        duration: float = 5.0
    ) -> Optional[float]:
        """
        Measure ship's acceleration over time

        Args:
            ship_id: Ship identifier
            thrust_direction: Thrust input vector
            duration: How long to measure (seconds)

        Returns:
            Average acceleration in m/s²
        """
        print(f"Measuring acceleration for {duration}s...")

        # Get initial velocity
        initial_vel = self.get_ship_velocity(ship_id)
        if not initial_vel:
            return None

        # Apply thrust
        self._post("/set_input", {
            "ship_id": ship_id,
            "thrust": {
                "x": thrust_direction[0],
                "y": thrust_direction[1],
                "z": thrust_direction[2]
            }
        })

        # Wait for duration
        time.sleep(duration)

        # Get final velocity
        final_vel = self.get_ship_velocity(ship_id)
        if not final_vel:
            return None

        # Stop thrust
        self._post("/set_input", {
            "ship_id": ship_id,
            "thrust": {"x": 0, "y": 0, "z": 0}
        })

        # Calculate acceleration (a = Δv / Δt)
        delta_v = (
            (final_vel[0] - initial_vel[0]) ** 2 +
            (final_vel[1] - initial_vel[1]) ** 2 +
            (final_vel[2] - initial_vel[2]) ** 2
        ) ** 0.5

        acceleration = delta_v / duration

        print(f"[OK] Measured acceleration: {acceleration:.2f} m/s²")
        return acceleration

    def measure_gravitational_force(
        self,
        ship_id: str,
        planet_id: str,
        duration: float = 5.0
    ) -> Optional[float]:
        """
        Measure gravitational force between ship and planet

        Returns:
            Average acceleration toward planet in m/s²
        """
        print(f"Measuring gravitational force for {duration}s...")

        # Get initial positions
        ship_pos = self.get_ship_position(ship_id)
        planet_pos = self.get_ship_position(planet_id)

        if not ship_pos or not planet_pos:
            return None

        # Calculate initial distance
        initial_distance = (
            (ship_pos[0] - planet_pos[0]) ** 2 +
            (ship_pos[1] - planet_pos[1]) ** 2 +
            (ship_pos[2] - planet_pos[2]) ** 2
        ) ** 0.5

        # Get initial velocity
        initial_vel = self.get_ship_velocity(ship_id)
        if not initial_vel:
            return None

        # Wait for gravity to act
        time.sleep(duration)

        # Get final velocity
        final_vel = self.get_ship_velocity(ship_id)
        if not final_vel:
            return None

        # Calculate velocity change magnitude
        delta_v = (
            (final_vel[0] - initial_vel[0]) ** 2 +
            (final_vel[1] - initial_vel[1]) ** 2 +
            (final_vel[2] - initial_vel[2]) ** 2
        ) ** 0.5

        # Acceleration = Δv / Δt
        acceleration = delta_v / duration

        print(f"[OK] Measured gravitational acceleration: {acceleration:.6f} m/s²")
        return acceleration

    # ============================================================================
    # NETWORK TESTING
    # ============================================================================

    def get_replicated_stats(self, ship_id: str, client_index: int) -> Optional[ShipStats]:
        """
        Get ship stats as seen by a specific client
        Used for testing network replication
        """
        response = self._get(f"/get_replicated_stats/{ship_id}?client={client_index}")

        if response and response.get("success"):
            data = response.get("data", {})
            return ShipStats(
                mass=data.get("mass", 1000.0),
                thrust_power=data.get("thrust_power", 1.0),
                max_velocity=data.get("max_velocity", 1.0),
                rotation_speed=data.get("rotation_speed", 1.0),
                acceleration=data.get("acceleration", 1.0),
                hull_integrity=data.get("hull_integrity", 100.0),
                shield_strength=data.get("shield_strength", 0.0),
                weapon_damage=data.get("weapon_damage", 1.0),
                energy_capacity=data.get("energy_capacity", 100.0),
                energy_regen_rate=data.get("energy_regen_rate", 10.0)
            )
        return None

    def verify_stat_replication(
        self,
        ship_id: str,
        expected_stats: ShipStats,
        tolerance: float = 0.01
    ) -> bool:
        """
        Verify that ship stats match expected values (within tolerance)
        """
        actual_stats = self.get_ship_stats(ship_id)

        if not actual_stats:
            print("[FAIL] Could not retrieve stats")
            return False

        # Check each stat
        checks = {
            "Mass": (actual_stats.mass, expected_stats.mass),
            "ThrustPower": (actual_stats.thrust_power, expected_stats.thrust_power),
            "MaxVelocity": (actual_stats.max_velocity, expected_stats.max_velocity),
        }

        all_passed = True
        for stat_name, (actual, expected) in checks.items():
            diff = abs(actual - expected)
            passed = diff <= tolerance

            if not passed:
                print(f"[FAIL] {stat_name}: expected {expected}, got {actual} (diff: {diff})")
                all_passed = False
            else:
                print(f"[OK] {stat_name}: {actual} (expected {expected})")

        return all_passed

    # ============================================================================
    # PROGRESSION TESTING
    # ============================================================================

    def add_xp(self, ship_id: str, amount: int) -> bool:
        """Add XP to player"""
        response = self._post("/add_xp", {
            "ship_id": ship_id,
            "amount": amount
        })
        return response and response.get("success", False)

    def unlock_part(self, ship_id: str, part_id: str) -> bool:
        """Unlock a part"""
        response = self._post("/unlock_part", {
            "ship_id": ship_id,
            "part_id": part_id
        })
        return response and response.get("success", False)

    def get_player_level(self, ship_id: str) -> Optional[int]:
        """Get player level"""
        response = self._get(f"/get_player_level/{ship_id}")
        if response and response.get("success"):
            return response.get("data", {}).get("level", 1)
        return None

    def get_credits(self, ship_id: str) -> Optional[int]:
        """Get player credits"""
        response = self._get(f"/get_credits/{ship_id}")
        if response and response.get("success"):
            return response.get("data", {}).get("credits", 0)
        return None

    # ============================================================================
    # LOADOUT TESTING
    # ============================================================================

    def save_loadout(self, ship_id: str, loadout_name: str) -> bool:
        """Save current loadout"""
        response = self._post("/save_loadout", {
            "ship_id": ship_id,
            "loadout_name": loadout_name
        })
        return response and response.get("success", False)

    def load_loadout(self, ship_id: str, loadout_index: int) -> bool:
        """Load a saved loadout"""
        response = self._post("/load_loadout", {
            "ship_id": ship_id,
            "loadout_index": loadout_index
        })
        return response and response.get("success", False)

    # ============================================================================
    # TEST UTILITIES
    # ============================================================================

    def log_test_result(self, result: TestResult):
        """Log a test result"""
        self.test_results.append(result)

        status = "[PASS]" if result.success else "[FAIL]"
        print(f"\n{status} {result.test_name}")
        print(f"  Duration: {result.duration:.2f}s")
        print(f"  Message: {result.message}")

        if result.details:
            print("  Details:")
            for key, value in result.details.items():
                print(f"    {key}: {value}")

    def generate_report(self, output_file: str = "test_report.json"):
        """Generate test report"""
        report = {
            "total_tests": len(self.test_results),
            "passed": sum(1 for r in self.test_results if r.success),
            "failed": sum(1 for r in self.test_results if not r.success),
            "total_duration": sum(r.duration for r in self.test_results),
            "tests": [
                {
                    "name": r.test_name,
                    "success": r.success,
                    "message": r.message,
                    "duration": r.duration,
                    "details": r.details
                }
                for r in self.test_results
            ]
        }

        with open(output_file, "w") as f:
            json.dump(report, f, indent=2)

        print(f"\n[OK] Test report saved to {output_file}")
        return report

    def print_summary(self):
        """Print test summary"""
        total = len(self.test_results)
        passed = sum(1 for r in self.test_results if r.success)
        failed = total - passed

        print("\n" + "=" * 70)
        print("TEST SUMMARY")
        print("=" * 70)
        print(f"Total Tests: {total}")
        print(f"Passed: {passed}")
        print(f"Failed: {failed}")
        print(f"Success Rate: {(passed/total*100 if total > 0 else 0):.1f}%")
        print("=" * 70)
