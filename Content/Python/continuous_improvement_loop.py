# Continuous Improvement Loop - Autonomous VR Game Development
# This script runs continuously to analyze, improve, and evolve the VR space farming game

import unreal
import time
import json
import requests
import os
from datetime import datetime
import threading

class ContinuousImprovementSystem:
    def __init__(self):
        self.backend_url = "http://localhost:8000"
        self.iteration_count = 0
        self.previous_feedback = ""
        self.improvement_active = True
        self.screenshot_interval = 30  # seconds between improvements
        self.development_goal = "Build a VR space farming game like Star Citizen with continuous autonomous development"
        
    def capture_screenshot(self):
        """Capture current viewport screenshot"""
        try:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = f"continuous_improvement_{timestamp}"
            
            # Focus on interesting areas
            self.focus_viewport()
            
            # Capture screenshot
            command = f"HighResShot 1920x1080 filename={filename}"
            unreal.SystemLibrary.execute_console_command(None, command)
            
            # Wait for async save
            time.sleep(2)
            
            # Construct full path
            project_dir = unreal.SystemLibrary.get_project_directory()
            screenshot_path = os.path.join(project_dir, "Alexander", "Saved", "Screenshots", "WindowsEditor", f"{filename}.png")
            
            if os.path.exists(screenshot_path):
                print(f"📸 Screenshot captured: {filename}")
                return screenshot_path
            else:
                print(f"⚠️ Screenshot file not found: {screenshot_path}")
                return None
                
        except Exception as e:
            print(f"❌ Screenshot capture failed: {e}")
            return None
    
    def focus_viewport(self):
        """Focus viewport on most interesting area"""
        try:
            all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
            
            # Priority focus areas
            focus_targets = []
            
            # Look for VR cockpit components
            for actor in all_actors:
                label = actor.get_actor_label().lower()
                if any(keyword in label for keyword in ['cockpit', 'vr', 'player', 'control']):
                    focus_targets.append(actor)
            
            # Look for any interactive elements
            if not focus_targets:
                for actor in all_actors:
                    if isinstance(actor, unreal.StaticMeshActor):
                        focus_targets.append(actor)
                        if len(focus_targets) >= 3:
                            break
            
            # Select first target to focus viewport
            if focus_targets:
                unreal.EditorLevelLibrary.set_selected_level_actors([focus_targets[0]])
                print(f"🎯 Focused viewport on: {focus_targets[0].get_actor_label()}")
                
        except Exception as e:
            print(f"⚠️ Viewport focus failed: {e}")
    
    def get_scene_metadata(self):
        """Collect metadata about current scene"""
        try:
            all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
            
            metadata = {
                "level_name": "Unknown",
                "actor_count": len(all_actors),
                "timestamp": datetime.now().isoformat(),
                "iteration": self.iteration_count,
                "actors_by_type": {}
            }
            
            # Count actors by type
            for actor in all_actors:
                class_name = actor.get_class().get_name()
                if class_name not in metadata["actors_by_type"]:
                    metadata["actors_by_type"][class_name] = 0
                metadata["actors_by_type"][class_name] += 1
            
            # Get current level name
            try:
                current_level = unreal.EditorLevelLibrary.get_editor_world()
                if current_level:
                    metadata["level_name"] = current_level.get_name()
            except:
                pass
            
            return metadata
            
        except Exception as e:
            print(f"⚠️ Metadata collection failed: {e}")
            return {"error": str(e)}
    
    def send_to_ai_backend(self, screenshot_path, metadata):
        """Send screenshot to AI backend for analysis"""
        try:
            # Prepare request
            request_data = {
                "image_path": screenshot_path,
                "metadata": json.dumps(metadata),
                "user_goal": self.development_goal,
                "previous_feedback": self.previous_feedback,
                "iteration_count": self.iteration_count
            }
            
            # Send to backend
            response = requests.post(f"{self.backend_url}/api/analyze_scene", json=request_data, timeout=30)
            
            if response.status_code == 200:
                analysis_result = response.json()
                print(f"🤖 AI Analysis complete (confidence: {analysis_result.get('confidence_score', 0):.2f})")
                return analysis_result
            else:
                print(f"❌ AI backend error: {response.status_code} - {response.text}")
                return None
                
        except Exception as e:
            print(f"❌ AI backend communication failed: {e}")
            return None
    
    def execute_ai_actions(self, analysis_result):
        """Execute the development actions suggested by AI"""
        if not analysis_result or "actions" not in analysis_result:
            return
        
        actions = analysis_result["actions"]
        print(f"⚡ Executing {len(actions)} AI-suggested actions...")
        
        for i, action in enumerate(actions, 1):
            try:
                print(f"   [{i}/{len(actions)}] {action['action_type']}: {action['target']}")
                print(f"       Reasoning: {action['reasoning']}")
                print(f"       Priority: {action['priority']}")
                
                # Execute the generated code
                if "executable_code" in action and action["executable_code"]:
                    try:
                        exec(action["executable_code"])
                        print(f"       ✅ Executed successfully")
                    except Exception as e:
                        print(f"       ❌ Execution failed: {e}")
                
                # Small delay between actions
                time.sleep(1)
                
            except Exception as e:
                print(f"   ❌ Action {i} failed: {e}")
        
        # Update feedback for next iteration
        if "analysis" in analysis_result:
            self.previous_feedback = analysis_result["analysis"][:500]  # Truncate to avoid context overflow
    
    def run_single_iteration(self):
        """Run one complete improvement iteration"""
        print(f"\n{'='*60}")
        print(f"🚀 CONTINUOUS IMPROVEMENT ITERATION #{self.iteration_count}")
        print(f"{'='*60}")
        
        # 1. Capture screenshot
        screenshot_path = self.capture_screenshot()
        if not screenshot_path:
            print("❌ Cannot continue without screenshot")
            return False
        
        # 2. Collect metadata
        metadata = self.get_scene_metadata()
        print(f"📊 Scene: {metadata.get('level_name')} ({metadata.get('actor_count')} actors)")
        
        # 3. Send to AI for analysis
        analysis_result = self.send_to_ai_backend(screenshot_path, metadata)
        if not analysis_result:
            print("❌ Cannot continue without AI analysis")
            return False
        
        # 4. Display AI analysis
        print(f"\n🤖 AI Analysis:")
        print(f"   Focus: {analysis_result.get('next_iteration_focus', 'Unknown')}")
        print(f"   Confidence: {analysis_result.get('confidence_score', 0):.2f}")
        
        # 5. Execute suggested improvements
        self.execute_ai_actions(analysis_result)
        
        # 6. Show suggestions
        if "suggestions" in analysis_result:
            print(f"\n💡 AI Suggestions:")
            for suggestion in analysis_result["suggestions"][:3]:
                print(f"   • {suggestion}")
        
        # 7. Update iteration count
        self.iteration_count += 1
        
        print(f"\n✅ Iteration #{self.iteration_count-1} complete")
        return True
    
    def start_continuous_loop(self):
        """Start the continuous improvement loop"""
        print("🚀 Starting Continuous Improvement Loop")
        print(f"   Backend: {self.backend_url}")
        print(f"   Interval: {self.screenshot_interval} seconds")
        print(f"   Goal: {self.development_goal}")
        print(f"   Press Ctrl+C to stop\n")
        
        try:
            while self.improvement_active:
                # Run one iteration
                success = self.run_single_iteration()
                
                if not success:
                    print("⚠️ Iteration failed, waiting before retry...")
                    time.sleep(10)  # Wait longer after failure
                    continue
                
                # Wait before next iteration
                print(f"\n⏳ Waiting {self.screenshot_interval} seconds before next iteration...")
                
                # Wait with interrupt check
                for _ in range(self.screenshot_interval):
                    if not self.improvement_active:
                        break
                    time.sleep(1)
                    
        except KeyboardInterrupt:
            print("\n🛑 Continuous improvement stopped by user")
        except Exception as e:
            print(f"\n❌ Continuous improvement error: {e}")
        finally:
            print("🏁 Continuous improvement loop ended")
    
    def stop(self):
        """Stop the continuous improvement loop"""
        self.improvement_active = False

def start_continuous_improvement():
    """Start the continuous improvement system"""
    print("🤖 INITIALIZING CONTINUOUS IMPROVEMENT SYSTEM")
    print("=" * 60)
    
    # Test AI backend connection
    try:
        response = requests.get("http://localhost:8000/api/health", timeout=5)
        if response.status_code == 200:
            print("✅ AI Backend connection verified")
        else:
            print("⚠️ AI Backend not responding properly")
    except Exception as e:
        print(f"❌ Cannot connect to AI Backend: {e}")
        print("   Please start the AI Backend first")
        return
    
    # Start the continuous improvement system
    cis = ContinuousImprovementSystem()
    
    # Run in a separate thread to avoid blocking Unreal
    improvement_thread = threading.Thread(target=cis.start_continuous_loop, daemon=True)
    improvement_thread.start()
    
    print("✅ Continuous improvement system started in background")
    print("   Check the Python console for real-time progress")
    print("   Use Ctrl+C in Python console to stop")

# Auto-start when imported
if __name__ == "__main__":
    start_continuous_improvement()
else:
    # Start after a delay when imported by init_unreal.py
    import threading
    import time
    
    def delayed_start():
        time.sleep(10)  # Wait 10 seconds for other systems to initialize
        start_continuous_improvement()
    
    start_thread = threading.Thread(target=delayed_start, daemon=True)
    start_thread.start()
