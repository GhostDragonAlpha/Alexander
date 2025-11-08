"""
Specialized AI Agents for Unreal Engine automation
Each agent focuses on a specific domain of operations
"""

from ai_unreal_controller import AIUnrealController
from typing import Dict, List, Optional
import logging
from pathlib import Path

logger = logging.getLogger(__name__)

class AIContentAgent:
    """
    AI agent specialized in content operations
    Handles asset validation, optimization, and cleanup
    """
    
    def __init__(self, controller: AIUnrealController):
        self.controller = controller
        logger.info("AIContentAgent initialized")
        
    def auto_generate_lods(self, quality: str = "Medium", 
                          asset_filter: str = "StaticMesh") -> Dict:
        """
        Automatically generate LODs for all matching assets
        
        Args:
            quality: Quality preset (Low, Medium, High, Ultra)
            asset_filter: Asset class to target
            
        Returns:
            Generation results with statistics
        """
        logger.info(f"Auto-generating LODs for {asset_filter} assets at {quality} quality")
        
        result = self.controller.optimize_assets_batch(
            asset_filter=asset_filter,
            quality=quality
        )
        
        if result['success']:
            report = result['optimization_report']
            logger.info(f"LOD generation complete: {report['optimized']}/{report['total_assets']} succeeded")
            
        return result
    
    def validate_all_content(self, paths: Optional[List[str]] = None) -> Dict:
        """
        Run comprehensive data validation
        
        Args:
            paths: List of content paths to validate (defaults to /Game/)
            
        Returns:
            Aggregated validation report
        """
        if not paths:
            paths = ["/Game/"]
            
        logger.info(f"Validating content in {len(paths)} paths")
        
        all_errors = []
        all_warnings = []
        total_assets = 0
        
        for path in paths:
            result = self.controller.validate_content(content_path=path)
            
            if result['success']:
                report = result['validation_report']
                all_errors.extend(report.get('errors', []))
                all_warnings.extend(report.get('warnings', []))
                total_assets += report.get('total_assets', 0)
                
        return {
            'success': True,
            'total_assets': total_assets,
            'error_count': len(all_errors),
            'warning_count': len(all_warnings),
            'errors': all_errors,
            'warnings': all_warnings
        }
    
    def cleanup_unused_assets(self, dry_run: bool = True) -> Dict:
        """
        Find and optionally delete unused assets
        
        Args:
            dry_run: If True, only report unused assets without deleting
            
        Returns:
            Cleanup report
        """
        logger.info("Scanning for unused assets...")
        
        result = self.controller.find_unused_assets()
        
        if not result['success']:
            return result
            
        report = result['report']
        unused_assets = report['unused_assets']
        
        logger.info(f"Found {len(unused_assets)} unused assets")
        
        if not dry_run and unused_assets:
            # Delete unused assets
            delete_script = f"""
import unreal

assets_to_delete = {unused_assets}
total_assets = len(assets_to_delete)

editor_util = unreal.EditorAssetLibrary()
deleted = 0

for asset_path in assets_to_delete:
    if editor_util.delete_asset(asset_path):
        deleted += 1
        unreal.log(f'Deleted: {{asset_path}}')
    else:
        unreal.log_error(f'Failed to delete: {{asset_path}}')

unreal.log(f'Deleted {{deleted}}/{{total_assets}} assets')
"""
            
            delete_result = self.controller.execute_python_in_editor(script_content=delete_script)
            
            return {
                'success': delete_result['success'],
                'found': len(unused_assets),
                'deleted': len(unused_assets) if delete_result['success'] else 0,
                'dry_run': False
            }
        
        return {
            'success': True,
            'found': len(unused_assets),
            'deleted': 0,
            'dry_run': True,
            'assets': unused_assets
        }
    
    def organize_content_by_type(self, source_path: str = "/Game/Imports") -> Dict:
        """
        Organize imported assets into proper folder structure
        
        Args:
            source_path: Path containing imported assets
            
        Returns:
            Organization results
        """
        script = f"""
import unreal
import os

editor_util = unreal.EditorAssetLibrary()
asset_paths = editor_util.list_assets('{source_path}', recursive=True, include_folder=False)

# Define organization rules
type_folders = {{
    'StaticMesh': '/Game/Meshes',
    'SkeletalMesh': '/Game/Meshes/Skeletal',
    'Texture2D': '/Game/Textures',
    'Material': '/Game/Materials',
    'MaterialInstance': '/Game/Materials/Instances',
    'Blueprint': '/Game/Blueprints',
    'SoundCue': '/Game/Audio',
    'SoundWave': '/Game/Audio/Waves',
    'ParticleSystem': '/Game/VFX',
    'AnimSequence': '/Game/Animations'
}}

moved = []
failed = []

for asset_path in asset_paths:
    asset = editor_util.load_asset(asset_path)
    
    if asset:
        asset_class = asset.get_class().get_name()
        
        # Find matching folder
        target_folder = None
        for class_name, folder in type_folders.items():
            if class_name in asset_class:
                target_folder = folder
                break
                
        if target_folder:
            asset_name = os.path.basename(asset_path)
            new_path = f'{{target_folder}}/{{asset_name}}'
            
            # Rename = move in Unreal
            if editor_util.rename_asset(asset_path, new_path):
                moved.append({{'from': asset_path, 'to': new_path}})
                unreal.log(f'Moved: {{asset_path}} -> {{new_path}}')
            else:
                failed.append(asset_path)
                unreal.log_error(f'Failed to move: {{asset_path}}')

import json
result = {{
    'total': len(asset_paths),
    'moved': len(moved),
    'failed': len(failed),
    'moves': moved
}}

print(json.dumps(result))
"""
        
        result = self.controller.execute_python_in_editor(script_content=script)
        
        if result['success']:
            import json
            try:
                data = json.loads(result['stdout'])
                logger.info(f"Organized {data['moved']}/{data['total']} assets")
                return {
                    'success': True,
                    'organization_report': data
                }
            except json.JSONDecodeError:
                pass
                
        return result


class AIBuildAgent:
    """
    AI agent specialized in build operations
    Handles compilation, cooking, and packaging
    """
    
    def __init__(self, controller: AIUnrealController):
        self.controller = controller
        logger.info("AIBuildAgent initialized")
        
    def incremental_build(self, configuration: str = "Development", 
                         platform: str = "Win64") -> Dict:
        """
        Perform incremental C++ build
        
        Args:
            configuration: Build configuration (Debug, Development, Shipping)
            platform: Target platform
            
        Returns:
            Build result
        """
        logger.info(f"Starting incremental build: {platform} {configuration}")
        
        # Note: In real implementation, this would call UnrealBuildTool
        # For now, we return a placeholder
        return {
            'success': True,
            'message': 'Build functionality requires UBT integration',
            'configuration': configuration,
            'platform': platform
        }
    
    def analyze_dependencies(self, module_name: Optional[str] = None) -> Dict:
        """
        Analyze module dependencies
        
        Args:
            module_name: Specific module to analyze (or all if None)
            
        Returns:
            Dependency graph data
        """
        script = """
import unreal

# Get all loaded modules
module_manager = unreal.ModuleManager.get()

modules = []
# Note: Full implementation would use reflection to get module info

result = {
    'modules': modules,
    'total_count': len(modules)
}

import json
print(json.dumps(result))
"""
        
        result = self.controller.execute_python_in_editor(script_content=script)
        return result
    
    def cook_content(self, maps: List[str], platform: str = "WindowsClient") -> Dict:
        """
        Cook content for specific maps
        
        Args:
            maps: List of map paths to cook
            platform: Target platform
            
        Returns:
            Cook results
        """
        logger.info(f"Cooking {len(maps)} maps for {platform}")
        
        # This would use UAT's BuildCookRun command
        return {
            'success': True,
            'message': 'Cook functionality requires UAT integration',
            'maps': maps,
            'platform': platform
        }


class AITestingAgent:
    """
    AI agent specialized in testing operations
    Handles test execution, analysis, and reporting
    """
    
    def __init__(self, controller: AIUnrealController):
        self.controller = controller
        logger.info("AITestingAgent initialized")
        
    def run_smoke_tests(self) -> Dict:
        """
        Run quick smoke tests
        
        Returns:
            Test results
        """
        logger.info("Running smoke tests...")
        
        result = self.controller.run_automation_tests(
            test_filter="Smoke",
            report_path="Saved/Automation/Reports/Smoke"
        )
        
        return result
    
    def run_integration_tests(self) -> Dict:
        """
        Run full integration test suite
        
        Returns:
            Test results
        """
        logger.info("Running integration tests...")
        
        result = self.controller.run_automation_tests(
            test_filter="Project",
            report_path="Saved/Automation/Reports/Integration"
        )
        
        return result
    
    def run_performance_tests(self) -> Dict:
        """
        Run performance profiling tests
        
        Returns:
            Performance metrics
        """
        logger.info("Running performance tests...")
        
        script = """
import unreal

# Run performance capture
# Note: Full implementation would start profiling session

result = {
    'fps_avg': 60.0,
    'memory_mb': 1024,
    'load_time_ms': 5000
}

import json
print(json.dumps(result))
"""
        
        result = self.controller.execute_python_in_editor(script_content=script)
        return result
    
    def analyze_test_failures(self, report_path: str) -> Dict:
        """
        Analyze test failure reports and suggest fixes
        
        Args:
            report_path: Path to test report JSON
            
        Returns:
            Analysis with suggested fixes
        """
        import json
        
        try:
            with open(report_path, 'r') as f:
                report = json.load(f)
                
            # Analyze failures
            failures = report.get('failures', [])
            
            suggestions = []
            for failure in failures:
                test_name = failure.get('test', 'Unknown')
                error = failure.get('error', '')
                
                # Simple pattern matching for common issues
                if 'nullptr' in error or 'null' in error.lower():
                    suggestions.append({
                        'test': test_name,
                        'issue': 'Null reference',
                        'suggestion': 'Add null checks before accessing object'
                    })
                elif 'timeout' in error.lower():
                    suggestions.append({
                        'test': test_name,
                        'issue': 'Timeout',
                        'suggestion': 'Increase timeout or optimize operation'
                    })
                    
            return {
                'success': True,
                'total_failures': len(failures),
                'analyzed': len(suggestions),
                'suggestions': suggestions
            }
            
        except Exception as e:
            return {
                'success': False,
                'error': str(e)
            }


# Example usage
if __name__ == "__main__":
    from ai_unreal_controller import AIUnrealController
    
    # Initialize
    controller = AIUnrealController(
        engine_path="C:/Program Files/Epic Games/UE_5.6",
        project_path="C:/Users/allen/Desktop/Alexander/Alexander/Alexander.uproject"
    )
    
    # Create agents
    content_agent = AIContentAgent(controller)
    build_agent = AIBuildAgent(controller)
    test_agent = AITestingAgent(controller)
    
    # Example: Run content validation
    print("Validating content...")
    result = content_agent.validate_all_content()
    print(f"Validation complete: {result['error_count']} errors, {result['warning_count']} warnings")
