"""
FINAL Fully Automated Profiling - Zero Manual Intervention
Loads SolarSystem map, runs for 3 minutes, collects and analyzes data
"""

import subprocess
import time
import json
import sys
from pathlib import Path
from datetime import datetime

# Set UTF-8 encoding
if sys.platform == "win32":
    import io
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

# Configuration
EDITOR = r"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe"
PROJECT = r"C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject"
MAP_TO_LOAD = "/Game/FlightTest"  # Primary testing level (SolarSystem needs content - Phase 8)
PROFILING_DIR = Path(r"C:\Users\allen\Desktop\Alexander\Alexander\Saved\Profiling")
PROFILE_DURATION = 180  # 3 minutes

def main():
    print("=" * 80)
    print("AUTOMATED PROFILING - SolarSystem Map")
    print("=" * 80)
    print(f"Time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"Map: {MAP_TO_LOAD}")
    print(f"Duration: {PROFILE_DURATION} seconds ({PROFILE_DURATION/60:.1f} minutes)")
    print()

    # Ensure directory exists
    PROFILING_DIR.mkdir(parents=True, exist_ok=True)

    # Get starting file count
    existing_files = list(PROFILING_DIR.glob("ProfileReport_*.json"))
    before_count = len(existing_files)
    print(f"[1/4] Existing profiling reports: {before_count}")
    print()

    # Launch game with specific map
    print("[2/4] Launching game...")
    print(f"  Resolution: 1280x720 windowed")
    print(f"  Map: {MAP_TO_LOAD}")
    print(f"  Mode: Standalone game (-game flag)")
    print()

    cmd = [
        EDITOR,
        PROJECT,
        MAP_TO_LOAD,  # Load specific map
        "-game",  # Standalone game mode
        "-ResX=1280",
        "-ResY=720",
        "-windowed",
        "-unattended"
    ]

    print(f"  Command: {' '.join(cmd)}")
    print()
    print(f"  Game will profile for {PROFILE_DURATION} seconds...")
    print("  Progress updates every 30 seconds")
    print()

    start_time = time.time()

    try:
        # Launch game
        process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )

        # Wait for profiling duration with progress updates
        print("  Profiling:")
        for i in range(PROFILE_DURATION):
            # Check if process exited early
            if process.poll() is not None:
                print(f"\\n  Game exited after {i} seconds")
                break

            # Progress updates
            if i > 0 and i % 30 == 0:
                print(f"    [{i:3}/{PROFILE_DURATION}s] Profiling...")

            time.sleep(1)

        # Terminate if still running
        if process.poll() is None:
            print(f"\\n  Terminating game after {PROFILE_DURATION}s...")
            process.terminate()
            try:
                process.wait(timeout=10)
            except subprocess.TimeoutExpired:
                print("  Force killing...")
                process.kill()

        elapsed = time.time() - start_time
        print(f"\\n  Session completed: {elapsed:.1f} seconds")
        print()

    except Exception as e:
        print(f"  ERROR: {e}")
        return 1

    # Wait for file system to flush
    time.sleep(3)

    # Find new profiling files
    print("[3/4] Locating profiling data...")
    json_files = list(PROFILING_DIR.glob("ProfileReport_*.json"))
    after_count = len(json_files)

    # Find files created after we started
    new_files = [f for f in json_files if f.stat().st_mtime > start_time - 10]

    if not new_files:
        print(f"  No new profiling data")
        print(f"  Files before: {before_count}, after: {after_count}")
        return 1

    latest_json = max(new_files, key=lambda p: p.stat().st_mtime)
    print(f"  Found: {latest_json.name}")
    print()

    # Analyze results
    print("[4/4] Analyzing results...")
    print()

    try:
        with open(latest_json, 'r', encoding='utf-8') as f:
            data = json.load(f)

        print("=" * 80)
        print("PROFILING RESULTS - " + MAP_TO_LOAD)
        print("=" * 80)
        print(f"Total Frames: {data.get('TotalFrames', 0)}")
        print(f"Average FPS: {data.get('AverageFPS', 0):.2f}")
        print(f"Average Frame Time: {data.get('AverageFrameTimeMs', 0):.3f}ms")
        print(f"Memory: {data.get('CurrentMemoryMB', 0)} MB (Peak: {data.get('PeakMemoryMB', 0)} MB)")
        print()

        if 'Profiles' in data and len(data['Profiles']) > 0:
            print("SYSTEM PERFORMANCE:")
            print("-" * 80)

            profiles = sorted(data['Profiles'], key=lambda x: x.get('AverageTimeMs', 0), reverse=True)

            for profile in profiles:
                name = profile.get('SystemName', profile.get('Name', 'Unknown'))
                avg = profile.get('AverageTimeMs', 0)
                max_t = profile.get('MaxTimeMs', 0)
                min_t = profile.get('MinTimeMs', 0)
                count = profile.get('SampleCount', profile.get('CallCount', 0))

                # Determine status (90 FPS target = 11.1ms budget)
                if avg > 8:
                    status = "CRITICAL"
                    symbol = "!!"
                elif avg > 5:
                    status = "HIGH"
                    symbol = "! "
                elif avg > 2:
                    status = "MEDIUM"
                    symbol = "~ "
                else:
                    status = "OK"
                    symbol = "  "

                print(f"{symbol} [{status:8}] {name:30} Avg: {avg:7.3f}ms  Max: {max_t:9.3f}ms  Samples: {count:6}")

            print()
            print("=" * 80)
            print("BOTTLENECK ANALYSIS")
            print("=" * 80)

            critical = [p for p in profiles if p.get('AverageTimeMs', 0) > 8]
            high = [p for p in profiles if 5 < p.get('AverageTimeMs', 0) <= 8]
            medium = [p for p in profiles if 2 < p.get('AverageTimeMs', 0) <= 5]
            ok = [p for p in profiles if p.get('AverageTimeMs', 0) <= 2]

            print(f"!! CRITICAL (>8ms): {len(critical)} systems")
            for p in critical:
                name = p.get('SystemName', p.get('Name', 'Unknown'))
                avg = p.get('AverageTimeMs', 0)
                print(f"     {name}: {avg:.3f}ms")

            if high:
                print()
                print(f"!  HIGH (5-8ms): {len(high)} systems")
                for p in high:
                    name = p.get('SystemName', p.get('Name', 'Unknown'))
                    avg = p.get('AverageTimeMs', 0)
                    print(f"     {name}: {avg:.3f}ms")

            if medium:
                print()
                print(f"~  MEDIUM (2-5ms): {len(medium)} systems")

            print()
            print(f"   OK (<2ms): {len(ok)} systems")

            print()
            print("=" * 80)
            print("NEXT STEPS")
            print("=" * 80)

            if critical:
                print("!! Critical bottlenecks found - Phase 10 optimization required:")
                for p in critical:
                    name = p.get('SystemName', p.get('Name', 'Unknown'))
                    print(f"   - Optimize {name}")
            elif high:
                print("!  High priority optimizations recommended")
            elif medium:
                print("~  Monitor these systems, consider optimization")
            else:
                print("   Excellent performance! Consider adding more systems to profiling")

            print()
            print("=" * 80)
            print(f"COMPLETE - Profiled {len(profiles)} systems")
            print(f"Report: {latest_json}")
            print("=" * 80)

            return 0
        else:
            print("WARNING: No profiling samples collected")
            print()
            print("Possible causes:")
            print("  - Map didn't load properly")
            print("  - No actors with profiled systems spawned")
            print("  - Game crashed during profiling")
            print()
            return 1

    except Exception as e:
        print(f"ERROR parsing results: {e}")
        return 1

if __name__ == "__main__":
    sys.exit(main())
