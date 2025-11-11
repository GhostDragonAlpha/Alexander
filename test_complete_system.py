#!/usr/bin/env python3
"""
Complete System Integration Test Suite
Tests the entire playtesting workflow from launch to reporting.
"""

import sys
import os
import json
import time
import requests
import threading
import subprocess
from pathlib import Path
from typing import Dict, List, Optional, Tuple
from datetime import datetime
import psutil

class IntegrationTestError(Exception):
    """Custom exception for integration test errors"""
    pass

class CompleteSystemTester:
    def __init__(self, config_path: str = "deployment_config.json"):
        self.config_path = config_path
        self.config = self.load_config()
        self.test_report = {
            "timestamp": datetime.now().isoformat(),
            "status": "in_progress",
            "tests": {},
            "performance": {},
            "errors": []
        }
        self.base_url = "http://localhost"
        self.services = {
            "game": 8080,
            "analysis": 8081,
            "dashboard": 8082
        }
        
    def log(self, message: str, level: str = "INFO"):
        """Log message with timestamp"""
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        print(f"[{timestamp}] [{level}] {message}")
        
    def load_config(self) -> Dict:
        """Load deployment configuration"""
        try:
            with open(self.config_path, 'r') as f:
                return json.load(f)
        except FileNotFoundError:
            self.log(f"Config file not found: {self.config_path}", "WARNING")
            return {}
            
    def check_service_health(self, service_name: str, port: int) -> bool:
        """Check if a service is healthy"""
        try:
            url = f"{self.base_url}:{port}/health"
            response = requests.get(url, timeout=5)
            if response.status_code == 200:
                self.test_report["tests"][f"{service_name}_health"] = {
                    "status": "pass",
                    "response_time": response.elapsed.total_seconds(),
                    "details": response.json() if response.content else {}
                }
                self.log(f"{service_name} service is healthy")
                return True
            else:
                raise IntegrationTestError(f"Service returned status {response.status_code}")
        except Exception as e:
            self.test_report["tests"][f"{service_name}_health"] = {
                "status": "fail",
                "error": str(e)
            }
            self.log(f"{service_name} service health check failed: {e}", "ERROR")
            return False
            
    def test_api_endpoints(self) -> bool:
        """Test all HTTP API endpoints"""
        endpoints = [
            # Game API endpoints
            ("game", 8080, "/api/screenshot", "POST"),
            ("game", 8080, "/api/performance", "GET"),
            ("game", 8080, "/api/state", "GET"),
            
            # Analysis API endpoints
            ("analysis", 8081, "/api/analyze", "POST"),
            ("analysis", 8081, "/api/issues", "GET"),
            ("analysis", 8081, "/api/report", "GET"),
            
            # Dashboard API endpoints
            ("dashboard", 8082, "/api/status", "GET"),
            ("dashboard", 8082, "/api/metrics", "GET"),
            ("dashboard", 8082, "/api/issues", "GET")
        ]
        
        all_passed = True
        for service_name, port, endpoint, method in endpoints:
            test_name = f"api_{service_name}_{endpoint.replace('/', '_')}"
            try:
                url = f"{self.base_url}:{port}{endpoint}"
                
                if method == "GET":
                    response = requests.get(url, timeout=10)
                elif method == "POST":
                    # For POST endpoints, send minimal data
                    response = requests.post(url, json={}, timeout=10)
                    
                if response.status_code in [200, 201, 204]:
                    self.test_report["tests"][test_name] = {
                        "status": "pass",
                        "response_time": response.elapsed.total_seconds(),
                        "status_code": response.status_code
                    }
                    self.log(f"API endpoint {endpoint} on {service_name} is working")
                else:
                    raise IntegrationTestError(f"Unexpected status code: {response.status_code}")
                    
            except Exception as e:
                self.test_report["tests"][test_name] = {
                    "status": "fail",
                    "error": str(e)
                }
                self.log(f"API endpoint test failed for {endpoint}: {e}", "ERROR")
                all_passed = False
                
        return all_passed
        
    def test_screenshot_capture(self) -> bool:
        """Test screenshot capture and transmission"""
        try:
            # Test screenshot capture endpoint
            url = f"{self.base_url}:8080/api/screenshot"
            response = requests.post(url, json={
                "quality": 85,
                "format": "png"
            }, timeout=30)
            
            if response.status_code == 200:
                screenshot_data = response.json()
                if "screenshot_path" in screenshot_data:
                    self.test_report["tests"]["screenshot_capture"] = {
                        "status": "pass",
                        "response_time": response.elapsed.total_seconds(),
                        "screenshot_path": screenshot_data["screenshot_path"]
                    }
                    self.log("Screenshot capture test passed")
                    
                    # Verify file exists
                    if Path(screenshot_data["screenshot_path"]).exists():
                        self.test_report["tests"]["screenshot_file"] = {
                            "status": "pass",
                            "file_size": Path(screenshot_data["screenshot_path"]).stat().st_size
                        }
                        self.log("Screenshot file verification passed")
                        return True
                    else:
                        raise IntegrationTestError("Screenshot file not found")
                else:
                    raise IntegrationTestError("Invalid screenshot response format")
            else:
                raise IntegrationTestError(f"Screenshot capture failed: {response.status_code}")
                
        except Exception as e:
            self.test_report["tests"]["screenshot_capture"] = {
                "status": "fail",
                "error": str(e)
            }
            self.log(f"Screenshot capture test failed: {e}", "ERROR")
            return False
            
    def test_visual_analysis(self) -> bool:
        """Test visual analysis pipeline"""
        try:
            # First capture a screenshot
            screenshot_url = f"{self.base_url}:8080/api/screenshot"
            screenshot_response = requests.post(screenshot_url, json={}, timeout=30)
            
            if screenshot_response.status_code != 200:
                raise IntegrationTestError("Failed to capture screenshot for analysis")
                
            screenshot_data = screenshot_response.json()
            
            # Then analyze it
            analysis_url = f"{self.base_url}:8081/api/analyze"
            analysis_response = requests.post(analysis_url, json={
                "screenshot_path": screenshot_data["screenshot_path"],
                "analysis_type": "comprehensive"
            }, timeout=60)
            
            if analysis_response.status_code == 200:
                analysis_data = analysis_response.json()
                self.test_report["tests"]["visual_analysis"] = {
                    "status": "pass",
                    "response_time": analysis_response.elapsed.total_seconds(),
                    "issues_found": len(analysis_data.get("issues", [])),
                    "analysis_complete": analysis_data.get("analysis_complete", False)
                }
                self.log(f"Visual analysis test passed, found {len(analysis_data.get('issues', []))} issues")
                return True
            else:
                raise IntegrationTestError(f"Analysis failed: {analysis_response.status_code}")
                
        except Exception as e:
            self.test_report["tests"]["visual_analysis"] = {
                "status": "fail",
                "error": str(e)
            }
            self.log(f"Visual analysis test failed: {e}", "ERROR")
            return False
            
    def test_performance_collection(self) -> bool:
        """Test performance data collection"""
        try:
            # Get performance data
            url = f"{self.base_url}:8080/api/performance"
            response = requests.get(url, timeout=10)
            
            if response.status_code == 200:
                performance_data = response.json()
                required_metrics = ["fps", "frame_time", "cpu_usage", "memory_usage"]
                
                missing_metrics = [m for m in required_metrics if m not in performance_data]
                if not missing_metrics:
                    self.test_report["tests"]["performance_collection"] = {
                        "status": "pass",
                        "response_time": response.elapsed.total_seconds(),
                        "metrics_collected": len(performance_data)
                    }
                    self.log(f"Performance collection test passed, collected {len(performance_data)} metrics")
                    return True
                else:
                    raise IntegrationTestError(f"Missing metrics: {missing_metrics}")
            else:
                raise IntegrationTestError(f"Performance endpoint failed: {response.status_code}")
                
        except Exception as e:
            self.test_report["tests"]["performance_collection"] = {
                "status": "fail",
                "error": str(e)
            }
            self.log(f"Performance collection test failed: {e}", "ERROR")
            return False
            
    def test_issue_detection(self) -> bool:
        """Test issue detection and reporting"""
        try:
            # Get issues from analysis service
            url = f"{self.base_url}:8081/api/issues"
            response = requests.get(url, timeout=10)
            
            if response.status_code == 200:
                issues_data = response.json()
                self.test_report["tests"]["issue_detection"] = {
                    "status": "pass",
                    "response_time": response.elapsed.total_seconds(),
                    "issues_count": len(issues_data.get("issues", [])),
                    "categories": list(set(issue.get("category") for issue in issues_data.get("issues", [])))
                }
                self.log(f"Issue detection test passed, found {len(issues_data.get('issues', []))} issues")
                return True
            else:
                raise IntegrationTestError(f"Issues endpoint failed: {response.status_code}")
                
        except Exception as e:
            self.test_report["tests"]["issue_detection"] = {
                "status": "fail",
                "error": str(e)
            }
            self.log(f"Issue detection test failed: {e}", "ERROR")
            return False
            
    def test_dashboard_functionality(self) -> bool:
        """Test dashboard functionality"""
        try:
            # Test dashboard status
            status_url = f"{self.base_url}:8082/api/status"
            status_response = requests.get(status_url, timeout=10)
            
            if status_response.status_code != 200:
                raise IntegrationTestError("Dashboard status endpoint failed")
                
            # Test dashboard metrics
            metrics_url = f"{self.base_url}:8082/api/metrics"
            metrics_response = requests.get(metrics_url, timeout=10)
            
            if metrics_response.status_code == 200:
                metrics_data = metrics_response.json()
                self.test_report["tests"]["dashboard_functionality"] = {
                    "status": "pass",
                    "status_response_time": status_response.elapsed.total_seconds(),
                    "metrics_response_time": metrics_response.elapsed.total_seconds(),
                    "metrics_available": len(metrics_data.get("metrics", []))
                }
                self.log(f"Dashboard functionality test passed, {len(metrics_data.get('metrics', []))} metrics available")
                return True
            else:
                raise IntegrationTestError(f"Dashboard metrics endpoint failed: {metrics_response.status_code}")
                
        except Exception as e:
            self.test_report["tests"]["dashboard_functionality"] = {
                "status": "fail",
                "error": str(e)
            }
            self.log(f"Dashboard functionality test failed: {e}", "ERROR")
            return False
            
    def test_data_storage(self) -> bool:
        """Test data storage and management"""
        try:
            # Check if data files are being created
            data_files = [
                "analysis_results/issues.json",
                "analysis_results/performance.json",
                "analysis_results/screenshots.json"
            ]
            
            files_found = 0
            for data_file in data_files:
                if Path(data_file).exists():
                    files_found += 1
                    file_size = Path(data_file).stat().st_size
                    self.test_report["tests"][f"data_file_{Path(data_file).stem}"] = {
                        "status": "pass",
                        "file_size": file_size
                    }
                    
            if files_found > 0:
                self.test_report["tests"]["data_storage"] = {
                    "status": "pass",
                    "files_found": files_found,
                    "total_files": len(data_files)
                }
                self.log(f"Data storage test passed, found {files_found}/{len(data_files)} data files")
                return True
            else:
                raise IntegrationTestError("No data files found")
                
        except Exception as e:
            self.test_report["tests"]["data_storage"] = {
                "status": "fail",
                "error": str(e)
            }
            self.log(f"Data storage test failed: {e}", "ERROR")
            return False
            
    def test_complete_workflow(self) -> bool:
        """Test the complete workflow: launch → screenshot → analyze → report"""
        try:
            self.log("Starting complete workflow test")
            
            # Step 1: Capture screenshot
            screenshot_url = f"{self.base_url}:8080/api/screenshot"
            screenshot_response = requests.post(screenshot_url, json={}, timeout=30)
            
            if screenshot_response.status_code != 200:
                raise IntegrationTestError("Workflow failed at screenshot capture")
                
            screenshot_data = screenshot_response.json()
            
            # Step 2: Analyze screenshot
            analysis_url = f"{self.base_url}:8081/api/analyze"
            analysis_response = requests.post(analysis_url, json={
                "screenshot_path": screenshot_data["screenshot_path"]
            }, timeout=60)
            
            if analysis_response.status_code != 200:
                raise IntegrationTestError("Workflow failed at analysis")
                
            # Step 3: Get performance data
            performance_url = f"{self.base_url}:8080/api/performance"
            performance_response = requests.get(performance_url, timeout=10)
            
            if performance_response.status_code != 200:
                raise IntegrationTestError("Workflow failed at performance collection")
                
            # Step 4: Get issues report
            issues_url = f"{self.base_url}:8081/api/issues"
            issues_response = requests.get(issues_url, timeout=10)
            
            if issues_response.status_code != 200:
                raise IntegrationTestError("Workflow failed at issues reporting")
                
            # Calculate total workflow time
            total_time = (screenshot_response.elapsed.total_seconds() + 
                         analysis_response.elapsed.total_seconds() + 
                         performance_response.elapsed.total_seconds() + 
                         issues_response.elapsed.total_seconds())
            
            self.test_report["tests"]["complete_workflow"] = {
                "status": "pass",
                "total_time": total_time,
                "screenshot_time": screenshot_response.elapsed.total_seconds(),
                "analysis_time": analysis_response.elapsed.total_seconds(),
                "performance_time": performance_response.elapsed.total_seconds(),
                "issues_time": issues_response.elapsed.total_seconds()
            }
            
            self.log(f"Complete workflow test passed, total time: {total_time:.2f}s")
            return True
            
        except Exception as e:
            self.test_report["tests"]["complete_workflow"] = {
                "status": "fail",
                "error": str(e)
            }
            self.log(f"Complete workflow test failed: {e}", "ERROR")
            return False
            
    def run_all_tests(self) -> bool:
        """Run all integration tests"""
        self.log("Starting Complete System Integration Tests")
        
        tests = [
            ("Service Health Checks", self.test_service_health),
            ("API Endpoints", self.test_api_endpoints),
            ("Screenshot Capture", self.test_screenshot_capture),
            ("Visual Analysis", self.test_visual_analysis),
            ("Performance Collection", self.test_performance_collection),
            ("Issue Detection", self.test_issue_detection),
            ("Dashboard Functionality", self.test_dashboard_functionality),
            ("Data Storage", self.test_data_storage),
            ("Complete Workflow", self.test_complete_workflow)
        ]
        
        # Test each service health first
        for service_name, port in self.services.items():
            if not self.check_service_health(service_name, port):
                self.log(f"Critical: {service_name} service is not healthy", "ERROR")
                return False
                
        # Run all other tests
        all_passed = True
        for test_name, test_func in tests[1:]:  # Skip service health as we already did it
            self.log(f"Running test: {test_name}")
            if not test_func():
                all_passed = False
                self.log(f"Test failed: {test_name}", "ERROR")
                
        # Generate final report
        self.generate_test_report()
        
        if all_passed:
            self.log("All integration tests passed!", "SUCCESS")
        else:
            self.log("Some integration tests failed", "ERROR")
            
        return all_passed
        
    def generate_test_report(self):
        """Generate comprehensive test report"""
        # Count passed/failed tests
        passed = sum(1 for test in self.test_report["tests"].values() 
                    if test.get("status") == "pass")
        failed = sum(1 for test in self.test_report["tests"].values() 
                    if test.get("status") == "fail")
                    
        self.test_report["summary"] = {
            "total_tests": len(self.test_report["tests"]),
            "passed": passed,
            "failed": failed,
            "success_rate": passed / len(self.test_report["tests"]) if self.test_report["tests"] else 0
        }
        
        self.test_report["status"] = "completed"
        
        # Save report
        report_path = "verification_reports/integration_test_report.json"
        with open(report_path, 'w') as f:
            json.dump(self.test_report, f, indent=2)
            
        self.log(f"Integration test report saved to: {report_path}")
        
        # Also save as markdown
        self.generate_markdown_report()
        
    def generate_markdown_report(self):
        """Generate markdown version of test report"""
        markdown_content = f"""# Integration Test Report

**Date:** {self.test_report["timestamp"]}
**Status:** {self.test_report["status"]}

## Summary
- **Total Tests:** {self.test_report["summary"]["total_tests"]}
- **Passed:** {self.test_report["summary"]["passed"]}
- **Failed:** {self.test_report["summary"]["failed"]}
- **Success Rate:** {self.test_report["summary"]["success_rate"]:.1%}

## Test Results
"""
        
        for test_name, test_result in self.test_report["tests"].items():
            status_icon = "✅" if test_result.get("status") == "pass" else "❌"
            markdown_content += f"\n### {status_icon} {test_name.replace('_', ' ').title()}\n"
            markdown_content += f"- **Status:** {test_result.get('status', 'unknown')}\n"
            
            if "response_time" in test_result:
                markdown_content += f"- **Response Time:** {test_result['response_time']:.2f}s\n"
            if "error" in test_result:
                markdown_content += f"- **Error:** {test_result['error']}\n"
                
        # Save markdown report
        markdown_path = "verification_reports/integration_test_report.md"
        with open(markdown_path, 'w') as f:
            f.write(markdown_content)
            
        self.log(f"Markdown test report saved to: {markdown_path}")

def main():
    """Main entry point"""
    tester = CompleteSystemTester()
    success = tester.run_all_tests()
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()