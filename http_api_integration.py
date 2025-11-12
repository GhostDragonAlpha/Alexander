#!/usr/bin/env python3
"""
HTTP API Integration for Unreal Engine Automation
Connects to Unreal Engine HTTP API on Port 8080
Implements ship spawning, input control, and state queries
"""

import json
import time
import logging
import requests
from typing import Dict, List, Optional, Any, Tuple
from dataclasses import dataclass, field
from enum import Enum
from datetime import datetime

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class HTTPAPIStatus(Enum):
    """HTTP API connection status"""
    DISCONNECTED = "disconnected"
    CONNECTING = "connecting"
    CONNECTED = "connected"
    ERROR = "error"


@dataclass
class ShipState:
    """Represents ship state from Unreal Engine"""
    ship_id: str
    position: Tuple[float, float, float]
    rotation: Tuple[float, float, float]
    velocity: Tuple[float, float, float]
    health: float
    shield: float
    is_alive: bool
    timestamp: datetime = field(default_factory=datetime.now)
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "ship_id": self.ship_id,
            "position": self.position,
            "rotation": self.rotation,
            "velocity": self.velocity,
            "health": self.health,
            "shield": self.shield,
            "is_alive": self.is_alive,
            "timestamp": self.timestamp.isoformat()
        }


@dataclass
class APIResponse:
    """HTTP API response wrapper"""
    success: bool
    data: Optional[Dict[str, Any]] = None
    error_message: Optional[str] = None
    status_code: Optional[int] = None
    response_time: float = 0.0


class UnrealHTTPAPIClient:
    """
    HTTP API Client for Unreal Engine Automation
    Connects to Unreal Engine's built-in HTTP server
    """
    
    def __init__(self, host: str = "localhost", port: int = 8080, timeout: int = 10):
        """
        Initialize HTTP API client
        
        Args:
            host: Host address (default: localhost)
            port: Port number (default: 8080)
            timeout: Request timeout in seconds
        """
        self.host = host
        self.port = port
        self.timeout = timeout
        self.base_url = f"http://{host}:{port}"
        self.session = requests.Session()
        self.is_connected = False
        self.last_health_check = None
        
        logger.info(f"Unreal HTTP API client initialized for {self.base_url}")
    
    def health_check(self) -> APIResponse:
        """
        Check if Unreal Engine HTTP API is available
        
        Returns:
            APIResponse with connection status
        """
        start_time = time.time()
        
        try:
            response = self.session.get(
                f"{self.base_url}/health",
                timeout=self.timeout
            )
            response_time = time.time() - start_time
            
            if response.status_code == 200:
                self.is_connected = True
                self.last_health_check = datetime.now()
                
                return APIResponse(
                    success=True,
                    data=response.json(),
                    status_code=200,
                    response_time=response_time
                )
            else:
                self.is_connected = False
                return APIResponse(
                    success=False,
                    error_message=f"Health check failed: {response.status_code}",
                    status_code=response.status_code,
                    response_time=response_time
                )
                
        except requests.exceptions.ConnectionError:
            self.is_connected = False
            return APIResponse(
                success=False,
                error_message="Connection refused - Unreal Engine may not be running",
                status_code=None,
                response_time=time.time() - start_time
            )
        except Exception as e:
            self.is_connected = False
            return APIResponse(
                success=False,
                error_message=f"Health check error: {str(e)}",
                status_code=None,
                response_time=time.time() - start_time
            )
    
    def spawn_ship(self, ship_class: str, location: Tuple[float, float, float], 
                   rotation: Tuple[float, float, float] = (0, 0, 0)) -> APIResponse:
        """
        Spawn a ship in Unreal Engine
        
        Args:
            ship_class: Ship class name (e.g., "BP_Spaceship_C")
            location: Spawn location (x, y, z)
            rotation: Spawn rotation (pitch, yaw, roll)
            
        Returns:
            APIResponse with spawn result
        """
        if not self.is_connected:
            return APIResponse(success=False, error_message="Not connected to Unreal Engine")
        
        start_time = time.time()
        
        try:
            payload = {
                "command": "spawn_actor",
                "parameters": {
                    "class_name": ship_class,
                    "location": {"x": location[0], "y": location[1], "z": location[2]},
                    "rotation": {"pitch": rotation[0], "yaw": rotation[1], "roll": rotation[2]}
                }
            }
            
            response = self.session.post(
                f"{self.base_url}/execute",
                json=payload,
                timeout=self.timeout
            )
            response_time = time.time() - start_time
            
            if response.status_code == 200:
                result = response.json()
                if result.get("success"):
                    return APIResponse(
                        success=True,
                        data=result,
                        status_code=200,
                        response_time=response_time
                    )
                else:
                    return APIResponse(
                        success=False,
                        error_message=result.get("error", "Unknown spawn error"),
                        status_code=200,
                        response_time=response_time
                    )
            else:
                return APIResponse(
                    success=False,
                    error_message=f"HTTP {response.status_code}: {response.text}",
                    status_code=response.status_code,
                    response_time=response_time
                )
                
        except Exception as e:
            return APIResponse(
                success=False,
                error_message=f"Spawn error: {str(e)}",
                status_code=None,
                response_time=time.time() - start_time
            )
    
    def send_input(self, ship_id: str, input_type: str, value: float) -> APIResponse:
        """
        Send input control to a ship
        
        Args:
            ship_id: Ship identifier
            input_type: Input type (throttle, pitch, yaw, roll, fire)
            value: Input value (0.0 to 1.0 for most inputs)
            
        Returns:
            APIResponse with input result
        """
        if not self.is_connected:
            return APIResponse(success=False, error_message="Not connected to Unreal Engine")
        
        start_time = time.time()
        
        try:
            payload = {
                "command": "ship_input",
                "parameters": {
                    "ship_id": ship_id,
                    "input_type": input_type,
                    "value": value
                }
            }
            
            response = self.session.post(
                f"{self.base_url}/execute",
                json=payload,
                timeout=self.timeout
            )
            response_time = time.time() - start_time
            
            if response.status_code == 200:
                return APIResponse(
                    success=True,
                    data=response.json(),
                    status_code=200,
                    response_time=response_time
                )
            else:
                return APIResponse(
                    success=False,
                    error_message=f"HTTP {response.status_code}: {response.text}",
                    status_code=response.status_code,
                    response_time=response_time
                )
                
        except Exception as e:
            return APIResponse(
                success=False,
                error_message=f"Input error: {str(e)}",
                status_code=None,
                response_time=time.time() - start_time
            )
    
    def get_ship_state(self, ship_id: str) -> APIResponse:
        """
        Get current state of a ship
        
        Args:
            ship_id: Ship identifier
            
        Returns:
            APIResponse with ship state
        """
        if not self.is_connected:
            return APIResponse(success=False, error_message="Not connected to Unreal Engine")
        
        start_time = time.time()
        
        try:
            response = self.session.get(
                f"{self.base_url}/ship/{ship_id}/state",
                timeout=self.timeout
            )
            response_time = time.time() - start_time
            
            if response.status_code == 200:
                data = response.json()
                ship_state = ShipState(
                    ship_id=ship_id,
                    position=(
                        data.get("position", {}).get("x", 0),
                        data.get("position", {}).get("y", 0),
                        data.get("position", {}).get("z", 0)
                    ),
                    rotation=(
                        data.get("rotation", {}).get("pitch", 0),
                        data.get("rotation", {}).get("yaw", 0),
                        data.get("rotation", {}).get("roll", 0)
                    ),
                    velocity=(
                        data.get("velocity", {}).get("x", 0),
                        data.get("velocity", {}).get("y", 0),
                        data.get("velocity", {}).get("z", 0)
                    ),
                    health=data.get("health", 100.0),
                    shield=data.get("shield", 100.0),
                    is_alive=data.get("is_alive", True)
                )
                
                return APIResponse(
                    success=True,
                    data=ship_state.to_dict(),
                    status_code=200,
                    response_time=response_time
                )
            else:
                return APIResponse(
                    success=False,
                    error_message=f"HTTP {response.status_code}: {response.text}",
                    status_code=response.status_code,
                    response_time=response_time
                )
                
        except Exception as e:
            return APIResponse(
                success=False,
                error_message=f"Get state error: {str(e)}",
                status_code=None,
                response_time=time.time() - start_time
            )
    
    def get_all_ships(self) -> APIResponse:
        """
        Get all active ships in the game
        
        Returns:
            APIResponse with list of ships
        """
        if not self.is_connected:
            return APIResponse(success=False, error_message="Not connected to Unreal Engine")
        
        start_time = time.time()
        
        try:
            response = self.session.get(
                f"{self.base_url}/ships",
                timeout=self.timeout
            )
            response_time = time.time() - start_time
            
            if response.status_code == 200:
                return APIResponse(
                    success=True,
                    data=response.json(),
                    status_code=200,
                    response_time=response_time
                )
            else:
                return APIResponse(
                    success=False,
                    error_message=f"HTTP {response.status_code}: {response.text}",
                    status_code=response.status_code,
                    response_time=response_time
                )
                
        except Exception as e:
            return APIResponse(
                success=False,
                error_message=f"Get ships error: {str(e)}",
                status_code=None,
                response_time=time.time() - start_time
            )
    
    def execute_console_command(self, command: str) -> APIResponse:
        """
        Execute Unreal Engine console command
        
        Args:
            command: Console command to execute
            
        Returns:
            APIResponse with execution result
        """
        if not self.is_connected:
            return APIResponse(success=False, error_message="Not connected to Unreal Engine")
        
        start_time = time.time()
        
        try:
            payload = {
                "command": "console",
                "parameters": {
                    "command_string": command
                }
            }
            
            response = self.session.post(
                f"{self.base_url}/execute",
                json=payload,
                timeout=self.timeout
            )
            response_time = time.time() - start_time
            
            if response.status_code == 200:
                return APIResponse(
                    success=True,
                    data=response.json(),
                    status_code=200,
                    response_time=response_time
                )
            else:
                return APIResponse(
                    success=False,
                    error_message=f"HTTP {response.status_code}: {response.text}",
                    status_code=response.status_code,
                    response_time=response_time
                )
                
        except Exception as e:
            return APIResponse(
                success=False,
                error_message=f"Console command error: {str(e)}",
                status_code=None,
                response_time=time.time() - start_time
            )
    
    def get_system_info(self) -> APIResponse:
        """
        Get Unreal Engine system information
        
        Returns:
            APIResponse with system info
        """
        if not self.is_connected:
            return APIResponse(success=False, error_message="Not connected to Unreal Engine")
        
        start_time = time.time()
        
        try:
            response = self.session.get(
                f"{self.base_url}/system/info",
                timeout=self.timeout
            )
            response_time = time.time() - start_time
            
            if response.status_code == 200:
                return APIResponse(
                    success=True,
                    data=response.json(),
                    status_code=200,
                    response_time=response_time
                )
            else:
                return APIResponse(
                    success=False,
                    error_message=f"HTTP {response.status_code}: {response.text}",
                    status_code=response.status_code,
                    response_time=response_time
                )
                
        except Exception as e:
            return APIResponse(
                success=False,
                error_message=f"System info error: {str(e)}",
                status_code=None,
                response_time=time.time() - start_time
            )
    
    def close(self):
        """Close the HTTP client session"""
        if self.session:
            self.session.close()
            logger.info("HTTP API client session closed")


def test_http_api_integration():
    """Test the HTTP API integration"""
    print("Testing Unreal Engine HTTP API Integration")
    print("=" * 50)
    
    # Initialize client
    client = UnrealHTTPAPIClient()
    
    # Test 1: Health check
    print("\n1. Testing health check...")
    health_response = client.health_check()
    
    if health_response.success:
        print(f"+ Health check passed ({health_response.response_time:.2f}s)")
        print(f"  Connected: {client.is_connected}")
        if health_response.data:
            print(f"  Engine version: {health_response.data.get('engine_version', 'Unknown')}")
            print(f"  Game mode: {health_response.data.get('game_mode', 'Unknown')}")
    else:
        print(f"- Health check failed: {health_response.error_message}")
        print("  Note: Unreal Engine must be running with HTTP API enabled")
    
    # Test 2: System info (if connected)
    if client.is_connected:
        print("\n2. Testing system info...")
        sys_response = client.get_system_info()
        
        if sys_response.success:
            print(f"+ System info retrieved ({sys_response.response_time:.2f}s)")
            if sys_response.data:
                print(f"  FPS: {sys_response.data.get('fps', 'Unknown')}")
                print(f"  Player count: {sys_response.data.get('player_count', 0)}")
        else:
            print(f"- System info failed: {sys_response.error_message}")
    
    # Test 3: Get all ships (if connected)
    if client.is_connected:
        print("\n3. Testing get all ships...")
        ships_response = client.get_all_ships()
        
        if ships_response.success:
            print(f"+ Ships retrieved ({ships_response.response_time:.2f}s)")
            if ships_response.data:
                ship_count = len(ships_response.data.get("ships", []))
                print(f"  Active ships: {ship_count}")
        else:
            print(f"- Get ships failed: {ships_response.error_message}")
    
    # Cleanup
    client.close()
    
    print("\n" + "=" * 50)
    if client.is_connected:
        print("+ HTTP API Integration test completed successfully")
        print("+ Unreal Engine is accessible via HTTP API")
        return True
    else:
        print("- HTTP API Integration test completed")
        print("- Note: Unreal Engine must be running for full functionality")
        return False


if __name__ == "__main__":
    success = test_http_api_integration()
    exit(0 if success else 1)