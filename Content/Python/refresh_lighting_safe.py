"""
Safe Lighting Refresh - Just viewport redraw, no heavy building
Use this instead of BuildLighting to avoid crashes
"""

import unreal

print("\n" + "="*70)
print("  🔄 SAFE LIGHTING REFRESH")
print("="*70 + "\n")

print("Refreshing viewport and construction scripts...\n")

try:
    # Gentle refresh commands only
    unreal.SystemLibrary.execute_console_command(None, "RerunConstructionScripts")
    print("✅ Construction scripts refreshed")

    unreal.SystemLibrary.execute_console_command(None, "Editor.RedrawAllViewports")
    print("✅ Viewports redrawn")

    print("\n" + "="*70)
    print("  ✅ SAFE REFRESH COMPLETE!")
    print("="*70 + "\n")

    print("Note: For full lighting build, use Unreal's Build menu manually")
    print("      (Build → Build Lighting Quality → Preview)\n")

except Exception as e:
    print(f"❌ Error: {e}")
    import traceback
    traceback.print_exc()
