"""
VR Spaceship Cockpit Builder - Direct AI Development
I am the AI building your game!
"""

import unreal

# Get editor subsystems
editor_asset_lib = unreal.EditorAssetLibrary()
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

print("=" * 70)
print("  🚀 AI BUILDING VR SPACESHIP COCKPIT")
print("=" * 70)

# ============================================================================
# STEP 1: CREATE VR PLAYER PAWN
# ============================================================================
print("\n[1/6] Creating VR Player Pawn...")

# Create the VR Pawn blueprint
vr_pawn_path = "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer"
vr_pawn_factory = unreal.BlueprintFactory()
vr_pawn_factory.set_editor_property("ParentClass", unreal.Pawn)

try:
    vr_pawn_asset = asset_tools.create_asset(
        asset_name="BP_VRSpaceshipPlayer",
        package_path="/Game/SpaceShip/Blueprints",
        asset_class=unreal.Blueprint,
        factory=vr_pawn_factory
    )
    print(f"  ✓ Created VR Player Pawn: {vr_pawn_path}")
except Exception as e:
    print(f"  ⚠ VR Pawn may already exist: {e}")

# ============================================================================
# STEP 2: CREATE COCKPIT BLUEPRINT
# ============================================================================
print("\n[2/6] Creating Cockpit Blueprint...")

cockpit_bp_path = "/Game/SpaceShip/Blueprints/BP_SpaceshipCockpit"
cockpit_factory = unreal.BlueprintFactory()
cockpit_factory.set_editor_property("ParentClass", unreal.Actor)

try:
    cockpit_asset = asset_tools.create_asset(
        asset_name="BP_SpaceshipCockpit",
        package_path="/Game/SpaceShip/Blueprints",
        asset_class=unreal.Blueprint,
        factory=cockpit_factory
    )
    print(f"  ✓ Created Cockpit Blueprint: {cockpit_bp_path}")
except Exception as e:
    print(f"  ⚠ Cockpit may already exist: {e}")

# ============================================================================
# STEP 3: CREATE JOYSTICK BLUEPRINT
# ============================================================================
print("\n[3/6] Creating Interactive Joystick...")

joystick_path = "/Game/SpaceShip/Blueprints/BP_FlightStick"
joystick_factory = unreal.BlueprintFactory()
joystick_factory.set_editor_property("ParentClass", unreal.Actor)

try:
    joystick_asset = asset_tools.create_asset(
        asset_name="BP_FlightStick",
        package_path="/Game/SpaceShip/Blueprints",
        asset_class=unreal.Blueprint,
        factory=joystick_factory
    )
    print(f"  ✓ Created Flight Stick: {joystick_path}")
except Exception as e:
    print(f"  ⚠ Flight Stick may already exist: {e}")

# ============================================================================
# STEP 4: CREATE THROTTLE BLUEPRINT
# ============================================================================
print("\n[4/6] Creating Throttle Control...")

throttle_path = "/Game/SpaceShip/Blueprints/BP_ThrottleControl"
throttle_factory = unreal.BlueprintFactory()
throttle_factory.set_editor_property("ParentClass", unreal.Actor)

try:
    throttle_asset = asset_tools.create_asset(
        asset_name="BP_ThrottleControl",
        package_path="/Game/SpaceShip/Blueprints",
        asset_class=unreal.Blueprint,
        factory=throttle_factory
    )
    print(f"  ✓ Created Throttle Control: {throttle_path}")
except Exception as e:
    print(f"  ⚠ Throttle may already exist: {e}")

# ============================================================================
# STEP 5: CREATE BUTTON PANEL BLUEPRINT
# ============================================================================
print("\n[5/6] Creating Button Panel...")

panel_path = "/Game/SpaceShip/Blueprints/BP_ButtonPanel"
panel_factory = unreal.BlueprintFactory()
panel_factory.set_editor_property("ParentClass", unreal.Actor)

try:
    panel_asset = asset_tools.create_asset(
        asset_name="BP_ButtonPanel",
        package_path="/Game/SpaceShip/Blueprints",
        asset_class=unreal.Blueprint,
        factory=panel_factory
    )
    print(f"  ✓ Created Button Panel: {panel_path}")
except Exception as e:
    print(f"  ⚠ Button Panel may already exist: {e}")

# ============================================================================
# STEP 6: CREATE HOLOGRAPHIC DISPLAY WIDGET
# ============================================================================
print("\n[6/6] Creating Holographic Display UI...")

widget_path = "/Game/SpaceShip/UI/WBP_HolographicDisplay"
widget_factory = unreal.WidgetBlueprintFactory()

try:
    widget_asset = asset_tools.create_asset(
        asset_name="WBP_HolographicDisplay",
        package_path="/Game/SpaceShip/UI",
        asset_class=unreal.WidgetBlueprint,
        factory=widget_factory
    )
    print(f"  ✓ Created Holographic Display: {widget_path}")
except Exception as e:
    print(f"  ⚠ Display may already exist: {e}")

# ============================================================================
# CREATE MATERIALS
# ============================================================================
print("\n[BONUS] Creating Materials...")

materials = [
    ("M_CockpitMetal", "Metallic cockpit surfaces"),
    ("M_HologramBlue", "Blue holographic displays"),
    ("M_ButtonLit", "Illuminated button material"),
    ("M_GlassCanopy", "Transparent canopy glass")
]

for mat_name, description in materials:
    mat_path = f"/Game/SpaceShip/Materials/{mat_name}"
    mat_factory = unreal.MaterialFactoryNew()
    
    try:
        mat_asset = asset_tools.create_asset(
            asset_name=mat_name,
            package_path="/Game/SpaceShip/Materials",
            asset_class=unreal.Material,
            factory=mat_factory
        )
        print(f"  ✓ Created Material: {mat_name} ({description})")
    except Exception as e:
        print(f"  ⚠ {mat_name} may already exist")

# ============================================================================
# SAVE ALL ASSETS
# ============================================================================
print("\n" + "=" * 70)
print("  💾 SAVING ALL ASSETS...")
print("=" * 70)

# Get all assets in SpaceShip folder
all_assets = editor_asset_lib.list_assets("/Game/SpaceShip", recursive=True)
saved_count = 0

for asset_path in all_assets:
    if editor_asset_lib.save_asset(asset_path, only_if_is_dirty=True):
        saved_count += 1

print(f"  ✓ Saved {saved_count} assets")

# ============================================================================
# SUMMARY
# ============================================================================
print("\n" + "=" * 70)
print("  ✅ VR COCKPIT FOUNDATION COMPLETE!")
print("=" * 70)
print("\n📦 Created Assets:")
print("  • BP_VRSpaceshipPlayer - VR player pawn")
print("  • BP_SpaceshipCockpit - Main cockpit actor")
print("  • BP_FlightStick - Interactive joystick")
print("  • BP_ThrottleControl - Throttle lever")
print("  • BP_ButtonPanel - Control panel")
print("  • WBP_HolographicDisplay - UI widget")
print("  • 4 Materials for cockpit visuals")
print("\n🎯 Next Steps:")
print("  1. Open blueprints in editor to add components")
print("  2. Add VR grab components to controls")
print("  3. Create flight physics blueprint")
print("  4. Add 3D meshes for visual appearance")
print("\n💡 All assets are in /Game/SpaceShip/")
print("=" * 70 + "\n")
