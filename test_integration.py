#!/usr/bin/env python3
"""
Integration Test for Continuous Playtesting System
Tests all components working together
"""

import json
import time
import sys
import threading
from pathlib import Path
from typing import Dict, Any, List
import requests
import subprocess

# Test configuration
TEST_CONFIG = "playtesting_config.json"
TEST_TIMEOUT = 300  # 5 minutes max for integration test

class IntegrationTest:
    """Integration test for continuous playtesting system"""
    
    def __init__(self):
        self.test_results: List[Dict[str, Any]] = []
        self.errors: List[str] = []
        self.warnings: List[str] = []
        
    def log(self, message: str, level: str = "INFO"):
        """Log test message"""
        print(f"[IntegrationTest] {level}: {message}")
        
    def add_result(self, test_name: str, passed: bool, details: str = ""):
        """Add test result"""
        result = {
            "test_name": test_name,
            "passed": passed,
            "details": details,
            "timestamp": time.time()
        }
        self.test_results.append(result)
        
        if passed:
            self.log(f"✅ {test_name} PASSED")
        else:
            self.log(f"❌ {test_name} FAILED: {details}", "ERROR")
            self.errors.append(f"{test_name}: {details}")
            
    def add_warning(self, message: str):
        """Add warning"""
        self.warnings.append(message)
        self.log(f"⚠️  {message}", "WARNING")
        
    def test_configuration_loading(self):
        """Test configuration loading"""
        try:
            with open(TEST_CONFIG, 'r') as f:
                config = json.load(f)
                
            # Check required sections
            required_sections = [
                "continuous_playtesting",
                "performance_collection", 
                "performance_thresholds",
                "screenshot_capture",
                "test_scenarios",
                "dashboard"
            ]
            
            missing_sections = []
            for section in required_sections:
                if section not in config:
                    missing_sections.append(section)
                    
            if missing_sections:
                self.add_result("Configuration Loading", False, 
                              f"Missing sections: {missing_sections}")
            else:
                self.add_result("Configuration Loading", True, 
                              f"Loaded {len(config)} configuration sections")
                              
        except Exception as e:
            self.add_result("Configuration Loading", False, str(e))
            
    def test_component_imports(self):
        """Test that all components can be imported"""
        components = [
            ("continuous_playtesting", "ContinuousPlaytestingOrchestrator"),
            ("performance_collector", "PerformanceCollector"),
            ("issue_reporter", "IssueReporter"),
            ("data_manager", "DataManager"),
            ("playtesting_dashboard", "PlaytestingDashboard"),
            ("playtesting_ci", "PlaytestingCI")
        ]
        
        for module_name, class_name in components:
            try:
                module = __import__(module_name)
                cls = getattr(module, class_name)
                self.add_result(f"Import {module_name}", True, f"Successfully imported {class_name}")
            except Exception as e:
                self.add_result(f"Import {module_name}", False, str(e))
                
    def test_directory_structure(self):
        """Test that required directories exist"""
        required_dirs = [
            "playtesting_data",
            "automation_config",
            "screenshots_visual_playtest"
        ]
        
        for dir_path in required_dirs:
            if Path(dir_path).exists():
                self.add_result(f"Directory {dir_path}", True, "Directory exists")
            else:
                self.add_result(f"Directory {dir_path}", False, "Directory does not exist")
                
    def test_existing_files(self):
        """Test that required files exist"""
        required_files = [
            "playtesting_config.json",
            "continuous_playtesting.py",
            "performance_collector.py",
            "issue_reporter.py",
            "data_manager.py",
            "playtesting_dashboard.py",
            "playtesting_ci.py",
            "launch_game_desktop.py",
            "capture_screenshot.py",
            "test_scenarios.py"
        ]
        
        for file_path in required_files:
            if Path(file_path).exists():
                self.add_result(f"File {file_path}", True, "File exists")
            else:
                self.add_result(f"File {file_path}", False, "File does not exist")
                
    def test_performance_collector_creation(self):
        """Test performance collector creation"""
        try:
            from performance_collector import PerformanceCollector
            
            collector = PerformanceCollector(session_id="test_session")
            self.add_result("PerformanceCollector Creation", True, "Created successfully")
            
            # Test metrics collection (without game running)
            metrics = collector.collect_metrics()
            if metrics:
                self.add_result("Performance Metrics Collection", True, 
                              f"Collected metrics: FPS={metrics.fps}, Memory={metrics.memory_usage_mb}MB")
            else:
                self.add_result("Performance Metrics Collection", False, 
                              "No metrics collected (game not running)")
                self.add_warning("Performance metrics collection requires game to be running")
                
        except Exception as e:
            self.add_result("PerformanceCollector Creation", False, str(e))
            
    def test_issue_reporter_creation(self):
        """Test issue reporter creation"""
        try:
            from issue_reporter import IssueReporter
            
            reporter = IssueReporter()
            self.add_result("IssueReporter Creation", True, "Created successfully")
            
            # Test issue detection (without game running)
            issues = reporter.detect_issues()
            self.add_result("Issue Detection", True, f"Detection completed, found {len(issues)} issues")
            
        except Exception as e:
            self.add_result("IssueReporter Creation", False, str(e))
            
    def test_data_manager_creation(self):
        """Test data manager creation"""
        try:
            from data_manager import DataManager
            
            manager = DataManager()
            self.add_result("DataManager Creation", True, "Created successfully")
            
            # Test storage stats
            stats = manager.get_storage_stats()
            if "error" not in stats:
                self.add_result("Storage Stats", True, f"Retrieved stats for {len(stats.get('directories', {}))} directories")
            else:
                self.add_result("Storage Stats", False, stats.get("error", "Unknown error"))
                
        except Exception as e:
            self.add_result("DataManager Creation", False, str(e))
            
    def test_configuration_validation(self):
        """Test configuration validation"""
        try:
            with open(TEST_CONFIG, 'r') as f:
                config = json.load(f)
                
            # Validate performance thresholds
            thresholds = config.get("performance_thresholds", {})
            required_thresholds = ["fps_min", "frame_time_max_ms", "memory_max_mb"]
            
            missing_thresholds = []
            for threshold in required_thresholds:
                if threshold not in thresholds:
                    missing_thresholds.append(threshold)
                    
            if missing_thresholds:
                self.add_result("Configuration Validation", False, 
                              f"Missing thresholds: {missing_thresholds}")
            else:
                self.add_result("Configuration Validation", True, 
                              f"All {len(required_thresholds)} thresholds defined")
                              
        except Exception as e:
            self.add_result("Configuration Validation", False, str(e))
            
    def test_dashboard_template_creation(self):
        """Test dashboard template creation"""
        try:
            from playtesting_dashboard import PlaytestingDashboard
            
            dashboard = PlaytestingDashboard()
            
            # Check if template directory was created
            template_dir = Path("templates")
            if template_dir.exists():
                template_file = template_dir / "dashboard.html"
                if template_file.exists():
                    self.add_result("Dashboard Template", True, "Template created successfully")
                else:
                    self.add_result("Dashboard Template", False, "Template file not created")
            else:
                self.add_result("Dashboard Template", False, "Template directory not created")
                
        except Exception as e:
            self.add_result("Dashboard Template Creation", False, str(e))
            
    def test_launch_script(self):
        """Test launch script"""
        script_path = "launch_continuous_playtesting.bat"
        
        if Path(script_path).exists():
            self.add_result("Launch Script", True, "Launch script exists")
            
            # Check script content
            try:
                with open(script_path, 'r') as f:
                    content = f.read()
                    
                required_commands = [
                    "python continuous_playtesting.py",
                    "playtesting_config.json"
                ]
                
                missing_commands = []
                for cmd in required_commands:
                    if cmd not in content:
                        missing_commands.append(cmd)
                        
                if missing_commands:
                    self.add_warning(f"Launch script missing commands: {missing_commands}")
                    
            except Exception as e:
                self.add_warning(f"Could not verify launch script content: {e}")
        else:
            self.add_result("Launch Script", False, "Launch script does not exist")
            
    def test_integration_scenario(self):
        """Test a basic integration scenario"""
        try:
            self.log("Testing basic integration scenario...")
            
            # Create a simple test scenario
            from performance_collector import PerformanceCollector
            from issue_reporter import IssueReporter
            
            # Create components
            collector = PerformanceCollector(session_id="integration_test")
            reporter = IssueReporter()
            
            # Simulate data flow
            test_metrics = collector.collect_metrics()
            
            if test_metrics:
                # Simulate issue detection with metrics
                issue_data = {
                    "issue_type": "test_integration",
                    "severity": "low",
                    "description": "Integration test issue",
                    "performance_metrics": test_metrics.to_dict() if hasattr(test_metrics, 'to_dict') else {}
                }
                
                reporter.queue_issue(issue_data)
                
                self.add_result("Integration Scenario", True, 
                              "Data flow between components successful")
            else:
                self.add_result("Integration Scenario", False, 
                              "Could not collect metrics for integration test")
                
        except Exception as e:
            self.add_result("Integration Scenario", False, str(e))
            
    def run_all_tests(self):
        """Run all integration tests"""
        self.log("=" * 70)
        self.log("STARTING INTEGRATION TESTS")
        self.log("=" * 70)
        
        start_time = time.time()
        
        # Run tests
        self.test_configuration_loading()
        self.test_component_imports()
        self.test_directory_structure()
        self.test_existing_files()
        self.test_performance_collector_creation()
        self.test_issue_reporter_creation()
        self.test_data_manager_creation()
        self.test_configuration_validation()
        self.test_dashboard_template_creation()
        self.test_launch_script()
        self.test_integration_scenario()
        
        # Calculate results
        total_tests = len(self.test_results)
        passed_tests = sum(1 for r in self.test_results if r["passed"])
        failed_tests = total_tests - passed_tests
        
        duration = time.time() - start_time
        
        # Generate report
        self.log("=" * 70)
        self.log("INTEGRATION TEST RESULTS")
        self.log("=" * 70)
        self.log(f"Total Tests: {total_tests}")
        self.log(f"Passed: {passed_tests}")
        self.log(f"Failed: {failed_tests}")
        self.log(f"Duration: {duration:.1f} seconds")
        
        if self.warnings:
            self.log(f"Warnings: {len(self.warnings)}")
            
        self.log("=" * 70)
        
        # Print detailed results
        self.log("\nDetailed Results:")
        for result in self.test_results:
            status = "✅ PASS" if result["passed"] else "❌ FAIL"
            self.log(f"  {status}: {result['test_name']}")
            if result["details"]:
                self.log(f"    {result['details']}")
                
        # Print warnings
        if self.warnings:
            self.log("\nWarnings:")
            for warning in self.warnings:
                self.log(f"  ⚠️  {warning}")
                
        # Save test report
        self.save_test_report()
        
        # Return overall result
        return failed_tests == 0
        
    def save_test_report(self):
        """Save test report to file"""
        try:
            report = {
                "timestamp": time.time(),
                "summary": {
                    "total_tests": len(self.test_results),
                    "passed": sum(1 for r in self.test_results if r["passed"]),
                    "failed": sum(1 for r in self.test_results if not r["passed"]),
                    "warnings": len(self.warnings)
                },
                "results": self.test_results,
                "warnings": self.warnings,
                "errors": self.errors
            }
            
            report_file = Path("playtesting_data/integration_test_report.json")
            report_file.parent.mkdir(exist_ok=True)
            
            with open(report_file, 'w') as f:
                json.dump(report, f, indent=2)
                
            self.log(f"Test report saved: {report_file}")
            
        except Exception as e:
            self.log(f"Error saving test report: {e}", "ERROR")

def main():
    """Main function"""
    print("=" * 70)
    print("CONTINUOUS PLAYTESTING INTEGRATION TEST")
    print("=" * 70)
    
    try:
        # Create and run integration test
        test = IntegrationTest()
        success = test.run_all_tests()
        
        print("\n" + "=" * 70)
        if success:
            print("✅ INTEGRATION TEST PASSED")
            print("All components are ready for continuous playtesting!")
        else:
            print("❌ INTEGRATION TEST FAILED")
            print("Please fix the issues above before running continuous playtesting.")
        print("=" * 70)
        
        sys.exit(0 if success else 1)
        
    except Exception as e:
        print(f"Error running integration test: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()