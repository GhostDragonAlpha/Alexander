#!/usr/bin/env python3
"""
Autonomous Verification System Test Runner
Main entry point for running automated verification tests
"""

import json
import time
import argparse
import sys
import os
from pathlib import Path
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass, asdict
from datetime import datetime
import requests
import subprocess
import signal
import threading
import logging

# Import verification components
from verification_system import AutonomousVerificationSystem
from game_launcher import GameLauncher
from visual_analyzer import VisualAnalyzer
from test_scenario_manager import TestScenarioManager
from performance_monitor import PerformanceMonitor
from error_detector import ErrorDetector
from report_generator import ReportGenerator

@dataclass
class VerificationConfig:
    """Configuration for verification system"""
    project_path: str
    unreal_editor_path: str
    map_name: str = "VRTemplateMap"
    game_mode: str = "/Script/Alexander.AutomationGameMode"
    resolution: str = "1280x720"
    quality: str = "medium"
    http_port: int = 8080
    screenshot_interval: float = 1.0
    performance_interval: float = 1.0
    max_test_duration: float = 3600.0
    output_directory: str = "verification_reports"
    test_suite: str = "CompleteGameLoop"
    enable_visual_analysis: bool = True
    enable_performance_monitoring: bool = True
    enable_error_detection: bool = True

class VerificationTestRunner:
    """Main test runner for autonomous verification system"""
    
    def __init__(self, config: VerificationConfig):
        self.config = config
        self.running = False
        self.verification_system = None
        self.game_launcher = None
        self.test_results = []
        self.start_time = None
        
        # Setup logging
        self.setup_logging()
        
        # Create output directory
        Path(config.output_directory).mkdir(parents=True, exist_ok=True)
        
    def setup_logging(self):
        """Setup logging configuration"""
        log_file = Path(self.config.output_directory) / f"verification_{datetime.now().strftime('%Y%m%d_%H%M%S')}.log"
        
        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s',
            handlers=[
                logging.FileHandler(log_file),
                logging.StreamHandler(sys.stdout)
            ]
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
            
    def run_verification(self) -> bool:
        """Run the complete verification process"""
        self.log("=" * 80)
        self.log("AUTONOMOUS VERIFICATION SYSTEM")
        self.log("=" * 80)
        self.log(f"Project: {self.config.project_path}")
        self.log(f"Test Suite: {self.config.test_suite}")
        self.log(f"Start Time: {datetime.now().isoformat()}")
        self.log("=" * 80)
        
        self.start_time = time.time()
        self.running = True
        
        try:
            # Step 1: Initialize verification system
            self.log("\n[Step 1/7] Initializing verification system...")
            if not self.initialize_verification_system():
                self.log("Failed to initialize verification system", "ERROR")
                return False
            self.log("✓ Verification system initialized")
            
            # Step 2: Launch game
            self.log("\n[Step 2/7] Launching game...")
            if not self.launch_game():
                self.log("Failed to launch game", "ERROR")
                return False
            self.log("✓ Game launched successfully")
            
            # Step 3: Wait for game to be ready
            self.log("\n[Step 3/7] Waiting for game to be ready...")
            if not self.wait_for_game_ready():
                self.log("Game failed to become ready", "ERROR")
                return False
            self.log("✓ Game is ready")
            
            # Step 4: Start monitoring
            self.log("\n[Step 4/7] Starting monitoring systems...")
            self.start_monitoring()
            self.log("✓ Monitoring systems started")
            
            # Step 5: Execute test scenarios
            self.log("\n[Step 5/7] Executing test scenarios...")
            test_results = self.execute_test_scenarios()
            self.test_results.extend(test_results)
            self.log(f"✓ Executed {len(test_results)} test scenarios")
            
            # Step 6: Collect final data
            self.log("\n[Step 6/7] Collecting final data...")
            self.collect_final_data()
            self.log("✓ Final data collected")
            
            # Step 7: Generate report
            self.log("\n[Step 7/7] Generating verification report...")
            self.generate_report()
            self.log("✓ Verification report generated")
            
            # Complete
            duration = time.time() - self.start_time
            self.log("\n" + "=" * 80)
            self.log("VERIFICATION COMPLETED SUCCESSFULLY")
            self.log(f"Duration: {duration:.1f} seconds")
            self.log(f"Test Results: {len(self.test_results)} scenarios executed")
            self.log("=" * 80)
            
            return True
            
        except Exception as e:
            self.log(f"Verification failed with exception: {e}", "ERROR")
            return False
        finally:
            self.cleanup()
            
    def initialize_verification_system(self) -> bool:
        """Initialize the verification system components"""
        try:
            # Create verification system
            self.verification_system = AutonomousVerificationSystem(self.config)
            
            # Initialize components
            self.verification_system.initialize_components()
            
            return True
        except Exception as e:
            self.log(f"Error initializing verification system: {e}", "ERROR")
            return False
            
    def launch_game(self) -> bool:
        """Launch the game"""
        try:
            # Create game launcher
            self.game_launcher = GameLauncher(self.config)
            
            # Launch game
            return self.game_launcher.launch()
        except Exception as e:
            self.log(f"Error launching game: {e}", "ERROR")
            return False
            
    def wait_for_game_ready(self) -> bool:
        """Wait for game to be ready"""
        timeout = 180  # 3 minutes
        start_time = time.time()
        
        while time.time() - start_time < timeout:
            try:
                # Check if game HTTP API is responding
                response = requests.get(f"http://localhost:{self.config.http_port}/status", timeout=5)
                if response.status_code == 200:
                    elapsed = int(time.time() - start_time)
                    self.log(f"Game ready (took {elapsed}s)")
                    return True
            except:
                pass
                
            time.sleep(2)
            
        return False
        
    def start_monitoring(self):
        """Start monitoring systems"""
        if self.verification_system:
            self.verification_system.start_monitoring()
            
    def execute_test_scenarios(self) -> List[Dict]:
        """Execute test scenarios"""
        if not self.verification_system:
            return []
            
        return self.verification_system.execute_test_suite(self.config.test_suite)
        
    def collect_final_data(self):
        """Collect final verification data"""
        if self.verification_system:
            self.verification_system.collect_final_data()
            
    def generate_report(self):
        """Generate verification report"""
        if self.verification_system:
            report_path = Path(self.config.output_directory) / f"verification_report_{datetime.now().strftime('%Y%m%d_%H%M%S')}.html"
            self.verification_system.generate_report(str(report_path))
            
    def cleanup(self):
        """Cleanup resources"""
        self.log("Cleaning up verification system...")
        
        if self.game_launcher:
            self.game_launcher.shutdown()
            
        if self.verification_system:
            self.verification_system.cleanup()
            
        self.running = False
        
    def signal_handler(self, signum, frame):
        """Handle shutdown signals"""
        self.log(f"Received signal {signum}, shutting down...")
        self.running = False
        self.cleanup()
        sys.exit(0)

def load_config(config_path: str) -> VerificationConfig:
    """Load configuration from file"""
    with open(config_path, 'r') as f:
        config_data = json.load(f)
        
    return VerificationConfig(**config_data)

def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description="Autonomous Verification System Test Runner")
    parser.add_argument("--config", type=str, default="verification_config.json", help="Configuration file path")
    parser.add_argument("--project", type=str, help="Project path (overrides config)")
    parser.add_argument("--test-suite", type=str, help="Test suite name (overrides config)")
    parser.add_argument("--output", type=str, help="Output directory (overrides config)")
    parser.add_argument("--timeout", type=int, help="Test timeout in seconds (overrides config)")
    
    args = parser.parse_args()
    
    # Load configuration
    config = load_config(args.config)
    
    # Override with command line arguments
    if args.project:
        config.project_path = args.project
    if args.test_suite:
        config.test_suite = args.test_suite
    if args.output:
        config.output_directory = args.output
    if args.timeout:
        config.max_test_duration = args.timeout
        
    # Create and run test runner
    runner = VerificationTestRunner(config)
    
    # Setup signal handlers
    signal.signal(signal.SIGINT, runner.signal_handler)
    signal.signal(signal.SIGTERM, runner.signal_handler)
    
    # Run verification
    success = runner.run_verification()
    
    # Exit with appropriate code
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()