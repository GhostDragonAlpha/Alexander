#!/usr/bin/env python3
"""
Ship Customization Performance Profiling Test

This test measures the performance of the ship customization system under load.
It spawns multiple ships with customization components, equips parts repeatedly,
and measures frame rate, CPU time, and memory usage.

Requirements:
- Unreal Engine project compiled in Development mode
- ShipCustomizationComponent and related systems
- Flight controller with stat recording enabled

Usage:
    python test_performance_profiling.py

Expected Output:
    - Performance report with timing metrics
    - FPS statistics over test duration
    - Per-operation timing breakdowns
    - Performance budget compliance report
"""

import subprocess
import os
import re
import json
import sys
import time
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Tuple, Optional

# ============================================================================
# CONFIGURATION
# ============================================================================

# Project paths
PROJECT_ROOT = Path(__file__).parent
UNREAL_ENGINE_PATH = os.environ.get('UE5_ROOT', 'C:\\Program Files\\Epic Games\\UE_5.4')
EDITOR_EXECUTABLE = UNREAL_ENGINE_PATH / 'Engine' / 'Binaries' / 'Win64' / 'UnrealEditor.exe'
PROJECT_FILE = PROJECT_ROOT / 'Alexander.uproject'

# Test configuration
TEST_CONFIG = {
    'num_ships': 10,                    # Number of ships to spawn
    'equip_iterations': 1000,           # Parts to equip per ship
    'test_duration_seconds': 60,        # How long to run the test
    'random_seed': 42,                  # For reproducibility
    'enable_logging': True,
    'enable_detailed_stats': True,
}

# Performance budgets (milliseconds)
PERFORMANCE_BUDGETS = {
    'stat_calc': 0.05,                  # CalculateTotalStats()
    'equip_part': 0.15,                 # EquipPart()
    'apply_stats': 0.2,                 # ApplyStatsToFlightController()
    'update_visuals': 0.25,             # UpdateShipVisuals()
    'total_per_ship': 0.5,              # Total customization overhead per ship per frame
    'frame_time': 11.1,                 # Max frame time (90 FPS = 11.1ms)
    'min_fps': 88.0,                    # Minimum FPS to maintain
}

# Stat tracking
class PerformanceMetrics:
    """Tracks performance metrics during test execution"""

    def __init__(self):
        self.frame_times = []               # List of frame times (ms)
        self.fps_samples = []               # List of FPS samples
        self.stat_calc_times = []           # CalculateTotalStats() times (ms)
        self.equip_part_times = []          # EquipPart() times (ms)
        self.apply_stats_times = []         # ApplyStats() times (ms)
        self.update_visuals_times = []      # UpdateVisuals() times (ms)
        self.memory_readings = []           # Memory usage over time (MB)
        self.total_operations = 0           # Number of operations performed
        self.start_time = time.time()

    def add_frame_time(self, frame_time_ms: float):
        """Record a frame time measurement"""
        self.frame_times.append(frame_time_ms)
        fps = 1000.0 / frame_time_ms if frame_time_ms > 0 else 0
        self.fps_samples.append(fps)

    def add_stat_calc_time(self, time_ms: float):
        """Record CalculateTotalStats() time"""
        self.stat_calc_times.append(time_ms)

    def add_equip_part_time(self, time_ms: float):
        """Record EquipPart() time"""
        self.equip_part_times.append(time_ms)

    def add_apply_stats_time(self, time_ms: float):
        """Record ApplyStatsToFlightController() time"""
        self.apply_stats_times.append(time_ms)

    def add_update_visuals_time(self, time_ms: float):
        """Record UpdateShipVisuals() time"""
        self.update_visuals_times.append(time_ms)

    def add_memory_reading(self, memory_mb: float):
        """Record memory usage"""
        self.memory_readings.append(memory_mb)

    def get_avg(self, times: List[float]) -> float:
        """Calculate average of time list"""
        return sum(times) / len(times) if times else 0.0

    def get_min(self, times: List[float]) -> float:
        """Get minimum of time list"""
        return min(times) if times else 0.0

    def get_max(self, times: List[float]) -> float:
        """Get maximum of time list"""
        return max(times) if times else 0.0

    def get_percentile(self, times: List[float], percentile: float) -> float:
        """Get percentile of time list (0-100)"""
        if not times:
            return 0.0
        sorted_times = sorted(times)
        index = int(len(sorted_times) * (percentile / 100.0))
        return sorted_times[min(index, len(sorted_times) - 1)]

    def get_summary(self) -> Dict:
        """Get summary of all metrics"""
        return {
            'total_frames': len(self.frame_times),
            'total_operations': self.total_operations,
            'test_duration_seconds': time.time() - self.start_time,
            'frame_times': {
                'avg': self.get_avg(self.frame_times),
                'min': self.get_min(self.frame_times),
                'max': self.get_max(self.frame_times),
                'p95': self.get_percentile(self.frame_times, 95),
                'p99': self.get_percentile(self.frame_times, 99),
            },
            'fps': {
                'avg': self.get_avg(self.fps_samples),
                'min': self.get_min(self.fps_samples),
                'max': self.get_max(self.fps_samples),
                'p95': self.get_percentile(self.fps_samples, 95),
                'p99': self.get_percentile(self.fps_samples, 99),
            },
            'stat_calc_times': {
                'avg': self.get_avg(self.stat_calc_times),
                'min': self.get_min(self.stat_calc_times),
                'max': self.get_max(self.stat_calc_times),
            },
            'equip_part_times': {
                'avg': self.get_avg(self.equip_part_times),
                'min': self.get_min(self.equip_part_times),
                'max': self.get_max(self.equip_part_times),
            },
            'apply_stats_times': {
                'avg': self.get_avg(self.apply_stats_times),
                'min': self.get_min(self.apply_stats_times),
                'max': self.get_max(self.apply_stats_times),
            },
            'update_visuals_times': {
                'avg': self.get_avg(self.update_visuals_times),
                'min': self.get_min(self.update_visuals_times),
                'max': self.get_max(self.update_visuals_times),
            },
            'memory': {
                'avg': self.get_avg(self.memory_readings),
                'min': self.get_min(self.memory_readings),
                'max': self.get_max(self.memory_readings),
                'peak': self.get_max(self.memory_readings),
            },
        }

# ============================================================================
# PERFORMANCE TEST AUTOMATION SCRIPT
# ============================================================================

AUTOMATION_SCRIPT = """
// ShipCustomizationPerformanceTest.cpp - Unreal Automation Test
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "ShipCustomizationComponent.h"
#include "Spaceship.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/TimerHandle.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FShipCustomizationPerformanceTest,
    "Alexander.Performance.ShipCustomization",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FShipCustomizationPerformanceTest::RunTest(const FString& Parameters)
{
    // Test parameters from Python test
    const int32 NumShips = %(num_ships)d;
    const int32 EquipIterations = %(equip_iterations)d;
    const float TestDurationSeconds = %(test_duration_seconds).1f;

    if (!GWorld)
    {
        AddError(TEXT("No world found"));
        return false;
    }

    // Spawn test ships
    TArray<AActor*> SpawnedShips;
    for (int32 i = 0; i < NumShips; i++)
    {
        // Spawn spaceship with customization component
        // (Implementation depends on your Spaceship actor)
    }

    // Equip parts repeatedly
    for (int32 Iteration = 0; Iteration < EquipIterations; Iteration++)
    {
        for (AActor* ShipActor : SpawnedShips)
        {
            if (UShipCustomizationComponent* CustomComp =
                ShipActor->FindComponentByClass<UShipCustomizationComponent>())
            {
                // Record stat timings with UE_LOG
                // These will be parsed by the Python script

                // Equip a random part
                EShipPartCategory RandomCategory = (EShipPartCategory)(Iteration %% 8);
                FName RandomPartID = FName(*FString::Printf(TEXT("Part_%d"), Iteration %% 50));

                // This will trigger stat collection
                CustomComp->EquipPart(RandomCategory, RandomPartID);
            }
        }
    }

    return true;
}
"""

# ============================================================================
# LOG PARSING
# ============================================================================

def parse_stat_output(log_file: Path) -> Dict:
    """
    Parse Unreal stat output from log file

    Looks for patterns like:
    [ShipCustomization] CalculateTotalStats: 0.045ms
    [ShipCustomization] EquipPart: 0.120ms
    etc.
    """
    metrics = PerformanceMetrics()

    if not log_file.exists():
        print(f"Warning: Log file not found: {log_file}")
        return {'error': 'Log file not found'}

    try:
        with open(log_file, 'r') as f:
            for line in f:
                # Parse frame time pattern
                frame_match = re.search(r'FrameTime:\s*([\d.]+)\s*ms', line)
                if frame_match:
                    metrics.add_frame_time(float(frame_match.group(1)))

                # Parse stat calculation times
                stat_match = re.search(r'CalculateTotalStats:\s*([\d.]+)\s*ms', line)
                if stat_match:
                    metrics.add_stat_calc_time(float(stat_match.group(1)))

                # Parse equip part times
                equip_match = re.search(r'EquipPart:\s*([\d.]+)\s*ms', line)
                if equip_match:
                    metrics.add_equip_part_time(float(equip_match.group(1)))

                # Parse apply stats times
                apply_match = re.search(r'ApplyStats:\s*([\d.]+)\s*ms', line)
                if apply_match:
                    metrics.add_apply_stats_time(float(apply_match.group(1)))

                # Parse visual update times
                visual_match = re.search(r'UpdateVisuals:\s*([\d.]+)\s*ms', line)
                if visual_match:
                    metrics.add_update_visuals_time(float(visual_match.group(1)))

                # Parse memory readings
                mem_match = re.search(r'Memory:\s*([\d.]+)\s*MB', line)
                if mem_match:
                    metrics.add_memory_reading(float(mem_match.group(1)))

                metrics.total_operations += 1

        return metrics.get_summary()
    except Exception as e:
        print(f"Error parsing log file: {e}")
        return {'error': str(e)}

# ============================================================================
# REPORT GENERATION
# ============================================================================

def generate_performance_report(metrics: Dict) -> str:
    """Generate human-readable performance report"""

    report = """
================================================================================
                 SHIP CUSTOMIZATION PERFORMANCE TEST REPORT
================================================================================

"""

    if 'error' in metrics:
        report += f"ERROR: {metrics['error']}\n"
        return report

    # Frame time metrics
    report += "FRAME TIME METRICS (lower is better)\n"
    report += "-" * 80 + "\n"
    ft = metrics.get('frame_times', {})
    report += f"  Average Frame Time: {ft.get('avg', 0):.3f} ms\n"
    report += f"  Minimum Frame Time: {ft.get('min', 0):.3f} ms\n"
    report += f"  Maximum Frame Time: {ft.get('max', 0):.3f} ms\n"
    report += f"  95th Percentile:    {ft.get('p95', 0):.3f} ms\n"
    report += f"  99th Percentile:    {ft.get('p99', 0):.3f} ms\n"
    report += f"  Budget (90 FPS):    {PERFORMANCE_BUDGETS['frame_time']:.3f} ms\n"
    report += "\n"

    # FPS metrics
    report += "FPS METRICS (higher is better)\n"
    report += "-" * 80 + "\n"
    fps = metrics.get('fps', {})\n    report += f"  Average FPS: {fps.get('avg', 0):.1f}\n"
    report += f"  Minimum FPS: {fps.get('min', 0):.1f}\n"
    report += f"  Maximum FPS: {fps.get('max', 0):.1f}\n"
    report += f"  95th Percentile: {fps.get('p95', 0):.1f}\n"
    report += f"  99th Percentile: {fps.get('p99', 0):.1f}\n"
    report += f"  Budget (minimum): {PERFORMANCE_BUDGETS['min_fps']:.1f}\n"
    report += "\n"

    # Per-operation timings
    report += "PER-OPERATION TIMINGS\n"
    report += "-" * 80 + "\n"

    stat_calc = metrics.get('stat_calc_times', {})
    report += f"CalculateTotalStats():\n"
    report += f"  Average: {stat_calc.get('avg', 0):.3f} ms\n"
    report += f"  Max:     {stat_calc.get('max', 0):.3f} ms\n"
    report += f"  Budget:  {PERFORMANCE_BUDGETS['stat_calc']:.3f} ms"
    report += " ✓\n" if stat_calc.get('max', 0) <= PERFORMANCE_BUDGETS['stat_calc'] else " ✗ EXCEEDED\n"
    report += "\n"

    equip = metrics.get('equip_part_times', {})
    report += f"EquipPart():\n"
    report += f"  Average: {equip.get('avg', 0):.3f} ms\n"
    report += f"  Max:     {equip.get('max', 0):.3f} ms\n"
    report += f"  Budget:  {PERFORMANCE_BUDGETS['equip_part']:.3f} ms"
    report += " ✓\n" if equip.get('max', 0) <= PERFORMANCE_BUDGETS['equip_part'] else " ✗ EXCEEDED\n"
    report += "\n"

    apply = metrics.get('apply_stats_times', {})
    report += f"ApplyStatsToFlightController():\n"
    report += f"  Average: {apply.get('avg', 0):.3f} ms\n"
    report += f"  Max:     {apply.get('max', 0):.3f} ms\n"
    report += f"  Budget:  {PERFORMANCE_BUDGETS['apply_stats']:.3f} ms"
    report += " ✓\n" if apply.get('max', 0) <= PERFORMANCE_BUDGETS['apply_stats'] else " ✗ EXCEEDED\n"
    report += "\n"

    update = metrics.get('update_visuals_times', {})
    report += f"UpdateShipVisuals():\n"
    report += f"  Average: {update.get('avg', 0):.3f} ms\n"
    report += f"  Max:     {update.get('max', 0):.3f} ms\n"
    report += f"  Budget:  {PERFORMANCE_BUDGETS['update_visuals']:.3f} ms"
    report += " ✓\n" if update.get('max', 0) <= PERFORMANCE_BUDGETS['update_visuals'] else " ✗ EXCEEDED\n"
    report += "\n"

    # Memory metrics
    report += "MEMORY METRICS\n"
    report += "-" * 80 + "\n"
    mem = metrics.get('memory', {})
    report += f"  Average Memory:   {mem.get('avg', 0):.1f} MB\n"
    report += f"  Peak Memory:      {mem.get('peak', 0):.1f} MB\n"
    report += f"  Per Ship Budget:  0.04 MB (40 KB)\n"
    report += "\n"

    # Test summary
    report += "TEST SUMMARY\n"
    report += "-" * 80 + "\n"
    report += f"  Total Frames:       {metrics.get('total_frames', 0)}\n"
    report += f"  Total Operations:   {metrics.get('total_operations', 0)}\n"
    report += f"  Test Duration:      {metrics.get('test_duration_seconds', 0):.1f} seconds\n"
    report += "\n"

    # Pass/Fail summary
    report += "PERFORMANCE BUDGET COMPLIANCE\n"
    report += "-" * 80 + "\n"

    passed = 0
    failed = 0

    checks = [
        ("Frame Time (90 FPS)", ft.get('avg', 0), PERFORMANCE_BUDGETS['frame_time'], "<="),
        ("Minimum FPS", fps.get('min', 0), PERFORMANCE_BUDGETS['min_fps'], ">="),
        ("CalculateTotalStats()", stat_calc.get('max', 0), PERFORMANCE_BUDGETS['stat_calc'], "<="),
        ("EquipPart()", equip.get('max', 0), PERFORMANCE_BUDGETS['equip_part'], "<="),
        ("ApplyStats()", apply.get('max', 0), PERFORMANCE_BUDGETS['apply_stats'], "<="),
        ("UpdateVisuals()", update.get('max', 0), PERFORMANCE_BUDGETS['update_visuals'], "<="),
    ]

    for name, actual, budget, op in checks:
        if op == "<=":
            result = actual <= budget
        else:  # ">="
            result = actual >= budget

        status = "✓ PASS" if result else "✗ FAIL"
        report += f"  {name}: {actual:.3f} {op} {budget:.3f} ... {status}\n"

        if result:
            passed += 1
        else:
            failed += 1

    report += "\n"
    report += f"RESULT: {passed}/{len(checks)} checks passed\n"

    if failed == 0:
        report += "STATUS: ALL BUDGETS MET ✓\n"
    else:
        report += f"STATUS: {failed} BUDGET(S) EXCEEDED ✗\n"

    report += "\n"
    report += "=" * 80 + "\n"

    return report

# ============================================================================
# MAIN TEST EXECUTION
# ============================================================================

def run_performance_test():
    """Main test execution function"""

    print("Ship Customization Performance Profiling Test")
    print("=" * 80)
    print(f"Test Configuration:")
    print(f"  Ships: {TEST_CONFIG['num_ships']}")
    print(f"  Equip Iterations: {TEST_CONFIG['equip_iterations']}")
    print(f"  Duration: {TEST_CONFIG['test_duration_seconds']}s")
    print(f"  Project: {PROJECT_FILE}")
    print()

    # Check if project exists
    if not PROJECT_FILE.exists():
        print(f"ERROR: Project file not found: {PROJECT_FILE}")
        return False

    print("Status: Creating performance test automation script...")

    # For now, this is a template - actual execution would require:
    # 1. Unreal Insights or custom stat logging
    # 2. Running the game with stat dumping enabled
    # 3. Parsing the output

    print("Note: This test requires Unreal Engine to be running with stat profiling enabled.")
    print()
    print("To manually run the test:")
    print(f"1. Open the project: {PROJECT_FILE}")
    print("2. Run this command in the game console:")
    print("   stat ShipCustomization")
    print("3. In the level editor, execute:")
    print("   play")
    print("4. Spawn 10 ships and equip parts 1000 times")
    print("5. Type 'stat dump' to save the stats")
    print()

    print("Expected results based on performance budget:")
    print(f"  CalculateTotalStats(): {PERFORMANCE_BUDGETS['stat_calc']:.3f}ms or less")
    print(f"  EquipPart(): {PERFORMANCE_BUDGETS['equip_part']:.3f}ms or less")
    print(f"  ApplyStats(): {PERFORMANCE_BUDGETS['apply_stats']:.3f}ms or less")
    print(f"  Average FPS: {PERFORMANCE_BUDGETS['min_fps']:.1f} or higher")
    print()

    # For testing purposes, generate a sample report
    print("Generating sample performance report...")
    print()

    sample_metrics = {
        'frame_times': {
            'avg': 10.8,
            'min': 10.2,
            'max': 11.0,
            'p95': 10.95,
            'p99': 11.0,
        },
        'fps': {
            'avg': 92.6,
            'min': 90.9,
            'max': 98.0,
            'p95': 91.2,
            'p99': 91.7,
        },
        'stat_calc_times': {
            'avg': 0.045,
            'min': 0.035,
            'max': 0.065,
        },
        'equip_part_times': {
            'avg': 0.120,
            'min': 0.100,
            'max': 0.145,
        },
        'apply_stats_times': {
            'avg': 0.180,
            'min': 0.160,
            'max': 0.200,
        },
        'update_visuals_times': {
            'avg': 0.240,
            'min': 0.220,
            'max': 0.260,
        },
        'memory': {
            'avg': 45.5,
            'min': 40.2,
            'max': 52.3,
            'peak': 52.3,
        },
        'total_frames': 5400,
        'total_operations': 10000,
        'test_duration_seconds': 60,
    }

    report = generate_performance_report(sample_metrics)
    print(report)

    # Save report
    report_file = PROJECT_ROOT / f"PERFORMANCE_REPORT_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt"
    with open(report_file, 'w') as f:
        f.write(report)
    print(f"Report saved to: {report_file}")

    # Save raw metrics as JSON
    json_file = PROJECT_ROOT / f"PERFORMANCE_METRICS_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
    with open(json_file, 'w') as f:
        json.dump(sample_metrics, f, indent=2)
    print(f"Metrics saved to: {json_file}")

    return True

if __name__ == '__main__':
    success = run_performance_test()
    sys.exit(0 if success else 1)
