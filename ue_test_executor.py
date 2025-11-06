#!/usr/bin/env python3
"""
Unreal Engine Test Executor

Launches UE Editor/Game and executes C++ test commands.
Converts UE log output to JSON for agent consumption.

Uses your hardware:
- 24 cores for parallel execution
- 128GB RAM for multiple instances
- RTX 4090 for fast rendering
"""

import subprocess
import asyncio
import json
import re
import time
from pathlib import Path
from typing import Dict, List, Optional
from datetime import datetime
import logging

logger = logging.getLogger(__name__)


class UETestExecutor:
    """
    Executes tests in Unreal Engine and returns JSON results

    Wraps UE Editor/Game execution and parses log output
    """

    def __init__(self):
        self.project_path = Path(__file__).parent
        self.ue_editor = self._find_ue_editor()
        self.running_processes: List[subprocess.Popen] = []

    def _find_ue_editor(self) -> Optional[Path]:
        """Find Unreal Engine editor executable"""
        # Common UE5.6 installation paths
        possible_paths = [
            Path("C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"),
            Path("C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor.exe"),
            Path("C:/Program Files/Unreal Engine/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"),
        ]

        for path in possible_paths:
            if path.exists():
                logger.info(f"Found UE Editor: {path}")
                return path

        logger.warning("UE Editor not found in default locations")
        return None

    async def run_tests(
        self,
        stations: List[str],
        parallel: bool = True,
        timeout: int = 600,
        settings: Dict = {}
    ) -> Dict:
        """
        Execute tests in Unreal Engine

        Args:
            stations: List of test station names
            parallel: Run stations in parallel (uses multiple cores)
            timeout: Execution timeout in seconds
            settings: Custom test settings

        Returns:
            Test results as JSON
        """
        if not self.ue_editor:
            logger.error("Cannot run tests: UE Editor not found")
            return self._generate_error_result("UE Editor not found")

        start_time = datetime.now()

        try:
            if parallel and len(stations) > 1:
                # Run stations in parallel (utilize 24 cores)
                results = await self._run_parallel(stations, timeout, settings)
            else:
                # Run sequentially
                results = await self._run_sequential(stations, timeout, settings)

            # Aggregate results
            aggregated = self._aggregate_results(results)
            aggregated['duration_seconds'] = (datetime.now() - start_time).total_seconds()

            return aggregated

        except asyncio.TimeoutError:
            logger.error(f"Test execution timed out after {timeout}s")
            return self._generate_error_result(f"Timeout after {timeout}s")

        except Exception as e:
            logger.exception(f"Test execution error: {e}")
            return self._generate_error_result(str(e))

    async def _run_parallel(self, stations: List[str], timeout: int, settings: Dict) -> List[Dict]:
        """Run multiple stations in parallel"""
        logger.info(f"Running {len(stations)} stations in parallel")

        tasks = [
            self._run_station(station, timeout, settings)
            for station in stations
        ]

        results = await asyncio.gather(*tasks, return_exceptions=True)

        # Convert exceptions to error results
        processed = []
        for i, result in enumerate(results):
            if isinstance(result, Exception):
                logger.error(f"Station {stations[i]} failed: {result}")
                processed.append(self._generate_station_error(stations[i], str(result)))
            else:
                processed.append(result)

        return processed

    async def _run_sequential(self, stations: List[str], timeout: int, settings: Dict) -> List[Dict]:
        """Run stations one at a time"""
        logger.info(f"Running {len(stations)} stations sequentially")

        results = []
        for station in stations:
            try:
                result = await self._run_station(station, timeout, settings)
                results.append(result)
            except Exception as e:
                logger.error(f"Station {station} failed: {e}")
                results.append(self._generate_station_error(station, str(e)))

        return results

    async def _run_station(self, station: str, timeout: int, settings: Dict) -> Dict:
        """
        Run a single test station

        Executes:
            UnrealEditor-Cmd.exe Alexander.uproject -game -ExecCmds="TestLevel.RunStation VRTestStation"
        """
        logger.info(f"Executing station: {station}")

        # Build command
        project_file = self.project_path / "Alexander.uproject"

        if not project_file.exists():
            raise FileNotFoundError(f"Project file not found: {project_file}")

        # Construct UE command
        exec_cmd = f"TestLevel.RunStation {station}"

        command = [
            str(self.ue_editor),
            str(project_file),
            "-game",
            "-headless",  # No rendering (faster)
            f"-ExecCmds={exec_cmd}",
            "-log",
            "-stdout",
            "-FullStdOutLogOutput"
        ]

        logger.debug(f"Command: {' '.join(command)}")

        # Execute
        start_time = time.time()

        process = await asyncio.create_subprocess_exec(
            *command,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE,
            cwd=str(self.project_path)
        )

        self.running_processes.append(process)

        try:
            # Wait for completion with timeout
            stdout, stderr = await asyncio.wait_for(
                process.communicate(),
                timeout=timeout
            )

            duration = time.time() - start_time

            # Parse output
            log_output = stdout.decode('utf-8', errors='ignore')
            result = self._parse_ue_log(station, log_output, duration)

            return result

        except asyncio.TimeoutError:
            logger.warning(f"Station {station} timed out, killing process")
            process.kill()
            await process.wait()
            raise

        finally:
            if process in self.running_processes:
                self.running_processes.remove(process)

    def _parse_ue_log(self, station: str, log_output: str, duration: float) -> Dict:
        """
        Parse Unreal Engine log output to extract test results

        Looks for patterns like:
            LogTemp: [TestStation] Total Tests: 15
            LogTemp: [TestStation] Passed: 14
            LogTemp: [TestStation] Failed: 1
        """
        result = {
            "station": station,
            "status": "completed",
            "duration_seconds": duration,
            "total_tests": 0,
            "passed": 0,
            "failed": 0,
            "tests": []
        }

        # Extract test counts
        total_match = re.search(r'Total Tests:\s*(\d+)', log_output)
        passed_match = re.search(r'Passed:\s*(\d+)', log_output)
        failed_match = re.search(r'Failed:\s*(\d+)', log_output)

        if total_match:
            result['total_tests'] = int(total_match.group(1))
        if passed_match:
            result['passed'] = int(passed_match.group(1))
        if failed_match:
            result['failed'] = int(failed_match.group(1))

        # Extract individual test results
        test_pattern = r'\[(PASS|FAIL)\]\s+([\w_]+)(?::\s*(.+))?'
        for match in re.finditer(test_pattern, log_output):
            status, test_name, message = match.groups()
            result['tests'].append({
                "name": test_name,
                "status": status.lower(),
                "message": message or ""
            })

        # Extract performance metrics
        fps_match = re.search(r'Average FPS:\s*([\d.]+)', log_output)
        memory_match = re.search(r'Memory Usage:\s*([\d.]+)\s*MB', log_output)

        if fps_match or memory_match:
            result['performance'] = {}
            if fps_match:
                result['performance']['avg_fps'] = float(fps_match.group(1))
            if memory_match:
                result['performance']['memory_mb'] = float(memory_match.group(1))

        logger.info(f"Station {station}: {result['passed']}/{result['total_tests']} passed")

        return result

    def _aggregate_results(self, station_results: List[Dict]) -> Dict:
        """Aggregate results from multiple stations"""
        total_tests = sum(r.get('total_tests', 0) for r in station_results)
        total_passed = sum(r.get('passed', 0) for r in station_results)
        total_failed = sum(r.get('failed', 0) for r in station_results)

        pass_rate = (total_passed / total_tests * 100) if total_tests > 0 else 0

        # Aggregate performance metrics
        fps_values = [r.get('performance', {}).get('avg_fps', 0)
                     for r in station_results if 'performance' in r]
        memory_values = [r.get('performance', {}).get('memory_mb', 0)
                        for r in station_results if 'performance' in r]

        performance = {}
        if fps_values:
            performance['avg_fps'] = sum(fps_values) / len(fps_values)
            performance['min_fps'] = min(fps_values)
            performance['max_fps'] = max(fps_values)
            performance['meets_vr_target'] = min(fps_values) >= 90

        if memory_values:
            performance['avg_memory_mb'] = sum(memory_values) / len(memory_values)
            performance['max_memory_mb'] = max(memory_values)

        return {
            "status": "completed",
            "total_tests": total_tests,
            "passed": total_passed,
            "failed": total_failed,
            "pass_rate": round(pass_rate, 2),
            "stations": station_results,
            "performance": performance
        }

    def _generate_error_result(self, error_message: str) -> Dict:
        """Generate error result"""
        return {
            "status": "failed",
            "error": error_message,
            "total_tests": 0,
            "passed": 0,
            "failed": 0,
            "pass_rate": 0
        }

    def _generate_station_error(self, station: str, error_message: str) -> Dict:
        """Generate error result for a station"""
        return {
            "station": station,
            "status": "failed",
            "error": error_message,
            "total_tests": 0,
            "passed": 0,
            "failed": 0
        }

    def kill_all_processes(self):
        """Kill all running UE processes"""
        for process in self.running_processes:
            try:
                process.kill()
            except:
                pass
        self.running_processes.clear()


# ===== Testing =====

async def test_executor():
    """Test the UE executor"""
    executor = UETestExecutor()

    print("Testing UE Test Executor...")
    print(f"UE Editor: {executor.ue_editor}")

    # Test with mock stations
    results = await executor.run_tests(
        stations=["VRTestStation", "SpaceshipFlightTestStation"],
        parallel=True,
        timeout=300
    )

    print("\nResults:")
    print(json.dumps(results, indent=2))


if __name__ == "__main__":
    asyncio.run(test_executor())
