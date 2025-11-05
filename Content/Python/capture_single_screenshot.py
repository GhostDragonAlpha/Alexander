"""
Single Screenshot Capture - Manual trigger
Run this script to capture one screenshot at a time
Author: AI Vision System
Date: November 3, 2025
"""

import unreal

def capture_one_screenshot(filename="screenshot"):
    """Capture a single high-resolution screenshot"""
    
    print("\n" + "="*60)
    print(f"📸 Capturing screenshot: {filename}.png")
    print("="*60)
    
    # Use the editor's high resolution screenshot command
    command = f"HighResShot 1920x1080 filename={filename}"
    unreal.SystemLibrary.execute_console_command(None, command)
    
    print(f"✅ Screenshot command executed: {filename}.png")
    print(f"📂 Will be saved to: Saved/Screenshots/WindowsEditor/")
    print("="*60 + "\n")
    
    return True

# When run directly, capture a test screenshot
if __name__ == "__main__":
    capture_one_screenshot("test_screenshot")
