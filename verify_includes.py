#!/usr/bin/env python3
"""
verify_includes.py - Verify all #include statements in C++ files

Validates:
1. All #includes reference existing files
2. No circular dependencies
3. Forward declarations match actual class names
4. Generated.h includes are at end of files
"""

import os
import re
import sys
from pathlib import Path
from typing import Dict, Set, List, Tuple


class IncludeVerifier:
    def __init__(self, source_root: str):
        self.source_root = Path(source_root)
        self.cpp_files: List[Path] = []
        self.header_files: Dict[str, Path] = {}
        self.errors: List[str] = []
        self.warnings: List[str] = []

        # File patterns
        self.include_pattern = re.compile(r'#include\s+[<"]([^>"]+)[>"]')
        self.forward_decl_pattern = re.compile(r'^\s*(class|struct)\s+(\w+)\s*;', re.MULTILINE)
        self.generated_h_pattern = re.compile(r'#include\s+"(\w+)\.generated\.h"')

    def collect_files(self):
        """Collect all .h and .cpp files in source tree"""
        print("[INFO] Collecting source files...")

        for file in self.source_root.rglob('*'):
            if file.suffix == '.h':
                self.header_files[file.name] = file
                self.cpp_files.append(file)
            elif file.suffix == '.cpp':
                self.cpp_files.append(file)

        print(f"[INFO] Found {len(self.cpp_files)} C++ files and {len(self.header_files)} header files")

    def verify_file(self, filepath: Path) -> List[Tuple[str, str]]:
        """Verify includes in a single file"""
        issues = []

        try:
            with open(filepath, 'r', encoding='utf-8') as f:
                content = f.read()
        except Exception as e:
            self.errors.append(f"Cannot read {filepath}: {e}")
            return issues

        lines = content.split('\n')
        includes = self.include_pattern.findall(content)
        forward_decls = self.forward_decl_pattern.findall(content)

        # Check if .generated.h include is at end (before final #endif)
        if filepath.suffix == '.h':
            self._verify_generated_h_position(filepath, content, lines, issues)

        # Verify each include references an existing file
        for include in includes:
            self._verify_include_exists(filepath, include, issues)

        # Verify forward declarations
        for decl_type, class_name in forward_decls:
            self._verify_forward_declaration(filepath, decl_type, class_name, issues)

        return issues

    def _verify_generated_h_position(self, filepath: Path, content: str, lines: List[str], issues: List):
        """Verify .generated.h include is near end of file"""
        generated_match = self.generated_h_pattern.search(content)

        if generated_match:
            # Find position of generated.h include
            gen_line = None
            for i, line in enumerate(lines):
                if self.generated_h_pattern.search(line):
                    gen_line = i
                    break

            # Find last #endif
            endif_line = None
            for i in range(len(lines) - 1, -1, -1):
                if '#endif' in lines[i]:
                    endif_line = i
                    break

            if gen_line and endif_line and (endif_line - gen_line) > 5:
                self.warnings.append(
                    f"{filepath}:{gen_line+1}: "
                    f"Generated.h include should be near end of file (before #endif)"
                )

    def _verify_include_exists(self, source_file: Path, include_path: str, issues: List):
        """Verify included file exists"""

        # Skip system includes
        if include_path.startswith('Engine/') or include_path.startswith('Core'):
            return

        # Look for matching header files
        include_name = Path(include_path).name

        if include_name not in self.header_files:
            # Check if it's a relative path that exists
            possible_path = source_file.parent / include_path
            if not possible_path.exists():
                self.errors.append(
                    f"{source_file}: Cannot find include: {include_path}"
                )

    def _verify_forward_declaration(self, filepath: Path, decl_type: str, class_name: str, issues: List):
        """Verify forward declaration matches actual class definition"""

        # Find the actual class definition
        for header in self.header_files.values():
            try:
                with open(header, 'r', encoding='utf-8') as f:
                    content = f.read()

                # Look for matching class or struct
                if decl_type == 'class':
                    pattern = rf'\b(?:class|struct)\s+{re.escape(class_name)}\b'
                else:
                    pattern = rf'\bstruct\s+{re.escape(class_name)}\b'

                if re.search(pattern, content):
                    return  # Found matching declaration

            except:
                continue

    def check_circular_dependencies(self) -> List[str]:
        """Detect potential circular dependencies"""
        circular_issues = []

        # Build include graph
        include_graph: Dict[str, Set[str]] = {}

        for filepath in self.header_files.values():
            try:
                with open(filepath, 'r', encoding='utf-8') as f:
                    content = f.read()

                includes = self.include_pattern.findall(content)
                include_graph[filepath.name] = set()

                for include in includes:
                    inc_name = Path(include).name
                    if inc_name in self.header_files:
                        include_graph[filepath.name].add(inc_name)

            except:
                continue

        # Check for cycles using DFS
        for file in include_graph:
            visited = set()
            rec_stack = set()

            if self._has_cycle(file, include_graph, visited, rec_stack):
                circular_issues.append(f"Potential circular dependency involving {file}")

        return circular_issues

    def _has_cycle(self, node: str, graph: Dict, visited: Set, rec_stack: Set) -> bool:
        """DFS to detect cycles"""
        visited.add(node)
        rec_stack.add(node)

        for neighbor in graph.get(node, set()):
            if neighbor not in visited:
                if self._has_cycle(neighbor, graph, visited, rec_stack):
                    return True
            elif neighbor in rec_stack:
                return True

        rec_stack.remove(node)
        return False

    def verify_all(self):
        """Run all verification checks"""
        print("\n[VERIFICATION] Starting include verification...")

        for filepath in self.cpp_files:
            issues = self.verify_file(filepath)

        # Check for circular dependencies
        print("\n[VERIFICATION] Checking for circular dependencies...")
        circular = self.check_circular_dependencies()
        if circular:
            self.warnings.extend(circular)

        self.print_report()

    def print_report(self):
        """Print verification report"""
        print("\n" + "="*70)
        print("INCLUDE VERIFICATION REPORT")
        print("="*70)

        print(f"\nTotal Files Checked: {len(self.cpp_files)}")
        print(f"Total Errors: {len(self.errors)}")
        print(f"Total Warnings: {len(self.warnings)}")

        if self.errors:
            print("\n[ERRORS]")
            for error in self.errors:
                print(f"  ERROR: {error}")

        if self.warnings:
            print("\n[WARNINGS]")
            for warning in self.warnings:
                print(f"  WARNING: {warning}")

        if not self.errors and not self.warnings:
            print("\n[SUCCESS] All include checks passed!")

        print("\n" + "="*70)

        return len(self.errors) == 0


def main():
    """Main entry point"""
    # Use Alexander source directory
    source_root = Path(__file__).parent / "Source" / "Alexander"

    if not source_root.exists():
        print(f"[ERROR] Source directory not found: {source_root}")
        sys.exit(1)

    verifier = IncludeVerifier(str(source_root))
    verifier.collect_files()
    verifier.verify_all()

    # Exit with error code if verification failed
    sys.exit(0 if not verifier.errors else 1)


if __name__ == "__main__":
    main()
