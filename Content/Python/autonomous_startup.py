"""
AUTONOMOUS STARTUP SCRIPT
This runs automatically when Unreal starts and performs validation
"""
import unreal
import json
import os
import time
from datetime import datetime

def autonomous_validation():
    """Run autonomous validation and save results"""
    
    # Mark start
    start_file = "C:/Users/allen/Desktop/Alexander/autonomous_start_marker.txt"
    with open(start_file, 'w') as f:
        f.write(f"Autonomous validation started at {datetime.now().isoformat()}")
    
    results = {
        'timestamp': datetime.now().isoformat(),
        'validation_type': 'direct_autonomous',
        'environment': {},
        'screenshot': {},
        'status': 'running'
    }
    
    try:
        unreal.log("AUTONOMOUS: Starting validation...")
        
        # Get level info
        current_world = unreal.EditorLevelLibrary.get_editor_world()
        level_name = current_world.get_name() if current_world else "Unknown"
        
        # Get all actors
        all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
        
        # Count different types
        cameras = [a for a in all_actors if isinstance(a, unreal.CameraActor)]
        text_actors = [a for a in all_actors if isinstance(a, unreal.TextRenderActor)]
        static_meshes = [a for a in all_actors if isinstance(a, unreal.StaticMeshActor)]
        
        # Find test cameras
        test_cameras = []
        required_cameras = ["Cam1_Overview_North", "Cam2_Detail_East", "Cam3_Detail_West", 
                          "Cam4_Character_Showcase", "Cam5_Isometric"]
        
        for cam_name in required_cameras:
            found = False
            for actor in all_actors:
                if actor and cam_name in str(actor.get_name()):
                    test_cameras.append(cam_name)
                    found = True
                    break
            if not found:
                break
        
        # Store environment results
        results['environment'] = {
            'level_name': level_name,
            'total_actors': len(all_actors),
            'cameras_total': len(cameras),
            'test_cameras_found': len(test_cameras),
            'text_labels': len(text_actors),
            'static_meshes': len(static_meshes),
            'required_cameras': required_cameras,
            'found_cameras': test_cameras
        }
        
        unreal.log(f"AUTONOMOUS: Found {len(test_cameras)}/5 test cameras")
        unreal.log(f"AUTONOMOUS: Level has {len(all_actors)} total actors")
        
        # Take screenshot
        try:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = f"autonomous_{timestamp}"
            command = f"HighResShot 1920x1080 filename={filename}"
            
            # Focus on first available camera if possible
            if test_cameras and cameras:
                for actor in all_actors:
                    if actor and test_cameras[0] in str(actor.get_name()):
                        unreal.EditorLevelLibrary.set_selected_level_actors([actor])
                        break
            
            unreal.SystemLibrary.execute_console_command(None, command)
            
            results['screenshot'] = {
                'filename': f"{filename}.png",
                'timestamp': timestamp,
                'camera_focused': test_cameras[0] if test_cameras else 'none'
            }
            
            unreal.log(f"AUTONOMOUS: Screenshot taken: {filename}.png")
            
        except Exception as e:
            unreal.log_error(f"AUTONOMOUS: Screenshot error: {e}")
            results['screenshot'] = {'error': str(e)}
        
        # Determine validation status
        if len(test_cameras) == 5 and len(static_meshes) >= 15:
            results['status'] = 'passed'
            unreal.log("AUTONOMOUS: Validation PASSED - Environment complete!")
        elif len(test_cameras) >= 3:
            results['status'] = 'partial'
            unreal.log("AUTONOMOUS: Validation PARTIAL - Some components missing")
        else:
            results['status'] = 'failed'
            unreal.log("AUTONOMOUS: Validation FAILED - Environment incomplete")
        
    except Exception as e:
        unreal.log_error(f"AUTONOMOUS: Critical error: {e}")
        results['status'] = 'error'
        results['error'] = str(e)
    
    # Save results
    results_file = "C:/Users/allen/Desktop/Alexander/direct_autonomous_results.json"
    try:
        with open(results_file, 'w') as f:
            json.dump(results, f, indent=2)
        unreal.log(f"AUTONOMOUS: Results saved to {results_file}")
    except Exception as e:
        unreal.log_error(f"AUTONOMOUS: Could not save results: {e}")
    
    # Mark completion
    completion_file = "C:/Users/allen/Desktop/Alexander/autonomous_complete_marker.txt"
    with open(completion_file, 'w') as f:
        f.write(f"Autonomous validation completed at {datetime.now().isoformat()}\n")
        f.write(f"Status: {results['status']}\n")
        f.write(f"Cameras: {len(test_cameras)}/5\n")
        f.write(f"Objects: {len(static_meshes)} static meshes\n")
    
    unreal.log("AUTONOMOUS: Validation sequence complete!")
    return results

# Auto-run when Unreal starts
try:
    autonomous_validation()
except Exception as e:
    unreal.log_error(f"AUTONOMOUS STARTUP ERROR: {e}")
