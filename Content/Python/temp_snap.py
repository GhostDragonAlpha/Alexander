import unreal
import time

timestamp = time.strftime("%Y%m%d_%H%M%S")
filename = f"snap_{timestamp}"
command = f"HighResShot 1920x1080 filename={filename}"
unreal.SystemLibrary.execute_console_command(None, command)
print(f"📸 Screenshot: {filename}.png")
