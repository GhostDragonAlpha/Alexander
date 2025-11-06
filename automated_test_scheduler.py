#!/usr/bin/env python3
"""
Automated Test Scheduler for Alexander Comprehensive Test Level

Runs tests at specified intervals with zero human intervention
Deploy on dedicated test server for continuous validation

Features:
- Full test suite twice daily (2 AM, 2 PM)
- Smoke tests every 4 hours
- Performance tests every 2 hours
- Automated result analysis
- Intelligent alerting (only for critical issues)
- Performance regression detection
- Auto-generated dashboards

Usage:
    python automated_test_scheduler.py [--config config.json]

Deploy as service:
    # Windows
    nssm install AlexanderTestScheduler "python" "C:\path\to\automated_test_scheduler.py"

    # Linux (systemd)
    sudo systemctl enable alexander-test-scheduler.service
"""

import schedule
import subprocess
import datetime
import json
import time
import logging
import sys
from pathlib import Path
from typing import Dict, List, Optional
import smtplib
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('test_scheduler.log'),
        logging.StreamHandler(sys.stdout)
    ]
)

logger = logging.getLogger(__name__)


class AutomatedTestScheduler:
    """Automated test scheduler with intelligent alerting"""

    def __init__(self, config_file: Optional[str] = None):
        self.results_dir = Path("TestResults")
        self.results_dir.mkdir(exist_ok=True)

        self.config = self.load_config(config_file)
        self.alert_history: List[Dict] = []
        self.baseline_metrics = self.load_baseline()

    def load_config(self, config_file: Optional[str]) -> Dict:
        """Load configuration from file or use defaults"""
        default_config = {
            "project_path": "C:/Users/allen/Desktop/Alexander/Alexander",
            "unreal_editor": "C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe",
            "python_runner": "run_comprehensive_test_level.py",
            "alert_email": "",
            "alert_slack_webhook": "",
            "alert_threshold": {
                "pass_rate": 80.0,
                "min_fps": 60.0,
                "max_memory_mb": 4096
            },
            "schedules": {
                "full_suite_times": ["02:00", "14:00"],
                "smoke_test_interval_hours": 4,
                "performance_test_interval_hours": 2
            }
        }

        if config_file and Path(config_file).exists():
            with open(config_file) as f:
                user_config = json.load(f)
                default_config.update(user_config)

        return default_config

    def load_baseline(self) -> Dict:
        """Load baseline performance metrics"""
        baseline_file = Path("baseline.json")
        if baseline_file.exists():
            with open(baseline_file) as f:
                return json.load(f)
        return {}

    def run_full_suite(self):
        """Run complete test suite (60-90 minutes)"""
        logger.info("=" * 80)
        logger.info("STARTING FULL TEST SUITE")
        logger.info("=" * 80)

        start_time = datetime.datetime.now()

        try:
            # Run comprehensive tests
            result = subprocess.run([
                sys.executable,
                self.config["python_runner"],
                "--all",
                "--headless",
                "--report-format", "json,html,csv",
                "--output-dir", str(self.results_dir),
                "--timeout", "5400"  # 90 minutes
            ], capture_output=True, text=True, timeout=6000)

            duration = (datetime.datetime.now() - start_time).total_seconds()
            logger.info(f"Full test suite completed in {duration:.1f}s")

            # Analyze results
            self.analyze_results()

            # Generate dashboard
            self.generate_dashboard()

            # Check if alerting needed
            if result.returncode != 0:
                self.evaluate_alert_necessity()

            return result.returncode

        except subprocess.TimeoutExpired:
            logger.error("Full test suite timed out after 100 minutes!")
            self.send_critical_alert("Test suite timeout - investigation required")
            return 2

        except Exception as e:
            logger.exception(f"Error running full test suite: {e}")
            self.send_critical_alert(f"Test execution error: {str(e)}")
            return 2

    def run_smoke_tests(self):
        """Quick smoke tests (15 minutes)"""
        logger.info("Running smoke tests...")

        try:
            result = subprocess.run([
                sys.executable,
                self.config["python_runner"],
                "--station", "VRTestStation",
                "--station", "PerformanceBenchmarkStation",
                "--timeout", "900"
            ], capture_output=True, text=True, timeout=1200)

            logger.info(f"Smoke tests completed with return code: {result.returncode}")

            if result.returncode != 0:
                logger.warning("Smoke tests failed - flagging for full test run")

            return result.returncode

        except Exception as e:
            logger.exception(f"Error running smoke tests: {e}")
            return 2

    def run_performance_tests(self):
        """Performance-only tests (30 minutes)"""
        logger.info("Running performance tests...")

        try:
            result = subprocess.run([
                sys.executable,
                self.config["python_runner"],
                "--station", "PerformanceBenchmarkStation",
                "--timeout", "1800"
            ], capture_output=True, text=True, timeout=2100)

            logger.info(f"Performance tests completed with return code: {result.returncode}")

            # Check for performance regressions
            self.check_performance_regression()

            return result.returncode

        except Exception as e:
            logger.exception(f"Error running performance tests: {e}")
            return 2

    def analyze_results(self):
        """Automated result analysis"""
        try:
            # Find latest results
            latest_files = sorted(self.results_dir.glob("TestResults_*.json"))
            if not latest_files:
                logger.warning("No test results found for analysis")
                return

            latest = latest_files[-1]
            logger.info(f"Analyzing results from: {latest}")

            with open(latest) as f:
                results = json.load(f)

            # Log key metrics
            logger.info(f"Total Tests: {results.get('totalTests', 0)}")
            logger.info(f"Passed: {results.get('passedTests', 0)}")
            logger.info(f"Failed: {results.get('failedTests', 0)}")
            logger.info(f"Pass Rate: {results.get('passRate', 0):.1f}%")

            # Check for regressions against baseline
            if self.baseline_metrics:
                self.compare_to_baseline(results)

        except Exception as e:
            logger.exception(f"Error analyzing results: {e}")

    def compare_to_baseline(self, current_results: Dict):
        """Compare current results against baseline"""
        logger.info("Comparing against baseline...")

        baseline_pass_rate = self.baseline_metrics.get("passRate", 0)
        current_pass_rate = current_results.get("passRate", 0)

        if current_pass_rate < baseline_pass_rate - 5.0:
            logger.warning(f"Pass rate regression detected: {current_pass_rate:.1f}% vs baseline {baseline_pass_rate:.1f}%")
            self.alert_history.append({
                "timestamp": datetime.datetime.now().isoformat(),
                "type": "regression",
                "metric": "pass_rate",
                "current": current_pass_rate,
                "baseline": baseline_pass_rate
            })

    def check_performance_regression(self):
        """Check for performance regressions"""
        logger.info("Checking for performance regressions...")

        try:
            latest_files = sorted(self.results_dir.glob("TestResults_*.json"))
            if not latest_files:
                return

            with open(latest_files[-1]) as f:
                results = json.load(f)

            # Extract performance metrics
            stations = results.get("stations", [])
            perf_station = next((s for s in stations if "Performance" in s.get("name", "")), None)

            if not perf_station:
                return

            # Check against baseline
            baseline_fps = self.baseline_metrics.get("averageFPS", 120)
            current_fps = perf_station.get("averageFPS", 0)

            if current_fps < baseline_fps * 0.9:  # 10% regression
                logger.warning(f"FPS regression: {current_fps:.1f} vs baseline {baseline_fps:.1f}")
                self.alert_history.append({
                    "timestamp": datetime.datetime.now().isoformat(),
                    "type": "performance_regression",
                    "metric": "fps",
                    "current": current_fps,
                    "baseline": baseline_fps,
                    "degradation_percent": ((baseline_fps - current_fps) / baseline_fps) * 100
                })

        except Exception as e:
            logger.exception(f"Error checking performance regression: {e}")

    def generate_dashboard(self):
        """Generate HTML dashboard"""
        logger.info("Generating test dashboard...")

        try:
            result = subprocess.run([
                sys.executable,
                "generate_dashboard.py",
                "--input", str(self.results_dir),
                "--output", "test_dashboard.html"
            ], capture_output=True, text=True, timeout=60)

            if result.returncode == 0:
                logger.info("Dashboard generated successfully: test_dashboard.html")
            else:
                logger.warning(f"Dashboard generation failed: {result.stderr}")

        except Exception as e:
            logger.exception(f"Error generating dashboard: {e}")

    def evaluate_alert_necessity(self):
        """Determine if humans need to be alerted"""
        try:
            # Load latest results
            latest_files = sorted(self.results_dir.glob("TestResults_*.json"))
            if not latest_files:
                return

            with open(latest_files[-1]) as f:
                results = json.load(f)

            # Check critical conditions
            critical_conditions = []

            pass_rate = results.get("passRate", 100)
            if pass_rate < self.config["alert_threshold"]["pass_rate"]:
                critical_conditions.append(f"Pass rate below threshold: {pass_rate:.1f}%")

            # Check if we should alert
            if critical_conditions:
                self.send_alert(critical_conditions, results)
            else:
                logger.info("No critical issues detected - no human alert needed")

        except Exception as e:
            logger.exception(f"Error evaluating alert necessity: {e}")

    def send_alert(self, conditions: List[str], results: Dict):
        """Send alert to configured channels"""
        logger.warning("CRITICAL ISSUES DETECTED - ALERTING HUMANS")

        alert_message = f"""
AUTOMATED TEST ALERT - Human Review Required

Critical Conditions:
{chr(10).join(f"  - {c}" for c in conditions)}

Test Summary:
  Total Tests: {results.get('totalTests', 0)}
  Passed: {results.get('passedTests', 0)}
  Failed: {results.get('failedTests', 0)}
  Pass Rate: {results.get('passRate', 0):.1f}%

Dashboard: test_dashboard.html
Time: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}

Action Required: Review test results and investigate failures.
"""

        logger.warning(alert_message)

        # Send to Slack if configured
        if self.config.get("alert_slack_webhook"):
            self.send_slack_alert(alert_message)

        # Send email if configured
        if self.config.get("alert_email"):
            self.send_email_alert(alert_message)

    def send_critical_alert(self, message: str):
        """Send critical alert (timeout, crash, etc.)"""
        logger.critical(f"CRITICAL ALERT: {message}")

        alert_message = f"""
CRITICAL TEST SYSTEM ALERT

{message}

Time: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}

Immediate action required!
"""

        if self.config.get("alert_slack_webhook"):
            self.send_slack_alert(alert_message)

        if self.config.get("alert_email"):
            self.send_email_alert(alert_message)

    def send_slack_alert(self, message: str):
        """Send alert to Slack"""
        try:
            import requests
            webhook_url = self.config["alert_slack_webhook"]

            payload = {
                "text": message,
                "username": "Alexander Test Bot",
                "icon_emoji": ":warning:"
            }

            response = requests.post(webhook_url, json=payload)
            if response.status_code == 200:
                logger.info("Slack alert sent successfully")
            else:
                logger.warning(f"Failed to send Slack alert: {response.status_code}")

        except ImportError:
            logger.warning("requests library not available for Slack alerts")
        except Exception as e:
            logger.exception(f"Error sending Slack alert: {e}")

    def send_email_alert(self, message: str):
        """Send alert via email"""
        try:
            # This is a placeholder - configure SMTP settings
            logger.info(f"Email alert would be sent to: {self.config['alert_email']}")
            # Actual email implementation would go here

        except Exception as e:
            logger.exception(f"Error sending email alert: {e}")

    def get_status_report(self) -> Dict:
        """Get current status report"""
        return {
            "scheduler_uptime": "Active",
            "last_full_suite": self.get_last_run_time("full"),
            "last_smoke_test": self.get_last_run_time("smoke"),
            "last_performance_test": self.get_last_run_time("performance"),
            "pending_alerts": len(self.alert_history),
            "results_directory": str(self.results_dir.absolute())
        }

    def get_last_run_time(self, test_type: str) -> str:
        """Get last run time for test type"""
        # This would track last run times
        return "Not run yet"


def main():
    """Main scheduler loop"""
    logger.info("=" * 80)
    logger.info("ALEXANDER AUTOMATED TEST SCHEDULER STARTING")
    logger.info("=" * 80)

    # Load config
    config_file = sys.argv[1] if len(sys.argv) > 1 else None
    scheduler = AutomatedTestScheduler(config_file)

    # Log configuration
    logger.info(f"Project Path: {scheduler.config['project_path']}")
    logger.info(f"Results Directory: {scheduler.results_dir.absolute()}")
    logger.info("Schedules:")
    logger.info(f"  Full Suite: {scheduler.config['schedules']['full_suite_times']}")
    logger.info(f"  Smoke Tests: Every {scheduler.config['schedules']['smoke_test_interval_hours']} hours")
    logger.info(f"  Performance Tests: Every {scheduler.config['schedules']['performance_test_interval_hours']} hours")

    # Schedule automated runs
    for time_str in scheduler.config['schedules']['full_suite_times']:
        schedule.every().day.at(time_str).do(scheduler.run_full_suite)
        logger.info(f"Scheduled full test suite at {time_str}")

    # Smoke tests every N hours
    smoke_interval = scheduler.config['schedules']['smoke_test_interval_hours']
    schedule.every(smoke_interval).hours.do(scheduler.run_smoke_tests)
    logger.info(f"Scheduled smoke tests every {smoke_interval} hours")

    # Performance tests every N hours
    perf_interval = scheduler.config['schedules']['performance_test_interval_hours']
    schedule.every(perf_interval).hours.do(scheduler.run_performance_tests)
    logger.info(f"Scheduled performance tests every {perf_interval} hours")

    logger.info("=" * 80)
    logger.info("SCHEDULER ACTIVE - Running continuously")
    logger.info("Press Ctrl+C to stop")
    logger.info("=" * 80)

    # Run forever
    try:
        while True:
            schedule.run_pending()
            time.sleep(60)  # Check every minute

    except KeyboardInterrupt:
        logger.info("Scheduler stopped by user")
    except Exception as e:
        logger.exception(f"Scheduler error: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
