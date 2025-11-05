"""
Remote Camera Capture - Executes via UDP remote execution when editor is open
This script is designed to be sent via socket to Unreal's Python remote execution
"""

import socket
import json

# The Python code to execute in Unreal
python_code = """
import unreal

print("\\n" + "="*70)
print("  📸 CAPTURING ADDITIONAL CAMERA VIEWS (Remote)")
print("="*70 + "\\n")

actors = unreal.EditorLevelLibrary.get_all_level_actors()

# Find and capture from Cam_SideView
for actor in actors:
    if not actor:
        continue
    if 'CameraActor' in actor.get_class().get_name():
        label = actor.get_actor_label()
        if 'SideView' in label:
            print(f"📸 Capturing from {label}...")
            unreal.EditorLevelLibrary.set_level_viewport_camera_info(
                actor.get_actor_location(),
                actor.get_actor_rotation()
            )
            unreal.SystemLibrary.execute_console_command(
                None,
                "HighResShot 1920x1080 filename=solar_sideview_fixed"
            )
            print(f"  ✅ Captured: solar_sideview_fixed.png\\n")
            break

# Find and capture from Cam_TopDown
for actor in actors:
    if not actor:
        continue
    if 'CameraActor' in actor.get_class().get_name():
        label = actor.get_actor_label()
        if 'TopDown' in label:
            print(f"📸 Capturing from {label}...")
            unreal.EditorLevelLibrary.set_level_viewport_camera_info(
                actor.get_actor_location(),
                actor.get_actor_rotation()
            )
            unreal.SystemLibrary.execute_console_command(
                None,
                "HighResShot 1920x1080 filename=solar_topdown_fixed"
            )
            print(f"  ✅ Captured: solar_topdown_fixed.png\\n")
            break

print("="*70)
print("  ✅ ADDITIONAL VIEWS CAPTURED!")
print("="*70)
print()
"""

def send_to_unreal():
    """Send Python code to Unreal via UDP multicast remote execution"""

    # Try method 1: JSON formatted (proper protocol)
    try:
        # Format as JSON object with version field
        message = {
            "version": 1,
            "command": "exec",
            "code": python_code
        }
        json_message = json.dumps(message).encode('utf-8')

        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.sendto(json_message, ('239.0.0.1', 6766))
        s.close()

        print("✅ Sent via JSON format (Method 1)")
        return True

    except Exception as e:
        print(f"❌ JSON format failed: {e}")

    # Try method 2: Direct code (fallback)
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.sendto(python_code.encode('utf-8'), ('239.0.0.1', 6766))
        s.close()

        print("✅ Sent via direct format (Method 2)")
        return True

    except Exception as e:
        print(f"❌ Direct format failed: {e}")
        return False

if __name__ == "__main__":
    print("\n" + "="*70)
    print("  📡 REMOTE CAMERA CAPTURE")
    print("="*70 + "\n")

    if send_to_unreal():
        print("\n✅ Command sent to Unreal Editor")
        print("⏱️  Check Unreal's Output Log for execution results")
        print("📁 Screenshots will be in: Alexander/Saved/Screenshots/WindowsEditor/")
    else:
        print("\n❌ Failed to send command")
        print("💡 Try the startup script method instead")
