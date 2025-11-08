
import unreal
from datetime import datetime

try:
    # Take screenshot with timestamp
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"orchestrated_{timestamp}"
    command = f"HighResShot 1920x1080 filename={filename}"
    
    unreal.SystemLibrary.execute_console_command(None, command)
    
    print(f"ORCHESTRATED SCREENSHOT: {filename}.png")
    
    # Save screenshot info
    with open("C:/Users/allen/Desktop/Alexander/latest_orchestrated_screenshot.txt", 'w') as f:
        f.write(f"{filename}.png")
    
except Exception as e:
    print(f"Screenshot error: {e}")
