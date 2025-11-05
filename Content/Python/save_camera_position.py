"""
Save Current Camera Position
Saves your current viewport camera position as the default startup view
"""

import unreal
import json
import os

print("\n" + "=" * 70)
print("  SAVE CAMERA POSITION")
print("=" * 70 + "\n")

# Get current viewport camera
level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
viewport_client = unreal.EditorLevelLibrary.get_editor_viewport_client(0)

if not viewport_client:
    print("❌ Could not get viewport client")
else:
    # Get camera location and rotation
    location = viewport_client.get_view_location()
    rotation = viewport_client.get_view_rotation()

    print(f"📍 Current Camera Position:")
    print(f"   Location: ({location.x:.1f}, {location.y:.1f}, {location.z:.1f})")
    print(f"   Rotation: (Pitch={rotation.pitch:.1f}, Yaw={rotation.yaw:.1f}, Roll={rotation.roll:.1f})")
    print()

    # Save to file
    camera_data = {
        "location": {"x": location.x, "y": location.y, "z": location.z},
        "rotation": {"pitch": rotation.pitch, "yaw": rotation.yaw, "roll": rotation.roll}
    }

    config_path = os.path.join(
        r'C:\Users\allen\Desktop\Alexander\Alexander\Content\Python',
        'saved_camera_position.json'
    )

    try:
        with open(config_path, 'w') as f:
            json.dump(camera_data, f, indent=2)
        print("✅ Camera position saved!")
        print(f"   Saved to: saved_camera_position.json")
        print()
        print("💡 Use 'load_camera_position.py' to restore this view")
    except Exception as e:
        print(f"❌ Error saving: {e}")

print("\n" + "=" * 70 + "\n")
