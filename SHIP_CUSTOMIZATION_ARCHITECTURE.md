# Ship Customization System - Architecture Documentation

**Project:** Alexander VR Space Flight Game
**Version:** 1.0
**Status:** Phase 1 Complete - Core C++ Backend with N-Body Physics Integration
**Last Updated:** November 7, 2025

---

## Table of Contents

1. [System Overview](#system-overview)
2. [Design Goals](#design-goals)
3. [Component Architecture](#component-architecture)
4. [Data Flow](#data-flow)
5. [Network Replication Strategy](#network-replication-strategy)
6. [Integration Points](#integration-points)
7. [Extension Points](#extension-points)
8. [Performance Considerations](#performance-considerations)

---

## System Overview

The Ship Customization System is a modular, data-driven architecture that enables players to customize their spacecraft with different parts, skins, and physics configurations. The system directly integrates with the game's n-body physics simulation, providing real-time performance modifications that affect both visual appearance and flight characteristics.

### Architecture Principles

- **Data-Driven Design**: All content (parts, skins, physics presets) is defined in Unreal Engine Data Tables and Data Assets
- **Component-Based**: Implemented as a reusable ActorComponent that can be added to any ship
- **Physics-First Integration**: Modifications directly affect mass, thrust, and gravitational calculations in the orbital mechanics system
- **Network-Ready**: Designed for multiplayer with optimized replication and client-side prediction support
- **Blueprint-Friendly**: All major functions exposed to Blueprint for designer iteration

### Technology Stack

- **Unreal Engine 5.6**: Core engine framework
- **C++ Core**: High-performance component logic
- **Data Tables**: Part and skin definitions (FShipPartData, FShipSkinData)
- **Data Assets**: Physics configuration presets (UShipPhysicsConfig)
- **SaveGame System**: Persistent progression storage
- **Network Replication**: Authoritative server model with client prediction

---

## Design Goals

### Primary Goals

1. **Meaningful Customization**
   - Parts must have tangible impact on ship performance
   - Trade-offs create interesting player choices
   - Multiple viable build strategies (racer, tank, balanced, etc.)

2. **Physics Integration**
   - Ship stats directly modify n-body physics calculations
   - Mass affects gravitational forces: F = G * m1 * m2 / r²
   - Thrust power modifies acceleration in Newtonian space
   - Velocity limits prevent unsafe speeds near planets

3. **Progression System**
   - Parts unlock through level progression
   - Credits provide meaningful economic choices
   - Rarity tiers (Common → Legendary) guide power scaling
   - Upgrade chains show clear progression paths

4. **Network Performance**
   - Physics state replicates efficiently
   - Client prediction for immediate feedback
   - Server authority prevents cheating
   - Bandwidth-optimized stat transmission

### Secondary Goals

- Easy content creation for designers (Data Tables)
- Visual customization through material skins
- Loadout presets for quick ship configuration
- Future-proof architecture for advanced features

---

## Component Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        SHIP CUSTOMIZATION SYSTEM                            │
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐ │
│  │              UShipCustomizationComponent (Core)                       │ │
│  │                                                                       │ │
│  │  [Data References]                [Current State]                    │ │
│  │  ┌─────────────────┐              ┌─────────────────┐                │ │
│  │  │ ShipPartsTable  │              │ CurrentLoadout  │                │ │
│  │  │ ShipSkinsTable  │              │ ProgressionData │                │ │
│  │  │ PhysicsConfig   │              │ TotalStats      │                │ │
│  │  └─────────────────┘              └─────────────────┘                │ │
│  │                                                                       │ │
│  │  [Core Functions]                                                    │ │
│  │  - EquipPart(Category, PartID)                                       │ │
│  │  - EquipSkin(SkinID)                                                 │ │
│  │  - CalculateTotalStats() → FShipStats                                │ │
│  │  - ApplyStatsToFlightController()                                    │ │
│  │  - UnlockPart(PartID), UnlockSkin(SkinID)                            │ │
│  │  - SaveLoadout(), LoadLoadout()                                      │ │
│  │  - SaveCustomizationData(), LoadCustomizationData()                  │ │
│  │                                                                       │ │
│  │  [Events/Delegates]                                                  │ │
│  │  - OnLoadoutChanged                                                  │ │
│  │  - OnPartEquipped(Category, PartID)                                  │ │
│  │  - OnSkinEquipped(SkinID)                                            │ │
│  │  - OnPartUnlocked(PartID)                                            │ │
│  │  - OnPlayerLevelUp(OldLevel, NewLevel)                               │ │
│  └───────────────────────────────────────────────────────────────────────┘ │
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐ │
│  │                        DATA LAYER                                     │ │
│  │                                                                       │ │
│  │  [Data Tables - FTableRowBase]          [Data Assets - UDataAsset]   │ │
│  │  ┌─────────────────────────┐            ┌─────────────────────────┐  │ │
│  │  │ FShipPartData           │            │ UShipPhysicsConfig      │  │ │
│  │  │ ├─ PartID (FName)       │            │ ├─ ConfigName          │  │ │
│  │  │ ├─ DisplayName (FText)  │            │ ├─ Mass                │  │ │
│  │  │ ├─ Category (Enum)      │            │ ├─ ThrustPower         │  │ │
│  │  │ ├─ Rarity (Enum)        │            │ ├─ MaxVelocity         │  │ │
│  │  │ ├─ LevelRequirement     │            │ ├─ AngularThrustPower  │  │ │
│  │  │ ├─ UnlockCost           │            │ ├─ FuelCapacity        │  │ │
│  │  │ ├─ StatModifiers        │            │ ├─ AccelerationMult    │  │ │
│  │  │ ├─ PartMesh             │            │ ├─ RotationSpeedMult   │  │ │
│  │  │ └─ Icon                 │            │ └─ StabilityAssist     │  │ │
│  │  └─────────────────────────┘            └─────────────────────────┘  │ │
│  │                                                                       │ │
│  │  ┌─────────────────────────┐                                         │ │
│  │  │ FShipSkinData           │                                         │ │
│  │  │ ├─ SkinID (FName)       │                                         │ │
│  │  │ ├─ DisplayName (FText)  │                                         │ │
│  │  │ ├─ Rarity (Enum)        │                                         │ │
│  │  │ ├─ SkinMaterial         │                                         │ │
│  │  │ ├─ Icon                 │                                         │ │
│  │  │ └─ UnlockCost           │                                         │ │
│  │  └─────────────────────────┘                                         │ │
│  └───────────────────────────────────────────────────────────────────────┘ │
│                                                                             │
│  ┌───────────────────────────────────────────────────────────────────────┐ │
│  │                      RUNTIME STRUCTURES                               │ │
│  │                                                                       │ │
│  │  FShipLoadout                        FPlayerProgressionData           │ │
│  │  ├─ LoadoutName                      ├─ PlayerLevel                  │ │
│  │  ├─ EquippedParts (Map)              ├─ TotalXP                      │ │
│  │  ├─ EquippedSkin                     ├─ Credits                      │ │
│  │  └─ TotalStats (cached)              ├─ UnlockedParts (Array)        │ │
│  │                                      ├─ UnlockedSkins (Array)        │ │
│  │  FShipStats (10 stats)               ├─ SavedLoadouts (Array)        │ │
│  │  ├─ ThrustPower (multiplier)         └─ ActiveLoadoutIndex           │ │
│  │  ├─ MaxVelocity (multiplier)                                         │ │
│  │  ├─ RotationSpeed (multiplier)                                       │ │
│  │  ├─ Acceleration (multiplier)                                        │ │
│  │  ├─ HullIntegrity (additive)                                         │ │
│  │  ├─ ShieldStrength (additive)                                        │ │
│  │  ├─ WeaponDamage (multiplier)                                        │ │
│  │  ├─ EnergyCapacity (additive)                                        │ │
│  │  ├─ EnergyRegenRate (additive)                                       │ │
│  │  └─ Mass (additive)                                                  │ │
│  └───────────────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────────────────┘
```

### Component Hierarchy

```
ASpaceship (AOrbitalBody)
├── UShipCustomizationComponent  [NEW]
├── UFlightController
├── UOrbitalMechanics
├── UStaticMeshComponent (ShipMesh)
├── UCockpitComponent
├── UEngineComponent
└── UShipSystemsManager
```

---

## Data Flow

### Startup Flow

```
┌─────────────────────────────────────────────────────────────────────────────┐
│ 1. COMPONENT INITIALIZATION (BeginPlay)                                    │
└─────────────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
                ┌─────────────────────────┐
                │ LoadCustomizationData() │
                │ (from SaveGame slot)    │
                └─────────────────────────┘
                              │
                              ▼
            ┌──────────────────────────────┐
            │ Has saved data?              │
            └──────────────────────────────┘
                  │                  │
            YES   │                  │  NO
                  │                  │
                  ▼                  ▼
      ┌───────────────────┐  ┌─────────────────────┐
      │ Load progression  │  │ InitializeDefault   │
      │ Load loadout      │  │ Loadout()           │
      └───────────────────┘  └─────────────────────┘
                  │                  │
                  └──────────┬───────┘
                              ▼
                ┌─────────────────────────┐
                │ CalculateTotalStats()   │
                │ (sum all equipped parts)│
                └─────────────────────────┘
                              │
                              ▼
                ┌─────────────────────────────────┐
                │ ApplyStatsToFlightController()  │
                │ (update physics integration)    │
                └─────────────────────────────────┘
                              │
                              ▼
                ┌─────────────────────────┐
                │ UpdateShipVisuals()     │
                │ (apply skin material)   │
                └─────────────────────────┘
```

### Part Equip Flow

```
┌─────────────────────────────────────────────────────────────────────────────┐
│ 2. EQUIPPING A PART (EquipPart)                                            │
└─────────────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
                ┌─────────────────────────┐
                │ GetPartData(PartID)     │
                │ (lookup in data table)  │
                └─────────────────────────┘
                              │
                              ▼
            ┌──────────────────────────────┐
            │ Validation Checks:           │
            │ - Part exists?               │
            │ - Part unlocked?             │
            │ - Meets level requirement?   │
            │ - Category matches?          │
            └──────────────────────────────┘
                  │                  │
            PASS  │                  │  FAIL
                  │                  │
                  │                  ▼
                  │            [Return false]
                  │
                  ▼
      ┌────────────────────────────┐
      │ Add to EquippedParts map   │
      │ CurrentLoadout.EquippedParts│
      │ .Add(Category, PartID)     │
      └────────────────────────────┘
                  │
                  ▼
      ┌────────────────────────────┐
      │ CalculateTotalStats()      │
      │ (recalculate all stats)    │
      └────────────────────────────┘
                  │
                  ▼
      ┌────────────────────────────────┐
      │ ApplyStatsToFlightController() │
      │ (update physics)               │
      └────────────────────────────────┘
                  │
                  ▼
      ┌────────────────────────────┐
      │ UpdateShipVisuals()        │
      │ (if mesh changed)          │
      └────────────────────────────┘
                  │
                  ▼
      ┌────────────────────────────┐
      │ Broadcast Events:          │
      │ - OnPartEquipped           │
      │ - OnLoadoutChanged         │
      └────────────────────────────┘
```

### Stats Calculation Flow

```
┌─────────────────────────────────────────────────────────────────────────────┐
│ 3. CALCULATING TOTAL STATS (CalculateTotalStats)                           │
└─────────────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
                ┌─────────────────────────┐
                │ Initialize base stats:  │
                │ ThrustPower = 1.0       │
                │ MaxVelocity = 1.0       │
                │ RotationSpeed = 1.0     │
                │ Acceleration = 1.0      │
                │ HullIntegrity = 100.0   │
                │ ShieldStrength = 0.0    │
                │ WeaponDamage = 1.0      │
                │ EnergyCapacity = 100.0  │
                │ EnergyRegenRate = 10.0  │
                │ Mass = 1000.0           │
                └─────────────────────────┘
                              │
                              ▼
        ┌─────────────────────────────────────┐
        │ For each equipped part:             │
        │   GetPartData(PartID)               │
        │   TotalStats += Part.StatModifiers  │
        └─────────────────────────────────────┘
                              │
                              ▼
                ┌─────────────────────────┐
                │ Return TotalStats       │
                └─────────────────────────┘

Example:
  Base: ThrustPower = 1.0
  + Engine_Advanced: ThrustPower = +0.5  →  1.5x
  + Thruster_Light: ThrustPower = +0.2   →  1.7x
  = Final ThrustPower = 1.7 (170% of base)

  Base: Mass = 1000 kg
  + Hull_Heavy: Mass = +500 kg           →  1500 kg
  + Shield_Fortress: Mass = +300 kg      →  1800 kg
  = Final Mass = 1800 kg
```

### Physics Integration Flow

```
┌─────────────────────────────────────────────────────────────────────────────┐
│ 4. APPLYING STATS TO PHYSICS (ApplyStatsToFlightController)                │
└─────────────────────────────────────────────────────────────────────────────┘
                              │
                              ▼
        ┌─────────────────────────────────────┐
        │ Get references:                     │
        │ - AOrbitalBody (ship owner)         │
        │ - UFlightController                 │
        └─────────────────────────────────────┘
                              │
                              ▼
            ┌──────────────────────────────┐
            │ Server or standalone?        │
            └──────────────────────────────┘
                  │                  │
            YES   │                  │  NO (Client)
                  │                  │
                  ▼                  ▼
    ┌─────────────────────────┐  ┌───────────────────────────┐
    │ SERVER: Apply directly  │  │ CLIENT: Send RPC to server│
    └─────────────────────────┘  └───────────────────────────┘
                  │                          │
                  │                          ▼
                  │            ┌─────────────────────────────┐
                  │            │ ServerApplyCustomization()  │
                  │            │ (authoritative update)      │
                  │            └─────────────────────────────┘
                  │                          │
                  └──────────────┬───────────┘
                                  ▼
              ┌────────────────────────────────────┐
              │ ORBITAL BODY (N-Body Physics)      │
              │ - OrbitalBody->Mass = Stats.Mass   │
              │ - OrbitalBody->ThrustPower =       │
              │     Stats.ThrustPower * 100000 N   │
              │ - OrbitalBody->StandardGrav...     │
              │     Parameter = G * Mass           │
              └────────────────────────────────────┘
                                  │
                                  ▼
              ┌────────────────────────────────────┐
              │ SPACESHIP (Compatibility)          │
              │ - Spaceship->MaxThrust =           │
              │     Stats.ThrustPower * 100000 N   │
              └────────────────────────────────────┘
                                  │
                                  ▼
              ┌────────────────────────────────────┐
              │ FLIGHT CONTROLLER (Safety)         │
              │ - FlightController->MaxSafeVelo... │
              │     = Stats.MaxVelocity * 1000 m/s │
              └────────────────────────────────────┘
```

---

## Network Replication Strategy

### Replication Model

**Authoritative Server with Client-Side Prediction**

- **Server**: Owns all physics state, validates all customization changes
- **Client**: Predicts local changes, reconciles with server updates
- **Bandwidth**: Optimized through delta compression and selective replication

### What Replicates

```
┌─────────────────────────────────────────────────────────────────────────────┐
│ REPLICATED DATA                                                             │
└─────────────────────────────────────────────────────────────────────────────┘

[UShipCustomizationComponent]
├── CurrentLoadout (FShipLoadout)                    [Replicated]
│   ├─ LoadoutName                                   → Client sees name
│   ├─ EquippedParts (Map<Category, PartID>)         → Client sees parts
│   ├─ EquippedSkin (FName)                          → Client sees skin
│   └─ TotalStats (FShipStats)                       → Client sees stats
│
└── ProgressionData (FPlayerProgressionData)         [NOT Replicated]
    ├─ PlayerLevel                                   → Client-only (SaveGame)
    ├─ Credits                                       → Client-only (SaveGame)
    ├─ UnlockedParts                                 → Client-only (SaveGame)
    └─ UnlockedSkins                                 → Client-only (SaveGame)

[AOrbitalBody] (via inheritance from ASpaceship)
├── Mass                                             [Replicated]
├── ThrustPower                                      [Replicated]
├── Velocity                                         [Replicated]
└── AngularVelocity                                  [Replicated]

[ASpaceship::FSpaceshipNetworkState]
├── ReplicatedMass                                   [Replicated]
├── ReplicatedThrustPower                            [Replicated]
└── ReplicatedMaxVelocity                            [Replicated]
```

### Network Flow

```
┌─────────────────────────────────────────────────────────────────────────────┐
│ CLIENT EQUIPS PART (Multiplayer)                                            │
└─────────────────────────────────────────────────────────────────────────────┘

CLIENT                          SERVER                         ALL CLIENTS
  │                               │                                 │
  │  EquipPart(Engine, "Adv")     │                                 │
  ├──────────────────────────────►│                                 │
  │                               │ Validate request                │
  │                               │ - Part exists?                  │
  │                               │ - Player owns it?               │
  │                               │ - Valid category?               │
  │                               │                                 │
  │                               │ Apply on server                 │
  │                               │ ├─ Update CurrentLoadout        │
  │                               │ ├─ CalculateTotalStats()        │
  │                               │ └─ Update OrbitalBody.Mass      │
  │                               │     OrbitalBody.ThrustPower     │
  │                               │                                 │
  │                               │ Replicate to all clients        │
  │                               ├────────────────────────────────►│
  │◄──────────────────────────────┤                                 │
  │                               │                                 │
  │ OnRep_CurrentLoadout()        │                             OnRep...()
  │ - Apply visual changes        │                             - Apply visual
  │ - Update local physics        │                             - Update physics
  │   (not authoritative)         │                               (not auth)
  │                               │                                 │

NOTES:
- Physics state is AUTHORITATIVE on server
- Clients apply visual changes immediately (for responsiveness)
- Clients sync physics from replicated AOrbitalBody properties
- Progression data (unlocks, credits) is CLIENT-ONLY (SaveGame-based)
```

### RPC Methods

```cpp
// Server RPC: Client requests customization change (with validation)
UFUNCTION(Server, Reliable, WithValidation)
void ASpaceship::ServerApplyCustomization(
    float NewMass,
    float NewThrustPower,
    float NewMaxVelocity
);

// Implementation validates:
// - Client owns this ship
// - Values are within safe ranges
// - No cheating detected
// Then applies to OrbitalBody properties, which auto-replicate
```

### Bandwidth Optimization

- **CurrentLoadout**: ~100 bytes (8 FName PartIDs + 1 FName SkinID + stats)
- **Physics State**: Replicates via existing AOrbitalBody (Mass, ThrustPower already replicated)
- **Update Rate**: CurrentLoadout only replicates on change (not every frame)
- **Compression**: FName uses indexed compression, stats use float quantization

---

## Integration Points

### 1. Integration with AOrbitalBody (N-Body Physics)

**Purpose**: Ship customization directly affects gravitational and orbital mechanics

**File**: `Source/Alexander/Public/OrbitalBody.h`

**Properties Modified**:

```cpp
// MASS: Affects gravitational pull and inertia
// F_gravity = G * m1 * m2 / r²
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
float Mass;  // ← Modified by FShipStats.Mass

// THRUST POWER: Affects acceleration
// a = F / m  →  Higher thrust = faster acceleration
UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Physics")
float ThrustPower;  // ← Modified by FShipStats.ThrustPower * 100000.0f

// STANDARD GRAVITATIONAL PARAMETER: μ = G * Mass
// Used in orbital mechanics calculations
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
float StandardGravitationalParameter;  // ← Recalculated when Mass changes
```

**Integration Code** (from `ShipCustomizationComponent.cpp:254-343`):

```cpp
void UShipCustomizationComponent::ApplyStatsToFlightController()
{
    AOrbitalBody* OrbitalBody = GetOrbitalBody();
    UFlightController* FlightController = GetFlightController();

    if (!OrbitalBody || !FlightController) return;

    FShipStats Stats = CurrentLoadout.TotalStats;

    // SERVER: Apply directly
    if (Owner->HasAuthority())
    {
        // 1. Apply Mass to OrbitalBody
        //    THIS AFFECTS GRAVITY CALCULATIONS!
        //    Heavier ships are pulled MORE by planets
        OrbitalBody->Mass = Stats.Mass;

        // 2. Apply ThrustPower
        //    Convert multiplier to Newtons
        OrbitalBody->ThrustPower = Stats.ThrustPower * 100000.0f;

        // 3. Compatibility with ASpaceship
        if (ASpaceship* Spaceship = Cast<ASpaceship>(OrbitalBody))
        {
            Spaceship->MaxThrust = Stats.ThrustPower * 100000.0f;
        }

        // 4. Recalculate Standard Gravitational Parameter
        //    μ = G * Mass (for orbital mechanics)
        const float G = 6.67430e-11f; // m³/(kg·s²)
        OrbitalBody->StandardGravitationalParameter = G * Stats.Mass;

        // 5. Apply MaxVelocity to FlightController
        FlightController->MaxSafeVelocity = Stats.MaxVelocity * 1000.0f;
    }
    // CLIENT: Request server to apply via RPC
    else
    {
        if (ASpaceship* Spaceship = Cast<ASpaceship>(OrbitalBody))
        {
            Spaceship->ServerApplyCustomization(
                Stats.Mass,
                Stats.ThrustPower * 100000.0f,
                Stats.MaxVelocity * 1000.0f
            );
        }
    }
}
```

**Physics Impact**:

- **Heavier ships** (high Mass):
  - Experience stronger gravitational pull near planets
  - Require more thrust to maintain orbit
  - Have higher inertia (slower to accelerate/decelerate)
  - More stable in atmospheric flight

- **Lighter ships** (low Mass):
  - Experience weaker gravitational pull
  - Can escape gravity wells easier
  - Accelerate/decelerate faster
  - More susceptible to turbulence

---

### 2. Integration with UFlightController (Flight Assistance)

**Purpose**: Ship stats modify flight control limits and safety systems

**File**: `Source/Alexander/Public/FlightController.h`

**Properties Modified**:

```cpp
// Maximum safe velocity (m/s)
// Prevents unsafe speeds near planets/asteroids
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight Assist")
float MaxSafeVelocity = 1000.0f;  // ← Modified by FShipStats.MaxVelocity * 1000.0f
```

**Future Integration Points** (Phase 2):

```cpp
// Rotation speed multiplier (from ship stats)
float RotationSpeedMultiplier;  // ← Will use FShipStats.RotationSpeed

// Acceleration multiplier (from ship stats)
float AccelerationMultiplier;  // ← Will use FShipStats.Acceleration

// Stability assist strength (from physics config)
float StabilityAssistStrength;  // ← Will use PhysicsConfig->StabilityAssistStrength
```

**Usage**:

- FlightController uses `MaxSafeVelocity` to limit thrust when approaching planets
- Higher `MaxVelocity` stat → ship can fly faster safely
- Lower `MaxVelocity` stat → ship auto-limits at lower speed (safer but slower)

---

### 3. Integration with ASpaceship (Compatibility Layer)

**Purpose**: Bridge between customization system and legacy Spaceship class

**File**: `Source/Alexander/Public/Spaceship.h`

**Properties Modified**:

```cpp
// Maximum thrust force (Newtons)
// Legacy property for backward compatibility
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
float MaxThrust = 100000.0f;  // ← Synced with OrbitalBody->ThrustPower
```

**Network RPC**:

```cpp
// Server RPC to apply ship customization (authority)
UFUNCTION(Server, Reliable, WithValidation)
void ServerApplyCustomization(
    float NewMass,
    float NewThrustPower,
    float NewMaxVelocity
);
```

**Network State Replication**:

```cpp
USTRUCT(BlueprintType)
struct FSpaceshipNetworkState
{
    // Ship customization stats (Phase 3 - Network Replication)
    float ReplicatedMass = 1000.0f;
    float ReplicatedThrustPower = 100000.0f;
    float ReplicatedMaxVelocity = 1000.0f;
};
```

**Purpose**: Ensures ship stats replicate correctly in multiplayer

---

### 4. Integration with UStaticMeshComponent (Visual Customization)

**Purpose**: Apply skin materials to ship mesh

**Methods**:

```cpp
void UShipCustomizationComponent::UpdateShipVisuals()
{
    ApplySkinMaterial();
    // TODO: Update part meshes (Phase 2)
}

void UShipCustomizationComponent::ApplySkinMaterial()
{
    UStaticMeshComponent* ShipMesh = GetShipMesh();
    if (!ShipMesh) return;

    // Get equipped skin data
    bool bFound = false;
    FShipSkinData SkinData = GetSkinData(CurrentLoadout.EquippedSkin, bFound);
    if (!bFound || SkinData.SkinMaterial.IsNull()) return;

    // Load and apply material
    UMaterialInstance* Material = SkinData.SkinMaterial.LoadSynchronous();
    if (Material)
    {
        for (int32 i = 0; i < ShipMesh->GetNumMaterials(); i++)
        {
            ShipMesh->SetMaterial(i, Material);
        }
    }
}
```

**Future**: Part meshes will swap based on equipped parts (engines, wings, weapons)

---

### 5. Integration with USaveGame (Persistence)

**Purpose**: Save/load player progression and customization

**Internal Class**:

```cpp
class UShipCustomizationSaveGame : public USaveGame
{
public:
    FPlayerProgressionData ProgressionData;
    FShipLoadout CurrentLoadout;
};
```

**Methods**:

```cpp
bool UShipCustomizationComponent::SaveCustomizationData()
{
    UShipCustomizationSaveGame* SaveGameInstance = CreateSaveGameObject(...);
    SaveGameInstance->ProgressionData = ProgressionData;
    SaveGameInstance->CurrentLoadout = CurrentLoadout;
    return UGameplayStatics::SaveGameToSlot(SaveGameInstance, "ShipCustomization", 0);
}

bool UShipCustomizationComponent::LoadCustomizationData()
{
    UShipCustomizationSaveGame* LoadedGame = LoadGameFromSlot("ShipCustomization", 0);
    if (LoadedGame)
    {
        ProgressionData = LoadedGame->ProgressionData;
        CurrentLoadout = LoadedGame->CurrentLoadout;
        return true;
    }
    return false;
}
```

**Persistence**:
- Saves on: Part equip, part unlock, skin equip, loadout save, XP gain, credits change
- Loads on: Component BeginPlay
- Slot: "ShipCustomization" (index 0)

---

## Extension Points

### 1. Adding New Part Categories

**Current Categories**:
```cpp
enum class EShipPartCategory : uint8
{
    Engine,      // Primary thrust
    Thrusters,   // Rotation/maneuverability
    Hull,        // Durability
    Wings,       // Aerodynamics
    Cockpit,     // Control interface
    Weapon,      // Offensive capability
    Shield,      // Defensive layer
    Utility      // Miscellaneous
};
```

**To Add New Category**:

1. Add to enum in `ShipCustomizationComponent.h`:
   ```cpp
   enum class EShipPartCategory : uint8
   {
       // ... existing categories ...
       Reactor UMETA(DisplayName = "Reactor")  // NEW
   };
   ```

2. Recompile C++ code

3. Create data table with `FShipPartData` rows, set Category to "Reactor"

4. Parts will automatically work with existing system (no code changes needed)

---

### 2. Adding New Ship Stats

**Current Stats** (10 total):
```cpp
struct FShipStats
{
    float ThrustPower;      // Multiplier
    float MaxVelocity;      // Multiplier
    float RotationSpeed;    // Multiplier
    float Acceleration;     // Multiplier
    float HullIntegrity;    // Additive (HP)
    float ShieldStrength;   // Additive
    float WeaponDamage;     // Multiplier
    float EnergyCapacity;   // Additive
    float EnergyRegenRate;  // Additive
    float Mass;             // Additive (kg)
};
```

**To Add New Stat**:

1. Add to `FShipStats` in `ShipCustomizationComponent.h`:
   ```cpp
   UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
   float CloakingEfficiency = 0.0f;  // NEW
   ```

2. Update `operator+` in `FShipStats`:
   ```cpp
   Result.CloakingEfficiency = CloakingEfficiency + Other.CloakingEfficiency;
   ```

3. Update `CalculateTotalStats()` base values:
   ```cpp
   TotalStats.CloakingEfficiency = 0.0f;  // Base value
   ```

4. Recompile C++ code

5. Data tables will automatically show new stat field in editor

6. Update `ApplyStatsToFlightController()` to use new stat:
   ```cpp
   if (ACloakingSystem* Cloak = GetOwner()->FindComponentByClass<ACloakingSystem>())
   {
       Cloak->CloakEfficiency = Stats.CloakingEfficiency;
   }
   ```

---

### 3. Adding Synergy Bonuses

**Pattern**: Detect equipped part combinations and apply bonuses

**Example Implementation**:

```cpp
FShipStats UShipCustomizationComponent::CalculateTotalStats()
{
    FShipStats TotalStats = /* ... base calculation ... */;

    // SYNERGY BONUS: "Combat Set"
    // If all combat-themed parts equipped, +10% weapon damage
    bool bHasCombatEngine = false;
    bool bHasCombatWeapon = false;
    bool bHasCombatShield = false;

    for (const TPair<EShipPartCategory, FName>& Pair : CurrentLoadout.EquippedParts)
    {
        FString PartName = Pair.Value.ToString();
        if (PartName.StartsWith("Combat_"))
        {
            if (Pair.Key == EShipPartCategory::Engine) bHasCombatEngine = true;
            if (Pair.Key == EShipPartCategory::Weapon) bHasCombatWeapon = true;
            if (Pair.Key == EShipPartCategory::Shield) bHasCombatShield = true;
        }
    }

    if (bHasCombatEngine && bHasCombatWeapon && bHasCombatShield)
    {
        TotalStats.WeaponDamage *= 1.1f;  // +10% bonus
        UE_LOG(LogTemp, Log, TEXT("Combat Set synergy activated! +10% weapon damage"));
    }

    return TotalStats;
}
```

**Naming Convention**:
- Prefix parts with set name: `Combat_Engine_Advanced`, `Combat_Weapon_Laser`, `Combat_Shield_Heavy`
- Detect prefix in code
- Apply bonus when full set equipped

---

### 4. Adding Part Upgrade System

**Pattern**: Parts can be upgraded in-place (level up)

**Data Structure** (already exists):

```cpp
struct FShipPartData
{
    // Parent part (for upgrade tree)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    FName ParentPartID;

    // Can this part be upgraded?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    bool bCanBeUpgraded = true;

    // Upgrade level (0 = base, 1-5 = upgrades)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    int32 UpgradeLevel = 0;
};
```

**Example Implementation**:

```cpp
UFUNCTION(BlueprintCallable, Category = "Progression")
bool UShipCustomizationComponent::UpgradePart(FName PartID)
{
    bool bFound = false;
    FShipPartData PartData = GetPartData(PartID, bFound);
    if (!bFound || !PartData.bCanBeUpgraded) return false;

    // Find next upgrade level part
    FName UpgradedPartID = FName(*FString::Printf(TEXT("%s_Lvl%d"),
        *PartData.PartID.ToString(), PartData.UpgradeLevel + 1));

    // Check if upgrade exists and is unlocked
    FShipPartData UpgradedPart = GetPartData(UpgradedPartID, bFound);
    if (!bFound) return false;

    if (!IsPartUnlocked(UpgradedPartID))
    {
        // Try to unlock (costs credits)
        if (!UnlockPart(UpgradedPartID)) return false;
    }

    // Equip upgraded part
    return EquipPart(PartData.Category, UpgradedPartID);
}
```

**Data Table Setup**:
```
Engine_Basic      (UpgradeLevel=0, bCanBeUpgraded=true)
Engine_Basic_Lvl1 (UpgradeLevel=1, ParentPartID="Engine_Basic", bCanBeUpgraded=true)
Engine_Basic_Lvl2 (UpgradeLevel=2, ParentPartID="Engine_Basic_Lvl1", bCanBeUpgraded=false)
```

---

### 5. Adding Procedural Part Generation

**Pattern**: Generate parts at runtime based on algorithms

**Example Implementation**:

```cpp
UFUNCTION(BlueprintCallable, Category = "Procedural")
FShipPartData UShipCustomizationComponent::GenerateRandomPart(
    EShipPartCategory Category,
    EShipPartRarity Rarity
)
{
    FShipPartData NewPart;
    NewPart.PartID = FName(*FString::Printf(TEXT("Procedural_%s_%d"),
        *UEnum::GetValueAsString(Category), FMath::Rand()));
    NewPart.DisplayName = FText::FromString("Mysterious Engine");
    NewPart.Category = Category;
    NewPart.Rarity = Rarity;

    // Generate random stats based on rarity
    float PowerMultiplier = 1.0f + (static_cast<int>(Rarity) * 0.2f);
    NewPart.StatModifiers.ThrustPower = FMath::FRandRange(0.8f, 1.2f) * PowerMultiplier;
    NewPart.StatModifiers.MaxVelocity = FMath::FRandRange(0.9f, 1.1f) * PowerMultiplier;
    NewPart.StatModifiers.Mass = FMath::FRandRange(800.0f, 1200.0f);

    return NewPart;
}
```

**Use Cases**:
- Loot drops from enemies
- Reward from completing missions
- Crafting system output
- Marketplace randomized inventory

---

### 6. Adding Loadout Comparison UI

**Pattern**: Compare two loadouts side-by-side

**Example Implementation**:

```cpp
UFUNCTION(BlueprintCallable, Category = "Loadout")
void UShipCustomizationComponent::CompareLoadouts(
    const FShipLoadout& LoadoutA,
    const FShipLoadout& LoadoutB,
    FShipStats& OutStatsA,
    FShipStats& OutStatsB,
    FShipStats& OutDifference
)
{
    // Calculate stats for each loadout
    FShipLoadout OriginalLoadout = CurrentLoadout;

    CurrentLoadout = LoadoutA;
    OutStatsA = CalculateTotalStats();

    CurrentLoadout = LoadoutB;
    OutStatsB = CalculateTotalStats();

    // Calculate difference
    OutDifference.ThrustPower = OutStatsB.ThrustPower - OutStatsA.ThrustPower;
    OutDifference.MaxVelocity = OutStatsB.MaxVelocity - OutStatsA.MaxVelocity;
    // ... etc for all stats ...

    // Restore original loadout
    CurrentLoadout = OriginalLoadout;
}
```

**UI Display**:
```
Loadout A          Loadout B          Difference
---------          ---------          ----------
Thrust: 1.5x       Thrust: 1.8x       +0.3x (↑)
Speed:  1.2x       Speed:  1.0x       -0.2x (↓)
Mass:   1500 kg    Mass:   1200 kg    -300 kg (↓)
```

---

## Performance Considerations

### 1. Data Table Lookup Performance

**Concern**: Frequent lookups in data tables (O(n) search)

**Optimization**:

```cpp
// CURRENT: Linear search through data table rows
FShipPartData* PartData = ShipPartsDataTable->FindRow<FShipPartData>(PartID, TEXT(""));

// OPTIMIZED: Cache frequently accessed parts
TMap<FName, FShipPartData> CachedParts;

void UShipCustomizationComponent::CachePartData()
{
    if (!ShipPartsDataTable) return;

    TArray<FName> RowNames = ShipPartsDataTable->GetRowNames();
    for (const FName& RowName : RowNames)
    {
        FShipPartData* PartData = ShipPartsDataTable->FindRow<FShipPartData>(RowName, TEXT(""));
        if (PartData)
        {
            CachedParts.Add(RowName, *PartData);
        }
    }
}

FShipPartData UShipCustomizationComponent::GetPartData(FName PartID, bool& bFound)
{
    if (CachedParts.Contains(PartID))
    {
        bFound = true;
        return CachedParts[PartID];
    }
    bFound = false;
    return FShipPartData();
}
```

**Impact**: O(n) → O(1) lookup, ~10x faster for large part catalogs

---

### 2. Stat Recalculation Frequency

**Concern**: `CalculateTotalStats()` called every time a part changes

**Current Behavior**:
- EquipPart() → CalculateTotalStats()
- UnequipPart() → CalculateTotalStats()
- LoadLoadout() → CalculateTotalStats()

**Optimization**: Cache stats, only recalculate on dirty flag

```cpp
private:
    bool bStatsAreDirty = true;
    FShipStats CachedTotalStats;

public:
    FShipStats UShipCustomizationComponent::GetTotalStats()
    {
        if (bStatsAreDirty)
        {
            CachedTotalStats = CalculateTotalStats();
            bStatsAreDirty = false;
        }
        return CachedTotalStats;
    }

    void MarkStatsDirty()
    {
        bStatsAreDirty = true;
    }
```

**Impact**: Prevents redundant calculations, ~5x faster for rapid part swaps

---

### 3. Network Bandwidth Usage

**Concern**: Replicating CurrentLoadout every frame is wasteful

**Current Behavior**:
- `CurrentLoadout` is marked `Replicated`
- Unreal replicates on change only (good!)
- But initial sync sends full struct (~100 bytes)

**Optimization**: Delta compression for stat values

```cpp
USTRUCT()
struct FCompressedShipStats
{
    // Quantize floats to 16-bit (0.01 precision)
    uint16 ThrustPower_Compressed;  // Multiplier * 100 → 0-655 range
    uint16 MaxVelocity_Compressed;
    // ... etc ...

    FShipStats Decompress() const
    {
        FShipStats Stats;
        Stats.ThrustPower = ThrustPower_Compressed / 100.0f;
        Stats.MaxVelocity = MaxVelocity_Compressed / 100.0f;
        return Stats;
    }
};
```

**Impact**: Reduces network load from ~100 bytes → ~20 bytes per update

---

### 4. SaveGame Write Frequency

**Concern**: Saving to disk on every part equip causes hitching

**Current Behavior**:
- EquipPart() → SaveCustomizationData() (disk write)
- UnlockPart() → SaveCustomizationData() (disk write)
- AddXP() → SaveCustomizationData() (disk write)

**Optimization**: Async save with dirty flag

```cpp
private:
    bool bNeedsSave = false;
    float TimeSinceLastSave = 0.0f;

public:
    void TickComponent(float DeltaTime, ...)
    {
        TimeSinceLastSave += DeltaTime;

        // Auto-save every 30 seconds if dirty
        if (bNeedsSave && TimeSinceLastSave >= 30.0f)
        {
            AsyncSaveCustomizationData();
            bNeedsSave = false;
            TimeSinceLastSave = 0.0f;
        }
    }

    void MarkDirtyForSave()
    {
        bNeedsSave = true;
    }

    void AsyncSaveCustomizationData()
    {
        // Use Async task to prevent hitching
        AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
        {
            SaveCustomizationData();
        });
    }
```

**Impact**: Eliminates disk write hitches, saves only when needed

---

### 5. Material Loading Performance

**Concern**: `LoadSynchronous()` blocks main thread when applying skins

**Current Behavior**:

```cpp
UMaterialInstance* Material = SkinData.SkinMaterial.LoadSynchronous();  // BLOCKS!
```

**Optimization**: Async loading with callback

```cpp
void UShipCustomizationComponent::ApplySkinMaterialAsync()
{
    if (CurrentLoadout.EquippedSkin.IsNone()) return;

    bool bFound = false;
    FShipSkinData SkinData = GetSkinData(CurrentLoadout.EquippedSkin, bFound);
    if (!bFound || SkinData.SkinMaterial.IsNull()) return;

    // Async load
    FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
    Streamable.RequestAsyncLoad(
        SkinData.SkinMaterial.ToSoftObjectPath(),
        FStreamableDelegate::CreateUObject(this, &UShipCustomizationComponent::OnSkinMaterialLoaded)
    );
}

void UShipCustomizationComponent::OnSkinMaterialLoaded()
{
    // Material is now loaded, apply to mesh
    UStaticMeshComponent* ShipMesh = GetShipMesh();
    if (!ShipMesh) return;

    bool bFound = false;
    FShipSkinData SkinData = GetSkinData(CurrentLoadout.EquippedSkin, bFound);
    UMaterialInstance* Material = Cast<UMaterialInstance>(SkinData.SkinMaterial.Get());

    if (Material)
    {
        for (int32 i = 0; i < ShipMesh->GetNumMaterials(); i++)
        {
            ShipMesh->SetMaterial(i, Material);
        }
    }
}
```

**Impact**: Prevents hitches, loads materials in background

---

## Summary

The Ship Customization System is a robust, data-driven architecture that seamlessly integrates with the game's n-body physics simulation. Key strengths:

- **Modular Design**: Component-based, can be added to any ship
- **Physics Integration**: Direct impact on orbital mechanics through Mass, Thrust, and Velocity
- **Network-Ready**: Authoritative server with client prediction
- **Designer-Friendly**: All content in Data Tables/Assets
- **Extensible**: Clear extension points for future features

**Current Status**: Phase 1 Complete (Core C++ Backend)

**Next Steps**: See SHIP_CUSTOMIZATION_ROADMAP.md for future phases

---

**Document Status**: COMPLETE
**Review Status**: READY FOR TECHNICAL REVIEW
**Recommended For**: Programmers, Technical Designers, System Architects
