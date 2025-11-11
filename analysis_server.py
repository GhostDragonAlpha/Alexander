"""
Analysis API Server for Visual Verification Pipeline
Provides REST API endpoints for screenshot analysis and state comparison.
"""

import json
import time
import base64
import io
from typing import Dict, List, Any, Optional
from dataclasses import dataclass
from pathlib import Path
import logging

# Flask for web server
from flask import Flask, request, jsonify
from flask_cors import CORS
import cv2
import numpy as np
from PIL import Image

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
class AnalysisRequest:
    """Request for screenshot analysis"""
    screenshot_data: str  # Base64 encoded image
    metadata: Dict[str, Any]
    analysis_options: Dict[str, bool]

@dataclass
class AnalysisResponse:
    """Response from screenshot analysis"""
    success: bool
    analysis_id: str
    timestamp: float
    processing_time: float
    results: Dict[str, Any]
    error: Optional[str] = None

class AnalysisServer:
    """API server for visual verification pipeline"""
    
    def __init__(self, config_path: str = "visual_analysis_config.json"):
        """Initialize the analysis server"""
        self.config = self._load_config(config_path)
        self.app = Flask(__name__)
        CORS(self.app)  # Enable CORS for all routes
        
        # Initialize analysis modules
        self.visual_analyzer = VisualAnalyzer(config_path)
        self.text_recognizer = TextRecognizer(config_path)
        self.object_detector = ObjectDetector(config_path)
        self.state_comparator = StateComparator(config_path)
        self.issue_classifier = IssueClassifier(config_path)
        
        # Analysis history
        self.analysis_history: List[Dict[str, Any]] = []
        self.max_history_size = 1000
        
        # Setup routes
        self._setup_routes()
        
        # Server configuration
        server_config = self.config.get("analysis_server", {})
        self.host = server_config.get("host", "localhost")
        self.port = server_config.get("port", 8081)
        self.max_file_size = server_config.get("max_file_size", 10485760)
        self.timeout = server_config.get("timeout", 2.0)
        
        logger.info(f"Analysis server initialized on {self.host}:{self.port}")
    
    def _load_config(self, config_path: str) -> Dict[str, Any]:
        """Load configuration from JSON file"""
        try:
            with open(config_path, 'r') as f:
                return json.load(f)
        except Exception as e:
            logger.error(f"Failed to load config: {e}")
            return {}
    
    def _setup_routes(self):
        """Setup Flask API routes"""
        
        @self.app.route('/analyze_screenshot', methods=['POST'])
        def analyze_screenshot():
            """Analyze a screenshot and return comprehensive results"""
            start_time = time.time()
            
            try:
                # Parse request
                request_data = request.get_json()
                if not request_data:
                    return jsonify({
                        "success": False,
                        "error": "No JSON data provided",
                        "timestamp": time.time(),
                        "processing_time": 0
                    }), 400
                
                # Extract screenshot data
                screenshot_data = request_data.get('screenshot_data')
                if not screenshot_data:
                    return jsonify({
                        "success": False,
                        "error": "No screenshot_data provided",
                        "timestamp": time.time(),
                        "processing_time": 0
                    }), 400
                
                metadata = request_data.get('metadata', {})
                analysis_options = request_data.get('analysis_options', {
                    "visual_analysis": True,
                    "text_recognition": True,
                    "object_detection": True,
                    "state_comparison": False,
                    "issue_classification": True
                })
                
                # Decode screenshot
                image_path = self._decode_screenshot(screenshot_data, metadata)
                if not image_path:
                    return jsonify({
                        "success": False,
                        "error": "Failed to decode screenshot data",
                        "timestamp": time.time(),
                        "processing_time": 0
                    }), 400
                
                # Perform analysis
                analysis_results = self._perform_analysis(image_path, analysis_options, metadata)
                
                # Clean up temporary file
                try:
                    Path(image_path).unlink()
                except:
                    pass
                
                processing_time = time.time() - start_time
                
                # Store in history
                self._store_analysis_history({
                    "analysis_id": analysis_results.get("analysis_id", f"analysis_{int(time.time())}"),
                    "timestamp": time.time(),
                    "processing_time": processing_time,
                    "metadata": metadata,
                    "results": analysis_results
                })
                
                return jsonify({
                    "success": True,
                    "analysis_id": analysis_results.get("analysis_id"),
                    "timestamp": time.time(),
                    "processing_time": processing_time,
                    "results": analysis_results
                })
                
            except Exception as e:
                logger.error(f"Screenshot analysis failed: {e}")
                return jsonify({
                    "success": False,
                    "error": str(e),
                    "timestamp": time.time(),
                    "processing_time": time.time() - start_time
                }), 500
        
        @self.app.route('/compare_states', methods=['POST'])
        def compare_states():
            """Compare two screenshots and identify differences"""
            start_time = time.time()
            
            try:
                request_data = request.get_json()
                if not request_data:
                    return jsonify({
                        "success": False,
                        "error": "No JSON data provided",
                        "timestamp": time.time(),
                        "processing_time": 0
                    }), 400
                
                # Extract screenshot data
                screenshot1_data = request_data.get('screenshot1_data')
                screenshot2_data = request_data.get('screenshot2_data')
                
                if not screenshot1_data or not screenshot2_data:
                    return jsonify({
                        "success": False,
                        "error": "Both screenshot1_data and screenshot2_data are required",
                        "timestamp": time.time(),
                        "processing_time": 0
                    }), 400
                
                metadata = request_data.get('metadata', {})
                
                # Decode screenshots
                screenshot1_path = self._decode_screenshot(screenshot1_data, metadata, suffix="_1")
                screenshot2_path = self._decode_screenshot(screenshot2_data, metadata, suffix="_2")
                
                if not screenshot1_path or not screenshot2_path:
                    return jsonify({
                        "success": False,
                        "error": "Failed to decode screenshot data",
                        "timestamp": time.time(),
                        "processing_time": 0
                    }), 400
                
                # Compare states
                comparison_results = self._compare_screenshots(
                    screenshot1_path, screenshot2_path, metadata
                )
                
                # Clean up temporary files
                for path in [screenshot1_path, screenshot2_path]:
                    try:
                        Path(path).unlink()
                    except:
                        pass
                
                processing_time = time.time() - start_time
                
                return jsonify({
                    "success": True,
                    "analysis_id": f"comparison_{int(time.time())}",
                    "timestamp": time.time(),
                    "processing_time": processing_time,
                    "results": comparison_results
                })
                
            except Exception as e:
                logger.error(f"State comparison failed: {e}")
                return jsonify({
                    "success": False,
                    "error": str(e),
                    "timestamp": time.time(),
                    "processing_time": time.time() - start_time
                }), 500
        
        @self.app.route('/analysis_history', methods=['GET'])
        def get_analysis_history():
            """Get analysis history with optional filtering"""
            try:
                # Get query parameters
                limit = request.args.get('limit', 100, type=int)
                offset = request.args.get('offset', 0, type=int)
                analysis_type = request.args.get('type', None)
                
                # Filter history
                filtered_history = self.analysis_history
                
                if analysis_type:
                    filtered_history = [
                        entry for entry in filtered_history 
                        if entry.get('results', {}).get('analysis_type') == analysis_type
                    ]
                
                # Apply pagination
                total_count = len(filtered_history)
                paginated_history = filtered_history[offset:offset + limit]
                
                return jsonify({
                    "success": True,
                    "total_count": total_count,
                    "limit": limit,
                    "offset": offset,
                    "history": paginated_history
                })
                
            except Exception as e:
                logger.error(f"Failed to retrieve analysis history: {e}")
                return jsonify({
                    "success": False,
                    "error": str(e)
                }), 500
        
        @self.app.route('/health', methods=['GET'])
        def health_check():
            """Health check endpoint"""
            return jsonify({
                "success": True,
                "status": "healthy",
                "timestamp": time.time(),
                "modules": {
                    "visual_analyzer": self.visual_analyzer is not None,
                    "text_recognizer": self.text_recognizer is not None,
                    "object_detector": self.object_detector is not None,
                    "state_comparator": self.state_comparator is not None,
                    "issue_classifier": self.issue_classifier is not None
                }
            })
        
        @self.app.route('/config', methods=['GET'])
        def get_config():
            """Get current configuration"""
            try:
                # Return non-sensitive config data
                config_summary = {
                    "server": self.config.get("analysis_server", {}),
                    "performance": self.config.get("performance", {}),
                    "analysis_options": {
                        "visual_analysis": True,
                        "text_recognition": True,
                        "object_detection": True,
                        "state_comparison": True,
                        "issue_classification": True
                    }
                }
                
                return jsonify({
                    "success": True,
                    "config": config_summary
                })
                
            except Exception as e:
                logger.error(f"Failed to retrieve config: {e}")
                return jsonify({
                    "success": False,
                    "error": str(e)
                }), 500
    
    def _decode_screenshot(self, screenshot_data: str, metadata: Dict[str, Any], suffix: str = "") -> Optional[str]:
        """Decode base64 screenshot data to temporary file"""
        try:
            # Remove data URL prefix if present
            if ',' in screenshot_data:
                screenshot_data = screenshot_data.split(',')[1]
            
            # Decode base64
            image_data = base64.b64decode(screenshot_data)
            
            # Create temporary file
            temp_dir = Path("temp_screenshots")
            temp_dir.mkdir(exist_ok=True)
            
            filename = f"screenshot_{int(time.time())}{suffix}.png"
            filepath = temp_dir / filename
            
            # Save image
            with open(filepath, 'wb') as f:
                f.write(image_data)
            
            # Verify image is valid
            img = cv2.imread(str(filepath))
            if img is None:
                filepath.unlink()
                return None
            
            return str(filepath)
            
        except Exception as e:
            logger.error(f"Failed to decode screenshot: {e}")
            return None
    
    def _perform_analysis(self, image_path: str, analysis_options: Dict[str, bool], 
                         metadata: Dict[str, Any]) -> Dict[str, Any]:
        """Perform comprehensive analysis based on options"""
        results = {
            "analysis_id": f"analysis_{int(time.time())}",
            "analysis_type": "comprehensive",
            "timestamp": time.time(),
            "modules": {}
        }
        
        try:
            # Visual analysis
            if analysis_options.get("visual_analysis", True):
                start_time = time.time()
                visual_results = self.visual_analyzer.analyze_screenshot(image_path)
                results["modules"]["visual_analysis"] = {
                    "enabled": True,
                    "processing_time": time.time() - start_time,
                    "results": visual_results
                }
            
            # Text recognition
            if analysis_options.get("text_recognition", True):
                start_time = time.time()
                text_results = self.text_recognizer.recognize_text(image_path)
                results["modules"]["text_recognition"] = {
                    "enabled": True,
                    "processing_time": time.time() - start_time,
                    "results": {
                        "text_elements_count": len(text_results.text_elements),
                        "average_confidence": text_results.average_confidence,
                        "processing_time": text_results.processing_time,
                        "detected_languages": text_results.detected_languages
                    }
                }
            
            # Object detection
            if analysis_options.get("object_detection", True):
                start_time = time.time()
                object_results = self.object_detector.detect_objects(image_path)
                results["modules"]["object_detection"] = {
                    "enabled": True,
                    "processing_time": time.time() - start_time,
                    "results": {
                        "objects_detected": len(object_results.detected_objects),
                        "model_confidence": object_results.model_confidence,
                        "tracking_info": len(object_results.tracking_info)
                    }
                }
            
            # State comparison (requires expected state)
            if analysis_options.get("state_comparison", False):
                expected_state_data = metadata.get('expected_state')
                if expected_state_data:
                    start_time = time.time()
                    expected_state = self._create_expected_state(expected_state_data)
                    state_results = self.state_comparator.compare_states(image_path, expected_state)
                    results["modules"]["state_comparison"] = {
                        "enabled": True,
                        "processing_time": time.time() - start_time,
                        "results": {
                            "is_valid": state_results.is_valid,
                            "discrepancies_count": len(state_results.discrepancies),
                            "critical_issues": state_results.critical_issues,
                            "warnings": state_results.warnings,
                            "overall_confidence": state_results.overall_confidence
                        }
                    }
            
            # Issue classification
            if analysis_options.get("issue_classification", True):
                start_time = time.time()
                
                # Prepare analysis results for classification
                classification_input = {}
                
                if "visual_analysis" in results["modules"]:
                    classification_input["visual_artifacts"] = results["modules"]["visual_analysis"]["results"].get("visual_artifacts", [])
                
                if "text_recognition" in results["modules"]:
                    classification_input["text_elements"] = text_results.text_elements
                
                if "state_comparison" in results["modules"]:
                    classification_input["state_discrepancies"] = state_results.discrepancies
                
                issue_results = self.issue_classifier.classify_issues(classification_input)
                results["modules"]["issue_classification"] = {
                    "enabled": True,
                    "processing_time": time.time() - start_time,
                    "results": {
                        "total_issues": len(issue_results.classified_issues),
                        "critical_count": issue_results.critical_count,
                        "high_count": issue_results.high_count,
                        "medium_count": issue_results.medium_count,
                        "low_count": issue_results.low_count,
                        "issue_summary": issue_results.issue_summary
                    }
                }
            
            # Calculate overall metrics
            total_time = sum(module["processing_time"] for module in results["modules"].values() if module["enabled"])
            results["overall_processing_time"] = total_time
            
            # Add metadata
            results["metadata"] = metadata
            
            return results
            
        except Exception as e:
            logger.error(f"Analysis failed: {e}")
            results["error"] = str(e)
            return results
    
    def _compare_screenshots(self, screenshot1_path: str, screenshot2_path: str, 
                           metadata: Dict[str, Any]) -> Dict[str, Any]:
        """Compare two screenshots and identify differences"""
        results = {
            "comparison_id": f"comparison_{int(time.time())}",
            "timestamp": time.time(),
            "differences": []
        }
        
        try:
            # Load both images
            image1 = cv2.imread(screenshot1_path)
            image2 = cv2.imread(screenshot2_path)
            
            if image1 is None or image2 is None:
                raise ValueError("Failed to load one or both screenshots")
            
            # Basic image comparison
            if image1.shape != image2.shape:
                results["differences"].append({
                    "type": "resolution_mismatch",
                    "description": f"Image resolutions differ: {image1.shape} vs {image2.shape}",
                    "severity": "medium"
                })
            
            # Calculate structural similarity
            gray1 = cv2.cvtColor(image1, cv2.COLOR_BGR2GRAY)
            gray2 = cv2.cvtColor(image2, cv2.COLOR_BGR2GRAY)
            
            # Resize to same dimensions if needed
            if gray1.shape != gray2.shape:
                gray2 = cv2.resize(gray2, (gray1.shape[1], gray1.shape[0]))
            
            # Calculate difference
            diff = cv2.absdiff(gray1, gray2)
            diff_score = np.mean(diff)
            
            results["difference_score"] = float(diff_score)
            
            if diff_score > 50:  # Significant difference threshold
                results["differences"].append({
                    "type": "visual_difference",
                    "description": f"Significant visual differences detected (score: {diff_score:.2f})",
                    "severity": "high" if diff_score > 100 else "medium"
                })
            
            # Analyze each image
            analysis1 = self.visual_analyzer.analyze_screenshot(screenshot1_path)
            analysis2 = self.visual_analyzer.analyze_screenshot(screenshot2_path)
            
            # Compare UI elements
            ui_diff = self._compare_ui_elements(
                analysis1.get("ui_elements", []),
                analysis2.get("ui_elements", [])
            )
            results["differences"].extend(ui_diff)
            
            # Compare text elements
            text1 = self.text_recognizer.recognize_text(screenshot1_path)
            text2 = self.text_recognizer.recognize_text(screenshot2_path)
            
            text_diff = self._compare_text_elements(text1.text_elements, text2.text_elements)
            results["differences"].extend(text_diff)
            
            # Compare objects
            objects1 = self.object_detector.detect_objects(screenshot1_path)
            objects2 = self.object_detector.detect_objects(screenshot2_path)
            
            object_diff = self._compare_objects(objects1.detected_objects, objects2.detected_objects)
            results["differences"].extend(object_diff)
            
            # Classify differences as issues
            if results["differences"]:
                classification_input = {"state_discrepancies": []}
                for diff in results["differences"]:
                    classification_input["state_discrepancies"].append(StateDiscrepancy(
                        discrepancy_type=diff["type"],
                        severity=diff["severity"],
                        expected_value=None,
                        actual_value=None,
                        location=(0, 0),
                        description=diff["description"],
                        confidence=0.8,
                        element_type="comparison"
                    ))
                
                issue_results = self.issue_classifier.classify_issues(classification_input)
                results["classified_issues"] = {
                    "total_issues": len(issue_results.classified_issues),
                    "by_severity": {
                        "critical": issue_results.critical_count,
                        "high": issue_results.high_count,
                        "medium": issue_results.medium_count,
                        "low": issue_results.low_count
                    }
                }
            
            return results
            
        except Exception as e:
            logger.error(f"Screenshot comparison failed: {e}")
            results["error"] = str(e)
            return results
    
    def _compare_ui_elements(self, ui1: List, ui2: List) -> List[Dict[str, Any]]:
        """Compare UI elements between two screenshots"""
        differences = []
        
        try:
            # Count elements by type
            types1 = [elem.element_type for elem in ui1]
            types2 = [elem.element_type for elem in ui2]
            
            # Find differences
            set1 = set(types1)
            set2 = set(types2)
            
            missing_in_2 = set1 - set2
            missing_in_1 = set2 - set1
            
            for elem_type in missing_in_2:
                count_diff = types1.count(elem_type) - types2.count(elem_type)
                if count_diff > 0:
                    differences.append({
                        "type": "ui_element_missing",
                        "description": f"{count_diff} {elem_type} element(s) missing in second screenshot",
                        "severity": "high"
                    })
            
            for elem_type in missing_in_1:
                count_diff = types2.count(elem_type) - types1.count(elem_type)
                if count_diff > 0:
                    differences.append({
                        "type": "ui_element_added",
                        "description": f"{count_diff} {elem_type} element(s) added in second screenshot",
                        "severity": "medium"
                    })
        
        except Exception as e:
            logger.error(f"UI element comparison failed: {e}")
        
        return differences
    
    def _compare_text_elements(self, text1: List, text2: List) -> List[Dict[str, Any]]:
        """Compare text elements between two screenshots"""
        differences = []
        
        try:
            # Extract text content
            texts1 = [elem.text for elem in text1]
            texts2 = [elem.text for elem in text2]
            
            # Find text differences
            set1 = set(texts1)
            set2 = set(texts2)
            
            missing_in_2 = set1 - set2
            missing_in_1 = set2 - set1
            
            if missing_in_2:
                differences.append({
                    "type": "text_missing",
                    "description": f"Text missing in second screenshot: {list(missing_in_2)[:3]}",
                    "severity": "high"
                })
            
            if missing_in_1:
                differences.append({
                    "type": "text_added",
                    "description": f"New text in second screenshot: {list(missing_in_1)[:3]}",
                    "severity": "medium"
                })
        
        except Exception as e:
            logger.error(f"Text element comparison failed: {e}")
        
        return differences
    
    def _compare_objects(self, objects1: List, objects2: List) -> List[Dict[str, Any]]:
        """Compare objects between two screenshots"""
        differences = []
        
        try:
            # Count objects by class
            classes1 = [obj.class_name for obj in objects1]
            classes2 = [obj.class_name for obj in objects2]
            
            # Find differences
            set1 = set(classes1)
            set2 = set(classes2)
            
            missing_in_2 = set1 - set2
            missing_in_1 = set2 - set1
            
            for obj_class in missing_in_2:
                count_diff = classes1.count(obj_class) - classes2.count(obj_class)
                if count_diff > 0:
                    differences.append({
                        "type": "object_missing",
                        "description": f"{count_diff} {obj_class} object(s) missing in second screenshot",
                        "severity": "high"
                    })
            
            for obj_class in missing_in_1:
                count_diff = classes2.count(obj_class) - classes1.count(obj_class)
                if count_diff > 0:
                    differences.append({
                        "type": "object_added",
                        "description": f"{count_diff} {obj_class} object(s) added in second screenshot",
                        "severity": "medium"
                    })
        
        except Exception as e:
            logger.error(f"Object comparison failed: {e}")
        
        return differences
    
    def _create_expected_state(self, state_data: Dict[str, Any]) -> ExpectedGameState:
        """Create ExpectedGameState from dictionary data"""
        return ExpectedGameState(
            ui_elements=state_data.get('ui_elements', []),
            text_elements=state_data.get('text_elements', []),
            object_counts=state_data.get('object_counts', {}),
            player_state=state_data.get('player_state', {}),
            mission_state=state_data.get('mission_state', {}),
            inventory_state=state_data.get('inventory_state', {})
        )
    
    def _store_analysis_history(self, analysis_entry: Dict[str, Any]):
        """Store analysis in history with size limit"""
        try:
            self.analysis_history.append(analysis_entry)
            
            # Maintain size limit
            if len(self.analysis_history) > self.max_history_size:
                self.analysis_history = self.analysis_history[-self.max_history_size:]
        
        except Exception as e:
            logger.error(f"Failed to store analysis history: {e}")
    
    def run(self):
        """Run the Flask server"""
        try:
            logger.info(f"Starting analysis server on {self.host}:{self.port}")
            self.app.run(
                host=self.host, 
                port=self.port, 
                debug=False, 
                threaded=True
            )
        except Exception as e:
            logger.error(f"Failed to start server: {e}")
            raise

# Example usage
if __name__ == "__main__":
    # Create and run server
    server = AnalysisServer()
    
    # For testing, you can also run a simple test
    print("Analysis Server starting...")
    print("Available endpoints:")
    print("  POST /analyze_screenshot - Analyze a screenshot")
    print("  POST /compare_states - Compare two screenshots")
    print("  GET  /analysis_history - Get analysis history")
    print("  GET  /health - Health check")
    print("  GET  /config - Get configuration")
    print(f"\nServer will run on http://{server.host}:{server.port}")
    
    server.run()