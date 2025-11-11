#!/usr/bin/env python3
"""
Build and Capture Script for Alexander Project - RUNTIME VERSION

This script:
1. Builds the Unreal project
2. Starts the game in standalone mode (maximized)
3. Waits for it to load and run
4. Takes system screenshots during gameplay
5. Collects log files
6. Shuts down the game
7. Outputs everything to 'capture_output_runtime/' directory

Usage:
    python build_and_capture_runtime.py
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
OUTPUT_DIR = PROJECT_DIR / "capture_output_runtime"
LOG_DIR = PROJECT_DIR / "Saved" / "Logs"

# Timing
BUILD_TIMEOUT = 600  # 10 minutes
GAME_LOAD_TIME = 30  # 30 seconds to load game
GAMEPLAY_DURATION = 30  # Run game for 30 seconds
SCREENSHOT_INTERVAL = 5  # Take screenshot every 5 seconds

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
    print(f"  ✅ {text}")

def print_error(text):
    """Print an error message"""
    print(f"  ❌ {text}")

def print_info(text):
    """Print an info message"""
    print(f"  ℹ️  {text}")

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
        time.sleep(1)
        return True
    except Exception as e:
        print_error(f"Failed to maximize window: {e}")
        return False

def capture_window_screenshot(hwnd, output_file):
    """Capture screenshot of a specific window"""
    try:
        left, top, right, bottom = win32gui.GetWindowRect(hwnd)
        width = right - left
        height = bottom - top

        screenshot = ImageGrab.grab(bbox=(left, top, right, bottom))
        screenshot.save(output_file)

        print_success(f"Screenshot saved: {output_file.name}")
        return True

    except Exception as e:
        print_error(f"Screenshot failed: {e}")
        return False

def start_game():
    """Start the game in standalone mode"""
    print_step("Starting game in standalone mode...")

    if not UNREAL_EDITOR.exists():
        print_error(f"Unreal Editor not found: {UNREAL_EDITOR}")
        return None

    if not PROJECT_FILE.exists():
        print_error(f"Project file not found: {PROJECT_FILE}")
        return None

    try:
        # Start game with -game flag for standalone mode
        process = subprocess.Popen(
            [str(UNREAL_EDITOR), str(PROJECT_FILE), "-game", "-log"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )

        print_success(f"Game started (PID: {process.pid})")
        return process

    except Exception as e:
        print_error(f"Failed to start game: {e}")
        return None

def wait_for_game_window():
    """Wait for the game window to appear"""
    print_step(f"Waiting for game to load ({GAME_LOAD_TIME}s)...")

    max_wait = 60  # 1 minute max
    start_time = time.time()

    while time.time() - start_time < max_wait:
        # Try both "Alexander" and "UnrealEditor" window titles
        windows = find_window_by_title("Alexander")
        if not windows:
            windows = find_window_by_title("UnrealEditor")

        if windows:
            hwnd, title = windows[0]
            print_success(f"Game window found: {title}")
            return hwnd
        time.sleep(2)

    print_error("Game window not found")
    return None

def capture_gameplay_screenshots(hwnd):
    """Capture multiple screenshots during gameplay"""
    print_step(f"Capturing gameplay for {GAMEPLAY_DURATION}s...")

    if not SCREENSHOT_AVAILABLE:
        print_error("Screenshot libraries not available")
        return []

    screenshots = []
    start_time = time.time()
    screenshot_count = 0

    while time.time() - start_time < GAMEPLAY_DURATION:
        elapsed = time.time() - start_time
        screenshot_count += 1

        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        screenshot_file = OUTPUT_DIR / f"gameplay_{screenshot_count:02d}_{timestamp}.png"

        print_info(f"Capturing screenshot {screenshot_count} (t={elapsed:.1f}s)")

        if capture_window_screenshot(hwnd, screenshot_file):
            screenshots.append(screenshot_file)

        # Wait for next screenshot
        time.sleep(SCREENSHOT_INTERVAL)

    print_success(f"Captured {len(screenshots)} screenshots")
    return screenshots

def collect_logs():
    """Collect the latest log files"""
    print_step("Collecting log files...")

    if not LOG_DIR.exists():
        print_error(f"Log directory not found: {LOG_DIR}")
        return

    try:
        log_files = list(LOG_DIR.glob("*.log"))
        if not log_files:
            print_error("No log files found")
            return

        log_files.sort(key=lambda x: x.stat().st_mtime, reverse=True)
        latest_log = log_files[0]

        output_log = OUTPUT_DIR / f"game_log_{datetime.now().strftime('%Y%m%d_%H%M%S')}.log"
        shutil.copy2(latest_log, output_log)

        print_success(f"Log copied: {output_log.name}")

        # Save last 100 lines for runtime
        with open(latest_log, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
            last_lines = lines[-100:]

        summary_file = OUTPUT_DIR / "log_summary.txt"
        with open(summary_file, 'w', encoding='utf-8') as f:
            f.write("Last 100 lines of game log:\n")
            f.write("="*70 + "\n")
            f.writelines(last_lines)

        print_info(f"Log summary saved: {summary_file.name}")

    except Exception as e:
        print_error(f"Failed to collect logs: {e}")

def kill_game(process):
    """Kill the game process"""
    print_step("Shutting down game...")

    try:
        process.terminate()
        try:
            process.wait(timeout=10)
            print_success("Game shut down gracefully")
            return
        except subprocess.TimeoutExpired:
            pass

        process.kill()
        process.wait()
        print_success("Game force closed")

    except Exception as e:
        print_error(f"Failed to close game: {e}")
        try:
            subprocess.run(["taskkill", "/F", "/IM", "UnrealEditor.exe"],
                         capture_output=True, check=False)
            print_info("Used taskkill to close game")
        except:
            pass

def create_summary_report(screenshots):
    """Create a summary report for AI review"""
    print_step("Creating summary report...")

    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

    summary = f"""# Alexander Project - Build and Capture Report (RUNTIME)

**Generated**: {timestamp}
**Screenshots Captured**: {len(screenshots)}
**Gameplay Duration**: {GAMEPLAY_DURATION}s

---

## Files in this directory:

1. **gameplay_XX_[timestamp].png** - Screenshots during gameplay ({len(screenshots)} total)
2. **game_log_[timestamp].log** - Complete game log file
3. **log_summary.txt** - Last 100 lines of log for quick review
4. **build_output.txt** - Full build output
5. **summary.md** - This file

---

## Screenshot Timeline:

"""

    for i, screenshot in enumerate(screenshots, 1):
        time_offset = (i - 1) * SCREENSHOT_INTERVAL
        summary += f"- **{screenshot.name}** - t={time_offset}s\n"

    summary += f"""

---

## Quick Review Checklist:

### Build Status
- [ ] Build succeeded without errors
- [ ] Runtime build successful

### Gameplay Screenshots
- [ ] Game launched successfully
- [ ] Visuals rendering correctly
- [ ] No obvious graphical glitches
- [ ] VR environment visible
- [ ] Space ships/objects present

### Performance
- [ ] Game runs smoothly
- [ ] No visible stuttering in screenshots
- [ ] Frame rate acceptable

### Log Analysis
- [ ] No error messages
- [ ] No warning messages
- [ ] Game systems initialized correctly

---

## Next Steps for AI Agent:

1. Review all gameplay screenshots sequentially
2. Check for visual regressions or artifacts
3. Scan log_summary.txt for runtime errors
4. Compare to previous runtime captures
5. If issues found, make fixes and re-run this script

---

## How to Run Again:

```bash
python build_and_capture_runtime.py
```

This will:
- Rebuild the project
- Launch game in standalone mode
- Capture screenshots during gameplay
- Update all logs
- Create a new report
"""

    summary_file = OUTPUT_DIR / "summary.md"
    with open(summary_file, 'w') as f:
        f.write(summary)

    print_success(f"Summary report created: {summary_file.name}")

def main():
    """Main execution flow"""
    print_header("ALEXANDER PROJECT - BUILD AND CAPTURE (RUNTIME)")

    print_info(f"Project: {PROJECT_FILE.name}")
    print_info(f"Output: {OUTPUT_DIR}")
    print_info(f"Gameplay Duration: {GAMEPLAY_DURATION}s")
    print_info(f"Screenshot Interval: {SCREENSHOT_INTERVAL}s")

    OUTPUT_DIR.mkdir(exist_ok=True)

    # Step 1: Build
    print_header("Step 1: Building Project")
    if not run_build():
        print_error("Build failed. Cannot continue.")
        return 1

    # Step 2: Start game
    print_header("Step 2: Starting Game")
    game_process = start_game()
    if not game_process:
        return 1

    screenshots = []

    try:
        # Step 3: Wait for game window
        print_header("Step 3: Waiting for Game")
        time.sleep(GAME_LOAD_TIME)

        hwnd = wait_for_game_window()
        if not hwnd:
            print_error("Could not find game window")
            return 1

        # Step 4: Maximize and capture gameplay
        print_header("Step 4: Capturing Gameplay")
        maximize_window(hwnd)
        time.sleep(2)  # Let window settle

        screenshots = capture_gameplay_screenshots(hwnd)

        # Step 5: Collect logs
        print_header("Step 5: Collecting Logs")
        collect_logs()

        # Step 6: Create summary
        print_header("Step 6: Creating Summary")
        create_summary_report(screenshots)

    finally:
        # Always clean up
        print_header("Step 7: Cleanup")
        kill_game(game_process)

    # Final summary
    print_header("RUNTIME CAPTURE COMPLETE")
    print_success("All data collected successfully!")
    print_info(f"\nReview files in: {OUTPUT_DIR}")
    print_info(f"\nScreenshots captured: {len(screenshots)}")
    print_info("  - gameplay_XX_[timestamp].png (multiple)")
    print_info("  - game_log_[timestamp].log")
    print_info("  - log_summary.txt")
    print_info("  - build_output.txt")
    print_info("  - summary.md")

    return 0

if __name__ == "__main__":
    sys.exit(main())
