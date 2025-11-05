"""
Load Saved Camera Position
Restores viewport to your saved camera position
"""

import unreal
import json
import os

print("\n" + "=" * 70)
print("  LOAD CAMERA POSITION")
print("=" * 70 + "\n")

config_path = os.path.join(
    r'C:\Users\allen\Desktop\Alexander\Alexander\Content\Python',
    'saved_camera_position.json'
)

if not os.path.exists(config_path):
    print("❌ No saved camera position found")
    print("   Use 'save_camera_position.py' first to save a position")
else:
    try:
        with open(config_path, 'r') as f:
            camera_data = json.load(f)

        location_data = camera_data.get('location', {})
        rotation_data = camera_data.get('rotation', {})

        location = unreal.Vector(
            location_data.get('x', 0),
            location_data.get('y', 0),
            location_data.get('z', 0)
        )

        rotation = unreal.Rotator(
            rotation_data.get('pitch', 0),
            rotation_data.get('yaw', 0),
            rotation_data.get('roll', 0)
        )

        print(f"📍 Loading Camera Position:")
        print(f"   Location: ({location.x:.1f}, {location.y:.1f}, {location.z:.1f})")
        print(f"   Rotation: (Pitch={rotation.pitch:.1f}, Yaw={rotation.yaw:.1f}, Roll={rotation.roll:.1f})")
        print()

        # Set viewport camera
        viewport_client = unreal.EditorLevelLibrary.get_editor_viewport_client(0)

        if viewport_client:
            viewport_client.set_view_location(location)
            viewport_client.set_view_rotation(rotation)
            print("✅ Camera position restored!")
        else:
            print("❌ Could not get viewport client")

    except Exception as e:
        print(f"❌ Error loading: {e}")
        import traceback
        traceback.print_exc()

print("\n" + "=" * 70 + "\n")
