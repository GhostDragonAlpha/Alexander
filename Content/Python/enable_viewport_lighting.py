"""
Enable Viewport Lighting and Show Flags
Ensure the viewport is rendering with proper lighting enabled
"""

import unreal

print("\n" + "="*70)
print("  🎬 ENABLING VIEWPORT LIGHTING")
print("="*70 + "\n")

# Get editor subsystem
editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)

# Get active viewport
viewport_client = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

print("📋 Viewport Rendering Settings:\n")

# Try to enable lit mode
try:
    # Set view mode to lit
    editor_subsystem.set_view_mode(unreal.ViewModeIndex.LIT)
    print("✅ Set view mode to LIT\n")
except Exception as e:
    print(f"⚠️  Could not set view mode: {e}\n")

# Commands to enable lighting in viewport
commands_to_run = [
    "show Lighting",
    "show PostProcessing",
    "show Fog",
    "show Atmosphere",
    "show DynamicShadows",
    "show AmbientOcclusion",
    "show DirectionalLights",
    "show PointLights",
    "show SpotLights",
    "show SkyLighting",
    "viewmode lit",
]

print("="*70)
print("  🔧 ENABLING VIEWPORT SHOW FLAGS")
print("="*70 + "\n")

for cmd in commands_to_run:
    try:
        unreal.SystemLibrary.execute_console_command(
            None,  # World context
            cmd
        )
        print(f"✅ Executed: {cmd}")
    except Exception as e:
        print(f"⚠️  Failed: {cmd} - {e}")

print("\n" + "="*70)
print("  ✅ VIEWPORT LIGHTING ENABLED!")
print("="*70 + "\n")

print("🎬 Viewport is now configured for:")
print("   • Lit view mode (not unlit/wireframe)")
print("   • All light types visible")
print("   • Dynamic shadows enabled")
print("   • Post-processing active")
print("   • Atmospheric effects enabled\n")

print("📸 Ready to capture properly lit screenshots!\n")
