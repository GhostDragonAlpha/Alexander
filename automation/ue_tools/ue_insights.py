"""
UnrealInsights.exe Automation Wrapper
======================================

Provides programmatic control over performance profiling:
- Start game with trace capture enabled
- Manage trace channels (CPU, GPU, memory, etc.)
- Find and analyze trace files
- Automated performance regression detection

Usage:
    from automation.ue_tools import ue_insights

    profiler = ue_insights.UnrealInsightsAutomation()

    # Start profiling session
    process = profiler.start_profiling_session(
        map_name="VRTemplateMap",
        trace_channels="cpu,gpu,frame,physics",
        duration_seconds=300
    )

    # Get latest trace
    trace_file = profiler.get_latest_trace()

    # Open in Insights
    profiler.open_in_insights(trace_file)
"""

import subprocess
import logging
import time
from pathlib import Path
from typing import Tuple, Optional, List
from datetime import datetime
from . import UNREAL_EDITOR, UNREAL_INSIGHTS, PROJECT_FILE

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class UnrealInsightsAutomation:
    """Wrapper for UnrealInsights profiling automation"""

    # Default trace storage location
    TRACE_DIR = Path.home() / "AppData/Local/UnrealEngine/Common/UnrealTrace/Store"

    def __init__(self, project_path: Optional[Path] = None):
        """
        Initialize Insights automation

        Args:
            project_path: Path to .uproject file (defaults to PROJECT_FILE)
        """
        self.project_path = Path(project_path) if project_path else PROJECT_FILE
        self.editor_exe = UNREAL_EDITOR
        self.insights_exe = UNREAL_INSIGHTS

        if not self.project_path.exists():
            raise FileNotFoundError(f"Project file not found: {self.project_path}")
        if not self.editor_exe.exists():
            raise FileNotFoundError(f"UnrealEditor.exe not found: {self.editor_exe}")
        if not self.insights_exe.exists():
            raise FileNotFoundError(f"UnrealInsights.exe not found: {self.insights_exe}")

    def start_profiling_session(
        self,
        map_name: str = "VRTemplateMap",
        trace_channels: str = "cpu,gpu,frame",
        resolution: Tuple[int, int] = (1280, 720),
        duration_seconds: Optional[int] = None,
        windowed: bool = True,
        game_mode: bool = True
    ) -> subprocess.Popen:
        """
        Start game with profiling enabled

        Args:
            map_name: Map to load
            trace_channels: Comma-separated trace channels
            resolution: Window resolution (width, height)
            duration_seconds: Auto-terminate after seconds (None = manual)
            windowed: Run in windowed mode
            game_mode: Run as game (not editor)

        Returns:
            Popen process object

        Available trace channels:
            - cpu: CPU profiling
            - gpu: GPU profiling
            - frame: Frame timing
            - loadtime: Asset loading
            - file: File I/O
            - bookmark: Manual bookmarks
            - physics: Physics simulation
            - memory: Memory allocations
            - networking: Network traffic
            - animation: Animation system
            - particles: Particle systems
        """
        logger.info(f"Starting profiling session: {map_name} with {trace_channels}")

        cmd = [
            str(self.editor_exe),
            str(self.project_path),
            map_name,
            f"-trace={trace_channels}",
            f"-ResX={resolution[0]}",
            f"-ResY={resolution[1]}"
        ]

        if game_mode:
            cmd.append("-game")

        if windowed:
            cmd.append("-windowed")

        logger.debug(f"Command: {' '.join(cmd)}")

        process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        if duration_seconds:
            logger.info(f"Will auto-terminate after {duration_seconds}s")
            # Could implement auto-termination with timer

        return process

    def get_latest_trace(self) -> Optional[Path]:
        """
        Find most recent trace file

        Returns:
            Path to latest .utrace file, or None if not found
        """
        if not self.TRACE_DIR.exists():
            logger.warning(f"Trace directory not found: {self.TRACE_DIR}")
            return None

        traces = list(self.TRACE_DIR.glob("*.utrace"))
        if not traces:
            logger.warning("No trace files found")
            return None

        latest = max(traces, key=lambda p: p.stat().st_mtime)
        logger.info(f"Latest trace: {latest.name}")
        return latest

    def get_traces_since(self, timestamp: datetime) -> List[Path]:
        """
        Get all traces created after timestamp

        Args:
            timestamp: Datetime threshold

        Returns:
            List of trace file paths
        """
        if not self.TRACE_DIR.exists():
            return []

        traces = []
        for trace_path in self.TRACE_DIR.glob("*.utrace"):
            mtime = datetime.fromtimestamp(trace_path.stat().st_mtime)
            if mtime > timestamp:
                traces.append(trace_path)

        return sorted(traces, key=lambda p: p.stat().st_mtime)

    def open_in_insights(self, trace_file: Path) -> subprocess.Popen:
        """
        Open trace file in UnrealInsights GUI

        Args:
            trace_file: Path to .utrace file

        Returns:
            Popen process object
        """
        logger.info(f"Opening trace in Insights: {trace_file.name}")

        cmd = [str(self.insights_exe), str(trace_file)]

        return subprocess.Popen(cmd)

    def extract_trace_stats(self, trace_file: Path) -> dict:
        """
        Extract basic statistics from trace file (placeholder)

        Note: Full trace parsing requires UnrealInsights Python API or
        custom .utrace binary parser. This is a placeholder for future
        implementation.

        Args:
            trace_file: Path to .utrace file

        Returns:
            Dictionary of statistics
        """
        logger.warning("Trace parsing not yet implemented - returning file stats only")

        if not trace_file.exists():
            return {"error": "Trace file not found"}

        stat = trace_file.stat()

        return {
            "file_name": trace_file.name,
            "file_size_mb": stat.st_size / (1024 * 1024),
            "created": datetime.fromtimestamp(stat.st_ctime).isoformat(),
            "modified": datetime.fromtimestamp(stat.st_mtime).isoformat(),
            "note": "Full trace parsing requires UnrealInsights API"
        }

    def run_profiling_workflow(
        self,
        map_name: str,
        test_name: str,
        trace_channels: str = "cpu,gpu,frame,physics",
        duration_seconds: int = 60
    ) -> Tuple[bool, Optional[Path]]:
        """
        Complete profiling workflow:
        1. Start game with tracing
        2. Wait for duration
        3. Terminate game
        4. Return trace file path

        Args:
            map_name: Map to profile
            test_name: Test identifier for logging
            trace_channels: Trace channels to capture
            duration_seconds: How long to profile

        Returns:
            Tuple of (success: bool, trace_file: Optional[Path])
        """
        logger.info(f"Running profiling workflow: {test_name}")

        # Mark start time
        start_time = datetime.now()

        # Start profiling
        process = self.start_profiling_session(
            map_name=map_name,
            trace_channels=trace_channels,
            duration_seconds=duration_seconds
        )

        try:
            # Wait for duration
            logger.info(f"Profiling for {duration_seconds}s...")
            time.sleep(duration_seconds)

            # Terminate game
            logger.info("Terminating profiling session")
            process.terminate()
            process.wait(timeout=10)

        except subprocess.TimeoutExpired:
            logger.error("Failed to terminate gracefully, killing process")
            process.kill()
            return False, None

        # Find trace file created during session
        traces = self.get_traces_since(start_time)

        if not traces:
            logger.error("No trace file created")
            return False, None

        trace_file = traces[-1]  # Get latest
        logger.info(f"Profiling complete: {trace_file.name}")

        return True, trace_file


# Convenience functions
def profile_map(map_name: str, duration: int = 60, **kwargs) -> Tuple[bool, Optional[Path]]:
    """Quick profile a map"""
    profiler = UnrealInsightsAutomation()
    return profiler.run_profiling_workflow(
        map_name=map_name,
        test_name=f"profile_{map_name}",
        duration_seconds=duration,
        **kwargs
    )


def open_latest_trace():
    """Quick open latest trace in Insights"""
    profiler = UnrealInsightsAutomation()
    trace = profiler.get_latest_trace()
    if trace:
        profiler.open_in_insights(trace)
    else:
        print("No trace files found")


if __name__ == "__main__":
    # Example usage
    import sys

    if len(sys.argv) < 2:
        print("Usage: python ue_insights.py [profile|open|latest] [args]")
        sys.exit(1)

    command = sys.argv[1]
    profiler = UnrealInsightsAutomation()

    if command == "profile":
        map_name = sys.argv[2] if len(sys.argv) > 2 else "VRTemplateMap"
        duration = int(sys.argv[3]) if len(sys.argv) > 3 else 60
        success, trace_file = profiler.run_profiling_workflow(
            map_name, f"manual_profile_{map_name}", duration_seconds=duration
        )
        if success:
            print(f"Trace created: {trace_file}")
            sys.exit(0)
        else:
            print("Profiling failed")
            sys.exit(1)

    elif command == "open":
        trace_file = Path(sys.argv[2]) if len(sys.argv) > 2 else profiler.get_latest_trace()
        if trace_file:
            profiler.open_in_insights(trace_file)
        else:
            print("No trace file specified or found")
            sys.exit(1)

    elif command == "latest":
        trace = profiler.get_latest_trace()
        if trace:
            print(f"Latest trace: {trace}")
            stats = profiler.extract_trace_stats(trace)
            for key, value in stats.items():
                print(f"  {key}: {value}")
        else:
            print("No traces found")

    else:
        print(f"Unknown command: {command}")
        sys.exit(1)
