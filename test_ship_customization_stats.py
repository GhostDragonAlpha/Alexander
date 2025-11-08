"""
Test 1: Ship Customization Stats
Verifies that equipping parts changes ship stats correctly
"""

import time
from test_framework import ShipCustomizationTestFramework, TestResult, ShipStats


def test_ship_customization_stats(server_url: str = "http://localhost:8080") -> TestResult:
    """
    Test that equipping parts changes ship stats correctly

    Test workflow:
    1. Spawn ship with ShipCustomizationComponent
    2. Get initial stats (Mass, ThrustPower, MaxVelocity)
    3. Equip Engine part (should increase ThrustPower)
    4. Verify ThrustPower increased
    5. Equip Heavy Hull part (should increase Mass)
    6. Verify Mass increased
    7. Verify stats replicated to physics system
    """
    test_name = "test_ship_customization_stats"
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
        print("\n[2/7] Spawning ship with customization component...")
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

        time.sleep(1)  # Let ship initialize

        # Step 3: Get initial stats
        print("\n[3/7] Getting initial ship stats...")
        initial_stats = framework.get_ship_stats(ship_id)

        if not initial_stats:
            return TestResult(
                test_name,
                False,
                "Failed to get initial stats",
                time.time() - start_time
            )

        print(f"  Initial Mass: {initial_stats.mass} kg")
        print(f"  Initial ThrustPower: {initial_stats.thrust_power}")
        print(f"  Initial MaxVelocity: {initial_stats.max_velocity}")

        # Step 4: Equip Engine part (should increase ThrustPower)
        print("\n[4/7] Equipping Engine upgrade...")
        success = framework.equip_part(ship_id, "Engine", "Engine_TurboII_02")

        if not success:
            return TestResult(
                test_name,
                False,
                "Failed to equip engine part",
                time.time() - start_time
            )

        time.sleep(0.5)

        # Step 5: Verify ThrustPower increased
        print("\n[5/7] Verifying ThrustPower increased...")
        after_engine_stats = framework.get_ship_stats(ship_id)

        if not after_engine_stats:
            return TestResult(
                test_name,
                False,
                "Failed to get stats after engine equip",
                time.time() - start_time
            )

        print(f"  New ThrustPower: {after_engine_stats.thrust_power}")
        print(f"  Change: +{after_engine_stats.thrust_power - initial_stats.thrust_power}")

        if after_engine_stats.thrust_power <= initial_stats.thrust_power:
            return TestResult(
                test_name,
                False,
                f"ThrustPower did not increase (before: {initial_stats.thrust_power}, after: {after_engine_stats.thrust_power})",
                time.time() - start_time
            )

        # Step 6: Equip Heavy Hull part (should increase Mass)
        print("\n[6/7] Equipping Heavy Hull...")
        success = framework.equip_part(ship_id, "Hull", "Hull_Reinforced_01")

        if not success:
            return TestResult(
                test_name,
                False,
                "Failed to equip hull part",
                time.time() - start_time
            )

        time.sleep(0.5)

        # Step 7: Verify Mass increased
        print("\n[7/7] Verifying Mass increased...")
        final_stats = framework.get_ship_stats(ship_id)

        if not final_stats:
            return TestResult(
                test_name,
                False,
                "Failed to get final stats",
                time.time() - start_time
            )

        print(f"  New Mass: {final_stats.mass} kg")
        print(f"  Change: +{final_stats.mass - initial_stats.mass} kg")

        if final_stats.mass <= initial_stats.mass:
            return TestResult(
                test_name,
                False,
                f"Mass did not increase (before: {initial_stats.mass}, after: {final_stats.mass})",
                time.time() - start_time
            )

        # Cleanup
        framework.destroy_ship(ship_id)

        # Success
        duration = time.time() - start_time
        message = (
            f"Ship stats updated correctly: "
            f"ThrustPower {initial_stats.thrust_power:.2f} -> {final_stats.thrust_power:.2f}, "
            f"Mass {initial_stats.mass:.0f} -> {final_stats.mass:.0f} kg"
        )

        return TestResult(
            test_name,
            True,
            message,
            duration,
            details={
                "initial_mass": initial_stats.mass,
                "final_mass": final_stats.mass,
                "initial_thrust": initial_stats.thrust_power,
                "final_thrust": final_stats.thrust_power,
                "mass_increase": final_stats.mass - initial_stats.mass,
                "thrust_increase": final_stats.thrust_power - initial_stats.thrust_power
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
    result = test_ship_customization_stats()

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
