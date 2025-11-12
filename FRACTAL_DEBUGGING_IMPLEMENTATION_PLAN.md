# Fractal Debugging Framework - Implementation Plan

## Overview

This document provides the complete implementation specification for the Fractal Debugging Framework. The framework transforms reactive debugging into systematic, pattern-based root cause analysis with measurable cascade effects.

## Implementation Files

### 1. [`fractal_debugger.py`](fractal_debugger.py:1) - Core Engine

**Purpose**: Central orchestrator for the Resonance Debugging Protocol

**Key Classes**:
```python
class ResonantScale(Enum):
    MICRO = "micro"      # Function/variable level
    MESO = "meso"        # Class/component level  
    MACRO = "macro"      # Module/system level
    META = "meta"        # Project/architecture level

class PatternMatch:
    pattern_id: str
    resonant_scale: ResonantScale
    confidence: float
    location: Dict[str, Any]
    context: Dict[str, Any]
    cascade_prediction: float

class FractalDebugger:
    def __init__(self, project_root: Path):
        self.pattern_detector = PatternSignatureEngine()
        self.cascade_analyzer = CascadeAnalyzer()
        self.intervention_planner = InterventionPlanner()
        self.state_scanner = FileBasedStateScanner()
        
    def analyze_build_failure(self, build_output: str) -> List[PatternMatch]:
        """Analyze build output and detect resonance patterns"""
        
    def calculate_cascade_impact(self, pattern: PatternMatch) -> CascadeMetrics:
        """Calculate predicted cascade effect of pattern"""
        
    def generate_intervention(self, pattern: PatternMatch) -> InterventionPlan:
        """Generate targeted intervention for pattern"""
```

**Implementation Details**:
- Load pattern signatures from [`pattern_signatures_unreal.py`](pattern_signatures_unreal.py:1)
- Integrate with state scanner for context awareness
- Support both real-time and post-build analysis modes
- Provide confidence scoring for pattern matches

### 2. [`pattern_signatures_unreal.py`](pattern_signatures_unreal.py:1) - UE Pattern Library

**Purpose**: Unreal Engine-specific pattern definitions and detection

**Pattern Definitions**:
```python
PATTERN_SIGNATURES = {
    "null_cascade_ue": {
        "pattern_id": "null_cascade_ue",
        "resonant_scale": "micro",
        "base_priority": 0.9,
        "cascade_weight": 0.85,
        "signatures": [
            r"Error:\s*Accessed\s+None",
            r"Error:\s*nullptr\s+dereference",
            r"Warning:\s*TWeakObjectPtr.*not\s+valid",
            r"Error:\s*UPROPERTY.*not\s+initialized"
        ],
        "context_indicators": [
            "TWeakObjectPtr",
            "UPROPERTY",
            "nullptr",
            "IsValid()"
        ],
        "file_patterns": ["*.cpp", "*.h"],
        "description": "Null pointer cascade - potential chain reaction crashes"
    },
    
    "state_desync_ue": {
        "pattern_id": "state_desync_ue", 
        "resonant_scale": "meso",
        "base_priority": 0.7,
        "cascade_weight": 0.6,
        "signatures": [
            r"Warning:\s*Replicated\s+property.*changed",
            r"Error:\s*Subsystem.*initialization\s+order",
            r"Warning:\s*State\s+mismatch",
            r"Error:\s*Replication\s+lag\s+detected"
        ],
        "context_indicators": [
            "Replicated",
            "Subsystem",
            "GameState",
            "Replication"
        ],
        "file_patterns": ["*.cpp", "*.h", "*.build.cs"],
        "description": "State desynchronization - inconsistent gameplay behavior"
    },
    
    "coupling_cascade_ue": {
        "pattern_id": "coupling_cascade_ue",
        "resonant_scale": "meso", 
        "base_priority": 0.8,
        "cascade_weight": 0.9,
        "signatures": [
            r"Error:\s*Circular\s+dependency",
            r"Warning:\s*Excessive\s+friend\s+class",
            r"Error:\s*Cross-module\s+include\s+dependency",
            r"Warning:\s*Tight\s+coupling\s+detected"
        ],
        "context_indicators": [
            "friend class",
            "Circular dependency",
            "cross-module",
            "tight coupling"
        ],
        "file_patterns": ["*.build.cs", "*.cpp", "*.h"],
        "description": "Tight coupling cascade - compilation and maintainability issues"
    },
    
    "api_drift_ue": {
        "pattern_id": "api_drift_ue",
        "resonant_scale": "macro",
        "base_priority": 0.75,
        "cascade_weight": 0.7,
        "signatures": [
            r"Error:\s*Function.*deprecated",
            r"Error:\s*Signature\s+mismatch",
            r"Warning:\s*API\s+version\s+conflict",
            r"Error:\s*Engine\s+version\s+incompatible"
        ],
        "context_indicators": [
            "deprecated",
            "signature mismatch", 
            "API version",
            "engine version"
        ],
        "file_patterns": ["*.cpp", "*.h", "*.uplugin"],
        "description": "API drift/deprecation - build failures across multiple files"
    }
}
```

**Detection Engine**:
```python
class PatternSignatureEngine:
    def __init__(self):
        self.signatures = self._load_signatures()
        self.compiled_patterns = self._compile_patterns()
        
    def detect_patterns(self, text: str, context: Dict) -> List[PatternMatch]:
        """Detect patterns in build output or code"""
        
    def calculate_dynamic_priority(self, pattern_id: str, 
                                 frequency: int, 
                                 confidence: float) -> float:
        """Calculate dynamic priority based on historical data"""
```

### 3. [`file_based_state_scanner.py`](file_based_state_scanner.py:1) - State Integration

**Purpose**: Monitor and integrate with existing file-based state coordination

**Key Functions**:
```python
class FileBasedStateScanner:
    def __init__(self, project_root: Path):
        self.session_state_path = project_root / "session_state.json"
        self.execution_queue_path = project_root / "execution_queue.json"
        self.build_log_pattern = "build_output*.txt"
        
    def get_session_context(self) -> Dict[str, Any]:
        """Extract context from session_state.json"""
        
    def get_execution_patterns(self) -> List[Dict[str, Any]]:
        """Analyze execution_queue.json for failure patterns"""
        
    def extract_build_metrics(self, build_output: str) -> BuildMetrics:
        """Parse build output for timing and error metrics"""
        
    def update_fractal_state(self, analysis_results: Dict):
        """Update framework state files"""
```

**Integration Points**:
- Read [`session_state.json`](session_state.json:1) for phase and task context
- Monitor [`execution_queue.json`](execution_queue.json:1) for pending/completed tasks
- Parse build output files for error patterns and metrics
- Write analysis results to [`fractal_analysis_results.json`](fractal_analysis_results.json:1)

### 4. [`intervention_executor.py`](intervention_executor.py:1) - Fix Application

**Purpose**: Apply targeted interventions and measure cascade effects

**Key Classes**:
```python
class InterventionPlan:
    pattern_id: str
    target_files: List[Path]
    fix_operations: List[Dict[str, Any]]
    predicted_cascade: CascadeMetrics
    rollback_plan: Dict[str, Any]

class CascadeMetrics:
    file_impact_score: float
    build_impact_score: float
    runtime_impact_score: float
    total_cascade_score: float
    files_modified: int
    dependencies_affected: int
    compilation_time_delta: float
    modules_rebuilt: int

class InterventionExecutor:
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.backup_manager = BackupManager()
        
    def execute_intervention(self, plan: InterventionPlan) -> ExecutionResult:
        """Apply fix and measure cascade effect"""
        
    def measure_cascade_impact(self, before: BuildMetrics, 
                              after: BuildMetrics) -> CascadeMetrics:
        """Calculate actual cascade effect of intervention"""
        
    def rollback_intervention(self, plan: InterventionPlan) -> bool:
        """Revert intervention if it causes issues"""
```

**Intervention Types**:
- **Null Cascade Fix**: Add nullptr checks, initialize UPROPERTY
- **State Desync Fix**: Reorder subsystem initialization, add replication validation
- **Coupling Fix**: Break circular dependencies, refactor cross-module includes
- **API Drift Fix**: Update deprecated functions, fix signature mismatches

### 5. [`fractal_build_analyzer.py`](fractal_build_analyzer.py:1) - Build Analysis

**Purpose**: Real-time and post-build analysis of compilation output

**Key Functions**:
```python
class FractalBuildAnalyzer:
    def __init__(self, debugger: FractalDebugger):
        self.debugger = debugger
        self.build_monitor = BuildMonitor()
        
    def monitor_realtime_build(self, process: subprocess.Popen):
        """Monitor build process in real-time"""
        
    def analyze_build_output(self, build_log: str) -> BuildAnalysis:
        """Post-build analysis of complete output"""
        
    def correlate_errors_to_patterns(self, errors: List[BuildError]) -> List[PatternMatch]:
        """Link build errors to pattern signatures"""
        
    def extract_build_metrics(self, build_log: str) -> BuildMetrics:
        """Extract timing, module, and performance metrics"""
```

**Build Metrics Extraction**:
- Compilation time per module
- Error/warning counts and types
- Module rebuild patterns
- Linking time and dependencies
- Header include cascade detection

## Output Schemas

### 6. [`fractal_analysis_results.json`](fractal_analysis_results.json:1) Schema

```json
{
  "analysis_metadata": {
    "timestamp": "2025-11-12T20:00:00Z",
    "build_id": "build_20251112_200000",
    "analyzer_version": "1.0.0",
    "analysis_duration_seconds": 2.45
  },
  "build_context": {
    "session_id": "PHASE4_COMPLETE",
    "phase": "CONTENT_EXPANSION",
    "configuration": "Development",
    "platform": "Win64"
  },
  "detected_patterns": [
    {
      "pattern_id": "null_cascade_ue",
      "resonant_scale": "micro",
      "confidence": 0.92,
      "dynamic_priority": 0.87,
      "locations": [
        {
          "file": "Source/Alexander/Private/Spaceship.cpp",
          "line": 156,
          "context": "TWeakObjectPtr<UFactionComponent> FactionPtr;"
        }
      ],
      "cascade_prediction": {
        "file_impact_score": 0.45,
        "build_impact_score": 0.3,
        "runtime_impact_score": 0.85,
        "total_cascade_score": 0.57
      },
      "recommended_intervention": {
        "type": "add_nullptr_check",
        "target_files": ["Source/Alexander/Private/Spaceship.cpp"],
        "estimated_fix_time_seconds": 30
      }
    }
  ],
  "cascade_analysis": {
    "total_patterns_detected": 1,
    "highest_priority_pattern": "null_cascade_ue",
    "predicted_build_impact": "medium",
    "recommended_action": "apply_intervention"
  },
  "build_metrics": {
    "total_errors": 1,
    "total_warnings": 3,
    "compilation_time_seconds": 156.7,
    "modules_built": 12,
    "modules_failed": 1
  }
}
```

### 7. [`intervention_log.json`](intervention_log.json:1) Schema

```json
{
  "intervention_metadata": {
    "intervention_id": "int_20251112_200100",
    "timestamp": "2025-11-12T20:01:00Z",
    "pattern_id": "null_cascade_ue",
    "triggered_by": "fractal_debugger",
    "automation_level": "semi_automated"
  },
  "intervention_details": {
    "target_pattern": "null_cascade_ue",
    "target_files": [
      "Source/Alexander/Private/Spaceship.cpp"
    ],
    "fix_operations": [
      {
        "type": "add_null_check",
        "file": "Source/Alexander/Private/Spaceship.cpp",
        "line": 158,
        "code_added": "if (FactionPtr.IsValid()) { /* existing code */ }"
      }
    ],
    "backup_created": true,
    "backup_location": "Backups/intervention_int_20251112_200100/"
  },
  "cascade_measurement": {
    "pre_intervention": {
      "build_success": false,
      "compilation_time": 156.7,
      "error_count": 1
    },
    "post_intervention": {
      "build_success": true,
      "compilation_time": 142.3,
      "error_count": 0
    },
    "cascade_metrics": {
      "file_impact_score": 0.42,
      "build_impact_score": -0.15,
      "runtime_impact_score": 0.0,
      "total_cascade_score": 0.11
    },
    "files_modified": 1,
    "dependencies_affected": 0,
    "modules_rebuilt": 1
  },
  "intervention_outcome": {
    "success": true,
    "build_fixed": true,
    "side_effects_detected": false,
    "validation_passed": true,
    "rollback_performed": false
  }
}
```

## Integration with [`build_and_test.py`](build_and_test.py:1)

### Integration Points

**1. Post-Build Hook**:
```python
# In build_and_test.py, add to build_project method
def build_project(self, configuration="Development"):
    success = self.run_command(command)
    
    # Fractal analysis integration
    if not success:
        from fractal_build_analyzer import FractalBuildAnalyzer
        analyzer = FractalBuildAnalyzer()
        
        # Analyze build failure
        analysis = analyzer.analyze_build_output(
            self.get_build_log()
        )
        
        # Save analysis results
        analyzer.save_analysis_results(analysis)
        
        # Generate intervention if patterns detected
        if analysis['detected_patterns']:
            debugger = FractalDebugger(self.project_root)
            intervention = debugger.generate_intervention(
                analysis['detected_patterns'][0]
            )
            
            # Log intervention plan
            with open("intervention_plan.json", "w") as f:
                json.dump(intervention, f, indent=2)
                
    return success
```

**2. Real-Time Monitoring**:
```python
# Add to run_command method for real-time analysis
def run_command(self, command, cwd=None, timeout=300):
    process = subprocess.Popen(
        command, 
        shell=True, 
        cwd=cwd or self.project_root,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    
    # Real-time fractal monitoring
    from fractal_build_analyzer import FractalBuildAnalyzer
    analyzer = FractalBuildAnalyzer()
    analyzer.monitor_realtime_build(process)
    
    stdout, stderr = process.communicate(timeout=timeout)
    # ... rest of method
```

## Example Workflow

### Sample Build Failure Analysis

**Input**: Build failure with error:
```
Error: Accessed None trying to read property FactionPtr
File: Source/Alexander/Private/Spaceship.cpp:156
```

**Framework Processing**:
1. **Pattern Detection**: [`fractal_build_analyzer.py`](fractal_build_analyzer.py:1) detects `null_cascade_ue` pattern
2. **Scale Identification**: [`fractal_debugger.py`](fractal_debugger.py:1) identifies Micro → Meso scale
3. **Cascade Prediction**: Calculates impact scores:
   - File Impact: 0.45 (moderate)
   - Build Impact: 0.3 (low-moderate) 
   - Runtime Impact: 0.85 (high)
   - Total: 0.57 (medium-high risk)

4. **Intervention Generation**: [`intervention_executor.py`](intervention_executor.py:1) creates plan:
   - Add nullptr check before property access
   - Target file: Spaceship.cpp
   - Estimated fix time: 30 seconds
   - Predicted cascade reduction: 65%

5. **Execution & Measurement**: Apply fix and measure actual cascade:
   - Build time reduced from 156.7s to 142.3s
   - Error count: 1 → 0
   - No side effects detected
   - Cascade score: 0.57 → 0.11 (81% improvement)

## Testing Strategy

### Test Case 1: Null Cascade Pattern
- **Input**: Build error with TWeakObjectPtr access
- **Expected**: Detect `null_cascade_ue`, generate nullptr check intervention
- **Validation**: Measure cascade improvement

### Test Case 2: State Desync Pattern  
- **Input**: Subsystem initialization order warning
- **Expected**: Detect `state_desync_ue`, recommend reordering
- **Validation**: Verify runtime behavior improvement

### Test Case 3: Coupling Cascade Pattern
- **Input**: Circular dependency error
- **Expected**: Detect `coupling_cascade_ue`, suggest refactoring
- **Validation**: Measure compilation time reduction

## Success Metrics

The framework implementation is successful when:

1. **Pattern Detection Accuracy**: >90% accuracy in identifying known patterns
2. **Cascade Prediction Correlation**: >0.7 correlation between predicted and actual cascade scores
3. **Intervention Success Rate**: >80% of interventions resolve target issues
4. **Integration Stability**: Zero impact on existing build pipeline when no patterns detected
5. **Performance Overhead**: <5% increase in build analysis time

## Implementation Priority

**Phase 1** (Core Framework):
1. [`fractal_debugger.py`](fractal_debugger.py:1) - Core engine
2. [`pattern_signatures_unreal.py`](pattern_signatures_unreal.py:1) - Pattern library
3. Output schemas ([`fractal_analysis_results.json`](fractal_analysis_results.json:1), [`intervention_log.json`](intervention_log.json:1))

**Phase 2** (Integration):
4. [`file_based_state_scanner.py`](file_based_state_scanner.py:1) - State integration
5. [`fractal_build_analyzer.py`](fractal_build_analyzer.py:1) - Build analysis
6. Integration with [`build_and_test.py`](build_and_test.py:1)

**Phase 3** (Execution):
7. [`intervention_executor.py`](intervention_executor.py:1) - Fix application
8. Example workflow and testing

This implementation plan provides a complete blueprint for building the Fractal Debugging Framework that will transform Alexander's debugging approach from reactive to systematic pattern-based analysis.