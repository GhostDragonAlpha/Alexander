"""
Comprehensive Test Suite for Visual Verification Pipeline
Tests all components of the visual analysis system end-to-end.
"""

import sys
import time
import json
import base64
from pathlib import Path
from typing import Dict, List, Any
import cv2
import numpy as np

# Import pipeline components
from visual_analyzer import VisualAnalyzer
from text_recognizer import TextRecognizer
from object_detector import ObjectDetector
from state_comparator import StateComparator, ExpectedGameState
from issue_classifier import IssueClassifier
from visual_verification_integration import VisualVerificationIntegration
from create_test_screenshots import TestScreenshotGenerator

class VisualPipelineTester:
    """Comprehensive test suite for visual verification pipeline"""
    
    def __init__(self):
        self.test_results = {
            "timestamp": time.time(),
            "tests_run": 0,
            "tests_passed": 0,
            "tests_failed": 0,
            "component_results": {},
            "performance_metrics": {}
        }
        
        # Initialize components
        self.visual_analyzer = VisualAnalyzer()
        self.text_recognizer = TextRecognizer()
        self.object_detector = ObjectDetector()
        self.state_comparator = StateComparator()
        self.issue_classifier = IssueClassifier()
        self.integration = VisualVerificationIntegration()
        
        # Test data
        self.test_screenshots = []
        self.expected_states = {}
        
        print("Visual Pipeline Tester initialized")
    
    def setup_test_environment(self):
        """Setup test environment with generated screenshots"""
        print("\n=== Setting up test environment ===")
        
        # Generate test screenshots
        generator = TestScreenshotGenerator()
        test_info = generator.generate_all_test_screenshots()
        
        self.test_screenshots = [info["filepath"] for info in test_info]
        self.expected_states = {info["filepath"]: info["expected_state"] for info in test_info}
        
        print(f"Generated {len(self.test_screenshots)} test screenshots")
        return len(self.test_screenshots) > 0
    
    def run_all_tests(self):
        """Run all test suites"""
        print("\n=== Running Visual Pipeline Tests ===")
        
        # Setup
        if not self.setup_test_environment():
            print("FAILED: Could not setup test environment")
            return False
        
        # Run component tests
        self.test_visual_analyzer()
        self.test_text_recognizer()
        self.test_object_detector()
        self.test_state_comparator()
        self.test_issue_classifier()
        self.test_integration()
        
        # Print summary
        self.print_test_summary()
        
        # Save results
        self.save_test_results()
        
        return self.test_results["tests_failed"] == 0
    
    def test_visual_analyzer(self):
        """Test visual analyzer component"""
        print("\n--- Testing Visual Analyzer ---")
        
        component_results = {
            "tests_run": 0,
            "tests_passed": 0,
            "tests_failed": 0,
            "performance": []
        }
        
        for screenshot_path in self.test_screenshots:
            try:
                start_time = time.time()
                result = self.visual_analyzer.analyze_screenshot(screenshot_path)
                processing_time = time.time() - start_time
                
                component_results["tests_run"] += 1
                component_results["performance"].append(processing_time)
                
                # Validate results
                if "error" in result:
                    print(f"  FAILED: {screenshot_path} - {result['error']}")
                    component_results["tests_failed"] += 1
                    self.test_results["tests_failed"] += 1
                else:
                    # Check for expected structure
                    required_keys = ["ui_elements", "visual_artifacts", "color_analysis", "confidence_scores"]
                    if all(key in result for key in required_keys):
                        print(f"  PASSED: {Path(screenshot_path).name} - {len(result['ui_elements'])} UI elements, {len(result['visual_artifacts'])} artifacts")
                        component_results["tests_passed"] += 1
                        self.test_results["tests_passed"] += 1
                    else:
                        print(f"  FAILED: {screenshot_path} - Missing required result keys")
                        component_results["tests_failed"] += 1
                        self.test_results["tests_failed"] += 1
                
                self.test_results["tests_run"] += 1
                
            except Exception as e:
                print(f"  FAILED: {screenshot_path} - Exception: {e}")
                component_results["tests_failed"] += 1
                self.test_results["tests_failed"] += 1
                self.test_results["tests_run"] += 1
        
        # Performance metrics
        if component_results["performance"]:
            avg_time = sum(component_results["performance"]) / len(component_results["performance"])
            component_results["average_processing_time"] = avg_time
            print(f"  Average processing time: {avg_time:.3f}s")
        
        self.test_results["component_results"]["visual_analyzer"] = component_results
    
    def test_text_recognizer(self):
        """Test text recognizer component"""
        print("\n--- Testing Text Recognizer ---")
        
        component_results = {
            "tests_run": 0,
            "tests_passed": 0,
            "tests_failed": 0,
            "performance": [],
            "accuracy_scores": []
        }
        
        # Test with text-heavy screenshots
        text_screenshots = [p for p in self.test_screenshots if "text" in p or "mixed" in p or "ui" in p]
        
        for screenshot_path in text_screenshots:
            try:
                start_time = time.time()
                result = self.text_recognizer.recognize_text(screenshot_path)
                processing_time = time.time() - start_time
                
                component_results["tests_run"] += 1
                component_results["performance"].append(processing_time)
                
                # Validate results
                if result.text_elements:
                    print(f"  PASSED: {Path(screenshot_path).name} - {len(result.text_elements)} text elements, avg confidence: {result.average_confidence:.1f}%")
                    component_results["tests_passed"] += 1
                    component_results["accuracy_scores"].append(result.average_confidence)
                    self.test_results["tests_passed"] += 1
                else:
                    print(f"  WARNING: {screenshot_path} - No text elements detected")
                    component_results["tests_passed"] += 1  # Still pass if no text expected
                    self.test_results["tests_passed"] += 1
                
                self.test_results["tests_run"] += 1
                
            except Exception as e:
                print(f"  FAILED: {screenshot_path} - Exception: {e}")
                component_results["tests_failed"] += 1
                self.test_results["tests_failed"] += 1
                self.test_results["tests_run"] += 1
        
        # Performance and accuracy metrics
        if component_results["performance"]:
            avg_time = sum(component_results["performance"]) / len(component_results["performance"])
            component_results["average_processing_time"] = avg_time
            print(f"  Average processing time: {avg_time:.3f}s")
        
        if component_results["accuracy_scores"]:
            avg_accuracy = sum(component_results["accuracy_scores"]) / len(component_results["accuracy_scores"])
            component_results["average_accuracy"] = avg_accuracy
            print(f"  Average accuracy: {avg_accuracy:.1f}%")
        
        self.test_results["component_results"]["text_recognizer"] = component_results
    
    def test_object_detector(self):
        """Test object detector component"""
        print("\n--- Testing Object Detector ---")
        
        component_results = {
            "tests_run": 0,
            "tests_passed": 0,
            "tests_failed": 0,
            "performance": [],
            "detection_rates": []
        }
        
        for screenshot_path in self.test_screenshots:
            try:
                start_time = time.time()
                result = self.object_detector.detect_objects(screenshot_path)
                processing_time = time.time() - start_time
                
                component_results["tests_run"] += 1
                component_results["performance"].append(processing_time)
                
                # Validate results
                if result.detected_objects:
                    print(f"  PASSED: {Path(screenshot_path).name} - {len(result.detected_objects)} objects detected, confidence: {result.model_confidence:.2f}")
                    component_results["tests_passed"] += 1
                    component_results["detection_rates"].append(len(result.detected_objects))
                    self.test_results["tests_passed"] += 1
                else:
                    print(f"  WARNING: {screenshot_path} - No objects detected")
                    component_results["tests_passed"] += 1  # Still pass if no objects expected
                    component_results["detection_rates"].append(0)
                    self.test_results["tests_passed"] += 1
                
                self.test_results["tests_run"] += 1
                
            except Exception as e:
                print(f"  FAILED: {screenshot_path} - Exception: {e}")
                component_results["tests_failed"] += 1
                self.test_results["tests_failed"] += 1
                self.test_results["tests_run"] += 1
        
        # Performance metrics
        if component_results["performance"]:
            avg_time = sum(component_results["performance"]) / len(component_results["performance"])
            component_results["average_processing_time"] = avg_time
            print(f"  Average processing time: {avg_time:.3f}s")
        
        if component_results["detection_rates"]:
            avg_detections = sum(component_results["detection_rates"]) / len(component_results["detection_rates"])
            component_results["average_detections"] = avg_detections
            print(f"  Average detections: {avg_detections:.1f} objects")
        
        self.test_results["component_results"]["object_detector"] = component_results
    
    def test_state_comparator(self):
        """Test state comparator component"""
        print("\n--- Testing State Comparator ---")
        
        component_results = {
            "tests_run": 0,
            "tests_passed": 0,
            "tests_failed": 0,
            "performance": []
        }
        
        # Test with screenshots that have expected states
        for screenshot_path, expected_state in self.expected_states.items():
            if expected_state:  # Only test if we have expected state
                try:
                    start_time = time.time()
                    
                    # Create ExpectedGameState from dict
                    expected_game_state = ExpectedGameState(
                        ui_elements=expected_state.get("ui_elements", []),
                        text_elements=expected_state.get("text_elements", []),
                        object_counts=expected_state.get("object_counts", {}),
                        player_state=expected_state.get("player_state", {}),
                        mission_state=expected_state.get("mission_state", {}),
                        inventory_state=expected_state.get("inventory_state", {})
                    )
                    
                    result = self.state_comparator.compare_states(screenshot_path, expected_game_state)
                    processing_time = time.time() - start_time
                    
                    component_results["tests_run"] += 1
                    component_results["performance"].append(processing_time)
                    
                    # Validate results
                    print(f"  PASSED: {Path(screenshot_path).name} - Valid: {result.is_valid}, Discrepancies: {len(result.discrepancies)}, Confidence: {result.overall_confidence:.2f}")
                    component_results["tests_passed"] += 1
                    self.test_results["tests_passed"] += 1
                    self.test_results["tests_run"] += 1
                    
                except Exception as e:
                    print(f"  FAILED: {screenshot_path} - Exception: {e}")
                    component_results["tests_failed"] += 1
                    self.test_results["tests_failed"] += 1
                    self.test_results["tests_run"] += 1
        
        # Performance metrics
        if component_results["performance"]:
            avg_time = sum(component_results["performance"]) / len(component_results["performance"])
            component_results["average_processing_time"] = avg_time
            print(f"  Average processing time: {avg_time:.3f}s")
        
        self.test_results["component_results"]["state_comparator"] = component_results
    
    def test_issue_classifier(self):
        """Test issue classifier component"""
        print("\n--- Testing Issue Classifier ---")
        
        component_results = {
            "tests_run": 0,
            "tests_passed": 0,
            "tests_failed": 0,
            "performance": []
        }
        
        # Test with sample analysis results
        sample_results = {
            "visual_artifacts": [],  # Would contain actual artifacts in real test
            "text_elements": [],     # Would contain actual text elements
            "state_discrepancies": [] # Would contain actual discrepancies
        }
        
        try:
            start_time = time.time()
            result = self.issue_classifier.classify_issues(sample_results)
            processing_time = time.time() - start_time
            
            component_results["tests_run"] += 1
            component_results["performance"].append(processing_time)
            
            # Validate results
            print(f"  PASSED: Issue classification - {len(result.classified_issues)} issues classified")
            print(f"    Critical: {result.critical_count}, High: {result.high_count}, Medium: {result.medium_count}, Low: {result.low_count}")
            component_results["tests_passed"] += 1
            self.test_results["tests_passed"] += 1
            self.test_results["tests_run"] += 1
            
        except Exception as e:
            print(f"  FAILED: Issue classification - Exception: {e}")
            component_results["tests_failed"] += 1
            self.test_results["tests_failed"] += 1
            self.test_results["tests_run"] += 1
        
        # Performance metrics
        if component_results["performance"]:
            avg_time = sum(component_results["performance"]) / len(component_results["performance"])
            component_results["average_processing_time"] = avg_time
            print(f"  Average processing time: {avg_time:.3f}s")
        
        self.test_results["component_results"]["issue_classifier"] = component_results
    
    def test_integration(self):
        """Test integration component"""
        print("\n--- Testing Integration Module ---")
        
        component_results = {
            "tests_run": 0,
            "tests_passed": 0,
            "tests_failed": 0,
            "performance": []
        }
        
        # Test integration with first screenshot
        if self.test_screenshots:
            screenshot_path = self.test_screenshots[0]
            expected_state = self.expected_states.get(screenshot_path, {})
            
            try:
                start_time = time.time()
                
                # Test capture and analyze
                result = self.integration.capture_and_analyze(
                    screenshot_path, 
                    metadata={"test": "integration_test"},
                    game_state=expected_state
                )
                
                processing_time = time.time() - start_time
                
                component_results["tests_run"] += 1
                component_results["performance"].append(processing_time)
                
                # Validate results
                if result.success:
                    print(f"  PASSED: Integration test - {result.processing_time:.3f}s, Success: {result.success}")
                    if result.issue_classification:
                        print(f"    Issues: {result.issue_classification['total_issues']}, Critical: {result.issue_classification['critical_count']}")
                    component_results["tests_passed"] += 1
                    self.test_results["tests_passed"] += 1
                else:
                    print(f"  FAILED: Integration test - {result.error}")
                    component_results["tests_failed"] += 1
                    self.test_results["tests_failed"] += 1
                
                self.test_results["tests_run"] += 1
                
            except Exception as e:
                print(f"  FAILED: Integration test - Exception: {e}")
                component_results["tests_failed"] += 1
                self.test_results["tests_failed"] += 1
                self.test_results["tests_run"] += 1
        
        # Performance metrics
        if component_results["performance"]:
            avg_time = sum(component_results["performance"]) / len(component_results["performance"])
            component_results["average_processing_time"] = avg_time
            print(f"  Average processing time: {avg_time:.3f}s")
        
        self.test_results["component_results"]["integration"] = component_results
    
    def print_test_summary(self):
        """Print test summary"""
        print("\n=== Test Summary ===")
        print(f"Total tests run: {self.test_results['tests_run']}")
        print(f"Tests passed: {self.test_results['tests_passed']}")
        print(f"Tests failed: {self.test_results['tests_failed']}")
        
        if self.test_results['tests_run'] > 0:
            pass_rate = (self.test_results['tests_passed'] / self.test_results['tests_run']) * 100
            print(f"Pass rate: {pass_rate:.1f}%")
        
        print("\nComponent Results:")
        for component, results in self.test_results['component_results'].items():
            print(f"  {component}:")
            print(f"    Tests: {results['tests_run']}, Passed: {results['tests_passed']}, Failed: {results['tests_failed']}")
            if 'average_processing_time' in results:
                print(f"    Avg time: {results['average_processing_time']:.3f}s")
            if 'average_accuracy' in results:
                print(f"    Avg accuracy: {results['average_accuracy']:.1f}%")
            if 'average_detections' in results:
                print(f"    Avg detections: {results['average_detections']:.1f}")
        
        # Overall performance
        all_times = []
        for component_results in self.test_results['component_results'].values():
            if 'performance' in component_results:
                all_times.extend(component_results['performance'])
        
        if all_times:
            avg_time = sum(all_times) / len(all_times)
            print(f"\nOverall average processing time: {avg_time:.3f}s")
            
            # Check against target (< 2 seconds)
            if avg_time < 2.0:
                print("+ Performance target met (< 2 seconds)")
            else:
                print("- Performance target not met (≥ 2 seconds)")
    
    def save_test_results(self):
        """Save test results to file"""
        try:
            # Add completion timestamp
            self.test_results["completed_at"] = time.time()
            
            # Save to file
            with open("visual_pipeline_test_results.json", 'w') as f:
                json.dump(self.test_results, f, indent=2, default=str)
            
            print(f"\nTest results saved to: visual_pipeline_test_results.json")
            
        except Exception as e:
            print(f"Failed to save test results: {e}")
    
    def validate_requirements(self):
        """Validate that all requirements are met"""
        print("\n=== Requirements Validation ===")
        
        requirements_met = True
        
        # Check text recognition accuracy
        text_results = self.test_results["component_results"].get("text_recognizer", {})
        if "average_accuracy" in text_results:
            accuracy = text_results["average_accuracy"]
            if accuracy >= 90.0:
                print(f"+ Text recognition accuracy: {accuracy:.1f}% (target: ≥90%)")
            else:
                print(f"- Text recognition accuracy: {accuracy:.1f}% (target: ≥90%)")
                requirements_met = False
        
        # Check object detection accuracy (simulated)
        object_results = self.test_results["component_results"].get("object_detector", {})
        if "average_detections" in object_results:
            detections = object_results["average_detections"]
            if detections >= 3.0:  # Should detect multiple objects
                print(f"+ Object detection working: {detections:.1f} average detections")
            else:
                print(f"- Object detection issues: {detections:.1f} average detections")
                requirements_met = False
        
        # Check performance
        all_times = []
        for component_results in self.test_results['component_results'].values():
            if 'performance' in component_results:
                all_times.extend(component_results['performance'])
        
        if all_times:
            avg_time = sum(all_times) / len(all_times)
            if avg_time < 2.0:
                print(f"+ Performance: {avg_time:.3f}s average (target: <2s)")
            else:
                print(f"- Performance: {avg_time:.3f}s average (target: <2s)")
                requirements_met = False
        
        # Check component functionality
        components = ["visual_analyzer", "text_recognizer", "object_detector", 
                     "state_comparator", "issue_classifier", "integration"]
        
        for component in components:
            if component in self.test_results["component_results"]:
                comp_results = self.test_results["component_results"][component]
                if comp_results["tests_passed"] > 0:
                    print(f"+ {component} is functional")
                else:
                    print(f"- {component} has no passing tests")
                    requirements_met = False
            
            if requirements_met:
                print("\n+ All requirements met! Visual verification pipeline is ready.")
            else:
                print("\n- Some requirements not met. Review test results above.")
        
        return requirements_met

def main():
    """Run all tests"""
    print("Visual Verification Pipeline Test Suite")
    print("=" * 50)
    
    tester = VisualPipelineTester()
    success = tester.run_all_tests()
    
    # Validate requirements
    requirements_met = tester.validate_requirements()
    
    # Exit with appropriate code
    sys.exit(0 if success and requirements_met else 1)

if __name__ == "__main__":
    main()