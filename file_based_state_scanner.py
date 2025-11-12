"""
File-Based State Scanner for the Fractal Debugging Framework.
Integrates with the existing file-based coordination system to detect
state desynchronization and coordination issues.
"""

import os
import json
import time
import re
from typing import Dict, List, Tuple, Optional, Any
from dataclasses import dataclass, asdict
from datetime import datetime

from pattern_signatures_unreal import SIGNATURES


@dataclass
class TemporalCoherence:
    """Temporal coherence analysis results"""
    micro_timestamp: float
    meso_timestamp: float
    macro_timestamp: float
    meta_timestamp: float
    gaps: Dict[str, float]
    coherence_score: float
    
    def to_dict(self):
        return asdict(self)


@dataclass
class WriteConflict:
    """Detected write conflict information"""
    file_path: str
    conflict_type: str
    processes: List[str]
    timestamp: float
    severity: str
    
    def to_dict(self):
        return asdict(self)


@dataclass
class StateDesync:
    """State desynchronization event"""
    agent_id: str
    expected_state: Dict
    actual_state: Dict
    divergence_point: str
    timestamp: float
    
    def to_dict(self):
        return asdict(self)


class FileBasedStateScanner:
    """
    Scanner for file-coordinated agent systems.
    Detects state desynchronization through file analysis.
    """
    
    def __init__(self, base_path: str):
        self.base_path = base_path
        self.state_files = {
            'micro': 'session_state.json',
            'meso': 'execution_queue.json',
            'macro': 'task_queue_*.json',
            'meta': 'project_state.json'
        }
        self.agent_patterns = [
            r'agent_\d+\.json',
            r'ai_agent_\w+\.json',
            r'worker_\d+\.json'
        ]
    
    def scan_temporal_coherence(self) -> TemporalCoherence:
        """
        Check if file modification times show proper coordination.
        State desync manifests as temporal gaps across scales.
        
        Returns:
            TemporalCoherence: Temporal analysis results
        """
        timestamps = {}
        
        # Scan each scale level
        for scale, pattern in self.state_files.items():
            timestamps[scale] = self._get_scale_timestamp(scale, pattern)
        
        # Calculate gaps between scales
        gaps = {}
        scales = ['micro', 'meso', 'macro', 'meta']
        
        for i in range(len(scales) - 1):
            current = scales[i]
            next_scale = scales[i + 1]
            
            if timestamps[current] > 0 and timestamps[next_scale] > 0:
                gap = abs(timestamps[current] - timestamps[next_scale])
                gaps[f"{current}-{next_scale}"] = gap
        
        # Calculate coherence score (lower gaps = higher coherence)
        if gaps:
            max_gap = max(gaps.values())
            if max_gap > 0:
                coherence_score = 1.0 - min(max_gap / 300.0, 1.0)  # 5 minute threshold
            else:
                coherence_score = 1.0
        else:
            coherence_score = 0.5  # Unknown coherence
        
        return TemporalCoherence(
            micro_timestamp=timestamps['micro'],
            meso_timestamp=timestamps['meso'],
            macro_timestamp=timestamps['macro'],
            meta_timestamp=timestamps['meta'],
            gaps=gaps,
            coherence_score=coherence_score
        )
    
    def _get_scale_timestamp(self, scale: str, pattern: str) -> float:
        """Get timestamp for a specific scale"""
        if '*' in pattern:
            # Handle wildcard patterns
            files = self._find_files(pattern)
            if files:
                return max(os.path.getmtime(f) for f in files)
        else:
            # Handle specific files
            file_path = os.path.join(self.base_path, pattern)
            if os.path.exists(file_path):
                return os.path.getmtime(file_path)
        
        return 0.0
    
    def _find_files(self, pattern: str) -> List[str]:
        """Find files matching wildcard pattern"""
        import glob
        search_path = os.path.join(self.base_path, pattern)
        return glob.glob(search_path)
    
    def detect_write_conflicts(self) -> List[WriteConflict]:
        """
        Detect resonance pattern in write contention.
        
        Returns:
            List[WriteConflict]: Detected write conflicts
        """
        conflicts = []
        
        # Check for file locks
        conflicts.extend(self._check_file_locks())
        
        # Check write sequences
        conflicts.extend(self._check_write_sequences())
        
        # Check coordination protocol
        conflicts.extend(self._check_coordination_protocol())
        
        return conflicts
    
    def _check_file_locks(self) -> List[WriteConflict]:
        """Check for file locking issues"""
        conflicts = []
        
        # Check common lock files
        lock_patterns = ['*.lock', '*.tmp', '*.pid']
        
        for pattern in lock_patterns:
            lock_files = self._find_files(pattern)
            for lock_file in lock_files:
                try:
                    # Try to open file exclusively
                    with open(lock_file, 'r+') as f:
                        pass
                except IOError:
                    # File is locked
                    conflicts.append(WriteConflict(
                        file_path=lock_file,
                        conflict_type='file_lock',
                        processes=self._get_locking_processes(lock_file),
                        timestamp=time.time(),
                        severity='high'
                    ))
        
        return conflicts
    
    def _check_write_sequences(self) -> List[WriteConflict]:
        """Check for write sequence violations"""
        conflicts = []
        
        # Check execution queue for sequence violations
        queue_file = os.path.join(self.base_path, 'execution_queue.json')
        if os.path.exists(queue_file):
            try:
                with open(queue_file, 'r') as f:
                    queue_data = json.load(f)
                
                # Check for duplicate or out-of-order entries
                if isinstance(queue_data, list):
                    seen_ids = set()
                    for i, item in enumerate(queue_data):
                        item_id = item.get('id', str(i))
                        if item_id in seen_ids:
                            conflicts.append(WriteConflict(
                                file_path=queue_file,
                                conflict_type='sequence_violation',
                                processes=['execution_queue'],
                                timestamp=time.time(),
                                severity='medium'
                            ))
                        seen_ids.add(item_id)
            except (json.JSONDecodeError, IOError):
                pass
        
        return conflicts
    
    def _check_coordination_protocol(self) -> List[WriteConflict]:
        """Check coordination protocol violations"""
        conflicts = []
        
        # Check session state consistency
        state_file = os.path.join(self.base_path, 'session_state.json')
        if os.path.exists(state_file):
            try:
                with open(state_file, 'r') as f:
                    state_data = json.load(f)
                
                # Check for required fields
                required_fields = ['session_id', 'timestamp', 'active_agents']
                missing_fields = [f for f in required_fields if f not in state_data]
                
                if missing_fields:
                    conflicts.append(WriteConflict(
                        file_path=state_file,
                        conflict_type='protocol_violation',
                        processes=['session_manager'],
                        timestamp=time.time(),
                        severity='high'
                    ))
            except (json.JSONDecodeError, IOError):
                conflicts.append(WriteConflict(
                    file_path=state_file,
                    conflict_type='corrupted_state',
                    processes=['session_manager'],
                    timestamp=time.time(),
                    severity='high'
                ))
        
        return conflicts
    
    def _get_locking_processes(self, lock_file: str) -> List[str]:
        """Get processes holding a file lock"""
        # This is a simplified version - in practice, you'd use platform-specific tools
        processes = []
        
        # Check if there's a corresponding .pid file
        pid_file = lock_file.replace('.lock', '.pid')
        if os.path.exists(pid_file):
            try:
                with open(pid_file, 'r') as f:
                    pid = f.read().strip()
                    processes.append(f"process_{pid}")
            except IOError:
                pass
        
        return processes
    
    def detect_state_desync(self) -> List[StateDesync]:
        """
        Detect state desynchronization across agents.
        
        Returns:
            List[StateDesync]: Detected desynchronization events
        """
        desync_events = []
        
        # Find all agent state files
        agent_files = self._find_agent_state_files()
        
        if len(agent_files) < 2:
            return desync_events  # Need at least 2 agents to detect desync
        
        # Load agent states
        agent_states = {}
        for agent_file in agent_files:
            try:
                with open(agent_file, 'r') as f:
                    state = json.load(f)
                    agent_id = os.path.basename(agent_file)
                    agent_states[agent_id] = state
            except (json.JSONDecodeError, IOError):
                continue
        
        # Check for desynchronization
        desync_events.extend(self._check_agent_desync(agent_states))
        
        # Check for workflow desync
        desync_events.extend(self._check_workflow_desync(agent_states))
        
        return desync_events
    
    def _find_agent_state_files(self) -> List[str]:
        """Find all agent state files"""
        agent_files = []
        
        for pattern in self.agent_patterns:
            agent_files.extend(self._find_files(pattern))
        
        return agent_files
    
    def _check_agent_desync(self, agent_states: Dict) -> List[StateDesync]:
        """Check for agent-level desynchronization"""
        desync_events = []
        
        if not agent_states:
            return desync_events
        
        # Find a reference agent (the one with the most recent timestamp)
        reference_agent = None
        max_timestamp = 0
        
        for agent_id, state in agent_states.items():
            timestamp = state.get('timestamp', 0)
            if timestamp > max_timestamp:
                max_timestamp = timestamp
                reference_agent = agent_id
        
        if not reference_agent:
            return desync_events
        
        # Compare other agents to reference
        reference_state = agent_states[reference_agent]
        
        for agent_id, state in agent_states.items():
            if agent_id == reference_agent:
                continue
            
            # Check for divergence
            divergence = self._find_divergence_point(reference_state, state)
            if divergence:
                desync_events.append(StateDesync(
                    agent_id=agent_id,
                    expected_state=reference_state,
                    actual_state=state,
                    divergence_point=divergence,
                    timestamp=time.time()
                ))
        
        return desync_events
    
    def _check_workflow_desync(self, agent_states: Dict) -> List[StateDesync]:
        """Check for workflow-level desynchronization"""
        desync_events = []
        
        # Check if agents agree on workflow state
        workflow_states = {}
        
        for agent_id, state in agent_states.items():
            workflow_state = state.get('workflow_state', {})
            workflow_id = workflow_state.get('id', 'unknown')
            
            if workflow_id not in workflow_states:
                workflow_states[workflow_id] = []
            workflow_states[workflow_id].append((agent_id, workflow_state))
        
        # Check for inconsistencies in each workflow
        for workflow_id, states in workflow_states.items():
            if len(states) < 2:
                continue
            
            # Compare states
            reference_agent, reference_state = states[0]
            
            for agent_id, state in states[1:]:
                divergence = self._find_divergence_point(reference_state, state)
                if divergence:
                    desync_events.append(StateDesync(
                        agent_id=agent_id,
                        expected_state=reference_state,
                        actual_state=state,
                        divergence_point=f"workflow_{workflow_id}_{divergence}",
                        timestamp=time.time()
                    ))
        
        return desync_events
    
    def _find_divergence_point(self, state1: Dict, state2: Dict, path: str = "") -> Optional[str]:
        """Find the point where two states diverge"""
        # Check keys
        all_keys = set(state1.keys()) | set(state2.keys())
        
        for key in all_keys:
            current_path = f"{path}.{key}" if path else key
            
            if key not in state1:
                return f"{current_path}_missing_in_reference"
            if key not in state2:
                return f"{current_path}_missing_in_actual"
            
            val1 = state1[key]
            val2 = state2[key]
            
            # Handle nested dictionaries
            if isinstance(val1, dict) and isinstance(val2, dict):
                divergence = self._find_divergence_point(val1, val2, current_path)
                if divergence:
                    return divergence
            # Handle lists
            elif isinstance(val1, list) and isinstance(val2, list):
                if len(val1) != len(val2):
                    return f"{current_path}_length_mismatch"
                for i, (item1, item2) in enumerate(zip(val1, val2)):
                    if isinstance(item1, dict) and isinstance(item2, dict):
                        divergence = self._find_divergence_point(item1, item2, f"{current_path}[{i}]")
                        if divergence:
                            return divergence
                    elif item1 != item2:
                        return f"{current_path}[{i}]_value_mismatch"
            # Handle primitive values
            elif val1 != val2:
                return f"{current_path}_value_mismatch"
        
        return None
    
    def generate_state_report(self) -> Dict:
        """
        Generate comprehensive state analysis report.
        
        Returns:
            Dict: Complete state analysis report
        """
        report = {
            'timestamp': time.time(),
            'temporal_coherence': {},
            'write_conflicts': [],
            'state_desync': [],
            'summary': {}
        }
        
        # Check temporal coherence
        temporal_coherence = self.scan_temporal_coherence()
        report['temporal_coherence'] = temporal_coherence.to_dict()
        
        # Detect write conflicts
        write_conflicts = self.detect_write_conflicts()
        report['write_conflicts'] = [wc.to_dict() for wc in write_conflicts]
        
        # Detect state desync
        state_desync = self.detect_state_desync()
        report['state_desync'] = [sd.to_dict() for sd in state_desync]
        
        # Generate summary
        total_issues = len(write_conflicts) + len(state_desync)
        report['summary'] = {
            'total_issues': total_issues,
            'temporal_coherence_score': temporal_coherence.coherence_score,
            'write_conflicts': len(write_conflicts),
            'state_desync_events': len(state_desync),
            'status': 'healthy' if total_issues == 0 else 'degraded' if total_issues < 5 else 'critical'
        }
        
        return report


def monitor_state_health(base_path: str, interval: int = 30) -> None:
    """
    Continuously monitor state health.
    
    Args:
        base_path: Base path to monitor
        interval: Check interval in seconds
    """
    scanner = FileBasedStateScanner(base_path)
    
    print(f"Starting state health monitor (interval: {interval}s)")
    
    try:
        while True:
            report = scanner.generate_state_report()
            
            if report['summary']['status'] != 'healthy':
                print(f"\n[WARNING] State health degraded: {report['summary']['status'].upper()}")
                print(f"  Issues: {report['summary']['total_issues']}")
                print(f"  Coherence: {report['summary']['temporal_coherence_score']:.2f}")
            
            time.sleep(interval)
    except KeyboardInterrupt:
        print("\nMonitoring stopped by user")


def save_state_report(report: Dict, output_path: str) -> None:
    """Save state report to file"""
    with open(output_path, 'w') as f:
        json.dump(report, f, indent=2, default=str)


def load_state_report(input_path: str) -> Dict:
    """Load state report from file"""
    with open(input_path, 'r') as f:
        return json.load(f)


def print_state_report(report: Dict) -> None:
    """Print human-readable state report"""
    print("\n" + "="*60)
    print("STATE HEALTH REPORT")
    print("="*60)
    
    summary = report.get('summary', {})
    print(f"Status: {summary.get('status', 'unknown').upper()}")
    print(f"Coherence Score: {summary.get('temporal_coherence_score', 0):.2f}")
    print(f"Total Issues: {summary.get('total_issues', 0)}")
    
    if report.get('write_conflicts'):
        print("\nWrite Conflicts:")
        for conflict in report['write_conflicts']:
            print(f"  - {conflict['file_path']}")
            print(f"    Type: {conflict['conflict_type']}")
            print(f"    Severity: {conflict['severity']}")
    
    if report.get('state_desync'):
        print("\nState Desync Events:")
        for desync in report['state_desync']:
            print(f"  - Agent: {desync['agent_id']}")
            print(f"    Divergence: {desync['divergence_point']}")
    
    print("\n" + "="*60)