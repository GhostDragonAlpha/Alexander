"""
Trigger Lighting Build and Scene Refresh
CRITICAL: After making lighting changes, must rebuild/refresh!
"""

import unreal

print("\n" + "="*70)
print("  🔨 TRIGGERING LIGHTING BUILD & REFRESH")
print("="*70 + "\n")

# Execute console commands to refresh lighting
commands = [
    "RecompileShaders Changed",  # Recompile shaders
    "r.InvalidateCachedShaders 1",  # Invalidate shader cache
    "BuildLighting",  # Trigger lighting build
]

print("Executing build commands:\n")

for cmd in commands:
    try:
        unreal.SystemLibrary.execute_console_command(None, cmd)
        print(f"✅ {cmd}")
    except Exception as e:
        print(f"⚠️  {cmd} - {e}")

print("\n" + "="*70)
print("  🔄 REFRESHING VIEWPORT")
print("="*70 + "\n")

# Refresh commands
refresh_commands = [
    "RerunConstructionScripts",
    "RefreshAllNodes",
    "Editor.RedrawAllViewports",
]

for cmd in refresh_commands:
    try:
        unreal.SystemLibrary.execute_console_command(None, cmd)
        print(f"✅ {cmd}")
    except Exception as e:
        print(f"⚠️  {cmd} - {e}")

print("\n" + "="*70)
print("  ✅ BUILD TRIGGERED!")
print("="*70 + "\n")

print("⏳ Wait 10-30 seconds for lighting to build...")
print("💡 Static skylight will recalculate")
print("💡 Movable lights will activate")
print("💡 Scene should become visible!\n")
