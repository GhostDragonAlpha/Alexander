#!/usr/bin/env python3
"""
Desktop Mode Game Launcher for Autonomous Playtesting
Launches the game in windowed desktop mode (not VR) with process monitoring and debugging features
"""

import subprocess
import sys
import time
import os
import json
import signal
import argparse
from pathlib import Path
from typing import Optional, Dict, Any
import psutil
import requests
from datetime import datetime

# Configuration
UNREAL_EDITOR = r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe"
PROJECT_FILE = r"C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject"
MAP_NAME = "VRTemplateMap"
GAME_MODE = "/Script/Alexander.AutomationGameMode"
CONFIG_DIR = Path("automation_config")
PID_FILE = CONFIG_DIR / "game_process.json"
LOG_FILE = CONFIG_DIR / "launcher.log"

class GameLauncher:
    """Desktop game launcher with process monitoring and automatic restart capabilities"""
    
    def __init__(self):
        self.process: Optional[subprocess.Popen] = None
        self.pid: Optional[int] = None
        self.monitor_thread = None
        self.running = False
        self.restart_count = 0
        self.max_restarts = 3
        
        # Create config directory
        CONFIG_DIR.mkdir(exist_ok=True)
        
        # Setup logging
        self.setup_logging()
        
    def setup_logging(self):
        """Setup logging configuration"""
        logging.basicConfig(
            filename=LOG_FILE,
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )
        self.logger = logging.getLogger(__name__)
        
    def log(self, message: str, level: str = "INFO"):
        """Log message to both console and file"""
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        print(f"[{timestamp}] {level}: {message}")
        
        if level == "INFO":
            self.logger.info(message)
        elif level == "WARNING":
            self.logger.warning(message)
        elif level == "ERROR":
            self.logger.error(message)
        elif level == "DEBUG":
            self.logger.debug(message)
            
    def parse_arguments(self) -> argparse.Namespace:
        """Parse command line arguments"""
        parser = argparse.ArgumentParser(
            description="Launch game in desktop mode for autonomous playtesting",
            formatter_class=argparse.RawDescriptionHelpFormatter,
            epilog="""
Examples:
  python launch_game_desktop.py
  python launch_game_desktop.py --resolution 1920x1080 --quality high
  python launch_game_desktop.py --resolution 1280x720 --quality low --interval 2
  python launch_game_desktop.py --no-console --timeout 300
            """
        )
        
        parser.add_argument(
            "--resolution",
            type=str,
            default="1280x720",
            help="Window resolution in format WIDTHxHEIGHT (default: 1280x720)"
        )
        
        parser.add_argument(
            "--quality",
            type=str,
            choices=["low", "medium", "high", "epic"],
            default="medium",
            help="Graphics quality preset (default: medium)"
        )
        
        parser.add_argument(
            "--interval",
            type=int,
            default=3,
            help="Screenshot capture interval in seconds (default: 3)"
        )
        
        parser.add_argument(
            "--timeout",
            type=int,
            default=0,
            help="Game session timeout in seconds (0 = no timeout, default: 0)"
        )
        
        parser.add_argument(
            "--no-console",
            action="store_true",
            help="Disable console window"
        )
        
        parser.add_argument(
            "--no-debug",
            action="store_true",
            help="Disable debugging features"
        )
        
        parser.add_argument(
            "--port",
            type=int,
            default=8080,
            help="HTTP API port (default: 8080)"
        )
        
        parser.add_argument(
            "--max-restarts",
            type=int,
            default=3,
            help="Maximum automatic restarts on crash (default: 3)"
        )
        
        return parser.parse_args()
        
    def parse_resolution(self, resolution_str: str) -> tuple:
        """Parse resolution string into width and height"""
        try:
            width, height = map(int, resolution_str.lower().split('x'))
            return width, height
        except:
            self.log(f"Invalid resolution format: {resolution_str}, using default 1280x720", "WARNING")
            return 1280, 720
            
    def get_quality_settings(self, quality: str) -> Dict[str, str]:
        """Get Unreal console commands for quality preset"""
        quality_settings = {
            "low": {
                "sg.ShadowQuality": "0",
                "sg.TextureQuality": "0",
                "sg.EffectsQuality": "0",
                "sg.PostProcessQuality": "0",
                "r.ScreenPercentage": "50"
            },
            "medium": {
                "sg.ShadowQuality": "1",
                "sg.TextureQuality": "1",
                "sg.EffectsQuality": "1",
                "sg.PostProcessQuality": "1",
                "r.ScreenPercentage": "75"
            },
            "high": {
                "sg.ShadowQuality": "2",
                "sg.TextureQuality": "2",
                "sg.EffectsQuality": "2",
                "sg.PostProcessQuality": "2",
                "r.ScreenPercentage": "100"
            },
            "epic": {
                "sg.ShadowQuality": "3",
                "sg.TextureQuality": "3",
                "sg.EffectsQuality": "3",
                "sg.PostProcessQuality": "3",
                "r.ScreenPercentage": "100"
            }
        }
        return quality_settings.get(quality, quality_settings["medium"])
        
    def save_process_info(self, args: argparse.Namespace):
        """Save process information to file"""
        process_info = {
            "pid": self.pid,
            "start_time": datetime.now().isoformat(),
            "resolution": f"{args.resolution}",
            "quality": args.quality,
            "capture_interval": args.interval,
            "port": args.port,
            "debug_enabled": not args.no_debug,
            "console_enabled": not args.no_console,
            "restart_count": self.restart_count,
            "max_restarts": args.max_restarts,
            "timeout": args.timeout
        }
        
        with open(PID_FILE, 'w') as f:
            json.dump(process_info, f, indent=2)
            
        self.log(f"Process info saved to {PID_FILE}")
        
    def build_launch_command(self, args: argparse.Namespace) -> list:
        """Build the game launch command"""
        width, height = self.parse_resolution(args.resolution)
        
        cmd = [
            UNREAL_EDITOR,
            PROJECT_FILE,
            f"{MAP_NAME}?game={GAME_MODE}",
            "-game",
            "-windowed",
            f"-ResX={width}",
            f"-ResY={height}",
            f"-HTTPPort={args.port}",
            "-nohmd",  # Disable VR
            "-log"
        ]
        
        # Add console settings
        if args.no_console:
            cmd.append("-silent")
        else:
            cmd.append("-console")
            
        # Add debug settings
        if not args.no_debug:
            cmd.extend([
                "-debug",
                "-forcelogflush",
                "-logcmds=\"LogHttp Log, LogAutomationAPI Log\""
            ])
            
        # Add quality settings as console commands
        quality_cmds = self.get_quality_settings(args.quality)
        for cvar, value in quality_cmds.items():
            cmd.append(f"-ExecCmds={cvar} {value}")
            
        return cmd
        
    def launch_game(self, args: argparse.Namespace) -> bool:
        """Launch the game process"""
        try:
            cmd = self.build_launch_command(args)
            
            self.log(f"Launching game with command: {' '.join(cmd)}")
            
            # Set environment variables
            env = os.environ.copy()
            env["UE_AUTOMATION_MODE"] = "1"
            
            # Launch process
            self.process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE if not args.no_console else subprocess.DEVNULL,
                stderr=subprocess.PIPE if not args.no_console else subprocess.DEVNULL,
                env=env,
                creationflags=subprocess.CREATE_NEW_CONSOLE if not args.no_console else 0
            )
            
            self.pid = self.process.pid
            self.running = True
            
            self.log(f"Game launched successfully (PID: {self.pid})")
            self.save_process_info(args)
            
            return True
            
        except Exception as e:
            self.log(f"Failed to launch game: {e}", "ERROR")
            return False
            
    def check_process_health(self) -> bool:
        """Check if game process is healthy"""
        if not self.process:
            return False
            
        try:
            # Check if process is still running
            if self.process.poll() is not None:
                self.log("Game process has terminated", "WARNING")
                return False
                
            # Check process memory usage
            process = psutil.Process(self.pid)
            memory_info = process.memory_info()
            
            # Alert if memory usage is too high (> 4GB)
            if memory_info.rss > 4 * 1024 * 1024 * 1024:
                self.log(f"High memory usage detected: {memory_info.rss / 1024 / 1024:.0f} MB", "WARNING")
                
            # Check if process is responsive (HTTP API)
            try:
                response = requests.get(f"http://localhost:{self.get_port()}/status", timeout=5)
                if response.status_code != 200:
                    self.log("Game HTTP API not responding correctly", "WARNING")
                    return False
            except:
                self.log("Game HTTP API not responding", "WARNING")
                return False
                
            return True
            
        except psutil.NoSuchProcess:
            self.log("Game process not found", "ERROR")
            return False
        except Exception as e:
            self.log(f"Error checking process health: {e}", "ERROR")
            return False
            
    def get_port(self) -> int:
        """Get the HTTP API port from saved config"""
        if PID_FILE.exists():
            try:
                with open(PID_FILE, 'r') as f:
                    data = json.load(f)
                    return data.get("port", 8080)
            except:
                pass
        return 8080
        
    def monitor_process(self, args: argparse.Namespace):
        """Monitor game process and handle restarts"""
        start_time = time.time()
        
        while self.running:
            try:
                # Check timeout
                if args.timeout > 0 and time.time() - start_time > args.timeout:
                    self.log(f"Session timeout reached ({args.timeout}s)", "INFO")
                    self.graceful_shutdown()
                    break
                    
                # Check process health
                if not self.check_process_health():
                    if self.restart_count < args.max_restarts:
                        self.restart_count += 1
                        self.log(f"Process unhealthy, attempting restart {self.restart_count}/{args.max_restarts}", "WARNING")
                        
                        # Wait a bit before restart
                        time.sleep(5)
                        
                        # Restart game
                        if self.launch_game(args):
                            start_time = time.time()  # Reset timeout
                            continue
                        else:
                            self.log("Failed to restart game", "ERROR")
                            break
                    else:
                        self.log("Max restarts reached, giving up", "ERROR")
                        break
                        
                # Sleep before next check
                time.sleep(10)
                
            except KeyboardInterrupt:
                self.log("Shutdown requested by user", "INFO")
                self.graceful_shutdown()
                break
            except Exception as e:
                self.log(f"Error in monitor loop: {e}", "ERROR")
                time.sleep(5)
                
    def graceful_shutdown(self):
        """Gracefully shutdown the game process"""
        self.log("Initiating graceful shutdown...")
        
        if self.process:
            try:
                # Try to shutdown via HTTP API first
                port = self.get_port()
                try:
                    requests.post(f"http://localhost:{port}/execute_command", 
                                json={"command": "quit"}, timeout=5)
                    self.log("Sent quit command via HTTP API")
                    time.sleep(3)
                except:
                    pass
                
                # Terminate process
                self.process.terminate()
                
                # Wait for graceful exit
                try:
                    self.process.wait(timeout=10)
                    self.log("Game process terminated gracefully")
                except subprocess.TimeoutExpired:
                    self.log("Process did not terminate gracefully, forcing kill", "WARNING")
                    self.process.kill()
                    self.process.wait()
                    
            except Exception as e:
                self.log(f"Error during shutdown: {e}", "ERROR")
                
        self.running = False
        
        # Clean up PID file
        if PID_FILE.exists():
            PID_FILE.unlink()
            self.log("Cleaned up PID file")
            
    def wait_for_http_api(self, port: int, timeout: int = 180) -> bool:
        """Wait for HTTP API to become available"""
        self.log(f"Waiting for HTTP API on port {port}...", "INFO")
        
        start_time = time.time()
        while time.time() - start_time < timeout:
            try:
                response = requests.get(f"http://localhost:{port}/status", timeout=2)
                if response.status_code == 200:
                    elapsed = int(time.time() - start_time)
                    self.log(f"HTTP API ready! (took {elapsed}s)", "INFO")
                    return True
            except:
                pass
            time.sleep(2)
            
        self.log(f"HTTP API timeout after {timeout}s", "ERROR")
        return False
        
    def run(self):
        """Main execution"""
        print("=" * 70)
        print("DESKTOP MODE GAME LAUNCHER")
        print("=" * 70)
        
        # Parse arguments
        args = self.parse_arguments()
        self.max_restarts = args.max_restarts
        
        # Check if already running
        if PID_FILE.exists():
            self.log("Found existing PID file, checking if process is still running...", "WARNING")
            try:
                with open(PID_FILE, 'r') as f:
                    data = json.load(f)
                    old_pid = data.get("pid")
                    
                if psutil.pid_exists(old_pid):
                    self.log(f"Game already running (PID: {old_pid})", "ERROR")
                    self.log("Please close the existing game instance first", "ERROR")
                    return 1
                else:
                    self.log("Stale PID file found, removing...", "INFO")
                    PID_FILE.unlink()
            except Exception as e:
                self.log(f"Error checking existing process: {e}", "WARNING")
                
        # Launch game
        if not self.launch_game(args):
            return 1
            
        # Wait for HTTP API
        if not self.wait_for_http_api(args.port):
            self.log("Failed to connect to game HTTP API", "ERROR")
            self.graceful_shutdown()
            return 1
            
        # Start monitoring
        self.log("Starting process monitor...", "INFO")
        self.monitor_process(args)
        
        # Cleanup
        self.log("Launcher shutting down...", "INFO")
        return 0

def signal_handler(signum, frame):
    """Handle shutdown signals"""
    print(f"\nReceived signal {signum}, shutting down...")
    if 'launcher' in globals():
        launcher.graceful_shutdown()

if __name__ == "__main__":
    # Setup signal handlers
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    
    # Create and run launcher
    launcher = GameLauncher()
    exit_code = launcher.run()
    sys.exit(exit_code)