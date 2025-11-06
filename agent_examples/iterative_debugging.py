#!/usr/bin/env python3
"""
Iterative Debugging Example

Shows how an AI agent can:
1. Run tests
2. Analyze failures
3. Apply fixes
4. Re-test to verify
5. Iterate until all tests pass

This is the "turn-based" workflow where the agent iteratively
debugs issues like a human developer would.
"""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from agent_sdk import TestServerController


def main():
    """
    Iterative debugging workflow for AI agents
    """

    print("=" * 80)
    print("EXAMPLE: Iterative Debugging")
    print("=" * 80)

    with TestServerController() as server:

        print("\n=== ITERATION 1: Initial Test Run ===")

        # Run all tests
        test_id = server.run_tests()
        results = server.wait_for_completion(test_id)

        test_results = results.get('results', {})
        failed_count = test_results.get('failed', 0)

        print(f"\nResults: {test_results.get('passed', 0)}/{test_results.get('total_tests', 0)} passed")

        if failed_count == 0:
            print("✓ All tests passed on first run!")
            return

        print(f"⚠ {failed_count} test(s) failed, analyzing...")

        # Analyze failures
        analysis = server.analyze_failures(test_id)

        print(f"\n=== ANALYSIS ===")
        failures = analysis.get('failures', [])
        print(f"Found {len(failures)} failure(s) to debug:")

        # Iterate through failures
        iteration = 1
        max_iterations = 5

        while failed_count > 0 and iteration <= max_iterations:

            print(f"\n=== ITERATION {iteration + 1}: Fix and Re-test ===")

            # Look for auto-fixable failures
            auto_fixable = [f for f in failures if f.get('can_auto_fix', False)]

            if not auto_fixable:
                print("No auto-fixable failures found.")
                print("Manual investigation required for:")
                for failure in failures:
                    print(f"  - {failure.get('station', 'Unknown')}.{failure.get('test', 'Unknown')}")
                    print(f"    Error: {failure.get('error', 'No details')}")
                break

            print(f"Found {len(auto_fixable)} auto-fixable failure(s):")

            # Apply fixes and rerun
            tests_to_rerun = []
            new_settings = {}

            for failure in auto_fixable:
                test_name = failure.get('test', 'Unknown')
                suggested_fix = failure.get('suggested_fix', {})

                print(f"\n  {test_name}:")
                print(f"    Error: {failure.get('error', '')}")
                print(f"    Fix: {suggested_fix.get('action', 'Unknown')}")
                print(f"    Confidence: {suggested_fix.get('confidence', 0):.0%}")

                # Extract fix parameters
                if suggested_fix.get('action') == 'adjust_tolerance':
                    parameter = suggested_fix.get('parameter', '')
                    new_value = suggested_fix.get('suggested_value', 0)
                    new_settings[parameter] = new_value
                    print(f"    Applying: {parameter} = {new_value}")

                # Add to rerun list
                station = failure.get('station', '')
                if station:
                    tests_to_rerun.append(station)

            # Remove duplicates
            tests_to_rerun = list(set(tests_to_rerun))

            if tests_to_rerun:
                print(f"\n  Rerunning {len(tests_to_rerun)} test(s) with new settings...")

                # Rerun with fixes
                rerun_id = server.rerun_tests(tests_to_rerun, new_settings)
                rerun_results = server.wait_for_completion(rerun_id)

                # Check new results
                rerun_test_results = rerun_results.get('results', {})
                new_failed_count = rerun_test_results.get('failed', 0)

                print(f"\n  Rerun Results:")
                print(f"    Passed: {rerun_test_results.get('passed', 0)}")
                print(f"    Failed: {new_failed_count}")

                if new_failed_count < failed_count:
                    print(f"  ✓ Progress! Reduced failures from {failed_count} to {new_failed_count}")
                    failed_count = new_failed_count

                    if new_failed_count > 0:
                        # Re-analyze remaining failures
                        analysis = server.analyze_failures(rerun_id)
                        failures = analysis.get('failures', [])
                elif new_failed_count == 0:
                    print("  ✓ All failures fixed!")
                    failed_count = 0
                else:
                    print(f"  ⚠ No improvement. Still {new_failed_count} failure(s).")
                    break

            iteration += 1

        # Final summary
        print(f"\n=== FINAL SUMMARY ===")

        if failed_count == 0:
            print("✓ SUCCESS! All tests now passing after iterative debugging.")
        else:
            print(f"⚠ {failed_count} failure(s) remain after {iteration} iteration(s).")
            print("These may require manual investigation or code changes.")

    print("\n" + "=" * 80)


if __name__ == "__main__":
    main()
