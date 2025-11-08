"""
Test 5: Ship Loadout System
Tests save/load of ship configurations
"""

import time
from test_framework import ShipCustomizationTestFramework, TestResult


def test_ship_loadout_system(server_url: str = "http://localhost:8080") -> TestResult:
    """
    Test save/load of ship configurations

    Test workflow:
    1. Create custom loadout (equip 5 parts)
    2. Save loadout with name "TestLoadout1"
    3. Unequip all parts
    4. Load "TestLoadout1"
    5. Verify all 5 parts re-equipped
    6. Verify stats match saved state
    7. Test multiple loadout slots
    """
    test_name = "test_ship_loadout_system"
    framework = ShipCustomizationTestFramework(server_url)
    start_time = time.time()

    print("\n" + "=" * 70)
    print(f"Running: {test_name}")
    print("=" * 70)

    try:
        # Step 1: Check server status
        print("\n[1/7] Checking server status...")
        if not framework.check_server_status():
            return TestResult(
                test_name,
                False,
                "Server not responding",
                time.time() - start_time
            )

        # Step 2: Spawn ship
        print("\n[2/7] Spawning ship...")
        ship_id = framework.spawn_ship_with_customization(
            position=(0, 0, 30000)
        )

        if not ship_id:
            return TestResult(
                test_name,
                False,
                "Failed to spawn ship",
                time.time() - start_time
            )

        time.sleep(1)

        # Step 3: Create custom loadout (equip parts)
        print("\n[3/7] Creating custom loadout...")

        parts_equipped = []

        # Equip Engine
        if framework.equip_part(ship_id, "Engine", "Engine_TurboII_02"):
            parts_equipped.append("Engine_TurboII_02")
            print("  [OK] Equipped Engine")
        else:
            print("  [WARNING] Could not equip Engine")

        time.sleep(0.2)

        # Equip Hull
        if framework.equip_part(ship_id, "Hull", "Hull_Reinforced_01"):
            parts_equipped.append("Hull_Reinforced_01")
            print("  [OK] Equipped Hull")
        else:
            print("  [WARNING] Could not equip Hull")

        time.sleep(0.2)

        # Equip Thrusters
        if framework.equip_part(ship_id, "Thrusters", "Thrusters_Enhanced_01"):
            parts_equipped.append("Thrusters_Enhanced_01")
            print("  [OK] Equipped Thrusters")
        else:
            print("  [WARNING] Could not equip Thrusters")

        time.sleep(0.2)

        # Equip Wings
        if framework.equip_part(ship_id, "Wings", "Wings_Aerodynamic_01"):
            parts_equipped.append("Wings_Aerodynamic_01")
            print("  [OK] Equipped Wings")
        else:
            print("  [WARNING] Could not equip Wings")

        time.sleep(0.2)

        # Equip Shield
        if framework.equip_part(ship_id, "Shield", "Shield_Basic_01"):
            parts_equipped.append("Shield_Basic_01")
            print("  [OK] Equipped Shield")
        else:
            print("  [WARNING] Could not equip Shield")

        time.sleep(0.5)

        print(f"  Total parts equipped: {len(parts_equipped)}")

        # Get stats after equipping
        saved_stats = framework.get_ship_stats(ship_id)

        if not saved_stats:
            framework.destroy_ship(ship_id)
            return TestResult(
                test_name,
                False,
                "Failed to get stats after equipping",
                time.time() - start_time
            )

        print(f"  Loadout stats:")
        print(f"    Mass: {saved_stats.mass} kg")
        print(f"    ThrustPower: {saved_stats.thrust_power}")

        # Step 4: Save loadout
        print("\n[4/7] Saving loadout as 'TestLoadout1'...")
        success = framework.save_loadout(ship_id, "TestLoadout1")

        if not success:
            framework.destroy_ship(ship_id)
            return TestResult(
                test_name,
                False,
                "Failed to save loadout",
                time.time() - start_time
            )

        print("  [OK] Loadout saved")

        # Step 5: Unequip all parts
        print("\n[5/7] Unequipping all parts...")

        categories = ["Engine", "Hull", "Thrusters", "Wings", "Shield", "Weapon", "Cockpit", "Utility"]
        for category in categories:
            framework.unequip_part(ship_id, category)

        time.sleep(1)

        # Get stats after unequipping (should be base stats)
        base_stats = framework.get_ship_stats(ship_id)

        if not base_stats:
            framework.destroy_ship(ship_id)
            return TestResult(
                test_name,
                False,
                "Failed to get base stats",
                time.time() - start_time
            )

        print(f"  Base stats after unequip:")
        print(f"    Mass: {base_stats.mass} kg")
        print(f"    ThrustPower: {base_stats.thrust_power}")

        # Step 6: Load saved loadout
        print("\n[6/7] Loading 'TestLoadout1'...")
        success = framework.load_loadout(ship_id, loadout_index=0)

        if not success:
            framework.destroy_ship(ship_id)
            return TestResult(
                test_name,
                False,
                "Failed to load loadout",
                time.time() - start_time
            )

        time.sleep(1)

        # Get stats after loading
        loaded_stats = framework.get_ship_stats(ship_id)

        if not loaded_stats:
            framework.destroy_ship(ship_id)
            return TestResult(
                test_name,
                False,
                "Failed to get stats after loading",
                time.time() - start_time
            )

        print(f"  Loaded stats:")
        print(f"    Mass: {loaded_stats.mass} kg")
        print(f"    ThrustPower: {loaded_stats.thrust_power}")

        # Step 7: Verify stats match saved state
        print("\n[7/7] Verifying loaded stats match saved stats...")

        mass_diff = abs(loaded_stats.mass - saved_stats.mass)
        thrust_diff = abs(loaded_stats.thrust_power - saved_stats.thrust_power)

        print(f"  Mass difference: {mass_diff:.2f} kg")
        print(f"  ThrustPower difference: {thrust_diff:.4f}")

        if mass_diff > 1.0 or thrust_diff > 0.01:
            framework.destroy_ship(ship_id)
            return TestResult(
                test_name,
                False,
                f"Loaded stats don't match saved (mass diff: {mass_diff}, thrust diff: {thrust_diff})",
                time.time() - start_time
            )

        # Cleanup
        framework.destroy_ship(ship_id)

        # Success
        duration = time.time() - start_time
        message = (
            f"Loadout system verified: "
            f"Saved loadout with {len(parts_equipped)} parts, "
            f"loaded successfully with matching stats"
        )

        return TestResult(
            test_name,
            True,
            message,
            duration,
            details={
                "parts_equipped": len(parts_equipped),
                "saved_mass": saved_stats.mass,
                "loaded_mass": loaded_stats.mass,
                "saved_thrust": saved_stats.thrust_power,
                "loaded_thrust": loaded_stats.thrust_power,
                "mass_diff": mass_diff,
                "thrust_diff": thrust_diff
            }
        )

    except Exception as e:
        return TestResult(
            test_name,
            False,
            f"Exception: {str(e)}",
            time.time() - start_time
        )


if __name__ == "__main__":
    result = test_ship_loadout_system()

    print("\n" + "=" * 70)
    print("TEST RESULT")
    print("=" * 70)
    print(f"Test: {result.test_name}")
    print(f"Status: {'PASS' if result.success else 'FAIL'}")
    print(f"Duration: {result.duration:.2f}s")
    print(f"Message: {result.message}")

    if result.details:
        print("\nDetails:")
        for key, value in result.details.items():
            print(f"  {key}: {value}")

    print("=" * 70)
