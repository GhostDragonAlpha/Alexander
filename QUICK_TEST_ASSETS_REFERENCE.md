# Quick Test Assets Reference Card

Quick reference for the most commonly used assets in automated testing.

## Copy-Paste Ready Asset Paths

### Maps
```
/Game/FlightTest
/Game/VRTemplate/Maps/VRTemplateMap
/Game/SolarSystem
/Game/Maps/TestLevel_Comprehensive
```

### Blueprints
```
/Game/VRTemplate/Blueprints/VRPawn
/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer
/Game/SpaceShip/Blueprints/BP_SpaceshipCockpit
/Game/VRTemplate/Blueprints/Pistol
/Game/LevelPrototyping/Interactable/JumpPad/BP_JumpPad
```

### Materials
```
/Game/Materials/M_Sun_Simple
/Game/Materials/M_Planet_Base
/Game/LevelPrototyping/Materials/M_PrototypeGrid
/Game/VRTemplate/Materials/M_VRCursor
```

### Textures
```
/Game/VRTemplate/Textures/T_Grid
/Game/LevelPrototyping/Textures/T_GridChecker_A
/Game/Characters/MannequinsXR/Textures/Manny/T_Manny_01_D
```

### Static Meshes
```
/Game/LevelPrototyping/Meshes/SM_Cube
/Game/LevelPrototyping/Meshes/SM_Cylinder
/Game/LevelPrototyping/Meshes/SM_Ramp
```

### Data Assets
```
/Game/Data/Missions/DataAssets/DA_Combat_Missions
/Game/Data/Missions/DataAssets/DA_Delivery_Missions
/Game/Data/Missions/DataAssets/DA_Mining_Missions
```

## Python Test Examples

### Asset Validation Test
```python
response = requests.get(
    f"{API_BASE}/validate_asset//Game/VRTemplate/Blueprints/VRPawn",
    timeout=5
)
```

### Asset Reload Test
```python
response = requests.post(
    f"{API_BASE}/reload_asset",
    json={"asset_path": "/Game/VRTemplate/Blueprints/VRPawn"},
    headers={"Content-Type": "application/json"}
)
```

### Asset Dependencies Test
```python
response = requests.get(
    f"{API_BASE}/get_asset_dependencies//Game/VRTemplate/Blueprints/VRPawn",
    timeout=5
)
```

### Map Loading Test
```python
response = requests.post(
    f"{API_BASE}/load_map",
    json={"map_path": "/Game/VRTemplate/Maps/VRTemplateMap"},
    headers={"Content-Type": "application/json"}
)
```

### List Assets Test
```python
response = requests.post(
    f"{API_BASE}/list_assets",
    json={"filter": {"class": "Blueprint"}},
    headers={"Content-Type": "application/json"}
)
```

## Asset Categories Quick Count

- **Maps:** 7 verified
- **Blueprints:** 40+ verified
- **Materials:** 20+ verified
- **Textures:** 30+ verified
- **Static Meshes:** 15+ verified
- **Mission Data Assets:** 11 verified

## Common Mistakes to Avoid

1. **Don't include file extensions** (.uasset, .umap)
   - WRONG: `/Game/VRTemplate/Blueprints/VRPawn.uasset`
   - RIGHT: `/Game/VRTemplate/Blueprints/VRPawn`

2. **Don't use disk paths** (use Unreal content paths)
   - WRONG: `Content/VRTemplate/Blueprints/VRPawn`
   - RIGHT: `/Game/VRTemplate/Blueprints/VRPawn`

3. **Don't test disabled assets**
   - WRONG: `/Game/Source/Alexander/Public/ProceduralMissionGenerator.h.disabled`
   - RIGHT: Use only active, enabled assets

4. **Don't assume BP_Spaceship exists**
   - WRONG: `/Game/Blueprints/BP_Spaceship` (doesn't exist!)
   - RIGHT: `/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer`

## Most Reliable Test Assets (Recommended)

These assets are battle-tested and have minimal dependencies:

1. **Blueprint:** `/Game/VRTemplate/Blueprints/VRPawn`
2. **Map:** `/Game/VRTemplate/Maps/VRTemplateMap`
3. **Material:** `/Game/Materials/M_Sun_Simple`
4. **Texture:** `/Game/VRTemplate/Textures/T_Grid`
5. **Mesh:** `/Game/LevelPrototyping/Meshes/SM_Cube`
6. **Data Asset:** `/Game/Data/Missions/DataAssets/DA_Combat_Missions`

## Full Documentation

For complete asset listings, see: **VALID_TEST_ASSETS.md**

## Last Updated

2025-11-09 - Initial creation after test data fixes
