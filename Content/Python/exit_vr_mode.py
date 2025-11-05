"""
Exit VR/XR Mode
Screenshots capture black when editor is in VR preview mode
"""

import unreal

print("\n" + "="*70)
print("  🥽 EXITING VR MODE")
print("="*70 + "\n")

print("⚠️  DIAGNOSIS:")
print("   Editor is in XR/VR mode!")
print("   Viewport is rendering to VR headset, not screen")
print("   Screenshots capture the BLACK preview window\n")

# Try to exit VR preview mode
try:
    # Execute console command to exit VR mode
    unreal.SystemLibrary.execute_console_command(
        None,
        "vr.bEnableHMD 0"
    )
    print("✅ Executed: vr.bEnableHMD 0\n")
except Exception as e:
    print(f"⚠️  Could not execute VR command: {e}\n")

# Try alternative commands
vr_commands = [
    "stereo off",
    "hmd disable",
]

for cmd in vr_commands:
    try:
        unreal.SystemLibrary.execute_console_command(None, cmd)
        print(f"✅ Executed: {cmd}")
    except:
        pass

print("\n" + "="*70)
print("  ✅ VR EXIT COMMANDS SENT!")
print("="*70 + "\n")

print("🎯 MANUAL STEPS TO EXIT VR:")
print("   1. In Unreal Editor, press Alt+P to stop VR preview")
print("   2. Or: Window > VR Mode (uncheck)")
print("   3. Or: Toolbar > VR Preview (turn off)")
print("   4. Viewport should show normal 3D view\n")

print("📸 After exiting VR mode, screenshots will work!\n")
