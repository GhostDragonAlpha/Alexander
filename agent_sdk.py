#!/usr/bin/env python3
"""
Agent SDK for Alexander Test Server

Simple Python library for AI agents to control the test server.

Usage:
    from agent_sdk import TestServerController

    # Agent starts server
    server = TestServerController()
    server.start()

    # Agent uses server
    test_id = server.run_tests(["VR", "Flight"])
    results = server.wait_for_completion(test_id)

    # Agent stops server
    server.shutdown()
"""

import subprocess
import time
import sys
import json
from pathlib import Path
from typing import Dict, List, Optional
import requests


class TestServerController:
    """
    Controller for AI agents to manage the test server

    Provides simple interface:
    - start() → Launches server
    - run_tests() → Executes tests
    - get_results() → Retrieves results
    - shutdown() → Stops server
    """

    def __init__(self, port: int = 8000, timeout: int = 30):
        self.port = port
        self.base_url = f"http://localhost:{port}"
        self.timeout = timeout
        self.process: Optional[subprocess.Popen] = None
        self.server_ready = False

    def start(self, wait_for_ready: bool = True) -> bool:
        """
        Start the test server

        Args:
            wait_for_ready: Wait for server to be fully ready

        Returns:
            True if server started successfully
        """
        if self.is_running():
            print(f"Server already running on port {self.port}")
            return True

        print(f"Starting test server on port {self.port}...")

        # Launch server process
        server_script = Path(__file__).parent / "test_server.py"

        if not server_script.exists():
            raise FileNotFoundError(f"test_server.py not found at {server_script}")

        self.process = subprocess.Popen(
            [sys.executable, str(server_script)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1
        )

        # Wait for server to be ready
        if wait_for_ready:
            return self.wait_for_ready()

        return True

    def wait_for_ready(self, timeout: Optional[int] = None) -> bool:
        """
        Wait for server to be ready to accept requests

        Args:
            timeout: Maximum seconds to wait (default: self.timeout)

        Returns:
            True if server became ready
        """
        timeout = timeout or self.timeout
        start_time = time.time()

        print("Waiting for server to be ready...", end="", flush=True)

        while time.time() - start_time < timeout:
            try:
                response = requests.get(f"{self.base_url}/api/status", timeout=1)
                if response.status_code == 200:
                    self.server_ready = True
                    print(" Ready!")
                    return True
            except requests.exceptions.RequestException:
                pass

            time.sleep(0.5)
            print(".", end="", flush=True)

        print(" Timeout!")
        return False

    def is_running(self) -> bool:
        """Check if server is currently running"""
        try:
            response = requests.get(f"{self.base_url}/api/status", timeout=1)
            return response.status_code == 200
        except requests.exceptions.RequestException:
            return False

    def get_status(self) -> Dict:
        """
        Get current server status

        Returns:
            Complete status snapshot (JSON)
        """
        response = requests.get(f"{self.base_url}/api/status")
        response.raise_for_status()
        return response.json()

    def run_tests(
        self,
        stations: Optional[List[str]] = None,
        parallel: bool = True,
        timeout: int = 600,
        settings: Optional[Dict] = None
    ) -> str:
        """
        Execute tests

        Args:
            stations: List of station names (or None for all)
            parallel: Run tests in parallel
            timeout: Test execution timeout in seconds
            settings: Custom test settings

        Returns:
            test_id for tracking this test run
        """
        payload = {
            "stations": stations or [],
            "parallel": parallel,
            "timeout": timeout,
            "settings": settings or {}
        }

        response = requests.post(f"{self.base_url}/api/run_tests", json=payload)
        response.raise_for_status()

        result = response.json()
        test_id = result["test_id"]

        print(f"Test started: {test_id}")
        return test_id

    def get_test_results(self, test_id: str) -> Dict:
        """
        Get results for specific test

        Args:
            test_id: Test execution ID

        Returns:
            Complete test results (JSON)
        """
        response = requests.get(f"{self.base_url}/api/tests/{test_id}")
        response.raise_for_status()
        return response.json()

    def wait_for_completion(
        self,
        test_id: str,
        poll_interval: float = 2.0,
        timeout: Optional[int] = None
    ) -> Dict:
        """
        Wait for test to complete and return results

        Args:
            test_id: Test execution ID
            poll_interval: Seconds between status checks
            timeout: Maximum wait time (None = wait forever)

        Returns:
            Complete test results when finished
        """
        start_time = time.time()

        print(f"Waiting for test {test_id} to complete...", end="", flush=True)

        while True:
            results = self.get_test_results(test_id)
            status = results["status"]

            if status in ["completed", "failed", "timeout"]:
                print(f" {status.upper()}!")
                return results

            if timeout and (time.time() - start_time) > timeout:
                print(" TIMEOUT!")
                raise TimeoutError(f"Test did not complete within {timeout}s")

            time.sleep(poll_interval)
            print(".", end="", flush=True)

    def analyze_failures(self, test_id: str, focus: str = "failures") -> Dict:
        """
        Analyze test results

        Args:
            test_id: Test execution ID
            focus: Analysis focus ("failures", "performance", "all")

        Returns:
            Analysis with suggested fixes
        """
        payload = {
            "test_id": test_id,
            "focus": focus
        }

        response = requests.post(f"{self.base_url}/api/analyze", json=payload)
        response.raise_for_status()
        return response.json()

    def rerun_tests(self, tests: List[str], settings: Optional[Dict] = None) -> str:
        """
        Rerun specific tests with new settings

        Args:
            tests: List of test names to rerun
            settings: New settings to apply

        Returns:
            new test_id for the rerun
        """
        payload = {
            "tests": tests,
            "settings": settings or {}
        }

        response = requests.post(f"{self.base_url}/api/rerun", json=payload)
        response.raise_for_status()

        result = response.json()
        return result["test_id"]

    def get_performance_metrics(self) -> Dict:
        """
        Get aggregated performance metrics

        Returns:
            Performance trends and statistics
        """
        response = requests.get(f"{self.base_url}/api/performance")
        response.raise_for_status()
        return response.json()

    def shutdown(self, wait_for_exit: bool = True) -> bool:
        """
        Gracefully shutdown the server

        Args:
            wait_for_exit: Wait for server process to exit

        Returns:
            True if shutdown successful
        """
        print("Shutting down server...")

        try:
            requests.post(f"{self.base_url}/api/shutdown", timeout=2)
        except requests.exceptions.RequestException:
            pass  # Server shuts down, connection closed

        if wait_for_exit and self.process:
            try:
                self.process.wait(timeout=5)
                print("Server stopped.")
                return True
            except subprocess.TimeoutExpired:
                print("Server did not stop gracefully, forcing...")
                self.process.kill()
                return False

        return True

    def __enter__(self):
        """Context manager: start server"""
        self.start()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager: shutdown server"""
        self.shutdown()


# ===== Helper Functions =====

def quick_test(stations: Optional[List[str]] = None) -> Dict:
    """
    Quick helper for running tests

    Usage:
        results = quick_test(["VR", "Flight"])

    Returns:
        Complete test results
    """
    with TestServerController() as server:
        test_id = server.run_tests(stations)
        results = server.wait_for_completion(test_id)
        return results


def debug_test_failures(test_id: Optional[str] = None) -> Dict:
    """
    Debug test failures with analysis

    Usage:
        analysis = debug_test_failures(test_id)

    Returns:
        Analysis with suggested fixes
    """
    with TestServerController() as server:
        if not test_id:
            # Run new test
            test_id = server.run_tests()
            results = server.wait_for_completion(test_id)

            if results.get('failed', 0) == 0:
                print("All tests passed!")
                return results

        # Analyze failures
        analysis = server.analyze_failures(test_id)
        return analysis


# ===== Example Usage =====

if __name__ == "__main__":
    """
    Example: Agent runs tests and analyzes results
    """

    print("=" * 80)
    print("AGENT SDK EXAMPLE")
    print("=" * 80)

    # Using context manager (auto start/stop)
    with TestServerController() as server:
        # Check status
        status = server.get_status()
        print(f"\nServer state: {status['server_state']}")
        print(f"Available stations: {len(status['available_stations'])}")

        # Run tests
        print("\n1. Running VR and Flight tests...")
        test_id = server.run_tests(["VRTestStation", "SpaceshipFlightTestStation"])

        # Wait for completion
        print("\n2. Waiting for completion...")
        results = server.wait_for_completion(test_id)

        # Display results
        print(f"\n3. Results:")
        print(f"   Total tests: {results['results']['total_tests']}")
        print(f"   Passed: {results['results']['passed']}")
        print(f"   Failed: {results['results']['failed']}")
        print(f"   Pass rate: {results['results']['pass_rate']}%")

        # Analyze if failures
        if results['results']['failed'] > 0:
            print("\n4. Analyzing failures...")
            analysis = server.analyze_failures(test_id)

            for failure in analysis.get('failures', []):
                print(f"\n   Failed test: {failure['test']}")
                print(f"   Error: {failure['error']}")

                if failure.get('can_auto_fix'):
                    print(f"   Suggested fix: {failure['suggested_fix']}")

        print("\n5. Getting performance metrics...")
        perf = server.get_performance_metrics()
        print(f"   Recent tests: {len(perf['recent_tests'])}")

    print("\nServer automatically stopped (context manager)")
    print("=" * 80)
