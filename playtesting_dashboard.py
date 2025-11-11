#!/usr/bin/env python3
"""
Playtesting Dashboard
Web dashboard providing real-time view of playtesting progress and results
"""

import json
import time
import logging
from pathlib import Path
from typing import Dict, Any, List
from datetime import datetime
from dataclasses import dataclass

# Flask for web server
from flask import Flask, render_template, jsonify, request
from flask_cors import CORS
import threading

# Configuration
DASHBOARD_CONFIG = {
    "port": 8082,
    "host": "localhost",
    "refresh_interval_seconds": 5,
    "max_displayed_issues": 50,
    "show_performance_graphs": True,
    "show_screenshot_preview": True,
    "enable_test_controls": True,
    "theme": "dark"
}

class PlaytestingDashboard:
    """Web dashboard for real-time playtesting monitoring"""
    
    def __init__(self, config_path: str = "playtesting_config.json"):
        self.config = self.load_config(config_path)
        self.app = Flask(__name__)
        CORS(self.app)
        
        # Dashboard state
        self.current_session = None
        self.current_iteration = 0
        self.test_results: List[Dict[str, Any]] = []
        self.performance_metrics: List[Dict[str, Any]] = []
        self.issues: List[Dict[str, Any]] = []
        self.screenshot_preview = None
        
        # Server configuration
        dashboard_config = self.config.get("dashboard", {})
        self.port = dashboard_config.get("port", 8082)
        self.host = dashboard_config.get("host", "localhost")
        self.refresh_interval = dashboard_config.get("refresh_interval_seconds", 5)
        
        # Setup logging
        self.setup_logging()
        
        # Setup routes
        self.setup_routes()
        
        self.log("Playtesting dashboard initialized")
        
    def setup_logging(self):
        """Setup logging configuration"""
        logging.basicConfig(
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
        print(f"[Dashboard] {level}: {message}")
        
    def load_config(self, config_path: str) -> Dict[str, Any]:
        """Load configuration from file"""
        try:
            with open(config_path, 'r') as f:
                return json.load(f)
        except Exception as e:
            self.log(f"Error loading config: {e}, using defaults", "WARNING")
            return DASHBOARD_CONFIG
            
    def setup_routes(self):
        """Setup Flask routes"""
        
        @self.app.route('/')
        def index():
            """Main dashboard page"""
            return render_template('dashboard.html', 
                                 refresh_interval=self.refresh_interval * 1000)  # Convert to ms
        
        @self.app.route('/api/status')
        def get_status():
            """Get current playtesting status"""
            return jsonify({
                "success": True,
                "session_id": self.current_session,
                "current_iteration": self.current_iteration,
                "total_iterations": len(self.test_results),
                "timestamp": datetime.now().isoformat(),
                "services": {
                    "screenshot_capture": self.is_service_running("screenshot"),
                    "performance_collection": self.is_service_running("performance"),
                    "issue_detection": self.is_service_running("issues")
                }
            })
        
        @self.app.route('/api/test_results')
        def get_test_results():
            """Get test results"""
            return jsonify({
                "success": True,
                "results": self.test_results[-20:],  # Last 20 results
                "summary": self.get_test_summary()
            })
        
        @self.app.route('/api/performance')
        def get_performance():
            """Get performance metrics"""
            return jsonify({
                "success": True,
                "metrics": self.performance_metrics[-100:],  # Last 100 metrics
                "current": self.performance_metrics[-1] if self.performance_metrics else None,
                "summary": self.get_performance_summary()
            })
        
        @self.app.route('/api/issues')
        def get_issues():
            """Get detected issues"""
            return jsonify({
                "success": True,
                "issues": self.issues[:self.config.get("max_displayed_issues", 50)],
                "summary": self.get_issues_summary()
            })
        
        @self.app.route('/api/screenshot')
        def get_screenshot():
            """Get current screenshot"""
            if self.screenshot_preview:
                return jsonify({
                    "success": True,
                    "screenshot": self.screenshot_preview
                })
            else:
                return jsonify({
                    "success": False,
                    "error": "No screenshot available"
                })
        
        @self.app.route('/api/control', methods=['POST'])
        def control():
            """Control playtesting (start/stop)"""
            try:
                data = request.get_json()
                action = data.get('action')
                
                if action == 'start':
                    # Start playtesting
                    self.start_playtesting()
                    return jsonify({"success": True, "message": "Playtesting started"})
                elif action == 'stop':
                    # Stop playtesting
                    self.stop_playtesting()
                    return jsonify({"success": True, "message": "Playtesting stopped"})
                else:
                    return jsonify({"success": False, "error": "Unknown action"}), 400
                    
            except Exception as e:
                return jsonify({"success": False, "error": str(e)}), 500
        
        @self.app.route('/api/config', methods=['GET', 'POST'])
        def config():
            """Get or update configuration"""
            if request.method == 'GET':
                return jsonify({
                    "success": True,
                    "config": self.config
                })
            else:
                try:
                    new_config = request.get_json()
                    self.update_config(new_config)
                    return jsonify({"success": True, "message": "Configuration updated"})
                except Exception as e:
                    return jsonify({"success": False, "error": str(e)}), 500
    
    def is_service_running(self, service_type: str) -> bool:
        """Check if a service is running"""
        # This would check actual service status in a real implementation
        return True
        
    def get_test_summary(self) -> Dict[str, Any]:
        """Get test results summary"""
        if not self.test_results:
            return {"total": 0, "passed": 0, "failed": 0, "error": 0}
            
        summary = {
            "total": len(self.test_results),
            "passed": sum(1 for r in self.test_results if r.get('status') == 'passed'),
            "failed": sum(1 for r in self.test_results if r.get('status') in ['failed', 'timeout']),
            "error": sum(1 for r in self.test_results if r.get('status') == 'error')
        }
        
        # Calculate success rate
        if summary["total"] > 0:
            summary["success_rate"] = summary["passed"] / summary["total"] * 100
        else:
            summary["success_rate"] = 0
            
        return summary
        
    def get_performance_summary(self) -> Dict[str, Any]:
        """Get performance metrics summary"""
        if not self.performance_metrics:
            return {"avg_fps": 0, "avg_memory": 0, "min_fps": 0, "max_fps": 0}
            
        fps_values = [m.get('fps', 0) for m in self.performance_metrics if m.get('fps', 0) > 0]
        memory_values = [m.get('memory_usage_mb', 0) for m in self.performance_metrics if m.get('memory_usage_mb', 0) > 0]
        
        if not fps_values:
            return {"avg_fps": 0, "avg_memory": 0, "min_fps": 0, "max_fps": 0}
            
        return {
            "avg_fps": sum(fps_values) / len(fps_values),
            "min_fps": min(fps_values),
            "max_fps": max(fps_values),
            "avg_memory": sum(memory_values) / len(memory_values) if memory_values else 0
        }
        
    def get_issues_summary(self) -> Dict[str, Any]:
        """Get issues summary"""
        if not self.issues:
            return {"total": 0, "critical": 0, "high": 0, "medium": 0, "low": 0}
            
        summary = {
            "total": len(self.issues),
            "critical": sum(1 for i in self.issues if i.get('severity') == 'critical'),
            "high": sum(1 for i in self.issues if i.get('severity') == 'high'),
            "medium": sum(1 for i in self.issues if i.get('severity') == 'medium'),
            "low": sum(1 for i in self.issues if i.get('severity') == 'low')
        }
        
        return summary
        
    def update_config(self, new_config: Dict[str, Any]):
        """Update configuration"""
        self.config.update(new_config)
        
        # Update server settings if changed
        if "port" in new_config:
            self.port = new_config["port"]
        if "host" in new_config:
            self.host = new_config["host"]
        if "refresh_interval_seconds" in new_config:
            self.refresh_interval = new_config["refresh_interval_seconds"]
            
        self.log("Configuration updated")
        
    def update_session_data(self, session_id: str, iteration: int):
        """Update current session data"""
        self.current_session = session_id
        self.current_iteration = iteration
        
    def add_test_result(self, result: Dict[str, Any]):
        """Add test result"""
        self.test_results.append(result)
        
    def add_performance_metrics(self, metrics: Dict[str, Any]):
        """Add performance metrics"""
        self.performance_metrics.append(metrics)
        
        # Keep only last 1000 metrics to prevent memory issues
        if len(self.performance_metrics) > 1000:
            self.performance_metrics = self.performance_metrics[-1000:]
            
    def add_issues(self, issues: List[Dict[str, Any]]):
        """Add detected issues"""
        self.issues.extend(issues)
        
        # Keep only last 1000 issues to prevent memory issues
        if len(self.issues) > 1000:
            self.issues = self.issues[-1000:]
            
    def update_screenshot(self, screenshot_data: str):
        """Update screenshot preview"""
        self.screenshot_preview = screenshot_data
        
    def start_playtesting(self):
        """Start playtesting (placeholder)"""
        self.log("Starting playtesting...")
        # In real implementation, this would start the continuous_playtesting.py process
        
    def stop_playtesting(self):
        """Stop playtesting (placeholder)"""
        self.log("Stopping playtesting...")
        # In real implementation, this would stop the continuous_playtesting.py process
        
    def run(self):
        """Run the dashboard server"""
        try:
            self.log(f"Starting dashboard server on {self.host}:{self.port}")
            
            # Create simple HTML template if it doesn't exist
            self.create_template()
            
            self.app.run(
                host=self.host,
                port=self.port,
                debug=False,
                threaded=True
            )
            
        except Exception as e:
            self.log(f"Failed to start dashboard: {e}", "ERROR")
            raise
            
    def create_template(self):
        """Create dashboard HTML template"""
        template_dir = Path("templates")
        template_dir.mkdir(exist_ok=True)
        
        template_file = template_dir / "dashboard.html"
        if not template_file.exists():
            html_content = """
<!DOCTYPE html>
<html>
<head>
    <title>Playtesting Dashboard</title>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f0f0f0;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        .header {
            background-color: #333;
            color: white;
            padding: 20px;
            border-radius: 5px;
            margin-bottom: 20px;
        }
        .status-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 20px;
        }
        .status-card {
            background-color: white;
            padding: 20px;
            border-radius: 5px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        .status-card h3 {
            margin-top: 0;
            color: #333;
        }
        .metric {
            display: flex;
            justify-content: space-between;
            margin: 10px 0;
        }
        .metric-value {
            font-weight: bold;
            color: #007bff;
        }
        .test-result {
            padding: 10px;
            margin: 5px 0;
            border-radius: 3px;
        }
        .test-result.passed {
            background-color: #d4edda;
            color: #155724;
        }
        .test-result.failed {
            background-color: #f8d7da;
            color: #721c24;
        }
        .test-result.error {
            background-color: #fff3cd;
            color: #856404;
        }
        .issue-critical {
            border-left: 4px solid #dc3545;
            padding-left: 10px;
        }
        .issue-high {
            border-left: 4px solid #fd7e14;
            padding-left: 10px;
        }
        .issue-medium {
            border-left: 4px solid #ffc107;
            padding-left: 10px;
        }
        .issue-low {
            border-left: 4px solid #28a745;
            padding-left: 10px;
        }
        .screenshot-container {
            text-align: center;
            margin: 20px 0;
        }
        .screenshot-container img {
            max-width: 100%;
            border: 1px solid #ddd;
            border-radius: 5px;
        }
        .controls {
            text-align: center;
            margin: 20px 0;
        }
        button {
            padding: 10px 20px;
            margin: 0 10px;
            font-size: 16px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
        }
        button.start {
            background-color: #28a745;
            color: white;
        }
        button.stop {
            background-color: #dc3545;
            color: white;
        }
        button:hover {
            opacity: 0.8;
        }
        .refresh-indicator {
            text-align: center;
            color: #666;
            font-size: 12px;
            margin-top: 10px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>Playtesting Dashboard</h1>
            <p>Session: <span id="session-id">-</span> | Iteration: <span id="iteration">-</span></p>
        </div>
        
        <div class="controls">
            <button class="start" onclick="startPlaytesting()">Start Playtesting</button>
            <button class="stop" onclick="stopPlaytesting()">Stop Playtesting</button>
        </div>
        
        <div class="status-grid">
            <div class="status-card">
                <h3>Test Results</h3>
                <div id="test-summary"></div>
                <div id="recent-tests"></div>
            </div>
            
            <div class="status-card">
                <h3>Performance</h3>
                <div id="performance-summary"></div>
            </div>
            
            <div class="status-card">
                <h3>Issues</h3>
                <div id="issues-summary"></div>
            </div>
        </div>
        
        <div class="status-card">
            <h3>Current Screenshot</h3>
            <div class="screenshot-container">
                <img id="screenshot" src="" alt="No screenshot available" style="display:none;">
                <p id="no-screenshot">No screenshot available</p>
            </div>
        </div>
        
        <div class="status-card">
            <h3>Recent Issues</h3>
            <div id="recent-issues"></div>
        </div>
        
        <div class="refresh-indicator">
            Auto-refreshing every <span id="refresh-interval">-</span> seconds
        </div>
    </div>
    
    <script>
        let refreshInterval;
        
        function updateDashboard() {
            // Update status
            fetch('/api/status')
                .then(response => response.json())
                .then(data => {
                    if (data.success) {
                        document.getElementById('session-id').textContent = data.session_id || '-';
                        document.getElementById('iteration').textContent = data.current_iteration || '-';
                    }
                });
            
            // Update test results
            fetch('/api/test_results')
                .then(response => response.json())
                .then(data => {
                    if (data.success) {
                        const summary = data.summary;
                        document.getElementById('test-summary').innerHTML = `
                            <div class="metric">
                                <span>Total Tests:</span>
                                <span class="metric-value">${summary.total}</span>
                            </div>
                            <div class="metric">
                                <span>Success Rate:</span>
                                <span class="metric-value">${summary.success_rate.toFixed(1)}%</span>
                            </div>
                        `;
                        
                        const recentTests = document.getElementById('recent-tests');
                        recentTests.innerHTML = data.results.slice(-5).map(result => `
                            <div class="test-result ${result.status}">
                                Iteration ${result.iteration}: ${result.scenario} - ${result.status}
                            </div>
                        `).join('');
                    }
                });
            
            // Update performance
            fetch('/api/performance')
                .then(response => response.json())
                .then(data => {
                    if (data.success) {
                        const summary = data.summary;
                        document.getElementById('performance-summary').innerHTML = `
                            <div class="metric">
                                <span>Avg FPS:</span>
                                <span class="metric-value">${summary.avg_fps.toFixed(1)}</span>
                            </div>
                            <div class="metric">
                                <span>Min/Max FPS:</span>
                                <span class="metric-value">${summary.min_fps.toFixed(0)}/${summary.max_fps.toFixed(0)}</span>
                            </div>
                            <div class="metric">
                                <span>Avg Memory:</span>
                                <span class="metric-value">${summary.avg_memory.toFixed(0)} MB</span>
                            </div>
                        `;
                    }
                });
            
            // Update issues
            fetch('/api/issues')
                .then(response => response.json())
                .then(data => {
                    if (data.success) {
                        const summary = data.summary;
                        document.getElementById('issues-summary').innerHTML = `
                            <div class="metric">
                                <span>Total Issues:</span>
                                <span class="metric-value">${summary.total}</span>
                            </div>
                            <div class="metric">
                                <span>Critical:</span>
                                <span class="metric-value" style="color: #dc3545;">${summary.critical}</span>
                            </div>
                            <div class="metric">
                                <span>High:</span>
                                <span class="metric-value" style="color: #fd7e14;">${summary.high}</span>
                            </div>
                        `;
                        
                        const recentIssues = document.getElementById('recent-issues');
                        recentIssues.innerHTML = data.issues.slice(-10).map(issue => `
                            <div class="issue-${issue.severity}">
                                <strong>${issue.severity.toUpperCase()}</strong>: ${issue.description}
                            </div>
                        `).join('');
                    }
                });
            
            // Update screenshot
            fetch('/api/screenshot')
                .then(response => response.json())
                .then(data => {
                    const screenshotImg = document.getElementById('screenshot');
                    const noScreenshot = document.getElementById('no-screenshot');
                    
                    if (data.success && data.screenshot) {
                        screenshotImg.src = 'data:image/jpeg;base64,' + data.screenshot;
                        screenshotImg.style.display = 'block';
                        noScreenshot.style.display = 'none';
                    } else {
                        screenshotImg.style.display = 'none';
                        noScreenshot.style.display = 'block';
                    }
                });
        }
        
        function startPlaytesting() {
            fetch('/api/control', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({action: 'start'})
            })
            .then(response => response.json())
            .then(data => {
                alert(data.message);
            });
        }
        
        function stopPlaytesting() {
            fetch('/api/control', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({action: 'stop'})
            })
            .then(response => response.json())
            .then(data => {
                alert(data.message);
            });
        }
        
        // Initialize dashboard
        document.addEventListener('DOMContentLoaded', function() {
            // Set refresh interval
            const refreshInterval = {{ refresh_interval }};
            document.getElementById('refresh-interval').textContent = refreshInterval;
            
            // Initial update
            updateDashboard();
            
            // Set up auto-refresh
            setInterval(updateDashboard, refreshInterval * 1000);
        });
    </script>
</body>
</html>
"""
            
            with open(template_file, 'w') as f:
                f.write(html_content)
                
            self.log(f"Dashboard template created: {template_file}")
            
        except Exception as e:
            self.log(f"Error creating dashboard template: {e}", "ERROR")

def main():
    """Main function"""
    import argparse
    
    parser = argparse.ArgumentParser(description="Playtesting Dashboard")
    parser.add_argument("--config", type=str, default="playtesting_config.json",
                       help="Configuration file path")
    parser.add_argument("--port", type=int, help="Dashboard port")
    parser.add_argument("--host", type=str, help="Dashboard host")
    
    args = parser.parse_args()
    
    print("=" * 70)
    print("PLAYTESTING DASHBOARD")
    print("=" * 70)
    
    # Create dashboard
    dashboard = PlaytestingDashboard(args.config)
    
    # Override config with command line args
    if args.port:
        dashboard.port = args.port
    if args.host:
        dashboard.host = args.host
        
    print(f"Starting dashboard on {dashboard.host}:{dashboard.port}")
    print("=" * 70)
    
    try:
        dashboard.run()
    except KeyboardInterrupt:
        print("\nShutting down dashboard...")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()