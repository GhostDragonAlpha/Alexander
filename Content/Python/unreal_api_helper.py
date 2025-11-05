"""
Unreal Engine 5.5+ Python API Helper
====================================
This module provides compatibility wrappers for deprecated Unreal Python APIs.
Use this instead of directly calling deprecated APIs like EditorLevelLibrary and EditorAssetLibrary.

Updated: 2025-11-04
Compatible with: Unreal Engine 5.5+
"""

import unreal


class UnrealAPIHelper:
    """Helper class providing UE5.5+ compatible API wrappers"""

    def __init__(self):
        # Get subsystems (UE5.5+ recommended approach)
        self.editor_actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        self.editor_asset_subsystem = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
        self.level_editor_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

        # Asset tools
        self.asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    # ========================================================================
    # LEVEL/WORLD OPERATIONS (replaces EditorLevelLibrary)
    # ========================================================================

    def get_editor_world(self):
        """Get current editor world - REPLACEMENT for EditorLevelLibrary.get_editor_world()"""
        return self.level_editor_subsystem.get_current_level()

    def spawn_actor_from_class(self, actor_class, location, rotation=None):
        """Spawn actor from class - REPLACEMENT for EditorLevelLibrary.spawn_actor_from_class()"""
        if rotation is None:
            rotation = unreal.Rotator(0, 0, 0)

        return self.editor_actor_subsystem.spawn_actor_from_class(
            actor_class,
            location,
            rotation
        )

    def spawn_actor_from_object(self, object_to_use, location, rotation=None):
        """Spawn actor from object - REPLACEMENT for EditorLevelLibrary.spawn_actor_from_object()"""
        if rotation is None:
            rotation = unreal.Rotator(0, 0, 0)

        return self.editor_actor_subsystem.spawn_actor_from_object(
            object_to_use,
            location,
            rotation
        )

    def get_all_level_actors(self):
        """Get all actors in level - REPLACEMENT for EditorLevelLibrary.get_all_level_actors()"""
        return self.editor_actor_subsystem.get_all_level_actors()

    def destroy_actor(self, actor):
        """Destroy an actor - REPLACEMENT for EditorLevelLibrary.destroy_actor()"""
        return self.editor_actor_subsystem.destroy_actor(actor)

    def load_level(self, level_path):
        """Load a level - REPLACEMENT for EditorLevelLibrary.load_level()"""
        return self.level_editor_subsystem.load_level(level_path)

    def save_current_level(self):
        """Save current level - REPLACEMENT for EditorLevelLibrary.save_current_level()"""
        return self.level_editor_subsystem.save_current_level()

    # ========================================================================
    # ASSET OPERATIONS (replaces EditorAssetLibrary)
    # ========================================================================

    def does_asset_exist(self, asset_path):
        """Check if asset exists - REPLACEMENT for EditorAssetLibrary.does_asset_exist()"""
        return self.editor_asset_subsystem.does_asset_exist(asset_path)

    def does_directory_exist(self, directory_path):
        """Check if directory exists - REPLACEMENT for EditorAssetLibrary.does_directory_exist()"""
        return self.editor_asset_subsystem.does_directory_exist(directory_path)

    def make_directory(self, directory_path):
        """Create directory - REPLACEMENT for EditorAssetLibrary.make_directory()"""
        return self.editor_asset_subsystem.make_directory(directory_path)

    def load_asset(self, asset_path):
        """Load asset - REPLACEMENT for EditorAssetLibrary.load_asset()"""
        return self.editor_asset_subsystem.load_asset(asset_path)

    def save_loaded_asset(self, asset, destination_path=None):
        """Save loaded asset - REPLACEMENT for EditorAssetLibrary.save_loaded_asset()"""
        if destination_path:
            return self.editor_asset_subsystem.save_asset(destination_path)
        else:
            return self.editor_asset_subsystem.save_loaded_asset(asset)

    def list_assets(self, directory_path, recursive=True):
        """List assets in directory - REPLACEMENT for EditorAssetLibrary.list_assets()"""
        return self.editor_asset_subsystem.list_assets(directory_path, recursive)

    def delete_asset(self, asset_path):
        """Delete asset - REPLACEMENT for EditorAssetLibrary.delete_asset()"""
        return self.editor_asset_subsystem.delete_asset(asset_path)

    def rename_asset(self, source_path, destination_path):
        """Rename asset - REPLACEMENT for EditorAssetLibrary.rename_asset()"""
        return self.editor_asset_subsystem.rename_asset(source_path, destination_path)

    def duplicate_asset(self, source_path, destination_path):
        """Duplicate asset - REPLACEMENT for EditorAssetLibrary.duplicate_asset()"""
        return self.editor_asset_subsystem.duplicate_asset(source_path, destination_path)

    # ========================================================================
    # MATERIAL OPERATIONS (proper MaterialEditingLibrary usage)
    # ========================================================================

    def create_material(self, material_name, package_path):
        """
        Create a new material - PROPER API for material creation

        NOTE: MaterialEditingLibrary.create_material() does NOT exist in UE Python API.
        Use AssetTools.create_asset() with MaterialFactoryNew instead.
        """
        # Ensure directory exists
        if not self.does_directory_exist(package_path):
            self.make_directory(package_path)

        # Create material using factory
        factory = unreal.MaterialFactoryNew()

        asset = self.asset_tools.create_asset(
            asset_name=material_name,
            package_path=package_path,
            asset_class=unreal.Material,
            factory=factory
        )

        return asset

    def set_material_scalar_parameter(self, material, parameter_name, value):
        """Set material scalar parameter"""
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            material, parameter_name, value
        )

    def set_material_vector_parameter(self, material, parameter_name, value):
        """Set material vector parameter"""
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
            material, parameter_name, value
        )

    # ========================================================================
    # C++ CLASS LOADING (proper approach for non-Blueprint classes)
    # ========================================================================

    def load_cpp_class(self, class_path):
        """
        Load a C++ class - PROPER way to load native classes

        DO NOT use EditorAssetLibrary.load_blueprint_class() for C++ classes!
        That function is ONLY for Blueprint classes.

        Examples:
            - For C++ class: use load_cpp_class("/Script/Alexander.Sun")
            - For Blueprint: use load_asset("/Game/Blueprints/BP_Sun.BP_Sun_C")
        """
        try:
            # Try to load as a C++ class
            loaded_class = unreal.load_class(None, class_path)
            if loaded_class:
                return loaded_class
        except Exception as e:
            unreal.log_warning(f"Failed to load C++ class '{class_path}': {e}")

        return None

    def load_blueprint_class(self, blueprint_path):
        """
        Load a Blueprint class - PROPER way to load Blueprint classes

        For Blueprint classes, you need to append "_C" to the asset path.
        Example: "/Game/Blueprints/BP_Sun.BP_Sun_C"
        """
        try:
            # Load the blueprint asset first
            blueprint_asset = self.load_asset(blueprint_path)

            if blueprint_asset and hasattr(blueprint_asset, 'generated_class'):
                return blueprint_asset.generated_class()
        except Exception as e:
            unreal.log_warning(f"Failed to load Blueprint class '{blueprint_path}': {e}")

        return None

    def spawn_cpp_actor(self, class_path, location, rotation=None):
        """
        Spawn an actor from a C++ class path

        Example: spawn_cpp_actor("/Script/Alexander.Sun", unreal.Vector(0,0,0))
        """
        actor_class = self.load_cpp_class(class_path)
        if actor_class:
            return self.spawn_actor_from_class(actor_class, location, rotation)
        else:
            unreal.log_error(f"Failed to spawn actor: C++ class not found: {class_path}")
            return None


# ========================================================================
# GLOBAL HELPER INSTANCE
# ========================================================================

# Create a global instance for easy importing
_helper_instance = None

def get_unreal_helper():
    """Get or create the global UnrealAPIHelper instance"""
    global _helper_instance
    if _helper_instance is None:
        _helper_instance = UnrealAPIHelper()
    return _helper_instance


# ========================================================================
# BACKWARD COMPATIBILITY LAYER
# ========================================================================

class EditorLevelLibraryCompat:
    """
    Backward compatibility wrapper for EditorLevelLibrary
    Use this as a drop-in replacement:

    Before: editor_level_lib = unreal.EditorLevelLibrary
    After:  editor_level_lib = EditorLevelLibraryCompat()
    """

    def __init__(self):
        self._helper = get_unreal_helper()

    def get_editor_world(self):
        return self._helper.get_editor_world()

    def spawn_actor_from_class(self, actor_class, location, rotation):
        return self._helper.spawn_actor_from_class(actor_class, location, rotation)

    def spawn_actor_from_object(self, object_to_use, location, rotation):
        return self._helper.spawn_actor_from_object(object_to_use, location, rotation)

    def get_all_level_actors(self):
        return self._helper.get_all_level_actors()

    def destroy_actor(self, actor):
        return self._helper.destroy_actor(actor)

    def load_level(self, level_path):
        return self._helper.load_level(level_path)

    def save_current_level(self):
        return self._helper.save_current_level()


class EditorAssetLibraryCompat:
    """
    Backward compatibility wrapper for EditorAssetLibrary
    Use this as a drop-in replacement:

    Before: editor_asset_lib = unreal.EditorAssetLibrary()
    After:  editor_asset_lib = EditorAssetLibraryCompat()
    """

    def __init__(self):
        self._helper = get_unreal_helper()

    def does_asset_exist(self, asset_path):
        return self._helper.does_asset_exist(asset_path)

    def does_directory_exist(self, directory_path):
        return self._helper.does_directory_exist(directory_path)

    def make_directory(self, directory_path):
        return self._helper.make_directory(directory_path)

    def load_asset(self, asset_path):
        return self._helper.load_asset(asset_path)

    def save_loaded_asset(self, asset, destination_path=None):
        return self._helper.save_loaded_asset(asset, destination_path)

    def list_assets(self, directory_path, recursive=True):
        return self._helper.list_assets(directory_path, recursive)

    def delete_asset(self, asset_path):
        return self._helper.delete_asset(asset_path)

    def rename_asset(self, source_path, destination_path):
        return self._helper.rename_asset(source_path, destination_path)

    def duplicate_asset(self, source_path, destination_path):
        return self._helper.duplicate_asset(source_path, destination_path)

    def load_blueprint_class(self, blueprint_path):
        """DEPRECATED: Use load_cpp_class for C++ classes or load_blueprint_class for Blueprints"""
        # This is the problematic method - try to intelligently route the call
        if "/Script/" in blueprint_path:
            # It's a C++ class
            return self._helper.load_cpp_class(blueprint_path)
        else:
            # It's a Blueprint
            return self._helper.load_blueprint_class(blueprint_path)


# ========================================================================
# USAGE EXAMPLES
# ========================================================================

if __name__ == "__main__":
    # Example usage:

    # New way (recommended):
    helper = get_unreal_helper()
    actor = helper.spawn_cpp_actor("/Script/Engine.StaticMeshActor", unreal.Vector(0, 0, 0))

    # Backward compatible way (for minimal code changes):
    editor_level_lib = EditorLevelLibraryCompat()
    editor_asset_lib = EditorAssetLibraryCompat()

    # Now you can use them as before, but with UE5.5+ APIs under the hood
    world = editor_level_lib.get_editor_world()
    exists = editor_asset_lib.does_asset_exist("/Game/SomeAsset")
