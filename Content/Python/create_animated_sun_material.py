"""
Step 4: Create fancy animated sun material with pulsing effect
"""
import unreal

print("\n" + "="*70)
print("  ✨ CREATING ANIMATED SUN MATERIAL")
print("="*70 + "\n")

# Create material asset
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
material_factory = unreal.MaterialFactoryNew()

# Create in Content/Materials folder
package_path = '/Game/Materials'
asset_name = 'M_Sun_Animated'

print(f"Creating animated material: {package_path}/{asset_name}")

material = asset_tools.create_asset(
    asset_name,
    package_path,
    unreal.Material,
    material_factory
)

if material:
    print(f"✅ Material created: {material.get_name()}")

    # Set to UNLIT shading model (won't block light)
    material.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
    print("  ✅ Set shading model to UNLIT")

    # Create time node for animation
    time_node = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionTime,
        -800, 0
    )
    print("  ✅ Added time node for animation")

    # Create sine wave for pulsing (0.5 to 1.5 multiplier)
    sine_node = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionSine,
        -600, 0
    )

    # Speed multiplier for sine wave
    speed_node = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionScalarParameter,
        -800, 100
    )
    speed_node.set_editor_property('parameter_name', 'PulseSpeed')
    speed_node.set_editor_property('default_value', 2.0)  # 2 Hz pulse
    print("  ✅ Added pulse speed parameter (default: 2.0 Hz)")

    # Multiply time by speed
    time_mult = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionMultiply,
        -700, 50
    )

    unreal.MaterialEditingLibrary.connect_material_expressions(time_node, '', time_mult, 'A')
    unreal.MaterialEditingLibrary.connect_material_expressions(speed_node, '', time_mult, 'B')
    unreal.MaterialEditingLibrary.connect_material_expressions(time_mult, '', sine_node, '')

    # Remap sine from [-1,1] to [0.8, 1.2] for subtle pulse
    # Formula: (sine * 0.2) + 1.0 = range [0.8 to 1.2]
    sine_scale = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionMultiply,
        -400, 0
    )

    scale_value = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionConstant,
        -500, 50
    )
    scale_value.set_editor_property('r', 0.2)

    unreal.MaterialEditingLibrary.connect_material_expressions(sine_node, '', sine_scale, 'A')
    unreal.MaterialEditingLibrary.connect_material_expressions(scale_value, '', sine_scale, 'B')

    sine_offset = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionAdd,
        -300, 0
    )

    offset_value = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionConstant,
        -400, 50
    )
    offset_value.set_editor_property('r', 1.0)

    unreal.MaterialEditingLibrary.connect_material_expressions(sine_scale, '', sine_offset, 'A')
    unreal.MaterialEditingLibrary.connect_material_expressions(offset_value, '', sine_offset, 'B')

    print("  ✅ Created pulsing animation (0.8x to 1.2x)")

    # Sun base color (bright yellow-orange)
    base_color = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionConstant3Vector,
        -600, -200
    )
    base_color.set_editor_property('constant', unreal.LinearColor(1.0, 0.8, 0.3, 1.0))
    print("  ✅ Set sun color: Yellow-orange")

    # Base intensity parameter
    base_intensity = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionScalarParameter,
        -600, -100
    )
    base_intensity.set_editor_property('parameter_name', 'BaseIntensity')
    base_intensity.set_editor_property('default_value', 10.0)
    print("  ✅ Added base intensity parameter (default: 10.0)")

    # Multiply color by base intensity
    color_intensity = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionMultiply,
        -400, -150
    )
    unreal.MaterialEditingLibrary.connect_material_expressions(base_color, '', color_intensity, 'A')
    unreal.MaterialEditingLibrary.connect_material_expressions(base_intensity, '', color_intensity, 'B')

    # Multiply by animated pulse
    final_emissive = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionMultiply,
        -200, -100
    )
    unreal.MaterialEditingLibrary.connect_material_expressions(color_intensity, '', final_emissive, 'A')
    unreal.MaterialEditingLibrary.connect_material_expressions(sine_offset, '', final_emissive, 'B')

    # Connect to emissive color
    unreal.MaterialEditingLibrary.connect_material_property(
        final_emissive, '',
        unreal.MaterialProperty.MP_EMISSIVE_COLOR
    )

    print("  ✅ Connected animated emissive output")

    # Compile and save
    unreal.MaterialEditingLibrary.recompile_material(material)
    print("  ✅ Material compiled")

    unreal.EditorAssetLibrary.save_loaded_asset(material)
    print("  ✅ Material saved")

    print(f"\n📦 Material path: {material.get_path_name()}")
else:
    print("❌ Failed to create material")

print("\n" + "="*70)
print("✅ ANIMATED SUN MATERIAL CREATED!")
print("="*70)
print("\n✨ Features:")
print("  • Pulsing animation (subtle 0.8x to 1.2x)")
print("  • Adjustable pulse speed (default 2 Hz)")
print("  • Adjustable base intensity (default 10.0)")
print("  • UNLIT shading (won't block point light)")
print("  • Bright yellow-orange glow")
