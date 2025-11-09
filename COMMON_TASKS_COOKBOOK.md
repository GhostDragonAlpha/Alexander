# Common Tasks Cookbook - Phase 8 Baseline

**Project**: Alexander VR Space Simulation
**Purpose**: Step-by-step recipes for common development tasks
**Format**: Problem → Solution → Verification → Time Estimate
**Last Updated**: November 8, 2025

---

## Table of Contents

1. [Content Creation Tasks](#content-creation-tasks)
2. [System Integration Tasks](#system-integration-tasks)
3. [Testing & Debugging Tasks](#testing--debugging-tasks)
4. [Performance Tasks](#performance-tasks)
5. [Troubleshooting Recipes](#troubleshooting-recipes)

---

## Content Creation Tasks

### Recipe 1: Add a New Mission Type

**Problem**: I want to create a new type of mission (e.g., "Sabotage" missions)

**Solution**:

**Step 1: Create Mission Template File** (5 minutes)
```bash
# Navigate to mission templates directory
cd Content/Data/Missions/Templates

# Create new template file
touch mission_templates_sabotage.json
```

**Step 2: Define Mission Structure** (15 minutes)
```json
{
  "sabotage_missions": [
    {
      "mission_id": "sabotage_facility_001",
      "mission_type": "Sabotage",
      "title_template": "Sabotage {target_facility}",
      "description_template": "Infiltrate {target_facility} and {sabotage_action} without being detected. {faction_name} must not trace this back to you.",
      "difficulty": "Hard",
      "objectives": [
        {
          "type": "ReachLocation",
          "description": "Infiltrate {target_facility}",
          "quantity": 1,
          "optional": false,
          "hidden": false
        },
        {
          "type": "InteractWithObject",
          "description": "{sabotage_action}",
          "quantity": 1,
          "optional": false,
          "hidden": false
        },
        {
          "type": "AvoidDetection",
          "description": "Escape without being detected",
          "quantity": 1,
          "optional": true,
          "hidden": true
        }
      ],
      "rewards": {
        "credits": 6000,
        "experience": 300,
        "items": ["stealth_upgrade"],
        "faction_rep": {"ShadowBrokers": 40, "TargetFaction": -60}
      },
      "requirements": {
        "min_level": 6,
        "required_faction": null,
        "prerequisite_missions": [],
        "required_equipment": ["stealth_module"]
      },
      "location": {
        "system": "SolarSystem",
        "max_distance": 30000,
        "location_type": "Station"
      },
      "time_limit": 1200,
      "repeatable": false,
      "variables": {
        "target_facility": ["Research Lab Alpha", "Military Depot 7", "Mining Control Center", "Communications Hub"],
        "sabotage_action": ["plant explosive charges", "corrupt navigation systems", "steal data cores", "disable life support"],
        "faction_name": ["The Syndicate", "Corporate Security", "Military Intelligence"]
      }
    }
  ]
}
```

**Step 3: Register Mission Type in Code** (10 minutes)

Edit `Source/Alexander/Public/MissionSystem.h`:
```cpp
// Add to EMissionType enum
UENUM(BlueprintType)
enum class EMissionType : uint8
{
    // ... existing types ...
    Sabotage        UMETA(DisplayName = "Sabotage"),
    // ... rest of types ...
};
```

Edit `Source/Alexander/Private/ProceduralMissionGenerator.cpp`:
```cpp
// Add to mission type weights
TMap<EMissionType, float> MissionTypeWeights = {
    // ... existing weights ...
    {EMissionType::Sabotage, 0.08f},  // 8% chance
};
```

**Step 4: Add Mission Icons/UI** (10 minutes)
```
1. Create mission icon: Content/UI/Icons/Icon_Mission_Sabotage.png (64x64)
2. Add to mission board UI widget
3. Set icon in mission data asset
```

**Step 5: Create Test Mission** (5 minutes)
```bash
# In Unreal Editor Console
GenerateMission Sabotage
```

**Verification**:
- [ ] Mission appears in mission board
- [ ] All objectives are functional
- [ ] Rewards are granted correctly
- [ ] Variables substitute properly in text
- [ ] Mission icon displays correctly
- [ ] Difficulty scales appropriately

**Time Estimate**: 45 minutes (initial setup) + 30 minutes per additional mission template

---

### Recipe 2: Create a Custom Space Station

**Problem**: I want to add a new space station with custom layout and services

**Solution**:

**Step 1: Create Station Blueprint** (10 minutes)
```
1. In Content Browser: Content/Blueprints/Stations/
2. Right-click → Blueprint Class → Actor
3. Name: BP_Station_[Type]_[Name] (e.g., BP_Station_Trading_FreeHaven)
4. Open blueprint
5. Add Static Mesh Component (station exterior model)
```

**Step 2: Add Core Components** (15 minutes)
```
In Blueprint Components panel:

✓ Add Component: Scene (Root)
  └─ Add Component: Static Mesh (StationMesh)
  └─ Add Component: Sphere Collision (DockingZone)
      └─ Set Sphere Radius: 5000
      └─ Set Collision: Overlap All Dynamic
  └─ Add Component: Audio Component (AmbientSound)
      └─ Set Sound: SFX_Station_Ambient_Hum
      └─ Set Auto Activate: True
  └─ Add Component: MissionBoardComponent
  └─ Add Component: TradingComponent
  └─ Add Component: DockingSystemComponent
  └─ Add Component: FactionComponent
```

**Step 3: Configure Station Properties** (10 minutes)
```cpp
In Blueprint Details panel:

Station Info:
├── Station Name: "Free Haven Trading Post"
├── Station Type: "Trading"
├── Max Docking Bays: 6
├── Docking Range: 5000.0

Economy Settings:
├── Price Modifier: 0.95 (5% discount on all goods)
├── Available Goods: [Fuel, Ore, Parts, Weapons, Food]
├── Special Services: [Repair, Refuel, Rearm, Upgrade]

Mission Settings:
├── Max Active Missions: 15
├── Mission Types: [Delivery, Trading, Mining, Exploration]
├── Mission Refresh Rate: 300.0 (5 minutes)

Faction:
├── Controlling Faction: "Traders Guild"
├── Friendly Factions: [Civilians, Miners, Explorers]
├── Hostile Factions: [Pirates]
```

**Step 4: Set Up Docking Bays** (20 minutes)
```
1. Add Scene Components for each docking bay
2. Position them around the station (use symmetry)
3. Set transforms:

Docking Bay 1 (Top):
├── Location: (0, 0, 500)
├── Rotation: (0, 0, 0)

Docking Bay 2 (Bottom):
├── Location: (0, 0, -500)
├── Rotation: (180, 0, 0)

Docking Bay 3-6 (Sides):
├── Location: Radial pattern around station
├── Rotation: Point inward

4. Add visual markers (lights, decals, arrow meshes)
5. Test docking approach paths
```

**Step 5: Configure Mission Board** (10 minutes)
```
1. In Event Graph:
   - Event BeginPlay
   - Get MissionBoardComponent
   - LoadMissionTemplates from JSON files
   - Set Mission Types
   - Initialize Mission Pool

2. Set Mission Template Paths:
   - Content/Data/Missions/Templates/mission_templates_delivery.json
   - Content/Data/Missions/Templates/mission_templates_trading.json
   - Content/Data/Missions/Templates/mission_templates_mining.json
```

**Step 6: Add Interior Spaces (Optional)** (30+ minutes)
```
1. Create interior level: Content/Maps/Interiors/Station_FreeHaven_Interior
2. Add modular interior pieces
3. Place service NPCs
4. Add navigation mesh
5. Create teleport points for VR
6. Link to exterior via docking transition
```

**Step 7: Place in World** (5 minutes)
```
1. Open main level: Content/Maps/SolarSystem
2. Drag BP_Station_Trading_FreeHaven into level
3. Position in orbit or at waypoint
4. Set Initial Transform
5. Add to navigation system
6. Register with StarSystemManager
```

**Verification**:
- [ ] Station appears in world at correct location
- [ ] Docking request works from 5000 units away
- [ ] All 6 docking bays are accessible
- [ ] Mission board shows appropriate missions
- [ ] Trading interface opens correctly
- [ ] Audio plays ambient station sounds
- [ ] Faction reputation affects prices
- [ ] Station icon shows on map/radar

**Time Estimate**: 1.5 hours (basic station) + 2-4 hours (with custom interior)

---

### Recipe 3: Add a New Weapon VFX

**Problem**: I want to create visual effects for a new weapon type

**Solution**:

**Step 1: Create Niagara System** (15 minutes)
```
1. Content Browser: Content/VFX/Weapons/
2. Right-click → Niagara System → New System from Template
3. Choose template: Simple Sprite Burst or Fountain
4. Name: NS_Weapon_[WeaponType]_[Effect] (e.g., NS_Weapon_PlasmaCannon_Projectile)
5. Open Niagara Editor
```

**Step 2: Configure Projectile Emitter** (20 minutes)
```
Emitter: Projectile Trail

Spawn Module:
├── Spawn Rate: 100 particles/sec
├── Spawn Burst: 0

Initialize Particle:
├── Lifetime Mode: Random
├── Lifetime Min: 0.3
├── Lifetime Max: 0.5
├── Color Mode: Direct Set
├── Color: FLinearColor(0.8, 0.3, 1.0) - Purple for plasma
├── Sprite Size Mode: Uniform
├── Sprite Size: FVector2D(15, 15)
├── Velocity Mode: Cone
├── Velocity Cone Angle: 5 degrees
├── Velocity Magnitude: 50 (relative to projectile)

Update Modules:
├── Add Module: Scale Color
    └── Scale Alpha Curve: 1.0 → 0.0 (fade out)
├── Add Module: Scale Sprite Size
    └── Size Curve: 1.0 → 0.3 (shrink)
├── Add Module: Drag
    └── Drag Coefficient: 2.0

Rendering:
├── Sprite Renderer Module
├── Material: M_Particle_Additive
├── Blend Mode: Additive
├── Sort Mode: View Depth
```

**Step 3: Create Impact Effect** (20 minutes)
```
1. Create new Niagara System: NS_Weapon_PlasmaCannon_Impact
2. Add Multiple Emitters:

Emitter 1: Flash
├── Spawn Burst: 1 particle
├── Lifetime: 0.1 seconds
├── Color: Bright Purple (1.5, 0.5, 2.0)
├── Size: 100
├── Scale Over Life: 1.0 → 0.0 (instant flash)

Emitter 2: Plasma Explosion
├── Spawn Burst: 50 particles
├── Lifetime: 0.5-1.0 seconds
├── Color: Purple gradient
├── Size: 5-20
├── Velocity: Sphere Random (300-600)
├── Gravity: -200
├── Drag: 3.0

Emitter 3: Energy Ring
├── Spawn Burst: 1 particle
├── Lifetime: 0.4 seconds
├── Mesh: Ring mesh
├── Color: Purple with high emissive
├── Scale: 0.1 → 3.0 (expand)
├── Opacity: 1.0 → 0.0 (fade)

Emitter 4: Scorch Mark (Decal)
├── Spawn Burst: 1 particle
├── Lifetime: 10.0 seconds (persistent)
├── Decal Material: M_Decal_PlasmaScorch
├── Size: 50
├── Fade In: 0.1 seconds
├── Fade Out: 2.0 seconds (after 8 seconds)
```

**Step 4: Create Material** (15 minutes)
```
1. Create Material: M_Weapon_PlasmaCannon_Trail
2. Material Domain: Surface (or User Interface for particles)
3. Blend Mode: Additive
4. Shading Model: Unlit

Material Graph:
├── Texture Sample (T_Particle_Soft)
├── Multiply by Color Parameter (Weapon Color)
├── Multiply by Scalar Parameter (Intensity: 5.0)
├── Connect to Emissive Color
├── Texture Alpha → Opacity
├── Add Fresnel (edge glow)
```

**Step 5: Create Material Instance** (5 minutes)
```
1. Right-click M_Weapon_PlasmaCannon_Trail → Create Material Instance
2. Name: MI_Weapon_PlasmaCannon_Purple
3. Set Parameters:
   ├── Weapon Color: (0.8, 0.3, 1.0, 1.0)
   ├── Intensity: 5.0
   ├── Fresnel Exponent: 3.0
   └── Fresnel Intensity: 2.0
```

**Step 6: Integrate with Weapon System** (15 minutes)

Edit weapon blueprint or C++ class:
```cpp
UPROPERTY(EditDefaultsOnly, Category = "VFX")
UNiagaraSystem* ProjectileTrailVFX;

UPROPERTY(EditDefaultsOnly, Category = "VFX")
UNiagaraSystem* ImpactVFX;

UPROPERTY(EditDefaultsOnly, Category = "VFX")
USoundBase* FireSound;

UPROPERTY(EditDefaultsOnly, Category = "VFX")
USoundBase* ImpactSound;

void AWeaponPlasmaCannon::FireWeapon()
{
    // Spawn projectile with trail VFX
    if (ProjectileTrailVFX)
    {
        UNiagaraComponent* Trail = UNiagaraFunctionLibrary::SpawnSystemAttached(
            ProjectileTrailVFX,
            ProjectileMesh,
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget,
            true
        );
    }

    // Play fire sound
    if (FireSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
    }
}

void AWeaponPlasmaCannon::OnProjectileImpact(const FHitResult& HitResult)
{
    // Spawn impact VFX
    if (ImpactVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            ImpactVFX,
            HitResult.Location,
            HitResult.Normal.Rotation()
        );
    }

    // Play impact sound
    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, HitResult.Location);
    }
}
```

**Step 7: Test and Optimize** (15 minutes)
```
1. Place weapon in test level
2. Fire at various surfaces (metal, rock, shields)
3. Check VFX performance (Stat Particles, Stat Niagara)
4. Adjust particle counts for 90 FPS target
5. Test in VR (check for motion sickness)
6. Adjust intensity for VR comfort
```

**Verification**:
- [ ] Projectile trail appears when firing
- [ ] Impact effect spawns at collision point
- [ ] Colors match weapon theme
- [ ] Performance stays above 90 FPS
- [ ] VFX visible from multiple angles
- [ ] No particle flickering in VR
- [ ] Sound effects synchronized with visuals
- [ ] Decals appear on impact surfaces

**Time Estimate**: 1.5 hours (basic VFX) + 1 hour (polish and optimization)

---

### Recipe 4: Create a Custom HUD Element

**Problem**: I want to add a new HUD component (e.g., mission tracker)

**Solution**:

**Step 1: Create Widget Blueprint** (5 minutes)
```
1. Content Browser: Content/UI/HUD/
2. Right-click → User Interface → Widget Blueprint
3. Name: WBP_HUD_MissionTracker
4. Open Widget Designer
```

**Step 2: Design Widget Layout** (20 minutes)
```
Widget Hierarchy:

Canvas Panel (Root)
└── Border (HUD_Container)
    ├── Background (Image)
    │   └── Brush: Solid Color (0.0, 0.1, 0.2, 0.8)
    │   └── Padding: (10, 10, 10, 10)
    ├── Vertical Box (Content_Container)
    │   ├── Text Block (Title)
    │   │   └── Text: "Active Mission"
    │   │   └── Font: Roboto Condensed, Size 18
    │   │   └── Color: Cyan (0.0, 0.7, 1.0)
    │   ├── Spacer (Height: 5)
    │   ├── Text Block (Mission_Title)
    │   │   └── Text: "Mission Name Here"
    │   │   └── Font: Roboto, Size 14
    │   │   └── Color: White
    │   ├── Spacer (Height: 10)
    │   ├── Vertical Box (Objectives_Container)
    │   │   └── (Dynamically populated with objective widgets)
    │   ├── Spacer (Height: 10)
    │   └── Progress Bar (Mission_Progress)
    │       └── Fill Color: Green (0.2, 1.0, 0.3)
    │       └── Background Color: Dark Gray
    └── Image (Mission_Icon)
        └── Position: Top Right corner
        └── Size: 32x32
```

**Step 3: Create Widget C++ Base Class** (30 minutes)

`MissionTrackerWidget.h`:
```cpp
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MissionTrackerWidget.generated.h"

UCLASS()
class ALEXANDER_API UMissionTrackerWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    // Bound widgets
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Mission_Title;

    UPROPERTY(meta = (BindWidget))
    class UVerticalBox* Objectives_Container;

    UPROPERTY(meta = (BindWidget))
    class UProgressBar* Mission_Progress;

    UPROPERTY(meta = (BindWidget))
    class UImage* Mission_Icon;

    // Widget to spawn for each objective
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> ObjectiveWidgetClass;

    // Update functions
    UFUNCTION(BlueprintCallable, Category = "Mission")
    void SetActiveMission(const FMissionData& MissionData);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void UpdateObjectiveProgress(int32 ObjectiveIndex, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void CompleteObjective(int32 ObjectiveIndex);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void ClearMission();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    TArray<class UObjectiveWidget*> ObjectiveWidgets;
    FMissionData CurrentMission;
    float TotalProgress;

    void UpdateProgress();
    void AnimateObjectiveComplete(int32 ObjectiveIndex);
};
```

`MissionTrackerWidget.cpp`:
```cpp
#include "MissionTrackerWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Components/Image.h"
#include "ObjectiveWidget.h"

void UMissionTrackerWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Initialize empty state
    ClearMission();
}

void UMissionTrackerWidget::SetActiveMission(const FMissionData& MissionData)
{
    CurrentMission = MissionData;

    // Set mission title
    if (Mission_Title)
    {
        Mission_Title->SetText(FText::FromString(MissionData.Title));
    }

    // Set mission icon
    if (Mission_Icon && MissionData.Icon)
    {
        Mission_Icon->SetBrushFromTexture(MissionData.Icon);
    }

    // Clear old objectives
    if (Objectives_Container)
    {
        Objectives_Container->ClearChildren();
        ObjectiveWidgets.Empty();
    }

    // Create objective widgets
    if (ObjectiveWidgetClass)
    {
        for (int32 i = 0; i < MissionData.Objectives.Num(); i++)
        {
            UObjectiveWidget* ObjWidget = CreateWidget<UObjectiveWidget>(this, ObjectiveWidgetClass);
            if (ObjWidget)
            {
                ObjWidget->SetObjectiveData(MissionData.Objectives[i]);
                Objectives_Container->AddChild(ObjWidget);
                ObjectiveWidgets.Add(ObjWidget);
            }
        }
    }

    UpdateProgress();
}

void UMissionTrackerWidget::UpdateObjectiveProgress(int32 ObjectiveIndex, float Progress)
{
    if (ObjectiveWidgets.IsValidIndex(ObjectiveIndex))
    {
        ObjectiveWidgets[ObjectiveIndex]->SetProgress(Progress);
        UpdateProgress();
    }
}

void UMissionTrackerWidget::CompleteObjective(int32 ObjectiveIndex)
{
    if (ObjectiveWidgets.IsValidIndex(ObjectiveIndex))
    {
        ObjectiveWidgets[ObjectiveIndex]->MarkComplete();
        AnimateObjectiveComplete(ObjectiveIndex);
        UpdateProgress();
    }
}

void UMissionTrackerWidget::ClearMission()
{
    if (Mission_Title)
    {
        Mission_Title->SetText(FText::FromString("No Active Mission"));
    }

    if (Objectives_Container)
    {
        Objectives_Container->ClearChildren();
    }

    if (Mission_Progress)
    {
        Mission_Progress->SetPercent(0.0f);
    }

    ObjectiveWidgets.Empty();
}

void UMissionTrackerWidget::UpdateProgress()
{
    if (!Mission_Progress || ObjectiveWidgets.Num() == 0)
    {
        return;
    }

    int32 CompletedObjectives = 0;
    for (UObjectiveWidget* ObjWidget : ObjectiveWidgets)
    {
        if (ObjWidget && ObjWidget->IsCompleted())
        {
            CompletedObjectives++;
        }
    }

    TotalProgress = static_cast<float>(CompletedObjectives) / ObjectiveWidgets.Num();
    Mission_Progress->SetPercent(TotalProgress);
}

void UMissionTrackerWidget::AnimateObjectiveComplete(int32 ObjectiveIndex)
{
    // Play completion animation (flash green, check mark, etc.)
    if (ObjectiveWidgets.IsValidIndex(ObjectiveIndex))
    {
        // Trigger blueprint animation or C++ animation
        ObjectiveWidgets[ObjectiveIndex]->PlayCompleteAnimation();
    }
}
```

**Step 4: Create Objective Widget** (15 minutes)

Similar process for `WBP_ObjectiveItem` widget (smaller component)

**Step 5: Add to Main HUD** (10 minutes)
```
1. Open main HUD blueprint: WBP_MainHUD
2. Add WBP_HUD_MissionTracker to canvas
3. Position in bottom-left corner
4. Set Anchors: (0.0, 1.0) - Bottom Left
5. Set Position: (20, -200)
6. Set Size: (300, 180)
7. Set Z-Order: Above other HUD elements
```

**Step 6: Hook Up to Mission System** (15 minutes)
```cpp
// In PlayerHUD or GameMode
UPROPERTY()
UMissionTrackerWidget* MissionTrackerWidget;

void APlayerHUD::BeginPlay()
{
    Super::BeginPlay();

    // Find mission tracker widget
    MissionTrackerWidget = Cast<UMissionTrackerWidget>(
        MainHUDWidget->GetWidgetFromName("WBP_HUD_MissionTracker")
    );
}

void APlayerHUD::OnMissionAccepted(const FMissionData& MissionData)
{
    if (MissionTrackerWidget)
    {
        MissionTrackerWidget->SetActiveMission(MissionData);
    }
}

void APlayerHUD::OnObjectiveUpdated(int32 ObjectiveIndex, float Progress)
{
    if (MissionTrackerWidget)
    {
        MissionTrackerWidget->UpdateObjectiveProgress(ObjectiveIndex, Progress);
    }
}
```

**Verification**:
- [ ] Widget appears in correct HUD position
- [ ] Mission title updates when mission accepted
- [ ] Objectives list populates correctly
- [ ] Progress bar updates as objectives complete
- [ ] Completion animation plays
- [ ] Widget clears when mission ends
- [ ] Readable in VR headset
- [ ] No performance impact (< 0.1ms)

**Time Estimate**: 1.5 hours (basic HUD) + 30 minutes (animations/polish)

---

### Recipe 5: Test Performance of New Content

**Problem**: I want to profile the performance impact of newly added content

**Solution**:

**Step 1: Prepare Test Environment** (5 minutes)
```bash
# Ensure you're on a clean build
cd /c/Users/allen/Desktop/Alexander/Alexander
python build_and_test.py --clean
```

**Step 2: Baseline Performance Capture** (10 minutes)
```
1. Launch editor or game
2. Open console (` key)
3. Run profiling commands:

# Start GPU profiling
stat GPU

# Start detailed profiling
stat SceneRendering
stat Particles
stat Niagara

# Capture baseline FPS
t.MaxFPS 0
stat FPS
stat Unit

# Record baseline values:
   - Frame Time: ___ ms (target < 11.1ms for 90 FPS)
   - Game Thread: ___ ms
   - Render Thread: ___ ms
   - GPU Time: ___ ms
```

**Step 3: Add New Content to Scene** (varies)
```
1. Place your new content in test level
2. Set up test scenario (e.g., spawn 10 VFX, load mission templates)
3. Save level
```

**Step 4: Performance Test with New Content** (15 minutes)
```
1. Reload level
2. Run same profiling commands
3. Record new values:
   - Frame Time: ___ ms
   - Game Thread: ___ ms
   - Render Thread: ___ ms
   - GPU Time: ___ ms
4. Calculate delta (difference from baseline)
```

**Step 5: Detailed Profiling** (20 minutes)
```bash
# Run automated profiling script
python autonomous_profiling_test.py

# Or use Unreal Insights
# 1. Launch with trace
UnrealEditor.exe Alexander.uproject -trace=cpu,gpu,frame

# 2. Open Unreal Insights
UnrealInsights.exe

# 3. Load trace file
# 4. Analyze Frame Time, CPU, GPU usage
```

**Step 6: Analyze Results** (15 minutes)
```
Performance Impact Analysis:

✓ Acceptable: < 1ms frame time increase
⚠ Warning: 1-2ms frame time increase
❌ Unacceptable: > 2ms frame time increase

Common Bottlenecks:
├── Too many particles? → Reduce spawn rate or lifetime
├── Too many draw calls? → Merge meshes, use instancing
├── Expensive materials? → Simplify shader, reduce instructions
├── Too many lights? → Use static lighting, reduce dynamic lights
└── Memory spikes? → Stream assets, reduce texture sizes

Optimization Priority:
1. GPU bottlenecks (most common in VR)
2. Render thread bottlenecks
3. Game thread bottlenecks
4. Memory/loading issues
```

**Step 7: Optimize if Needed** (varies)
```
Common Optimizations:

Particles/VFX:
├── Reduce particle count (divide by 2)
├── Reduce emitter lifetime
├── Use LOD (distant VFX simpler)
├── Disable VFX beyond certain distance
└── Use GPU particles instead of CPU

Meshes:
├── Reduce polygon count
├── Use LODs (Auto LOD in Unreal)
├── Merge static meshes
├── Use Instanced Static Meshes
└── Enable Hierarchical LOD (HLOD)

Materials:
├── Reduce instruction count
├── Simplify complex nodes
├── Use material instances
├── Disable features (tessellation, parallax)
└── Reduce texture samples

Blueprints:
├── Move heavy logic to C++
├── Reduce Tick frequency
├── Use timers instead of Tick
├── Cache component references
└── Avoid expensive nodes in hot paths
```

**Step 8: Re-test After Optimization** (10 minutes)
```
1. Rebuild project
2. Re-run performance test
3. Verify frame time is acceptable
4. Document optimizations made
```

**Verification**:
- [ ] Frame time < 11.1ms (90 FPS) in VR
- [ ] No hitching or stuttering
- [ ] Memory usage stable
- [ ] GPU usage < 90%
- [ ] CPU usage balanced across cores
- [ ] Load times acceptable (< 5 seconds)

**Time Estimate**: 1-2 hours (including optimization cycle)

---

## System Integration Tasks

### Recipe 6: Integrate New System with Existing Codebase

**Problem**: I created a new gameplay system and need to integrate it with existing systems

**Solution**:

**Step 1: Identify Integration Points** (15 minutes)
```
Create Integration Checklist:

Systems to Connect:
├── [ ] Player Character (for input/interaction)
├── [ ] Game Mode (for initialization)
├── [ ] HUD/UI (for player feedback)
├── [ ] Save System (for persistence)
├── [ ] Mission System (if missions interact)
├── [ ] Economy System (if costs/rewards)
├── [ ] Faction System (if reputation matters)
└── [ ] Achievement System (if unlockables)

Data Flow:
├── [ ] What data does your system need?
├── [ ] What data does your system provide?
├── [ ] What events does your system trigger?
└── [ ] What events does your system listen for?
```

**Step 2: Create System Component** (30 minutes)
```cpp
// Example: Asteroid Mining System Component

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class ALEXANDER_API UAsteroidMiningComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAsteroidMiningComponent();

    // Initialization
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                                FActorComponentTickFunction* ThisTickFunction) override;

    // Public API
    UFUNCTION(BlueprintCallable, Category = "Mining")
    bool StartMining(AAsteroid* TargetAsteroid);

    UFUNCTION(BlueprintCallable, Category = "Mining")
    void StopMining();

    UFUNCTION(BlueprintCallable, Category = "Mining")
    float GetMiningProgress() const { return MiningProgress; }

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMiningComplete,
        EMineralType, MineralType, int32, Quantity);
    UPROPERTY(BlueprintAssignable, Category = "Mining")
    FOnMiningComplete OnMiningComplete;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMiningFailed,
        FString, Reason);
    UPROPERTY(BlueprintAssignable, Category = "Mining")
    FOnMiningFailed OnMiningFailed;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Mining")
    float MiningSpeed = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Mining")
    float MiningRange = 500.0f;

private:
    UPROPERTY()
    AAsteroid* CurrentAsteroid;

    float MiningProgress;
    bool bIsMining;

    void UpdateMiningProgress(float DeltaTime);
    void CompleteMining();
};
```

**Step 3: Register with Game Systems** (20 minutes)
```cpp
// In GameMode or GameInstance

void AAlexanderGameMode::InitializeGameSystems()
{
    // Register mining system with economy
    if (EconomySystem && MiningSystem)
    {
        MiningSystem->OnMiningComplete.AddDynamic(
            this, &AAlexanderGameMode::OnPlayerMinedResources
        );
    }

    // Register with mission system
    if (MissionSystem && MiningSystem)
    {
        MissionSystem->RegisterObjectiveHandler(
            "CollectMinerals",
            MiningSystem,
            &UAsteroidMiningComponent::OnMiningComplete
        );
    }

    // Register with save system
    if (SaveGameSystem && MiningSystem)
    {
        SaveGameSystem->RegisterSaveableComponent(MiningSystem);
    }
}

void AAlexanderGameMode::OnPlayerMinedResources(EMineralType Type, int32 Quantity)
{
    // Update economy
    EconomySystem->AddPlayerResource(Type, Quantity);

    // Trigger mission objective update
    MissionSystem->UpdateObjective("CollectMinerals", Type, Quantity);

    // Update UI
    PlayerHUD->ShowNotification(
        FText::FromString(FString::Printf(TEXT("Mined %d %s"), Quantity, *GetMineralName(Type))),
        ENotificationType::Success
    );

    // Check achievements
    AchievementSystem->CheckMiningAchievements(Type, Quantity);
}
```

**Step 4: Update UI Integration** (25 minutes)
```cpp
// Create UI widget for system
// Add to HUD
// Bind to system events

void UPlayerHUD::InitializeMiningUI()
{
    if (MiningWidgetClass)
    {
        MiningWidget = CreateWidget<UMiningWidget>(GetOwningPlayer(), MiningWidgetClass);
        if (MiningWidget)
        {
            MiningWidget->AddToViewport();

            // Bind to mining system events
            if (MiningSystem)
            {
                MiningSystem->OnMiningComplete.AddDynamic(
                    MiningWidget, &UMiningWidget::OnMiningComplete
                );

                MiningSystem->OnMiningStarted.AddDynamic(
                    MiningWidget, &UMiningWidget::ShowMiningProgress
                );
            }
        }
    }
}
```

**Step 5: Add Save/Load Support** (20 minutes)
```cpp
// Implement save interface

class UAsteroidMiningComponent : public UActorComponent, public ISaveableComponent
{
    // ... existing code ...

    // ISaveableComponent interface
    virtual FSaveData GetSaveData() const override
    {
        FSaveData Data;
        Data.AddFloat("MiningProgress", MiningProgress);
        Data.AddBool("bIsMining", bIsMining);
        if (CurrentAsteroid)
        {
            Data.AddString("CurrentAsteroidID", CurrentAsteroid->GetUniqueID());
        }
        return Data;
    }

    virtual void LoadSaveData(const FSaveData& Data) override
    {
        MiningProgress = Data.GetFloat("MiningProgress", 0.0f);
        bIsMining = Data.GetBool("bIsMining", false);

        FString AsteroidID = Data.GetString("CurrentAsteroidID", "");
        if (!AsteroidID.IsEmpty() && bIsMining)
        {
            // Resume mining on saved asteroid
            CurrentAsteroid = FindAsteroidByID(AsteroidID);
        }
    }
};
```

**Step 6: Test Integration** (30 minutes)
```
Integration Test Checklist:

Basic Functionality:
├── [ ] System initializes correctly
├── [ ] System responds to player input
├── [ ] System updates game state
└── [ ] System cleans up on shutdown

Economy Integration:
├── [ ] Resources added to inventory
├── [ ] Costs deducted correctly
└── [ ] Economy balancing works

Mission Integration:
├── [ ] Mining objectives track progress
├── [ ] Objectives complete correctly
└── [ ] Mission rewards granted

UI Integration:
├── [ ] UI updates when system state changes
├── [ ] Notifications appear correctly
└── [ ] UI is readable in VR

Save/Load:
├── [ ] System state saves correctly
├── [ ] System state loads correctly
└── [ ] No data loss on save/load cycle

Performance:
├── [ ] No frame drops when system active
├── [ ] Memory usage acceptable
└── [ ] No leaks detected
```

**Verification**:
- [ ] All integration points tested
- [ ] No crashes or errors
- [ ] Events trigger correctly
- [ ] Data flows between systems
- [ ] UI updates properly
- [ ] Save/load works
- [ ] Performance acceptable

**Time Estimate**: 2-3 hours (simple system) + 1-2 hours (testing)

---

## Testing & Debugging Tasks

### Recipe 7: Debug Why Missions Aren't Showing

**Problem**: Missions aren't appearing on the mission board

**Solution**:

**Diagnostic Step 1: Check Mission JSON Loading** (5 minutes)
```cpp
// Add logging to mission template loading
void UMissionBoardComponent::LoadTemplatesFromJSON(const FString& JsonString)
{
    UE_LOG(LogMission, Log, TEXT("Loading mission templates from JSON..."));

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        UE_LOG(LogMission, Log, TEXT("JSON parsed successfully"));

        // Check if mission arrays exist
        if (JsonObject->HasField("combat_missions"))
        {
            TArray<TSharedPtr<FJsonValue>> CombatMissions = JsonObject->GetArrayField("combat_missions");
            UE_LOG(LogMission, Log, TEXT("Found %d combat missions"), CombatMissions.Num());
        }
        else
        {
            UE_LOG(LogMission, Error, TEXT("No 'combat_missions' field in JSON!"));
        }
    }
    else
    {
        UE_LOG(LogMission, Error, TEXT("Failed to parse JSON!"));
    }
}

// Check console for these logs when mission board initializes
```

**Diagnostic Step 2: Verify File Paths** (5 minutes)
```
1. Open Output Log (Window → Developer Tools → Output Log)
2. Filter by "LogMission"
3. Check for errors like:
   - "File not found: ..."
   - "Failed to load file: ..."
   - "Invalid JSON: ..."

Common Issues:
├── Wrong file path (Content/ vs /Game/)
├── File extension missing (.json)
├── File not packaged (check .uproject settings)
└── File permissions (read-only, locked)
```

**Diagnostic Step 3: Check Mission Type Filters** (5 minutes)
```cpp
// In Mission Board UI or component
void UMissionBoardComponent::FilterMissions()
{
    UE_LOG(LogMission, Log, TEXT("Filtering missions..."));
    UE_LOG(LogMission, Log, TEXT("Available mission types: %d"), AvailableMissionTypes.Num());

    for (const FString& MissionType : AvailableMissionTypes)
    {
        UE_LOG(LogMission, Log, TEXT("  - %s"), *MissionType);
    }

    UE_LOG(LogMission, Log, TEXT("Total missions before filter: %d"), AllMissions.Num());
    // ... filtering logic ...
    UE_LOG(LogMission, Log, TEXT("Total missions after filter: %d"), FilteredMissions.Num());
}
```

**Diagnostic Step 4: Check Player Requirements** (10 minutes)
```cpp
// Log why missions are rejected
bool UMissionBoardComponent::IsMissionAvailable(const FMissionData& Mission)
{
    // Check level requirement
    if (PlayerLevel < Mission.Requirements.MinLevel)
    {
        UE_LOG(LogMission, Warning, TEXT("Mission '%s' rejected: Player level %d < required %d"),
            *Mission.Title, PlayerLevel, Mission.Requirements.MinLevel);
        return false;
    }

    // Check prerequisite missions
    for (const FString& PrereqID : Mission.Requirements.PrerequisiteMissions)
    {
        if (!HasCompletedMission(PrereqID))
        {
            UE_LOG(LogMission, Warning, TEXT("Mission '%s' rejected: Prerequisite '%s' not completed"),
                *Mission.Title, *PrereqID);
            return false;
        }
    }

    // Check faction requirement
    if (!Mission.Requirements.RequiredFaction.IsEmpty())
    {
        float FactionRep = GetFactionReputation(Mission.Requirements.RequiredFaction);
        if (FactionRep < 0.0f)
        {
            UE_LOG(LogMission, Warning, TEXT("Mission '%s' rejected: Negative reputation with %s"),
                *Mission.Title, *Mission.Requirements.RequiredFaction);
            return false;
        }
    }

    UE_LOG(LogMission, Log, TEXT("Mission '%s' is available!"), *Mission.Title);
    return true;
}
```

**Diagnostic Step 5: Debug Mission Board UI** (10 minutes)
```
1. Open Mission Board widget blueprint
2. Add debug overlays to show:
   - Total missions loaded: __
   - Filtered missions: __
   - Visible missions: __
3. Check if missions are loaded but not displayed:
   - Scroll view issue?
   - Widget generation failing?
   - Visibility settings wrong?
```

**Common Fixes**:

**Fix 1: Correct File Path** (2 minutes)
```cpp
// Wrong:
"Content/Data/Missions/Templates/mission_templates_combat.json"

// Correct:
"/Game/Data/Missions/Templates/mission_templates_combat.json"

// Or use project-relative path:
FPaths::ProjectContentDir() + TEXT("Data/Missions/Templates/mission_templates_combat.json")
```

**Fix 2: Add Mission Types to Station** (3 minutes)
```cpp
// In station blueprint or C++ initialization
void ASpaceStation::BeginPlay()
{
    Super::BeginPlay();

    if (MissionBoard)
    {
        // Make sure mission types are enabled!
        TArray<FString> EnabledTypes = {"Combat", "Delivery", "Exploration"};
        MissionBoard->SetMissionTypes(EnabledTypes);
    }
}
```

**Fix 3: Lower Level Requirements** (2 minutes)
```json
{
  "requirements": {
    "min_level": 1,  // Changed from 5 to 1 for testing
    "required_faction": null,
    "prerequisite_missions": []  // Removed prerequisites for testing
  }
}
```

**Fix 4: Force Mission Refresh** (2 minutes)
```
// In console or code
MissionBoard->RefreshMissions();

// Or in blueprint:
Event BeginPlay
  → Delay (1 second)
  → Mission Board Component → Refresh Missions
```

**Verification**:
- [ ] Mission templates load without errors
- [ ] Missions pass all requirement checks
- [ ] Missions appear in UI
- [ ] Mission count matches expected
- [ ] Mission details display correctly
- [ ] Accepting missions works

**Time Estimate**: 30-45 minutes (diagnosis and fix)

---

## Performance Tasks

### Recipe 8: Optimize a Slow System

**Problem**: A specific system is causing frame drops

**Solution**:

**Step 1: Identify the Bottleneck** (15 minutes)
```
Use Unreal Profiler:

1. stat StartFile
2. Reproduce slow scenario
3. stat StopFile
4. Open .ue4stats file in Session Frontend

Look for:
├── Functions taking > 1ms
├── Functions called many times per frame
├── GPU shaders with high cost
└── Memory allocations in hot paths
```

**Step 2: Analyze System Architecture** (20 minutes)
```
Ask these questions:

Frequency:
├── Does this need to run every frame? (Tick)
├── Can it run every N frames?
├── Can it run on a timer instead?
└── Can it be event-driven?

Complexity:
├── Are there nested loops?
├── Are there expensive operations (sqrt, sin, cos)?
├── Are there dynamic allocations?
└── Are there cache misses?

Redundancy:
├── Are results being recalculated?
├── Can results be cached?
├── Are there duplicate operations?
└── Can work be precomputed?
```

**Step 3: Apply Optimizations** (varies)

**Optimization 1: Reduce Tick Frequency**
```cpp
// Before:
void AMyActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateExpensiveSystem();  // Every frame!
}

// After:
void AMyActor::BeginPlay()
{
    Super::BeginPlay();

    // Tick every 0.1 seconds instead of every frame
    PrimaryActorTick.TickInterval = 0.1f;

    // Or disable tick and use timer
    PrimaryActorTick.bCanEverTick = false;
    GetWorld()->GetTimerManager().SetTimer(
        UpdateTimer,
        this,
        &AMyActor::UpdateExpensiveSystem,
        0.1f,
        true  // Looping
    );
}
```

**Optimization 2: Cache Expensive Calculations**
```cpp
// Before:
void ASpaceship::Tick(float DeltaTime)
{
    FVector PlayerLocation = GetPlayerLocation();  // Component lookup every frame
    float Distance = FVector::Dist(GetActorLocation(), PlayerLocation);  // Sqrt every frame

    if (Distance < 1000.0f)
    {
        // Do something
    }
}

// After:
class ASpaceship
{
private:
    FVector CachedPlayerLocation;
    float CachedDistanceSquared;  // Store squared distance to avoid sqrt
    float CacheLifetime = 0.0f;
    const float CacheDuration = 0.5f;  // Update cache every 0.5s
};

void ASpaceship::Tick(float DeltaTime)
{
    CacheLifetime += DeltaTime;

    if (CacheLifetime >= CacheDuration)
    {
        CachedPlayerLocation = GetPlayerLocation();
        CachedDistanceSquared = FVector::DistSquared(GetActorLocation(), CachedPlayerLocation);
        CacheLifetime = 0.0f;
    }

    if (CachedDistanceSquared < 1000000.0f)  // 1000^2
    {
        // Do something
    }
}
```

**Optimization 3: Move to C++ from Blueprint**
```cpp
// Blueprint visual scripting can be 10x slower than C++
// Move hot paths to C++

// Before (Blueprint):
// OnTick → ForEachLoop → ComplexCalculation → SetVariable

// After (C++):
void AOptimizedActor::Tick(float DeltaTime)
{
    for (AActor* Actor : CachedActorArray)  // Array cached, not TActorIterator
    {
        OptimizedCalculation(Actor);  // Inlined C++ function
    }
}

FORCEINLINE void OptimizedCalculation(AActor* Actor)
{
    // Compiler can optimize this
    // No blueprint overhead
}
```

**Optimization 4: Use Object Pooling**
```cpp
// Before: Spawn/Destroy every frame (expensive!)
void AWeapon::Fire()
{
    AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(...);
    // ... projectile destroys itself after lifetime
}

// After: Object pool (reuse objects)
class UProjectilePool
{
public:
    AProjectile* GetProjectile()
    {
        if (AvailableProjectiles.Num() > 0)
        {
            return AvailableProjectiles.Pop();  // Reuse!
        }
        else
        {
            return CreateNewProjectile();  // Only if pool empty
        }
    }

    void ReturnProjectile(AProjectile* Projectile)
    {
        Projectile->Reset();
        Projectile->SetActorHiddenInGame(true);
        AvailableProjectiles.Add(Projectile);
    }

private:
    TArray<AProjectile*> AvailableProjectiles;
};
```

**Optimization 5: LOD (Level of Detail)**
```cpp
// Reduce complexity based on distance

void AComplexActor::Tick(float DeltaTime)
{
    float DistanceToPlayer = GetDistanceToPlayer();

    if (DistanceToPlayer < 1000.0f)
    {
        // Full detail
        UpdatePhysics(DeltaTime);
        UpdateAnimation(DeltaTime);
        UpdateParticles(DeltaTime);
    }
    else if (DistanceToPlayer < 5000.0f)
    {
        // Medium detail
        UpdatePhysics(DeltaTime);
        // Skip animation and particles
    }
    else
    {
        // Low detail
        // Only update essential logic
        UpdatePosition(DeltaTime);
    }
}
```

**Step 4: Profile After Optimization** (10 minutes)
```
1. Rebuild project
2. Run same profiling test
3. Compare results:
   - Before: X ms
   - After: Y ms
   - Improvement: Z%
4. Verify no functionality broken
```

**Step 5: Document Optimization** (5 minutes)
```
Add comment in code:
/*
 * Optimization: Reduced tick frequency from every frame to 0.1s
 * Reason: Distance checks don't need frame-perfect accuracy
 * Performance Gain: 2.5ms → 0.1ms per frame
 * Date: 2025-11-08
 */
```

**Verification**:
- [ ] Frame time improved
- [ ] Functionality unchanged
- [ ] No new bugs introduced
- [ ] Optimization documented
- [ ] Team notified of changes

**Time Estimate**: 1-3 hours (depending on complexity)

---

## Troubleshooting Recipes

### Common Issues and Quick Fixes

**Issue: VR Performance Drops**
```
Checklist:
├── [ ] Disable motion blur (stat PostProcessing)
├── [ ] Reduce particle counts (stat Particles)
├── [ ] Check draw calls (stat RHI)
├── [ ] Disable expensive post-processing
├── [ ] Use Forward Rendering for VR
└── [ ] Enable Instanced Stereo Rendering
```

**Issue: Content Not Appearing**
```
Checklist:
├── [ ] Check file path is correct
├── [ ] Verify asset is cooked/packaged
├── [ ] Check visibility settings
├── [ ] Verify spawn location (in view?)
├── [ ] Check collision (blocking spawn?)
└── [ ] Look for errors in Output Log
```

**Issue: Crashes on Launch**
```
Checklist:
├── [ ] Check for circular dependencies
├── [ ] Verify all includes are correct
├── [ ] Check for null pointer access
├── [ ] Review recent changes (git diff)
├── [ ] Clean and rebuild project
└── [ ] Check crash logs (Saved/Logs/)
```

**Issue: Save/Load Not Working**
```
Checklist:
├── [ ] Verify save game class is correct
├── [ ] Check save game slot name
├── [ ] Ensure save directory is writable
├── [ ] Check for serialization errors
├── [ ] Verify load happens after initialization
└── [ ] Check save file exists (Saved/SaveGames/)
```

---

**Document Version**: 1.0
**Last Updated**: November 8, 2025
**Maintained By**: Development Team
**Feedback**: Report issues or suggest new recipes
