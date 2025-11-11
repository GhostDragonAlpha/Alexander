"""
Text Recognition System for Gameplay Screenshot Analysis
Uses Tesseract OCR to extract text from screenshots and identify UI text elements.
"""

import cv2
import numpy as np
import pytesseract
import json
import time
import re
from typing import Dict, List, Tuple, Optional, Any
from dataclasses import dataclass
from pathlib import Path
import logging

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

@dataclass
class TextElement:
    """Represents a detected text element"""
    text: str
    x: int
    y: int
    width: int
    height: int
    confidence: float
    element_type: str
    is_numeric: bool
    is_ui_element: bool

@dataclass
class OCRResult:
    """Complete OCR results"""
    text_elements: List[TextElement]
    full_text: str
    processing_time: float
    average_confidence: float
    detected_languages: List[str]

class TextRecognizer:
    """Text recognition system for game screenshots using Tesseract OCR"""
    
    def __init__(self, config_path: str = "visual_analysis_config.json"):
        """Initialize the text recognizer with configuration"""
        self.config = self._load_config(config_path)
        self.tesseract_config = self.config.get("text_recognizer", {}).get("tesseract_config", {})
        self.text_filters = self.config.get("text_recognizer", {}).get("text_filters", {})
        
        self._setup_tesseract()
        self._compile_patterns()
    
    def _load_config(self, config_path: str) -> Dict[str, Any]:
        """Load configuration from JSON file"""
        try:
            with open(config_path, 'r') as f:
                return json.load(f)
        except Exception as e:
            logger.error(f"Failed to load config: {e}")
            return {}
    
    def _setup_tesseract(self):
        """Setup Tesseract OCR engine"""
        try:
            # Set Tesseract path if specified in config
            tesseract_path = self.tesseract_config.get("tesseract_path")
            if tesseract_path:
                pytesseract.pytesseract.tesseract_cmd = tesseract_path
            
            # Test Tesseract availability
            version = pytesseract.get_tesseract_version()
            logger.info(f"Tesseract version: {version}")
            
            # Configure Tesseract parameters
            self.tess_config_str = self._build_tesseract_config()
            
        except Exception as e:
            logger.error(f"Tesseract setup failed: {e}")
            logger.warning("Text recognition will not be available")
            # Set default config string to prevent attribute errors
            self.tess_config_str = "--oem 3 --psm 11 -l eng"
    
    def _build_tesseract_config(self) -> str:
        """Build Tesseract configuration string"""
        config_parts = []
        
        # OEM (Engine Mode)
        oem = self.tesseract_config.get("oem", 3)
        config_parts.append(f"--oem {oem}")
        
        # PSM (Page Segmentation Mode)
        psm = self.tesseract_config.get("psm", 11)
        config_parts.append(f"--psm {psm}")
        
        # Language
        lang = self.tesseract_config.get("lang", "eng")
        config_parts.append(f"-l {lang}")
        
        # Character whitelist
        whitelist = self.tesseract_config.get("whitelist_characters")
        if whitelist:
            config_parts.append(f'-c tessedit_char_whitelist={whitelist}')
        
        # Additional configurations
        config_parts.append("-c preserve_interword_spaces=1")
        
        return " ".join(config_parts)
    
    def _compile_patterns(self):
        """Compile regex patterns for text filtering"""
        try:
            # Numeric patterns
            numeric_patterns = self.text_filters.get("numeric_patterns", [])
            self.numeric_regexes = [re.compile(pattern) for pattern in numeric_patterns]
            
            # UI element patterns
            ui_patterns = self.text_filters.get("ui_element_patterns", [])
            self.ui_regexes = [re.compile(pattern, re.IGNORECASE) for pattern in ui_patterns]
            
            logger.info(f"Compiled {len(self.numeric_regexes)} numeric patterns and {len(self.ui_regexes)} UI patterns")
            
        except Exception as e:
            logger.error(f"Pattern compilation failed: {e}")
            self.numeric_regexes = []
            self.ui_regexes = []
    
    def recognize_text(self, image_path: str) -> OCRResult:
        """
        Recognize text in a screenshot
        
        Args:
            image_path: Path to the screenshot image
            
        Returns:
            OCRResult containing all text recognition results
        """
        start_time = time.time()
        
        try:
            # Load image
            image = cv2.imread(image_path)
            if image is None:
                raise ValueError(f"Failed to load image: {image_path}")
            
            # Preprocess image
            preprocessed = self._preprocess_image(image)
            
            # Perform OCR
            ocr_data = pytesseract.image_to_data(
                preprocessed, 
                config=self.tess_config_str,
                output_type=pytesseract.Output.DICT
            )
            
            # Extract text elements
            text_elements = self._extract_text_elements(ocr_data, image)
            
            # Get full text
            full_text = pytesseract.image_to_string(preprocessed, config=self.tess_config_str)
            
            # Detect languages
            languages = self._detect_languages(preprocessed)
            
            # Calculate average confidence
            avg_confidence = np.mean([elem.confidence for elem in text_elements]) if text_elements else 0.0
            
            processing_time = time.time() - start_time
            
            result = OCRResult(
                text_elements=text_elements,
                full_text=full_text,
                processing_time=processing_time,
                average_confidence=float(avg_confidence),
                detected_languages=languages
            )
            
            logger.info(f"OCR completed in {processing_time:.3f}s, found {len(text_elements)} text elements")
            
            return result
            
        except Exception as e:
            logger.error(f"OCR failed: {e}")
            return OCRResult(
                text_elements=[],
                full_text="",
                processing_time=time.time() - start_time,
                average_confidence=0.0,
                detected_languages=[]
            )
    
    def _preprocess_image(self, image: np.ndarray) -> np.ndarray:
        """Preprocess image for better OCR results"""
        try:
            preprocessing_config = self.config.get("text_recognizer", {}).get("preprocessing", {})
            
            # Convert to grayscale
            gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
            
            # Denoise if enabled
            if preprocessing_config.get("denoise", True):
                gray = cv2.fastNlMeansDenoising(gray, None, 10, 7, 21)
            
            # Enhance contrast if enabled
            if preprocessing_config.get("contrast_enhancement", True):
                gray = cv2.equalizeHist(gray)
            
            # Apply thresholding if enabled
            if preprocessing_config.get("thresholding", True):
                # Use adaptive thresholding for better results
                gray = cv2.adaptiveThreshold(
                    gray, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, 
                    cv2.THRESH_BINARY, 11, 2
                )
            
            # Deskew if enabled
            if preprocessing_config.get("deskew", True):
                gray = self._deskew_image(gray)
            
            return gray
            
        except Exception as e:
            logger.warning(f"Image preprocessing failed: {e}")
            return cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    
    def _deskew_image(self, image: np.ndarray) -> np.ndarray:
        """Deskew (rotate) image to improve OCR accuracy"""
        try:
            # Find contours
            coords = np.column_stack(np.where(image > 0))
            
            if len(coords) < 10:  # Not enough content to deskew
                return image
            
            # Calculate angle
            angle = cv2.minAreaRect(coords)[-1]
            
            if angle < -45:
                angle = -(90 + angle)
            else:
                angle = -angle
            
            # Rotate image
            (h, w) = image.shape[:2]
            center = (w // 2, h // 2)
            M = cv2.getRotationMatrix2D(center, angle, 1.0)
            rotated = cv2.warpAffine(image, M, (w, h), 
                                   flags=cv2.INTER_CUBIC, 
                                   borderMode=cv2.BORDER_REPLICATE)
            
            return rotated
            
        except Exception as e:
            logger.warning(f"Deskewing failed: {e}")
            return image
    
    def _extract_text_elements(self, ocr_data: Dict[str, Any], original_image: np.ndarray) -> List[TextElement]:
        """Extract individual text elements from OCR data"""
        elements = []
        
        try:
            min_text_length = self.text_filters.get("min_text_length", 2)
            max_text_length = self.text_filters.get("max_text_length", 100)
            min_confidence = self.tesseract_config.get("min_confidence", 60)
            
            n_boxes = len(ocr_data['level'])
            
            for i in range(n_boxes):
                text = ocr_data['text'][i].strip()
                conf = int(ocr_data['conf'][i])
                
                # Filter by confidence and text length
                if conf < min_confidence or not text:
                    continue
                
                if len(text) < min_text_length or len(text) > max_text_length:
                    continue
                
                # Get bounding box
                x = ocr_data['left'][i]
                y = ocr_data['top'][i]
                w = ocr_data['width'][i]
                h = ocr_data['height'][i]
                
                # Classify text element
                element_type = self._classify_text_element(text)
                is_numeric = self._is_numeric_text(text)
                is_ui_element = self._is_ui_element(text)
                
                elements.append(TextElement(
                    text=text,
                    x=x,
                    y=y,
                    width=w,
                    height=h,
                    confidence=float(conf),
                    element_type=element_type,
                    is_numeric=is_numeric,
                    is_ui_element=is_ui_element
                ))
            
            # Merge nearby text elements that might belong together
            elements = self._merge_text_elements(elements)
            
            logger.info(f"Extracted {len(elements)} text elements")
            
        except Exception as e:
            logger.error(f"Text element extraction failed: {e}")
        
        return elements
    
    def _classify_text_element(self, text: str) -> str:
        """Classify text element type"""
        try:
            # Check for numeric values
            if self._is_numeric_text(text):
                return "numeric_value"
            
            # Check for UI elements
            if self._is_ui_element(text):
                return "ui_element"
            
            # Check for common game text patterns
            if any(keyword in text.lower() for keyword in ["mission", "objective", "quest"]):
                return "mission_text"
            
            if any(keyword in text.lower() for keyword in ["health", "shield", "armor", "energy"]):
                return "status_text"
            
            if any(keyword in text.lower() for keyword in ["credit", "money", "gold", "resource"]):
                return "resource_text"
            
            if any(keyword in text.lower() for keyword in ["inventory", "item", "equipment", "weapon"]):
                return "inventory_text"
            
            # Default classification
            return "general_text"
            
        except Exception as e:
            logger.warning(f"Text classification failed: {e}")
            return "unknown"
    
    def _is_numeric_text(self, text: str) -> bool:
        """Check if text is numeric"""
        try:
            # Clean text
            cleaned = text.replace(",", "").replace(".", "").replace(":", "").replace("/", "")
            
            # Check against numeric patterns
            for pattern in self.numeric_regexes:
                if pattern.match(text):
                    return True
            
            # Check if it's a pure number
            if cleaned.isdigit():
                return True
            
            return False
            
        except Exception as e:
            logger.warning(f"Numeric text check failed: {e}")
            return False
    
    def _is_ui_element(self, text: str) -> bool:
        """Check if text is a UI element"""
        try:
            # Check against UI patterns
            for pattern in self.ui_regexes:
                if pattern.search(text):
                    return True
            
            # Check for common UI button labels
            ui_labels = ["ok", "cancel", "apply", "back", "next", "previous", "menu", 
                        "options", "settings", "quit", "exit", "resume", "start", 
                        "load", "save", "delete", "confirm", "yes", "no"]
            
            if text.lower() in ui_labels:
                return True
            
            return False
            
        except Exception as e:
            logger.warning(f"UI element check failed: {e}")
            return False
    
    def _merge_text_elements(self, elements: List[TextElement]) -> List[TextElement]:
        """Merge nearby text elements that belong together"""
        try:
            if len(elements) < 2:
                return elements
            
            merged = []
            used_indices = set()
            
            # Sort by vertical position, then horizontal
            sorted_elements = sorted(elements, key=lambda e: (e.y, e.x))
            
            for i, elem1 in enumerate(sorted_elements):
                if i in used_indices:
                    continue
                
                # Find nearby elements
                nearby_group = [elem1]
                used_indices.add(i)
                
                for j, elem2 in enumerate(sorted_elements[i+1:], i+1):
                    if j in used_indices:
                        continue
                    
                    # Check if elements are close enough
                    vertical_overlap = self._check_vertical_overlap(elem1, elem2)
                    horizontal_distance = abs(elem2.x - (elem1.x + elem1.width))
                    
                    # Merge criteria: vertical overlap and close horizontal distance
                    if vertical_overlap > 0.5 and horizontal_distance < 50:
                        nearby_group.append(elem2)
                        used_indices.add(j)
                
                # Merge group into single element if appropriate
                if len(nearby_group) > 1:
                    merged_elem = self._merge_element_group(nearby_group)
                    merged.append(merged_elem)
                else:
                    merged.append(elem1)
            
            return merged
            
        except Exception as e:
            logger.warning(f"Text element merging failed: {e}")
            return elements
    
    def _check_vertical_overlap(self, elem1: TextElement, elem2: TextElement) -> float:
        """Check vertical overlap between two elements"""
        try:
            # Calculate overlap
            y1_min, y1_max = elem1.y, elem1.y + elem1.height
            y2_min, y2_max = elem2.y, elem2.y + elem2.height
            
            overlap_start = max(y1_min, y2_min)
            overlap_end = min(y1_max, y2_max)
            
            if overlap_end <= overlap_start:
                return 0.0
            
            # Calculate overlap ratio relative to smaller element
            overlap_height = overlap_end - overlap_start
            smaller_height = min(elem1.height, elem2.height)
            
            return overlap_height / smaller_height if smaller_height > 0 else 0.0
            
        except Exception as e:
            logger.warning(f"Vertical overlap check failed: {e}")
            return 0.0
    
    def _merge_element_group(self, group: List[TextElement]) -> TextElement:
        """Merge a group of text elements into one"""
        try:
            # Sort by horizontal position
            sorted_group = sorted(group, key=lambda e: e.x)
            
            # Combine text
            combined_text = " ".join(elem.text for elem in sorted_group)
            
            # Calculate bounding box
            min_x = min(elem.x for elem in group)
            min_y = min(elem.y for elem in group)
            max_x = max(elem.x + elem.width for elem in group)
            max_y = max(elem.y + elem.height for elem in group)
            
            width = max_x - min_x
            height = max_y - min_y
            
            # Average confidence
            avg_confidence = np.mean([elem.confidence for elem in group])
            
            # Determine element type (use most specific)
            element_types = [elem.element_type for elem in group]
            priority_types = ["mission_text", "status_text", "resource_text", "inventory_text", "ui_element", "numeric_value"]
            
            final_type = "general_text"
            for p_type in priority_types:
                if p_type in element_types:
                    final_type = p_type
                    break
            
            # Check if merged text is numeric or UI
            is_numeric = self._is_numeric_text(combined_text)
            is_ui_element = self._is_ui_element(combined_text)
            
            return TextElement(
                text=combined_text,
                x=min_x,
                y=min_y,
                width=width,
                height=height,
                confidence=float(avg_confidence),
                element_type=final_type,
                is_numeric=is_numeric,
                is_ui_element=is_ui_element
            )
            
        except Exception as e:
            logger.warning(f"Element group merging failed: {e}")
            return group[0] if group else None
    
    def _detect_languages(self, image: np.ndarray) -> List[str]:
        """Detect languages in the image"""
        try:
            # Use Tesseract to detect script
            osd_data = pytesseract.image_to_osd(image, output_type=pytesseract.Output.DICT)
            
            languages = []
            
            if osd_data.get("script"):
                script = osd_data["script"]
                if script == "Latin":
                    languages.append("en")
                elif script == "Cyrillic":
                    languages.append("ru")
                elif script == "Han":
                    languages.append("zh")
            
            return languages
            
        except Exception as e:
            logger.debug(f"Language detection failed: {e}")
            return ["en"]  # Default to English
    
    def extract_numerical_values(self, image_path: str) -> List[Dict[str, Any]]:
        """
        Extract only numerical values from screenshot
        
        Args:
            image_path: Path to the screenshot image
            
        Returns:
            List of dictionaries containing numeric values and their properties
        """
        result = self.recognize_text(image_path)
        
        numeric_values = []
        
        for elem in result.text_elements:
            if elem.is_numeric:
                # Try to parse the numeric value
                value = self._parse_numeric_value(elem.text)
                
                if value is not None:
                    numeric_values.append({
                        "value": value,
                        "text": elem.text,
                        "x": elem.x,
                        "y": elem.y,
                        "width": elem.width,
                        "height": elem.height,
                        "confidence": elem.confidence,
                        "element_type": elem.element_type
                    })
        
        return numeric_values
    
    def _parse_numeric_value(self, text: str) -> Optional[float]:
        """Parse numeric value from text"""
        try:
            # Remove common separators
            cleaned = text.replace(",", "").replace(" ", "")
            
            # Handle different formats
            if ":" in cleaned:  # Time format
                parts = cleaned.split(":")
                if len(parts) == 2:
                    return float(parts[0]) + float(parts[1]) / 60.0
            
            if "/" in cleaned:  # Fraction format
                parts = cleaned.split("/")
                if len(parts) == 2 and parts[1] != "0":
                    return float(parts[0]) / float(parts[1])
            
            # Standard number parsing
            return float(cleaned)
            
        except ValueError:
            return None
        except Exception as e:
            logger.warning(f"Numeric parsing failed for '{text}': {e}")
            return None
    
    def extract_ui_elements(self, image_path: str) -> List[Dict[str, Any]]:
        """
        Extract only UI elements from screenshot
        
        Args:
            image_path: Path to the screenshot image
            
        Returns:
            List of dictionaries containing UI elements
        """
        result = self.recognize_text(image_path)
        
        ui_elements = []
        
        for elem in result.text_elements:
            if elem.is_ui_element:
                ui_elements.append({
                    "text": elem.text,
                    "x": elem.x,
                    "y": elem.y,
                    "width": elem.width,
                    "height": elem.height,
                    "confidence": elem.confidence,
                    "element_type": elem.element_type
                })
        
        return ui_elements
    
    def find_text_by_pattern(self, image_path: str, pattern: str) -> List[TextElement]:
        """
        Find text matching a specific pattern
        
        Args:
            image_path: Path to the screenshot image
            pattern: Regex pattern to match
            
        Returns:
            List of text elements matching the pattern
        """
        result = self.recognize_text(image_path)
        
        try:
            regex = re.compile(pattern, re.IGNORECASE)
            matches = [elem for elem in result.text_elements if regex.search(elem.text)]
            return matches
        except Exception as e:
            logger.error(f"Pattern matching failed: {e}")
            return []
    
    def get_text_at_location(self, image_path: str, x: int, y: int, 
                           width: int = 100, height: int = 50) -> Optional[TextElement]:
        """
        Get text at a specific location
        
        Args:
            image_path: Path to the screenshot image
            x, y: Top-left coordinates
            width, height: Region size
            
        Returns:
            TextElement if text found, None otherwise
        """
        try:
            # Load image
            image = cv2.imread(image_path)
            if image is None:
                return None
            
            # Extract region of interest
            roi = image[y:y+height, x:x+width]
            
            if roi.size == 0:
                return None
            
            # Preprocess ROI
            preprocessed = self._preprocess_image(roi)
            
            # Perform OCR on ROI
            text = pytesseract.image_to_string(preprocessed, config=self.tess_config_str).strip()
            
            if not text:
                return None
            
            # Get confidence
            data = pytesseract.image_to_data(preprocessed, config=self.tess_config_str, 
                                           output_type=pytesseract.Output.DICT)
            
            if data['text']:
                confidence = np.mean([conf for conf in data['conf'] if conf != '-1'])
            else:
                confidence = 0.0
            
            # Classify text
            element_type = self._classify_text_element(text)
            is_numeric = self._is_numeric_text(text)
            is_ui_element = self._is_ui_element(text)
            
            return TextElement(
                text=text,
                x=x,
                y=y,
                width=width,
                height=height,
                confidence=float(confidence),
                element_type=element_type,
                is_numeric=is_numeric,
                is_ui_element=is_ui_element
            )
            
        except Exception as e:
            logger.error(f"Text extraction at location failed: {e}")
            return None

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
    
    recognizer = TextRecognizer()
    
    # Full OCR
    result = recognizer.recognize_text(image_path)
    
    print(json.dumps({
        "text_elements_found": len(result.text_elements),
        "average_confidence": result.average_confidence,
        "processing_time": result.processing_time,
        "detected_languages": result.detected_languages
    }, indent=2))
    
    # Show first few text elements
    print("\nDetected Text Elements:")
    for elem in result.text_elements[:10]:
        print(f"  - '{elem.text}' (conf: {elem.confidence:.1f}, type: {elem.element_type})")
    
    # Extract numeric values
    numeric_values = recognizer.extract_numerical_values(image_path)
    if numeric_values:
        print(f"\nNumeric Values Found: {len(numeric_values)}")
        for val in numeric_values[:5]:
            print(f"  - {val['text']} = {val['value']} (conf: {val['confidence']:.1f})")
    
    # Extract UI elements
    ui_elements = recognizer.extract_ui_elements(image_path)
    if ui_elements:
        print(f"\nUI Elements Found: {len(ui_elements)}")
        for elem in ui_elements[:5]:
            print(f"  - '{elem['text']}' (type: {elem['element_type']})")