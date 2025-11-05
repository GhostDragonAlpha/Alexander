# init_unreal.py - Configuration-Based Startup System
# Automatically executed by Unreal Editor on startup
# Configure workflow in workflow_config.json (NO code editing required!)

import unreal
import sys
import os
import json

print("=" * 70)
print("  CONFIGURATION-BASED STARTUP SYSTEM")
print("=" * 70)

# Add Python path
content_python = r'C:\Users\allen\Desktop\Alexander\Alexander\Content\Python'
if content_python not in sys.path:
    sys.path.insert(0, content_python)

def load_workflow_config():
    """Load workflow configuration from JSON file"""
    config_path = os.path.join(content_python, 'workflow_config.json')

    if not os.path.exists(config_path):
        print("⚠️  WARNING: workflow_config.json not found")
        print("   Creating default configuration...")
        return None

    try:
        with open(config_path, 'r') as f:
            config = json.load(f)
        print("✅ Loaded workflow configuration")
        return config
    except Exception as e:
        print(f"❌ Error loading config: {e}")
        return None

def get_current_level():
    """Get the current level name"""
    try:
        # Use UE5.5+ subsystem API
        level_editor_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        current_level = level_editor_subsystem.get_current_level()
        if current_level:
            return current_level.get_name()
    except:
        pass
    return "Unknown"

def switch_to_level(level_path):
    """Switch to specified level if it exists"""
    try:
        # Use UE5.5+ subsystem APIs
        editor_asset_subsystem = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
        level_editor_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

        if editor_asset_subsystem.does_asset_exist(level_path):
            level_editor_subsystem.load_level(level_path)
            print(f"   ✅ Loaded level: {level_path}")
            return True
        else:
            print(f"   ℹ️  Level doesn't exist yet: {level_path}")
            return False
    except Exception as e:
        print(f"   ⚠️  Could not switch level: {e}")
        return False

def run_phase(phase_name, phase_config):
    """Execute a single phase based on its configuration"""

    print(f"\n{'='*70}")
    print(f"  {phase_name.upper().replace('_', ' ')}")
    print(f"  {phase_config.get('description', 'No description')}")
    print(f"{'='*70}")

    # Check if phase is enabled
    if not phase_config.get('enabled', False):
        print("   ⏭️  SKIPPED (disabled in config)")
        return False

    # Get script path
    script_name = phase_config.get('script')
    if not script_name:
        print("   ❌ No script specified in config")
        return False

    script_path = os.path.join(content_python, script_name)
    if not os.path.exists(script_path):
        print(f"   ❌ Script not found: {script_name}")
        return False

    # Handle level switching if specified
    target_level = phase_config.get('target_level')
    if target_level:
        current_level = get_current_level()
        print(f"   Current level: {current_level}")

        if phase_config.get('creates_level', False):
            print(f"   This phase will create: {target_level}")
        else:
            print(f"   Target level: {target_level}")
            if current_level != target_level.split('/')[-1]:
                if not switch_to_level(target_level):
                    print(f"   ⚠️  Continuing in current level: {current_level}")

    # Execute the script
    try:
        print(f"   ▶️  Executing: {script_name}")
        print()

        with open(script_path, 'r') as f:
            script_code = f.read()

        exec(script_code, {'__name__': '__main__'})

        print()
        print(f"   ✅ {phase_name} complete")
        return True

    except Exception as e:
        print()
        print(f"   ❌ Error in {phase_name}: {e}")
        import traceback
        traceback.print_exc()
        return False

def main():
    """Main startup execution"""

    print("\n🔧 Loading workflow configuration...")
    config = load_workflow_config()

    if not config:
        print("❌ Could not load configuration - aborting startup")
        return

    # Get phases
    phases = config.get('phases', {})
    settings = config.get('settings', {})

    # Show configuration summary
    enabled_phases = [name for name, cfg in phases.items() if cfg.get('enabled', False)]

    print(f"\n📋 Workflow Summary:")
    print(f"   Total phases defined: {len(phases)}")
    print(f"   Enabled phases: {len(enabled_phases)}")

    if enabled_phases:
        print(f"\n   Phases that will run:")
        for phase_name in enabled_phases:
            desc = phases[phase_name].get('description', 'No description')
            print(f"      ✓ {phase_name}: {desc}")
    else:
        print(f"\n   ⚠️  No phases enabled - nothing will execute")
        print(f"   Edit workflow_config.json to enable phases")

    print(f"\n{'='*70}")

    # Execute enabled phases in order
    results = {}
    for phase_name, phase_config in phases.items():
        if phase_config.get('enabled', False):
            success = run_phase(phase_name, phase_config)
            results[phase_name] = success

    # Summary
    print(f"\n{'='*70}")
    print(f"  STARTUP COMPLETE")
    print(f"{'='*70}")

    successful = [name for name, success in results.items() if success]
    failed = [name for name, success in results.items() if not success]

    if successful:
        print(f"\n✅ Successful phases ({len(successful)}):")
        for name in successful:
            print(f"   - {name}")

    if failed:
        print(f"\n❌ Failed phases ({len(failed)}):")
        for name in failed:
            print(f"   - {name}")

    if not results:
        print(f"\nℹ️  No phases were executed (all disabled)")

    print(f"\n{'='*70}")
    print(f"💡 TIP: Edit workflow_config.json to enable/disable phases")
    print(f"💡 TIP: Edit custom_startup.py for your own custom code")
    print(f"{'='*70}\n")

    # Run custom startup code if it exists
    custom_script = os.path.join(content_python, 'custom_startup.py')
    if os.path.exists(custom_script):
        try:
            print("🔧 Running custom startup script...")
            with open(custom_script, 'r') as f:
                exec(f.read())
        except Exception as e:
            print(f"⚠️  Error in custom startup: {e}")

    # =========================================================================
    # DISABLED: AUTO-SCREENSHOT (commented out for manual control)
    # Re-enable if you want automatic screenshots on every startup
    # =========================================================================
    # auto_screenshot_script = os.path.join(content_python, 'auto_screenshot_simple.py')
    # if os.path.exists(auto_screenshot_script):
    #     try:
    #         with open(auto_screenshot_script, 'r') as f:
    #             exec(f.read())
    #     except Exception as e:
    #         print(f"⚠️  Auto-screenshot failed: {e}")

# Run main startup
try:
    main()
except Exception as e:
    print(f"❌ CRITICAL ERROR in startup system: {e}")
    import traceback
    traceback.print_exc()
