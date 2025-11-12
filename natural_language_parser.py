"""
Natural Language Parser - Enhanced goal parsing using Kimi API
Understands complex development goals, extracts requirements, identifies dependencies and risks
"""

import re
import json
import logging
from typing import Dict, List, Optional, Any, Tuple
from dataclasses import dataclass, field
from enum import Enum
from datetime import datetime

from kimi_api_client import KimiAPIClient, APIResponse

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class GoalType(Enum):
    """Types of development goals"""
    FEATURE_IMPLEMENTATION = "feature_implementation"
    BUG_FIX = "bug_fix"
    PERFORMANCE_OPTIMIZATION = "performance_optimization"
    CODE_REFACTORING = "code_refactoring"
    TESTING = "testing"
    DOCUMENTATION = "documentation"
    DEPLOYMENT = "deployment"
    INTEGRATION = "integration"
    RESEARCH = "research"
    ANALYSIS = "analysis"
    UNKNOWN = "unknown"


class ComplexityLevel(Enum):
    """Complexity levels for tasks"""
    SIMPLE = "simple"
    MODERATE = "moderate"
    COMPLEX = "complex"
    VERY_COMPLEX = "very_complex"


class RiskLevel(Enum):
    """Risk levels for identified risks"""
    LOW = "low"
    MEDIUM = "medium"
    HIGH = "high"
    CRITICAL = "critical"


@dataclass
class ParsedRequirement:
    """Parsed requirement from goal description"""
    requirement_id: str
    description: str
    priority: str  # high, medium, low
    is_functional: bool
    is_technical: bool
    estimated_effort_hours: Optional[float] = None
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "requirement_id": self.requirement_id,
            "description": self.description,
            "priority": self.priority,
            "is_functional": self.is_functional,
            "is_technical": self.is_technical,
            "estimated_effort_hours": self.estimated_effort_hours
        }


@dataclass
class ParsedConstraint:
    """Parsed constraint or limitation"""
    constraint_id: str
    description: str
    constraint_type: str  # technical, business, resource, time
    impact_level: str  # high, medium, low
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "constraint_id": self.constraint_id,
            "description": self.description,
            "constraint_type": self.constraint_type,
            "impact_level": self.impact_level
        }


@dataclass
class IdentifiedRisk:
    """Identified risk with mitigation strategy"""
    risk_id: str
    description: str
    risk_level: RiskLevel
    probability: str  # high, medium, low
    impact: str  # high, medium, low
    mitigation_strategy: Optional[str] = None
    contingency_plan: Optional[str] = None
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "risk_id": self.risk_id,
            "description": self.description,
            "risk_level": self.risk_level.value,
            "probability": self.probability,
            "impact": self.impact,
            "mitigation_strategy": self.mitigation_strategy,
            "contingency_plan": self.contingency_plan
        }


@dataclass
class ParsedGoal:
    """Completely parsed development goal"""
    goal_id: str
    original_description: str
    goal_type: GoalType
    summary: str
    estimated_complexity: str
    requirements: List[ParsedRequirement] = field(default_factory=list)
    constraints: List[ParsedConstraint] = field(default_factory=list)
    identified_risks: List[IdentifiedRisk] = field(default_factory=list)
    success_criteria: List[str] = field(default_factory=list)
    dependencies: List[str] = field(default_factory=list)
    estimated_duration_hours: Optional[float] = None
    created_at: datetime = field(default_factory=datetime.now)
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "goal_id": self.goal_id,
            "original_description": self.original_description,
            "goal_type": self.goal_type.value,
            "summary": self.summary,
            "estimated_complexity": self.estimated_complexity,
            "requirements": [req.to_dict() for req in self.requirements],
            "constraints": [constraint.to_dict() for constraint in self.constraints],
            "identified_risks": [risk.to_dict() for risk in self.identified_risks],
            "success_criteria": self.success_criteria,
            "dependencies": self.dependencies,
            "estimated_duration_hours": self.estimated_duration_hours,
            "created_at": self.created_at.isoformat()
        }


class NaturalLanguageParser:
    """
    Enhanced natural language parser using Kimi API
    Understands complex development goals and extracts structured information
    """

    def __init__(self, kimi_client: Optional[KimiAPIClient] = None):
        """
        Initialize natural language parser
        
        Args:
            kimi_client: Kimi API client instance (creates new if not provided)
        """
        self.kimi_client = kimi_client or KimiAPIClient()
        
        # Keywords for goal type classification
        self.goal_type_keywords = {
            GoalType.FEATURE_IMPLEMENTATION: [
                "add", "implement", "create", "build", "develop", "feature", "new", "support"
            ],
            GoalType.BUG_FIX: [
                "fix", "bug", "error", "issue", "problem", "broken", "doesn't work", "crash"
            ],
            GoalType.PERFORMANCE_OPTIMIZATION: [
                "optimize", "performance", "speed", "slow", "fast", "efficient", "bottleneck"
            ],
            GoalType.CODE_REFACTORING: [
                "refactor", "clean up", "restructure", "improve", "simplify", "modernize"
            ],
            GoalType.TESTING: [
                "test", "validate", "verify", "check", "ensure", "coverage", "automated test"
            ],
            GoalType.DOCUMENTATION: [
                "document", "readme", "wiki", "comments", "explain", "guide", "tutorial"
            ],
            GoalType.DEPLOYMENT: [
                "deploy", "release", "publish", "ship", "production", "live"
            ],
            GoalType.INTEGRATION: [
                "integrate", "connect", "interface", "api", "third-party", "service"
            ],
            GoalType.RESEARCH: [
                "research", "investigate", "explore", "study", "analyze", "prototype"
            ],
            GoalType.ANALYSIS: [
                "analyze", "review", "assess", "evaluate", "audit", "examine"
            ]
        }

    def parse_development_goal(self, goal_description: str, context: Optional[str] = None) -> ParsedGoal:
        """
        Parse a natural language development goal using Kimi AI
        
        Args:
            goal_description: Natural language description of the goal
            context: Additional context about the project
            
        Returns:
            ParsedGoal with extracted information
        """
        logger.info(f"Parsing development goal: {goal_description[:100]}...")
        
        # Use Kimi AI for comprehensive parsing
        parsed_data = self._parse_with_kimi(goal_description, context)
        
        # Create goal ID
        goal_id = f"goal_{int(datetime.now().timestamp())}"
        
        # Create parsed goal
        parsed_goal = ParsedGoal(
            goal_id=goal_id,
            original_description=goal_description,
            goal_type=parsed_data.get("goal_type", GoalType.UNKNOWN),
            summary=parsed_data.get("summary", goal_description[:200]),
            estimated_complexity=parsed_data.get("complexity", "moderate"),
            requirements=parsed_data.get("requirements", []),
            constraints=parsed_data.get("constraints", []),
            identified_risks=parsed_data.get("risks", []),
            success_criteria=parsed_data.get("success_criteria", []),
            dependencies=parsed_data.get("dependencies", []),
            estimated_duration_hours=parsed_data.get("estimated_duration_hours")
        )
        
        logger.info(f"Parsed goal {goal_id} as type: {parsed_goal.goal_type.value}")
        logger.info(f"  Requirements: {len(parsed_goal.requirements)}")
        logger.info(f"  Constraints: {len(parsed_goal.constraints)}")
        logger.info(f"  Risks: {len(parsed_goal.identified_risks)}")
        
        return parsed_goal

    def _parse_with_kimi(self, goal_description: str, context: Optional[str]) -> Dict[str, Any]:
        """
        Use Kimi AI to parse the goal description
        
        Args:
            goal_description: Natural language goal description
            context: Additional context
            
        Returns:
            Parsed data dictionary
        """
        prompt = f"""Analyze this development goal and extract structured information:

GOAL: {goal_description}

CONTEXT: {context or "No additional context provided"}

Please provide a comprehensive analysis with the following information:

1. GOAL_TYPE: Classify the goal type (feature_implementation, bug_fix, performance_optimization, code_refactoring, testing, documentation, deployment, integration, research, analysis, unknown)

2. SUMMARY: Brief summary (1-2 sentences) of what needs to be accomplished

3. COMPLEXITY: Estimate complexity (simple, moderate, complex, very_complex)

4. ESTIMATED_DURATION_HOURS: Estimated time to complete in hours (be realistic)

5. REQUIREMENTS: List of specific requirements (functional and technical)
   Format as a JSON array with requirement objects

6. CONSTRAINTS: List of constraints and limitations
   Format as a JSON array with constraint objects

7. RISKS: List of potential risks with mitigation strategies
   Format as a JSON array with risk objects

8. SUCCESS_CRITERIA: List of success criteria (what defines success)

9. DEPENDENCIES: List of dependencies (other tasks, systems, resources)

Format your response as a JSON object with these keys: goal_type, summary, complexity, estimated_duration_hours, requirements, constraints, risks, success_criteria, dependencies

Be thorough, realistic, and specific in your analysis.
"""
        
        messages = [
            {
                "role": "system",
                "content": "You are an expert software project manager and business analyst. Analyze development goals and extract detailed, structured information including requirements, constraints, risks, and success criteria."
            },
            {"role": "user", "content": prompt}
        ]
        
        response = self.kimi_client.chat_completion(messages)
        
        if response.success and response.content:
            try:
                # Extract JSON from response
                content = response.content.strip()
                
                # Handle code blocks
                if "```json" in content:
                    content = content.split("```json")[1].split("```")[0].strip()
                elif "```" in content:
                    content = content.split("```")[1].split("```")[0].strip()
                
                parsed_data = json.loads(content)
                
                # Convert string values to proper types
                parsed_data = self._convert_parsed_data(parsed_data)
                
                return parsed_data
                
            except json.JSONDecodeError as e:
                logger.error(f"Failed to parse Kimi response: {e}")
                return self._get_default_parsed_data(goal_description)
            except Exception as e:
                logger.error(f"Error processing Kimi response: {e}")
                return self._get_default_parsed_data(goal_description)
        else:
            logger.error(f"Kimi parsing failed: {response.error_message}")
            return self._get_default_parsed_data(goal_description)

    def _convert_parsed_data(self, data: Dict[str, Any]) -> Dict[str, Any]:
        """
        Convert parsed data to proper types
        
        Args:
            data: Raw parsed data
            
        Returns:
            Converted data
        """
        # Convert goal type string to enum
        if "goal_type" in data and isinstance(data["goal_type"], str):
            try:
                data["goal_type"] = GoalType(data["goal_type"])
            except ValueError:
                data["goal_type"] = GoalType.UNKNOWN
        
        # Convert requirements
        if "requirements" in data:
            requirements = []
            for req_data in data["requirements"]:
                requirement = ParsedRequirement(
                    requirement_id=req_data.get("requirement_id", f"req_{len(requirements)}"),
                    description=req_data.get("description", ""),
                    priority=req_data.get("priority", "medium"),
                    is_functional=req_data.get("is_functional", True),
                    is_technical=req_data.get("is_technical", False),
                    estimated_effort_hours=req_data.get("estimated_effort_hours")
                )
                requirements.append(requirement)
            data["requirements"] = requirements
        
        # Convert constraints
        if "constraints" in data:
            constraints = []
            for con_data in data["constraints"]:
                constraint = ParsedConstraint(
                    constraint_id=con_data.get("constraint_id", f"con_{len(constraints)}"),
                    description=con_data.get("description", ""),
                    constraint_type=con_data.get("constraint_type", "technical"),
                    impact_level=con_data.get("impact_level", "medium")
                )
                constraints.append(constraint)
            data["constraints"] = constraints
        
        # Convert risks
        if "risks" in data:
            risks = []
            for risk_data in data["risks"]:
                risk = IdentifiedRisk(
                    risk_id=risk_data.get("risk_id", f"risk_{len(risks)}"),
                    description=risk_data.get("description", ""),
                    risk_level=RiskLevel(risk_data.get("risk_level", "medium")),
                    probability=risk_data.get("probability", "medium"),
                    impact=risk_data.get("impact", "medium"),
                    mitigation_strategy=risk_data.get("mitigation_strategy"),
                    contingency_plan=risk_data.get("contingency_plan")
                )
                risks.append(risk)
            data["risks"] = risks
        
        return data

    def _get_default_parsed_data(self, goal_description: str) -> Dict[str, Any]:
        """
        Get default parsed data when Kimi is unavailable
        
        Args:
            goal_description: Original goal description
            
        Returns:
            Default parsed data
        """
        # Classify goal type based on keywords
        goal_type = self._classify_goal_type(goal_description)
        
        return {
            "goal_type": goal_type,
            "summary": f"Default summary: {goal_description[:100]}...",
            "complexity": "moderate",
            "estimated_duration_hours": 4.0,
            "requirements": [
                ParsedRequirement(
                    requirement_id="req_001",
                    description="Complete the described task",
                    priority="high",
                    is_functional=True,
                    is_technical=False,
                    estimated_effort_hours=4.0
                )
            ],
            "constraints": [],
            "risks": [
                IdentifiedRisk(
                    risk_id="risk_001",
                    description="Unknown complexity may affect timeline",
                    risk_level=RiskLevel.MEDIUM,
                    probability="medium",
                    impact="medium",
                    mitigation_strategy="Break down into smaller tasks",
                    contingency_plan="Extend timeline if needed"
                )
            ],
            "success_criteria": ["Task completed successfully", "No critical errors"],
            "dependencies": []
        }

    def _classify_goal_type(self, goal_description: str) -> GoalType:
        """
        Classify goal type based on keywords
        
        Args:
            goal_description: Goal description
            
        Returns:
            GoalType
        """
        goal_desc_lower = goal_description.lower()
        
        # Check each goal type for keyword matches
        for goal_type, keywords in self.goal_type_keywords.items():
            if any(keyword in goal_desc_lower for keyword in keywords):
                return goal_type
        
        return GoalType.UNKNOWN

    def extract_code_references(self, text: str) -> List[str]:
        """
        Extract code file references from text
        
        Args:
            text: Text to analyze
            
        Returns:
            List of likely code file references
        """
        # Patterns for code files
        patterns = [
            r'\b\w+\.(py|cpp|h|cs|js|ts|java|json|xml|yaml|yml|md|txt)\b',  # File extensions
            r'\b\w+/\w+\.(py|cpp|h|cs|js|ts|java|json|xml|yaml|yml|md|txt)\b',  # Paths with extensions
            r'\b[A-Z][a-zA-Z]+\.(py|cpp|h|cs|js|ts|java)\b',  # Class files
        ]
        
        code_refs = []
        for pattern in patterns:
            matches = re.findall(pattern, text)
            code_refs.extend(matches)
        
        return list(set(code_refs))

    def identify_technical_terms(self, text: str) -> List[str]:
        """
        Identify technical terms in text
        
        Args:
            text: Text to analyze
            
        Returns:
            List of technical terms
        """
        # Common technical terms (customize for your domain)
        technical_terms = [
            "API", "HTTP", "UDP", "TCP", "JSON", "XML", "REST", "GraphQL",
            "Unreal", "Unity", "Blueprint", "C++", "Python", "JavaScript",
            "database", "server", "client", "frontend", "backend",
            "multicast", "port", "endpoint", "request", "response",
            "function", "class", "method", "variable", "algorithm",
            "performance", "optimization", "refactoring", "testing"
        ]
        
        found_terms = []
        text_lower = text.lower()
        
        for term in technical_terms:
            if term.lower() in text_lower:
                found_terms.append(term)
        
        return list(set(found_terms))

    def analyze_goal_sentiment(self, goal_description: str) -> Dict[str, Any]:
        """
        Analyze sentiment of goal description
        
        Args:
            goal_description: Goal description
            
        Returns:
            Sentiment analysis results
        """
        prompt = f"""Analyze the sentiment and urgency of this development goal:

GOAL: {goal_description}

Provide:
1. SENTIMENT: positive, negative, or neutral
2. URGENCY: high, medium, or low
3. CONFIDENCE: high, medium, or low (confidence in understanding)
4. KEY_EMOTIONS: List of key emotions detected

Format as JSON with these keys.
"""
        
        messages = [
            {"role": "system", "content": "You are a sentiment analysis expert. Analyze text for sentiment, urgency, and emotional tone."},
            {"role": "user", "content": prompt}
        ]
        
        response = self.kimi_client.chat_completion(messages)
        
        if response.success and response.content:
            try:
                # Extract JSON
                content = response.content.strip()
                if "```" in content:
                    content = content.split("```")[1].split("```")[0].strip()
                
                sentiment_data = json.loads(content)
                return sentiment_data
                
            except (json.JSONDecodeError, IndexError):
                return {
                    "sentiment": "neutral",
                    "urgency": "medium",
                    "confidence": "medium",
                    "key_emotions": ["neutral"]
                }
        else:
            return {
                "sentiment": "neutral",
                "urgency": "medium",
                "confidence": "low",
                "key_emotions": ["uncertain"]
            }


# ============================================================================
# TESTING AND USAGE EXAMPLES
# ============================================================================

def test_natural_language_parser():
    """Test the Natural Language Parser"""
    try:
        print("Testing Natural Language Parser")
        print("=" * 50)
        
        # Initialize parser
        parser = NaturalLanguageParser()
        
        # Test cases
        test_goals = [
            "Fix the bug where ships don't spawn correctly in multiplayer mode",
            "Optimize the physics calculations for better performance when many ships are present",
            "Add a new flight assist mode for docking maneuvers that helps players align with stations",
            "Refactor the ship customization code to make it more maintainable and add unit tests",
            "Create documentation for the UDP multicast communication system"
        ]
        
        for i, goal_desc in enumerate(test_goals, 1):
            print(f"\n{i}. Testing: {goal_desc[:60]}...")
            
            # Parse goal
            parsed_goal = parser.parse_development_goal(goal_desc)
            
            print(f"   ✓ Goal type: {parsed_goal.goal_type.value}")
            print(f"   ✓ Complexity: {parsed_goal.estimated_complexity}")
            print(f"   ✓ Requirements: {len(parsed_goal.requirements)}")
            print(f"   ✓ Constraints: {len(parsed_goal.constraints)}")
            print(f"   ✓ Risks: {len(parsed_goal.identified_risks)}")
            
            # Show first requirement if available
            if parsed_goal.requirements:
                req = parsed_goal.requirements[0]
                print(f"   ✓ First requirement: {req.description[:50]}...")
            
            # Show first risk if available
            if parsed_goal.identified_risks:
                risk = parsed_goal.identified_risks[0]
                print(f"   ✓ First risk: {risk.description[:50]}...")
        
        # Test code reference extraction
        print("\n2. Testing code reference extraction...")
        sample_text = "We need to fix the bug in ship_spawner.py and update the config in settings.json"
        code_refs = parser.extract_code_references(sample_text)
        print(f"   ✓ Found code references: {code_refs}")
        
        # Test technical term identification
        print("\n3. Testing technical term identification...")
        tech_terms = parser.identify_technical_terms(sample_text)
        print(f"   ✓ Found technical terms: {tech_terms}")
        
        # Test sentiment analysis
        print("\n4. Testing sentiment analysis...")
        sentiment = parser.analyze_goal_sentiment("Urgently fix the critical bug that's crashing the game!")
        print(f"   ✓ Sentiment: {sentiment['sentiment']}")
        print(f"   ✓ Urgency: {sentiment['urgency']}")
        print(f"   ✓ Confidence: {sentiment['confidence']}")
        
        print("\n" + "=" * 50)
        print("✓ All natural language parser tests passed!")
        return True
        
    except Exception as e:
        print(f"✗ Natural language parser test failed: {e}")
        import traceback
        traceback.print_exc()
        return False


if __name__ == "__main__":
    success = test_natural_language_parser()
    exit(0 if success else 1)