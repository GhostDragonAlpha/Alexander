"""
Auto Screenshot - Simple & Reliable
Captures current viewport on every restart
Works with any level, any content
"""

import unreal
import time
from datetime import datetime

print("\n" + "=" * 70)
print("  AUTO-SCREENSHOT ON STARTUP")
print("=" * 70 + "\n")

# Wait for editor to stabilize
print("⏱️  Waiting 3 seconds for editor to stabilize...")
time.sleep(3)

# Generate timestamp filename
timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
filename = f"auto_startup_{timestamp}"

print(f"📸 Capturing screenshot: {filename}.png")
print()

try:
    # Execute screenshot command
    command = f"HighResShot 1920x1080 filename={filename}"
    unreal.SystemLibrary.execute_console_command(None, command)

    print("✅ Screenshot command executed!")
    print(f"📂 Saving to: Saved/Screenshots/WindowsEditor/{filename}.png")
    print("⏱️  File will be ready in 2-3 seconds...")
    print()

    # Note: HighResShot is asynchronous, file saves after this script completes

except Exception as e:
    print(f"❌ Error capturing screenshot: {e}")
    import traceback
    traceback.print_exc()

print("=" * 70 + "\n")
