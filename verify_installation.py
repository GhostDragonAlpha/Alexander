#!/usr/bin/env python3
"""
Installation Verification Script

Checks that all components of the AI agent-controlled test server are installed
and ready to use.
"""

import sys
from pathlib import Path


def check_dependencies():
    """Check all Python dependencies are installed"""
    print("Checking Python dependencies...")

    dependencies = {
        'fastapi': 'Web framework for API server',
        'uvicorn': 'ASGI server',
        'pydantic': 'Data validation',
        'websockets': 'WebSocket support',
        'PIL': 'Image processing (Pillow)',
        'imagehash': 'Perceptual hashing',
        'requests': 'HTTP client',
        'schedule': 'Task scheduling'
    }

    missing = []
    for module, description in dependencies.items():
        try:
            __import__(module)
            print(f"  [OK] {module:15} - {description}")
        except ImportError:
            print(f"  [FAIL] {module:15} - {description}")
            missing.append(module)

    return len(missing) == 0, missing


def check_files():
    """Check all required files exist"""
    print("\nChecking required files...")

    files = {
        'test_server.py': 'FastAPI test server',
        'agent_sdk.py': 'Agent SDK controller',
        'ue_test_executor.py': 'Unreal Engine executor',
        'web_ui.html': 'Web dashboard',
        'agent_examples/basic_test_run.py': 'Basic example',
        'agent_examples/iterative_debugging.py': 'Debugging example',
        'agent_examples/performance_optimization.py': 'Performance example'
    }

    missing = []
    for file_path, description in files.items():
        path = Path(file_path)
        if path.exists():
            size_kb = path.stat().st_size / 1024
            print(f"  [OK] {file_path:45} - {description:20} ({size_kb:.1f} KB)")
        else:
            print(f"  [FAIL] {file_path:45} - {description}")
            missing.append(file_path)

    return len(missing) == 0, missing


def check_modules():
    """Check modules can be imported"""
    print("\nChecking module imports...")

    modules = {
        'agent_sdk': 'TestServerController',
        'test_server': 'TestServer',
        'ue_test_executor': 'UETestExecutor'
    }

    failed = []
    for module_name, class_name in modules.items():
        try:
            module = __import__(module_name)
            if hasattr(module, class_name):
                print(f"  [OK] {module_name:20} - {class_name} class available")
            else:
                print(f"  [WARN] {module_name:20} - {class_name} class not found")
                failed.append(module_name)
        except Exception as e:
            print(f"  [FAIL] {module_name:20} - Import error: {e}")
            failed.append(module_name)

    return len(failed) == 0, failed


def check_ue_editor():
    """Check if Unreal Engine editor is found"""
    print("\nChecking Unreal Engine installation...")

    try:
        from ue_test_executor import UETestExecutor
        executor = UETestExecutor()

        if executor.ue_editor:
            print(f"  [OK] UE Editor found: {executor.ue_editor}")
            return True, None
        else:
            print(f"  [WARN] UE Editor not found in default locations")
            print(f"         You may need to configure the path manually")
            return False, "UE Editor not found"
    except Exception as e:
        print(f"  [FAIL] Error checking UE Editor: {e}")
        return False, str(e)


def main():
    """Run all verification checks"""
    print("=" * 80)
    print("AI Agent-Controlled Test Server - Installation Verification")
    print("=" * 80)

    results = {}

    # Check dependencies
    deps_ok, missing_deps = check_dependencies()
    results['dependencies'] = deps_ok

    # Check files
    files_ok, missing_files = check_files()
    results['files'] = files_ok

    # Check modules
    modules_ok, failed_modules = check_modules()
    results['modules'] = modules_ok

    # Check UE Editor
    ue_ok, ue_error = check_ue_editor()
    results['ue_editor'] = ue_ok

    # Summary
    print("\n" + "=" * 80)
    print("VERIFICATION SUMMARY")
    print("=" * 80)

    all_ok = all(results.values())

    for check, status in results.items():
        status_str = "[PASS]" if status else "[FAIL]"
        print(f"{status_str} {check.replace('_', ' ').title()}")

    print("=" * 80)

    if all_ok:
        print("\n SUCCESS! All components verified and ready to use.")
        print("\nNext steps:")
        print("  1. Start the test server manually:")
        print("     python test_server.py")
        print("\n  2. Or use the agent SDK (recommended):")
        print("     python agent_examples/basic_test_run.py")
        print("\n  3. View the web dashboard:")
        print("     Open web_ui.html in your browser while server is running")
        return 0
    else:
        print("\n ERRORS DETECTED - Some components are missing or not working.")

        if not results['dependencies'] and missing_deps:
            print(f"\n  Missing dependencies: {', '.join(missing_deps)}")
            print(f"  Install with: pip install {' '.join(missing_deps)}")

        if not results['files'] and missing_files:
            print(f"\n  Missing files: {', '.join(missing_files)}")

        if not results['modules'] and failed_modules:
            print(f"\n  Failed modules: {', '.join(failed_modules)}")

        if not results['ue_editor']:
            print(f"\n  UE Editor issue: {ue_error}")

        return 1


if __name__ == "__main__":
    sys.exit(main())
