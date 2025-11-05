"""
Create VERY BRIGHT animated sun material (intensity 500 default)
"""
import unreal

print("\n" + "="*70)
print("  ⭐ CREATING VERY BRIGHT SUN MATERIAL")
print("="*70 + "\n")

# Delete old material if exists
old_path = '/Game/Materials/M_Sun_Animated'
if unreal.EditorAssetLibrary.does_asset_exist(old_path):
    unreal.EditorAssetLibrary.delete_asset(old_path)
    print("  🗑️ Deleted old material")

# Create material
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
material_factory = unreal.MaterialFactoryNew()

material = asset_tools.create_asset(
    'M_Sun_Animated',
    '/Game/Materials',
    unreal.Material,
    material_factory
)

if material:
    print(f"✅ Created material")

    # UNLIT shading
    material.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)

    # Time node
    time_node = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionTime, -800, 0
    )

    # Pulse speed parameter
    speed_node = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionScalarParameter, -800, 100
    )
    speed_node.set_editor_property('parameter_name', 'PulseSpeed')
    speed_node.set_editor_property('default_value', 2.0)

    # Multiply time by speed
    time_mult = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionMultiply, -700, 50
    )
    unreal.MaterialEditingLibrary.connect_material_expressions(time_node, '', time_mult, 'A')
    unreal.MaterialEditingLibrary.connect_material_expressions(speed_node, '', time_mult, 'B')

    # Sine wave
    sine_node = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionSine, -600, 0
    )
    unreal.MaterialEditingLibrary.connect_material_expressions(time_mult, '', sine_node, '')

    # Scale sine
    sine_scale = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionMultiply, -400, 0
    )
    scale_value = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -500, 50
    )
    scale_value.set_editor_property('r', 0.2)
    unreal.MaterialEditingLibrary.connect_material_expressions(sine_node, '', sine_scale, 'A')
    unreal.MaterialEditingLibrary.connect_material_expressions(scale_value, '', sine_scale, 'B')

    # Add offset
    sine_offset = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionAdd, -300, 0
    )
    offset_value = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -400, 50
    )
    offset_value.set_editor_property('r', 1.0)
    unreal.MaterialEditingLibrary.connect_material_expressions(sine_scale, '', sine_offset, 'A')
    unreal.MaterialEditingLibrary.connect_material_expressions(offset_value, '', sine_offset, 'B')

    # Sun color
    base_color = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant3Vector, -600, -200
    )
    base_color.set_editor_property('constant', unreal.LinearColor(1.0, 0.8, 0.3, 1.0))

    # VERY HIGH intensity - 500.0!
    base_intensity = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionScalarParameter, -600, -100
    )
    base_intensity.set_editor_property('parameter_name', 'BaseIntensity')
    base_intensity.set_editor_property('default_value', 500.0)  # MUCH BRIGHTER!
    print("  ✅ Set BaseIntensity default to 500.0")

    # Multiply color by intensity
    color_intensity = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionMultiply, -400, -150
    )
    unreal.MaterialEditingLibrary.connect_material_expressions(base_color, '', color_intensity, 'A')
    unreal.MaterialEditingLibrary.connect_material_expressions(base_intensity, '', color_intensity, 'B')

    # Multiply by pulse
    final_emissive = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionMultiply, -200, -100
    )
    unreal.MaterialEditingLibrary.connect_material_expressions(color_intensity, '', final_emissive, 'A')
    unreal.MaterialEditingLibrary.connect_material_expressions(sine_offset, '', final_emissive, 'B')

    # Connect to emissive
    unreal.MaterialEditingLibrary.connect_material_property(
        final_emissive, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR
    )

    # Compile and save
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)

    # Apply to star
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    for actor in actors:
        if actor and 'Star' in actor.get_actor_label():
            actor.static_mesh_component.set_material(0, material)
            print(f"  ✅ Applied to {actor.get_actor_label()}")
            break

print("\n" + "="*70)
print("✅ VERY BRIGHT SUN CREATED!")
print("="*70)
print("\n🌟 Intensity: 500.0 (50x brighter than before!)")
