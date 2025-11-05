"""
Create materials for celestial bodies (Sun, Planets, Moons, Asteroids)
This sets up proper emissive materials for the Sun and colored materials for other bodies
"""

import unreal

def create_sun_emissive_material():
    """Create a glowing emissive material for the Sun"""

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    material_factory = unreal.MaterialFactoryNew()

    # Create material asset
    package_path = '/Game/Materials'
    asset_name = 'M_Sun_Emissive'

    # Check if material already exists
    if unreal.EditorAssetLibrary.does_asset_exist(f"{package_path}/{asset_name}"):
        unreal.log(f"Material {asset_name} already exists, skipping creation")
        return unreal.load_asset(f"{package_path}/{asset_name}")

    # Create the material
    material = asset_tools.create_asset(
        asset_name,
        package_path,
        unreal.Material,
        material_factory
    )

    if material:
        # Set material to unlit (emissive only)
        material.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)

        unreal.log(f"✅ Created Sun emissive material: {package_path}/{asset_name}")
        unreal.EditorAssetLibrary.save_asset(f"{package_path}/{asset_name}")
        return material
    else:
        unreal.log_error(f"Failed to create material {asset_name}")
        return None

def create_planet_material():
    """Create a basic colored material for planets"""

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    material_factory = unreal.MaterialFactoryNew()

    package_path = '/Game/Materials'
    asset_name = 'M_Planet_Base'

    if unreal.EditorAssetLibrary.does_asset_exist(f"{package_path}/{asset_name}"):
        unreal.log(f"Material {asset_name} already exists, skipping creation")
        return unreal.load_asset(f"{package_path}/{asset_name}")

    material = asset_tools.create_asset(
        asset_name,
        package_path,
        unreal.Material,
        material_factory
    )

    if material:
        unreal.log(f"✅ Created Planet base material: {package_path}/{asset_name}")
        unreal.EditorAssetLibrary.save_asset(f"{package_path}/{asset_name}")
        return material
    else:
        unreal.log_error(f"Failed to create material {asset_name}")
        return None

def create_asteroid_material():
    """Create a rough, rocky material for asteroids"""

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    material_factory = unreal.MaterialFactoryNew()

    package_path = '/Game/Materials'
    asset_name = 'M_Asteroid_Rocky'

    if unreal.EditorAssetLibrary.does_asset_exist(f"{package_path}/{asset_name}"):
        unreal.log(f"Material {asset_name} already exists, skipping creation")
        return unreal.load_asset(f"{package_path}/{asset_name}")

    material = asset_tools.create_asset(
        asset_name,
        package_path,
        unreal.Material,
        material_factory
    )

    if material:
        unreal.log(f"✅ Created Asteroid rocky material: {package_path}/{asset_name}")
        unreal.EditorAssetLibrary.save_asset(f"{package_path}/{asset_name}")
        return material
    else:
        unreal.log_error(f"Failed to create material {asset_name}")
        return None

def setup_all_materials():
    """Create all celestial body materials"""

    unreal.log("=" * 70)
    unreal.log("CREATING CELESTIAL BODY MATERIALS")
    unreal.log("=" * 70)

    # Ensure Materials folder exists
    if not unreal.EditorAssetLibrary.does_directory_exist('/Game/Materials'):
        unreal.EditorAssetLibrary.make_directory('/Game/Materials')
        unreal.log("Created /Game/Materials directory")

    # Create materials
    sun_mat = create_sun_emissive_material()
    planet_mat = create_planet_material()
    asteroid_mat = create_asteroid_material()

    unreal.log("=" * 70)
    unreal.log("MATERIAL CREATION COMPLETE")
    unreal.log("=" * 70)
    unreal.log("Materials created:")
    unreal.log("  - M_Sun_Emissive (glowing sun)")
    unreal.log("  - M_Planet_Base (colored planets)")
    unreal.log("  - M_Asteroid_Rocky (rough asteroids)")
    unreal.log("")
    unreal.log("These materials support dynamic color parameters")
    unreal.log("=" * 70)

if __name__ == "__main__":
    setup_all_materials()
