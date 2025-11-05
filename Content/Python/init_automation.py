# Unreal Python Init Script - Auto-executed on startup
import sys
import os

# Add content python path
content_path = r'C:\Users\allen\Desktop\Alexander\Alexander\Content\Python'
if content_path not in sys.path:
    sys.path.append(content_path)

print("=" * 70)
print("  EXECUTING AUTOMATED BUILD")
print("=" * 70)

# Execute the automated build
try:
    exec(open(r'C:\Users\allen\Desktop\Alexander\Alexander\Content\Python\automated_build.py').read())
    print("\n✓ Automated build script executed successfully")
except Exception as e:
    print(f"\n✗ Error during automated build: {e}")
    import traceback
    traceback.print_exc()
