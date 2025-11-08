
import unreal
from datetime import datetime

try:
    # Take high-resolution screenshot
    timestamp = "20251108_003145"
    filename = f"autonomous_dev_{timestamp}_iter_3"
    
    # Focus on overview if possible
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    cameras = [a for a in all_actors if isinstance(a, unreal.CameraActor)]
    
    # Try to focus on overview camera
    for actor in all_actors:
        if actor and "Overview" in str(actor.get_name()):
            unreal.EditorLevelLibrary.set_selected_level_actors([actor])
            break
    
    # Capture screenshot
    command = f"HighResShot 1920x1080 filename={filename}"
    unreal.SystemLibrary.execute_console_command(None, command)
    
    print(f"DEVELOPMENT SCREENSHOT: {filename}.png")
    
    # Save screenshot metadata
    with open("C:/Users/allen/Desktop/Alexander/latest_dev_screenshot.json", 'w') as f:
        import json
        json.dump({
            "filename": f"{filename}.png",
            "timestamp": timestamp,
            "iteration": 3,
            "type": "development_analysis"
        }, f, indent=2)
    
    # Mark completion
    with open("C:/Users/allen/Desktop/Alexander/screenshot_complete.txt", 'w') as f:
        f.write(f"Screenshot captured: {filename}.png")
    
except Exception as e:
    print(f"Screenshot error: {e}")
    with open("C:/Users/allen/Desktop/Alexander/screenshot_complete.txt", 'w') as f:
        f.write(f"Screenshot error: {str(e)}")
