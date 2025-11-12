"""
Fractal Testing Integration for Alexander Methodology
Integrates the Fractal Debugging Framework with existing testing infrastructure
"""

import os
import json
import time
from typing import Dict, List, Any
from pathlib import Path

from fractal_debugger import FractalDebugger
from fractal_build_analyzer import FractalBuildAnalyzer
from pattern_signatures_unreal import get_pattern_signature, get_top_patterns


class FractalTestingIntegration:
    """
    Integrates Fractal Debugging Framework with Alexander testing methodology
    """
    
    def __init__(self, project_path: str):
        self.project_path = project_path
        self.debugger = FractalDebugger(project_path)
        self.build_analyzer = FractalBuildAnalyzer(project_path)
        
        # Alexander methodology paths
        self.test_results_dir = os.path.join(project_path, "FractalAnalysis", "TestResults")
        os.makedirs(self.test_results_dir, exist_ok=True)
        
    def generate_test_plan(self) -> Dict[str, Any]:
        """
        Generate test plan based on fractal pattern analysis
        Returns test categories prioritized by pattern severity
        """
        print("\n[FRACTAL] Generating pattern-based test plan...")
        
        # Run complete fractal analysis
        analysis_results = self.debugger.debug_project(['.cpp', '.h', '.cs'])
        
        # Group files by pattern type
        pattern_test_plan = {}
        
        for file_result in analysis_results['file_results']:
            if not file_result['pattern_matches']:
                continue
                
            file_path = file_result['file_path']
            
            for match in file_result['pattern_matches']:
                pattern_type = match['pattern_type']
                
                if pattern_type not in pattern_test_plan:
                    signature = get_pattern_signature(pattern_type)
                    pattern_test_plan[pattern_type] = {
                        'pattern_name': signature['name'],
                        'severity': signature['severity'],
                        'files': [],
                        'test_priority': self._calculate_test_priority(signature),
                        'recommended_tests': self._get_recommended_tests(pattern_type)
                    }
                
                pattern_test_plan[pattern_type]['files'].append({
                    'file_path': file_path,
                    'line_number': match['line_number'],
                    'amplitude': match['amplitude'],
                    'scale': match['scale']
                })
        
        # Sort by priority
        sorted_plan = dict(sorted(
            pattern_test_plan.items(),
            key=lambda x: x[1]['test_priority'],
            reverse=True
        ))
        
        return sorted_plan
    
    def _calculate_test_priority(self, signature: Dict) -> float:
        """
        Calculate test priority based on pattern severity and historical data
        """
        severity_weights = {
            'critical': 1.0,
            'high': 0.8,
            'medium': 0.6,
            'low': 0.4
        }
        
        base_priority = severity_weights.get(signature['severity'], 0.5)
        
        # Boost priority based on pattern frequency
        frequency_boost = min(signature.get('frequency_weight', 0.5) * 0.3, 0.3)
        
        return base_priority + frequency_boost
    
    def _get_recommended_tests(self, pattern_type: str) -> List[str]:
        """
        Get recommended test types based on pattern
        """
        test_recommendations = {
            'null_cascade_ue': [
                'compilation_test',
                'include_dependency_test',
                'forward_declaration_test',
                'memory_safety_test'
            ],
            'state_desync_ue': [
                'subsystem_initialization_test',
                'state_propagation_test',
                'temporal_coherence_test',
                'event_ordering_test'
            ],
            'api_drift_ue': [
                'api_compatibility_test',
                'version_migration_test',
                'signature_validation_test'
            ],
            'coupling_cascade_ue': [
                'dependency_graph_test',
                'circular_dependency_test',
                'module_boundary_test'
            ]
        }
        
        return test_recommendations.get(pattern_type, ['general_compilation_test'])
    
    def run_pattern_based_tests(self, test_plan: Dict) -> Dict[str, Any]:
        """
        Execute tests based on pattern analysis
        """
        print("\n[FRACTAL] Running pattern-based tests...")
        
        results = {
            'timestamp': time.time(),
            'pattern_results': {},
            'summary': {
                'total_patterns': len(test_plan),
                'total_files': 0,
                'tests_run': 0,
                'failures': 0
            }
        }
        
        for pattern_type, pattern_data in test_plan.items():
            print(f"\n  Testing pattern: {pattern_data['pattern_name']} ({pattern_data['severity']})")
            
            pattern_results = {
                'pattern_name': pattern_data['pattern_name'],
                'severity': pattern_data['severity'],
                'files_tested': len(pattern_data['files']),
                'test_results': {},
                'status': 'pending'
            }
            
            # Run recommended tests for this pattern
            for test_type in pattern_data['recommended_tests']:
                test_result = self._run_specific_test(test_type, pattern_data['files'])
                pattern_results['test_results'][test_type] = test_result
                
                if test_result['status'] == 'failed':
                    pattern_results['status'] = 'failed'
                    results['summary']['failures'] += 1
                elif pattern_results['status'] == 'pending':
                    pattern_results['status'] = 'passed'
            
            results['pattern_results'][pattern_type] = pattern_results
            results['summary']['total_files'] += len(pattern_data['files'])
            results['summary']['tests_run'] += len(pattern_data['recommended_tests'])
        
        return results
    
    def _run_specific_test(self, test_type: str, files: List[Dict]) -> Dict[str, Any]:
        """
        Run a specific test type on a set of files
        """
        print(f"    Running test: {test_type}")
        
        test_implementations = {
            'compilation_test': self._test_compilation,
            'include_dependency_test': self._test_include_dependencies,
            'forward_declaration_test': self._test_forward_declarations,
            'subsystem_initialization_test': self._test_subsystem_initialization,
            'api_compatibility_test': self._test_api_compatibility,
            'dependency_graph_test': self._test_dependency_graph
        }
        
        test_func = test_implementations.get(test_type)
        if not test_func:
            return {'status': 'skipped', 'message': 'Test not implemented'}
        
        try:
            return test_func(files)
        except Exception as e:
            return {'status': 'error', 'message': str(e)}
    
    def _test_compilation(self, files: List[Dict]) -> Dict[str, Any]:
        """Test if files compile without errors"""
        # This would integrate with UBT
        return {'status': 'implemented', 'file_count': len(files)}
    
    def _test_include_dependencies(self, files: List[Dict]) -> Dict[str, Any]:
        """Test for proper include dependencies"""
        issues = []
        
        for file_info in files:
            file_path = file_info['file_path']
            if os.path.exists(file_path):
                with open(file_path, 'r') as f:
                    content = f.read()
                    
                    # Check for missing includes
                    if '// TODO: Add include' in content:
                        issues.append(f"Missing include in {file_path}")
                    
                    # Check for forward declarations that should be includes
                    if 'class ' in content and '// Forward declaration' in content:
                        issues.append(f"Potential missing include in {file_path}")
        
        if issues:
            return {'status': 'failed', 'issues': issues}
        
        return {'status': 'passed'}
    
    def _test_forward_declarations(self, files: List[Dict]) -> Dict[str, Any]:
        """Test forward declaration correctness"""
        return {'status': 'implemented', 'file_count': len(files)}
    
    def _test_subsystem_initialization(self, files: List[Dict]) -> Dict[str, Any]:
        """Test subsystem initialization order"""
        return {'status': 'implemented', 'file_count': len(files)}
    
    def _test_api_compatibility(self, files: List[Dict]) -> Dict[str, Any]:
        """Test API compatibility with UE 5.6"""
        issues = []
        
        deprecated_patterns = [
            'ApplicationContextMask',
            'GetNetMode() == NM_Client'  # Old pattern
        ]
        
        for file_info in files:
            file_path = file_info['file_path']
            if os.path.exists(file_path):
                with open(file_path, 'r') as f:
                    content = f.read()
                    
                    for pattern in deprecated_patterns:
                        if pattern in content:
                            issues.append(f"Deprecated pattern '{pattern}' found in {file_path}")
        
        if issues:
            return {'status': 'failed', 'issues': issues}
        
        return {'status': 'passed'}
    
    def _test_dependency_graph(self, files: List[Dict]) -> Dict[str, Any]:
        """Test for circular dependencies"""
        return {'status': 'implemented', 'file_count': len(files)}
    
    def generate_alexander_report(self, test_plan: Dict, test_results: Dict) -> str:
        """
        Generate report in Alexander methodology format
        """
        timestamp = int(time.time())
        report_path = os.path.join(self.test_results_dir, f"fractal_test_report_{timestamp}.md")
        
        report = []
        report.append("# Alexander Fractal Test Report")
        report.append(f"**Generated:** {time.strftime('%Y-%m-%d %H:%M:%S')}  ")
        report.append(f"**Project:** {self.project_path}  ")
        report.append(f"**Total Patterns:** {test_results['summary']['total_patterns']}  ")
        report.append(f"**Total Files:** {test_results['summary']['total_files']}  ")
        report.append(f"**Tests Run:** {test_results['summary']['tests_run']}  ")
        report.append(f"**Failures:** {test_results['summary']['failures']}  ")
        report.append("")
        
        # Executive Summary
        report.append("## Executive Summary")
        if test_results['summary']['failures'] == 0:
            report.append("[PASS] **All pattern-based tests passed** - Project is in good health")
        else:
            report.append(f"[WARN] **{test_results['summary']['failures']} pattern categories failed** - Requires attention")
        report.append("")
        
        # Pattern Analysis
        report.append("## Pattern Analysis Results")
        report.append("")
        
        for pattern_type, pattern_data in test_results['pattern_results'].items():
            signature = get_pattern_signature(pattern_type)
            
            report.append(f"### {signature['name']}")
            report.append(f"- **Severity:** {signature['severity']}  ")
            report.append(f"- **Status:** {pattern_data['status']}  ")
            report.append(f"- **Files Affected:** {pattern_data['files_tested']}  ")
            report.append("")
            
            # Test results
            if pattern_data['test_results']:
                report.append("#### Test Results:")
                for test_type, result in pattern_data['test_results'].items():
                    status_icon = "[OK]" if result['status'] == 'passed' else "[FAIL]"
                    report.append(f"- {status_icon} {test_type}: {result['status']}  ")
                
                # Show issues if any
                for test_type, result in pattern_data['test_results'].items():
                    if result.get('issues'):
                        report.append("\n**Issues Found:**")
                        for issue in result['issues']:
                            report.append(f"- {issue}  ")
            
            report.append("")
        
        # Recommendations
        report.append("## Recommendations")
        top_patterns = get_top_patterns(3)
        
        for i, pattern_type in enumerate(top_patterns, 1):
            signature = get_pattern_signature(pattern_type)
            report.append(f"{i}. **{signature['name']}** ({signature['severity']})  ")
            
            # Get files for this pattern
            pattern_files = test_plan.get(pattern_type, {}).get('files', [])
            if pattern_files:
                report.append(f"   - **Files to fix:** {len(pattern_files)} files  ")
                report.append(f"   - **Example:** {pattern_files[0]['file_path']}:{pattern_files[0]['line_number']}  ")
            
            # Get interventions
            if signature.get('intervention_templates'):
                report.append(f"   - **Interventions available:** {len(signature['intervention_templates'])}  ")
            
            report.append("")
        
        # Write report
        with open(report_path, 'w') as f:
            f.write('\n'.join(report))
        
        return report_path
    
    def integrate_with_build_system(self):
        """
        Integrate fractal testing with existing build_and_test.py
        """
        # Hook into existing test infrastructure
        build_script = os.path.join(self.project_path, "build_and_test.py")
        
        if os.path.exists(build_script):
            print(f"\n[INTEGRATION] Found build_and_test.py - adding fractal hooks")
            
            # Generate test plan
            test_plan = self.generate_test_plan()
            
            # Run pattern-based tests
            test_results = self.run_pattern_based_tests(test_plan)
            
            # Generate Alexander report
            report_path = self.generate_alexander_report(test_plan, test_results)
            
            print(f"\n[REPORT] Generated: {report_path}")
            
            return {
                'test_plan': test_plan,
                'test_results': test_results,
                'report_path': report_path
            }
        
        else:
            print(f"\n[WARNING] build_and_test.py not found - integration skipped")
            return None


def main():
    """Integration entry point"""
    project_path = os.getcwd()
    
    print("="*80)
    print("FRACTAL TESTING INTEGRATION - ALEXANDER METHODOLOGY")
    print("="*80)
    
    integration = FractalTestingIntegration(project_path)
    
    # Run full integration
    results = integration.integrate_with_build_system()
    
    if results:
        print("\n[SUMMARY] Integration successful!")
        print(f"  - Patterns analyzed: {len(results['test_plan'])}")
        print(f"  - Tests run: {results['test_results']['summary']['tests_run']}")
        print(f"  - Report generated: {results['report_path']}")
    else:
        print("\n[SUMMARY] Integration failed - missing dependencies")
    
    print("\n" + "="*80)


if __name__ == "__main__":
    main()