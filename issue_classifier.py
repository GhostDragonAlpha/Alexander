"""
Issue Classification System for Gameplay Screenshot Analysis
Categorizes detected issues into types and assigns severity levels with suggested fixes.
"""

import json
import time
from typing import Dict, List, Tuple, Optional, Any
from dataclasses import dataclass
from pathlib import Path
import logging

# Import other modules
from visual_analyzer import VisualArtifact, UIElement
from text_recognizer import TextElement
from object_detector import DetectedObject
from state_comparator import StateDiscrepancy

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

@dataclass
class ClassifiedIssue:
    """Represents a classified issue with severity and suggested fix"""
    issue_id: str
    issue_type: str
    severity: str
    category: str
    description: str
    location: Tuple[int, int]
    confidence: float
    suggested_fix: str
    affected_components: List[str]
    frequency: int
    first_seen: float
    last_seen: float

@dataclass
class IssueHistory:
    """Tracks issue history and frequency"""
    issue_type: str
    category: str
    occurrences: int
    first_seen: float
    last_seen: float
    severity_trend: List[Tuple[float, str]]  # (timestamp, severity)
    affected_screenshots: List[str]

@dataclass
class ClassificationResult:
    """Complete issue classification results"""
    classified_issues: List[ClassifiedIssue]
    issue_summary: Dict[str, Any]
    processing_time: float
    critical_count: int
    high_count: int
    medium_count: int
    low_count: int

class IssueClassifier:
    """Issue classification system for game screenshot analysis"""
    
    def __init__(self, config_path: str = "visual_analysis_config.json"):
        """Initialize the issue classifier with configuration"""
        self.config = self._load_config(config_path)
        self.issue_history: Dict[str, IssueHistory] = {}
        self.classification_rules = self.config.get("issue_classifier", {})
        
        self._load_issue_patterns()
    
    def _load_config(self, config_path: str) -> Dict[str, Any]:
        """Load configuration from JSON file"""
        try:
            with open(config_path, 'r') as f:
                return json.load(f)
        except Exception as e:
            logger.error(f"Failed to load config: {e}")
            return {}
    
    def _load_issue_patterns(self):
        """Load issue classification patterns and rules"""
        try:
            self.severity_levels = self.classification_rules.get("severity_levels", {})
            self.issue_categories = self.classification_rules.get("issue_categories", {})
            
            # Compile issue patterns for faster matching
            self.issue_patterns = {
                "visual_bug": {
                    "patterns": ["tearing", "flicker", "artifact", "render", "texture", "glitch"],
                    "keywords": ["screen tearing", "flickering", "rendering issue", "texture problem", "visual glitch"]
                },
                "ui_problem": {
                    "patterns": ["ui", "interface", "menu", "button", "text", "alignment"],
                    "keywords": ["misaligned", "missing text", "wrong position", "scaling issue", "ui element"]
                },
                "gameplay_error": {
                    "patterns": ["state", "object", "count", "value", "mission", "inventory"],
                    "keywords": ["incorrect state", "missing object", "wrong count", "invalid value", "state mismatch"]
                },
                "performance_issue": {
                    "patterns": ["fps", "performance", "loading", "stutter", "memory", "lag"],
                    "keywords": ["low fps", "performance issue", "loading delay", "stuttering", "memory spike"]
                }
            }
            
            logger.info(f"Loaded {len(self.issue_categories)} issue categories")
            
        except Exception as e:
            logger.error(f"Issue pattern loading failed: {e}")
    
    def classify_issues(self, analysis_results: Dict[str, Any]) -> ClassificationResult:
        """
        Classify issues from analysis results
        
        Args:
            analysis_results: Dictionary containing results from all analysis modules
            
        Returns:
            ClassificationResult containing classified issues
        """
        start_time = time.time()
        
        try:
            classified_issues = []
            
            # Classify visual artifacts
            if "visual_artifacts" in analysis_results:
                visual_issues = self._classify_visual_artifacts(
                    analysis_results["visual_artifacts"]
                )
                classified_issues.extend(visual_issues)
            
            # Classify text issues
            if "text_elements" in analysis_results:
                text_issues = self._classify_text_issues(
                    analysis_results["text_elements"]
                )
                classified_issues.extend(text_issues)
            
            # Classify object detection issues
            if "object_discrepancies" in analysis_results:
                object_issues = self._classify_object_issues(
                    analysis_results["object_discrepancies"]
                )
                classified_issues.extend(object_issues)
            
            # Classify state comparison issues
            if "state_discrepancies" in analysis_results:
                state_issues = self._classify_state_issues(
                    analysis_results["state_discrepancies"]
                )
                classified_issues.extend(state_issues)
            
            # Update issue history
            self._update_issue_history(classified_issues)
            
            # Generate summary
            issue_summary = self._generate_issue_summary(classified_issues)
            
            # Count issues by severity
            severity_counts = self._count_severity_levels(classified_issues)
            
            processing_time = time.time() - start_time
            
            result = ClassificationResult(
                classified_issues=classified_issues,
                issue_summary=issue_summary,
                processing_time=processing_time,
                critical_count=severity_counts["critical"],
                high_count=severity_counts["high"],
                medium_count=severity_counts["medium"],
                low_count=severity_counts["low"]
            )
            
            logger.info(f"Issue classification completed in {processing_time:.3f}s, found {len(classified_issues)} issues")
            
            return result
            
        except Exception as e:
            logger.error(f"Issue classification failed: {e}")
            return ClassificationResult(
                classified_issues=[],
                issue_summary={},
                processing_time=time.time() - start_time,
                critical_count=0,
                high_count=0,
                medium_count=0,
                low_count=0
            )
    
    def _classify_visual_artifacts(self, artifacts: List[VisualArtifact]) -> List[ClassifiedIssue]:
        """Classify visual artifacts into specific issue types"""
        classified = []
        
        try:
            for artifact in artifacts:
                # Determine issue type and category
                issue_type = self._determine_visual_issue_type(artifact)
                category = "visual_bug"
                
                # Determine severity
                severity = self._calculate_severity(artifact.severity, category)
                
                # Generate suggested fix
                suggested_fix = self._get_suggested_fix(issue_type, category)
                
                # Create issue ID
                issue_id = f"visual_{issue_type}_{int(time.time())}_{len(classified)}"
                
                classified.append(ClassifiedIssue(
                    issue_id=issue_id,
                    issue_type=issue_type,
                    severity=severity,
                    category=category,
                    description=artifact.description,
                    location=artifact.location,
                    confidence=artifact.confidence,
                    suggested_fix=suggested_fix,
                    affected_components=self._get_affected_components(issue_type),
                    frequency=1,
                    first_seen=time.time(),
                    last_seen=time.time()
                ))
        
        except Exception as e:
            logger.error(f"Visual artifact classification failed: {e}")
        
        return classified
    
    def _determine_visual_issue_type(self, artifact: VisualArtifact) -> str:
        """Determine specific issue type for visual artifact"""
        try:
            artifact_type = artifact.artifact_type.lower()
            
            if "tearing" in artifact_type:
                return "screen_tearing"
            elif "flicker" in artifact_type:
                return "screen_flickering"
            elif "color" in artifact_type:
                return "color_rendering_issue"
            elif "blur" in artifact_type:
                return "image_blurriness"
            elif "sharp" in artifact_type:
                return "oversharpening_artifact"
            else:
                return "general_visual_artifact"
        
        except Exception as e:
            logger.warning(f"Visual issue type determination failed: {e}")
            return "unknown_visual_issue"
    
    def _classify_text_issues(self, text_elements: List[TextElement]) -> List[ClassifiedIssue]:
        """Classify text-related issues"""
        classified = []
        
        try:
            # Check for low confidence text recognition
            min_confidence_threshold = self.tesseract_config.get("min_confidence", 60)
            
            for elem in text_elements:
                if elem.confidence < min_confidence_threshold:
                    issue_id = f"text_low_confidence_{int(time.time())}_{len(classified)}"
                    
                    classified.append(ClassifiedIssue(
                        issue_id=issue_id,
                        issue_type="low_ocr_confidence",
                        severity="medium",
                        category="ui_problem",
                        description=f"Low OCR confidence ({elem.confidence:.1f}%) for text: '{elem.text}'",
                        location=(elem.x, elem.y),
                        confidence=1.0 - (elem.confidence / 100.0),
                        suggested_fix="Improve image quality or adjust OCR preprocessing parameters",
                        affected_components=["text_recognition", "ui_detection"],
                        frequency=1,
                        first_seen=time.time(),
                        last_seen=time.time()
                    ))
            
            # Check for potential text cutoff
            if len(text_elements) > 1:
                classified.extend(self._detect_text_cutoff_issues(text_elements))
        
        except Exception as e:
            logger.error(f"Text issue classification failed: {e}")
        
        return classified
    
    def _detect_text_cutoff_issues(self, text_elements: List[TextElement]) -> List[ClassifiedIssue]:
        """Detect text that might be cut off or incomplete"""
        issues = []
        
        try:
            # Sort by position to find adjacent text elements
            sorted_elements = sorted(text_elements, key=lambda e: (e.y, e.x))
            
            for i in range(len(sorted_elements) - 1):
                elem1 = sorted_elements[i]
                elem2 = sorted_elements[i + 1]
                
                # Check if elements are close horizontally and on same line
                horizontal_gap = elem2.x - (elem1.x + elem1.width)
                vertical_overlap = self._check_vertical_overlap(elem1, elem2)
                
                # If close and same line, might be cut text
                if horizontal_gap < 10 and vertical_overlap > 0.7:
                    # Check if text ends abruptly
                    if elem1.text.endswith(("...", "-", "…")) or len(elem1.text) < 3:
                        issue_id = f"text_cutoff_{int(time.time())}_{len(issues)}"
                        
                        issues.append(ClassifiedIssue(
                            issue_id=issue_id,
                            issue_type="possible_text_cutoff",
                            severity="low",
                            category="ui_problem",
                            description=f"Text may be cut off: '{elem1.text}'",
                            location=(elem1.x, elem1.y),
                            confidence=0.6,
                            suggested_fix="Check UI layout and text bounding boxes",
                            affected_components=["ui_layout", "text_rendering"],
                            frequency=1,
                            first_seen=time.time(),
                            last_seen=time.time()
                        ))
        
        except Exception as e:
            logger.warning(f"Text cutoff detection failed: {e}")
        
        return issues
    
    def _classify_object_issues(self, object_discrepancies: List[StateDiscrepancy]) -> List[ClassifiedIssue]:
        """Classify object detection issues"""
        classified = []
        
        try:
            for discrepancy in object_discrepancies:
                if discrepancy.element_type == "object":
                    issue_type = self._determine_object_issue_type(discrepancy)
                    category = "gameplay_error"
                    
                    severity = self._calculate_severity(discrepancy.confidence, category)
                    
                    suggested_fix = self._get_suggested_fix(issue_type, category)
                    
                    issue_id = f"object_{issue_type}_{int(time.time())}_{len(classified)}"
                    
                    classified.append(ClassifiedIssue(
                        issue_id=issue_id,
                        issue_type=issue_type,
                        severity=severity,
                        category=category,
                        description=discrepancy.description,
                        location=discrepancy.location,
                        confidence=discrepancy.confidence,
                        suggested_fix=suggested_fix,
                        affected_components=self._get_affected_components(issue_type),
                        frequency=1,
                        first_seen=time.time(),
                        last_seen=time.time()
                    ))
        
        except Exception as e:
            logger.error(f"Object issue classification failed: {e}")
        
        return classified
    
    def _determine_object_issue_type(self, discrepancy: StateDiscrepancy) -> str:
        """Determine specific issue type for object discrepancy"""
        try:
            discrepancy_type = discrepancy.discrepancy_type
            
            if "missing" in discrepancy_type:
                return "missing_game_object"
            elif "count" in discrepancy_type:
                return "incorrect_object_count"
            elif "position" in discrepancy_type:
                return "object_position_error"
            else:
                return "general_object_issue"
        
        except Exception as e:
            logger.warning(f"Object issue type determination failed: {e}")
            return "unknown_object_issue"
    
    def _classify_state_issues(self, state_discrepancies: List[StateDiscrepancy]) -> List[ClassifiedIssue]:
        """Classify state comparison issues"""
        classified = []
        
        try:
            for discrepancy in state_discrepancies:
                # Determine category based on element type
                if discrepancy.element_type == "ui":
                    category = "ui_problem"
                elif discrepancy.element_type == "text":
                    category = "ui_problem"
                elif discrepancy.element_type == "object":
                    category = "gameplay_error"
                elif discrepancy.element_type == "critical_ui":
                    category = "ui_problem"
                else:
                    category = "gameplay_error"
                
                issue_type = self._determine_state_issue_type(discrepancy)
                severity = discrepancy.severity  # Use pre-determined severity
                
                suggested_fix = self._get_suggested_fix(issue_type, category)
                
                issue_id = f"state_{issue_type}_{int(time.time())}_{len(classified)}"
                
                classified.append(ClassifiedIssue(
                    issue_id=issue_id,
                    issue_type=issue_type,
                    severity=severity,
                    category=category,
                    description=discrepancy.description,
                    location=discrepancy.location,
                    confidence=discrepancy.confidence,
                    suggested_fix=suggested_fix,
                    affected_components=self._get_affected_components(issue_type),
                    frequency=1,
                    first_seen=time.time(),
                    last_seen=time.time()
                ))
        
        except Exception as e:
            logger.error(f"State issue classification failed: {e}")
        
        return classified
    
    def _determine_state_issue_type(self, discrepancy: StateDiscrepancy) -> str:
        """Determine specific issue type for state discrepancy"""
        try:
            discrepancy_type = discrepancy.discrepancy_type
            
            type_mapping = {
                "missing_ui_element": "ui_element_missing",
                "ui_position_mismatch": "ui_layout_error",
                "missing_text": "text_content_missing",
                "text_position_mismatch": "text_layout_error",
                "object_count_mismatch": "game_state_inconsistent",
                "missing_critical_element": "critical_ui_missing"
            }
            
            return type_mapping.get(discrepancy_type, "general_state_error")
        
        except Exception as e:
            logger.warning(f"State issue type determination failed: {e}")
            return "unknown_state_error"
    
    def _calculate_severity(self, confidence: float, category: str) -> str:
        """Calculate severity level based on confidence and category"""
        try:
            severity_config = self.severity_levels
            
            # Get thresholds for the category
            category_config = self.issue_categories.get(category, {})
            base_threshold = category_config.get("weight", 1.0)
            
            # Adjust confidence by category weight
            weighted_confidence = confidence * base_threshold
            
            # Determine severity based on thresholds
            if weighted_confidence >= severity_config.get("critical", {}).get("threshold", 0.9):
                return "critical"
            elif weighted_confidence >= severity_config.get("high", {}).get("threshold", 0.7):
                return "high"
            elif weighted_confidence >= severity_config.get("medium", {}).get("threshold", 0.5):
                return "medium"
            else:
                return "low"
        
        except Exception as e:
            logger.warning(f"Severity calculation failed: {e}")
            return "medium"
    
    def _get_suggested_fix(self, issue_type: str, category: str) -> str:
        """Get suggested fix based on issue type and category"""
        try:
            fix_suggestions = {
                "visual_bug": {
                    "screen_tearing": "Enable V-Sync or triple buffering in graphics settings",
                    "screen_flickering": "Check graphics drivers and disable HDR if enabled",
                    "color_rendering_issue": "Verify color depth settings and monitor calibration",
                    "image_blurriness": "Check render scale and anti-aliasing settings",
                    "oversharpening_artifact": "Reduce sharpness settings in graphics options",
                    "general_visual_artifact": "Update graphics drivers and verify game files"
                },
                "ui_problem": {
                    "low_ocr_confidence": "Improve screenshot quality or adjust OCR preprocessing",
                    "possible_text_cutoff": "Review UI layout and text bounding boxes",
                    "ui_element_missing": "Check UI element visibility and rendering",
                    "ui_layout_error": "Verify UI scaling and resolution settings",
                    "text_content_missing": "Check font rendering and text visibility",
                    "text_layout_error": "Review UI positioning and alignment",
                    "critical_ui_missing": "Verify core UI components are properly initialized"
                },
                "gameplay_error": {
                    "missing_game_object": "Check object spawning and visibility logic",
                    "incorrect_object_count": "Verify object management and cleanup systems",
                    "object_position_error": "Review physics and transformation systems",
                    "game_state_inconsistent": "Check state synchronization and replication",
                    "general_object_issue": "Verify object lifecycle management",
                    "general_state_error": "Review game state management system"
                },
                "performance_issue": {
                    "low_fps": "Optimize graphics settings and reduce scene complexity",
                    "stuttering": "Check for memory leaks and optimize asset loading",
                    "loading_delay": "Optimize asset streaming and preload critical resources",
                    "memory_spike": "Profile memory usage and implement cleanup systems"
                }
            }
            
            category_fixes = fix_suggestions.get(category, {})
            return category_fixes.get(issue_type, "Investigate and debug the issue further")
        
        except Exception as e:
            logger.warning(f"Suggested fix retrieval failed: {e}")
            return "Investigate the issue and implement appropriate fix"
    
    def _get_affected_components(self, issue_type: str) -> List[str]:
        """Get list of affected components for an issue type"""
        try:
            component_mapping = {
                "screen_tearing": ["graphics", "rendering", "display"],
                "screen_flickering": ["graphics", "rendering", "display"],
                "color_rendering_issue": ["graphics", "rendering", "materials"],
                "image_blurriness": ["graphics", "rendering", "post_processing"],
                "oversharpening_artifact": ["graphics", "rendering", "post_processing"],
                "low_ocr_confidence": ["text_recognition", "image_processing"],
                "possible_text_cutoff": ["ui", "text_rendering", "layout"],
                "ui_element_missing": ["ui", "rendering", "game_state"],
                "ui_layout_error": ["ui", "layout", "scaling"],
                "text_content_missing": ["ui", "text_rendering", "fonts"],
                "text_layout_error": ["ui", "layout", "text_rendering"],
                "critical_ui_missing": ["ui", "game_state", "initialization"],
                "missing_game_object": ["gameplay", "spawning", "visibility"],
                "incorrect_object_count": ["gameplay", "object_management", "cleanup"],
                "object_position_error": ["gameplay", "physics", "transform"],
                "game_state_inconsistent": ["gameplay", "state_management", "networking"],
                "low_fps": ["performance", "graphics", "optimization"],
                "stuttering": ["performance", "memory", "asset_loading"],
                "loading_delay": ["performance", "asset_streaming", "io"],
                "memory_spike": ["performance", "memory_management", "cleanup"]
            }
            
            return component_mapping.get(issue_type, ["unknown"])
        
        except Exception as e:
            logger.warning(f"Affected components determination failed: {e}")
            return ["unknown"]
    
    def _update_issue_history(self, issues: List[ClassifiedIssue]):
        """Update issue history with new occurrences"""
        try:
            current_time = time.time()
            
            for issue in issues:
                # Create history key
                history_key = f"{issue.issue_type}_{issue.location[0]}_{issue.location[1]}"
                
                if history_key in self.issue_history:
                    # Update existing history
                    history = self.issue_history[history_key]
                    history.occurrences += 1
                    history.last_seen = current_time
                    history.severity_trend.append((current_time, issue.severity))
                    # Note: We would track affected_screenshots if we had screenshot IDs
                else:
                    # Create new history entry
                    self.issue_history[history_key] = IssueHistory(
                        issue_type=issue.issue_type,
                        category=issue.category,
                        occurrences=1,
                        first_seen=current_time,
                        last_seen=current_time,
                        severity_trend=[(current_time, issue.severity)],
                        affected_screenshots=[]
                    )
                
                # Update issue frequency from history
                issue.frequency = self.issue_history[history_key].occurrences
        
        except Exception as e:
            logger.error(f"Issue history update failed: {e}")
    
    def _generate_issue_summary(self, issues: List[ClassifiedIssue]) -> Dict[str, Any]:
        """Generate summary statistics for classified issues"""
        try:
            summary = {
                "total_issues": len(issues),
                "by_category": {},
                "by_severity": {},
                "by_type": {},
                "most_frequent": [],
                "recommendations": []
            }
            
            # Count by category
            for issue in issues:
                category = issue.category
                summary["by_category"][category] = summary["by_category"].get(category, 0) + 1
                
                severity = issue.severity
                summary["by_severity"][severity] = summary["by_severity"].get(severity, 0) + 1
                
                issue_type = issue.issue_type
                summary["by_type"][issue_type] = summary["by_type"].get(issue_type, 0) + 1
            
            # Find most frequent issues
            sorted_issues = sorted(issues, key=lambda x: x.frequency, reverse=True)
            summary["most_frequent"] = [
                {
                    "issue_type": issue.issue_type,
                    "frequency": issue.frequency,
                    "severity": issue.severity
                }
                for issue in sorted_issues[:5]
            ]
            
            # Generate recommendations
            summary["recommendations"] = self._generate_recommendations(issues)
            
            return summary
        
        except Exception as e:
            logger.error(f"Issue summary generation failed: {e}")
            return {}
    
    def _count_severity_levels(self, issues: List[ClassifiedIssue]) -> Dict[str, int]:
        """Count issues by severity level"""
        try:
            counts = {
                "critical": 0,
                "high": 0,
                "medium": 0,
                "low": 0
            }
            
            for issue in issues:
                if issue.severity in counts:
                    counts[issue.severity] += 1
            
            return counts
        
        except Exception as e:
            logger.error(f"Severity counting failed: {e}")
            return {"critical": 0, "high": 0, "medium": 0, "low": 0}
    
    def _generate_recommendations(self, issues: List[ClassifiedIssue]) -> List[str]:
        """Generate prioritized recommendations based on issues"""
        try:
            recommendations = []
            
            # Group issues by category
            issues_by_category = {}
            for issue in issues:
                if issue.category not in issues_by_category:
                    issues_by_category[issue.category] = []
                issues_by_category[issue.category].append(issue)
            
            # Generate recommendations for each category
            for category, category_issues in issues_by_category.items():
                critical_count = sum(1 for issue in category_issues if issue.severity == "critical")
                high_count = sum(1 for issue in category_issues if issue.severity == "high")
                
                if critical_count > 0:
                    recommendations.append(
                        f"URGENT: Fix {critical_count} critical {category.replace('_', ' ')} issues immediately"
                    )
                elif high_count > 2:
                    recommendations.append(
                        f"Priority: Address {high_count} high-severity {category.replace('_', ' ')} issues"
                    )
                
                # Add specific fixes for frequent issues
                most_frequent = max(category_issues, key=lambda x: x.frequency)
                if most_frequent.frequency > 2:
                    recommendations.append(
                        f"Investigate recurring issue: {most_frequent.issue_type} ({most_frequent.frequency} occurrences)"
                    )
            
            # Add general recommendations if no specific ones
            if not recommendations:
                recommendations.append("No critical issues found. Continue monitoring.")
            
            return recommendations
        
        except Exception as e:
            logger.error(f"Recommendation generation failed: {e}")
            return ["Error generating recommendations"]
    
    def get_issue_statistics(self) -> Dict[str, Any]:
        """Get statistics about classified issues"""
        try:
            total_occurrences = sum(history.occurrences for history in self.issue_history.values())
            unique_issues = len(self.issue_history)
            
            # Find most common issue type
            type_counts = {}
            for history in self.issue_history.values():
                type_counts[history.issue_type] = type_counts.get(history.issue_type, 0) + history.occurrences
            
            most_common_type = max(type_counts.items(), key=lambda x: x[1]) if type_counts else ("none", 0)
            
            return {
                "total_occurrences": total_occurrences,
                "unique_issues": unique_issues,
                "most_common_type": most_common_type[0],
                "most_common_count": most_common_type[1],
                "categories": list(set(history.category for history in self.issue_history.values()))
            }
        
        except Exception as e:
            logger.error(f"Statistics calculation failed: {e}")
            return {}
    
    def export_issue_history(self, filepath: str):
        """Export issue history to JSON file"""
        try:
            history_data = {}
            for key, history in self.issue_history.items():
                history_data[key] = {
                    "issue_type": history.issue_type,
                    "category": history.category,
                    "occurrences": history.occurrences,
                    "first_seen": history.first_seen,
                    "last_seen": history.last_seen,
                    "severity_trend": history.severity_trend,
                    "affected_screenshots": history.affected_screenshots
                }
            
            with open(filepath, 'w') as f:
                json.dump(history_data, f, indent=2)
            
            logger.info(f"Issue history exported to {filepath}")
        
        except Exception as e:
            logger.error(f"Failed to export issue history: {e}")
    
    def load_issue_history(self, filepath: str):
        """Load issue history from JSON file"""
        try:
            with open(filepath, 'r') as f:
                history_data = json.load(f)
            
            self.issue_history = {}
            for key, data in history_data.items():
                self.issue_history[key] = IssueHistory(
                    issue_type=data["issue_type"],
                    category=data["category"],
                    occurrences=data["occurrences"],
                    first_seen=data["first_seen"],
                    last_seen=data["last_seen"],
                    severity_trend=data["severity_trend"],
                    affected_screenshots=data["affected_screenshots"]
                )
            
            logger.info(f"Issue history loaded from {filepath}")
        
        except Exception as e:
            logger.error(f"Failed to load issue history: {e}")

# Example usage and testing
if __name__ == "__main__":
    classifier = IssueClassifier()
    
    # Example analysis results
    example_results = {
        "visual_artifacts": [
            # This would be actual VisualArtifact objects
        ],
        "text_elements": [
            # This would be actual TextElement objects
        ],
        "object_discrepancies": [
            # This would be actual StateDiscrepancy objects
        ],
        "state_discrepancies": [
            # This would be actual StateDiscrepancy objects
        ]
    }
    
    # Classify issues
    result = classifier.classify_issues(example_results)
    
    print(json.dumps({
        "total_issues": len(result.classified_issues),
        "critical": result.critical_count,
        "high": result.high_count,
        "medium": result.medium_count,
        "low": result.low_count,
        "processing_time": result.processing_time
    }, indent=2))
    
    # Show issue summary
    if result.issue_summary:
        print(f"\nIssue Summary: {json.dumps(result.issue_summary, indent=2)}")