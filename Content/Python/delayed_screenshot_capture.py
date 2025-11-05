"""
Delayed Screenshot Capture System
Runs AFTER editor is fully loaded using a timer
Author: AI Vision System
Date: November 3, 2025
"""

import unreal
import os

# Import the screenshot system
import sys
python_path = os.path.join(unreal.SystemLibrary.get_project_directory(), "Content", "Python")
if python_path not in sys.path:
    sys.path.append(python_path)

from enhanced_screenshot_system import run_enhanced_screenshot_system

def delayed_screenshot_callback():
    """This runs after a delay to ensure editor is fully ready"""
    print("\n" + "="*80)
    print("🚀 STARTING DELAYED SCREENSHOT CAPTURE")
    print("="*80)
    print("Editor is now fully loaded and ready for screenshots!\n")
    
    # Run the screenshot system
    run_enhanced_screenshot_system()
    
    print("\n" + "="*80)
    print("✅ DELAYED SCREENSHOT CAPTURE COMPLETE!")
    print("="*80)

def setup_delayed_screenshot_capture(delay_seconds=5.0):
    """Set up a timer to capture screenshots after editor loads"""
    print(f"\n⏰ Screenshot capture will start in {delay_seconds} seconds...")
    print("Waiting for editor to be fully interactive...")
    
    # Use Unreal's timer system to delay execution
    # This runs in the editor tick, not during Python init
    timer_handle = unreal.PythonBPLib.set_timer(delayed_screenshot_callback, delay_seconds, False)
    
    return timer_handle

# Auto-start when imported during init
if __name__ == "__main__" or True:
    print("\n📸 Delayed Screenshot System Initialized")
    setup_delayed_screenshot_capture(10.0)  # Wait 10 seconds after init completes
