"""
Autonomous UI Generator for Unreal Engine
Creates baseline UI widgets using UMG (Unreal Motion Graphics)

Usage in Unreal Editor Python console:
    import autonomous_ui_generator
    autonomous_ui_generator.document_ui_requirements()
    autonomous_ui_generator.create_ui_templates()
"""

import unreal

# Output directory for UI widgets
OUTPUT_DIR = "/Game/UI/Generated"

# UI Widget Configurations
UI_CONFIGS = {
    # HUD Components
    "HUD_SpeedIndicator": {
        "category": "hud",
        "type": "progress_bar",
        "description": "Current speed display with visual bar",
        "position": "bottom_left",
        "size": (200, 50),
        "components": [
            {"type": "text", "label": "Speed:"},
            {"type": "progress_bar", "variable": "current_speed"},
            {"type": "text", "value": "{speed} m/s"}
        ]
    },
    "HUD_AltitudeDisplay": {
        "category": "hud",
        "type": "text",
        "description": "Altitude/distance from nearest body",
        "position": "bottom_left",
        "size": (200, 30),
        "components": [
            {"type": "text", "label": "Altitude:"},
            {"type": "text", "value": "{altitude} km"}
        ]
    },
    "HUD_FuelGauge": {
        "category": "hud",
        "type": "progress_bar",
        "description": "Fuel level indicator",
        "position": "top_right",
        "size": (150, 40),
        "components": [
            {"type": "image", "icon": "fuel_icon"},
            {"type": "progress_bar", "variable": "fuel_level", "color": "orange"},
            {"type": "text", "value": "{fuel}%"}
        ]
    },
    "HUD_ShieldBar": {
        "category": "hud",
        "type": "progress_bar",
        "description": "Shield strength bar",
        "position": "top_left",
        "size": (200, 30),
        "components": [
            {"type": "progress_bar", "variable": "shield_strength", "color": "blue"},
            {"type": "text", "value": "{shields}%"}
        ]
    },
    "HUD_HealthBar": {
        "category": "hud",
        "type": "progress_bar",
        "description": "Hull health bar",
        "position": "top_left",
        "size": (200, 30),
        "components": [
            {"type": "progress_bar", "variable": "hull_health", "color": "red"},
            {"type": "text", "value": "{health}%"}
        ]
    },
    "HUD_WeaponStatus": {
        "category": "hud",
        "type": "horizontal_box",
        "description": "Weapon status and ammo",
        "position": "bottom_right",
        "size": (250, 80),
        "components": [
            {"type": "image", "icon": "weapon_icon"},
            {"type": "text", "label": "Weapon:"},
            {"type": "text", "value": "{weapon_name}"},
            {"type": "progress_bar", "variable": "weapon_heat", "color": "orange"},
            {"type": "text", "value": "Ammo: {ammo}/{max_ammo}"}
        ]
    },
    "HUD_Minimap": {
        "category": "hud",
        "type": "image",
        "description": "Minimap/radar display",
        "position": "top_right",
        "size": (150, 150),
        "components": [
            {"type": "image", "render_target": "minimap_rt"},
            {"type": "border", "style": "radar_border"}
        ]
    },
    "HUD_ObjectiveTracker": {
        "category": "hud",
        "type": "vertical_box",
        "description": "Active mission objectives",
        "position": "right",
        "size": (300, 200),
        "components": [
            {"type": "text", "label": "Objectives:", "style": "header"},
            {"type": "scroll_box", "dynamic_content": "objectives_list"}
        ]
    },
    "HUD_TargetInfo": {
        "category": "hud",
        "type": "overlay",
        "description": "Current target information",
        "position": "center_top",
        "size": (250, 100),
        "components": [
            {"type": "text", "value": "{target_name}"},
            {"type": "text", "value": "Distance: {distance}m"},
            {"type": "progress_bar", "variable": "target_health", "color": "red"}
        ]
    },

    # Mission Board UI
    "UI_MissionBoard": {
        "category": "menu",
        "type": "canvas_panel",
        "description": "Mission selection board",
        "size": (800, 600),
        "components": [
            {"type": "border", "style": "panel_background"},
            {"type": "text", "label": "Available Missions", "style": "title"},
            {"type": "scroll_box", "content": "mission_list"},
            {"type": "button", "label": "Accept Mission"},
            {"type": "button", "label": "Close"}
        ]
    },
    "UI_MissionCard": {
        "category": "menu",
        "type": "vertical_box",
        "description": "Individual mission display card",
        "size": (350, 150),
        "components": [
            {"type": "image", "icon": "mission_type_icon"},
            {"type": "text", "value": "{mission_title}", "style": "mission_title"},
            {"type": "text", "value": "{mission_description}"},
            {"type": "horizontal_box", "content": [
                {"type": "text", "label": "Reward:"},
                {"type": "image", "icon": "credits_icon"},
                {"type": "text", "value": "{reward_credits}"}
            ]},
            {"type": "text", "value": "Difficulty: {difficulty}"}
        ]
    },

    # Main Menu
    "UI_MainMenu": {
        "category": "menu",
        "type": "canvas_panel",
        "description": "Main menu screen",
        "size": (1920, 1080),
        "components": [
            {"type": "image", "background": "space_background"},
            {"type": "text", "label": "Alexander", "style": "title_large"},
            {"type": "vertical_box", "buttons": [
                {"type": "button", "label": "New Game"},
                {"type": "button", "label": "Continue"},
                {"type": "button", "label": "Settings"},
                {"type": "button", "label": "Exit"}
            ]}
        ]
    },

    # Trading Interface
    "UI_TradingInterface": {
        "category": "menu",
        "type": "horizontal_box",
        "description": "Trading/shop interface",
        "size": (1200, 700),
        "components": [
            {"type": "vertical_box", "label": "Your Inventory"},
            {"type": "vertical_box", "label": "Station Inventory"},
            {"type": "vertical_box", "label": "Transaction Summary"}
        ]
    },

    # Ship Customization
    "UI_ShipCustomization": {
        "category": "menu",
        "type": "overlay",
        "description": "Ship customization screen",
        "size": (1400, 800),
        "components": [
            {"type": "viewport", "display": "ship_preview"},
            {"type": "scroll_box", "content": "parts_list"},
            {"type": "vertical_box", "content": "stats_display"}
        ]
    },

    # Settings Menu
    "UI_SettingsMenu": {
        "category": "menu",
        "type": "vertical_box",
        "description": "Game settings",
        "size": (600, 500),
        "components": [
            {"type": "text", "label": "Settings", "style": "header"},
            {"type": "slider", "label": "Master Volume"},
            {"type": "slider", "label": "Graphics Quality"},
            {"type": "checkbox", "label": "VSync"},
            {"type": "checkbox", "label": "Motion Blur"},
            {"type": "button", "label": "Apply"},
            {"type": "button", "label": "Cancel"}
        ]
    },

    # Tutorial Overlay
    "UI_TutorialOverlay": {
        "category": "tutorial",
        "type": "overlay",
        "description": "Tutorial instruction overlay",
        "size": (400, 200),
        "components": [
            {"type": "border", "style": "tutorial_background"},
            {"type": "text", "value": "{tutorial_text}"},
            {"type": "image", "icon": "{input_icon}"},
            {"type": "progress_bar", "variable": "tutorial_progress"}
        ]
    },

    # Notification Pop-up
    "UI_Notification": {
        "category": "ui_feedback",
        "type": "border",
        "description": "Notification pop-up message",
        "position": "top_center",
        "size": (350, 80),
        "components": [
            {"type": "image", "icon": "notification_icon"},
            {"type": "text", "value": "{notification_text}"}
        ]
    }
}


def document_ui_requirements():
    """Document all UI requirements"""
    unreal.log("=" * 80)
    unreal.log("UI REQUIREMENTS DOCUMENTATION")
    unreal.log("=" * 80)

    categories = {}
    for name, config in UI_CONFIGS.items():
        category = config["category"]
        if category not in categories:
            categories[category] = []
        categories[category].append((name, config))

    for category, widgets in categories.items():
        unreal.log(f"\n{category.upper().replace('_', ' ')}:")
        unreal.log("-" * 40)
        for name, config in widgets:
            unreal.log(f"  {name}")
            unreal.log(f"    Description: {config['description']}")
            unreal.log(f"    Type: {config['type']}")
            if 'size' in config:
                unreal.log(f"    Size: {config['size']}")
            if 'position' in config:
                unreal.log(f"    Position: {config['position']}")
            unreal.log(f"    Components: {len(config.get('components', []))}")
            unreal.log("")

    unreal.log("=" * 80)
    unreal.log(f"TOTAL UI WIDGETS: {len(UI_CONFIGS)}")
    unreal.log("=" * 80)


def create_ui_directory_structure():
    """Create directory structure for UI assets"""
    unreal.log("Creating UI directory structure...")

    # Create main directory
    if not unreal.EditorAssetLibrary.does_directory_exist(OUTPUT_DIR):
        unreal.EditorAssetLibrary.make_directory(OUTPUT_DIR)

    # Create category subdirectories
    categories = ["hud", "menu", "tutorial", "ui_feedback", "icons"]

    for category in categories:
        category_dir = f"{OUTPUT_DIR}/{category}"
        if not unreal.EditorAssetLibrary.does_directory_exist(category_dir):
            unreal.EditorAssetLibrary.make_directory(category_dir)
            unreal.log(f"  Created: {category_dir}")

    unreal.log("✓ UI directory structure created")


def create_ui_templates():
    """Create UI template structure"""
    unreal.log("=" * 80)
    unreal.log("UI TEMPLATE CREATION - Starting")
    unreal.log("=" * 80)

    create_ui_directory_structure()

    unreal.log("\n" + "=" * 80)
    unreal.log("UI TEMPLATE CREATION - Complete")
    unreal.log(f"Directory structure created for {len(UI_CONFIGS)} UI widgets")
    unreal.log("\nNOTE: UMG widgets should be created manually or from blueprints:")
    unreal.log("  1. Right-click in Content Browser → User Interface → Widget Blueprint")
    unreal.log("  2. Add components per UI_CONFIGS specifications")
    unreal.log("  3. Save to appropriate category folder")
    unreal.log("  4. Reference: UI_CONFIGS in this script")
    unreal.log("=" * 80)


def export_ui_config():
    """Export UI configuration to JSON for reference"""
    import json

    config_path = "C:/Users/allen/Desktop/Alexander/Alexander/Content/Data/ui_config.json"

    # Convert to JSON-serializable format
    export_data = {}
    for name, config in UI_CONFIGS.items():
        export_data[name] = {
            "category": config["category"],
            "description": config["description"],
            "type": config["type"],
            "size": config.get("size", (0, 0)),
            "position": config.get("position", "custom"),
            "num_components": len(config.get("components", []))
        }

    with open(config_path, 'w', encoding='utf-8') as f:
        json.dump(export_data, f, indent=2)

    unreal.log(f"✓ Exported UI configuration to: {config_path}")
    return config_path


def get_ui_summary():
    """Get summary of UI widgets by category"""
    summary = {}
    for name, config in UI_CONFIGS.items():
        category = config["category"]
        if category not in summary:
            summary[category] = 0
        summary[category] += 1

    unreal.log("\nUI Summary by Category:")
    for category, count in summary.items():
        unreal.log(f"  {category}: {count} widgets")

    unreal.log(f"\nTotal: {len(UI_CONFIGS)} UI widgets")

    return summary


if __name__ == "__main__":
    unreal.log("\nAutonomous UI Generator")
    unreal.log("Usage:")
    unreal.log("  import autonomous_ui_generator")
    unreal.log("  autonomous_ui_generator.document_ui_requirements()")
    unreal.log("  autonomous_ui_generator.create_ui_templates()")
    unreal.log("  autonomous_ui_generator.export_ui_config()")
    unreal.log("  autonomous_ui_generator.get_ui_summary()")
