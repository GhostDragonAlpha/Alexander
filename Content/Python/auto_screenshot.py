"""
Auto Screenshot After Load - Executes automatically via init_unreal.py
Takes one screenshot 5 seconds after editor finishes loading
"""

import unreal
import time

def delayed_screenshot():
    """Wait a bit, focus on cockpit, then take screenshot"""
    print("\n⏳ Waiting 5 seconds for editor to stabilize...")
    time.sleep(5)
    
    # Focus on the cockpit first!
    print("\n🎯 Focusing viewport on cockpit...")
    editor_level_lib = unreal.EditorLevelLibrary()
    all_actors = editor_level_lib.get_all_level_actors()
    
    cockpit = None
    for actor in all_actors:
        label = actor.get_actor_label()
        if "Cockpit_Base" in label or "Pilot_Seat" in label:
            cockpit = actor
            break
    
    if cockpit:
        print(f"✅ Found: {cockpit.get_actor_label()}")
        # Select it so viewport focuses on it
        unreal.EditorLevelLibrary.set_selected_level_actors([cockpit])
        print("📷 Viewport should now show the cockpit!")
    else:
        print("⚠️ Cockpit not found - will capture whatever is visible")
    
    # Take screenshot
    timestamp = time.strftime("%Y%m%d_%H%M%S")
    filename = f"auto_{timestamp}"
    
    print(f"\n📸 Taking screenshot: {filename}.png")
    command = f"HighResShot 1920x1080 filename={filename}"
    unreal.SystemLibrary.execute_console_command(None, command)
    
    print(f"✅ Screenshot command executed!")
    print(f"📂 File will save to: Saved/Screenshots/WindowsEditor/{filename}.png")
    print("⏱️  Give it 2-3 seconds to save...\n")

# Execute immediately
delayed_screenshot()
