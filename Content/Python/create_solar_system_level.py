"""
Create a brand new clean Solar System level
This will be the new default level, replacing AIVisionTestArena
"""

import unreal

def create_clean_solar_system_level():
    """Create a new empty level for the solar system"""

    unreal.log("=" * 70)
    unreal.log("CREATING NEW SOLAR SYSTEM LEVEL")
    unreal.log("=" * 70)

    level_path = '/Game/SolarSystem'

    # Check if level already exists
    if unreal.EditorAssetLibrary.does_asset_exist(level_path):
        unreal.log(f"Level {level_path} already exists")
        unreal.EditorLevelLibrary.load_level(level_path)
        unreal.log(f"✅ Loaded existing level: {level_path}")
        return True

    # Create new empty level
    try:
        unreal.EditorLevelLibrary.new_level(level_path)
        unreal.log(f"✅ Created new level: {level_path}")

        # The level is now open and ready for population
        unreal.log("Level is clean and ready for solar system setup")
        unreal.log("=" * 70)
        return True

    except Exception as e:
        unreal.log_error(f"Failed to create level: {e}")
        return False

if __name__ == "__main__":
    create_clean_solar_system_level()
