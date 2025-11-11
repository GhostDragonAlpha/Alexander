"""
Visual Verification Integration Module
Connects screenshot capture system to analysis pipeline and manages the complete workflow.
"""

import json
import time
import base64
import logging
from typing import Dict, List, Any, Optional, Callable
from pathlib import Path
from dataclasses import dataclass
import requests
import threading
from queue import Queue
import schedule

# Import analysis modules
from visual_analyzer import VisualAnalyzer
from text_recognizer import TextRecognizer
from object_detector import ObjectDetector
from state_comparator import StateComparator, ExpectedGameState
from issue_classifier import IssueClassifier

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

@dataclass
class ScreenshotCapture:
    """Represents a captured screenshot"""
    screenshot_path: str
    timestamp: float
    metadata: Dict[str, Any]
    game_state: Optional[Dict[str, Any]] = None

@dataclass
class AnalysisResult:
    """Complete analysis result for a screenshot"""
    screenshot_id: str
    timestamp: float
    visual_analysis: Optional[Dict[str, Any]] = None
    text_recognition: Optional[Dict[str, Any]] = None
    object_detection: Optional[Dict[str, Any]] = None
    state_comparison: Optional[Dict[str, Any]] = None
    issue_classification: Optional[Dict[str, Any]] = None
    processing_time: float = 0.0
    success: bool = True
    error: Optional[str] = None

@dataclass
class VerificationReport:
    """Comprehensive verification report"""
    report_id: str
    generated_at: float
    screenshot_count: int
    issues_found: int
    critical_issues: int
    recommendations: List[str]
    detailed_results: List[AnalysisResult]

class VisualVerificationIntegration:
    """Main integration class for visual verification pipeline"""
    
    def __init__(self, config_path: str = "visual_analysis_config.json", 
                 server_url: str = "http://localhost:8081"):
        """Initialize the visual verification integration"""
        self.config = self._load_config(config_path)
        self.server_url = server_url
        self.capture_directory = Path("screenshots_visual_playtest")
        self.results_directory = Path("analysis_results")
        self.reports_directory = Path("verification_reports")
        
        # Create directories
        self.capture_directory.mkdir(exist_ok=True)
        self.results_directory.mkdir(exist_ok=True)
        self.reports_directory.mkdir(exist_ok=True)
        
        # Initialize components
        self.visual_analyzer = VisualAnalyzer(config_path)
        self.text_recognizer = TextRecognizer(config_path)
        self.object_detector = ObjectDetector(config_path)
        self.state_comparator = StateComparator(config_path)
        self.issue_classifier = IssueClassifier(config_path)
        
        # Processing queue
        self.analysis_queue = Queue()
        self.is_processing = False
        self.processing_thread = None
        
        # Alert callbacks
        self.alert_callbacks: List[Callable] = []
        
        # Statistics
        self.stats = {
            "total_screenshots": 0,
            "total_analyses": 0,
            "total_issues": 0,
            "critical_issues": 0,
            "average_processing_time": 0.0
        }
        
        logger.info("Visual verification integration initialized")
    
    def _load_config(self, config_path: str) -> Dict[str, Any]:
        """Load configuration from JSON file"""
        try:
            with open(config_path, 'r') as f:
                return json.load(f)
        except Exception as e:
            logger.error(f"Failed to load config: {e}")
            return {}
    
    def start_processing(self):
        """Start the background processing thread"""
        if not self.is_processing:
            self.is_processing = True
            self.processing_thread = threading.Thread(target=self._process_queue, daemon=True)
            self.processing_thread.start()
            logger.info("Started background processing thread")
    
    def stop_processing(self):
        """Stop the background processing thread"""
        self.is_processing = False
        if self.processing_thread:
            self.processing_thread.join(timeout=5)
            logger.info("Stopped background processing thread")
    
    def _process_queue(self):
        """Process screenshots in the queue"""
        while self.is_processing:
            try:
                # Get next screenshot from queue (with timeout)
                try:
                    capture = self.analysis_queue.get(timeout=1.0)
                except:
                    continue  # Timeout, continue loop
                
                # Process screenshot
                self._process_screenshot(capture)
                
                # Mark task as done
                self.analysis_queue.task_done()
                
            except Exception as e:
                logger.error(f"Queue processing error: {e}")
                time.sleep(1)  # Prevent tight loop on errors
    
    def capture_and_analyze(self, screenshot_path: str, metadata: Dict[str, Any] = None, 
                          game_state: Dict[str, Any] = None) -> AnalysisResult:
        """
        Capture and immediately analyze a screenshot
        
        Args:
            screenshot_path: Path to screenshot file
            metadata: Additional metadata about the screenshot
            game_state: Current game state for validation
            
        Returns:
            AnalysisResult containing all analysis results
        """
        try:
            # Create capture object
            capture = ScreenshotCapture(
                screenshot_path=screenshot_path,
                timestamp=time.time(),
                metadata=metadata or {},
                game_state=game_state
            )
            
            # Process immediately
            result = self._process_screenshot(capture)
            
            # Check for critical issues and trigger alerts
            if result.issue_classification:
                critical_count = result.issue_classification.get("critical_count", 0)
                if critical_count > 0:
                    self._trigger_alerts(result)
            
            return result
            
        except Exception as e:
            logger.error(f"Capture and analyze failed: {e}")
            return AnalysisResult(
                screenshot_id=f"error_{int(time.time())}",
                timestamp=time.time(),
                success=False,
                error=str(e)
            )
    
    def queue_screenshot(self, screenshot_path: str, metadata: Dict[str, Any] = None, 
                        game_state: Dict[str, Any] = None):
        """
        Queue a screenshot for background processing
        
        Args:
            screenshot_path: Path to screenshot file
            metadata: Additional metadata about the screenshot
            game_state: Current game state for validation
        """
        try:
            capture = ScreenshotCapture(
                screenshot_path=screenshot_path,
                timestamp=time.time(),
                metadata=metadata or {},
                game_state=game_state
            )
            
            self.analysis_queue.put(capture)
            self.stats["total_screenshots"] += 1
            
            logger.info(f"Queued screenshot: {screenshot_path} (queue size: {self.analysis_queue.qsize()})")
            
        except Exception as e:
            logger.error(f"Failed to queue screenshot: {e}")
    
    def _process_screenshot(self, capture: ScreenshotCapture) -> AnalysisResult:
        """Process a single screenshot through the analysis pipeline"""
        start_time = time.time()
        result = AnalysisResult(
            screenshot_id=f"screenshot_{int(capture.timestamp)}",
            timestamp=capture.timestamp
        )
        
        try:
            # Verify screenshot exists
            if not Path(capture.screenshot_path).exists():
                raise FileNotFoundError(f"Screenshot not found: {capture.screenshot_path}")
            
            # Perform analyses based on configuration
            analysis_options = self.config.get("analysis_options", {
                "visual_analysis": True,
                "text_recognition": True,
                "object_detection": True,
                "state_comparison": bool(capture.game_state),
                "issue_classification": True
            })
            
            # Visual analysis
            if analysis_options.get("visual_analysis", True):
                try:
                    visual_results = self.visual_analyzer.analyze_screenshot(capture.screenshot_path)
                    result.visual_analysis = visual_results
                    logger.debug(f"Visual analysis completed: {len(visual_results.get('ui_elements', []))} UI elements")
                except Exception as e:
                    logger.warning(f"Visual analysis failed: {e}")
            
            # Text recognition
            if analysis_options.get("text_recognition", True):
                try:
                    text_results = self.text_recognizer.recognize_text(capture.screenshot_path)
                    result.text_recognition = {
                        "text_elements_count": len(text_results.text_elements),
                        "average_confidence": text_results.average_confidence,
                        "processing_time": text_results.processing_time,
                        "detected_languages": text_results.detected_languages
                    }
                    logger.debug(f"Text recognition completed: {len(text_results.text_elements)} elements")
                except Exception as e:
                    logger.warning(f"Text recognition failed: {e}")
            
            # Object detection
            if analysis_options.get("object_detection", True):
                try:
                    object_results = self.object_detector.detect_objects(capture.screenshot_path)
                    result.object_detection = {
                        "objects_detected": len(object_results.detected_objects),
                        "model_confidence": object_results.model_confidence,
                        "processing_time": object_results.processing_time,
                        "tracking_info": len(object_results.tracking_info)
                    }
                    logger.debug(f"Object detection completed: {len(object_results.detected_objects)} objects")
                except Exception as e:
                    logger.warning(f"Object detection failed: {e}")
            
            # State comparison
            if analysis_options.get("state_comparison", False) and capture.game_state:
                try:
                    # Create expected state from game state
                    expected_state = self._create_expected_state(capture.game_state)
                    state_results = self.state_comparator.compare_states(
                        capture.screenshot_path, expected_state
                    )
                    result.state_comparison = {
                        "is_valid": state_results.is_valid,
                        "discrepancies_count": len(state_results.discrepancies),
                        "critical_issues": state_results.critical_issues,
                        "warnings": state_results.warnings,
                        "overall_confidence": state_results.overall_confidence,
                        "validation_time": state_results.validation_time
                    }
                    logger.debug(f"State comparison completed: {len(state_results.discrepancies)} discrepancies")
                except Exception as e:
                    logger.warning(f"State comparison failed: {e}")
            
            # Issue classification
            if analysis_options.get("issue_classification", True):
                try:
                    # Prepare classification input
                    classification_input = {}
                    
                    if result.visual_analysis:
                        classification_input["visual_artifacts"] = result.visual_analysis.get("visual_artifacts", [])
                    
                    if result.text_recognition and "text_results" in locals():
                        classification_input["text_elements"] = text_results.text_elements
                    
                    if result.state_comparison and "state_results" in locals():
                        classification_input["state_discrepancies"] = state_results.discrepancies
                    
                    issue_results = self.issue_classifier.classify_issues(classification_input)
                    result.issue_classification = {
                        "total_issues": len(issue_results.classified_issues),
                        "critical_count": issue_results.critical_count,
                        "high_count": issue_results.high_count,
                        "medium_count": issue_results.medium_count,
                        "low_count": issue_results.low_count,
                        "issue_summary": issue_results.issue_summary,
                        "processing_time": issue_results.processing_time
                    }
                    
                    # Update statistics
                    self.stats["total_issues"] += len(issue_results.classified_issues)
                    self.stats["critical_issues"] += issue_results.critical_count
                    
                    logger.debug(f"Issue classification completed: {len(issue_results.classified_issues)} issues")
                except Exception as e:
                    logger.warning(f"Issue classification failed: {e}")
            
            # Calculate total processing time
            result.processing_time = time.time() - start_time
            result.success = True
            
            # Update statistics
            self.stats["total_analyses"] += 1
            self._update_average_processing_time(result.processing_time)
            
            # Store results
            self._store_results(result, capture)
            
            logger.info(f"Screenshot processed successfully in {result.processing_time:.3f}s")
            
        except Exception as e:
            logger.error(f"Screenshot processing failed: {e}")
            result.success = False
            result.error = str(e)
            result.processing_time = time.time() - start_time
        
        return result
    
    def _create_expected_state(self, game_state: Dict[str, Any]) -> ExpectedGameState:
        """Create expected game state from game state data"""
        try:
            return ExpectedGameState(
                ui_elements=game_state.get("ui_elements", []),
                text_elements=game_state.get("text_elements", []),
                object_counts=game_state.get("object_counts", {}),
                player_state=game_state.get("player_state", {}),
                mission_state=game_state.get("mission_state", {}),
                inventory_state=game_state.get("inventory_state", {})
            )
        except Exception as e:
            logger.error(f"Failed to create expected state: {e}")
            return ExpectedGameState(
                ui_elements=[], text_elements=[], object_counts={},
                player_state={}, mission_state={}, inventory_state={}
            )
    
    def _update_average_processing_time(self, processing_time: float):
        """Update average processing time statistic"""
        try:
            if self.stats["average_processing_time"] == 0:
                self.stats["average_processing_time"] = processing_time
            else:
                # Weighted average
                total_time = self.stats["average_processing_time"] * (self.stats["total_analyses"] - 1)
                self.stats["average_processing_time"] = (total_time + processing_time) / self.stats["total_analyses"]
        except Exception as e:
            logger.warning(f"Failed to update average processing time: {e}")
    
    def _store_results(self, result: AnalysisResult, capture: ScreenshotCapture):
        """Store analysis results to disk"""
        try:
            # Create result data
            result_data = {
                "screenshot_id": result.screenshot_id,
                "timestamp": result.timestamp,
                "screenshot_path": capture.screenshot_path,
                "metadata": capture.metadata,
                "game_state": capture.game_state,
                "analysis_results": {
                    "visual_analysis": result.visual_analysis,
                    "text_recognition": result.text_recognition,
                    "object_detection": result.object_detection,
                    "state_comparison": result.state_comparison,
                    "issue_classification": result.issue_classification
                },
                "processing_time": result.processing_time,
                "success": result.success,
                "error": result.error
            }
            
            # Save to file
            result_file = self.results_directory / f"{result.screenshot_id}.json"
            with open(result_file, 'w') as f:
                json.dump(result_data, f, indent=2, default=str)
            
            logger.debug(f"Results stored: {result_file}")
            
        except Exception as e:
            logger.error(f"Failed to store results: {e}")
    
    def add_alert_callback(self, callback: Callable[[AnalysisResult], None]):
        """Add a callback for critical issue alerts"""
        self.alert_callbacks.append(callback)
        logger.info(f"Added alert callback: {callback.__name__}")
    
    def _trigger_alerts(self, result: AnalysisResult):
        """Trigger alert callbacks for critical issues"""
        try:
            for callback in self.alert_callbacks:
                try:
                    callback(result)
                except Exception as e:
                    logger.error(f"Alert callback failed: {e}")
            
            logger.info(f"Triggered {len(self.alert_callbacks)} alert callbacks for critical issues")
            
        except Exception as e:
            logger.error(f"Alert triggering failed: {e}")
    
    def generate_verification_report(self, screenshot_ids: List[str] = None) -> VerificationReport:
        """
        Generate a comprehensive verification report
        
        Args:
            screenshot_ids: List of specific screenshot IDs to include (None for all)
            
        Returns:
            VerificationReport containing aggregated results
        """
        try:
            report_id = f"report_{int(time.time())}"
            generated_at = time.time()
            
            # Load results
            results = []
            if screenshot_ids:
                for screenshot_id in screenshot_ids:
                    result_file = self.results_directory / f"{screenshot_id}.json"
                    if result_file.exists():
                        with open(result_file, 'r') as f:
                            results.append(json.load(f))
            else:
                # Load all results
                for result_file in self.results_directory.glob("*.json"):
                    with open(result_file, 'r') as f:
                        results.append(json.load(f))
            
            # Aggregate statistics
            screenshot_count = len(results)
            issues_found = 0
            critical_issues = 0
            all_recommendations = set()
            
            for result in results:
                if result.get("analysis_results", {}).get("issue_classification"):
                    issue_data = result["analysis_results"]["issue_classification"]
                    issues_found += issue_data.get("total_issues", 0)
                    critical_issues += issue_data.get("critical_count", 0)
                    
                    # Collect recommendations
                    summary = issue_data.get("issue_summary", {})
                    recommendations = summary.get("recommendations", [])
                    all_recommendations.update(recommendations)
            
            # Create report
            report = VerificationReport(
                report_id=report_id,
                generated_at=generated_at,
                screenshot_count=screenshot_count,
                issues_found=issues_found,
                critical_issues=critical_issues,
                recommendations=list(all_recommendations),
                detailed_results=results
            )
            
            # Store report
            self._store_report(report)
            
            logger.info(f"Generated verification report: {report_id} ({screenshot_count} screenshots, {issues_found} issues)")
            
            return report
            
        except Exception as e:
            logger.error(f"Report generation failed: {e}")
            return VerificationReport(
                report_id=f"error_{int(time.time())}",
                generated_at=time.time(),
                screenshot_count=0,
                issues_found=0,
                critical_issues=0,
                recommendations=[],
                detailed_results=[]
            )
    
    def _store_report(self, report: VerificationReport):
        """Store verification report to disk"""
        try:
            report_data = {
                "report_id": report.report_id,
                "generated_at": report.generated_at,
                "screenshot_count": report.screenshot_count,
                "issues_found": report.issues_found,
                "critical_issues": report.critical_issues,
                "recommendations": report.recommendations,
                "detailed_results": report.detailed_results
            }
            
            report_file = self.reports_directory / f"{report.report_id}.json"
            with open(report_file, 'w') as f:
                json.dump(report_data, f, indent=2, default=str)
            
            logger.info(f"Report stored: {report_file}")
            
        except Exception as e:
            logger.error(f"Failed to store report: {e}")
    
    def send_to_server(self, screenshot_path: str, metadata: Dict[str, Any] = None, 
                      game_state: Dict[str, Any] = None) -> Optional[Dict[str, Any]]:
        """
        Send screenshot to analysis server for processing
        
        Args:
            screenshot_path: Path to screenshot file
            metadata: Additional metadata
            game_state: Current game state
            
        Returns:
            Server response or None if failed
        """
        try:
            # Read screenshot
            with open(screenshot_path, 'rb') as f:
                screenshot_data = f.read()
            
            # Encode to base64
            encoded_screenshot = base64.b64encode(screenshot_data).decode('utf-8')
            
            # Prepare request
            request_data = {
                "screenshot_data": f"data:image/png;base64,{encoded_screenshot}",
                "metadata": metadata or {},
                "analysis_options": {
                    "visual_analysis": True,
                    "text_recognition": True,
                    "object_detection": True,
                    "state_comparison": game_state is not None,
                    "issue_classification": True
                }
            }
            
            # Add expected state if provided
            if game_state:
                request_data["expected_state"] = game_state
            
            # Send to server
            response = requests.post(
                f"{self.server_url}/analyze_screenshot",
                json=request_data,
                timeout=30
            )
            
            if response.status_code == 200:
                logger.info(f"Successfully sent screenshot to server: {screenshot_path}")
                return response.json()
            else:
                logger.error(f"Server returned error: {response.status_code} - {response.text}")
                return None
                
        except Exception as e:
            logger.error(f"Failed to send screenshot to server: {e}")
            return None
    
    def schedule_periodic_analysis(self, screenshot_directory: str, interval_minutes: int = 5):
        """
        Schedule periodic analysis of screenshots in a directory
        
        Args:
            screenshot_directory: Directory to monitor for screenshots
            interval_minutes: Analysis interval in minutes
        """
        try:
            def analyze_new_screenshots():
                directory = Path(screenshot_directory)
                if not directory.exists():
                    logger.warning(f"Screenshot directory does not exist: {directory}")
                    return
                
                # Find new screenshots
                screenshot_files = list(directory.glob("*.png")) + list(directory.glob("*.jpg"))
                
                for screenshot_file in screenshot_files:
                    # Check if already analyzed
                    result_file = self.results_directory / f"{screenshot_file.stem}.json"
                    if not result_file.exists():
                        logger.info(f"Found new screenshot: {screenshot_file}")
                        self.queue_screenshot(str(screenshot_file))
            
            # Schedule the task
            schedule.every(interval_minutes).minutes.do(analyze_new_screenshots)
            
            logger.info(f"Scheduled periodic analysis every {interval_minutes} minutes for {screenshot_directory}")
            
            # Run scheduler in background
            def run_scheduler():
                while self.is_processing:
                    schedule.run_pending()
                    time.sleep(60)  # Check every minute
            
            scheduler_thread = threading.Thread(target=run_scheduler, daemon=True)
            scheduler_thread.start()
            
        except Exception as e:
            logger.error(f"Failed to schedule periodic analysis: {e}")
    
    def get_statistics(self) -> Dict[str, Any]:
        """Get integration statistics"""
        return self.stats.copy()
    
    def reset_statistics(self):
        """Reset integration statistics"""
        self.stats = {
            "total_screenshots": 0,
            "total_analyses": 0,
            "total_issues": 0,
            "critical_issues": 0,
            "average_processing_time": 0.0
        }
        logger.info("Statistics reset")
    
    def cleanup_old_results(self, days_to_keep: int = 7):
        """Clean up old analysis results"""
        try:
            cutoff_time = time.time() - (days_to_keep * 24 * 60 * 60)
            
            # Clean results directory
            for result_file in self.results_directory.glob("*.json"):
                if result_file.stat().st_mtime < cutoff_time:
                    result_file.unlink()
                    logger.debug(f"Deleted old result: {result_file}")
            
            # Clean reports directory
            for report_file in self.reports_directory.glob("*.json"):
                if report_file.stat().st_mtime < cutoff_time:
                    report_file.unlink()
                    logger.debug(f"Deleted old report: {report_file}")
            
            logger.info(f"Cleaned up results older than {days_to_keep} days")
            
        except Exception as e:
            logger.error(f"Cleanup failed: {e}")

# Example usage and testing
if __name__ == "__main__":
    import sys
    
    # Create integration instance
    integration = VisualVerificationIntegration()
    
    # Start processing
    integration.start_processing()
    
    # Example: Process a test screenshot if provided
    if len(sys.argv) > 1:
        screenshot_path = sys.argv[1]
        if Path(screenshot_path).exists():
            logger.info(f"Processing test screenshot: {screenshot_path}")
            
            # Add example metadata
            metadata = {
                "test_name": "integration_test",
                "game_version": "1.0.0",
                "screen_resolution": "1920x1080"
            }
            
            # Add example game state
            game_state = {
                "player_health": 100,
                "player_credits": 5000,
                "current_mission": "test_mission",
                "ui_elements": [
                    {"type": "health_bar", "position": {"x": 10, "y": 10}},
                    {"type": "minimap", "position": {"x": 1700, "y": 800}}
                ],
                "object_counts": {
                    "ship_player": 1,
                    "asteroid": 5
                }
            }
            
            # Process screenshot
            result = integration.capture_and_analyze(screenshot_path, metadata, game_state)
            
            # Print results
            if result.success:
                print(f"Analysis completed in {result.processing_time:.3f}s")
                if result.issue_classification:
                    print(f"Issues found: {result.issue_classification['total_issues']} "
                          f"(critical: {result.issue_classification['critical_count']})")
            else:
                print(f"Analysis failed: {result.error}")
            
            # Generate report
            report = integration.generate_verification_report([result.screenshot_id])
            print(f"\nVerification Report: {report.report_id}")
            print(f"Critical issues: {report.critical_issues}")
            print(f"Recommendations: {len(report.recommendations)}")
            
        else:
            logger.error(f"Screenshot not found: {screenshot_path}")
    else:
        logger.info("No screenshot provided. Integration system ready.")
        print("Visual Verification Integration is running.")
        print("Use capture_and_analyze() or queue_screenshot() to process screenshots.")
    
    # Keep running for background processing
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        logger.info("Shutting down integration...")
        integration.stop_processing()