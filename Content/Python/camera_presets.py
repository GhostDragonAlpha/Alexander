"""
Camera Presets - Quick Navigation
Jump to predefined camera positions for different areas
"""

import unreal

print("\n" + "=" * 70)
print("  CAMERA PRESETS")
print("=" * 70 + "\n")

# Define preset positions
presets = {
    "overview": {
        "location": unreal.Vector(0, -2000, 1500),
        "rotation": unreal.Rotator(-20, 90, 0),
        "description": "High overview of scene center"
    },
    "close_center": {
        "location": unreal.Vector(0, -500, 200),
        "rotation": unreal.Rotator(-10, 90, 0),
        "description": "Close view of scene center"
    },
    "north_view": {
        "location": unreal.Vector(-2000, 0, 500),
        "rotation": unreal.Rotator(0, 0, 0),
        "description": "View from north"
    },
    "south_view": {
        "location": unreal.Vector(2000, 0, 500),
        "rotation": unreal.Rotator(0, 180, 0),
        "description": "View from south"
    },
    "ground_level": {
        "location": unreal.Vector(0, -200, 100),
        "rotation": unreal.Rotator(0, 90, 0),
        "description": "Ground level view"
    }
}

print("Available presets:")
for i, (name, preset) in enumerate(presets.items(), 1):
    print(f"  {i}. {name}: {preset['description']}")

print("\nTo use a preset, uncomment and modify this line:")
print("  # selected_preset = 'overview'")
print()

# ============================================================================
# ✏️ EDIT THIS LINE to choose a preset:
# ============================================================================

selected_preset = 'overview'  # Change this to: overview, close_center, north_view, south_view, ground_level

# ============================================================================

if selected_preset in presets:
    preset = presets[selected_preset]

    print(f"📍 Loading preset: {selected_preset}")
    print(f"   {preset['description']}")
    print()

    location = preset['location']
    rotation = preset['rotation']

    print(f"   Location: ({location.x:.1f}, {location.y:.1f}, {location.z:.1f})")
    print(f"   Rotation: (Pitch={rotation.pitch:.1f}, Yaw={rotation.yaw:.1f}, Roll={rotation.roll:.1f})")
    print()

    # Apply to viewport
    viewport_client = unreal.EditorLevelLibrary.get_editor_viewport_client(0)

    if viewport_client:
        viewport_client.set_view_location(location)
        viewport_client.set_view_rotation(rotation)
        print("✅ Camera moved to preset position!")
    else:
        print("❌ Could not get viewport client")
else:
    print(f"❌ Unknown preset: {selected_preset}")
    print("   Available: " + ", ".join(presets.keys()))

print("\n" + "=" * 70 + "\n")
