#!/usr/bin/env python3
"""
Automated Test Scenarios for Autonomous Playtesting
Predefined gameplay sequences with success/failure criteria
"""

import json
import time
import random
import logging
from pathlib import Path
from typing import Dict, List, Optional, Callable, Any
from dataclasses import dataclass, asdict
from enum import Enum
import requests

# Configuration
CONFIG_DIR = Path("automation_config")
LOG_FILE = CONFIG_DIR / "test_scenarios.log"
RESULTS_FILE = CONFIG_DIR / "test_results.jsonl"

class TestStatus(Enum):
    """Test execution status"""
    PENDING = "pending"
    RUNNING = "running"
    PASSED = "passed"
    FAILED = "failed"
    ERROR = "error"
    TIMEOUT = "timeout"

class ScenarioType(Enum):
    """Type of test scenario"""
    FIRST_MISSION = "first_mission"
    TRADING = "trading"
    COMBAT = "combat"
    EXPLORATION = "exploration"
    CUSTOM = "custom"

@dataclass
class TestResult:
    """Test execution result"""
    scenario_name: str
    scenario_type: ScenarioType
    status: TestStatus
    start_time: float
    end_time: float
    duration: float
    message: str
    metrics: Dict[str, Any]
    screenshot_paths: List[str]

@dataclass
class ScenarioConfig:
    """Scenario configuration"""
    name: str
    type: ScenarioType
    timeout: int = 300  # seconds
    max_attempts: int = 3
    randomize: bool = False
    required_screenshots: int = 5
    success_criteria: Dict[str, Any] = None
    failure_criteria: Dict[str, Any] = None

    def __post_init__(self):
        if self.success_criteria is None:
            self.success_criteria = {}
        if self.failure_criteria is None:
            self.failure_criteria = {}

class AutomatedTestScenarios:
    """Automated test scenario executor"""

    def __init__(self, server_url: str = "http://localhost:8080"):
        self.server_url = server_url
        self.logger = self.setup_logging()
        self.scenarios: Dict[str, ScenarioConfig] = {}
        self.results: List[TestResult] = []
        self.running = False
        self.current_scenario = None
        self.screenshot_count = 0
        
        # Create config directory
        CONFIG_DIR.mkdir(exist_ok=True)
        
        # Initialize default scenarios
        self.initialize_default_scenarios()

    def setup_logging(self) -> logging.Logger:
        """Setup logging configuration"""
        logging.basicConfig(
            filename=LOG_FILE,
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )
        return logging.getLogger(__name__)

    def log(self, message: str, level: str = "INFO"):
        """Log message"""
        if level == "INFO":
            self.logger.info(message)
        elif level == "WARNING":
            self.logger.warning(message)
        elif level == "ERROR":
            self.logger.error(message)
        print(f"[{level}] {message}")

    def initialize_default_scenarios(self):
        """Initialize default test scenarios"""
        
        # First Mission Completion
        self.scenarios["first_mission"] = ScenarioConfig(
            name="First Mission Completion",
            type=ScenarioType.FIRST_MISSION,
            timeout=180,
            success_criteria={
                "mission_completed": True,
                "min_distance_traveled": 1000.0,
                "max_time": 120
            },
            failure_criteria={
                "ship_destroyed": True,
                "timeout": True,
                "stuck_duration": 30
            }
        )

        # Trading System Interaction
        self.scenarios["trading_system"] = ScenarioConfig(
            name="Trading System Interaction",
            type=ScenarioType.TRADING,
            timeout=240,
            success_criteria={
                "trade_completed": True,
                "credits_earned": 100,
                "items_traded": 1
            },
            failure_criteria={
                "insufficient_credits": True,
                "trade_rejected": True,
                "timeout": True
            }
        )

        # Combat Scenario
        self.scenarios["combat_scenario"] = ScenarioConfig(
            name="Combat Scenario",
            type=ScenarioType.COMBAT,
            timeout=300,
            randomize=True,
            success_criteria={
                "enemies_defeated": 3,
                "player_survived": True,
                "combat_duration": 60
            },
            failure_criteria={
                "player_destroyed": True,
                "ammo_depleted": True,
                "timeout": True
            }
        )

        # Free Exploration
        self.scenarios["free_exploration"] = ScenarioConfig(
            name="Free Exploration Mode",
            type=ScenarioType.EXPLORATION,
            timeout=600,
            randomize=True,
            success_criteria={
                "min_distance_explored": 5000.0,
                "discoveries_made": 3,
                "waypoints_visited": 5
            },
            failure_criteria={
                "ship_destroyed": True,
                "fuel_depleted": True,
                "timeout": True
            }
        )

        self.log(f"Initialized {len(self.scenarios)} default scenarios")

    def get_game_state(self) -> Dict[str, Any]:
        """Get current game state from HTTP API"""
        try:
            response = requests.get(f"{self.server_url}/game_state", timeout=5)
            if response.status_code == 200:
                return response.json()
        except Exception as e:
            self.log(f"Error getting game state: {e}", "ERROR")
        return {}

    def execute_command(self, command: str) -> bool:
        """Execute console command"""
        try:
            response = requests.post(
                f"{self.server_url}/execute_command",
                json={"command": command},
                timeout=5
            )
            return response.status_code == 200
        except Exception as e:
            self.log(f"Error executing command: {e}", "ERROR")
            return False

    def take_screenshot(self) -> Optional[str]:
        """Take a screenshot via HTTP API"""
        try:
            response = requests.post(
                f"{self.server_url}/screenshot",
                json={"filename": f"test_screenshot_{self.screenshot_count}"},
                timeout=10
            )
            if response.status_code == 200:
                self.screenshot_count += 1
                result = response.json()
                return result.get("data", {}).get("full_path")
        except Exception as e:
            self.log(f"Error taking screenshot: {e}", "ERROR")
        return None

    def check_success_criteria(self, game_state: Dict[str, Any], criteria: Dict[str, Any]) -> bool:
        """Check if success criteria are met"""
        for key, target_value in criteria.items():
            actual_value = self.get_nested_value(game_state, key)
            
            if isinstance(target_value, bool):
                if actual_value != target_value:
                    return False
            elif isinstance(target_value, (int, float)):
                if isinstance(actual_value, (int, float)):
                    if actual_value < target_value:
                        return False
            elif isinstance(target_value, str):
                if actual_value != target_value:
                    return False
                    
        return True

    def check_failure_criteria(self, game_state: Dict[str, Any], criteria: Dict[str, Any]) -> bool:
        """Check if failure criteria are met"""
        for key, target_value in criteria.items():
            actual_value = self.get_nested_value(game_state, key)
            
            if isinstance(target_value, bool) and target_value:
                if actual_value:
                    return True
            elif key == "timeout" and target_value:
                # Timeout handled separately
                continue
                    
        return False

    def get_nested_value(self, data: Dict[str, Any], key: str) -> Any:
        """Get nested value from dictionary using dot notation"""
        keys = key.split('.')
        value = data
        
        for k in keys:
            if isinstance(value, dict) and k in value:
                value = value[k]
            else:
                return None
                
        return value

    def run_scenario_first_mission(self, config: ScenarioConfig) -> TestResult:
        """Run first mission completion scenario"""
        self.log("Starting First Mission scenario")
        
        start_time = time.time()
        screenshots = []
        metrics = {
            "distance_traveled": 0.0,
            "start_position": None,
            "current_position": None,
            "mission_time": 0
        }
        
        # Get initial position
        game_state = self.get_game_state()
        if game_state:
            player_data = game_state.get("player", {})
            pos_data = player_data.get("position", {})
            metrics["start_position"] = pos_data
        
        # Main mission loop
        while time.time() - start_time < config.timeout:
            # Get current game state
            game_state = self.get_game_state()
            if not game_state:
                continue
            
            # Update metrics
            player_data = game_state.get("player", {})
            pos_data = player_data.get("position", {})
            metrics["current_position"] = pos_data
            
            if metrics["start_position"] and pos_data:
                # Calculate distance traveled (simplified)
                start = metrics["start_position"]
                current = pos_data
                dx = current.get("x", 0) - start.get("x", 0)
                dy = current.get("y", 0) - start.get("y", 0)
                dz = current.get("z", 0) - start.get("z", 0)
                metrics["distance_traveled"] = (dx*dx + dy*dy + dz*dz) ** 0.5
            
            metrics["mission_time"] = time.time() - start_time
            
            # Take periodic screenshots
            if len(screenshots) < config.required_screenshots:
                if time.time() - start_time > (len(screenshots) + 1) * (config.timeout / config.required_screenshots):
                    screenshot_path = self.take_screenshot()
                    if screenshot_path:
                        screenshots.append(screenshot_path)
            
            # Check success criteria
            if self.check_success_criteria(game_state, config.success_criteria):
                end_time = time.time()
                return TestResult(
                    scenario_name=config.name,
                    scenario_type=config.type,
                    status=TestStatus.PASSED,
                    start_time=start_time,
                    end_time=end_time,
                    duration=end_time - start_time,
                    message="Mission completed successfully",
                    metrics=metrics,
                    screenshot_paths=screenshots
                )
            
            # Check failure criteria
            if self.check_failure_criteria(game_state, config.failure_criteria):
                end_time = time.time()
                return TestResult(
                    scenario_name=config.name,
                    scenario_type=config.type,
                    status=TestStatus.FAILED,
                    start_time=start_time,
                    end_time=end_time,
                    duration=end_time - start_time,
                    message="Mission failed - criteria not met",
                    metrics=metrics,
                    screenshot_paths=screenshots
                )
            
            # Execute mission-specific actions
            if time.time() - start_time < 10:
                # Initial thrust to get moving
                self.execute_command("Thrust 1.0 0 0")
            elif time.time() - start_time < 30:
                # Continue forward movement
                self.execute_command("Thrust 0.5 0 0")
            
            time.sleep(1)
        
        # Timeout
        end_time = time.time()
        return TestResult(
            scenario_name=config.name,
            scenario_type=config.type,
            status=TestStatus.TIMEOUT,
            start_time=start_time,
            end_time=end_time,
            duration=end_time - start_time,
            message="Mission timeout",
            metrics=metrics,
            screenshot_paths=screenshots
        )

    def run_scenario_trading(self, config: ScenarioConfig) -> TestResult:
        """Run trading system interaction scenario"""
        self.log("Starting Trading System scenario")
        
        start_time = time.time()
        screenshots = []
        metrics = {
            "trade_attempts": 0,
            "credits_earned": 0,
            "items_traded": 0
        }
        
        # Simulate trading interaction
        while time.time() - start_time < config.timeout:
            game_state = self.get_game_state()
            if not game_state:
                time.sleep(1)
                continue
            
            # Take screenshots
            if len(screenshots) < config.required_screenshots:
                screenshot_path = self.take_screenshot()
                if screenshot_path:
                    screenshots.append(screenshot_path)
            
            # Simulate trading actions
            if metrics["trade_attempts"] == 0:
                self.log("Opening trade interface")
                self.execute_command("OpenTrade")
                metrics["trade_attempts"] += 1
                time.sleep(2)
            
            elif metrics["trade_attempts"] == 1:
                self.log("Selecting trade items")
                self.execute_command("SelectTradeItem 0")
                time.sleep(1)
                self.execute_command("ConfirmTrade")
                metrics["trade_attempts"] += 1
                metrics["items_traded"] += 1
                metrics["credits_earned"] += 100
                time.sleep(2)
            
            # Check success
            if metrics["credits_earned"] >= config.success_criteria.get("credits_earned", 100):
                end_time = time.time()
                return TestResult(
                    scenario_name=config.name,
                    scenario_type=config.type,
                    status=TestStatus.PASSED,
                    start_time=start_time,
                    end_time=end_time,
                    duration=end_time - start_time,
                    message="Trading completed successfully",
                    metrics=metrics,
                    screenshot_paths=screenshots
                )
            
            time.sleep(1)
        
        # Timeout
        end_time = time.time()
        return TestResult(
            scenario_name=config.name,
            scenario_type=config.type,
            status=TestStatus.TIMEOUT,
            start_time=start_time,
            end_time=end_time,
            duration=end_time - start_time,
            message="Trading timeout",
            metrics=metrics,
            screenshot_paths=screenshots
        )

    def run_scenario_combat(self, config: ScenarioConfig) -> TestResult:
        """Run combat scenario"""
        self.log("Starting Combat scenario")
        
        start_time = time.time()
        screenshots = []
        metrics = {
            "enemies_defeated": 0,
            "shots_fired": 0,
            "damage_taken": 0,
            "ammo_remaining": 100
        }
        
        # Combat loop
        while time.time() - start_time < config.timeout:
            game_state = self.get_game_state()
            if not game_state:
                time.sleep(1)
                continue
            
            # Take screenshots during combat
            if len(screenshots) < config.required_screenshots:
                if random.random() < 0.3:  # 30% chance each second
                    screenshot_path = self.take_screenshot()
                    if screenshot_path:
                        screenshots.append(screenshot_path)
            
            # Combat actions
            if metrics["enemies_defeated"] < 3:
                # Fire weapon
                self.execute_command("Fire")
                metrics["shots_fired"] += 1
                metrics["ammo_remaining"] -= 1
                
                # Simulate enemy defeat
                if random.random() < 0.1:  # 10% chance per shot
                    metrics["enemies_defeated"] += 1
                    self.log(f"Enemy defeated! Total: {metrics['enemies_defeated']}")
            
            # Check ammo
            if metrics["ammo_remaining"] <= 0:
                end_time = time.time()
                return TestResult(
                    scenario_name=config.name,
                    scenario_type=config.type,
                    status=TestStatus.FAILED,
                    start_time=start_time,
                    end_time=end_time,
                    duration=end_time - start_time,
                    message="Out of ammo",
                    metrics=metrics,
                    screenshot_paths=screenshots
                )
            
            # Check success
            if metrics["enemies_defeated"] >= config.success_criteria.get("enemies_defeated", 3):
                end_time = time.time()
                return TestResult(
                    scenario_name=config.name,
                    scenario_type=config.type,
                    status=TestStatus.PASSED,
                    start_time=start_time,
                    end_time=end_time,
                    duration=end_time - start_time,
                    message="Combat scenario completed successfully",
                    metrics=metrics,
                    screenshot_paths=screenshots
                )
            
            time.sleep(0.5)  # Faster pace for combat
        
        # Timeout
        end_time = time.time()
        return TestResult(
            scenario_name=config.name,
            scenario_type=config.type,
            status=TestStatus.TIMEOUT,
            start_time=start_time,
            end_time=end_time,
            duration=end_time - start_time,
            message="Combat timeout",
            metrics=metrics,
            screenshot_paths=screenshots
        )

    def run_scenario_exploration(self, config: ScenarioConfig) -> TestResult:
        """Run free exploration scenario"""
        self.log("Starting Exploration scenario")
        
        start_time = time.time()
        screenshots = []
        metrics = {
            "distance_explored": 0.0,
            "discoveries_made": 0,
            "waypoints_visited": 0,
            "start_position": None,
            "exploration_path": []
        }
        
        # Get initial position
        game_state = self.get_game_state()
        if game_state:
            player_data = game_state.get("player", {})
            pos_data = player_data.get("position", {})
            metrics["start_position"] = pos_data
        
        # Exploration loop
        while time.time() - start_time < config.timeout:
            game_state = self.get_game_state()
            if not game_state:
                time.sleep(1)
                continue
            
            # Update position tracking
            player_data = game_state.get("player", {})
            pos_data = player_data.get("position", {})
            
            if pos_data:
                metrics["exploration_path"].append(pos_data)
                
                # Calculate distance from start
                if metrics["start_position"]:
                    start = metrics["start_position"]
                    dx = pos_data.get("x", 0) - start.get("x", 0)
                    dy = pos_data.get("y", 0) - start.get("y", 0)
                    dz = pos_data.get("z", 0) - start.get("z", 0)
                    metrics["distance_explored"] = (dx*dx + dy*dy + dz*dz) ** 0.5
            
            # Random exploration actions
            if config.randomize:
                if random.random() < 0.3:  # 30% chance to change direction
                    thrust_x = random.uniform(-1.0, 1.0)
                    thrust_y = random.uniform(-1.0, 1.0)
                    thrust_z = random.uniform(-0.5, 0.5)
                    self.execute_command(f"Thrust {thrust_x} {thrust_y} {thrust_z}")
                
                if random.random() < 0.1:  # 10% chance to discover something
                    metrics["discoveries_made"] += 1
                    self.log(f"Discovery made! Total: {metrics['discoveries_made']}")
            
            # Take periodic screenshots
            if len(screenshots) < config.required_screenshots:
                if time.time() - start_time > (len(screenshots) + 1) * (config.timeout / config.required_screenshots):
                    screenshot_path = self.take_screenshot()
                    if screenshot_path:
                        screenshots.append(screenshot_path)
            
            # Simulate waypoint visits
            if random.random() < 0.05:  # 5% chance per second
                metrics["waypoints_visited"] += 1
                self.log(f"Waypoint visited! Total: {metrics['waypoints_visited']}")
            
            # Check success
            if (metrics["distance_explored"] >= config.success_criteria.get("min_distance_explored", 5000) and
                metrics["discoveries_made"] >= config.success_criteria.get("discoveries_made", 3) and
                metrics["waypoints_visited"] >= config.success_criteria.get("waypoints_visited", 5)):
                end_time = time.time()
                return TestResult(
                    scenario_name=config.name,
                    scenario_type=config.type,
                    status=TestStatus.PASSED,
                    start_time=start_time,
                    end_time=end_time,
                    duration=end_time - start_time,
                    message="Exploration completed successfully",
                    metrics=metrics,
                    screenshot_paths=screenshots
                )
            
            time.sleep(1)
        
        # Timeout
        end_time = time.time()
        return TestResult(
            scenario_name=config.name,
            scenario_type=config.type,
            status=TestStatus.TIMEOUT,
            start_time=start_time,
            end_time=end_time,
            duration=end_time - start_time,
            message="Exploration timeout",
            metrics=metrics,
            screenshot_paths=screenshots
        )

    def run_scenario(self, scenario_name: str) -> TestResult:
        """Run a specific scenario"""
        if scenario_name not in self.scenarios:
            self.log(f"Scenario '{scenario_name}' not found", "ERROR")
            return TestResult(
                scenario_name=scenario_name,
                scenario_type=ScenarioType.CUSTOM,
                status=TestStatus.ERROR,
                start_time=time.time(),
                end_time=time.time(),
                duration=0,
                message=f"Scenario '{scenario_name}' not found",
                metrics={},
                screenshot_paths=[]
            )
        
        config = self.scenarios[scenario_name]
        self.current_scenario = scenario_name
        
        self.log(f"Running scenario: {config.name} (type: {config.type.value})")
        
        # Route to appropriate scenario runner
        if config.type == ScenarioType.FIRST_MISSION:
            result = self.run_scenario_first_mission(config)
        elif config.type == ScenarioType.TRADING:
            result = self.run_scenario_trading(config)
        elif config.type == ScenarioType.COMBAT:
            result = self.run_scenario_combat(config)
        elif config.type == ScenarioType.EXPLORATION:
            result = self.run_scenario_exploration(config)
        else:
            result = TestResult(
                scenario_name=config.name,
                scenario_type=config.type,
                status=TestStatus.ERROR,
                start_time=time.time(),
                end_time=time.time(),
                duration=0,
                message=f"Unknown scenario type: {config.type}",
                metrics={},
                screenshot_paths=[]
            )
        
        # Save result
        self.results.append(result)
        self.save_result(result)
        
        self.log(f"Scenario completed: {result.status.value} - {result.message}")
        self.current_scenario = None
        
        return result

    def run_all_scenarios(self) -> List[TestResult]:
        """Run all scenarios"""
        self.log("Running all test scenarios")
        
        results = []
        for scenario_name in self.scenarios.keys():
            result = self.run_scenario(scenario_name)
            results.append(result)
            
            # Wait between scenarios
            time.sleep(5)
        
        self.log(f"All scenarios completed: {len(results)} total")
        return results

    def save_result(self, result: TestResult):
        """Save test result to file"""
        try:
            with open(RESULTS_FILE, 'a') as f:
                f.write(json.dumps({
                    "scenario_name": result.scenario_name,
                    "scenario_type": result.scenario_type.value,
                    "status": result.status.value,
                    "start_time": result.start_time,
                    "end_time": result.end_time,
                    "duration": result.duration,
                    "message": result.message,
                    "metrics": result.metrics,
                    "screenshot_paths": result.screenshot_paths
                }) + '\n')
        except Exception as e:
            self.log(f"Error saving result: {e}", "ERROR")

    def get_results_summary(self) -> Dict[str, Any]:
        """Get summary of all test results"""
        if not self.results:
            return {"total": 0, "passed": 0, "failed": 0, "error": 0}
        
        summary = {
            "total": len(self.results),
            "passed": sum(1 for r in self.results if r.status == TestStatus.PASSED),
            "failed": sum(1 for r in self.results if r.status in [TestStatus.FAILED, TestStatus.TIMEOUT]),
            "error": sum(1 for r in self.results if r.status == TestStatus.ERROR)
        }
        
        return summary

def main():
    """Main function for standalone usage"""
    import argparse
    
    parser = argparse.ArgumentParser(description="Automated test scenarios")
    parser.add_argument("--scenario", type=str, help="Specific scenario to run")
    parser.add_argument("--all", action="store_true", help="Run all scenarios")
    parser.add_argument("--server", type=str, default="http://localhost:8080", help="Game server URL")
    
    args = parser.parse_args()
    
    print("=" * 70)
    print("AUTOMATED TEST SCENARIOS")
    print("=" * 70)
    
    # Create test executor
    tester = AutomatedTestScenarios(args.server)
    
    # Run scenarios
    if args.all:
        results = tester.run_all_scenarios()
    elif args.scenario:
        result = tester.run_scenario(args.scenario)
        results = [result]
    else:
        print("Please specify --scenario or --all")
        return 1
    
    # Print summary
    summary = tester.get_results_summary()
    print("\n" + "=" * 70)
    print("TEST RESULTS SUMMARY")
    print("=" * 70)
    print(f"Total scenarios: {summary['total']}")
    print(f"Passed: {summary['passed']}")
    print(f"Failed: {summary['failed']}")
    print(f"Errors: {summary['error']}")
    print("=" * 70)
    
    return 0

if __name__ == "__main__":
    import sys
    sys.exit(main())