"""
AUTOMATED VALIDATION - UNREAL SIDE
This script runs inside Unreal Engine to perform automated validation
Can be triggered via remote execution or manual execution
"""
import unreal
import json
import os
import time
from datetime import datetime

class UnrealAutomatedValidator:
    def __init__(self):
        self.results_dir = "C:/Users/allen/Desktop/Alexander"
        self.screenshots_dir = "C:/Users/allen/Desktop/Alexander/Alexander/Saved/Screenshots/WindowsEditor"
    
    def log_to_file(self, message):
        """Log message to external file for monitoring"""
        log_file = os.path.join(self.results_dir, "unreal_validation.log")
        timestamp = datetime.now().strftime("%H:%M:%S")
        
        # Clean message of unicode characters for Windows compatibility
        clean_message = message.encode('ascii', 'ignore').decode('ascii')
        
        try:
            with open(log_file, 'a', encoding='utf-8') as f:
                f.write(f"[{timestamp}] {clean_message}\n")
        except:
            pass  # Fail silently if can't write to file
        
        # Also log to Unreal console (use clean message)
        unreal.log(f"[AUTO-VALIDATE] {clean_message}")
    
    def validate_test_environment(self):
        """Comprehensive automated validation of test environment"""
        self.log_to_file("Starting automated environment validation...")
        
        results = {
            'timestamp': datetime.now().isoformat(),
            'validation_type': 'automated',
            'cameras': {
                'found': [],
                'missing': [],
                'total_required': 5
            },
            'scene_objects': {
                'total_actors': 0,
                'text_labels': 0,
                'static_meshes': 0,
                'lights': 0,
                'camera_actors': 0
            },
            'lighting': {
                'sun_light': False,
                'sky_light': False,
                'post_process_volume': False
            },
            'zones': {
                'reference_grid': False,
                'material_test': False,
                'object_library': False,
                'before_after': False
            },
            'issues': [],
            'warnings': [],
            'status': 'unknown'
        }
        
        try:
            # Get all actors in the level
            all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
            results['scene_objects']['total_actors'] = len(all_actors)
            self.log_to_file(f"Found {len(all_actors)} total actors in level")
            
            # Check required cameras
            required_cameras = [
                "Cam1_Overview_North",
                "Cam2_Detail_East", 
                "Cam3_Detail_West",
                "Cam4_Character_Showcase",
                "Cam5_Isometric"
            ]
            
            for cam_name in required_cameras:
                found = False
                for actor in all_actors:
                    if actor and cam_name in str(actor.get_name()):
                        results['cameras']['found'].append(cam_name)
                        found = True
                        break
                
                if not found:
                    results['cameras']['missing'].append(cam_name)
            
            # Count different actor types
            for actor in all_actors:
                if actor:
                    actor_type = type(actor).__name__
                    
                    if isinstance(actor, unreal.TextRenderActor):
                        results['scene_objects']['text_labels'] += 1
                    elif isinstance(actor, unreal.StaticMeshActor):
                        results['scene_objects']['static_meshes'] += 1
                    elif isinstance(actor, unreal.CameraActor):
                        results['scene_objects']['camera_actors'] += 1
                    elif 'Light' in actor_type:
                        results['scene_objects']['lights'] += 1
                        
                        # Check specific light types
                        if 'Directional' in actor_type or 'Sun' in str(actor.get_name()):
                            results['lighting']['sun_light'] = True
                        elif 'Sky' in actor_type or 'Sky' in str(actor.get_name()):
                            results['lighting']['sky_light'] = True
                    
                    elif isinstance(actor, unreal.PostProcessVolume):
                        results['lighting']['post_process_volume'] = True
                    
                    # Check for zone indicators by name/label
                    actor_name = str(actor.get_name()).upper()
                    if 'REFERENCE' in actor_name or 'GRID' in actor_name:
                        results['zones']['reference_grid'] = True
                    elif 'MATERIAL' in actor_name:
                        results['zones']['material_test'] = True
                    elif 'OBJECT' in actor_name or 'LIBRARY' in actor_name:
                        results['zones']['object_library'] = True
                    elif 'BEFORE' in actor_name or 'AFTER' in actor_name:
                        results['zones']['before_after'] = True
            
            # Analyze results and determine status
            cameras_found = len(results['cameras']['found'])
            cameras_required = results['cameras']['total_required']
            
            if cameras_found == cameras_required:
                self.log_to_file(f"✅ All {cameras_required} cameras found")
            else:
                issue_msg = f"Only {cameras_found}/{cameras_required} cameras found"
                results['issues'].append(issue_msg)
                self.log_to_file(f"❌ {issue_msg}")
            
            # Check minimum object counts
            if results['scene_objects']['static_meshes'] < 15:
                warning_msg = f"Only {results['scene_objects']['static_meshes']} static meshes found (expected ~20+)"
                results['warnings'].append(warning_msg)
                self.log_to_file(f"⚠️ {warning_msg}")
            
            if results['scene_objects']['text_labels'] < 20:
                warning_msg = f"Only {results['scene_objects']['text_labels']} text labels found (expected ~50+)"
                results['warnings'].append(warning_msg)
                self.log_to_file(f"⚠️ {warning_msg}")
            
            # Check lighting
            if not results['lighting']['sun_light']:
                results['issues'].append("Missing directional light (sun)")
            if not results['lighting']['sky_light']:
                results['issues'].append("Missing sky light")
            
            # Determine overall status
            if len(results['issues']) == 0:
                if len(results['warnings']) == 0:
                    results['status'] = 'passed'
                    self.log_to_file("✅ VALIDATION PASSED - Environment is complete!")
                else:
                    results['status'] = 'passed_with_warnings'
                    self.log_to_file("⚠️ VALIDATION PASSED WITH WARNINGS - Minor issues found")
            else:
                results['status'] = 'failed'
                self.log_to_file("❌ VALIDATION FAILED - Critical issues found")
            
            # Save results to file for external monitoring
            results_file = os.path.join(self.results_dir, "unreal_validation_results.json")
            with open(results_file, 'w') as f:
                json.dump(results, f, indent=2)
            
            self.log_to_file(f"Results saved to: {results_file}")
            
            # Display summary in Unreal console
            self.log_to_file("=" * 50)
            self.log_to_file("AUTOMATED VALIDATION SUMMARY:")
            self.log_to_file(f"Status: {results['status']}")
            self.log_to_file(f"Cameras: {cameras_found}/{cameras_required}")
            self.log_to_file(f"Static Meshes: {results['scene_objects']['static_meshes']}")
            self.log_to_file(f"Text Labels: {results['scene_objects']['text_labels']}")
            self.log_to_file(f"Lights: {results['scene_objects']['lights']}")
            self.log_to_file("=" * 50)
            
            return results
            
        except Exception as e:
            error_msg = f"Validation error: {str(e)}"
            results['status'] = 'error'
            results['issues'].append(error_msg)
            self.log_to_file(f"❌ {error_msg}")
            return results
    
    def take_automated_screenshot(self, camera_name=None):
        """Take automated screenshot, optionally from specific camera"""
        self.log_to_file("Taking automated screenshot...")
        
        try:
            # If camera specified, try to focus on it first
            if camera_name:
                all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
                for actor in all_actors:
                    if actor and camera_name in str(actor.get_name()):
                        unreal.EditorLevelLibrary.set_selected_level_actors([actor])
                        self.log_to_file(f"Focused on camera: {camera_name}")
                        break
            
            # Generate filename
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = f"auto_validation_{timestamp}"
            
            # Take high-resolution screenshot
            command = f"HighResShot 1920x1080 filename={filename}"
            unreal.SystemLibrary.execute_console_command(None, command)
            
            self.log_to_file(f"Screenshot command executed: {filename}.png")
            
            # Save screenshot info for external monitoring
            screenshot_info = {
                'timestamp': datetime.now().isoformat(),
                'filename': f"{filename}.png",
                'camera': camera_name or "current_view",
                'resolution': "1920x1080",
                'path': f"{self.screenshots_dir}/{filename}.png"
            }
            
            info_file = os.path.join(self.results_dir, "last_screenshot_info.json")
            with open(info_file, 'w') as f:
                json.dump(screenshot_info, f, indent=2)
            
            return screenshot_info
            
        except Exception as e:
            self.log_to_file(f"❌ Screenshot error: {str(e)}")
            return None
    
    def run_complete_automated_validation(self):
        """Run complete automated validation sequence"""
        self.log_to_file("🚀 STARTING COMPLETE AUTOMATED VALIDATION")
        
        # Step 1: Validate environment
        validation_results = self.validate_test_environment()
        
        # Step 2: Take screenshot from overview camera if available
        camera_to_use = None
        if "Cam1_Overview_North" in validation_results['cameras']['found']:
            camera_to_use = "Cam1_Overview_North"
        elif validation_results['cameras']['found']:
            camera_to_use = validation_results['cameras']['found'][0]
        
        screenshot_info = self.take_automated_screenshot(camera_to_use)
        
        # Step 3: Compile final results
        final_results = {
            'timestamp': datetime.now().isoformat(),
            'validation': validation_results,
            'screenshot': screenshot_info,
            'automation_complete': True,
            'ready_for_ai_testing': validation_results['status'] in ['passed', 'passed_with_warnings']
        }
        
        # Save comprehensive results
        final_file = os.path.join(self.results_dir, "automated_validation_complete.json")
        with open(final_file, 'w') as f:
            json.dump(final_results, f, indent=2)
        
        self.log_to_file("🎉 AUTOMATED VALIDATION SEQUENCE COMPLETE!")
        self.log_to_file(f"Final results: {final_file}")
        
        # Show dialog with results
        if validation_results['status'] == 'passed':
            message = "✅ Automated Validation PASSED!\n\nEnvironment is ready for AI testing."
        elif validation_results['status'] == 'passed_with_warnings':
            message = "⚠️ Automated Validation PASSED with warnings.\n\nEnvironment is mostly ready, check logs for details."
        else:
            message = "❌ Automated Validation FAILED.\n\nEnvironment needs rebuilding or fixes."
        
        unreal.EditorDialog.show_message(
            unreal.Text("Automated Validation Complete"),
            unreal.Text(message),
            unreal.AppMsgType.OK
        )
        
        return final_results

# Create validator instance
validator = UnrealAutomatedValidator()

# Auto-run complete validation when script is executed
validator.run_complete_automated_validation()