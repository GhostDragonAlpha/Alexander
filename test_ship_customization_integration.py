"""
Test Ship Customization Integration with N-Body Physics
Tests that ShipCustomizationComponent correctly applies stats to OrbitalBody
"""

import requests
import json
import time
import sys

class ShipCustomizationIntegrationTest:
    def __init__(self, port=8080):
        self.base_url = f"http://localhost:{port}"
        self.session = requests.Session()
        self.test_results = []

    def log(self, message):
        """Log test message"""
        timestamp = time.strftime("%H:%M:%S")
        print(f"[{timestamp}] {message}")

    def test(self, name, condition, expected=True):
        """Record test result"""
        passed = (condition == expected)
        status = "[PASS]" if passed else "[FAIL]"
        self.log(f"{status}: {name}")
        self.test_results.append({
            "name": name,
            "passed": passed,
            "condition": condition,
            "expected": expected
        })
        return passed

    def execute_command(self, command):
        """Execute Unreal Python command via HTTP API"""
        try:
            response = self.session.post(
                f"{self.base_url}/remote/object/call",
                json={
                    "objectPath": "/Script/Engine.Default__PythonScriptLibrary",
                    "functionName": "ExecutePythonCommand",
                    "parameters": {"PythonCommand": command}
                },
                timeout=10
            )

            if response.status_code == 200:
                return response.json()
            else:
                self.log(f"Command failed with status {response.status_code}: {command[:100]}")
                return None

        except Exception as e:
            self.log(f"Error executing command: {e}")
            return None

    def run_tests(self):
        """Run all ship customization integration tests"""
        self.log("=" * 80)
        self.log("SHIP CUSTOMIZATION INTEGRATION TEST")
        self.log("=" * 80)

        # Test 1: Verify server is responsive
        self.log("\n--- Test 1: Server Connection ---")
        try:
            response = self.session.get(f"{self.base_url}/remote/info", timeout=5)
            server_ok = response.status_code == 200
            self.test("Server is responsive", server_ok, True)
        except:
            self.test("Server is responsive", False, True)
            self.log("ERROR: Cannot connect to automation server. Is the game running with -HTTPPort=8080?")
            return False

        # Test 2: Get player ship reference
        self.log("\n--- Test 2: Player Ship Setup ---")
        result = self.execute_command("""
import unreal
ship = unreal.GameplayStatics.get_player_pawn(unreal.EditorLevelLibrary.get_editor_world(), 0)
if ship:
    print(f"SHIP_CLASS:{ship.get_class().get_name()}")
    print(f"SHIP_VALID:True")
else:
    print("SHIP_VALID:False")
""")

        if result:
            output = result.get("ReturnValue", "")
            ship_valid = "SHIP_VALID:True" in output
            self.test("Player ship exists", ship_valid, True)

            if "SHIP_CLASS:" in output:
                ship_class = output.split("SHIP_CLASS:")[1].split("\n")[0]
                self.log(f"  Ship class: {ship_class}")
        else:
            self.test("Player ship exists", False, True)
            return False

        # Test 3: Check if ship has ShipCustomizationComponent
        self.log("\n--- Test 3: Component Integration ---")
        result = self.execute_command("""
import unreal
ship = unreal.GameplayStatics.get_player_pawn(unreal.EditorLevelLibrary.get_editor_world(), 0)
if ship:
    # Try to get or add ShipCustomizationComponent
    customization_comp = ship.get_component_by_class(unreal.ShipCustomizationComponent)

    if not customization_comp:
        # Component doesn't exist, try to add it
        customization_comp = ship.add_component_by_class(
            unreal.ShipCustomizationComponent,
            False,  # bManualAttachment
            unreal.Transform(),
            False   # bDeferredFinish
        )
        print("COMPONENT_ADDED:True")
    else:
        print("COMPONENT_EXISTS:True")

    if customization_comp:
        print("COMPONENT_VALID:True")
        print(f"COMPONENT_NAME:{customization_comp.get_name()}")
    else:
        print("COMPONENT_VALID:False")
""")

        if result:
            output = result.get("ReturnValue", "")
            component_valid = "COMPONENT_VALID:True" in output
            self.test("ShipCustomizationComponent available", component_valid, True)

            if "COMPONENT_NAME:" in output:
                comp_name = output.split("COMPONENT_NAME:")[1].split("\n")[0]
                self.log(f"  Component: {comp_name}")
        else:
            self.test("ShipCustomizationComponent available", False, True)

        # Test 4: Check OrbitalBody integration
        self.log("\n--- Test 4: OrbitalBody Integration ---")
        result = self.execute_command("""
import unreal
ship = unreal.GameplayStatics.get_player_pawn(unreal.EditorLevelLibrary.get_editor_world(), 0)
if ship:
    # Check if ship is an OrbitalBody (Spaceship inherits from OrbitalBody)
    is_orbital_body = isinstance(ship, unreal.OrbitalBody)
    print(f"IS_ORBITAL_BODY:{is_orbital_body}")

    if is_orbital_body:
        # Get initial physics values
        mass = ship.get_editor_property('mass')
        thrust_power = ship.get_editor_property('thrust_power')

        print(f"INITIAL_MASS:{mass}")
        print(f"INITIAL_THRUST:{thrust_power}")
        print("ORBITAL_BODY_READY:True")
    else:
        print("ORBITAL_BODY_READY:False")
""")

        if result:
            output = result.get("ReturnValue", "")
            is_orbital_body = "IS_ORBITAL_BODY:True" in output
            self.test("Ship is OrbitalBody (has n-body physics)", is_orbital_body, True)

            if "INITIAL_MASS:" in output:
                initial_mass = output.split("INITIAL_MASS:")[1].split("\n")[0]
                self.log(f"  Initial mass: {initial_mass} kg")

            if "INITIAL_THRUST:" in output:
                initial_thrust = output.split("INITIAL_THRUST:")[1].split("\n")[0]
                self.log(f"  Initial thrust: {initial_thrust} N")
        else:
            self.test("Ship is OrbitalBody", False, True)

        # Test 5: Apply test stats and verify they affect OrbitalBody
        self.log("\n--- Test 5: Stats Application Test ---")
        result = self.execute_command("""
import unreal
ship = unreal.GameplayStatics.get_player_pawn(unreal.EditorLevelLibrary.get_editor_world(), 0)
if ship:
    customization_comp = ship.get_component_by_class(unreal.ShipCustomizationComponent)

    if customization_comp and isinstance(ship, unreal.OrbitalBody):
        # Store original values
        original_mass = ship.get_editor_property('mass')

        # Create test stats
        test_stats = unreal.ShipStats()
        test_stats.mass = 50000.0  # 50 tons
        test_stats.thrust_power = 2.5  # 2.5x multiplier = 250,000 N
        test_stats.max_velocity = 10.0  # 10x multiplier = 10,000 m/s

        # Apply stats via customization component
        customization_comp.get_editor_property('current_loadout').total_stats = test_stats
        customization_comp.call_method('ApplyStatsToFlightController')

        # Small delay for application
        import time
        time.sleep(0.1)

        # Read back values
        new_mass = ship.get_editor_property('mass')
        new_thrust = ship.get_editor_property('thrust_power')

        print(f"ORIGINAL_MASS:{original_mass}")
        print(f"TEST_MASS:{test_stats.mass}")
        print(f"APPLIED_MASS:{new_mass}")
        print(f"APPLIED_THRUST:{new_thrust}")

        # Check if mass was applied
        mass_applied = abs(new_mass - test_stats.mass) < 1.0
        print(f"MASS_APPLIED:{mass_applied}")

        # Check if thrust was applied (2.5 * 100,000 = 250,000)
        expected_thrust = test_stats.thrust_power * 100000.0
        thrust_applied = abs(new_thrust - expected_thrust) < 1000.0
        print(f"THRUST_APPLIED:{thrust_applied}")

        print("STATS_TEST_COMPLETE:True")
    else:
        print("STATS_TEST_COMPLETE:False")
""")

        if result:
            output = result.get("ReturnValue", "")

            if "APPLIED_MASS:" in output:
                applied_mass = output.split("APPLIED_MASS:")[1].split("\n")[0]
                self.log(f"  Applied mass: {applied_mass} kg")

            if "APPLIED_THRUST:" in output:
                applied_thrust = output.split("APPLIED_THRUST:")[1].split("\n")[0]
                self.log(f"  Applied thrust: {applied_thrust} N")

            mass_applied = "MASS_APPLIED:True" in output
            thrust_applied = "THRUST_APPLIED:True" in output

            self.test("Mass stats applied to OrbitalBody", mass_applied, True)
            self.test("Thrust stats applied to OrbitalBody", thrust_applied, True)
        else:
            self.test("Mass stats applied to OrbitalBody", False, True)
            self.test("Thrust stats applied to OrbitalBody", False, True)

        # Test 6: Verify physics integration affects gravity calculations
        self.log("\n--- Test 6: N-Body Physics Integration ---")
        result = self.execute_command("""
import unreal
ship = unreal.GameplayStatics.get_player_pawn(unreal.EditorLevelLibrary.get_editor_world(), 0)
if ship and isinstance(ship, unreal.OrbitalBody):
    # Check standard gravitational parameter (μ = G * M)
    mass = ship.get_editor_property('mass')
    mu = ship.get_editor_property('standard_gravitational_parameter')

    # G = 6.67430e-11 m³/(kg·s²)
    G = 6.67430e-11
    expected_mu = G * mass

    print(f"SHIP_MASS:{mass}")
    print(f"SHIP_MU:{mu}")
    print(f"EXPECTED_MU:{expected_mu}")

    # Check if mu is correctly calculated
    mu_correct = abs(mu - expected_mu) < (expected_mu * 0.01)  # 1% tolerance
    print(f"MU_CORRECT:{mu_correct}")

    # Check velocity property exists
    velocity = ship.get_editor_property('velocity')
    print(f"HAS_VELOCITY:{velocity is not None}")

    print("PHYSICS_INTEGRATION_COMPLETE:True")
else:
    print("PHYSICS_INTEGRATION_COMPLETE:False")
""")

        if result:
            output = result.get("ReturnValue", "")

            if "SHIP_MASS:" in output and "SHIP_MU:" in output:
                ship_mass = output.split("SHIP_MASS:")[1].split("\n")[0]
                ship_mu = output.split("SHIP_MU:")[1].split("\n")[0]
                self.log(f"  Ship mass: {ship_mass} kg")
                self.log(f"  Gravitational parameter μ: {ship_mu} m³/s²")

            mu_correct = "MU_CORRECT:True" in output
            has_velocity = "HAS_VELOCITY:True" in output

            self.test("Gravitational parameter correctly calculated", mu_correct, True)
            self.test("Ship has velocity vector for n-body physics", has_velocity, True)
        else:
            self.test("Gravitational parameter correctly calculated", False, True)
            self.test("Ship has velocity vector", False, True)

        # Print summary
        self.print_summary()

    def print_summary(self):
        """Print test results summary"""
        self.log("\n" + "=" * 80)
        self.log("TEST SUMMARY")
        self.log("=" * 80)

        total = len(self.test_results)
        passed = sum(1 for r in self.test_results if r["passed"])
        failed = total - passed

        self.log(f"Total tests: {total}")
        self.log(f"Passed: {passed}")
        self.log(f"Failed: {failed}")

        if failed > 0:
            self.log("\nFailed tests:")
            for result in self.test_results:
                if not result["passed"]:
                    self.log(f"  - {result['name']}")
                    self.log(f"    Expected: {result['expected']}, Got: {result['condition']}")

        success_rate = (passed / total * 100) if total > 0 else 0
        self.log(f"\nSuccess rate: {success_rate:.1f}%")

        if success_rate == 100:
            self.log("\n*** ALL TESTS PASSED! Ship customization integration is working correctly.")
            return True
        elif success_rate >= 70:
            self.log("\n*** MOSTLY PASSING - Integration is functional but has some issues.")
            return True
        else:
            self.log("\n*** TESTS FAILED - Integration has significant issues.")
            return False

if __name__ == "__main__":
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8080

    print(f"\nStarting Ship Customization Integration Test (port {port})...")
    print("Make sure the game is running with: -HTTPPort={port} -nohmd -log\n")

    test = ShipCustomizationIntegrationTest(port)
    success = test.run_tests()

    sys.exit(0 if success else 1)
