#!/usr/bin/env python3
"""
Build and Capture Script for Alexander Project

This script:
1. Builds the Unreal project
2. Starts the editor (maximized)
3. Waits for full load
4. Takes a system screenshot of the editor window
5. Collects log files
6. Shuts down the editor
7. Outputs everything to 'capture_output/' directory

Usage:
    python build_and_capture.py
"""

import subprocess
import time
import os
import sys
import shutil
from datetime import datetime
from pathlib import Path

# Try to import screenshot libraries
try:
    from PIL import ImageGrab
    import win32gui
    import win32con
    import win32ui
    SCREENSHOT_AVAILABLE = True
except ImportError:
    print("⚠️  PIL or pywin32 not installed. Install with:")
    print("    pip install pillow pywin32")
    SCREENSHOT_AVAILABLE = False

# Configuration
PROJECT_DIR = Path(__file__).parent
PROJECT_FILE = PROJECT_DIR / "Alexander.uproject"
UNREAL_EDITOR = Path("C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor.exe")
BUILD_BAT = PROJECT_DIR / "quick_build.bat"
OUTPUT_DIR = PROJECT_DIR / "capture_output"
LOG_DIR = PROJECT_DIR / "Saved" / "Logs"

# Timing
BUILD_TIMEOUT = 600  # 10 minutes
EDITOR_LOAD_TIME = 60  # 60 seconds to fully load
CAPTURE_DELAY = 5  # Extra seconds after load before screenshot

def print_header(text):
    """Print a formatted header"""
    print(f"\n{'='*70}")
    print(f"  {text}")
    print(f"{'='*70}\n")

def print_step(text):
    """Print a step message"""
    print(f"[STEP] {text}")

def print_success(text):
    """Print a success message"""
    print(f"  [OK] {text}")

def print_error(text):
    """Print an error message"""
    print(f"  [ERROR] {text}")

def print_info(text):
    """Print an info message"""
    print(f"  [INFO] {text}")

def run_build():
    """Build the project using quick_build.bat"""
    print_step("Building project...")

    if not BUILD_BAT.exists():
        print_error(f"Build script not found: {BUILD_BAT}")
        return False

    start_time = time.time()

    try:
        result = subprocess.run(
            [str(BUILD_BAT)],
            cwd=str(PROJECT_DIR),
            capture_output=True,
            text=True,
            timeout=BUILD_TIMEOUT
        )

        duration = time.time() - start_time

        # Save build output
        OUTPUT_DIR.mkdir(exist_ok=True)
        build_output_file = OUTPUT_DIR / "build_output.txt"
        with open(build_output_file, 'w') as f:
            f.write(result.stdout)
            if result.stderr:
                f.write("\n\n=== STDERR ===\n")
                f.write(result.stderr)

        if result.returncode == 0:
            print_success(f"Build succeeded in {duration:.1f} seconds")
            print_info(f"Build output saved to: {build_output_file}")
            return True
        else:
            print_error(f"Build failed with exit code {result.returncode}")
            print_info(f"Build output saved to: {build_output_file}")
            print("\nLast 20 lines of build output:")
            print(result.stdout.split('\n')[-20:])
            return False

    except subprocess.TimeoutExpired:
        print_error(f"Build timed out after {BUILD_TIMEOUT} seconds")
        return False
    except Exception as e:
        print_error(f"Build error: {e}")
        return False

def find_window_by_title(partial_title):
    """Find a window by partial title match"""
    def callback(hwnd, windows):
        if win32gui.IsWindowVisible(hwnd):
            title = win32gui.GetWindowText(hwnd)
            if partial_title.lower() in title.lower():
                windows.append((hwnd, title))
        return True

    windows = []
    win32gui.EnumWindows(callback, windows)
    return windows

def maximize_window(hwnd):
    """Maximize a window"""
    try:
        win32gui.ShowWindow(hwnd, win32con.SW_MAXIMIZE)
        time.sleep(1)  # Wait for maximize animation
        return True
    except Exception as e:
        print_error(f"Failed to maximize window: {e}")
        return False

def capture_window_screenshot(hwnd, output_file):
    """Capture screenshot of a specific window"""
    try:
        # Get window rect
        left, top, right, bottom = win32gui.GetWindowRect(hwnd)
        width = right - left
        height = bottom - top

        # Capture the screen area
        screenshot = ImageGrab.grab(bbox=(left, top, right, bottom))
        screenshot.save(output_file)

        print_success(f"Screenshot saved: {output_file}")
        print_info(f"Resolution: {width}x{height}")
        return True

    except Exception as e:
        print_error(f"Screenshot failed: {e}")
        return False

def start_editor():
    """Start the Unreal Editor"""
    print_step("Starting Unreal Editor...")

    if not UNREAL_EDITOR.exists():
        print_error(f"Unreal Editor not found: {UNREAL_EDITOR}")
        return None

    if not PROJECT_FILE.exists():
        print_error(f"Project file not found: {PROJECT_FILE}")
        return None

    try:
        # Start editor process
        process = subprocess.Popen(
            [str(UNREAL_EDITOR), str(PROJECT_FILE)],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        print_success(f"Editor started (PID: {process.pid})")
        return process

    except Exception as e:
        print_error(f"Failed to start editor: {e}")
        return None

def wait_for_editor_window():
    """Wait for the editor window to appear"""
    print_step(f"Waiting for editor to load ({EDITOR_LOAD_TIME}s)...")

    # Wait for editor to appear
    max_wait = 120  # 2 minutes max
    start_time = time.time()

    while time.time() - start_time < max_wait:
        windows = find_window_by_title("Unreal Editor")
        if windows:
            hwnd, title = windows[0]
            print_success(f"Editor window found: {title}")
            return hwnd
        time.sleep(2)

    print_error("Editor window not found")
    return None

def collect_logs():
    """Collect the latest log files"""
    print_step("Collecting log files...")

    if not LOG_DIR.exists():
        print_error(f"Log directory not found: {LOG_DIR}")
        return

    try:
        # Find the latest log file
        log_files = list(LOG_DIR.glob("*.log"))
        if not log_files:
            print_error("No log files found")
            return

        # Sort by modification time
        log_files.sort(key=lambda x: x.stat().st_mtime, reverse=True)
        latest_log = log_files[0]

        # Copy to output directory
        output_log = OUTPUT_DIR / f"editor_log_{datetime.now().strftime('%Y%m%d_%H%M%S')}.log"
        shutil.copy2(latest_log, output_log)

        print_success(f"Log copied: {output_log}")

        # Show last 50 lines
        with open(latest_log, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
            last_lines = lines[-50:]

        # Save last lines to summary
        summary_file = OUTPUT_DIR / "log_summary.txt"
        with open(summary_file, 'w', encoding='utf-8') as f:
            f.write("Last 50 lines of editor log:\n")
            f.write("="*70 + "\n")
            f.writelines(last_lines)

        print_info(f"Log summary saved: {summary_file}")

    except Exception as e:
        print_error(f"Failed to collect logs: {e}")

def kill_editor(process):
    """Kill the editor process"""
    print_step("Shutting down editor...")

    try:
        # Try graceful termination first
        process.terminate()
        try:
            process.wait(timeout=10)
            print_success("Editor shut down gracefully")
            return
        except subprocess.TimeoutExpired:
            pass

        # Force kill if still running
        process.kill()
        process.wait()
        print_success("Editor force closed")

    except Exception as e:
        print_error(f"Failed to close editor: {e}")
        # Try taskkill as last resort
        try:
            subprocess.run(["taskkill", "/F", "/IM", "UnrealEditor.exe"],
                         capture_output=True, check=False)
            print_info("Used taskkill to close editor")
        except:
            pass

def create_summary_report():
    """Create a summary report for AI review"""
    print_step("Creating summary report...")

    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

    summary = f"""# Alexander Project - Build and Capture Report

**Generated**: {timestamp}

---

## Files in this directory:

1. **editor_screenshot.png** - Full editor window screenshot
2. **editor_log_[timestamp].log** - Complete editor log file
3. **log_summary.txt** - Last 50 lines of log for quick review
4. **build_output.txt** - Full build output
5. **summary.md** - This file

---

## Quick Review Checklist:

### Build Status
- [ ] Build succeeded without errors
- [ ] No compilation warnings
- [ ] Build time acceptable

### Editor Screenshot
- [ ] Editor loaded successfully
- [ ] No visible errors in UI
- [ ] Content browser visible
- [ ] Viewport shows expected content

### Log Analysis
- [ ] No error messages
- [ ] No warning messages
- [ ] Modules loaded correctly
- [ ] Plugins initialized

---

## Next Steps for AI Agent:

1. Review the screenshot for visual issues
2. Check build_output.txt for compilation issues
3. Scan log_summary.txt for errors/warnings
4. If issues found, make fixes and re-run this script
5. Repeat until all systems operational

---

## How to Run Again:

```bash
python build_and_capture.py
```

This will:
- Rebuild the project
- Restart the editor
- Capture a new screenshot
- Update all logs
- Create a new report
"""

    summary_file = OUTPUT_DIR / "summary.md"
    with open(summary_file, 'w') as f:
        f.write(summary)

    print_success(f"Summary report created: {summary_file}")

def main():
    """Main execution flow"""
    print_header("ALEXANDER PROJECT - BUILD AND CAPTURE")

    print_info(f"Project: {PROJECT_FILE.name}")
    print_info(f"Output: {OUTPUT_DIR}")

    # Create output directory
    OUTPUT_DIR.mkdir(exist_ok=True)

    # Step 1: Build the project
    print_header("Step 1: Building Project")
    if not run_build():
        print_error("Build failed. Cannot continue.")
        return 1

    # Step 2: Start the editor
    print_header("Step 2: Starting Editor")
    editor_process = start_editor()
    if not editor_process:
        return 1

    try:
        # Step 3: Wait for editor window
        print_header("Step 3: Waiting for Editor")
        time.sleep(EDITOR_LOAD_TIME)

        hwnd = wait_for_editor_window()
        if not hwnd:
            print_error("Could not find editor window")
            return 1

        # Step 4: Maximize and capture screenshot
        if SCREENSHOT_AVAILABLE:
            print_header("Step 4: Capturing Screenshot")

            print_step("Maximizing window...")
            maximize_window(hwnd)

            print_step(f"Waiting {CAPTURE_DELAY}s for UI to settle...")
            time.sleep(CAPTURE_DELAY)

            screenshot_file = OUTPUT_DIR / "editor_screenshot.png"
            capture_window_screenshot(hwnd, screenshot_file)
        else:
            print_error("Screenshot libraries not available")

        # Step 5: Collect logs
        print_header("Step 5: Collecting Logs")
        collect_logs()

        # Step 6: Create summary
        print_header("Step 6: Creating Summary")
        create_summary_report()

    finally:
        # Always clean up
        print_header("Step 7: Cleanup")
        kill_editor(editor_process)

    # Final summary
    print_header("CAPTURE COMPLETE")
    print_success("All data collected successfully!")
    print_info(f"Review files in: {OUTPUT_DIR}")
    print_info("Files created:")
    print_info("  - editor_screenshot.png")
    print_info("  - editor_log_[timestamp].log")
    print_info("  - log_summary.txt")
    print_info("  - build_output.txt")
    print_info("  - summary.md")

    return 0

if __name__ == "__main__":
    sys.exit(main())
