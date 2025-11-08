# AI Agent System - Quick Reference

## 🚀 Quick Commands

### Via build_and_test.py (Recommended)
```powershell
# Daily maintenance (10-15 min)
python build_and_test.py ai daily

# Pre-commit check (2-5 min) - Run before every commit
python build_and_test.py ai precommit

# Content creation (5-10 min) - Run after importing assets
python build_and_test.py ai content environment

# Performance optimization (15-30 min) - Run weekly
python build_and_test.py ai optimize

# View history (last 10 runs)
python build_and_test.py ai history 10
```

### Direct Orchestrator
```powershell
python ai_workflow_orchestrator.py daily_maintenance
python ai_workflow_orchestrator.py pre_commit
python ai_workflow_orchestrator.py content_creation --content-type character
python ai_workflow_orchestrator.py performance_optimization
python ai_workflow_orchestrator.py history --limit 20
```

## 📊 What Each Workflow Does

| Workflow | Duration | When to Use |
|----------|----------|-------------|
| **daily** | 10-15 min | Nightly automated run |
| **precommit** | 2-5 min | Before every git commit |
| **content** | 5-10 min | After importing new assets |
| **optimize** | 15-30 min | Weekly or before releases |

## 🔧 Programmatic API

### Basic Setup
```python
from ai_unreal_controller import AIUnrealController
from ai_agents import AIContentAgent, AIBuildAgent, AITestingAgent

controller = AIUnrealController(
    engine_path="C:/Program Files/Epic Games/UE_5.6",
    project_path="C:/path/to/Alexander.uproject"
)

content = AIContentAgent(controller)
build = AIBuildAgent(controller)
test = AITestingAgent(controller)
```

### Content Operations
```python
# Validate all content
result = content.validate_all_content()
print(f"Errors: {result['error_count']}, Warnings: {result['warning_count']}")

# Auto-generate LODs
result = content.auto_generate_lods(quality="High")
print(f"Optimized: {result['optimization_report']['optimized']} assets")

# Find unused assets
result = content.cleanup_unused_assets(dry_run=True)
print(f"Found {result['found']} unused assets")

# Organize imported assets
result = content.organize_content_by_type(source_path="/Game/Imports")
print(f"Organized {result['organization_report']['moved']} assets")
```

### Testing Operations
```python
# Run smoke tests
result = test.run_smoke_tests()

# Run integration tests
result = test.run_integration_tests()

# Run performance tests
result = test.run_performance_tests()
```

### Direct UE Operations
```python
# Execute Python in UE
script = """
import unreal
unreal.log('Hello from AI agent!')
"""
result = controller.execute_python_in_editor(script_content=script)

# Run commandlet
result = controller.execute_commandlet("ResavePackages")

# Build lighting
result = controller.build_lighting(map_path="/Game/Maps/MainLevel", quality="Medium")

# Generate reports
asset_report = controller.generate_asset_report()
unused_assets = controller.find_unused_assets()
```

## 📁 Reports Location

All workflow reports saved to: `Saved/AI_Reports/`

File format: `{workflow_type}_{timestamp}.json`

Example:
```json
{
  "workflow": "daily_maintenance",
  "start_time": "2025-01-15T10:30:00",
  "duration_seconds": 735.5,
  "overall_success": true,
  "steps": [
    {"name": "content_validation", "success": true, "errors": 0, "warnings": 5},
    {"name": "unused_assets", "success": true, "found": 23},
    {"name": "asset_report", "success": true, "total_assets": 1523},
    {"name": "smoke_tests", "success": true}
  ]
}
```

## 🔄 Git Hook Integration

Create `.git/hooks/pre-commit`:
```bash
#!/bin/sh
echo "Running AI pre-commit validation..."
python build_and_test.py ai precommit
if [ $? -ne 0 ]; then
    echo "❌ Pre-commit validation failed!"
    exit 1
fi
echo "✅ Pre-commit validation passed!"
```

Make executable:
```powershell
chmod +x .git/hooks/pre-commit
```

## 🎯 Daily Workflow Schedule

### Morning (9 AM)
- Review `Saved/AI_Reports/daily_maintenance_*.json` from night
- Check for errors/warnings

### Before Commit
```powershell
python build_and_test.py ai precommit
```

### After Asset Import
```powershell
python build_and_test.py ai content environment
```

### Friday (Weekly)
```powershell
python build_and_test.py ai optimize
```

### Nightly (Automated)
```powershell
# Add to Task Scheduler (Windows)
python build_and_test.py ai daily
```

## 🐛 Troubleshooting

### Command Timeout
```python
# Increase timeout for long operations
result = controller.execute_python_in_editor(
    script_content=script,
    timeout=1800  # 30 minutes
)
```

### Path Issues
```python
# Verify paths exist
from pathlib import Path
engine = Path("C:/Program Files/Epic Games/UE_5.6")
project = Path("C:/path/to/Alexander.uproject")

print(f"Engine exists: {engine.exists()}")
print(f"Project exists: {project.exists()}")
```

### View Detailed Errors
```python
result = content.validate_all_content()
for error in result['errors']:
    print(f"Asset: {error['asset']}")
    print(f"Class: {error['class']}")
```

## 📚 File Reference

| File | Purpose |
|------|---------|
| `ai_unreal_controller.py` | Core UE interface |
| `ai_agents.py` | Specialized agents |
| `ai_workflow_orchestrator.py` | Workflow coordination |
| `test_ai_system.py` | Test suite |
| `AI_AGENT_SYSTEM_README.md` | Full documentation |
| `AI_AGENT_IMPLEMENTATION_COMPLETE.md` | Implementation summary |

## 🎓 Learning Path

1. **Start Simple:** Run `python build_and_test.py ai daily`
2. **Understand Output:** Check `Saved/AI_Reports/` JSON
3. **Use Pre-Commit:** Add git hook for validation
4. **Explore API:** Try programmatic operations
5. **Custom Workflows:** Create your own using orchestrator

## ⚡ Performance Tips

- Use `precommit` workflow for speed (2-5 min)
- Run `daily` workflow during off-hours
- Set specific paths for faster validation:
  ```python
  content.validate_all_content(paths=["/Game/Maps", "/Game/Meshes"])
  ```
- Use dry-run for unsafe operations:
  ```python
  content.cleanup_unused_assets(dry_run=True)
  ```

## 🆘 Getting Help

1. Check `Saved/AI_Reports/` for detailed logs
2. Review Python console output
3. Read `AI_AGENT_SYSTEM_README.md`
4. Run test suite: `python test_ai_system.py`

---

**Quick Test:** `python test_ai_system.py`  
**Full Docs:** `AI_AGENT_SYSTEM_README.md`  
**Status:** ✓ Operational
