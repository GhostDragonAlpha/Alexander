"""
Direct Blueprint Creation - I AM THE AI!
This creates blueprints immediately when executed.
"""

import unreal

print("\n" + "="*70)
print("  🤖 AI CREATING VR SPACESHIP BLUEPRINTS")
print("="*70 + "\n")

# Get the asset tools
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
editor_asset_lib = unreal.EditorAssetLibrary()

# Ensure folders exist
folders = [
    "/Game/SpaceShip/Blueprints",
    "/Game/SpaceShip/Materials",
    "/Game/SpaceShip/UI"
]

for folder in folders:
    if not editor_asset_lib.does_directory_exist(folder):
        editor_asset_lib.make_directory(folder)
        print(f"✓ Created folder: {folder}")

print("\n📘 Creating Blueprints...\n")

# Blueprint definitions: (name, parent_class, description)
blueprints_to_create = [
    ("BP_VRSpaceshipPlayer", unreal.Pawn, "VR player pawn with motion controllers"),
    ("BP_SpaceshipCockpit", unreal.Actor, "Main cockpit structure"),
    ("BP_FlightStick", unreal.Actor, "Interactive flight stick"),
    ("BP_ThrottleControl", unreal.Actor, "Throttle lever control"),
    ("BP_ButtonPanel", unreal.Actor, "Button control panel"),
]

created_blueprints = []

for bp_name, parent_class, description in blueprints_to_create:
    bp_path = f"/Game/SpaceShip/Blueprints/{bp_name}"
    
    # Check if already exists
    if editor_asset_lib.does_asset_exist(bp_path):
        print(f"  ⚠ {bp_name} already exists")
        created_blueprints.append(bp_name)
        continue
    
    # Create blueprint factory
    factory = unreal.BlueprintFactory()
    factory.set_editor_property("ParentClass", parent_class)
    
    try:
        # Create the asset
        new_asset = asset_tools.create_asset(
            asset_name=bp_name,
            package_path="/Game/SpaceShip/Blueprints",
            asset_class=unreal.Blueprint,
            factory=factory
        )
        
        if new_asset:
            print(f"  ✅ Created: {bp_name} - {description}")
            created_blueprints.append(bp_name)
            
            # Save the asset immediately
            editor_asset_lib.save_loaded_asset(new_asset)
        else:
            print(f"  ❌ Failed to create: {bp_name}")
            
    except Exception as e:
        print(f"  ❌ Error creating {bp_name}: {str(e)}")

print(f"\n🎨 Creating Materials...\n")

# Material definitions
materials_to_create = [
    ("M_CockpitMetal", "Metallic surfaces"),
    ("M_HologramBlue", "Blue holographic displays"),
    ("M_ButtonLit", "Illuminated buttons"),
    ("M_GlassCanopy", "Transparent canopy")
]

created_materials = []

for mat_name, description in materials_to_create:
    mat_path = f"/Game/SpaceShip/Materials/{mat_name}"
    
    if editor_asset_lib.does_asset_exist(mat_path):
        print(f"  ⚠ {mat_name} already exists")
        created_materials.append(mat_name)
        continue
    
    factory = unreal.MaterialFactoryNew()
    
    try:
        new_mat = asset_tools.create_asset(
            asset_name=mat_name,
            package_path="/Game/SpaceShip/Materials",
            asset_class=unreal.Material,
            factory=factory
        )
        
        if new_mat:
            print(f"  ✅ Created: {mat_name} - {description}")
            created_materials.append(mat_name)
            editor_asset_lib.save_loaded_asset(new_mat)
        else:
            print(f"  ❌ Failed to create: {mat_name}")
            
    except Exception as e:
        print(f"  ❌ Error creating {mat_name}: {str(e)}")

print(f"\n🖥️  Creating UI Widget...\n")

# Create UI widget
widget_name = "WBP_HolographicDisplay"
widget_path = f"/Game/SpaceShip/UI/{widget_name}"

if editor_asset_lib.does_asset_exist(widget_path):
    print(f"  ⚠ {widget_name} already exists")
else:
    factory = unreal.WidgetBlueprintFactory()
    
    try:
        new_widget = asset_tools.create_asset(
            asset_name=widget_name,
            package_path="/Game/SpaceShip/UI",
            asset_class=unreal.WidgetBlueprint,
            factory=factory
        )
        
        if new_widget:
            print(f"  ✅ Created: {widget_name} - Cockpit HUD display")
            editor_asset_lib.save_loaded_asset(new_widget)
        else:
            print(f"  ❌ Failed to create: {widget_name}")
            
    except Exception as e:
        print(f"  ❌ Error creating {widget_name}: {str(e)}")

print("\n" + "="*70)
print("  ✅ BUILD COMPLETE!")
print("="*70)
print(f"\n📦 Created {len(created_blueprints)} blueprints")
print(f"🎨 Created {len(created_materials)} materials")
print(f"🖥️  Created 1 UI widget")
print(f"\n💡 Open Content Browser → SpaceShip to see your assets!")
print("="*70 + "\n")
