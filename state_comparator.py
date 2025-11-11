"""
State Comparison Engine for Gameplay Screenshot Analysis
Compares current screenshot with expected game state and validates UI elements.
"""

import cv2
import numpy as np
import json
import time
from typing import Dict, List, Tuple, Optional, Any
from dataclasses import dataclass
from pathlib import Path
import logging

# Import other modules
from visual_analyzer import VisualAnalyzer, UIElement
from text_recognizer import TextRecognizer, TextElement
from object_detector import ObjectDetector, DetectedObject

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

@dataclass
class StateDiscrepancy:
    """Represents a discrepancy between visual and expected state"""
    discrepancy_type: str
    severity: str
    expected_value: Any
    actual_value: Any
    location: Tuple[int, int]
    description: str
    confidence: float
    element_type: str

@dataclass
class StateValidationResult:
    """Complete state validation results"""
    is_valid: bool
    discrepancies: List[StateDiscrepancy]
    validation_time: float
    overall_confidence: float
    critical_issues: int
    warnings: int

@dataclass
class ExpectedGameState:
    """Expected game state for validation"""
    ui_elements: List[Dict[str, Any]]
    text_elements: List[Dict[str, Any]]
    object_counts: Dict[str, int]
    player_state: Dict[str, Any]
    mission_state: Dict[str, Any]
    inventory_state: Dict[str, Any]

class StateComparator:
    """State comparison engine for validating game screenshots against expected state"""
    
    def __init__(self, config_path: str = "visual_analysis_config.json"):
        """Initialize the state comparator with configuration"""
        self.config = self._load_config(config_path)
        self.visual_analyzer = VisualAnalyzer(config_path)
        self.text_recognizer = TextRecognizer(config_path)
        self.object_detector = ObjectDetector(config_path)
        
        self.validation_rules = self.config.get("state_comparator", {}).get("validation_rules", {})
        self.critical_elements = self.config.get("state_comparator", {}).get("critical_elements", [])
    
    def _load_config(self, config_path: str) -> Dict[str, Any]:
        """Load configuration from JSON file"""
        try:
            with open(config_path, 'r') as f:
                return json.load(f)
        except Exception as e:
            logger.error(f"Failed to load config: {e}")
            return {}
    
    def compare_states(self, screenshot_path: str, expected_state: ExpectedGameState) -> StateValidationResult:
        """
        Compare screenshot with expected game state
        
        Args:
            screenshot_path: Path to the screenshot image
            expected_state: ExpectedGameState object with validation criteria
            
        Returns:
            StateValidationResult containing comparison results
        """
        start_time = time.time()
        
        try:
            # Analyze screenshot
            visual_results = self.visual_analyzer.analyze_screenshot(screenshot_path)
            text_results = self.text_recognizer.recognize_text(screenshot_path)
            object_results = self.object_detector.detect_objects(screenshot_path)
            
            discrepancies = []
            
            # Validate UI elements
            ui_discrepancies = self._validate_ui_elements(
                visual_results.get("ui_elements", []), 
                expected_state.ui_elements
            )
            discrepancies.extend(ui_discrepancies)
            
            # Validate text elements
            text_discrepancies = self._validate_text_elements(
                text_results.text_elements, 
                expected_state.text_elements
            )
            discrepancies.extend(text_discrepancies)
            
            # Validate object counts
            object_discrepancies = self._validate_object_counts(
                object_results.detected_objects, 
                expected_state.object_counts
            )
            discrepancies.extend(object_discrepancies)
            
            # Validate critical elements
            critical_discrepancies = self._validate_critical_elements(
                visual_results, text_results, object_results
            )
            discrepancies.extend(critical_discrepancies)
            
            # Calculate overall results
            is_valid = len([d for d in discrepancies if d.severity in ["critical", "high"]]) == 0
            critical_issues = len([d for d in discrepancies if d.severity == "critical"])
            warnings = len([d for d in discrepancies if d.severity == "medium"])
            
            # Calculate overall confidence
            overall_confidence = self._calculate_overall_confidence(discrepancies)
            
            validation_time = time.time() - start_time
            
            result = StateValidationResult(
                is_valid=is_valid,
                discrepancies=discrepancies,
                validation_time=validation_time,
                overall_confidence=overall_confidence,
                critical_issues=critical_issues,
                warnings=warnings
            )
            
            logger.info(f"State validation completed in {validation_time:.3f}s, found {len(discrepancies)} discrepancies")
            
            return result
            
        except Exception as e:
            logger.error(f"State comparison failed: {e}")
            return StateValidationResult(
                is_valid=False,
                discrepancies=[StateDiscrepancy(
                    discrepancy_type="validation_error",
                    severity="critical",
                    expected_value=None,
                    actual_value=str(e),
                    location=(0, 0),
                    description=f"Validation failed: {str(e)}",
                    confidence=1.0,
                    element_type="system"
                )],
                validation_time=time.time() - start_time,
                overall_confidence=0.0,
                critical_issues=1,
                warnings=0
            )
    
    def _validate_ui_elements(self, detected_ui: List[UIElement], 
                            expected_ui: List[Dict[str, Any]]) -> List[StateDiscrepancy]:
        """Validate UI elements against expected state"""
        discrepancies = []
        
        try:
            position_tolerance = self.validation_rules.get("ui_position_tolerance", 10)
            
            for expected_element in expected_ui:
                element_type = expected_element.get("type")
                expected_position = expected_element.get("position", {})
                expected_x = expected_position.get("x", 0)
                expected_y = expected_position.get("y", 0)
                
                # Find matching detected element
                matching_elements = [
                    elem for elem in detected_ui 
                    if elem.element_type == element_type
                ]
                
                if not matching_elements:
                    # Missing UI element
                    discrepancies.append(StateDiscrepancy(
                        discrepancy_type="missing_ui_element",
                        severity="high",
                        expected_value=expected_element,
                        actual_value=None,
                        location=(expected_x, expected_y),
                        description=f"Missing UI element: {element_type}",
                        confidence=0.9,
                        element_type="ui"
                    ))
                    continue
                
                # Check position of closest matching element
                closest_element = min(
                    matching_elements, 
                    key=lambda elem: np.sqrt((elem.x - expected_x)**2 + (elem.y - expected_y)**2)
                )
                
                # Calculate position difference
                x_diff = abs(closest_element.x - expected_x)
                y_diff = abs(closest_element.y - expected_y)
                
                if x_diff > position_tolerance or y_diff > position_tolerance:
                    discrepancies.append(StateDiscrepancy(
                        discrepancy_type="ui_position_mismatch",
                        severity="medium",
                        expected_value={"x": expected_x, "y": expected_y},
                        actual_value={"x": closest_element.x, "y": closest_element.y},
                        location=(closest_element.x, closest_element.y),
                        description=f"UI element '{element_type}' position mismatch: expected ({expected_x}, {expected_y}), got ({closest_element.x}, {closest_element.y})",
                        confidence=0.8,
                        element_type="ui"
                    ))
        
        except Exception as e:
            logger.error(f"UI validation failed: {e}")
        
        return discrepancies
    
    def _validate_text_elements(self, detected_text: List[TextElement], 
                              expected_text: List[Dict[str, Any]]) -> List[StateDiscrepancy]:
        """Validate text elements against expected state"""
        discrepancies = []
        
        try:
            text_match_threshold = self.validation_rules.get("text_match_threshold", 0.9)
            
            for expected_element in expected_text:
                expected_content = expected_element.get("text", "")
                expected_location = expected_element.get("location", {})
                expected_x = expected_location.get("x", 0)
                expected_y = expected_location.get("y", 0)
                
                # Find matching detected text
                matching_texts = [
                    elem for elem in detected_text 
                    if self._text_similarity(elem.text, expected_content) > text_match_threshold
                ]
                
                if not matching_texts:
                    # Missing text
                    discrepancies.append(StateDiscrepancy(
                        discrepancy_type="missing_text",
                        severity="high",
                        expected_value=expected_content,
                        actual_value=None,
                        location=(expected_x, expected_y),
                        description=f"Missing expected text: '{expected_content}'",
                        confidence=0.9,
                        element_type="text"
                    ))
                    continue
                
                # Check if any matching text is at expected location
                location_match = False
                position_tolerance = self.validation_rules.get("ui_position_tolerance", 10)
                
                for detected in matching_texts:
                    x_diff = abs(detected.x - expected_x)
                    y_diff = abs(detected.y - expected_y)
                    
                    if x_diff <= position_tolerance and y_diff <= position_tolerance:
                        location_match = True
                        break
                
                if not location_match:
                    # Text found but in wrong location
                    discrepancies.append(StateDiscrepancy(
                        discrepancy_type="text_position_mismatch",
                        severity="medium",
                        expected_value={"text": expected_content, "location": {"x": expected_x, "y": expected_y}},
                        actual_value={"text": matching_texts[0].text, "location": {"x": matching_texts[0].x, "y": matching_texts[0].y}},
                        location=(matching_texts[0].x, matching_texts[0].y),
                        description=f"Text '{expected_content}' found at wrong location",
                        confidence=0.7,
                        element_type="text"
                    ))
        
        except Exception as e:
            logger.error(f"Text validation failed: {e}")
        
        return discrepancies
    
    def _validate_object_counts(self, detected_objects: List[DetectedObject], 
                              expected_counts: Dict[str, int]) -> List[StateDiscrepancy]:
        """Validate object counts against expected state"""
        discrepancies = []
        
        try:
            count_tolerance = self.validation_rules.get("object_count_tolerance", 0.1)
            
            # Count detected objects by class
            detected_counts = {}
            for obj in detected_objects:
                if obj.class_name not in detected_counts:
                    detected_counts[obj.class_name] = 0
                detected_counts[obj.class_name] += 1
            
            # Validate each expected object type
            for object_type, expected_count in expected_counts.items():
                detected_count = detected_counts.get(object_type, 0)
                
                # Calculate difference
                count_diff = abs(detected_count - expected_count)
                tolerance = max(1, int(expected_count * count_tolerance))
                
                if count_diff > tolerance:
                    severity = "high" if expected_count > 0 and detected_count == 0 else "medium"
                    
                    discrepancies.append(StateDiscrepancy(
                        discrepancy_type="object_count_mismatch",
                        severity=severity,
                        expected_value=expected_count,
                        actual_value=detected_count,
                        location=(0, 0),
                        description=f"Object count mismatch for '{object_type}': expected {expected_count}, found {detected_count}",
                        confidence=0.85,
                        element_type="object"
                    ))
        
        except Exception as e:
            logger.error(f"Object count validation failed: {e}")
        
        return discrepancies
    
    def _validate_critical_elements(self, visual_results: Dict[str, Any], 
                                  text_results: Any, 
                                  object_results: Any) -> List[StateDiscrepancy]:
        """Validate critical UI elements that must always be present"""
        discrepancies = []
        
        try:
            detected_ui_types = [elem.element_type for elem in visual_results.get("ui_elements", [])]
            detected_texts = [elem.text.lower() for elem in text_results.text_elements]
            
            for critical_element in self.critical_elements:
                found = False
                
                # Check UI elements
                if critical_element in detected_ui_types:
                    found = True
                
                # Check text elements
                if any(critical_element in text for text in detected_texts):
                    found = True
                
                # Check for health/shield bars in visual analysis
                if critical_element in ["health_bar", "shield_bar"]:
                    # Look for rectangular UI elements in typical positions
                    for ui_elem in visual_results.get("ui_elements", []):
                        if (ui_elem.element_type in ["rectangle_element", "ui_element"] and 
                            ui_elem.x < 200 and ui_elem.y < 100):  # Top-left area
                            found = True
                            break
                
                # Check for minimap
                if critical_element == "minimap":
                    # Look for circular/square elements in corner
                    for ui_elem in visual_results.get("ui_elements", []):
                        if (ui_elem.element_type in ["circular_element", "square_element"] and 
                            ui_elem.x > image.shape[1] - 200 and ui_elem.y > image.shape[0] - 200):
                            found = True
                            break
                
                if not found:
                    discrepancies.append(StateDiscrepancy(
                        discrepancy_type="missing_critical_element",
                        severity="critical",
                        expected_value=critical_element,
                        actual_value=None,
                        location=(0, 0),
                        description=f"Critical UI element missing: {critical_element}",
                        confidence=0.95,
                        element_type="critical_ui"
                    ))
        
        except Exception as e:
            logger.error(f"Critical element validation failed: {e}")
        
        return discrepancies
    
    def _text_similarity(self, text1: str, text2: str) -> float:
        """Calculate similarity between two text strings"""
        try:
            # Simple similarity based on character overlap
            text1, text2 = text1.lower().strip(), text2.lower().strip()
            
            if not text1 or not text2:
                return 0.0
            
            # Exact match
            if text1 == text2:
                return 1.0
            
            # Check if one is substring of another
            if text1 in text2 or text2 in text1:
                return 0.9
            
            # Calculate character overlap
            set1, set2 = set(text1), set(text2)
            intersection = len(set1.intersection(set2))
            union = len(set1.union(set2))
            
            jaccard_similarity = intersection / union if union > 0 else 0.0
            
            return jaccard_similarity
        
        except Exception as e:
            logger.warning(f"Text similarity calculation failed: {e}")
            return 0.0
    
    def _calculate_overall_confidence(self, discrepancies: List[StateDiscrepancy]) -> float:
        """Calculate overall confidence based on discrepancies"""
        try:
            if not discrepancies:
                return 1.0
            
            # Weight by severity
            severity_weights = {
                "critical": 0.0,
                "high": 0.3,
                "medium": 0.7,
                "low": 0.9
            }
            
            total_weight = 0.0
            count = 0
            
            for discrepancy in discrepancies:
                severity = discrepancy.severity
                weight = severity_weights.get(severity, 0.5)
                total_weight += weight
                count += 1
            
            overall_confidence = 1.0 - (total_weight / count) if count > 0 else 1.0
            
            return max(0.0, min(1.0, overall_confidence))
        
        except Exception as e:
            logger.error(f"Overall confidence calculation failed: {e}")
            return 0.0
    
    def generate_validation_report(self, result: StateValidationResult) -> str:
        """Generate a human-readable validation report"""
        try:
            report_lines = [
                "=== Game State Validation Report ===",
                f"Valid: {'YES' if result.is_valid else 'NO'}",
                f"Overall Confidence: {result.overall_confidence:.2%}",
                f"Critical Issues: {result.critical_issues}",
                f"Warnings: {result.warnings}",
                f"Validation Time: {result.validation_time:.3f}s",
                "",
                "Discrepancies:"
            ]
            
            if not result.discrepancies:
                report_lines.append("  None - All validations passed!")
            else:
                for i, discrepancy in enumerate(result.discrepancies, 1):
                    report_lines.append(
                        f"  {i}. [{discrepancy.severity.upper()}] {discrepancy.description} "
                        f"(conf: {discrepancy.confidence:.2f})"
                    )
            
            return "\n".join(report_lines)
        
        except Exception as e:
            logger.error(f"Report generation failed: {e}")
            return f"Failed to generate report: {e}"
    
    def create_expected_state_template(self, screenshot_path: str) -> ExpectedGameState:
        """
        Create an expected state template from a reference screenshot
        
        Args:
            screenshot_path: Path to reference screenshot
            
        Returns:
            ExpectedGameState template
        """
        try:
            # Analyze reference screenshot
            visual_results = self.visual_analyzer.analyze_screenshot(screenshot_path)
            text_results = self.text_recognizer.recognize_text(screenshot_path)
            object_results = self.object_detector.detect_objects(screenshot_path)
            
            # Create expected state
            expected_ui = []
            for ui_elem in visual_results.get("ui_elements", []):
                expected_ui.append({
                    "type": ui_elem.element_type,
                    "position": {"x": ui_elem.x, "y": ui_elem.y},
                    "size": {"width": ui_elem.width, "height": ui_elem.height}
                })
            
            expected_text = []
            for text_elem in text_results.text_elements:
                expected_text.append({
                    "text": text_elem.text,
                    "location": {"x": text_elem.x, "y": text_elem.y},
                    "type": text_elem.element_type
                })
            
            expected_objects = {}
            for obj in object_results.detected_objects:
                if obj.class_name not in expected_objects:
                    expected_objects[obj.class_name] = 0
                expected_objects[obj.class_name] += 1
            
            return ExpectedGameState(
                ui_elements=expected_ui,
                text_elements=expected_text,
                object_counts=expected_objects,
                player_state={},
                mission_state={},
                inventory_state={}
            )
        
        except Exception as e:
            logger.error(f"Template creation failed: {e}")
            return ExpectedGameState(
                ui_elements=[],
                text_elements=[],
                object_counts={},
                player_state={},
                mission_state={},
                inventory_state={}
            )

# Example usage and testing
if __name__ == "__main__":
    import sys
    
    if len(sys.argv) > 1:
        image_path = sys.argv[1]
    else:
        # Use a test image if available
        test_images = list(Path(".").glob("*.png")) + list(Path(".").glob("*.jpg"))
        if test_images:
            image_path = str(test_images[0])
        else:
            logger.error("No image file provided and no test images found")
            sys.exit(1)
    
    comparator = StateComparator()
    
    # Create expected state template from the image itself (for testing)
    expected_state = comparator.create_expected_state_template(image_path)
    
    # Compare states
    result = comparator.compare_states(image_path, expected_state)
    
    # Generate and print report
    report = comparator.generate_validation_report(result)
    print(report)