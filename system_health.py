#!/usr/bin/env python3
"""
System Health Monitor
Continuously monitors all system components and provides health status dashboard.
"""

import sys
import os
import json
import time
import threading
import requests
import psutil
from pathlib import Path
from typing import Dict, List, Optional
from datetime import datetime
import subprocess

class HealthMonitorError(Exception):
    """Custom exception for health monitor errors"""
    pass

class SystemHealthMonitor:
    def __init__(self, config_path: str = "deployment_config.json"):
        self.config_path = config_path
        self.config = self.load_config()
        self.monitoring = False
        self.health_data = {
            "timestamp": datetime.now().isoformat(),
            "status": "initializing",
            "services": {},
            "system": {},
            "alerts": [],
            "history": []
        }
        self.base_url = "http://localhost"
        self.services = {
            "game": {"port": 8080, "process_name": None},
            "analysis": {"port": 8081, "process_name": None},
            "dashboard": {"port": 8082, "process_name": None}
        }
        self.alert_thresholds = {
            "cpu_usage": 85.0,  # Percentage
            "memory_usage": 80.0,  # Percentage
            "disk_usage": 85.0,  # Percentage
            "response_time": 5.0,  # Seconds
            "service_downtime": 30.0  # Seconds
        }
        
    def log(self, message: str, level: str = "INFO"):
        """Log message with timestamp"""
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        print(f"[{timestamp}] [{level}] {message}")
        
    def load_config(self) -> Dict:
        """Load deployment configuration"""
        try:
            with open(self.config_path, 'r') as f:
                return json.load(f)
        except FileNotFoundError:
            self.log(f"Config file not found: {self.config_path}", "WARNING")
            return {}
            
    def check_service_health(self, service_name: str) -> Dict:
        """Check health of a specific service"""
        service_info = self.services[service_name]
        health_status = {
            "name": service_name,
            "status": "unknown",
            "response_time": 0,
            "uptime": 0,
            "last_check": datetime.now().isoformat(),
            "errors": []
        }
        
        try:
            # Check HTTP health endpoint
            url = f"{self.base_url}:{service_info['port']}/health"
            start_time = time.time()
            response = requests.get(url, timeout=5)
            response_time = time.time() - start_time
            
            if response.status_code == 200:
                health_status["status"] = "healthy"
                health_status["response_time"] = response_time
                
                # Parse uptime if available
                if response.content:
                    try:
                        health_data = response.json()
                        health_status["uptime"] = health_data.get("uptime", 0)
                    except:
                        pass
                        
                if response_time > self.alert_thresholds["response_time"]:
                    health_status["errors"].append(f"Slow response time: {response_time:.2f}s")
                    
            else:
                health_status["status"] = "unhealthy"
                health_status["errors"].append(f"HTTP {response.status_code}")
                
        except requests.exceptions.ConnectionError:
            health_status["status"] = "down"
            health_status["errors"].append("Connection refused")
        except requests.exceptions.Timeout:
            health_status["status"] = "timeout"
            health_status["errors"].append("Request timeout")
        except Exception as e:
            health_status["status"] = "error"
            health_status["errors"].append(str(e))
            
        return health_status
        
    def check_system_resources(self) -> Dict:
        """Check system resource usage"""
        resources = {
            "timestamp": datetime.now().isoformat(),
            "cpu": {
                "usage_percent": psutil.cpu_percent(interval=1),
                "core_count": psutil.cpu_count(),
                "frequency": psutil.cpu_freq()._asdict() if psutil.cpu_freq() else {}
            },
            "memory": {
                "usage_percent": psutil.virtual_memory().percent,
                "total_gb": psutil.virtual_memory().total / (1024**3),
                "available_gb": psutil.virtual_memory().available / (1024**3),
                "used_gb": psutil.virtual_memory().used / (1024**3)
            },
            "disk": {
                "usage_percent": psutil.disk_usage('/').percent,
                "total_gb": psutil.disk_usage('/').total / (1024**3),
                "free_gb": psutil.disk_usage('/').free / (1024**3),
                "used_gb": psutil.disk_usage('/').used / (1024**3)
            },
            "network": {
                "connections": len(psutil.net_connections()),
                "interfaces": len(psutil.net_if_addrs())
            }
        }
        
        return resources
        
    def check_process_health(self) -> Dict:
        """Check health of related processes"""
        processes = {}
        
        # Look for Python processes related to our services
        for proc in psutil.process_iter(['pid', 'name', 'cmdline', 'cpu_percent', 'memory_info']):
            try:
                cmdline = proc.info.get('cmdline', [])
                if cmdline and any('python' in cmd.lower() for cmd in cmdline):
                    for service_name in self.services.keys():
                        if any(service_name in cmd.lower() for cmd in cmdline):
                            processes[service_name] = {
                                "pid": proc.info['pid'],
                                "name": proc.info['name'],
                                "cpu_percent": proc.info['cpu_percent'],
                                "memory_mb": proc.info['memory_info'].rss / (1024**2) if proc.info['memory_info'] else 0,
                                "status": "running"
                            }
                            break
            except (psutil.NoSuchProcess, psutil.AccessDenied):
                pass
                
        return processes
        
    def generate_alerts(self, health_data: Dict):
        """Generate alerts based on health data"""
        alerts = []
        
        # Check service health
        for service_name, service_health in health_data["services"].items():
            if service_health["status"] in ["down", "unhealthy", "timeout", "error"]:
                alerts.append({
                    "level": "critical",
                    "category": "service",
                    "service": service_name,
                    "message": f"{service_name} service is {service_health['status']}",
                    "timestamp": datetime.now().isoformat()
                })
            elif service_health["response_time"] > self.alert_thresholds["response_time"]:
                alerts.append({
                    "level": "warning",
                    "category": "performance",
                    "service": service_name,
                    "message": f"{service_name} response time is slow ({service_health['response_time']:.2f}s)",
                    "timestamp": datetime.now().isoformat()
                })
                
        # Check system resources
        system_data = health_data["system"]
        
        if system_data["cpu"]["usage_percent"] > self.alert_thresholds["cpu_usage"]:
            alerts.append({
                "level": "warning",
                "category": "system",
                "message": f"High CPU usage: {system_data['cpu']['usage_percent']:.1f}%",
                "timestamp": datetime.now().isoformat()
            })
            
        if system_data["memory"]["usage_percent"] > self.alert_thresholds["memory_usage"]:
            alerts.append({
                "level": "warning",
                "category": "system",
                "message": f"High memory usage: {system_data['memory']['usage_percent']:.1f}%",
                "timestamp": datetime.now().isoformat()
            })
            
        if system_data["disk"]["usage_percent"] > self.alert_thresholds["disk_usage"]:
            alerts.append({
                "level": "critical",
                "category": "system",
                "message": f"Critical disk usage: {system_data['disk']['usage_percent']:.1f}%",
                "timestamp": datetime.now().isoformat()
            })
            
        return alerts
        
    def update_health_data(self):
        """Update health data for all components"""
        # Check services
        for service_name in self.services.keys():
            self.health_data["services"][service_name] = self.check_service_health(service_name)
            
        # Check system resources
        self.health_data["system"] = self.check_system_resources()
        
        # Check processes
        self.health_data["processes"] = self.check_process_health()
        
        # Generate alerts
        self.health_data["alerts"] = self.generate_alerts(self.health_data)
        
        # Update timestamp and status
        self.health_data["timestamp"] = datetime.now().isoformat()
        
        # Determine overall status
        service_statuses = [s["status"] for s in self.health_data["services"].values()]
        if any(status in ["down", "error"] for status in service_statuses):
            self.health_data["status"] = "critical"
        elif any(status == "unhealthy" for status in service_statuses):
            self.health_data["status"] = "warning"
        else:
            self.health_data["status"] = "healthy"
            
        # Add to history
        self.health_data["history"].append({
            "timestamp": self.health_data["timestamp"],
            "status": self.health_data["status"],
            "service_count": len(self.health_data["services"]),
            "alert_count": len(self.health_data["alerts"])
        })
        
        # Keep only last 100 history entries
        if len(self.health_data["history"]) > 100:
            self.health_data["history"] = self.health_data["history"][-100:]
            
    def display_health_dashboard(self):
        """Display health dashboard"""
        os.system('cls' if os.name == 'nt' else 'clear')
        
        print("=" * 80)
        print(" " * 25 + "SYSTEM HEALTH DASHBOARD")
        print("=" * 80)
        print(f"Last Update: {self.health_data['timestamp']}")
        print(f"Overall Status: {self.get_status_emoji(self.health_data['status'])} {self.health_data['status'].upper()}")
        print("=" * 80)
        
        # Services Section
        print("\n[SERVICES]:")
        print("-" * 80)
        for service_name, service_health in self.health_data["services"].items():
            status_emoji = self.get_status_emoji(service_health["status"])
            print(f"{status_emoji} {service_name:12} | {service_health['status']:12} | "
                  f"Response: {service_health['response_time']*1000:.0f}ms | "
                  f"Uptime: {self.format_uptime(service_health['uptime'])}")
            
        # System Resources Section
        print("\n[SYSTEM RESOURCES]:")
        print("-" * 80)
        system_data = self.health_data["system"]
        
        cpu_status = "[HIGH]" if system_data["cpu"]["usage_percent"] > self.alert_thresholds["cpu_usage"] else "[OK]"
        print(f"{cpu_status} CPU Usage:    {system_data['cpu']['usage_percent']:6.1f}% | "
              f"Cores: {system_data['cpu']['core_count']}")
        
        memory_status = "[HIGH]" if system_data["memory"]["usage_percent"] > self.alert_thresholds["memory_usage"] else "[OK]"
        print(f"{memory_status} Memory Usage: {system_data['memory']['usage_percent']:6.1f}% | "
              f"Used: {system_data['memory']['used_gb']:.1f}GB / {system_data['memory']['total_gb']:.1f}GB")
        
        disk_status = "[HIGH]" if system_data["disk"]["usage_percent"] > self.alert_thresholds["disk_usage"] else "[OK]"
        print(f"{disk_status} Disk Usage:   {system_data['disk']['usage_percent']:6.1f}% | "
              f"Free: {system_data['disk']['free_gb']:.1f}GB / {system_data['disk']['total_gb']:.1f}GB")
        
        # Alerts Section
        if self.health_data["alerts"]:
            print("\n[ACTIVE ALERTS]:")
            print("-" * 80)
            for alert in self.health_data["alerts"]:
                level_status = "[CRIT]" if alert["level"] == "critical" else "[WARN]"
                print(f"{level_status} [{alert['category']}] {alert['message']}")
        else:
            print("\n[No active alerts]")
            
        # Process Section
        if self.health_data.get("processes"):
            print("\n[PROCESSES]:")
            print("-" * 80)
            for proc_name, proc_info in self.health_data["processes"].items():
                print(f"[OK] {proc_name:12} | PID: {proc_info['pid']:6} | "
                      f"CPU: {proc_info['cpu_percent']:5.1f}% | "
                      f"Memory: {proc_info['memory_mb']:6.1f}MB")
        
        print("\n" + "=" * 80)
        print("Press Ctrl+C to stop monitoring")
        print("=" * 80)
        
    def get_status_emoji(self, status: str) -> str:
        """Get emoji for status"""
        status_emojis = {
            "healthy": "[OK]",
            "warning": "[WARN]",
            "critical": "[CRIT]",
            "down": "[DOWN]",
            "unhealthy": "[UNH]",
            "timeout": "[TIME]",
            "error": "[ERR]",
            "unknown": "[?]"
        }
        return status_emojis.get(status, "[?]")
        
    def format_uptime(self, uptime_seconds: float) -> str:
        """Format uptime in human readable format"""
        if uptime_seconds < 60:
            return f"{int(uptime_seconds)}s"
        elif uptime_seconds < 3600:
            return f"{int(uptime_seconds / 60)}m"
        elif uptime_seconds < 86400:
            return f"{int(uptime_seconds / 3600)}h {int((uptime_seconds % 3600) / 60)}m"
        else:
            return f"{int(uptime_seconds / 86400)}d {int((uptime_seconds % 86400) / 3600)}h"
            
    def save_health_report(self):
        """Save health report to file"""
        report_path = "verification_reports/health_report.json"
        with open(report_path, 'w') as f:
            json.dump(self.health_data, f, indent=2)
            
        # Save a summary version
        summary_path = "verification_reports/health_summary.json"
        summary_data = {
            "timestamp": self.health_data["timestamp"],
            "status": self.health_data["status"],
            "services": {name: data["status"] for name, data in self.health_data["services"].items()},
            "system": {
                "cpu_percent": self.health_data["system"]["cpu"]["usage_percent"],
                "memory_percent": self.health_data["system"]["memory"]["usage_percent"],
                "disk_percent": self.health_data["system"]["disk"]["usage_percent"]
            },
            "alert_count": len(self.health_data["alerts"])
        }
        
        with open(summary_path, 'w') as f:
            json.dump(summary_data, f, indent=2)
            
    def monitor_continuously(self, interval: int = 10):
        """Monitor system health continuously"""
        self.log("Starting continuous health monitoring")
        self.monitoring = True
        
        try:
            while self.monitoring:
                # Update health data
                self.update_health_data()
                
                # Display dashboard
                self.display_health_dashboard()
                
                # Save report
                self.save_health_report()
                
                # Wait for next update
                time.sleep(interval)
                
        except KeyboardInterrupt:
            self.log("Monitoring stopped by user")
            self.monitoring = False
        except Exception as e:
            self.log(f"Monitoring error: {e}", "ERROR")
            self.monitoring = False
            
    def start_monitoring(self, interval: int = 10):
        """Start monitoring in a separate thread"""
        monitor_thread = threading.Thread(
            target=self.monitor_continuously,
            args=(interval,),
            daemon=True
        )
        monitor_thread.start()
        
        try:
            # Keep main thread alive
            while self.monitoring:
                time.sleep(1)
        except KeyboardInterrupt:
            self.log("Shutting down health monitor...")
            self.monitoring = False
            
    def generate_health_certificate(self):
        """Generate health certificate"""
        certificate = {
            "timestamp": datetime.now().isoformat(),
            "system_status": self.health_data["status"],
            "services": {
                name: {
                    "status": data["status"],
                    "response_time_ms": data["response_time"] * 1000,
                    "uptime_seconds": data["uptime"]
                }
                for name, data in self.health_data["services"].items()
            },
            "system_resources": {
                "cpu_usage_percent": self.health_data["system"]["cpu"]["usage_percent"],
                "memory_usage_percent": self.health_data["system"]["memory"]["usage_percent"],
                "disk_usage_percent": self.health_data["system"]["disk"]["usage_percent"]
            },
            "alerts": len(self.health_data["alerts"]),
            "certificate_issued": True
        }
        
        # Save certificate
        cert_path = "verification_reports/health_certificate.json"
        with open(cert_path, 'w') as f:
            json.dump(certificate, f, indent=2)
            
        self.log(f"Health certificate saved to: {cert_path}")
        return certificate

def main():
    """Main entry point"""
    import argparse
    
    parser = argparse.ArgumentParser(description="System Health Monitor")
    parser.add_argument("--interval", type=int, default=10, 
                       help="Monitoring interval in seconds")
    parser.add_argument("--once", action="store_true",
                       help="Run once and exit")
    parser.add_argument("--certificate", action="store_true",
                       help="Generate health certificate")
    
    args = parser.parse_args()
    
    monitor = SystemHealthMonitor()
    
    if args.certificate:
        # Update health data once and generate certificate
        monitor.update_health_data()
        monitor.generate_health_certificate()
    elif args.once:
        # Run once and display dashboard
        monitor.update_health_data()
        monitor.display_health_dashboard()
        monitor.save_health_report()
    else:
        # Start continuous monitoring
        monitor.start_monitoring(interval=args.interval)

if __name__ == "__main__":
    main()