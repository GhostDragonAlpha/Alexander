#!/usr/bin/env python3
"""
Load Test Controller
Orchestrates multiple test clients and manages load test scenarios
"""

import json
import time
import threading
import logging
from typing import Dict, List, Optional
from datetime import datetime
from test_client import TestClient, create_test_client

class LoadTestController:
    def __init__(self, config_file: str = "automation_config/load_test_scenarios.json"):
        self.config_file = config_file
        self.config = self._load_config()
        self.clients: List[TestClient] = []
        self.test_results = {}
        self.is_running = False
        self.start_time = None
        
        # Setup logging
        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
        )
        self.logger = logging.getLogger("LoadTestController")
        
    def _load_config(self) -> Dict:
        """Load configuration from JSON file"""
        try:
            with open(self.config_file, 'r') as f:
                return json.load(f)
        except Exception as e:
            self.logger.error(f"Failed to load config: {e}")
            return {}
    
    def check_server_health(self) -> bool:
        """Check if game server is healthy"""
        # Create a temporary client for health check
        temp_client = create_test_client("health_check")
        return temp_client.check_server_health()
    
    def create_clients(self, scenario_name: str) -> bool:
        """Create test clients for a scenario"""
        scenarios = self.config.get("load_test_scenarios", {})
        scenario = scenarios.get(scenario_name)
        
        if not scenario:
            self.logger.error(f"Scenario '{scenario_name}' not found")
            return False
        
        player_count = scenario.get("player_count", 1)
        self.logger.info(f"Creating {player_count} test clients for scenario '{scenario_name}'")
        
        # Clear existing clients
        self.clients.clear()
        
        # Create new clients
        for i in range(player_count):
            client_id = f"load_test_client_{i:03d}"
            client = create_test_client(client_id)
            self.clients.append(client)
        
        self.logger.info(f"Successfully created {len(self.clients)} clients")
        return True
    
    def start_scenario(self, scenario_name: str) -> bool:
        """Start executing a load test scenario"""
        scenarios = self.config.get("load_test_scenarios", {})
        scenario = scenarios.get(scenario_name)
        
        if not scenario:
            self.logger.error(f"Scenario '{scenario_name}' not found")
            return False
        
        # Check server health first
        if not self.check_server_health():
            self.logger.error("Server health check failed")
            return False
        
        # Create clients
        if not self.create_clients(scenario_name):
            return False
        
        self.logger.info(f"Starting scenario '{scenario_name}'")
        self.is_running = True
        self.start_time = time.time()
        
        # Start client threads
        threads = []
        ramp_up = scenario.get("ramp_up_seconds", 30)
        interval = ramp_up / len(self.clients) if len(self.clients) > 0 else 0
        
        for i, client in enumerate(self.clients):
            # Stagger client start times for ramp-up
            def start_client_delayed(client, delay):
                time.sleep(delay)
                if self.is_running:
                    client.run_scenario(scenario)
            
            thread = threading.Thread(
                target=start_client_delayed,
                args=(client, i * interval)
            )
            thread.daemon = True
            thread.start()
            threads.append(thread)
        
        # Monitor scenario progress
        self._monitor_scenario(scenario)
        
        return True
    
    def _monitor_scenario(self, scenario: Dict):
        """Monitor scenario execution and collect metrics"""
        duration = scenario.get("duration_seconds", 300)
        pass_criteria = scenario.get("pass_criteria", {})
        
        self.logger.info(f"Monitoring scenario for {duration} seconds")
        
        while self.is_running and (time.time() - self.start_time) < duration:
            time.sleep(10)  # Check every 10 seconds
            
            # Collect metrics from all clients
            all_stats = self.get_all_client_stats()
            self._log_progress(all_stats)
            
            # Check if we should stop early
            if self._should_stop_early(all_stats, pass_criteria):
                self.logger.warning("Stopping scenario early due to failure criteria")
                self.stop()
                break
        
        self.logger.info("Scenario monitoring completed")
    
    def _log_progress(self, stats: List[Dict]):
        """Log current progress"""
        if not stats:
            return
        
        total_actions = sum(s.get("action_count", 0) for s in stats)
        total_errors = sum(s.get("error_count", 0) for s in stats)
        active_clients = sum(1 for s in stats if s.get("is_running", False))
        
        self.logger.info(
            f"Progress - Clients: {active_clients}/{len(self.clients)}, "
            f"Actions: {total_actions}, Errors: {total_errors}"
        )
    
    def _should_stop_early(self, stats: List[Dict], pass_criteria: Dict) -> bool:
        """Check if scenario should stop early due to failures"""
        if not stats:
            return True
        
        # Check server stability
        if not pass_criteria.get("server_stability", True):
            return True
        
        # Check if too many clients have failed
        failed_clients = sum(1 for s in stats if s.get("error_count", 0) > 10)
        if failed_clients > len(self.clients) * 0.5:  # More than 50% failed
            self.logger.error(f"Too many clients failed: {failed_clients}/{len(self.clients)}")
            return True
        
        return False
    
    def get_all_client_stats(self) -> List[Dict]:
        """Get statistics from all clients"""
        return [client.get_stats() for client in self.clients]
    
    def generate_report(self, scenario_name: str) -> Dict:
        """Generate test report"""
        stats = self.get_all_client_stats()
        
        if not stats:
            return {"error": "No client statistics available"}
        
        # Aggregate metrics
        total_actions = sum(s.get("action_count", 0) for s in stats)
        total_errors = sum(s.get("error_count", 0) for s in stats)
        total_runtime = sum(s.get("runtime_seconds", 0) for s in stats) / len(stats)
        
        # Performance metrics
        all_fps = []
        all_memory = []
        all_latency = []
        
        for s in stats:
            metrics = s.get("performance_metrics", {})
            all_fps.extend(metrics.get("fps", []))
            all_memory.extend(metrics.get("memory_mb", []))
            all_latency.extend(metrics.get("latency_ms", []))
        
        # Calculate pass/fail
        scenarios = self.config.get("load_test_scenarios", {})
        scenario = scenarios.get(scenario_name, {})
        pass_criteria = scenario.get("pass_criteria", {})
        
        passed = self._evaluate_pass_criteria(
            pass_criteria, all_fps, all_memory, all_latency, total_errors
        )
        
        report = {
            "scenario_name": scenario_name,
            "timestamp": datetime.now().isoformat(),
            "duration_seconds": total_runtime,
            "client_count": len(self.clients),
            "total_actions": total_actions,
            "total_errors": total_errors,
            "error_rate": total_errors / max(total_actions, 1),
            "performance_metrics": {
                "avg_fps": sum(all_fps) / max(len(all_fps), 1),
                "avg_memory_mb": sum(all_memory) / max(len(all_memory), 1),
                "avg_latency_ms": sum(all_latency) / max(len(all_latency), 1),
                "min_fps": min(all_fps) if all_fps else 0,
                "max_memory_mb": max(all_memory) if all_memory else 0,
                "max_latency_ms": max(all_latency) if all_latency else 0
            },
            "pass_criteria": pass_criteria,
            "passed": passed,
            "client_stats": stats
        }
        
        return report
    
    def _evaluate_pass_criteria(self, pass_criteria: Dict, fps: List[float], 
                               memory: List[float], latency: List[float], 
                               errors: int) -> bool:
        """Evaluate if test passed based on criteria"""
        if not pass_criteria:
            return True
        
        # Check FPS
        if fps and pass_criteria.get("min_fps"):
            avg_fps = sum(fps) / len(fps)
            if avg_fps < pass_criteria["min_fps"]:
                self.logger.warning(f"FPS criteria failed: {avg_fps} < {pass_criteria['min_fps']}")
                return False
        
        # Check memory
        if memory and pass_criteria.get("max_memory_mb"):
            max_memory = max(memory)
            if max_memory > pass_criteria["max_memory_mb"]:
                self.logger.warning(f"Memory criteria failed: {max_memory} > {pass_criteria['max_memory_mb']}")
                return False
        
        # Check latency
        if latency and pass_criteria.get("max_latency_ms"):
            avg_latency = sum(latency) / len(latency)
            if avg_latency > pass_criteria["max_latency_ms"]:
                self.logger.warning(f"Latency criteria failed: {avg_latency} > {pass_criteria['max_latency_ms']}")
                return False
        
        # Check server stability
        if not pass_criteria.get("server_stability", True):
            self.logger.warning("Server stability criteria failed")
            return False
        
        return True
    
    def save_report(self, report: Dict, filename: Optional[str] = None):
        """Save report to file"""
        if not filename:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = f"load_test_report_{timestamp}.json"
        
        try:
            with open(filename, 'w') as f:
                json.dump(report, f, indent=2)
            self.logger.info(f"Report saved to {filename}")
        except Exception as e:
            self.logger.error(f"Failed to save report: {e}")
    
    def stop(self):
        """Stop all clients and test"""
        self.logger.info("Stopping load test")
        self.is_running = False
        
        for client in self.clients:
            client.stop()
        
        self.logger.info("Load test stopped")
    
    def list_scenarios(self) -> List[str]:
        """List available scenarios"""
        scenarios = self.config.get("load_test_scenarios", {})
        return list(scenarios.keys())

def run_load_test(scenario_name: str, config_file: str = "automation_config/load_test_scenarios.json") -> Dict:
    """Run a complete load test scenario"""
    controller = LoadTestController(config_file)
    
    try:
        # Start scenario
        if not controller.start_scenario(scenario_name):
            return {"error": "Failed to start scenario"}
        
        # Wait for completion
        scenarios = controller.config.get("load_test_scenarios", {})
        scenario = scenarios.get(scenario_name, {})
        duration = scenario.get("duration_seconds", 300)
        
        time.sleep(duration + 30)  # Add buffer for cleanup
        
        # Generate and return report
        report = controller.generate_report(scenario_name)
        controller.save_report(report)
        
        return report
        
    except Exception as e:
        controller.logger.error(f"Load test failed: {e}")
        return {"error": str(e)}
    finally:
        controller.stop()

if __name__ == "__main__":
    # List available scenarios
    controller = LoadTestController()
    scenarios = controller.list_scenarios()
    print("Available scenarios:", scenarios)
    
    # Quick test
    if "quick" in scenarios:
        print("\nRunning quick test...")
        report = run_load_test("quick")
        print(f"Test passed: {report.get('passed', False)}")
        print(f"Total actions: {report.get('total_actions', 0)}")
        print(f"Error rate: {report.get('error_rate', 0):.2%}")