"""
Intervention Executor for the Fractal Debugging Framework.
Applies targeted interventions and measures cascade effects.
"""

import os
import re
import json
import time
import shutil
from typing import Dict, List, Tuple, Optional, Any
from dataclasses import dataclass, asdict

from pattern_signatures_unreal import update_pattern_history
from fractal_debugger import FractalDebugger, AmplitudeSpectrum


@dataclass
class InterventionResult:
    """Result of an intervention application"""
    success: bool
    error_message: str
    files_modified: List[str]
    cascade_measurement: Optional['CascadeMeasurement']
    execution_time: float
    
    def to_dict(self):
        return {
            'success': self.success,
            'error_message': self.error_message,
            'files_modified': self.files_modified,
            'cascade_measurement': self.cascade_measurement.to_dict() if self.cascade_measurement else None,
            'execution_time': self.execution_time
        }


@dataclass
class CascadeMeasurement:
    """Measurements of cascade effect from intervention"""
    baseline_spectrum: AmplitudeSpectrum
    new_spectrum: AmplitudeSpectrum
    file_impact: float
    build_impact: float
    runtime_impact: float
    overall_score: float
    affected_files: List[str]
    
    def to_dict(self):
        return {
            'baseline_spectrum': self.baseline_spectrum.to_dict(),
            'new_spectrum': self.new_spectrum.to_dict(),
            'file_impact': self.file_impact,
            'build_impact': self.build_impact,
            'runtime_impact': self.runtime_impact,
            'overall_score': self.overall_score,
            'affected_files': self.affected_files
        }


class InterventionExecutor:
    """
    Executes interventions and measures their cascade effects.
    """
    
    def __init__(self, base_path: str, backup_dir: str = None):
        self.base_path = base_path
        self.backup_dir = backup_dir or os.path.join(base_path, '.fractal_backups')
        self.debugger = FractalDebugger(base_path)
        self.ensure_backup_dir()
    
    def ensure_backup_dir(self):
        """Ensure backup directory exists"""
        if not os.path.exists(self.backup_dir):
            os.makedirs(self.backup_dir)
    
    def backup_file(self, file_path: str) -> str:
        """
        Create backup of file before intervention.
        
        Args:
            file_path: Path to file to backup
            
        Returns:
            str: Path to backup file
        """
        if not os.path.exists(file_path):
            return ""
        
        # Create timestamped backup
        timestamp = int(time.time())
        rel_path = os.path.relpath(file_path, self.base_path)
        backup_name = f"{rel_path.replace(os.sep, '_')}_{timestamp}.bak"
        backup_path = os.path.join(self.backup_dir, backup_name)
        
        # Ensure backup subdirectory exists
        backup_subdir = os.path.dirname(backup_path)
        if not os.path.exists(backup_subdir):
            os.makedirs(backup_subdir)
        
        # Copy file
        shutil.copy2(file_path, backup_path)
        return backup_path
    
    def apply_intervention(self, intervention: Dict) -> InterventionResult:
        """
        Apply a single intervention to a file.
        
        Args:
            intervention: Intervention dictionary
            
        Returns:
            InterventionResult: Result of intervention
        """
        start_time = time.time()
        
        file_path = intervention.get('file_path')
        if not file_path or not os.path.exists(file_path):
            return InterventionResult(
                success=False,
                error_message=f"File not found: {file_path}",
                files_modified=[],
                cascade_measurement=None,
                execution_time=0.0
            )
        
        # Backup original file
        backup_path = self.backup_file(file_path)
        
        try:
            # Read file content
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # Get original line
            line_number = intervention.get('line_number', 1)
            lines = content.split('\n')
            
            if line_number > len(lines):
                return InterventionResult(
                    success=False,
                    error_message=f"Line number {line_number} exceeds file length",
                    files_modified=[],
                    cascade_measurement=None,
                    execution_time=time.time() - start_time
                )
            
            # Apply pattern replacement
            original_line = lines[line_number - 1]
            pattern = intervention.get('pattern', '')
            replacement = intervention.get('replacement_code', '')
            
            if not pattern:
                # Use the original line as pattern
                pattern = re.escape(original_line.strip())
            
            # Apply replacement
            new_line = re.sub(pattern, replacement, original_line)
            
            if new_line == original_line:
                return InterventionResult(
                    success=False,
                    error_message="No changes made - pattern didn't match",
                    files_modified=[],
                    cascade_measurement=None,
                    execution_time=time.time() - start_time
                )
            
            # Update line
            lines[line_number - 1] = new_line
            
            # Write back
            new_content = '\n'.join(lines)
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(new_content)
            
            # Measure cascade effect
            cascade_measurement = self.measure_cascade_effect(
                intervention, backup_path, file_path
            )
            
            execution_time = time.time() - start_time
            
            return InterventionResult(
                success=True,
                error_message="",
                files_modified=[file_path],
                cascade_measurement=cascade_measurement,
                execution_time=execution_time
            )
            
        except Exception as e:
            # Restore from backup on error
            if backup_path and os.path.exists(backup_path):
                shutil.copy2(backup_path, file_path)
            
            return InterventionResult(
                success=False,
                error_message=str(e),
                files_modified=[],
                cascade_measurement=None,
                execution_time=time.time() - start_time
            )
    
    def apply_interventions(self, interventions: List[Dict], 
                           batch_mode: bool = False) -> List[InterventionResult]:
        """
        Apply multiple interventions.
        
        Args:
            interventions: List of intervention dictionaries
            batch_mode: Whether to apply in batch (all or nothing)
            
        Returns:
            List[InterventionResult]: Results for each intervention
        """
        results = []
        applied_interventions = []
        
        if batch_mode:
            # Batch mode: all or nothing
            batch_backup = f"batch_backup_{int(time.time())}"
            batch_backup_dir = os.path.join(self.backup_dir, batch_backup)
            os.makedirs(batch_backup_dir)
            
            try:
                # Backup all affected files
                affected_files = set()
                for intervention in interventions:
                    file_path = intervention.get('file_path')
                    if file_path:
                        affected_files.add(file_path)
                
                for file_path in affected_files:
                    if os.path.exists(file_path):
                        backup_name = os.path.basename(file_path) + ".bak"
                        shutil.copy2(file_path, os.path.join(batch_backup_dir, backup_name))
                
                # Apply all interventions
                for intervention in interventions:
                    result = self.apply_intervention(intervention)
                    results.append(result)
                    
                    if result.success:
                        applied_interventions.append(intervention)
                
                # If any failed in batch mode, rollback all
                if any(not r.success for r in results):
                    # Restore all backups
                    for file_path in affected_files:
                        backup_name = os.path.basename(file_path) + ".bak"
                        backup_path = os.path.join(batch_backup_dir, backup_name)
                        if os.path.exists(backup_path):
                            shutil.copy2(backup_path, file_path)
                    
                    # Mark all as failed
                    for i, result in enumerate(results):
                        if result.success:
                            results[i] = InterventionResult(
                                success=False,
                                error_message="Batch rollback due to other intervention failure",
                                files_modified=[],
                                cascade_measurement=None,
                                execution_time=result.execution_time
                            )
                    
                    applied_interventions = []
                
            finally:
                # Cleanup batch backup
                if os.path.exists(batch_backup_dir):
                    shutil.rmtree(batch_backup_dir)
        else:
            # Individual mode: apply each intervention separately
            for intervention in interventions:
                result = self.apply_intervention(intervention)
                results.append(result)
                
                if result.success:
                    applied_interventions.append(intervention)
        
        # Update pattern history for successful interventions
        for intervention in applied_interventions:
            pattern_type = intervention.get('pattern_type')
            cascade_score = 0.0
            
            if results and results[0].cascade_measurement:
                cascade_score = results[0].cascade_measurement.overall_score
            
            if pattern_type:
                update_pattern_history(
                    pattern_type=pattern_type,
                    success=True,
                    cascade_score=cascade_score,
                    affected_files=[intervention.get('file_path', '')]
                )
        
        return results
    
    def measure_cascade_effect(self, intervention: Dict, backup_path: str, 
                              modified_path: str) -> CascadeMeasurement:
        """
        Measure the cascade effect of an intervention.
        
        Args:
            intervention: The intervention that was applied
            backup_path: Path to backup of original file
            modified_path: Path to modified file
            
        Returns:
            CascadeMeasurement: Cascade effect measurements
        """
        # Analyze original file (from backup)
        original_results = self.debugger.debug(backup_path) if os.path.exists(backup_path) else {}
        
        # Analyze modified file
        new_results = self.debugger.debug(modified_path)
        
        # Calculate baseline spectrum
        baseline_spectrum = AmplitudeSpectrum()
        if original_results and 'resonant_analysis' in original_results:
            for analysis in original_results['resonant_analysis']:
                scale = analysis.get('resonant_scale', '')
                amplitude = analysis.get('amplitude', 0.0)
                
                if scale == 'micro':
                    baseline_spectrum.micro = amplitude
                elif scale == 'meso':
                    baseline_spectrum.meso = amplitude
                elif scale == 'macro':
                    baseline_spectrum.macro = amplitude
                elif scale == 'meta':
                    baseline_spectrum.meta = amplitude
        
        # Calculate new spectrum
        new_spectrum = AmplitudeSpectrum()
        if new_results and 'resonant_analysis' in new_results:
            for analysis in new_results['resonant_analysis']:
                scale = analysis.get('resonant_scale', '')
                amplitude = analysis.get('amplitude', 0.0)
                
                if scale == 'micro':
                    new_spectrum.micro = amplitude
                elif scale == 'meso':
                    new_spectrum.meso = amplitude
                elif scale == 'macro':
                    new_spectrum.macro = amplitude
                elif scale == 'meta':
                    new_spectrum.meta = amplitude
        
        # Calculate impact scores
        file_impact = self._calculate_file_impact(intervention, original_results, new_results)
        build_impact = self._calculate_build_impact(intervention, original_results, new_results)
        runtime_impact = self._calculate_runtime_impact(intervention, original_results, new_results)
        
        # Overall cascade score (weighted average)
        overall_score = (file_impact * 0.3 + build_impact * 0.3 + runtime_impact * 0.4)
        
        # Find affected files through dependency analysis
        affected_files = self._find_affected_files(intervention)
        
        return CascadeMeasurement(
            baseline_spectrum=baseline_spectrum,
            new_spectrum=new_spectrum,
            file_impact=file_impact,
            build_impact=build_impact,
            runtime_impact=runtime_impact,
            overall_score=overall_score,
            affected_files=affected_files
        )
    
    def _calculate_file_impact(self, intervention: Dict, original_results: Dict, 
                              new_results: Dict) -> float:
        """Calculate file-level impact (0.0 to 1.0)"""
        # Base impact from intervention template
        cascade_prediction = intervention.get('cascade_prediction', {})
        base_impact = cascade_prediction.get('file_impact', 0.5)
        
        # Adjust based on actual pattern reduction
        original_patterns = len(original_results.get('pattern_matches', []))
        new_patterns = len(new_results.get('pattern_matches', []))
        
        if original_patterns > 0:
            reduction = (original_patterns - new_patterns) / original_patterns
            base_impact = max(base_impact, reduction)
        
        return min(base_impact, 1.0)
    
    def _calculate_build_impact(self, intervention: Dict, original_results: Dict,
                               new_results: Dict) -> float:
        """Calculate build-level impact (0.0 to 1.0)"""
        cascade_prediction = intervention.get('cascade_prediction', {})
        base_impact = cascade_prediction.get('build_impact', 0.5)
        
        # Check if this is a common pattern across many files
        pattern_type = intervention.get('pattern_type', '')
        if pattern_type:
            # Estimate build impact based on pattern type frequency
            pattern_signatures = {
                'null_cascade_ue': 0.8,
                'api_drift_ue': 0.7,
                'state_desync_ue': 0.6,
                'coupling_cascade_ue': 0.5
            }
            base_impact = max(base_impact, pattern_signatures.get(pattern_type, 0.5))
        
        return min(base_impact, 1.0)
    
    def _calculate_runtime_impact(self, intervention: Dict, original_results: Dict,
                                 new_results: Dict) -> float:
        """Calculate runtime-level impact (0.0 to 1.0)"""
        cascade_prediction = intervention.get('cascade_prediction', {})
        base_impact = cascade_prediction.get('runtime_impact', 0.5)
        
        # Runtime impact is higher for pointer safety and initialization issues
        resonance_point = intervention.get('resonance_point', '')
        runtime_weights = {
            'pointer_safety': 0.9,
            'initialization_order': 0.8,
            'subsystem_order': 0.7,
            'replication_timing': 0.6,
            'include_management': 0.3
        }
        
        base_impact = max(base_impact, runtime_weights.get(resonance_point, 0.5))
        
        return min(base_impact, 1.0)
    
    def _find_affected_files(self, intervention: Dict) -> List[str]:
        """Find files affected by this intervention"""
        affected_files = []
        
        # Add the direct file
        file_path = intervention.get('file_path')
        if file_path:
            affected_files.append(file_path)
        
        # Find files with similar patterns
        pattern_type = intervention.get('pattern_type', '')
        if pattern_type:
            # This is a simplified version - in practice, you'd analyze dependencies
            # For now, return just the direct file
            pass
        
        return affected_files
    
    def restore_from_backup(self, backup_path: str, original_path: str) -> bool:
        """
        Restore a file from backup.
        
        Args:
            backup_path: Path to backup file
            original_path: Path to restore to
            
        Returns:
            bool: Success status
        """
        try:
            if os.path.exists(backup_path):
                shutil.copy2(backup_path, original_path)
                return True
            return False
        except Exception:
            return False
    
    def cleanup_old_backups(self, max_age_days: int = 7):
        """
        Clean up old backup files.
        
        Args:
            max_age_days: Maximum age of backups to keep (in days)
        """
        if not os.path.exists(self.backup_dir):
            return
        
        current_time = time.time()
        max_age_seconds = max_age_days * 24 * 60 * 60
        
        for filename in os.listdir(self.backup_dir):
            file_path = os.path.join(self.backup_dir, filename)
            if os.path.isfile(file_path):
                file_age = current_time - os.path.getmtime(file_path)
                if file_age > max_age_seconds:
                    try:
                        os.remove(file_path)
                    except Exception:
                        pass  # Ignore deletion errors


def execute_intervention_plan(interventions: List[Dict], base_path: str, 
                            batch_mode: bool = False) -> Dict:
    """
    Execute a complete intervention plan.
    
    Args:
        interventions: List of interventions to apply
        base_path: Base project path
        batch_mode: Whether to use batch mode
        
    Returns:
        Dict: Execution summary
    """
    executor = InterventionExecutor(base_path)
    
    print(f"\nExecuting {len(interventions)} interventions...")
    
    results = executor.apply_interventions(interventions, batch_mode=batch_mode)
    
    # Generate summary
    successful = sum(1 for r in results if r.success)
    failed = len(results) - successful
    
    total_cascade_score = 0.0
    total_execution_time = 0.0
    
    for result in results:
        total_execution_time += result.execution_time
        if result.cascade_measurement:
            total_cascade_score += result.cascade_measurement.overall_score
    
    avg_cascade_score = total_cascade_score / successful if successful > 0 else 0.0
    
    summary = {
        'total_interventions': len(interventions),
        'successful': successful,
        'failed': failed,
        'success_rate': successful / len(interventions) if interventions else 0.0,
        'average_cascade_score': avg_cascade_score,
        'total_execution_time': total_execution_time,
        'results': [result.to_dict() for result in results]
    }
    
    # Print summary
    print(f"\nIntervention Execution Summary:")
    print(f"  Total: {summary['total_interventions']}")
    print(f"  Successful: {summary['successful']}")
    print(f"  Failed: {summary['failed']}")
    print(f"  Success Rate: {summary['success_rate']:.2%}")
    print(f"  Avg Cascade Score: {summary['average_cascade_score']:.2f}")
    print(f"  Total Time: {summary['total_execution_time']:.2f}s")
    
    return summary


def save_intervention_results(results: Dict, output_path: str):
    """Save intervention results to file"""
    with open(output_path, 'w') as f:
        json.dump(results, f, indent=2, default=str)


def load_intervention_results(input_path: str) -> Dict:
    """Load intervention results from file"""
    with open(input_path, 'r') as f:
        return json.load(f)