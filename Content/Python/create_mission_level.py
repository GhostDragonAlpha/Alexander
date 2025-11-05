"""
Create Mission Level - Automated Level Setup for First Mission
Follows CRITICAL_WORKFLOW_ORDER.md - Always trigger build after creation!

This script creates and configures the MissionLevel with all required actors.
Run this AFTER mission_system_setup.py and AFTER compiling C++ code.

Usage:
    python remote_client.py --file "create_mission_level.py"
    
Then MUST run:
    python remote_client.py --file "trigger_lighting_build.py"
"""

import unreal

def log_info(message):
    """Log information message"""
    unreal.log(f"[LEVEL SETUP] {message}")

def log_warning(message):
    """Log warning message"""
    unreal.log_warning(f"[LEVEL SETUP] {message}")

def log_error(message):
    """Log error message"""
    unreal.log_error(f"[LEVEL SETUP] {message}")

def create_new_level():
    """Create a new empty level for the mission"""
    log_info("Creating new mission level...")
    
    level_path = '/Game/Levels/MissionLevel'
    
    # Check if level already exists
    if unreal.EditorAssetLibrary.does_asset_exist(level_path):
        log_warning(f"Level already exists at {level_path}")
        log_info("Loading existing level...")
        unreal.EditorLevelLibrary.load_level(level_path)
        return True
    
    try:
        # Create new level
        log_info("Creating new empty level...")
        unreal.EditorLevelLibrary.new_level('/Game/Levels/', 'MissionLevel')
        log_info(f"✓ Level created: {level_path}")
        return True
    except Exception as e:
        log_error(f"Failed to create level: {e}")
        return False

def add_directional_light():
    """Add sun (directional light) to the level"""
    log_info("Adding directional light (sun)...")
    
    try:
        # Spawn directional light
        light = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.DirectionalLight,
            unreal.Vector(0, 0, 10000),
            unreal.Rotator(-45, 0, 0)
        )
        
        if light:
            # Configure light properties
            light_component = light.get_editor_property('light_component')
            if light_component:
                light_component.set_editor_property('intensity', 10.0)
                light_component.set_editor_property('light_color', unreal.LinearColor(1.0, 0.98, 0.94, 1.0))
                light_component.set_editor_property('cast_shadows', True)
            
            light.set_actor_label('Sun')
            log_info("✓ Directional light created")
            return light
        else:
            log_error("Failed to spawn directional light")
            return None
    except Exception as e:
        log_error(f"Error creating directional light: {e}")
        return None

def add_sky_atmosphere():
    """Add sky atmosphere for realistic atmosphere"""
    log_info("Adding sky atmosphere...")
    
    try:
        sky = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.SkyAtmosphere,
            unreal.Vector(0, 0, 0),
            unreal.Rotator(0, 0, 0)
        )
        
        if sky:
            sky.set_actor_label('SkyAtmosphere')
            log_info("✓ Sky atmosphere created")
            return sky
        else:
            log_warning("Could not create sky atmosphere")
            return None
    except Exception as e:
        log_warning(f"Sky atmosphere creation failed: {e}")
        return None

def add_post_process_volume():
    """Add post process volume for visual effects"""
    log_info("Adding post process volume...")
    
    try:
        ppv = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.PostProcessVolume,
            unreal.Vector(0, 0, 0),
            unreal.Rotator(0, 0, 0)
        )
        
        if ppv:
            ppv.set_editor_property('unbound', True)
            ppv.set_actor_label('PostProcessVolume')
            log_info("✓ Post process volume created")
            return ppv
        else:
            log_warning("Could not create post process volume")
            return None
    except Exception as e:
        log_warning(f"Post process volume creation failed: {e}")
        return None

def add_player_start():
    """Add player start location"""
    log_info("Adding player start...")
    
    try:
        player_start = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.PlayerStart,
            unreal.Vector(0, 0, 100),  # Slightly above ground
            unreal.Rotator(0, 0, 0)
        )
        
        if player_start:
            player_start.set_actor_label('PlayerStart')
            log_info("✓ Player start created at origin")
            return player_start
        else:
            log_error("Failed to create player start")
            return None
    except Exception as e:
        log_error(f"Error creating player start: {e}")
        return None

def create_planet_placeholder():
    """Create placeholder for planet (requires actual Planet BP)"""
    log_info("Creating planet placeholder...")
    
    try:
        # Create a large sphere as placeholder until actual planet BP is available
        sphere = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(0, 0, -50000),  # Below surface
            unreal.Rotator(0, 0, 0)
        )
        
        if sphere:
            sphere.set_actor_label('Planet_Placeholder')
            sphere.set_actor_scale3d(unreal.Vector(500, 500, 500))  # Large scale
            log_info("✓ Planet placeholder created")
            log_warning("MANUAL STEP: Replace with actual Planet BP from project")
            return sphere
        else:
            log_warning("Could not create planet placeholder")
            return None
    except Exception as e:
        log_warning(f"Planet placeholder creation failed: {e}")
        return None

def create_moon_placeholder():
    """Create placeholder for moon"""
    log_info("Creating moon placeholder...")
    
    try:
        # Moon at 50km (5000000cm) distance
        moon = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(5000000, 0, 0),  # 50km away
            unreal.Rotator(0, 0, 0)
        )
        
        if moon:
            moon.set_actor_label('Moon_Placeholder')
            moon.set_actor_scale3d(unreal.Vector(135, 135, 135))  # ~1/4 planet size
            log_info("✓ Moon placeholder created at 50km distance")
            log_warning("MANUAL STEP: Replace with actual Moon BP from project")
            return moon
        else:
            log_warning("Could not create moon placeholder")
            return None
    except Exception as e:
        log_warning(f"Moon placeholder creation failed: {e}")
        return None

def create_landing_zone_marker():
    """Create visual markers for landing zones"""
    log_info("Creating landing zone markers...")
    
    markers_created = []
    
    try:
        # Planet landing zone marker
        planet_marker = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(500, 0, 0),  # Near player start
            unreal.Rotator(0, 0, 0)
        )
        
        if planet_marker:
            planet_marker.set_actor_label('LandingZone_Planet')
            planet_marker.set_actor_scale3d(unreal.Vector(20, 20, 0.1))
            markers_created.append(planet_marker)
            log_info("✓ Planet landing zone marker created")
        
        # Moon landing zone marker
        moon_marker = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(5000500, 0, 0),  # Near moon
            unreal.Rotator(0, 0, 0)
        )
        
        if moon_marker:
            moon_marker.set_actor_label('LandingZone_Moon')
            moon_marker.set_actor_scale3d(unreal.Vector(20, 20, 0.1))
            markers_created.append(moon_marker)
            log_info("✓ Moon landing zone marker created")
        
        log_warning("MANUAL STEP: Apply emissive landing zone material to markers")
        return markers_created
        
    except Exception as e:
        log_warning(f"Landing zone marker creation failed: {e}")
        return markers_created

def setup_world_settings():
    """Configure world settings for the mission"""
    log_info("Configuring world settings...")
    
    try:
        world = unreal.EditorLevelLibrary.get_editor_world()
        if not world:
            log_error("Could not get editor world")
            return False
        
        # Get world settings
        world_settings = world.get_world_settings()
        if world_settings:
            # Set gravity (will be overridden on moon)
            world_settings.set_editor_property('global_gravity_z', -980.0)
            log_info("✓ World gravity set to -980 (Earth-like)")
        
        log_warning("MANUAL STEP: Set Default Game Mode in Project Settings")
        log_warning("  Project Settings > Maps & Modes > Default GameMode")
        log_warning("  Select: BP_FirstMissionGameMode")
        
        return True
    except Exception as e:
        log_error(f"Failed to configure world settings: {e}")
        return False

def create_spaceship_placeholder():
    """Create placeholder for spaceship"""
    log_info("Creating spaceship placeholder...")
    
    try:
        # Create cube as placeholder
        ship = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.StaticMeshActor,
            unreal.Vector(500, 0, 100),  # Near player start
            unreal.Rotator(0, 0, 0)
        )
        
        if ship:
            ship.set_actor_label('Spaceship_Placeholder')
            ship.set_actor_scale3d(unreal.Vector(5, 10, 3))  # Ship-like proportions
            log_info("✓ Spaceship placeholder created")
            log_warning("MANUAL STEP: Replace with actual Spaceship BP")
            log_warning("MANUAL STEP: Add ShipEntryComponent to spaceship")
            log_warning("  Select spaceship > Details > Add Component > ShipEntryComponent")
            return ship
        else:
            log_warning("Could not create spaceship placeholder")
            return None
    except Exception as e:
        log_warning(f"Spaceship placeholder creation failed: {e}")
        return None

def save_level():
    """Save the current level"""
    log_info("Saving level...")
    
    try:
        success = unreal.EditorLevelLibrary.save_current_level()
        if success:
            log_info("✓ Level saved successfully")
        else:
            log_warning("Level save reported failure")
        return success
    except Exception as e:
        log_error(f"Failed to save level: {e}")
        return False

def print_manual_steps():
    """Print remaining manual steps"""
    log_info("=" * 60)
    log_info("LEVEL CREATION COMPLETE")
    log_info("=" * 60)
    log_info("")
    log_info("⚠️ CRITICAL NEXT STEP:")
    log_info("  python remote_client.py --file \"trigger_lighting_build.py\"")
    log_info("")
    log_info("MANUAL STEPS REQUIRED:")
    log_info("")
    log_info("1. Replace Placeholders with Actual Actors:")
    log_info("   - Delete 'Planet_Placeholder'")
    log_info("   - Drag actual Planet BP to (0, 0, -50000)")
    log_info("   - Delete 'Moon_Placeholder'")
    log_info("   - Drag actual Moon BP to (5000000, 0, 0)")
    log_info("   - Delete 'Spaceship_Placeholder'")
    log_info("   - Drag actual Spaceship BP to (500, 0, 100)")
    log_info("")
    log_info("2. Add ShipEntryComponent to Spaceship:")
    log_info("   - Select spaceship in level")
    log_info("   - Details panel > Add Component > ShipEntryComponent")
    log_info("   - Configure entry points (cockpit front, emergency side)")
    log_info("")
    log_info("3. Add LandingGuidanceSystem Actors:")
    log_info("   - Drag 2x LandingGuidanceSystem into level")
    log_info("   - Place near landing zone markers")
    log_info("   - Configure Target Planet references")
    log_info("")
    log_info("4. Apply Materials:")
    log_info("   - Landing zone markers: Emissive material")
    log_info("   - Planet/Moon: Appropriate terrain materials")
    log_info("   - Spaceship: Ship hull material")
    log_info("")
    log_info("5. Configure Project Settings:")
    log_info("   - Project Settings > Maps & Modes")
    log_info("   - Default GameMode: BP_FirstMissionGameMode")
    log_info("   - Default Pawn Class: BP_SurfacePlayer")
    log_info("   - Editor Startup Map: MissionLevel")
    log_info("   - Game Default Map: MissionLevel")
    log_info("")
    log_info("6. Test Level:")
    log_info("   - Press Alt+P to Play In Editor")
    log_info("   - Verify player spawns correctly")
    log_info("   - Check if ship is visible and accessible")
    log_info("")
    log_info("📖 See UNREAL_EDITOR_SETUP_GUIDE.md Phase 6 for details")
    log_info("=" * 60)

def main():
    """Main level setup function"""
    log_info("Starting Mission Level Setup...")
    log_info("=" * 60)
    
    # Step 1: Create new level
    if not create_new_level():
        log_error("Failed to create level. Aborting.")
        return False
    
    # Step 2: Add basic lighting
    sun = add_directional_light()
    
    # Step 3: Add atmosphere
    sky = add_sky_atmosphere()
    
    # Step 4: Add post process
    ppv = add_post_process_volume()
    
    # Step 5: Add player start
    player_start = add_player_start()
    if not player_start:
        log_error("Critical: Could not create player start!")
    
    # Step 6: Create planet placeholder
    planet = create_planet_placeholder()
    
    # Step 7: Create moon placeholder
    moon = create_moon_placeholder()
    
    # Step 8: Create landing zone markers
    markers = create_landing_zone_marker()
    
    # Step 9: Create spaceship placeholder
    ship = create_spaceship_placeholder()
    
    # Step 10: Configure world settings
    setup_world_settings()
    
    # Step 11: Save level
    save_level()
    
    # Step 12: Print manual steps
    print_manual_steps()
    
    log_info("Level setup script completed!")
    log_info("Remember to trigger lighting build next!")
    return True

# Execute main function
if __name__ == '__main__':
    main()
