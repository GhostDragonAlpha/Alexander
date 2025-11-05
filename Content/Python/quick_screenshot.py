"""
Quick Screenshot - Simple viewport capture
No camera switching, just captures current view
"""

import unreal

print("\n" + "="*70)
print("  📸 QUICK SCREENSHOT")
print("="*70 + "\n")

try:
    # Simple screenshot command
    filename = "test_lighting_check"

    print(f"Capturing current viewport as: {filename}.png")
    print()

    unreal.SystemLibrary.execute_console_command(
        None,
        f"HighResShot 1920x1080 filename={filename}"
    )

    print("✅ Screenshot command executed!")
    print(f"📁 Saved to: Alexander/Saved/Screenshots/WindowsEditor/")
    print(f"📝 Filename: {filename}.png")
    print()
    print("=" * 70)
    print("  ✅ DONE!")
    print("=" * 70)
    print()
    print("🎯 Next: Check if scene is visible in screenshot")
    print("      - If black: Need manual lighting build in Unreal")
    print("      - If visible: Movable lights working!")
    print()

except Exception as e:
    print(f"❌ Error: {e}")
    import traceback
    traceback.print_exc()
