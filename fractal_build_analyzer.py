"""
Fractal Build Analyzer - Integrates the Fractal Debugging Framework with Unreal Build Tool output.
Analyzes compilation errors and generates targeted interventions using the Resonance Debugging Protocol.
"""

import os
import re
import json
import time
import subprocess
from typing import Dict, List, Tuple, Optional, Any
from dataclasses import dataclass, asdict

from pattern_signatures_unreal import SIGNATURES, get_pattern_signature
from fractal_debugger import FractalDebugger
from intervention_executor import InterventionExecutor


@dataclass
class BuildError:
    """Parsed build error information"""
    file_path: str
    line_number: int
    error_code: str
    message: str
    raw_text: str
    severity: str = "error"
    
    def to_dict(self):
        return asdict(self)


@dataclass
class BuildAnalysis:
    """Complete build analysis results"""
    timestamp: float
    total_errors: int
    error_categories: Dict[str, int]
    pattern_matches: List[Dict]
    interventions: List[Dict]
    summary: Dict
    
    def to_dict(self):
        return {
            'timestamp': self.timestamp,
            'total_errors': self.total_errors,
            'error_categories': self.error_categories,
            'pattern_matches': self.pattern_matches,
            'interventions': self.interventions,
            'summary': self.summary
        }


class FractalBuildAnalyzer:
    """
    Analyzes Unreal Build Tool output through the Fractal Debugging Framework lens.
    """
    
    def __init__(self, project_path: str):
        self.project_path = project_path
        self.debugger = FractalDebugger(project_path)
        self.error_patterns = {
            'null_reference': r"error C2065: '(.*?)': undeclared identifier",
            'missing_type': r"error C2027: use of undefined type '(.*?)'",
            'member_not_found': r"error C2039: '(.*?)': is not a member of '(.*?)'",
            'signature_mismatch': r"error C2660: '(.*?)': function does not take (\d+) arguments",
            'redefinition': r"error C2084: function '(.*?)' already has a body",
            'namespace_not_found': r"error C2653: '(.*?)': is not a class or namespace name",
            'identifier_not_found': r"error C3861: '(.*?)': identifier not found",
            'static_assert_failed': r"error C2338: static_assert failed: '(.*?)'",
            'cannot_convert': r"error C2440: '(.*?)': cannot convert from '(.*?)' to '(.*?)'",
            'no_operator': r"error C2679: binary '(.*?)': no operator found",
            'ambiguous_call': r"error C2593: '(.*?)' is ambiguous",
            'illegal_reference': r"error C2597: illegal reference to non-static member '(.*?)'",
            'non_standard_syntax': r"error C3867: '(.*?)': non-standard syntax",
            'cannot_access': r"error C2248: '(.*?)': cannot access (.*?) member",
            'fatal_error': r"fatal error C1083: Cannot open include file: '(.*?)'",
            'link_error': r"LINK : fatal error LNK(\d+): (.*)",
            'dll_error': r"ERROR opening file (.*?) for write after retrying",
            'macro_error': r"note: the template instantiation context",
            'overloaded_function': r"error C2556: '(.*?)': overloaded function differs only by return type",
            'function_not_found': r"error C2511: '(.*?)': overloaded member function not found in '(.*?)'"
        }
    
    def parse_build_output(self, build_output: str) -> List[BuildError]:
        """
        Parse Unreal Build Tool output into structured errors.
        
        Args:
            build_output: Raw build output text
            
        Returns:
            List[BuildError]: Parsed build errors
        """
        errors = []
        lines = build_output.split('\n')
        
        for line in lines:
            line = line.strip()
            if not line or 'error' not in line.lower():
                continue
            
            # Parse file path and line number (common format: path(line,col):)
            file_match = re.search(r'([A-Z]:[/\\][^:(]+)\((\d+),?(\d+)?\):', line, re.IGNORECASE)
            
            if file_match:
                file_path = file_match.group(1)
                line_number = int(file_match.group(2))
                
                # Determine error type and extract details
                error_type, message, severity = self._categorize_error(line)
                
                error = BuildError(
                    file_path=file_path,
                    line_number=line_number,
                    error_code=error_type,
                    message=message,
                    raw_text=line,
                    severity=severity
                )
                errors.append(error)
            
            # Also check for linker errors (different format)
            elif 'LINK :' in line:
                link_match = re.search(r'LINK : fatal error (LNK\d+): (.+)', line)
                if link_match:
                    error = BuildError(
                        file_path="linker",
                        line_number=0,
                        error_code=link_match.group(1),
                        message=link_match.group(2),
                        raw_text=line,
                        severity="link_error"
                    )
                    errors.append(error)
            
            # DLL locking errors
            elif 'ERROR opening file' in line:
                dll_match = re.search(r"ERROR opening file (.+) for write", line)
                if dll_match:
                    error = BuildError(
                        file_path=dll_match.group(1),
                        line_number=0,
                        error_code="DLL_LOCK",
                        message="File locked by another process",
                        raw_text=line,
                        severity="lock_error"
                    )
                    errors.append(error)
        
        return errors
    
    def _categorize_error(self, error_line: str) -> Tuple[str, str, str]:
        """
        Categorize an error line into type, message, and severity.
        
        Args:
            error_line: Error line text
            
        Returns:
            Tuple[str, str, str]: (error_type, message, severity)
        """
        for error_type, pattern in self.error_patterns.items():
            match = re.search(pattern, error_line)
            if match:
                # Extract meaningful message
                if error_type == 'null_reference':
                    message = f"Undeclared identifier: {match.group(1)}"
                elif error_type == 'missing_type':
                    message = f"Undefined type: {match.group(1)}"
                elif error_type == 'member_not_found':
                    message = f"Member '{match.group(1)}' not found in {match.group(2)}"
                elif error_type == 'signature_mismatch':
                    message = f"Function {match.group(1)} argument count mismatch"
                elif error_type == 'redefinition':
                    message = f"Function redefinition: {match.group(1)}"
                elif error_type == 'namespace_not_found':
                    message = f"Namespace/Class not found: {match.group(1)}"
                elif error_type == 'identifier_not_found':
                    message = f"Identifier not found: {match.group(1)}"
                elif error_type == 'static_assert_failed':
                    message = f"Static assertion failed: {match.group(1)}"
                elif error_type == 'cannot_convert':
                    message = f"Cannot convert {match.group(2)} to {match.group(3)}"
                elif error_type == 'no_operator':
                    message = f"No operator found for {match.group(1)}"
                elif error_type == 'ambiguous_call':
                    message = f"Ambiguous function call: {match.group(1)}"
                elif error_type == 'illegal_reference':
                    message = f"Illegal reference to non-static member: {match.group(1)}"
                elif error_type == 'non_standard_syntax':
                    message = f"Non-standard syntax: {match.group(1)}"
                elif error_type == 'cannot_access':
                    message = f"Cannot access member: {match.group(1)}"
                elif error_type == 'fatal_error':
                    message = f"Cannot open include file: {match.group(1)}"
                elif error_type == 'overloaded_function':
                    message = f"Overloaded function differs only by return type: {match.group(1)}"
                elif error_type == 'function_not_found':
                    message = f"Member function not found: {match.group(1)}"
                else:
                    message = error_line
                
                severity = "error"
                if "fatal error" in error_line:
                    severity = "fatal"
                elif "warning" in error_line.lower():
                    severity = "warning"
                
                return error_type, message, severity
        
        # Default categorization
        return "unknown", error_line, "error"
    
    def map_error_to_pattern(self, error: BuildError) -> Optional[str]:
        """
        Map a build error to a fractal pattern signature.
        
        Args:
            error: Build error to map
            
        Returns:
            Optional[str]: Pattern type or None
        """
        error_code = error.error_code
        
        # Map error types to patterns
        pattern_mapping = {
            'null_reference': 'null_cascade_ue',
            'missing_type': 'null_cascade_ue',
            'member_not_found': 'null_cascade_ue',
            'namespace_not_found': 'null_cascade_ue',
            'identifier_not_found': 'null_cascade_ue',
            'fatal_error': 'null_cascade_ue',
            'signature_mismatch': 'api_drift_ue',
            'redefinition': 'api_drift_ue',
            'static_assert_failed': 'api_drift_ue',
            'cannot_convert': 'api_drift_ue',
            'no_operator': 'api_drift_ue',
            'ambiguous_call': 'api_drift_ue',
            'overloaded_function': 'api_drift_ue',
            'function_not_found': 'api_drift_ue',
            'illegal_reference': 'state_desync_ue',
            'non_standard_syntax': 'state_desync_ue',
            'cannot_access': 'coupling_cascade_ue',
            'link_error': 'coupling_cascade_ue',
            'dll_error': 'state_desync_ue'
        }
        
        return pattern_mapping.get(error_code)
    
    def analyze_build_errors(self, build_output: str) -> BuildAnalysis:
        """
        Analyze build output and generate fractal debugging insights.
        
        Args:
            build_output: Raw build output from UBT
            
        Returns:
            BuildAnalysis: Complete analysis results
        """
        # Parse errors
        errors = self.parse_build_output(build_output)
        
        # Categorize by pattern
        pattern_errors = {}
        for error in errors:
            pattern_type = self.map_error_to_pattern(error)
            if pattern_type:
                if pattern_type not in pattern_errors:
                    pattern_errors[pattern_type] = []
                pattern_errors[pattern_type].append(error)
        
        # Analyze each pattern group
        pattern_matches = []
        interventions = []
        error_categories = {}
        
        for pattern_type, error_list in pattern_errors.items():
            signature = get_pattern_signature(pattern_type)
            if not signature:
                continue
            
            # Count errors by category
            error_categories[signature['name']] = len(error_list)
            
            # Analyze the most severe file for this pattern
            if error_list:
                # Group by file
                files = {}
                for error in error_list:
                    if error.file_path not in files:
                        files[error.file_path] = []
                    files[error.file_path].append(error)
                
                # Analyze the file with most errors
                target_file = max(files.keys(), key=lambda f: len(files[f]))
                
                # Run fractal analysis on this file
                if os.path.exists(target_file):
                    file_results = self.debugger.debug(target_file)
                    
                    # Add pattern context
                    for match in file_results.get('pattern_matches', []):
                        match['build_error_context'] = [
                            {
                                'line': e.line_number,
                                'message': e.message,
                                'code': e.error_code
                            }
                            for e in files[target_file]
                        ]
                        pattern_matches.append(match)
                    
                    # Collect interventions
                    interventions.extend(file_results.get('interventions', []))
        
        # Generate summary
        summary = self._generate_summary(errors, pattern_errors, interventions)
        
        return BuildAnalysis(
            timestamp=time.time(),
            total_errors=len(errors),
            error_categories=error_categories,
            pattern_matches=pattern_matches,
            interventions=interventions,
            summary=summary
        )
    
    def _generate_summary(self, errors: List[BuildError], 
                         pattern_errors: Dict[str, List[BuildError]],
                         interventions: List[Dict]) -> Dict:
        """
        Generate analysis summary.
        
        Args:
            errors: All build errors
            pattern_errors: Errors grouped by pattern
            interventions: Generated interventions
            
        Returns:
            Dict: Summary information
        """
        if not errors:
            return {
                'status': 'success',
                'message': 'No build errors detected',
                'recommended_action': None
            }
        
        # Find dominant pattern
        dominant_pattern = None
        max_errors = 0
        
        for pattern_type, error_list in pattern_errors.items():
            if len(error_list) > max_errors:
                max_errors = len(error_list)
                dominant_pattern = pattern_type
        
        # Calculate severity
        fatal_errors = sum(1 for e in errors if e.severity == 'fatal')
        error_count = len(errors)
        
        if fatal_errors > 0:
            severity = 'critical'
        elif error_count > 50:
            severity = 'high'
        elif error_count > 20:
            severity = 'medium'
        else:
            severity = 'low'
        
        # Generate recommendation
        if interventions:
            # Sort by confidence and cascade score
            top_interventions = sorted(
                interventions,
                key=lambda x: x.get('confidence', 0) * x.get('cascade_prediction', {}).get('overall_score', 0),
                reverse=True
            )
            
            if top_interventions:
                recommended = top_interventions[0]
                recommended_action = f"Apply intervention: {recommended['description']} " \
                                     f"(confidence: {recommended['confidence']:.2f})"
            else:
                recommended_action = "Manual review required - no high-confidence interventions"
        else:
            recommended_action = "Manual review required - no automated interventions available"
        
        return {
            'status': f'failed_{severity}',
            'total_errors': error_count,
            'fatal_errors': fatal_errors,
            'dominant_pattern': get_pattern_signature(dominant_pattern)['name'] if dominant_pattern else 'none',
            'pattern_coverage': len(pattern_errors) / len(SIGNATURES) if SIGNATURES else 0,
            'intervention_count': len(interventions),
            'recommended_action': recommended_action
        }
    
    def run_build_and_analyze(self, build_command: str) -> BuildAnalysis:
        """
        Run build command and analyze results.
        
        Args:
            build_command: Command to run build
            
        Returns:
            BuildAnalysis: Analysis of build results
        """
        print(f"Running build: {build_command}")
        
        try:
            # Run build command
            result = subprocess.run(
                build_command,
                shell=True,
                capture_output=True,
                text=True,
                cwd=self.project_path,
                timeout=1800  # 30 minute timeout
            )
            
            build_output = result.stdout + "\n" + result.stderr
            
            # Analyze output
            analysis = self.analyze_build_errors(build_output)
            
            print(f"\nBuild completed with {analysis.total_errors} errors")
            print(f"Analysis generated {len(analysis.interventions)} interventions")
            
            return analysis
            
        except subprocess.TimeoutExpired:
            return BuildAnalysis(
                timestamp=time.time(),
                total_errors=1,
                error_categories={'Build Timeout': 1},
                pattern_matches=[],
                interventions=[],
                summary={
                    'status': 'timeout',
                    'message': 'Build timed out after 30 minutes',
                    'recommended_action': 'Check for infinite loops or hanging processes'
                }
            )
        except Exception as e:
            return BuildAnalysis(
                timestamp=time.time(),
                total_errors=1,
                error_categories={'Build Error': 1},
                pattern_matches=[],
                interventions=[],
                summary={
                    'status': 'error',
                    'message': f'Build failed: {str(e)}',
                    'recommended_action': 'Check build command and environment'
                }
            )
    
    def generate_intervention_plan(self, analysis: BuildAnalysis, 
                                  max_interventions: int = 10) -> List[Dict]:
        """
        Generate prioritized intervention plan from analysis.
        
        Args:
            analysis: Build analysis results
            max_interventions: Maximum interventions to include
            
        Returns:
            List[Dict]: Prioritized intervention list
        """
        interventions = analysis.interventions
        
        if not interventions:
            return []
        
        # Score each intervention
        scored_interventions = []
        for intervention in interventions:
            # Base score on confidence
            score = intervention.get('confidence', 0.0)
            
            # Boost based on cascade prediction
            cascade = intervention.get('cascade_prediction', {})
            cascade_score = (
                cascade.get('file_impact', 0) * 0.3 +
                cascade.get('build_impact', 0) * 0.3 +
                cascade.get('runtime_impact', 0) * 0.4
            )
            
            # Adjust by pattern severity
            pattern_type = intervention.get('pattern_type', '')
            signature = get_pattern_signature(pattern_type)
            if signature:
                severity_weights = {'critical': 1.0, 'high': 0.8, 'medium': 0.6, 'low': 0.4}
                severity_boost = severity_weights.get(signature['severity'], 0.5)
                
                # Combine scores
                final_score = score * 0.4 + cascade_score * 0.4 + severity_boost * 0.2
            else:
                final_score = score * 0.5 + cascade_score * 0.5
            
            scored_interventions.append((final_score, intervention))
        
        # Sort by score and return top interventions
        scored_interventions.sort(key=lambda x: x[0], reverse=True)
        
        return [intervention for score, intervention in scored_interventions[:max_interventions]]


def analyze_current_build(project_path: str, build_output_path: Optional[str] = None) -> BuildAnalysis:
    """
    Analyze current or recent build.
    
    Args:
        project_path: Path to Unreal project
        build_output_path: Optional path to build output file
        
    Returns:
        BuildAnalysis: Analysis results
    """
    analyzer = FractalBuildAnalyzer(project_path)
    
    if build_output_path and os.path.exists(build_output_path):
        # Analyze from file
        with open(build_output_path, 'r') as f:
            build_output = f.read()
        
        return analyzer.analyze_build_errors(build_output)
    else:
        # Run fresh build
        uproject_file = os.path.join(project_path, "Alexander.uproject")
        build_cmd = f'ubt.bat AlexanderEditor Win64 Development -Project="{uproject_file}" -WaitMutex -FromMsBuild'
        
        return analyzer.run_build_and_analyze(build_cmd)


def print_build_analysis(analysis: BuildAnalysis):
    """Print human-readable build analysis"""
    print("\n" + "="*80)
    print("FRACTAL BUILD ANALYSIS")
    print("="*80)
    
    summary = analysis.summary
    print(f"\nBuild Status: {summary.get('status', 'unknown').upper()}")
    print(f"Total Errors: {analysis.total_errors}")
    print(f"Error Categories: {len(analysis.error_categories)}")
    print(f"Interventions Generated: {len(analysis.interventions)}")
    print(f"\nRecommended Action: {summary.get('recommended_action', 'None')}")
    
    if analysis.error_categories:
        print("\nError Categories:")
        for category, count in analysis.error_categories.items():
            print(f"  - {category}: {count} errors")
    
    if analysis.interventions:
        print("\nTop 5 Recommended Interventions:")
        for i, intervention in enumerate(analysis.interventions[:5]):
            print(f"\n  {i+1}. {intervention['description']}")
            print(f"     File: {intervention['file_path']}:{intervention['line_number']}")
            print(f"     Confidence: {intervention['confidence']:.2f}")
            cascade = intervention.get('cascade_prediction', {})
            print(f"     Predicted Impact - File: {cascade.get('file_impact', 0):.2f}, "
                  f"Build: {cascade.get('build_impact', 0):.2f}, "
                  f"Runtime: {cascade.get('runtime_impact', 0):.2f}")
    
    print("\n" + "="*80)


def save_analysis(analysis: BuildAnalysis, output_path: str):
    """Save analysis to JSON file"""
    with open(output_path, 'w') as f:
        json.dump(analysis.to_dict(), f, indent=2, default=str)


def load_analysis(input_path: str) -> BuildAnalysis:
    """Load analysis from JSON file"""
    with open(input_path, 'r') as f:
        data = json.load(f)
    
    return BuildAnalysis(**data)


def execute_intervention_plan_from_analysis(analysis: BuildAnalysis, project_path: str, 
                                          auto_apply: bool = False):
    """
    Generate and optionally execute intervention plan from analysis.
    
    Args:
        analysis: Build analysis results
        project_path: Project path
        auto_apply: Whether to auto-apply interventions
    """
    analyzer = FractalBuildAnalyzer(project_path)
    
    # Generate intervention plan
    interventions = analyzer.generate_intervention_plan(analysis)
    
    print(f"\nGenerated {len(interventions)} prioritized interventions")
    
    if not interventions:
        print("No interventions available. Manual review required.")
        return
    
    # Print intervention summary
    print("\nIntervention Plan:")
    total_cascade_score = 0
    
    for i, intervention in enumerate(interventions):
        cascade = intervention.get('cascade_prediction', {})
        cascade_score = (
            cascade.get('file_impact', 0) * 0.3 +
            cascade.get('build_impact', 0) * 0.3 +
            cascade.get('runtime_impact', 0) * 0.4
        )
        total_cascade_score += cascade_score
        
        print(f"\n  {i+1}. {intervention['description']}")
        print(f"     Location: {intervention['file_path']}:{intervention['line_number']}")
        print(f"     Confidence: {intervention['confidence']:.2f}")
        print(f"     Cascade Score: {cascade_score:.2f}")
    
    avg_cascade_score = total_cascade_score / len(interventions)
    print(f"\nAverage Cascade Score: {avg_cascade_score:.2f}")
    
    if auto_apply:
        print(f"\nAuto-applying {len(interventions)} interventions...")
        executor = InterventionExecutor(project_path)
        results = executor.apply_interventions(interventions, batch_mode=True)
        
        successful = sum(1 for r in results if r.success)
        print(f"Applied {successful}/{len(results)} interventions successfully")
        
        if successful > 0:
            print(f"\nRecommended: Run build again to verify fixes")
    else:
        print(f"\nRecommended: Review interventions and apply selectively")
        print(f"To auto-apply, run with auto_apply=True")


# Example usage and testing
if __name__ == "__main__":
    # Test with sample build output
    sample_output = """
C:\\Users\\allen\\Desktop\\Alexander\\Alexander\\Source\\Alexander\\Private\\FarmPlot.cpp(314,17): error C2556: 'void AFarmPlot::WaterPlot(float)': overloaded function differs only by return type from 'bool AFarmPlot::WaterPlot(float)'
C:\\Users\\allen\\Desktop\\Alexander\\Alexander\\Source\\Alexander\\Private\\FarmPlot.cpp(333,17): error C2511: 'void AFarmPlot::FertilizePlot(float)': overloaded member function not found in 'AFarmPlot'
C:\\Users\\allen\\Desktop\\Alexander\\Alexander\\Source\\Alexander\\Tests\\OrbitalMechanicsTest.cpp(30,16): error C2653: 'FOrbitalMechanics': is not a class or namespace name
C:\\Users\\allen\\Desktop\\Alexander\\Alexander\\Source\\Alexander\\Private\\OrbitalMechanicsManager.cpp(232,12): error C2039: 'Tick': is not a member of 'UWorldSubsystem'
LINK : fatal error LNK1104: cannot open file 'C:\\Users\\allen\\Desktop\\Alexander\\Alexander\\Plugins\\AIVisionSystem\\Binaries\\Win64\\UnrealEditor-AIVisionSystem.dll'
    """
    
    # Create analyzer
    analyzer = FractalBuildAnalyzer("C:\\Users\\allen\\Desktop\\Alexander\\Alexander")
    
    # Analyze build output
    analysis = analyzer.analyze_build_errors(sample_output)
    
    # Print results
    print_build_analysis(analysis)
    
    # Generate and display intervention plan
    execute_intervention_plan_from_analysis(analysis, "C:\\Users\\allen\\Desktop\\Alexander\\Alexander")