"""
AI Workflow Orchestrator
Coordinates multiple AI agents to execute complex workflows
"""

from ai_unreal_controller import AIUnrealController
from ai_agents import AIContentAgent, AIBuildAgent, AITestingAgent
from typing import Dict, List, Optional
from datetime import datetime
import logging
import json
from pathlib import Path

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class WorkflowOrchestrator:
    """
    High-level orchestrator that coordinates AI agents
    Implements complete workflow pipelines
    """
    
    def __init__(self, engine_path: str, project_path: str):
        """
        Initialize orchestrator with all agents
        
        Args:
            engine_path: Path to UE installation
            project_path: Path to .uproject file
        """
        self.controller = AIUnrealController(engine_path, project_path)
        
        # Initialize specialized agents
        self.content_agent = AIContentAgent(self.controller)
        self.build_agent = AIBuildAgent(self.controller)
        self.test_agent = AITestingAgent(self.controller)
        
        self.reports_dir = Path("Saved/AI_Reports")
        self.reports_dir.mkdir(parents=True, exist_ok=True)
        
        logger.info("WorkflowOrchestrator initialized with all agents")
        
    def daily_maintenance_workflow(self) -> Dict:
        """
        Complete daily maintenance workflow
        - Validate all content
        - Find unused assets
        - Generate asset report
        - Run smoke tests
        - Generate summary report
        
        Returns:
            Complete maintenance report
        """
        logger.info("=== Starting Daily Maintenance Workflow ===")
        
        start_time = datetime.now()
        workflow_results = {
            'workflow': 'daily_maintenance',
            'start_time': start_time.isoformat(),
            'steps': []
        }
        
        # Step 1: Content Validation
        logger.info("Step 1: Content Validation")
        validation_result = self.content_agent.validate_all_content()
        workflow_results['steps'].append({
            'name': 'content_validation',
            'success': validation_result['success'],
            'errors': validation_result.get('error_count', 0),
            'warnings': validation_result.get('warning_count', 0)
        })
        
        # Step 2: Find Unused Assets
        logger.info("Step 2: Unused Asset Detection")
        unused_result = self.content_agent.cleanup_unused_assets(dry_run=True)
        workflow_results['steps'].append({
            'name': 'unused_assets',
            'success': unused_result['success'],
            'found': unused_result.get('found', 0)
        })
        
        # Step 3: Generate Asset Report
        logger.info("Step 3: Asset Report Generation")
        report_result = self.controller.generate_asset_report()
        workflow_results['steps'].append({
            'name': 'asset_report',
            'success': report_result['success'],
            'total_assets': report_result.get('report', {}).get('total_assets', 0)
        })
        
        # Step 4: Run Smoke Tests
        logger.info("Step 4: Smoke Tests")
        test_result = self.test_agent.run_smoke_tests()
        workflow_results['steps'].append({
            'name': 'smoke_tests',
            'success': test_result['success']
        })
        
        # Complete workflow
        end_time = datetime.now()
        workflow_results['end_time'] = end_time.isoformat()
        workflow_results['duration_seconds'] = (end_time - start_time).total_seconds()
        workflow_results['overall_success'] = all(
            step['success'] for step in workflow_results['steps']
        )
        
        # Save report
        report_path = self.reports_dir / f"daily_maintenance_{start_time.strftime('%Y%m%d_%H%M%S')}.json"
        report_path.write_text(json.dumps(workflow_results, indent=2))
        
        logger.info(f"=== Daily Maintenance Complete ({workflow_results['duration_seconds']:.1f}s) ===")
        logger.info(f"Report saved: {report_path}")
        
        return workflow_results
    
    def pre_commit_workflow(self, changed_files: Optional[List[str]] = None) -> Dict:
        """
        Fast validation workflow before commits
        - Validate changed content
        - Run affected tests
        - Check for errors
        
        Args:
            changed_files: List of changed file paths
            
        Returns:
            Validation results
        """
        logger.info("=== Starting Pre-Commit Validation ===")
        
        start_time = datetime.now()
        workflow_results = {
            'workflow': 'pre_commit',
            'start_time': start_time.isoformat(),
            'changed_files': changed_files or [],
            'steps': []
        }
        
        # Step 1: Validate Changed Content
        logger.info("Step 1: Validating changed content")
        
        # Extract content paths from changed files
        content_paths = []
        if changed_files:
            for file_path in changed_files:
                if '/Content/' in file_path:
                    # Convert to Unreal path
                    parts = file_path.split('/Content/')
                    if len(parts) > 1:
                        content_path = '/Game/' + parts[1].rsplit('.', 1)[0]
                        content_paths.append(content_path)
        
        if content_paths:
            validation_result = self.content_agent.validate_all_content(paths=content_paths)
        else:
            # No content changes, quick validation
            validation_result = {'success': True, 'error_count': 0}
            
        workflow_results['steps'].append({
            'name': 'content_validation',
            'success': validation_result['success'],
            'errors': validation_result.get('error_count', 0)
        })
        
        # Step 2: Run Quick Smoke Tests
        logger.info("Step 2: Quick smoke tests")
        test_result = self.test_agent.run_smoke_tests()
        workflow_results['steps'].append({
            'name': 'smoke_tests',
            'success': test_result['success']
        })
        
        # Complete workflow
        end_time = datetime.now()
        workflow_results['end_time'] = end_time.isoformat()
        workflow_results['duration_seconds'] = (end_time - start_time).total_seconds()
        workflow_results['overall_success'] = all(
            step['success'] for step in workflow_results['steps']
        )
        
        # Quick report
        if workflow_results['overall_success']:
            logger.info(f"=== Pre-Commit Validation PASSED ({workflow_results['duration_seconds']:.1f}s) ===")
        else:
            logger.error(f"=== Pre-Commit Validation FAILED ({workflow_results['duration_seconds']:.1f}s) ===")
            
        return workflow_results
    
    def content_creation_workflow(self, content_type: str = "environment") -> Dict:
        """
        Guided content creation workflow
        - Setup folder structure
        - Import assets
        - Auto-organize
        - Generate LODs
        - Validate
        
        Args:
            content_type: Type of content (environment, character, prop, etc.)
            
        Returns:
            Creation workflow results
        """
        logger.info(f"=== Starting Content Creation Workflow: {content_type} ===")
        
        start_time = datetime.now()
        workflow_results = {
            'workflow': 'content_creation',
            'content_type': content_type,
            'start_time': start_time.isoformat(),
            'steps': []
        }
        
        # Step 1: Organize imported assets
        logger.info("Step 1: Organizing imported assets")
        organize_result = self.content_agent.organize_content_by_type()
        workflow_results['steps'].append({
            'name': 'organize_assets',
            'success': organize_result['success'],
            'moved': organize_result.get('organization_report', {}).get('moved', 0)
        })
        
        # Step 2: Auto-generate LODs
        logger.info("Step 2: Generating LODs")
        lod_result = self.content_agent.auto_generate_lods(quality="Medium")
        workflow_results['steps'].append({
            'name': 'generate_lods',
            'success': lod_result['success'],
            'optimized': lod_result.get('optimization_report', {}).get('optimized', 0)
        })
        
        # Step 3: Validate content
        logger.info("Step 3: Content validation")
        validation_result = self.content_agent.validate_all_content()
        workflow_results['steps'].append({
            'name': 'content_validation',
            'success': validation_result['success'],
            'errors': validation_result.get('error_count', 0)
        })
        
        # Complete workflow
        end_time = datetime.now()
        workflow_results['end_time'] = end_time.isoformat()
        workflow_results['duration_seconds'] = (end_time - start_time).total_seconds()
        workflow_results['overall_success'] = all(
            step['success'] for step in workflow_results['steps']
        )
        
        # Save report
        report_path = self.reports_dir / f"content_creation_{content_type}_{start_time.strftime('%Y%m%d_%H%M%S')}.json"
        report_path.write_text(json.dumps(workflow_results, indent=2))
        
        logger.info(f"=== Content Creation Complete ({workflow_results['duration_seconds']:.1f}s) ===")
        logger.info(f"Report saved: {report_path}")
        
        return workflow_results
    
    def performance_optimization_workflow(self) -> Dict:
        """
        Comprehensive performance optimization workflow
        - Analyze current performance
        - Optimize assets
        - Run performance tests
        - Generate report
        
        Returns:
            Optimization results
        """
        logger.info("=== Starting Performance Optimization Workflow ===")
        
        start_time = datetime.now()
        workflow_results = {
            'workflow': 'performance_optimization',
            'start_time': start_time.isoformat(),
            'steps': []
        }
        
        # Step 1: Baseline performance
        logger.info("Step 1: Baseline performance measurement")
        baseline_result = self.test_agent.run_performance_tests()
        workflow_results['steps'].append({
            'name': 'baseline_performance',
            'success': baseline_result['success'],
            'metrics': baseline_result
        })
        
        # Step 2: Optimize static meshes
        logger.info("Step 2: Static mesh optimization")
        mesh_result = self.content_agent.auto_generate_lods(
            quality="High",
            asset_filter="StaticMesh"
        )
        workflow_results['steps'].append({
            'name': 'optimize_static_meshes',
            'success': mesh_result['success'],
            'optimized': mesh_result.get('optimization_report', {}).get('optimized', 0)
        })
        
        # Step 3: Optimize textures
        logger.info("Step 3: Texture optimization")
        texture_result = self.content_agent.auto_generate_lods(
            quality="High",
            asset_filter="Texture2D"
        )
        workflow_results['steps'].append({
            'name': 'optimize_textures',
            'success': texture_result['success'],
            'optimized': texture_result.get('optimization_report', {}).get('optimized', 0)
        })
        
        # Step 4: Post-optimization performance
        logger.info("Step 4: Post-optimization measurement")
        final_result = self.test_agent.run_performance_tests()
        workflow_results['steps'].append({
            'name': 'final_performance',
            'success': final_result['success'],
            'metrics': final_result
        })
        
        # Complete workflow
        end_time = datetime.now()
        workflow_results['end_time'] = end_time.isoformat()
        workflow_results['duration_seconds'] = (end_time - start_time).total_seconds()
        workflow_results['overall_success'] = all(
            step['success'] for step in workflow_results['steps']
        )
        
        # Calculate improvements
        if baseline_result['success'] and final_result['success']:
            import json
            baseline_data = json.loads(baseline_result.get('stdout', '{}'))
            final_data = json.loads(final_result.get('stdout', '{}'))
            
            workflow_results['improvements'] = {
                'fps_change': final_data.get('fps_avg', 0) - baseline_data.get('fps_avg', 0),
                'memory_change_mb': baseline_data.get('memory_mb', 0) - final_data.get('memory_mb', 0)
            }
        
        # Save report
        report_path = self.reports_dir / f"performance_optimization_{start_time.strftime('%Y%m%d_%H%M%S')}.json"
        report_path.write_text(json.dumps(workflow_results, indent=2))
        
        logger.info(f"=== Performance Optimization Complete ({workflow_results['duration_seconds']:.1f}s) ===")
        logger.info(f"Report saved: {report_path}")
        
        return workflow_results
    
    def get_workflow_history(self, workflow_type: Optional[str] = None, limit: int = 10) -> List[Dict]:
        """
        Get history of executed workflows
        
        Args:
            workflow_type: Filter by workflow type (optional)
            limit: Maximum number of results
            
        Returns:
            List of workflow reports
        """
        reports = []
        
        for report_file in sorted(self.reports_dir.glob("*.json"), reverse=True):
            if len(reports) >= limit:
                break
                
            try:
                data = json.loads(report_file.read_text())
                
                if workflow_type and data.get('workflow') != workflow_type:
                    continue
                    
                reports.append({
                    'file': str(report_file),
                    'workflow': data.get('workflow'),
                    'timestamp': data.get('start_time'),
                    'success': data.get('overall_success'),
                    'duration': data.get('duration_seconds')
                })
                
            except Exception as e:
                logger.warning(f"Failed to read report {report_file}: {e}")
                
        return reports


# CLI interface
if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(description="AI Workflow Orchestrator for Unreal Engine")
    parser.add_argument('workflow', choices=[
        'daily_maintenance',
        'pre_commit',
        'content_creation',
        'performance_optimization',
        'history'
    ], help="Workflow to execute")
    parser.add_argument('--content-type', default='environment', 
                       help="Content type for content_creation workflow")
    parser.add_argument('--limit', type=int, default=10,
                       help="Limit for history query")
    
    args = parser.parse_args()
    
    # Initialize orchestrator
    orchestrator = WorkflowOrchestrator(
        engine_path="C:/Program Files/Epic Games/UE_5.6",
        project_path="C:/Users/allen/Desktop/Alexander/Alexander/Alexander.uproject"
    )
    
    # Execute workflow
    if args.workflow == 'daily_maintenance':
        result = orchestrator.daily_maintenance_workflow()
        print(f"\nDaily Maintenance: {'SUCCESS' if result['overall_success'] else 'FAILED'}")
        print(f"Duration: {result['duration_seconds']:.1f}s")
        
    elif args.workflow == 'pre_commit':
        result = orchestrator.pre_commit_workflow()
        print(f"\nPre-Commit Validation: {'PASSED' if result['overall_success'] else 'FAILED'}")
        
    elif args.workflow == 'content_creation':
        result = orchestrator.content_creation_workflow(content_type=args.content_type)
        print(f"\nContent Creation: {'SUCCESS' if result['overall_success'] else 'FAILED'}")
        
    elif args.workflow == 'performance_optimization':
        result = orchestrator.performance_optimization_workflow()
        print(f"\nPerformance Optimization: {'SUCCESS' if result['overall_success'] else 'FAILED'}")
        
    elif args.workflow == 'history':
        history = orchestrator.get_workflow_history(limit=args.limit)
        print(f"\nWorkflow History (last {len(history)} runs):")
        for item in history:
            print(f"  [{item['timestamp']}] {item['workflow']}: {'SUCCESS' if item['success'] else 'FAILED'} ({item['duration']:.1f}s)")
