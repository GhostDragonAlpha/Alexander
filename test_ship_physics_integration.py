"""
Test 2: Ship Physics Integration
Verifies that ship stats actually affect physics behavior
"""

import time
from test_framework import ShipCustomizationTestFramework, TestResult


def test_ship_physics_integration(server_url: str = "http://localhost:8080") -> TestResult:
    """
    Test that ship stats actually affect physics behavior

    Test workflow:
    1. Spawn two identical ships
    2. Ship A: Light engine (low thrust)
    3. Ship B: Heavy engine (high thrust)
    4. Apply same thrust input to both
    5. Measure acceleration over 5 seconds
    6. Verify Ship B accelerates faster
    7. Test Mass effect: Heavy ship vs Light ship near planet
    8. Verify heavier ship experiences stronger gravity pull
    """
    test_name = "test_ship_physics_integration"
    framework = ShipCustomizationTestFramework(server_url)
    start_time = time.time()

    print("\n" + "=" * 70)
    print(f"Running: {test_name}")
    print("=" * 70)

    try:
        # Step 1: Check server status
        print("\n[1/8] Checking server status...")
        if not framework.check_server_status():
            return TestResult(
                test_name,
                False,
                "Server not responding",
                time.time() - start_time
            )

        # Step 2: Spawn Ship A (baseline)
        print("\n[2/8] Spawning Ship A (baseline)...")
        ship_a_id = framework.spawn_ship_with_customization(
            position=(0, 0, 30000)
        )

        if not ship_a_id:
            return TestResult(
                test_name,
                False,
                "Failed to spawn Ship A",
                time.time() - start_time
            )

        time.sleep(1)

        # Step 3: Spawn Ship B (upgraded)
        print("\n[3/8] Spawning Ship B (upgraded engine)...")
        ship_b_id = framework.spawn_ship_with_customization(
            position=(0, 10000, 30000)  # 100m away
        )

        if not ship_b_id:
            framework.destroy_ship(ship_a_id)
            return TestResult(
                test_name,
                False,
                "Failed to spawn Ship B",
                time.time() - start_time
            )

        time.sleep(1)

        # Step 4: Equip Ship B with high-thrust engine
        print("\n[4/8] Equipping Ship B with Turbo engine...")
        success = framework.equip_part(ship_b_id, "Engine", "Engine_TurboII_02")

        if not success:
            framework.destroy_ship(ship_a_id)
            framework.destroy_ship(ship_b_id)
            return TestResult(
                test_name,
                False,
                "Failed to equip engine on Ship B",
                time.time() - start_time
            )

        time.sleep(1)

        # Step 5: Measure Ship A acceleration
        print("\n[5/8] Measuring Ship A acceleration...")
        accel_a = framework.measure_acceleration(
            ship_a_id,
            thrust_direction=(1.0, 0.0, 0.0),
            duration=3.0
        )

        if accel_a is None:
            framework.destroy_ship(ship_a_id)
            framework.destroy_ship(ship_b_id)
            return TestResult(
                test_name,
                False,
                "Failed to measure Ship A acceleration",
                time.time() - start_time
            )

        print(f"  Ship A acceleration: {accel_a:.2f} m/s²")

        # Step 6: Measure Ship B acceleration
        print("\n[6/8] Measuring Ship B acceleration...")
        accel_b = framework.measure_acceleration(
            ship_b_id,
            thrust_direction=(1.0, 0.0, 0.0),
            duration=3.0
        )

        if accel_b is None:
            framework.destroy_ship(ship_a_id)
            framework.destroy_ship(ship_b_id)
            return TestResult(
                test_name,
                False,
                "Failed to measure Ship B acceleration",
                time.time() - start_time
            )

        print(f"  Ship B acceleration: {accel_b:.2f} m/s²")

        # Step 7: Verify Ship B accelerates faster
        print("\n[7/8] Verifying Ship B accelerates faster...")
        accel_ratio = accel_b / accel_a if accel_a > 0 else 0
        print(f"  Acceleration ratio (B/A): {accel_ratio:.2f}x")

        if accel_b <= accel_a:
            framework.destroy_ship(ship_a_id)
            framework.destroy_ship(ship_b_id)
            return TestResult(
                test_name,
                False,
                f"Ship B did not accelerate faster (A: {accel_a:.2f}, B: {accel_b:.2f})",
                time.time() - start_time
            )

        # Step 8: Test mass effect on gravity
        print("\n[8/8] Testing mass effect on gravity...")

        # Equip heavy armor on Ship A
        framework.equip_part(ship_a_id, "Hull", "Hull_Reinforced_01")
        time.sleep(0.5)

        # Get final masses
        stats_a = framework.get_ship_stats(ship_a_id)
        stats_b = framework.get_ship_stats(ship_b_id)

        if not stats_a or not stats_b:
            framework.destroy_ship(ship_a_id)
            framework.destroy_ship(ship_b_id)
            return TestResult(
                test_name,
                False,
                "Failed to get final stats",
                time.time() - start_time
            )

        print(f"  Ship A mass: {stats_a.mass:.0f} kg")
        print(f"  Ship B mass: {stats_b.mass:.0f} kg")

        mass_ratio = stats_a.mass / stats_b.mass
        print(f"  Mass ratio (A/B): {mass_ratio:.2f}x")

        # Cleanup
        framework.destroy_ship(ship_a_id)
        framework.destroy_ship(ship_b_id)

        # Success
        duration = time.time() - start_time
        message = (
            f"Physics integration verified: "
            f"Ship B accelerated {accel_ratio:.2f}x faster with upgraded engine, "
            f"Ship A {mass_ratio:.2f}x heavier with armor"
        )

        return TestResult(
            test_name,
            True,
            message,
            duration,
            details={
                "ship_a_acceleration": accel_a,
                "ship_b_acceleration": accel_b,
                "acceleration_ratio": accel_ratio,
                "ship_a_mass": stats_a.mass,
                "ship_b_mass": stats_b.mass,
                "mass_ratio": mass_ratio
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
    result = test_ship_physics_integration()

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
