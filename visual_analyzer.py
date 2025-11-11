"""
Visual Analysis Engine for Gameplay Screenshot Analysis
Performs comprehensive image analysis using OpenCV to detect UI elements,
visual artifacts, screen transitions, and color distributions.
"""

import cv2
import numpy as np
import json
import time
from typing import Dict, List, Tuple, Optional, Any
from dataclasses import dataclass
from pathlib import Path
import logging

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

@dataclass
class UIElement:
    """Represents a detected UI element"""
    x: int
    y: int
    width: int
    height: int
    element_type: str
    confidence: float
    aspect_ratio: float
    area: int

@dataclass
class VisualArtifact:
    """Represents a detected visual artifact"""
    artifact_type: str
    severity: float
    location: Tuple[int, int]
    description: str
    confidence: float

@dataclass
class ColorAnalysis:
    """Color distribution analysis results"""
    dominant_colors: List[Tuple[int, int, int]]
    brightness_mean: float
    brightness_std: float
    saturation_mean: float
    color_variance: float
    histogram: np.ndarray

@dataclass
class TransitionDetection:
    """Screen transition detection results"""
    is_transition: bool
    transition_type: str
    confidence: float
    frame_diff_score: float

class VisualAnalyzer:
    """Main visual analysis engine for gameplay screenshots"""
    
    def __init__(self, config_path: str = "visual_analysis_config.json"):
        """Initialize the visual analyzer with configuration"""
        self.config = self._load_config(config_path)
        self.previous_frame: Optional[np.ndarray] = None
        self.ui_templates: Dict[str, np.ndarray] = {}
        self._load_ui_templates()
        
    def _load_config(self, config_path: str) -> Dict[str, Any]:
        """Load configuration from JSON file"""
        try:
            with open(config_path, 'r') as f:
                return json.load(f)
        except Exception as e:
            logger.error(f"Failed to load config: {e}")
            return {}
    
    def _load_ui_templates(self):
        """Load UI template images for pattern matching"""
        template_dir = Path("ui_templates")
        if template_dir.exists():
            for template_file in template_dir.glob("*.png"):
                try:
                    template = cv2.imread(str(template_file), cv2.IMREAD_GRAYSCALE)
                    if template is not None:
                        self.ui_templates[template_file.stem] = template
                        logger.info(f"Loaded UI template: {template_file.stem}")
                except Exception as e:
                    logger.warning(f"Failed to load template {template_file}: {e}")
    
    def analyze_screenshot(self, image_path: str) -> Dict[str, Any]:
        """
        Perform comprehensive analysis of a screenshot
        
        Args:
            image_path: Path to the screenshot image
            
        Returns:
            Dictionary containing all analysis results
        """
        start_time = time.time()
        
        try:
            # Load image
            image = cv2.imread(image_path)
            if image is None:
                raise ValueError(f"Failed to load image: {image_path}")
            
            results = {
                "timestamp": time.time(),
                "image_path": image_path,
                "image_shape": image.shape,
                "analysis_duration": 0.0,
                "ui_elements": [],
                "visual_artifacts": [],
                "color_analysis": None,
                "transition_analysis": None,
                "confidence_scores": {}
            }
            
            # Perform individual analyses
            results["ui_elements"] = self._detect_ui_elements(image)
            results["visual_artifacts"] = self._detect_visual_artifacts(image)
            results["color_analysis"] = self._analyze_color_distribution(image)
            results["transition_analysis"] = self._detect_screen_transitions(image)
            
            # Calculate overall confidence
            results["confidence_scores"] = self._calculate_confidence_scores(results)
            
            # Update analysis duration
            results["analysis_duration"] = time.time() - start_time
            
            logger.info(f"Analysis completed in {results['analysis_duration']:.3f}s")
            return results
            
        except Exception as e:
            logger.error(f"Analysis failed: {e}")
            return {"error": str(e), "timestamp": time.time()}
    
    def _detect_ui_elements(self, image: np.ndarray) -> List[UIElement]:
        """Detect UI elements in the screenshot"""
        ui_elements = []
        
        try:
            # Convert to grayscale for edge detection
            gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
            
            # Apply Gaussian blur to reduce noise
            blurred = cv2.GaussianBlur(gray, (5, 5), 0)
            
            # Detect edges using Canny
            edges = cv2.Canny(blurred, 
                             self.config.get("ui_detection", {}).get("edge_threshold", 50),
                             self.config.get("ui_detection", {}).get("edge_threshold", 50) * 2)
            
            # Find contours
            contours, _ = cv2.findContours(edges, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
            
            min_area = self.config.get("ui_detection", {}).get("min_contour_area", 100)
            max_area = self.config.get("ui_detection", {}).get("max_contour_area", 100000)
            aspect_tolerance = self.config.get("ui_detection", {}).get("aspect_ratio_tolerance", 0.2)
            
            for contour in contours:
                area = cv2.contourArea(contour)
                
                if min_area < area < max_area:
                    # Get bounding rectangle
                    x, y, w, h = cv2.boundingRect(contour)
                    
                    # Calculate aspect ratio
                    aspect_ratio = w / h if h > 0 else 0
                    
                    # Approximate polygon
                    epsilon = 0.02 * cv2.arcLength(contour, True)
                    approx = cv2.approxPolyDP(contour, epsilon, True)
                    
                    # Determine element type based on shape and characteristics
                    element_type = self._classify_ui_element(contour, approx, aspect_ratio)
                    
                    # Calculate confidence based on shape regularity
                    confidence = self._calculate_shape_confidence(contour, approx)
                    
                    if confidence > self.config.get("ui_detection", {}).get("confidence_threshold", 0.7):
                        ui_elements.append(UIElement(
                            x=x, y=y, width=w, height=h,
                            element_type=element_type,
                            confidence=confidence,
                            aspect_ratio=aspect_ratio,
                            area=area
                        ))
            
            # Detect UI elements using template matching
            template_matches = self._match_ui_templates(gray)
            ui_elements.extend(template_matches)
            
            logger.info(f"Detected {len(ui_elements)} UI elements")
            
        except Exception as e:
            logger.error(f"UI detection failed: {e}")
        
        return ui_elements
    
    def _classify_ui_element(self, contour: np.ndarray, approx: np.ndarray, 
                           aspect_ratio: float) -> str:
        """Classify UI element type based on shape characteristics"""
        num_vertices = len(approx)
        
        if num_vertices == 3:
            return "triangle_element"
        elif num_vertices == 4:
            if 0.8 <= aspect_ratio <= 1.2:
                return "square_element"
            else:
                return "rectangle_element"
        elif num_vertices > 4:
            return "circular_element"
        else:
            return "irregular_element"
    
    def _calculate_shape_confidence(self, contour: np.ndarray, 
                                  approx: np.ndarray) -> float:
        """Calculate confidence score for shape detection"""
        try:
            area = cv2.contourArea(contour)
            hull_area = cv2.contourArea(cv2.convexHull(contour))
            
            if hull_area > 0:
                solidity = area / hull_area
                return float(solidity)
            return 0.0
        except:
            return 0.0
    
    def _match_ui_templates(self, gray_image: np.ndarray) -> List[UIElement]:
        """Match UI templates in the image"""
        matches = []
        
        for template_name, template in self.ui_templates.items():
            try:
                # Check if template is smaller than image
                if template.shape[0] > gray_image.shape[0] or \
                   template.shape[1] > gray_image.shape[1]:
                    continue
                
                # Perform template matching
                result = cv2.matchTemplate(gray_image, template, cv2.TM_CCOEFF_NORMED)
                threshold = 0.8
                
                locations = np.where(result >= threshold)
                
                for pt in zip(*locations[::-1]):
                    confidence = float(result[pt[1], pt[0]])
                    matches.append(UIElement(
                        x=pt[0], y=pt[1],
                        width=template.shape[1], height=template.shape[0],
                        element_type=f"template_{template_name}",
                        confidence=confidence,
                        aspect_ratio=template.shape[1] / template.shape[0],
                        area=template.shape[0] * template.shape[1]
                    ))
            
            except Exception as e:
                logger.warning(f"Template matching failed for {template_name}: {e}")
        
        return matches
    
    def _detect_visual_artifacts(self, image: np.ndarray) -> List[VisualArtifact]:
        """Detect visual artifacts like tearing, flickering, rendering issues"""
        artifacts = []
        
        try:
            # Check for screen tearing (horizontal lines with sharp differences)
            tearing_artifacts = self._detect_screen_tearing(image)
            artifacts.extend(tearing_artifacts)
            
            # Check for unusual color patterns (rendering issues)
            color_artifacts = self._detect_color_anomalies(image)
            artifacts.extend(color_artifacts)
            
            # Check for sharpness/blurriness issues
            sharpness_artifacts = self._detect_sharpness_issues(image)
            artifacts.extend(sharpness_artifacts)
            
            logger.info(f"Detected {len(artifacts)} visual artifacts")
            
        except Exception as e:
            logger.error(f"Artifact detection failed: {e}")
        
        return artifacts
    
    def _detect_screen_tearing(self, image: np.ndarray) -> List[VisualArtifact]:
        """Detect screen tearing artifacts"""
        artifacts = []
        
        try:
            gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
            
            # Calculate horizontal differences
            diff_horizontal = np.abs(np.diff(gray.astype(np.float32), axis=0))
            
            # Find rows with high differences (potential tearing)
            tearing_threshold = self.config.get("artifact_detection", {}).get("tearing_threshold", 0.15) * 255
            
            tearing_rows = np.where(np.mean(diff_horizontal, axis=1) > tearing_threshold)[0]
            
            for row in tearing_rows:
                severity = float(np.mean(diff_horizontal[row]) / 255.0)
                artifacts.append(VisualArtifact(
                    artifact_type="screen_tearing",
                    severity=severity,
                    location=(0, row),
                    description=f"Screen tearing detected at row {row}",
                    confidence=severity
                ))
        
        except Exception as e:
            logger.warning(f"Tearing detection failed: {e}")
        
        return artifacts
    
    def _detect_color_anomalies(self, image: np.ndarray) -> List[VisualArtifact]:
        """Detect unusual color patterns indicating rendering issues"""
        artifacts = []
        
        try:
            # Convert to HSV for better color analysis
            hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
            
            # Check for saturated colors that might indicate artifacts
            saturation = hsv[:, :, 1]
            mean_saturation = np.mean(saturation)
            
            # Find regions with unusually high saturation
            high_sat_threshold = mean_saturation * 2
            high_sat_regions = np.where(saturation > high_sat_threshold)
            
            if len(high_sat_regions[0]) > 0:
                # Calculate bounding box of anomalous region
                y_min, y_max = np.min(high_sat_regions[0]), np.max(high_sat_regions[0])
                x_min, x_max = np.min(high_sat_regions[1]), np.max(high_sat_regions[1])
                
                severity = float(len(high_sat_regions[0]) / (image.shape[0] * image.shape[1]))
                
                artifacts.append(VisualArtifact(
                    artifact_type="color_anomaly",
                    severity=severity,
                    location=(x_min, y_min),
                    description=f"Unusual color saturation detected",
                    confidence=min(severity * 2, 1.0)
                ))
        
        except Exception as e:
            logger.warning(f"Color anomaly detection failed: {e}")
        
        return artifacts
    
    def _detect_sharpness_issues(self, image: np.ndarray) -> List[VisualArtifact]:
        """Detect sharpness and blurriness issues"""
        artifacts = []
        
        try:
            gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
            
            # Calculate Laplacian variance as sharpness measure
            laplacian = cv2.Laplacian(gray, cv2.CV_64F)
            sharpness = laplacian.var()
            
            sharpness_threshold = self.config.get("artifact_detection", {}).get("sharpness_threshold", 100)
            
            if sharpness < sharpness_threshold:
                severity = 1.0 - (sharpness / sharpness_threshold)
                artifacts.append(VisualArtifact(
                    artifact_type="blurriness",
                    severity=float(severity),
                    location=(0, 0),
                    description=f"Image appears blurry (sharpness: {sharpness:.2f})",
                    confidence=float(severity)
                ))
            
            # Check for over-sharpening artifacts
            elif sharpness > sharpness_threshold * 10:
                severity = min((sharpness / sharpness_threshold - 10) / 10, 1.0)
                artifacts.append(VisualArtifact(
                    artifact_type="oversharpening",
                    severity=float(severity),
                    location=(0, 0),
                    description=f"Possible over-sharpening detected (sharpness: {sharpness:.2f})",
                    confidence=float(severity)
                ))
        
        except Exception as e:
            logger.warning(f"Sharpness detection failed: {e}")
        
        return artifacts
    
    def _analyze_color_distribution(self, image: np.ndarray) -> ColorAnalysis:
        """Analyze color distribution and brightness levels"""
        try:
            # Convert to different color spaces
            hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
            lab = cv2.cvtColor(image, cv2.COLOR_BGR2LAB)
            
            # Calculate brightness (from L channel in LAB)
            brightness = lab[:, :, 0]
            brightness_mean = float(np.mean(brightness))
            brightness_std = float(np.std(brightness))
            
            # Calculate saturation (from S channel in HSV)
            saturation = hsv[:, :, 1]
            saturation_mean = float(np.mean(saturation))
            
            # Calculate color variance
            color_variance = float(np.std(image.reshape(-1, 3), axis=0).mean())
            
            # Get dominant colors using k-means
            dominant_colors = self._get_dominant_colors(image)
            
            # Calculate color histogram
            hist_b = cv2.calcHist([image], [0], None, [256], [0, 256])
            hist_g = cv2.calcHist([image], [1], None, [256], [0, 256])
            hist_r = cv2.calcHist([image], [2], None, [256], [0, 256])
            histogram = np.concatenate([hist_b, hist_g, hist_r], axis=1)
            
            return ColorAnalysis(
                dominant_colors=dominant_colors,
                brightness_mean=brightness_mean,
                brightness_std=brightness_std,
                saturation_mean=saturation_mean,
                color_variance=color_variance,
                histogram=histogram
            )
        
        except Exception as e:
            logger.error(f"Color analysis failed: {e}")
            return ColorAnalysis(
                dominant_colors=[],
                brightness_mean=0.0,
                brightness_std=0.0,
                saturation_mean=0.0,
                color_variance=0.0,
                histogram=np.array([])
            )
    
    def _get_dominant_colors(self, image: np.ndarray, k: int = 5) -> List[Tuple[int, int, int]]:
        """Extract dominant colors using k-means clustering"""
        try:
            # Reshape image to pixel list
            pixels = image.reshape(-1, 3)
            pixels = np.float32(pixels)
            
            # Define criteria and apply k-means
            criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 100, 0.2)
            _, labels, centers = cv2.kmeans(pixels, k, None, criteria, 10, cv2.KMEANS_RANDOM_CENTERS)
            
            # Convert to integer colors
            dominant_colors = [tuple(map(int, center)) for center in centers]
            
            return dominant_colors
        
        except Exception as e:
            logger.warning(f"Dominant color extraction failed: {e}")
            return []
    
    def _detect_screen_transitions(self, image: np.ndarray) -> TransitionDetection:
        """Detect screen transitions and loading states"""
        try:
            is_transition = False
            transition_type = "none"
            confidence = 0.0
            frame_diff_score = 0.0
            
            # Convert to grayscale
            gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
            
            # Check for black/fade screens
            mean_brightness = np.mean(gray)
            black_threshold = self.config.get("transition_detection", {}).get("black_frame_threshold", 10)
            
            if mean_brightness < black_threshold:
                is_transition = True
                transition_type = "fade_to_black"
                confidence = 1.0 - (mean_brightness / black_threshold)
            
            # Check for loading indicators (if previous frame exists)
            if self.previous_frame is not None:
                # Calculate frame difference
                prev_gray = cv2.cvtColor(self.previous_frame, cv2.COLOR_BGR2GRAY)
                
                if prev_gray.shape == gray.shape:
                    diff = cv2.absdiff(gray, prev_gray)
                    frame_diff_score = float(np.sum(diff))
                    
                    diff_threshold = self.config.get("transition_detection", {}).get("frame_diff_threshold", 5000)
                    
                    if frame_diff_score > diff_threshold:
                        is_transition = True
                        transition_type = "scene_change"
                        confidence = min(frame_diff_score / (diff_threshold * 2), 1.0)
            
            # Update previous frame
            self.previous_frame = image.copy()
            
            return TransitionDetection(
                is_transition=is_transition,
                transition_type=transition_type,
                confidence=confidence,
                frame_diff_score=frame_diff_score
            )
        
        except Exception as e:
            logger.error(f"Transition detection failed: {e}")
            return TransitionDetection(
                is_transition=False,
                transition_type="none",
                confidence=0.0,
                frame_diff_score=0.0
            )
    
    def _calculate_confidence_scores(self, results: Dict[str, Any]) -> Dict[str, float]:
        """Calculate confidence scores for each analysis component"""
        confidence_scores = {}
        
        try:
            # UI detection confidence
            if results["ui_elements"]:
                ui_confidences = [elem.confidence for elem in results["ui_elements"]]
                confidence_scores["ui_detection"] = float(np.mean(ui_confidences))
            else:
                confidence_scores["ui_detection"] = 0.0
            
            # Artifact detection confidence
            if results["visual_artifacts"]:
                artifact_confidences = [art.confidence for art in results["visual_artifacts"]]
                confidence_scores["artifact_detection"] = float(np.mean(artifact_confidences))
            else:
                confidence_scores["artifact_detection"] = 1.0  # No artifacts is good
            
            # Color analysis confidence (based on variance)
            if results["color_analysis"]:
                color_confidence = min(results["color_analysis"].color_variance / 50.0, 1.0)
                confidence_scores["color_analysis"] = float(color_confidence)
            else:
                confidence_scores["color_analysis"] = 0.0
            
            # Transition detection confidence
            if results["transition_analysis"]:
                confidence_scores["transition_detection"] = results["transition_analysis"].confidence
            else:
                confidence_scores["transition_detection"] = 0.0
            
            # Overall confidence (weighted average)
            weights = {"ui_detection": 0.3, "artifact_detection": 0.3, 
                      "color_analysis": 0.2, "transition_detection": 0.2}
            
            overall_confidence = sum(confidence_scores[k] * weights[k] 
                                   for k in weights if k in confidence_scores)
            confidence_scores["overall"] = float(overall_confidence)
        
        except Exception as e:
            logger.error(f"Confidence calculation failed: {e}")
            confidence_scores = {"overall": 0.0}
        
        return confidence_scores

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
    
    analyzer = VisualAnalyzer()
    results = analyzer.analyze_screenshot(image_path)
    
    # Print results
    print(json.dumps({
        "ui_elements_count": len(results["ui_elements"]),
        "artifacts_count": len(results["visual_artifacts"]),
        "confidence_scores": results["confidence_scores"],
        "analysis_duration": results["analysis_duration"]
    }, indent=2))