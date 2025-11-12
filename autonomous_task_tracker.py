"""
Autonomous Task Tracker - Foundation for AI Learning System

Tracks task execution, success/failure patterns, and enables decision improvement.
Part of Phase 1 implementation for AI Agent Autonomous Development Plan.

Usage:
    from autonomous_task_tracker import TaskTracker
    
    tracker = TaskTracker()
    task_id = tracker.start_task("fix_compilation_error", {"file": "MyClass.cpp"})
    tracker.complete_task(task_id, success=True, metrics={"duration": 45})
    
    # Query history
    recent = tracker.get_recent_tasks(limit=10)
    stats = tracker.get_success_rate("fix_compilation_error")
"""

import json
import time
from datetime import datetime, timedelta
from pathlib import Path
from typing import Dict, List, Optional, Any
from dataclasses import dataclass, asdict
from enum import Enum


class TaskStatus(Enum):
    """Task execution status"""
    PENDING = "pending"
    IN_PROGRESS = "in_progress"
    SUCCESS = "success"
    FAILED = "failed"
    ROLLED_BACK = "rolled_back"


@dataclass
class Task:
    """Represents a single autonomous task execution"""
    task_id: str
    task_type: str
    description: str
    parameters: Dict[str, Any]
    status: str
    start_time: str
    end_time: Optional[str] = None
    duration_seconds: Optional[float] = None
    success: Optional[bool] = None
    error_message: Optional[str] = None
    metrics: Optional[Dict[str, Any]] = None
    rollback_performed: bool = False


class TaskTracker:
    """Tracks autonomous task execution for learning and improvement"""
    
    def __init__(self, storage_path: Optional[Path] = None):
        """
        Initialize task tracker
        
        Args:
            storage_path: Path to store task history (default: Saved/AI_TaskHistory)
        """
        if storage_path is None:
            storage_path = Path(__file__).parent / "Saved" / "AI_TaskHistory"
        
        self.storage_path = Path(storage_path)
        self.storage_path.mkdir(parents=True, exist_ok=True)
        
        self.current_tasks: Dict[str, Task] = {}
        self._load_current_tasks()
    
    def _generate_task_id(self, task_type: str) -> str:
        """Generate unique task ID"""
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
        return f"{task_type}_{timestamp}"
    
    def _get_task_file(self, task_id: str) -> Path:
        """Get file path for task storage"""
        date_str = datetime.now().strftime("%Y%m%d")
        return self.storage_path / f"tasks_{date_str}.jsonl"
    
    def _load_current_tasks(self):
        """Load in-progress tasks from today's file"""
        today_file = self._get_task_file("temp")
        if today_file.exists():
            with open(today_file, 'r') as f:
                for line in f:
                    task_dict = json.loads(line.strip())
                    task = Task(**task_dict)
                    if task.status in [TaskStatus.PENDING.value, TaskStatus.IN_PROGRESS.value]:
                        self.current_tasks[task.task_id] = task
    
    def _save_task(self, task: Task):
        """Append task to JSONL file"""
        task_file = self._get_task_file(task.task_id)
        with open(task_file, 'a') as f:
            f.write(json.dumps(asdict(task)) + '\n')
    
    def start_task(
        self, 
        task_type: str, 
        parameters: Dict[str, Any],
        description: str = ""
    ) -> str:
        """
        Start tracking a new task
        
        Args:
            task_type: Type of task (e.g., "fix_compilation_error", "optimize_assets")
            parameters: Task parameters
            description: Human-readable description
            
        Returns:
            task_id: Unique task identifier
        """
        task_id = self._generate_task_id(task_type)
        
        task = Task(
            task_id=task_id,
            task_type=task_type,
            description=description or f"Execute {task_type}",
            parameters=parameters,
            status=TaskStatus.IN_PROGRESS.value,
            start_time=datetime.now().isoformat()
        )
        
        self.current_tasks[task_id] = task
        self._save_task(task)
        
        return task_id
    
    def complete_task(
        self,
        task_id: str,
        success: bool,
        metrics: Optional[Dict[str, Any]] = None,
        error_message: Optional[str] = None
    ):
        """
        Mark task as complete
        
        Args:
            task_id: Task identifier
            success: Whether task succeeded
            metrics: Performance metrics (duration, resources, etc.)
            error_message: Error details if failed
        """
        if task_id not in self.current_tasks:
            raise ValueError(f"Task {task_id} not found in current tasks")
        
        task = self.current_tasks[task_id]
        end_time = datetime.now()
        start_time = datetime.fromisoformat(task.start_time)
        
        task.status = TaskStatus.SUCCESS.value if success else TaskStatus.FAILED.value
        task.success = success
        task.end_time = end_time.isoformat()
        task.duration_seconds = (end_time - start_time).total_seconds()
        task.metrics = metrics or {}
        task.error_message = error_message
        
        self._save_task(task)
        del self.current_tasks[task_id]
    
    def rollback_task(self, task_id: str, reason: str):
        """
        Mark task as rolled back
        
        Args:
            task_id: Task identifier
            reason: Rollback reason
        """
        if task_id not in self.current_tasks:
            raise ValueError(f"Task {task_id} not found")
        
        task = self.current_tasks[task_id]
        task.status = TaskStatus.ROLLED_BACK.value
        task.success = False
        task.rollback_performed = True
        task.error_message = f"Rolled back: {reason}"
        task.end_time = datetime.now().isoformat()
        
        self._save_task(task)
        del self.current_tasks[task_id]
    
    def get_recent_tasks(
        self, 
        limit: int = 100, 
        task_type: Optional[str] = None,
        days_back: int = 7
    ) -> List[Task]:
        """
        Get recent task history
        
        Args:
            limit: Maximum number of tasks to return
            task_type: Filter by task type (optional)
            days_back: How many days of history to search
            
        Returns:
            List of tasks, most recent first
        """
        tasks = []
        
        # Iterate through recent days
        for i in range(days_back):
            date = datetime.now() - timedelta(days=i)
            date_str = date.strftime("%Y%m%d")
            task_file = self.storage_path / f"tasks_{date_str}.jsonl"
            
            if not task_file.exists():
                continue
            
            with open(task_file, 'r') as f:
                for line in f:
                    task_dict = json.loads(line.strip())
                    task = Task(**task_dict)
                    
                    if task_type is None or task.task_type == task_type:
                        tasks.append(task)
                    
                    if len(tasks) >= limit:
                        break
            
            if len(tasks) >= limit:
                break
        
        # Sort by start time, most recent first
        tasks.sort(key=lambda t: t.start_time, reverse=True)
        return tasks[:limit]
    
    def get_success_rate(
        self, 
        task_type: str, 
        days_back: int = 30
    ) -> Dict[str, Any]:
        """
        Calculate success rate for a task type
        
        Args:
            task_type: Type of task to analyze
            days_back: Days of history to analyze
            
        Returns:
            Dictionary with success metrics
        """
        tasks = self.get_recent_tasks(limit=10000, task_type=task_type, days_back=days_back)
        
        if not tasks:
            return {
                "task_type": task_type,
                "total": 0,
                "success": 0,
                "failed": 0,
                "rolled_back": 0,
                "success_rate": 0.0
            }
        
        success_count = sum(1 for t in tasks if t.success)
        failed_count = sum(1 for t in tasks if t.success is False and not t.rollback_performed)
        rollback_count = sum(1 for t in tasks if t.rollback_performed)
        
        return {
            "task_type": task_type,
            "total": len(tasks),
            "success": success_count,
            "failed": failed_count,
            "rolled_back": rollback_count,
            "success_rate": success_count / len(tasks) if tasks else 0.0,
            "avg_duration": sum(t.duration_seconds for t in tasks if t.duration_seconds) / len(tasks)
        }
    
    def get_failure_patterns(
        self, 
        task_type: Optional[str] = None,
        days_back: int = 30
    ) -> List[Dict[str, Any]]:
        """
        Analyze failure patterns to improve decision making
        
        Args:
            task_type: Filter by task type (optional)
            days_back: Days of history to analyze
            
        Returns:
            List of failure patterns with frequency
        """
        tasks = self.get_recent_tasks(limit=10000, task_type=task_type, days_back=days_back)
        failed_tasks = [t for t in tasks if not t.success]
        
        # Group by error message
        error_counts: Dict[str, int] = {}
        error_examples: Dict[str, Task] = {}
        
        for task in failed_tasks:
            error = task.error_message or "Unknown error"
            error_counts[error] = error_counts.get(error, 0) + 1
            if error not in error_examples:
                error_examples[error] = task
        
        # Sort by frequency
        patterns = [
            {
                "error": error,
                "count": count,
                "percentage": count / len(failed_tasks) * 100 if failed_tasks else 0,
                "example_task_id": error_examples[error].task_id,
                "example_parameters": error_examples[error].parameters
            }
            for error, count in sorted(error_counts.items(), key=lambda x: x[1], reverse=True)
        ]
        
        return patterns
    
    def generate_report(self, days_back: int = 7) -> str:
        """
        Generate human-readable report
        
        Args:
            days_back: Days of history to include
            
        Returns:
            Markdown formatted report
        """
        all_tasks = self.get_recent_tasks(limit=10000, days_back=days_back)
        
        if not all_tasks:
            return "# Autonomous Task Report\n\nNo tasks executed in the last {days_back} days."
        
        # Calculate overall stats
        total = len(all_tasks)
        success = sum(1 for t in all_tasks if t.success)
        failed = sum(1 for t in all_tasks if t.success is False and not t.rollback_performed)
        rolled_back = sum(1 for t in all_tasks if t.rollback_performed)
        
        # Group by task type
        task_types = {}
        for task in all_tasks:
            if task.task_type not in task_types:
                task_types[task.task_type] = []
            task_types[task.task_type].append(task)
        
        report = f"""# Autonomous Task Report

**Period**: Last {days_back} days
**Generated**: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}

## Overall Statistics

- **Total Tasks**: {total}
- **Successful**: {success} ({success/total*100:.1f}%)
- **Failed**: {failed} ({failed/total*100:.1f}%)
- **Rolled Back**: {rolled_back} ({rolled_back/total*100:.1f}%)

## By Task Type

"""
        
        for task_type, tasks in sorted(task_types.items()):
            type_success = sum(1 for t in tasks if t.success)
            type_total = len(tasks)
            avg_duration = sum(t.duration_seconds for t in tasks if t.duration_seconds) / type_total
            
            report += f"""### {task_type}
- Count: {type_total}
- Success Rate: {type_success/type_total*100:.1f}%
- Avg Duration: {avg_duration:.1f}s

"""
        
        # Recent failures
        recent_failures = [t for t in all_tasks[:20] if not t.success]
        if recent_failures:
            report += "\n## Recent Failures\n\n"
            for task in recent_failures[:5]:
                report += f"- **{task.task_type}** ({task.start_time}): {task.error_message or 'Unknown error'}\n"
        
        return report


def main():
    """Demo usage"""
    tracker = TaskTracker()
    
    # Simulate some tasks
    print("Simulating autonomous tasks...")
    
    # Success case
    task1 = tracker.start_task(
        "fix_compilation_error",
        {"file": "Source/Alexander/Private/SpaceshipPawn.cpp", "error": "undeclared identifier"},
        "Fix compilation error in SpaceshipPawn"
    )
    time.sleep(0.1)
    tracker.complete_task(task1, success=True, metrics={"lines_changed": 3})
    
    # Failure case
    task2 = tracker.start_task(
        "optimize_assets",
        {"asset_type": "StaticMesh", "quality": "High"},
        "Optimize static meshes"
    )
    time.sleep(0.1)
    tracker.complete_task(
        task2, 
        success=False, 
        error_message="Asset validation failed: 3 corrupted meshes"
    )
    
    # Generate report
    print("\n" + "="*80)
    print(tracker.generate_report(days_back=1))
    print("="*80)
    
    # Show success rates
    print("\nSuccess Rates:")
    for task_type in ["fix_compilation_error", "optimize_assets"]:
        stats = tracker.get_success_rate(task_type, days_back=1)
        if stats['total'] > 0:
            print(f"  {task_type}: {stats['success_rate']*100:.1f}% ({stats['success']}/{stats['total']})")


if __name__ == "__main__":
    main()
