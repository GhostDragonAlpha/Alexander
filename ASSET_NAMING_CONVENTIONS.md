# Asset Naming Conventions Guide

**Project**: Alexander VR Space Simulation  
**Version**: 1.0  
**Last Updated**: November 7, 2025  
**Purpose**: Standardize asset naming across the project for consistency and maintainability

---

## Table of Contents

1. [General Principles](#general-principles)
2. [File Naming Structure](#file-naming-structure)
3. [Asset Type Prefixes](#asset-type-prefixes)
4. [Category Guidelines](#category-guidelines)
5. [Examples by Asset Type](#examples-by-asset-type)
6. [Version Control](#version-control)
7. [Common Mistakes](#common-mistakes)

---

## General Principles

### Core Rules

1. **Use PascalCase**: Every word capitalized, no spaces or underscores between words
   - ✅ `SpaceStationHub`
   - ❌ `space_station_hub` or `spacestationhub`

2. **Prefix with Type**: Start with asset type prefix for easy sorting
   - ✅ `SM_Station_Main`
   - ❌ `MainStation`

3. **Be Descriptive**: Name should clearly indicate what the asset is
   - ✅ `T_Metal_Scratched_01_D` (Texture, Metal, Scratched variant 01, Diffuse)
   - ❌ `T_Tex1`

4. **Use Suffixes for Variants**: Number variants sequentially
   - ✅ `SM_Asteroid_Small_01`, `SM_Asteroid_Small_02`
   - ❌ `SM_Asteroid_Small_A`, `SM_Asteroid_Small_New`

5. **Avoid Special Characters**: Use only letters, numbers, and underscores
   - ✅ `SM_Weapon_LaserCannon_Mk2`
   - ❌ `SM_Weapon_Laser-Cannon (Mk.2)`

6. **Keep Reasonable Length**: Aim for 3-5 segments
   - ✅ `SM_Station_Interior_Wall_01`
   - ❌ `SM_SpaceStationInteriorWallSegmentForDockingBay01`

---

## File Naming Structure

### Standard Format

```
[Prefix]_[Category]_[Description]_[Variant]_[Suffix]
```

### Breakdown

- **Prefix**: Asset type (2-3 characters) - REQUIRED
- **Category**: General category (1 word) - REQUIRED
- **Description**: Specific description (1-3 words) - REQUIRED
- **Variant**: Numerical variant (01, 02, etc.) - OPTIONAL
- **Suffix**: Material channel or LOD - OPTIONAL

### Examples

```
SM_Station_MainHub_01        (Static Mesh, Station category, MainHub, variant 01)
T_Metal_Rusty_01_D           (Texture, Metal, Rusty, variant 01, Diffuse)
M_Ship_Fighter_Hull          (Material, Ship, Fighter, Hull)
BP_Enemy_Pirate_Fighter      (Blueprint, Enemy, Pirate, Fighter)
PS_Engine_Thruster_Blue      (Particle System, Engine, Thruster, Blue)
```

---

## Asset Type Prefixes

### 3D Assets

| Prefix | Asset Type | Example |
|--------|-----------|---------|
| `SM_` | Static Mesh | `SM_Asteroid_Large_03` |
| `SK_` | Skeletal Mesh | `SK_Character_Pilot_Male` |
| `SKM_` | Skeletal Mesh | `SKM_Ship_Fighter` |

### Materials & Textures

| Prefix | Asset Type | Example |
|--------|-----------|---------|
| `M_` | Material | `M_Metal_Brushed` |
| `MI_` | Material Instance | `MI_Metal_Brushed_Blue` |
| `MF_` | Material Function | `MF_Noise_Procedural` |
| `T_` | Texture | `T_Rock_Crater_01_D` |

### Texture Suffixes

| Suffix | Channel | Description |
|--------|---------|-------------|
| `_D` | Diffuse/Albedo | Base color |
| `_N` | Normal | Normal map |
| `_R` | Roughness | Roughness map |
| `_M` | Metallic | Metallic map |
| `_AO` | Ambient Occlusion | AO map |
| `_E` | Emissive | Emissive/glow |
| `_H` | Height | Height/displacement |
| `_OM` | Occlusion/Roughness/Metallic | Packed ORM |

### Blueprints

| Prefix | Asset Type | Example |
|--------|-----------|---------|
| `BP_` | Blueprint | `BP_Spaceship_Fighter` |
| `AC_` | Actor Component | `AC_Shield_Generator` |
| `WBP_` | Widget Blueprint | `WBP_HUD_Main` |
| `ABP_` | Animation Blueprint | `ABP_Character_Pilot` |

### Effects & Audio

| Prefix | Asset Type | Example |
|--------|-----------|---------|
| `PS_` | Particle System (Cascade) | `PS_Explosion_Large` |
| `NS_` | Niagara System | `NS_Engine_Trail` |
| `NE_` | Niagara Emitter | `NE_Sparks_Metal` |
| `SFX_` | Sound Effect | `SFX_Weapon_Laser_Fire` |
| `MUS_` | Music Track | `MUS_Combat_Intense_01` |
| `SC_` | Sound Cue | `SC_Footsteps_Metal` |

### Animations

| Prefix | Asset Type | Example |
|--------|-----------|---------|
| `AM_` | Animation Montage | `AM_Character_Jump` |
| `AS_` | Animation Sequence | `AS_Character_Walk` |
| `BS_` | Blend Space | `BS_Character_Movement` |

### Data Assets

| Prefix | Asset Type | Example |
|--------|-----------|---------|
| `DA_` | Data Asset | `DA_Mission_Combat_001` |
| `DT_` | Data Table | `DT_Items_Weapons` |
| `CU_` | Curve | `CU_Damage_Falloff` |

### Physics & Collision

| Prefix | Asset Type | Example |
|--------|-----------|---------|
| `PHY_` | Physics Asset | `PHY_Character_Pilot` |
| `PM_` | Physical Material | `PM_Metal_Hard` |

---

## Category Guidelines

### Space Stations

```
SM_Station_[Type]_[Component]_[Variant]

Types: Main, Trading, Military, Research, Mining
Components: Exterior, Interior, Docking, Corridor, etc.

Examples:
SM_Station_Main_Exterior_01
SM_Station_Trading_DockingBay_02
SM_Station_Military_Turret_01
```

### Spaceships

```
SM_Ship_[Class]_[Component]_[Variant]

Classes: Fighter, Freighter, Capital, Racer, Mining
Components: Hull, Wing, Engine, Cockpit, Weapon

Examples:
SM_Ship_Fighter_Hull_Light
SM_Ship_Freighter_Cargo_Large
SM_Ship_Capital_Bridge_01
```

### Planets & Terrain

```
SM_Planet_[Type]_[Feature]_[Variant]

Types: Earth, Ice, Desert, Gas, Volcanic
Features: Surface, Rock, Crater, Mountain

Examples:
SM_Planet_Desert_Rock_01
SM_Planet_Ice_Crater_Large
T_Planet_Earth_Surface_D
```

### Weapons & Equipment

```
SM_Weapon_[Type]_[Variant]

Types: Laser, Plasma, Kinetic, Missile, Railgun

Examples:
SM_Weapon_Laser_Cannon_Mk1
SM_Weapon_Missile_Launcher_Heavy
VFX_Weapon_Plasma_Impact
```

### UI Elements

```
WBP_[Screen]_[Component]_[Variant]

Screens: HUD, Menu, Mission, Trading, Map
Components: Button, Panel, Icon, Bar

Examples:
WBP_HUD_ShieldBar
WBP_Menu_Main_Background
T_UI_Icon_Weapon_01
```

### VFX (Visual Effects)

```
NS_[Category]_[Effect]_[Variant]

Categories: Weapon, Engine, Environment, Impact, Atmospheric

Examples:
NS_Weapon_Laser_Beam_Red
NS_Engine_Thruster_Trail
NS_Impact_Explosion_Large
PS_Environment_Dust_Storm
```

### Audio

```
SFX_[Category]_[Action]_[Variant]

Categories: Weapon, Ship, UI, Environment, Ambient

Examples:
SFX_Weapon_Laser_Fire_01
SFX_Ship_Engine_Startup
SFX_UI_Button_Click
SFX_Environment_Wind_Light
MUS_Exploration_Ambient_01
```

---

## Examples by Asset Type

### Static Meshes - Space Stations

```
SM_Station_Main_Exterior_Core
SM_Station_Main_Exterior_Docking_01
SM_Station_Main_Exterior_Docking_02
SM_Station_Main_Interior_Corridor_Straight
SM_Station_Main_Interior_Corridor_Corner
SM_Station_Main_Interior_Room_Large
SM_Station_Trading_Exterior_Market
SM_Station_Military_Exterior_Armored
```

### Materials - Metals

```
M_Metal_Base
MI_Metal_Steel_Clean
MI_Metal_Steel_Scratched
MI_Metal_Steel_Rusty
T_Metal_Steel_01_D
T_Metal_Steel_01_N
T_Metal_Steel_01_R
T_Metal_Steel_01_M
```

### Blueprints - Gameplay

```
BP_Spaceship_Fighter_Player
BP_Spaceship_Fighter_Enemy
BP_Spaceship_Freighter_Cargo
BP_Weapon_LaserCannon_Basic
BP_Weapon_MissileLauncher_Advanced
BP_Pickup_Health_Small
BP_Pickup_Ammo_Energy
```

### Widgets - HUD

```
WBP_HUD_Main
WBP_HUD_ShieldBar
WBP_HUD_HealthBar
WBP_HUD_Crosshair
WBP_HUD_Minimap
WBP_HUD_WeaponStatus
```

### VFX - Combat

```
NS_Weapon_Laser_Beam_Red
NS_Weapon_Laser_Beam_Blue
NS_Weapon_Laser_Impact
NS_Weapon_Plasma_Projectile
NS_Weapon_Plasma_Impact
NS_Explosion_Small
NS_Explosion_Medium
NS_Explosion_Large
```

### Audio - Ships

```
SFX_Ship_Engine_Idle_Loop
SFX_Ship_Engine_Thrust_Loop
SFX_Ship_Engine_Startup
SFX_Ship_Engine_Shutdown
SFX_Ship_Damage_Hit_Metal
SFX_Ship_Damage_Explosion
SFX_Ship_Landing_Gear_Deploy
SFX_Ship_Door_Open
```

---

## Version Control

### LOD Naming

```
SM_Ship_Fighter_LOD0   (Full detail)
SM_Ship_Fighter_LOD1   (High detail)
SM_Ship_Fighter_LOD2   (Medium detail)
SM_Ship_Fighter_LOD3   (Low detail)
SM_Ship_Fighter_LOD4   (Very low detail)
```

### Iteration Versions

For work-in-progress versions, use `_WIP` or `_Test`:

```
SM_Station_MainHub_WIP
M_Metal_Rusty_Test
```

For archived versions:

```
SM_Station_MainHub_Old
SM_Station_MainHub_v01
```

---

## Common Mistakes

### ❌ Avoid These

```
Mesh1                          (Not descriptive)
my_cool_ship                   (Lowercase, subjective)
SHIP-FIGHTER-01                (Dashes, all caps)
SpaceStation (Final Version)   (Spaces, parentheses)
sm_station_mainHub             (Mixed case)
SuperAwesomeSpaceshipThatFliesReallyFast  (Too long)
```

### ✅ Use These Instead

```
SM_Ship_Fighter_01
SM_Ship_Fighter_02
SM_Station_Main_Exterior
SM_Station_Main_Interior_Corridor
SM_Spaceship_Fast_Racer
```

---

## Organization in Folders

### Folder Structure Example

```
Content/
├── Assets/
│   ├── Stations/
│   │   ├── Meshes/
│   │   │   ├── SM_Station_Main_Exterior_01
│   │   │   └── SM_Station_Main_Interior_Corridor
│   │   ├── Materials/
│   │   │   ├── M_Station_Metal
│   │   │   └── MI_Station_Metal_Rusty
│   │   └── Textures/
│   │       ├── T_Station_Metal_01_D
│   │       └── T_Station_Metal_01_N
│   ├── Ships/
│   │   ├── Fighter/
│   │   │   ├── Meshes/
│   │   │   ├── Materials/
│   │   │   └── Textures/
│   │   └── Freighter/
│   └── Weapons/
├── Blueprints/
│   ├── Ships/
│   │   ├── BP_Ship_Fighter_Player
│   │   └── BP_Ship_Fighter_Enemy
│   ├── Weapons/
│   └── Systems/
├── VFX/
│   ├── Weapons/
│   │   ├── NS_Weapon_Laser_Beam
│   │   └── NS_Weapon_Explosion
│   └── Environment/
└── Audio/
    ├── SFX/
    │   ├── Ships/
    │   └── Weapons/
    └── Music/
```

---

## Quick Reference Chart

| Asset Type | Prefix | Example | Notes |
|------------|--------|---------|-------|
| Static Mesh | `SM_` | `SM_Station_Main_01` | Most common 3D asset |
| Material | `M_` | `M_Metal_Brushed` | Base material |
| Material Instance | `MI_` | `MI_Metal_Blue` | Material variant |
| Texture | `T_` | `T_Metal_01_D` | Always include channel suffix |
| Blueprint | `BP_` | `BP_Ship_Fighter` | Gameplay objects |
| Widget | `WBP_` | `WBP_HUD_Main` | UI elements |
| Niagara System | `NS_` | `NS_Engine_Trail` | VFX (preferred) |
| Particle System | `PS_` | `PS_Explosion` | VFX (legacy) |
| Sound Effect | `SFX_` | `SFX_Laser_Fire` | Audio |
| Music | `MUS_` | `MUS_Combat_01` | Music tracks |
| Data Asset | `DA_` | `DA_Mission_001` | Data files |

---

## Enforcement

### Review Checklist

Before committing new assets, verify:

- [ ] Asset has correct prefix
- [ ] Name uses PascalCase
- [ ] Name is descriptive and clear
- [ ] Variant numbers are sequential (01, 02, etc.)
- [ ] Texture has proper channel suffix (_D, _N, etc.)
- [ ] No special characters or spaces
- [ ] Asset is in correct folder
- [ ] Name length is reasonable (< 50 characters)

### Tools

- Use Unreal's "Reference Viewer" to check dependencies before renaming
- Use "Asset Audit" plugin to find naming violations
- Set up editor preferences to warn about naming issues

---

**Questions?** Refer to this document or contact the technical art lead.

**Updates**: This document may be updated as naming needs evolve. Check the version number and date at the top.
