#!/usr/bin/env python3
"""
Fix Name Conflicts Script
Renames conflicting classes, structs, and enums to use Alexander prefix
"""

import re
import sys
from pathlib import Path

def replace_in_file(file_path, replacements):
    """Perform replacements in a file."""
    print(f"Processing {file_path}...")
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    original_content = content
    changes_made = 0
    
    for old_name, new_name in replacements.items():
        # Replace class declarations (including A prefix)
        if old_name.startswith('A'):
            content = re.sub(rf'class {old_name}\\b', f'class {new_name}', content)
        # Replace struct declarations
        content = re.sub(rf'struct {old_name}\\b', f'struct {new_name}', content)
        # Replace enum declarations
        content = re.sub(rf'enum {old_name}\\b', f'enum {new_name}', content)
        # Replace enum class declarations
        content = re.sub(rf'enum class {old_name}\\b', f'enum class {new_name}', content)
        # Replace UENUM declarations
        content = re.sub(rf'UENUM\\([^)]*\\)\\s*enum class {old_name}\\b',
                        lambda m: m.group(0).replace(f'enum class {old_name}', f'enum class {new_name}'), content)
        
        # For class names, also need to rename the file if it matches
        if old_name.startswith('A') and file_path.name == f'{old_name[1:]}.h':
            new_file_path = file_path.parent / f'{new_name[1:]}.h'
            print(f"  [INFO] Should rename file {file_path} to {new_file_path}")
    
    if content != original_content:
        with open(file_path, 'w') as f:
            f.write(content)
        print(f"  [OK] Fixed name conflicts in {file_path}")
        return True
    else:
        print(f"  - No changes needed for {file_path}")
        return False

def main():
    """Main function to fix all name conflicts."""
    # Define replacements - using base names without prefixes for classes
    replacements = {
        'IrrigationSystem': 'AlexanderIrrigationSystem',  # For AIrrigationSystem -> AAlexanderIrrigationSystem
        'PerformanceMetrics': 'AlexanderPerformanceMetrics',  # For FPerformanceMetrics -> FAlexanderPerformanceMetrics
        'HarvestResult': 'AlexanderHarvestResult',  # For FHarvestResult -> FAlexanderHarvestResult
        'MissionDifficulty': 'AlexanderMissionDifficulty',  # For EMissionDifficulty -> EAlexanderMissionDifficulty
        'MissionStatus': 'AlexanderMissionStatus',  # For EMissionStatus -> EAlexanderMissionStatus
        'TestStatus': 'AlexanderTestStatus',  # For ETestStatus -> EAlexanderTestStatus
        'SpatialQueryResult': 'AlexanderSpatialQueryResult',  # For FSpatialQueryResult -> FAlexanderSpatialQueryResult
        'ResourceDeposit': 'AlexanderResourceDeposit'  # For FResourceDeposit -> FAlexanderResourceDeposit
    }
    
    # Files that need to be processed
    files_to_process = [
        # AIrrigationSystem conflicts
        "Source/Alexander/Public/IrrigationSystem.h",
        "Source/Alexander/Public/FarmingInfrastructure.h",
        
        # FPerformanceMetrics conflicts
        "Source/Alexander/Public/PerformanceSelfMonitor.h",
        "Source/Alexander/Public/PerformanceMonitor.h",
        "Source/Alexander/Public/PerformanceTestingSystem.h",
        
        # FHarvestResult conflicts
        "Source/Alexander/Public/PlanetaryFarmingSystem.h",
        "Source/Alexander/Public/FarmPlot.h",
        
        # EMissionDifficulty/EMissionStatus conflicts
        "Source/Alexander/Public/TradeMissionSystem.h",
        "Source/Alexander/Public/MissionBoardComponent.h",
        
        # ETestStatus conflicts
        "Source/Alexander/Public/Testing/TestDataStructures.h",
        "Source/Alexander/Public/TestScenarioManager.h",
        
        # FSpatialQueryResult conflicts
        "Source/Alexander/Public/StarSystemManager.h",
        "Source/Alexander/Public/SpatialPartitioningOctree.h",
        
        # FResourceDeposit conflicts
        "Source/Alexander/Planetary/PlanetaryResourcesComponent.h",
        "Source/Alexander/Public/ResourceGatheringSystem.h"
    ]
    
    print("=" * 70)
    print("Name Conflict Fix Script")
    print("=" * 70)
    
    files_modified = 0
    
    for file_path in files_to_process:
        full_path = Path(file_path)
        if full_path.exists():
            if replace_in_file(full_path, replacements):
                files_modified += 1
        else:
            print(f"WARNING: File not found: {file_path}")
    
    print("=" * 70)
    print(f"Summary: Modified {files_modified} files")
    print("=" * 70)
    
    return 0 if files_modified > 0 else 1

if __name__ == "__main__":
    sys.exit(main())