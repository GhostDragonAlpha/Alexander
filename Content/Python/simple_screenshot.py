"""
Simple manual screenshot capture using viewport
Takes one screenshot of current view
"""

import unreal

def take_viewport_screenshot():
    """Take a screenshot of the current viewport"""
    
    print("\n" + "=" * 60)
    print("SIMPLE SCREENSHOT CAPTURE")
    print("=" * 60)
    
    try:
        # Use the editor viewport screenshot method
        print("\n📸 Capturing viewport screenshot...")
        
        # Get screenshot directory
        screenshot_dir = "C:/Users/allen/Desktop/Alexander/Alexander/Saved/Screenshots/Windows"
        
        # Execute screenshot command
        command = "shot"
        unreal.SystemLibrary.execute_console_command(None, command)
        
        print(f"✅ Screenshot captured!")
        print(f"📂 Saved to: {screenshot_dir}")
        print("\n💡 Screenshot saved as ScreenShot00XXX.png")
        
        return True
        
    except Exception as e:
        print(f"❌ Error: {e}")
        import traceback
        traceback.print_exc()
        return False

# Execute
take_viewport_screenshot()
