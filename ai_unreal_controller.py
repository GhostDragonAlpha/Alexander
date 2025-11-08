"""
AI Unreal Controller - Core controller for programmatic Unreal Engine operations
Provides AI agents with direct access to Unreal Engine command-line tools
"""

import subprocess
import json
from pathlib import Path
from typing import Dict, List, Optional, Tuple
from enum import Enum
import logging

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class UnrealTool(Enum):
    """Available Unreal Engine command-line tools"""
    EDITOR_CMD = "UnrealEditor-Cmd.exe"
    UAT = "UnrealAutomationTool.exe"
    UBT = "UnrealBuildTool.exe"
    FRONTEND = "UnrealFrontend.exe"
    INSIGHTS = "UnrealInsights.exe"
    VERSION_SELECTOR = "UnrealVersionSelector.exe"

class AIUnrealController:
    """
    Core controller class for AI agents to interact with Unreal Engine
    Provides high-level APIs for common operations
    """
    
    def __init__(self, engine_path: str, project_path: str):
        """
        Initialize the controller
        
        Args:
            engine_path: Path to UE installation (e.g., "C:/Program Files/Epic Games/UE_5.6")
            project_path: Path to .uproject file
        """
        self.engine_path = Path(engine_path)
        self.project_path = Path(project_path)
        self.binaries = self.engine_path / "Engine/Binaries/Win64"
        self.dotnet_binaries = self.engine_path / "Engine/Binaries/DotNET"
        
        # Verify paths exist
        if not self.engine_path.exists():
            raise ValueError(f"Engine path not found: {engine_path}")
        if not self.project_path.exists():
            raise ValueError(f"Project path not found: {project_path}")
            
        logger.info(f"AIUnrealController initialized for project: {self.project_path.name}")
        
    def execute_python_in_editor(self, script_content: Optional[str] = None, script_path: Optional[str] = None, 
                                 args: Optional[Dict] = None, timeout: int = 300) -> Dict:
        """
        Execute Python script in Unreal Editor context
        
        Args:
            script_content: Python code to execute (will create temp file)
            script_path: Path to existing Python script file
            args: Arguments to pass to script
            timeout: Execution timeout in seconds
            
        Returns:
            Dict with success, stdout, stderr, returncode
        """
        if script_content and not script_path:
            # Create temporary script
            temp_dir = Path("Saved/Temp")
            temp_dir.mkdir(parents=True, exist_ok=True)
            temp_script = temp_dir / "ai_temp_script.py"
            temp_script.write_text(script_content)
            script_path = str(temp_script)
            
        if not script_path:
            raise ValueError("Either script_content or script_path must be provided")
            
        cmd = [
            str(self.binaries / UnrealTool.EDITOR_CMD.value),
            str(self.project_path),
            "-run=pythonscript",
            f"-script={script_path}",
            "-stdout",
            "-unattended",
            "-nopause",
            "-nullrhi",  # No rendering for faster execution
            "-nosplash",
            "-noloadstartuppackages",
            "-NoVerifyGC",
            "-noxrstereo",  # Disable XR
            "-messaging"  # Enable messaging for automation
        ]
        
        if args:
            cmd.append(f"-scriptargs={json.dumps(args)}")
            
        logger.info(f"Executing Python in editor: {script_path}")
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout)
            
            return {
                "success": result.returncode == 0,
                "stdout": result.stdout,
                "stderr": result.stderr,
                "returncode": result.returncode
            }
        except subprocess.TimeoutExpired:
            logger.error(f"Script execution timed out after {timeout} seconds")
            return {
                "success": False,
                "stdout": "",
                "stderr": "Execution timed out",
                "returncode": -1
            }
    
    def execute_commandlet(self, commandlet_name: str, params: Optional[List[str]] = None) -> Dict:
        """
        Execute Unreal commandlet
        
        Args:
            commandlet_name: Name of commandlet (e.g., "ResavePackages")
            params: Additional parameters
            
        Returns:
            Execution result dictionary
        """
        cmd = [
            str(self.binaries / UnrealTool.EDITOR_CMD.value),
            str(self.project_path),
            f"-run={commandlet_name}",
            "-unattended",
            "-nopause"
        ]
        
        if params:
            cmd.extend(params)
            
        logger.info(f"Executing commandlet: {commandlet_name}")
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        return {
            "success": result.returncode == 0,
            "stdout": result.stdout,
            "stderr": result.stderr,
            "returncode": result.returncode
        }
    
    def validate_content(self, content_path: str = "/Game/") -> Dict:
        """
        Run data validation on content
        
        Args:
            content_path: Path to validate (e.g., "/Game/Maps")
            
        Returns:
            Validation results with errors and warnings
        """
        script = f"""
import unreal

# Run data validation
validator = unreal.DataValidationSubsystem()
assets_to_validate = []

# Get asset registry
asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()

# Get assets in path
filter = unreal.ARFilter(
    package_paths=['{content_path}'],
    recursive_paths=True
)

asset_data_list = asset_registry.get_assets(filter)

errors = []
warnings = []

for asset_data in asset_data_list:
    asset = asset_data.get_asset()
    if asset:
        # Validate asset
        result = validator.is_asset_valid(asset)
        if not result:
            errors.append({{
                'asset': str(asset_data.package_name),
                'class': str(asset_data.asset_class)
            }})

import json
report = {{
    'total_assets': len(asset_data_list),
    'errors': errors,
    'warnings': warnings
}}

print(json.dumps(report))
"""
        
        result = self.execute_python_in_editor(script_content=script)
        
        # Parse JSON output
        if result['success']:
            try:
                report = json.loads(result['stdout'])
                return {
                    "success": True,
                    "validation_report": report
                }
            except json.JSONDecodeError:
                return {
                    "success": False,
                    "error": "Failed to parse validation output"
                }
        
        return result
    
    def run_automation_tests(self, test_filter: str = "Project", 
                            report_path: Optional[str] = None) -> Dict:
        """
        Execute automation tests
        
        Args:
            test_filter: Test filter (Project, Smoke, Integration, etc.)
            report_path: Path to save test report
            
        Returns:
            Test execution results
        """
        if not report_path:
            report_path = "Saved/Automation/Reports"
            
        cmd = [
            str(self.binaries / UnrealTool.EDITOR_CMD.value),
            str(self.project_path),
            f"-ExecCmds=Automation RunTests {test_filter}",
            "-unattended",
            "-nopause",
            "-nullrhi",
            f"-ReportOutputPath={report_path}"
        ]
        
        logger.info(f"Running automation tests: {test_filter}")
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
        
        return {
            "success": result.returncode == 0,
            "output": result.stdout,
            "report_path": report_path,
            "returncode": result.returncode
        }
    
    def build_lighting(self, map_path: str, quality: str = "Preview") -> Dict:
        """
        Build lighting for a map
        
        Args:
            map_path: Path to map (e.g., "/Game/Maps/MainLevel")
            quality: Build quality (Preview, Medium, High, Production)
            
        Returns:
            Build result
        """
        script = f"""
import unreal

# Load map
editor_level_lib = unreal.EditorLevelLibrary()
map_loaded = editor_level_lib.load_level('{map_path}')

if map_loaded:
    # Build lighting
    unreal.EditorLevelLibrary.build_lighting(
        quality=unreal.LightingBuildQuality.{quality.upper()}
    )
    unreal.log('Lighting build completed')
else:
    unreal.log_error('Failed to load map')
"""
        
        return self.execute_python_in_editor(script_content=script, timeout=3600)
    
    def generate_asset_report(self, output_path: Optional[str] = None) -> Dict:
        """
        Generate comprehensive asset usage report
        
        Args:
            output_path: Path to save report JSON
            
        Returns:
            Asset report data
        """
        if not output_path:
            output_path = "Saved/Reports/asset_report.json"
            
        script = f"""
import unreal
import json

# Get asset registry
asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()

# Get all assets
all_assets = asset_registry.get_all_assets()

report = {{
    'total_assets': len(all_assets),
    'by_type': {{}},
    'by_path': {{}},
    'total_size_estimate': 0
}}

for asset_data in all_assets:
    asset_class = str(asset_data.asset_class)
    package_path = str(asset_data.package_path)
    
    # Count by type
    if asset_class not in report['by_type']:
        report['by_type'][asset_class] = 0
    report['by_type'][asset_class] += 1
    
    # Count by path
    path_parts = package_path.split('/')
    if len(path_parts) > 2:
        root_path = '/'.join(path_parts[:3])
        if root_path not in report['by_path']:
            report['by_path'][root_path] = 0
        report['by_path'][root_path] += 1

# Save report
import os
os.makedirs(os.path.dirname('{output_path}'), exist_ok=True)

with open('{output_path}', 'w') as f:
    json.dump(report, f, indent=2)

print(json.dumps(report))
unreal.log('Asset report generated')
"""
        
        result = self.execute_python_in_editor(script_content=script)
        
        if result['success']:
            try:
                report = json.loads(result['stdout'])
                return {
                    "success": True,
                    "report": report,
                    "report_path": output_path
                }
            except json.JSONDecodeError:
                pass
                
        return result
    
    def find_unused_assets(self, output_path: Optional[str] = None) -> Dict:
        """
        Find assets with no references
        
        Args:
            output_path: Path to save results
            
        Returns:
            List of unused assets
        """
        if not output_path:
            output_path = "Saved/Reports/unused_assets.json"
            
        script = f"""
import unreal
import json

editor_util = unreal.EditorAssetLibrary()

# Get all assets
asset_paths = editor_util.list_assets('/Game/', recursive=True, include_folder=False)

unused = []

for path in asset_paths:
    # Find references
    references = editor_util.find_package_referencers_for_asset(path)
    
    if len(references) == 0:
        unused.append(path)

report = {{
    'total_checked': len(asset_paths),
    'unused_count': len(unused),
    'unused_assets': unused
}}

# Save report
import os
os.makedirs(os.path.dirname('{output_path}'), exist_ok=True)

with open('{output_path}', 'w') as f:
    json.dump(report, f, indent=2)

print(json.dumps(report))
unreal.log(f'Found {{len(unused)}} unused assets')
"""
        
        result = self.execute_python_in_editor(script_content=script, timeout=600)
        
        if result['success']:
            try:
                report = json.loads(result['stdout'])
                return {
                    "success": True,
                    "report": report,
                    "report_path": output_path
                }
            except json.JSONDecodeError:
                pass
                
        return result
    
    def optimize_assets_batch(self, asset_filter: str = "StaticMesh", 
                              quality: str = "Medium") -> Dict:
        """
        Batch optimize assets using ContentOptimizer
        
        Args:
            asset_filter: Asset class to filter (StaticMesh, Texture2D, etc.)
            quality: Optimization quality preset
            
        Returns:
            Optimization results
        """
        script = f"""
import unreal
import json

# Get ContentOptimizer
optimizer_class = unreal.load_class(None, '/Script/Alexander.ContentOptimizer')
optimizer = unreal.ContentOptimizer()

# Get quality preset
quality_enum = unreal.OptimizationQuality.{quality.upper()}
lod_settings = optimizer.get_lod_preset(quality_enum)

# Find assets
asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
asset_data_list = asset_registry.get_assets_by_class('{asset_filter}')

results = []
success_count = 0

for asset_data in asset_data_list:
    asset = asset_data.get_asset()
    if asset:
        result = optimizer.generate_lods_for_static_mesh(asset, lod_settings)
        
        if result.success:
            success_count += 1
            
        results.append({{
            'asset': str(asset_data.asset_name),
            'success': result.success,
            'original_tris': result.original_triangle_count,
            'optimized_tris': result.optimized_triangle_count,
            'reduction': result.optimization_percentage
        }})

report = {{
    'total_assets': len(asset_data_list),
    'optimized': success_count,
    'failed': len(asset_data_list) - success_count,
    'results': results
}}

print(json.dumps(report))
unreal.log(f'Optimized {{success_count}}/{{len(asset_data_list)}} assets')
"""
        
        result = self.execute_python_in_editor(script_content=script, timeout=1800)
        
        if result['success']:
            try:
                report = json.loads(result['stdout'])
                return {
                    "success": True,
                    "optimization_report": report
                }
            except json.JSONDecodeError:
                pass
                
        return result


# Example usage
if __name__ == "__main__":
    # Initialize controller
    controller = AIUnrealController(
        engine_path="C:/Program Files/Epic Games/UE_5.6",
        project_path="C:/Users/allen/Desktop/Alexander/Alexander/Alexander.uproject"
    )
    
    # Example: Generate asset report
    print("Generating asset report...")
    result = controller.generate_asset_report()
    
    if result['success']:
        print(f"Report generated successfully: {result['report_path']}")
        print(f"Total assets: {result['report']['total_assets']}")
    else:
        print(f"Failed: {result.get('stderr', 'Unknown error')}")
