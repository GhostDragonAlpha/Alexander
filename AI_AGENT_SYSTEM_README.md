# AI Agent System for Unreal Engine

**Complete AI-driven automation system for Unreal Engine 5.6 project management**

## Overview

This system provides intelligent automation of Unreal Engine workflows using AI agents that interact with UE's command-line tools. The architecture consists of three layers:

1. **AIUnrealController** - Low-level UE tool wrapper
2. **Specialized Agents** - Domain-specific automation (Content, Build, Testing)
3. **WorkflowOrchestrator** - High-level workflow coordination

## Architecture

```
WorkflowOrchestrator
├── AIContentAgent
│   ├── Asset Validation
│   ├── LOD Generation
│   ├── Content Organization
│   └── Unused Asset Cleanup
├── AIBuildAgent
│   ├── Incremental Builds
│   ├── Dependency Analysis
│   └── Content Cooking
└── AITestingAgent
    ├── Smoke Tests
    ├── Integration Tests
    ├── Performance Tests
    └── Failure Analysis

AIUnrealController
├── UnrealEditor-Cmd.exe (Python scripts, commandlets)
├── UnrealAutomationTool.exe (Build orchestration)
├── UnrealBuildTool.exe (C++ compilation)
└── UnrealFrontend.exe (Packaging)
```

## Installation

No installation required. System uses:
- Python 3.x (already installed)
- Unreal Engine 5.6 CLI tools (already available)

Files added:
- `ai_unreal_controller.py` - Core controller (~550 lines)
- `ai_agents.py` - Specialized agents (~400 lines)
- `ai_workflow_orchestrator.py` - Workflow coordination (~450 lines)
- Integration with existing `build_and_test.py`

## Quick Start

### Via build_and_test.py (Recommended)

```powershell
# Daily maintenance workflow
python build_and_test.py ai daily

# Pre-commit validation (fast)
python build_and_test.py ai precommit

# Content creation workflow
python build_and_test.py ai content environment

# Performance optimization
python build_and_test.py ai optimize

# View workflow history
python build_and_test.py ai history 10
```

### Direct Orchestrator Usage

```powershell
# Daily maintenance
python ai_workflow_orchestrator.py daily_maintenance

# Pre-commit validation
python ai_workflow_orchestrator.py pre_commit

# Content creation
python ai_workflow_orchestrator.py content_creation --content-type environment

# Performance optimization
python ai_workflow_orchestrator.py performance_optimization

# View history
python ai_workflow_orchestrator.py history --limit 10
```

### Using Individual Agents

```python
from ai_unreal_controller import AIUnrealController
from ai_agents import AIContentAgent, AIBuildAgent, AITestingAgent

# Initialize controller
controller = AIUnrealController(
    engine_path="C:/Program Files/Epic Games/UE_5.6",
    project_path="C:/Users/allen/Desktop/Alexander/Alexander/Alexander.uproject"
)

# Create agents
content_agent = AIContentAgent(controller)
build_agent = AIBuildAgent(controller)
test_agent = AITestingAgent(controller)

# Run operations
validation = content_agent.validate_all_content()
lods = content_agent.auto_generate_lods(quality="High")
tests = test_agent.run_smoke_tests()
```

## Workflows

### 1. Daily Maintenance Workflow

**Purpose:** Comprehensive daily checks and maintenance  
**Duration:** ~10-15 minutes  
**Steps:**
1. Validate all content assets
2. Find unused assets
3. Generate asset usage report
4. Run smoke tests
5. Generate summary report

**Usage:**
```powershell
python build_and_test.py ai daily
```

**Output:**
- `Saved/AI_Reports/daily_maintenance_YYYYMMDD_HHMMSS.json`
- Console summary with errors/warnings/test results

### 2. Pre-Commit Workflow

**Purpose:** Fast validation before committing changes  
**Duration:** ~2-5 minutes  
**Steps:**
1. Validate changed content
2. Run quick smoke tests
3. Report blocking issues

**Usage:**
```powershell
python build_and_test.py ai precommit
```

**Best Practice:** Add as pre-commit git hook:
```bash
# .git/hooks/pre-commit
#!/bin/sh
python build_and_test.py ai precommit || exit 1
```

### 3. Content Creation Workflow

**Purpose:** Guided workflow for new content  
**Duration:** ~5-10 minutes  
**Steps:**
1. Organize imported assets by type
2. Auto-generate LODs for meshes
3. Validate content integrity
4. Generate creation report

**Usage:**
```powershell
# For environment content
python build_and_test.py ai content environment

# For character content
python build_and_test.py ai content character
```

### 4. Performance Optimization Workflow

**Purpose:** Automated performance optimization  
**Duration:** ~15-30 minutes  
**Steps:**
1. Baseline performance measurement
2. Optimize static meshes (LOD generation)
3. Optimize textures (streaming config)
4. Post-optimization measurement
5. Calculate improvements

**Usage:**
```powershell
python build_and_test.py ai optimize
```

## Core Controller API

### AIUnrealController

```python
controller = AIUnrealController(engine_path, project_path)

# Execute Python in UE context
result = controller.execute_python_in_editor(
    script_content="import unreal; unreal.log('Hello')"
)

# Run UE commandlet
result = controller.execute_commandlet("ResavePackages")

# Validate content
result = controller.validate_content(content_path="/Game/Maps")

# Run automation tests
result = controller.run_automation_tests(test_filter="Smoke")

# Build lighting
result = controller.build_lighting(map_path="/Game/Maps/MainLevel")

# Generate reports
asset_report = controller.generate_asset_report()
unused_assets = controller.find_unused_assets()

# Batch optimize
result = controller.optimize_assets_batch(
    asset_filter="StaticMesh",
    quality="Medium"
)
```

## Specialized Agents

### AIContentAgent

```python
content_agent = AIContentAgent(controller)

# LOD generation
lods = content_agent.auto_generate_lods(
    quality="Medium",  # Low, Medium, High, Ultra
    asset_filter="StaticMesh"
)

# Content validation
validation = content_agent.validate_all_content(
    paths=["/Game/Maps", "/Game/Meshes"]
)

# Cleanup unused assets
cleanup = content_agent.cleanup_unused_assets(
    dry_run=True  # Set False to actually delete
)

# Organize imports
organized = content_agent.organize_content_by_type(
    source_path="/Game/Imports"
)
```

### AIBuildAgent

```python
build_agent = AIBuildAgent(controller)

# Incremental build
build = build_agent.incremental_build(
    configuration="Development",
    platform="Win64"
)

# Dependency analysis
deps = build_agent.analyze_dependencies(module_name="Alexander")

# Cook content
cook = build_agent.cook_content(
    maps=["/Game/Maps/MainLevel"],
    platform="WindowsClient"
)
```

### AITestingAgent

```python
test_agent = AITestingAgent(controller)

# Run tests
smoke = test_agent.run_smoke_tests()
integration = test_agent.run_integration_tests()
performance = test_agent.run_performance_tests()

# Analyze failures
analysis = test_agent.analyze_test_failures(
    report_path="Saved/Automation/Reports/test_report.json"
)
```

## Reports and Monitoring

All workflows generate JSON reports in `Saved/AI_Reports/`:

```json
{
  "workflow": "daily_maintenance",
  "start_time": "2025-01-15T10:30:00",
  "end_time": "2025-01-15T10:42:15",
  "duration_seconds": 735.5,
  "overall_success": true,
  "steps": [
    {
      "name": "content_validation",
      "success": true,
      "errors": 0,
      "warnings": 5
    },
    {
      "name": "unused_assets",
      "success": true,
      "found": 23
    }
  ]
}
```

View history:
```powershell
python build_and_test.py ai history 10
```

## Integration with Existing Systems

### build_and_test.py Integration

The AI system integrates seamlessly:

```powershell
# Traditional build
python build_and_test.py build

# AI-enhanced validation
python build_and_test.py ai precommit

# Traditional test
python build_and_test.py test

# AI-enhanced optimization
python build_and_test.py ai optimize
```

### Git Hooks

Add `.git/hooks/pre-commit`:
```bash
#!/bin/sh
echo "Running AI pre-commit validation..."
python build_and_test.py ai precommit
if [ $? -ne 0 ]; then
    echo "Pre-commit validation failed. Fix issues before committing."
    exit 1
fi
```

### CI/CD Integration

Add to CI pipeline:
```yaml
# .github/workflows/ci.yml
- name: AI Daily Maintenance
  run: python build_and_test.py ai daily
  
- name: Upload Reports
  uses: actions/upload-artifact@v2
  with:
    name: ai-reports
    path: Saved/AI_Reports/
```

## Advanced Usage

### Custom Python Scripts in UE

```python
# Define custom script
script = """
import unreal

# Custom validation logic
asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
blueprints = asset_registry.get_assets_by_class('Blueprint')

for bp_data in blueprints:
    bp = bp_data.get_asset()
    # Validate blueprint...
    unreal.log(f'Validated: {bp_data.asset_name}')
"""

# Execute in UE
result = controller.execute_python_in_editor(script_content=script)
```

### Custom Workflows

```python
from ai_workflow_orchestrator import WorkflowOrchestrator

orchestrator = WorkflowOrchestrator(engine_path, project_path)

# Custom workflow combining operations
def custom_release_workflow():
    # Step 1: Optimize
    orchestrator.performance_optimization_workflow()
    
    # Step 2: Validate
    result = orchestrator.content_agent.validate_all_content()
    
    # Step 3: Test
    orchestrator.test_agent.run_integration_tests()
    
    # Step 4: Package
    orchestrator.build_agent.cook_content(["/Game/Maps/MainLevel"])
```

## Troubleshooting

### Python Script Execution Timeout

If scripts timeout (default 300s):
```python
controller.execute_python_in_editor(
    script_content=script,
    timeout=1800  # 30 minutes
)
```

### UnrealEditor-Cmd.exe Not Found

Verify engine path:
```python
controller = AIUnrealController(
    engine_path="C:/Program Files/Epic Games/UE_5.6",  # Verify this path
    project_path="path/to/project.uproject"
)
```

### Content Validation Errors

Review detailed errors:
```python
result = content_agent.validate_all_content()
for error in result['errors']:
    print(f"Asset: {error['asset']}, Class: {error['class']}")
```

## Performance Considerations

- **Daily Maintenance:** Run during low-activity hours (nightly)
- **Pre-Commit:** Fast (2-5 min), safe for frequent use
- **Content Creation:** Run after importing new assets
- **Optimization:** Run weekly or before major releases

## Future Enhancements

Planned features:
- [ ] Web dashboard for workflow monitoring
- [ ] Slack/Discord notifications
- [ ] Machine learning for failure prediction
- [ ] Automated fix suggestions
- [ ] Multi-project support
- [ ] Cloud build integration

## Technical Details

### Executables Used

| Tool | Purpose | Integration |
|------|---------|-------------|
| UnrealEditor-Cmd.exe | Python scripts, commandlets | Primary |
| UnrealAutomationTool.exe | Build orchestration | Future |
| UnrealBuildTool.exe | C++ compilation | Future |
| UnrealFrontend.exe | Packaging | Existing |

### Python API Coverage

The system uses UE's Python API for:
- Asset registry queries
- Content validation
- LOD generation (via ContentOptimizer)
- Material/texture optimization
- Test execution

## Support

For issues or questions:
1. Check `Saved/AI_Reports/` for detailed logs
2. Review Python console output
3. Check `Saved/Logs/Python.log` in UE editor
4. Examine workflow JSON reports

## License

Part of Alexander project. Internal use only.

---

**System Status:** ✓ Operational  
**Last Updated:** January 2025  
**Version:** 1.0.0
