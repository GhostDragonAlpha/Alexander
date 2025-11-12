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
        print("  [OK] Initialization successful")


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
    print("  [OK] MSVC error parsing successful")


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
    print("  [OK] MSVC warning parsing successful")


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
    print("  [OK] Clang error parsing successful")


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
    print("  [OK] Schema validation successful")


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
        print("  [OK] Report saving successful")


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
            print(f"  [FAIL] {e}")
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