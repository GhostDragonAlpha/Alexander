# Valid Test Assets Reference

This document lists verified assets that exist in the Alexander project for use in automated testing.

## Maps/Levels

### Primary Test Maps
- `/Game/FlightTest` - Main flight testing map
- `/Game/VRTemplate/Maps/VRTemplateMap` - VR template testing map
- `/Game/SolarSystem` - Solar system simulation map
- `/Game/AIVisionTestArena` - AI vision testing arena

### Specialized Test Maps
- `/Game/Maps/TestLevel_Comprehensive` - Comprehensive testing level
- `/Game/Maps/TestMap_Backup` - Backup test map
- `/Game/SciFi_Props/Maps/Lvl_Showroom` - SciFi props showroom

## Blueprints

### Spaceship Blueprints
- `/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer` - VR spaceship player controller
- `/Game/SpaceShip/Blueprints/BP_SpaceshipCockpit` - Spaceship cockpit
- `/Game/SpaceShip/Blueprints/BP_FlightPhysicsController` - Flight physics controller
- `/Game/SpaceShip/Blueprints/BP_OrbitalMechanics` - Orbital mechanics system
- `/Game/SpaceShip/Blueprints/BP_GravityWell` - Gravity well simulation

### Spaceship Controls
- `/Game/SpaceShip/Blueprints/BP_FlightStick` - Flight stick controller
- `/Game/SpaceShip/Blueprints/BP_ThrottleControl` - Throttle control
- `/Game/SpaceShip/Blueprints/BP_ButtonPanel` - Button panel interface

### VR Template Blueprints
- `/Game/VRTemplate/Blueprints/VRPawn` - VR pawn (recommended for testing)
- `/Game/VRTemplate/Blueprints/VRGameMode` - VR game mode
- `/Game/VRTemplate/Blueprints/Pistol` - VR pistol
- `/Game/VRTemplate/Blueprints/Projectile` - VR projectile
- `/Game/VRTemplate/Blueprints/Menu` - VR menu system
- `/Game/VRTemplate/Blueprints/WidgetMenu` - VR widget menu
- `/Game/VRTemplate/Blueprints/VRTeleportVisualizer` - VR teleport visualizer

### VR Interactables
- `/Game/VRTemplate/Blueprints/Grabbable_SmallCube` - Grabbable cube
- `/Game/VRTemplate/Blueprints/GrabComponent` - Grab component
- `/Game/VRTemplate/Blueprints/GrabType` - Grab type enum

### Level Prototyping
- `/Game/LevelPrototyping/Interactable/Door/BP_DoorFrame` - Door frame
- `/Game/LevelPrototyping/Interactable/JumpPad/BP_JumpPad` - Jump pad
- `/Game/LevelPrototyping/Interactable/Target/BP_WobbleTarget` - Wobble target

### UI Blueprints (Widget Blueprints)

#### HUD Widgets
- `/Game/UI/Generated/hud/WBP_HUD_SpeedIndicator` - Speed indicator
- `/Game/UI/Generated/hud/WBP_HUD_AltitudeDisplay` - Altitude display
- `/Game/UI/Generated/hud/WBP_HUD_FuelGauge` - Fuel gauge
- `/Game/UI/Generated/hud/WBP_HUD_HealthBar` - Health bar
- `/Game/UI/Generated/hud/WBP_HUD_ShieldBar` - Shield bar
- `/Game/UI/Generated/hud/WBP_HUD_WeaponStatus` - Weapon status
- `/Game/UI/Generated/hud/WBP_HUD_TargetInfo` - Target info
- `/Game/UI/Generated/hud/WBP_HUD_Minimap` - Minimap
- `/Game/UI/Generated/hud/WBP_HUD_ObjectiveTracker` - Objective tracker

#### Menu Widgets
- `/Game/UI/Generated/menu/WBP_UI_MainMenu` - Main menu
- `/Game/UI/Generated/menu/WBP_UI_MissionBoard` - Mission board
- `/Game/UI/Generated/menu/WBP_UI_MissionCard` - Mission card
- `/Game/UI/Generated/menu/WBP_UI_SettingsMenu` - Settings menu
- `/Game/UI/Generated/menu/WBP_UI_ShipCustomization` - Ship customization
- `/Game/UI/Generated/menu/WBP_UI_TradingInterface` - Trading interface

#### Tutorial/Feedback Widgets
- `/Game/UI/Generated/tutorial/WBP_UI_TutorialOverlay` - Tutorial overlay
- `/Game/UI/Generated/ui_feedback/WBP_UI_Notification` - Notification widget

## Materials

### Space Materials
- `/Game/Materials/M_Sun_Animated` - Animated sun material
- `/Game/Materials/M_Sun_Emissive` - Emissive sun material
- `/Game/Materials/M_Sun_Simple` - Simple sun material
- `/Game/Materials/M_Planet_Base` - Base planet material
- `/Game/Materials/M_Asteroid_Rocky` - Rocky asteroid material

### Prototype Materials
- `/Game/LevelPrototyping/Materials/M_PrototypeGrid` - Prototype grid material
- `/Game/LevelPrototyping/Materials/M_FlatCol` - Flat color material
- `/Game/LevelPrototyping/Materials/MF_ProcGrid` - Procedural grid material function

### VR Template Materials
- `/Game/VRTemplate/Materials/M_VRCursor` - VR cursor material
- `/Game/VRTemplate/Materials/M_TeleportCylinder` - Teleport cylinder material
- `/Game/VRTemplate/Materials/M_TeleportNoise` - Teleport noise material
- `/Game/VRTemplate/Materials/M_Projectile` - Projectile material
- `/Game/VRTemplate/Materials/M_GridRotation` - Grid rotation material

### Character Materials
- `/Game/Characters/MannequinsXR/Materials/M_Mannequin` - Mannequin material

## Textures

### Character Textures (Manny)
- `/Game/Characters/MannequinsXR/Textures/Manny/T_Manny_01_D` - Diffuse texture
- `/Game/Characters/MannequinsXR/Textures/Manny/T_Manny_01_N` - Normal texture
- `/Game/Characters/MannequinsXR/Textures/Manny/T_Manny_01_BN` - Bent normal
- `/Game/Characters/MannequinsXR/Textures/Manny/T_Manny_01_MSR_MSK` - Metal/Specular/Roughness mask

### VR Template Textures
- `/Game/VRTemplate/Textures/T_Grid` - Grid texture

### Level Prototyping Textures
- `/Game/LevelPrototyping/Textures/T_GridChecker_A` - Grid checker texture

### Shared Textures
- `/Game/Characters/MannequinsXR/Textures/Shared/T_UE_Logo_M` - UE logo texture

## Static Meshes

### Prototype Meshes
- `/Game/LevelPrototyping/Meshes/SM_Cube` - Basic cube
- `/Game/LevelPrototyping/Meshes/SM_Cylinder` - Basic cylinder
- `/Game/LevelPrototyping/Meshes/SM_Ramp` - Basic ramp
- `/Game/LevelPrototyping/Meshes/SM_ChamferCube` - Chamfered cube
- `/Game/LevelPrototyping/Meshes/SM_QuarterCylinder` - Quarter cylinder

### Door Meshes
- `/Game/LevelPrototyping/Interactable/Door/Assets/Meshes/SM_Door` - Door mesh
- `/Game/LevelPrototyping/Interactable/Door/Assets/Meshes/SM_DoorFrame_Corner` - Door frame corner
- `/Game/LevelPrototyping/Interactable/Door/Assets/Meshes/SM_DoorFrame_Edge` - Door frame edge

### JumpPad Meshes
- `/Game/LevelPrototyping/Interactable/JumpPad/Assets/Meshes/SM_CircularBand` - Circular band
- `/Game/LevelPrototyping/Interactable/JumpPad/Assets/Meshes/SM_CircularGlow` - Circular glow

### Target Meshes
- `/Game/LevelPrototyping/Interactable/Target/Assets/SM_TargetBaseMesh` - Target base mesh

## Skeletal Meshes

### Character Meshes
- `/Game/Characters/MannequinsXR/Meshes/SK_MannequinsXR` - Base mannequin skeleton
- `/Game/Characters/MannequinsXR/Meshes/SKM_MannyXR_left` - Manny left hand
- `/Game/Characters/MannequinsXR/Meshes/SKM_MannyXR_right` - Manny right hand
- `/Game/Characters/MannequinsXR/Meshes/SKM_QuinnXR_left` - Quinn left hand
- `/Game/Characters/MannequinsXR/Meshes/SKM_QuinnXR_right` - Quinn right hand

## Animations

### Hand Animations
- `/Game/Characters/MannequinsXR/Animations/A_MannequinsXR_Idle_Right` - Idle right hand
- `/Game/Characters/MannequinsXR/Animations/A_MannequinsXR_Grasp_Right` - Grasp right hand
- `/Game/Characters/MannequinsXR/Animations/A_MannequinsXR_Point_Right` - Point right hand
- `/Game/Characters/MannequinsXR/Animations/A_MannequinsXR_ThumbUp_Right` - Thumbs up right hand
- `/Game/Characters/MannequinsXR/Animations/A_MannequinsXR_IndexCurl_Right` - Index curl right hand

## Data Assets

### Mission Templates
- `/Game/Data/Missions/DataAssets/DA_Combat_Missions` - Combat missions
- `/Game/Data/Missions/DataAssets/DA_Delivery_Missions` - Delivery missions
- `/Game/Data/Missions/DataAssets/DA_Mining_Missions` - Mining missions
- `/Game/Data/Missions/DataAssets/DA_Exploration_Missions` - Exploration missions
- `/Game/Data/Missions/DataAssets/DA_Escort_Missions` - Escort missions
- `/Game/Data/Missions/DataAssets/DA_Rescue_Missions` - Rescue missions
- `/Game/Data/Missions/DataAssets/DA_Salvage_Missions` - Salvage missions
- `/Game/Data/Missions/DataAssets/DA_Bounty_Missions` - Bounty missions
- `/Game/Data/Missions/DataAssets/DA_Assassination_Missions` - Assassination missions
- `/Game/Data/Missions/DataAssets/DA_Reconnaissance_Missions` - Reconnaissance missions
- `/Game/Data/Missions/DataAssets/DA_Smuggling_Missions` - Smuggling missions

## VFX (Niagara Systems)

### VR Template VFX
- `/Game/VRTemplate/VFX/NS_MenuLaser` - Menu laser effect
- `/Game/VRTemplate/VFX/NS_TeleportRing` - Teleport ring effect
- `/Game/VRTemplate/VFX/NS_TeleportTrace` - Teleport trace effect
- `/Game/VRTemplate/VFX/NS_PlayAreaBounds` - Play area bounds effect

### Space VFX
- `/Game/Effects/NS_SunStorms` - Sun storm effect

## Audio

### VR Template Audio
- `/Game/VRTemplate/Audio/Fire01` - Fire sound
- `/Game/VRTemplate/Audio/Fire_Cue` - Fire sound cue

## Haptics

### VR Haptic Effects
- `/Game/VRTemplate/Haptics/GrabHapticEffect` - Grab haptic feedback
- `/Game/VRTemplate/Haptics/PistolFireHapticEffect` - Pistol fire haptic feedback

## Input Actions

### VR Input Actions
- `/Game/VRTemplate/Input/Actions/IA_Grab_Left` - Grab left action
- `/Game/VRTemplate/Input/Actions/IA_Grab_Right` - Grab right action
- `/Game/VRTemplate/Input/Actions/IA_Shoot_Left` - Shoot left action
- `/Game/VRTemplate/Input/Actions/IA_Shoot_Right` - Shoot right action
- `/Game/VRTemplate/Input/Actions/IA_Move` - Move action
- `/Game/VRTemplate/Input/Actions/IA_Turn` - Turn action
- `/Game/VRTemplate/Input/Actions/IA_Menu_Toggle_Left` - Toggle menu left
- `/Game/VRTemplate/Input/Actions/IA_Menu_Toggle_Right` - Toggle menu right

## Recommended Assets for Testing

### For Blueprint Testing
Use `/Game/VRTemplate/Blueprints/VRPawn` - This is a stable, complete blueprint with dependencies

### For Material Testing
Use `/Game/Materials/M_Sun_Simple` - Simple material without complex dependencies

### For Texture Testing
Use `/Game/VRTemplate/Textures/T_Grid` - Simple texture asset

### For Map Testing
Use `/Game/FlightTest` or `/Game/VRTemplate/Maps/VRTemplateMap` - Both are complete, loadable maps

### For Static Mesh Testing
Use `/Game/LevelPrototyping/Meshes/SM_Cube` - Basic geometry, minimal dependencies

### For Data Asset Testing
Use `/Game/Data/Missions/DataAssets/DA_Combat_Missions` - Complete mission template

## Usage Notes

1. All paths use the Unreal Engine content path format: `/Game/...`
2. The `/Game/` prefix maps to the `Content/` folder on disk
3. Asset paths do NOT include the file extension (.uasset or .umap)
4. Blueprint class references may need the `_C` suffix when spawning actors
5. Some assets may have dependencies - use `/get_asset_dependencies` to check

## Testing Best Practices

1. Start with simple assets (textures, static meshes) before testing complex ones (blueprints)
2. Use VR template assets for reliable testing - they are well-tested by Epic
3. Avoid testing with "disabled" assets (files ending in .disabled)
4. Test map loading separately from other operations
5. Some assets may only be loadable when specific systems are initialized
