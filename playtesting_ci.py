#!/usr/bin/env python3
"""
Continuous Integration Module for Playtesting
Integrates with Git to automatically run playtests on code changes
"""

import json
import time
import logging
import subprocess
from pathlib import Path
from typing import Dict, Any, List, Optional
from datetime import datetime
import requests
import threading

# Configuration
CI_CONFIG_FILE = Path("playtesting_config.json")
CI_LOG_FILE = Path("automation_config/ci_integration.log")
CI_RESULTS_DIR = Path("playtesting_data/ci_results")

class PlaytestingCI:
    """Continuous Integration for automated playtesting"""
    
    def __init__(self, config_path: str = "playtesting_config.json"):
        self.config = self.load_config(config_path)
        self.running = False
        self.monitor_thread: Optional[threading.Thread] = None
        
        # Git configuration
        ci_config = self.config.get("ci_integration", {})
        self.git_repo_path = ci_config.get("git_repository_path", "")
        self.run_on_commit = ci_config.get("run_on_commit", True)
        self.run_on_pull_request = ci_config.get("run_on_pull_request", True)
        self.fail_on_critical = ci_config.get("fail_build_on_critical_issues", True)
        self.baseline_branch = ci_config.get("baseline_comparison_branch", "main")
        
        # State tracking
        self.last_commit_hash = None
        self.ci_results: List[Dict[str, Any]] = []
        
        # Setup
        CI_RESULTS_DIR.mkdir(parents=True, exist_ok=True)
        self.setup_logging()
        
        self.log("Playtesting CI initialized")
        
    def setup_logging(self):
        """Setup logging configuration"""
        CI_LOG_FILE.parent.mkdir(exist_ok=True)
        
        logging.basicConfig(
            filename=CI_LOG_FILE,
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )
        self.logger = logging.getLogger(__name__)
        
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
            
        print(f"[CI] {level}: {message}")
        
    def load_config(self, config_path: str) -> Dict[str, Any]:
        """Load configuration from file"""
        try:
            with open(config_path, 'r') as f:
                return json.load(f)
        except Exception as e:
            self.log(f"Error loading config: {e}, using defaults", "WARNING")
            return {
                "ci_integration": {
                    "enabled": False,
                    "git_repository_path": "",
                    "run_on_commit": True,
                    "run_on_pull_request": True,
                    "fail_build_on_critical_issues": True,
                    "generate_reports_for_pr": True,
                    "baseline_comparison_branch": "main"
                }
            }
            
    def is_git_available(self) -> bool:
        """Check if git is available and repository is accessible"""
        try:
            if not self.git_repo_path:
                return False
                
            result = subprocess.run(
                ["git", "-C", self.git_repo_path, "status"],
                capture_output=True,
                text=True,
                timeout=10
            )
            
            return result.returncode == 0
            
        except Exception as e:
            self.log(f"Git not available: {e}", "WARNING")
            return False
            
    def get_current_commit_hash(self) -> Optional[str]:
        """Get current commit hash"""
        try:
            result = subprocess.run(
                ["git", "-C", self.git_repo_path, "rev-parse", "HEAD"],
                capture_output=True,
                text=True,
                timeout=10
            )
            
            if result.returncode == 0:
                return result.stdout.strip()
            else:
                self.log(f"Error getting commit hash: {result.stderr}", "ERROR")
                return None
                
        except Exception as e:
            self.log(f"Error getting commit hash: {e}", "ERROR")
            return None
            
    def get_commit_info(self, commit_hash: str) -> Optional[Dict[str, Any]]:
        """Get information about a specific commit"""
        try:
            # Get commit message
            msg_result = subprocess.run(
                ["git", "-C", self.git_repo_path, "log", "-1", "--pretty=format:%s", commit_hash],
                capture_output=True,
                text=True,
                timeout=10
            )
            
            # Get changed files
            files_result = subprocess.run(
                ["git", "-C", self.git_repo_path, "diff-tree", "--no-commit-id", "--name-only", "-r", commit_hash],
                capture_output=True,
                text=True,
                timeout=10
            )
            
            if msg_result.returncode == 0:
                return {
                    "hash": commit_hash,
                    "message": msg_result.stdout.strip(),
                    "changed_files": files_result.stdout.strip().split('\n') if files_result.returncode == 0 else [],
                    "timestamp": datetime.now().isoformat()
                }
            else:
                self.log(f"Error getting commit info: {msg_result.stderr}", "ERROR")
                return None
                
        except Exception as e:
            self.log(f"Error getting commit info: {e}", "ERROR")
            return None
            
    def check_for_new_commits(self) -> bool:
        """Check if there are new commits since last check"""
        try:
            current_hash = self.get_current_commit_hash()
            
            if not current_hash:
                return False
                
            if current_hash != self.last_commit_hash:
                self.log(f"New commit detected: {current_hash[:8]}")
                self.last_commit_hash = current_hash
                return True
                
            return False
            
        except Exception as e:
            self.log(f"Error checking for new commits: {e}", "ERROR")
            return False
            
    def run_playtesting(self, commit_info: Dict[str, Any]) -> Dict[str, Any]:
        """Run playtesting for a commit"""
        try:
            self.log(f"Starting playtesting for commit {commit_info['hash'][:8]}")
            
            # Import the continuous playtesting orchestrator
            from continuous_playtesting import ContinuousPlaytestingOrchestrator
            
            # Create orchestrator
            orchestrator = ContinuousPlaytestingOrchestrator()
            
            # Generate CI-specific session ID
            session_id = f"ci_{commit_info['hash'][:8]}_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
            orchestrator.current_session_id = session_id
            
            # Run limited playtesting (fewer iterations for CI)
            orchestrator.config["continuous_playtesting"]["max_iterations"] = 2  # Quick CI test
            
            # Run playtesting
            start_time = time.time()
            
            try:
                orchestrator.initialize_components()
                
                if orchestrator.launch_game():
                    orchestrator.start_components()
                    
                    # Run a few iterations
                    for i in range(2):  # Quick test
                        if not orchestrator.run_test_iteration():
                            break
                        time.sleep(30)  # Short delay between iterations
                        
                    orchestrator.stop_components()
                    success = True
                else:
                    success = False
                    
            except Exception as e:
                self.log(f"Error during playtesting: {e}", "ERROR")
                success = False
                
            duration = time.time() - start_time
            
            # Generate results
            results = {
                "commit_hash": commit_info["hash"],
                "commit_message": commit_info["message"],
                "session_id": session_id,
                "success": success,
                "duration_seconds": duration,
                "timestamp": datetime.now().isoformat(),
                "test_summary": orchestrator.get_test_summary() if hasattr(orchestrator, 'get_test_summary') else {},
                "issues_detected": len(orchestrator.issue_reporter.known_issues) if orchestrator.issue_reporter else 0
            }
            
            # Check for critical issues
            if orchestrator.issue_reporter:
                critical_issues = sum(1 for issue in orchestrator.issue_reporter.known_issues.values() 
                                    if issue.severity == "critical")
                results["critical_issues"] = critical_issues
                
                # Determine pass/fail
                if self.fail_on_critical and critical_issues > 0:
                    results["build_status"] = "failed"
                    results["failure_reason"] = f"{critical_issues} critical issues detected"
                elif not success:
                    results["build_status"] = "failed"
                    results["failure_reason"] = "Playtesting execution failed"
                else:
                    results["build_status"] = "passed"
                    
            self.log(f"Playtesting completed: {results['build_status']}")
            
            return results
            
        except Exception as e:
            self.log(f"Error running playtesting: {e}", "ERROR")
            return {
                "commit_hash": commit_info["hash"],
                "success": False,
                "error": str(e),
                "timestamp": datetime.now().isoformat()
            }
            
    def save_ci_results(self, results: Dict[str, Any]):
        """Save CI results"""
        try:
            # Save to JSON file
            result_file = CI_RESULTS_DIR / f"ci_result_{results['session_id']}.json"
            with open(result_file, 'w') as f:
                json.dump(results, f, indent=2)
                
            # Append to results list
            self.ci_results.append(results)
            
            self.log(f"CI results saved: {result_file}")
            
        except Exception as e:
            self.log(f"Error saving CI results: {e}", "ERROR")
            
    def generate_ci_report(self, results: Dict[str, Any]) -> str:
        """Generate CI report for pull request or commit"""
        try:
            report_lines = [
                "# Playtesting CI Report",
                "",
                f"**Commit:** {results['commit_hash'][:8]}",
                f"**Message:** {results['commit_message']}",
                f"**Status:** {'✅ PASSED' if results['build_status'] == 'passed' else '❌ FAILED'}",
                f"**Duration:** {results['duration_seconds']:.1f} seconds",
                "",
                "## Test Results",
                f"- **Total Tests:** {results['test_summary'].get('total', 0)}",
                f"- **Passed:** {results['test_summary'].get('passed', 0)}",
                f"- **Failed:** {results['test_summary'].get('failed', 0)}",
                f"- **Success Rate:** {results['test_summary'].get('success_rate', 0):.1f}%",
                "",
                "## Issues Detected",
                f"- **Total Issues:** {results.get('issues_detected', 0)}",
                f"- **Critical Issues:** {results.get('critical_issues', 0)}",
                ""
            ]
            
            if results['build_status'] == 'failed':
                report_lines.extend([
                    "## Failure Reason",
                    f"{results.get('failure_reason', 'Unknown failure')}",
                    ""
                ])
                
            report_lines.extend([
                "## Recommendations",
                "Based on playtesting results:",
                ""
            ])
            
            # Add recommendations based on results
            if results['test_summary'].get('success_rate', 0) < 80:
                report_lines.append("- ⚠️ Test success rate is below 80%, review test failures")
                
            if results.get('critical_issues', 0) > 0:
                report_lines.append("- 🚨 Critical issues detected, address before merging")
                
            if results['test_summary'].get('failed', 0) > 0:
                report_lines.append("- 🔍 Review failed tests and fix underlying issues")
                
            if results['test_summary'].get('success_rate', 0) >= 90 and results.get('critical_issues', 0) == 0:
                report_lines.append("- ✅ Playtesting results look good for merge")
                
            return "\n".join(report_lines)
            
        except Exception as e:
            self.log(f"Error generating CI report: {e}", "ERROR")
            return f"Error generating report: {e}"
            
    def post_pr_comment(self, results: Dict[str, Any]):
        """Post comment to pull request (placeholder for GitHub API integration)"""
        try:
            report = self.generate_ci_report(results)
            
            # In real implementation, would use GitHub API to post comment
            report_file = CI_RESULTS_DIR / f"pr_comment_{results['session_id']}.md"
            with open(report_file, 'w') as f:
                f.write(report)
                
            self.log(f"PR comment saved: {report_file}")
            
        except Exception as e:
            self.log(f"Error posting PR comment: {e}", "ERROR")
            
    def monitor_loop(self):
        """Main monitoring loop"""
        self.log("Starting CI monitoring loop...")
        
        while self.running:
            try:
                # Check for new commits
                if self.check_for_new_commits():
                    commit_info = self.get_commit_info(self.last_commit_hash)
                    
                    if commit_info:
                        # Run playtesting
                        results = self.run_playtesting(commit_info)
                        
                        # Save results
                        self.save_ci_results(results)
                        
                        # Generate and post report
                        if self.config.get("ci_integration", {}).get("generate_reports_for_pr", True):
                            self.post_pr_comment(results)
                            
                        # Check if build should fail
                        if results.get("build_status") == "failed" and self.fail_on_critical:
                            self.log("Build failed due to critical issues", "ERROR")
                            
                # Sleep between checks
                time.sleep(60)  # Check every minute
                
            except Exception as e:
                self.log(f"Error in monitoring loop: {e}", "ERROR")
                time.sleep(60)  # Continue monitoring even on error
                
        self.log("CI monitoring loop stopped")
        
    def start(self):
        """Start CI monitoring"""
        if not self.is_git_available():
            self.log("Git repository not available, CI monitoring disabled", "WARNING")
            return False
            
        if self.running:
            self.log("CI monitoring already running", "WARNING")
            return False
            
        # Get initial commit hash
        self.last_commit_hash = self.get_current_commit_hash()
        if self.last_commit_hash:
            self.log(f"Initial commit: {self.last_commit_hash[:8]}")
            
        self.running = True
        self.monitor_thread = threading.Thread(target=self.monitor_loop, daemon=True)
        self.monitor_thread.start()
        
        self.log("CI monitoring started")
        return True
        
    def stop(self):
        """Stop CI monitoring"""
        if not self.running:
            self.log("CI monitoring not running", "WARNING")
            return
            
        self.running = False
        
        if self.monitor_thread:
            self.monitor_thread.join(timeout=5)
            
        self.log("CI monitoring stopped")
        
    def run_manual_test(self, commit_hash: str = None) -> Dict[str, Any]:
        """Run manual CI test for specific commit"""
        try:
            if not commit_hash:
                commit_hash = self.get_current_commit_hash()
                
            if not commit_hash:
                self.log("No commit hash available", "ERROR")
                return {"success": False, "error": "No commit hash"}
                
            commit_info = self.get_commit_info(commit_hash)
            
            if not commit_info:
                self.log("Could not get commit info", "ERROR")
                return {"success": False, "error": "Could not get commit info"}
                
            # Run playtesting
            results = self.run_playtesting(commit_info)
            
            # Save results
            self.save_ci_results(results)
            
            return results
            
        except Exception as e:
            self.log(f"Error in manual test: {e}", "ERROR")
            return {"success": False, "error": str(e)}
            
    def get_ci_history(self, limit: int = 10) -> List[Dict[str, Any]]:
        """Get CI history"""
        try:
            history = []
            
            # Load from result files
            result_files = sorted(CI_RESULTS_DIR.glob("ci_result_*.json"), reverse=True)
            
            for result_file in result_files[:limit]:
                try:
                    with open(result_file, 'r') as f:
                        history.append(json.load(f))
                except Exception as e:
                    self.log(f"Error loading CI result {result_file}: {e}", "WARNING")
                    
            return history
            
        except Exception as e:
            self.log(f"Error getting CI history: {e}", "ERROR")
            return []

def main():
    """Main function"""
    import argparse
    
    parser = argparse.ArgumentParser(description="Playtesting CI Integration")
    parser.add_argument("--config", type=str, default="playtesting_config.json",
                       help="Configuration file path")
    parser.add_argument("--action", type=str, choices=["start", "stop", "test", "history"],
                       required=True, help="Action to perform")
    parser.add_argument("--commit", type=str, help="Specific commit hash to test")
    
    args = parser.parse_args()
    
    print("=" * 70)
    print("PLAYTESTING CI INTEGRATION")
    print("=" * 70)
    
    # Create CI instance
    ci = PlaytestingCI(args.config)
    
    if args.action == "start":
        if ci.start():
            print("CI monitoring started successfully")
            print("Press Ctrl+C to stop...")
            try:
                while True:
                    time.sleep(1)
            except KeyboardInterrupt:
                print("\nStopping CI monitoring...")
                ci.stop()
        else:
            print("Failed to start CI monitoring")
            
    elif args.action == "stop":
        ci.stop()
        print("CI monitoring stopped")
        
    elif args.action == "test":
        print("Running manual CI test...")
        results = ci.run_manual_test(args.commit)
        
        print("\n" + "=" * 70)
        print("CI TEST RESULTS")
        print("=" * 70)
        print(json.dumps(results, indent=2))
        print("=" * 70)
        
        if results.get("build_status") == "passed":
            print("✅ CI test PASSED")
        else:
            print("❌ CI test FAILED")
            
    elif args.action == "history":
        history = ci.get_ci_history()
        
        print("\n" + "=" * 70)
        print("CI HISTORY")
        print("=" * 70)
        
        for result in history:
            status_icon = "✅" if result.get("build_status") == "passed" else "❌"
            print(f"{status_icon} {result['commit_hash'][:8]} - {result.get('build_status', 'unknown')} "
                  f"({result['duration_seconds']:.1f}s)")
                  
        print("=" * 70)

if __name__ == "__main__":
    main()