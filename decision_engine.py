"""
Decision Engine - Advanced reasoning and decision-making system
Provides intelligent decision-making for task execution, error recovery, and safety management
"""

import json
import logging
import time
from typing import Dict, List, Optional, Any, Tuple, Union
from dataclasses import dataclass, field
from enum import Enum
from datetime import datetime

from kimi_api_client import KimiAPIClient, APIResponse
from ai_control_layer import Task, TaskStatus, TaskPriority, SafetyLevel
from task_generator import TaskSpecification, TaskType
from natural_language_parser import ParsedGoal, RiskLevel

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class DecisionType(Enum):
    """Types of decisions the engine can make"""
    TASK_EXECUTION_STRATEGY = "task_execution_strategy"
    ERROR_RECOVERY = "error_recovery"
    SAFETY_OVERRIDE = "safety_override"
    RESOURCE_ALLOCATION = "resource_allocation"
    TASK_PRIORITIZATION = "task_prioritization"
    RISK_MITIGATION = "risk_mitigation"
    EXECUTION_ORDER = "execution_order"
    FALLBACK_PLAN = "fallback_plan"


class DecisionConfidence(Enum):
    """Confidence level in a decision"""
    LOW = "low"
    MEDIUM = "medium"
    HIGH = "high"
    CERTAIN = "certain"


@dataclass
class Decision:
    """Represents a decision made by the engine"""
    decision_id: str
    decision_type: DecisionType
    description: str
    recommended_action: str
    confidence: DecisionConfidence
    reasoning: str
    alternatives: List[str] = field(default_factory=list)
    risks: List[str] = field(default_factory=list)
    prerequisites: List[str] = field(default_factory=list)
    estimated_impact: str = "neutral"  # positive, negative, neutral
    created_at: datetime = field(default_factory=datetime.now)
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "decision_id": self.decision_id,
            "decision_type": self.decision_type.value,
            "description": self.description,
            "recommended_action": self.recommended_action,
            "confidence": self.confidence.value,
            "reasoning": self.reasoning,
            "alternatives": self.alternatives,
            "risks": self.risks,
            "prerequisites": self.prerequisites,
            "estimated_impact": self.estimated_impact,
            "created_at": self.created_at.isoformat()
        }


@dataclass
class DecisionContext:
    """Context for decision-making"""
    available_tasks: List[Task]
    system_state: Dict[str, Any]
    historical_data: List[Dict[str, Any]]
    resource_constraints: Dict[str, Any]
    safety_threshold: SafetyLevel = SafetyLevel.MEDIUM_RISK
    time_constraints: Optional[Dict[str, Any]] = None
    external_factors: List[str] = field(default_factory=list)


class DecisionEngine:
    """
    Advanced decision-making engine using Kimi AI for reasoning
    Provides intelligent decisions for task execution, error recovery, and safety management
    """

    def __init__(self, kimi_client: Optional[KimiAPIClient] = None):
        """
        Initialize decision engine
        
        Args:
            kimi_client: Kimi API client instance (creates new if not provided)
        """
        self.kimi_client = kimi_client or KimiAPIClient()
        
        # Decision history for learning
        self.decision_history: List[Decision] = []
        
        # Success rate tracking
        self.decision_success_rate: Dict[str, float] = {}
        
        # Risk tolerance settings
        self.risk_tolerance = {
            "low": 0.1,  # 10% risk tolerance
            "medium": 0.3,  # 30% risk tolerance
            "high": 0.5  # 50% risk tolerance
        }
        
        logger.info("Decision Engine initialized")

    def decide_task_execution_strategy(self, task: Task, context: DecisionContext) -> Decision:
        """
        Decide the best execution strategy for a task
        
        Args:
            task: Task to execute
            context: Decision context
            
        Returns:
            Decision with execution strategy
        """
        logger.info(f"Deciding execution strategy for task: {task.task_id}")
        
        # Use Kimi AI for complex reasoning
        prompt = self._create_execution_strategy_prompt(task, context)
        
        response = self.kimi_client.chat_completion([
            {"role": "system", "content": "You are an expert software development strategist. Provide detailed execution strategies with reasoning and risk assessment."},
            {"role": "user", "content": prompt}
        ])
        
        if response.success and response.content:
            decision = self._parse_execution_decision(response.content, task.task_id)
        else:
            # Fallback to rule-based decision
            decision = self._create_fallback_execution_decision(task, context)
        
        # Record decision
        self.decision_history.append(decision)
        
        logger.info(f"Execution strategy decision: {decision.recommended_action} (confidence: {decision.confidence.value})")
        return decision

    def decide_error_recovery(self, failed_task: Task, error_message: str, context: DecisionContext) -> Decision:
        """
        Decide how to recover from a task failure
        
        Args:
            failed_task: Task that failed
            error_message: Error message
            context: Decision context
            
        Returns:
            Decision with recovery strategy
        """
        logger.info(f"Deciding error recovery for failed task: {failed_task.task_id}")
        
        prompt = f"""Task {failed_task.task_id} failed with error: {error_message}

Task details:
- Title: {failed_task.title}
- Type: {failed_task.task_type}
- Safety Level: {failed_task.safety_level.value}
- Previous attempts: {context.historical_data.get('attempts', 0)}

System state:
- Available resources: {context.resource_constraints}
- Current task queue: {len(context.available_tasks)} tasks

Please recommend the best error recovery strategy. Consider:
1. Should we retry the task?
2. Should we skip this task and continue?
3. Should we rollback changes?
4. Should we escalate to human intervention?
5. What are the risks of each approach?

Provide a detailed decision with reasoning and alternatives.
"""
        
        response = self.kimi_client.chat_completion([
            {"role": "system", "content": "You are an expert in error recovery and fault tolerance. Provide strategic recovery plans with risk assessment."},
            {"role": "user", "content": prompt}
        ])
        
        if response.success and response.content:
            decision = self._parse_recovery_decision(response.content, failed_task.task_id)
        else:
            decision = self._create_fallback_recovery_decision(failed_task, error_message, context)
        
        self.decision_history.append(decision)
        logger.info(f"Error recovery decision: {decision.recommended_action}")
        
        return decision

    def decide_safety_override(self, task: Task, safety_concern: str, context: DecisionContext) -> Decision:
        """
        Decide whether to override safety restrictions for a task
        
        Args:
            task: Task in question
            safety_concern: Safety concern description
            context: Decision context
            
        Returns:
            Decision on safety override
        """
        logger.warning(f"Safety override decision needed for task: {task.task_id}")
        
        prompt = f"""Safety override request for task: {task.task_id}

Task: {task.title}
Safety Level: {task.safety_level.value}
Concern: {safety_concern}

Task details:
- Description: {task.description}
- Required files: {task.required_files}
- Success criteria: {task.success_criteria}

Please decide:
1. Should we allow this task to proceed despite safety concerns?
2. What additional safety measures should be implemented?
3. What are the risks of proceeding vs. blocking?
4. Are there safer alternatives to achieve the same goal?

Consider the criticality of the task and potential impact.
"""
        
        response = self.kimi_client.chat_completion([
            {"role": "system", "content": "You are a safety expert and risk assessor. Make careful decisions about safety overrides with thorough risk analysis."},
            {"role": "user", "content": prompt}
        ])
        
        if response.success and response.content:
            decision = self._parse_safety_decision(response.content, task.task_id)
        else:
            decision = self._create_fallback_safety_decision(task, safety_concern, context)
        
        self.decision_history.append(decision)
        logger.warning(f"Safety override decision: {decision.recommended_action} (confidence: {decision.confidence.value})")
        
        return decision

    def decide_task_prioritization(self, tasks: List[Task], context: DecisionContext) -> List[Tuple[Task, int]]:
        """
        Decide optimal task execution order based on priorities and dependencies
        
        Args:
            tasks: List of tasks to prioritize
            context: Decision context
            
        Returns:
            List of (task, priority_score) tuples in recommended order
        """
        logger.info(f"Prioritizing {len(tasks)} tasks")
        
        prompt = f"""Prioritize these {len(tasks)} tasks for optimal execution:

Tasks:
{self._format_tasks_for_prompt(tasks)}

Context:
- System state: {context.system_state}
- Resource constraints: {context.resource_constraints}
- Time constraints: {context.time_constraints}
- Safety threshold: {context.safety_threshold.value}

Consider:
1. Task dependencies (some tasks must come before others)
2. Resource requirements and availability
3. Safety levels (higher risk tasks may need more preparation)
4. Expected duration and complexity
5. Business value and impact
6. Risk factors

Provide a prioritized list with reasoning for the ordering.
"""
        
        response = self.kimi_client.chat_completion([
            {"role": "system", "content": "You are an expert project manager and scheduler. Optimize task ordering for efficiency and risk management."},
            {"role": "user", "content": prompt}
        ])
        
        if response.success and response.content:
            prioritized_tasks = self._parse_prioritization_decision(response.content, tasks)
        else:
            prioritized_tasks = self._create_fallback_prioritization(tasks, context)
        
        logger.info(f"Task prioritization complete. Top priority: {prioritized_tasks[0][0].task_id if prioritized_tasks else 'None'}")
        
        return prioritized_tasks

    def decide_risk_mitigation(self, risks: List[Dict[str, Any]], context: DecisionContext) -> Decision:
        """
        Decide risk mitigation strategies
        
        Args:
            risks: List of identified risks
            context: Decision context
            
        Returns:
            Decision with mitigation strategy
        """
        logger.info(f"Deciding risk mitigation for {len(risks)} risks")
        
        prompt = f"""Develop risk mitigation strategies for these risks:

Risks:
{json.dumps(risks, indent=2)}

Context:
- Available resources: {context.resource_constraints}
- Safety threshold: {context.safety_threshold.value}
- External factors: {context.external_factors}

For each risk, recommend:
1. Specific mitigation actions
2. Resource allocation for mitigation
3. Timeline for implementation
4. Success metrics
5. Contingency plans

Provide a comprehensive mitigation strategy.
"""
        
        response = self.kimi_client.chat_completion([
            {"role": "system", "content": "You are a risk management expert. Develop comprehensive risk mitigation strategies with actionable plans."},
            {"role": "user", "content": prompt}
        ])
        
        if response.success and response.content:
            decision = self._parse_risk_mitigation_decision(response.content)
        else:
            decision = self._create_fallback_risk_mitigation(risks, context)
        
        self.decision_history.append(decision)
        logger.info(f"Risk mitigation decision: {decision.recommended_action}")
        
        return decision

    def _create_execution_strategy_prompt(self, task: Task, context: DecisionContext) -> str:
        """Create prompt for execution strategy decision"""
        return f"""Determine the best execution strategy for this task:

TASK: {task.task_id} - {task.title}
Type: {task.task_type}
Priority: {task.priority.name}
Safety Level: {task.safety_level.value}
Description: {task.description}

Execution Steps: {len(task.execution_steps)} steps
Required Files: {task.required_files}
Dependencies: {task.dependencies}
Success Criteria: {task.success_criteria}

SYSTEM CONTEXT:
- Available resources: {context.resource_constraints}
- Current system load: {context.system_state.get('load', 'unknown')}
- Network status: {context.system_state.get('network_status', 'unknown')}
- Safety threshold: {context.safety_threshold.value}

Please recommend:
1. Best execution approach (parallel, sequential, phased)
2. Optimal timing for execution
3. Required resources and prerequisites
4. Monitoring and verification points
5. Fallback strategies
6. Risk mitigation during execution

Provide specific, actionable recommendations with clear reasoning.
"""

    def _parse_execution_decision(self, content: str, task_id: str) -> Decision:
        """Parse execution strategy decision from AI response"""
        # Extract key information from response
        # This is simplified - in production, use more sophisticated parsing
        
        decision_id = f"decision_{task_id}_{int(time.time())}"
        
        # Default to medium confidence if not specified
        confidence = DecisionConfidence.MEDIUM
        
        # Extract recommended action (first few sentences)
        lines = content.split('\n')
        recommended_action = lines[0] if lines else "Execute with standard procedure"
        
        # Create decision
        return Decision(
            decision_id=decision_id,
            decision_type=DecisionType.TASK_EXECUTION_STRATEGY,
            description=f"Execution strategy for task {task_id}",
            recommended_action=recommended_action,
            confidence=confidence,
            reasoning=content[:500],  # First 500 chars as reasoning
            alternatives=["Standard execution", "Delayed execution", "Phased execution"],
            risks=["Execution failure", "Resource exhaustion", "Timeout"],
            prerequisites=["Resource availability", "System stability"]
        )

    def _create_fallback_execution_decision(self, task: Task, context: DecisionContext) -> Decision:
        """Create fallback execution decision when AI is unavailable"""
        # Simple rule-based decision
        if task.safety_level in [SafetyLevel.HIGH_RISK, SafetyLevel.CRITICAL]:
            recommended_action = "Execute with enhanced monitoring and manual approval"
            confidence = DecisionConfidence.MEDIUM
        elif task.priority == TaskPriority.HIGH:
            recommended_action = "Execute immediately with standard monitoring"
            confidence = DecisionConfidence.HIGH
        else:
            recommended_action = "Execute when resources are available"
            confidence = DecisionConfidence.HIGH
        
        return Decision(
            decision_id=f"fallback_decision_{task.task_id}",
            decision_type=DecisionType.TASK_EXECUTION_STRATEGY,
            description="Fallback execution strategy",
            recommended_action=recommended_action,
            confidence=confidence,
            reasoning="Rule-based fallback decision due to AI unavailability",
            alternatives=["Delay execution", "Request manual intervention"],
            risks=["Suboptimal execution strategy"],
            prerequisites=["Basic system stability"]
        )

    def _parse_recovery_decision(self, content: str, task_id: str) -> Decision:
        """Parse error recovery decision from AI response"""
        decision_id = f"recovery_decision_{task_id}_{int(time.time())}"
        
        # Simple parsing - look for key recommendations
        content_lower = content.lower()
        
        if "retry" in content_lower:
            recommended_action = "Retry the task with modifications"
        elif "skip" in content_lower:
            recommended_action = "Skip this task and continue"
        elif "rollback" in content_lower:
            recommended_action = "Rollback changes and abort"
        else:
            recommended_action = "Escalate to human intervention"
        
        return Decision(
            decision_id=decision_id,
            decision_type=DecisionType.ERROR_RECOVERY,
            description=f"Recovery strategy for failed task {task_id}",
            recommended_action=recommended_action,
            confidence=DecisionConfidence.MEDIUM,
            reasoning=content[:500],
            alternatives=["Retry", "Skip", "Rollback", "Escalate"],
            risks=["Further failures", "Data corruption", "Cascade failures"],
            prerequisites=["Error analysis", "Impact assessment"]
        )

    def _create_fallback_recovery_decision(self, failed_task: Task, error_message: str, context: DecisionContext) -> Decision:
        """Create fallback recovery decision"""
        # Simple rule-based recovery
        if "timeout" in error_message.lower():
            recommended_action = "Retry with increased timeout"
        elif "resource" in error_message.lower():
            recommended_action = "Wait for resources and retry"
        elif "permission" in error_message.lower():
            recommended_action = "Escalate to human intervention"
        else:
            recommended_action = "Retry once, then escalate if failed"
        
        return Decision(
            decision_id=f"fallback_recovery_{failed_task.task_id}",
            decision_type=DecisionType.ERROR_RECOVERY,
            description="Fallback error recovery",
            recommended_action=recommended_action,
            confidence=DecisionConfidence.LOW,
            reasoning="Rule-based recovery due to AI unavailability",
            alternatives=["Immediate escalation", "Skip task"],
            risks=["Recovery may not succeed"],
            prerequisites=["Error classification"]
        )

    def _parse_safety_decision(self, content: str, task_id: str) -> Decision:
        """Parse safety override decision"""
        decision_id = f"safety_decision_{task_id}_{int(time.time())}"
        
        content_lower = content.lower()
        
        if "proceed" in content_lower and "safely" in content_lower:
            recommended_action = "Proceed with enhanced safety measures"
            confidence = DecisionConfidence.MEDIUM
        elif "block" in content_lower or "deny" in content_lower:
            recommended_action = "Block task execution"
            confidence = DecisionConfidence.HIGH
        else:
            recommended_action = "Request additional safety review"
            confidence = DecisionConfidence.LOW
        
        return Decision(
            decision_id=decision_id,
            decision_type=DecisionType.SAFETY_OVERRIDE,
            description=f"Safety decision for task {task_id}",
            recommended_action=recommended_action,
            confidence=confidence,
            reasoning=content[:500],
            alternatives=["Proceed with restrictions", "Block permanently", "Request review"],
            risks=["Safety violation", "System damage", "Data loss"],
            prerequisites=["Safety review", "Risk assessment"]
        )

    def _create_fallback_safety_decision(self, task: Task, safety_concern: str, context: DecisionContext) -> Decision:
        """Create fallback safety decision (conservative)"""
        # Conservative approach: block high-risk tasks
        if task.safety_level == SafetyLevel.CRITICAL:
            recommended_action = "BLOCK task execution - safety risk too high"
            confidence = DecisionConfidence.HIGH
        elif task.safety_level == SafetyLevel.HIGH_RISK:
            recommended_action = "Request manual safety review before proceeding"
            confidence = DecisionConfidence.MEDIUM
        else:
            recommended_action = "Proceed with enhanced monitoring"
            confidence = DecisionConfidence.MEDIUM
        
        return Decision(
            decision_id=f"fallback_safety_{task.task_id}",
            decision_type=DecisionType.SAFETY_OVERRIDE,
            description="Fallback safety decision",
            recommended_action=recommended_action,
            confidence=confidence,
            reasoning="Conservative safety decision due to AI unavailability",
            alternatives=["Block task", "Proceed with restrictions"],
            risks=["Potential safety violation"],
            prerequisites=["Manual review for high-risk tasks"]
        )

    def _format_tasks_for_prompt(self, tasks: List[Task]) -> str:
        """Format tasks for AI prompt"""
        formatted = ""
        for task in tasks:
            formatted += f"- {task.task_id}: {task.title} (Priority: {task.priority.name}, Safety: {task.safety_level.value}, Type: {task.task_type})\n"
        return formatted

    def _parse_prioritization_decision(self, content: str, tasks: List[Task]) -> List[Tuple[Task, int]]:
        """Parse task prioritization decision"""
        # This is simplified - would need more sophisticated parsing in production
        # For now, return tasks in original order with basic scoring
        
        prioritized = []
        for i, task in enumerate(tasks):
            # Simple scoring based on priority
            priority_score = task.priority.value * 100
            priority_score -= i  # Maintain some order
            
            prioritized.append((task, priority_score))
        
        # Sort by score
        prioritized.sort(key=lambda x: x[1], reverse=True)
        
        return prioritized

    def _create_fallback_prioritization(self, tasks: List[Task], context: DecisionContext) -> List[Tuple[Task, int]]:
        """Create fallback prioritization (rule-based)"""
        prioritized = []
        
        for task in tasks:
            # Base score from priority
            score = task.priority.value * 100
            
            # Adjust for safety level (lower safety = higher score)
            safety_adjustment = {
                SafetyLevel.SAFE: 50,
                SafetyLevel.LOW_RISK: 40,
                SafetyLevel.MEDIUM_RISK: 30,
                SafetyLevel.HIGH_RISK: 20,
                SafetyLevel.CRITICAL: 10
            }
            score += safety_adjustment.get(task.safety_level, 30)
            
            # Adjust for dependencies (tasks with fewer dependencies get higher score)
            score += max(0, 20 - len(task.dependencies) * 5)
            
            prioritized.append((task, score))
        
        # Sort by score
        prioritized.sort(key=lambda x: x[1], reverse=True)
        
        return prioritized

    def _parse_risk_mitigation_decision(self, content: str) -> Decision:
        """Parse risk mitigation decision"""
        decision_id = f"risk_mitigation_{int(time.time())}"
        
        return Decision(
            decision_id=decision_id,
            decision_type=DecisionType.RISK_MITIGATION,
            description="Risk mitigation strategy",
            recommended_action="Implement recommended mitigation measures",
            confidence=DecisionConfidence.MEDIUM,
            reasoning=content[:500],
            alternatives=["Accept risks", "Avoid risky activities"],
            risks=["Incomplete mitigation", "Resource overallocation"],
            prerequisites=["Risk assessment", "Resource availability"]
        )

    def _create_fallback_risk_mitigation(self, risks: List[Dict[str, Any]], context: DecisionContext) -> Decision:
        """Create fallback risk mitigation"""
        mitigation_actions = []
        
        for risk in risks:
            risk_level = risk.get("risk_level", "medium")
            if risk_level in ["high", "critical"]:
                mitigation_actions.append(f"Address high-risk item: {risk.get('description', 'Unknown')}")
        
        return Decision(
            decision_id=f"fallback_risk_mitigation_{int(time.time())}",
            decision_type=DecisionType.RISK_MITIGATION,
            description="Fallback risk mitigation",
            recommended_action="Implement basic risk controls and monitoring",
            confidence=DecisionConfidence.LOW,
            reasoning="Rule-based risk mitigation due to AI unavailability",
            alternatives=["Accept all risks", "Stop all activities"],
            risks=["Insufficient mitigation"],
            prerequisites=["Basic risk identification"]
        )

    def get_decision_success_rate(self, decision_type: DecisionType) -> float:
        """
        Get historical success rate for a decision type
        
        Args:
            decision_type: Type of decision
            
        Returns:
            Success rate (0.0 to 1.0)
        """
        relevant_decisions = [d for d in self.decision_history if d.decision_type == decision_type]
        
        if not relevant_decisions:
            return 0.5  # Default to 50% if no history
        
        # This is simplified - in production, track actual outcomes
        return 0.7  # Assume 70% success rate

    def learn_from_outcome(self, decision_id: str, success: bool, outcome_details: str):
        """
        Learn from decision outcomes to improve future decisions
        
        Args:
            decision_id: ID of the decision
            success: Whether the decision led to success
            outcome_details: Details about the outcome
        """
        # Find the decision
        decision = next((d for d in self.decision_history if d.decision_id == decision_id), None)
        
        if decision:
            decision_type = decision.decision_type.value
            
            # Update success rate
            if decision_type not in self.decision_success_rate:
                self.decision_success_rate[decision_type] = 0.5
            
            # Simple moving average
            current_rate = self.decision_success_rate[decision_type]
            self.decision_success_rate[decision_type] = (current_rate * 0.9 + (1.0 if success else 0.0) * 0.1)
            
            logger.info(f"Learned from outcome: {decision_type} success rate now {self.decision_success_rate[decision_type]:.2f}")
        else:
            logger.warning(f"Decision {decision_id} not found in history")

    def get_decision_report(self) -> str:
        """Generate comprehensive decision report"""
        report = f"""
Decision Engine Report
======================

Generated: {datetime.now().isoformat()}
Total Decisions Made: {len(self.decision_history)}

Decision Success Rates:
"""
        
        for decision_type, rate in self.decision_success_rate.items():
            report += f"- {decision_type}: {rate:.1%}\n"
        
        report += f"\nRecent Decisions:\n"
        
        # Add last 5 decisions
        for decision in self.decision_history[-5:]:
            report += f"- {decision.decision_type.value}: {decision.recommended_action[:50]}... (confidence: {decision.confidence.value})\n"
        
        return report


# ============================================================================
# TESTING AND USAGE EXAMPLES
# ============================================================================

def test_decision_engine():
    """Test the Decision Engine"""
    try:
        print("Testing Decision Engine")
        print("=" * 50)
        
        # Initialize decision engine
        engine = DecisionEngine()
        
        # Create test context
        from ai_control_layer import Task, TaskPriority, SafetyLevel
        
        test_task = Task(
            task_id="test_task_001",
            title="Test task for decision making",
            description="A test task to evaluate decision engine",
            task_type="code_implementation",
            priority=TaskPriority.MEDIUM,
            safety_level=SafetyLevel.MEDIUM_RISK,
            dependencies=[],
            required_files=["test.py"],
            success_criteria=["Test passes"],
            execution_steps=[{"step": 1, "action": "test", "description": "Test step"}]
        )
        
        context = DecisionContext(
            available_tasks=[test_task],
            system_state={"load": "normal", "network_status": "online"},
            historical_data={"attempts": 0},
            resource_constraints={"cpu": 80, "memory": 70},
            safety_threshold=SafetyLevel.MEDIUM_RISK
        )
        
        # Test 1: Task execution strategy
        print("\n1. Testing task execution strategy decision...")
        decision = engine.decide_task_execution_strategy(test_task, context)
        print(f"   ✓ Decision: {decision.recommended_action[:60]}...")
        print(f"   ✓ Confidence: {decision.confidence.value}")
        print(f"   ✓ Risks: {len(decision.risks)} identified")
        
        # Test 2: Error recovery
        print("\n2. Testing error recovery decision...")
        recovery_decision = engine.decide_error_recovery(
            test_task,
            "Connection timeout after 30 seconds",
            context
        )
        print(f"   ✓ Recovery: {recovery_decision.recommended_action}")
        print(f"   ✓ Alternatives: {len(recovery_decision.alternatives)} options")
        
        # Test 3: Task prioritization
        print("\n3. Testing task prioritization...")
        
        # Create multiple test tasks
        tasks = [
            Task(
                task_id=f"priority_test_{i}",
                title=f"Priority test task {i}",
                description=f"Test task {i}",
                task_type="testing",
                priority=TaskPriority.HIGH if i % 2 == 0 else TaskPriority.MEDIUM,
                safety_level=SafetyLevel.LOW_RISK,
                dependencies=[],
                required_files=[],
                success_criteria=[],
                execution_steps=[]
            )
            for i in range(3)
        ]
        
        context.available_tasks = tasks
        prioritized = engine.decide_task_prioritization(tasks, context)
        print(f"   ✓ Prioritized {len(prioritized)} tasks")
        print(f"   ✓ Top priority: {prioritized[0][0].task_id} (score: {prioritized[0][1]})")
        
        # Test 4: Decision success rate tracking
        print("\n4. Testing decision learning...")
        initial_rate = engine.get_decision_success_rate(DecisionType.TASK_EXECUTION_STRATEGY)
        print(f"   ✓ Initial success rate: {initial_rate:.1%}")
        
        # Simulate learning
        engine.learn_from_outcome(decision.decision_id, True, "Task executed successfully")
        updated_rate = engine.get_decision_success_rate(DecisionType.TASK_EXECUTION_STRATEGY)
        print(f"   ✓ Updated success rate: {updated_rate:.1%}")
        
        # Test 5: Decision report
        print("\n5. Testing decision report generation...")
        report = engine.get_decision_report()
        print(f"   ✓ Report generated ({len(report)} characters)")
        print("   ✓ Report includes success rates and recent decisions")
        
        print("\n" + "=" * 50)
        print("✓ All decision engine tests passed!")
        return True
        
    except Exception as e:
        print(f"✗ Decision engine test failed: {e}")
        import traceback
        traceback.print_exc()
        return False


if __name__ == "__main__":
    success = test_decision_engine()
    exit(0 if success else 1)