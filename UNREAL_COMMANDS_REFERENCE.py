"""
Quick reference for AI Vision System commands in Unreal Console
Press ` (backtick) in Unreal Editor to open console
"""

# ============================================
# FULL AUTO TEST (RUN THIS FIRST)
# ============================================
py exec(open(r'C:\Users\allen\Desktop\Alexander\Alexander\run_full_auto_test.py').read())


# ============================================
# MANUAL STEP-BY-STEP (if needed)
# ============================================

# 1. Load AI Executor
py exec(open(r'C:\Users\allen\Desktop\Alexander\Alexander\Content\Python\ai_executor.py').read())

# 2. Load Integration Test
py exec(open(r'C:\Users\allen\Desktop\Alexander\Alexander\test_ai_vision_integration.py').read())

# 3. Run test manually
py run_full_integration_test()


# ============================================
# START AI DEVELOPMENT
# ============================================

# Start building with AI
py start_ai_dev('Build VR spaceship cockpit with interactive joystick and throttle controls')

# Check AI development status
py ai_status()

# Stop AI auto-development
py stop_ai_dev()


# ============================================
# MANUAL BLUEPRINT CREATION
# ============================================

# Create a new blueprint
py create_blueprint('BP_SpaceshipCockpit', 'Pawn', '/Game/Blueprints/')

# Spawn an actor in the level
py spawn_actor_in_level('/Script/Engine.DirectionalLight', (0, 0, 500), (0, 0, 0), 'MainSun')

# Adjust lighting
py adjust_directional_light(5.0, 6500)


# ============================================
# DEBUGGING COMMANDS
# ============================================

# Check Python environment
py import sys; print(f"Python: {sys.version}")

# Check unreal module
py import unreal; print(f"Unreal API: {unreal.__version__ if hasattr(unreal, '__version__') else 'Available'}")

# List all actors in level
py import unreal; world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world(); actors = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Actor); print(f"Actors in level: {len(actors)}")

# Get current level name
py import unreal; print(unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).get_current_level().get_name())


# ============================================
# AI BACKEND TEST
# ============================================

# Test AI backend connection
py import http.client; conn = http.client.HTTPConnection("localhost", 8000); conn.request("GET", "/api/health"); print(f"AI Backend: {conn.getresponse().status}"); conn.close()
