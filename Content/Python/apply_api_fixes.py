"""
Batch API Fix Script for Alexander Project
==========================================
This script updates all remaining Python files to use UE5.5+ compatible APIs.

Run this ONCE to fix all deprecated API usage in phase scripts.
"""

import unreal
import os
import re

# List of files that need fixing (relative to Content/Python/)
FILES_TO_FIX = [
    "multiplayer_foundation.py",
    "advanced_content_expansion.py",
    "next_generation_vr_systems.py",
    "ai_society_evolution.py",
    "ai_society_evolution_complete.py",
]

def add_material_helper(file_path):
    """Add material helper function to file if it doesn't exist"""
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Check if helper already exists
    if 'def create_material_helper' in content:
        print(f"  ✓ Material helper already exists in {os.path.basename(file_path)}")
        return

    # Find import section
    import_section_end = content.find('\nclass ')
    if import_section_end == -1:
        import_section_end = content.find('\ndef ')

    # Helper code to insert
    helper_code = '''
def create_material_helper(material_name, package_path):
    """
    Helper function to create materials properly without deprecated API
    FIXED: MaterialEditingLibrary.create_material() does NOT exist in UE Python API
    """
    # Ensure directory exists
    editor_asset_subsystem = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
    if not editor_asset_subsystem.does_directory_exist(package_path):
        editor_asset_subsystem.make_directory(package_path)

    # Create material using factory
    factory = unreal.MaterialFactoryNew()
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    material = asset_tools.create_asset(
        asset_name=material_name,
        package_path=package_path,
        asset_class=unreal.Material,
        factory=factory
    )

    if material:
        # Save the material
        editor_asset_subsystem.save_loaded_asset(material)
        return material

    return None

'''

    # Insert helper before first class or function
    new_content = content[:import_section_end] + helper_code + content[import_section_end:]

    # Write back
    with open(file_path, 'w', encoding='utf-8') as f:
        f.write(new_content)

    print(f"  ✅ Added material helper to {os.path.basename(file_path)}")


def add_subsystem_init(file_path):
    """Add subsystem initialization to __init__ method"""
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Check if subsystems already initialized
    if 'self.editor_actor_subsystem' in content:
        print(f"  ✓ Subsystems already initialized in {os.path.basename(file_path)}")
        return

    # Find __init__ method
    init_pattern = r'(def __init__\(self\):[\s\S]*?)([\n\s]+def )'
    match = re.search(init_pattern, content)

    if not match:
        print(f"  ⚠ Could not find __init__ in {os.path.basename(file_path)}")
        return

    # Subsystem initialization code
    subsystem_code = '''

        # Use UE5.5+ subsystems (FIXED from deprecated APIs)
        self.editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        self.editor_asset_subsystem = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
'''

    # Insert before next method
    new_content = content.replace(
        match.group(0),
        match.group(1) + subsystem_code + match.group(2)
    )

    # Write back
    with open(file_path, 'w', encoding='utf-8') as f:
        f.write(new_content)

    print(f"  ✅ Added subsystem initialization to {os.path.basename(file_path)}")


def replace_deprecated_api_calls(file_path):
    """Replace deprecated API calls with subsystem calls"""
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    original_content = content
    replacements_made = 0

    # EditorLevelLibrary replacements
    replacements = [
        (r'unreal\.EditorLevelLibrary\.spawn_actor_from_class\(',
         'self.editor_actor_subsystem.spawn_actor_from_class('),
        (r'unreal\.EditorLevelLibrary\.spawn_actor_from_object\(',
         'self.editor_actor_subsystem.spawn_actor_from_object('),
        (r'unreal\.EditorLevelLibrary\.get_editor_world\(',
         'self.level_editor_subsystem.get_current_level('),
        (r'unreal\.EditorLevelLibrary\.destroy_actor\(',
         'self.editor_actor_subsystem.destroy_actor('),

        # EditorAssetLibrary replacements
        (r'unreal\.EditorAssetLibrary\.load_asset\(',
         'self.editor_asset_subsystem.load_asset('),
        (r'unreal\.EditorAssetLibrary\.does_asset_exist\(',
         'self.editor_asset_subsystem.does_asset_exist('),
        (r'unreal\.EditorAssetLibrary\.does_directory_exist\(',
         'self.editor_asset_subsystem.does_directory_exist('),
        (r'unreal\.EditorAssetLibrary\.make_directory\(',
         'self.editor_asset_subsystem.make_directory('),
        (r'unreal\.EditorAssetLibrary\.save_loaded_asset\(',
         'self.editor_asset_subsystem.save_loaded_asset('),

        # MaterialEditingLibrary.create_material (BROKEN API)
        (r'unreal\.MaterialEditingLibrary\.create_material\(["\']([^"\']+)["\'],\s*["\']([^"\']+)["\']\)',
         r'create_material_helper("\1", "\2")'),
    ]

    for pattern, replacement in replacements:
        content, count = re.subn(pattern, replacement, content)
        replacements_made += count

    if replacements_made > 0:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"  ✅ Made {replacements_made} API replacements in {os.path.basename(file_path)}")
    else:
        print(f"  ✓ No deprecated APIs found in {os.path.basename(file_path)}")


def fix_file(file_path):
    """Apply all fixes to a single file"""
    print(f"\n📝 Fixing {os.path.basename(file_path)}...")

    if not os.path.exists(file_path):
        print(f"  ❌ File not found: {file_path}")
        return False

    try:
        # Add material helper
        add_material_helper(file_path)

        # Add subsystem initialization
        add_subsystem_init(file_path)

        # Replace deprecated API calls
        replace_deprecated_api_calls(file_path)

        return True
    except Exception as e:
        print(f"  ❌ Error fixing file: {e}")
        import traceback
        traceback.print_exc()
        return False


def main():
    """Main execution function"""
    print("="*70)
    print("  BATCH API FIX SCRIPT FOR ALEXANDER PROJECT")
    print("  Updating Python scripts to UE5.5+ compatible APIs")
    print("="*70)

    # Get Content/Python directory
    content_python = r'C:\Users\allen\Desktop\Alexander\Alexander\Content\Python'

    successful = 0
    failed = 0

    for filename in FILES_TO_FIX:
        file_path = os.path.join(content_python, filename)

        if fix_file(file_path):
            successful += 1
        else:
            failed += 1

    print("\n" + "="*70)
    print("  BATCH FIX COMPLETE")
    print("="*70)
    print(f"\n✅ Successfully fixed: {successful} files")
    if failed > 0:
        print(f"❌ Failed to fix: {failed} files")
    print("\n💡 TIP: Run individual phase scripts to verify fixes")
    print("="*70 + "\n")


if __name__ == "__main__":
    main()
