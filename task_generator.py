"""
Task Generator - Intelligent task creation using Kimi API
Generates detailed, actionable tasks with dependencies, safety classification, and execution plans
"""

import re
import json
import logging
import uuid
from typing import Dict, List, Optional, Any, Tuple
from dataclasses import dataclass, field
from enum import Enum
from datetime import datetime

from kimi_api_client import KimiAPIClient, APIResponse
from ai_control_layer import TaskStatus, TaskPriority, SafetyLevel, Task
from natural_language_parser import ParsedGoal, ParsedRequirement, ParsedConstraint, IdentifiedRisk

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class TaskType(Enum):
    """Specific task types for development"""
    CODE_IMPLEMENTATION = "code_implementation"
    CODE_REFACTORING = "code_refactoring"
    BUG_FIX = "bug_fix"
    TESTING = "testing"
    ANALYSIS = "analysis"
    DOCUMENTATION = "documentation"
    DEPLOYMENT = "deployment"
    INTEGRATION = "integration"
    OPTIMIZATION = "optimization"
    RESEARCH = "research"
    UNREAL_COMMAND = "unreal_command"
    FILE_MODIFICATION = "file_modification"
    VALIDATION = "validation"


@dataclass
class ExecutionStep:
    """Detailed execution step for a task"""
    step_number: int
    action: str
    description: str
    expected_output: Optional[str] = None
    verification_method: Optional[str] = None
    timeout_seconds: int = 300
    rollback_action: Optional[str] = None
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "step_number": self.step_number,
            "action": self.action,
            "description": self.description,
            "expected_output": self.expected_output,
            "verification_method": self.verification_method,
            "timeout_seconds": self.timeout_seconds,
            "rollback_action": self.rollback_action
        }


@dataclass
class TaskSpecification:
    """Complete task specification"""
    task_id: str
    title: str
    description: str
    task_type: TaskType
    priority: TaskPriority
    safety_level: SafetyLevel
    estimated_duration_minutes: int
    required_skills: List[str]
    required_files: List[str]
    dependencies: List[str]
    execution_steps: List[ExecutionStep]
    success_criteria: List[str]
    failure_conditions: List[str]
    rollback_plan: Optional[str] = None
    created_at: datetime = field(default_factory=datetime.now)
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "task_id": self.task_id,
            "title": self.title,
            "description": self.description,
            "task_type": self.task_type.value,
            "priority": self.priority.name,
            "safety_level": self.safety_level.value,
            "estimated_duration_minutes": self.estimated_duration_minutes,
            "required_skills": self.required_skills,
            "required_files": self.required_files,
            "dependencies": self.dependencies,
            "execution_steps": [step.to_dict() for step in self.execution_steps],
            "success_criteria": self.success_criteria,
            "failure_conditions": self.failure_conditions,
            "rollback_plan": self.rollback_plan,
            "created_at": self.created_at.isoformat()
        }


class TaskGenerator:
    """
    Intelligent task generator using Kimi API
    Creates detailed, actionable tasks from parsed goals and requirements
    """

    def __init__(self, kimi_client: Optional[KimiAPIClient] = None):
        """
        Initialize task generator
        
        Args:
            kimi_client: Kimi API client instance (creates new if not provided)
        """
        self.kimi_client = kimi_client or KimiAPIClient()
        
        # Task type mappings for different goal types
        self.goal_type_task_mapping = {
            "feature_implementation": [
                TaskType.RESEARCH,
                TaskType.CODE_IMPLEMENTATION,
                TaskType.TESTING,
                TaskType.VALIDATION,
                TaskType.DOCUMENTATION
            ],
            "bug_fix": [
                TaskType.ANALYSIS,
                TaskType.CODE_REFACTORING,
                TaskType.TESTING,
                TaskType.VALIDATION
            ],
            "performance_optimization": [
                TaskType.ANALYSIS,
                TaskType.OPTIMIZATION,
                TaskType.TESTING,
                TaskType.VALIDATION
            ],
            "code_refactoring": [
                TaskType.ANALYSIS,
                TaskType.CODE_REFACTORING,
                TaskType.TESTING,
                TaskType.VALIDATION
            ],
            "testing": [
                TaskType.ANALYSIS,
                TaskType.TESTING,
                TaskType.VALIDATION
            ],
            "documentation": [
                TaskType.ANALYSIS,
                TaskType.DOCUMENTATION,
                TaskType.VALIDATION
            ],
            "deployment": [
                TaskType.VALIDATION,
                TaskType.DEPLOYMENT,
                TaskType.INTEGRATION
            ],
            "analysis": [
                TaskType.RESEARCH,
                TaskType.ANALYSIS,
                TaskType.DOCUMENTATION
            ]
        }

    def generate_tasks_from_goal(self, parsed_goal: ParsedGoal) -> List[TaskSpecification]:
        """
        Generate detailed tasks from a parsed development goal
        
        Args:
            parsed_goal: Parsed development goal
            
        Returns:
            List of task specifications
        """
        logger.info(f"Generating tasks for goal: {parsed_goal.goal_id}")
        
        tasks = []
        
        # Get task types for this goal type
        goal_type_value = parsed_goal.goal_type.value
        task_types = self.goal_type_task_mapping.get(goal_type_value, [TaskType.CODE_IMPLEMENTATION])
        
        # Generate tasks for each task type
        for i, task_type in enumerate(task_types):
            task_spec = self._generate_task_for_type(
                parsed_goal,
                task_type,
                i,
                len(task_types)
            )
            tasks.append(task_spec)
        
        # Add dependency information
        tasks = self._add_task_dependencies(tasks)
        
        logger.info(f"Generated {len(tasks)} tasks for goal {parsed_goal.goal_id}")
        return tasks

    def _generate_task_for_type(
        self,
        parsed_goal: ParsedGoal,
        task_type: TaskType,
        index: int,
        total_tasks: int
    ) -> TaskSpecification:
        """
        Generate a specific task for a task type
        
        Args:
            parsed_goal: Parsed development goal
            task_type: Type of task to generate
            index: Task index
            total_tasks: Total number of tasks
            
        Returns:
            Task specification
        """
        # Use Kimi AI to generate detailed task specification
        task_details = self._generate_task_details_with_kimi(parsed_goal, task_type)
        
        # Create task ID
        task_id = f"task_{parsed_goal.goal_id}_{task_type.value}_{index}"
        
        # Determine priority based on goal priority and task order
        priority = self._determine_task_priority(parsed_goal, index, total_tasks)
        
        # Determine safety level based on task type and requirements
        safety_level = self._determine_safety_level(task_type, parsed_goal)
        
        # Extract required files from requirements
        required_files = self._extract_required_files(parsed_goal)
        
        # Create execution steps
        execution_steps = self._create_execution_steps(task_type, task_details)
        
        # Create success criteria
        success_criteria = self._create_success_criteria(task_type, parsed_goal)
        
        # Create failure conditions
        failure_conditions = self._create_failure_conditions(task_type)
        
        # Create rollback plan for risky tasks
        rollback_plan = self._create_rollback_plan(safety_level, task_type)
        
        task_spec = TaskSpecification(
            task_id=task_id,
            title=task_details.get("title", f"{task_type.value.replace('_', ' ').title()} - {parsed_goal.summary[:50]}"),
            description=task_details.get("description", f"Execute {task_type.value} for goal: {parsed_goal.summary}"),
            task_type=task_type,
            priority=priority,
            safety_level=safety_level,
            estimated_duration_minutes=task_details.get("estimated_duration", 30),
            required_skills=task_details.get("required_skills", ["programming", "problem_solving"]),
            required_files=required_files,
            dependencies=[],  # Will be set later
            execution_steps=execution_steps,
            success_criteria=success_criteria,
            failure_conditions=failure_conditions,
            rollback_plan=rollback_plan
        )
        
        return task_spec

    def _generate_task_details_with_kimi(
        self,
        parsed_goal: ParsedGoal,
        task_type: TaskType
    ) -> Dict[str, Any]:
        """
        Use Kimi AI to generate detailed task information
        
        Args:
            parsed_goal: Parsed development goal
            task_type: Type of task to generate
            
        Returns:
            Task details dictionary
        """
        prompt = f"""Generate a detailed task specification for the following:

GOAL: {parsed_goal.original_description}
GOAL TYPE: {parsed_goal.goal_type.value}
TASK TYPE: {task_type.value}
COMPLEXITY: {parsed_goal.estimated_complexity}

Please provide:
1. TASK_TITLE: Clear, concise title for this task
2. DESCRIPTION: Detailed description of what needs to be done
3. ESTIMATED_DURATION: Estimated duration in minutes (be realistic)
4. REQUIRED_SKILLS: List of skills needed (e.g., Python, C++, Unreal Engine)
5. KEY_CONSIDERATIONS: Important things to consider during execution
6. POTENTIAL_PITFALLS: Common issues to watch out for

Format your response as a JSON object with these exact keys:
{{
    "title": "...",
    "description": "...",
    "estimated_duration": 60,
    "required_skills": ["skill1", "skill2"],
    "key_considerations": ["...", "..."],
    "potential_pitfalls": ["...", "..."]
}}

Be specific, actionable, and realistic in your estimates.
"""
        
        messages = [
            {
                "role": "system",
                "content": "You are an expert software project manager and task planner. Create detailed, realistic task specifications for development projects."
            },
            {"role": "user", "content": prompt}
        ]
        
        response = self.kimi_client.chat_completion(messages)
        
        if response.success and response.content:
            try:
                # Extract JSON from response
                content = response.content.strip()
                if "```json" in content:
                    content = content.split("```json")[1].split("```")[0].strip()
                elif "```" in content:
                    content = content.split("```")[1].split("```")[0].strip()
                
                task_details = json.loads(content)
                return task_details
                
            except json.JSONDecodeError as e:
                logger.error(f"Failed to parse Kimi task details: {e}")
                return self._get_default_task_details(task_type)
        else:
            logger.error(f"Kimi task generation failed: {response.error_message}")
            return self._get_default_task_details(task_type)

    def _get_default_task_details(self, task_type: TaskType) -> Dict[str, Any]:
        """
        Get default task details when Kimi is unavailable
        
        Args:
            task_type: Type of task
            
        Returns:
            Default task details
        """
        defaults = {
            TaskType.CODE_IMPLEMENTATION: {
                "title": "Implement code changes",
                "description": "Implement the required code changes based on requirements",
                "estimated_duration": 60,
                "required_skills": ["programming", "problem_solving"],
                "key_considerations": ["Code quality", "Testing", "Documentation"],
                "potential_pitfalls": ["Scope creep", "Technical debt", "Integration issues"]
            },
            TaskType.TESTING: {
                "title": "Execute tests",
                "description": "Create and execute tests to validate functionality",
                "estimated_duration": 30,
                "required_skills": ["testing", "debugging"],
                "key_considerations": ["Test coverage", "Edge cases", "Performance"],
                "potential_pitfalls": ["Incomplete test coverage", "Flaky tests", "Environment issues"]
            },
            TaskType.ANALYSIS: {
                "title": "Analyze requirements",
                "description": "Analyze requirements and create implementation plan",
                "estimated_duration": 45,
                "required_skills": ["analysis", "problem_solving"],
                "key_considerations": ["Requirements clarity", "Technical feasibility", "Resource availability"],
                "potential_pitfalls": ["Ambiguous requirements", "Technical constraints", "Scope misunderstanding"]
            }
        }
        
        return defaults.get(task_type, defaults[TaskType.CODE_IMPLEMENTATION])

    def _determine_task_priority(
        self,
        parsed_goal: ParsedGoal,
        index: int,
        total_tasks: int
    ) -> TaskPriority:
        """
        Determine task priority based on goal and position
        
        Args:
            parsed_goal: Parsed development goal
            index: Task index
            total_tasks: Total number of tasks
            
        Returns:
            TaskPriority
        """
        # First and last tasks often have higher priority
        if index == 0 or index == total_tasks - 1:
            return TaskPriority.HIGH
        
        # Check if goal has high-priority indicators
        high_priority_indicators = ["urgent", "critical", "important", "asap", "blocking"]
        goal_desc_lower = parsed_goal.original_description.lower()
        
        if any(indicator in goal_desc_lower for indicator in high_priority_indicators):
            return TaskPriority.HIGH
        
        # Default to medium priority
        return TaskPriority.MEDIUM

    def _determine_safety_level(self, task_type: TaskType, parsed_goal: ParsedGoal) -> SafetyLevel:
        """
        Determine safety level for a task
        
        Args:
            task_type: Type of task
            parsed_goal: Parsed development goal
            
        Returns:
            SafetyLevel
        """
        # High-risk task types
        if task_type in [TaskType.DEPLOYMENT, TaskType.CODE_REFACTORING]:
            # Check for critical systems
            critical_indicators = ["production", "critical", "core", "infrastructure"]
            goal_desc_lower = parsed_goal.original_description.lower()
            
            if any(indicator in goal_desc_lower for indicator in critical_indicators):
                return SafetyLevel.CRITICAL
            
            return SafetyLevel.HIGH_RISK
        
        # Medium-risk task types
        if task_type in [TaskType.CODE_IMPLEMENTATION, TaskType.INTEGRATION]:
            return SafetyLevel.MEDIUM_RISK
        
        # Low-risk task types
        if task_type in [TaskType.ANALYSIS, TaskType.RESEARCH, TaskType.DOCUMENTATION]:
            return SafetyLevel.LOW_RISK
        
        # Default to medium risk
        return SafetyLevel.MEDIUM_RISK

    def _extract_required_files(self, parsed_goal: ParsedGoal) -> List[str]:
        """
        Extract required files from parsed goal
        
        Args:
            parsed_goal: Parsed development goal
            
        Returns:
            List of required file paths
        """
        required_files = []
        
        # Add files mentioned in requirements
        for requirement in parsed_goal.requirements:
            # Simple extraction - in production, use more sophisticated parsing
            if "file" in requirement.description.lower() or "." in requirement.description:
                # Extract potential file paths
                words = requirement.description.split()
                for word in words:
                    if "." in word and len(word) > 3:  # Likely a file reference
                        required_files.append(word)
        
        # Remove duplicates
        return list(set(required_files))

    def _create_execution_steps(self, task_type: TaskType, task_details: Dict[str, Any]) -> List[ExecutionStep]:
        """
        Create execution steps for a task
        
        Args:
            task_type: Type of task
            task_details: Task details from Kimi
            
        Returns:
            List of execution steps
        """
        steps = []
        
        # Common step templates
        templates = {
            TaskType.CODE_IMPLEMENTATION: [
                {
                    "step_number": 1,
                    "action": "analyze_requirements",
                    "description": "Analyze requirements and understand what needs to be implemented",
                    "expected_output": "Clear understanding of requirements",
                    "verification_method": "Requirements document reviewed",
                    "timeout_seconds": 300
                },
                {
                    "step_number": 2,
                    "action": "design_solution",
                    "description": "Design the solution architecture and implementation approach",
                    "expected_output": "Implementation plan",
                    "verification_method": "Design reviewed and approved",
                    "timeout_seconds": 600
                },
                {
                    "step_number": 3,
                    "action": "implement_code",
                    "description": "Implement the code changes according to the design",
                    "expected_output": "Working code implementation",
                    "verification_method": "Code compiles without errors",
                    "timeout_seconds": 1800,
                    "rollback_action": "Revert code changes"
                },
                {
                    "step_number": 4,
                    "action": "test_implementation",
                    "description": "Test the implementation to ensure it works correctly",
                    "expected_output": "Tests pass successfully",
                    "verification_method": "All tests pass",
                    "timeout_seconds": 600
                }
            ],
            TaskType.TESTING: [
                {
                    "step_number": 1,
                    "action": "identify_test_cases",
                    "description": "Identify test cases based on requirements",
                    "expected_output": "Test case list",
                    "verification_method": "Test cases reviewed",
                    "timeout_seconds": 300
                },
                {
                    "step_number": 2,
                    "action": "create_test_scripts",
                    "description": "Create automated test scripts",
                    "expected_output": "Test scripts created",
                    "verification_method": "Scripts compile/run",
                    "timeout_seconds": 600
                },
                {
                    "step_number": 3,
                    "action": "execute_tests",
                    "description": "Execute tests and collect results",
                    "expected_output": "Test results",
                    "verification_method": "Results analyzed",
                    "timeout_seconds": 900
                }
            ],
            TaskType.ANALYSIS: [
                {
                    "step_number": 1,
                    "action": "gather_information",
                    "description": "Gather all relevant information and context",
                    "expected_output": "Information collected",
                    "verification_method": "Complete information gathered",
                    "timeout_seconds": 300
                },
                {
                    "step_number": 2,
                    "action": "analyze_data",
                    "description": "Analyze the gathered information",
                    "expected_output": "Analysis results",
                    "verification_method": "Analysis complete",
                    "timeout_seconds": 600
                },
                {
                    "step_number": 3,
                    "action": "document_findings",
                    "description": "Document analysis findings and recommendations",
                    "expected_output": "Analysis report",
                    "verification_method": "Report reviewed",
                    "timeout_seconds": 300
                }
            ]
        }
        
        # Get steps for task type
        step_data_list = templates.get(task_type, templates[TaskType.CODE_IMPLEMENTATION])
        
        # Convert to ExecutionStep objects
        for step_data in step_data_list:
            step = ExecutionStep(**step_data)
            steps.append(step)
        
        return steps

    def _create_success_criteria(self, task_type: TaskType, parsed_goal: ParsedGoal) -> List[str]:
        """
        Create success criteria for a task
        
        Args:
            task_type: Type of task
            parsed_goal: Parsed development goal
            
        Returns:
            List of success criteria
        """
        criteria = []
        
        # Base criteria for all tasks
        criteria.extend([
            "Task completed without critical errors",
            "All execution steps completed successfully",
            "Results meet quality standards"
        ])
        
        # Type-specific criteria
        if task_type == TaskType.CODE_IMPLEMENTATION:
            criteria.extend([
                "Code compiles without errors",
                "Code follows project style guidelines",
                "Basic functionality tested and working"
            ])
        elif task_type == TaskType.TESTING:
            criteria.extend([
                "All test cases executed",
                "Test results documented",
                "Issues identified and logged"
            ])
        elif task_type == TaskType.ANALYSIS:
            criteria.extend([
                "Analysis is thorough and complete",
                "Findings are well-documented",
                "Recommendations are actionable"
            ])
        
        # Add goal-specific criteria
        if parsed_goal.success_criteria:
            criteria.extend(parsed_goal.success_criteria[:2])  # Add up to 2 goal criteria
        
        return criteria

    def _create_failure_conditions(self, task_type: TaskType) -> List[str]:
        """
        Create failure conditions for a task
        
        Args:
            task_type: Type of task
            
        Returns:
            List of failure conditions
        """
        conditions = [
            "Critical error occurs during execution",
            "Task exceeds timeout duration",
            "System becomes unresponsive",
            "Data corruption detected"
        ]
        
        if task_type == TaskType.CODE_IMPLEMENTATION:
            conditions.extend([
                "Code fails to compile",
                "Basic functionality is broken",
                "Existing features are negatively impacted"
            ])
        elif task_type == TaskType.TESTING:
            conditions.extend([
                "Test environment is unavailable",
                "Test data is corrupted or missing"
            ])
        
        return conditions

    def _create_rollback_plan(self, safety_level: SafetyLevel, task_type: TaskType) -> Optional[str]:
        """
        Create rollback plan for a task
        
        Args:
            safety_level: Safety level of task
            task_type: Type of task
            
        Returns:
            Rollback plan or None if not needed
        """
        # Only create rollback plans for medium risk and above
        if safety_level in [SafetyLevel.LOW_RISK, SafetyLevel.SAFE]:
            return None
        
        rollback_plans = {
            TaskType.CODE_IMPLEMENTATION: "Revert all code changes using version control (git revert)",
            TaskType.CODE_REFACTORING: "Restore from backup or revert refactoring changes",
            TaskType.DEPLOYMENT: "Rollback deployment to previous stable version",
            TaskType.INTEGRATION: "Disable integration and restore previous configuration",
            TaskType.FILE_MODIFICATION: "Restore original files from backup"
        }
        
        return rollback_plans.get(task_type, "Restore system to previous state using backups")

    def _add_task_dependencies(self, tasks: List[TaskSpecification]) -> List[TaskSpecification]:
        """
        Add dependency information between tasks
        
        Args:
            tasks: List of task specifications
            
        Returns:
            Tasks with dependencies set
        """
        # Simple dependency logic: sequential tasks depend on previous ones
        for i, task in enumerate(tasks):
            if i > 0:
                # Depend on previous task
                task.dependencies.append(tasks[i-1].task_id)
            
            # Add additional logic for specific task type dependencies
            if task.task_type == TaskType.TESTING:
                # Testing tasks depend on implementation tasks
                impl_tasks = [t.task_id for t in tasks if t.task_type == TaskType.CODE_IMPLEMENTATION]
                task.dependencies.extend(impl_tasks)
            
            elif task.task_type == TaskType.VALIDATION:
                # Validation depends on testing
                test_tasks = [t.task_id for t in tasks if t.task_type == TaskType.TESTING]
                task.dependencies.extend(test_tasks)
        
        return tasks

    def convert_to_ai_control_task(self, task_spec: TaskSpecification) -> Task:
        """
        Convert TaskSpecification to AI Control Layer Task
        
        Args:
            task_spec: Task specification
            
        Returns:
            AI Control Layer Task
        """
        return Task(
            task_id=task_spec.task_id,
            title=task_spec.title,
            description=task_spec.description,
            task_type=task_spec.task_type.value,
            priority=task_spec.priority,
            safety_level=task_spec.safety_level,
            dependencies=task_spec.dependencies,
            required_files=task_spec.required_files,
            success_criteria=task_spec.success_criteria,
            execution_steps=[step.to_dict() for step in task_spec.execution_steps]
        )


# ============================================================================
# TESTING AND USAGE EXAMPLES
# ============================================================================

def test_task_generator():
    """Test the Task Generator"""
    try:
        print("Testing Task Generator")
        print("=" * 50)
        
        # Initialize generator
        generator = TaskGenerator()
        
        # Create a sample parsed goal
        from natural_language_parser import ParsedGoal, GoalType
        
        sample_goal = ParsedGoal(
            goal_id="test_goal_001",
            original_description="Fix the bug where ships don't spawn correctly in multiplayer",
            goal_type=GoalType.BUG_FIX,
            summary="Fix multiplayer ship spawning bug",
            estimated_complexity="moderate",
            success_criteria=["Ships spawn correctly in multiplayer", "No regression in single player"]
        )
        
        # Generate tasks
        print("\n1. Generating tasks from parsed goal...")
        task_specs = generator.generate_tasks_from_goal(sample_goal)
        
        print(f"✓ Generated {len(task_specs)} tasks:")
        for i, task_spec in enumerate(task_specs, 1):
            print(f"   {i}. {task_spec.title} ({task_spec.task_type.value}) - Priority: {task_spec.priority.name}")
            print(f"      Safety: {task_spec.safety_level.value}, Duration: {task_spec.estimated_duration_minutes}min")
            print(f"      Steps: {len(task_spec.execution_steps)}")
        
        # Test task conversion
        print("\n2. Testing task conversion to AI Control Layer format...")
        if task_specs:
            ai_task = generator.convert_to_ai_control_task(task_specs[0])
            print(f"✓ Converted task: {ai_task.title}")
            print(f"   Status: {ai_task.status.value}")
            print(f"   Dependencies: {len(ai_task.dependencies)}")
        
        # Test with different goal types
        print("\n3. Testing different goal types...")
        
        goal_types_to_test = [
            (GoalType.PERFORMANCE_OPTIMIZATION, "Optimize the physics calculations for better performance"),
            (GoalType.FEATURE_IMPLEMENTATION, "Add a new flight assist mode for docking maneuvers")
        ]
        
        for goal_type, description in goal_types_to_test:
            test_goal = ParsedGoal(
                goal_id=f"test_goal_{goal_type.value}",
                original_description=description,
                goal_type=goal_type,
                summary=f"Test {goal_type.value}",
                estimated_complexity="moderate"
            )
            
            tasks = generator.generate_tasks_from_goal(test_goal)
            print(f"   ✓ {goal_type.value}: {len(tasks)} tasks generated")
        
        print("\n" + "=" * 50)
        print("✓ All task generator tests passed!")
        return True
        
    except Exception as e:
        print(f"✗ Task generator test failed: {e}")
        import traceback
        traceback.print_exc()
        return False


if __name__ == "__main__":
    success = test_task_generator()
    exit(0 if success else 1)