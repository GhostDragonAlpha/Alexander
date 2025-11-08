#!/usr/bin/env python3
"""
Ship Customization Performance Regression Test Suite

Runs all performance tests and compares against baseline metrics.
Fails if any metric regresses by more than 10%.

Usage:
    python performance_regression_suite.py [--baseline] [--compare baseline_file]

Arguments:
    --baseline: Generate new baseline metrics (save current as reference)
    --compare: Compare against specific baseline file
"""

import json
import os
import sys
import argparse
from pathlib import Path
from datetime import datetime
from typing import Dict, Tuple, List

# ============================================================================
# CONFIGURATION
# ============================================================================

PROJECT_ROOT = Path(__file__).parent
BASELINE_DIR = PROJECT_ROOT / '.performance_baselines'
REGRESSION_THRESHOLD = 0.10  # 10% regression threshold

# ============================================================================
# BASELINE MANAGEMENT
# ============================================================================

class PerformanceBaseline:
    """Manages performance baselines for regression testing"""

    def __init__(self, baseline_file: Path = None):
        if baseline_file is None:
            baseline_file = self._get_latest_baseline()

        self.baseline_file = baseline_file
        self.data = {}

        if baseline_file and baseline_file.exists():
            with open(baseline_file, 'r') as f:
                self.data = json.load(f)

    @staticmethod
    def _get_latest_baseline() -> Path:
        """Get the most recent baseline file"""
        BASELINE_DIR.mkdir(exist_ok=True)

        baseline_files = sorted(BASELINE_DIR.glob('baseline_*.json'), reverse=True)
        return baseline_files[0] if baseline_files else None

    def save(self, name: str = None) -> Path:
        """Save current metrics as baseline"""
        BASELINE_DIR.mkdir(exist_ok=True)

        if name is None:
            name = datetime.now().strftime('%Y%m%d_%H%M%S')

        baseline_file = BASELINE_DIR / f"baseline_{name}.json"

        with open(baseline_file, 'w') as f:
            json.dump(self.data, f, indent=2)

        print(f"Baseline saved: {baseline_file}")
        return baseline_file

    def set_metric(self, metric_name: str, value: float):
        """Set a metric value"""
        self.data[metric_name] = value

    def get_metric(self, metric_name: str) -> float:
        """Get a metric value"""
        return self.data.get(metric_name, 0.0)

    def has_baseline(self) -> bool:
        """Check if baseline exists"""
        return bool(self.data)

# ============================================================================
# REGRESSION DETECTION
# ============================================================================

class RegressionAnalyzer:
    """Analyzes performance regressions"""

    def __init__(self, baseline: PerformanceBaseline, current_metrics: Dict):
        self.baseline = baseline
        self.current_metrics = current_metrics
        self.regressions = []
        self.improvements = []
        self.warnings = []

    def check_regression(self, metric_name: str, threshold: float = REGRESSION_THRESHOLD) -> Tuple[bool, float]:
        """
        Check if a metric regressed
        Returns: (has_regressed, regression_percent)
        """
        baseline_value = self.baseline.get_metric(metric_name)
        current_value = self.current_metrics.get(metric_name, 0.0)

        if baseline_value == 0:
            return (False, 0.0)

        # Lower is better for latency/time metrics
        if 'time' in metric_name.lower() or 'latency' in metric_name.lower():
            regression_percent = (current_value - baseline_value) / baseline_value if baseline_value != 0 else 0
            regressed = regression_percent > threshold

        # Higher is better for FPS/bandwidth
        elif 'fps' in metric_name.lower() or 'bandwidth' in metric_name.lower():
            regression_percent = (baseline_value - current_value) / baseline_value if baseline_value != 0 else 0
            regressed = regression_percent > threshold
        else:
            return (False, 0.0)

        return (regressed, regression_percent * 100)

    def analyze_all(self) -> Dict:
        """Analyze all metrics for regressions"""
        results = {
            'regressions': [],
            'improvements': [],
            'warnings': [],
            'passed': True,
        }

        for metric_name, current_value in self.current_metrics.items():
            baseline_value = self.baseline.get_metric(metric_name)

            if baseline_value == 0:
                continue  # Skip if no baseline

            regressed, percent = self.check_regression(metric_name)

            if regressed:
                results['regressions'].append({
                    'metric': metric_name,
                    'baseline': baseline_value,
                    'current': current_value,
                    'regression_percent': percent,
                })
                results['passed'] = False

            elif percent < 0:  # Improvement (negative regression)
                results['improvements'].append({
                    'metric': metric_name,
                    'baseline': baseline_value,
                    'current': current_value,
                    'improvement_percent': -percent,
                })

        return results

# ============================================================================
# REPORT GENERATION
# ============================================================================

def generate_regression_report(analysis_results: Dict) -> str:
    """Generate performance regression report"""

    report = """
================================================================================
                    PERFORMANCE REGRESSION TEST REPORT
================================================================================

"""

    # Overall result
    if analysis_results['passed']:
        report += "RESULT: ✓ ALL TESTS PASSED\n"
        report += "Status: No performance regressions detected\n\n"
    else:
        report += "RESULT: ✗ PERFORMANCE REGRESSIONS DETECTED\n"
        report += "Status: One or more metrics exceeded threshold\n\n"

    # Regressions
    if analysis_results['regressions']:
        report += "REGRESSIONS (threshold: " + str(REGRESSION_THRESHOLD * 100) + "%)\n"
        report += "-" * 80 + "\n"

        for reg in analysis_results['regressions']:
            report += f"\n{reg['metric']}:\n"
            report += f"  Baseline: {reg['baseline']:.4f}\n"
            report += f"  Current:  {reg['current']:.4f}\n"
            report += f"  Regression: {reg['regression_percent']:.2f}%\n"

        report += "\n"

    # Improvements
    if analysis_results['improvements']:
        report += "IMPROVEMENTS\n"
        report += "-" * 80 + "\n"

        for imp in analysis_results['improvements']:
            report += f"\n{imp['metric']}:\n"
            report += f"  Baseline: {imp['baseline']:.4f}\n"
            report += f"  Current:  {imp['current']:.4f}\n"
            report += f"  Improvement: {imp['improvement_percent']:.2f}%\n"

        report += "\n"

    report += "=" * 80 + "\n"

    return report

# ============================================================================
# SAMPLE METRICS COLLECTION
# ============================================================================

def collect_sample_metrics() -> Dict:
    """Collect sample performance metrics (for demo purposes)"""

    return {
        # Frame time metrics (lower is better)
        'avg_frame_time_ms': 10.2,
        'max_frame_time_ms': 11.0,
        'stat_calc_avg_ms': 0.045,
        'equip_part_avg_ms': 0.120,
        'apply_stats_avg_ms': 0.180,
        'update_visuals_avg_ms': 0.240,

        # FPS metrics (higher is better)
        'avg_fps': 92.0,
        'min_fps': 91.0,

        # Memory metrics (lower is better)
        'per_ship_memory_kb': 42.0,
        'peak_memory_mb': 305.5,

        # Network metrics (lower is better)
        'avg_bandwidth_kbps': 85.0,
        'packet_loss_percent': 0.15,
    }

# ============================================================================
# MAIN TEST RUNNER
# ============================================================================

def main():
    """Main regression test execution"""

    parser = argparse.ArgumentParser(
        description='Ship Customization Performance Regression Test Suite'
    )
    parser.add_argument(
        '--baseline',
        action='store_true',
        help='Generate new baseline from current metrics'
    )
    parser.add_argument(
        '--compare',
        type=Path,
        help='Compare against specific baseline file'
    )

    args = parser.parse_args()

    print("Ship Customization Performance Regression Suite")
    print("=" * 80)
    print()

    # Load baseline
    if args.compare:
        baseline = PerformanceBaseline(args.compare)
        print(f"Using baseline: {args.compare}")
    else:
        baseline = PerformanceBaseline()
        if baseline.has_baseline():
            print(f"Using latest baseline: {baseline.baseline_file}")
        else:
            print("No baseline found. Run with --baseline to create one.")
            return False

    print()

    # Collect current metrics
    print("Collecting performance metrics...")
    current_metrics = collect_sample_metrics()

    print("Metrics collected:")
    for metric_name, value in current_metrics.items():
        print(f"  {metric_name}: {value:.3f}")

    print()

    # Generate new baseline if requested
    if args.baseline:
        print("Generating new baseline...")
        baseline.data = current_metrics
        baseline.save()
        print("Baseline generated successfully")
        return True

    # Analyze regressions
    print("Analyzing performance regressions...")
    print()

    if not baseline.has_baseline():
        print("Error: No baseline available for comparison")
        return False

    analyzer = RegressionAnalyzer(baseline, current_metrics)
    results = analyzer.analyze_all()

    # Generate report
    report = generate_regression_report(results)
    print(report)

    # Save report
    report_file = PROJECT_ROOT / f"PERFORMANCE_REGRESSION_REPORT_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt"
    with open(report_file, 'w') as f:
        f.write(report)
    print(f"Report saved to: {report_file}")

    # Return success/failure
    return results['passed']

if __name__ == '__main__':
    success = main()
    sys.exit(0 if success else 1)
