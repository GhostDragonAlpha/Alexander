"""
Capture screenshot of the RUNNING Unreal Editor GUI window
This shows what YOU see on screen
"""

import subprocess
import time
from datetime import datetime
import os

SCREENSHOTS_DIR = r"C:\Users\allen\Desktop\Alexander\Alexander\Screenshots"

def capture_editor_window():
    """Use Windows built-in screenshot for active window"""

    # Ensure screenshots directory exists
    os.makedirs(SCREENSHOTS_DIR, exist_ok=True)

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"editor_view_{timestamp}.png"
    filepath = os.path.join(SCREENSHOTS_DIR, filename)

    print("=" * 70)
    print("CAPTURING UNREAL EDITOR WINDOW")
    print("=" * 70)
    print(f"\n[1/3] Target file: {filename}")

    # Use PowerShell to capture active window
    # This requires the Unreal Editor window to be in focus
    ps_script = f"""
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

# Focus Unreal Editor window
$unreal = Get-Process -Name "UnrealEditor" -ErrorAction SilentlyContinue
if ($unreal) {{
    $unreal | ForEach-Object {{
        $_.MainWindowHandle | Out-Null
    }}
    Start-Sleep -Milliseconds 500
}}

# Take screenshot
$screen = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
$bitmap = New-Object System.Drawing.Bitmap $screen.Width, $screen.Height
$graphics = [System.Drawing.Graphics]::FromImage($bitmap)
$graphics.CopyFromScreen($screen.Location, [System.Drawing.Point]::Empty, $screen.Size)
$bitmap.Save("{filepath}", [System.Drawing.Imaging.ImageFormat]::Png)
$graphics.Dispose()
$bitmap.Dispose()
"""

    print("[2/3] Capturing screen...")

    try:
        result = subprocess.run(
            ["powershell", "-Command", ps_script],
            capture_output=True,
            text=True,
            timeout=10
        )

        if os.path.exists(filepath):
            size = os.path.getsize(filepath)
            print(f"[3/3] ✓ Screenshot saved: {filepath}")
            print(f"      Size: {size:,} bytes")
            return filepath
        else:
            print("[3/3] ✗ Screenshot file not created")
            return None

    except Exception as e:
        print(f"[3/3] ✗ Error: {e}")
        return None

if __name__ == "__main__":
    print("\nMake sure Unreal Editor window is visible on screen!")
    print("Capturing in 3 seconds...\n")
    time.sleep(3)

    result = capture_editor_window()

    if result:
        print("\n" + "=" * 70)
        print("SUCCESS - Screenshot captured")
        print("=" * 70)
    else:
        print("\n" + "=" * 70)
        print("FAILED - Could not capture screenshot")
        print("=" * 70)
