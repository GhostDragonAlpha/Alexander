#!/usr/bin/env python3
"""
Test Suite for AI Agent Autonomous Development System
Comprehensive testing of all components with Kimi API integration
"""

import sys
import os
import time
import json
from pathlib import Path
from typing import List, Dict, Any

# Add current directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

# Import all components
try:
    from kimi_api_client import KimiAPIClient, test_kimi_client
    from ai_control_layer import AIControlLayer, test_ai_control_layer
    from natural_language_parser import NaturalLanguageParser, test_natural_language_parser
    from task_generator import TaskGenerator, test_task_generator
    from decision_engine import DecisionEngine, test_decision_engine
    print("+ All components imported successfully")
except ImportError as e:
    print(f"- Import error: {e}")
    print("Please ensure all required modules are in the same directory")
    sys.exit(1)


class TestResult:
    """Test result tracking"""
    def __init__(self, test_name: str, success: bool, message: str, duration: float):
        self.test_name = test_name
        self.success = success
        self.message = message
        self.duration = duration


class AITestSuite:
    """Comprehensive test suite for AI Agent system"""
    
    def __init__(self):
        self.test_results: List[TestResult] = []
        self.start_time = time.time()
        
    def run_test(self, test_name: str, test_func, *args, **kwargs) -> bool:
        """Run a single test and track results"""
        print(f"\n{'='*60}")
        print(f"Running: {test_name}")
        print(f"{'='*60}")
        
        test_start = time.time()
        
        try:
            success = test_func(*args, **kwargs)
            duration = time.time() - test_start
            
            if success:
                print(f"+ {test_name} PASSED ({duration:.2f}s)")
                self.test_results.append(TestResult(test_name, True, "Success", duration))
            else:
                print(f"- {test_name} FAILED ({duration:.2f}s)")
                self.test_results.append(TestResult(test_name, False, "Test returned False", duration))
            
            return success
            
        except Exception as e:
            duration = time.time() - test_start
            print(f"- {test_name} ERROR ({duration:.2f}s): {e}")
            self.test_results.append(TestResult(test_name, False, f"Exception: {e}", duration))
            return False
    
    def run_all_tests(self) -> bool:
        """Run all component tests"""
        print("AI Agent Autonomous Development System - Test Suite")
        print("=" * 60)
        print(f"Started: {time.strftime('%Y-%m-%d %H:%M:%S')}")
        print("=" * 60)
        
        # Test 1: Kimi API Client
        self.run_test("Kimi API Client", test_kimi_client)
        
        # Test 2: Natural Language Parser
        self.run_test("Natural Language Parser", test_natural_language_parser)
        
        # Test 3: Task Generator
        self.run_test("Task Generator", test_task_generator)
        
        # Test 4: Decision Engine
        self.run_test("Decision Engine", test_decision_engine)
        
        # Test 5: AI Control Layer
        self.run_test("AI Control Layer", test_ai_control_layer)
        
        # Test 6: Integration test (all components working together)
        self.run_test("Integration Test", self.test_integration)
        
        return self.print_summary()
    
    def test_integration(self) -> bool:
        """Test all components working together"""
        print("\nTesting component integration...")
        
        try:
            # Initialize all components
            print("  Initializing components...")
            kimi_client = KimiAPIClient()
            parser = NaturalLanguageParser(kimi_client)
            task_generator = TaskGenerator(kimi_client)
            decision_engine = DecisionEngine(kimi_client)
            control_layer = AIControlLayer(kim_api_key=kimi_client.api_key)
            
            print("  ✓ All components initialized")
            
            # Test workflow: Goal -> Parse -> Generate Tasks -> Make Decisions
            test_goal = "Fix the bug where ships don't spawn correctly in multiplayer mode"
            
            print(f"\n  Processing goal: {test_goal}")
            
            # Step 1: Parse goal
            parsed_goal = parser.parse_development_goal(test_goal)
            print(f"  ✓ Goal parsed: {parsed_goal.goal_type.value}")
            
            # Step 2: Generate tasks
            task_specs = task_generator.generate_tasks_from_goal(parsed_goal)
            print(f"  ✓ Tasks generated: {len(task_specs)}")
            
            # Step 3: Convert to AI Control tasks
            ai_tasks = [task_generator.convert_to_ai_control_task(spec) for spec in task_specs]
            print(f"  ✓ Tasks converted for execution")
            
            # Step 4: Make execution decisions
            from decision_engine import DecisionContext
            context = DecisionContext(
                available_tasks=ai_tasks,
                system_state={"load": "normal", "network_status": "online"},
                historical_data={"attempts": 0},
                resource_constraints={"cpu": 80, "memory": 70},
                safety_threshold=control_layer.SafetyLevel.MEDIUM_RISK
            )
            
            for task in ai_tasks[:2]:  # Test with first 2 tasks
                decision = decision_engine.decide_task_execution_strategy(task, context)
                print(f"  ✓ Decision made for {task.task_id}: {decision.confidence.value} confidence")
            
            # Step 5: Test system status
            status = control_layer.get_system_status()
            print(f"  ✓ System status retrieved: {status['active_goals']} active goals")
            
            # Cleanup
            control_layer.cleanup()
            kimi_client.close()
            
            print("\n  ✓ Integration test completed successfully")
            return True
            
        except Exception as e:
            print(f"  ✗ Integration test failed: {e}")
            import traceback
            traceback.print_exc()
            return False
    
    def print_summary(self) -> bool:
        """Print test summary"""
        total_duration = time.time() - self.start_time
        
        print(f"\n{'='*60}")
        print("TEST SUMMARY")
        print(f"{'='*60}")
        
        passed = sum(1 for r in self.test_results if r.success)
        failed = sum(1 for r in self.test_results if not r.success)
        total = len(self.test_results)
        
        print(f"Total Tests: {total}")
        print(f"Passed: {passed} ✓")
        print(f"Failed: {failed} ✗")
        print(f"Success Rate: {passed/total*100:.1f}%" if total > 0 else "N/A")
        print(f"Total Duration: {total_duration:.2f}s")
        
        if failed > 0:
            print(f"\nFailed Tests:")
            for result in self.test_results:
                if not result.success:
                    print(f"  - {result.test_name}: {result.message}")
        
        print(f"\n{'='*60}")
        
        return failed == 0


def test_environment_setup():
    """Test environment setup and dependencies"""
    print("Testing Environment Setup")
    print("=" * 60)
    
    tests_passed = 0
    total_tests = 5
    
    # Test 1: Python version
    print("\n1. Checking Python version...")
    if sys.version_info >= (3, 8):
        print(f"   + Python {sys.version.split()[0]} (compatible)")
        tests_passed += 1
    else:
        print(f"   - Python {sys.version.split()[0]} (requires 3.8+)")
    
    # Test 2: Environment file
    print("\n2. Checking .env file...")
    env_path = Path(".env")
    if env_path.exists():
        print("   + .env file exists")
        # Check for API key
        with open(env_path) as f:
            content = f.read()
            if "KIMI_API_KEY=your_kimi_api_key_here" in content:
                print("   ! .env file contains placeholder API key")
                print("   -> Please add your real Kimi API key to .env")
            elif "KIMI_API_KEY=" in content and "your_kimi_api_key_here" not in content:
                print("   + API key appears to be configured")
                tests_passed += 1
    else:
        print("   - .env file not found")
        print("   -> Please create .env file with your Kimi API key")
    
    # Test 3: Required packages
    print("\n3. Checking required packages...")
    required_packages = ["requests", "dotenv", "pydantic"]
    missing_packages = []
    
    for package in required_packages:
        try:
            __import__(package)
            print(f"   + {package}")
            tests_passed += 1
        except ImportError:
            print(f"   - {package}")
            missing_packages.append(package)
    
    # Test 4: Project structure
    print("\n4. Checking project structure...")
    required_files = [
        "kimi_api_client.py",
        "ai_control_layer.py",
        "natural_language_parser.py",
        "task_generator.py",
        "decision_engine.py"
    ]
    
    for file_path in required_files:
        if Path(file_path).exists():
            print(f"   + {file_path}")
            tests_passed += 1
        else:
            print(f"   - {file_path}")
    
    # Test 5: API connectivity (optional)
    print("\n5. Testing API connectivity...")
    try:
        # This will only work if API key is configured
        client = KimiAPIClient()
        test_response = client.chat_completion([
            {"role": "user", "content": "Test connection"}
        ])
        
        if test_response.success:
            print("   + Kimi API connection successful")
            tests_passed += 1
        else:
            print(f"   ! Kimi API test failed: {test_response.error_message}")
            print("   -> This may be due to invalid API key or network issues")
    except Exception as e:
        print(f"   ! API test skipped: {e}")
        print("   -> API key may not be configured yet")
    
    print(f"\n{'='*60}")
    print(f"Environment Setup: {tests_passed}/{total_tests} tests passed")
    
    if missing_packages:
        print(f"\nTo install missing packages, run:")
        print(f"pip install {' '.join(missing_packages)}")
    
    return tests_passed == total_tests


def main():
    """Main test execution"""
    print("AI Agent Autonomous Development System")
    print("Comprehensive Test Suite")
    print("=" * 60)
    
    # Test environment first
    if not test_environment_setup():
        print("\n! Environment setup incomplete. Some tests may fail.")
        response = input("Continue anyway? (y/n): ")
        if response.lower() != 'y':
            print("Tests cancelled.")
            sys.exit(1)
    
    # Run comprehensive tests
    test_suite = AITestSuite()
    success = test_suite.run_all_tests()
    
    # Print final result
    if success:
        print("\n*** ALL TESTS PASSED! ***")
        print("\nThe AI Agent Autonomous Development System is ready for use.")
        print("\nNext steps:")
        print("1. Add your Kimi API key to .env file")
        print("2. Run: python ai_control_layer.py to test the system")
        print("3. Start using the system for autonomous development!")
        sys.exit(0)
    else:
        print("\n*** SOME TESTS FAILED")
        print("\nPlease review the errors above and fix any issues.")
        print("Common issues:")
        print("- Missing API key in .env file")
        print("- Missing Python packages (run: pip install -r requirements.txt)")
        print("- Network connectivity issues")
        sys.exit(1)


if __name__ == "__main__":
    main()