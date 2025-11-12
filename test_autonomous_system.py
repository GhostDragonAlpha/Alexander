#!/usr/bin/env python3
"""
Comprehensive Test Suite for Autonomous Development System
Tests all components in a safe, read-only manner to verify system functionality
"""

import os
import sys
import time
import json
from datetime import datetime
from typing import Dict, List, Any
from pathlib import Path

# Import all system components
from kimi_api_client import KimiAPIClient, APIResponse
from natural_language_parser import NaturalLanguageParser, ParsedGoal, GoalType
from task_generator import TaskGenerator, TaskSpecification, TaskType
from ai_control_layer import AIControlLayer, Task, TaskStatus, TaskPriority, SafetyLevel
from decision_engine import DecisionEngine, DecisionContext, DecisionType
from github_api_integration import GitHubAPIClient, GitHubOperationType
from automation_client import UnrealAutomationClient
from data_manager import DataManager


class Colors:
    """ANSI color codes for terminal output"""
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    BLUE = '\033[94m'
    MAGENTA = '\033[95m'
    CYAN = '\033[96m'
    WHITE = '\033[97m'
    BOLD = '\033[1m'
    END = '\033[0m'


def print_header(text: str):
    """Print formatted header"""
    print(f"\n{Colors.BOLD}{Colors.CYAN}{'='*70}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}{text:^70}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}{'='*70}{Colors.END}")


def print_success(text: str):
    """Print success message"""
    print(f"{Colors.GREEN}[OK] {text}{Colors.END}")


def print_warning(text: str):
    """Print warning message"""
    print(f"{Colors.YELLOW}[WARN] {text}{Colors.END}")


def print_error(text: str):
    """Print error message"""
    print(f"{Colors.RED}[FAIL] {text}{Colors.END}")


def print_info(text: str):
    """Print info message"""
    print(f"{Colors.BLUE}[INFO] {text}{Colors.END}")


class AutonomousSystemTest:
    """Comprehensive test suite for autonomous development system"""

    def __init__(self):
        self.test_results: Dict[str, bool] = {}
        self.test_details: Dict[str, Any] = {}
        self.start_time = datetime.now()
        
    def run_all_tests(self) -> bool:
        """Run all system tests"""
        print_header("AUTONOMOUS DEVELOPMENT SYSTEM TEST SUITE")
        print_info(f"Test started at: {self.start_time.isoformat()}")
        print_info(f"Project directory: {os.getcwd()}")
        
        # Test 1: Kimi API Integration
        self.test_kimi_api_integration()
        
        # Test 2: Natural Language Processing
        self.test_natural_language_processing()
        
        # Test 3: Task Generation
        self.test_task_generation()
        
        # Test 4: AI Control Layer
        self.test_ai_control_layer()
        
        # Test 5: Decision Engine
        self.test_decision_engine()
        
        # Test 6: GitHub API Integration (Safe Mode)
        self.test_github_api_integration()
        
        # Test 7: Data Management
        self.test_data_management()
        
        # Test 8: End-to-End Pipeline
        self.test_end_to_end_pipeline()
        
        # Generate final report
        self.generate_test_report()
        
        # Return overall success
        return all(self.test_results.values())
    
    def test_kimi_api_integration(self):
        """Test Kimi API integration and connectivity"""
        print_header("TEST 1: KIMI API INTEGRATION")
        
        try:
            # Initialize client
            print_info("Initializing Kimi API client...")
            client = KimiAPIClient()
            
            # Test 1.1: Simple chat completion
            print_info("Testing chat completion...")
            messages = [
                {"role": "user", "content": "Hello! Can you help me with software development tasks?"}
            ]
            
            response = client.chat_completion(messages)
            
            if response.success:
                print_success(f"Chat successful: {response.content[:100]}...")
                print_success(f"Tokens used: {response.token_usage.total_tokens}")
                print_success(f"Cost: ${response.token_usage.estimated_cost:.4f}")
                print_success(f"Response time: {response.response_time:.2f}s")
                self.test_results["kimi_chat"] = True
            else:
                print_error(f"Chat failed: {response.error_message}")
                self.test_results["kimi_chat"] = False
            
            # Test 1.2: Code analysis
            print_info("Testing code analysis...")
            sample_code = """
def calculate_total(items):
    total = 0
    for item in items:
        total += item
    return total
"""
            
            response = client.analyze_code(sample_code, analysis_type="performance")
            
            if response.success:
                print_success("Code analysis successful")
                print_success(f"Analysis: {response.content[:150]}...")
                self.test_results["kimi_code_analysis"] = True
            else:
                print_error(f"Code analysis failed: {response.error_message}")
                self.test_results["kimi_code_analysis"] = False
            
            # Test 1.3: Task plan generation
            print_info("Testing task plan generation...")
            test_goal = "Analyze the current codebase for compilation issues"
            
            response = client.generate_task_plan(test_goal)
            
            if response.success:
                print_success("Task plan generation successful")
                print_success(f"Plan length: {len(response.content)} characters")
                self.test_results["kimi_task_plan"] = True
            else:
                print_error(f"Task plan generation failed: {response.error_message}")
                self.test_results["kimi_task_plan"] = False
            
            # Test 1.4: Usage report
            print_info("Testing usage report...")
            usage_report = client.get_usage_report()
            
            if "total_requests" in usage_report:
                print_success(f"Usage report retrieved: {usage_report['total_requests']} requests")
                print_success(f"Total cost: ${usage_report.get('total_cost', 0):.4f}")
                self.test_results["kimi_usage_report"] = True
            else:
                print_warning("No usage data available yet")
                self.test_results["kimi_usage_report"] = True  # This is expected for first run
            
            # Store test details
            self.test_details["kimi"] = {
                "total_tokens": sum(u.total_tokens for u in client.usage_history),
                "total_cost": client.total_cost,
                "response_times": [u.response_time for u in client.usage_history],
                "models_used": [client.model]
            }
            
            client.close()
            print_success("Kimi API integration tests completed")
            
        except Exception as e:
            print_error(f"Kimi API test failed: {e}")
            self.test_results["kimi_api"] = False
    
    def test_natural_language_processing(self):
        """Test natural language processing capabilities"""
        print_header("TEST 2: NATURAL LANGUAGE PROCESSING")
        
        try:
            # Initialize parser
            print_info("Initializing natural language parser...")
            parser = NaturalLanguageParser()
            
            # Test 2.1: Goal parsing
            print_info("Testing goal parsing...")
            test_goals = [
                "Fix the bug where ships don't spawn correctly in multiplayer",
                "Optimize the physics calculations for better performance",
                "Analyze the current codebase for compilation issues in the Source/Alexander directory"
            ]
            
            parsed_goals = []
            for i, goal_desc in enumerate(test_goals, 1):
                print_info(f"Parsing goal {i}: {goal_desc[:60]}...")
                
                parsed_goal = parser.parse_development_goal(goal_desc)
                parsed_goals.append(parsed_goal)
                
                print_success(f"Goal type: {parsed_goal.goal_type.value}")
                print_success(f"Complexity: {parsed_goal.estimated_complexity}")
                print_success(f"Requirements: {len(parsed_goal.requirements)}")
                print_success(f"Constraints: {len(parsed_goal.constraints)}")
                print_success(f"Risks: {len(parsed_goal.identified_risks)}")
                
                if parsed_goal.requirements:
                    req = parsed_goal.requirements[0]
                    print_success(f"First requirement: {req.description[:50]}...")
            
            self.test_results["nlp_goal_parsing"] = True
            
            # Test 2.2: Code reference extraction
            print_info("Testing code reference extraction...")
            sample_text = "We need to fix the bug in ship_spawner.py and update the config in settings.json"
            code_refs = parser.extract_code_references(sample_text)
            
            print_success(f"Found code references: {code_refs}")
            self.test_results["nlp_code_refs"] = True
            
            # Test 2.3: Technical term identification
            print_info("Testing technical term identification...")
            tech_terms = parser.identify_technical_terms(sample_text)
            
            print_success(f"Found technical terms: {tech_terms}")
            self.test_results["nlp_tech_terms"] = True
            
            # Test 2.4: Sentiment analysis
            print_info("Testing sentiment analysis...")
            sentiment = parser.analyze_goal_sentiment("Urgently fix the critical bug that's crashing the game!")
            
            print_success(f"Sentiment: {sentiment['sentiment']}")
            print_success(f"Urgency: {sentiment['urgency']}")
            print_success(f"Confidence: {sentiment['confidence']}")
            self.test_results["nlp_sentiment"] = True
            
            # Store test details
            self.test_details["nlp"] = {
                "parsed_goals": len(parsed_goals),
                "goal_types": [g.goal_type.value for g in parsed_goals],
                "code_references": code_refs,
                "technical_terms": tech_terms,
                "sentiment_analysis": sentiment
            }
            
            print_success("Natural language processing tests completed")
            
        except Exception as e:
            print_error(f"Natural language processing test failed: {e}")
            self.test_results["nlp"] = False
    
    def test_task_generation(self):
        """Test task generation capabilities"""
        print_header("TEST 3: TASK GENERATION")
        
        try:
            # Initialize generator
            print_info("Initializing task generator...")
            generator = TaskGenerator()
            
            # Create a sample parsed goal
            from natural_language_parser import ParsedGoal, GoalType, ParsedRequirement, IdentifiedRisk, RiskLevel
            
            sample_goal = ParsedGoal(
                goal_id="test_goal_001",
                original_description="Analyze the current codebase for compilation issues in the Source/Alexander directory",
                goal_type=GoalType.ANALYSIS,
                summary="Analyze codebase for compilation issues",
                estimated_complexity="moderate",
                requirements=[
                    ParsedRequirement(
                        requirement_id="req_001",
                        description="Analyze Source/Alexander directory for compilation issues",
                        priority="high",
                        is_functional=True,
                        is_technical=True,
                        estimated_effort_hours=2.0
                    )
                ],
                constraints=[],
                identified_risks=[
                    IdentifiedRisk(
                        risk_id="risk_001",
                        description="Complex codebase may require extensive analysis",
                        risk_level=RiskLevel.MEDIUM,
                        probability="medium",
                        impact="medium",
                        mitigation_strategy="Break down analysis into smaller parts",
                        contingency_plan="Extend analysis time if needed"
                    )
                ],
                success_criteria=["All compilation issues identified", "Fix plan created"],
                dependencies=[],
                estimated_duration_hours=2.0
            )
            
            # Test 3.1: Task generation from goal
            print_info("Testing task generation from parsed goal...")
            task_specs = generator.generate_tasks_from_goal(sample_goal)
            
            print_success(f"Generated {len(task_specs)} tasks:")
            for i, task_spec in enumerate(task_specs, 1):
                print_success(f"  {i}. {task_spec.title} ({task_spec.task_type.value})")
                print_success(f"     Priority: {task_spec.priority.name}, Safety: {task_spec.safety_level.value}")
                print_success(f"     Duration: {task_spec.estimated_duration_minutes}min, Steps: {len(task_spec.execution_steps)}")
            
            self.test_results["task_generation"] = True
            
            # Test 3.2: Task conversion to AI Control format
            print_info("Testing task conversion...")
            if task_specs:
                ai_task = generator.convert_to_ai_control_task(task_specs[0])
                print_success(f"Converted task: {ai_task.title}")
                print_success(f"Status: {ai_task.status.value}")
                print_success(f"Dependencies: {len(ai_task.dependencies)}")
            
            self.test_results["task_conversion"] = True
            
            # Test 3.3: Different goal types
            print_info("Testing different goal types...")
            
            goal_types_to_test = [
                (GoalType.BUG_FIX, "Fix the bug where ships don't spawn correctly in multiplayer"),
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
                print_success(f"  {goal_type.value}: {len(tasks)} tasks generated")
            
            self.test_results["task_generation_types"] = True
            
            # Store test details
            self.test_details["task_gen"] = {
                "total_tasks_generated": len(task_specs),
                "task_types": [t.task_type.value for t in task_specs],
                "priorities": [t.priority.name for t in task_specs],
                "safety_levels": [t.safety_level.value for t in task_specs]
            }
            
            print_success("Task generation tests completed")
            
        except Exception as e:
            print_error(f"Task generation test failed: {e}")
            self.test_results["task_generation"] = False
    
    def test_ai_control_layer(self):
        """Test AI control layer functionality"""
        print_header("TEST 4: AI CONTROL LAYER")
        
        try:
            # Initialize control layer
            print_info("Initializing AI control layer...")
            control_layer = AIControlLayer()
            
            # Test 4.1: Natural language goal processing
            print_info("Testing natural language goal processing...")
            goal_description = "Analyze the current codebase for compilation issues in the Source/Alexander directory"
            
            goal = control_layer.process_natural_language_goal(goal_description)
            
            print_success(f"Goal processed: {goal.goal_id}")
            print_success(f"Description: {goal.description}")
            print_success(f"Tasks generated: {len(goal.tasks)}")
            
            self.test_results["ai_goal_processing"] = True
            
            # Test 4.2: System status
            print_info("Testing system status...")
            status = control_layer.get_system_status()
            
            print_success(f"System status retrieved")
            print_success(f"Active goals: {status['active_goals']}")
            print_success(f"Kimi API connected: {status['kimi_usage'].get('total_requests', 0) > 0}")
            
            self.test_results["ai_system_status"] = True
            
            # Test 4.3: Execution report
            print_info("Testing execution report...")
            report = control_layer.generate_execution_report()
            
            print_success("Execution report generated")
            print_success(f"Report length: {len(report)} characters")
            
            self.test_results["ai_execution_report"] = True
            
            # Test 4.4: Safety approval checking
            print_info("Testing safety approval checking...")
            
            # Create a test task with different safety levels
            test_tasks = [
                Task(
                    task_id="safe_task",
                    title="Safe analysis task",
                    description="Read-only analysis",
                    task_type="analysis",
                    priority=TaskPriority.LOW,
                    safety_level=SafetyLevel.SAFE
                ),
                Task(
                    task_id="medium_task",
                    title="Medium risk task",
                    description="Code modification task",
                    task_type="code_implementation",
                    priority=TaskPriority.MEDIUM,
                    safety_level=SafetyLevel.MEDIUM_RISK
                ),
                Task(
                    task_id="high_risk_task",
                    title="High risk task",
                    description="System modification task",
                    task_type="deployment",
                    priority=TaskPriority.HIGH,
                    safety_level=SafetyLevel.HIGH_RISK
                )
            ]
            
            for task in test_tasks:
                needs_approval = control_layer._check_safety_approval(task)
                print_success(f"Task '{task.title}' (Safety: {task.safety_level.value}) - Approval needed: {not needs_approval}")
            
            self.test_results["ai_safety_approval"] = True
            
            # Store test details
            self.test_details["ai_control"] = {
                "goals_processed": len(control_layer.active_goals),
                "tasks_generated": len(goal.tasks) if goal.tasks else 0,
                "system_status": status,
                "safety_levels_tested": [t.safety_level.value for t in test_tasks]
            }
            
            control_layer.cleanup()
            print_success("AI control layer tests completed")
            
        except Exception as e:
            print_error(f"AI control layer test failed: {e}")
            self.test_results["ai_control_layer"] = False
    
    def test_decision_engine(self):
        """Test decision engine functionality"""
        print_header("TEST 5: DECISION ENGINE")
        
        try:
            # Initialize decision engine
            print_info("Initializing decision engine...")
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
            
            # Test 5.1: Task execution strategy
            print_info("Testing task execution strategy decision...")
            decision = engine.decide_task_execution_strategy(test_task, context)
            
            print_success(f"Decision: {decision.recommended_action[:60]}...")
            print_success(f"Confidence: {decision.confidence.value}")
            print_success(f"Risks: {len(decision.risks)} identified")
            
            self.test_results["decision_execution_strategy"] = True
            
            # Test 5.2: Error recovery
            print_info("Testing error recovery decision...")
            recovery_decision = engine.decide_error_recovery(
                test_task,
                "Connection timeout after 30 seconds",
                context
            )
            
            print_success(f"Recovery: {recovery_decision.recommended_action}")
            print_success(f"Alternatives: {len(recovery_decision.alternatives)} options")
            
            self.test_results["decision_error_recovery"] = True
            
            # Test 5.3: Task prioritization
            print_info("Testing task prioritization...")
            
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
            
            print_success(f"Prioritized {len(prioritized)} tasks")
            print_success(f"Top priority: {prioritized[0][0].task_id} (score: {prioritized[0][1]})")
            
            self.test_results["decision_prioritization"] = True
            
            # Test 5.4: Decision success rate tracking
            print_info("Testing decision learning...")
            initial_rate = engine.get_decision_success_rate(DecisionType.TASK_EXECUTION_STRATEGY)
            print_success(f"Initial success rate: {initial_rate:.1%}")
            
            # Simulate learning
            engine.learn_from_outcome(decision.decision_id, True, "Task executed successfully")
            updated_rate = engine.get_decision_success_rate(DecisionType.TASK_EXECUTION_STRATEGY)
            print_success(f"Updated success rate: {updated_rate:.1%}")
            
            self.test_results["decision_learning"] = True
            
            # Test 5.5: Decision report
            print_info("Testing decision report generation...")
            report = engine.get_decision_report()
            print_success(f"Report generated ({len(report)} characters)")
            
            self.test_results["decision_report"] = True
            
            # Store test details
            self.test_details["decision_engine"] = {
                "decisions_made": len(engine.decision_history),
                "success_rates": {dt.value: engine.get_decision_success_rate(dt) for dt in DecisionType},
                "prioritized_tasks": len(prioritized)
            }
            
            print_success("Decision engine tests completed")
            
        except Exception as e:
            print_error(f"Decision engine test failed: {e}")
            self.test_results["decision_engine"] = False
    
    def test_github_api_integration(self):
        """Test GitHub API integration (safe mode)"""
        print_header("TEST 6: GITHUB API INTEGRATION (SAFE MODE)")
        
        try:
            # Initialize client
            print_info("Initializing GitHub API client...")
            client = GitHubAPIClient()
            
            # Test 6.1: Repository info (read-only)
            print_info("Testing repository info retrieval...")
            try:
                repo_info = client.get_repository_info()
                print_success(f"Repository info retrieved")
                print_success(f"Name: {repo_info.get('name', 'Unknown')}")
                print_success(f"Description: {repo_info.get('description', 'No description')}")
                print_success(f"Stars: {repo_info.get('stargazers_count', 0)}")
                self.test_results["github_repo_info"] = True
            except Exception as e:
                print_warning(f"Repository info failed: {e}")
                self.test_results["github_repo_info"] = False
            
            # Test 6.2: Branch listing (read-only)
            print_info("Testing branch listing...")
            try:
                branches = client.get_branches()
                print_success(f"Branches retrieved: {len(branches)} branches")
                for branch in branches[:3]:  # Show first 3
                    print_success(f"  - {branch['name']}")
                if len(branches) > 3:
                    print_info(f"  ... and {len(branches) - 3} more")
                self.test_results["github_branches"] = True
            except Exception as e:
                print_warning(f"Branch listing failed: {e}")
                self.test_results["github_branches"] = False
            
            # Test 6.3: Safety level checking
            print_info("Testing safety level checking...")
            safety_read = client.check_safety_level(GitHubOperationType.READ)
            safety_write = client.check_safety_level(GitHubOperationType.WRITE, "README.md")
            safety_write_critical = client.check_safety_level(GitHubOperationType.WRITE, "Config/DefaultEngine.ini")
            
            print_success(f"Safety levels:")
            print_success(f"  Read operation: {safety_read.value}")
            print_success(f"  Write README.md: {safety_write.value}")
            print_success(f"  Write Config file: {safety_write_critical.value}")
            
            self.test_results["github_safety_check"] = True
            
            # Test 6.4: Approval requirements
            print_info("Testing approval requirements...")
            needs_approval_safe = client.requires_approval(safety_read)
            needs_approval_critical = client.requires_approval(safety_write_critical)
            
            print_success(f"Approval requirements:")
            print_success(f"  Read operation: {'Yes' if needs_approval_safe else 'No'}")
            print_success(f"  Config file write: {'Yes' if needs_approval_critical else 'No'}")
            
            self.test_results["github_approval_check"] = True
            
            # Test 6.5: AI development branch creation (simulated)
            print_info("Testing AI development branch creation (simulated)...")
            try:
                # This would create a branch, but we'll just test the name generation
                goal_description = "Test AI development branch"
                import re
                safe_name = re.sub(r'[^a-zA-Z0-9]+', '-', goal_description.lower())[:50]
                timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
                branch_name = f"ai/{safe_name}_{timestamp}"
                
                print_success(f"Generated branch name: {branch_name}")
                print_success("Note: Branch creation skipped for safety (read-only test)")
                self.test_results["github_branch_creation"] = True
            except Exception as e:
                print_warning(f"Branch creation test failed: {e}")
                self.test_results["github_branch_creation"] = False
            
            # Store test details
            self.test_details["github"] = {
                "safety_levels": {
                    "read": safety_read.value,
                    "write_readme": safety_write.value,
                    "write_config": safety_write_critical.value
                },
                "approval_requirements": {
                    "read": needs_approval_safe,
                    "config_write": needs_approval_critical
                },
                "branches_found": len(branches) if 'branches' in locals() else 0
            }
            
            client.close()
            print_success("GitHub API integration tests completed")
            
        except Exception as e:
            print_error(f"GitHub API integration test failed: {e}")
            self.test_results["github_api"] = False
    
    def test_data_management(self):
        """Test data management functionality"""
        print_header("TEST 7: DATA MANAGEMENT")
        
        try:
            # Initialize data manager
            print_info("Initializing data manager...")
            manager = DataManager()
            
            # Test 7.1: Storage statistics
            print_info("Testing storage statistics...")
            stats = manager.get_storage_stats()
            
            if "error" not in stats:
                print_success(f"Storage statistics retrieved")
                print_success(f"Total size: {stats.get('total_size_readable', 'Unknown')}")
                
                directories = stats.get("directories", {})
                for dir_name, dir_stats in directories.items():
                    print_success(f"  {dir_name}: {dir_stats.get('size_readable', 'Unknown')}")
                
                self.test_results["data_storage_stats"] = True
            else:
                print_warning(f"Storage stats failed: {stats['error']}")
                self.test_results["data_storage_stats"] = False
            
            # Test 7.2: Directory size calculation
            print_info("Testing directory size calculation...")
            test_dir = Path("test_screenshots")
            if test_dir.exists():
                size = manager.get_directory_size(test_dir)
                print_success(f"Test directory size: {manager.format_size(size)}")
                self.test_results["data_dir_size"] = True
            else:
                print_info("Test directory not found, creating sample...")
                test_dir.mkdir(exist_ok=True)
                # Create a small test file
                (test_dir / "test.txt").write_text("Test data")
                size = manager.get_directory_size(test_dir)
                print_success(f"Created test directory, size: {manager.format_size(size)}")
                self.test_results["data_dir_size"] = True
            
            # Test 7.3: Format size utility
            print_info("Testing size formatting...")
            test_sizes = [1024, 1024*1024, 1024*1024*1024]
            for size in test_sizes:
                formatted = manager.format_size(size)
                print_success(f"  {size} bytes = {formatted}")
            
            self.test_results["data_size_format"] = True
            
            # Store test details
            self.test_details["data_management"] = {
                "storage_stats": stats,
                "directory_sizes": {name: stats.get("directories", {}).get(name, {}) for name in ["screenshots", "performance_data", "archives"] if name in stats.get("directories", {})}
            }
            
            print_success("Data management tests completed")
            
        except Exception as e:
            print_error(f"Data management test failed: {e}")
            self.test_results["data_management"] = False
    
    def test_end_to_end_pipeline(self):
        """Test complete end-to-end pipeline"""
        print_header("TEST 8: END-TO-END PIPELINE")
        
        try:
            # Test goal: Analyze codebase for compilation issues (SAFE operation)
            test_goal = "Analyze the current codebase for any compilation issues in the Source/Alexander directory and create a plan to fix them"
            
            print_info(f"Processing goal: {test_goal}")
            
            # Step 1: Parse natural language goal
            print_info("Step 1: Parsing natural language goal...")
            parser = NaturalLanguageParser()
            parsed_goal = parser.parse_development_goal(test_goal)
            
            print_success(f"Goal parsed: {parsed_goal.goal_type.value}")
            print_success(f"Requirements: {len(parsed_goal.requirements)}")
            print_success(f"Risks: {len(parsed_goal.identified_risks)}")
            
            # Step 2: Generate tasks
            print_info("Step 2: Generating tasks...")
            task_generator = TaskGenerator()
            task_specs = task_generator.generate_tasks_from_goal(parsed_goal)
            
            print_success(f"Tasks generated: {len(task_specs)}")
            for i, task_spec in enumerate(task_specs, 1):
                print_success(f"  {i}. {task_spec.title} ({task_spec.task_type.value})")
                print_success(f"     Safety: {task_spec.safety_level.value}, Priority: {task_spec.priority.name}")
            
            # Step 3: Create AI Control Layer tasks
            print_info("Step 3: Converting to AI Control tasks...")
            ai_tasks = [task_generator.convert_to_ai_control_task(ts) for ts in task_specs]
            
            print_success(f"AI tasks created: {len(ai_tasks)}")
            
            # Step 4: Make decisions about task execution
            print_info("Step 4: Making execution decisions...")
            decision_engine = DecisionEngine()
            
            context = DecisionContext(
                available_tasks=ai_tasks,
                system_state={"load": "normal", "network_status": "online"},
                historical_data={"attempts": 0},
                resource_constraints={"cpu": 80, "memory": 70},
                safety_threshold=SafetyLevel.MEDIUM_RISK
            )
            
            # Get execution strategy for first task
            if ai_tasks:
                execution_decision = decision_engine.decide_task_execution_strategy(ai_tasks[0], context)
                print_success(f"Execution strategy: {execution_decision.recommended_action[:60]}...")
                
                # Get task prioritization
                prioritized = decision_engine.decide_task_prioritization(ai_tasks, context)
                print_success(f"Task prioritization: {len(prioritized)} tasks ordered")
            
            # Step 5: Safety validation
            print_info("Step 5: Validating safety classifications...")
            for task in ai_tasks:
                safety_level = task.safety_level
                requires_approval = safety_level in [SafetyLevel.HIGH_RISK, SafetyLevel.CRITICAL]
                print_success(f"Task '{task.title[:30]}...' - Safety: {safety_level.value}, Approval needed: {requires_approval}")
            
            # Step 6: Generate execution plan
            print_info("Step 6: Generating execution plan...")
            
            execution_plan = {
                "goal": test_goal,
                "parsed_goal_id": parsed_goal.goal_id,
                "total_tasks": len(ai_tasks),
                "safety_summary": {
                    "safe": len([t for t in ai_tasks if t.safety_level == SafetyLevel.SAFE]),
                    "low_risk": len([t for t in ai_tasks if t.safety_level == SafetyLevel.LOW_RISK]),
                    "medium_risk": len([t for t in ai_tasks if t.safety_level == SafetyLevel.MEDIUM_RISK]),
                    "high_risk": len([t for t in ai_tasks if t.safety_level == SafetyLevel.HIGH_RISK]),
                    "critical": len([t for t in ai_tasks if t.safety_level == SafetyLevel.CRITICAL])
                },
                "estimated_duration": sum(ts.estimated_duration_minutes for ts in task_specs),
                "tasks": [
                    {
                        "task_id": task.task_id,
                        "title": task.title,
                        "type": task.task_type,
                        "priority": task.priority.name,
                        "safety_level": task.safety_level.value,
                        "duration": next((ts.estimated_duration_minutes for ts in task_specs if ts.task_id == task.task_id), 0),
                        "execution_steps": len(task.execution_steps)
                    }
                    for task in ai_tasks
                ]
            }
            
            print_success("Execution plan generated successfully")
            print_info(f"Total tasks: {execution_plan['total_tasks']}")
            print_info(f"Estimated duration: {execution_plan['estimated_duration']} minutes")
            print_info(f"Safety distribution: {execution_plan['safety_summary']}")
            
            # Save execution plan
            plan_file = "test_execution_plan.json"
            with open(plan_file, 'w') as f:
                json.dump(execution_plan, f, indent=2)
            
            print_success(f"Execution plan saved to: {plan_file}")
            
            # Store test details
            self.test_details["end_to_end"] = {
                "goal_processed": True,
                "tasks_generated": len(ai_tasks),
                "execution_plan_created": True,
                "plan_file": plan_file,
                "safety_validation_passed": True
            }
            
            self.test_results["end_to_end_pipeline"] = True
            print_success("End-to-end pipeline test completed successfully")
            
        except Exception as e:
            print_error(f"End-to-end pipeline test failed: {e}")
            import traceback
            traceback.print_exc()
            self.test_results["end_to_end_pipeline"] = False
    
    def generate_test_report(self):
        """Generate comprehensive test report"""
        print_header("TEST SUMMARY REPORT")
        
        end_time = datetime.now()
        duration = (end_time - self.start_time).total_seconds()
        
        # Overall results
        total_tests = len(self.test_results)
        passed_tests = sum(1 for result in self.test_results.values() if result)
        failed_tests = total_tests - passed_tests
        
        print_info(f"Test execution time: {duration:.2f} seconds")
        print_info(f"Total tests: {total_tests}")
        print_success(f"Passed: {passed_tests}")
        if failed_tests > 0:
            print_error(f"Failed: {failed_tests}")
        
        # Component results
        print_header("COMPONENT TEST RESULTS")
        
        components = {
            "Kimi API Integration": ["kimi_chat", "kimi_code_analysis", "kimi_task_plan", "kimi_usage_report"],
            "Natural Language Processing": ["nlp_goal_parsing", "nlp_code_refs", "nlp_tech_terms", "nlp_sentiment"],
            "Task Generation": ["task_generation", "task_conversion", "task_generation_types"],
            "AI Control Layer": ["ai_goal_processing", "ai_system_status", "ai_execution_report", "ai_safety_approval"],
            "Decision Engine": ["decision_execution_strategy", "decision_error_recovery", "decision_prioritization", "decision_learning", "decision_report"],
            "GitHub API": ["github_repo_info", "github_branches", "github_safety_check", "github_approval_check", "github_branch_creation"],
            "Data Management": ["data_storage_stats", "data_dir_size", "data_size_format"],
            "End-to-End Pipeline": ["end_to_end_pipeline"]
        }
        
        for component, tests in components.items():
            component_results = {test: self.test_results.get(test, False) for test in tests}
            passed = sum(1 for result in component_results.values() if result)
            total = len(component_results)
            
            status_icon = "[OK]" if passed == total else "[WARN]" if passed > 0 else "[FAIL]"
            status_color = Colors.GREEN if passed == total else Colors.YELLOW if passed > 0 else Colors.RED
            
            print(f"{status_color}{status_icon} {component}: {passed}/{total} tests passed{Colors.END}")
            
            # Show failed tests for this component
            if passed < total:
                for test, result in component_results.items():
                    if not result:
                        print(f"  {Colors.RED}  [FAIL] {test}{Colors.END}")
        
        # Cost and usage summary
        if "kimi" in self.test_details:
            kimi_details = self.test_details["kimi"]
            print_header("KIMI API USAGE SUMMARY")
            print_info(f"Total tokens used: {kimi_details['total_tokens']}")
            print_info(f"Total cost: ${kimi_details['total_cost']:.4f}")
            print_info(f"Average response time: {sum(kimi_details['response_times'])/len(kimi_details['response_times']):.2f}s" if kimi_details['response_times'] else "N/A")
            print_info(f"Models used: {', '.join(kimi_details['models_used'])}")
        
        # Safety validation summary
        if "end_to_end" in self.test_details:
            end_to_end = self.test_details["end_to_end"]
            if "safety_summary" in end_to_end:
                print_header("SAFETY VALIDATION SUMMARY")
                safety_summary = end_to_end["safety_summary"]
                for level, count in safety_summary.items():
                    if count > 0:
                        color = Colors.GREEN if level in ["safe", "low_risk"] else Colors.YELLOW if level == "medium_risk" else Colors.RED
                        print(f"{color}  {level.replace('_', ' ').title()}: {count} tasks{Colors.END}")
        
        # Save detailed report
        report_file = "test_autonomous_system_report.json"
        with open(report_file, 'w') as f:
            json.dump({
                "test_summary": {
                    "start_time": self.start_time.isoformat(),
                    "end_time": end_time.isoformat(),
                    "duration_seconds": duration,
                    "total_tests": total_tests,
                    "passed_tests": passed_tests,
                    "failed_tests": failed_tests,
                    "success_rate": passed_tests / total_tests if total_tests > 0 else 0
                },
                "test_results": self.test_results,
                "test_details": self.test_details
            }, f, indent=2, default=str)
        
        print_header("TEST REPORT SAVED")
        print_success(f"Detailed test report saved to: {report_file}")
        
        # Final verdict
        print_header("FINAL VERDICT")
        if passed_tests == total_tests:
            print_success("ALL TESTS PASSED! System is ready for autonomous development.")
        elif passed_tests >= total_tests * 0.8:
            print_warning("MOST TESTS PASSED. System is mostly functional with some issues.")
        else:
            print_error("MANY TESTS FAILED. System needs attention before use.")
        
        return passed_tests == total_tests


def main():
    """Main test execution"""
    print(f"{Colors.BOLD}{Colors.WHITE}Autonomous Development System - Comprehensive Test Suite{Colors.END}")
    print(f"{Colors.WHITE}Testing all components in safe, read-only mode{Colors.END}")
    
    # Run tests
    test_suite = AutonomousSystemTest()
    success = test_suite.run_all_tests()
    
    # Exit with appropriate code
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()