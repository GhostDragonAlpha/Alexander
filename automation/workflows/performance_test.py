"""
Performance Testing Workflow
============================

Automated performance profiling workflow:
1. Build game in Shipping mode
2. Run profiling session with Unreal Insights
3. Extract performance metrics
4. Compare against baseline
5. Generate performance report

Usage:
    python automation/workflows/performance_test.py [map_name] [duration]
"""

import sys
import logging
import json
from pathlib import Path
from datetime import datetime

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent))

from ue_tools import ue_build, ue_insights

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


class PerformanceTestWorkflow:
    """Automated performance profiling workflow"""

    def __init__(self, map_name: str = "VRTemplateMap", duration: int = 60):
        self.map_name = map_name
        self.duration = duration
        self.results_dir = Path.cwd() / "PerformanceResults"
        self.results_dir.mkdir(parents=True, exist_ok=True)

        self.timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        self.test_name = f"{map_name}_{self.timestamp}"

    def run(self):
        """Execute performance test workflow"""
        logger.info(f"=== Performance Test: {self.test_name} ===")

        try:
            # Step 1: Profile the map
            trace_file = self.run_profiling()
            if not trace_file:
                return False

            # Step 2: Extract metrics
            metrics = self.extract_metrics(trace_file)

            # Step 3: Generate report
            self.generate_report(metrics, trace_file)

            logger.info("=== Performance Test SUCCEEDED ===")
            return True

        except Exception as e:
            logger.exception(f"Performance test failed: {e}")
            return False

    def run_profiling(self) -> Path:
        """Step 1: Run profiling session"""
        logger.info(f"Profiling {self.map_name} for {self.duration}s...")

        profiler = ue_insights.UnrealInsightsAutomation()

        success, trace_file = profiler.run_profiling_workflow(
            map_name=self.map_name,
            test_name=self.test_name,
            trace_channels="cpu,gpu,frame,physics,memory",
            duration_seconds=self.duration
        )

        if not success:
            logger.error("Profiling failed")
            return None

        logger.info(f"Profiling complete: {trace_file.name}")
        return trace_file

    def extract_metrics(self, trace_file: Path) -> dict:
        """Step 2: Extract performance metrics"""
        logger.info("Extracting performance metrics...")

        profiler = ue_insights.UnrealInsightsAutomation()
        stats = profiler.extract_trace_stats(trace_file)

        # Add timestamp and test info
        stats["test_name"] = self.test_name
        stats["map_name"] = self.map_name
        stats["duration_seconds"] = self.duration
        stats["timestamp"] = self.timestamp

        logger.info(f"Metrics extracted: {len(stats)} entries")
        return stats

    def generate_report(self, metrics: dict, trace_file: Path):
        """Step 3: Generate performance report"""
        logger.info("Generating performance report...")

        # Save JSON metrics
        json_path = self.results_dir / f"{self.test_name}_metrics.json"
        with open(json_path, 'w') as f:
            json.dump(metrics, f, indent=2)

        # Save text report
        report_path = self.results_dir / f"{self.test_name}_report.txt"
        with open(report_path, 'w') as f:
            f.write(f"Performance Test Report: {self.test_name}\n")
            f.write("=" * 60 + "\n\n")

            f.write(f"Map: {self.map_name}\n")
            f.write(f"Duration: {self.duration}s\n")
            f.write(f"Timestamp: {self.timestamp}\n")
            f.write(f"Trace File: {trace_file.name}\n\n")

            f.write("Metrics:\n")
            f.write("-" * 60 + "\n")

            for key, value in metrics.items():
                if key not in ["test_name", "map_name", "duration_seconds", "timestamp"]:
                    f.write(f"  {key}: {value}\n")

            f.write("\n")
            f.write(f"Full trace available at: {trace_file}\n")
            f.write(f"Open with: UnrealInsights.exe {trace_file}\n")

        logger.info(f"Report saved:")
        logger.info(f"  JSON: {json_path}")
        logger.info(f"  Text: {report_path}")


def main():
    """Main entry point"""
    map_name = sys.argv[1] if len(sys.argv) > 1 else "VRTemplateMap"
    duration = int(sys.argv[2]) if len(sys.argv) > 2 else 60

    workflow = PerformanceTestWorkflow(map_name, duration)
    success = workflow.run()

    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
