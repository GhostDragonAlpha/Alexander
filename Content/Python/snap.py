"""
Manual Screenshot Command - Run this whenever you want to capture the current view
Just execute this script in Unreal's Python console to take a screenshot
Author: AI Vision System
Date: November 3, 2025
"""

import unreal
import os
import time

def snap():
    """Take a screenshot of current viewport - simple and fast"""
    
    # Generate filename with timestamp
    timestamp = time.strftime("%Y%m%d_%H%M%S")
    filename = f"snap_{timestamp}"
    
    # Execute screenshot command
    command = f"HighResShot 1920x1080 filename={filename}"
    unreal.SystemLibrary.execute_console_command(None, command)
    
    # Get the path where it will be saved
    screenshot_path = os.path.join(
        unreal.SystemLibrary.get_project_directory(),
        "Saved", "Screenshots", "WindowsEditor", f"{filename}.png"
    )
    
    print(f"\n📸 Screenshot captured: {filename}.png")
    print(f"📂 Path: {screenshot_path}")
    print(f"⏱️  Saving... (should appear in ~1-2 seconds)\n")
    
    return screenshot_path

# Execute immediately when script is run
if __name__ == "__main__":
    snap()
