#!/usr/bin/env python3
"""
Parse Unreal Engine compilation output and structure it according to the compilation error schema.
"""

import json
import re
import sys
from datetime import datetime
from pathlib import Path

def parse_compilation_output(output_text):
    """
    Parse Unreal Engine compilation output and extract errors and warnings.
    """
    results = {
        "success": False,
        "total_errors": 0,
        "total_warnings": 0,
        "errors": [],
        "warnings": [],
        "modules_built": []
    }
    
    # Regular expressions for parsing
    error_patterns = [
        # Fatal errors
        r'(?P<file>.+?)\((?P<line>\d+),(?P<column>\d+)\):\s+fatal error (?P<code>C\d+):\s+(?P<message>.+?)(?=\n|$)',
        # Regular errors
        r'(?P<file>.+?)\((?P<line>\d+),(?P<column>\d+)\):\s+error (?P<code>C\d+):\s+(?P<message>.+?)(?=\n|$)',
        # Error without code
        r'(?P<file>.+?)\((?P<line>\d+),(?P<column>\d+)\):\s+error:\s+(?P<message>.+?)(?=\n|$)',
    ]
    
    warning_patterns = [
        # Warnings with code
        r'(?P<file>.+?)\((?P<line>\d+),(?P<column>\d+)\):\s+warning (?P<code>C\d+):\s+(?P<message>.+?)(?=\n|$)',
        # Warnings without code
        r'(?P<file>.+?)\((?P<line>\d+),(?P<column>\d+)\):\s+warning:\s+(?P<message>.+?)(?=\n|$)',
    ]
    
    # Parse errors
    for pattern in error_patterns:
        matches = re.finditer(pattern, output_text, re.MULTILINE)
        for match in matches:
            error_data = {
                "file": match.group('file').strip(),
                "line": int(match.group('line')),
                "column": int(match.group('column')),
                "message": match.group('message').strip(),
                "severity": "error"
            }
            if 'code' in match.groupdict():
                error_data["error_code"] = match.group('code')
            
            # Extract context (surrounding lines)
            context_start = max(0, match.start() - 200)
            context_end = min(len(output_text), match.end() + 200)
            error_data["context"] = output_text[context_start:context_end].strip()
            
            results["errors"].append(error_data)
            results["total_errors"] += 1
    
    # Parse warnings
    for pattern in warning_patterns:
        matches = re.finditer(pattern, output_text, re.MULTILINE)
        for match in matches:
            warning_data = {
                "file": match.group('file').strip(),
                "line": int(match.group('line')),
                "column": int(match.group('column')),
                "message": match.group('message').strip(),
                "severity": "warning"
            }
            if 'code' in match.groupdict():
                warning_data["warning_code"] = match.group('code')
            
            results["warnings"].append(warning_data)
            results["total_warnings"] += 1
    
    return results

def create_compilation_report(output_text, exit_code):
    """
    Create a compilation report following the schema.
    """
    # Parse the compilation results
    results = parse_compilation_output(output_text)
    results["success"] = (exit_code == 0)
    
    # Extract build configuration from command
    build_config = {
        "configuration": "Development",
        "platform": "Win64",
        "project_path": "c:\\Users\\allen\\Desktop\\Alexander\\Alexander\\Alexander.uproject",
        "engine_path": "C:\\Program Files\\Epic Games\\UE_5.6"
    }
    
    # Create execution info
    execution = {
        "start_time": datetime.utcnow().isoformat() + "Z",
        "end_time": datetime.utcnow().isoformat() + "Z",  # Approximate
        "duration_seconds": 15.61,  # From build output
        "exit_code": exit_code,
        "command_executed": "\"C:\\Program Files\\Epic Games\\UE_5.6\\Engine\\Build\\BatchFiles\\Build.bat\" AlexanderEditor Win64 Development -Project=\"c:\\Users\\allen\\Desktop\\Alexander\\Alexander\\Alexander.uproject\" -WaitMutex -FromMsBuild"
    }
    
    # Create the final report
    report = {
        "collector_type": "compilation_errors",
        "timestamp": datetime.utcnow().isoformat() + "Z",
        "build_config": build_config,
        "execution": execution,
        "results": results
    }
    
    return report

def main():
    # Read the compilation output from stdin or file
    if len(sys.argv) > 1:
        # Read from file
        with open(sys.argv[1], 'r', encoding='utf-8') as f:
            output_text = f.read()
    else:
        # Read from stdin
        output_text = sys.stdin.read()
    
    # Create the report (exit code 6 indicates compilation failure)
    report = create_compilation_report(output_text, exit_code=6)
    
    # Save to JSON file
    output_file = "compilation_errors_report.json"
    with open(output_file, 'w', encoding='utf-8') as f:
        json.dump(report, f, indent=2, ensure_ascii=False)
    
    print(f"Compilation report saved to: {output_file}")
    print(f"Total errors: {report['results']['total_errors']}")
    print(f"Total warnings: {report['results']['total_warnings']}")
    
    # Print summary of error types
    error_codes = {}
    for error in report['results']['errors']:
        code = error.get('error_code', 'UNKNOWN')
        error_codes[code] = error_codes.get(code, 0) + 1
    
    print("\nError breakdown by code:")
    for code, count in sorted(error_codes.items()):
        print(f"  {code}: {count}")

if __name__ == "__main__":
    main()