#!/usr/bin/env python3
"""
Comprehensive Name Conflict Fix
Handles all name conflict replacements systematically
"""

import re
import sys
from pathlib import Path

def fix_all_name_conflicts():
    """Fix all name conflicts in one comprehensive pass."""
    
    # Define all replacements
    replacements = {
        # Class names
        'AIrrigationSystem': 'AAlexanderIrrigationSystem',
        
        # Struct names
        'FPerformanceMetrics': 'FAlexanderPerformanceMetrics',
        'FHarvestResult': 'FAlexanderHarvestResult',
        'FSpatialQueryResult': 'FAlexanderSpatialQueryResult',
        'FResourceDeposit': 'FAlexanderResourceDeposit',
        
        # Enum names
        'EMissionDifficulty': 'EAlexanderMissionDifficulty',
        'EMissionStatus': 'EAlexanderMissionStatus',
        'ETestStatus': 'EAlexanderTestStatus'
    }
    
    # Get all header files in Source directory
    source_dir = Path("Source/Alexander")
    header_files = list(source_dir.rglob("*.h"))
    
    print("=" * 70)
    print("Comprehensive Name Conflict Fix")
    print("=" * 70)
    print(f"Found {len(header_files)} header files to process")
    
    files_modified = 0
    
    for header_file in header_files:
        try:
            with open(header_file, 'r', encoding='utf-8') as f:
                content = f.read()
            
            original_content = content
            file_modified = False
            
            # Apply all replacements
            for old_name, new_name in replacements.items():
                # Replace class declarations
                if old_name.startswith('A'):
                    content = re.sub(rf'class {old_name}\\b', f'class {new_name}', content)
                # Replace struct declarations  
                elif old_name.startswith('F'):
                    content = re.sub(rf'struct {old_name}\\b', f'struct {new_name}', content)
                # Replace enum declarations
                elif old_name.startswith('E'):
                    content = re.sub(rf'enum class {old_name}\\b', f'enum class {new_name}', content)
                
                # Also replace in UPROPERTY and other references
                content = re.sub(rf'\\b{old_name}\\b', new_name, content)
            
            if content != original_content:
                with open(header_file, 'w', encoding='utf-8') as f:
                    f.write(content)
                print(f"  [OK] Fixed {header_file}")
                files_modified += 1
                file_modified = True
            
            # Special handling for IrrigationSystem.h - rename the file itself
            if header_file.name == 'IrrigationSystem.h' and 'AAlexanderIrrigationSystem' in content:
                new_path = header_file.parent / 'AlexanderIrrigationSystem.h'
                header_file.rename(new_path)
                print(f"  [OK] Renamed {header_file} to {new_path}")
                files_modified += 1
                
        except Exception as e:
            print(f"  [ERROR] Failed to process {header_file}: {e}")
    
    print("=" * 70)
    print(f"Summary: Modified {files_modified} files")
    print("=" * 70)
    
    return 0 if files_modified > 0 else 1

if __name__ == "__main__":
    sys.exit(fix_all_name_conflicts())