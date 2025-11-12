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
        print(f"[INFO] Compilation Error Collector: Starting build...")
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
        success_str = "YES" if results['success'] else "NO"
        print(f"Success: {success_str}")
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
        print(f"[ERROR] {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()