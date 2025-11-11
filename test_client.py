#!/usr/bin/env python3
"""
Test Client Simulator for Load Testing
Simulates player actions and communicates with game server via HTTP API
"""

import json
import time
import random
import requests
import threading
from typing import Dict, List, Optional
import logging

class TestClient:
    def __init__(self, client_id: str, server_url: str = "http://localhost:8080", 
                 config_file: str = "automation_config/load_test_scenarios.json"):
        self.client_id = client_id
        self.server_url = server_url
        self.config = self._load_config(config_file)
        self.client_settings = self.config.get("client_settings", {})
        self.is_running = False
        self.current_action = "idle"
        self.action_count = 0
        self.error_count = 0
        self.start_time = None
        self.performance_metrics = {
            "fps": [],
            "memory_mb": [],
            "latency_ms": []
        }
        
        # Setup logging
        logging.basicConfig(level=logging.INFO)
        self.logger = logging.getLogger(f"TestClient-{client_id}")
        
    def _load_config(self, config_file: str) -> Dict:
        """Load configuration from JSON file"""
        try:
            with open(config_file, 'r') as f:
                return json.load(f)
        except Exception as e:
            self.logger.error(f"Failed to load config: {e}")
            return {}
    
    def _make_request(self, endpoint: str, data: Dict = None) -> Optional[Dict]:
        """Make HTTP request to server with error handling"""
        try:
            url = f"{self.server_url}{endpoint}"
            timeout = self.client_settings.get("http_timeout_seconds", 10)
            
            if data:
                response = requests.post(url, json=data, timeout=timeout)
            else:
                response = requests.get(url, timeout=timeout)
            
            response.raise_for_status()
            return response.json()
        except Exception as e:
            self.error_count += 1
            self.logger.warning(f"Request failed: {e}")
            return None
    
    def check_server_health(self) -> bool:
        """Check if server is healthy"""
        health_config = self.config.get("server_health_check", {})
        endpoint = health_config.get("endpoint", "/health")
        
        for attempt in range(health_config.get("retry_attempts", 3)):
            result = self._make_request(endpoint)
            if result and result.get("status") == "healthy":
                return True
            time.sleep(health_config.get("retry_delay_seconds", 2))
        
        return False
    
    def get_game_state(self) -> Optional[Dict]:
        """Get current game state from server"""
        return self._make_request("/game_state")
    
    def perform_movement(self) -> bool:
        """Simulate player movement"""
        movement_data = {
            "client_id": self.client_id,
            "action": "move",
            "position": {
                "x": random.uniform(-10000, 10000),
                "y": random.uniform(-10000, 10000),
                "z": random.uniform(-10000, 10000)
            },
            "velocity": {
                "x": random.uniform(-500, 500),
                "y": random.uniform(-500, 500),
                "z": random.uniform(-500, 500)
            }
        }
        
        result = self._make_request("/player_action", movement_data)
        return result is not None
    
    def perform_combat(self) -> bool:
        """Simulate combat action"""
        combat_data = {
            "client_id": self.client_id,
            "action": "combat",
            "target_id": f"target_{random.randint(1, 100)}",
            "weapon_type": random.choice(["laser", "missile", "plasma"]),
            "position": {
                "x": random.uniform(-2000, 2000),
                "y": random.uniform(-2000, 2000),
                "z": random.uniform(-2000, 2000)
            }
        }
        
        result = self._make_request("/player_action", combat_data)
        return result is not None
    
    def perform_trading(self) -> bool:
        """Simulate trading action"""
        trading_data = {
            "client_id": self.client_id,
            "action": "trade",
            "trade_type": random.choice(["buy", "sell"]),
            "item_id": f"item_{random.randint(1, 50)}",
            "quantity": random.randint(1, 10),
            "price": random.uniform(10, 1000)
        }
        
        result = self._make_request("/player_action", trading_data)
        return result is not None
    
    def perform_exploration(self) -> bool:
        """Simulate exploration action"""
        exploration_data = {
            "client_id": self.client_id,
            "action": "explore",
            "area_id": f"area_{random.randint(1, 20)}",
            "discovery_type": random.choice(["asteroid", "derelict", "anomaly", "resource"]),
            "position": {
                "x": random.uniform(-10000, 10000),
                "y": random.uniform(-10000, 10000),
                "z": random.uniform(-10000, 10000)
            }
        }
        
        result = self._make_request("/player_action", exploration_data)
        return result is not None
    
    def perform_mission(self) -> bool:
        """Simulate mission action"""
        mission_data = {
            "client_id": self.client_id,
            "action": "mission",
            "mission_id": f"mission_{random.randint(1, 30)}",
            "mission_type": random.choice(["delivery", "combat", "exploration", "escort"]),
            "status": random.choice(["accept", "progress", "complete"])
        }
        
        result = self._make_request("/player_action", mission_data)
        return result is not None
    
    def collect_performance_metrics(self):
        """Collect performance metrics from server"""
        metrics = self._make_request("/performance_metrics")
        if metrics:
            self.performance_metrics["fps"].append(metrics.get("fps", 0))
            self.performance_metrics["memory_mb"].append(metrics.get("memory_mb", 0))
            self.performance_metrics["latency_ms"].append(metrics.get("latency_ms", 0))
    
    def run_action(self, action_type: str) -> bool:
        """Execute a specific action type"""
        self.current_action = action_type
        self.action_count += 1
        
        action_map = {
            "combat": self.perform_combat,
            "trading": self.perform_trading,
            "exploration": self.perform_exploration,
            "mission": self.perform_mission
        }
        
        action_func = action_map.get(action_type, self.perform_movement)
        
        try:
            success = action_func()
            if success:
                self.logger.debug(f"Client {self.client_id}: {action_type} action completed")
            else:
                self.logger.warning(f"Client {self.client_id}: {action_type} action failed")
            return success
        except Exception as e:
            self.logger.error(f"Client {self.client_id}: Action error - {e}")
            self.error_count += 1
            return False
    
    def run_scenario(self, scenario_config: Dict):
        """Run a complete test scenario"""
        self.is_running = True
        self.start_time = time.time()
        
        if not self.check_server_health():
            self.logger.error(f"Client {self.client_id}: Server health check failed")
            self.is_running = False
            return
        
        self.logger.info(f"Client {self.client_id}: Starting scenario with config: {scenario_config}")
        
        action_dist = scenario_config.get("action_distribution", {})
        action_types = list(action_dist.keys())
        action_weights = list(action_dist.values())
        
        interval = self.client_settings.get("action_interval_seconds", 2.0)
        duration = scenario_config.get("duration_seconds", 300)
        
        while self.is_running and (time.time() - self.start_time) < duration:
            # Select action based on distribution
            action_type = random.choices(action_types, weights=action_weights, k=1)[0]
            
            # Perform action
            self.run_action(action_type)
            
            # Collect metrics periodically
            if self.action_count % 10 == 0:
                self.collect_performance_metrics()
            
            # Wait before next action
            time.sleep(interval)
        
        self.is_running = False
        self.logger.info(f"Client {self.client_id}: Scenario completed. Actions: {self.action_count}, Errors: {self.error_count}")
    
    def get_stats(self) -> Dict:
        """Get client statistics"""
        runtime = time.time() - self.start_time if self.start_time else 0
        
        return {
            "client_id": self.client_id,
            "is_running": self.is_running,
            "action_count": self.action_count,
            "error_count": self.error_count,
            "runtime_seconds": runtime,
            "current_action": self.current_action,
            "performance_metrics": self.performance_metrics
        }
    
    def stop(self):
        """Stop the client"""
        self.is_running = False
        self.logger.info(f"Client {self.client_id}: Stopped")

def create_test_client(client_id: str, server_url: str = "http://localhost:8080") -> TestClient:
    """Factory function to create a test client"""
    return TestClient(client_id, server_url)

if __name__ == "__main__":
    # Simple test
    client = create_test_client("test_client_001")
    
    # Quick health check
    if client.check_server_health():
        print("Server is healthy")
        
        # Run a quick action
        client.run_action("exploration")
        print(f"Client stats: {client.get_stats()}")
    else:
        print("Server health check failed")