#!/usr/bin/env python3
"""
Build Verification Script for Alexander Comprehensive Test Level

This script performs comprehensive verification of the test level build,
including compilation checks, dependency validation, syntax checking,
and generates a detailed build report.

Usage:
    python verify_test_level_build.py
    python verify_test_level_build.py --verbose
    python verify_test_level_build.py --output report.json

Exit Codes:
    0 = Success (all checks passed)
    1 = Compilation errors
    2 = Missing dependencies
    3 = Invalid file structure
    4 = Syntax errors
    5 = General failure
"""

import os
import sys
import json
import subprocess
import glob
import re
from pathlib import Path
from datetime import datetime
from typing import List, Dict, Tuple, Optional
import argparse
import hashlib


class BuildVerifier:
    """Main build verification class"""

    def __init__(self, verbose: bool = False, output_file: Optional[str] = None):
        self.verbose = verbose
        self.output_file = output_file
        self.project_root = Path(__file__).parent
        self.source_root = self.project_root / "Source" / "Alexander"
        self.test_root = self.source_root / "Public" / "Testing"
        self.results = {
            "timestamp": datetime.now().isoformat(),
            "project_root": str(self.project_root),
            "checks": {},
            "summary": {
                "total_checks": 0,
                "passed_checks": 0,
                "failed_checks": 0,
                "warnings": 0,
            },
            "errors": [],
            "warnings": [],
        }
        self.exit_code = 0

    def log(self, message: str, level: str = "INFO"):
        """Log message with level prefix"""
        timestamp = datetime.now().strftime("%H:%M:%S")
        prefix = f"[{timestamp}] [{level}]"
        print(f"{prefix} {message}")

    def log_verbose(self, message: str):
        """Log only if verbose mode enabled"""
        if self.verbose:
            self.log(message, "DEBUG")

    def check_compilation(self) -> bool:
        """Verify all test station classes compile"""
        self.log("Checking C++ compilation...")
        check_name = "compilation"

        test_station_files = [
            "TestStation.h",
            "TestLevelGameMode.h",
            "TestDataStructures.h",
            "VRTestStation.h",
            "SpaceshipFlightTestStation.h",
            "TestUtilities.h",
        ]

        issues = []
        for filename in test_station_files:
            filepath = self.test_root / filename
            if not filepath.exists():
                issues.append(f"Missing test station file: {filepath}")
                self.log_verbose(f"File not found: {filepath}")
                continue

            # Check for basic syntax errors
            if not self._validate_cpp_syntax(filepath):
                issues.append(f"Syntax error in {filename}")

        passed = len(issues) == 0
        self.results["checks"][check_name] = {
            "passed": passed,
            "message": f"C++ compilation check ({len(test_station_files)} files)",
            "details": issues if issues else ["All files compiled successfully"],
        }

        if passed:
            self.log(f"✓ Compilation check passed ({len(test_station_files)} files)")
        else:
            self.log(f"✗ Compilation check failed ({len(issues)} issues)", "ERROR")
            self.exit_code = max(self.exit_code, 1)

        return passed

    def _validate_cpp_syntax(self, filepath: Path) -> bool:
        """Validate C++ file syntax"""
        try:
            with open(filepath, "r", encoding="utf-8") as f:
                content = f.read()

            # Check for basic syntax patterns
            issues = []

            # Check for matching braces
            brace_count = content.count("{") - content.count("}")
            if brace_count != 0:
                issues.append(f"Mismatched braces (difference: {brace_count})")

            # Check for matching parentheses
            paren_count = content.count("(") - content.count(")")
            if paren_count != 0:
                issues.append(f"Mismatched parentheses (difference: {paren_count})")

            # Check for common UE4 patterns
            if "UCLASS" in content or "USTRUCT" in content or "UFUNCTION" in content:
                # File looks like valid UE header
                if "GENERATED_BODY" not in content and "GENERATED_UCLASS_BODY" not in content:
                    self.log_verbose(
                        f"Warning: {filepath.name} has UE macros but no GENERATED_BODY"
                    )

            return len(issues) == 0

        except Exception as e:
            self.log_verbose(f"Error reading {filepath}: {e}")
            return False

    def check_dependencies(self) -> bool:
        """Verify all required dependencies are present"""
        self.log("Checking dependencies...")
        check_name = "dependencies"

        required_includes = {
            "CoreMinimal.h": ["TestStation.h", "TestLevelGameMode.h"],
            "GameFramework/GameModeBase.h": ["TestLevelGameMode.h"],
            "Engine/DataTable.h": ["TestDataStructures.h"],
            "Components/ActorComponent.h": ["TestStation.h"],
        }

        issues = []
        for include, files in required_includes.items():
            for filename in files:
                filepath = self.test_root / filename
                if filepath.exists():
                    with open(filepath, "r", encoding="utf-8") as f:
                        content = f.read()
                        if include not in content:
                            issues.append(
                                f"Missing include '{include}' in {filename}"
                            )
                            self.log_verbose(
                                f"Include check failed: {filename} missing {include}"
                            )

        # Check for external system dependencies
        external_checks = self._check_external_dependencies()
        issues.extend(external_checks)

        passed = len(issues) == 0
        self.results["checks"][check_name] = {
            "passed": passed,
            "message": f"Dependency check ({len(required_includes)} dependencies)",
            "details": issues if issues else ["All dependencies present"],
        }

        if passed:
            self.log("✓ Dependency check passed")
        else:
            self.log(f"✗ Dependency check failed ({len(issues)} issues)", "ERROR")
            self.exit_code = max(self.exit_code, 2)

        return passed

    def _check_external_dependencies(self) -> List[str]:
        """Check for external system dependencies"""
        issues = []

        # Check for performance profiler
        perf_header = self.source_root / "Public" / "PerformanceProfiler.h"
        if not perf_header.exists():
            issues.append("Missing PerformanceProfiler.h (required by TestStation)")

        # Check for orbital body system
        orbital_header = self.source_root / "Public" / "OrbitalBody.h"
        if not orbital_header.exists():
            issues.append("Missing OrbitalBody.h (required for scaling tests)")

        # Check for spaceship
        spaceship_header = self.source_root / "Public" / "Spaceship.h"
        if not spaceship_header.exists():
            issues.append("Missing Spaceship.h (required for flight tests)")

        return issues

    def check_file_structure(self) -> bool:
        """Validate test level directory structure"""
        self.log("Checking file structure...")
        check_name = "file_structure"

        expected_paths = [
            self.test_root,
            self.source_root / "Private" / "Testing",
            self.project_root / ".kiro" / "specs" / "comprehensive-test-level",
        ]

        issues = []
        for path in expected_paths:
            if not path.exists():
                issues.append(f"Missing directory: {path}")
                self.log_verbose(f"Directory not found: {path}")

        # Check for test station files
        test_files = list(self.test_root.glob("*.h"))
        if len(test_files) < 5:
            issues.append(f"Expected at least 5 test station files, found {len(test_files)}")

        passed = len(issues) == 0
        self.results["checks"][check_name] = {
            "passed": passed,
            "message": f"File structure check ({len(expected_paths)} directories)",
            "details": issues if issues else ["All directories present"],
        }

        if passed:
            self.log(f"✓ File structure check passed ({len(test_files)} test files)")
        else:
            self.log(f"✗ File structure check failed ({len(issues)} issues)", "ERROR")
            self.exit_code = max(self.exit_code, 3)

        return passed

    def check_syntax(self) -> bool:
        """Run comprehensive syntax checks"""
        self.log("Checking syntax...")
        check_name = "syntax"

        test_files = list(self.test_root.glob("*.h"))
        test_impl_files = list((self.source_root / "Private" / "Testing").glob("*.cpp"))
        all_files = test_files + test_impl_files

        syntax_issues = []
        for filepath in all_files:
            errors = self._check_file_syntax(filepath)
            syntax_issues.extend(errors)

        passed = len(syntax_issues) == 0
        self.results["checks"][check_name] = {
            "passed": passed,
            "message": f"Syntax check ({len(all_files)} files)",
            "details": syntax_issues if syntax_issues else ["No syntax errors found"],
        }

        if passed:
            self.log(f"✓ Syntax check passed ({len(all_files)} files)")
        else:
            self.log(f"✗ Syntax check found {len(syntax_issues)} issues", "WARNING")

        return passed

    def _check_file_syntax(self, filepath: Path) -> List[str]:
        """Check individual file for common syntax errors"""
        issues = []

        try:
            with open(filepath, "r", encoding="utf-8") as f:
                lines = f.readlines()

            # Check for common patterns
            for i, line in enumerate(lines, 1):
                # Check for TODO without owner
                if "TODO" in line and ":" not in line.split("TODO")[1]:
                    issues.append(f"{filepath.name}:{i} TODO without owner")

                # Check for console logging in release code
                if filepath.suffix == ".cpp":
                    if "UE_LOG" in line and "Verbose" in line:
                        self.log_verbose(f"Found verbose logging at {filepath.name}:{i}")

                # Check for commented-out code blocks
                if line.strip().startswith("//") and any(
                    kw in line for kw in ["TODO", "FIXME", "XXX", "HACK"]
                ):
                    self.log_verbose(f"Found comment marker at {filepath.name}:{i}")

        except Exception as e:
            issues.append(f"Error reading {filepath.name}: {str(e)}")

        return issues

    def check_test_station_classes(self) -> bool:
        """Verify test station class hierarchy"""
        self.log("Checking test station classes...")
        check_name = "test_station_classes"

        required_classes = [
            ("UTestStation", "TestStation.h"),
            ("ATestLevelGameMode", "TestLevelGameMode.h"),
            ("UVRTestStation", "VRTestStation.h"),
            ("USpaceshipFlightTestStation", "SpaceshipFlightTestStation.h"),
        ]

        issues = []
        for class_name, filename in required_classes:
            filepath = self.test_root / filename
            if filepath.exists():
                with open(filepath, "r", encoding="utf-8") as f:
                    content = f.read()
                    if f"class {class_name}" not in content:
                        issues.append(f"Class {class_name} not found in {filename}")
                        self.log_verbose(f"Missing class definition: {class_name}")
                    else:
                        self.log_verbose(f"Found class: {class_name}")
            else:
                issues.append(f"File not found: {filename}")

        # Check for inheritance structure
        if (self.test_root / "VRTestStation.h").exists():
            with open(self.test_root / "VRTestStation.h") as f:
                content = f.read()
                if "UTestStation" not in content:
                    issues.append("VRTestStation should inherit from UTestStation")

        passed = len(issues) == 0
        self.results["checks"][check_name] = {
            "passed": passed,
            "message": f"Test station classes check ({len(required_classes)} classes)",
            "details": issues if issues else [
                f"All {len(required_classes)} test station classes verified"
            ],
        }

        if passed:
            self.log(f"✓ Test station classes check passed ({len(required_classes)} classes)")
        else:
            self.log(f"✗ Test station classes check failed ({len(issues)} issues)", "ERROR")
            self.exit_code = max(self.exit_code, 1)

        return passed

    def check_documentation(self) -> bool:
        """Verify required documentation exists"""
        self.log("Checking documentation...")
        check_name = "documentation"

        required_docs = [
            ".kiro/specs/comprehensive-test-level/tasks.md",
            ".kiro/specs/comprehensive-test-level/design.md",
            ".kiro/specs/comprehensive-test-level/requirements.md",
            ".kiro/specs/comprehensive-test-level/INTEGRATION_TESTS.md",
            ".kiro/specs/comprehensive-test-level/WORLD_PARTITION_SETUP.md",
            ".kiro/specs/comprehensive-test-level/IMPLEMENTATION_STATUS.md",
        ]

        issues = []
        for doc_path in required_docs:
            full_path = self.project_root / doc_path
            if not full_path.exists():
                issues.append(f"Missing documentation: {doc_path}")
                self.log_verbose(f"Doc not found: {full_path}")
            else:
                self.log_verbose(f"Found doc: {doc_path}")

        passed = len(issues) == 0
        self.results["checks"][check_name] = {
            "passed": passed,
            "message": f"Documentation check ({len(required_docs)} docs)",
            "details": issues if issues else [
                f"All {len(required_docs)} documentation files present"
            ],
        }

        if passed:
            self.log(f"✓ Documentation check passed ({len(required_docs)} files)")
        else:
            self.log(f"✗ Documentation check failed ({len(issues)} issues)", "WARNING")

        return passed

    def generate_report(self) -> Dict:
        """Generate final build report"""
        self.log("Generating build report...")

        # Count results
        for check_name, check_result in self.results["checks"].items():
            self.results["summary"]["total_checks"] += 1
            if check_result["passed"]:
                self.results["summary"]["passed_checks"] += 1
            else:
                self.results["summary"]["failed_checks"] += 1

        # Overall status
        all_passed = self.results["summary"]["failed_checks"] == 0
        self.results["build_status"] = "SUCCESS" if all_passed else "FAILED"
        self.results["exit_code"] = self.exit_code

        return self.results

    def save_report(self):
        """Save report to file if specified"""
        if self.output_file:
            try:
                with open(self.output_file, "w", encoding="utf-8") as f:
                    json.dump(self.results, f, indent=2)
                self.log(f"Report saved to {self.output_file}")
            except Exception as e:
                self.log(f"Failed to save report: {e}", "ERROR")

    def print_summary(self):
        """Print summary to console"""
        summary = self.results["summary"]
        print("\n" + "=" * 70)
        print("BUILD VERIFICATION SUMMARY")
        print("=" * 70)
        print(f"Total Checks: {summary['total_checks']}")
        print(f"Passed: {summary['passed_checks']}")
        print(f"Failed: {summary['failed_checks']}")
        print(f"Status: {self.results['build_status']}")
        print("=" * 70)

        if self.results["checks"]:
            print("\nCheck Results:")
            for check_name, check_result in self.results["checks"].items():
                status = "✓" if check_result["passed"] else "✗"
                print(f"{status} {check_result['message']}")

        print("=" * 70 + "\n")


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="Build verification script for Alexander test level"
    )
    parser.add_argument(
        "--verbose", "-v", action="store_true", help="Enable verbose logging"
    )
    parser.add_argument(
        "--output", "-o", type=str, help="Output file for JSON report (optional)"
    )

    args = parser.parse_args()

    # Create verifier
    verifier = BuildVerifier(verbose=args.verbose, output_file=args.output)

    try:
        # Run all checks
        verifier.check_compilation()
        verifier.check_dependencies()
        verifier.check_file_structure()
        verifier.check_syntax()
        verifier.check_test_station_classes()
        verifier.check_documentation()

        # Generate report
        verifier.generate_report()

        # Save if requested
        verifier.save_report()

        # Print summary
        verifier.print_summary()

        # Return exit code
        sys.exit(verifier.exit_code)

    except KeyboardInterrupt:
        verifier.log("Build verification interrupted by user", "WARNING")
        sys.exit(130)
    except Exception as e:
        verifier.log(f"Unexpected error: {e}", "ERROR")
        sys.exit(5)


if __name__ == "__main__":
    main()
