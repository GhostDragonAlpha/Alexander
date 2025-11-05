"""
Screenshot + View - Captures screenshot and opens it
Author: AI Vision System  
Date: November 3, 2025
"""

import unreal
import os
import time
import subprocess

def snap_and_view():
    """Take screenshot and open it in default image viewer"""
    
    # Generate filename
    timestamp = time.strftime("%Y%m%d_%H%M%S")
    filename = f"snap_{timestamp}"
    
    # Get full path
    screenshot_dir = os.path.join(
        unreal.SystemLibrary.get_project_directory(),
        "Saved", "Screenshots", "WindowsEditor"
    )
    screenshot_path = os.path.join(screenshot_dir, f"{filename}.png")
    
    # Take screenshot
    command = f"HighResShot 1920x1080 filename={filename}"
    unreal.SystemLibrary.execute_console_command(None, command)
    
    print(f"\n📸 Screenshot: {filename}.png")
    print(f"📂 Saving to: {screenshot_path}")
    print("⏱️  Waiting for file to save...")
    
    # Wait for file to appear (max 5 seconds)
    max_wait = 5.0
    elapsed = 0
    check_interval = 0.2
    
    while elapsed < max_wait:
        time.sleep(check_interval)
        elapsed += check_interval
        
        if os.path.exists(screenshot_path):
            print(f"✅ Screenshot saved! (took {elapsed:.1f}s)")
            
            # Open in default image viewer
            print("🖼️  Opening image...")
            subprocess.Popen(['start', screenshot_path], shell=True)
            
            return screenshot_path
    
    print(f"⚠️  File not found after {max_wait}s - may still be saving")
    return screenshot_path

# Execute when run
if __name__ == "__main__":
    snap_and_view()
