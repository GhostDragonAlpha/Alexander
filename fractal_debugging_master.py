#!/usr/bin/env python3
"""
Fractal Debugging Framework - Master Integration Script
Orchestrates the complete Resonance Debugging Protocol workflow
"""

import os
import sys
import json
import time
import argparse
from typing import Dict, List, Optional

# Import framework components
from pattern_signatures_unreal import get_top_patterns, get_pattern_statistics, get_pattern_signature
from fractal_debugger import FractalDebugger, save_results, print_analysis_summary
from file_based_state_scanner import FileBasedStateScanner, save_state_report, print_state_report
from intervention_executor import InterventionExecutor, execute_intervention_plan, save_intervention_results
from fractal_build_analyzer import FractalBuildAnalyzer, print_build_analysis, execute_intervention_plan_from_analysis


class FractalDebuggingMaster:
    """
    Master orchestrator for the Fractal Debugging Framework.
    Coordinates all components and manages the complete workflow.
    """
    
    def __init__(self, project_path: str):
        self.project_path = project_path
        self.debugger = FractalDebugger(project_path)
        self.state_scanner = FileBasedStateScanner(project_path)
        self.intervention_executor = InterventionExecutor(project_path)
        self.build_analyzer = FractalBuildAnalyzer(project_path)
        
        # Ensure output directories exist
        self.output_dir = os.path.join(project_path, "FractalAnalysis")
        os.makedirs(self.output_dir, exist_ok=True)
        
        self.backup_dir = os.path.join(project_path, ".fractal_backups")
        os.makedirs(self.backup_dir, exist_ok=True)
    
    def run_complete_analysis(self, build_output: Optional[str] = None) -> Dict:
        """
        Run complete fractal analysis workflow.
        
        Args:
            build_output: Optional build output to analyze
            
        Returns:
            Dict: Complete analysis results
        """
        print("\n" + "="*80)
        print("FRACTAL DEBUGGING FRAMEWORK - COMPLETE ANALYSIS")
        print("="*80)
        
        results = {
            'timestamp': time.time(),
            'project_path': self.project_path,
            'workflow': {},
            'summary': {}
        }
        
        # Step 1: State Health Analysis
        print("\n[1/4] Analyzing state health...")
        state_report = self.state_scanner.generate_state_report()
        results['workflow']['state_health'] = state_report
        
        if state_report['summary']['status'] != 'healthy':
            print(f"  [!] State health: {state_report['summary']['status'].upper()}")
            print(f"  [i] Coherence score: {state_report['summary']['temporal_coherence_score']:.2f}")
        else:
            print("  [OK] State health: HEALTHY")
        
        # Step 2: Pattern Detection (Project-wide)
        print("\n[2/4] Scanning for bug patterns...")
        project_analysis = self.debugger.debug_project(['.cpp', '.h'])
        results['workflow']['pattern_detection'] = project_analysis
        
        pattern_count = project_analysis['project_summary']['total_pattern_matches']
        file_coverage = project_analysis['project_summary']['files_with_issues']
        
        print(f"  [SCAN] Detected {pattern_count} pattern matches across {file_coverage} files")
        
        # Step 3: Build Analysis (if build output provided)
        if build_output:
            print("\n[3/4] Analyzing build output...")
            build_analysis = self.build_analyzer.analyze_build_errors(build_output)
            results['workflow']['build_analysis'] = build_analysis.to_dict()
            
            print(f"  [BUILD] Generated {len(build_analysis.interventions)} interventions")
        else:
            print("\n[3/4] Skipping build analysis (no output provided)")
            results['workflow']['build_analysis'] = None
        
        # Step 4: Intervention Planning
        print("\n[4/4] Generating intervention plan...")
        
        all_interventions = []
        
        # Add interventions from pattern detection
        if 'top_patterns' in project_analysis:
            for pattern_type in project_analysis['top_patterns']:
                signature = get_pattern_signature(pattern_type)
                if signature:
                    # Create high-level intervention for pattern
                    intervention = {
                        'pattern_type': pattern_type,
                        'description': f"Address {signature['name']} pattern across project",
                        'scope': 'project_wide',
                        'priority': signature['severity'],
                        'estimated_effort': 'medium'
                    }
                    all_interventions.append(intervention)
        
        # Add interventions from build analysis
        if build_output:
            build_interventions = self.build_analyzer.generate_intervention_plan(build_analysis)
            all_interventions.extend(build_interventions)
        
        results['workflow']['intervention_plan'] = all_interventions
        
        print(f"  [PLAN] Generated {len(all_interventions)} total interventions")
        
        # Generate overall summary
        results['summary'] = self._generate_overall_summary(results)
        
        # Save results
        self._save_analysis_results(results)
        
        return results
    
    def _generate_overall_summary(self, results: Dict) -> Dict:
        """
        Generate overall analysis summary.
        
        Args:
            results: Complete analysis results
            
        Returns:
            Dict: Summary information
        """
        summary = {
            'status': 'healthy',
            'issues_found': 0,
            'recommended_action': 'No action required',
            'priority_patterns': [],
            'estimated_fix_time': '0 minutes'
        }
        
        # Check state health
        state_health = results['workflow']['state_health']['summary']
        if state_health['status'] != 'healthy':
            summary['status'] = 'degraded'
            summary['issues_found'] += state_health['total_issues']
        
        # Check pattern detection
        pattern_results = results['workflow']['pattern_detection']
        pattern_count = pattern_results['project_summary']['total_pattern_matches']
        
        if pattern_count > 0:
            summary['status'] = 'degraded' if pattern_count < 50 else 'critical'
            summary['issues_found'] += pattern_count
        
        # Check build analysis
        build_results = results['workflow']['build_analysis']
        if build_results:
            build_summary = build_results['summary']
            if 'failed' in build_summary['status']:
                summary['status'] = 'critical'
                summary['issues_found'] += build_results['total_errors']
        
        # Get top patterns
        summary['priority_patterns'] = get_top_patterns(3)
        
        # Estimate fix time
        if summary['issues_found'] < 10:
            summary['estimated_fix_time'] = '15-30 minutes'
        elif summary['issues_found'] < 50:
            summary['estimated_fix_time'] = '1-2 hours'
        elif summary['issues_found'] < 200:
            summary['estimated_fix_time'] = '4-8 hours'
        else:
            summary['estimated_fix_time'] = '1-3 days'
        
        # Generate recommendation
        if summary['status'] == 'critical':
            summary['recommended_action'] = 'Immediate intervention required - apply top priority fixes'
        elif summary['status'] == 'degraded':
            summary['recommended_action'] = 'Apply targeted interventions to improve stability'
        else:
            summary['recommended_action'] = 'Monitor and maintain current health'
        
        return summary
    
    def _save_analysis_results(self, results: Dict):
        """Save analysis results to files"""
        timestamp = int(time.time())
        
        # Save main results
        results_file = os.path.join(self.output_dir, f"analysis_{timestamp}.json")
        with open(results_file, 'w') as f:
            json.dump(results, f, indent=2, default=str)
        
        # Save intervention plan
        if 'intervention_plan' in results['workflow']:
            plan_file = os.path.join(self.output_dir, f"intervention_plan_{timestamp}.json")
            with open(plan_file, 'w') as f:
                json.dump(results['workflow']['intervention_plan'], f, indent=2)
        
        # Save state report
        if 'state_health' in results['workflow']:
            state_file = os.path.join(self.output_dir, f"state_report_{timestamp}.json")
            with open(state_file, 'w') as f:
                json.dump(results['workflow']['state_health'], f, indent=2)
        
        print(f"\n[SAVE] Results saved to: {self.output_dir}")
        print(f"   - Main analysis: analysis_{timestamp}.json")
        print(f"   - Intervention plan: intervention_plan_{timestamp}.json")
        print(f"   - State report: state_report_{timestamp}.json")
    
    def execute_interventions(self, interventions: List[Dict], 
                             batch_mode: bool = False) -> Dict:
        """
        Execute a list of interventions.
        
        Args:
            interventions: List of interventions to execute
            batch_mode: Whether to use batch mode (all or nothing)
            
        Returns:
            Dict: Execution results
        """
        print(f"\n[EXEC] Executing {len(interventions)} interventions...")
        
        # Filter for file-specific interventions
        file_interventions = [
            inv for inv in interventions 
            if 'file_path' in inv and 'line_number' in inv
        ]
        
        if not file_interventions:
            print("  [!] No file-specific interventions to apply")
            return {'executed': 0, 'results': []}
        
        execution_results = execute_intervention_plan(file_interventions, self.project_path, batch_mode)
        
        # Save execution results
        timestamp = int(time.time())
        results_file = os.path.join(self.output_dir, f"execution_results_{timestamp}.json")
        save_intervention_results(execution_results, results_file)
        
        print(f"\n[SAVE] Execution results saved to: execution_results_{timestamp}.json")
        
        return execution_results
    
    def run_continuous_monitoring(self, interval: int = 60):
        """
        Run continuous monitoring of project health.
        
        Args:
            interval: Check interval in seconds
        """
        print(f"\n[MONITOR] Starting continuous monitoring (interval: {interval}s)")
        print("Press Ctrl+C to stop\n")
        
        try:
            while True:
                # Quick health check
                state_report = self.state_scanner.generate_state_report()
                
                if state_report['summary']['status'] != 'healthy':
                    print(f"\n[ALERT] Health alert: {state_report['summary']['status'].upper()}")
                    print(f"   Issues: {state_report['summary']['total_issues']}")
                    
                    # Run full analysis on degradation
                    results = self.run_complete_analysis()
                    print(f"\n[ANALYSIS] Full analysis completed. Status: {results['summary']['status']}")
                
                time.sleep(interval)
        
        except KeyboardInterrupt:
            print("\n\n[STOP] Monitoring stopped by user")
    
    def print_report(self, results: Dict):
        """
        Print human-readable report of analysis results.
        
        Args:
            results: Analysis results to report
        """
        summary = results['summary']
        
        print("\n" + "="*80)
        print("FRACTAL DEBUGGING REPORT")
        print("="*80)
        
        print(f"\n[STATUS] Overall Status: {summary['status'].upper()}")
        print(f"[COUNT] Total Issues Found: {summary['issues_found']}")
        print(f"[TIME] Estimated Fix Time: {summary['estimated_fix_time']}")
        print(f"[ACTION] Recommended Action: {summary['recommended_action']}")
        
        if summary['priority_patterns']:
            print(f"\n[PRIORITY] Priority Patterns:")
            for i, pattern in enumerate(summary['priority_patterns'], 1):
                signature = get_pattern_signature(pattern)
                if signature:
                    print(f"   {i}. {signature['name']} ({signature['severity']})")
        
        # State health details
        state_health = results['workflow']['state_health']['summary']
        if state_health['status'] != 'healthy':
            print(f"\n[HEALTH] State Health: {state_health['status'].upper()}")
            print(f"   Coherence Score: {state_health['temporal_coherence_score']:.2f}")
            print(f"   Write Conflicts: {state_health['write_conflicts']}")
            print(f"   Desync Events: {state_health['state_desync_events']}")
        
        # Pattern detection details
        pattern_results = results['workflow']['pattern_detection']['project_summary']
        print(f"\n[SCAN] Pattern Detection:")
        print(f"   Files Analyzed: {pattern_results['total_files_analyzed']}")
        print(f"   Files with Issues: {pattern_results['files_with_issues']}")
        print(f"   Pattern Matches: {pattern_results['total_pattern_matches']}")
        
        # Build analysis details
        if results['workflow']['build_analysis']:
            build_results = results['workflow']['build_analysis']
            print(f"\n[BUILD] Build Analysis:")
            print(f"   Build Errors: {build_results['total_errors']}")
            print(f"   Error Categories: {len(build_results['error_categories'])}")
            print(f"   Interventions: {len(build_results['interventions'])}")
        
        # Intervention plan
        interventions = results['workflow']['intervention_plan']
        if interventions:
            print(f"\n[PLAN] Intervention Plan ({len(interventions)} items):")
            for i, intervention in enumerate(interventions[:5], 1):
                print(f"   {i}. {intervention['description']}")
                if 'confidence' in intervention:
                    print(f"      Confidence: {intervention['confidence']:.2f}")
        
        print("\n" + "="*80)


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description='Fractal Debugging Framework')
    parser.add_argument('project_path', help='Path to Unreal project')
    parser.add_argument('--build-output', help='Path to build output file')
    parser.add_argument('--mode', choices=['analyze', 'monitor', 'fix', 'full'], 
                       default='analyze', help='Operation mode')
    parser.add_argument('--interval', type=int, default=60, 
                       help='Monitoring interval in seconds')
    parser.add_argument('--auto-apply', action='store_true',
                       help='Auto-apply interventions')
    
    args = parser.parse_args()
    
    # Create master orchestrator
    master = FractalDebuggingMaster(args.project_path)
    
    # Load build output if provided
    build_output = None
    if args.build_output and os.path.exists(args.build_output):
        with open(args.build_output, 'r') as f:
            build_output = f.read()
    
    # Execute based on mode
    if args.mode == 'analyze':
        results = master.run_complete_analysis(build_output)
        master.print_report(results)
    
    elif args.mode == 'monitor':
        master.run_continuous_monitoring(args.interval)
    
    elif args.mode == 'fix':
        # Analyze first
        results = master.run_complete_analysis(build_output)
        
        # Execute interventions
        if results['workflow']['intervention_plan']:
            master.execute_interventions(
                results['workflow']['intervention_plan'], 
                batch_mode=True
            )
        else:
            print("No interventions to apply")
    
    elif args.mode == 'full':
        # Complete workflow: analyze -> intervene -> verify
        print("\n[PHASE 1] Analysis")
        results = master.run_complete_analysis(build_output)
        master.print_report(results)
        
        if results['workflow']['intervention_plan']:
            print("\n[PHASE 2] Intervention")
            execution_results = master.execute_interventions(
                results['workflow']['intervention_plan'], 
                batch_mode=True
            )
            
            if execution_results['successful'] > 0:
                print("\n[PHASE 3] Verification")
                print("   Running build verification...")
                
                # Re-run analysis to verify fixes
                verify_results = master.run_complete_analysis()
                master.print_report(verify_results)
                
                if verify_results['summary']['status'] == 'healthy':
                    print("\n[SUCCESS] Project health restored!")
                else:
                    print("\n[PARTIAL] Some issues remain - manual review needed")
            else:
                print("\n[FAILED] No interventions were successful")
        else:
            print("\n[OK] No interventions needed - project is healthy")


if __name__ == "__main__":
    main()