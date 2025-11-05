# ai_executor.py
# This script runs INSIDE Unreal Engine to execute AI-generated development commands

import unreal
import json
import time
from pathlib import Path

class AIAutoDeveloper:
    """
    Autonomous development system that executes AI-generated actions in Unreal Engine
    """
    
    def __init__(self, backend_url="http://localhost:8000"):
        self.backend_url = backend_url
        self.auto_dev_enabled = False
        self.development_goal = ""
        self.actions_executed = 0
        
        unreal.log("🤖 AI Auto-Developer initialized")
    
    def start_auto_development(self, goal: str, interval: float = 10.0):
        """
        Start autonomous development loop
        
        Args:
            goal: Development goal for AI to work toward
            interval: Seconds between development iterations
        """
        self.development_goal = goal
        self.auto_dev_enabled = True
        
        unreal.log("=" * 60)
        unreal.log("🚀 AI AUTO-DEVELOPMENT STARTED")
        unreal.log("=" * 60)
        unreal.log(f"📋 Goal: {goal}")
        unreal.log(f"⏱️  Interval: {interval} seconds")
        unreal.log(f"🌐 Backend: {self.backend_url}")
        unreal.log("")
        
        # Find or spawn screenshot capture actor
        self.setup_screenshot_actor()
        
        unreal.log("✓ Ready to receive AI guidance")
        unreal.log("  (AI will analyze screenshots and provide development actions)")
        unreal.log("")
    
    def setup_screenshot_actor(self):
        """
        Find or spawn the AI Screenshot Capture actor
        """
        # Check if actor already exists
        all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
        
        for actor in all_actors:
            if actor.get_class().get_name() == "AIScreenshotCapture_C":
                unreal.log("✓ Found existing AI Screenshot Capture actor")
                return actor
        
        # Spawn new actor if not found
        unreal.log("📷 Spawning AI Screenshot Capture actor...")
        # Note: In production, spawn the C++ actor here
        return None
    
    def execute_action(self, action: dict):
        """
        Execute a single AI-generated action
        
        Args:
            action: Dictionary containing action details
        """
        action_type = action.get("action_type")
        target = action.get("target")
        reasoning = action.get("reasoning", "")
        code = action.get("executable_code", "")
        
        unreal.log("")
        unreal.log("=" * 60)
        unreal.log(f"⚡ EXECUTING ACTION #{self.actions_executed + 1}")
        unreal.log("=" * 60)
        unreal.log(f"   Type: {action_type}")
        unreal.log(f"   Target: {target}")
        unreal.log(f"   Reason: {reasoning}")
        unreal.log("")
        
        if not code:
            unreal.log_warning("⚠️  No executable code provided")
            return False
        
        try:
            # Execute the AI-generated Python code
            exec(code, globals())
            
            self.actions_executed += 1
            unreal.log("")
            unreal.log(f"✅ Action completed successfully!")
            unreal.log(f"   Total actions executed: {self.actions_executed}")
            unreal.log("")
            return True
            
        except Exception as e:
            unreal.log_error(f"❌ Action failed: {str(e)}")
            unreal.log_error(f"   Code that failed:")
            for line in code.split('\n')[:10]:  # Show first 10 lines
                unreal.log_error(f"   {line}")
            return False
    
    def stop_auto_development(self):
        """
        Stop autonomous development
        """
        self.auto_dev_enabled = False
        
        unreal.log("")
        unreal.log("=" * 60)
        unreal.log("🛑 AI AUTO-DEVELOPMENT STOPPED")
        unreal.log("=" * 60)
        unreal.log(f"   Total actions executed: {self.actions_executed}")
        unreal.log(f"   Development goal: {self.development_goal}")
        unreal.log("")
    
    def get_status(self):
        """
        Get current status of auto-development system
        """
        status = {
            "enabled": self.auto_dev_enabled,
            "goal": self.development_goal,
            "actions_executed": self.actions_executed,
            "backend_url": self.backend_url
        }
        
        unreal.log("📊 AI Auto-Developer Status:")
        unreal.log(f"   Enabled: {status['enabled']}")
        unreal.log(f"   Goal: {status['goal']}")
        unreal.log(f"   Actions Executed: {status['actions_executed']}")
        
        return status


# =============================================================================
# GLOBAL INSTANCE AND CONVENIENCE FUNCTIONS
# =============================================================================

# Global AI developer instance
ai_dev = AIAutoDeveloper()

def start_ai_dev(goal: str = "Build a VR space farming game like Star Citizen"):
    """
    Convenience function: Start AI auto-development
    
    Usage in Unreal console:
        py exec(open('Content/Python/ai_executor.py').read())
        py start_ai_dev("Add interactive cockpit controls")
    """
    ai_dev.start_auto_development(goal)

def stop_ai_dev():
    """
    Convenience function: Stop AI auto-development
    
    Usage in Unreal console:
        py stop_ai_dev()
    """
    ai_dev.stop_auto_development()

def execute_ai_action(action_json: str):
    """
    Convenience function: Execute a single action from JSON
    
    Usage:
        py execute_ai_action('{"action_type": "create_blueprint", ...}')
    """
    try:
        action = json.loads(action_json)
        ai_dev.execute_action(action)
    except Exception as e:
        unreal.log_error(f"Failed to parse action: {e}")

def ai_status():
    """
    Convenience function: Get current status
    
    Usage in Unreal console:
        py ai_status()
    """
    return ai_dev.get_status()


# =============================================================================
# HELPER FUNCTIONS FOR COMMON UNREAL ENGINE OPERATIONS
# =============================================================================

def create_blueprint(name: str, parent_class_name: str, package_path: str):
    """
    Helper: Create a new Blueprint
    
    Args:
        name: Blueprint name (e.g., "BP_PlayerShip")
        parent_class_name: Parent class (e.g., "Pawn", "Actor")
        package_path: Where to save (e.g., "/Game/Blueprints/")
    """
    try:
        # Get parent class
        if parent_class_name == "Pawn":
            parent_class = unreal.Pawn
        elif parent_class_name == "Actor":
            parent_class = unreal.Actor
        elif parent_class_name == "Character":
            parent_class = unreal.Character
        else:
            parent_class = unreal.Actor
        
        # Create blueprint
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        blueprint_factory = unreal.BlueprintFactory()
        blueprint_factory.set_editor_property('parent_class', parent_class)
        
        blueprint = asset_tools.create_asset(
            asset_name=name,
            package_path=package_path,
            asset_class=unreal.Blueprint,
            factory=blueprint_factory
        )
        
        unreal.log(f"✓ Created Blueprint: {name}")
        return blueprint
        
    except Exception as e:
        unreal.log_error(f"Failed to create blueprint: {e}")
        return None

def spawn_actor_in_level(actor_class_path: str, location: tuple = (0, 0, 0), rotation: tuple = (0, 0, 0), label: str = ""):
    """
    Helper: Spawn an actor in the current level
    
    Args:
        actor_class_path: Path to actor class
        location: (X, Y, Z) coordinates
        rotation: (Pitch, Yaw, Roll) in degrees
        label: Actor label in outliner
    """
    try:
        editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        
        actor_class = unreal.load_class(None, actor_class_path)
        if not actor_class:
            unreal.log_error(f"Actor class not found: {actor_class_path}")
            return None
        
        actor_location = unreal.Vector(location[0], location[1], location[2])
        actor_rotation = unreal.Rotator(rotation[0], rotation[1], rotation[2])
        
        actor = editor_actor_subsystem.spawn_actor_from_class(
            actor_class,
            actor_location,
            actor_rotation
        )
        
        if label and actor:
            actor.set_actor_label(label)
        
        unreal.log(f"✓ Spawned actor: {label or actor_class_path}")
        return actor
        
    except Exception as e:
        unreal.log_error(f"Failed to spawn actor: {e}")
        return None

def adjust_directional_light(intensity: float = 3.0, temperature: float = 6500):
    """
    Helper: Adjust the main directional light
    
    Args:
        intensity: Light intensity
        temperature: Color temperature in Kelvin
    """
    try:
        all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
        
        for actor in all_actors:
            if isinstance(actor, unreal.DirectionalLight):
                light_component = actor.get_component_by_class(unreal.DirectionalLightComponent)
                if light_component:
                    light_component.set_intensity(intensity)
                    unreal.log(f"✓ Adjusted DirectionalLight intensity to {intensity}")
                    return True
        
        unreal.log_warning("No DirectionalLight found in level")
        return False
        
    except Exception as e:
        unreal.log_error(f"Failed to adjust light: {e}")
        return False


# =============================================================================
# AUTO-RUN ON IMPORT (for testing)
# =============================================================================

unreal.log("")
unreal.log("🤖 AI Executor loaded successfully!")
unreal.log("   Available commands:")
unreal.log("   - start_ai_dev('your goal here')")
unreal.log("   - stop_ai_dev()")
unreal.log("   - ai_status()")
unreal.log("   - execute_ai_action(json_string)")
unreal.log("")
