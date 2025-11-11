#!/usr/bin/env python3
"""
Issue Detection and Reporting System
Receives analysis results, correlates with performance data, and generates structured issue reports
"""

import json
import time
import logging
import sqlite3
import base64
from pathlib import Path
from typing import Dict, Any, List, Optional, Tuple
from datetime import datetime
from dataclasses import dataclass, asdict
import requests
import threading

# Configuration
ISSUES_DB = Path("playtesting_data/issues.db")
ISSUES_DIR = Path("playtesting_data/issues")
REPORTS_DIR = Path("playtesting_data/reports")
CONFIG_DIR = Path("automation_config")
LOG_FILE = CONFIG_DIR / "issue_reporter.log"

@dataclass
class IssueReport:
    """Structured issue report"""
    issue_id: str
    timestamp: datetime
    issue_type: str
    severity: str  # critical, high, medium, low
    description: str
    screenshot_base64: Optional[str]
    game_state: Dict[str, Any]
    performance_metrics: Dict[str, Any]
    stack_trace: Optional[str]
    recommended_fix: str
    location: Optional[str]  # File/line number if available
    frequency: int
    first_seen: datetime
    last_seen: datetime
    status: str  # open, investigating, fixed, closed
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary"""
        data = asdict(self)
        data['timestamp'] = self.timestamp.isoformat()
        data['first_seen'] = self.first_seen.isoformat()
        data['last_seen'] = self.last_seen.isoformat()
        return data

class IssueReporter:
    """Issue detection, tracking, and reporting system"""
    
    def __init__(self, server_url: str = "http://localhost:8080", 
                 analysis_server_url: str = "http://localhost:8081"):
        self.server_url = server_url
        self.analysis_server_url = analysis_server_url
        self.running = False
        self.processing_thread: Optional[threading.Thread] = None
        
        # Issue tracking
        self.issues_db_path = ISSUES_DB
        self.known_issues: Dict[str, IssueReport] = {}
        self.issue_queue: List[Dict[str, Any]] = []
        self.queue_lock = threading.Lock()
        
        # Configuration
        self.severity_threshold = "medium"
        self.min_frequency_for_pattern = 3
        self.correlate_with_performance = True
        self.save_screenshots_with_issues = True
        
        # Setup directories
        ISSUES_DIR.mkdir(parents=True, exist_ok=True)
        REPORTS_DIR.mkdir(parents=True, exist_ok=True)
        CONFIG_DIR.mkdir(exist_ok=True)
        
        # Setup logging
        self.setup_logging()
        
        # Initialize database
        self.init_database()
        
        self.log("Issue reporter initialized")
        
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
            
        print(f"[IssueReporter] {level}: {message}")
        
    def init_database(self):
        """Initialize SQLite database for issue tracking"""
        try:
            self.issues_db_path.parent.mkdir(parents=True, exist_ok=True)
            
            conn = sqlite3.connect(self.issues_db_path)
            cursor = conn.cursor()
            
            # Create issues table
            cursor.execute('''
                CREATE TABLE IF NOT EXISTS issues (
                    issue_id TEXT PRIMARY KEY,
                    timestamp TEXT NOT NULL,
                    issue_type TEXT NOT NULL,
                    severity TEXT NOT NULL,
                    description TEXT NOT NULL,
                    screenshot_base64 TEXT,
                    game_state TEXT,
                    performance_metrics TEXT,
                    stack_trace TEXT,
                    recommended_fix TEXT,
                    location TEXT,
                    frequency INTEGER DEFAULT 1,
                    first_seen TEXT NOT NULL,
                    last_seen TEXT NOT NULL,
                    status TEXT DEFAULT 'open'
                )
            ''')
            
            # Create index for faster queries
            cursor.execute('CREATE INDEX IF NOT EXISTS idx_issue_type ON issues(issue_type)')
            cursor.execute('CREATE INDEX IF NOT EXISTS idx_severity ON issues(severity)')
            cursor.execute('CREATE INDEX IF NOT EXISTS idx_status ON issues(status)')
            cursor.execute('CREATE INDEX IF NOT EXISTS idx_timestamp ON issues(timestamp)')
            
            conn.commit()
            conn.close()
            
            self.log("Issues database initialized")
            
        except Exception as e:
            self.log(f"Error initializing database: {e}", "ERROR")
            
    def load_known_issues(self):
        """Load known issues from database"""
        try:
            conn = sqlite3.connect(self.issues_db_path)
            cursor = conn.cursor()
            
            cursor.execute('SELECT * FROM issues WHERE status != "closed"')
            rows = cursor.fetchall()
            
            for row in rows:
                issue = IssueReport(
                    issue_id=row[0],
                    timestamp=datetime.fromisoformat(row[1]),
                    issue_type=row[2],
                    severity=row[3],
                    description=row[4],
                    screenshot_base64=row[5],
                    game_state=json.loads(row[6]) if row[6] else {},
                    performance_metrics=json.loads(row[7]) if row[7] else {},
                    stack_trace=row[8],
                    recommended_fix=row[9],
                    location=row[10],
                    frequency=row[11],
                    first_seen=datetime.fromisoformat(row[12]),
                    last_seen=datetime.fromisoformat(row[13]),
                    status=row[14]
                )
                self.known_issues[issue.issue_id] = issue
                
            conn.close()
            
            self.log(f"Loaded {len(self.known_issues)} known issues from database")
            
        except Exception as e:
            self.log(f"Error loading known issues: {e}", "ERROR")
            
    def generate_issue_id(self, issue_type: str, game_state: Dict[str, Any]) -> str:
        """Generate unique issue ID based on type and context"""
        import hashlib
        
        # Create a hash based on issue type and relevant game state
        context_data = {
            "issue_type": issue_type,
            "map_name": game_state.get("map_name", ""),
            "player_position": game_state.get("player", {}).get("position", {}),
            "game_mode": game_state.get("game_mode", "")
        }
        
        context_str = json.dumps(context_data, sort_keys=True)
        hash_obj = hashlib.md5(context_str.encode())
        
        return f"{issue_type}_{hash_obj.hexdigest()[:8]}"
        
    def get_game_state(self) -> Dict[str, Any]:
        """Get current game state from HTTP API"""
        try:
            response = requests.get(f"{self.server_url}/game_state", timeout=5)
            if response.status_code == 200:
                return response.json()
        except Exception as e:
            self.log(f"Error getting game state: {e}", "WARNING")
            
        return {}
        
    def get_performance_metrics(self) -> Dict[str, Any]:
        """Get current performance metrics"""
        try:
            response = requests.get(f"{self.server_url}/performance_metrics", timeout=5)
            if response.status_code == 200:
                return response.json()
        except Exception as e:
            self.log(f"Error getting performance metrics: {e}", "WARNING")
            
        return {}
        
    def analyze_screenshot(self, screenshot_base64: str) -> Dict[str, Any]:
        """Analyze screenshot for issues using analysis server"""
        try:
            payload = {
                "screenshot_data": screenshot_base64,
                "metadata": {},
                "analysis_options": {
                    "visual_analysis": True,
                    "text_recognition": True,
                    "object_detection": True,
                    "state_comparison": False,
                    "issue_classification": True
                }
            }
            
            response = requests.post(
                f"{self.analysis_server_url}/analyze_screenshot",
                json=payload,
                timeout=30
            )
            
            if response.status_code == 200:
                return response.json()
            else:
                self.log(f"Analysis server returned status {response.status_code}", "WARNING")
                return {}
                
        except Exception as e:
            self.log(f"Error analyzing screenshot: {e}", "WARNING")
            return {}
            
    def detect_issues(self, screenshot_data: Optional[str] = None) -> List[IssueReport]:
        """Detect issues from screenshot and game state"""
        issues = []
        
        try:
            # Get game state
            game_state = self.get_game_state()
            
            # Get performance metrics
            performance_metrics = self.get_performance_metrics()
            
            # Analyze screenshot if provided
            analysis_results = {}
            if screenshot_data:
                analysis_results = self.analyze_screenshot(screenshot_data)
                
            # Check for performance issues
            performance_issues = self.detect_performance_issues(performance_metrics, game_state)
            issues.extend(performance_issues)
            
            # Check for visual issues from analysis
            if analysis_results:
                visual_issues = self.detect_visual_issues(analysis_results, game_state, performance_metrics)
                issues.extend(visual_issues)
                
            # Check for gameplay logic issues
            gameplay_issues = self.detect_gameplay_issues(game_state, performance_metrics)
            issues.extend(gameplay_issues)
            
        except Exception as e:
            self.log(f"Error detecting issues: {e}", "ERROR")
            
        return issues
        
    def detect_performance_issues(self, performance_metrics: Dict[str, Any], 
                                 game_state: Dict[str, Any]) -> List[IssueReport]:
        """Detect performance-related issues"""
        issues = []
        
        try:
            # Check low FPS
            fps = performance_metrics.get("fps", 0)
            if fps < 30:
                issue_id = self.generate_issue_id("low_fps", game_state)
                issues.append(IssueReport(
                    issue_id=issue_id,
                    timestamp=datetime.now(),
                    issue_type="performance",
                    severity="high" if fps < 20 else "medium",
                    description=f"Low FPS detected: {fps:.1f}",
                    screenshot_base64=None,
                    game_state=game_state,
                    performance_metrics=performance_metrics,
                    stack_trace=None,
                    recommended_fix=self.get_fps_recommendation(fps, performance_metrics),
                    location=None,
                    frequency=1,
                    first_seen=datetime.now(),
                    last_seen=datetime.now(),
                    status="open"
                ))
                
            # Check high memory usage
            memory_mb = performance_metrics.get("memory_mb", 0)
            if memory_mb > 4096:  # 4GB
                issue_id = self.generate_issue_id("high_memory", game_state)
                issues.append(IssueReport(
                    issue_id=issue_id,
                    timestamp=datetime.now(),
                    issue_type="performance",
                    severity="high" if memory_mb > 6144 else "medium",
                    description=f"High memory usage: {memory_mb:.0f}MB",
                    screenshot_base64=None,
                    game_state=game_state,
                    performance_metrics=performance_metrics,
                    stack_trace=None,
                    recommended_fix=self.get_memory_recommendation(memory_mb, performance_metrics),
                    location=None,
                    frequency=1,
                    first_seen=datetime.now(),
                    last_seen=datetime.now(),
                    status="open"
                ))
                
            # Check high frame time
            frame_time = performance_metrics.get("frame_time_ms", 0)
            if frame_time > 33.3:  # Above 30 FPS threshold
                issue_id = self.generate_issue_id("high_frame_time", game_state)
                issues.append(IssueReport(
                    issue_id=issue_id,
                    timestamp=datetime.now(),
                    issue_type="performance",
                    severity="high" if frame_time > 50 else "medium",
                    description=f"High frame time: {frame_time:.1f}ms",
                    screenshot_base64=None,
                    game_state=game_state,
                    performance_metrics=performance_metrics,
                    stack_trace=None,
                    recommended_fix=self.get_frame_time_recommendation(frame_time, performance_metrics),
                    location=None,
                    frequency=1,
                    first_seen=datetime.now(),
                    last_seen=datetime.now(),
                    status="open"
                ))
                
        except Exception as e:
            self.log(f"Error detecting performance issues: {e}", "WARNING")
            
        return issues
        
    def detect_visual_issues(self, analysis_results: Dict[str, Any], 
                            game_state: Dict[str, Any],
                            performance_metrics: Dict[str, Any]) -> List[IssueReport]:
        """Detect visual issues from analysis results"""
        issues = []
        
        try:
            results = analysis_results.get("results", {})
            
            # Check for visual artifacts
            if "modules" in results:
                modules = results["modules"]
                
                # Check issue classification results
                if "issue_classification" in modules:
                    classification_results = modules["issue_classification"]["results"]
                    
                    for issue_data in classification_results.get("classified_issues", []):
                        issue_id = self.generate_issue_id(f"visual_{issue_data['type']}", game_state)
                        
                        issues.append(IssueReport(
                            issue_id=issue_id,
                            timestamp=datetime.now(),
                            issue_type="visual",
                            severity=issue_data.get("severity", "medium"),
                            description=issue_data.get("description", "Visual issue detected"),
                            screenshot_base64=None,  # Will be added if needed
                            game_state=game_state,
                            performance_metrics=performance_metrics,
                            stack_trace=None,
                            recommended_fix=issue_data.get("recommended_fix", "Investigate visual settings"),
                            location=issue_data.get("location"),
                            frequency=1,
                            first_seen=datetime.now(),
                            last_seen=datetime.now(),
                            status="open"
                        ))
                        
        except Exception as e:
            self.log(f"Error detecting visual issues: {e}", "WARNING")
            
        return issues
        
    def detect_gameplay_issues(self, game_state: Dict[str, Any], 
                              performance_metrics: Dict[str, Any]) -> List[IssueReport]:
        """Detect gameplay logic issues"""
        issues = []
        
        try:
            # Check for player stuck
            player_data = game_state.get("player", {})
            position = player_data.get("position", {})
            
            if position and self.is_player_stuck(position):
                issue_id = self.generate_issue_id("player_stuck", game_state)
                issues.append(IssueReport(
                    issue_id=issue_id,
                    timestamp=datetime.now(),
                    issue_type="gameplay",
                    severity="high",
                    description="Player appears to be stuck",
                    screenshot_base64=None,
                    game_state=game_state,
                    performance_metrics=performance_metrics,
                    stack_trace=None,
                    recommended_fix="Check collision detection and player movement logic",
                    location=None,
                    frequency=1,
                    first_seen=datetime.now(),
                    last_seen=datetime.now(),
                    status="open"
                ))
                
            # Check for mission progression issues
            mission_state = game_state.get("mission", {})
            if mission_state and self.is_mission_blocked(mission_state):
                issue_id = self.generate_issue_id("mission_blocked", game_state)
                issues.append(IssueReport(
                    issue_id=issue_id,
                    timestamp=datetime.now(),
                    issue_type="gameplay",
                    severity="critical",
                    description="Mission progression blocked",
                    screenshot_base64=None,
                    game_state=game_state,
                    performance_metrics=performance_metrics,
                    stack_trace=None,
                    recommended_fix="Check mission trigger conditions and objectives",
                    location=None,
                    frequency=1,
                    first_seen=datetime.now(),
                    last_seen=datetime.now(),
                    status="open"
                ))
                
        except Exception as e:
            self.log(f"Error detecting gameplay issues: {e}", "WARNING")
            
        return issues
        
    def is_player_stuck(self, position: Dict[str, float]) -> bool:
        """Check if player is stuck (simplified implementation)"""
        # This would track player position history in a real implementation
        return False
        
    def is_mission_blocked(self, mission_state: Dict[str, Any]) -> bool:
        """Check if mission is blocked"""
        # Check if mission has been in same state for too long
        return False
        
    def get_fps_recommendation(self, fps: float, metrics: Dict[str, Any]) -> str:
        """Get FPS improvement recommendation"""
        if fps < 20:
            return "Severe FPS drop detected. Consider: 1) Reducing draw calls, 2) Optimizing shaders, 3) Implementing LODs, 4) Culling distant objects"
        elif fps < 30:
            return "Low FPS detected. Consider: 1) Reducing shadow quality, 2) Lowering texture resolution, 3) Optimizing particle effects"
        else:
            return "Monitor FPS for further drops"
            
    def get_memory_recommendation(self, memory_mb: float, metrics: Dict[str, Any]) -> str:
        """Get memory optimization recommendation"""
        if memory_mb > 6144:  # 6GB
            return "Critical memory usage. Consider: 1) Implementing texture streaming, 2) Reducing asset memory footprint, 3) Unloading unused assets, 4) Implementing object pooling"
        elif memory_mb > 4096:  # 4GB
            return "High memory usage. Consider: 1) Optimizing texture sizes, 2) Reducing mesh complexity, 3) Implementing asset cleanup"
        else:
            return "Monitor memory usage"
            
    def get_frame_time_recommendation(self, frame_time: float, metrics: Dict[str, Any]) -> str:
        """Get frame time optimization recommendation"""
        if frame_time > 50:
            return "Severe frame time spike. Consider: 1) Profiling CPU usage, 2) Optimizing game logic, 3) Reducing physics calculations, 4) Implementing async operations"
        elif frame_time > 33.3:
            return "High frame time detected. Consider: 1) Optimizing tick functions, 2) Reducing component updates, 3) Implementing time slicing"
        else:
            return "Monitor frame time for spikes"
            
    def process_issue(self, issue: IssueReport, screenshot_base64: Optional[str] = None):
        """Process a detected issue"""
        try:
            # Check if this is a known issue
            if issue.issue_id in self.known_issues:
                # Update existing issue
                existing_issue = self.known_issues[issue.issue_id]
                existing_issue.frequency += 1
                existing_issue.last_seen = datetime.now()
                
                # Update severity if it's getting worse
                if issue.severity == "critical" and existing_issue.severity != "critical":
                    existing_issue.severity = "critical"
                    
                # Save updated issue
                self.save_issue(existing_issue)
                
                self.log(f"Updated existing issue: {issue.issue_id} (frequency: {existing_issue.frequency})")
            else:
                # New issue
                if screenshot_base64 and self.save_screenshots_with_issues:
                    issue.screenshot_base64 = screenshot_base64
                    
                self.known_issues[issue.issue_id] = issue
                self.save_issue(issue)
                
                self.log(f"New issue detected: {issue.issue_id} - {issue.description}")
                
                # Check if we should create a GitHub ticket
                if self.should_create_github_ticket(issue):
                    self.create_github_ticket(issue)
                    
        except Exception as e:
            self.log(f"Error processing issue: {e}", "ERROR")
            
    def should_create_github_ticket(self, issue: IssueReport) -> bool:
        """Determine if a GitHub ticket should be created"""
        try:
            # Only create tickets for critical/high severity issues
            if issue.severity not in ["critical", "high"]:
                return False
                
            # Only create tickets for frequent issues
            if issue.frequency < self.min_frequency_for_pattern:
                return False
                
            # Check if issue has been around for a while
            issue_age = datetime.now() - issue.first_seen
            if issue_age.total_seconds() < 3600:  # Less than 1 hour
                return False
                
            return True
            
        except Exception as e:
            self.log(f"Error checking GitHub ticket criteria: {e}", "WARNING")
            return False
            
    def create_github_ticket(self, issue: IssueReport):
        """Create a GitHub-style issue ticket"""
        try:
            # This would integrate with GitHub API in a real implementation
            ticket_data = {
                "title": f"[{issue.severity.upper()}] {issue.description}",
                "body": self.generate_github_issue_body(issue),
                "labels": [issue.issue_type, issue.severity, "auto-detected"]
            }
            
            # Save ticket locally (in real implementation, would post to GitHub API)
            ticket_file = ISSUES_DIR / f"github_ticket_{issue.issue_id}.json"
            with open(ticket_file, 'w') as f:
                json.dump(ticket_data, f, indent=2)
                
            self.log(f"GitHub ticket created: {ticket_file}")
            
        except Exception as e:
            self.log(f"Error creating GitHub ticket: {e}", "ERROR")
            
    def generate_github_issue_body(self, issue: IssueReport) -> str:
        """Generate GitHub issue body"""
        body = f"""## Issue Description
{issue.description}

## Severity
{issue.severity.upper()}

## Frequency
Detected {issue.frequency} times

## First Seen
{issue.first_seen.isoformat()}

## Last Seen
{issue.last_seen.isoformat()}

## Game State
```json
{json.dumps(issue.game_state, indent=2)}
```

## Performance Metrics
```json
{json.dumps(issue.performance_metrics, indent=2)}
```

## Recommended Fix
{issue.recommended_fix}

## Location
{issue.location or "Not specified"}

## Auto-Generated
This issue was automatically detected during playtesting.
"""
        return body
        
    def save_issue(self, issue: IssueReport):
        """Save issue to database"""
        try:
            conn = sqlite3.connect(self.issues_db_path)
            cursor = conn.cursor()
            
            # Check if issue already exists
            cursor.execute('SELECT issue_id FROM issues WHERE issue_id = ?', (issue.issue_id,))
            existing = cursor.fetchone()
            
            if existing:
                # Update existing issue
                cursor.execute('''
                    UPDATE issues 
                    SET frequency = ?, last_seen = ?, severity = ?, status = ?
                    WHERE issue_id = ?
                ''', (issue.frequency, issue.last_seen.isoformat(), issue.severity, issue.status, issue.issue_id))
            else:
                # Insert new issue
                cursor.execute('''
                    INSERT INTO issues (
                        issue_id, timestamp, issue_type, severity, description,
                        screenshot_base64, game_state, performance_metrics,
                        stack_trace, recommended_fix, location, frequency,
                        first_seen, last_seen, status
                    ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
                ''', (
                    issue.issue_id, issue.timestamp.isoformat(), issue.issue_type,
                    issue.severity, issue.description, issue.screenshot_base64,
                    json.dumps(issue.game_state), json.dumps(issue.performance_metrics),
                    issue.stack_trace, issue.recommended_fix, issue.location,
                    issue.frequency, issue.first_seen.isoformat(),
                    issue.last_seen.isoformat(), issue.status
                ))
                
            conn.commit()
            conn.close()
            
        except Exception as e:
            self.log(f"Error saving issue to database: {e}", "ERROR")
            
    def generate_summary_report(self) -> Dict[str, Any]:
        """Generate issue summary report"""
        try:
            conn = sqlite3.connect(self.issues_db_path)
            cursor = conn.cursor()
            
            # Get issue statistics
            cursor.execute('''
                SELECT severity, COUNT(*) as count, SUM(frequency) as total_frequency
                FROM issues
                WHERE status != 'closed'
                GROUP BY severity
            ''')
            
            severity_stats = {}
            for row in cursor.fetchall():
                severity_stats[row[0]] = {
                    "unique_issues": row[1],
                    "total_occurrences": row[2]
                }
                
            # Get top issues by frequency
            cursor.execute('''
                SELECT issue_type, severity, description, frequency, first_seen, last_seen
                FROM issues
                WHERE status != 'closed'
                ORDER BY frequency DESC
                LIMIT 10
            ''')
            
            top_issues = []
            for row in cursor.fetchall():
                top_issues.append({
                    "issue_type": row[0],
                    "severity": row[1],
                    "description": row[2],
                    "frequency": row[3],
                    "first_seen": row[4],
                    "last_seen": row[5]
                })
                
            # Get recent issues
            cursor.execute('''
                SELECT issue_id, issue_type, severity, description, timestamp
                FROM issues
                WHERE timestamp > datetime('now', '-24 hours')
                ORDER BY timestamp DESC
            ''')
            
            recent_issues = []
            for row in cursor.fetchall():
                recent_issues.append({
                    "issue_id": row[0],
                    "issue_type": row[1],
                    "severity": row[2],
                    "description": row[3],
                    "timestamp": row[4]
                })
                
            conn.close()
            
            report = {
                "generated_at": datetime.now().isoformat(),
                "total_open_issues": len(self.known_issues),
                "severity_breakdown": severity_stats,
                "top_issues_by_frequency": top_issues,
                "recent_issues": recent_issues,
                "issue_types": self.get_issue_type_distribution(),
                "recommendations": self.generate_recommendations()
            }
            
            return report
            
        except Exception as e:
            self.log(f"Error generating summary report: {e}", "ERROR")
            return {"error": str(e)}
            
    def get_issue_type_distribution(self) -> Dict[str, int]:
        """Get distribution of issue types"""
        distribution = {}
        
        try:
            conn = sqlite3.connect(self.issues_db_path)
            cursor = conn.cursor()
            
            cursor.execute('''
                SELECT issue_type, COUNT(*) as count
                FROM issues
                WHERE status != 'closed'
                GROUP BY issue_type
            ''')
            
            for row in cursor.fetchall():
                distribution[row[0]] = row[1]
                
            conn.close()
            
        except Exception as e:
            self.log(f"Error getting issue type distribution: {e}", "WARNING")
            
        return distribution
        
    def generate_recommendations(self) -> List[str]:
        """Generate recommendations based on issue patterns"""
        recommendations = []
        
        try:
            # Analyze issue patterns
            type_distribution = self.get_issue_type_distribution()
            
            if type_distribution.get("performance", 0) > 5:
                recommendations.append("Multiple performance issues detected. Consider running comprehensive performance optimization.")
                
            if type_distribution.get("visual", 0) > 3:
                recommendations.append("Multiple visual issues detected. Review rendering pipeline and asset quality settings.")
                
            if type_distribution.get("gameplay", 0) > 2:
                recommendations.append("Gameplay logic issues detected. Review mission system and player interaction code.")
                
            # Check for frequent issues
            conn = sqlite3.connect(self.issues_db_path)
            cursor = conn.cursor()
            
            cursor.execute('''
                SELECT description, frequency
                FROM issues
                WHERE status != 'closed' AND frequency >= ?
                ORDER BY frequency DESC
            ''', (self.min_frequency_for_pattern,))
            
            frequent_issues = cursor.fetchall()
            
            for issue in frequent_issues:
                recommendations.append(f"Frequent issue: '{issue[0]}' (occurred {issue[1]} times). Prioritize fix.")
                
            conn.close()
            
        except Exception as e:
            self.log(f"Error generating recommendations: {e}", "WARNING")
            
        return recommendations
        
    def save_summary_report(self, report: Dict[str, Any]):
        """Save summary report to file"""
        try:
            report_file = REPORTS_DIR / f"issue_summary_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
            
            with open(report_file, 'w') as f:
                json.dump(report, f, indent=2)
                
            self.log(f"Issue summary report saved: {report_file}")
            
        except Exception as e:
            self.log(f"Error saving summary report: {e}", "ERROR")
            
    def processing_loop(self):
        """Main issue processing loop"""
        self.log("Starting issue processing loop...")
        
        while self.running:
            try:
                # Process queued issues
                with self.queue_lock:
                    while self.issue_queue:
                        issue_data = self.issue_queue.pop(0)
                        self.process_queued_issue(issue_data)
                        
                # Sleep briefly
                time.sleep(1)
                
            except Exception as e:
                self.log(f"Error in processing loop: {e}", "ERROR")
                time.sleep(5)  # Wait longer on error
                
        self.log("Issue processing loop stopped")
        
    def process_queued_issue(self, issue_data: Dict[str, Any]):
        """Process a single queued issue"""
        try:
            # Create issue report from queued data
            issue = IssueReport(
                issue_id=issue_data.get("issue_id", self.generate_issue_id("queued", {})),
                timestamp=datetime.now(),
                issue_type=issue_data.get("issue_type", "unknown"),
                severity=issue_data.get("severity", "medium"),
                description=issue_data.get("description", "No description"),
                screenshot_base64=issue_data.get("screenshot_base64"),
                game_state=issue_data.get("game_state", {}),
                performance_metrics=issue_data.get("performance_metrics", {}),
                stack_trace=issue_data.get("stack_trace"),
                recommended_fix=issue_data.get("recommended_fix", "Investigate issue"),
                location=issue_data.get("location"),
                frequency=1,
                first_seen=datetime.now(),
                last_seen=datetime.now(),
                status="open"
            )
            
            self.process_issue(issue, issue_data.get("screenshot_base64"))
            
        except Exception as e:
            self.log(f"Error processing queued issue: {e}", "ERROR")
            
    def queue_issue(self, issue_data: Dict[str, Any]):
        """Queue an issue for processing"""
        with self.queue_lock:
            self.issue_queue.append(issue_data)
            
    def start(self):
        """Start issue processing"""
        if self.running:
            self.log("Issue processing already running", "WARNING")
            return
            
        # Load known issues
        self.load_known_issues()
        
        self.running = True
        self.processing_thread = threading.Thread(target=self.processing_loop, daemon=True)
        self.processing_thread.start()
        
        self.log("Issue reporter started")
        
    def stop(self):
        """Stop issue processing"""
        if not self.running:
            self.log("Issue processing not running", "WARNING")
            return
            
        self.running = False
        
        if self.processing_thread:
            self.processing_thread.join(timeout=5)
            
        self.log("Issue reporter stopped")
        
    def get_issue_stats(self) -> Dict[str, Any]:
        """Get issue statistics"""
        try:
            return {
                "total_known_issues": len(self.known_issues),
                "queue_size": len(self.issue_queue),
                "severity_distribution": {
                    severity: sum(1 for issue in self.known_issues.values() if issue.severity == severity)
                    for severity in ["critical", "high", "medium", "low"]
                },
                "type_distribution": self.get_issue_type_distribution()
            }
        except Exception as e:
            self.log(f"Error getting issue stats: {e}", "WARNING")
            return {}

def main():
    """Main function for standalone usage"""
    import argparse
    
    parser = argparse.ArgumentParser(description="Issue Detection and Reporting System")
    parser.add_argument("--server", type=str, default="http://localhost:8080",
                       help="Game server URL")
    parser.add_argument("--analysis-server", type=str, default="http://localhost:8081",
                       help="Analysis server URL")
    parser.add_argument("--action", type=str, choices=["detect", "report", "stats"],
                       required=True, help="Action to perform")
    
    args = parser.parse_args()
    
    print("=" * 70)
    print("ISSUE DETECTION AND REPORTING SYSTEM")
    print("=" * 70)
    print(f"Game Server: {args.server}")
    print(f"Analysis Server: {args.analysis_server}")
    print("=" * 70)
    
    # Create issue reporter
    reporter = IssueReporter(args.server, args.analysis_server)
    
    try:
        if args.action == "detect":
            print("\nDetecting issues...")
            issues = reporter.detect_issues()
            print(f"Detected {len(issues)} issues:")
            for issue in issues:
                print(f"  - {issue.severity.upper()}: {issue.description}")
                
        elif args.action == "report":
            print("\nGenerating summary report...")
            report = reporter.generate_summary_report()
            reporter.save_summary_report(report)
            print("Report generated and saved")
            
        elif args.action == "stats":
            stats = reporter.get_issue_stats()
            print("\n" + "=" * 70)
            print("ISSUE STATISTICS")
            print("=" * 70)
            print(json.dumps(stats, indent=2))
            print("=" * 70)
            
    except KeyboardInterrupt:
        print("\nShutting down...")
        
    print("\nDone.")

if __name__ == "__main__":
    main()