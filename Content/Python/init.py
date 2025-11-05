"""
Editor startup module - This runs when Unreal Editor starts
Place this in: Content/Python/init.py
Configure in Project Settings > Python > Startup Scripts
"""

import unreal_engine as ue
import unreal

@unreal.uclass()
class AutoBuildStartup(unreal.EditorUtilityWidget):
    pass

def on_editor_startup():
    """Called when editor starts"""
    import os
    import sys
    
    # Add Python path
    python_path = r'C:\Users\allen\Desktop\Alexander\Alexander\Content\Python'
    if python_path not in sys.path:
        sys.path.insert(0, python_path)
    
    # Execute the auto-build script
    try:
        unreal.log("Starting automated cockpit build...")
        exec(open(os.path.join(python_path, 'init_auto_startup.py')).read())
    except Exception as e:
        unreal.log_error(f"Failed to run auto-build: {e}")

# Register the startup callback
if __name__ == '__main__':
    on_editor_startup()
