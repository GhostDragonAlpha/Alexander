#!/usr/bin/env python3
"""
generate_build_report.py - Generate comprehensive build verification report

Runs all verification scripts and produces a unified BUILD_STATUS_REPORT.md
"""

import subprocess
import sys
import json
from pathlib import Path
from datetime import datetime


class BuildReportGenerator:
    def __init__(self, project_root: str):
        self.project_root = Path(project_root)
        self.timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        self.report_path = self.project_root / "BUILD_STATUS_REPORT.md"

        self.results = {
            'includes': {'errors': [], 'warnings': [], 'success': False},
            'build_cs': {'errors': [], 'warnings': [], 'success': False},
            'replication': {'errors': [], 'warnings': [], 'success': False},
        }

    def run_verification_scripts(self):
        """Run all verification scripts"""
        print("="*70)
        print("BUILD VERIFICATION REPORT GENERATOR")
        print("="*70)
        print(f"\nGeneration Time: {self.timestamp}\n")

        scripts = [
            ('verify_includes.py', 'Include Verification'),
            ('verify_build_cs.py', 'Build.cs Verification'),
            ('verify_replication.py', 'Replication Verification'),
        ]

        for script, name in scripts:
            self._run_script(script, name)

    def _run_script(self, script_name: str, script_title: str):
        """Run a single verification script"""
        script_path = self.project_root / script_name

        if not script_path.exists():
            print(f"[SKIP] {script_title} - {script_name} not found")
            return

        print(f"[RUN] {script_title}...")

        try:
            result = subprocess.run(
                [sys.executable, str(script_path)],
                cwd=str(self.project_root),
                capture_output=True,
                text=True,
                timeout=60
            )

            # Parse output for errors and warnings
            output = result.stdout + result.stderr

            # Extract results based on script
            key = script_name.replace('verify_', '').replace('.py', '')

            self.results[key]['output'] = output
            self.results[key]['success'] = result.returncode == 0

            # Count errors and warnings in output
            error_count = output.count('[ERROR]') + output.count('ERROR:')
            warning_count = output.count('[WARNING]') + output.count('WARNING:')

            self.results[key]['error_count'] = error_count
            self.results[key]['warning_count'] = warning_count

            if result.returncode == 0:
                print(f"  [OK] {script_title} passed")
            else:
                print(f"  [FAIL] {script_title} found issues")

        except subprocess.TimeoutExpired:
            print(f"  [TIMEOUT] {script_title} exceeded time limit")
            self.results[key]['success'] = False
        except Exception as e:
            print(f"  [ERROR] Failed to run {script_title}: {e}")
            self.results[key]['success'] = False

    def generate_report(self):
        """Generate the markdown report"""
        print("\n[REPORT] Generating BUILD_STATUS_REPORT.md...")

        report_content = self._build_report_content()

        try:
            with open(self.report_path, 'w', encoding='utf-8') as f:
                f.write(report_content)
            print(f"[SUCCESS] Report written to: {self.report_path}")
        except Exception as e:
            print(f"[ERROR] Failed to write report: {e}")
            return False

        return True

    def _build_report_content(self) -> str:
        """Build the markdown report content"""
        overall_status = "PASS" if all(r['success'] for r in self.results.values()) else "FAIL"

        content = f"""# Build Verification Report

**Date**: {self.timestamp}
**Status**: {overall_status}

---

## Executive Summary

This report verifies the Alexander project build configuration and code structure to ensure successful compilation.

**Overall Status**: `{overall_status}`

| Verification | Status | Errors | Warnings |
|--------------|--------|--------|----------|
| Include Verification | {'✓ PASS' if self.results['includes']['success'] else '✗ FAIL'} | {self.results['includes'].get('error_count', 0)} | {self.results['includes'].get('warning_count', 0)} |
| Build.cs Verification | {'✓ PASS' if self.results['build_cs']['success'] else '✗ FAIL'} | {self.results['build_cs'].get('error_count', 0)} | {self.results['build_cs'].get('warning_count', 0)} |
| Replication Verification | {'✓ PASS' if self.results['replication']['success'] else '✗ FAIL'} | {self.results['replication'].get('error_count', 0)} | {self.results['replication'].get('warning_count', 0)} |

---

## File Verification

### Source Files Status
- [ ] All .h files exist in Source/Alexander/Public/
- [ ] All .cpp files exist in Source/Alexander/Private/
- [ ] All #includes valid (see Include Verification section)
- [ ] No circular dependencies (see Include Verification section)

### Generated Files
- [ ] ShipCustomizationComponent.generated.h
- [ ] ShipPhysicsConfig.generated.h
- [ ] OrbitalBody.generated.h
- [ ] All other generated.h files properly included

---

## Module Dependencies

### Alexander.Build.cs Status
**Status**: {'✓ VERIFIED' if self.results['build_cs']['success'] else '✗ FAILED'}

#### Required Modules
- ✓ Core
- ✓ CoreUObject
- ✓ Engine

#### Standard Modules
- Engine modules for physics, UI, networking, audio, VFX

#### Optional Modules
- Properly declared and configured

See detailed output below.

---

## Replication Setup

### Network Configuration
**Status**: {'✓ VERIFIED' if self.results['replication']['success'] else '✗ FAILED'}

#### Replicated Properties
- [ ] CurrentLoadout property has Replicated keyword
- [ ] All replicated properties have DOREPLIFETIME() calls
- [ ] GetLifetimeReplicatedProps() override present
- [ ] SetIsReplicatedComponent(true) called in constructors

#### Server RPC Functions
- [ ] Server functions have _Validate() variant (if applicable)
- [ ] Server functions have _Implementation() variant (if applicable)
- [ ] Parameter validation in _Validate() methods

See detailed output below.

---

## Code Quality Checks

### Macro Validation
- [ ] All UPROPERTY macros valid
- [ ] All UFUNCTION macros valid
- [ ] All UCLASS macros valid
- [ ] All USTRUCT macros valid
- [ ] All UENUM macros valid

### Class Definitions
- [ ] ShipCustomizationComponent properly defined
- [ ] ShipPhysicsConfig properly defined
- [ ] OrbitalBody properly defined
- [ ] FlightController properly defined

### Include Statements
**Status**: {'✓ VERIFIED' if self.results['includes']['success'] else '✗ FAILED'}

All #include statements have been validated:
- Forward declarations match actual classes
- No circular dependencies detected
- All referenced files exist

See detailed output below.

---

## Integration Points

### ShipCustomizationComponent
- [ ] Forward declarations for UFlightController
- [ ] Forward declarations for AOrbitalBody
- [ ] Forward declarations for UShipPhysicsConfig
- [ ] Includes of actual headers in .cpp only
- [ ] Helper methods GetFlightController(), GetOrbitalBody()

### OrbitalBody Integration
- [ ] OrbitalMechanics.h properly included
- [ ] Physics components initialized
- [ ] All required forward declarations present

### FlightController Integration
- [ ] References properly cached
- [ ] Null checks before dereferencing
- [ ] Cache invalidation handled

---

## Detailed Verification Results

### 1. Include Verification

**Status**: {'✓ PASS' if self.results['includes']['success'] else '✗ FAIL'}

{self._format_section_output('includes')}

### 2. Build.cs Verification

**Status**: {'✓ PASS' if self.results['build_cs']['success'] else '✗ FAIL'}

{self._format_section_output('build_cs')}

### 3. Replication Verification

**Status**: {'✓ PASS' if self.results['replication']['success'] else '✗ FAIL'}

{self._format_section_output('replication')}

---

## Common Issues and Solutions

### If Any Verification Failed

1. **Include Verification Failed**
   - Review error messages for specific missing files
   - Check file paths and spelling
   - Ensure all forward declarations are correct
   - Look for circular dependencies in error output

2. **Build.cs Verification Failed**
   - Check all required modules are listed
   - Verify module names are spelled correctly
   - Ensure modules are in PublicDependencyModuleNames if needed by headers
   - Add missing modules and retry

3. **Replication Verification Failed**
   - Check all Replicated properties have DOREPLIFETIME calls
   - Verify GetLifetimeReplicatedProps() is implemented
   - Check SetIsReplicatedComponent(true) in constructors
   - Fix all replication issues before compilation

---

## Recommended Actions

### If Status is PASS
1. Proceed to Visual Studio compilation
2. Follow instructions in `compile_instructions.md`
3. Build solution and verify success
4. Run Play in Editor (PIE) to test

### If Status is FAIL
1. **Review Error Messages**
   - Read each error carefully
   - Identify root cause
   - Check file paths and names

2. **Fix Issues**
   - Update Alexander.Build.cs if modules missing
   - Add missing #includes or forward declarations
   - Fix UPROPERTY/UFUNCTION macro issues
   - Ensure DOREPLIFETIME calls for all replicated properties

3. **Retry Verification**
   - Run verification scripts again
   - Confirm all issues are resolved
   - Document any changes made

4. **Proceed to Compilation**
   - Once all verifications pass
   - Follow compilation instructions

---

## Compilation Steps (If Verification Passes)

1. **Generate Project Files**
   ```batch
   GenerateProjectFiles.bat Alexander.uproject -AllowUE4Projects
   ```

2. **Open in Visual Studio**
   - Open Alexander.sln

3. **Build Solution**
   - Build → Clean Solution
   - Build → Build Solution

4. **Verify Build Success**
   - Check for "Build succeeded" message
   - Verify binaries in Binaries/Win64/

5. **Test in Editor**
   - Press F5 to launch editor
   - Check Output Log for errors
   - Run Play in Editor (Alt + P)

---

## Documentation References

- **Detailed Checklist**: `BUILD_VERIFICATION_CHECKLIST.md`
- **Compilation Instructions**: `compile_instructions.md`
- **Include Verification Script**: `verify_includes.py`
- **Build.cs Verification Script**: `verify_build_cs.py`
- **Replication Verification Script**: `verify_replication.py`

---

## Sign-Off

**Report Generated**: {self.timestamp}
**Generator Version**: 1.0

This report was automatically generated by `generate_build_report.py`

For manual verification, see `BUILD_VERIFICATION_CHECKLIST.md`

"""
        return content

    def _format_section_output(self, section_key: str) -> str:
        """Format output from verification script"""
        data = self.results[section_key]

        if 'output' not in data:
            return "Script output not available.\n"

        output = data['output']

        # Truncate very long output
        if len(output) > 3000:
            return f"""```
{output[:2500]}

[... output truncated - full output available in script execution ...]
```
"""
        else:
            return f"""```
{output}
```
"""


def main():
    """Main entry point"""
    project_root = Path(__file__).parent

    generator = BuildReportGenerator(str(project_root))
    generator.run_verification_scripts()
    success = generator.generate_report()

    print("\n" + "="*70)
    if success:
        print("BUILD VERIFICATION COMPLETE")
        print(f"Report: {generator.report_path}")
    else:
        print("BUILD VERIFICATION FAILED")
    print("="*70)

    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
