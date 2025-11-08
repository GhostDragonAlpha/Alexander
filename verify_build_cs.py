#!/usr/bin/env python3
"""
verify_build_cs.py - Verify Alexander.Build.cs module dependencies

Validates:
1. Required modules are present (Core, CoreUObject, Engine, etc.)
2. All modules referenced in code are in dependencies
3. Public/private module split is correct
4. Include paths are correctly specified
"""

import os
import re
import sys
from pathlib import Path
from typing import Set, List


class BuildCSVerifier:
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.build_cs_file = self.project_root / "Source" / "Alexander" / "Alexander.Build.cs"

        # Modules that might be referenced in code
        self.expected_modules = {
            # Core required modules
            'Core': True,
            'CoreUObject': True,
            'Engine': True,

            # Standard modules
            'InputCore': False,
            'Niagara': False,
            'HTTP': False,
            'Json': False,
            'JsonUtilities': False,
            'RHI': False,
            'RenderCore': False,
            'Slate': False,
            'SlateCore': False,
            'UMG': False,
            'ToolMenus': False,
            'ProceduralMeshComponent': False,
            'GameplayTags': False,
            'GameplayTasks': False,
            'AIModule': False,
            'NavigationSystem': False,
            'AudioMixer': False,
            'DeveloperSettings': False,
            'EnhancedInput': False,
            'HeadMountedDisplay': False,
            'CinematicCamera': False,
            'XRBase': False,
            'PhysicsCore': False,
            'Sockets': False,
            'Networking': False,
        }

        self.errors: List[str] = []
        self.warnings: List[str] = []
        self.found_modules: Set[str] = set()
        self.public_modules: Set[str] = set()
        self.private_modules: Set[str] = set()

    def read_build_cs(self) -> str:
        """Read Alexander.Build.cs file"""
        if not self.build_cs_file.exists():
            self.errors.append(f"Build.cs file not found: {self.build_cs_file}")
            return ""

        try:
            with open(self.build_cs_file, 'r', encoding='utf-8') as f:
                return f.read()
        except Exception as e:
            self.errors.append(f"Cannot read {self.build_cs_file}: {e}")
            return ""

    def verify_modules(self, content: str):
        """Verify all required modules are present"""
        print("[VERIFICATION] Checking module declarations...")

        # Parse PublicDependencyModuleNames
        public_pattern = r'PublicDependencyModuleNames\.AddRange\(new string\[\]\s*\{([^}]+)\}'
        public_match = re.search(public_pattern, content)

        if public_match:
            modules_text = public_match.group(1)
            for module in re.findall(r'"([^"]+)"', modules_text):
                self.found_modules.add(module)
                self.public_modules.add(module)

        # Parse PrivateDependencyModuleNames
        private_pattern = r'PrivateDependencyModuleNames\.AddRange\(new string\[\]\s*\{([^}]+)\}'
        private_match = re.search(private_pattern, content)

        if private_match:
            modules_text = private_match.group(1)
            for module in re.findall(r'"([^"]+)"', modules_text):
                self.found_modules.add(module)
                self.private_modules.add(module)

        # Check required modules
        print("\n[MODULE CHECK] Verifying required modules...")
        for module, required in self.expected_modules.items():
            if required and module not in self.found_modules:
                self.errors.append(f"Missing required module: {module}")
            elif module in self.found_modules:
                print(f"  [OK] {module}")

    def check_module_placement(self):
        """Verify modules are in correct public/private sections"""
        print("\n[PLACEMENT CHECK] Verifying module placement...")

        # UMG should be public (used by data assets/UI)
        if 'UMG' in self.private_modules:
            self.warnings.append("UMG should be in PublicDependencyModuleNames (used by data assets)")

        # Engine should be public (always needed)
        if 'Engine' in self.private_modules:
            self.errors.append("Engine should be in PublicDependencyModuleNames")

        # Core modules should be public
        if 'Core' in self.private_modules or 'CoreUObject' in self.private_modules:
            self.errors.append("Core/CoreUObject must be in PublicDependencyModuleNames")

    def check_include_paths(self, content: str):
        """Verify include paths are correctly set"""
        print("\n[INCLUDE PATH CHECK] Verifying include paths...")

        required_paths = [
            "Planetary",
            "CelestialScaling/Public",
            "Public/Testing"
        ]

        for path in required_paths:
            if path in content:
                print(f"  [OK] Include path for {path}")
            else:
                self.warnings.append(f"Include path not found or not configured: {path}")

    def check_code_references(self) -> Set[str]:
        """Scan source files for module references"""
        print("\n[CODE SCAN] Analyzing source code for module references...")

        referenced_modules: Set[str] = set()

        # Common include patterns and their required modules
        include_patterns = {
            r'#include\s+[<"]CoreMinimal': 'Core',
            r'#include\s+[<"]Engine/': 'Engine',
            r'#include\s+[<"]Niagara/': 'Niagara',
            r'#include\s+[<"]Json': 'Json',
            r'#include\s+[<"]Engine/DataAsset': 'Engine',
            r'#include\s+[<"]UObject/': 'CoreUObject',
            r'#include\s+[<"]Containers/': 'Core',
            r'#include\s+[<"]Components/': 'Engine',
            r'#include\s+[<"]GameFramework/': 'Engine',
            r'#include\s+[<"]Net/': 'Engine',
            r'#include\s+[<"]PhysicsCore/': 'PhysicsCore',
            r'#include\s+[<"]AI/': 'AIModule',
            r'#include\s+[<"]Navigation/': 'NavigationSystem',
        }

        # Scan all source files
        source_root = self.project_root / "Source" / "Alexander"

        for cpp_file in source_root.rglob("*.cpp"):
            try:
                with open(cpp_file, 'r', encoding='utf-8') as f:
                    content = f.read()

                for pattern, module in include_patterns.items():
                    if re.search(pattern, content):
                        referenced_modules.add(module)

            except:
                continue

        # Check if referenced modules are declared
        print("\n[REFERENCE CHECK] Checking if referenced modules are declared...")
        for module in sorted(referenced_modules):
            if module in self.found_modules:
                print(f"  [OK] {module} is declared")
            else:
                self.warnings.append(f"Module referenced but not declared: {module}")

        return referenced_modules

    def verify_all(self) -> bool:
        """Run all verification checks"""
        print("="*70)
        print("ALEXANDER.BUILD.CS VERIFICATION")
        print("="*70)

        content = self.read_build_cs()
        if not content:
            return False

        self.verify_modules(content)
        self.check_module_placement()
        self.check_include_paths(content)
        self.check_code_references()

        self.print_report()
        return len(self.errors) == 0

    def print_report(self):
        """Print verification report"""
        print("\n" + "="*70)
        print("BUILD.CS VERIFICATION REPORT")
        print("="*70)

        print(f"\nModules Found: {len(self.found_modules)}")
        print(f"  Public: {len(self.public_modules)}")
        print(f"  Private: {len(self.private_modules)}")

        if self.found_modules:
            print("\n[MODULES DECLARED]")
            for module in sorted(self.found_modules):
                category = "public" if module in self.public_modules else "private"
                print(f"  [{category}] {module}")

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
            print("\n[SUCCESS] All Build.cs checks passed!")

        print("\n" + "="*70)


def main():
    """Main entry point"""
    project_root = Path(__file__).parent

    verifier = BuildCSVerifier(str(project_root))
    success = verifier.verify_all()

    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
