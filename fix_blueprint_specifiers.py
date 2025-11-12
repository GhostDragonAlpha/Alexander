#!/usr/bin/env python3
"""
Fix Blueprint Specifiers Script
Removes BlueprintReadOnly/BlueprintReadWrite from private members in header files
"""

import re
import sys
from pathlib import Path

def fix_header_file(file_path):
    """Fix Blueprint specifier issues in a header file."""
    print(f"Processing {file_path}...")
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    original_content = content
    
    # Track if we're in a private section
    in_private_section = False
    lines = content.split('\n')
    fixed_lines = []
    
    i = 0
    while i < len(lines):
        line = lines[i]
        
        # Check for access specifiers
        if line.strip().startswith('public:'):
            in_private_section = False
        elif line.strip().startswith('protected:'):
            in_private_section = False
        elif line.strip().startswith('private:'):
            in_private_section = True
        
        # If we're in private section, remove Blueprint specifiers from UPROPERTY
        if in_private_section and 'UPROPERTY(' in line:
            # Remove BlueprintReadOnly and BlueprintReadWrite
            line = re.sub(r',?\s*BlueprintReadOnly', '', line)
            line = re.sub(r',?\s*BlueprintReadWrite', '', line)
            
            # Clean up any double commas or leading/trailing commas
            line = re.sub(r'\(\s*,', '(', line)
            line = re.sub(r',\s*\)', ')', line)
            line = re.sub(r',,', ',', line)
        
        # Handle UENUM scope issue specifically for OrbitalMechanicsManager.h
        if 'OrbitalMechanicsManager.h' in str(file_path) and 'UENUM' in line and 'class' in line:
            # Move UENUM to global scope by removing it from class scope
            # This is a simplified fix - the UENUM should be moved outside the class
            print(f"  WARNING: UENUM in class scope detected. Manual review needed for {file_path}")
        
        fixed_lines.append(line)
        i += 1
    
    # Join lines back together
    fixed_content = '\n'.join(fixed_lines)
    
    # Check if we made changes
    if fixed_content != original_content:
        # Write the fixed content back
        with open(file_path, 'w') as f:
            f.write(fixed_content)
        print(f"  [OK] Fixed Blueprint specifiers in {file_path}")
        return True
    else:
        print(f"  - No changes needed for {file_path}")
        return False

def main():
    """Main function to fix all header files."""
    # List of files to fix based on compilation errors
    files_to_fix = [
        "Source/Alexander/Public/PerformanceMonitor.h",
        "Source/Alexander/Public/GameLauncher.h",
        "Source/Alexander/Public/OrbitalMechanicsManager.h",
        "Source/Alexander/Public/TestScenarioManager.h",
        "Source/Alexander/Public/VisualAnalyzer.h",
        "Source/Alexander/Public/ErrorDetector.h"
    ]
    
    print("=" * 70)
    print("Blueprint Specifier Fix Script")
    print("=" * 70)
    
    files_modified = 0
    
    for file_path in files_to_fix:
        full_path = Path(file_path)
        if full_path.exists():
            if fix_header_file(full_path):
                files_modified += 1
        else:
            print(f"ERROR: File not found: {file_path}")
    
    print("=" * 70)
    print(f"Summary: Modified {files_modified} files")
    print("=" * 70)
    
    # Special handling for OrbitalMechanicsManager.h UENUM issue
    orbital_file = Path("Source/Alexander/Public/OrbitalMechanicsManager.h")
    if orbital_file.exists():
        print("\nSpecial handling needed for OrbitalMechanicsManager.h:")
        print("- UENUM is incorrectly placed inside class scope")
        print("- Should be moved to global scope (outside the class)")
        print("- Please review this file manually")
    
    return 0 if files_modified > 0 else 1

if __name__ == "__main__":
    sys.exit(main())