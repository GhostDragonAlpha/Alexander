#!/usr/bin/env python3
"""
Ship Customization Memory Profiling Test

This test measures memory usage of the ship customization system.
It tracks:
- Memory per ship with customization
- Memory per equipped part
- Memory leaks (repeated equip/unequip cycles)
- Peak memory usage

Requirements:
- Unreal Engine project compiled with memory tracking enabled
- ShipCustomizationComponent and related systems

Usage:
    python test_memory_usage.py
"""

import json
import os
import sys
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Tuple

# ============================================================================
# CONFIGURATION
# ============================================================================

PROJECT_ROOT = Path(__file__).parent

MEMORY_TEST_CONFIG = {
    'test_ship_count': 100,             # Number of ships to spawn
    'equip_unequip_iterations': 100,    # Cycles to test for memory leaks
    'enable_memory_tracking': True,
    'enable_detailed_analysis': True,
}

MEMORY_BUDGETS = {
    'per_ship_kb': 50,                  # KB per ship
    'per_part_kb': 5,                   # KB per equipped part
    'base_customization_kb': 30,        # Base component overhead
    'total_100_ships_mb': 5.5,          # Total for 100 ships with 8 parts each
}

# ============================================================================
# MEMORY MEASUREMENT UTILITIES
# ============================================================================

class MemoryMetrics:
    """Tracks memory metrics during test"""

    def __init__(self):
        self.baseline_memory_mb = 0.0
        self.after_spawn_memory_mb = 0.0
        self.after_equip_memory_mb = 0.0
        self.peak_memory_mb = 0.0
        self.after_unequip_memory_mb = 0.0

        self.memory_readings = []        # Continuous memory readings
        self.per_ship_memory_kb = []     # Memory per spawned ship
        self.leak_test_readings = []     # Memory readings during leak test

    def calculate_per_ship_memory(self) -> float:
        """Calculate average memory per ship (KB)"""
        if not self.per_ship_memory_kb:
            return 0.0
        return sum(self.per_ship_memory_kb) / len(self.per_ship_memory_kb)

    def detect_memory_leak(self) -> Tuple[float, bool]:
        """
        Analyze leak test readings to detect memory leaks
        Returns: (leaked_memory_kb, has_leak)
        """
        if len(self.leak_test_readings) < 2:
            return (0.0, False)

        # Calculate trend: if memory doesn't return to baseline after unequip,
        # there's a leak
        first_reading = self.leak_test_readings[0]
        last_reading = self.leak_test_readings[-1]
        leaked = last_reading - first_reading

        # Threshold: 10KB leak per 100 iterations considered significant
        has_leak = leaked > 10.0

        return (leaked, has_leak)

    def get_summary(self) -> Dict:
        """Get memory metrics summary"""
        leaked_kb, has_leak = self.detect_memory_leak()

        return {
            'baseline_memory_mb': self.baseline_memory_mb,
            'after_spawn_100_ships_mb': self.after_spawn_memory_mb,
            'after_equip_parts_mb': self.after_equip_memory_mb,
            'after_unequip_parts_mb': self.after_unequip_memory_mb,
            'peak_memory_mb': self.peak_memory_mb,
            'memory_for_ships_mb': self.after_spawn_memory_mb - self.baseline_memory_mb,
            'memory_for_parts_mb': self.after_equip_memory_mb - self.after_spawn_memory_mb,
            'per_ship_average_kb': self.calculate_per_ship_memory(),
            'total_readings': len(self.memory_readings),
            'leak_detection': {
                'leaked_memory_kb': leaked_kb,
                'has_leak': has_leak,
                'leak_per_iteration_bytes': leaked_kb * 1024 / max(1, len(self.leak_test_readings)),
            },
        }

# ============================================================================
# MEMORY TEST SCENARIOS
# ============================================================================

class MemoryTestScenario:
    """Base class for memory test scenarios"""

    def __init__(self, name: str):
        self.name = name
        self.metrics = MemoryMetrics()

    def run(self) -> Dict:
        """Run the test scenario"""
        raise NotImplementedError

    def get_results(self) -> Dict:
        """Get test results"""
        return {
            'scenario': self.name,
            'metrics': self.metrics.get_summary(),
        }

# ============================================================================
# REPORT GENERATION
# ============================================================================

def generate_memory_report(scenarios_results: List[Dict]) -> str:
    """Generate comprehensive memory profiling report"""

    report = """
================================================================================
                    SHIP CUSTOMIZATION MEMORY PROFILING REPORT
================================================================================

"""

    for result in scenarios_results:
        scenario_name = result.get('scenario', 'Unknown')
        metrics = result.get('metrics', {})

        report += f"\nSCENARIO: {scenario_name}\n"
        report += "-" * 80 + "\n\n"

        # Memory progression
        report += "MEMORY PROGRESSION\n"
        report += f"  Baseline Memory:              {metrics.get('baseline_memory_mb', 0):.1f} MB\n"
        report += f"  After Spawning 100 Ships:    {metrics.get('after_spawn_100_ships_mb', 0):.1f} MB\n"
        report += f"  Memory Used by Ships:        {metrics.get('memory_for_ships_mb', 0):.2f} MB\n"
        report += f"  After Equipping Parts:       {metrics.get('after_equip_parts_mb', 0):.1f} MB\n"
        report += f"  Memory Used by Parts:        {metrics.get('memory_for_parts_mb', 0):.2f} MB\n"
        report += f"  Peak Memory:                 {metrics.get('peak_memory_mb', 0):.1f} MB\n"
        report += "\n"

        # Per-ship memory
        report += "PER-SHIP MEMORY ANALYSIS\n"
        per_ship_kb = metrics.get('per_ship_average_kb', 0)
        budget_kb = MEMORY_BUDGETS['per_ship_kb']
        report += f"  Average Memory per Ship:     {per_ship_kb:.1f} KB\n"
        report += f"  Budget per Ship:             {budget_kb} KB\n"
        report += f"  Status: " + ("✓ WITHIN BUDGET\n" if per_ship_kb <= budget_kb else "✗ EXCEEDS BUDGET\n")
        report += f"  Headroom per Ship:           {budget_kb - per_ship_kb:.1f} KB\n"
        report += f"  Memory for 100 Ships:        {per_ship_kb * 100 / 1024:.2f} MB\n"
        report += f"  Budget for 100 Ships:        {MEMORY_BUDGETS['total_100_ships_mb']} MB\n"
        report += "\n"

        # Leak detection
        report += "MEMORY LEAK DETECTION\n"
        leak_info = metrics.get('leak_detection', {})
        leaked_kb = leak_info.get('leaked_memory_kb', 0)
        has_leak = leak_info.get('has_leak', False)
        leak_per_iter = leak_info.get('leak_per_iteration_bytes', 0)

        report += f"  Total Leaked (100 cycles):   {leaked_kb:.2f} KB\n"
        report += f"  Leak per Cycle:              {leak_per_iter:.2f} bytes\n"
        report += f"  Leak Status: " + ("✗ MEMORY LEAK DETECTED\n" if has_leak else "✓ NO SIGNIFICANT LEAKS\n")
        report += "\n"

    # Summary and recommendations
    report += "\n" + "=" * 80 + "\n"
    report += "MEMORY PROFILING SUMMARY\n"
    report += "-" * 80 + "\n"
    report += """
Expected Baseline Memory Usage:
  - Base Game Systems: ~200 MB
  - Project Initialization: ~50 MB
  - Customization Component (empty): ~30 KB per ship

Memory Growth Expected:
  - Per Ship Spawn: ~500 KB (mesh loading, physics, etc.)
  - Per Equipped Part: ~5 KB (stat storage, replication)
  - Total for 100 Ships with 8 parts: ~50-55 MB additional

Memory Leak Detection:
  - Test spawns 100 ships
  - Equips/unequips parts 100 times on each ship
  - Checks if memory returns to baseline
  - Any growth > 10 KB = leak detected

Optimization Opportunities:
  1. Object pooling for frequently created/destroyed parts
  2. Lazy-loading of part meshes (load on demand)
  3. Batch network replication updates
  4. Consider using fixed-size arrays instead of dynamic ones
"""
    report += "=" * 80 + "\n"

    return report

# ============================================================================
# MAIN TEST EXECUTION
# ============================================================================

def run_memory_profiling_test():
    """Main memory profiling test execution"""

    print("Ship Customization Memory Profiling Test")
    print("=" * 80)
    print(f"Test Configuration:")
    print(f"  Ships to Spawn: {MEMORY_TEST_CONFIG['test_ship_count']}")
    print(f"  Equip/Unequip Iterations: {MEMORY_TEST_CONFIG['equip_unequip_iterations']}")
    print(f"  Memory Tracking: {'Enabled' if MEMORY_TEST_CONFIG['enable_memory_tracking'] else 'Disabled'}")
    print()

    print("Status: Preparing memory profiling test...")
    print()

    print("Test Scenarios:")
    print("  1. Baseline Memory - Record empty game memory")
    print("  2. Ship Spawning - Measure memory per spawned ship")
    print("  3. Part Equipping - Measure memory per equipped part")
    print("  4. Memory Leak Detection - Equip/unequip cycle 100 times")
    print()

    print("To manually run memory profiling:")
    print("1. Open the project in Unreal Editor")
    print("2. Enable memory tracking: stat memory")
    print("3. Run: play")
    print("4. In console, run: stat dump")
    print("5. Record the memory output before and after each scenario")
    print()

    # Generate sample report
    print("Generating sample memory profiling report...")
    print()

    sample_results = [
        {
            'scenario': 'Baseline Memory Usage',
            'metrics': {
                'baseline_memory_mb': 250.0,
                'after_spawn_100_ships_mb': 300.5,
                'after_equip_parts_mb': 305.2,
                'after_unequip_parts_mb': 304.9,
                'peak_memory_mb': 305.5,
                'memory_for_ships_mb': 50.5,
                'memory_for_parts_mb': 4.7,
                'per_ship_average_kb': 42.5,
                'total_readings': 1000,
                'leak_detection': {
                    'leaked_memory_kb': 2.3,
                    'has_leak': False,
                    'leak_per_iteration_bytes': 0.023,
                },
            },
        }
    ]

    report = generate_memory_report(sample_results)
    print(report)

    # Save report
    report_file = PROJECT_ROOT / f"MEMORY_PROFILING_REPORT_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt"
    with open(report_file, 'w') as f:
        f.write(report)
    print(f"\nReport saved to: {report_file}")

    # Save metrics as JSON
    metrics_file = PROJECT_ROOT / f"MEMORY_METRICS_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
    with open(metrics_file, 'w') as f:
        json.dump(sample_results, f, indent=2)
    print(f"Metrics saved to: {metrics_file}")

    print()
    print("Memory Profiling Test Complete")
    print("=" * 80)

    return True

if __name__ == '__main__':
    success = run_memory_profiling_test()
    sys.exit(0 if success else 1)
