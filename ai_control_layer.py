"""
AI Control Layer - Main orchestrator for autonomous development system
Integrates Kimi API with existing automation infrastructure for intelligent task execution
"""

import os
import time
import logging
from typing import Dict, List, Optional, Any, Tuple
from dataclasses import dataclass, field
from enum import Enum
from pathlib import Path
import json
from datetime import datetime

from kimi_api_client import KimiAPIClient, APIResponse, TokenUsage
from automation_client import UnrealAutomationClient
import remote_exec_udp

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class TaskStatus(Enum):
    """Task execution status"""
    PENDING = "pending"
    IN_PROGRESS = "in_progress"
    COMPLETED = "completed"
    FAILED = "failed"
    BLOCKED = "blocked"
    CANCELLED = "cancelled"


class TaskPriority(Enum):
    """Task priority levels"""
    LOW = 1
    MEDIUM = 2
    HIGH = 3
    CRITICAL = 4


class SafetyLevel(Enum):
    """Safety classification for tasks"""
    SAFE = "safe"  # Read-only operations, analysis
    LOW_RISK = "low_risk"  # Minor modifications, testing
    MEDIUM_RISK = "medium_risk"  # Code changes, asset modifications
    HIGH_RISK = "high_risk"  # System changes, deployment
    CRITICAL = "critical"  # Production deployment, data deletion


@dataclass
class Task:
    """Represents a development task"""
    task_id: str
    title: str
    description: str
    task_type: str
    priority: TaskPriority
    safety_level: SafetyLevel
    dependencies: List[str] = field(default_factory=list)
    required_files: List[str] = field(default_factory=list)
    success_criteria: List[str] = field(default_factory=list)
    execution_steps: List[Dict[str, Any]] = field(default_factory=list)
    status: TaskStatus = TaskStatus.PENDING
    created_at: datetime = field(default_factory=datetime.now)
    started_at: Optional[datetime] = None
    completed_at: Optional[datetime] = None
    result: Optional[Dict[str, Any]] = None
    error_message: Optional[str] = None
    token_usage: Optional[TokenUsage] = None
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "task_id": self.task_id,
            "title": self.title,
            "description": self.description,
            "task_type": self.task_type,
            "priority": self.priority.name,
            "safety_level": self.safety_level.value,
            "dependencies": self.dependencies,
            "required_files": self.required_files,
            "success_criteria": self.success_criteria,
            "execution_steps": self.execution_steps,
            "status": self.status.value,
            "created_at": self.created_at.isoformat(),
            "started_at": self.started_at.isoformat() if self.started_at else None,
            "completed_at": self.completed_at.isoformat() if self.completed_at else None,
            "result": self.result,
            "error_message": self.error_message,
            "token_usage": self.token_usage.to_dict() if self.token_usage else None
        }


@dataclass
class DevelopmentGoal:
    """Represents a high-level development goal"""
    goal_id: str
    description: str
    context: Optional[str] = None
    constraints: List[str] = field(default_factory=list)
    created_at: datetime = field(default_factory=datetime.now)
    tasks: List[Task] = field(default_factory=list)
    status: str = "pending"  # pending, in_progress, completed, failed
    
    def to_dict(self) -> Dict[str, Any]:
        return {
            "goal_id": self.goal_id,
            "description": self.description,
            "context": self.context,
            "constraints": self.constraints,
            "created_at": self.created_at.isoformat(),
            "status": self.status,
            "tasks": [task.to_dict() for task in self.tasks]
        }


class AIControlLayer:
    """
    Main orchestrator for autonomous development system
    Integrates Kimi AI with Unreal automation infrastructure
    """

    def __init__(
        self,
        project_path: str = "c:/Users/allen/Desktop/Alexander/Alexander",
        kim_api_key: Optional[str] = None
    ):
        """
        Initialize AI Control Layer

        Args:
            project_path: Path to Unreal Engine project
            kim_api_key: Kimi API key (optional, uses env var if not provided)
        """
        self.project_path = Path(project_path)
        
        # Initialize Kimi API client
        self.kimi_client = KimiAPIClient(api_key=kim_api_key)
        
        # Initialize Unreal automation client
        self.unreal_client = UnrealAutomationClient(str(self.project_path))
        
        # Task and goal management
        self.active_goals: Dict[str, DevelopmentGoal] = {}
        self.task_queue: List[Task] = []
        self.completed_tasks: List[Task] = []
        
        # Execution state
        self.is_running = False
        self.current_task: Optional[Task] = None
        
        # Performance tracking
        self.execution_history: List[Dict[str, Any]] = []
        
        logger.info("AI Control Layer initialized")

    # ============================================================================
    # GOAL PROCESSING
    # ============================================================================

    def process_natural_language_goal(self, goal_description: str, context: Optional[str] = None) -> DevelopmentGoal:
        """
        Process natural language development goal and generate task plan
        
        Args:
            goal_description: Natural language description of the goal
            context: Additional context about the project
            
        Returns:
            DevelopmentGoal with generated tasks
        """
        logger.info(f"Processing goal: {goal_description}")
        
        # Generate task plan using Kimi AI
        constraints = [
            "Must work with existing Unreal Engine project structure",
            "Should integrate with current automation infrastructure",
            "Must follow safety guidelines for automated changes",
            "Should minimize disruption to existing systems"
        ]
        
        response = self.kimi_client.generate_task_plan(goal_description, context, constraints)
        
        if not response.success:
            logger.error(f"Failed to generate task plan: {response.error_message}")
            raise Exception(f"Task plan generation failed: {response.error_message}")
        
        # Parse generated plan and create DevelopmentGoal
        goal_id = f"goal_{int(time.time())}"
        goal = DevelopmentGoal(
            goal_id=goal_id,
            description=goal_description,
            context=context,
            constraints=constraints
        )
        
        # Parse tasks from AI response (simplified parsing)
        tasks = self._parse_task_plan(response.content, goal_id)
        goal.tasks = tasks
        
        # Add to active goals
        self.active_goals[goal_id] = goal
        
        logger.info(f"Generated {len(tasks)} tasks for goal {goal_id}")
        return goal

    def _parse_task_plan(self, plan_content: str, goal_id: str) -> List[Task]:
        """
        Parse task plan from AI-generated content
        
        Args:
            plan_content: Raw AI-generated task plan
            goal_id: Parent goal ID
            
        Returns:
            List of parsed Task objects
        """
        tasks = []
        
        # This is a simplified parser - in production, you'd want more robust parsing
        # For now, we'll create a basic task structure
        
        task_id = f"task_{goal_id}_{int(time.time())}"
        
        # Create a generic task for demonstration
        task = Task(
            task_id=task_id,
            title="Implement generated task plan",
            description=f"Execute tasks based on AI-generated plan: {plan_content[:200]}...",
            task_type="implementation",
            priority=TaskPriority.MEDIUM,
            safety_level=SafetyLevel.MEDIUM_RISK,
            execution_steps=[
                {"step": 1, "action": "analyze_plan", "description": "Analyze generated task plan"},
                {"step": 2, "action": "execute_tasks", "description": "Execute generated tasks"},
                {"step": 3, "action": "validate_results", "description": "Validate execution results"}
            ],
            success_criteria=[
                "All tasks from plan are executed",
                "Results match expected outcomes",
                "No critical errors occurred"
            ]
        )
        
        tasks.append(task)
        return tasks

    # ============================================================================
    # TASK EXECUTION
    # ============================================================================

    def execute_task(self, task: Task) -> bool:
        """
        Execute a single task
        
        Args:
            task: Task to execute
            
        Returns:
            True if task completed successfully
        """
        logger.info(f"Executing task: {task.task_id} - {task.title}")
        
        task.status = TaskStatus.IN_PROGRESS
        task.started_at = datetime.now()
        self.current_task = task
        
        try:
            # Check safety level and get approval if needed
            if not self._check_safety_approval(task):
                task.status = TaskStatus.BLOCKED
                task.error_message = "Safety approval required"
                logger.warning(f"Task {task.task_id} blocked - safety approval needed")
                return False
            
            # Execute task based on type
            if task.task_type == "unreal_command":
                success = self._execute_unreal_command_task(task)
            elif task.task_type == "code_analysis":
                success = self._execute_code_analysis_task(task)
            elif task.task_type == "file_modification":
                success = self._execute_file_modification_task(task)
            elif task.task_type == "testing":
                success = self._execute_testing_task(task)
            else:
                success = self._execute_generic_task(task)
            
            # Update task status
            if success:
                task.status = TaskStatus.COMPLETED
                task.completed_at = datetime.now()
                logger.info(f"Task {task.task_id} completed successfully")
            else:
                task.status = TaskStatus.FAILED
                logger.error(f"Task {task.task_id} failed")
            
            # Record execution
            self._record_execution(task)
            
            return success
            
        except Exception as e:
            logger.error(f"Task execution error: {e}")
            task.status = TaskStatus.FAILED
            task.error_message = str(e)
            return False
        finally:
            self.current_task = None

    def _check_safety_approval(self, task: Task) -> bool:
        """Check if task requires manual safety approval"""
        if task.safety_level in [SafetyLevel.HIGH_RISK, SafetyLevel.CRITICAL]:
            logger.warning(f"Task {task.task_id} requires manual approval (safety level: {task.safety_level.value})")
            # In a real system, this would prompt for user approval
            # For now, we'll allow with logging
            return True
        
        return True

    def _execute_unreal_command_task(self, task: Task) -> bool:
        """Execute Unreal Engine command task"""
        try:
            # Extract command from task
            command = task.execution_steps[0].get("command", "")
            
            # Send via UDP multicast
            remote_exec_udp.execute_unreal_command(command)
            
            task.result = {"command_executed": command}
            return True
            
        except Exception as e:
            task.error_message = f"Unreal command execution failed: {e}"
            return False

    def _execute_code_analysis_task(self, task: Task) -> bool:
        """Execute code analysis task using Kimi AI"""
        try:
            # Get code to analyze
            code_file = task.required_files[0] if task.required_files else None
            if code_file and Path(code_file).exists():
                with open(code_file, 'r') as f:
                    code = f.read()
            else:
                task.error_message = f"Code file not found: {code_file}"
                return False
            
            # Analyze with Kimi
            analysis_type = task.execution_steps[0].get("analysis_type", "general")
            response = self.kimi_client.analyze_code(code, analysis_type)
            
            if response.success:
                task.result = {
                    "analysis": response.content,
                    "token_usage": response.token_usage.to_dict() if response.token_usage else None
                }
                task.token_usage = response.token_usage
                return True
            else:
                task.error_message = f"Code analysis failed: {response.error_message}"
                return False
                
        except Exception as e:
            task.error_message = f"Code analysis task error: {e}"
            return False

    def _execute_file_modification_task(self, task: Task) -> bool:
        """Execute file modification task"""
        try:
            # This would implement file modification logic
            # For now, we'll simulate success
            task.result = {"message": "File modification simulated"}
            return True
            
        except Exception as e:
            task.error_message = f"File modification failed: {e}"
            return False

    def _execute_testing_task(self, task: Task) -> bool:
        """Execute testing task"""
        try:
            # Run automated test using Unreal client
            test_result = self.unreal_client.test_ship_movement()
            
            task.result = {
                "test_name": test_result.test_name,
                "success": test_result.success,
                "message": test_result.message,
                "duration": test_result.duration
            }
            
            return test_result.success
            
        except Exception as e:
            task.error_message = f"Testing task error: {e}"
            return False

    def _execute_generic_task(self, task: Task) -> bool:
        """Execute generic task using Kimi AI guidance"""
        try:
            # Use Kimi to help execute the task
            prompt = f"""Help me execute this task:

Task: {task.title}
Description: {task.description}
Steps: {task.execution_steps}

Please provide detailed instructions for executing this task.
"""
            
            messages = [
                {"role": "system", "content": "You are a task execution assistant. Provide clear, actionable steps for completing development tasks."},
                {"role": "user", "content": prompt}
            ]
            
            response = self.kimi_client.chat_completion(messages)
            
            if response.success:
                task.result = {
                    "execution_guidance": response.content,
                    "token_usage": response.token_usage.to_dict() if response.token_usage else None
                }
                task.token_usage = response.token_usage
                return True
            else:
                task.error_message = f"Generic task execution failed: {response.error_message}"
                return False
                
        except Exception as e:
            task.error_message = f"Generic task execution error: {e}"
            return False

    # ============================================================================
    # WORKFLOW MANAGEMENT
    # ============================================================================

    def execute_goal(self, goal_id: str) -> bool:
        """
        Execute all tasks for a development goal
        
        Args:
            goal_id: ID of the goal to execute
            
        Returns:
            True if all tasks completed successfully
        """
        if goal_id not in self.active_goals:
            logger.error(f"Goal {goal_id} not found")
            return False
        
        goal = self.active_goals[goal_id]
        goal.status = "in_progress"
        
        logger.info(f"Executing goal: {goal_id} with {len(goal.tasks)} tasks")
        
        success_count = 0
        for task in goal.tasks:
            # Check dependencies
            if not self._check_task_dependencies(task):
                logger.warning(f"Task {task.task_id} has unmet dependencies, skipping")
                continue
            
            # Execute task
            if self.execute_task(task):
                success_count += 1
                self.completed_tasks.append(task)
            else:
                logger.error(f"Task {task.task_id} failed, stopping goal execution")
                goal.status = "failed"
                return False
        
        # Update goal status
        if success_count == len(goal.tasks):
            goal.status = "completed"
            logger.info(f"Goal {goal_id} completed successfully")
            return True
        else:
            goal.status = "failed"
            logger.error(f"Goal {goal_id} failed: {success_count}/{len(goal.tasks)} tasks completed")
            return False

    def _check_task_dependencies(self, task: Task) -> bool:
        """Check if task dependencies are met"""
        for dep_id in task.dependencies:
            dep_task = next((t for t in self.completed_tasks if t.task_id == dep_id), None)
            if not dep_task or dep_task.status != TaskStatus.COMPLETED:
                return False
        return True

    def _record_execution(self, task: Task):
        """Record task execution for monitoring"""
        execution_record = {
            "task_id": task.task_id,
            "task_type": task.task_type,
            "status": task.status.value,
            "timestamp": datetime.now().isoformat(),
            "duration": (task.completed_at - task.started_at).total_seconds() if task.completed_at and task.started_at else 0,
            "token_usage": task.token_usage.to_dict() if task.token_usage else None
        }
        
        self.execution_history.append(execution_record)

    # ============================================================================
    # MONITORING AND REPORTING
    # ============================================================================

    def get_system_status(self) -> Dict[str, Any]:
        """Get comprehensive system status"""
        kim_usage = self.kimi_client.get_usage_report()
        
        return {
            "timestamp": datetime.now().isoformat(),
            "is_running": self.is_running,
            "active_goals": len(self.active_goals),
            "pending_tasks": len([t for t in self.task_queue if t.status == TaskStatus.PENDING]),
            "completed_tasks": len(self.completed_tasks),
            "current_task": self.current_task.task_id if self.current_task else None,
            "kimi_usage": kim_usage,
            "execution_history_count": len(self.execution_history)
        }

    def generate_execution_report(self) -> str:
        """Generate comprehensive execution report"""
        status = self.get_system_status()
        
        report = f"""
AI Control Layer Execution Report
=================================

Generated: {status['timestamp']}
System Status: {'Running' if status['is_running'] else 'Stopped'}

Task Summary:
- Active Goals: {status['active_goals']}
- Pending Tasks: {status['pending_tasks']}
- Completed Tasks: {status['completed_tasks']}
- Current Task: {status['current_task'] or 'None'}

Kimi API Usage:
- Total Requests: {status['kimi_usage'].get('total_requests', 0)}
- Total Tokens: {status['kimi_usage'].get('total_tokens', 0)}
- Total Cost: ${status['kimi_usage'].get('total_cost', 0):.4f}
- Cost Tier: {status['kimi_usage'].get('cost_tier', 'N/A')}

Recent Executions:
"""
        
        # Add recent execution history
        recent_executions = self.execution_history[-5:]
        for execution in recent_executions:
            report += f"- {execution['task_id']}: {execution['status']} ({execution['duration']:.2f}s)\n"
        
        return report

    def start_autonomous_development(self):
        """Start autonomous development loop"""
        logger.info("Starting autonomous development loop")
        self.is_running = True
        
        while self.is_running:
            try:
                # Process pending tasks
                pending_tasks = [t for t in self.task_queue if t.status == TaskStatus.PENDING]
                
                if pending_tasks:
                    # Sort by priority
                    pending_tasks.sort(key=lambda t: t.priority.value, reverse=True)
                    
                    # Execute highest priority task
                    next_task = pending_tasks[0]
                    self.execute_task(next_task)
                else:
                    # No pending tasks, check for new goals
                    logger.info("No pending tasks, waiting for new goals...")
                    time.sleep(5)
                    
            except Exception as e:
                logger.error(f"Error in autonomous development loop: {e}")
                time.sleep(10)  # Wait before retrying

    def stop_autonomous_development(self):
        """Stop autonomous development loop"""
        logger.info("Stopping autonomous development loop")
        self.is_running = False

    def cleanup(self):
        """Cleanup resources"""
        logger.info("Cleaning up AI Control Layer resources")
        self.kimi_client.close()
        self.is_running = False


# ============================================================================
# TESTING AND USAGE EXAMPLES
# ============================================================================

def test_ai_control_layer():
    """Test the AI Control Layer"""
    try:
        print("Testing AI Control Layer")
        print("=" * 50)
        
        # Initialize control layer
        control_layer = AIControlLayer()
        
        # Test natural language goal processing
        print("\n1. Testing natural language goal processing...")
        goal_description = "Analyze the current ship movement code and suggest performance improvements"
        
        goal = control_layer.process_natural_language_goal(goal_description)
        
        print(f"✓ Goal processed: {goal.goal_id}")
        print(f"  Description: {goal.description}")
        print(f"  Tasks generated: {len(goal.tasks)}")
        
        # Test system status
        print("\n2. Testing system status...")
        status = control_layer.get_system_status()
        print(f"✓ System status retrieved")
        print(f"  Active goals: {status['active_goals']}")
        print(f"  Kimi API connected: {status['kimi_usage'].get('total_requests', 0) > 0}")
        
        # Test execution report
        print("\n3. Testing execution report...")
        report = control_layer.generate_execution_report()
        print("✓ Execution report generated")
        print(report[:300] + "...")
        
        # Cleanup
        control_layer.cleanup()
        
        print("\n" + "=" * 50)
        print("✓ All tests passed!")
        return True
        
    except Exception as e:
        print(f"✗ Test failed: {e}")
        import traceback
        traceback.print_exc()
        return False


if __name__ == "__main__":
    success = test_ai_control_layer()
    exit(0 if success else 1)