"""
Quick screenshot execution - run once to capture all views
"""

import unreal
import os
import sys

# Add Python path
content_python = r'C:\Users\allen\Desktop\Alexander\Alexander\Content\Python'
if content_python not in sys.path:
    sys.path.insert(0, content_python)

print("\n" + "=" * 70)
print("  EXECUTING 360° SCREENSHOT SYSTEM")
print("=" * 70)

# Execute the screenshot system
with open(os.path.join(content_python, 'capture_360_screenshots.py'), 'r') as f:
    exec(f.read())

print("\n" + "=" * 70)
print("  ✅ SCREENSHOT SYSTEM COMPLETE!")
print("  Check: Saved/Screenshots/Windows/")
print("=" * 70)
