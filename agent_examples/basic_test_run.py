#!/usr/bin/env python3
"""
Basic Agent Test Run Example

Shows the simplest way for an AI agent to:
1. Start the test server
2. Run tests
3. Get results
4. Stop the server
"""

import sys
from pathlib import Path

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent))

from agent_sdk import TestServerController


def main():
    """
    Basic test execution workflow for AI agents
    """

    print("=" * 80)
    print("EXAMPLE: Basic Agent Test Run")
    print("=" * 80)

    # Using context manager (automatically starts and stops server)
    with TestServerController() as server:

        print("\n1. Server started and ready")

        # Get initial status
        status = server.get_status()
        print(f"   Available stations: {len(status['available_stations'])}")
        for station in status['available_stations']:
            print(f"     - {station}")

        # Run VR and Flight tests
        print("\n2. Running tests...")
        test_id = server.run_tests(
            stations=["VRTestStation", "SpaceshipFlightTestStation"],
            parallel=True,
            timeout=600
        )
        print(f"   Test ID: {test_id}")

        # Wait for completion
        print("\n3. Waiting for test completion...")
        results = server.wait_for_completion(test_id)

        # Display results
        print("\n4. Test Results:")
        test_results = results.get('results', {})
        print(f"   Status: {results['status']}")
        print(f"   Total tests: {test_results.get('total_tests', 0)}")
        print(f"   Passed: {test_results.get('passed', 0)}")
        print(f"   Failed: {test_results.get('failed', 0)}")
        print(f"   Pass rate: {test_results.get('pass_rate', 0):.1f}%")

        if 'performance' in test_results:
            perf = test_results['performance']
            print(f"\n5. Performance:")
            print(f"   Average FPS: {perf.get('avg_fps', 0):.1f}")
            print(f"   Memory: {perf.get('avg_memory_mb', 0):.0f} MB")
            print(f"   VR Target Met: {perf.get('meets_vr_target', False)}")

        # Check if failures exist
        if test_results.get('failed', 0) > 0:
            print(f"\n⚠ {test_results['failed']} test(s) failed!")
            print("   See 'iterative_debugging.py' for how to handle failures")
        else:
            print("\n✓ All tests passed!")

    print("\n6. Server stopped")
    print("=" * 80)


if __name__ == "__main__":
    main()
