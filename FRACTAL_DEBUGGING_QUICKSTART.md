# Fractal Debugging Framework - Quick Start Guide

## Overview

The **Fractal Debugging Framework (FDF)** implements the **Resonance Debugging Protocol** - a systematic, pattern-based approach to debugging that identifies root causes across multiple scales (micro → meso → macro → meta) and measures cascade effects of interventions.

## Core Components

### 1. **Pattern Signatures** (`pattern_signatures_unreal.py`)
Defines Unreal Engine-specific bug patterns:
- **null_cascade_ue**: Null pointer cascades, missing includes, uninitialized members
- **state_desync_ue**: State synchronization issues across subsystems
- **coupling_cascade_ue**: Excessive coupling and circular dependencies
- **api_drift_ue**: Deprecated API usage and signature mismatches

### 2. **Fractal Debugger** (`fractal_debugger.py`)
Core engine that:
- Detects patterns at variable, function, class, and system levels
- Identifies resonant scales (where interventions have maximum impact)
- Analyzes harmonic structure to determine if issues are systemic
- Generates targeted interventions with cascade predictions

### 3. **File-Based State Scanner** (`file_based_state_scanner.py`)
Integrates with file-coordinated systems to:
- Detect state desynchronization through temporal analysis
- Identify write conflicts and coordination issues
- Monitor agent health in distributed systems

### 4. **Intervention Executor** (`intervention_executor.py`)
- Applies targeted interventions with automatic backups
- Measures actual cascade effects vs. predictions
- Tracks historical success rates for pattern types
- Supports batch and individual intervention modes

### 5. **Fractal Build Analyzer** (`fractal_build_analyzer.py`)
- Parses Unreal Build Tool output
- Maps compilation errors to fractal patterns
- Generates prioritized intervention plans
- Integrates with the complete workflow

### 6. **Master Orchestrator** (`fractal_debugging_master.py`)
Coordinates all components and provides:
- Complete analysis workflow
- Continuous monitoring
- Automated intervention execution
- Comprehensive reporting

## Quick Start

### Basic Analysis
```bash
# Analyze current project state
python fractal_debugging_master.py "C:\Users\allen\Desktop\Alexander\Alexander" --mode analyze

# Analyze with build output
python fractal_debugging_master.py "C:\Users\allen\Desktop\Alexander\Alexander" --mode analyze --build-output build_output.txt

# Run build and analyze
python fractal_debugging_master.py "C:\Users\allen\Desktop\Alexander\Alexander" --mode full
```

### Intervention Modes
```bash
# Generate and display intervention plan
python fractal_debugging_master.py "C:\Users\allen\Desktop\Alexander\Alexander" --mode fix

# Auto-apply interventions
python fractal_debugging_master.py "C:\Users\allen\Desktop\Alexander\Alexander" --mode fix --auto-apply

# Complete workflow: analyze → fix → verify
python fractal_debugging_master.py "C:\Users\allen\Desktop\Alexander\Alexander" --mode full --auto-apply
```

### Continuous Monitoring
```bash
# Monitor project health every 60 seconds
python fractal_debugging_master.py "C:\Users\allen\Desktop\Alexander\Alexander" --mode monitor

# Custom interval
python fractal_debugging_master.py "C:\Users\allen\Desktop\Alexander\Alexander" --mode monitor --interval 30
```

## Framework Usage Examples

### Example 1: Analyzing Build Failures
```python
from fractal_build_analyzer import FractalBuildAnalyzer

# Create analyzer
analyzer = FractalBuildAnalyzer("C:\\Users\\allen\\Desktop\\Alexander\\Alexander")

# Analyze build output
build_output = """... your build output ..."""
analysis = analyzer.analyze_build_errors(build_output)

# Print results
print_build_analysis(analysis)

# Generate intervention plan
interventions = analyzer.generate_intervention_plan(analysis)

# Apply interventions
from intervention_executor import execute_intervention_plan
results = execute_intervention_plan(interventions, project_path)
```

### Example 2: Project-Wide Pattern Detection
```python
from fractal_debugger import FractalDebugger

# Create debugger
debugger = FractalDebugger("C:\\Users\\allen\\Desktop\\Alexander\\Alexander")

# Analyze entire project
results = debugger.debug_project()

# Print summary
print_analysis_summary(results)

# Get top patterns
from pattern_signatures_unreal import get_top_patterns
top_patterns = get_top_patterns(3)
```

### Example 3: State Health Monitoring
```python
from file_based_state_scanner import FileBasedStateScanner

# Create scanner
scanner = FileBasedStateScanner("C:\\Users\\allen\\Desktop\\Alexander\\Alexander")

# Generate state report
report = scanner.generate_state_report()

# Print report
print_state_report(report)

# Continuous monitoring
monitor_state_health(project_path, interval=60)
```

## Understanding Output

### Pattern Detection Output
```json
{
  "pattern_type": "null_cascade_ue",
  "file_path": "Source/Alexander/Private/TradeShipAutomation.cpp",
  "line_number": 45,
  "amplitude": 0.75,
  "scale": "meso",
  "resonant_scale": "meso",
  "is_systemic": true,
  "coherence_score": 0.82
}
```

**Key Metrics:**
- **Amplitude**: Pattern strength (0.0 to 1.0)
- **Scale**: Where pattern manifests (micro/meso/macro/meta)
- **Resonant Scale**: Optimal intervention point
- **Systemic**: Whether pattern cascades across scales
- **Coherence Score**: Cross-scale correlation (higher = more systemic)

### Intervention Output
```json
{
  "description": "Convert TWeakObjectPtr operator bool() to IsValid() checks",
  "confidence": 0.85,
  "cascade_prediction": {
    "file_impact": 0.7,
    "build_impact": 0.8,
    "runtime_impact": 0.9
  },
  "prerequisites": [
    "Verify TWeakObjectPtr type",
    "Check object lifecycle"
  ]
}
```

**Key Metrics:**
- **Confidence**: Intervention success probability
- **Cascade Prediction**: Expected impact across dimensions
- **Prerequisites**: Required pre-conditions for safe application

### Cascade Measurement
```json
{
  "overall_score": 0.82,
  "file_impact": 0.75,
  "build_impact": 0.85,
  "runtime_impact": 0.88,
  "affected_files": [
    "Source/Alexander/Private/TradeShipAutomation.cpp",
    "Source/Alexander/Private/TradingEconomySystem.cpp"
  ]
}
```

**Key Metrics:**
- **Overall Score**: Weighted impact (F:30%, B:30%, R:40%)
- **Dimension Scores**: Impact on specific areas
- **Affected Files**: Files influenced by cascade

## Best Practices

### 1. **Start with Analysis**
Always run analysis before applying interventions to understand the full scope.

### 2. **Trust the Resonance**
Focus interventions on the resonant scale - this is where you'll get maximum cascade effect.

### 3. **Measure Everything**
The framework tracks historical success rates - use this data to refine your approach.

### 4. **Batch with Caution**
Use batch mode carefully - it's "all or nothing" and can roll back if any intervention fails.

### 5. **Monitor Continuously**
Run continuous monitoring during active development to catch issues early.

## Integration with Existing Workflow

### Hook into Build Process
```python
# Add to build_and_test.py
from fractal_build_analyzer import analyze_current_build

def run_build_with_analysis():
    # Run build
    build_result = run_build()
    
    # Analyze results
    analysis = analyze_current_build(project_path)
    
    # Auto-apply if needed
    if analysis.total_errors > 0:
        execute_intervention_plan_from_analysis(analysis, project_path)
```

### CI/CD Integration
```yaml
# Add to CI pipeline
- name: Run Fractal Analysis
  run: |
    python fractal_debugging_master.py . --mode analyze --build-output build.log
    
- name: Apply Auto-Fixes
  if: failure()
  run: |
    python fractal_debugging_master.py . --mode fix --auto-apply
```

## Troubleshooting

### "No patterns detected"
- Ensure you're analyzing the right file types (`.cpp`, `.h`)
- Check that files are in the expected directory structure
- Verify pattern signatures match your codebase

### "Interventions not generated"
- Patterns may be detected but not actionable
- Check pattern confidence scores
- Review if patterns are below intervention thresholds

### "Cascade effects not as predicted"
- Update pattern history with actual results
- Adjust cascade prediction weights based on experience
- Consider edge cases in your specific codebase

## Advanced Usage

### Custom Pattern Signatures
```python
# Extend pattern_signatures_unreal.py
SIGNATURES['my_custom_pattern'] = {
    'name': 'My Custom Pattern',
    'indicators': ['custom_indicator_1', 'custom_indicator_2'],
    'scales': {'micro': ['line'], 'meso': ['function']},
    'resonance_points': ['custom_point'],
    'intervention_templates': {
        'custom_point': {
            'description': 'Custom intervention',
            'pattern': r'custom_pattern',
            'replacement': r'custom_fix',
            'cascade_prediction': {'file_impact': 0.7, 'build_impact': 0.6, 'runtime_impact': 0.8}
        }
    },
    'severity': 'medium',
    'frequency_weight': 0.5
}
```

### Custom Analysis Pipeline
```python
# Build custom analysis pipeline
from fractal_debugger import FractalInspector, ResonanceDetector
from intervention_executor import InterventionProtocol

# Create custom components
inspector = FractalInspector(project_path)
detector = ResonanceDetector()
protocol = InterventionProtocol('null_cascade_ue', 'meso')

# Run custom analysis
spectrum = inspector.scan_all_scales(signature, file_path)
resonant_scale, amplitude = detector.find_resonant_scale(spectrum)
interventions = protocol.generate_interventions(matches)
```

## Support and Contributing

The framework is designed to be extensible. To add new pattern types:
1. Define pattern signature in `pattern_signatures_unreal.py`
2. Add regex patterns to `PATTERN_REGEX`
3. Update intervention templates
4. Test with real-world examples
5. Update success rates based on results

## License

This framework is part of the Alexander project and follows the same licensing terms.