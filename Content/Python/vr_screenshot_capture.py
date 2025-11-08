
import unreal
from datetime import datetime

try:
    # Take VR validation screenshot
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"vr_validation_{timestamp}"
    
    # Try to focus on VR setup or overview
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    
    # Look for VR-related actors or default to overview
    for actor in all_actors:
        if actor and any(term in str(actor.get_name()).lower() 
                        for term in ["vr", "player", "pawn", "overview"]):
            unreal.EditorLevelLibrary.set_selected_level_actors([actor])
            break
    
    # Capture high-resolution screenshot
    command = f"HighResShot 2560x1440 filename={filename}"
    unreal.SystemLibrary.execute_console_command(None, command)
    
    print(f"VR VALIDATION SCREENSHOT: {filename}.png")
    
    # Save screenshot metadata
    with open("C:/Users/allen/Desktop/Alexander/vr_screenshot_info.json", 'w') as f:
        import json
        json.dump({
            "filename": f"{filename}.png",
            "timestamp": timestamp,
            "type": "vr_validation",
            "resolution": "2560x1440"
        }, f, indent=2)
    
except Exception as e:
    print(f"VR screenshot error: {e}")
