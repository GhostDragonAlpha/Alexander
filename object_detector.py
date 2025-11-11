"""
Object Detection System for Gameplay Screenshot Analysis
Uses pre-trained models to detect game objects, ships, planets, and UI elements.
"""

import cv2
import numpy as np
import json
import time
from typing import Dict, List, Tuple, Optional, Any
from dataclasses import dataclass
from pathlib import Path
import logging

# Try to import torch, but make it optional
try:
    import torch
    TORCH_AVAILABLE = True
except ImportError:
    TORCH_AVAILABLE = False
    torch = None

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

@dataclass
class DetectedObject:
    """Represents a detected game object"""
    class_name: str
    confidence: float
    x: int
    y: int
    width: int
    height: int
    center_x: float
    center_y: float
    object_id: Optional[int] = None
    tracked: bool = False
    disappearance_count: int = 0

@dataclass
class ObjectTracking:
    """Object tracking information"""
    object_id: int
    positions: List[Tuple[float, float]]
    confidences: List[float]
    class_name: str
    first_seen: float
    last_seen: float
    disappeared: bool = False

@dataclass
class DetectionResult:
    """Complete object detection results"""
    detected_objects: List[DetectedObject]
    frame_number: int
    processing_time: float
    model_confidence: float
    tracking_info: List[ObjectTracking]

class ObjectDetector:
    """Object detection system for game screenshots"""
    
    def __init__(self, config_path: str = "visual_analysis_config.json"):
        """Initialize the object detector with configuration"""
        self.config = self._load_config(config_path)
        self.model = None
        self.device = self._setup_device()
        self.object_classes = self.config.get("object_detector", {}).get("object_classes", [])
        self.tracked_objects: Dict[int, ObjectTracking] = {}
        self.next_object_id = 0
        self.frame_number = 0
        
        self._load_model()
    
    def _load_config(self, config_path: str) -> Dict[str, Any]:
        """Load configuration from JSON file"""
        try:
            with open(config_path, 'r') as f:
                return json.load(f)
        except Exception as e:
            logger.error(f"Failed to load config: {e}")
            return {}
    
    def _setup_device(self):
        """Setup PyTorch device (CPU/GPU)"""
        try:
            if torch.cuda.is_available() and self.config.get("performance", {}).get("enable_gpu", True):
                device = torch.device("cuda")
                logger.info(f"Using GPU: {torch.cuda.get_device_name()}")
            else:
                device = torch.device("cpu")
                logger.info("Using CPU for inference")
            
            return device
        except Exception as e:
            logger.warning(f"PyTorch not available: {e}")
            return None
    
    def _load_model(self):
        """Load the object detection model"""
        try:
            model_config = self.config.get("object_detector", {}).get("model_config", {})
            model_type = model_config.get("model_type", "yolov5")
            
            if model_type == "yolov5":
                self._load_yolov5_model(model_config)
            else:
                logger.warning(f"Unsupported model type: {model_type}")
                self._load_fallback_model()
            
        except Exception as e:
            logger.error(f"Model loading failed: {e}")
            self._load_fallback_model()
    
    def _load_yolov5_model(self, model_config: Dict[str, Any]):
        """Load YOLOv5 model"""
        if not TORCH_AVAILABLE:
            logger.warning("PyTorch not available, skipping YOLOv5 model loading")
            self._load_fallback_model()
            return
            
        try:
            # Try to load YOLOv5
            model_path = model_config.get("model_path", "yolov5s")
            
            if Path(model_path).exists():
                # Load custom model
                self.model = torch.hub.load('ultralytics/yolov5', 'custom', path=model_path)
            else:
                # Load pretrained model
                self.model = torch.hub.load('ultralytics/yolov5', 'yolov5s', pretrained=True)
            
            if self.device:
                self.model.to(self.device)
            
            # Configure model
            self.model.conf = model_config.get("confidence_threshold", 0.5)
            self.model.iou = model_config.get("nms_threshold", 0.4)
            
            logger.info("YOLOv5 model loaded successfully")
            
        except Exception as e:
            logger.error(f"YOLOv5 loading failed: {e}")
            self._load_fallback_model()
    
    def _load_fallback_model(self):
        """Load a simple fallback model using OpenCV DNN"""
        try:
            logger.warning("Loading fallback OpenCV DNN model")
            
            # Use MobileNet SSD as fallback
            model_file = "models/MobileNetSSD_deploy.caffemodel"
            config_file = "models/MobileNetSSD_deploy.prototxt"
            
            if Path(model_file).exists() and Path(config_file).exists():
                self.model = cv2.dnn.readNetFromCaffe(config_file, model_file)
                
                if self.device and torch.cuda.is_available():
                    self.model.setPreferableBackend(cv2.dnn.DNN_BACKEND_CUDA)
                    self.model.setPreferableTarget(cv2.dnn.DNN_TARGET_CUDA)
                
                logger.info("Fallback OpenCV DNN model loaded")
            else:
                logger.warning("No fallback model files found")
                self.model = None
        
        except Exception as e:
            logger.error(f"Fallback model loading failed: {e}")
            self.model = None
    
    def detect_objects(self, image_path: str) -> DetectionResult:
        """
        Detect objects in a screenshot
        
        Args:
            image_path: Path to the screenshot image
            
        Returns:
            DetectionResult containing all detected objects
        """
        start_time = time.time()
        
        try:
            # Load image
            image = cv2.imread(image_path)
            if image is None:
                raise ValueError(f"Failed to load image: {image_path}")
            
            self.frame_number += 1
            
            detected_objects = []
            model_confidence = 0.0
            
            if self.model is not None:
                # Perform detection based on model type
                if hasattr(self.model, '__module__') and 'yolov5' in self.model.__module__:
                    detected_objects = self._detect_with_yolov5(image)
                else:
                    detected_objects = self._detect_with_opencv(image)
                
                # Update model confidence
                if detected_objects:
                    model_confidence = np.mean([obj.confidence for obj in detected_objects])
                
                # Perform object tracking
                self._update_tracking(detected_objects)
            
            processing_time = time.time() - start_time
            
            result = DetectionResult(
                detected_objects=detected_objects,
                frame_number=self.frame_number,
                processing_time=processing_time,
                model_confidence=float(model_confidence),
                tracking_info=list(self.tracked_objects.values())
            )
            
            logger.info(f"Object detection completed in {processing_time:.3f}s, found {len(detected_objects)} objects")
            
            return result
            
        except Exception as e:
            logger.error(f"Object detection failed: {e}")
            return DetectionResult(
                detected_objects=[],
                frame_number=self.frame_number,
                processing_time=time.time() - start_time,
                model_confidence=0.0,
                tracking_info=[]
            )
    
    def _detect_with_yolov5(self, image: np.ndarray) -> List[DetectedObject]:
        """Detect objects using YOLOv5"""
        objects = []
        
        if not TORCH_AVAILABLE or not hasattr(self.model, 'xyxy'):
            logger.warning("YOLOv5 not available, using fallback detection")
            return self._detect_with_opencv(image)
        
        try:
            # Perform inference
            results = self.model(image)
            
            # Extract detections
            detections = results.xyxy[0].cpu().numpy()
            
            for detection in detections:
                x1, y1, x2, y2, confidence, class_id = detection[:6]
                
                # Filter by confidence
                if confidence < self.model.conf:
                    continue
                
                # Get class name
                if len(self.object_classes) > int(class_id):
                    class_name = self.object_classes[int(class_id)]
                else:
                    class_name = f"class_{int(class_id)}"
                
                # Calculate center coordinates
                center_x = (x1 + x2) / 2.0
                center_y = (y1 + y2) / 2.0
                
                objects.append(DetectedObject(
                    class_name=class_name,
                    confidence=float(confidence),
                    x=int(x1),
                    y=int(y1),
                    width=int(x2 - x1),
                    height=int(y2 - y1),
                    center_x=center_x,
                    center_y=center_y
                ))
        
        except Exception as e:
            logger.error(f"YOLOv5 detection failed: {e}")
        
        return objects
    
    def _detect_with_opencv(self, image: np.ndarray) -> List[DetectedObject]:
        """Detect objects using OpenCV DNN (fallback)"""
        objects = []
        
        try:
            # Prepare input blob
            input_size = self.config.get("object_detector", {}).get("model_config", {}).get("input_size", [300, 300])
            blob = cv2.dnn.blobFromImage(
                image, 
                0.007843, 
                (input_size[0], input_size[1]), 
                127.5, 
                False
            )
            
            # Perform inference
            self.model.setInput(blob)
            detections = self.model.forward()
            
            # Extract detections
            confidence_threshold = self.config.get("object_detector", {}).get("model_config", {}).get("confidence_threshold", 0.5)
            
            for i in range(detections.shape[2]):
                confidence = detections[0, 0, i, 2]
                
                if confidence > confidence_threshold:
                    class_id = int(detections[0, 0, i, 1])
                    
                    # Get bounding box
                    h, w = image.shape[:2]
                    box = detections[0, 0, i, 3:7] * np.array([w, h, w, h])
                    x1, y1, x2, y2 = box.astype("int")
                    
                    # Map class ID to game object class
                    class_name = self._map_class_id(class_id)
                    
                    # Calculate center coordinates
                    center_x = (x1 + x2) / 2.0
                    center_y = (y1 + y2) / 2.0
                    
                    objects.append(DetectedObject(
                        class_name=class_name,
                        confidence=float(confidence),
                        x=x1,
                        y=y1,
                        width=x2 - x1,
                        height=y2 - y1,
                        center_x=center_x,
                        center_y=center_y
                    ))
        
        except Exception as e:
            logger.error(f"OpenCV detection failed: {e}")
        
        return objects
    
    def _map_class_id(self, class_id: int) -> str:
        """Map OpenCV class ID to game object class"""
        # MobileNet SSD class mapping (simplified for game objects)
        class_mapping = {
            0: "ship_player",      # background -> player ship
            1: "ship_enemy",       # aeroplane -> enemy ship
            2: "station",          # bicycle -> space station
            5: "ship_ally",        # bus -> ally ship
            7: "asteroid",         # train -> asteroid
            15: "planet_terrestrial", # bird -> planet
            16: "planet_gas_giant",   # cat -> gas giant
            19: "debris",          # cow -> debris
            20: "weapon_projectile"   # elephant -> projectile
        }
        
        return class_mapping.get(class_id, f"unknown_{class_id}")
    
    def _update_tracking(self, detected_objects: List[DetectedObject]):
        """Update object tracking information"""
        try:
            tracking_config = self.config.get("object_detector", {}).get("tracking", {})
            max_disappeared = tracking_config.get("max_disappeared", 5)
            max_distance = tracking_config.get("max_distance", 100)
            min_tracking_confidence = tracking_config.get("min_tracking_confidence", 0.3)
            
            current_time = time.time()
            
            if not detected_objects:
                # Increment disappearance count for all tracked objects
                for obj_id, tracking_info in list(self.tracked_objects.items()):
                    tracking_info.disappearance_count += 1
                    if tracking_info.disappearance_count > max_disappeared:
                        tracking_info.disappeared = True
                return
            
            # If no tracked objects, register all detected objects
            if not self.tracked_objects:
                for obj in detected_objects:
                    if obj.confidence >= min_tracking_confidence:
                        self._register_object(obj, current_time)
                return
            
            # Get object IDs and centers of tracked objects
            object_ids = list(self.tracked_objects.keys())
            object_centers = [(self.tracked_objects[obj_id].positions[-1] if self.tracked_objects[obj_id].positions else (0, 0)) 
                            for obj_id in object_ids]
            
            # Get centers of detected objects
            detected_centers = [(obj.center_x, obj.center_y) for obj in detected_objects]
            
            # Compute distances between tracked objects and detected objects
            distances = self._calculate_distances(object_centers, detected_centers)
            
            # Match objects based on minimum distance
            used_rows = set()
            used_cols = set()
            
            for row in range(len(distances)):
                if row in used_rows:
                    continue
                
                min_col = None
                min_distance = float("inf")
                
                for col in range(len(distances[row])):
                    if col in used_cols:
                        continue
                    
                    if distances[row][col] < min_distance:
                        min_distance = distances[row][col]
                        min_col = col
                
                if min_col is not None and min_distance < max_distance:
                    # Update existing object
                    obj_id = object_ids[row]
                    self._update_object(obj_id, detected_objects[min_col], current_time)
                    used_rows.add(row)
                    used_cols.add(min_col)
            
            # Register unmatched detected objects as new objects
            for col in range(len(detected_objects)):
                if col not in used_cols:
                    obj = detected_objects[col]
                    if obj.confidence >= min_tracking_confidence:
                        self._register_object(obj, current_time)
            
            # Mark unmatched tracked objects as disappeared
            for row in range(len(object_ids)):
                if row not in used_rows:
                    obj_id = object_ids[row]
                    self.tracked_objects[obj_id].disappearance_count += 1
                    if self.tracked_objects[obj_id].disappearance_count > max_disappeared:
                        self.tracked_objects[obj_id].disappeared = True
        
        except Exception as e:
            logger.error(f"Tracking update failed: {e}")
    
    def _calculate_distances(self, centers1: List[Tuple[float, float]], 
                           centers2: List[Tuple[float, float]]) -> np.ndarray:
        """Calculate Euclidean distances between two sets of points"""
        try:
            centers1_array = np.array(centers1)
            centers2_array = np.array(centers2)
            
            # Expand dimensions for broadcasting
            centers1_expanded = np.expand_dims(centers1_array, axis=1)
            centers2_expanded = np.expand_dims(centers2_array, axis=0)
            
            # Calculate Euclidean distances
            distances = np.sqrt(np.sum((centers1_expanded - centers2_expanded) ** 2, axis=2))
            
            return distances
        
        except Exception as e:
            logger.error(f"Distance calculation failed: {e}")
            return np.array([])
    
    def _register_object(self, obj: DetectedObject, current_time: float):
        """Register a new object for tracking"""
        try:
            obj_id = self.next_object_id
            self.next_object_id += 1
            
            tracking_info = ObjectTracking(
                object_id=obj_id,
                positions=[(obj.center_x, obj.center_y)],
                confidences=[obj.confidence],
                class_name=obj.class_name,
                first_seen=current_time,
                last_seen=current_time,
                disappeared=False
            )
            
            self.tracked_objects[obj_id] = tracking_info
            obj.object_id = obj_id
            obj.tracked = True
            
        except Exception as e:
            logger.error(f"Object registration failed: {e}")
    
    def _update_object(self, obj_id: int, obj: DetectedObject, current_time: float):
        """Update existing tracked object"""
        try:
            if obj_id in self.tracked_objects:
                tracking_info = self.tracked_objects[obj_id]
                tracking_info.positions.append((obj.center_x, obj.center_y))
                tracking_info.confidences.append(obj.confidence)
                tracking_info.last_seen = current_time
                tracking_info.disappearance_count = 0
                tracking_info.disappeared = False
                
                obj.object_id = obj_id
                obj.tracked = True
        
        except Exception as e:
            logger.error(f"Object update failed: {e}")
    
    def detect_specific_objects(self, image_path: str, target_classes: List[str]) -> List[DetectedObject]:
        """
        Detect only specific object classes
        
        Args:
            image_path: Path to the screenshot image
            target_classes: List of target class names
            
        Returns:
            List of detected objects of target classes
        """
        result = self.detect_objects(image_path)
        return [obj for obj in result.detected_objects if obj.class_name in target_classes]
    
    def get_object_movement(self, object_id: int) -> Optional[List[Tuple[float, float]]]:
        """
        Get movement history of a tracked object
        
        Args:
            object_id: ID of the tracked object
            
        Returns:
            List of positions or None if object not found
        """
        if object_id in self.tracked_objects:
            return self.tracked_objects[object_id].positions
        return None
    
    def get_tracking_statistics(self) -> Dict[str, Any]:
        """Get tracking statistics"""
        try:
            total_tracked = len(self.tracked_objects)
            active_tracked = sum(1 for obj in self.tracked_objects.values() if not obj.disappeared)
            disappeared_tracked = sum(1 for obj in self.tracked_objects.values() if obj.disappeared)
            
            # Calculate average tracking duration
            current_time = time.time()
            tracking_durations = [current_time - obj.first_seen for obj in self.tracked_objects.values() if not obj.disappeared]
            avg_duration = np.mean(tracking_durations) if tracking_durations else 0.0
            
            return {
                "total_objects_tracked": total_tracked,
                "active_tracks": active_tracked,
                "disappeared_tracks": disappeared_tracked,
                "average_tracking_duration": float(avg_duration),
                "next_object_id": self.next_object_id
            }
        
        except Exception as e:
            logger.error(f"Statistics calculation failed: {e}")
            return {}

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
    
    detector = ObjectDetector()
    
    # Full detection
    result = detector.detect_objects(image_path)
    
    print(json.dumps({
        "objects_found": len(result.detected_objects),
        "model_confidence": result.model_confidence,
        "processing_time": result.processing_time,
        "frame_number": result.frame_number
    }, indent=2))
    
    # Show first few objects
    print("\nDetected Objects:")
    for obj in result.detected_objects[:5]:
        print(f"  - {obj.class_name} (conf: {obj.confidence:.2f}) at ({obj.x}, {obj.y})")
    
    # Tracking statistics
    stats = detector.get_tracking_statistics()
    if stats:
        print(f"\nTracking Statistics: {json.dumps(stats, indent=2)}")