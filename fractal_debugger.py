"""
Core Fractal Debugging Framework implementation.
Provides the Resonance Debugging Protocol engine for pattern detection,
scale analysis, and intervention generation.
"""

import os
import re
import json
import time
from typing import Dict, List, Tuple, Optional, Any
from dataclasses import dataclass, asdict
from enum import Enum

from pattern_signatures_unreal import (
    SIGNATURES, 
    PATTERN_REGEX, 
    get_pattern_signature,
    get_all_signatures,
    update_pattern_history,
    predict_pattern_priority,
    get_top_patterns
)


class Scale(Enum):
    """Resonant scale classifications"""
    MICRO = "micro"    # Variable/line level
    MESO = "meso"      # Function/class level
    MACRO = "macro"    # Component/subsystem level
    META = "meta"      # System/architecture level


@dataclass
class PatternMatch:
    """Represents a detected pattern match"""
    pattern_type: str
    file_path: str
    line_number: int
    line_content: str
    matched_text: str
    scale: str
    amplitude: float
    context: List[str]
    
    def to_dict(self):
        return asdict(self)


@dataclass
class AmplitudeSpectrum:
    """Amplitude measurements across all scales"""
    micro: float = 0.0
    meso: float = 0.0
    macro: float = 0.0
    meta: float = 0.0
    
    def to_dict(self):
        return asdict(self)
    
    def get_max_scale(self) -> Tuple[str, float]:
        """Get the scale with maximum amplitude"""
        scales = {
            'micro': self.micro,
            'meso': self.meso,
            'macro': self.macro,
            'meta': self.meta
        }
        max_scale = max(scales.items(), key=lambda x: x[1])
        return max_scale


@dataclass
class HarmonicAnalysis:
    """Harmonic structure analysis results"""
    correlations: Dict[str, float]
    is_systemic: bool
    coherence_score: float
    
    def to_dict(self):
        return {
            'correlations': self.correlations,
            'is_systemic': self.is_systemic,
            'coherence_score': self.coherence_score
        }


@dataclass
class Intervention:
    """Represents a generated intervention"""
    pattern_type: str
    resonance_point: str
    description: str
    file_path: str
    line_number: int
    original_code: str
    replacement_code: str
    cascade_prediction: Dict[str, float]
    confidence: float
    prerequisites: List[str]
    
    def to_dict(self):
        return asdict(self)


@dataclass
class CascadeMeasurement:
    """Measurements of cascade effect"""
    baseline_spectrum: AmplitudeSpectrum
    new_spectrum: AmplitudeSpectrum
    file_impact: float
    build_impact: float
    runtime_impact: float
    overall_score: float
    
    def to_dict(self):
        return {
            'baseline_spectrum': self.baseline_spectrum.to_dict(),
            'new_spectrum': self.new_spectrum.to_dict(),
            'file_impact': self.file_impact,
            'build_impact': self.build_impact,
            'runtime_impact': self.runtime_impact,
            'overall_score': self.overall_score
        }


class FractalInspector:
    """
    Inspects code for pattern signatures at different scales.
    """
    
    def __init__(self, base_path: str):
        self.base_path = base_path
        self.supported_extensions = ['.cpp', '.h', '.cs']
    
    def observe_pattern_at_scale(self, signature: Dict, file_path: str, 
                                 scale: Scale) -> float:
        """
        Observe a pattern signature at a specific scale in a file.
        
        Args:
            signature: Pattern signature dictionary
            file_path: Path to file to inspect
            scale: Scale to observe at
            
        Returns:
            float: Amplitude (0.0 to 1.0) of pattern manifestation
        """
        if not os.path.exists(file_path):
            return 0.0
        
        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                lines = content.split('\n')
        except Exception:
            return 0.0
        
        pattern_type = signature['name']
        if pattern_type not in PATTERN_REGEX:
            return 0.0
        
        scale_patterns = PATTERN_REGEX[pattern_type]
        matches = []
        
        # Check for pattern matches based on scale
        for pattern_name, regex in scale_patterns.items():
            try:
                for i, line in enumerate(lines):
                    if re.search(regex, line, re.IGNORECASE):
                        # Calculate context amplitude based on scale
                        context_amplitude = self._calculate_context_amplitude(
                            lines, i, scale
                        )
                        matches.append(context_amplitude)
            except re.error:
                continue
        
        if not matches:
            return 0.0
        
        # Return average amplitude
        return sum(matches) / len(matches)
    
    def _calculate_context_amplitude(self, lines: List[str], line_idx: int, 
                                     scale: Scale) -> float:
        """
        Calculate amplitude based on context around the match.
        
        Args:
            lines: All lines in the file
            line_idx: Index of the matching line
            scale: Scale to calculate for
            
        Returns:
            float: Context amplitude
        """
        # Base amplitude for finding a match
        base_amplitude = 0.5
        
        # Adjust based on scale-specific context
        if scale == Scale.MICRO:
            # Look for variable-level patterns
            if line_idx > 0 and self._is_variable_line(lines[line_idx - 1]):
                base_amplitude += 0.3
            if line_idx < len(lines) - 1 and self._is_variable_line(lines[line_idx + 1]):
                base_amplitude += 0.2
        
        elif scale == Scale.MESO:
            # Look for function-level patterns
            if self._is_in_function_scope(lines, line_idx):
                base_amplitude += 0.4
        
        elif scale == Scale.MACRO:
            # Look for class/component-level patterns
            if self._is_in_class_scope(lines, line_idx):
                base_amplitude += 0.5
        
        elif scale == Scale.META:
            # Look for system-level patterns
            if self._is_system_level_pattern(lines, line_idx):
                base_amplitude += 0.6
        
        return min(base_amplitude, 1.0)
    
    def _is_variable_line(self, line: str) -> bool:
        """Check if line contains variable declarations"""
        patterns = [
            r'\w+\s+\w+\s*[=;]',
            r'\w+\*\s+\w+\s*[=;]',
            r'T\w+Ptr<\w+>\s+\w+\s*[=;]'
        ]
        return any(re.search(p, line) for p in patterns)
    
    def _is_in_function_scope(self, lines: List[str], line_idx: int) -> bool:
        """Check if line is within function scope"""
        # Look backwards for function definition
        for i in range(line_idx - 1, max(-1, line_idx - 50), -1):
            if re.search(r'\w+\s+\w+\s*\([^)]*\)\s*[{;]', lines[i]):
                return True
        return False
    
    def _is_in_class_scope(self, lines: List[str], line_idx: int) -> bool:
        """Check if line is within class scope"""
        # Look backwards for class/struct definition
        for i in range(line_idx - 1, max(-1, line_idx - 100), -1):
            if re.search(r'(class|struct)\s+\w+\s*[:{]', lines[i]):
                return True
        return False
    
    def _is_system_level_pattern(self, lines: List[str], line_idx: int) -> bool:
        """Check if line shows system-level patterns"""
        # Look for subsystem, manager, or system-level constructs
        line = lines[line_idx].lower()
        system_keywords = ['subsystem', 'manager', 'system', 'initialize', 'deinitialize']
        return any(keyword in line for keyword in system_keywords)
    
    def scan_all_scales(self, signature: Dict, file_path: str) -> AmplitudeSpectrum:
        """
        Scan for pattern signature across all scales simultaneously.
        
        Args:
            signature: Pattern signature dictionary
            file_path: Path to file to scan
            
        Returns:
            AmplitudeSpectrum: Amplitude measurements across all scales
        """
        spectrum = AmplitudeSpectrum()
        
        spectrum.micro = self.observe_pattern_at_scale(signature, file_path, Scale.MICRO)
        spectrum.meso = self.observe_pattern_at_scale(signature, file_path, Scale.MESO)
        spectrum.macro = self.observe_pattern_at_scale(signature, file_path, Scale.MACRO)
        spectrum.meta = self.observe_pattern_at_scale(signature, file_path, Scale.META)
        
        return spectrum
    
    def scan_file_for_patterns(self, file_path: str) -> List[PatternMatch]:
        """
        Scan a file for all pattern signatures.
        
        Args:
            file_path: Path to file to scan
            
        Returns:
            List[PatternMatch]: List of detected pattern matches
        """
        matches = []
        
        if not os.path.exists(file_path):
            return matches
        
        # Check file extension
        ext = os.path.splitext(file_path)[1].lower()
        if ext not in self.supported_extensions:
            return matches
        
        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                lines = f.readlines()
        except Exception:
            return matches
        
        # Check each pattern type
        for pattern_type, signature in get_all_signatures().items():
            if pattern_type not in PATTERN_REGEX:
                continue
            
            pattern_regexes = PATTERN_REGEX[pattern_type]
            
            for line_idx, line in enumerate(lines):
                for pattern_name, regex in pattern_regexes.items():
                    try:
                        match = re.search(regex, line, re.IGNORECASE)
                        if match:
                            # Determine scale based on pattern type and context
                            scale = self._determine_scale(line, line_idx, lines, pattern_type)
                            
                            # Calculate amplitude
                            amplitude = self._calculate_match_amplitude(
                                match, line, scale, pattern_type
                            )
                            
                            # Get context lines
                            context = self._get_context(lines, line_idx)
                            
                            pattern_match = PatternMatch(
                                pattern_type=pattern_type,
                                file_path=file_path,
                                line_number=line_idx + 1,
                                line_content=line.strip(),
                                matched_text=match.group(0),
                                scale=scale.value,
                                amplitude=amplitude,
                                context=context
                            )
                            matches.append(pattern_match)
                    except re.error:
                        continue
        
        return matches
    
    def _determine_scale(self, line: str, line_idx: int, lines: List[str], 
                        pattern_type: str) -> Scale:
        """Determine the scale of a pattern match"""
        # Check for system-level patterns
        if any(keyword in line.lower() for keyword in ['subsystem', 'manager', 'system']):
            return Scale.META
        
        # Check for class-level patterns
        if 'class' in line or 'UCLASS' in line:
            return Scale.MACRO
        
        # Check for function-level patterns
        if 'void ' in line or 'virtual ' in line or 'UFUNCTION' in line:
            return Scale.MESO
        
        # Check context for function scope
        if self._is_in_function_scope(lines, line_idx):
            return Scale.MESO
        
        # Check context for class scope
        if self._is_in_class_scope(lines, line_idx):
            return Scale.MACRO
        
        # Default to micro for variable-level patterns
        return Scale.MICRO
    
    def _calculate_match_amplitude(self, match, line: str, scale: Scale, 
                                  pattern_type: str) -> float:
        """Calculate amplitude for a specific match"""
        base_amplitude = 0.6
        
        # Adjust based on pattern severity
        signature = get_pattern_signature(pattern_type)
        if signature:
            severity_weights = {'critical': 0.3, 'high': 0.2, 'medium': 0.1, 'low': 0.0}
            base_amplitude += severity_weights.get(signature['severity'], 0.0)
        
        # Adjust based on match quality
        if len(match.group(0)) > 20:  # Longer matches are more specific
            base_amplitude += 0.1
        
        return min(base_amplitude, 1.0)
    
    def _get_context(self, lines: List[str], line_idx: int, context_size: int = 3) -> List[str]:
        """Get context lines around a match"""
        start = max(0, line_idx - context_size)
        end = min(len(lines), line_idx + context_size + 1)
        return [line.strip() for line in lines[start:end]]


class ResonanceDetector:
    """
    Analyzes harmonic structure and finds resonant scales.
    """
    
    def find_resonant_scale(self, spectrum: AmplitudeSpectrum) -> Tuple[str, float]:
        """
        Find the scale where pattern has maximum amplitude.
        
        Args:
            spectrum: Amplitude spectrum across scales
            
        Returns:
            Tuple[str, float]: Resonant scale and its amplitude
        """
        return spectrum.get_max_scale()
    
    def analyze_harmonic_structure(self, spectrum: AmplitudeSpectrum) -> HarmonicAnalysis:
        """
        Analyze relationships between scales to determine if pattern is systemic.
        
        Args:
            spectrum: Amplitude spectrum across scales
            
        Returns:
            HarmonicAnalysis: Analysis of harmonic structure
        """
        # Calculate correlations between scales
        scales = {
            'micro': spectrum.micro,
            'meso': spectrum.meso,
            'macro': spectrum.macro,
            'meta': spectrum.meta
        }
        
        correlations = {}
        scale_names = list(scales.keys())
        
        # Calculate pairwise correlations
        for i, scale1 in enumerate(scale_names):
            for scale2 in scale_names[i + 1:]:
                # Simple correlation based on amplitude similarity
                amp1 = scales[scale1]
                amp2 = scales[scale2]
                
                # Correlation is high if both scales show significant amplitude
                if amp1 > 0.3 and amp2 > 0.3:
                    correlation = 1.0 - abs(amp1 - amp2)
                else:
                    correlation = min(amp1, amp2)
                
                correlations[f"{scale1}-{scale2}"] = correlation
        
        # Determine if pattern is systemic
        # Systemic if multiple scales show significant amplitude
        significant_scales = sum(1 for amp in scales.values() if amp > 0.3)
        is_systemic = significant_scales >= 2
        
        # Calculate coherence score (average of top correlations)
        if correlations:
            top_correlations = sorted(correlations.values(), reverse=True)[:2]
            coherence_score = sum(top_correlations) / len(top_correlations)
        else:
            coherence_score = 0.0
        
        return HarmonicAnalysis(
            correlations=correlations,
            is_systemic=is_systemic,
            coherence_score=coherence_score
        )


class InterventionProtocol:
    """
    Generates and applies interventions at resonance points.
    """
    
    def __init__(self, pattern_type: str, resonant_scale: Scale):
        self.pattern_type = pattern_type
        self.resonant_scale = resonant_scale
        self.signature = get_pattern_signature(pattern_type)
    
    def generate_interventions(self, matches: List[PatternMatch]) -> List[Intervention]:
        """
        Generate targeted interventions for detected pattern matches.
        
        Args:
            matches: List of pattern matches
            
        Returns:
            List[Intervention]: Generated interventions
        """
        if not self.signature:
            return []
        
        interventions = []
        templates = self.signature.get('intervention_templates', {})
        
        for match in matches:
            for resonance_point, template in templates.items():
                # Check if this intervention applies to the match scale
                if not self._is_scale_compatible(match.scale, resonance_point):
                    continue
                
                intervention = self._create_intervention(match, resonance_point, template)
                if intervention:
                    interventions.append(intervention)
        
        return interventions
    
    def _is_scale_compatible(self, match_scale: str, resonance_point: str) -> bool:
        """Check if intervention is compatible with match scale"""
        # Map resonance points to applicable scales
        scale_compatibility = {
            'pointer_safety': ['micro', 'meso'],
            'include_management': ['micro', 'macro', 'meta'],
            'initialization_order': ['meso', 'macro'],
            'subsystem_order': ['macro', 'meta'],
            'replication_timing': ['meso', 'macro'],
            'event_propagation': ['meso', 'macro'],
            'interface_abstraction': ['macro', 'meta'],
            'dependency_injection': ['meso', 'macro'],
            'signature_updates': ['micro', 'meso'],
            'macro_replacement': ['micro', 'meso'],
            'enum_standardization': ['meso', 'macro']
        }
        
        compatible_scales = scale_compatibility.get(resonance_point, [])
        return match_scale in compatible_scales
    
    def _create_intervention(self, match: PatternMatch, resonance_point: str, 
                            template: Dict) -> Optional[Intervention]:
        """Create a specific intervention from a template"""
        try:
            # Check if we can apply the pattern replacement
            if 'pattern' not in template or 'replacement' not in template:
                return None
            
            # Create regex pattern
            pattern = re.compile(template['pattern'])
            
            # Test if the pattern matches the line
            if not pattern.search(match.line_content):
                return None
            
            # Generate replacement
            replacement = template['replacement']
            
            # Calculate confidence based on match quality
            confidence = match.amplitude * 0.8  # Base confidence on match amplitude
            
            # Adjust confidence based on historical success
            stats = get_pattern_statistics()
            if self.pattern_type in stats:
                confidence *= stats[self.pattern_type]['fix_success_rate']
            
            return Intervention(
                pattern_type=self.pattern_type,
                resonance_point=resonance_point,
                description=template['description'],
                file_path=match.file_path,
                line_number=match.line_number,
                original_code=match.line_content,
                replacement_code=replacement,
                cascade_prediction=template.get('cascade_prediction', {
                    'file_impact': 0.5,
                    'build_impact': 0.5,
                    'runtime_impact': 0.5
                }),
                confidence=min(confidence, 1.0),
                prerequisites=self._get_prerequisites(resonance_point)
            )
        except Exception:
            return None
    
    def _get_prerequisites(self, resonance_point: str) -> List[str]:
        """Get prerequisites for an intervention"""
        prereq_map = {
            'pointer_safety': ['Verify TWeakObjectPtr type', 'Check object lifecycle'],
            'include_management': ['Check for circular dependencies', 'Verify forward declarations'],
            'initialization_order': ['Analyze constructor dependencies', 'Check initialization order'],
            'subsystem_order': ['Map subsystem dependencies', 'Verify initialization sequence'],
            'replication_timing': ['Check network authority', 'Verify replication settings'],
            'event_propagation': ['Verify delegate bindings', 'Check event order'],
            'interface_abstraction': ['Design interface contract', 'Plan migration path'],
            'dependency_injection': ['Identify injection points', 'Verify lifetime management'],
            'signature_updates': ['Check all call sites', 'Verify parameter types'],
            'macro_replacement': ['Test macro expansion', 'Verify compatibility'],
            'enum_standardization': ['Update all references', 'Check blueprint usage']
        }
        
        return prereq_map.get(resonance_point, [])


class FractalDebugger:
    """
    Main Fractal Debugging Framework engine.
    """
    
    def __init__(self, base_path: str):
        self.base_path = base_path
        self.inspector = FractalInspector(base_path)
        self.detector = ResonanceDetector()
    
    def debug(self, file_path: str) -> Dict:
        """
        Run complete fractal debug analysis on a file.
        
        Args:
            file_path: Path to file to analyze
            
        Returns:
            Dict: Complete analysis results
        """
        results = {
            'file_path': file_path,
            'timestamp': time.time(),
            'pattern_matches': [],
            'resonant_analysis': [],
            'interventions': [],
            'summary': {}
        }
        
        # Scan for all pattern matches
        matches = self.inspector.scan_file_for_patterns(file_path)
        results['pattern_matches'] = [match.to_dict() for match in matches]
        
        if not matches:
            results['summary'] = {
                'status': 'clean',
                'message': 'No patterns detected'
            }
            return results
        
        # Group matches by pattern type
        matches_by_type = {}
        for match in matches:
            if match.pattern_type not in matches_by_type:
                matches_by_type[match.pattern_type] = []
            matches_by_type[match.pattern_type].append(match)
        
        # Analyze each pattern type
        for pattern_type, pattern_matches in matches_by_type.items():
            signature = get_pattern_signature(pattern_type)
            if not signature:
                continue
            
            # Scan across scales
            spectrum = self.inspector.scan_all_scales(signature, file_path)
            
            # Find resonant scale
            resonant_scale, amplitude = self.detector.find_resonant_scale(spectrum)
            
            # Analyze harmonic structure
            harmonic_analysis = self.detector.analyze_harmonic_structure(spectrum)
            
            # Generate interventions
            protocol = InterventionProtocol(pattern_type, Scale(resonant_scale))
            interventions = protocol.generate_interventions(pattern_matches)
            
            # Store analysis
            results['resonant_analysis'].append({
                'pattern_type': pattern_type,
                'pattern_name': signature['name'],
                'spectrum': spectrum.to_dict(),
                'resonant_scale': resonant_scale,
                'amplitude': amplitude,
                'is_systemic': harmonic_analysis.is_systemic,
                'coherence_score': harmonic_analysis.coherence_score,
                'match_count': len(pattern_matches)
            })
            
            results['interventions'].extend(
                [intervention.to_dict() for intervention in interventions]
            )
        
        # Generate summary
        if results['interventions']:
            top_intervention = max(
                results['interventions'], 
                key=lambda x: x['confidence']
            )
            results['summary'] = {
                'status': 'issues_found',
                'total_patterns': len(matches_by_type),
                'total_interventions': len(results['interventions']),
                'top_pattern': top_intervention['pattern_type'],
                'recommended_action': f"Apply intervention: {top_intervention['description']}"
            }
        else:
            results['summary'] = {
                'status': 'patterns_detected',
                'total_patterns': len(matches_by_type),
                'message': 'Patterns detected but no interventions generated'
            }
        
        return results
    
    def debug_project(self, file_extensions: List[str] = None) -> Dict:
        """
        Run complete fractal debug analysis on entire project.
        
        Args:
            file_extensions: List of file extensions to analyze
            
        Returns:
            Dict: Project-wide analysis results
        """
        if file_extensions is None:
            file_extensions = ['.cpp', '.h']
        
        results = {
            'project_path': self.base_path,
            'timestamp': time.time(),
            'file_results': [],
            'project_summary': {},
            'top_patterns': []
        }
        
        # Find all relevant files
        files_to_analyze = []
        for root, dirs, files in os.walk(self.base_path):
            # Skip common directories to ignore
            dirs[:] = [d for d in dirs if d not in ['.git', 'Binaries', 'Intermediate', 'Saved']]
            
            for file in files:
                ext = os.path.splitext(file)[1].lower()
                if ext in file_extensions:
                    files_to_analyze.append(os.path.join(root, file))
        
        # Analyze each file
        total_matches = 0
        pattern_counts = {}
        
        for file_path in files_to_analyze:
            file_result = self.debug(file_path)
            results['file_results'].append(file_result)
            
            # Aggregate statistics
            for match in file_result['pattern_matches']:
                pattern_type = match['pattern_type']
                pattern_counts[pattern_type] = pattern_counts.get(pattern_type, 0) + 1
                total_matches += 1
        
        # Generate project summary
        results['project_summary'] = {
            'total_files_analyzed': len(files_to_analyze),
            'files_with_issues': sum(1 for fr in results['file_results'] if fr['pattern_matches']),
            'total_pattern_matches': total_matches,
            'unique_pattern_types': len(pattern_counts),
            'pattern_distribution': pattern_counts
        }
        
        # Get top patterns by priority
        results['top_patterns'] = get_top_patterns(5)
        
        return results


# Utility functions
def save_results(results: Dict, output_path: str):
    """Save analysis results to JSON file"""
    with open(output_path, 'w') as f:
        json.dump(results, f, indent=2, default=str)


def load_results(input_path: str) -> Dict:
    """Load analysis results from JSON file"""
    with open(input_path, 'r') as f:
        return json.load(f)


def print_analysis_summary(results: Dict):
    """Print a human-readable summary of analysis results"""
    print("\n" + "="*60)
    print("FRACTAL DEBUGGING ANALYSIS SUMMARY")
    print("="*60)
    
    if 'summary' in results:
        summary = results['summary']
        print(f"Status: {summary.get('status', 'unknown')}")
        
        if summary.get('status') == 'issues_found':
            print(f"Patterns detected: {summary.get('total_patterns', 0)}")
            print(f"Interventions available: {summary.get('total_interventions', 0)}")
            print(f"\nRecommended action: {summary.get('recommended_action', 'None')}")
    
    if 'resonant_analysis' in results:
        print("\nResonant Analysis:")
        for analysis in results['resonant_analysis']:
            print(f"  - {analysis['pattern_name']}")
            print(f"    Resonant scale: {analysis['resonant_scale']} "
                  f"(amplitude: {analysis['amplitude']:.2f})")
            print(f"    Systemic: {analysis['is_systemic']} "
                  f"(coherence: {analysis['coherence_score']:.2f})")
            print(f"    Matches: {analysis['match_count']}")
    
    if 'interventions' in results:
        print("\nTop Interventions:")
        # Sort by confidence
        interventions = sorted(
            results['interventions'], 
            key=lambda x: x['confidence'], 
            reverse=True
        )
        for i, intervention in enumerate(interventions[:3]):
            print(f"  {i+1}. {intervention['description']}")
            print(f"     Confidence: {intervention['confidence']:.2f}")
            print(f"     Cascade prediction: "
                  f"F:{intervention['cascade_prediction']['file_impact']:.2f} "
                  f"B:{intervention['cascade_prediction']['build_impact']:.2f} "
                  f"R:{intervention['cascade_prediction']['runtime_impact']:.2f}")
    
    print("\n" + "="*60)