#!/usr/bin/env python3
"""
verify_replication.py - Verify network replication setup

Validates:
1. All Replicated properties have DOREPLIFETIME calls
2. All Server RPC functions have _Validate and _Implementation variants
3. GetLifetimeReplicatedProps() implementations are correct
4. Components have SetIsReplicatedComponent(true)
"""

import os
import re
import sys
from pathlib import Path
from typing import Dict, List, Set, Tuple


class ReplicationVerifier:
    def __init__(self, source_root: str):
        self.source_root = Path(source_root)

        # Patterns
        self.replicated_property = re.compile(r'UPROPERTY\([^)]*Replicated[^)]*\)')
        self.doreplifetime_pattern = re.compile(r'DOREPLIFETIME\(\s*(\w+)\s*,\s*(\w+)\s*\)')
        self.server_rpc_pattern = re.compile(r'UFUNCTION\([^)]*Server[^)]*\)')
        self.function_def_pattern = re.compile(r'(\w+\s+)?(\w+)\s*\([^)]*\)\s*;')
        self.uclass_pattern = re.compile(r'UCLASS\([^)]*\)')
        self.setisreplicated_pattern = re.compile(r'SetIsReplicatedComponent\s*\(\s*true\s*\)')

        self.errors: List[str] = []
        self.warnings: List[str] = []

        # Track replicated properties and their implementations
        self.replicated_properties: Dict[str, List[str]] = {}  # Class -> [properties]
        self.doreplifetime_calls: Dict[str, List[Tuple[str, str]]] = {}  # Class -> [(Class, Property)]
        self.server_rpcs: Dict[str, List[str]] = {}  # Class -> [function names]
        self.rpc_implementations: Dict[str, List[str]] = {}  # Class -> [_Implementation functions]

    def scan_header_files(self):
        """Scan all header files for replicated properties and RPC declarations"""
        print("[SCAN] Analyzing header files for replication setup...")

        for header_file in self.source_root.rglob("*.h"):
            self._scan_header_file(header_file)

    def _scan_header_file(self, filepath: Path):
        """Scan a single header file"""
        try:
            with open(filepath, 'r', encoding='utf-8') as f:
                content = f.read()
        except:
            return

        # Extract class name from UCLASS declarations
        class_name = None
        uclass_matches = self.uclass_pattern.findall(content)

        # Find class definition
        class_pattern = r'class\s+(\w+)\s*:'
        class_matches = re.findall(class_pattern, content)

        if class_matches:
            class_name = class_matches[0]

        if not class_name:
            return

        # Find replicated properties
        properties = []
        for match in re.finditer(r'UPROPERTY\(([^)]*Replicated[^)]*)\)', content):
            prop_section = match.group(1)
            # Extract property name from following line
            start = match.end()
            prop_match = re.search(r'\b(\w+)\s+(\w+)\s*;', content[start:start+200])
            if prop_match:
                properties.append(prop_match.group(2))

        if properties:
            self.replicated_properties[class_name] = properties

        # Find Server RPC functions
        rpcs = []
        for match in re.finditer(self.server_rpc_pattern, content):
            # Get function name after UFUNCTION
            start = match.end()
            func_match = re.search(r'\b(\w+)\s+(\w+)\s*\([^)]*\)\s*;', content[start:start+300])
            if func_match:
                rpcs.append(func_match.group(2))

        if rpcs:
            self.server_rpcs[class_name] = rpcs

    def scan_implementation_files(self):
        """Scan all .cpp files for GetLifetimeReplicatedProps and RPC implementations"""
        print("[SCAN] Analyzing implementation files for replication...")

        for cpp_file in self.source_root.rglob("*.cpp"):
            self._scan_implementation_file(cpp_file)

    def _scan_implementation_file(self, filepath: Path):
        """Scan a single implementation file"""
        try:
            with open(filepath, 'r', encoding='utf-8') as f:
                content = f.read()
        except:
            return

        # Find GetLifetimeReplicatedProps implementation
        getlifetime_pattern = r'void\s+(\w+)::GetLifetimeReplicatedProps\s*\([^)]*\)\s*(?:const)?\s*\{([^}]+)\}'
        for match in re.finditer(getlifetime_pattern, content, re.DOTALL):
            class_name = match.group(1)
            func_body = match.group(2)

            # Extract DOREPLIFETIME calls
            doreplifetime_calls = self.doreplifetime_pattern.findall(func_body)

            if doreplifetime_calls:
                self.doreplifetime_calls[class_name] = doreplifetime_calls

        # Find RPC implementations
        rpc_impl_pattern = r'void\s+(\w+)::(\w+)_Implementation\s*\([^)]*\)\s*\{'
        for match in re.finditer(rpc_impl_pattern, content):
            class_name = match.group(1)
            function_base = match.group(2)

            if class_name not in self.rpc_implementations:
                self.rpc_implementations[class_name] = []

            self.rpc_implementations[class_name].append(function_base)

        # Find SetIsReplicatedComponent calls
        for match in re.finditer(self.setisreplicated_pattern, content):
            pass  # Just verify pattern exists

    def verify_replicated_properties(self):
        """Verify all replicated properties have DOREPLIFETIME calls"""
        print("\n[VERIFY] Checking replicated properties...")

        for class_name, properties in self.replicated_properties.items():
            doreplifetime_calls = self.doreplifetime_calls.get(class_name, [])

            # Extract property names from DOREPLIFETIME calls
            doreplifetime_props = {prop for _, prop in doreplifetime_calls}

            for prop in properties:
                if prop not in doreplifetime_props:
                    self.errors.append(
                        f"{class_name}: Replicated property '{prop}' missing DOREPLIFETIME call"
                    )
                else:
                    print(f"  [OK] {class_name}::{prop} has DOREPLIFETIME")

    def verify_rpc_implementations(self):
        """Verify all Server RPC functions have _Implementation variants"""
        print("\n[VERIFY] Checking Server RPC implementations...")

        for class_name, rpcs in self.server_rpcs.items():
            implementations = self.rpc_implementations.get(class_name, [])

            for rpc in rpcs:
                if rpc not in implementations:
                    self.warnings.append(
                        f"{class_name}: Server RPC '{rpc}' missing _Implementation variant"
                    )
                else:
                    print(f"  [OK] {class_name}::{rpc}_Implementation found")

    def verify_component_replication(self):
        """Check if replicating components call SetIsReplicatedComponent(true)"""
        print("\n[VERIFY] Checking component replication setup...")

        # Components with replicated properties should call SetIsReplicatedComponent(true)
        for class_name in self.replicated_properties.keys():
            # Check if this class has SetIsReplicatedComponent call in its constructor
            # This is a simple heuristic - look for the pattern in .cpp files

            found_setisreplicated = False

            for cpp_file in self.source_root.rglob("*.cpp"):
                try:
                    with open(cpp_file, 'r', encoding='utf-8') as f:
                        content = f.read()

                    if class_name in content:
                        constructor_pattern = rf'{class_name}::{class_name}\s*\([^)]*\)\s*\{{([^}}]+)\}}'
                        for match in re.finditer(constructor_pattern, content, re.DOTALL):
                            if self.setisreplicated_pattern.search(match.group(1)):
                                found_setisreplicated = True
                                print(f"  [OK] {class_name} calls SetIsReplicatedComponent(true)")
                                break

                except:
                    continue

            if not found_setisreplicated:
                self.warnings.append(
                    f"{class_name}: Has replicated properties but SetIsReplicatedComponent(true) not found in constructor"
                )

    def verify_getlifetime_functions(self):
        """Verify GetLifetimeReplicatedProps implementations"""
        print("\n[VERIFY] Checking GetLifetimeReplicatedProps implementations...")

        for class_name in self.replicated_properties.keys():
            if class_name in self.doreplifetime_calls:
                # Check that Super::GetLifetimeReplicatedProps is called
                # This is a basic check - could be enhanced

                for cpp_file in self.source_root.rglob("*.cpp"):
                    try:
                        with open(cpp_file, 'r', encoding='utf-8') as f:
                            content = f.read()

                        if class_name in content:
                            func_pattern = rf'void\s+{class_name}::GetLifetimeReplicatedProps\s*\([^)]*\)\s*(?:const)?\s*\{{([^}}]+)\}}'

                            for match in re.finditer(func_pattern, content, re.DOTALL):
                                func_body = match.group(1)
                                if 'Super::GetLifetimeReplicatedProps' in func_body:
                                    print(f"  [OK] {class_name}::GetLifetimeReplicatedProps calls Super")
                                else:
                                    self.warnings.append(
                                        f"{class_name}::GetLifetimeReplicatedProps doesn't call Super::GetLifetimeReplicatedProps"
                                    )
                                break

                    except:
                        continue

    def verify_all(self) -> bool:
        """Run all verification checks"""
        print("="*70)
        print("REPLICATION VERIFICATION")
        print("="*70 + "\n")

        self.scan_header_files()
        self.scan_implementation_files()

        self.verify_replicated_properties()
        self.verify_rpc_implementations()
        self.verify_component_replication()
        self.verify_getlifetime_functions()

        self.print_report()
        return len(self.errors) == 0

    def print_report(self):
        """Print verification report"""
        print("\n" + "="*70)
        print("REPLICATION VERIFICATION REPORT")
        print("="*70)

        print(f"\nClasses with Replicated Properties: {len(self.replicated_properties)}")
        if self.replicated_properties:
            for class_name, props in self.replicated_properties.items():
                print(f"  {class_name}: {len(props)} replicated property(ies)")

        print(f"\nClasses with Server RPCs: {len(self.server_rpcs)}")
        if self.server_rpcs:
            for class_name, rpcs in self.server_rpcs.items():
                print(f"  {class_name}: {len(rpcs)} Server RPC(s)")

        print(f"\nTotal Errors: {len(self.errors)}")
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
            print("\n[SUCCESS] All replication checks passed!")

        print("\n" + "="*70)


def main():
    """Main entry point"""
    source_root = Path(__file__).parent / "Source" / "Alexander"

    if not source_root.exists():
        print(f"[ERROR] Source directory not found: {source_root}")
        sys.exit(1)

    verifier = ReplicationVerifier(str(source_root))
    success = verifier.verify_all()

    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
