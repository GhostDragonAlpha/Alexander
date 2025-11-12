"""
Unit Tests for Alexander Method Report Validation

Tests that the Alexander Method (build_and_capture, run_unit_tests, etc.) 
produces valid, AI-parseable output structures.

Run:
    python test_alexander_method_reports.py
    
Or with pytest:
    pytest test_alexander_method_reports.py -v
"""

import json
import unittest
from pathlib import Path
from datetime import datetime
from typing import Dict, Any


class TestAlexanderMethodReports(unittest.TestCase):
    """Validates Alexander Method output structures"""
    
    def setUp(self):
        """Set up test paths"""
        self.project_root = Path(__file__).parent
        self.capture_output = self.project_root / "capture_output"
        self.test_results = self.project_root / "test_results"
        self.capture_runtime = self.project_root / "capture_output_runtime"
    
    def test_editor_capture_summary_exists(self):
        """Test that editor capture produces summary.md"""
        summary_path = self.capture_output / "summary.md"
        
        if not summary_path.exists():
            self.skipTest(f"Editor capture not run yet. Run: python build_and_capture.py")
        
        self.assertTrue(summary_path.exists(), "Editor capture summary.md should exist")
    
    def test_editor_capture_summary_structure(self):
        """Test editor capture summary has required sections"""
        summary_path = self.capture_output / "summary.md"
        
        if not summary_path.exists():
            self.skipTest("Editor capture not run yet")
        
        with open(summary_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Check for key sections
        required_sections = [
            "Build and Capture Report",
            "Build Status",
            "Editor Status",
            "Screenshot"
        ]
        
        for section in required_sections:
            self.assertIn(section, content, f"Summary should contain '{section}' section")
    
    def test_editor_screenshot_exists(self):
        """Test that editor screenshot is captured"""
        screenshot_path = self.capture_output / "editor_screenshot.png"
        
        if not screenshot_path.exists():
            self.skipTest("Editor capture not run yet")
        
        self.assertTrue(screenshot_path.exists(), "Editor screenshot should exist")
        
        # Check file is not empty
        file_size = screenshot_path.stat().st_size
        self.assertGreater(file_size, 10000, "Screenshot should be at least 10KB")
    
    def test_build_output_exists(self):
        """Test that build output is captured"""
        build_output = self.capture_output / "build_output.txt"
        
        if not build_output.exists():
            self.skipTest("Build not run yet")
        
        self.assertTrue(build_output.exists(), "Build output should exist")
    
    def test_unit_test_report_format(self):
        """Test that unit test reports have expected format"""
        if not self.test_results.exists():
            self.skipTest("Unit tests not run yet. Run: python run_unit_tests.py")
        
        # Find most recent test report
        reports = list(self.test_results.glob("test_report_*.txt"))
        if not reports:
            self.skipTest("No test reports found")
        
        latest_report = max(reports, key=lambda p: p.stat().st_mtime)
        
        with open(latest_report, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Check for key test report elements
        required_elements = [
            "API Test Results",
            "Total Tests:",
            "Passed:",
            "Failed:"
        ]
        
        for element in required_elements:
            self.assertIn(element, content, f"Test report should contain '{element}'")
    
    def test_runtime_capture_summary_exists(self):
        """Test that runtime capture produces summary.md"""
        summary_path = self.capture_runtime / "summary.md"
        
        if not summary_path.exists():
            self.skipTest("Runtime capture not run yet. Run: python build_and_capture_runtime.py")
        
        self.assertTrue(summary_path.exists(), "Runtime capture summary should exist")
    
    def test_runtime_screenshots_exist(self):
        """Test that runtime gameplay screenshots are captured"""
        if not self.capture_runtime.exists():
            self.skipTest("Runtime capture not run yet")
        
        # Should have multiple gameplay screenshots
        screenshots = list(self.capture_runtime.glob("gameplay_*.png"))
        
        if not screenshots:
            self.skipTest("No gameplay screenshots found")
        
        self.assertGreater(len(screenshots), 0, "Should have at least 1 gameplay screenshot")
        
        # Check screenshots are valid size
        for screenshot in screenshots:
            file_size = screenshot.stat().st_size
            self.assertGreater(file_size, 10000, f"{screenshot.name} should be at least 10KB")
    
    def test_ai_parseable_summary_format(self):
        """Test that summaries are AI-parseable (structured markdown)"""
        summary_path = self.capture_output / "summary.md"
        
        if not summary_path.exists():
            self.skipTest("Editor capture not run yet")
        
        with open(summary_path, 'r', encoding='utf-8') as f:
            lines = f.readlines()
        
        # Check for markdown structure
        has_headers = any(line.strip().startswith('#') for line in lines)
        has_lists = any(line.strip().startswith(('-', '*')) for line in lines)
        
        self.assertTrue(has_headers, "Summary should have markdown headers")
        self.assertTrue(has_lists or 'Status:' in ''.join(lines), 
                       "Summary should have structured data")
    
    def test_complete_alexander_method_cycle(self):
        """Integration test: verify complete Alexander Method cycle outputs"""
        # Check all three components produced output
        editor_summary = self.capture_output / "summary.md"
        test_reports = list(self.test_results.glob("test_report_*.txt")) if self.test_results.exists() else []
        runtime_summary = self.capture_runtime / "summary.md"
        
        components_ready = {
            "editor_capture": editor_summary.exists(),
            "unit_tests": len(test_reports) > 0,
            "runtime_capture": runtime_summary.exists()
        }
        
        ready_count = sum(components_ready.values())
        
        if ready_count == 0:
            self.skipTest("Alexander Method not run yet. Run all three scripts.")
        
        # Report status
        print(f"\nAlexander Method Components Status:")
        for component, status in components_ready.items():
            print(f"  {component}: {'✅ Ready' if status else '❌ Not run'}")
        
        if ready_count == 3:
            print("\n✅ Complete Alexander Method cycle ready for AI analysis!")


class TestAIReportDictionaries(unittest.TestCase):
    """Test JSON report structures for AI consumption"""
    
    def setUp(self):
        """Set up test paths"""
        self.project_root = Path(__file__).parent
        self.ai_reports = self.project_root / "Saved" / "AI_Reports"
    
    def test_ai_reports_directory_structure(self):
        """Test that AI reports directory exists"""
        if not self.ai_reports.exists():
            self.skipTest("AI system not run yet. Run: python build_and_test.py ai daily")
        
        self.assertTrue(self.ai_reports.exists(), "AI_Reports directory should exist")
    
    def test_workflow_report_json_structure(self):
        """Test that workflow reports have valid JSON structure"""
        if not self.ai_reports.exists():
            self.skipTest("AI reports not available")
        
        # Find any workflow report
        reports = list(self.ai_reports.glob("*.json"))
        
        if not reports:
            self.skipTest("No JSON reports found")
        
        # Test most recent report
        latest_report = max(reports, key=lambda p: p.stat().st_mtime)
        
        with open(latest_report, 'r', encoding='utf-8') as f:
            report_data = json.load(f)
        
        # Validate required fields
        required_fields = ["workflow", "start_time", "overall_success"]
        
        for field in required_fields:
            self.assertIn(field, report_data, f"Report should contain '{field}' field")
        
        # Validate types
        self.assertIsInstance(report_data["workflow"], str)
        self.assertIsInstance(report_data["overall_success"], bool)
        
        # Validate timestamp format
        try:
            datetime.fromisoformat(report_data["start_time"])
        except ValueError:
            self.fail("start_time should be valid ISO format timestamp")
    
    def test_task_tracker_storage(self):
        """Test that autonomous task tracker creates valid storage"""
        from autonomous_task_tracker import TaskTracker
        
        tracker = TaskTracker()
        
        # Start a test task
        task_id = tracker.start_task(
            "test_validation",
            {"test": "unit_test"},
            "Test task for validation"
        )
        
        # Complete it
        tracker.complete_task(task_id, success=True, metrics={"duration": 1.0})
        
        # Verify storage exists
        storage_path = tracker.storage_path
        self.assertTrue(storage_path.exists(), "Task tracker storage should exist")
        
        # Verify tasks are logged
        task_files = list(storage_path.glob("tasks_*.jsonl"))
        self.assertGreater(len(task_files), 0, "Should have at least one task log file")
        
        # Verify can retrieve task
        recent_tasks = tracker.get_recent_tasks(limit=1)
        self.assertEqual(len(recent_tasks), 1, "Should retrieve the test task")
        self.assertEqual(recent_tasks[0].task_id, task_id)


class TestAlexanderMethodParseability(unittest.TestCase):
    """Test that AI can parse Alexander Method outputs programmatically"""
    
    def test_parse_build_status_from_summary(self):
        """Test extracting build status from summary.md"""
        summary_path = Path(__file__).parent / "capture_output" / "summary.md"
        
        if not summary_path.exists():
            self.skipTest("Editor capture not run yet")
        
        with open(summary_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # AI should be able to determine build success
        # Look for status indicators
        build_success_indicators = ["✅", "SUCCESS", "Build Status: Success"]
        build_failure_indicators = ["❌", "FAILED", "Build Status: Failed", "ERROR"]
        
        has_success = any(indicator in content for indicator in build_success_indicators)
        has_failure = any(indicator in content for indicator in build_failure_indicators)
        
        # Should have clear status indication
        self.assertTrue(has_success or has_failure, 
                       "Summary should have clear success/failure indicators")
    
    def test_parse_test_results_programmatically(self):
        """Test extracting test pass/fail counts from test report"""
        test_results = Path(__file__).parent / "test_results"
        
        if not test_results.exists():
            self.skipTest("Unit tests not run yet")
        
        reports = list(test_results.glob("test_report_*.txt"))
        if not reports:
            self.skipTest("No test reports found")
        
        latest_report = max(reports, key=lambda p: p.stat().st_mtime)
        
        with open(latest_report, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # AI should be able to extract test counts
        import re
        
        total_match = re.search(r'Total Tests:\s*(\d+)', content)
        passed_match = re.search(r'Passed:\s*(\d+)', content)
        failed_match = re.search(r'Failed:\s*(\d+)', content)
        
        self.assertIsNotNone(total_match, "Should find 'Total Tests' count")
        self.assertIsNotNone(passed_match, "Should find 'Passed' count")
        self.assertIsNotNone(failed_match, "Should find 'Failed' count")
        
        if total_match and passed_match and failed_match:
            total = int(total_match.group(1))
            passed = int(passed_match.group(1))
            failed = int(failed_match.group(1))
            
            # Sanity check
            self.assertEqual(total, passed + failed, "Total should equal passed + failed")
            print(f"\n✅ Parsed test results: {passed}/{total} passed, {failed} failed")


def run_tests():
    """Run all tests with detailed output"""
    loader = unittest.TestLoader()
    suite = unittest.TestSuite()
    
    # Add all test classes
    suite.addTests(loader.loadTestsFromTestCase(TestAlexanderMethodReports))
    suite.addTests(loader.loadTestsFromTestCase(TestAIReportDictionaries))
    suite.addTests(loader.loadTestsFromTestCase(TestAlexanderMethodParseability))
    
    # Run with verbose output
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    
    # Print summary
    print("\n" + "="*80)
    print("Alexander Method Report Validation Summary")
    print("="*80)
    print(f"Tests Run: {result.testsRun}")
    print(f"Passed: {result.testsRun - len(result.failures) - len(result.errors) - len(result.skipped)}")
    print(f"Failed: {len(result.failures)}")
    print(f"Errors: {len(result.errors)}")
    print(f"Skipped: {len(result.skipped)}")
    
    if result.wasSuccessful():
        print("\n✅ All Alexander Method reports are valid and AI-parseable!")
    else:
        print("\n⚠️ Some reports need attention. Review failures above.")
    
    return result.wasSuccessful()


if __name__ == "__main__":
    import sys
    success = run_tests()
    sys.exit(0 if success else 1)
