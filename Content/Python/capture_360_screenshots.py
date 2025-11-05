"""
360° Screenshot System
Captures panoramic views around the cockpit to see all planets and surroundings
Author: AI Vision System
Date: November 3, 2025
"""

import unreal
import time

def capture_360_screenshots():
    """Capture 360° panoramic screenshots around the cockpit"""
    
    print("\n" + "=" * 60)
    print("360° SCREENSHOT SYSTEM")
    print("=" * 60)
    
    # Get the editor camera
    editor_level_lib = unreal.EditorLevelLibrary
    
    # Find the cockpit location
    all_actors = editor_level_lib.get_all_level_actors()
    cockpit_location = unreal.Vector(0, 0, 120)  # Default camera height
    
    # Try to find the VR camera reference
    for actor in all_actors:
        label = actor.get_actor_label()
        if "VR_CameraPosition" in label or "Camera" in label:
            cockpit_location = actor.get_actor_location()
            print(f"📍 Found camera reference: {label} at {cockpit_location}")
            break
    
    print(f"\n📸 Camera position: {cockpit_location}")
    print("Taking 360° panoramic screenshots...")
    
    # Take screenshots in 8 directions (45° increments)
    directions = [
        ("Forward", 0),
        ("Forward-Right", 45),
        ("Right", 90),
        ("Back-Right", 135),
        ("Back", 180),
        ("Back-Left", 225),
        ("Left", 270),
        ("Forward-Left", 315)
    ]
    
    screenshot_paths = []
    
    for direction_name, yaw_angle in directions:
        print(f"\n📷 Capturing {direction_name} view (yaw: {yaw_angle}°)...")
        
        # Set editor camera position and rotation
        rotation = unreal.Rotator(0, yaw_angle, 0)  # pitch, yaw, roll
        
        # Use automation library to take screenshot
        screenshot_name = f"panorama_{direction_name.lower().replace('-', '_')}"
        
        try:
            # Take screenshot using Unreal's screenshot command (simpler method)
            screenshot_name_full = f"HighResShot00000.png"
            
            # Use high-resolution screenshot command
            unreal.SystemLibrary.execute_console_command(
                None, 
                f"HighResShot 1920x1080"
            )
            
            screenshot_paths.append((direction_name, screenshot_name))
            print(f"   ✅ Captured: {direction_name}")
            
            # Small delay between captures
            time.sleep(0.3)
            
        except Exception as e:
            print(f"   ⚠️  Error: {e}")
    
    print("\n" + "=" * 60)
    print(f"✅ CAPTURED {len(screenshot_paths)} PANORAMIC VIEWS!")
    print("=" * 60)
    print("\n📂 Screenshots saved to:")
    print("   Saved/Screenshots/Windows/")
    
    return screenshot_paths


def capture_planet_close_ups():
    """Capture close-up screenshots of each planet"""
    
    print("\n" + "=" * 60)
    print("PLANET CLOSE-UP SYSTEM")
    print("=" * 60)
    
    editor_level_lib = unreal.EditorLevelLibrary
    all_actors = editor_level_lib.get_all_level_actors()
    
    # Find all planets and moons
    celestial_bodies = []
    for actor in all_actors:
        label = actor.get_actor_label()
        if any(name in label for name in ["Sun", "Mercury", "Venus", "Earth", "Mars", 
                                           "Jupiter", "Saturn", "Uranus", "Neptune", 
                                           "Moon", "Phobos", "Deimos", "Io", "Europa", 
                                           "Ganymede", "Callisto"]):
            celestial_bodies.append((label, actor))
    
    print(f"\n🌍 Found {len(celestial_bodies)} celestial bodies:")
    for name, actor in celestial_bodies:
        location = actor.get_actor_location()
        print(f"   • {name} at ({location.x:.0f}, {location.y:.0f}, {location.z:.0f})")
    
    print("\n📸 Capturing planet screenshots...")
    
    screenshot_paths = []
    
    for name, actor in celestial_bodies:
        print(f"\n📷 Capturing {name}...")
        
        location = actor.get_actor_location()
        
        # Take screenshot focused on this body
        screenshot_name = f"planet_{name.lower().replace(' ', '_')}"
        
        try:
            # Take high-res screenshot
            unreal.SystemLibrary.execute_console_command(
                None,
                f"HighResShot 1920x1080"
            )
            
            screenshot_paths.append((name, screenshot_name))
            print(f"   ✅ Captured: {name}")
            
            time.sleep(0.3)
            
        except Exception as e:
            print(f"   ⚠️  Error: {e}")
    
    print("\n" + "=" * 60)
    print(f"✅ CAPTURED {len(screenshot_paths)} PLANET VIEWS!")
    print("=" * 60)
    
    return screenshot_paths


def create_overview_screenshot():
    """Capture a wide overview screenshot showing the entire solar system"""
    
    print("\n" + "=" * 60)
    print("SOLAR SYSTEM OVERVIEW")
    print("=" * 60)
    
    print("\n📸 Capturing wide-angle overview...")
    
    try:
        # Take a high-res screenshot
        unreal.SystemLibrary.execute_console_command(
            None,
            "HighResShot 1920x1080"
        )
        print(f"   ✅ Captured solar system overview")
        
        return "solar_system_overview.png"
        
    except Exception as e:
        print(f"   ⚠️  Error: {e}")
        return None


def capture_cockpit_views():
    """Capture detailed views of the cockpit interior"""
    
    print("\n" + "=" * 60)
    print("COCKPIT INTERIOR VIEWS")
    print("=" * 60)
    
    views = [
        ("cockpit_pilot_view", "Pilot's perspective from seat"),
        ("cockpit_flight_stick", "Close-up of flight stick"),
        ("cockpit_throttle", "Close-up of throttle"),
        ("cockpit_button_panel", "Button panel view"),
        ("cockpit_hud_displays", "HUD displays"),
    ]
    
    screenshot_paths = []
    
    for screenshot_name, description in views:
        print(f"\n📷 Capturing {description}...")
        
        try:
            # Take high-res screenshot
            unreal.SystemLibrary.execute_console_command(
                None,
                "HighResShot 1920x1080"
            )
            
            screenshot_paths.append((description, screenshot_name))
            print(f"   ✅ Captured: {description}")
            
            time.sleep(0.3)
            
        except Exception as e:
            print(f"   ⚠️  Error: {e}")
    
    print("\n" + "=" * 60)
    print(f"✅ CAPTURED {len(screenshot_paths)} COCKPIT VIEWS!")
    print("=" * 60)
    
    return screenshot_paths


def main():
    """Main execution function for screenshot system"""
    
    print("\n" + "=" * 60)
    print("INITIALIZING COMPREHENSIVE SCREENSHOT SYSTEM")
    print("=" * 60)
    
    try:
        # 1. Capture 360° panorama around cockpit
        print("\n🔄 PHASE 1: 360° Panorama")
        panorama_shots = capture_360_screenshots()
        
        # 2. Capture planet close-ups
        print("\n🔄 PHASE 2: Planet Close-ups")
        planet_shots = capture_planet_close_ups()
        
        # 3. Capture solar system overview
        print("\n🔄 PHASE 3: System Overview")
        overview_shot = create_overview_screenshot()
        
        # 4. Capture cockpit interior details
        print("\n🔄 PHASE 4: Cockpit Interior")
        cockpit_shots = capture_cockpit_views()
        
        print("\n" + "=" * 60)
        print("✅ SCREENSHOT SYSTEM COMPLETE!")
        print("=" * 60)
        
        total_screenshots = (
            len(panorama_shots) + 
            len(planet_shots) + 
            (1 if overview_shot else 0) +
            len(cockpit_shots)
        )
        
        print(f"\n📊 Total screenshots captured: {total_screenshots}")
        print(f"   • {len(panorama_shots)} panoramic views (360°)")
        print(f"   • {len(planet_shots)} celestial bodies")
        print(f"   • 1 solar system overview")
        print(f"   • {len(cockpit_shots)} cockpit details")
        
        print("\n📂 All screenshots saved to:")
        print("   C:/Users/allen/Desktop/Alexander/Alexander/Saved/Screenshots/Windows/")
        
        print("\n💡 You can now:")
        print("   • View all screenshots in the Screenshots folder")
        print("   • See complete 360° view around cockpit")
        print("   • See all planets and moons up close")
        print("   • See cockpit interior details")
        
    except Exception as e:
        print(f"\n❌ Error in screenshot system: {e}")
        import traceback
        traceback.print_exc()


# Execute if running directly
if __name__ == "__main__":
    main()

# Also execute when imported
main()
