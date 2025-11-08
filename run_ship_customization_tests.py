"""
Ship Customization Test Runner
Runs all 7 test scripts in sequence and generates a comprehensive report
"""

import time
import json
from datetime import datetime
from pathlib import Path

# Import all test modules
from test_ship_customization_stats import test_ship_customization_stats
from test_ship_physics_integration import test_ship_physics_integration
from test_ship_customization_network import test_ship_customization_network
from test_physics_config_data_assets import test_physics_config_data_assets
from test_ship_loadout_system import test_ship_loadout_system
from test_ship_progression import test_ship_progression
from test_gravity_with_customization import test_gravity_with_customization


def print_banner(text: str):
    """Print a formatted banner"""
    border = "=" * 80
    print(f"\n{border}")
    print(f"{text.center(80)}")
    print(f"{border}\n")


def run_all_tests(server_url: str = "http://localhost:8080"):
    """
    Run all ship customization tests in sequence

    Returns:
        Dictionary with test results and summary
    """
    print_banner("SHIP CUSTOMIZATION TEST SUITE")

    start_time = time.time()
    results = []

    # Test suite configuration
    tests = [
        ("Test 1: Ship Customization Stats", test_ship_customization_stats),
        ("Test 2: Ship Physics Integration", test_ship_physics_integration),
        ("Test 3: Ship Customization Network", test_ship_customization_network),
        ("Test 4: Physics Config Data Assets", test_physics_config_data_assets),
        ("Test 5: Ship Loadout System", test_ship_loadout_system),
        ("Test 6: Ship Progression", test_ship_progression),
        ("Test 7: Gravity with Customization", test_gravity_with_customization),
    ]

    # Run each test
    for i, (test_name, test_func) in enumerate(tests, 1):
        print_banner(f"Running {test_name} ({i}/{len(tests)})")

        try:
            result = test_func(server_url)
            results.append(result)

            # Print immediate result
            status = "[PASS]" if result.success else "[FAIL]"
            print(f"\n{status} {result.test_name}")
            print(f"Duration: {result.duration:.2f}s")
            print(f"Message: {result.message}")

        except Exception as e:
            print(f"\n[ERROR] Test crashed: {str(e)}")
            from test_framework import TestResult
            results.append(TestResult(
                test_name=test_name,
                success=False,
                message=f"Test crashed: {str(e)}",
                duration=0.0
            ))

        # Small delay between tests
        time.sleep(2)

    # Calculate summary statistics
    total_duration = time.time() - start_time
    total_tests = len(results)
    passed = sum(1 for r in results if r.success)
    failed = total_tests - passed
    success_rate = (passed / total_tests * 100) if total_tests > 0 else 0

    # Print summary
    print_banner("TEST SUMMARY")

    print(f"Total Tests:    {total_tests}")
    print(f"Passed:         {passed}")
    print(f"Failed:         {failed}")
    print(f"Success Rate:   {success_rate:.1f}%")
    print(f"Total Duration: {total_duration:.2f}s")
    print()

    # Print individual test results
    print("Individual Test Results:")
    print("-" * 80)
    for result in results:
        status = "PASS" if result.success else "FAIL"
        print(f"  [{status}] {result.test_name:<50} {result.duration:>6.2f}s")
    print("-" * 80)

    # Create detailed report
    report = {
        "test_suite": "Ship Customization System",
        "timestamp": datetime.now().isoformat(),
        "server_url": server_url,
        "summary": {
            "total_tests": total_tests,
            "passed": passed,
            "failed": failed,
            "success_rate": success_rate,
            "total_duration": total_duration
        },
        "tests": [
            {
                "name": r.test_name,
                "success": r.success,
                "message": r.message,
                "duration": r.duration,
                "details": r.details if hasattr(r, 'details') else None
            }
            for r in results
        ]
    }

    return report


def save_report_json(report: dict, output_file: str = "SHIP_CUSTOMIZATION_TEST_REPORT.json"):
    """Save test report as JSON"""
    with open(output_file, "w") as f:
        json.dump(report, f, indent=2)
    print(f"\n[OK] JSON report saved to: {output_file}")


def save_report_markdown(report: dict, output_file: str = "SHIP_CUSTOMIZATION_TEST_REPORT.md"):
    """Save test report as Markdown"""

    md_content = f"""# Ship Customization Test Report

**Date:** {report['timestamp']}
**Server:** {report['server_url']}
**Test Suite:** {report['test_suite']}

---

## Summary

| Metric | Value |
|--------|-------|
| Total Tests | {report['summary']['total_tests']} |
| Passed | {report['summary']['passed']} |
| Failed | {report['summary']['failed']} |
| Success Rate | {report['summary']['success_rate']:.1f}% |
| Total Duration | {report['summary']['total_duration']:.2f}s |

---

## Test Results

"""

    for test in report['tests']:
        status_emoji = "✅" if test['success'] else "❌"
        md_content += f"### {status_emoji} {test['name']}\n\n"
        md_content += f"**Status:** {'PASS' if test['success'] else 'FAIL'}  \n"
        md_content += f"**Duration:** {test['duration']:.2f}s  \n"
        md_content += f"**Message:** {test['message']}  \n\n"

        if test.get('details'):
            md_content += "**Details:**\n\n"
            for key, value in test['details'].items():
                md_content += f"- `{key}`: {value}\n"
            md_content += "\n"

        md_content += "---\n\n"

    # Add test descriptions
    md_content += """## Test Descriptions

### Test 1: Ship Customization Stats
Verifies that equipping parts changes ship stats correctly. Tests basic stat modification through the customization component.

### Test 2: Ship Physics Integration
Verifies that ship stats actually affect physics behavior. Measures acceleration differences between ships with different thrust configurations.

### Test 3: Ship Customization Network
Tests multiplayer replication of ship stats. Ensures stats are correctly synchronized across server and clients.

### Test 4: Physics Config Data Assets
Tests UShipPhysicsConfig data asset application. Verifies that preset configurations apply correctly and can be combined with parts.

### Test 5: Ship Loadout System
Tests save/load of ship configurations. Verifies that loadouts can be saved, loaded, and restore correct stats.

### Test 6: Ship Progression
Tests unlock system and progression. Verifies XP, leveling, credits, and part unlocking mechanics.

### Test 7: Gravity with Customization
Tests n-body gravity with variable ship mass. Verifies that mass changes affect gravitational interactions according to F = G*m1*m2/r².

---

## Conclusion

"""

    if report['summary']['failed'] == 0:
        md_content += "🎉 **All tests passed!** The ship customization system is fully functional.\n"
    else:
        md_content += f"⚠️ **{report['summary']['failed']} test(s) failed.** Please review the failures above.\n"

    md_content += f"\n**Total test execution time:** {report['summary']['total_duration']:.2f} seconds\n"

    with open(output_file, "w") as f:
        f.write(md_content)

    print(f"[OK] Markdown report saved to: {output_file}")


def main():
    """Main entry point"""
    print_banner("Ship Customization Test Suite")
    print("This test suite verifies the integration of:")
    print("  - Ship Customization Component")
    print("  - N-Body Physics System")
    print("  - Network Replication")
    print("  - Progression System")
    print()

    # Get server URL (could be command-line argument)
    server_url = "http://localhost:8080"
    print(f"Server URL: {server_url}")
    print()

    input("Press ENTER to start tests (or Ctrl+C to cancel)...")

    # Run all tests
    report = run_all_tests(server_url)

    # Save reports
    save_report_json(report)
    save_report_markdown(report)

    print_banner("TEST SUITE COMPLETE")

    # Exit with appropriate code
    if report['summary']['failed'] > 0:
        print(f"❌ {report['summary']['failed']} test(s) failed")
        exit(1)
    else:
        print("✅ All tests passed!")
        exit(0)


if __name__ == "__main__":
    main()
