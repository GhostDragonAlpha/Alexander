#!/usr/bin/env python3
"""
Performance Data Collection System
Collects real-time performance metrics during playtests and stores them in time-series format
"""

import json
import time
import logging
import threading
import csv
from pathlib import Path
from typing import Dict, Any, List, Optional
from datetime import datetime
import requests
import psutil
from dataclasses import dataclass, asdict

# Configuration
PERFORMANCE_DATA_DIR = Path("playtesting_data/performance")
CONFIG_DIR = Path("automation_config")
LOG_FILE = CONFIG_DIR / "performance_collector.log"

@dataclass
class PerformanceMetrics:
    """Performance metrics snapshot"""
    timestamp: datetime
    fps: float
    frame_time: float
    memory_usage_mb: float
    cpu_usage_percent: float
    gpu_usage_percent: float
    draw_calls: int
    triangle_count: int
    actor_count: int
    component_count: int
    tick_time: float
    session_id: str
    iteration: int
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary"""
        data = asdict(self)
        data['timestamp'] = self.timestamp.isoformat()
        return data

class PerformanceCollector:
    """Real-time performance metrics collector"""
    
    def __init__(self, server_url: str = "http://localhost:8080", session_id: str = "default"):
        self.server_url = server_url
        self.session_id = session_id
        self.running = False
        self.collection_thread: Optional[threading.Thread] = None
        self.metrics_history: List[PerformanceMetrics] = []
        self.max_history_size = 10000  # Maximum metrics to keep in memory
        
        # Configuration
        self.collection_interval = 5.0  # seconds between collections
        self.baseline_metrics: Optional[PerformanceMetrics] = None
        self.performance_thresholds = {
            "fps_min": 30,
            "frame_time_max": 33.3,  # ms (30 FPS)
            "memory_max_mb": 4096,   # 4GB
            "cpu_max_percent": 80,
            "gpu_max_percent": 90
        }
        
        # Setup directories
        PERFORMANCE_DATA_DIR.mkdir(parents=True, exist_ok=True)
        CONFIG_DIR.mkdir(exist_ok=True)
        
        # Setup logging
        self.setup_logging()
        
        self.log("Performance collector initialized")
        
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
        """Log message"""
        if level == "INFO":
            self.logger.info(message)
        elif level == "WARNING":
            self.logger.warning(message)
        elif level == "ERROR":
            self.logger.error(message)
        elif level == "DEBUG":
            self.logger.debug(message)
            
        print(f"[PerformanceCollector] {level}: {message}")
        
    def get_system_metrics(self) -> Dict[str, float]:
        """Get system-level performance metrics"""
        try:
            # Get CPU usage
            cpu_percent = psutil.cpu_percent(interval=1)
            
            # Get memory usage
            memory = psutil.virtual_memory()
            memory_usage_mb = memory.used / (1024 * 1024)  # Convert to MB
            
            # Get process-specific metrics if game is running
            process_metrics = self.get_game_process_metrics()
            
            return {
                "cpu_usage_percent": cpu_percent,
                "memory_usage_mb": memory_usage_mb,
                "process_cpu_percent": process_metrics.get("cpu_percent", 0),
                "process_memory_mb": process_metrics.get("memory_mb", 0)
            }
            
        except Exception as e:
            self.log(f"Error getting system metrics: {e}", "WARNING")
            return {}
            
    def get_game_process_metrics(self) -> Dict[str, float]:
        """Get metrics specific to the game process"""
        try:
            # Look for Unreal Editor or game process
            for proc in psutil.process_iter(['pid', 'name', 'cpu_percent', 'memory_info']):
                try:
                    if 'UnrealEditor' in proc.info['name'] or 'Alexander' in proc.info['name']:
                        memory_mb = proc.info['memory_info'].rss / (1024 * 1024)
                        cpu_percent = proc.info['cpu_percent']
                        return {
                            "cpu_percent": cpu_percent,
                            "memory_mb": memory_mb
                        }
                except (psutil.NoSuchProcess, psutil.AccessDenied):
                    continue
                    
        except Exception as e:
            self.log(f"Error getting game process metrics: {e}", "WARNING")
            
        return {}
        
    def get_game_metrics(self) -> Dict[str, Any]:
        """Get performance metrics from game via HTTP API"""
        try:
            response = requests.get(
                f"{self.server_url}/performance_metrics",
                timeout=5
            )
            
            if response.status_code == 200:
                return response.json()
            else:
                self.log(f"Game metrics API returned status {response.status_code}", "WARNING")
                return {}
                
        except requests.exceptions.ConnectionError:
            self.log("Cannot connect to game metrics API", "WARNING")
            return {}
        except Exception as e:
            self.log(f"Error getting game metrics: {e}", "WARNING")
            return {}
            
    def collect_metrics(self) -> Optional[PerformanceMetrics]:
        """Collect a single performance metrics snapshot"""
        try:
            # Get game metrics
            game_metrics = self.get_game_metrics()
            
            # Get system metrics
            system_metrics = self.get_system_metrics()
            
            # Combine metrics
            metrics = PerformanceMetrics(
                timestamp=datetime.now(),
                fps=game_metrics.get("fps", 0),
                frame_time=game_metrics.get("frame_time_ms", 0),
                memory_usage_mb=game_metrics.get("memory_mb", 
                                               system_metrics.get("memory_usage_mb", 0)),
                cpu_usage_percent=game_metrics.get("cpu_percent", 
                                                 system_metrics.get("cpu_usage_percent", 0)),
                gpu_usage_percent=game_metrics.get("gpu_percent", 0),
                draw_calls=game_metrics.get("draw_calls", 0),
                triangle_count=game_metrics.get("triangle_count", 0),
                actor_count=game_metrics.get("actor_count", 0),
                component_count=game_metrics.get("component_count", 0),
                tick_time=game_metrics.get("tick_time_ms", 0),
                session_id=self.session_id,
                iteration=0  # Will be updated by orchestrator
            )
            
            return metrics
            
        except Exception as e:
            self.log(f"Error collecting metrics: {e}", "WARNING")
            return None
            
    def set_baseline(self, baseline_metrics: PerformanceMetrics):
        """Set baseline metrics for comparison"""
        self.baseline_metrics = baseline_metrics
        self.log("Baseline metrics set")
        
    def check_performance_regression(self, current_metrics: PerformanceMetrics) -> List[Dict[str, Any]]:
        """Check for performance regressions compared to baseline"""
        regressions = []
        
        if not self.baseline_metrics:
            return regressions
            
        try:
            # Check FPS regression
            if current_metrics.fps < self.baseline_metrics.fps * 0.9:  # 10% drop
                regressions.append({
                    "type": "fps_regression",
                    "severity": "high" if current_metrics.fps < self.baseline_metrics.fps * 0.8 else "medium",
                    "baseline": self.baseline_metrics.fps,
                    "current": current_metrics.fps,
                    "message": f"FPS dropped from {self.baseline_metrics.fps:.1f} to {current_metrics.fps:.1f}"
                })
                
            # Check memory regression
            if current_metrics.memory_usage_mb > self.baseline_metrics.memory_usage_mb * 1.2:  # 20% increase
                regressions.append({
                    "type": "memory_regression",
                    "severity": "high" if current_metrics.memory_usage_mb > self.baseline_metrics.memory_usage_mb * 1.5 else "medium",
                    "baseline": self.baseline_metrics.memory_usage_mb,
                    "current": current_metrics.memory_usage_mb,
                    "message": f"Memory usage increased from {self.baseline_metrics.memory_usage_mb:.0f}MB to {current_metrics.memory_usage_mb:.0f}MB"
                })
                
            # Check frame time regression
            if current_metrics.frame_time > self.baseline_metrics.frame_time * 1.2:
                regressions.append({
                    "type": "frame_time_regression",
                    "severity": "high" if current_metrics.frame_time > self.baseline_metrics.frame_time * 1.5 else "medium",
                    "baseline": self.baseline_metrics.frame_time,
                    "current": current_metrics.frame_time,
                    "message": f"Frame time increased from {self.baseline_metrics.frame_time:.1f}ms to {current_metrics.frame_time:.1f}ms"
                })
                
        except Exception as e:
            self.log(f"Error checking performance regression: {e}", "WARNING")
            
        return regressions
        
    def check_threshold_violations(self, metrics: PerformanceMetrics) -> List[Dict[str, Any]]:
        """Check if metrics exceed defined thresholds"""
        violations = []
        
        try:
            # Check FPS minimum
            if metrics.fps < self.performance_thresholds["fps_min"]:
                violations.append({
                    "type": "low_fps",
                    "severity": "critical" if metrics.fps < 20 else "high",
                    "value": metrics.fps,
                    "threshold": self.performance_thresholds["fps_min"],
                    "message": f"FPS {metrics.fps:.1f} below threshold {self.performance_thresholds['fps_min']}"
                })
                
            # Check frame time maximum
            if metrics.frame_time > self.performance_thresholds["frame_time_max"]:
                violations.append({
                    "type": "high_frame_time",
                    "severity": "critical" if metrics.frame_time > 50 else "high",
                    "value": metrics.frame_time,
                    "threshold": self.performance_thresholds["frame_time_max"],
                    "message": f"Frame time {metrics.frame_time:.1f}ms exceeds threshold {self.performance_thresholds['frame_time_max']}ms"
                })
                
            # Check memory maximum
            if metrics.memory_usage_mb > self.performance_thresholds["memory_max_mb"]:
                violations.append({
                    "type": "high_memory_usage",
                    "severity": "critical" if metrics.memory_usage_mb > 6144 else "high",
                    "value": metrics.memory_usage_mb,
                    "threshold": self.performance_thresholds["memory_max_mb"],
                    "message": f"Memory usage {metrics.memory_usage_mb:.0f}MB exceeds threshold {self.performance_thresholds['memory_max_mb']}MB"
                })
                
            # Check CPU maximum
            if metrics.cpu_usage_percent > self.performance_thresholds["cpu_max_percent"]:
                violations.append({
                    "type": "high_cpu_usage",
                    "severity": "high",
                    "value": metrics.cpu_usage_percent,
                    "threshold": self.performance_thresholds["cpu_max_percent"],
                    "message": f"CPU usage {metrics.cpu_usage_percent:.1f}% exceeds threshold {self.performance_thresholds['cpu_max_percent']}%"
                })
                
        except Exception as e:
            self.log(f"Error checking threshold violations: {e}", "WARNING")
            
        return violations
        
    def collection_loop(self):
        """Main collection loop"""
        self.log("Starting performance collection loop...")
        
        while self.running:
            try:
                # Collect metrics
                metrics = self.collect_metrics()
                
                if metrics:
                    # Add to history
                    self.metrics_history.append(metrics)
                    
                    # Maintain history size limit
                    if len(self.metrics_history) > self.max_history_size:
                        self.metrics_history = self.metrics_history[-self.max_history_size:]
                        
                    # Check for issues
                    regressions = self.check_performance_regression(metrics)
                    violations = self.check_threshold_violations(metrics)
                    
                    # Log issues
                    for regression in regressions:
                        self.log(f"Performance regression: {regression['message']}", "WARNING")
                        
                    for violation in violations:
                        self.log(f"Threshold violation: {violation['message']}", "WARNING")
                        
                    # Save to file
                    self.save_metrics(metrics)
                    
                # Sleep until next collection
                time.sleep(self.collection_interval)
                
            except Exception as e:
                self.log(f"Error in collection loop: {e}", "ERROR")
                time.sleep(self.collection_interval)  # Continue even on error
                
        self.log("Performance collection loop stopped")
        
    def save_metrics(self, metrics: PerformanceMetrics):
        """Save metrics to file"""
        try:
            # Create session directory
            session_dir = PERFORMANCE_DATA_DIR / self.session_id
            session_dir.mkdir(exist_ok=True)
            
            # Save to JSON file
            json_file = session_dir / "performance_metrics.jsonl"
            with open(json_file, 'a') as f:
                f.write(json.dumps(metrics.to_dict()) + '\n')
                
        except Exception as e:
            self.log(f"Error saving metrics: {e}", "WARNING")
            
    def save_metrics_csv(self, metrics_list: List[PerformanceMetrics], filename: str = None):
        """Save metrics to CSV file"""
        if not filename:
            filename = f"performance_{self.session_id}_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"
            
        try:
            csv_file = PERFORMANCE_DATA_DIR / filename
            
            with open(csv_file, 'w', newline='') as f:
                if metrics_list:
                    # Get field names from first metric
                    fieldnames = list(metrics_list[0].to_dict().keys())
                    writer = csv.DictWriter(f, fieldnames=fieldnames)
                    writer.writeheader()
                    
                    # Write all metrics
                    for metrics in metrics_list:
                        writer.writerow(metrics.to_dict())
                        
            self.log(f"Metrics saved to CSV: {csv_file}")
            
        except Exception as e:
            self.log(f"Error saving metrics to CSV: {e}", "WARNING")
            
    def generate_performance_report(self) -> Dict[str, Any]:
        """Generate performance analysis report"""
        if not self.metrics_history:
            return {"error": "No metrics collected"}
            
        try:
            # Calculate statistics
            fps_values = [m.fps for m in self.metrics_history if m.fps > 0]
            memory_values = [m.memory_usage_mb for m in self.metrics_history if m.memory_usage_mb > 0]
            frame_time_values = [m.frame_time for m in self.metrics_history if m.frame_time > 0]
            
            report = {
                "session_id": self.session_id,
                "total_samples": len(self.metrics_history),
                "duration_minutes": (self.metrics_history[-1].timestamp - self.metrics_history[0].timestamp).total_seconds() / 60,
                "fps": {
                    "average": sum(fps_values) / len(fps_values) if fps_values else 0,
                    "min": min(fps_values) if fps_values else 0,
                    "max": max(fps_values) if fps_values else 0,
                    "std_dev": self.calculate_std_dev(fps_values) if len(fps_values) > 1 else 0
                },
                "memory_usage_mb": {
                    "average": sum(memory_values) / len(memory_values) if memory_values else 0,
                    "min": min(memory_values) if memory_values else 0,
                    "max": max(memory_values) if memory_values else 0,
                    "std_dev": self.calculate_std_dev(memory_values) if len(memory_values) > 1 else 0
                },
                "frame_time_ms": {
                    "average": sum(frame_time_values) / len(frame_time_values) if frame_time_values else 0,
                    "min": min(frame_time_values) if frame_time_values else 0,
                    "max": max(frame_time_values) if frame_time_values else 0,
                    "std_dev": self.calculate_std_dev(frame_time_values) if len(frame_time_values) > 1 else 0
                },
                "threshold_violations": [],
                "performance_regressions": []
            }
            
            # Check for threshold violations in recent metrics
            recent_metrics = self.metrics_history[-10:]  # Last 10 samples
            for metrics in recent_metrics:
                violations = self.check_threshold_violations(metrics)
                if violations:
                    report["threshold_violations"].extend(violations)
                    
            # Check for regressions if baseline exists
            if self.baseline_metrics and recent_metrics:
                regressions = self.check_performance_regression(recent_metrics[-1])
                report["performance_regressions"] = regressions
                
            return report
            
        except Exception as e:
            self.log(f"Error generating performance report: {e}", "ERROR")
            return {"error": str(e)}
            
    def calculate_std_dev(self, values: List[float]) -> float:
        """Calculate standard deviation"""
        if len(values) < 2:
            return 0
            
        mean = sum(values) / len(values)
        variance = sum((x - mean) ** 2 for x in values) / len(values)
        return variance ** 0.5
        
    def start(self, interval: float = None):
        """Start performance collection"""
        if self.running:
            self.log("Collection already running", "WARNING")
            return
            
        if interval is not None:
            self.collection_interval = interval
            
        self.running = True
        self.collection_thread = threading.Thread(target=self.collection_loop, daemon=True)
        self.collection_thread.start()
        
        self.log(f"Performance collection started (interval: {self.collection_interval}s)")
        
    def stop(self):
        """Stop performance collection"""
        if not self.running:
            self.log("Collection not running", "WARNING")
            return
            
        self.running = False
        
        if self.collection_thread:
            self.collection_thread.join(timeout=5)
            
        self.log("Performance collection stopped")
        
    def get_current_metrics(self) -> Optional[PerformanceMetrics]:
        """Get the most recent metrics"""
        if self.metrics_history:
            return self.metrics_history[-1]
        return None
        
    def get_metrics_history(self, limit: int = 100) -> List[PerformanceMetrics]:
        """Get recent metrics history"""
        return self.metrics_history[-limit:]
        
    def set_performance_thresholds(self, thresholds: Dict[str, float]):
        """Update performance thresholds"""
        self.performance_thresholds.update(thresholds)
        self.log(f"Performance thresholds updated: {thresholds}")

def main():
    """Main function for standalone usage"""
    import argparse
    
    parser = argparse.ArgumentParser(description="Performance Data Collection System")
    parser.add_argument("--server", type=str, default="http://localhost:8080", 
                       help="Game server URL")
    parser.add_argument("--session", type=str, default="default",
                       help="Session ID")
    parser.add_argument("--interval", type=float, default=5.0,
                       help="Collection interval in seconds")
    parser.add_argument("--duration", type=float, default=0,
                       help="Collection duration in seconds (0 = unlimited)")
    
    args = parser.parse_args()
    
    print("=" * 70)
    print("PERFORMANCE DATA COLLECTION SYSTEM")
    print("=" * 70)
    print(f"Server: {args.server}")
    print(f"Session: {args.session}")
    print(f"Interval: {args.interval}s")
    print(f"Duration: {args.duration if args.duration > 0 else 'unlimited'}")
    print("=" * 70)
    
    # Create collector
    collector = PerformanceCollector(args.server, args.session)
    
    try:
        # Start collection
        collector.start(args.interval)
        
        # Run for specified duration or until interrupted
        if args.duration > 0:
            time.sleep(args.duration)
            collector.stop()
            
            # Generate and print report
            report = collector.generate_performance_report()
            print("\n" + "=" * 70)
            print("PERFORMANCE REPORT")
            print("=" * 70)
            print(json.dumps(report, indent=2))
            print("=" * 70)
        else:
            print("\nPress Ctrl+C to stop...")
            while True:
                time.sleep(1)
                
    except KeyboardInterrupt:
        print("\nShutting down...")
        collector.stop()
        
    # Print final statistics
    current = collector.get_current_metrics()
    if current:
        print("\n" + "=" * 70)
        print("FINAL METRICS")
        print("=" * 70)
        print(f"FPS: {current.fps:.1f}")
        print(f"Frame Time: {current.frame_time:.1f}ms")
        print(f"Memory: {current.memory_usage_mb:.0f}MB")
        print(f"CPU: {current.cpu_usage_percent:.1f}%")
        print(f"GPU: {current.gpu_usage_percent:.1f}%")
        print("=" * 70)

if __name__ == "__main__":
    main()