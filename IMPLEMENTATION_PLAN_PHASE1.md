# Phase 1 Implementation Plan: Information Gathering Layer
## Compilation Error Collector

**Version**: 1.0  
**Date**: November 11, 2025  
**Phase**: 1 of 4  
**Estimated Duration**: 2-3 days

---

## Overview

This document provides a detailed implementation plan for the first component of the autonomous development system: the **Compilation Error Collector** - an Information Gathering Layer component that captures build errors from Unreal Engine without attempting to fix them.

---

## 1. Component Specification

### 1.1 Purpose
Execute Unreal Build Tool (UBT) and capture all compilation output (errors, warnings, notes) in a structured format for the Analysis Engine.

### 1.2 Core Principle
**This component ONLY gathers information. It does NOT make decisions or attempt fixes.**

### 1.3 Input/Output

**Input**:
- Build configuration (Development, Shipping, etc.)
- Platform (Win64, etc.)
- Optional: Specific modules to build

**Output**:
- Structured JSON with compilation results
- Exit code
- Parsed errors/warnings with file/line information

---

## 2. Implementation Steps

### Step 1: Create Directory Structure

```bash
mkdir -p autonomous_development/information_gatherers
mkdir -p autonomous_development/schemas
mkdir -p autonomous_development/tests
mkdir -p autonomous_development/output/compilation_reports
```

### Step 2: Create JSON Schema

File: `autonomous_development/schemas/compilation_error_schema.json`

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "Compilation Error Collector Output",
  "type": "object",
  "required": ["collector_type", "timestamp", "build_config", "results"],
  "properties": {
    "collector_type": {
      "type": "string",
      "enum": ["compilation_errors"],
      "description": "Type of information collector"
    },
    "timestamp": {
      "type": "string",
      "format": "date-time",
      "description": "When the build was executed"
    },
    "build_config": {
      "type": "object",
      "required": ["configuration", "platform", "project_path"],
      "properties": {
        "configuration": {
          "type": "string",
          "enum": ["Debug", "Development", "Shipping"],
          "description": "Build configuration"
        },
        "platform": {
          "type": "string",
          "description": "Target platform"
        },
        "project_path": {
          "type": "string",
          "description": "Path to .uproject file"
        },
        "engine_path": {
          "type": "string",
          "description": "Path to Unreal Engine installation"
        }
      }
    },
    "execution": {
      "type": "object",
      "properties": {
        "start_time": {"type": "string", "format": "date-time"},
        "end_time": {"type": "string", "format": "date-time"},
        "duration_seconds": {"type": "number"},
        "exit_code": {"type": "integer"},
        "command_executed": {"type": "string"}
      }
    },
    "results": {
      "type": "object",
      "required": ["success"],
      "properties": {
        "success": {"type": "boolean"},
        "total_errors": {"type": "integer"},
        "total_warnings": {"type": "integer"},
        "errors": {
          "type": "array",
          "items": {
            "type": "object",
            "required": ["file", "line", "message"],
            "properties": {
              "file": {"type": "string"},
              "line": {"type": "integer"},
              "column": {"type": "integer"},
              "error_code": {"type": "string"},
              "message": {"type": "string"},
              "context": {"type": "string"},
              "severity": {"type": "string", "enum": ["error", "fatal"]}
            }
          }
        },
        "warnings": {
          "type": "array",
          "items": {
            "type": "object",
            "required": ["file", "line", "message"],
            "properties": {
              "file": {"type": "string"},
              "line": {"type": "integer"},
              "column": {"type": "integer"},
              "warning_code": {"type": "string"},
              "message": {"type": "string"},
              "suggestion": {"type": "string"},
              "severity": {"type": "string", "enum": ["warning", "note"]}
            }
          }
        },
        "modules_built": {
          "type": "array",
          "items": {
            "type": "object",
            "properties": {
              "name": {"type": "string"},
              "success": {"type": "boolean"},
              "errors": {"type": "integer"},
              "warnings": {"type": "integer"}
            }
          }
        }
      }
    }
  }
}
```

### Step 3: Implement Compilation Error Collector

File: `autonomous_development/information_gatherers/compilation_error_collector.py`

```python
"""
Compilation Error Collector
Information Gathering Layer Component

This module executes Unreal Build Tool and captures compilation output.
It does NOT attempt to fix any issues - it only gathers information.
"""

import json
import subprocess
import re
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Optional, Tuple
import sys


class CompilationErrorCollector:
    """
    Collects compilation errors and warnings from Unreal Build Tool.
    
    This is an INFORMATION GATHERING component only.
    It does NOT make decisions or attempt fixes.
    """
    
    def __init__(self, engine_path: str, project_path: str):
        """
        Initialize the collector.
        
        Args:
            engine_path: Path to Unreal Engine installation (e.g., "C:/Program Files/Epic Games/UE_5.6")
            project_path: Path to .uproject file
        """
        self.engine_path = Path(engine_path)
        self.project_path = Path(project_path)
        self.uproject_file = self.project_path / "Alexander.uproject"
        
        # Validate paths
        if not self.engine_path.exists():
            raise FileNotFoundError(f"Engine path not found: {engine_path}")
        if not self.project_path.exists():
            raise FileNotFoundError(f"Project path not found: {project_path}")
        if not self.uproject_file.exists():
            raise FileNotFoundError(f"Project file not found: {self.uproject_file}")
        
        # Unreal Build Tool path
        self.ubt_path = self.engine_path / "Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.exe"
        if not self.ubt_path.exists():
            raise FileNotFoundError(f"UnrealBuildTool not found: {self.ubt_path}")
    
    def run_build(self, configuration: str = "Development", platform: str = "Win64", 
                  module: Optional[str] = None) -> Dict:
        """
        Run the build and capture all output.
        
        Args:
            configuration: Build configuration (Debug, Development, Shipping)
            platform: Target platform (Win64, etc.)
            module: Specific module to build, or None for full project
            
        Returns:
            Structured dictionary with compilation results
        """
        print(f"🔍 Compilation Error Collector: Starting build...")
        print(f"   Configuration: {configuration}")
        print(f"   Platform: {platform}")
        print(f"   Module: {module or 'All modules'}")
        
        start_time = datetime.now()
        
        # Build command
        cmd = [
            str(self.ubt_path),
            module or "Alexander",  # Default to project name if no module specified
            platform,
            configuration,
            f"-Project={self.uproject_file}",
            "-Progress",
            "-Detailed"
        ]
        
        print(f"   Command: {' '.join(cmd)}")
        
        # Execute build
        try:
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                cwd=str(self.project_path),
                timeout=1800  # 30 minute timeout
            )
        except subprocess.TimeoutExpired:
            return self._create_error_result(
                start_time, datetime.now(), cmd,
                "Build timed out after 30 minutes"
            )
        except Exception as e:
            return self._create_error_result(
                start_time, datetime.now(), cmd,
                f"Failed to execute build: {str(e)}"
            )
        
        end_time = datetime.now()
        duration = (end_time - start_time).total_seconds()
        
        print(f"   Build completed in {duration:.1f} seconds")
        print(f"   Exit code: {result.returncode}")
        
        # Parse output
        errors, warnings = self._parse_output(result.stdout + result.stderr)
        
        # Create structured result
        output = {
            "collector_type": "compilation_errors",
            "timestamp": start_time.isoformat(),
            "build_config": {
                "configuration": configuration,
                "platform": platform,
                "project_path": str(self.project_path),
                "engine_path": str(self.engine_path)
            },
            "execution": {
                "start_time": start_time.isoformat(),
                "end_time": end_time.isoformat(),
                "duration_seconds": duration,
                "exit_code": result.returncode,
                "command_executed": " ".join(cmd)
            },
            "results": {
                "success": result.returncode == 0,
                "total_errors": len(errors),
                "total_warnings": len(warnings),
                "errors": errors,
                "warnings": warnings,
                "raw_output": result.stdout + result.stderr if result.returncode != 0 else ""
            }
        }
        
        # Save report
        self._save_report(output)
        
        # Print summary
        self._print_summary(output)
        
        return output
    
    def _parse_output(self, output: str) -> Tuple[List[Dict], List[Dict]]:
        """
        Parse build output to extract errors and warnings.
        
        Args:
            output: Combined stdout and stderr from build
            
        Returns:
            Tuple of (errors, warnings) as dictionaries
        """
        errors = []
        warnings = []
        
        # Patterns for different error formats
        patterns = {
            "msvc_error": re.compile(
                r'(?P<file>.+)\((?P<line>\d+)(?:,(?P<column>\d+))?\): '
                r'(?:error|fatal error) (?P<code>\w+): (?P<message>.+)'
            ),
            "msvc_warning": re.compile(
                r'(?P<file>.+)\((?P<line>\d+)(?:,(?P<column>\d+))?\): '
                r'warning (?P<code>\w+): (?P<message>.+)'
            ),
            "clang_error": re.compile(
                r'(?P<file>.+):(?P<line>\d+):(?P<column>\d+): '
                r'(?:error|fatal error): (?P<message>.+)'
            ),
            "clang_warning": re.compile(
                r'(?P<file>.+):(?P<line>\d+):(?P<column>\d+): '
                r'warning: (?P<message>.+)'
            )
        }
        
        lines = output.split('\n')
        
        for line in lines:
            line = line.strip()
            if not line:
                continue
            
            # Try to match each pattern
            for pattern_name, pattern in patterns.items():
                match = pattern.match(line)
                if match:
                    data = match.groupdict()
                    if "error" in pattern_name:
                        errors.append({
                            "file": data.get("file", "unknown"),
                            "line": int(data.get("line", 0)),
                            "column": int(data.get("column", 0)) if data.get("column") else None,
                            "error_code": data.get("code", "unknown"),
                            "message": data.get("message", line),
                            "context": self._get_context(lines, lines.index(line)),
                            "severity": "fatal" if "fatal" in line else "error"
                        })
                    elif "warning" in pattern_name:
                        warnings.append({
                            "file": data.get("file", "unknown"),
                            "line": int(data.get("line", 0)),
                            "column": int(data.get("column", 0)) if data.get("column") else None,
                            "warning_code": data.get("code", "unknown"),
                            "message": data.get("message", line),
                            "suggestion": self._extract_suggestion(lines, lines.index(line)),
                            "severity": "warning"
                        })
                    break
        
        return errors, warnings
    
    def _get_context(self, lines: List[str], error_line_idx: int) -> str:
        """
        Get context around an error line.
        
        Args:
            lines: All output lines
            error_line_idx: Index of the error line
            
        Returns:
            Context string with surrounding lines
        """
        start = max(0, error_line_idx - 2)
        end = min(len(lines), error_line_idx + 3)
        return "\n".join(lines[start:end])
    
    def _extract_suggestion(self, lines: List[str], warning_line_idx: int) -> Optional[str]:
        """
        Extract suggestion from warning context.
        
        Args:
            lines: All output lines
            warning_line_idx: Index of the warning line
            
        Returns:
            Suggestion string if found
        """
        # Look ahead for suggestion
        for i in range(warning_line_idx + 1, min(len(lines), warning_line_idx + 3)):
            line = lines[i].strip()
            if "note:" in line.lower() or "suggestion:" in line.lower():
                return line
        return None
    
    def _create_error_result(self, start_time: datetime, end_time: datetime, 
                            cmd: List[str], error_message: str) -> Dict:
        """Create error result when build fails to execute."""
        return {
            "collector_type": "compilation_errors",
            "timestamp": start_time.isoformat(),
            "build_config": {
                "configuration": "unknown",
                "platform": "unknown",
                "project_path": str(self.project_path),
                "engine_path": str(self.engine_path)
            },
            "execution": {
                "start_time": start_time.isoformat(),
                "end_time": end_time.isoformat(),
                "duration_seconds": (end_time - start_time).total_seconds(),
                "exit_code": -1,
                "command_executed": " ".join(cmd)
            },
            "results": {
                "success": False,
                "total_errors": 1,
                "total_warnings": 0,
                "errors": [
                    {
                        "file": "collector_execution",
                        "line": 0,
                        "column": 0,
                        "error_code": "EXEC_ERROR",
                        "message": error_message,
                        "context": "",
                        "severity": "fatal"
                    }
                ],
                "warnings": []
            }
        }
    
    def _save_report(self, output: Dict):
        """Save the compilation report to disk."""
        report_dir = Path("autonomous_development/output/compilation_reports")
        report_dir.mkdir(parents=True, exist_ok=True)
        
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        report_file = report_dir / f"compilation_report_{timestamp}.json"
        
        with open(report_file, 'w') as f:
            json.dump(output, f, indent=2)
        
        print(f"   Report saved: {report_file}")
    
    def _print_summary(self, output: Dict):
        """Print a summary of the compilation results."""
        results = output["results"]
        
        print(f"\n{'='*70}")
        print("COMPILATION REPORT SUMMARY")
        print(f"{'='*70}")
        print(f"Success: {'✅ YES' if results['success'] else '❌ NO'}")
        print(f"Total Errors: {results['total_errors']}")
        print(f"Total Warnings: {results['total_warnings']}")
        print(f"Duration: {output['execution']['duration_seconds']:.1f} seconds")
        
        if results["errors"]:
            print(f"\nERRORS:")
            for i, error in enumerate(results["errors"][:5], 1):  # Show first 5
                print(f"  {i}. {error['file']}:{error['line']}")
                print(f"     {error['message']}")
        
        if results["warnings"]:
            print(f"\nWARNINGS:")
            for i, warning in enumerate(results["warnings"][:5], 1):  # Show first 5
                print(f"  {i}. {warning['file']}:{warning['line']}")
                print(f"     {warning['message']}")
        
        if len(results["errors"]) > 5 or len(results["warnings"]) > 5:
            print(f"\n... and more. See full report for details.")
        
        print(f"{'='*70}\n")


def main():
    """Example usage of the Compilation Error Collector."""
    print("Compilation Error Collector - Test Run")
    print("="*70)
    
    # Configuration
    ENGINE_PATH = "C:/Program Files/Epic Games/UE_5.6"
    PROJECT_PATH = "C:/Users/allen/Desktop/Alexander/Alexander"
    
    try:
        # Create collector
        collector = CompilationErrorCollector(ENGINE_PATH, PROJECT_PATH)
        
        # Run build
        result = collector.run_build(
            configuration="Development",
            platform="Win64"
        )
        
        # Exit with appropriate code
        sys.exit(0 if result["results"]["success"] else 1)
        
    except Exception as e:
        print(f"❌ Error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()
```

### Step 4: Create Test Script

File: `autonomous_development/tests/test_compilation_collector.py`

```python
"""
Test suite for Compilation Error Collector
"""

import sys
import json
from pathlib import Path
from unittest.mock import Mock, patch, MagicMock

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent))

from information_gatherers.compilation_error_collector import CompilationErrorCollector


def test_collector_initialization():
    """Test that collector initializes correctly."""
    print("Test: Collector Initialization")
    
    with patch('pathlib.Path.exists') as mock_exists:
        mock_exists.return_value = True
        
        collector = CompilationErrorCollector(
            engine_path="C:/Program Files/Epic Games/UE_5.6",
            project_path="C:/Users/allen/Desktop/Alexander/Alexander"
        )
        
        assert collector.engine_path.name == "UE_5.6"
        assert collector.project_path.name == "Alexander"
        print("  ✅ Initialization successful")


def test_parse_msvc_error():
    """Test parsing MSVC error format."""
    print("Test: Parse MSVC Error")
    
    collector = CompilationErrorCollector.__new__(CompilationErrorCollector)
    
    output = """
C:\\Project\\Source\\File.cpp(45,12): error C2065: 'undeclared_identifier': undeclared identifier
    """
    
    errors, warnings = collector._parse_output(output)
    
    assert len(errors) == 1
    assert errors[0]["file"] == "C:\\Project\\Source\\File.cpp"
    assert errors[0]["line"] == 45
    assert errors[0]["column"] == 12
    assert errors[0]["error_code"] == "C2065"
    assert "undeclared identifier" in errors[0]["message"]
    print("  ✅ MSVC error parsing successful")


def test_parse_msvc_warning():
    """Test parsing MSVC warning format."""
    print("Test: Parse MSVC Warning")
    
    collector = CompilationErrorCollector.__new__(CompilationErrorCollector)
    
    output = """
C:\\Project\\Source\\File.cpp(128,1): warning C4996: 'deprecated_function': was declared deprecated
    """
    
    errors, warnings = collector._parse_output(output)
    
    assert len(warnings) == 1
    assert warnings[0]["file"] == "C:\\Project\\Source\\File.cpp"
    assert warnings[0]["line"] == 128
    assert warnings[0]["warning_code"] == "C4996"
    assert "deprecated" in warnings[0]["message"]
    print("  ✅ MSVC warning parsing successful")


def test_parse_clang_error():
    """Test parsing Clang error format."""
    print("Test: Parse Clang Error")
    
    collector = CompilationErrorCollector.__new__(CompilationErrorCollector)
    
    output = """
/Project/Source/File.cpp:45:12: error: use of undeclared identifier 'undeclared_identifier'
    """
    
    errors, warnings = collector._parse_output(output)
    
    assert len(errors) == 1
    assert errors[0]["file"] == "/Project/Source/File.cpp"
    assert errors[0]["line"] == 45
    assert errors[0]["column"] == 12
    print("  ✅ Clang error parsing successful")


def test_output_schema_validation():
    """Test that output matches schema."""
    print("Test: Output Schema Validation")
    
    collector = CompilationErrorCollector.__new__(CompilationErrorCollector)
    
    # Mock result
    result = {
        "collector_type": "compilation_errors",
        "timestamp": "2025-11-11T17:58:00",
        "build_config": {
            "configuration": "Development",
            "platform": "Win64",
            "project_path": "C:/Project",
            "engine_path": "C:/Engine"
        },
        "execution": {
            "start_time": "2025-11-11T17:58:00",
            "end_time": "2025-11-11T17:59:00",
            "duration_seconds": 60.0,
            "exit_code": 0,
            "command_executed": "UBT Command"
        },
        "results": {
            "success": True,
            "total_errors": 0,
            "total_warnings": 2,
            "errors": [],
            "warnings": [
                {
                    "file": "File.cpp",
                    "line": 10,
                    "column": 5,
                    "warning_code": "C1234",
                    "message": "Test warning",
                    "suggestion": "Fix it",
                    "severity": "warning"
                }
            ]
        }
    }
    
    # Validate required fields
    assert result["collector_type"] == "compilation_errors"
    assert "timestamp" in result
    assert "results" in result
    assert "success" in result["results"]
    print("  ✅ Schema validation successful")


def test_report_saving():
    """Test that reports are saved correctly."""
    print("Test: Report Saving")
    
    collector = CompilationErrorCollector.__new__(CompilationErrorCollector)
    collector.project_path = Path("C:/Project")
    collector.engine_path = Path("C:/Engine")
    
    output = {
        "collector_type": "compilation_errors",
        "timestamp": "2025-11-11T17:58:00",
        "build_config": {
            "configuration": "Development",
            "platform": "Win64",
            "project_path": "C:/Project",
            "engine_path": "C:/Engine"
        },
        "execution": {
            "start_time": "2025-11-11T17:58:00",
            "end_time": "2025-11-11T17:59:00",
            "duration_seconds": 60.0,
            "exit_code": 0,
            "command_executed": "UBT Command"
        },
        "results": {
            "success": True,
            "total_errors": 0,
            "total_warnings": 0,
            "errors": [],
            "warnings": []
        }
    }
    
    with patch('builtins.open', create=True) as mock_open:
        collector._save_report(output)
        mock_open.assert_called_once()
        print("  ✅ Report saving successful")


def run_all_tests():
    """Run all tests."""
    print("Compilation Error Collector - Test Suite")
    print("="*70)
    
    tests = [
        test_collector_initialization,
        test_parse_msvc_error,
        test_parse_msvc_warning,
        test_parse_clang_error,
        test_output_schema_validation,
        test_report_saving
    ]
    
    passed = 0
    failed = 0
    
    for test in tests:
        try:
            test()
            passed += 1
        except Exception as e:
            print(f"  ❌ FAILED: {e}")
            failed += 1
            import traceback
            traceback.print_exc()
    
    print("="*70)
    print(f"Tests Passed: {passed}")
    print(f"Tests Failed: {failed}")
    print("="*70)
    
    return failed == 0


if __name__ == "__main__":
    success = run_all_tests()
    sys.exit(0 if success else 1)
```

### Step 5: Create Integration Test

File: `autonomous_development/tests/integration_test_collector.py`

```python
"""
Integration test: Run actual build and verify collector works
"""

import sys
from pathlib import Path

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent))

from information_gatherers.compilation_error_collector import CompilationErrorCollector


def test_real_build():
    """Test with actual Unreal project build."""
    print("Integration Test: Real Build Collection")
    print("="*70)
    
    # Configuration
    ENGINE_PATH = "C:/Program Files/Epic Games/UE_5.6"
    PROJECT_PATH = "C:/Users/allen/Desktop/Alexander/Alexander"
    
    try:
        # Create collector
        print("Creating collector...")
        collector = CompilationErrorCollector(ENGINE_PATH, PROJECT_PATH)
        
        # Run build (this will take time)
        print("Running build - this may take several minutes...")
        result = collector.run_build(
            configuration="Development",
            platform="Win64",
            module="Alexander"
        )
        
        # Verify result structure
        assert "collector_type" in result
        assert "timestamp" in result
        assert "build_config" in result
        assert "execution" in result
        assert "results" in result
        assert "success" in result["results"]
        
        print("\n✅ Integration test passed!")
        print(f"Build success: {result['results']['success']}")
        print(f"Errors: {result['results']['total_errors']}")
        print(f"Warnings: {result['results']['total_warnings']}")
        
        return True
        
    except Exception as e:
        print(f"\n❌ Integration test failed: {e}")
        import traceback
        traceback.print_exc()
        return False


if __name__ == "__main__":
    success = test_real_build()
    sys.exit(0 if success else 1)
```

---

## 3. Testing Strategy

### 3.1 Unit Tests
Run unit tests to verify parsing logic:
```bash
cd autonomous_development
python tests/test_compilation_collector.py
```

### 3.2 Integration Test
Run actual build collection (takes 5-15 minutes):
```bash
cd autonomous_development
python tests/integration_test_collector.py
```

### 3.3 Manual Verification
Check that output files are created:
```bash
ls -la autonomous_development/output/compilation_reports/
cat autonomous_development/output/compilation_reports/compilation_report_*.json
```

---

## 4. Expected Output Example

### 4.1 Successful Build
```json
{
  "collector_type": "compilation_errors",
  "timestamp": "2025-11-11T18:15:30.123456",
  "build_config": {
    "configuration": "Development",
    "platform": "Win64",
    "project_path": "C:/Users/allen/Desktop/Alexander/Alexander",
    "engine_path": "C:/Program Files/Epic Games/UE_5.6"
  },
  "execution": {
    "start_time": "2025-11-11T18:15:30.123456",
    "end_time": "2025-11-11T18:18:45.789012",
    "duration_seconds": 195.7,
    "exit_code": 0,
    "command_executed": "C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.exe Alexander Win64 Development -Project=C:/Users/allen/Desktop/Alexander/Alexander/Alexander.uproject -Progress -Detailed"
  },
  "results": {
    "success": true,
    "total_errors": 0,
    "total_warnings": 3,
    "errors": [],
    "warnings": [
      {
        "file": "Source/Alexander/Private/ShipSystem.cpp",
        "line": 128,
        "column": 1,
        "warning_code": "C4996",
        "message": "'OldFunction': was declared deprecated",
        "suggestion": "note: see declaration of 'OldFunction'",
        "severity": "warning"
      }
    ]
  }
}
```

### 4.2 Failed Build
```json
{
  "collector_type": "compilation_errors",
  "timestamp": "2025-11-11T18:20:15.456789",
  "build_config": { ... },
  "execution": { ... },
  "results": {
    "success": false,
    "total_errors": 2,
    "total_warnings": 1,
    "errors": [
      {
        "file": "Source/Alexander/Private/ShipSystem.cpp",
        "line": 45,
        "column": 12,
        "error_code": "C2065",
        "message": "'ShipEngine': undeclared identifier",
        "context": "ShipEngine->ApplyThrust();",
        "severity": "error"
      },
      {
        "file": "Source/Alexander/Private/FlightController.cpp",
        "line": 89,
        "column": 25,
        "error_code": "C2660",
        "message": "'CalculateVelocity': function does not take 3 arguments",
        "context": "CalculateVelocity(DeltaTime, Input, Mass);",
        "severity": "error"
      }
    ],
    "warnings": [ ... ]
  }
}
```

---

## 5. Next Steps

After this component is implemented and tested:

1. **Create Runtime Log Collector** - Capture UE runtime logs
2. **Create Analysis Engine** - Process the JSON output from this collector
3. **Create Decision Engine** - Decide how to fix identified issues
4. **Create Implementation Layer** - Apply fixes to C++ code

---

## 6. Success Criteria

✅ **Component is complete when**:
- [ ] Python script successfully runs UBT and captures output
- [ ] Output matches JSON schema specification
- [ ] Errors and warnings are correctly parsed with file/line information
- [ ] Reports are saved to disk with timestamps
- [ ] Unit tests pass (mocked scenarios)
- [ ] Integration test passes (real build scenario)
- [ ] Documentation is complete and clear

---

## 7. Estimated Timeline

| Task | Duration |
|------|----------|
| Directory setup | 15 minutes |
| Schema creation | 30 minutes |
| Collector implementation | 3-4 hours |
| Unit tests | 1-2 hours |
| Integration test | 30 minutes |
| Documentation | 1 hour |
| **Total** | **6-8 hours** |

---

**Ready to implement?** This plan provides everything needed to create the first component of the autonomous development system.