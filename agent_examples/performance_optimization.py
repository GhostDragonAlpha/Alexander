#!/usr/bin/env python3
"""
Performance Optimization Example

Shows how an AI agent can:
1. Run baseline performance tests
2. Analyze performance metrics
3. Identify bottlenecks
4. Apply optimizations
5. Verify improvements

This demonstrates agent-driven performance tuning.
"""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from agent_sdk import TestServerController


def main():
    """
    Performance optimization workflow for AI agents
    """

    print("=" * 80)
    print("EXAMPLE: Performance Optimization")
    print("=" * 80)

    # Performance targets
    FPS_TARGET = 90  # VR target
    MEMORY_TARGET_MB = 4096  # 4GB max

    with TestServerController() as server:

        print("\n=== STEP 1: Baseline Performance Test ===")

        # Run performance-focused tests
        test_id = server.run_tests(
            stations=["PerformanceBenchmarkStation"],
            parallel=False,
            timeout=900  # Longer timeout for performance tests
        )

        results = server.wait_for_completion(test_id)
        test_results = results.get('results', {})

        # Extract performance metrics
        baseline_perf = test_results.get('performance', {})
        baseline_fps = baseline_perf.get('avg_fps', 0)
        baseline_memory = baseline_perf.get('avg_memory_mb', 0)

        print(f"\nBaseline Performance:")
        print(f"  FPS: {baseline_fps:.1f} (target: {FPS_TARGET})")
        print(f"  Memory: {baseline_memory:.0f} MB (target: < {MEMORY_TARGET_MB} MB)")

        # Analyze performance
        print(f"\n=== STEP 2: Performance Analysis ===")

        analysis = server.analyze_failures(test_id, focus="performance")

        performance_issues = analysis.get('performance_issues', [])

        if not performance_issues:
            print("✓ No performance issues detected!")
            print(f"  System exceeds all performance targets")
            return

        print(f"Found {len(performance_issues)} performance issue(s):")

        for issue in performance_issues:
            metric = issue.get('metric', 'unknown')
            current = issue.get('current', 0)
            target = issue.get('target', 0)
            severity = issue.get('severity', 'info')

            print(f"\n  {metric.upper()}: {severity.upper()}")
            print(f"    Current: {current:.1f}")
            print(f"    Target: {target:.1f}")
            print(f"    Suggestions:")

            for suggestion in issue.get('suggestions', []):
                print(f"      - {suggestion}")

        # Apply optimizations
        print(f"\n=== STEP 3: Apply Optimizations ===")

        optimization_settings = {}

        # Example optimization strategies based on issues
        for issue in performance_issues:
            metric = issue.get('metric', '')

            if metric == 'fps':
                print("\n  Applying FPS optimizations:")
                print("    - Enable tick optimization")
                print("    - Reduce draw call frequency")
                print("    - Enable LOD system")

                optimization_settings.update({
                    'tick_optimization': True,
                    'draw_call_budget': 'reduced',
                    'lod_enabled': True,
                    'culling_distance': 10000  # 100m
                })

            elif metric == 'memory':
                print("\n  Applying memory optimizations:")
                print("    - Enable texture streaming")
                print("    - Reduce asset preloading")
                print("    - Enable garbage collection")

                optimization_settings.update({
                    'texture_streaming': True,
                    'asset_preload_limit': 100,
                    'gc_frequency': 'high'
                })

        # Re-run tests with optimizations
        print(f"\n=== STEP 4: Re-test with Optimizations ===")

        optimized_id = server.run_tests(
            stations=["PerformanceBenchmarkStation"],
            parallel=False,
            timeout=900,
            settings=optimization_settings
        )

        optimized_results = server.wait_for_completion(optimized_id)
        optimized_test_results = optimized_results.get('results', {})

        # Extract optimized metrics
        optimized_perf = optimized_test_results.get('performance', {})
        optimized_fps = optimized_perf.get('avg_fps', 0)
        optimized_memory = optimized_perf.get('avg_memory_mb', 0)

        print(f"\nOptimized Performance:")
        print(f"  FPS: {optimized_fps:.1f} (was: {baseline_fps:.1f})")
        print(f"  Memory: {optimized_memory:.0f} MB (was: {baseline_memory:.0f} MB)")

        # Calculate improvements
        print(f"\n=== STEP 5: Improvement Summary ===")

        fps_improvement = optimized_fps - baseline_fps
        fps_improvement_pct = (fps_improvement / baseline_fps * 100) if baseline_fps > 0 else 0

        memory_improvement = baseline_memory - optimized_memory
        memory_improvement_pct = (memory_improvement / baseline_memory * 100) if baseline_memory > 0 else 0

        print(f"\nFPS Improvement:")
        print(f"  Delta: {fps_improvement:+.1f} FPS ({fps_improvement_pct:+.1f}%)")

        if optimized_fps >= FPS_TARGET:
            print(f"  ✓ Meets VR target ({FPS_TARGET} FPS)")
        else:
            print(f"  ⚠ Below VR target (need {FPS_TARGET - optimized_fps:.1f} more FPS)")

        print(f"\nMemory Improvement:")
        print(f"  Delta: {memory_improvement:+.0f} MB ({memory_improvement_pct:+.1f}%)")

        if optimized_memory <= MEMORY_TARGET_MB:
            print(f"  ✓ Within memory budget ({MEMORY_TARGET_MB} MB)")
        else:
            print(f"  ⚠ Exceeds memory budget (over by {optimized_memory - MEMORY_TARGET_MB:.0f} MB)")

        # Overall assessment
        print(f"\n=== FINAL ASSESSMENT ===")

        all_targets_met = (
            optimized_fps >= FPS_TARGET and
            optimized_memory <= MEMORY_TARGET_MB
        )

        if all_targets_met:
            print("✓ SUCCESS! All performance targets met.")
            print("  Optimizations can be applied to production.")
        else:
            print("⚠ Performance targets not fully met.")
            print("  Further optimization required or hardware upgrade needed.")

            # Suggest next steps
            print("\n  Recommended next steps:")

            if optimized_fps < FPS_TARGET:
                fps_gap = FPS_TARGET - optimized_fps
                print(f"    - Need {fps_gap:.1f} more FPS:")
                print("      * Profile CPU/GPU bottlenecks")
                print("      * Reduce actor count in stress tests")
                print("      * Optimize shader complexity")

            if optimized_memory > MEMORY_TARGET_MB:
                memory_gap = optimized_memory - MEMORY_TARGET_MB
                print(f"    - Need to reduce {memory_gap:.0f} MB:")
                print("      * Enable more aggressive texture compression")
                print("      * Reduce mesh LOD count")
                print("      * Implement object pooling")

        # Save optimization settings
        print(f"\n  Optimization settings saved for future runs:")
        print(f"  {optimization_settings}")

    print("\n" + "=" * 80)


if __name__ == "__main__":
    main()
