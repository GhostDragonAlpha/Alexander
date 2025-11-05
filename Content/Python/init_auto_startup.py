# Auto-execute on Unreal Editor startup
import unreal
import sys
import os

print("=" * 70)
print("  AUTO-BUILDING VR SPACESHIP COCKPIT")
print("=" * 70)

# Add Python path
content_python = r'C:\Users\allen\Desktop\Alexander\Alexander\Content\Python'
if content_python not in sys.path:
    sys.path.append(content_python)

try:
    # Load AI Executor
    print("\n[1/3] Loading AI Executor...")
    with open(os.path.join(content_python, 'ai_executor.py'), 'r') as f:
        exec(f.read())
    print("✓ AI Executor loaded")
    
    # Build Cockpit
    print("\n[2/3] Building Cockpit Foundation...")
    with open(os.path.join(content_python, 'build_cockpit.py'), 'r') as f:
        exec(f.read())
    print("✓ Cockpit foundation created")
    
    # Start AI Development (if function exists)
    print("\n[3/3] Starting AI Development...")
    try:
        start_ai_dev('Build VR spaceship cockpit with interactive joystick, throttle, and holographic displays')
        print("✓ AI development started")
    except NameError:
        print("⚠ AI development function not available (this is normal)")
    
    print("\n" + "=" * 70)
    print("  🚀 AUTO-BUILD COMPLETE!")
    print("=" * 70)
    
except Exception as e:
    print(f"Error during auto-build: {e}")
    import traceback
    traceback.print_exc()
