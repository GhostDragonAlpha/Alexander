"""
Post-Startup Screenshot System - Uses Unreal's slate tick system
Captures screenshots AFTER editor is fully loaded and interactive
Author: AI Vision System
Date: November 3, 2025
"""

import unreal
import time

# Global state for screenshot capture
screenshot_state = {
    'active': False,
    'current_index': 0,
    'total_count': 0,
    'capture_list': [],
    'last_capture_time': 0,
    'delay_between_shots': 2.0  # 2 seconds between screenshots
}


def setup_screenshot_queue():
    """Prepare the list of all screenshots to capture"""
    
    screenshot_state['capture_list'] = []
    
    # Celestial bodies
    celestial_names = [
        "Sun", "Mercury", "Venus", "Earth", "Mars",
        "Jupiter", "Saturn", "Uranus", "Neptune",
        "Moon", "Phobos", "Deimos",
        "Io", "Europa", "Ganymede", "Callisto"
    ]
    
    for body in celestial_names:
        screenshot_state['capture_list'].append({
            'type': 'planet',
            'name': body,
            'filename': f"planet_{body.lower()}"
        })
    
    # Cockpit components
    cockpit_parts = [
        "Cockpit_Base", "Pilot_Seat", "FlightStick", "Throttle",
        "ButtonPanel", "HUD_Left", "HUD_Right", "VR_CameraPosition"
    ]
    
    for part in cockpit_parts:
        screenshot_state['capture_list'].append({
            'type': 'cockpit',
            'name': part,
            'filename': f"cockpit_{part.lower()}"
        })
    
    # Overview shots
    screenshot_state['capture_list'].append({'type': 'overview', 'name': 'Solar System Wide', 'filename': 'solar_system_wide'})
    screenshot_state['capture_list'].append({'type': 'overview', 'name': 'Inner Planets', 'filename': 'solar_system_inner'})
    screenshot_state['capture_list'].append({'type': 'overview', 'name': 'Outer Planets', 'filename': 'solar_system_outer'})
    screenshot_state['capture_list'].append({'type': 'overview', 'name': 'Cockpit Overview', 'filename': 'cockpit_overview'})
    
    # Reference grid
    screenshot_state['capture_list'].append({'type': 'grid', 'name': 'Reference Grid Overview', 'filename': 'reference_grid_overview'})
    screenshot_state['capture_list'].append({'type': 'grid', 'name': 'Reference Grid Close', 'filename': 'reference_grid_close'})
    
    screenshot_state['total_count'] = len(screenshot_state['capture_list'])
    
    print(f"\n📋 Screenshot queue prepared: {screenshot_state['total_count']} items")


def slate_tick_callback(delta_time):
    """Called every frame by Unreal's slate tick - this is where we capture screenshots"""
    
    if not screenshot_state['active']:
        return
    
    current_time = time.time()
    
    # Check if enough time has passed since last capture
    if current_time - screenshot_state['last_capture_time'] < screenshot_state['delay_between_shots']:
        return
    
    # Get current screenshot to capture
    if screenshot_state['current_index'] >= screenshot_state['total_count']:
        # All done!
        screenshot_state['active'] = False
        unreal.unregister_slate_post_tick_callback(slate_tick_callback)
        print("\n" + "="*80)
        print("✅ ALL SCREENSHOTS CAPTURED SUCCESSFULLY!")
        print("="*80)
        print(f"📊 Total: {screenshot_state['total_count']} screenshots")
        print("📂 Saved to: Saved/Screenshots/WindowsEditor/")
        return
    
    # Capture current screenshot
    item = screenshot_state['capture_list'][screenshot_state['current_index']]
    
    print(f"\n[{screenshot_state['current_index'] + 1}/{screenshot_state['total_count']}] 📸 Capturing: {item['name']}...")
    
    # Execute HighResShot command
    command = f"HighResShot 1920x1080 filename={item['filename']}"
    unreal.SystemLibrary.execute_console_command(None, command)
    
    # Update state
    screenshot_state['current_index'] += 1
    screenshot_state['last_capture_time'] = current_time


def start_screenshot_capture():
    """Start the screenshot capture process using slate tick"""
    
    print("\n" + "="*80)
    print("🚀 STARTING POST-STARTUP SCREENSHOT SYSTEM")
    print("="*80)
    print("\nThis system captures screenshots AFTER editor is fully loaded")
    print("Using Unreal's slate tick callback for proper timing")
    
    # Setup the queue
    setup_screenshot_queue()
    
    # Reset state
    screenshot_state['active'] = True
    screenshot_state['current_index'] = 0
    screenshot_state['last_capture_time'] = time.time()
    
    # Register slate tick callback
    unreal.register_slate_post_tick_callback(slate_tick_callback)
    
    print(f"\n⏱️  Capturing 1 screenshot every {screenshot_state['delay_between_shots']} seconds")
    print(f"⏱️  Estimated time: {screenshot_state['total_count'] * screenshot_state['delay_between_shots']:.0f} seconds")
    print("\n✅ Screenshot system activated!")
    print("="*80)


# Main execution
if __name__ == "__main__":
    start_screenshot_capture()
