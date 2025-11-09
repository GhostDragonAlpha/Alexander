"""
Mission Template Importer for Unreal Engine
Loads JSON mission templates and creates UMissionTemplateDataAssetClass Data Assets

Usage:
    Run this script in Unreal Editor's Python console:
    import load_mission_templates
    load_mission_templates.import_all_mission_templates()
"""

import unreal
import json
import os

# Mapping from JSON mission types to Unreal Enum
MISSION_TYPE_MAP = {
    "Combat": unreal.MissionTypeDataAsset.COMBAT,
    "Exploration": unreal.MissionTypeDataAsset.EXPLORATION,
    "Trading": unreal.MissionTypeDataAsset.TRADING,
    "Diplomatic": unreal.MissionTypeDataAsset.DIPLOMATIC,
    "Rescue": unreal.MissionTypeDataAsset.RESCUE,
    "Stealth": unreal.MissionTypeDataAsset.STEALTH,
    "Mining": unreal.MissionTypeDataAsset.MINING,
    "Construction": unreal.MissionTypeDataAsset.CONSTRUCTION,
    "Escort": unreal.MissionTypeDataAsset.ESCORT,
    "Investigation": unreal.MissionTypeDataAsset.INVESTIGATION,
    "Assassination": unreal.MissionTypeDataAsset.ASSASSINATION,
    "Smuggling": unreal.MissionTypeDataAsset.SMUGGLING,
    "Delivery": unreal.MissionTypeDataAsset.ESCORT,  # Map Delivery to Escort for now
    "Bounty": unreal.MissionTypeDataAsset.COMBAT,  # Map Bounty to Combat
    "Reconnaissance": unreal.MissionTypeDataAsset.EXPLORATION,  # Map Reconnaissance to Exploration
    "Salvage": unreal.MissionTypeDataAsset.EXPLORATION,  # Map Salvage to Exploration
}

# Mission template JSON files location
TEMPLATES_DIR = "C:/Users/allen/Desktop/Alexander/Alexander/Content/Data/Missions/Templates"
OUTPUT_DIR = "/Game/Data/Missions/DataAssets"


def create_mission_objective(obj_data):
    """Create FMissionObjectiveDataAsset from JSON data"""
    objective = unreal.MissionObjectiveDataAsset()

    objective.description = unreal.Text(obj_data.get("description", ""))
    objective.objective_type = unreal.Name(obj_data.get("type", "Unknown"))
    objective.target_count = obj_data.get("quantity", 1)
    objective.current_count = 0
    objective.optional = obj_data.get("optional", False)
    objective.is_complete = False

    return objective


def create_mission_rewards(rewards_data):
    """Create FMissionRewardsDataAsset from JSON data"""
    rewards = unreal.MissionRewardsDataAsset()

    rewards.credits = rewards_data.get("credits", 1000)
    rewards.experience = rewards_data.get("experience", 100)

    # Handle faction reputation
    faction_rep = rewards_data.get("faction_rep", {})
    if faction_rep:
        # Get first faction (simplification - data asset only supports one faction)
        first_faction = list(faction_rep.keys())[0] if faction_rep else "None"
        rewards.faction_id = unreal.Name(first_faction)
        rewards.reputation_gain = faction_rep.get(first_faction, 10)

    # Handle item rewards
    items = rewards_data.get("items", [])
    rewards.item_rewards = [unreal.Name(item) for item in items]

    return rewards


def parse_difficulty(difficulty_str):
    """Convert difficulty string to integer level"""
    difficulty_map = {
        "Easy": 1,
        "Normal": 3,
        "Hard": 5,
        "Very Hard": 7,
        "Extreme": 10
    }
    return difficulty_map.get(difficulty_str, 3)


def create_mission_template(mission_data):
    """Create FMissionTemplateData from JSON data"""
    template = unreal.MissionTemplateData()

    # Basic info
    template.template_id = unreal.Name(mission_data.get("mission_id", "unknown"))

    # Mission type
    mission_type_str = mission_data.get("mission_type", "Combat")
    template.type = MISSION_TYPE_MAP.get(mission_type_str, unreal.MissionTypeDataAsset.COMBAT)

    # Texts (handle template strings - Unreal will need to do variable substitution)
    template.title = unreal.Text(mission_data.get("title_template", mission_data.get("title", "Mission")))
    template.description = unreal.Text(mission_data.get("description_template", mission_data.get("description", "")))
    template.briefing_text = unreal.Text(mission_data.get("briefing", ""))

    # Objectives
    objectives_data = mission_data.get("objectives", [])
    template.objectives = [create_mission_objective(obj) for obj in objectives_data]

    # Rewards
    rewards_data = mission_data.get("rewards", {})
    template.rewards = create_mission_rewards(rewards_data)

    # Requirements
    requirements = mission_data.get("requirements", {})
    template.min_player_level = requirements.get("min_level", 1)
    template.min_reputation = requirements.get("min_reputation", 0)

    # Prerequisites
    prereqs = requirements.get("prerequisite_missions", [])
    template.prerequisite_missions = [unreal.Name(prereq) for prereq in prereqs]

    # Difficulty
    difficulty_str = mission_data.get("difficulty", "Normal")
    template.difficulty_level = parse_difficulty(difficulty_str)

    # Location requirements
    location_data = mission_data.get("location", {})
    location_type = location_data.get("location_type", "Space")
    template.required_tags = [unreal.Name(location_type)]

    # Repeatable
    template.is_repeatable = mission_data.get("repeatable", False)
    template.repeat_cooldown = mission_data.get("cooldown_hours", 24.0) if template.is_repeatable else 0.0

    # Generation weight (default to 100)
    template.generation_weight = 100

    return template


def import_mission_category(json_file_path, category_name):
    """Import all missions from a JSON file into a Data Asset"""

    print(f"Importing {category_name} missions from {json_file_path}...")

    # Read JSON file
    with open(json_file_path, 'r', encoding='utf-8') as f:
        data = json.load(f)

    # Get missions array (format varies by file)
    missions = []
    if isinstance(data, dict):
        # Try different possible keys
        for key in data.keys():
            if isinstance(data[key], list):
                missions = data[key]
                break
    elif isinstance(data, list):
        missions = data

    if not missions:
        print(f"WARNING: No missions found in {json_file_path}")
        return None

    # Create Data Asset
    asset_name = f"DA_{category_name}_Missions"
    asset_path = f"{OUTPUT_DIR}/{asset_name}"

    # Create the data asset
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    data_asset = asset_tools.create_asset(
        asset_name=asset_name,
        package_path=OUTPUT_DIR,
        asset_class=unreal.MissionTemplateDataAssetClass,
        factory=unreal.DataAssetFactory()
    )

    if not data_asset:
        print(f"ERROR: Failed to create data asset {asset_path}")
        return None

    # Set category info
    data_asset.set_editor_property("category_name", unreal.Name(category_name))
    data_asset.set_editor_property("category_description",
                                    unreal.Text(f"{category_name} mission templates"))

    # Convert JSON missions to template data
    templates = []
    for mission_data in missions:
        try:
            template = create_mission_template(mission_data)
            templates.append(template)
        except Exception as e:
            print(f"ERROR: Failed to convert mission {mission_data.get('mission_id', 'unknown')}: {e}")

    # Set templates array
    data_asset.set_editor_property("templates", templates)

    # Save the asset
    unreal.EditorAssetLibrary.save_loaded_asset(data_asset)

    print(f"✓ Created {asset_path} with {len(templates)} missions")

    return data_asset


def import_all_mission_templates():
    """Import all mission template JSON files"""

    print("=" * 80)
    print("MISSION TEMPLATE IMPORT - Starting")
    print("=" * 80)

    # Create output directory if it doesn't exist
    if not unreal.EditorAssetLibrary.does_directory_exist(OUTPUT_DIR):
        unreal.EditorAssetLibrary.make_directory(OUTPUT_DIR)
        print(f"Created directory: {OUTPUT_DIR}")

    # Mission category files
    mission_files = {
        "Combat": "mission_templates_combat.json",
        "Assassination": "mission_templates_assassination.json",
        "Bounty": "mission_templates_bounty.json",
        "Delivery": "mission_templates_delivery.json",
        "Escort": "mission_templates_escort.json",
        "Exploration": "mission_templates_exploration.json",
        "Mining": "mission_templates_mining.json",
        "Reconnaissance": "mission_templates_reconnaissance.json",
        "Rescue": "mission_templates_rescue.json",
        "Salvage": "mission_templates_salvage.json",
        "Smuggling": "mission_templates_smuggling.json",
    }

    created_assets = []

    for category, filename in mission_files.items():
        json_path = os.path.join(TEMPLATES_DIR, filename)

        if not os.path.exists(json_path):
            print(f"WARNING: File not found: {json_path}")
            continue

        try:
            asset = import_mission_category(json_path, category)
            if asset:
                created_assets.append(asset)
        except Exception as e:
            print(f"ERROR: Failed to import {category}: {e}")

    print("=" * 80)
    print(f"MISSION TEMPLATE IMPORT - Complete")
    print(f"Created {len(created_assets)} Data Assets")
    print(f"Location: {OUTPUT_DIR}")
    print("=" * 80)

    return created_assets


def verify_mission_templates():
    """Verify that mission templates were imported correctly"""

    print("=" * 80)
    print("VERIFICATION - Checking Mission Template Data Assets")
    print("=" * 80)

    # Find all mission data assets
    assets = unreal.EditorAssetLibrary.list_assets(OUTPUT_DIR, recursive=False)

    total_missions = 0

    for asset_path in assets:
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)

        if isinstance(asset, unreal.MissionTemplateDataAssetClass):
            templates = asset.get_editor_property("templates")
            category = asset.get_editor_property("category_name")

            print(f"✓ {category}: {len(templates)} missions")
            total_missions += len(templates)

    print("=" * 80)
    print(f"Total missions imported: {total_missions}")
    print("=" * 80)


# Auto-run when script is executed
if __name__ == "__main__":
    print("\n\nMission Template Importer")
    print("To run, execute in Unreal Python console:")
    print("  import load_mission_templates")
    print("  load_mission_templates.import_all_mission_templates()")
    print("  load_mission_templates.verify_mission_templates()")
