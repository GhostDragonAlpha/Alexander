"""
Force Exit VR Mode and Test Screenshot
"""

import unreal
import time

print("\n" + "="*70)
print("  🥽 FORCE EXITING VR MODE")
print("="*70 + "\n")

# Execute ALL VR disable commands
vr_disable_commands = [
    "vr.bEnableHMD 0",
    "hmd enable 0",
    "stereo off",
    "r.EnableVR 0",
    "vr.EnableHMD 0",
]

print("Sending VR disable commands:\n")
for cmd in vr_disable_commands:
    try:
        unreal.SystemLibrary.execute_console_command(None, cmd)
        print(f"✅ {cmd}")
    except Exception as e:
        print(f"⚠️  {cmd} - {e}")

# Wait a moment for commands to take effect
time.sleep(1)

print("\n" + "="*70)
print("  📸 TESTING SCREENSHOT CAPTURE")
print("="*70 + "\n")

# Take a quick test screenshot
automation = unreal.AutomationLibrary()
timestamp = time.strftime("%Y%m%d_%H%M%S")
filename = f"vr_exit_test_{timestamp}"

try:
    automation.take_high_res_screenshot(1920, 1080, filename)
    print(f"✅ Test screenshot captured: {filename}.png\n")
except Exception as e:
    print(f"⚠️  Screenshot failed: {e}\n")

print("="*70)
print("  ✅ VR EXIT COMPLETE!")
print("="*70 + "\n")

print("🎯 Next: Check if test screenshot is visible or still black\n")
