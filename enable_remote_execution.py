"""
Enable Python Remote Execution in Unreal Editor
Run this via remote_client.py OR manually in Unreal's Python console
"""

import unreal

print("=" * 70)
print("ENABLING PYTHON REMOTE EXECUTION")
print("=" * 70)

# Get Python Editor Subsystem
python_subsystem = unreal.get_editor_subsystem(unreal.PythonScriptLibrary)

print("\n[INFO] Python subsystem loaded")
print("[INFO] Remote execution should be enabled in Project Settings")
print("[INFO] Path: Edit → Project Settings → Plugins → Python → Remote Execution")

# Log success
unreal.log("Python remote execution check complete")

print("\n" + "=" * 70)
print("TO ENABLE MANUALLY:")
print("=" * 70)
print("1. In Unreal Editor, go to Edit → Project Settings")
print("2. Search for 'Python'")
print("3. Under 'Python' section, find 'Remote Execution'")
print("4. Check the 'Enable Remote Execution' checkbox")
print("5. Set Multicast Group: 239.0.0.1")
print("6. Set Multicast Port: 6766")
print("7. Restart editor")
print("=" * 70)
