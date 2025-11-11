#!/usr/bin/env python3
"""
Screenshot Capture System for Autonomous Playtesting
High-frequency screenshot capture with base64 encoding, compression, and metadata embedding
"""

import base64
import json
import time
import logging
import threading
from pathlib import Path
from typing import Optional, Dict, Any, Tuple
from datetime import datetime
import requests
import io
from PIL import Image
import numpy as np
import cv2
import mss
import mss.tools

# Configuration
SCREENSHOTS_DIR = Path("screenshots_visual_playtest")
CONFIG_DIR = Path("automation_config")
CONFIG_FILE = CONFIG_DIR / "screenshot_config.json"
METADATA_FILE = SCREENSHOTS_DIR / "metadata.jsonl"
MAX_SCREENSHOTS = 1000  # Maximum screenshots to keep
JPEG_QUALITY = 85  # JPEG compression quality (1-100)

class ScreenshotCapture:
    """High-frequency screenshot capture system with AI transmission capabilities"""
    
    def __init__(self, server_url: str = "http://localhost:8080", capture_interval: float = 3.0):
        self.server_url = server_url
        self.capture_interval = capture_interval
        self.running = False
        self.capture_thread: Optional[threading.Thread] = None
        self.screenshot_count = 0
        self.last_capture_time = 0
        
        # Create directories
        SCREENSHOTS_DIR.mkdir(exist_ok=True)
        CONFIG_DIR.mkdir(exist_ok=True)
        
        # Setup logging
        self.setup_logging()
        
        # Initialize MSS for screen capture
        self.sct = mss.mss()
        
        # Load configuration
        self.config = self.load_config()
        
        self.log("Screenshot capture system initialized")
        
    def setup_logging(self):
        """Setup logging configuration"""
        logging.basicConfig(
            filename=CONFIG_DIR / "screenshot_capture.log",
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
            
    def load_config(self) -> Dict[str, Any]:
        """Load configuration from file or create default"""
        default_config = {
            "capture_interval": 3.0,
            "jpeg_quality": JPEG_QUALITY,
            "max_screenshots": MAX_SCREENSHOTS,
            "auto_cleanup": True,
            "capture_region": None,  # None = full screen, or [x, y, width, height]
            "resize_to": None,  # None = no resize, or [width, height]
            "enable_compression": True,
            "metadata_embedding": True,
            "server_endpoint": "/screenshot"
        }
        
        if CONFIG_FILE.exists():
            try:
                with open(CONFIG_FILE, 'r') as f:
                    config = json.load(f)
                    # Merge with defaults
                    default_config.update(config)
                    self.log(f"Configuration loaded from {CONFIG_FILE}")
            except Exception as e:
                self.log(f"Error loading config: {e}, using defaults", "WARNING")
        else:
            self.save_config(default_config)
            
        return default_config
        
    def save_config(self, config: Dict[str, Any]):
        """Save configuration to file"""
        try:
            with open(CONFIG_FILE, 'w') as f:
                json.dump(config, f, indent=2)
            self.log(f"Configuration saved to {CONFIG_FILE}")
        except Exception as e:
            self.log(f"Error saving config: {e}", "ERROR")
            
    def update_config(self, **kwargs):
        """Update configuration and save"""
        self.config.update(kwargs)
        self.save_config(self.config)
        self.capture_interval = self.config.get("capture_interval", 3.0)
        self.log(f"Configuration updated: {kwargs}")
        
    def get_game_window(self) -> Optional[Dict[str, int]]:
        """Find the game window and return its region"""
        try:
            # Try to find Unreal Editor window
            for monitor in self.sct.monitors[1:]:  # Skip the first monitor (all monitors)
                # Capture a small test region to check if it's the game window
                # This is a simplified approach - in production, you'd use window detection
                return {
                    "left": monitor["left"],
                    "top": monitor["top"],
                    "width": monitor["width"],
                    "height": monitor["height"]
                }
        except Exception as e:
            self.log(f"Error finding game window: {e}", "WARNING")
            
        return None
        
    def capture_screen(self) -> Optional[np.ndarray]:
        """Capture screen using MSS"""
        try:
            # Get capture region
            region = self.config.get("capture_region")
            if region:
                monitor = {
                    "left": region[0],
                    "top": region[1],
                    "width": region[2],
                    "height": region[3]
                }
            else:
                # Try to find game window
                game_window = self.get_game_window()
                if game_window:
                    monitor = game_window
                else:
                    # Use primary monitor
                    monitor = self.sct.monitors[1] if len(self.sct.monitors) > 1 else self.sct.monitors[0]
            
            # Capture screen
            screenshot = self.sct.grab(monitor)
            
            # Convert to numpy array
            img = np.array(screenshot)
            
            # Convert BGRA to RGB
            if img.shape[2] == 4:
                img = cv2.cvtColor(img, cv2.COLOR_BGRA2RGB)
                
            # Resize if configured
            resize_to = self.config.get("resize_to")
            if resize_to:
                img = cv2.resize(img, tuple(resize_to), interpolation=cv2.INTER_AREA)
                
            return img
            
        except Exception as e:
            self.log(f"Error capturing screen: {e}", "ERROR")
            return None
            
    def encode_to_base64(self, image: np.ndarray, quality: int = JPEG_QUALITY) -> Tuple[str, Tuple[int, int]]:
        """
        Encode image to base64 with JPEG compression
        Returns: (base64_string, (width, height))
        """
        try:
            # Convert RGB to BGR for OpenCV
            if len(image.shape) == 3 and image.shape[2] == 3:
                image_bgr = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)
            else:
                image_bgr = image
                
            # Encode to JPEG
            encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), quality]
            _, buffer = cv2.imencode('.jpg', image_bgr, encode_param)
            
            # Convert to base64
            base64_string = base64.b64encode(buffer).decode('utf-8')
            
            return base64_string, (image.shape[1], image.shape[0])
            
        except Exception as e:
            self.log(f"Error encoding image to base64: {e}", "ERROR")
            return "", (0, 0)
            
    def save_screenshot(self, image: np.ndarray, metadata: Dict[str, Any]) -> Optional[str]:
        """Save screenshot to disk with metadata"""
        try:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
            filename = f"screenshot_{timestamp}.jpg"
            filepath = SCREENSHOTS_DIR / filename
            
            # Convert RGB to BGR for OpenCV
            if len(image.shape) == 3 and image.shape[2] == 3:
                image_bgr = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)
            else:
                image_bgr = image
                
            # Save image
            quality = self.config.get("jpeg_quality", JPEG_QUALITY)
            cv2.imwrite(str(filepath), image_bgr, [int(cv2.IMWRITE_JPEG_QUALITY), quality])
            
            # Save metadata
            if self.config.get("metadata_embedding", True):
                metadata["filename"] = filename
                metadata["filepath"] = str(filepath)
                metadata["timestamp"] = datetime.now().isoformat()
                
                with open(METADATA_FILE, 'a') as f:
                    f.write(json.dumps(metadata) + '\n')
                    
            self.log(f"Screenshot saved: {filename}")
            return str(filepath)
            
        except Exception as e:
            self.log(f"Error saving screenshot: {e}", "ERROR")
            return None
            
    def cleanup_old_screenshots(self):
        """Clean up old screenshots if over limit"""
        try:
            max_screenshots = self.config.get("max_screenshots", MAX_SCREENSHOTS)
            
            # Get all screenshot files
            screenshots = list(SCREENSHOTS_DIR.glob("screenshot_*.jpg"))
            screenshots.sort(key=lambda x: x.stat().st_mtime)
            
            # Remove old screenshots if over limit
            if len(screenshots) > max_screenshots:
                to_remove = len(screenshots) - max_screenshots
                for i in range(to_remove):
                    screenshots[i].unlink()
                    self.log(f"Removed old screenshot: {screenshots[i].name}")
                    
        except Exception as e:
            self.log(f"Error cleaning up screenshots: {e}", "WARNING")
            
    def get_game_state(self) -> Dict[str, Any]:
        """Get current game state from HTTP API"""
        try:
            response = requests.get(f"{self.server_url}/game_state", timeout=5)
            if response.status_code == 200:
                return response.json()
        except Exception as e:
            self.log(f"Error getting game state: {e}", "WARNING")
            
        return {}
        
    def send_screenshot_to_ai(self, base64_image: str, metadata: Dict[str, Any]) -> bool:
        """Send screenshot to AI system via HTTP API"""
        try:
            payload = {
                "image_base64": base64_image,
                "metadata": metadata,
                "timestamp": datetime.now().isoformat()
            }
            
            response = requests.post(
                f"{self.server_url}{self.config.get('server_endpoint', '/screenshot')}",
                json=payload,
                timeout=10
            )
            
            if response.status_code == 200:
                self.log("Screenshot sent to AI successfully")
                return True
            else:
                self.log(f"Failed to send screenshot to AI: {response.status_code}", "WARNING")
                return False
                
        except Exception as e:
            self.log(f"Error sending screenshot to AI: {e}", "ERROR")
            return False
            
    def capture_and_process(self) -> Optional[Dict[str, Any]]:
        """Capture, process, and return screenshot data"""
        try:
            # Capture screen
            image = self.capture_screen()
            if image is None:
                self.log("Failed to capture screen", "WARNING")
                return None
                
            # Get game state
            game_state = self.get_game_state()
            
            # Create metadata
            metadata = {
                "capture_number": self.screenshot_count,
                "game_state": game_state,
                "capture_interval": self.capture_interval,
                "image_size": (image.shape[1], image.shape[0])
            }
            
            # Encode to base64
            base64_image, dimensions = self.encode_to_base64(
                image, 
                self.config.get("jpeg_quality", JPEG_QUALITY)
            )
            
            if not base64_image:
                self.log("Failed to encode image", "WARNING")
                return None
                
            # Update metadata with dimensions
            metadata["dimensions"] = dimensions
            metadata["base64_size"] = len(base64_image)
            
            # Save to disk
            filepath = self.save_screenshot(image, metadata)
            
            # Send to AI if configured
            if self.config.get("enable_ai_transmission", True):
                self.send_screenshot_to_ai(base64_image, metadata)
                
            self.screenshot_count += 1
            
            # Cleanup old screenshots
            if self.config.get("auto_cleanup", True):
                self.cleanup_old_screenshots()
                
            return {
                "base64": base64_image,
                "metadata": metadata,
                "filepath": filepath
            }
            
        except Exception as e:
            self.log(f"Error in capture and process: {e}", "ERROR")
            return None
            
    def capture_loop(self):
        """Main capture loop"""
        self.log("Starting capture loop...")
        
        while self.running:
            try:
                current_time = time.time()
                
                # Check if it's time to capture
                if current_time - self.last_capture_time >= self.capture_interval:
                    self.log(f"Capturing screenshot #{self.screenshot_count + 1}")
                    
                    result = self.capture_and_process()
                    
                    if result:
                        self.last_capture_time = current_time
                        self.log(f"Screenshot captured and processed successfully")
                    else:
                        self.log("Failed to capture screenshot", "WARNING")
                        
                # Sleep briefly to avoid busy waiting
                time.sleep(0.1)
                
            except Exception as e:
                self.log(f"Error in capture loop: {e}", "ERROR")
                time.sleep(1)  # Wait longer on error
                
        self.log("Capture loop stopped")
        
    def start(self, interval: Optional[float] = None):
        """Start screenshot capture"""
        if self.running:
            self.log("Capture already running", "WARNING")
            return
            
        if interval is not None:
            self.capture_interval = interval
            
        self.running = True
        self.capture_thread = threading.Thread(target=self.capture_loop, daemon=True)
        self.capture_thread.start()
        
        self.log(f"Screenshot capture started (interval: {self.capture_interval}s)")
        
    def stop(self):
        """Stop screenshot capture"""
        if not self.running:
            self.log("Capture not running", "WARNING")
            return
            
        self.running = False
        
        if self.capture_thread:
            self.capture_thread.join(timeout=5)
            
        self.log("Screenshot capture stopped")
        
    def get_stats(self) -> Dict[str, Any]:
        """Get capture statistics"""
        return {
            "screenshot_count": self.screenshot_count,
            "running": self.running,
            "capture_interval": self.capture_interval,
            "last_capture": self.last_capture_time,
            "config": self.config
        }
        
    def set_interval(self, interval: float):
        """Update capture interval"""
        self.capture_interval = interval
        self.config["capture_interval"] = interval
        self.save_config(self.config)
        self.log(f"Capture interval updated to {interval}s")

def main():
    """Main function for standalone usage"""
    import argparse
    
    parser = argparse.ArgumentParser(description="Screenshot capture system")
    parser.add_argument("--interval", type=float, default=3.0, help="Capture interval in seconds")
    parser.add_argument("--server", type=str, default="http://localhost:8080", help="Game server URL")
    parser.add_argument("--duration", type=float, default=0, help="Capture duration in seconds (0 = unlimited)")
    parser.add_argument("--quality", type=int, default=JPEG_QUALITY, help="JPEG quality (1-100)")
    
    args = parser.parse_args()
    
    print("=" * 70)
    print("SCREENSHOT CAPTURE SYSTEM")
    print("=" * 70)
    print(f"Server: {args.server}")
    print(f"Interval: {args.interval}s")
    print(f"Quality: {args.quality}")
    print(f"Duration: {args.duration if args.duration > 0 else 'unlimited'}")
    print("=" * 70)
    
    # Create capture instance
    capture = ScreenshotCapture(args.server, args.capture_interval)
    
    # Update quality if specified
    if args.quality != JPEG_QUALITY:
        capture.update_config(jpeg_quality=args.quality)
    
    try:
        # Start capture
        capture.start()
        
        # Run for specified duration or until interrupted
        if args.duration > 0:
            time.sleep(args.duration)
            capture.stop()
        else:
            print("\nPress Ctrl+C to stop...")
            while True:
                time.sleep(1)
                
    except KeyboardInterrupt:
        print("\nShutting down...")
        capture.stop()
        
    # Print statistics
    stats = capture.get_stats()
    print("\n" + "=" * 70)
    print("CAPTURE STATISTICS")
    print("=" * 70)
    print(f"Total screenshots: {stats['screenshot_count']}")
    print(f"Saved to: {SCREENSHOTS_DIR}")
    print("=" * 70)

if __name__ == "__main__":
    main()