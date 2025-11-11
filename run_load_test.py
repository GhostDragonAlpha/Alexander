#!/usr/bin/env python3
"""
Quick Test Executor for Load Testing
Simple command-line interface to run load tests
"""

import sys
import argparse
import json
import time
from typing import Dict, Optional
from load_test_controller import LoadTestController, run_load_test

def print_header():
    """Print application header"""
    print("=" * 60)
    print("Alexander Game - Load Testing System")
    print("Alpha Testing Version")
    print("=" * 60)
    print()

def print_scenarios():
    """Print available test scenarios"""
    controller = LoadTestController()
    scenarios = controller.list_scenarios()
    
    print("Available Test Scenarios:")
    print("-" * 30)
    for i, scenario in enumerate(scenarios, 1):
        print(f"{i}. {scenario}")
    print()

def run_quick_test():
    """Run a quick 4-player test"""
    print("Running Quick Test (4 players, 2 minutes)...")
    print("-" * 50)
    
    try:
        report = run_load_test("quick")
        
        if "error" in report:
            print(f"ERROR: {report['error']}")
            return False
        
        print_results(report)
        return report.get("passed", False)
        
    except Exception as e:
        print(f"ERROR: Test execution failed: {e}")
        return False

def run_scenario_test(scenario_name: str):
    """Run a specific scenario test"""
    print(f"Running {scenario_name} Test...")
    print("-" * 50)
    
    try:
        report = run_load_test(scenario_name)
        
        if "error" in report:
            print(f"ERROR: {report['error']}")
            return False
        
        print_results(report)
        return report.get("passed", False)
        
    except Exception as e:
        print(f"ERROR: Test execution failed: {e}")
        return False

def print_results(report: Dict):
    """Print test results in a formatted way"""
    print("\nTEST RESULTS")
    print("=" * 50)
    print(f"Scenario: {report.get('scenario_name', 'Unknown')}")
    print(f"Timestamp: {report.get('timestamp', 'Unknown')}")
    print(f"Duration: {report.get('duration_seconds', 0):.1f} seconds")
    print(f"Clients: {report.get('client_count', 0)}")
    print(f"Total Actions: {report.get('total_actions', 0)}")
    print(f"Total Errors: {report.get('total_errors', 0)}")
    print(f"Error Rate: {report.get('error_rate', 0):.2%}")
    
    print("\nPERFORMANCE METRICS")
    print("-" * 30)
    metrics = report.get("performance_metrics", {})
    print(f"Average FPS: {metrics.get('avg_fps', 0):.1f}")
    print(f"Average Memory: {metrics.get('avg_memory_mb', 0):.1f} MB")
    print(f"Average Latency: {metrics.get('avg_latency_ms', 0):.1f} ms")
    print(f"Min FPS: {metrics.get('min_fps', 0):.1f}")
    print(f"Max Memory: {metrics.get('max_memory_mb', 0):.1f} MB")
    print(f"Max Latency: {metrics.get('max_latency_ms', 0):.1f} ms")
    
    print("\nPASS/FAIL RESULT")
    print("-" * 20)
    passed = report.get("passed", False)
    status = "PASSED" if passed else "FAILED"
    print(f"Overall Result: {status}")
    
    if not passed:
        print("\nFailure Details:")
        criteria = report.get("pass_criteria", {})
        for key, value in criteria.items():
            print(f"  - {key}: {value}")

def check_server_status():
    """Check if game server is running and healthy"""
    print("Checking Server Status...")
    print("-" * 30)
    
    controller = LoadTestController()
    
    if controller.check_server_health():
        print("SUCCESS: Server is healthy and responding")
        return True
    else:
        print("FAILED: Server health check failed")
        print("Please make sure the game server is running and accessible")
        return False

def interactive_mode():
    """Run in interactive mode"""
    print_header()
    
    # Check server first
    if not check_server_status():
        response = input("\nContinue anyway? (y/n): ")
        if response.lower() != 'y':
            return
    
    while True:
        print("\nMAIN MENU")
        print("-" * 20)
        print("1. Run Quick Test (4 players)")
        print("2. Run 8 Player Test")
        print("3. Run 16 Player Test")
        print("4. Run 32 Player Test")
        print("5. Run 64 Player Test")
        print("6. Check Server Status")
        print("7. List Available Scenarios")
        print("8. Exit")
        
        choice = input("\nEnter your choice (1-8): ").strip()
        
        if choice == '1':
            run_quick_test()
        elif choice == '2':
            run_scenario_test("8_player")
        elif choice == '3':
            run_scenario_test("16_player")
        elif choice == '4':
            run_scenario_test("32_player")
        elif choice == '5':
            run_scenario_test("64_player")
        elif choice == '6':
            check_server_status()
        elif choice == '7':
            print_scenarios()
        elif choice == '8':
            print("Exiting...")
            break
        else:
            print("Invalid choice. Please try again.")

def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description='Alexander Game Load Testing System',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  python run_load_test.py              # Interactive mode
  python run_load_test.py quick        # Run quick test
  python run_load_test.py 32_player    # Run 32 player test
  python run_load_test.py --list       # List scenarios
  python run_load_test.py --status     # Check server status
        '''
    )
    
    parser.add_argument(
        'scenario',
        nargs='?',
        help='Test scenario to run (quick, 8_player, 16_player, 32_player, 64_player)'
    )
    
    parser.add_argument(
        '--list', '-l',
        action='store_true',
        help='List available test scenarios'
    )
    
    parser.add_argument(
        '--status', '-s',
        action='store_true',
        help='Check server status'
    )
    
    parser.add_argument(
        '--config',
        default='automation_config/load_test_scenarios.json',
        help='Path to configuration file'
    )
    
    args = parser.parse_args()
    
    # Handle flags
    if args.list:
        print_scenarios()
        return
    
    if args.status:
        check_server_status()
        return
    
    # Handle scenario argument
    if args.scenario:
        scenario = args.scenario.lower()
        valid_scenarios = ["quick", "8_player", "16_player", "32_player", "64_player"]
        
        if scenario in valid_scenarios:
            print_header()
            if check_server_status():
                success = run_scenario_test(scenario)
                sys.exit(0 if success else 1)
            else:
                print("Cannot run test - server not available")
                sys.exit(1)
        else:
            print(f"Error: Unknown scenario '{scenario}'")
            print(f"Valid scenarios: {', '.join(valid_scenarios)}")
            sys.exit(1)
    else:
        # Interactive mode
        interactive_mode()

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\nTest interrupted by user")
        sys.exit(1)
    except Exception as e:
        print(f"\nERROR: Unexpected error: {e}")
        sys.exit(1)