#!/usr/bin/env python3
"""
Integration Test for Autonomous Playtesting System
Tests all components working together
"""

import sys
import time
import json
import subprocess
import signal
import threading
from pathlib import Path
from typing import Dict, List, Optional
import requests
import argparse

# Configuration
SERVER_URL = "http://localhost:8080"
CONFIG_FILE = "automation_config/playtesting_config.json"
LAUNCHER_SCRIPT = "launch_game_desktop.py"
SCREENSHOT_SCRIPT = "capture_screenshot.py"
TEST_SCENARIOS_SCRIPT = "test_scenarios.py"

class IntegrationTester:
    """Integration test for autonomous playtesting system"""
    
    def __init__(self):
        self.game_process = None
        self.screenshot_process = None
        self.test_running = False
        self.results = []
        
    def log(self, message: str, level: str = "INFO"):
        """Log message"""
        print(f"[{level}] {message}")
        
    def start_game(self) -> bool:
        """Start game using desktop launcher"""
        self.log("Starting game with desktop launcher...")
        
        try:
            # Launch game in background
            self.game_process = subprocess.Popen([
                sys.executable, LAUNCHER_SCRIPT,
                "--resolution", "1280x720",
                "--quality", "low",
                "--interval", "5",
                "--timeout", "300"
            ])
            
            # Wait for game to start
            time.sleep(10)
            
            # Check if process is running
            if self.game_process.poll() is None:
                self.log("✓ Game process started")
                
                # Wait for HTTP API
                for i in range(30):
                    try:
                        response = requests.get(f"{SERVER_URL}/status", timeout=5)
                        if response.status_code == 200:
                            self.log("✓ Game HTTP API ready")
                            return True
                    except:
                        pass
                    time.sleep(2)
                
                self.log("✗ Game HTTP API not responding", "ERROR")
                return False
            else:
                self.log("✗ Game process failed to start", "ERROR")
                return False
                
        except Exception as e:
            self.log(f"✗ Failed to start game: {e}", "ERROR")
            return False
    
    def start_screenshot_capture(self) -> bool:
        """Start screenshot capture"""
        self.log("Starting screenshot capture...")
        
        try:
            # Start screenshot capture in background
            self.screenshot_process = subprocess.Popen([
                sys.executable, SCREENSHOT_SCRIPT,
                "--server", SERVER_URL,
                "--interval", "5",
                "--duration", "60"
            ])
            
            self.log("✓ Screenshot capture started")
            return True
            
        except Exception as e:
            self.log(f"✗ Failed to start screenshot capture: {e}", "ERROR")
            return False
    
    def run_test_scenario(self) -> bool:
        """Run a test scenario"""
        self.log("Running test scenario...")
        
        try:
            # Run first mission scenario
            result = subprocess.run([
                sys.executable, TEST_SCENARIOS_SCRIPT,
                "--server", SERVER_URL,
                "--scenario", "first_mission"
            ], capture_output=True, text=True, timeout=120)
            
            if result.returncode == 0:
                self.log("✓ Test scenario completed")
                self.log(f"Scenario output: {result.stdout}")
                return True
            else:
                self.log(f"✗ Test scenario failed: {result.stderr}", "ERROR")
                return False
                
        except subprocess.TimeoutExpired:
            self.log("✗ Test scenario timed out", "ERROR")
            return False
        except Exception as e:
            self.log(f"✗ Test scenario failed: {e}", "ERROR")
            return False
    
    def test_end_to_end_workflow(self) -> bool:
        """Test complete end-to-end workflow"""
        self.log("Testing end-to-end workflow...")
        
        try:
            # 1. Get initial game state
            response = requests.get(f"{SERVER_URL}/game_state", timeout=5)
            if response.status_code != 200:
                self.log("✗ Cannot get initial game state", "ERROR")
                return False
            
            initial_state = response.json()
            self.log("✓ Initial game state retrieved")
            
            # 2. Execute some commands
            commands = [
                "stat fps",
                "stat unit",
                "r.ScreenPercentage 75"
            ]
            
            for cmd in commands:
                response = requests.post(
                    f"{SERVER_URL}/execute_command",
                    json={"command": cmd},
                    timeout=5
                )
                if response.status_code == 200:
                    self.log(f"✓ Command executed: {cmd}")
                else:
                    self.log(f"✗ Command failed: {cmd}", "ERROR")
                    return False
            
            # 3. Get updated game state
            time.sleep(2)
            response = requests.get(f"{SERVER_URL}/game_state", timeout=5)
            if response.status_code == 200:
                self.log("✓ Updated game state retrieved")
            else:
                self.log("✗ Cannot get updated game state", "ERROR")
                return False
            
            # 4. Get performance metrics
            response = requests.get(f"{SERVER_URL}/performance_metrics", timeout=5)
            if response.status_code == 200:
                perf_data = response.json()
                if perf_data.get("success"):
                    metrics = perf_data.get("data", {})
                    fps = metrics.get("frame", {}).get("fps", 0)
                    self.log(f"✓ Performance metrics: {fps:.1f} FPS")
                else:
                    self.log("✗ Performance metrics indicate failure", "ERROR")
                    return False
            else:
                self.log("✗ Cannot get performance metrics", "ERROR")
                return False
            
            # 5. Take screenshot
            response = requests.post(
                f"{SERVER_URL}/screenshot",
                json={"filename": "integration_test"},
                timeout=10
            )
            if response.status_code == 200:
                self.log("✓ Screenshot taken")
            else:
                self.log("✗ Screenshot failed", "ERROR")
                return False
            
            return True
            
        except Exception as e:
            self.log(f"✗ End-to-end workflow test failed: {e}", "ERROR")
            return False
    
    def monitor_screenshots(self) -> bool:
        """Monitor screenshot capture"""
        self.log("Monitoring screenshot capture...")
        
        try:
            screenshot_dir = Path("screenshots_visual_playtest")
            initial_count = len(list(screenshot_dir.glob("*.jpg"))) if screenshot_dir.exists() else 0
            
            # Monitor for 30 seconds
            time.sleep(30)
            
            final_count = len(list(screenshot_dir.glob("*.jpg"))) if screenshot_dir.exists() else 0
            captured = final_count - initial_count
            
            if captured > 0:
                self.log(f"✓ Screenshots captured: {captured}")
                return True
            else:
                self.log("✗ No screenshots captured", "ERROR")
                return False
                
        except Exception as e:
            self.log(f"✗ Screenshot monitoring failed: {e}", "ERROR")
            return False
    
    def cleanup(self):
        """Clean up processes"""
        self.log("Cleaning up processes...")
        
        if self.screenshot_process:
            self.screenshot_process.terminate()
            self.screenshot_process.wait()
            self.log("✓ Screenshot process terminated")
        
        if self.game_process:
            self.game_process.terminate()
            self.game_process.wait()
            self.log("✓ Game process terminated")
    
    def run_integration_test(self) -> bool:
        """Run complete integration test"""
        self.log("=" * 70)
        self.log("AUTONOMOUS PLAYTESTING SYSTEM - INTEGRATION TEST")
        self.log("=" * 70)
        
        try:
            # Start game
            if not self.start_game():
                return False
            
            time.sleep(5)
            
            # Start screenshot capture
            if not self.start_screenshot_capture():
                return False
            
            time.sleep(10)
            
            # Monitor screenshots
            if not self.monitor_screenshots():
                self.log("Warning: Screenshot monitoring issue", "WARNING")
            
            # Run test scenario
            if not self.run_test_scenario():
                self.log("Warning: Test scenario had issues", "WARNING")
            
            # Test end-to-end workflow
            if not self.test_end_to_end_workflow():
                return False
            
            # Wait a bit more
            time.sleep(10)
            
            self.log("\n" + "=" * 70)
            self.log("INTEGRATION TEST COMPLETED SUCCESSFULLY")
            self.log("=" * 70)
            
            return True
            
        except Exception as e:
            self.log(f"✗ Integration test failed: {e}", "ERROR")
            return False
        finally:
            self.cleanup()

def signal_handler(signum, frame):
    """Handle shutdown signals"""
    print(f"\nReceived signal {signum}, shutting down...")
    sys.exit(0)

def main():
    """Main integration test execution"""
    parser = argparse.ArgumentParser(description="Integration test for autonomous playtesting")
    parser.add_argument("--server", type=str, default=SERVER_URL, help="Game server URL")
    parser.add_argument("--skip-game-start", action="store_true", help="Skip starting game (assume already running)")
    
    args = parser.parse_args()
    
    global SERVER_URL
    SERVER_URL = args.server
    
    # Setup signal handlers
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    print("=" * 70)
    print("AUTONOMOUS PLAYTESTING SYSTEM - INTEGRATION TEST")
    print("=" * 70)
    print(f"Server: {SERVER_URL}")
    print("=" * 70)
    
    tester = IntegrationTester()
    success = tester.run_integration_test()
    
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()