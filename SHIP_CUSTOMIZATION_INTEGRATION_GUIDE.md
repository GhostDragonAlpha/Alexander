# Ship Customization System Integration with N-Body Physics

**Date:** 2025-11-07
**Status:** Integration Architecture & Documentation
**Version:** 1.0

---

## Overview

The Ship Customization System integrates with the unified n-body physics engine to create a complete gameplay loop where player equipment choices directly affect orbital mechanics, gravitational interactions, and flight performance. This document explains how ship stats flow through the system and integrate with physics calculations.

### Core Philosophy

Ship customization stats are not merely cosmetic or combat-focused—they are **physics-first parameters** that affect:

- **Gravitational Interactions:** Ship mass affects both the gravitational force exerted on other objects and inertia
- **Flight Dynamics:** Thrust power translates directly to acceleration using F = ma
- **Orbital Mechanics:** Combined mass and thrust determine orbital velocity and escape velocity
- **Network Replication:** All stat changes must replicate to other players to maintain consistent simulation

---

## Key Integration Points

### 1. Mass → Gravitational Force Calculation

**The Equation:**
```
F = G * m1 * m2 / r²
```

**Integration Flow:**

```
UShipCustomizationComponent
    └─> FShipStats.Mass (default: 1000 kg)
        └─> AOrbitalBody component
            └─> GravitySimulator
                └─> F = G * (m1 * m2) / r²
                    └─> Physical force applied to all nearby bodies
```

**Code Path:**
1. **Stat Definition:** `Source/Alexander/Public/ShipCustomizationComponent.h:85` - `FShipStats.Mass`
2. **Customization Update:** `Source/Alexander/Private/ShipCustomizationComponent.cpp` - `ApplyStatsToFlightController()`
3. **Physics Integration:** `Source/Alexander/CelestialScaling/Public/GravitySimulator.h:59` - `CalculateTotalGravitationalForce()`
4. **Applied Force:** `Source/Alexander/CelestialScaling/Public/GravitySimulator.h:99` - `ApplyGravityToActor()`

**Physics Details:**
- **Gravitational Constant (G):** 6.674e-11 m³/(kg·s²)
- **Default Ship Mass:** 1000 kg (about 1 ton—realistic for a small spacecraft)
- **Mass Range:** 500 kg (fighter) to 10,000+ kg (capital ship)
- **Effect:** A ship with 2x mass experiences 2x gravitational force from other bodies AND exerts 2x gravity on other objects

**Important Warnings:**
- WARNING: Increasing mass also increases **inertia** (resistance to acceleration)
  - Formula: `Acceleration = ThrustForce / Mass`
  - A 2x heavier ship needs 2x thrust to achieve same acceleration
- WARNING: In multiplayer, mass changes must replicate to all clients or the simulation will desynchronize
  - Use `OnRep_CurrentLoadout()` to propagate mass changes to GravitySimulator

---

### 2. ThrustPower → Acceleration

**The Equation:**
```
Acceleration = ThrustForce / Mass
ThrustForce = ThrustPower * BaseThrust
```

**Integration Flow:**

```
UShipCustomizationComponent
    └─> FShipStats.ThrustPower (multiplier: 0.5 to 2.0)
        └─> UFlightController
            └─> Acceleration = ThrustForce / Mass
                └─> Velocity += Acceleration * DeltaTime
                    └─> Position += Velocity * DeltaTime
```

**Code Path:**
1. **Stat Definition:** `Source/Alexander/Public/ShipCustomizationComponent.h:48` - `FShipStats.ThrustPower`
2. **Applied to FlightController:** `Source/Alexander/Private/ShipCustomizationComponent.cpp` - `ApplyStatsToFlightController()`
3. **Flight Controller Integration:** `Source/Alexander/Public/FlightController.h` - Uses ThrustPower multiplier
4. **Physics Update:** `Source/Alexander/Public/FlightController.h` - `UpdateFlightPhysics()` applies thrust as acceleration

**Physics Details:**
- **Base Thrust Force:** 50,000 N (configurable)
- **ThrustPower Stat:** Multiplier that scales thrust (0.5 = half thrust, 2.0 = double thrust)
- **Example Calculation:**
  - Stock ship: Mass=1000 kg, ThrustPower=1.0
    - ThrustForce = 50,000 * 1.0 = 50,000 N
    - Acceleration = 50,000 / 1000 = 50 m/s² (5G)
  - Upgraded engine: Mass=1200 kg, ThrustPower=1.5
    - ThrustForce = 50,000 * 1.5 = 75,000 N
    - Acceleration = 75,000 / 1200 = 62.5 m/s² (6.4G)

**Important Notes:**
- Changes to ThrustPower stat immediately affect flight responsiveness
- Must be updated in FlightController when customization changes
- In origin-centered physics, thrust is applied relative to ship's local frame, not world frame

---

### 3. MaxVelocity → Flight Controller Safety Limit

**The Equation:**
```
ActualMaxVelocity = BaseMaxVelocity * MaxVelocity
Velocity = Clamp(Velocity, -ActualMaxVelocity, ActualMaxVelocity)
```

**Integration Flow:**

```
UShipCustomizationComponent
    └─> FShipStats.MaxVelocity (multiplier: 0.8 to 1.5)
        └─> UFlightController
            └─> MaxVelocityClamp in UpdateFlightPhysics()
                └─> Prevents excessive speeds
```

**Code Path:**
1. **Stat Definition:** `Source/Alexander/Public/ShipCustomizationComponent.h:52` - `FShipStats.MaxVelocity`
2. **Applied to FlightController:** `Source/Alexander/Private/ShipCustomizationComponent.cpp` - `ApplyStatsToFlightController()`
3. **Velocity Clamping:** `Source/Alexander/Private/FlightController.cpp` - UpdateFlightPhysics() enforces limit

**Physics Details:**
- **Base Maximum Velocity:** 10,000 m/s (10 km/s - about 7% speed of light)
- **MaxVelocity Stat:** Multiplier to adjust top speed
- **Example:**
  - Stock engines: BaseMaxVel=10,000 m/s, MaxVelocity=1.0 → 10,000 m/s max
  - High-performance engines: BaseMaxVel=10,000 m/s, MaxVelocity=1.3 → 13,000 m/s max

**Important Notes:**
- This is a hard limit—velocity is clamped every frame
- Affects escape velocity calculations (escape_v = sqrt(2 * G * M / r))
- Critical for preventing numerical issues in physics simulation
- In multiplayer, all clients must apply the same limit to prevent desyncs

---

### 4. Mass Changes → Orbital Mechanics Impact

**The Equation:**
```
Orbital Velocity = sqrt(G * M / r)
Escape Velocity = sqrt(2 * G * M / r)
where M includes all massive objects (including the ship itself for multi-body calculations)
```

**Integration Scenario:**

A player equips heavier armor (increases mass from 1000 kg to 1200 kg):

1. **Local Effect:** Ship becomes harder to accelerate (5% less responsive)
   - Old accel: 50 m/s²
   - New accel: 41.67 m/s²

2. **Gravitational Effect:** Ship now exerts slightly more gravity
   - Other ships experience 20% increased gravitational pull
   - All bodies' orbits shift by ~1-2% depending on distance

3. **Multi-Body Effect:** System's total gravitational interactions change
   - Conservation of momentum still holds
   - Total system energy conserves (with validation checks)

4. **Network Effect:** Must notify all connected clients
   - RPC: `ServerApplyCustomization()` informs server of change
   - Server validates and broadcasts via `ReplicatedNetworkState`
   - All clients update their GravitySimulator

**Critical Code Paths:**

```cpp
// 1. Customization change occurs
void UShipCustomizationComponent::EquipPart(EShipPartCategory Category, FName PartID)
{
    // ... equip logic ...
    ApplyStatsToFlightController();  // Local update

    if (GetOwnerRole() == ROLE_Authority)
    {
        ServerApplyCustomization();   // Send to server
    }
}

// 2. Server receives and validates
void ASpaceship::ServerApplyCustomization_Implementation(const FShipLoadout& NewLoadout)
{
    CustomizationComponent->CurrentLoadout = NewLoadout;

    // Update physics
    float NewMass = NewLoadout.TotalStats.Mass;
    if (OrbitalBody)
    {
        OrbitalBody->SetMass(NewMass);  // Updates gravity calculations
    }

    // Notify all clients
    ReplicatedNetworkState.TotalMass = NewMass;
    OnRep_NetworkState();  // Triggers update on all clients
}

// 3. All clients receive update
void ASpaceship::OnRep_NetworkState()
{
    // Update local copy
    CurrentLoadout.TotalStats.Mass = ReplicatedNetworkState.TotalMass;

    // Update gravity simulator
    if (GravitySimulator)
    {
        GravitySimulator->UpdateBodyMass(this, NewMass);
    }
}
```

---

## Component Architecture

### Hierarchy Diagram

```
ASpaceship (Player Ship)
├── UShipCustomizationComponent
│   ├── FShipStats (current stats from equipped parts)
│   ├── FShipLoadout (current equipped items)
│   ├── FPlayerProgressionData (level, XP, unlocks)
│   └── Events: OnLoadoutChanged, OnPartEquipped, etc.
│
├── AOrbitalBody (Optional - for physics)
│   ├── Mass (synced from FShipStats.Mass)
│   ├── Position (in floating-origin space)
│   └── Velocity (from FlightController)
│
├── UFlightController
│   ├── ThrustPower (from FShipStats.ThrustPower)
│   ├── MaxVelocity (from FShipStats.MaxVelocity)
│   ├── Acceleration (from FShipStats.Acceleration)
│   └── UpdateFlightPhysics() [called every frame]
│
└── Network Replication
    ├── ReplicatedNetworkState
    │   ├── VirtualPosition (sector + local offset)
    │   ├── Velocity, Rotation
    │   ├── ActiveScaleFactor
    │   └── TotalMass (from customization)
    │
    └── RPCs:
        ├── ServerApplyCustomization() [client → server]
        └── ClientReceiveCustomization() [server → all clients]
```

### Data Flow Diagram

```
Player Equipment Change
│
├─→ UShipCustomizationComponent::EquipPart()
│   │
│   ├─→ CurrentLoadout updated
│   ├─→ CalculateTotalStats() [aggregates all equipped parts]
│   ├─→ ApplyStatsToFlightController() [local update]
│   │
│   └─→ if (IsLocallyControlled)
│       └─→ ServerApplyCustomization_Implementation()
│           │
│           ├─→ Server validates equipment
│           ├─→ AOrbitalBody::SetMass(NewMass)
│           │   └─→ Notifies GravitySimulator
│           │
│           ├─→ UGravitySimulator::UpdateBodyMass()
│           │   └─→ Recalculates all gravity interactions
│           │
│           └─→ Broadcasts via ReplicatedNetworkState
│               └─→ OnRep_NetworkState() on all clients
│                   └─→ Clients update their physics caches
```

---

## Network Replication

### Ship Customization Replication Strategy

**Network Authority Model:**
- **Server Authority:** Server owns all ship equipment and validates changes
- **Client Prediction:** Clients predict equipment changes locally for UI responsiveness
- **Authority Override:** Server's decision is final—clients always reconcile to server state

**RPC Methods:**

#### ServerApplyCustomization (Client → Server, Reliable)
```cpp
/**
 * Client requests customization change
 * Server validates and broadcasts to all clients
 */
UFUNCTION(Server, Reliable, WithValidation)
void ServerApplyCustomization(const FShipLoadout& NewLoadout);

// Validation checks:
// 1. Player has required level
// 2. Player has unlocked all parts
// 3. All stats within allowed ranges
// 4. Total mass <= MaxMass constraint
// 5. Total power consumption <= MaxPower constraint
```

#### ClientReceiveCustomization (Server → All Clients, Reliable)
```cpp
/**
 * Server broadcasts customization change to all clients
 * Ensures consistent simulation state across network
 */
UFUNCTION(Client, Reliable)
void ClientReceiveCustomization(const FShipLoadout& UpdatedLoadout);

// Updates made on client:
// 1. CurrentLoadout replicated property updated
// 2. GravitySimulator notified of mass change
// 3. FlightController stats updated
// 4. Visual meshes updated (parts, colors, etc.)
```

### Replication Structures

**FShipLoadout (already replicated)**
```cpp
USTRUCT(BlueprintType)
struct FShipLoadout
{
    // Loaded parts map
    TMap<EShipPartCategory, FName> EquippedParts;

    // Calculated stats (updated every change)
    FShipStats TotalStats;

    // Mark dirty to trigger replication
    // Mark as UPROPERTY(Replicated) in component
};
```

**Network State Extensions (in FSpaceshipNetworkState)**
```cpp
struct FSpaceshipNetworkState
{
    // ... existing position, rotation, velocity ...

    // CUSTOMIZATION ADDITIONS:
    float TotalMass;                    // 4 bytes
    float ThrustPowerMultiplier;        // 4 bytes
    float MaxVelocityMultiplier;        // 4 bytes
    FLinearColor CurrentPaintScheme;    // 16 bytes

    // Total: ~28 additional bytes per update
};
```

**Network Optimization:**
- Only changed stats are sent (delta compression)
- Cosmetic changes (paint, decals) sent at lower frequency (5 Hz)
- Gameplay stats (mass, thrust) sent at update frequency (20 Hz)
- Estimated bandwidth: ~30 bytes additional per ship update

---

## Data Asset System

### Ship Part Data Tables

**Location:** `Content/GameData/ShipParts_DT` (or similar)

**Structure:** FShipPartData (see ShipCustomizationComponent.h:109)

**Example Entry - "Reinforced Hull":**
```
PartID: Hull_Reinforced_01
DisplayName: "Reinforced Hull"
Category: Hull
Rarity: Epic
LevelRequirement: 15
UnlockCost: 5000 credits

StatModifiers:
  - Mass: +200 kg
  - HullIntegrity: +50 HP
  - (No thrust/velocity change)

PartMesh: SM_Hull_Reinforced
```

**Example Entry - "Thruster Upgrade":**
```
PartID: Engine_TurboII_02
DisplayName: "Turbo II Engine"
Category: Engine
Rarity: Rare
LevelRequirement: 10
UnlockCost: 3000 credits

StatModifiers:
  - ThrustPower: +0.3 (30% boost)
  - Mass: +150 kg
  - EnergyConsumption: +20
  - MaxVelocity: +0.1
```

### Create Ship Presets

**Blueprint Usage Example:**

```blueprint
// Create a "Fighter" preset
LET SpaceshipActor = Player's Spaceship

// Create Fighter Loadout
Create Loadout:
  - Engine: TurboII (ThrustPower +0.3)
  - Hull: Standard (no modifiers)
  - Thrusters: Enhanced (Acceleration +0.2)
  - Result Stats:
    - Mass: 1000 + 0 + 50 = 1050 kg
    - ThrustPower: 1.0 + 0.3 + 0.05 = 1.35
    - MaxVelocity: 1.0 (no bonuses)
    - Acceleration: 1.0 + 0.2 = 1.2

// Apply to ship
SpaceshipActor.GetCustomizationComponent().LoadLoadout(FighterLoadout)

// Result Physics:
// - Acceleration: ThrustForce / Mass = (50000 * 1.35) / 1050 = 64.3 m/s² (6.6G)
// - More responsive due to better thrust-to-mass ratio
// - Good for dogfighting, worse for long-range travel
```

---

## Code Examples

### C++ Usage Example

```cpp
void AMySpaceship::SetupCustomization()
{
    // Get customization component
    UShipCustomizationComponent* CustomizationComp =
        FindComponentByClass<UShipCustomizationComponent>();

    if (!CustomizationComp) return;

    // Listen for loadout changes
    CustomizationComp->OnLoadoutChanged.AddDynamic(this, &AMySpaceship::OnLoadoutChanged);

    // Load a saved preset
    CustomizationComp->LoadLoadout(0);  // Load first saved loadout
}

void AMySpaceship::OnLoadoutChanged()
{
    // Called whenever equipment changes

    // Get new stats
    FShipStats NewStats = GetCustomizationComponent()->CalculateTotalStats();

    // Update flight physics
    UFlightController* FlightController =
        FindComponentByClass<UFlightController>();

    if (FlightController)
    {
        FlightController->SetThrustMultiplier(NewStats.ThrustPower);
        FlightController->SetMaxVelocity(NewStats.MaxVelocity * 10000.0f);
    }

    // Update orbital body mass (if using n-body physics)
    if (OrbitalBody)
    {
        OrbitalBody->SetMass(NewStats.Mass);

        // Notify gravity system of mass change
        if (UGameInstance* GameInstance = GetGameInstance())
        {
            if (UGravitySimulator* GravSim =
                GameInstance->GetSubsystem<UGravitySimulator>())
            {
                GravSim->UpdateBodyMass(OrbitalBody, NewStats.Mass);
            }
        }
    }

    // Update visuals
    CustomizationComp->UpdateShipVisuals();

    // Network: Notify server of change (if not already server)
    if (GetLocalRole() != ROLE_Authority)
    {
        ServerApplyCustomization(NewStats);
    }
}

UFUNCTION(Server, Reliable)
void AMySpaceship::ServerApplyCustomization(const FShipStats& NewStats)
{
    // Server validates the change
    if (!ValidateCustomizationChange(NewStats))
    {
        // Revert client
        ClientRejectCustomization(CurrentLoadout);
        return;
    }

    // Apply server-side
    CurrentLoadout.TotalStats = NewStats;

    // Broadcast to all clients
    MulticastApplyCustomization(NewStats);
}

UFUNCTION(NetMulticast, Reliable)
void AMySpaceship::MulticastApplyCustomization(const FShipStats& NewStats)
{
    // All clients update their copy
    CurrentLoadout.TotalStats = NewStats;

    if (OrbitalBody)
    {
        OrbitalBody->SetMass(NewStats.Mass);
    }

    UpdateShipVisuals();
}
```

### Blueprint Usage Example

```
Event: Player Presses "Apply Loadout" Button
│
├─→ Get Spaceship (Owner)
├─→ Get Customization Component
│
├─→ Set Current Loadout = Selected Loadout
│   └─→ Triggers Event: On Loadout Changed
│
├─→ Get Total Stats from Customization Component
│
├─→ Apply Stats to Flight Controller
│   ├─→ Set Thrust Multiplier = TotalStats.ThrustPower
│   ├─→ Set Max Velocity = TotalStats.MaxVelocity * 10000
│   └─→ Set Acceleration = TotalStats.Acceleration
│
├─→ Update Ship Visuals
│   ├─→ Apply equipped part meshes
│   └─→ Apply paint scheme colors
│
└─→ Server: Apply Customization
    ├─→ Server validates equipped parts
    ├─→ Server updates own state
    └─→ Multicast: Update all clients
```

---

## Testing Guide

### Test 1: Mass Effect on Gravity

**Objective:** Verify that mass changes correctly affect gravitational forces

**Setup:**
1. Create test map with 3 ships (A, B, C)
2. Ship A: 1000 kg, stationary
3. Ship B: 1000 kg, 500m away
4. Ship C: 2000 kg, 500m away (same distance as B)

**Test Steps:**
1. Run simulation 30 seconds
2. Measure Ship A's velocity after 30s due to gravity
3. Unequip Ship C's heavy armor (reduce to 1000 kg)
4. Reset positions
5. Run simulation 30 seconds again
6. Compare velocities: Should be 2x less for the second run

**Expected Result:**
- First run: Velocity toward C ≈ 2 * (velocity toward B)
- Second run: Velocity toward C ≈ velocity toward B
- Ratio: ~2:1 (due to mass change from 2000 to 1000)

**Verification Code:**
```cpp
void ATestShipGravity::VerifyMassEffect()
{
    // Calculate expected force ratio
    float Mass_A = 1000.0f;
    float Mass_B = 1000.0f;
    float Mass_C_Before = 2000.0f;
    float Mass_C_After = 1000.0f;
    float Distance = 50000.0f;  // 500m in cm

    // F = G * m1 * m2 / r²
    float Force_AC_Before = (6.674e-11 * Mass_A * Mass_C_Before) / (Distance * Distance);
    float Force_AB = (6.674e-11 * Mass_A * Mass_B) / (Distance * Distance);
    float Force_AC_After = (6.674e-11 * Mass_A * Mass_C_After) / (Distance * Distance);

    // Ratio should be 2:1
    float Ratio_Before = Force_AC_Before / Force_AB;  // Should be ~2.0
    float Ratio_After = Force_AC_After / Force_AB;    // Should be ~1.0

    UE_LOG(LogTemp, Warning, TEXT("Force Ratio Before: %f (expected ~2.0)"), Ratio_Before);
    UE_LOG(LogTemp, Warning, TEXT("Force Ratio After: %f (expected ~1.0)"), Ratio_After);
}
```

### Test 2: Thrust-to-Mass Ratio Effect

**Objective:** Verify that acceleration changes with thrust and mass

**Setup:**
1. Create single test ship
2. Measure acceleration with stock equipment
3. Equip high-thrust engine (ThrustPower: 1.0 → 1.3)
4. Measure new acceleration
5. Equip heavy armor (Mass: 1000 → 1200)
6. Measure acceleration again

**Expected Results:**
- Base: Accel = 50,000 N / 1000 kg = 50 m/s²
- After thrust upgrade: Accel = 65,000 N / 1000 kg = 65 m/s² (30% increase)
- After armor: Accel = 65,000 N / 1200 kg = 54.17 m/s² (decrease from adding mass)
- Net effect: High thrust engine = fast, heavy ship = slow

**Test Code:**
```cpp
void ATestShipPhysics::VerifyThrustMassRatio()
{
    ASpaceship* TestShip = GetTestShip();
    UFlightController* FlightCtrl = TestShip->FindComponentByClass<UFlightController>();

    // Test 1: Baseline
    float BaseAccel = FlightCtrl->GetMaxAcceleration();
    UE_LOG(LogTemp, Warning, TEXT("Base Acceleration: %f m/s²"), BaseAccel);

    // Test 2: After thrust upgrade
    EquipPart(TestShip, "Engine_TurboII");
    float UpgradedAccel = FlightCtrl->GetMaxAcceleration();
    UE_LOG(LogTemp, Warning, TEXT("After thrust upgrade: %f m/s² (%.1f%% increase)"),
        UpgradedAccel, (UpgradedAccel / BaseAccel - 1.0f) * 100.0f);

    // Test 3: After adding armor
    EquipPart(TestShip, "Hull_Reinforced");
    float WithArmorAccel = FlightCtrl->GetMaxAcceleration();
    UE_LOG(LogTemp, Warning, TEXT("After armor: %f m/s² (%.1f%% of upgraded)"),
        WithArmorAccel, (WithArmorAccel / UpgradedAccel) * 100.0f);
}
```

### Test 3: Network Replication

**Objective:** Verify that customization changes replicate correctly in multiplayer

**Setup:**
1. Create server + 2 clients
2. Player 1 changes equipment
3. Observe changes on Player 1, Player 2, and Server

**Test Steps:**
1. Player 1 equips heavy armor (+200 kg)
2. Wait 1 second for replication
3. Query Player 2's view of Player 1's mass
4. Query Server's view of Player 1's mass
5. All three should match

**Expected Result:**
- Server mass = 1200 kg
- Player 1's local mass = 1200 kg
- Player 2's replicated mass = 1200 kg
- Tolerance: ±0.01 kg

**Test Code:**
```cpp
void ATestNetworkCustomization::VerifyReplication()
{
    // Get players
    ASpaceship* ServerShip = GetServerShip(0);
    ASpaceship* ClientShip = GetClientShip(0);

    // Initial mass
    float InitialMass = ServerShip->GetTotalMass();

    // Client equips armor
    ClientShip->GetCustomizationComponent()->EquipPart(EShipPartCategory::Hull, "Hull_Reinforced");

    // Wait for replication (1 frame should be enough with Reliable RPC)
    GetWorld()->GetTimerManager().SetTimerForNextTick([this, ServerShip, ClientShip]()
    {
        float ServerMass = ServerShip->GetTotalMass();
        float ClientMass = ClientShip->GetTotalMass();

        float ExpectedMass = InitialMass + 200.0f;  // Armor adds 200 kg

        UE_LOG(LogTemp, Warning, TEXT("Server Mass: %f"), ServerMass);
        UE_LOG(LogTemp, Warning, TEXT("Client Mass: %f"), ClientMass);
        UE_LOG(LogTemp, Warning, TEXT("Expected: %f"), ExpectedMass);

        if (FMath::IsNearlyEqual(ServerMass, ExpectedMass, 0.01f) &&
            FMath::IsNearlyEqual(ClientMass, ExpectedMass, 0.01f))
        {
            UE_LOG(LogTemp, Warning, TEXT("PASS: Replication successful"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("FAIL: Replication mismatch"));
        }
    });
}
```

### Test 4: Orbital Mechanics

**Objective:** Verify that orbital velocity calculations are correct with customized ships

**Setup:**
1. Create two ships: Orbiter and Massive Body
2. Calculate required orbital velocity
3. Set Orbiter's velocity tangent to Massive Body
4. Let simulation run

**Expected Result:**
- Orbiter maintains stable circular orbit (or elliptical, depending on setup)
- Orbital period = 2π * sqrt(r³ / (G*M))
- Energy conserved to within <1%

**Test Code:**
```cpp
void ATestOrbitalMechanics::VerifyOrbitalStability()
{
    // Setup
    ASpaceship* Orbiter = SpawnOrbiter();
    ASpaceship* MassiveBody = SpawnMassiveBody(10000.0f);  // 10,000 kg

    float OrbitalRadius = 50000.0f;  // 500 km in cm
    float MassKg = 10000.0f;

    // Calculate orbital velocity: v = sqrt(G*M/r)
    float G = 6.674e-11;
    float OrbitalVelocity = sqrt(G * MassKg / OrbitalRadius);

    // Set Orbiter tangent to body
    Orbiter->SetActorLocation(MassiveBody->GetActorLocation() + FVector(OrbitalRadius, 0, 0));
    Orbiter->SetVelocity(FVector(0, OrbitalVelocity, 0));

    // Run simulation 1000 frames
    for (int i = 0; i < 1000; i++)
    {
        GetWorld()->Tick(LEVELTICK_All, 0.016f);  // 60 FPS
    }

    // Check orbital stability
    float FinalDistance = FVector::Dist(Orbiter->GetActorLocation(), MassiveBody->GetActorLocation());
    float DistanceError = FMath::Abs(FinalDistance - OrbitalRadius) / OrbitalRadius;

    UE_LOG(LogTemp, Warning, TEXT("Orbital Radius Error: %.2f%%"), DistanceError * 100.0f);

    if (DistanceError < 0.01f)  // Less than 1% error
    {
        UE_LOG(LogTemp, Warning, TEXT("PASS: Orbit is stable"));
    }
}
```

### Test 5: Performance Scaling

**Objective:** Verify that system performance remains acceptable with heavy customization

**Setup:**
1. Create 100 ships, each with different customizations
2. Vary mass from 500 kg to 5000 kg
3. Measure physics simulation time
4. Measure network bandwidth

**Expected Result:**
- Physics time: <16ms per frame (for 60 FPS)
- Network bandwidth: <100 kbps total for 100 ships
- Gravity calculations: Still accurate despite high count

**Test Code:**
```cpp
void ATestPerformanceScaling::MeasurePhysicsPerformance()
{
    // Spawn 100 ships with varied customizations
    for (int i = 0; i < 100; i++)
    {
        ASpaceship* Ship = SpawnShip();

        // Randomize mass
        float RandomMass = FMath::RandRange(500.0f, 5000.0f);
        Ship->GetCustomizationComponent()->ApplyMassModifier(RandomMass - 1000.0f);
    }

    // Measure gravity calculation time
    double StartTime = FPlatformTime::Seconds();

    GravitySimulator->UpdateAllForces();

    double EndTime = FPlatformTime::Seconds();
    double ElapsedMS = (EndTime - StartTime) * 1000.0;

    UE_LOG(LogTemp, Warning, TEXT("Gravity calculation time: %.2f ms"), ElapsedMS);

    if (ElapsedMS < 16.67)  // 60 FPS target
    {
        UE_LOG(LogTemp, Warning, TEXT("PASS: Within frame budget"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("FAIL: Exceeds frame budget"));
    }
}
```

---

## Performance Considerations

### CPU Impact

**Per-Ship Customization:**
- **Stat Calculation:** ~0.1ms (occurs on equipment change, not every frame)
- **Physics Update:** ~0.3ms (mass affects gravity calculations)
- **Network Replication:** ~0.2ms (sending/receiving updates)

**Total: ~0.6ms per ship per frame** (negligible impact)

### Network Impact

**Bandwidth per customization update:**
- Equipment change RPC: 200 bytes
- Mass/thrust replication: 20 bytes
- Frequency: Only when customization changes (typically 1-2 times per session)

**Estimated:** <10 bytes/sec per player average (minimal)

### Memory Impact

**Per-Ship:**
- `FShipLoadout`: ~2 KB (24 equipped parts × 50 bytes + overhead)
- `FShipStats`: ~100 bytes
- Part cache: ~5 KB (if caching part data)

**Total: ~10 KB per ship** (negligible)

---

## Integration Checklist

- [ ] Ship Customization Component created and added to Spaceship
- [ ] Data Tables created for Ship Parts and Skins
- [ ] Customization stats read by FlightController
- [ ] Mass changes update OrbitalBody and GravitySimulator
- [ ] Network replication working (ServerApplyCustomization RPC)
- [ ] All clients receive stat updates via ReplicatedNetworkState
- [ ] Physics validation confirms conservation laws still hold
- [ ] Performance monitoring shows acceptable impact (<1ms per frame)
- [ ] Test suite passes (mass effect, thrust scaling, replication, orbits)
- [ ] Documentation complete and reviewed

---

## Warnings & Gotchas

### WARNING 1: Mass Inertia Coupling
When increasing mass, don't forget to also increase thrust if you want to maintain acceleration:
- **Problem:** Add 500 kg armor without upgrading engine → Ship becomes sluggish
- **Solution:** Always pair mass upgrades with thrust upgrades
- **Testing:** Monitor Acceleration = ThrustForce / Mass ratio

### WARNING 2: Network Desynchronization
If server and client have different mass values, gravity calculations will diverge:
- **Problem:** Client predicts old mass, server uses new mass → positions drift
- **Solution:** Always send ServerApplyCustomization before applying locally
- **Testing:** Verify all 3 machines (server, client1, client2) have matching mass

### WARNING 3: Extreme Mass Values
Physics breaks down at extreme scales:
- **Min Mass:** 100 kg (too light → excessive acceleration, numerical errors)
- **Max Mass:** 50,000 kg (too heavy → very slow, unrealistic)
- **Solution:** Clamp customization stats to realistic ranges

### WARNING 4: Stat Application Timing
FlightController might not be ready when customization applies:
- **Problem:** ApplyStatsToFlightController() called before FlightController exists
- **Solution:** Use GetFlightController() helper that initializes if needed
- **Testing:** Add null checks and log warnings

### WARNING 5: Orbital Decay
Slight precision errors accumulate over long simulations:
- **Problem:** After 1 hour of play, orbits drift significantly
- **Solution:** Run validation checks periodically (see UNIFIED_GRAVITY_IMPLEMENTATION_SUMMARY.md)
- **Testing:** Run orbit test for extended time period, track energy conservation

---

## Related Documentation

- **[UNIFIED_GRAVITY_IMPLEMENTATION_SUMMARY.md](./UNIFIED_GRAVITY_IMPLEMENTATION_SUMMARY.md)** - Core n-body physics system
- **[AGENT4_SPACESHIP_NETWORK_DOCUMENTATION.md](./AGENT4_SPACESHIP_NETWORK_DOCUMENTATION.md)** - Network architecture and replication
- **[Source/Alexander/Public/ShipCustomizationComponent.h](./Source/Alexander/Public/ShipCustomizationComponent.h)** - Component API
- **[Source/Alexander/CelestialScaling/Public/GravitySimulator.h](./Source/Alexander/CelestialScaling/Public/GravitySimulator.h)** - Gravity calculations

---

## Summary

The Ship Customization System creates a physics-first gameplay loop where equipment directly affects orbital mechanics. Every stat change—especially mass—has real physics consequences that propagate through the simulation and across the network. By understanding the integration points and following the testing guide, you can safely implement ship customization while maintaining simulation accuracy and network consistency.

**Key Takeaway:** Mass is not just a number—it affects gravity, inertia, and the entire multi-body simulation. Changes must be validated, replicated, and tested.

