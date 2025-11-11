"""
Test Screenshot Generator for Visual Verification Pipeline
Creates synthetic test screenshots to validate the visual analysis system.
"""

import cv2
import numpy as np
import json
from pathlib import Path
from typing import Dict, List, Any, Tuple
import random

class TestScreenshotGenerator:
    """Generates synthetic test screenshots for validation"""
    
    def __init__(self, output_dir: str = "test_screenshots"):
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(exist_ok=True)
        
        # Test configurations
        self.resolutions = [(1920, 1080), (1280, 720), (2560, 1440)]
        self.colors = {
            "ui_blue": (41, 128, 185),
            "ui_green": (39, 174, 96),
            "ui_red": (231, 76, 60),
            "ui_yellow": (241, 196, 15),
            "space_black": (10, 10, 20),
            "planet_brown": (139, 69, 19),
            "ship_gray": (128, 128, 128)
        }
        
        print(f"Test Screenshot Generator initialized. Output: {self.output_dir}")
    
    def generate_all_test_screenshots(self) -> List[Dict[str, Any]]:
        """Generate all test screenshots"""
        print("Generating test screenshots...")
        
        test_screenshots = []
        
        # Generate different types of test screenshots
        test_screenshots.append(self.generate_ui_screenshot())
        test_screenshots.append(self.generate_space_scene())
        test_screenshots.append(self.generate_text_overlay())
        test_screenshots.append(self.generate_mixed_scene())
        test_screenshots.append(self.generate_artifact_test())
        test_screenshots.append(self.generate_low_contrast())
        test_screenshots.append(self.generate_high_contrast())
        test_screenshots.append(self.generate_color_test())
        
        print(f"Generated {len(test_screenshots)} test screenshots")
        return test_screenshots
    
    def generate_ui_screenshot(self) -> Dict[str, Any]:
        """Generate UI-heavy screenshot"""
        width, height = 1920, 1080
        image = np.zeros((height, width, 3), dtype=np.uint8)
        image[:] = self.colors["space_black"]
        
        # Health bar (top-left)
        self._draw_health_bar(image, 20, 20, 200, 30, 0.75)
        
        # Shield bar
        self._draw_shield_bar(image, 20, 60, 200, 30, 0.60)
        
        # Minimap (bottom-right)
        self._draw_minimap(image, width - 220, height - 220, 200, 200)
        
        # Mission objectives (top-right)
        self._draw_mission_objectives(image, width - 300, 20, 280, 150)
        
        # Inventory grid (bottom-left)
        self._draw_inventory_grid(image, 20, height - 200, 300, 180)
        
        # Crosshair (center)
        self._draw_crosshair(image, width // 2, height // 2, 40)
        
        # Button labels
        self._draw_text(image, "MENU", 50, height - 50, self.colors["ui_yellow"], 1.0)
        self._draw_text(image, "OPTIONS", 150, height - 50, self.colors["ui_yellow"], 1.0)
        self._draw_text(image, "QUIT", 280, height - 50, self.colors["ui_red"], 1.0)
        
        # Save image
        filepath = self.output_dir / "ui_test.png"
        cv2.imwrite(str(filepath), image)
        
        expected_state = {
            "ui_elements": [
                {"type": "health_bar", "position": {"x": 20, "y": 20}},
                {"type": "shield_bar", "position": {"x": 20, "y": 60}},
                {"type": "minimap", "position": {"x": width - 220, "y": height - 220}},
                {"type": "mission_objectives", "position": {"x": width - 300, "y": 20}}
            ],
            "text_elements": [
                {"text": "MENU", "location": {"x": 50, "y": height - 50}},
                {"text": "OPTIONS", "location": {"x": 150, "y": height - 50}},
                {"text": "QUIT", "location": {"x": 280, "y": height - 50}}
            ],
            "object_counts": {}
        }
        
        return {
            "filepath": str(filepath),
            "type": "ui_heavy",
            "description": "UI elements test with health bars, minimap, and text",
            "expected_state": expected_state
        }
    
    def generate_space_scene(self) -> Dict[str, Any]:
        """Generate space scene with ships and planets"""
        width, height = 1920, 1080
        image = np.zeros((height, width, 3), dtype=np.uint8)
        image[:] = self.colors["space_black"]
        
        # Add stars
        self._draw_stars(image, 200)
        
        # Add planet (large circle)
        planet_x, planet_y = width - 400, height // 2
        cv2.circle(image, (planet_x, planet_y), 150, self.colors["planet_brown"], -1)
        cv2.circle(image, (planet_x - 50, planet_y - 50), 30, (160, 82, 45), -1)  # Crater
        
        # Add player ship (center)
        self._draw_ship(image, width // 2, height // 2, self.colors["ui_green"], "player")
        
        # Add enemy ships
        self._draw_ship(image, 300, 200, self.colors["ui_red"], "enemy")
        self._draw_ship(image, 600, 400, self.colors["ui_red"], "enemy")
        self._draw_ship(image, 400, 600, self.colors["ui_red"], "enemy")
        
        # Add asteroids
        for i in range(5):
            x = random.randint(100, width - 100)
            y = random.randint(100, height - 100)
            self._draw_asteroid(image, x, y, random.randint(20, 40))
        
        # Save image
        filepath = self.output_dir / "space_scene.png"
        cv2.imwrite(str(filepath), image)
        
        expected_state = {
            "ui_elements": [],
            "text_elements": [],
            "object_counts": {
                "ship_player": 1,
                "ship_enemy": 3,
                "asteroid": 5,
                "planet_terrestrial": 1
            }
        }
        
        return {
            "filepath": str(filepath),
            "type": "space_scene",
            "description": "Space scene with ships, planets, and asteroids",
            "expected_state": expected_state
        }
    
    def generate_text_overlay(self) -> Dict[str, Any]:
        """Generate screenshot with various text elements"""
        width, height = 1920, 1080
        image = np.zeros((height, width, 3), dtype=np.uint8)
        image[:] = (30, 30, 40)
        
        # Mission objectives
        self._draw_text(image, "MISSION OBJECTIVES", 100, 100, self.colors["ui_yellow"], 1.5)
        self._draw_text(image, "1. Destroy enemy base", 120, 150, (255, 255, 255), 1.0)
        self._draw_text(image, "2. Collect 3 artifacts", 120, 190, (255, 255, 255), 1.0)
        self._draw_text(image, "3. Return to station", 120, 230, (255, 255, 255), 1.0)
        
        # Status text
        self._draw_text(image, "HEALTH: 75/100", 100, 300, self.colors["ui_green"], 1.2)
        self._draw_text(image, "SHIELD: 50/75", 100, 340, self.colors["ui_blue"], 1.2)
        self._draw_text(image, "CREDITS: 5,250", 100, 380, self.colors["ui_yellow"], 1.2)
        
        # Coordinates
        self._draw_text(image, "X: 1250 Y: 840 Z: 320", width - 400, 100, (200, 200, 200), 1.0)
        
        # Inventory items
        self._draw_text(image, "INVENTORY", 100, 500, self.colors["ui_yellow"], 1.3)
        self._draw_text(image, "• Laser Cannon x2", 120, 550, (255, 255, 255), 1.0)
        self._draw_text(image, "• Shield Booster", 120, 590, (255, 255, 255), 1.0)
        self._draw_text(image, "• Repair Kit x3", 120, 630, (255, 255, 255), 1.0)
        
        # Button labels
        self._draw_text(image, "[F] INTERACT", width - 200, height - 100, self.colors["ui_green"], 1.0)
        self._draw_text(image, "[ESC] MENU", width - 200, height - 60, self.colors["ui_yellow"], 1.0)
        
        # Save image
        filepath = self.output_dir / "text_overlay.png"
        cv2.imwrite(str(filepath), image)
        
        expected_state = {
            "ui_elements": [],
            "text_elements": [
                {"text": "MISSION OBJECTIVES", "location": {"x": 100, "y": 100}},
                {"text": "HEALTH: 75/100", "location": {"x": 100, "y": 300}},
                {"text": "SHIELD: 50/75", "location": {"x": 100, "y": 340}},
                {"text": "CREDITS: 5,250", "location": {"x": 100, "y": 380}},
                {"text": "INVENTORY", "location": {"x": 100, "y": 500}}
            ],
            "object_counts": {}
        }
        
        return {
            "filepath": str(filepath),
            "type": "text_heavy",
            "description": "Text overlay test with mission objectives, status, and inventory",
            "expected_state": expected_state
        }
    
    def generate_mixed_scene(self) -> Dict[str, Any]:
        """Generate mixed scene with UI, objects, and text"""
        width, height = 1920, 1080
        image = np.zeros((height, width, 3), dtype=np.uint8)
        image[:] = self.colors["space_black"]
        
        # Add stars
        self._draw_stars(image, 150)
        
        # Add planet
        cv2.circle(image, (width - 300, 200), 100, self.colors["planet_brown"], -1)
        
        # Add ships
        self._draw_ship(image, width // 2, height // 2, self.colors["ui_green"], "player")
        self._draw_ship(image, 400, 300, self.colors["ui_red"], "enemy")
        
        # Add UI elements
        self._draw_health_bar(image, 20, 20, 200, 30, 0.85)
        self._draw_minimap(image, width - 220, height - 220, 200, 200)
        
        # Add text
        self._draw_text(image, "TARGET: Enemy Fighter", width // 2 - 150, 50, self.colors["ui_yellow"], 1.2)
        self._draw_text(image, "DISTANCE: 2.5km", width // 2 - 100, height - 50, (255, 255, 255), 1.0)
        
        # Save image
        filepath = self.output_dir / "mixed_scene.png"
        cv2.imwrite(str(filepath), image)
        
        expected_state = {
            "ui_elements": [
                {"type": "health_bar", "position": {"x": 20, "y": 20}},
                {"type": "minimap", "position": {"x": width - 220, "y": height - 220}}
            ],
            "text_elements": [
                {"text": "TARGET: Enemy Fighter", "location": {"x": width // 2 - 150, "y": 50}},
                {"text": "DISTANCE: 2.5km", "location": {"x": width // 2 - 100, "y": height - 50}}
            ],
            "object_counts": {
                "ship_player": 1,
                "ship_enemy": 1,
                "planet_terrestrial": 1
            }
        }
        
        return {
            "filepath": str(filepath),
            "type": "mixed",
            "description": "Mixed scene with UI, objects, and text elements",
            "expected_state": expected_state
        }
    
    def generate_artifact_test(self) -> Dict[str, Any]:
        """Generate screenshot with visual artifacts for testing"""
        width, height = 1920, 1080
        image = np.zeros((height, width, 3), dtype=np.uint8)
        image[:] = self.colors["space_black"]
        
        # Add normal content
        self._draw_stars(image, 100)
        self._draw_ship(image, width // 2, height // 2, self.colors["ui_green"], "player")
        
        # Add screen tearing effect (horizontal lines)
        for y in range(0, height, 20):
            if y % 40 == 0:
                # Shift every other line to simulate tearing
                line = image[y:y+1, :].copy()
                image[y:y+1, 2:] = line[:, :-2]
        
        # Add color anomalies (bright spots)
        for _ in range(5):
            x = random.randint(100, width - 100)
            y = random.randint(100, height - 100)
            cv2.circle(image, (x, y), 30, (0, 255, 255), -1)  # Bright yellow circles
        
        # Save image
        filepath = self.output_dir / "artifact_test.png"
        cv2.imwrite(str(filepath), image)
        
        return {
            "filepath": str(filepath),
            "type": "artifact_test",
            "description": "Screenshot with visual artifacts for artifact detection testing",
            "expected_state": {}
        }
    
    def generate_low_contrast(self) -> Dict[str, Any]:
        """Generate low contrast screenshot"""
        width, height = 1920, 1080
        image = np.zeros((height, width, 3), dtype=np.uint8)
        image[:] = (50, 50, 55)  # Very dark gray
        
        # Add low contrast elements
        cv2.rectangle(image, (100, 100), (300, 200), (60, 60, 65), -1)
        self._draw_text(image, "Low Contrast Text", 120, 160, (70, 70, 75), 1.0)
        
        # Save image
        filepath = self.output_dir / "low_contrast.png"
        cv2.imwrite(str(filepath), image)
        
        return {
            "filepath": str(filepath),
            "type": "low_contrast",
            "description": "Low contrast screenshot for OCR testing",
            "expected_state": {}
        }
    
    def generate_high_contrast(self) -> Dict[str, Any]:
        """Generate high contrast screenshot"""
        width, height = 1920, 1080
        image = np.zeros((height, width, 3), dtype=np.uint8)
        image[:] = (255, 255, 255)  # White background
        
        # Add high contrast elements
        cv2.rectangle(image, (100, 100), (400, 200), (0, 0, 0), -1)
        self._draw_text(image, "HIGH CONTRAST", 120, 160, (255, 255, 255), 1.5)
        
        # Save image
        filepath = self.output_dir / "high_contrast.png"
        cv2.imwrite(str(filepath), image)
        
        return {
            "filepath": str(filepath),
            "type": "high_contrast",
            "description": "High contrast screenshot for OCR testing",
            "expected_state": {}
        }
    
    def generate_color_test(self) -> Dict[str, Any]:
        """Generate color test screenshot"""
        width, height = 1920, 1080
        image = np.zeros((height, width, 3), dtype=np.uint8)
        
        # Create color gradient
        for y in range(height):
            for x in range(width):
                r = int((x / width) * 255)
                g = int((y / height) * 255)
                b = int(((x + y) / (width + height)) * 255)
                image[y, x] = (b, g, r)
        
        # Add UI elements
        self._draw_text(image, "COLOR TEST", width // 2 - 100, 50, (255, 255, 255), 2.0)
        
        # Save image
        filepath = self.output_dir / "color_test.png"
        cv2.imwrite(str(filepath), image)
        
        return {
            "filepath": str(filepath),
            "type": "color_test",
            "description": "Color test screenshot for color analysis",
            "expected_state": {}
        }
    
    def _draw_health_bar(self, image: np.ndarray, x: int, y: int, width: int, height: int, health_percent: float):
        """Draw a health bar"""
        # Background
        cv2.rectangle(image, (x, y), (x + width, y + height), (50, 50, 50), -1)
        # Health fill
        fill_width = int(width * health_percent)
        cv2.rectangle(image, (x, y), (x + fill_width, y + height), self.colors["ui_green"], -1)
        # Border
        cv2.rectangle(image, (x, y), (x + width, y + height), (255, 255, 255), 2)
    
    def _draw_shield_bar(self, image: np.ndarray, x: int, y: int, width: int, height: int, shield_percent: float):
        """Draw a shield bar"""
        # Background
        cv2.rectangle(image, (x, y), (x + width, y + height), (50, 50, 50), -1)
        # Shield fill
        fill_width = int(width * shield_percent)
        cv2.rectangle(image, (x, y), (x + fill_width, y + height), self.colors["ui_blue"], -1)
        # Border
        cv2.rectangle(image, (x, y), (x + width, y + height), (255, 255, 255), 2)
    
    def _draw_minimap(self, image: np.ndarray, x: int, y: int, width: int, height: int):
        """Draw a minimap"""
        # Background
        cv2.rectangle(image, (x, y), (x + width, y + height), (20, 20, 30), -1)
        # Border
        cv2.rectangle(image, (x, y), (x + width, y + height), (255, 255, 255), 2)
        
        # Add minimap elements
        center_x, center_y = x + width // 2, y + height // 2
        
        # Player position (center)
        cv2.circle(image, (center_x, center_y), 3, self.colors["ui_green"], -1)
        
        # Enemy positions
        cv2.circle(image, (center_x + 30, center_y - 20), 2, self.colors["ui_red"], -1)
        cv2.circle(image, (center_x - 40, center_y + 30), 2, self.colors["ui_red"], -1)
        
        # Waypoint
        cv2.circle(image, (center_x + 60, center_y - 50), 3, self.colors["ui_yellow"], -1)
    
    def _draw_mission_objectives(self, image: np.ndarray, x: int, y: int, width: int, height: int):
        """Draw mission objectives panel"""
        # Background
        cv2.rectangle(image, (x, y), (x + width, y + height), (30, 30, 40, 200), -1)
        # Border
        cv2.rectangle(image, (x, y), (x + width, y + height), (255, 255, 255), 1)
        
        # Title
        self._draw_text(image, "MISSION", x + 10, y + 30, self.colors["ui_yellow"], 1.0)
        
        # Objectives
        objectives = [
            "Destroy enemy base",
            "Collect artifacts",
            "Return to station"
        ]
        
        for i, objective in enumerate(objectives):
            color = self.colors["ui_green"] if i == 0 else (150, 150, 150)
            self._draw_text(image, f"{'✓' if i > 0 else '•'} {objective}", x + 15, y + 60 + i * 25, color, 0.8)
    
    def _draw_inventory_grid(self, image: np.ndarray, x: int, y: int, width: int, height: int):
        """Draw inventory grid"""
        # Background
        cv2.rectangle(image, (x, y), (x + width, y + height), (40, 40, 50), -1)
        # Border
        cv2.rectangle(image, (x, y), (x + width, y + height), (255, 255, 255), 2)
        
        # Grid cells
        cell_size = 50
        cols = width // cell_size
        rows = height // cell_size
        
        for row in range(rows):
            for col in range(cols):
                cell_x = x + col * cell_size
                cell_y = y + row * cell_size
                
                # Cell border
                cv2.rectangle(image, (cell_x, cell_y), (cell_x + cell_size, cell_y + cell_size), (100, 100, 100), 1)
                
                # Add items to some cells
                if random.random() > 0.6:
                    # Draw simple item icon
                    cv2.circle(image, (cell_x + cell_size // 2, cell_y + cell_size // 2), 15, (200, 200, 50), -1)
    
    def _draw_crosshair(self, image: np.ndarray, x: int, y: int, size: int):
        """Draw crosshair"""
        color = (0, 255, 0)
        thickness = 2
        
        # Horizontal line
        cv2.line(image, (x - size, y), (x + size, y), color, thickness)
        # Vertical line
        cv2.line(image, (x, y - size), (x, y + size), color, thickness)
        # Center dot
        cv2.circle(image, (x, y), 2, color, -1)
    
    def _draw_ship(self, image: np.ndarray, x: int, y: int, color: Tuple[int, int, int], ship_type: str = "player"):
        """Draw a simple ship shape"""
        # Main body
        points = np.array([
            [x, y - 20],
            [x - 15, y + 20],
            [x + 15, y + 20]
        ], np.int32)
        cv2.fillPoly(image, [points], color)
        
        # Wings
        cv2.line(image, (x - 20, y), (x - 30, y + 10), color, 3)
        cv2.line(image, (x + 20, y), (x + 30, y + 10), color, 3)
        
        # Engine glow
        engine_color = (0, 200, 255) if ship_type == "player" else (255, 100, 0)
        cv2.circle(image, (x, y + 15), 5, engine_color, -1)
    
    def _draw_asteroid(self, image: np.ndarray, x: int, y: int, size: int):
        """Draw a simple asteroid"""
        # Irregular circle
        points = []
        for angle in range(0, 360, 30):
            radius = size + random.randint(-5, 5)
            rad_angle = np.radians(angle)
            px = int(x + radius * np.cos(rad_angle))
            py = int(y + radius * np.sin(rad_angle))
            points.append([px, py])
        
        points = np.array(points, np.int32)
        cv2.fillPoly(image, [points], (101, 67, 33))
    
    def _draw_stars(self, image: np.ndarray, count: int):
        """Draw random stars"""
        height, width = image.shape[:2]
        for _ in range(count):
            x = random.randint(0, width - 1)
            y = random.randint(0, height - 1)
            brightness = random.randint(150, 255)
            cv2.circle(image, (x, y), random.randint(1, 2), (brightness, brightness, brightness), -1)
    
    def _draw_text(self, image: np.ndarray, text: str, x: int, y: int, 
                   color: Tuple[int, int, int], scale: float = 1.0):
        """Draw text on image"""
        font = cv2.FONT_HERSHEY_SIMPLEX
        thickness = 2 if scale > 1.0 else 1
        
        cv2.putText(image, text, (x, y), font, scale, color, thickness, cv2.LINE_AA)
    
    def _check_vertical_overlap(self, elem1, elem2) -> float:
        """Check vertical overlap between two elements (for text merging test)"""
        y1_min, y1_max = elem1.y, elem1.y + elem1.height
        y2_min, y2_max = elem2.y, elem2.y + elem2.height
        
        overlap_start = max(y1_min, y2_min)
        overlap_end = min(y1_max, y2_max)
        
        if overlap_end <= overlap_start:
            return 0.0
        
        overlap_height = overlap_end - overlap_start
        smaller_height = min(elem1.height, elem2.height)
        
        return overlap_height / smaller_height if smaller_height > 0 else 0.0

# Example usage
if __name__ == "__main__":
    generator = TestScreenshotGenerator()
    test_screenshots = generator.generate_all_test_screenshots()
    
    print("\nGenerated test screenshots:")
    for screenshot in test_screenshots:
        print(f"  - {screenshot['type']}: {screenshot['filepath']}")
        if screenshot['expected_state']:
            print(f"    Expected objects: {screenshot['expected_state'].get('object_counts', {})}")