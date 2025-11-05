"""
Step 2: Create simple emissive material for sun
This material will glow but won't block the point light
"""
import unreal

print("\n" + "="*70)
print("  ⭐ CREATING SIMPLE SUN MATERIAL")
print("="*70 + "\n")

# Create material asset
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
material_factory = unreal.MaterialFactoryNew()

# Create in Content/Materials folder
package_path = '/Game/Materials'
asset_name = 'M_Sun_Simple'

print(f"Creating material: {package_path}/{asset_name}")

# Create the material
material = asset_tools.create_asset(
    asset_name,
    package_path,
    unreal.Material,
    material_factory
)

if material:
    print(f"✅ Material created: {material.get_name()}")

    # Set material properties
    # Unlit shading model so it won't block light
    material.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)

    print("  ✅ Set shading model to UNLIT (won't block light)")

    # Create emissive color expression (bright yellow-orange for sun)
    emissive_node = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionConstant3Vector,
        -400, 0
    )

    # Set sun color (bright yellow-orange)
    emissive_node.set_editor_property('constant', unreal.LinearColor(1.0, 0.8, 0.3, 1.0))
    print("  ✅ Set emissive color: Yellow-orange")

    # Create intensity multiplier
    multiply_node = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionMultiply,
        -200, 0
    )

    intensity_node = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionScalarParameter,
        -400, 100
    )
    intensity_node.set_editor_property('parameter_name', 'Intensity')
    intensity_node.set_editor_property('default_value', 10.0)  # Start at 10x brightness
    print("  ✅ Added intensity parameter (default: 10.0)")

    # Connect nodes: Color * Intensity = Final Emissive
    unreal.MaterialEditingLibrary.connect_material_expressions(
        emissive_node, '',
        multiply_node, 'A'
    )

    unreal.MaterialEditingLibrary.connect_material_expressions(
        intensity_node, '',
        multiply_node, 'B'
    )

    unreal.MaterialEditingLibrary.connect_material_property(
        multiply_node, '',
        unreal.MaterialProperty.MP_EMISSIVE_COLOR
    )

    print("  ✅ Connected emissive nodes")

    # Compile and save
    unreal.MaterialEditingLibrary.recompile_material(material)
    print("  ✅ Material compiled")

    # Save the asset
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    print("  ✅ Material saved")

    print(f"\n📦 Material path: {material.get_path_name()}")
else:
    print("❌ Failed to create material")

print("\n" + "="*70)
print("✅ SIMPLE SUN MATERIAL CREATED!")
print("="*70)
print("\nNext step: Apply this material to the star")
