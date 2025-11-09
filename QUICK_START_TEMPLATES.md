# Quick Start Templates - Phase 8 Baseline

**Project**: Alexander VR Space Simulation
**Purpose**: Ready-to-use templates for common Phase 8 development tasks
**Last Updated**: November 8, 2025
**Status**: Production-ready templates and examples

---

## Table of Contents

1. [Mission Template Creator](#mission-template-creator)
2. [Station Blueprint Template](#station-blueprint-template)
3. [VFX Configuration Template](#vfx-configuration-template)
4. [UI Widget Templates](#ui-widget-templates)
5. [Test Level Template](#test-level-template)

---

## Mission Template Creator

### Basic Mission Template (Fill-in-the-Blank)

```json
{
  "mission_id": "TYPE_NAME_###",
  "mission_type": "MISSION_TYPE",
  "title_template": "MISSION_TITLE_WITH_{VARIABLES}",
  "description_template": "MISSION_DESCRIPTION_WITH_{VARIABLES}",
  "difficulty": "DIFFICULTY_LEVEL",
  "objectives": [
    {
      "type": "OBJECTIVE_TYPE",
      "description": "OBJECTIVE_DESCRIPTION",
      "quantity": NUMBER,
      "optional": false,
      "hidden": false
    }
  ],
  "rewards": {
    "credits": AMOUNT,
    "experience": AMOUNT,
    "items": [],
    "faction_rep": {"FACTION_NAME": AMOUNT}
  },
  "requirements": {
    "min_level": LEVEL,
    "required_faction": null,
    "prerequisite_missions": []
  },
  "location": {
    "system": "SolarSystem",
    "max_distance": DISTANCE,
    "location_type": "LOCATION_TYPE"
  },
  "time_limit": null,
  "repeatable": true,
  "variables": {
    "VARIABLE_NAME": ["OPTION_1", "OPTION_2", "OPTION_3"]
  }
}
```

### Quick Reference

**Mission Types**: Combat, Delivery, Exploration, Escort, Mining, Assassination, Reconnaissance, Rescue, Smuggling, Bounty, Salvage, Story

**Difficulty Levels**: Trivial, Easy, Normal, Hard, Extreme, Legendary

**Objective Types**:
- `KillTarget` - Eliminate specific enemy
- `KillEnemies` - Destroy multiple enemies
- `DeliverCargo` - Transport cargo to destination
- `DefendLocation` - Protect area from waves
- `EscortTarget` - Protect friendly ship
- `ScanObject` - Scan specific object
- `CollectItems` - Gather resources/items
- `ReachLocation` - Navigate to waypoint
- `SurviveTime` - Survive for duration
- `InvestigateArea` - Explore location

**Location Types**: Space, Planet, Station, Asteroid, Debris

---

### Example 1: Combat Mission Template

```json
{
  "mission_id": "combat_pirate_assault_001",
  "mission_type": "Combat",
  "title_template": "Assault on {pirate_base}",
  "description_template": "Intelligence has located a {pirate_faction} stronghold at {pirate_base}. Eliminate {enemy_count} hostiles and destroy their {target_structure}. Expect heavy resistance.",
  "difficulty": "Hard",
  "objectives": [
    {
      "type": "KillEnemies",
      "description": "Eliminate hostile fighters",
      "quantity": 8,
      "optional": false,
      "hidden": false
    },
    {
      "type": "DestroyStructure",
      "description": "Destroy {target_structure}",
      "quantity": 1,
      "optional": false,
      "hidden": false
    },
    {
      "type": "SurviveTime",
      "description": "Survive until reinforcements arrive",
      "quantity": 300,
      "optional": true,
      "hidden": false
    }
  ],
  "rewards": {
    "credits": 5000,
    "experience": 250,
    "items": ["weapon_upgrade_rare"],
    "faction_rep": {"Militia": 25, "Pirates": -50}
  },
  "requirements": {
    "min_level": 5,
    "required_faction": null,
    "prerequisite_missions": ["combat_pirate_scout_001"]
  },
  "location": {
    "system": "SolarSystem",
    "max_distance": 25000,
    "location_type": "Asteroid"
  },
  "time_limit": 1200,
  "repeatable": false,
  "variables": {
    "pirate_base": ["Skull Rock", "Black Fortress", "Viper's Nest", "Crimson Outpost"],
    "pirate_faction": ["Red Skulls", "Void Reapers", "Dark Star Pirates"],
    "enemy_count": [8, 10, 12],
    "target_structure": ["command center", "weapons platform", "shield generator"]
  }
}
```

**Copy-Paste Checklist**:
- [ ] Replace `mission_id` with unique identifier
- [ ] Update `title_template` and `description_template`
- [ ] Set appropriate `difficulty` level
- [ ] Configure `objectives` array
- [ ] Set reward amounts (credits, experience, items)
- [ ] Set `min_level` requirement
- [ ] Configure `location` parameters
- [ ] Add `variables` for randomization
- [ ] Test mission spawns correctly
- [ ] Verify all objectives complete

---

### Example 2: Delivery Mission Template

```json
{
  "mission_id": "delivery_urgent_medical_001",
  "mission_type": "Delivery",
  "title_template": "Emergency Medical Supply Run to {destination}",
  "description_template": "A {disaster_type} has occurred at {destination}. Deliver {cargo_amount} units of medical supplies within {time_limit} minutes. Speed is critical - lives depend on you.",
  "difficulty": "Normal",
  "objectives": [
    {
      "type": "DeliverCargo",
      "description": "Deliver medical supplies to {destination}",
      "quantity": 50,
      "optional": false,
      "hidden": false
    },
    {
      "type": "AvoidDamage",
      "description": "Cargo must arrive undamaged",
      "quantity": 95,
      "optional": true,
      "hidden": false
    }
  ],
  "rewards": {
    "credits": 3000,
    "experience": 180,
    "items": [],
    "faction_rep": {"Medical_Corps": 30, "Civilians": 20}
  },
  "requirements": {
    "min_level": 2,
    "required_faction": null,
    "prerequisite_missions": [],
    "required_cargo_capacity": 50
  },
  "location": {
    "system": "SolarSystem",
    "max_distance": 20000,
    "location_type": "Station"
  },
  "time_limit": 600,
  "repeatable": true,
  "variables": {
    "destination": ["Outpost Alpha", "Mining Colony 7", "Research Station Kepler", "Agricultural Dome 3"],
    "disaster_type": ["disease outbreak", "industrial accident", "radiation leak", "explosion"],
    "cargo_amount": [50, 75, 100],
    "time_limit": [10, 15, 20]
  }
}
```

---

### Example 3: Exploration Mission Template

```json
{
  "mission_id": "exploration_ancient_artifact_001",
  "mission_type": "Exploration",
  "title_template": "Archaeological Survey: {artifact_name}",
  "description_template": "Ancient {civilization} ruins have been detected at {location}. Scan the site and recover {artifact_name}. Be cautious - the area may contain {hazard_type}.",
  "difficulty": "Normal",
  "objectives": [
    {
      "type": "ReachLocation",
      "description": "Navigate to survey coordinates",
      "quantity": 1,
      "optional": false,
      "hidden": false
    },
    {
      "type": "ScanObject",
      "description": "Scan ancient structure",
      "quantity": 3,
      "optional": false,
      "hidden": false
    },
    {
      "type": "CollectItems",
      "description": "Recover {artifact_name}",
      "quantity": 1,
      "optional": false,
      "hidden": false
    },
    {
      "type": "AvoidDetection",
      "description": "Complete mission without triggering defenses",
      "quantity": 1,
      "optional": true,
      "hidden": true
    }
  ],
  "rewards": {
    "credits": 4000,
    "experience": 300,
    "items": ["ancient_artifact"],
    "faction_rep": {"Scientists": 40}
  },
  "requirements": {
    "min_level": 4,
    "required_faction": null,
    "prerequisite_missions": [],
    "required_equipment": ["advanced_scanner"]
  },
  "location": {
    "system": "SolarSystem",
    "max_distance": 30000,
    "location_type": "Debris"
  },
  "time_limit": null,
  "repeatable": false,
  "variables": {
    "artifact_name": ["Precursor Data Core", "Ancient Power Cell", "Alien Star Map", "Quantum Resonator"],
    "civilization": ["Precursor", "Ancient Alien", "Lost Human Colony"],
    "location": ["Debris Field Gamma", "Abandoned Station", "Asteroid Cavern", "Derelict Vessel"],
    "hazard_type": ["automated defenses", "radiation", "unstable gravity", "hostile wildlife"]
  }
}
```

---

### Example 4: Escort Mission Template

```json
{
  "mission_id": "escort_convoy_001",
  "mission_type": "Escort",
  "title_template": "Escort {convoy_type} Through {danger_zone}",
  "description_template": "Provide armed escort for {convoy_type} traveling through the dangerous {danger_zone}. Protect convoy from {threat_type}. All ships must survive.",
  "difficulty": "Hard",
  "objectives": [
    {
      "type": "EscortTarget",
      "description": "Protect convoy ships",
      "quantity": 3,
      "optional": false,
      "hidden": false
    },
    {
      "type": "KillEnemies",
      "description": "Eliminate attacking forces",
      "quantity": 6,
      "optional": false,
      "hidden": false
    },
    {
      "type": "ReachLocation",
      "description": "Safely reach destination",
      "quantity": 1,
      "optional": false,
      "hidden": false
    }
  ],
  "rewards": {
    "credits": 4500,
    "experience": 220,
    "items": [],
    "faction_rep": {"Traders_Guild": 35}
  },
  "requirements": {
    "min_level": 4,
    "required_faction": null,
    "prerequisite_missions": []
  },
  "location": {
    "system": "SolarSystem",
    "max_distance": 35000,
    "location_type": "Space"
  },
  "time_limit": 900,
  "repeatable": true,
  "variables": {
    "convoy_type": ["trading convoy", "refugee fleet", "mining haulers", "diplomatic envoy"],
    "danger_zone": ["Pirate Corridor", "Contested Zone", "Asteroid Belt", "Nebula Passage"],
    "threat_type": ["pirate raiders", "hostile mercenaries", "rogue AI drones", "territorial aliens"]
  }
}
```

---

### Mission Validation Checklist

Use this checklist before submitting any new mission:

**Configuration**:
- [ ] `mission_id` is unique and follows naming convention (type_name_###)
- [ ] `mission_type` matches one of the valid types
- [ ] `difficulty` is appropriate for min_level and rewards
- [ ] All template variables use {curly_braces}

**Objectives**:
- [ ] At least 1 non-optional objective exists
- [ ] Objective types are valid and implemented
- [ ] Quantities are reasonable and balanced
- [ ] Hidden objectives are truly secret (no description spoilers)

**Rewards**:
- [ ] Credits scale with difficulty (Trivial: 500-1000, Easy: 1000-2000, Normal: 2000-4000, Hard: 4000-7000, Extreme: 7000-12000, Legendary: 12000+)
- [ ] Experience scales with difficulty and time investment
- [ ] Faction reputation changes make narrative sense
- [ ] Item rewards are appropriate for mission type

**Requirements**:
- [ ] `min_level` is balanced for difficulty
- [ ] Prerequisites don't create circular dependencies
- [ ] Special equipment is obtainable at min_level
- [ ] Cargo capacity requirements are reasonable

**Variables**:
- [ ] All template variables have matching entries in variables object
- [ ] Variable options provide meaningful variety
- [ ] At least 3-5 options per variable
- [ ] Options maintain mission tone and difficulty

**Testing**:
- [ ] Mission spawns without errors
- [ ] All objectives can be completed
- [ ] Rewards are granted correctly
- [ ] Variables substitute properly in title/description
- [ ] Time limits are achievable
- [ ] Repeatable missions function correctly

---

## Station Blueprint Template

### Blueprint Class Setup

**File Location**: `Content/Blueprints/Stations/BP_Station_[StationType]_[Name].uasset`

**Parent Class**: `AActor` or `ASpaceStation` (if base class exists)

### Required Components Checklist

```cpp
// Core Components
✓ Root Component (Scene Component)
✓ Static Mesh Component (Station Exterior)
✓ Collision Component (Box/Sphere for docking detection)
✓ Audio Component (Ambient station sounds)
✓ Point Light Component (Station lighting)

// Gameplay Components
✓ Mission Board Component (for mission distribution)
✓ Trading Component (for economy interaction)
✓ Docking System Component (for ship landing)
✓ Faction Component (for reputation system)

// VR Components
✓ VR Teleport Markers (interior navigation)
✓ VR Interaction Volumes (button/panel interaction)
✓ Holographic UI Components (VR interfaces)
```

### Station Blueprint Template (Copy-Paste)

**C++ Header Template** (`SpaceStationBase.h`):

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpaceStationBase.generated.h"

UCLASS(Blueprintable)
class ALEXANDER_API ASpaceStationBase : public AActor
{
    GENERATED_BODY()

public:
    ASpaceStationBase();

    // ===== CORE COMPONENTS =====

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Station")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Station")
    UStaticMeshComponent* StationMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Station")
    class USphereComponent* DockingZone;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Station")
    class UAudioComponent* AmbientAudio;

    // ===== GAMEPLAY COMPONENTS =====

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay")
    class UMissionBoardComponent* MissionBoard;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay")
    class UTradingComponent* TradingSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay")
    class UDockingSystemComponent* DockingSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gameplay")
    class UFactionComponent* FactionAffiliation;

    // ===== STATION PROPERTIES =====

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Info")
    FString StationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Info")
    FString StationType; // "Hub", "Trading", "Military", "Research", "Mining"

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Info")
    int32 MaxDockingBays;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station Info")
    float DockingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
    float PriceModifier; // 0.8 = 20% discount, 1.2 = 20% markup

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Missions")
    int32 MaxActiveMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Missions")
    TArray<FString> AvailableMissionTypes;

    // ===== FUNCTIONS =====

    UFUNCTION(BlueprintCallable, Category = "Station")
    void InitializeStation();

    UFUNCTION(BlueprintCallable, Category = "Docking")
    bool RequestDocking(class ASpaceship* Ship);

    UFUNCTION(BlueprintCallable, Category = "Docking")
    void ReleaseDocking(int32 BayNumber);

    UFUNCTION(BlueprintCallable, Category = "Missions")
    TArray<FMissionData> GetAvailableMissions();

    UFUNCTION(BlueprintCallable, Category = "Trading")
    void OpenTradingInterface(class APlayerController* Player);

protected:
    virtual void BeginPlay() override;

private:
    TArray<bool> DockingBayOccupied;
};
```

**C++ Implementation Template** (`SpaceStationBase.cpp`):

```cpp
#include "SpaceStationBase.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "MissionBoardComponent.h"
#include "TradingComponent.h"
#include "DockingSystemComponent.h"
#include "FactionComponent.h"

ASpaceStationBase::ASpaceStationBase()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    // Create station mesh
    StationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StationMesh"));
    StationMesh->SetupAttachment(RootComponent);
    StationMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StationMesh->SetCollisionResponseToAllChannels(ECR_Block);

    // Create docking zone
    DockingZone = CreateDefaultSubobject<USphereComponent>(TEXT("DockingZone"));
    DockingZone->SetupAttachment(RootComponent);
    DockingZone->SetSphereRadius(5000.0f);
    DockingZone->SetCollisionResponseToAllChannels(ECR_Overlap);

    // Create ambient audio
    AmbientAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudio->SetupAttachment(RootComponent);
    AmbientAudio->bAutoActivate = true;

    // Create gameplay components
    MissionBoard = CreateDefaultSubobject<UMissionBoardComponent>(TEXT("MissionBoard"));
    TradingSystem = CreateDefaultSubobject<UTradingComponent>(TEXT("TradingSystem"));
    DockingSystem = CreateDefaultSubobject<UDockingSystemComponent>(TEXT("DockingSystem"));
    FactionAffiliation = CreateDefaultSubobject<UFactionComponent>(TEXT("FactionAffiliation"));

    // Default values
    StationName = TEXT("Space Station");
    StationType = TEXT("Hub");
    MaxDockingBays = 4;
    DockingRange = 5000.0f;
    PriceModifier = 1.0f;
    MaxActiveMissions = 10;
}

void ASpaceStationBase::BeginPlay()
{
    Super::BeginPlay();
    InitializeStation();
}

void ASpaceStationBase::InitializeStation()
{
    // Initialize docking bays
    DockingBayOccupied.SetNum(MaxDockingBays);
    for (int32 i = 0; i < MaxDockingBays; i++)
    {
        DockingBayOccupied[i] = false;
    }

    // Initialize mission board
    if (MissionBoard)
    {
        MissionBoard->SetMaxMissions(MaxActiveMissions);
        MissionBoard->SetMissionTypes(AvailableMissionTypes);
    }

    // Initialize trading system
    if (TradingSystem)
    {
        TradingSystem->SetPriceModifier(PriceModifier);
    }

    UE_LOG(LogTemp, Log, TEXT("Station initialized: %s (Type: %s)"), *StationName, *StationType);
}

bool ASpaceStationBase::RequestDocking(ASpaceship* Ship)
{
    if (!Ship || !DockingSystem)
    {
        return false;
    }

    // Find available bay
    for (int32 i = 0; i < MaxDockingBays; i++)
    {
        if (!DockingBayOccupied[i])
        {
            DockingBayOccupied[i] = true;
            DockingSystem->AssignDockingBay(Ship, i);
            return true;
        }
    }

    return false; // No bays available
}

void ASpaceStationBase::ReleaseDocking(int32 BayNumber)
{
    if (BayNumber >= 0 && BayNumber < MaxDockingBays)
    {
        DockingBayOccupied[BayNumber] = false;
    }
}

TArray<FMissionData> ASpaceStationBase::GetAvailableMissions()
{
    if (MissionBoard)
    {
        return MissionBoard->GetMissions();
    }
    return TArray<FMissionData>();
}

void ASpaceStationBase::OpenTradingInterface(APlayerController* Player)
{
    if (TradingSystem && Player)
    {
        TradingSystem->OpenTradingUI(Player);
    }
}
```

### Docking Setup Template

**Docking Transform Configuration**:

```cpp
// Add to your station blueprint or C++ class
TArray<FTransform> DockingBayTransforms;

void SetupDockingBays()
{
    // Bay 1 - Top docking port
    FTransform Bay1;
    Bay1.SetLocation(FVector(0, 0, 500));
    Bay1.SetRotation(FQuat::Identity);
    DockingBayTransforms.Add(Bay1);

    // Bay 2 - Bottom docking port
    FTransform Bay2;
    Bay2.SetLocation(FVector(0, 0, -500));
    Bay2.SetRotation(FQuat(FVector(0, 1, 0), PI));
    DockingBayTransforms.Add(Bay2);

    // Bay 3 - Side docking port (left)
    FTransform Bay3;
    Bay3.SetLocation(FVector(-500, 0, 0));
    Bay3.SetRotation(FQuat(FVector(0, 0, 1), -PI/2));
    DockingBayTransforms.Add(Bay3);

    // Bay 4 - Side docking port (right)
    FTransform Bay4;
    Bay4.SetLocation(FVector(500, 0, 0));
    Bay4.SetRotation(FQuat(FVector(0, 0, 1), PI/2));
    DockingBayTransforms.Add(Bay4);
}
```

### Mission Board Integration Template

**Add to Station Blueprint**:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Missions")
TArray<FString> MissionTemplateFiles = {
    "Content/Data/Missions/Templates/mission_templates_combat.json",
    "Content/Data/Missions/Templates/mission_templates_delivery.json",
    "Content/Data/Missions/Templates/mission_templates_exploration.json"
};

void LoadMissionTemplates()
{
    for (const FString& TemplateFile : MissionTemplateFiles)
    {
        FString JsonString;
        if (FFileHelper::LoadFileToString(JsonString, *TemplateFile))
        {
            MissionBoard->LoadTemplatesFromJSON(JsonString);
        }
    }
}
```

---

## VFX Configuration Template

### Niagara System Setup for Common Effects

#### 1. Engine Trail VFX Template

**System Name**: `NS_Engine_Trail_[ShipClass]_[Color]`
**Example**: `NS_Engine_Trail_Fighter_Blue`

**Emitter Setup**:

```
Emitter Properties:
├── Spawn Rate: 100-500 particles/sec
├── Lifetime: 0.5-2.0 seconds
├── Local Space: False
└── Bounds: Fixed (1000, 1000, 1000)

Spawn Module:
├── Spawn Rate: 200
└── Spawn Burst: 0

Initialize Particle:
├── Lifetime: Uniform 0.8-1.2
├── Color: RGB (R, G, B, 1.0)
├── Sprite Size: Uniform (10, 20)
├── Velocity: Cone (Speed: -500, Angle: 10)
└── Rotation: Random 0-360

Update Modules:
├── Gravity Force (Z: -100)
├── Drag (Coefficient: 0.5)
├── Color Over Life (Fade from 1.0 to 0.0)
├── Scale Over Life (Start: 1.0, End: 0.2)
└── Sprite Rotation Rate (Degrees/sec: 90)

Rendering:
├── Sprite Renderer
├── Material: M_Particle_Trail
├── Blend Mode: Additive
└── Sort Order: ViewDistance
```

**Color Presets**:
- Blue: (0.2, 0.5, 1.0) - Standard fighters
- Red: (1.0, 0.3, 0.2) - Military/aggressive
- Green: (0.3, 1.0, 0.4) - Eco-friendly
- Purple: (0.8, 0.3, 1.0) - Advanced tech
- Orange: (1.0, 0.6, 0.2) - Mining ships
- White: (0.9, 0.9, 1.0) - Luxury/civilian

---

#### 2. Weapon Impact VFX Template

**System Name**: `NS_Impact_[WeaponType]_[Size]`
**Example**: `NS_Impact_Laser_Medium`

**Emitter Setup**:

```
Emitter 1: Flash
├── Spawn Burst: 1 particle
├── Lifetime: 0.1 seconds
├── Sprite Size: 50-100
├── Color: Weapon color (bright)
└── Scale Over Life: 1.0 → 0.0 (rapid decay)

Emitter 2: Sparks
├── Spawn Burst: 10-30 particles
├── Lifetime: 0.2-0.5 seconds
├── Sprite Size: 2-5
├── Velocity: Sphere (Speed: 200-500)
├── Gravity: -980
└── Color: Yellow-Orange

Emitter 3: Shockwave
├── Spawn Burst: 1 particle
├── Lifetime: 0.3 seconds
├── Mesh: Sphere
├── Scale: 0.1 → 2.0 (expand)
└── Opacity: 1.0 → 0.0 (fade)

Emitter 4: Debris
├── Spawn Burst: 5-15 particles
├── Lifetime: 1.0-2.0 seconds
├── Mesh: Small debris chunks
├── Velocity: Random sphere
├── Rotation Rate: Random
└── Gravity: -980
```

**Weapon Type Variations**:

| Weapon Type | Color | Impact Size | Spark Count | Special Effect |
|-------------|-------|-------------|-------------|----------------|
| Laser | Bright Red/Blue | 50-80 | 15-20 | Lens flare |
| Plasma | Purple/Pink | 80-120 | 25-30 | Electric arcs |
| Kinetic | Orange/Yellow | 40-60 | 30-40 | Metal debris |
| Missile | Red/Orange | 150-300 | 50-100 | Smoke plume |
| Energy | Cyan/White | 100-150 | 20-25 | Energy ripples |
| EMP | Blue/White | 200-400 | 5-10 | Lightning bolts |

---

#### 3. Weather Effect VFX Template

**System Name**: `NS_Weather_[Type]_[Intensity]`
**Example**: `NS_Weather_Rain_Heavy`

**Rain Effect Setup**:

```
Emitter: Rain Particles
├── Spawn Rate: 10000-20000 (heavy), 2000-5000 (light)
├── Lifetime: 2.0-4.0 seconds
├── Local Space: False
├── Spawn Box: 5000x5000x1000 (above player)

Initialize Particle:
├── Lifetime: Uniform 2.0-3.0
├── Sprite Size: (2, 20) - streaked
├── Color: RGBA (0.7, 0.7, 0.8, 0.5)
├── Velocity: (0, 0, -2000) + wind offset
└── Rotation: Align to velocity

Update Modules:
├── Wind Force (variable direction)
├── Kill Volume (ground plane)
└── Color Over Life (maintain opacity)

Splash Effects:
├── Trigger: Collision with ground
├── Spawn Burst: 3-5 particles
├── Lifetime: 0.2 seconds
├── Sprite Size: 5-15
└── Color: White (fade out)
```

---

### Material Instance Templates

#### Engine Trail Material Instance

**Base Material**: `M_Particle_Trail`

**Parameters to Expose**:

```cpp
// Color parameters
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
FLinearColor TrailColor = FLinearColor(0.2, 0.5, 1.0, 1.0);

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
float Brightness = 2.0;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
float Emissive = 5.0;

// Shape parameters
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape")
float TrailWidth = 1.0;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shape")
float DistortionStrength = 0.5;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
float ScrollSpeed = 1.0;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
float NoiseScale = 0.1;
```

**Material Instance Naming**:
- `MI_Trail_Fighter_Blue`
- `MI_Trail_Hauler_Orange`
- `MI_Trail_Military_Red`

---

### Sci-Fi Color Presets

**Standard Color Palette**:

```cpp
// Faction Colors
FLinearColor Militia_Blue = FLinearColor(0.2, 0.5, 1.0);
FLinearColor Pirates_Red = FLinearColor(1.0, 0.2, 0.2);
FLinearColor Traders_Green = FLinearColor(0.3, 1.0, 0.4);
FLinearColor Scientists_Cyan = FLinearColor(0.3, 0.9, 0.9);
FLinearColor Military_Orange = FLinearColor(1.0, 0.5, 0.2);

// UI Colors
FLinearColor UI_Primary = FLinearColor(0.0, 0.7, 1.0); // Cyan
FLinearColor UI_Secondary = FLinearColor(0.0, 0.4, 0.6); // Dark Cyan
FLinearColor UI_Success = FLinearColor(0.2, 1.0, 0.3); // Green
FLinearColor UI_Warning = FLinearColor(1.0, 0.8, 0.0); // Yellow
FLinearColor UI_Error = FLinearColor(1.0, 0.2, 0.2); // Red
FLinearColor UI_Neutral = FLinearColor(0.7, 0.7, 0.7); // Gray

// Environment Colors
FLinearColor Space_Dark = FLinearColor(0.01, 0.01, 0.02);
FLinearColor Space_StarLight = FLinearColor(1.0, 0.95, 0.9);
FLinearColor Nebula_Purple = FLinearColor(0.6, 0.2, 0.8);
FLinearColor Nebula_Blue = FLinearColor(0.2, 0.4, 1.0);
FLinearColor Asteroid_Brown = FLinearColor(0.3, 0.25, 0.2);
```

---

## UI Widget Templates

### 1. HUD Component Template

**Widget Name**: `WBP_HUD_[ComponentName]`
**Example**: `WBP_HUD_HealthBar`

**Blueprint Widget Structure**:

```
Canvas Panel
└── HUD_Container (Overlay)
    ├── Background (Image - semi-transparent)
    ├── Content_Container (Horizontal Box)
    │   ├── Icon (Image)
    │   ├── Label (Text Block)
    │   └── Value (Text Block - dynamic)
    └── Alert_Overlay (Overlay - conditional visibility)
        └── Warning_Flash (Image - animated)
```

**Widget C++ Base Class Template**:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDComponentBase.generated.h"

UCLASS()
class ALEXANDER_API UHUDComponentBase : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    class UImage* Background;

    UPROPERTY(meta = (BindWidget))
    class UImage* Icon;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Label;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Value;

    UPROPERTY(meta = (BindWidget))
    class UImage* Warning_Flash;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
    FLinearColor NormalColor = FLinearColor(0.0, 0.7, 1.0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
    FLinearColor WarningColor = FLinearColor(1.0, 0.8, 0.0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
    FLinearColor CriticalColor = FLinearColor(1.0, 0.2, 0.2);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
    float WarningThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD")
    float CriticalThreshold = 0.15f;

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateValue(float CurrentValue, float MaxValue);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetWarningState(bool bIsWarning);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    float CurrentWarningAlpha = 0.0f;
    bool bWarningActive = false;
};
```

**Widget Implementation**:

```cpp
#include "HUDComponentBase.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UHUDComponentBase::NativeConstruct()
{
    Super::NativeConstruct();

    if (Label)
    {
        Label->SetColorAndOpacity(FSlateColor(NormalColor));
    }
}

void UHUDComponentBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bWarningActive && Warning_Flash)
    {
        CurrentWarningAlpha += InDeltaTime * 3.0f;
        float Alpha = (FMath::Sin(CurrentWarningAlpha) + 1.0f) * 0.5f;
        Warning_Flash->SetOpacity(Alpha * 0.5f);
    }
}

void UHUDComponentBase::UpdateValue(float CurrentValue, float MaxValue)
{
    if (!Value || MaxValue <= 0)
    {
        return;
    }

    float Percentage = CurrentValue / MaxValue;
    Value->SetText(FText::AsNumber(FMath::RoundToInt(CurrentValue)));

    // Update color based on threshold
    FLinearColor DisplayColor = NormalColor;
    if (Percentage <= CriticalThreshold)
    {
        DisplayColor = CriticalColor;
        SetWarningState(true);
    }
    else if (Percentage <= WarningThreshold)
    {
        DisplayColor = WarningColor;
        SetWarningState(true);
    }
    else
    {
        SetWarningState(false);
    }

    if (Value)
    {
        Value->SetColorAndOpacity(FSlateColor(DisplayColor));
    }
}

void UHUDComponentBase::SetWarningState(bool bIsWarning)
{
    bWarningActive = bIsWarning;
    if (Warning_Flash)
    {
        Warning_Flash->SetVisibility(bIsWarning ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    }
}
```

---

### 2. Menu Screen Template

**Widget Name**: `WBP_Menu_[MenuName]`
**Example**: `WBP_Menu_MainMenu`

**Layout Structure**:

```
Canvas Panel
├── Background_Blur (Image - background blur effect)
├── Title_Container (Vertical Box)
│   └── Title (Text Block - Large)
├── Menu_Container (Vertical Box)
│   ├── Button_Container_1 (Horizontal Box)
│   │   ├── Icon (Image)
│   │   └── Button_1 (Button + Text)
│   ├── Button_Container_2 (Horizontal Box)
│   ├── Button_Container_3 (Horizontal Box)
│   └── ...
└── Footer_Container (Horizontal Box)
    └── Version_Text (Text Block)
```

**Copy-Paste Button Style**:

```
Button Style:
├── Normal: Hovered (opacity 0.7)
├── Hovered: Bright (opacity 1.0, glow effect)
├── Pressed: Darker (opacity 0.5)
├── Padding: (20, 10, 20, 10)
└── Font: Roboto Condensed, Size 24

Colors:
├── Normal: (0.0, 0.4, 0.6, 0.7)
├── Hovered: (0.0, 0.7, 1.0, 1.0)
├── Pressed: (0.0, 0.3, 0.5, 0.9)
└── Text: (0.9, 0.9, 0.9, 1.0)
```

---

### 3. Dialog Box Template

**Widget Name**: `WBP_Dialog_[DialogType]`
**Example**: `WBP_Dialog_Confirmation`

**Structure**:

```
Overlay
├── Modal_Background (Image - dimmed overlay)
└── Dialog_Container (Border)
    ├── Content (Vertical Box)
    │   ├── Title (Text Block)
    │   ├── Message (Text Block - wrap text)
    │   ├── Spacer
    │   └── Buttons (Horizontal Box)
    │       ├── Button_Confirm (Button)
    │       ├── Spacer
    │       └── Button_Cancel (Button)
    └── Close_Button (Button - top right)
```

**C++ Dialog Base Class**:

```cpp
UCLASS()
class ALEXANDER_API UDialogBoxBase : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Title;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Message;

    UPROPERTY(meta = (BindWidget))
    class UButton* Button_Confirm;

    UPROPERTY(meta = (BindWidget))
    class UButton* Button_Cancel;

    UPROPERTY(meta = (BindWidget))
    class UButton* Close_Button;

    UFUNCTION(BlueprintCallable, Category = "Dialog")
    void ShowDialog(const FText& TitleText, const FText& MessageText);

    UFUNCTION(BlueprintCallable, Category = "Dialog")
    void CloseDialog();

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogConfirmed);
    UPROPERTY(BlueprintAssignable, Category = "Dialog")
    FOnDialogConfirmed OnConfirmed;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogCancelled);
    UPROPERTY(BlueprintAssignable, Category = "Dialog")
    FOnDialogCancelled OnCancelled;

protected:
    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnConfirmClicked();

    UFUNCTION()
    void OnCancelClicked();
};
```

---

### 4. Notification Template

**Widget Name**: `WBP_Notification_[Type]`
**Example**: `WBP_Notification_Toast`

**Structure**:

```
Border (animated slide-in from top)
├── Content (Horizontal Box)
│   ├── Icon (Image - matches notification type)
│   ├── Message (Text Block)
│   └── Progress_Bar (Progress Bar - optional auto-dismiss)
└── Close_Button (Button - small)
```

**Animation Setup**:

```
Slide In Animation (0.3 seconds):
├── Opacity: 0.0 → 1.0
├── Translation Y: -100 → 0
└── Easing: EaseOut

Slide Out Animation (0.2 seconds):
├── Opacity: 1.0 → 0.0
├── Translation Y: 0 → -100
└── Easing: EaseIn

Auto-Dismiss Timer: 3-5 seconds
```

**Notification Types**:

| Type | Icon Color | Background Color | Use Case |
|------|-----------|------------------|----------|
| Success | Green | (0.2, 1.0, 0.3, 0.9) | Mission completed, item acquired |
| Warning | Yellow | (1.0, 0.8, 0.0, 0.9) | Low fuel, shield warning |
| Error | Red | (1.0, 0.2, 0.2, 0.9) | Mission failed, critical error |
| Info | Cyan | (0.0, 0.7, 1.0, 0.9) | New mission available, tip |

---

## Test Level Template

### Empty Test Level Setup

**Level Name**: `TestLevel_[Feature]_[TestType]`
**Example**: `TestLevel_Combat_Performance`

**Required Actors Checklist**:

```
✓ Player Start (VR-enabled)
✓ Directional Light (Sun)
✓ Sky Atmosphere
✓ Exponential Height Fog
✓ Post Process Volume (VR settings)
✓ Nav Mesh Bounds Volume (if AI testing)
✓ Trigger Volumes (for test sequences)
✓ Test Actors/Systems (feature being tested)
✓ Profiling Markers (performance testing)
```

### Lighting Configuration Template

**Directional Light (Sun)**:

```
Transform:
├── Location: (0, 0, 0)
├── Rotation: (-45, -45, 0) - Adjust for desired time of day
└── Scale: (1, 1, 1)

Properties:
├── Intensity: 10.0 lux (space), 3.0 lux (station interior)
├── Light Color: (255, 245, 235) - Slightly warm white
├── Temperature: 6500K
├── Atmosphere Sun Light: True (if using Sky Atmosphere)
├── Cast Shadows: True
├── Dynamic Shadows: True
└── Shadow Map Method: Shadow Maps
```

**Sky Atmosphere**:

```
Planet:
├── Ground Radius: 637000.0 (Earth-like)
├── Atmosphere Height: 60.0
└── Rayleigh Scattering: Default

Art Direction:
├── Aerial Perspective: 1.0
├── Sky Luminance Factor: 1.0
└── Multi Scattering Factor: 1.0
```

**Exponential Height Fog**:

```
Fog:
├── Fog Density: 0.02 (space), 0.001 (clear), 0.1 (dense)
├── Fog Height Falloff: 0.2
├── Fog Max Opacity: 1.0
└── Start Distance: 0

Inscattering:
├── Inscattering Color: (0.05, 0.05, 0.08) - Space blue
└── Directional Inscattering: 0.5
```

### Post-Processing for VR

**Post Process Volume Settings**:

```
Exposure:
├── Method: Manual
├── Exposure Compensation: 0.0
├── Min Brightness: 0.5
├── Max Brightness: 2.0
└── Auto Exposure: False (to prevent VR nausea)

Bloom:
├── Intensity: 0.3 (subtle for VR)
├── Threshold: 1.0
└── Size Scale: 2.0

Ambient Occlusion:
├── Intensity: 0.5
├── Radius: 40.0
└── Static Fraction: 1.0

Motion Blur:
├── Amount: 0.0 (DISABLED for VR comfort)
└── Max: 0.0

Screen Space Reflections:
├── Intensity: 50.0
├── Quality: 50.0
└── Max Roughness: 0.8

Anti-Aliasing:
├── Method: TAA (Temporal Anti-Aliasing)
├── Quality: High
└── Sharpness: 0.2

VR Specific:
├── Chromatic Aberration: 0.0 (disabled)
├── Vignette: 0.0 (disabled)
├── Film Grain: 0.0 (disabled)
└── Lens Flares: Minimal (reduce VR artifacts)
```

### Profiling Triggers Template

**Profiling Trigger Actor Setup**:

```cpp
UCLASS()
class ALEXANDER_API AProfilingTrigger : public AActor
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UBoxComponent* TriggerVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    FString ProfilerEventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    bool bStartProfiling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    float ProfileDuration = 10.0f;

    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent,
                               AActor* OtherActor,
                               UPrimitiveComponent* OtherComp,
                               int32 OtherBodyIndex,
                               bool bFromSweep,
                               const FHitResult& SweepResult);
};
```

**Profiling Trigger Placement**:

```
Common Locations:
├── Combat_Start_Trigger: Begin of combat arena
├── Performance_Stress_Zone: High-complexity area
├── VFX_Heavy_Zone: Particle-intensive region
├── AI_Spawn_Trigger: Before AI encounter
└── Optimization_Checkpoint: Before/after optimization
```

---

## Quick Command Reference

### Build Commands

```bash
# Development build (fast iteration)
"C:/Program Files/Epic Games/UE_5.6/Engine/Build/BatchFiles/Build.bat" AlexanderEditor Win64 Development "C:/Users/allen/Desktop/Alexander/Alexander/Alexander.uproject"

# Shipping build (final performance test)
"C:/Program Files/Epic Games/UE_5.6/Engine/Build/BatchFiles/Build.bat" Alexander Win64 Shipping "C:/Users/allen/Desktop/Alexander/Alexander/Alexander.uproject"

# Clean build (remove intermediate files)
"C:/Program Files/Epic Games/UE_5.6/Engine/Build/BatchFiles/Clean.bat" AlexanderEditor Win64 Development "C:/Users/allen/Desktop/Alexander/Alexander/Alexander.uproject"
```

### Test Commands

```bash
# Run automated tests
cd /c/Users/allen/Desktop/Alexander/Alexander
python build_and_test.py

# Run profiling test
python autonomous_profiling_test.py

# Run specific test level
"C:/Program Files/Epic Games/UE_5.6/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "C:/Users/allen/Desktop/Alexander/Alexander/Alexander.uproject" /Game/Maps/TestLevel_Combat -ExecCmds="Automation RunTests Combat" -unattended -nopause -testexit="Automation Test Queue Empty"
```

### Git Commands

```bash
# Check status
git status

# Create feature branch
git checkout -b feature/new-mission-type

# Stage and commit
git add Content/Data/Missions/Templates/mission_templates_newtype.json
git commit -m "Add new mission type templates"

# Push to remote
git push origin feature/new-mission-type

# Create pull request (using gh CLI)
gh pr create --title "Add new mission type templates" --body "Summary of changes..."
```

---

## Template Usage Checklist

Before using any template:

- [ ] Read the template documentation thoroughly
- [ ] Understand which parameters need customization
- [ ] Check that required systems are implemented
- [ ] Verify naming conventions match project standards
- [ ] Test in isolated environment first
- [ ] Profile performance impact
- [ ] Document any customizations made
- [ ] Update version control with changes
- [ ] Add entry to project changelog
- [ ] Share template improvements with team

---

**Document Version**: 1.0
**Last Updated**: November 8, 2025
**Next Review**: After first template usage feedback
**Maintained By**: Development Team
