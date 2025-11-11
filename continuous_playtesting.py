#!/usr/bin/env python3
"""
Continuous Playtesting Orchestrator
Main system that integrates all components into an automated, self-running testing framework
"""

import json
import time
import logging
import signal
import sys
import argparse
import subprocess
from pathlib import Path
from typing import Dict, Any, List, Optional
from datetime import datetime
import threading
import requests
import psutil

# Import our components
from launch_game_desktop import GameLauncher
from capture_screenshot import ScreenshotCapture
from test_scenarios import AutomatedTestScenarios, TestStatus
from performance_collector import PerformanceCollector, PerformanceMetrics
from issue_reporter import IssueReporter, IssueReport
from data_manager import DataManager
from playtesting_dashboard import PlaytestingDashboard

# Configuration
CONFIG_FILE = Path("playtesting_config.json")
LOG_DIR = Path("automation_config/logs")
SESSIONS_DIR = Path("playtesting_data/sessions")

class ContinuousPlaytestingOrchestrator:
    """Main orchestrator for continuous playtesting"""
    
    def __init__(self, config_path: str = "playtesting_config.json"):
        self.config = self.load_config(config_path)
        self.running = False
        self.current_session_id = None
        self.current_iteration = 0
        self.consecutive_failures = 0
        
        # Components
        self.game_launcher: Optional[GameLauncher] = None
        self.screenshot_capture: Optional[ScreenshotCapture] = None
        self.test_scenarios: Optional[AutomatedTestScenarios] = None
        self.performance_collector: Optional[PerformanceCollector] = None
        self.issue_reporter: Optional[IssueReporter] = None
        self.data_manager: Optional[DataManager] = None
        self.dashboard: Optional[PlaytestingDashboard] = None
        
        # State tracking
        self.session_start_time = None
        self.test_results: List[Dict[str, Any]] = []
        self.performance_baseline: Optional[PerformanceMetrics] = None
        
        # Setup
        LOG_DIR.mkdir(parents=True, exist_ok=True)
        SESSIONS_DIR.mkdir(parents=True, exist_ok=True)
        self.setup_logging()
        
        self.log("Continuous playtesting orchestrator initialized")
        
    def setup_logging(self):
        """Setup logging configuration"""
        log_file = LOG_DIR / f"playtesting_{datetime.now().strftime('%Y%m%d_%H%M%S')}.log"
        
        logging.basicConfig(
            filename=log_file,
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )
        self.logger = logging.getLogger(__name__)
        
        # Also log to console
        console_handler = logging.StreamHandler()
        console_handler.setLevel(logging.INFO)
        self.logger.addHandler(console_handler)
        
    def log(self, message: str, level: str = "INFO"):
        """Log message"""
        if level == "INFO":
            self.logger.info(message)
        elif level == "WARNING":
            self.logger.warning(message)
        elif level == "ERROR":
            self.logger.error(message)
        elif level == "DEBUG":
            self.logger.debug(message)
            
        print(f"[{datetime.now().strftime('%H:%M:%S')}] {level}: {message}")
        
    def load_config(self, config_path: str) -> Dict[str, Any]:
        """Load configuration from file"""
        try:
            with open(config_path, 'r') as f:
                return json.load(f)
        except Exception as e:
            self.log(f"Error loading config: {e}, using defaults", "WARNING")
            return {}
            
    def generate_session_id(self) -> str:
        """Generate unique session ID"""
        return f"session_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
        
    def initialize_components(self):
        """Initialize all playtesting components"""
        try:
            self.log("Initializing playtesting components...")
            
            # Initialize game launcher
            self.game_launcher = GameLauncher()
            
            # Initialize screenshot capture
            screenshot_config = self.config.get("screenshot_capture", {})
            self.screenshot_capture = ScreenshotCapture(
                server_url="http://localhost:8080",
                capture_interval=screenshot_config.get("capture_interval_seconds", 3.0)
            )
            
            # Initialize test scenarios
            self.test_scenarios = AutomatedTestScenarios(server_url="http://localhost:8080")
            
            # Initialize performance collector
            perf_config = self.config.get("performance_collection", {})
            self.performance_collector = PerformanceCollector(
                server_url="http://localhost:8080",
                session_id=self.current_session_id
            )
            
            # Initialize issue reporter
            self.issue_reporter = IssueReporter(
                server_url="http://localhost:8080",
                analysis_server_url="http://localhost:8081"
            )
            
            # Initialize data manager
            self.data_manager = DataManager()
            
            # Initialize dashboard
            self.dashboard = PlaytestingDashboard()
            
            self.log("All components initialized successfully")
            
        except Exception as e:
            self.log(f"Error initializing components: {e}", "ERROR")
            raise
            
    def start_components(self):
        """Start all components"""
        try:
            self.log("Starting playtesting components...")
            
            # Start performance collector
            perf_config = self.config.get("performance_collection", {})
            self.performance_collector.start(
                interval=perf_config.get("collection_interval_seconds", 5.0)
            )
            
            # Start screenshot capture
            self.screenshot_capture.start()
            
            # Start issue reporter
            self.issue_reporter.start()
            
            # Start data manager
            self.data_manager.start()
            
            # Start dashboard in separate thread
            dashboard_thread = threading.Thread(
                target=self.dashboard.run,
                daemon=True
            )
            dashboard_thread.start()
            
            self.log("All components started")
            
        except Exception as e:
            self.log(f"Error starting components: {e}", "ERROR")
            raise
            
    def stop_components(self):
        """Stop all components gracefully"""
        self.log("Stopping playtesting components...")
        
        try:
            # Stop screenshot capture
            if self.screenshot_capture:
                self.screenshot_capture.stop()
                
            # Stop performance collector
            if self.performance_collector:
                self.performance_collector.stop()
                
            # Stop issue reporter
            if self.issue_reporter:
                self.issue_reporter.stop()
                
            # Stop data manager
            if self.data_manager:
                self.data_manager.stop()
                
            self.log("All components stopped")
            
        except Exception as e:
            self.log(f"Error stopping components: {e}", "ERROR")
            
    def launch_game(self) -> bool:
        """Launch the game"""
        try:
            self.log("Launching game...")
            
            # Parse launch arguments from config
            game_config = self.config.get("game_launcher", {})
            
            # Build launch arguments
            launch_args = [
                "python", "launch_game_desktop.py",
                "--resolution", game_config.get("default_resolution", "1280x720"),
                "--quality", game_config.get("default_quality", "medium"),
                "--interval", str(self.config.get("screenshot_capture", {}).get("capture_interval_seconds", 3)),
                "--port", str(game_config.get("http_api_port", 8080)),
                "--max-restarts", str(game_config.get("max_restarts", 3))
            ]
            
            if not game_config.get("enable_console", True):
                launch_args.append("--no-console")
                
            if not game_config.get("enable_debug", True):
                launch_args.append("--no-debug")
                
            # Launch game process
            process = subprocess.Popen(
                launch_args,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
            
            # Wait for game to be ready
            if not self.wait_for_game_ready(game_config.get("http_api_port", 8080)):
                self.log("Game failed to start properly", "ERROR")
                return False
                
            self.log("Game launched successfully")
            return True
            
        except Exception as e:
            self.log(f"Error launching game: {e}", "ERROR")
            return False
            
    def wait_for_game_ready(self, port: int, timeout: int = 180) -> bool:
        """Wait for game HTTP API to be ready"""
        self.log(f"Waiting for game HTTP API on port {port}...")
        
        start_time = time.time()
        while time.time() - start_time < timeout:
            try:
                response = requests.get(f"http://localhost:{port}/status", timeout=2)
                if response.status_code == 200:
                    elapsed = int(time.time() - start_time)
                    self.log(f"Game ready! (took {elapsed}s)")
                    return True
            except:
                pass
            time.sleep(2)
            
        self.log(f"Game failed to become ready within {timeout}s", "ERROR")
        return False
        
    def run_test_iteration(self) -> bool:
        """Run a single test iteration"""
        try:
            self.current_iteration += 1
            iteration_start = time.time()
            
            self.log(f"Starting iteration {self.current_iteration}")
            
            # Update dashboard
            if self.dashboard:
                self.dashboard.update_session_data(
                    self.current_session_id,
                    self.current_iteration
                )
            
            # Run test scenarios
            test_results = self.test_scenarios.run_all_scenarios()
            
            # Process results
            for result in test_results:
                result_data = {
                    "iteration": self.current_iteration,
                    "scenario": result.scenario_name,
                    "type": result.scenario_type.value,
                    "status": result.status.value,
                    "duration": result.duration,
                    "message": result.message,
                    "timestamp": datetime.now().isoformat()
                }
                
                self.test_results.append(result_data)
                
                # Update dashboard
                if self.dashboard:
                    self.dashboard.add_test_result(result_data)
                
                # Log result
                self.log(f"Test completed: {result.scenario_name} - {result.status.value}")
                
                # Check for failures
                if result.status in [TestStatus.FAILED, TestStatus.ERROR, TestStatus.TIMEOUT]:
                    self.consecutive_failures += 1
                    
                    # Detect issues from test failure
                    if self.issue_reporter:
                        issue_data = {
                            "issue_type": "test_failure",
                            "severity": "high",
                            "description": f"Test failed: {result.message}",
                            "game_state": self.test_scenarios.get_game_state(),
                            "location": f"test_scenario:{result.scenario_name}"
                        }
                        self.issue_reporter.queue_issue(issue_data)
                else:
                    self.consecutive_failures = 0
                    
            # Collect performance baseline on first iteration
            if self.current_iteration == 1:
                self.establish_performance_baseline()
                
            # Check performance regressions
            self.check_performance_regressions()
            
            # Detect issues from current state
            self.detect_issues()
            
            iteration_duration = time.time() - iteration_start
            self.log(f"Iteration {self.current_iteration} completed in {iteration_duration:.1f}s")
            
            return True
            
        except Exception as e:
            self.log(f"Error in test iteration: {e}", "ERROR")
            self.consecutive_failures += 1
            return False
            
    def establish_performance_baseline(self):
        """Establish performance baseline"""
        try:
            self.log("Establishing performance baseline...")
            
            # Get current metrics
            current_metrics = self.performance_collector.get_current_metrics()
            
            if current_metrics:
                self.performance_baseline = current_metrics
                self.performance_collector.set_baseline(current_metrics)
                self.log("Performance baseline established")
            else:
                self.log("Could not establish performance baseline", "WARNING")
                
        except Exception as e:
            self.log(f"Error establishing baseline: {e}", "WARNING")
            
    def check_performance_regressions(self):
        """Check for performance regressions"""
        try:
            if not self.performance_baseline:
                return
                
            current_metrics = self.performance_collector.get_current_metrics()
            
            if current_metrics:
                regressions = self.performance_collector.check_performance_regression(current_metrics)
                
                for regression in regressions:
                    self.log(f"Performance regression: {regression['message']}", "WARNING")
                    
                    # Create issue for regression
                    if self.issue_reporter:
                        issue_data = {
                            "issue_type": "performance_regression",
                            "severity": regression["severity"],
                            "description": regression["message"],
                            "game_state": self.test_scenarios.get_game_state(),
                            "performance_metrics": current_metrics.to_dict() if hasattr(current_metrics, 'to_dict') else current_metrics
                        }
                        self.issue_reporter.queue_issue(issue_data)
                        
        except Exception as e:
            self.log(f"Error checking performance regressions: {e}", "WARNING")
            
    def detect_issues(self):
        """Detect and report issues"""
        try:
            if not self.issue_reporter:
                return
                
            # Get current screenshot
            screenshot_data = None
            if self.screenshot_capture:
                stats = self.screenshot_capture.get_stats()
                if stats["screenshot_count"] > 0:
                    # In real implementation, would get actual screenshot data
                    pass
                    
            # Detect issues
            issues = self.issue_reporter.detect_issues(screenshot_data)
            
            # Process detected issues
            for issue in issues:
                self.issue_reporter.process_issue(issue, screenshot_data)
                
                # Update dashboard
                if self.dashboard:
                    self.dashboard.add_issues([issue.to_dict()])
                    
            if issues:
                self.log(f"Detected {len(issues)} issues")
                
        except Exception as e:
            self.log(f"Error detecting issues: {e}", "WARNING")
            
    def check_session_timeout(self) -> bool:
        """Check if session has timed out"""
        if not self.session_start_time:
            return False
            
        timeout_minutes = self.config.get("continuous_playtesting", {}).get("session_timeout_minutes", 60)
        elapsed_minutes = (time.time() - self.session_start_time) / 60
        
        if elapsed_minutes > timeout_minutes:
            self.log(f"Session timeout reached ({timeout_minutes} minutes)", "INFO")
            return True
            
        return False
        
    def check_consecutive_failures(self) -> bool:
        """Check if too many consecutive failures occurred"""
        max_failures = self.config.get("continuous_playtesting", {}).get("max_consecutive_failures", 3)
        
        if self.consecutive_failures >= max_failures:
            self.log(f"Too many consecutive failures ({self.consecutive_failures}), stopping", "ERROR")
            return True
            
        return False
        
    def save_session_data(self):
        """Save session data and generate reports"""
        try:
            self.log("Saving session data...")
            
            session_dir = SESSIONS_DIR / self.current_session_id
            session_dir.mkdir(exist_ok=True)
            
            # Save test results
            results_file = session_dir / "test_results.json"
            with open(results_file, 'w') as f:
                json.dump(self.test_results, f, indent=2)
                
            # Save performance report
            if self.performance_collector:
                perf_report = self.performance_collector.generate_performance_report()
                perf_file = session_dir / "performance_report.json"
                with open(perf_file, 'w') as f:
                    json.dump(perf_report, f, indent=2)
                    
            # Save issue summary
            if self.issue_reporter:
                issue_report = self.issue_reporter.generate_summary_report()
                issue_file = session_dir / "issue_report.json"
                with open(issue_file, 'w') as f:
                    json.dump(issue_report, f, indent=2)
                    
            # Save session summary
            session_summary = {
                "session_id": self.current_session_id,
                "start_time": datetime.fromtimestamp(self.session_start_time).isoformat(),
                "end_time": datetime.now().isoformat(),
                "duration_minutes": (time.time() - self.session_start_time) / 60,
                "total_iterations": self.current_iteration,
                "test_results": self.get_test_summary(),
                "performance_summary": self.performance_collector.generate_performance_report() if self.performance_collector else {},
                "issue_summary": self.issue_reporter.generate_summary_report() if self.issue_reporter else {}
            }
            
            summary_file = session_dir / "session_summary.json"
            with open(summary_file, 'w') as f:
                json.dump(session_summary, f, indent=2)
                
            self.log(f"Session data saved to {session_dir}")
            
        except Exception as e:
            self.log(f"Error saving session data: {e}", "ERROR")
            
    def get_test_summary(self) -> Dict[str, Any]:
        """Get test results summary"""
        if not self.test_results:
            return {"total": 0, "passed": 0, "failed": 0, "error": 0}
            
        summary = {
            "total": len(self.test_results),
            "passed": sum(1 for r in self.test_results if r['status'] == 'passed'),
            "failed": sum(1 for r in self.test_results if r['status'] in ['failed', 'timeout']),
            "error": sum(1 for r in self.test_results if r['status'] == 'error')
        }
        
        if summary["total"] > 0:
            summary["success_rate"] = summary["passed"] / summary["total"] * 100
        else:
            summary["success_rate"] = 0
            
        return summary
        
    def run_continuous_loop(self):
        """Main continuous playtesting loop"""
        self.log("=" * 70)
        self.log("CONTINUOUS PLAYTESTING STARTED")
        self.log("=" * 70)
        self.log(f"Session ID: {self.current_session_id}")
        self.log(f"Start Time: {datetime.now().isoformat()}")
        self.log("=" * 70)
        
        self.session_start_time = time.time()
        
        try:
            # Initialize components
            self.initialize_components()
            
            # Launch game
            if not self.launch_game():
                raise Exception("Failed to launch game")
                
            # Start components
            self.start_components()
            
            # Wait for initial stabilization
            self.log("Waiting for initial stabilization...")
            time.sleep(10)
            
            # Main loop
            while self.running:
                iteration_start = time.time()
                
                # Run test iteration
                if not self.run_test_iteration():
                    self.log("Test iteration failed", "WARNING")
                    
                # Check stopping conditions
                if self.check_session_timeout():
                    break
                    
                if self.check_consecutive_failures():
                    break
                    
                # Check max iterations
                max_iterations = self.config.get("continuous_playtesting", {}).get("max_iterations", 0)
                if max_iterations > 0 and self.current_iteration >= max_iterations:
                    self.log(f"Reached maximum iterations ({max_iterations})", "INFO")
                    break
                    
                # Delay between iterations
                iteration_delay = self.config.get("continuous_playtesting", {}).get("iteration_delay_seconds", 30)
                elapsed = time.time() - iteration_start
                
                if elapsed < iteration_delay:
                    sleep_time = iteration_delay - elapsed
                    self.log(f"Sleeping for {sleep_time:.1f}s before next iteration...")
                    time.sleep(sleep_time)
                    
        except KeyboardInterrupt:
            self.log("Shutdown requested by user", "INFO")
        except Exception as e:
            self.log(f"Error in continuous loop: {e}", "ERROR")
        finally:
            self.shutdown()
            
    def shutdown(self):
        """Graceful shutdown"""
        self.log("=" * 70)
        self.log("SHUTTING DOWN PLAYTESTING SYSTEM")
        self.log("=" * 70)
        
        # Stop main loop
        self.running = False
        
        # Save session data
        self.save_session_data()
        
        # Stop components
        self.stop_components()
        
        # Generate final report
        self.generate_final_report()
        
        self.log("Playtesting system shutdown complete")
        
    def generate_final_report(self):
        """Generate final playtesting report"""
        try:
            summary = self.get_test_summary()
            
            self.log("=" * 70)
            self.log("FINAL PLAYTESTING REPORT")
            self.log("=" * 70)
            self.log(f"Session ID: {self.current_session_id}")
            self.log(f"Total Iterations: {self.current_iteration}")
            self.log(f"Total Tests: {summary['total']}")
            self.log(f"Passed: {summary['passed']}")
            self.log(f"Failed: {summary['failed']}")
            self.log(f"Errors: {summary['error']}")
            self.log(f"Success Rate: {summary['success_rate']:.1f}%")
            self.log(f"Duration: {(time.time() - self.session_start_time) / 60:.1f} minutes")
            self.log("=" * 70)
            
            # Save final report
            report_file = SESSIONS_DIR / self.current_session_id / "final_report.txt"
            with open(report_file, 'w') as f:
                f.write("CONTINUOUS PLAYTESTING FINAL REPORT\n")
                f.write("=" * 70 + "\n")
                f.write(f"Session ID: {self.current_session_id}\n")
                f.write(f"Generated: {datetime.now().isoformat()}\n")
                f.write("=" * 70 + "\n\n")
                f.write(f"Total Iterations: {self.current_iteration}\n")
                f.write(f"Total Tests: {summary['total']}\n")
                f.write(f"Passed: {summary['passed']}\n")
                f.write(f"Failed: {summary['failed']}\n")
                f.write(f"Errors: {summary['error']}\n")
                f.write(f"Success Rate: {summary['success_rate']:.1f}%\n")
                f.write(f"Duration: {(time.time() - self.session_start_time) / 60:.1f} minutes\n")
                
            self.log(f"Final report saved: {report_file}")
            
        except Exception as e:
            self.log(f"Error generating final report: {e}", "ERROR")
            
    def run(self, max_iterations: int = 0):
        """Run the continuous playtesting system"""
        # Generate session ID
        self.current_session_id = self.generate_session_id()
        
        # Override max iterations if specified
        if max_iterations > 0:
            self.config["continuous_playtesting"]["max_iterations"] = max_iterations
            
        # Set up signal handlers
        signal.signal(signal.SIGINT, self.signal_handler)
        signal.signal(signal.SIGTERM, self.signal_handler)
        
        # Start the continuous loop
        self.running = True
        self.run_continuous_loop()
        
    def signal_handler(self, signum, frame):
        """Handle shutdown signals"""
        self.log(f"Received signal {signum}, shutting down...")
        self.running = False

def main():
    """Main function"""
    parser = argparse.ArgumentParser(description="Continuous Playtesting System")
    parser.add_argument("--config", type=str, default="playtesting_config.json",
                       help="Configuration file path")
    parser.add_argument("--iterations", type=int, default=0,
                       help="Maximum iterations (0 = unlimited)")
    parser.add_argument("--manual", action="store_true",
                       help="Manual mode - launch game without automated testing")
    
    args = parser.parse_args()
    
    print("=" * 70)
    print("CONTINUOUS PLAYTESTING SYSTEM")
    print("=" * 70)
    print(f"Config: {args.config}")
    print(f"Mode: {'Manual' if args.manual else 'Automated'}")
    if args.iterations > 0:
        print(f"Max Iterations: {args.iterations}")
    print("=" * 70)
    
    try:
        # Create orchestrator
        orchestrator = ContinuousPlaytestingOrchestrator(args.config)
        
        if args.manual:
            # Manual mode - just launch game and components
            orchestrator.log("Starting in manual mode...")
            orchestrator.current_session_id = orchestrator.generate_session_id()
            orchestrator.initialize_components()
            
            if orchestrator.launch_game():
                orchestrator.start_components()
                orchestrator.log("Game launched in manual mode. Press Ctrl+C to exit.")
                
                # Keep running until interrupted
                try:
                    while True:
                        time.sleep(1)
                except KeyboardInterrupt:
                    pass
                    
                orchestrator.shutdown()
            else:
                orchestrator.log("Failed to launch game", "ERROR")
        else:
            # Automated mode
            orchestrator.run(max_iterations=args.iterations)
            
    except KeyboardInterrupt:
        print("\nShutdown requested...")
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)
        
    print("=" * 70)
    print("PLAYTESTING COMPLETE")
    print("=" * 70)

if __name__ == "__main__":
    main()