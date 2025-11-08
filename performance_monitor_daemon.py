#!/usr/bin/env python3
"""
Ship Customization Performance Monitor Daemon

Runs in background during development.
Monitors game performance in real-time.
Alerts when performance dips below target.
Logs performance events.
Generates daily reports.

Usage:
    python performance_monitor_daemon.py [--interval 5] [--log-file perf.log]

Arguments:
    --interval: Check interval in seconds (default: 5)
    --log-file: Log file path (default: performance_monitor.log)
    --fps-threshold: Alert if FPS drops below this (default: 88)
    --frametime-threshold: Alert if frame time exceeds this ms (default: 12)
"""

import subprocess
import os
import sys
import time
import json
import argparse
import threading
from pathlib import Path
from datetime import datetime, timedelta
from typing import Dict, List, Optional
from collections import deque

# ============================================================================
# CONFIGURATION
# ============================================================================

PROJECT_ROOT = Path(__file__).parent
LOGS_DIR = PROJECT_ROOT / 'performance_logs'

class PerformanceMonitorConfig:
    """Configuration for performance monitoring"""

    def __init__(self):
        self.check_interval = 5  # seconds
        self.log_file = LOGS_DIR / 'performance_monitor.log'
        self.fps_threshold = 88.0  # Alert if below this
        self.frametime_threshold = 12.0  # Alert if above this (ms)
        self.memory_threshold_mb = 400.0  # Alert if above this
        self.sample_buffer_size = 60  # Keep last 60 samples
        self.report_interval_hours = 24  # Daily reports
        self.enable_alerts = True
        self.enable_logging = True

# ============================================================================
# PERFORMANCE METRICS BUFFER
# ============================================================================

class PerformanceMetricsBuffer:
    """Circular buffer for performance metrics"""

    def __init__(self, size: int = 60):
        self.size = size
        self.frame_times = deque(maxlen=size)
        self.fps_samples = deque(maxlen=size)
        self.memory_samples = deque(maxlen=size)
        self.timestamps = deque(maxlen=size)
        self.stat_calc_times = deque(maxlen=size)
        self.equip_times = deque(maxlen=size)

    def add_sample(self, frame_time_ms: float, memory_mb: float, stat_calc_ms: float = 0, equip_ms: float = 0):
        """Add a performance sample"""
        fps = 1000.0 / frame_time_ms if frame_time_ms > 0 else 0

        self.frame_times.append(frame_time_ms)
        self.fps_samples.append(fps)
        self.memory_samples.append(memory_mb)
        self.timestamps.append(datetime.now())
        self.stat_calc_times.append(stat_calc_ms)
        self.equip_times.append(equip_ms)

    def get_average_frame_time(self) -> float:
        """Get average frame time"""
        return sum(self.frame_times) / len(self.frame_times) if self.frame_times else 0.0

    def get_average_fps(self) -> float:
        """Get average FPS"""
        return sum(self.fps_samples) / len(self.fps_samples) if self.fps_samples else 0.0

    def get_min_fps(self) -> float:
        """Get minimum FPS"""
        return min(self.fps_samples) if self.fps_samples else 0.0

    def get_max_frame_time(self) -> float:
        """Get maximum frame time"""
        return max(self.frame_times) if self.frame_times else 0.0

    def get_average_memory(self) -> float:
        """Get average memory usage"""
        return sum(self.memory_samples) / len(self.memory_samples) if self.memory_samples else 0.0

    def get_peak_memory(self) -> float:
        """Get peak memory usage"""
        return max(self.memory_samples) if self.memory_samples else 0.0

    def get_average_stat_calc_time(self) -> float:
        """Get average stat calculation time"""
        return sum(self.stat_calc_times) / len(self.stat_calc_times) if self.stat_calc_times else 0.0

    def get_summary(self) -> Dict:
        """Get summary of metrics"""
        return {
            'timestamp': datetime.now().isoformat(),
            'sample_count': len(self.frame_times),
            'avg_frame_time_ms': self.get_average_frame_time(),
            'avg_fps': self.get_average_fps(),
            'min_fps': self.get_min_fps(),
            'max_frame_time_ms': self.get_max_frame_time(),
            'avg_memory_mb': self.get_average_memory(),
            'peak_memory_mb': self.get_peak_memory(),
            'avg_stat_calc_time_ms': self.get_average_stat_calc_time(),
        }

# ============================================================================
# PERFORMANCE MONITORING
# ============================================================================

class PerformanceMonitor:
    """Monitors game performance in real-time"""

    def __init__(self, config: PerformanceMonitorConfig):
        self.config = config
        self.metrics_buffer = PerformanceMetricsBuffer(config.sample_buffer_size)
        self.alerts_log = []
        self.is_running = False
        self.last_report_time = datetime.now()

        # Create logs directory
        LOGS_DIR.mkdir(exist_ok=True)

    def log_message(self, message: str, level: str = "INFO"):
        """Log a message"""
        if not self.config.enable_logging:
            return

        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        log_message = f"[{timestamp}] [{level}] {message}"

        print(log_message)

        with open(self.config.log_file, 'a') as f:
            f.write(log_message + '\n')

    def check_performance(self, frame_time_ms: float, memory_mb: float):
        """Check performance metrics against thresholds"""
        fps = 1000.0 / frame_time_ms if frame_time_ms > 0 else 0

        self.metrics_buffer.add_sample(frame_time_ms, memory_mb)

        alerts = []

        # Check frame time
        if frame_time_ms > self.config.frametime_threshold:
            alerts.append(f"Frame time exceeds budget: {frame_time_ms:.2f}ms > {self.config.frametime_threshold}ms")

        # Check FPS
        if fps < self.config.fps_threshold:
            alerts.append(f"FPS below threshold: {fps:.1f} < {self.config.fps_threshold}")

        # Check memory
        if memory_mb > self.config.memory_threshold_mb:
            alerts.append(f"Memory usage high: {memory_mb:.1f}MB > {self.config.memory_threshold_mb}MB")

        # Log alerts
        if alerts and self.config.enable_alerts:
            for alert in alerts:
                self.log_message(alert, "WARNING")
                self.alerts_log.append({
                    'timestamp': datetime.now().isoformat(),
                    'message': alert,
                    'frame_time_ms': frame_time_ms,
                    'fps': fps,
                    'memory_mb': memory_mb,
                })

    def should_generate_report(self) -> bool:
        """Check if daily report should be generated"""
        now = datetime.now()
        return (now - self.last_report_time) >= timedelta(hours=self.config.report_interval_hours)

    def generate_daily_report(self):
        """Generate daily performance report"""
        if not self.should_generate_report():
            return

        self.last_report_time = datetime.now()

        summary = self.metrics_buffer.get_summary()
        alert_count = len(self.alerts_log)

        report = f"""
================================================================================
                        DAILY PERFORMANCE REPORT
                          {datetime.now().strftime("%Y-%m-%d")}
================================================================================

PERFORMANCE SUMMARY
  Average Frame Time:  {summary['avg_frame_time_ms']:.2f} ms
  Average FPS:         {summary['avg_fps']:.1f}
  Minimum FPS:         {summary['min_fps']:.1f}
  Max Frame Time:      {summary['max_frame_time_ms']:.2f} ms
  Average Memory:      {summary['avg_memory_mb']:.1f} MB
  Peak Memory:         {summary['peak_memory_mb']:.1f} MB

ALERTS
  Total Alerts:        {alert_count}
  Threshold (FPS):     {self.config.fps_threshold}
  Threshold (Time):    {self.config.frametime_threshold} ms

STATUS
  Overall:             {'✓ GOOD' if alert_count == 0 else '✗ ISSUES DETECTED'}

================================================================================
"""

        report_file = LOGS_DIR / f"daily_report_{datetime.now().strftime('%Y%m%d')}.txt"
        with open(report_file, 'a') as f:
            f.write(report)

        self.log_message(f"Daily report generated: {report_file}")

    def get_status(self) -> Dict:
        """Get current performance status"""
        summary = self.metrics_buffer.get_summary()
        alert_count = len(self.alerts_log)

        status_ok = (
            summary['avg_fps'] >= self.config.fps_threshold and
            summary['avg_frame_time_ms'] <= self.config.frametime_threshold and
            summary['avg_memory_mb'] <= self.config.memory_threshold_mb
        )

        return {
            'status': 'GOOD' if status_ok else 'WARNING',
            'metrics': summary,
            'alert_count': alert_count,
            'latest_alerts': self.alerts_log[-5:] if self.alerts_log else [],
        }

    def run(self):
        """Run monitoring loop"""
        self.is_running = True
        self.log_message("Performance Monitor started", "INFO")

        sample_count = 0

        try:
            while self.is_running:
                # Simulate collecting performance data
                # In real implementation, would connect to running game instance
                # and read performance counters via:
                # - Unreal's stat system
                # - Windows Performance Monitor
                # - Custom telemetry from game

                # For now, demonstrate structure
                frame_time_ms = 10.5 + (sample_count % 10) * 0.1
                memory_mb = 300 + (sample_count % 20) * 0.5

                self.check_performance(frame_time_ms, memory_mb)
                self.generate_daily_report()

                sample_count += 1

                time.sleep(self.config.check_interval)

        except KeyboardInterrupt:
            self.log_message("Performance Monitor stopped by user", "INFO")
            self.is_running = False
        except Exception as e:
            self.log_message(f"Error in monitor loop: {e}", "ERROR")
            self.is_running = False

    def stop(self):
        """Stop monitoring"""
        self.is_running = False
        self.log_message("Performance Monitor stopping", "INFO")

# ============================================================================
# MAIN EXECUTION
# ============================================================================

def main():
    """Main daemon execution"""

    parser = argparse.ArgumentParser(
        description='Ship Customization Performance Monitor Daemon'
    )
    parser.add_argument(
        '--interval',
        type=int,
        default=5,
        help='Check interval in seconds'
    )
    parser.add_argument(
        '--log-file',
        type=Path,
        help='Log file path'
    )
    parser.add_argument(
        '--fps-threshold',
        type=float,
        default=88.0,
        help='Alert if FPS drops below this'
    )
    parser.add_argument(
        '--frametime-threshold',
        type=float,
        default=12.0,
        help='Alert if frame time exceeds this ms'
    )

    args = parser.parse_args()

    # Create config
    config = PerformanceMonitorConfig()
    config.check_interval = args.interval
    config.fps_threshold = args.fps_threshold
    config.frametime_threshold = args.frametime_threshold

    if args.log_file:
        config.log_file = args.log_file

    # Create monitor
    monitor = PerformanceMonitor(config)

    print("Ship Customization Performance Monitor Daemon")
    print("=" * 80)
    print(f"Check Interval: {config.check_interval}s")
    print(f"FPS Threshold: {config.fps_threshold}")
    print(f"Frame Time Threshold: {config.frametime_threshold}ms")
    print(f"Log File: {config.log_file}")
    print()
    print("Monitoring started... (Press Ctrl+C to stop)")
    print()

    # Run monitor in background thread
    monitor_thread = threading.Thread(target=monitor.run, daemon=True)
    monitor_thread.start()

    # Main loop for status display
    try:
        while True:
            time.sleep(10)

            # Display status
            status = monitor.get_status()
            metrics = status['metrics']

            print("\r" + "=" * 80)
            print(f"Status: {status['status']} | "
                  f"FPS: {metrics['avg_fps']:.1f} | "
                  f"Time: {metrics['avg_frame_time_ms']:.2f}ms | "
                  f"Memory: {metrics['avg_memory_mb']:.1f}MB | "
                  f"Alerts: {status['alert_count']}")
            print("=" * 80)

    except KeyboardInterrupt:
        print("\n\nShutting down monitor...")
        monitor.stop()
        monitor_thread.join(timeout=5)
        print("Monitor stopped")
        return True

if __name__ == '__main__':
    success = main()
    sys.exit(0 if success else 1)
