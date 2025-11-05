"""
Focus Viewport on VR Spaceship - Centers camera on the cockpit
"""

import unreal

def focus_on_cockpit():
    """Find the cockpit and focus viewport on it"""
    
    print("\n🎯 Focusing viewport on VR Spaceship...")
    
    editor_level_lib = unreal.EditorLevelLibrary()
    all_actors = editor_level_lib.get_all_level_actors()
    
    # Find cockpit base
    cockpit = None
    for actor in all_actors:
        if "Cockpit_Base" in actor.get_actor_label():
            cockpit = actor
            break
    
    if cockpit:
        print(f"✅ Found cockpit at: {cockpit.get_actor_location()}")
        
        # Select the cockpit
        unreal.EditorLevelLibrary.set_selected_level_actors([cockpit])
        
        # Focus viewport on selected actor (F key equivalent)
        unreal.LevelEditorSubsystem().pilot_level_actor(cockpit)
        
        print("📸 Viewport focused on cockpit!")
        print("Take a screenshot now to see the VR spaceship!")
    else:
        print("⚠️ Cockpit not found! Scene may not have built correctly.")
        print(f"Total actors in scene: {len(all_actors)}")

focus_on_cockpit()
