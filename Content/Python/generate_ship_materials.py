#!/usr/bin/env python3
"""
Generate Ship Customization Materials
Creates material instances for all ship skins defined in DT_ShipSkins.csv
"""

import unreal

# Material creation settings
MASTER_MATERIAL_PATH = "/Game/Materials/Ships/M_Ship_Master"
OUTPUT_PATH = "/Game/Materials/Ships/"

def create_master_material():
    """Create the master material for ship skins"""
    print("Creating master ship material...")

    # Check if master material already exists
    if unreal.EditorAssetLibrary.does_asset_exist(MASTER_MATERIAL_PATH):
        print(f"  Master material already exists at {MASTER_MATERIAL_PATH}")
        return unreal.load_asset(MASTER_MATERIAL_PATH)

    # Create new material
    material_factory = unreal.MaterialFactoryNew()
    master_material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "M_Ship_Master",
        "/Game/Materials/Ships",
        unreal.Material,
        material_factory
    )

    print(f"  Created master material: {MASTER_MATERIAL_PATH}")
    return master_material

def create_material_instance(name, base_color, metallic=0.0, roughness=0.5, emissive_strength=0.0, emissive_color=(0,0,0)):
    """Create a material instance with specified parameters"""

    asset_path = f"{OUTPUT_PATH}{name}"

    # Check if material instance already exists
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        print(f"  Material instance already exists: {name}")
        return unreal.load_asset(asset_path)

    # Load master material
    master_material = unreal.load_asset(MASTER_MATERIAL_PATH)
    if not master_material:
        print(f"  ERROR: Could not load master material at {MASTER_MATERIAL_PATH}")
        return None

    # Create material instance
    material_instance_factory = unreal.MaterialInstanceConstantFactoryNew()
    material_instance_factory.initial_parent = master_material

    material_instance = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name,
        OUTPUT_PATH,
        unreal.MaterialInstanceConstant,
        material_instance_factory
    )

    if material_instance:
        # Set vector parameter for base color
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
            material_instance,
            "BaseColor",
            unreal.LinearColor(base_color[0], base_color[1], base_color[2], 1.0)
        )

        # Set scalar parameters
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            material_instance, "Metallic", metallic
        )
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            material_instance, "Roughness", roughness
        )
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            material_instance, "EmissiveStrength", emissive_strength
        )

        # Set emissive color if needed
        if emissive_strength > 0:
            unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
                material_instance,
                "EmissiveColor",
                unreal.LinearColor(emissive_color[0], emissive_color[1], emissive_color[2], 1.0)
            )

        print(f"  Created: {name}")
        return material_instance
    else:
        print(f"  ERROR: Failed to create {name}")
        return None

def generate_all_materials():
    """Generate all ship skin materials"""

    print("="*70)
    print("GENERATING SHIP CUSTOMIZATION MATERIALS")
    print("="*70)

    # Create master material
    master_material = create_master_material()
    if not master_material:
        print("ERROR: Could not create/load master material. Aborting.")
        return

    print("\nCreating material instances...")

    # BASIC SOLID COLORS
    create_material_instance("M_Ship_Default", (0.5, 0.5, 0.5), metallic=0.3, roughness=0.6)
    create_material_instance("M_Ship_White", (0.9, 0.9, 0.9), metallic=0.1, roughness=0.4)
    create_material_instance("M_Ship_Black", (0.1, 0.1, 0.1), metallic=0.2, roughness=0.8)
    create_material_instance("M_Ship_Red", (0.8, 0.1, 0.1), metallic=0.3, roughness=0.5)
    create_material_instance("M_Ship_Blue", (0.1, 0.3, 0.8), metallic=0.3, roughness=0.5)
    create_material_instance("M_Ship_Green", (0.2, 0.6, 0.2), metallic=0.3, roughness=0.5)
    create_material_instance("M_Ship_Yellow", (0.9, 0.9, 0.1), metallic=0.3, roughness=0.5)
    create_material_instance("M_Ship_Orange", (0.9, 0.5, 0.1), metallic=0.3, roughness=0.5)
    create_material_instance("M_Ship_Purple", (0.6, 0.2, 0.8), metallic=0.3, roughness=0.5)
    create_material_instance("M_Ship_Pink", (0.9, 0.4, 0.7), metallic=0.3, roughness=0.5)

    # METALLIC FINISHES
    create_material_instance("M_Ship_Chrome", (0.8, 0.8, 0.8), metallic=1.0, roughness=0.1)
    create_material_instance("M_Ship_Gold", (1.0, 0.84, 0.0), metallic=1.0, roughness=0.2)
    create_material_instance("M_Ship_Silver", (0.75, 0.75, 0.75), metallic=1.0, roughness=0.15)
    create_material_instance("M_Ship_Copper", (0.72, 0.45, 0.20), metallic=1.0, roughness=0.25)
    create_material_instance("M_Ship_Titanium", (0.6, 0.6, 0.7), metallic=0.9, roughness=0.3)
    create_material_instance("M_Ship_Bronze", (0.8, 0.5, 0.2), metallic=1.0, roughness=0.3)

    # SPECIAL EFFECTS (Using emissive for glow effects)
    create_material_instance("M_Ship_Camo_Digital", (0.4, 0.5, 0.4), metallic=0.2, roughness=0.7)
    create_material_instance("M_Ship_Camo_Urban", (0.6, 0.6, 0.6), metallic=0.2, roughness=0.7)
    create_material_instance("M_Ship_Camo_Hex", (0.3, 0.4, 0.3), metallic=0.3, roughness=0.6)
    create_material_instance("M_Ship_Carbon", (0.1, 0.1, 0.1), metallic=0.5, roughness=0.3)
    create_material_instance("M_Ship_Brushed", (0.6, 0.6, 0.6), metallic=0.8, roughness=0.4)

    # GLOWING/EMISSIVE SKINS
    create_material_instance("M_Ship_Neon_Blue", (0.1, 0.3, 0.5), metallic=0.2, roughness=0.5,
                           emissive_strength=5.0, emissive_color=(0.2, 0.5, 1.0))
    create_material_instance("M_Ship_Neon_Green", (0.1, 0.5, 0.1), metallic=0.2, roughness=0.5,
                           emissive_strength=5.0, emissive_color=(0.2, 1.0, 0.2))
    create_material_instance("M_Ship_Neon_Red", (0.5, 0.1, 0.1), metallic=0.2, roughness=0.5,
                           emissive_strength=5.0, emissive_color=(1.0, 0.2, 0.2))
    create_material_instance("M_Ship_Neon_Purple", (0.5, 0.1, 0.5), metallic=0.2, roughness=0.5,
                           emissive_strength=5.0, emissive_color=(0.8, 0.2, 1.0))
    create_material_instance("M_Ship_Holographic", (0.5, 0.5, 0.5), metallic=0.9, roughness=0.1,
                           emissive_strength=3.0, emissive_color=(0.7, 0.7, 1.0))
    create_material_instance("M_Ship_Plasma_Blue", (0.2, 0.3, 0.6), metallic=0.3, roughness=0.4,
                           emissive_strength=8.0, emissive_color=(0.3, 0.5, 1.0))
    create_material_instance("M_Ship_Plasma_Green", (0.2, 0.6, 0.2), metallic=0.3, roughness=0.4,
                           emissive_strength=8.0, emissive_color=(0.3, 1.0, 0.3))
    create_material_instance("M_Ship_Energy_Shield", (0.3, 0.5, 0.8), metallic=0.1, roughness=0.3,
                           emissive_strength=4.0, emissive_color=(0.4, 0.7, 1.0))

    # ANIMATED/SPECIAL EFFECTS
    create_material_instance("M_Ship_Lightning", (0.2, 0.2, 0.4), metallic=0.3, roughness=0.5,
                           emissive_strength=10.0, emissive_color=(0.5, 0.7, 1.0))
    create_material_instance("M_Ship_Fire", (0.4, 0.2, 0.1), metallic=0.2, roughness=0.6,
                           emissive_strength=12.0, emissive_color=(1.0, 0.5, 0.1))
    create_material_instance("M_Ship_Ice", (0.7, 0.8, 0.9), metallic=0.4, roughness=0.2,
                           emissive_strength=2.0, emissive_color=(0.7, 0.9, 1.0))
    create_material_instance("M_Ship_Void", (0.05, 0.05, 0.1), metallic=0.5, roughness=0.7,
                           emissive_strength=1.0, emissive_color=(0.1, 0.0, 0.2))
    create_material_instance("M_Ship_Starfield", (0.1, 0.1, 0.2), metallic=0.3, roughness=0.5,
                           emissive_strength=3.0, emissive_color=(0.3, 0.3, 0.6))

    # LEGENDARY PRESTIGE SKINS
    create_material_instance("M_Ship_Quantum_Flux", (0.4, 0.3, 0.6), metallic=0.7, roughness=0.2,
                           emissive_strength=15.0, emissive_color=(0.6, 0.4, 1.0))
    create_material_instance("M_Ship_Antimatter", (0.2, 0.1, 0.3), metallic=0.5, roughness=0.3,
                           emissive_strength=20.0, emissive_color=(0.8, 0.2, 1.0))
    create_material_instance("M_Ship_Singularity", (0.05, 0.05, 0.05), metallic=0.9, roughness=0.1,
                           emissive_strength=25.0, emissive_color=(0.2, 0.0, 0.5))
    create_material_instance("M_Ship_Dimensional", (0.3, 0.2, 0.4), metallic=0.6, roughness=0.3,
                           emissive_strength=18.0, emissive_color=(0.7, 0.3, 0.9))
    create_material_instance("M_Ship_Temporal", (0.4, 0.3, 0.5), metallic=0.7, roughness=0.2,
                           emissive_strength=22.0, emissive_color=(0.5, 0.7, 1.0))
    create_material_instance("M_Ship_Reality_Bender", (0.5, 0.4, 0.6), metallic=0.8, roughness=0.15,
                           emissive_strength=30.0, emissive_color=(1.0, 0.5, 1.0))
    create_material_instance("M_Ship_God_Mode", (0.9, 0.9, 0.9), metallic=0.5, roughness=0.1,
                           emissive_strength=50.0, emissive_color=(1.0, 1.0, 1.0))

    # TEAM/FACTION COLORS
    create_material_instance("M_Ship_Alliance", (0.2, 0.4, 0.8), metallic=0.4, roughness=0.5)
    create_material_instance("M_Ship_Empire", (0.7, 0.1, 0.1), metallic=0.4, roughness=0.5)
    create_material_instance("M_Ship_Federation", (0.9, 0.9, 0.9), metallic=0.3, roughness=0.4)
    create_material_instance("M_Ship_Pirates", (0.1, 0.1, 0.1), metallic=0.2, roughness=0.7)
    create_material_instance("M_Ship_Mercenary", (0.5, 0.5, 0.5), metallic=0.4, roughness=0.6)

    # RACING/SPORT SKINS
    create_material_instance("M_Ship_Racing_Red", (0.9, 0.1, 0.1), metallic=0.6, roughness=0.3)
    create_material_instance("M_Ship_Racing_Blue", (0.1, 0.2, 0.9), metallic=0.6, roughness=0.3)
    create_material_instance("M_Ship_Checkered", (0.5, 0.5, 0.5), metallic=0.5, roughness=0.4)

    # PRESTIGE/ACHIEVEMENT SKINS
    create_material_instance("M_Ship_Veteran", (0.6, 0.5, 0.4), metallic=0.7, roughness=0.3,
                           emissive_strength=2.0, emissive_color=(0.8, 0.6, 0.3))
    create_material_instance("M_Ship_Elite", (0.3, 0.3, 0.4), metallic=0.8, roughness=0.2,
                           emissive_strength=5.0, emissive_color=(0.5, 0.5, 0.8))
    create_material_instance("M_Ship_Legendary_Pilot", (0.8, 0.6, 0.2), metallic=0.9, roughness=0.1,
                           emissive_strength=10.0, emissive_color=(1.0, 0.8, 0.2))
    create_material_instance("M_Ship_Ultimate", (0.5, 0.3, 0.7), metallic=1.0, roughness=0.05,
                           emissive_strength=100.0, emissive_color=(1.0, 0.8, 1.0))

    print("\n" + "="*70)
    print("MATERIAL GENERATION COMPLETE")
    print("="*70)
    print(f"\nMaterials created in: {OUTPUT_PATH}")
    print("\nNext steps:")
    print("1. Open the master material and add material parameters:")
    print("   - BaseColor (Vector)")
    print("   - Metallic (Scalar)")
    print("   - Roughness (Scalar)")
    print("   - EmissiveStrength (Scalar)")
    print("   - EmissiveColor (Vector)")
    print("2. Connect parameters to appropriate material nodes")
    print("3. Save all assets")

if __name__ == "__main__":
    generate_all_materials()
