# Unreal Engine AI Agent Integration Guide

## Overview
This guide details how to leverage Unreal Engine's command-line tools for AI-driven automation and programmatic control of the development workflow.

## Key Executables for AI Agent Control

### 1. **UnrealEditor-Cmd.exe** ⭐ PRIMARY TOOL
**Purpose**: Headless command-line editor for automation
**AI Agent Use Cases**:
- Run Python scripts inside UE environment
- Execute Blueprint operations
- Batch asset processing
- Automated testing without UI
- Content generation and validation

**Integration Example**:
```python
def run_unreal_python_script(script_path):
    """Execute Python script in Unreal Engine context"""
    cmd = [
        "C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe",
        "C:/Users/allen/Desktop/Alexander/Alexander/Alexander.uproject",
        "-run=pythonscript",
        f"-script={script_path}",
        "-stdout",
        "-unattended",
        "-nopause"
    ]
    return subprocess.run(cmd, capture_output=True, text=True)
```

### 2. **UnrealAutomationTool.exe** ⭐ BUILD AUTOMATION
**Purpose**: High-level build and automation orchestration
**AI Agent Use Cases**:
- Complex build pipelines
- Multi-platform builds
- Custom automation tasks
- CI/CD integration

**Integration Example**:
```python
def execute_automation_task(task_name, project_path):
    """Run UAT custom automation task"""
    cmd = [
        "C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/DotNET/UnrealAutomationTool/UnrealAutomationTool.exe",
        task_name,
        f"-project={project_path}",
        "-nopause"
    ]
    return subprocess.run(cmd, capture_output=True, text=True)
```

### 3. **UnrealBuildTool.exe** ⭐ CODE COMPILATION
**Purpose**: C++ code compilation and project generation
**AI Agent Use Cases**:
- Incremental builds
- Module dependency analysis
- Custom build configurations
- Header tool execution

**Integration Example**:
```python
def build_module(module_name, config="Development", platform="Win64"):
    """Build specific module"""
    cmd = [
        "C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.exe",
        module_name,
        platform,
        config,
        "-Project=C:/Users/allen/Desktop/Alexander/Alexander/Alexander.uproject",
        "-Progress"
    ]
    return subprocess.run(cmd, capture_output=True, text=True)
```

### 4. **UnrealFrontend.exe** (Already Integrated)
**Purpose**: Complete build/cook/package pipeline
**AI Agent Use Cases**: See existing automated_build.py

### 5. **UnrealInsights.exe** 📊 PERFORMANCE ANALYSIS
**Purpose**: Performance profiling and analysis
**AI Agent Use Cases**:
- Automated performance regression testing
- Profile data analysis
- Performance report generation

### 6. **UnrealVersionSelector.exe**
**Purpose**: Project file association and engine version management
**AI Agent Use Cases**:
- Multi-engine project management
- Engine version switching

### 7. **CrashReportClient.exe**
**Purpose**: Crash data collection and reporting
**AI Agent Use Cases**:
- Automated crash analysis
- Crash pattern detection
- Stability monitoring

## AI Agent Integration Architecture

### Agent Hierarchy
```
┌─────────────────────────────────────────┐
│     AI Orchestrator Agent               │
│  (Coordinates all operations)           │
└───────────┬─────────────────────────────┘
            │
    ┌───────┴────────┬─────────────────────┬──────────────┐
    │                │                     │              │
┌───▼────┐  ┌───────▼────┐  ┌────────────▼───┐  ┌──────▼─────┐
│ Build  │  │  Content   │  │    Testing     │  │  Analysis  │
│ Agent  │  │   Agent    │  │     Agent      │  │   Agent    │
└───┬────┘  └─────┬──────┘  └────────┬───────┘  └──────┬─────┘
    │             │                  │                  │
    │             │                  │                  │
  UBT/UAT    Editor-Cmd          Editor-Cmd        Insights
```

### Implementation: AI Agent Controller

```python
# ai_unreal_controller.py
import subprocess
import json
from pathlib import Path
from typing import Dict, List, Optional
from enum import Enum

class UnrealTool(Enum):
    EDITOR_CMD = "UnrealEditor-Cmd.exe"
    UAT = "UnrealAutomationTool.exe"
    UBT = "UnrealBuildTool.exe"
    FRONTEND = "UnrealFrontend.exe"
    INSIGHTS = "UnrealInsights.exe"

class AIUnrealController:
    """AI Agent controller for Unreal Engine operations"""
    
    def __init__(self, engine_path: str, project_path: str):
        self.engine_path = Path(engine_path)
        self.project_path = Path(project_path)
        self.binaries = self.engine_path / "Engine/Binaries/Win64"
        self.dotnet_binaries = self.engine_path / "Engine/Binaries/DotNET"
        
    def execute_python_in_editor(self, script_path: str, args: Dict = None) -> Dict:
        """Execute Python script in Unreal Editor context"""
        cmd = [
            str(self.binaries / UnrealTool.EDITOR_CMD.value),
            str(self.project_path),
            "-run=pythonscript",
            f"-script={script_path}",
            "-stdout",
            "-unattended",
            "-nopause",
            "-nullrhi"  # No rendering for faster execution
        ]
        
        if args:
            cmd.append(f"-scriptargs={json.dumps(args)}")
            
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        return {
            "success": result.returncode == 0,
            "stdout": result.stdout,
            "stderr": result.stderr,
            "returncode": result.returncode
        }
    
    def batch_process_assets(self, asset_paths: List[str], operation: str) -> Dict:
        """Batch process assets using Editor commandlet"""
        script = self._generate_batch_script(asset_paths, operation)
        return self.execute_python_in_editor(script)
    
    def validate_content(self, content_path: str) -> Dict:
        """Run content validation checks"""
        cmd = [
            str(self.binaries / UnrealTool.EDITOR_CMD.value),
            str(self.project_path),
            "-run=DataValidation",
            f"-path={content_path}",
            "-unattended",
            "-nopause"
        ]
        
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        # Parse validation results
        errors = self._parse_validation_output(result.stdout)
        
        return {
            "success": result.returncode == 0,
            "errors": errors,
            "warnings": [],  # Parse from output
            "info": []
        }
    
    def run_automation_tests(self, test_filter: str = None) -> Dict:
        """Execute automated tests"""
        cmd = [
            str(self.binaries / UnrealTool.EDITOR_CMD.value),
            str(self.project_path),
            "-ExecCmds=Automation RunTests " + (test_filter or "Now"),
            "-unattended",
            "-nopause",
            "-nullrhi",
            "-ReportOutputPath=C:/Users/allen/Desktop/Alexander/Alexander/Saved/Automation/Reports"
        ]
        
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        return {
            "success": result.returncode == 0,
            "output": result.stdout,
            "report_path": "Saved/Automation/Reports"
        }
    
    def generate_asset_report(self) -> Dict:
        """Generate comprehensive asset usage report"""
        script = """
import unreal

# Get asset registry
asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()

# Get all assets
all_assets = asset_registry.get_all_assets()

report = {
    'total_assets': len(all_assets),
    'by_type': {},
    'large_assets': [],
    'unused_assets': []
}

for asset_data in all_assets:
    asset_class = str(asset_data.asset_class)
    
    if asset_class not in report['by_type']:
        report['by_type'][asset_class] = 0
    report['by_type'][asset_class] += 1

# Save report
import json
with open('C:/Users/allen/Desktop/Alexander/Alexander/Saved/Reports/asset_report.json', 'w') as f:
    json.dump(report, f, indent=2)

unreal.log('Asset report generated')
"""
        
        temp_script = Path("Saved/Temp/generate_report.py")
        temp_script.parent.mkdir(parents=True, exist_ok=True)
        temp_script.write_text(script)
        
        return self.execute_python_in_editor(str(temp_script))
    
    def optimize_assets_ai(self, quality_preset: str = "Medium") -> Dict:
        """Use ContentOptimizer with AI-driven settings"""
        script = f"""
import unreal

# Load ContentOptimizer
optimizer = unreal.ContentOptimizer()

# Get quality preset
quality = unreal.OptimizationQuality.{quality_preset.upper()}

# Find all static meshes
asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
all_assets = asset_registry.get_assets_by_class('StaticMesh')

results = []

for asset_data in all_assets:
    asset = asset_data.get_asset()
    if asset:
        result = optimizer.generate_lods_for_static_mesh(asset, optimizer.get_lod_preset(quality))
        results.append({{
            'asset': asset_data.asset_name,
            'success': result.success,
            'reduction': result.optimization_percentage
        }})

unreal.log(f'Optimized {{len(results)}} assets')
"""
        
        temp_script = Path("Saved/Temp/optimize_assets.py")
        temp_script.write_text(script)
        
        return self.execute_python_in_editor(str(temp_script))
    
    def _generate_batch_script(self, asset_paths: List[str], operation: str) -> str:
        """Generate Python script for batch operations"""
        # Implementation details
        pass
    
    def _parse_validation_output(self, output: str) -> List[Dict]:
        """Parse validation errors from output"""
        # Implementation details
        pass

class AIContentAgent:
    """Specialized agent for content operations"""
    
    def __init__(self, controller: AIUnrealController):
        self.controller = controller
        
    def auto_generate_lods(self):
        """Automatically generate LODs for all meshes"""
        return self.controller.optimize_assets_ai("Medium")
    
    def validate_all_content(self):
        """Run comprehensive content validation"""
        return self.controller.validate_content("/Game/")
    
    def cleanup_unused_assets(self):
        """Find and report unused assets"""
        script = """
import unreal

# Use AssetToolsHelpers to find unused assets
editor_util = unreal.EditorAssetLibrary()
asset_paths = editor_util.list_assets('/Game/', recursive=True)

unused = []
for path in asset_paths:
    references = editor_util.find_package_referencers_for_asset(path)
    if len(references) == 0:
        unused.append(path)

unreal.log(f'Found {len(unused)} unused assets')

import json
with open('C:/Users/allen/Desktop/Alexander/Alexander/Saved/Reports/unused_assets.json', 'w') as f:
    json.dump(unused, f, indent=2)
"""
        
        temp_script = Path("Saved/Temp/find_unused.py")
        temp_script.parent.mkdir(parents=True, exist_ok=True)
        temp_script.write_text(script)
        
        return self.controller.execute_python_in_editor(str(temp_script))

class AIBuildAgent:
    """Specialized agent for build operations"""
    
    def __init__(self, controller: AIUnrealController):
        self.controller = controller
        
    def incremental_build(self, changed_files: List[str]):
        """Smart incremental build based on file changes"""
        # Analyze which modules need rebuilding
        affected_modules = self._analyze_dependencies(changed_files)
        
        results = []
        for module in affected_modules:
            result = subprocess.run([
                str(controller.binaries.parent / "DotNET/UnrealBuildTool/UnrealBuildTool.exe"),
                module,
                "Win64",
                "Development",
                f"-Project={controller.project_path}"
            ], capture_output=True)
            
            results.append({
                'module': module,
                'success': result.returncode == 0
            })
            
        return results
    
    def _analyze_dependencies(self, files: List[str]) -> List[str]:
        """Analyze which modules are affected by file changes"""
        # Implementation: Parse .Build.cs files and dependency graphs
        return ["Alexander"]  # Simplified

class AITestingAgent:
    """Specialized agent for testing operations"""
    
    def __init__(self, controller: AIUnrealController):
        self.controller = controller
        
    def run_smoke_tests(self):
        """Run quick smoke tests"""
        return self.controller.run_automation_tests("Smoke")
    
    def run_integration_tests(self):
        """Run integration tests"""
        return self.controller.run_automation_tests("Integration")
    
    def run_performance_tests(self):
        """Run performance benchmark tests"""
        return self.controller.run_automation_tests("Performance")
```

## Integrated Workflow Script

```python
# ai_workflow_orchestrator.py
"""
AI-driven workflow orchestrator for Unreal Engine
Coordinates multiple AI agents for automated development tasks
"""

from ai_unreal_controller import (
    AIUnrealController, 
    AIContentAgent, 
    AIBuildAgent, 
    AITestingAgent
)
import argparse
import json
from pathlib import Path

class WorkflowOrchestrator:
    """Orchestrates AI agents for complete development workflows"""
    
    def __init__(self):
        self.controller = AIUnrealController(
            engine_path="C:/Program Files/Epic Games/UE_5.6",
            project_path="C:/Users/allen/Desktop/Alexander/Alexander/Alexander.uproject"
        )
        
        self.content_agent = AIContentAgent(self.controller)
        self.build_agent = AIBuildAgent(self.controller)
        self.test_agent = AITestingAgent(self.controller)
        
    def daily_maintenance_workflow(self):
        """Complete daily maintenance workflow"""
        print("🤖 Starting AI Daily Maintenance Workflow")
        
        steps = []
        
        # Step 1: Content Validation
        print("📋 Step 1: Validating content...")
        validation = self.content_agent.validate_all_content()
        steps.append(("Content Validation", validation))
        
        # Step 2: Asset Optimization
        print("⚡ Step 2: Optimizing assets...")
        optimization = self.content_agent.auto_generate_lods()
        steps.append(("Asset Optimization", optimization))
        
        # Step 3: Cleanup unused assets
        print("🧹 Step 3: Finding unused assets...")
        cleanup = self.content_agent.cleanup_unused_assets()
        steps.append(("Cleanup Analysis", cleanup))
        
        # Step 4: Run smoke tests
        print("🧪 Step 4: Running smoke tests...")
        tests = self.test_agent.run_smoke_tests()
        steps.append(("Smoke Tests", tests))
        
        # Generate report
        report = self._generate_report(steps)
        self._save_report(report, "daily_maintenance")
        
        return report
    
    def pre_commit_workflow(self, changed_files: List[str]):
        """Pre-commit validation workflow"""
        print("🤖 Starting AI Pre-Commit Workflow")
        
        # 1. Incremental build
        build_result = self.build_agent.incremental_build(changed_files)
        
        if not all(r['success'] for r in build_result):
            return {"success": False, "error": "Build failed"}
        
        # 2. Run relevant tests
        test_result = self.test_agent.run_smoke_tests()
        
        return {
            "success": test_result['success'],
            "build": build_result,
            "tests": test_result
        }
    
    def content_creation_workflow(self, asset_type: str):
        """AI-assisted content creation workflow"""
        # Implementation for guided content creation
        pass
    
    def _generate_report(self, steps):
        """Generate workflow report"""
        return {
            "timestamp": datetime.now().isoformat(),
            "steps": steps,
            "overall_success": all(s[1].get('success', False) for s in steps)
        }
    
    def _save_report(self, report, workflow_name):
        """Save workflow report"""
        report_dir = Path("Saved/Reports/Workflows")
        report_dir.mkdir(parents=True, exist_ok=True)
        
        filename = f"{workflow_name}_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
        with open(report_dir / filename, 'w') as f:
            json.dump(report, f, indent=2)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="AI Workflow Orchestrator")
    parser.add_argument('--workflow', choices=['daily', 'pre-commit', 'content'], 
                       default='daily')
    
    args = parser.parse_args()
    
    orchestrator = WorkflowOrchestrator()
    
    if args.workflow == 'daily':
        result = orchestrator.daily_maintenance_workflow()
    elif args.workflow == 'pre-commit':
        # Get changed files from git
        result = orchestrator.pre_commit_workflow([])
    
    print("\n✅ Workflow Complete!")
    print(json.dumps(result, indent=2))
```

## Integration with Existing Systems

### Update build_and_test.py
Add AI agent commands:

```python
# Add to build_and_test.py

def ai_workflow(args):
    """Run AI-driven workflow"""
    from ai_workflow_orchestrator import WorkflowOrchestrator
    
    orchestrator = WorkflowOrchestrator()
    
    if args.type == 'daily':
        orchestrator.daily_maintenance_workflow()
    elif args.type == 'validate':
        orchestrator.content_agent.validate_all_content()
    elif args.type == 'optimize':
        orchestrator.content_agent.auto_generate_lods()

# Add subcommand
ai_parser = subparsers.add_parser('ai', help='AI agent workflows')
ai_parser.add_argument('--type', choices=['daily', 'validate', 'optimize'])
ai_parser.set_defaults(func=ai_workflow)
```

## Next Steps

1. **Implement ai_unreal_controller.py** - Core controller class
2. **Create specialized agents** - Content, Build, Testing agents
3. **Build orchestrator** - Workflow coordination
4. **Add to build pipeline** - Integrate with existing automation
5. **Create monitoring dashboard** - Track AI agent activities
6. **Add learning system** - Learn from past builds/operations

## Benefits

✅ **Automated Content Validation** - Continuous quality checks
✅ **Smart Asset Optimization** - AI-driven LOD generation  
✅ **Incremental Builds** - Only rebuild what changed
✅ **Automated Testing** - Run tests on every change
✅ **Performance Monitoring** - Track metrics over time
✅ **Unused Asset Detection** - Clean up project bloat
✅ **Intelligent Workflows** - Context-aware automation
