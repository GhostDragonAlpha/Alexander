
import unreal

# Spawn ship
editor_subsystem = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
world = editor_subsystem.get_editor_world()

ship_asset = unreal.EditorAssetLibrary.load_asset("/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer")
if ship_asset:
    ship_class = ship_asset.generated_class()
    ship = actor_subsystem.spawn_actor_from_class(
        ship_class,
        unreal.Vector(400, 0, 300),
        unreal.Rotator(0, 0, 0)
    )
    if ship:
        print(f"SPAWN_SUCCESS:{ship.get_name()}")
    else:
        print("SPAWN_FAILED")
else:
    print("SPAWN_FAILED:CLASS_NOT_FOUND")
