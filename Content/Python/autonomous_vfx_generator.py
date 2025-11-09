"""
Autonomous VFX Generator for Unreal Engine
Creates baseline VFX systems using Cascade (legacy) or documented Niagara templates

Note: Niagara system creation via Python is complex. This script documents
      the VFX requirements and can spawn template-based systems.

Usage in Unreal Editor Python console:
    import autonomous_vfx_generator
    autonomous_vfx_generator.document_vfx_requirements()
    autonomous_vfx_generator.create_vfx_templates()
"""

import unreal

# Output directory for VFX
OUTPUT_DIR = "/Game/VFX/Generated"

# VFX System Configurations
VFX_CONFIGS = {
    # Ship Engine Trails
    "EngineTrail_Small": {
        "category": "ship_engines",
        "type": "niagara",
        "description": "Small fighter engine trail",
        "color": (0.2, 0.5, 1.0),  # Blue
        "size": "small",
        "intensity": 0.7,
        "template": "NS_SmokeTrail"  # Use built-in smoke trail template
    },
    "EngineTrail_Medium": {
        "category": "ship_engines",
        "type": "niagara",
        "description": "Medium transport engine trail",
        "color": (0.3, 0.6, 1.0),
        "size": "medium",
        "intensity": 0.8,
        "template": "NS_SmokeTrail"
    },
    "EngineTrail_Large": {
        "category": "ship_engines",
        "type": "niagara",
        "description": "Large freighter engine trail",
        "color": (0.4, 0.7, 1.0),
        "size": "large",
        "intensity": 1.0,
        "template": "NS_SmokeTrail"
    },
    "EngineTrail_Military": {
        "category": "ship_engines",
        "type": "niagara",
        "description": "Military combat engine (orange/red)",
        "color": (1.0, 0.4, 0.1),  # Orange-red
        "size": "medium",
        "intensity": 0.9,
        "template": "NS_Fire"
    },
    "EngineTrail_Racing": {
        "category": "ship_engines",
        "type": "niagara",
        "description": "Racing engine (bright blue plasma)",
        "color": (0.1, 0.8, 1.0),  # Bright blue
        "size": "small",
        "intensity": 1.0,
        "template": "NS_Electric"
    },

    # Weapon Effects
    "Weapon_LaserBeam": {
        "category": "weapons",
        "type": "niagara",
        "description": "Continuous laser beam",
        "color": (1.0, 0.2, 0.2),  # Red
        "beam": True,
        "template": "NS_BeamTrail"
    },
    "Weapon_LaserImpact": {
        "category": "weapons",
        "type": "niagara",
        "description": "Laser impact sparks",
        "color": (1.0, 0.8, 0.3),  # Yellow-orange
        "burst": True,
        "template": "NS_Sparks"
    },
    "Weapon_ProjectileTrail": {
        "category": "weapons",
        "type": "niagara",
        "description": "Projectile weapon trail",
        "color": (1.0, 0.5, 0.0),  # Orange
        "template": "NS_SmokeTrail"
    },
    "Weapon_ProjectileExplosion": {
        "category": "weapons",
        "type": "niagara",
        "description": "Projectile explosion",
        "color": (1.0, 0.3, 0.0),  # Red-orange
        "burst": True,
        "template": "NS_Explosion"
    },
    "Weapon_MissileTrail": {
        "category": "weapons",
        "type": "niagara",
        "description": "Missile smoke trail",
        "color": (0.8, 0.8, 0.8),  # Gray smoke
        "template": "NS_SmokeTrail"
    },
    "Weapon_MissileExplosion": {
        "category": "weapons",
        "type": "niagara",
        "description": "Large missile explosion",
        "color": (1.0, 0.4, 0.0),  # Orange
        "burst": True,
        "large": True,
        "template": "NS_Explosion"
    },

    # Environment Effects
    "Env_SolarWind": {
        "category": "environment",
        "type": "niagara",
        "description": "Solar wind particles",
        "color": (1.0, 0.9, 0.7),  # Pale yellow
        "template": "NS_ParticleFlow"
    },
    "Env_AsteroidDebris": {
        "category": "environment",
        "type": "niagara",
        "description": "Asteroid debris field",
        "color": (0.5, 0.5, 0.5),  # Gray
        "template": "NS_Dust"
    },
    "Env_NebulaFog": {
        "category": "environment",
        "type": "niagara",
        "description": "Nebula fog/clouds",
        "color": (0.6, 0.3, 0.8),  # Purple
        "template": "NS_Fog"
    },
    "Env_ElectricalStorm": {
        "category": "environment",
        "type": "niagara",
        "description": "Electrical storm arcs",
        "color": (0.3, 0.7, 1.0),  # Electric blue
        "template": "NS_Lightning"
    },

    # UI Effects
    "UI_DamageIndicator": {
        "category": "ui",
        "type": "niagara",
        "description": "Damage indicator particles",
        "color": (1.0, 0.0, 0.0),  # Red
        "screen_space": True,
        "template": "NS_Sparks"
    },
    "UI_ShieldHit": {
        "category": "ui",
        "type": "niagara",
        "description": "Shield hit ripple effect",
        "color": (0.0, 0.5, 1.0),  # Blue
        "screen_space": True,
        "template": "NS_Ripple"
    }
}


def document_vfx_requirements():
    """Document all VFX requirements for manual/automated creation"""
    unreal.log("=" * 80)
    unreal.log("VFX REQUIREMENTS DOCUMENTATION")
    unreal.log("=" * 80)

    categories = {}
    for name, config in VFX_CONFIGS.items():
        category = config["category"]
        if category not in categories:
            categories[category] = []
        categories[category].append((name, config))

    for category, systems in categories.items():
        unreal.log(f"\n{category.upper().replace('_', ' ')}:")
        unreal.log("-" * 40)
        for name, config in systems:
            unreal.log(f"  {name}")
            unreal.log(f"    Description: {config['description']}")
            unreal.log(f"    Template: {config.get('template', 'Custom')}")
            unreal.log(f"    Color: RGB{config.get('color', (1,1,1))}")
            if 'size' in config:
                unreal.log(f"    Size: {config['size']}")
            if 'burst' in config:
                unreal.log(f"    Type: Burst/One-shot")
            if 'beam' in config:
                unreal.log(f"    Type: Beam")
            unreal.log("")

    unreal.log("=" * 80)
    unreal.log(f"TOTAL VFX SYSTEMS: {len(VFX_CONFIGS)}")
    unreal.log("=" * 80)


def create_vfx_placeholder_blueprint(vfx_name, config):
    """
    Create a Blueprint actor that can serve as a VFX placeholder
    Will use particle system component when Niagara assets are created
    """
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    blueprint_name = f"BP_VFX_{vfx_name}"
    blueprint_path = f"{OUTPUT_DIR}/{config['category']}/{blueprint_name}"

    # Create directory structure
    category_dir = f"{OUTPUT_DIR}/{config['category']}"
    if not unreal.EditorAssetLibrary.does_directory_exist(category_dir):
        unreal.EditorAssetLibrary.make_directory(category_dir)

    unreal.log(f"  Creating VFX placeholder: {vfx_name}")

    # For now, log the requirements - actual Niagara creation is complex
    # and better done through Editor UI or content examples

    return blueprint_path


def create_vfx_templates():
    """Create VFX template placeholders"""
    unreal.log("=" * 80)
    unreal.log("VFX TEMPLATE CREATION - Starting")
    unreal.log("=" * 80)

    # Create output directory
    if not unreal.EditorAssetLibrary.does_directory_exist(OUTPUT_DIR):
        unreal.EditorAssetLibrary.make_directory(OUTPUT_DIR)

    created = {}

    for category in ["ship_engines", "weapons", "environment", "ui"]:
        category_dir = f"{OUTPUT_DIR}/{category}"
        if not unreal.EditorAssetLibrary.does_directory_exist(category_dir):
            unreal.EditorAssetLibrary.make_directory(category_dir)
            unreal.log(f"Created directory: {category_dir}")

    unreal.log("\n" + "=" * 80)
    unreal.log("VFX TEMPLATE CREATION - Complete")
    unreal.log(f"Created directory structure for {len(VFX_CONFIGS)} VFX systems")
    unreal.log("\nNOTE: Niagara systems should be created manually or from templates:")
    unreal.log("  1. Right-click in Content Browser → Niagara System")
    unreal.log("  2. Choose template (Smoke Trail, Explosion, etc.)")
    unreal.log("  3. Customize color and intensity per VFX_CONFIGS")
    unreal.log("  4. Save to appropriate category folder")
    unreal.log("=" * 80)

    return created


def export_vfx_config():
    """Export VFX configuration to JSON for reference"""
    import json

    config_path = "C:/Users/allen/Desktop/Alexander/Alexander/Content/Data/vfx_config.json"

    # Convert to JSON-serializable format
    export_data = {}
    for name, config in VFX_CONFIGS.items():
        export_data[name] = {
            "category": config["category"],
            "description": config["description"],
            "color_rgb": config.get("color", (1, 1, 1)),
            "template": config.get("template", "Custom"),
            "size": config.get("size", "medium"),
            "type": config.get("type", "niagara")
        }

    with open(config_path, 'w', encoding='utf-8') as f:
        json.dump(export_data, f, indent=2)

    unreal.log(f"✓ Exported VFX configuration to: {config_path}")
    return config_path


def get_vfx_summary():
    """Get summary of VFX systems by category"""
    summary = {}
    for name, config in VFX_CONFIGS.items():
        category = config["category"]
        if category not in summary:
            summary[category] = 0
        summary[category] += 1

    unreal.log("\nVFX Summary by Category:")
    for category, count in summary.items():
        unreal.log(f"  {category}: {count} systems")

    unreal.log(f"\nTotal: {len(VFX_CONFIGS)} VFX systems")

    return summary


if __name__ == "__main__":
    unreal.log("\nAutonomous VFX Generator")
    unreal.log("Usage:")
    unreal.log("  import autonomous_vfx_generator")
    unreal.log("  autonomous_vfx_generator.document_vfx_requirements()")
    unreal.log("  autonomous_vfx_generator.create_vfx_templates()")
    unreal.log("  autonomous_vfx_generator.export_vfx_config()")
    unreal.log("  autonomous_vfx_generator.get_vfx_summary()")
