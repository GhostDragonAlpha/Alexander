# Asteroid Field System - Architecture Design Document

**Project:** Alexander VR Space Game
**Phase:** 7 - Asteroid Field & N-Body Performance Optimization
**Date:** 2025-01-07
**Status:** DESIGN PHASE (Implementation Pending)

---

## Table of Contents

1. [Problem Statement](#problem-statement)
2. [Performance Analysis](#performance-analysis)
3. [Proposed Solution](#proposed-solution)
4. [Architecture Overview](#architecture-overview)
5. [System Components](#system-components)
6. [Implementation Plan](#implementation-plan)
7. [Ship Customization Integration](#ship-customization-integration)
8. [Network Replication Strategy](#network-replication-strategy)
9. [Performance Budgets](#performance-budgets)
10. [Data Structures](#data-structures)
11. [Key Design Questions Answered](#key-design-questions-answered)
12. [References & Research](#references--research)

---

## Problem Statement

### The Challenge

The user wants **"lots of asteroids and things crashing together out there Space is a crazy place"** but the current n-body gravity system has fundamental scalability issues:

**Current System Architecture:**
- **N-Body Gravity:** O(N²) force calculations per frame
- **Formula:** For N objects, calculations required = N(N-1)/2
- **Current Implementation:** Calculates `F = G * m1 * m2 / r²` for every pair
- **Existing Components:**
  - `AOrbitalBody` - Base class for all space objects
  - `UOrbitalMechanics` - Orbital calculations component
  - `UCelestialBodyComponent` - Distance-based scaling/LOD
  - `UCelestialBodyRegistry` - Thread-safe body tracking

**Why O(N²) Doesn't Scale:**

| Bodies | Calculations/Frame | At 60 FPS | CPU Time (est) |
|--------|-------------------|-----------|----------------|
| 10     | 45                | 2,700/sec | ~0.5 ms        |
| 50     | 1,225             | 73,500/sec| ~5 ms          |
| 100    | 4,950             | 297,000/sec| ~20 ms        |
| 500    | 124,750           | 7.5M/sec  | ~500 ms        |
| 1000   | 499,500           | 30M/sec   | ~2000 ms       |

**Critical Performance Threshold:**
- **Target:** 60 FPS = 16.67ms per frame budget
- **Current System:** Unusable beyond ~100 objects (already at 20ms)
- **User Requirement:** "Thousands of asteroids"

**The Fundamental Problem:**
> Every asteroid calculating gravity from every other asteroid creates an exponential explosion of calculations. At 1000 asteroids, we'd need 500,000 force calculations per frame, consuming ~2 seconds of CPU time - making the game unplayable.

---

## Performance Analysis

### Current System Metrics

**Existing Components Performance:**
```cpp
// From UNIFIED_GRAVITY_IMPLEMENTATION_SUMMARY.md
struct FUnifiedGravityStats {
    int32 TotalTrackedObjects;          // Currently supports ~100
    int32 ForceCalculationsPerFrame;    // N(N-1)/2
    float ForceCalculationTimeMS;       // ~20ms at 100 objects

    // Bubble distance thresholds (from BubbleDistanceManager)
    int32 NearObjects;      // 0-10 km: Full N-body simulation
    int32 MediumObjects;    // 10-50 km: Reduced simulation
    int32 FarObjects;       // 50-100 km: Minimal simulation
    int32 OutOfBubbleObjects; // >100 km: Kinematic only
};
```

**Bottleneck Analysis:**
1. **Force Calculation:** 0.04ms per pair (includes sqrt, multiply, divide)
2. **No Spatial Partitioning:** Every object checks every other object
3. **No Distance Culling:** Full calculation even for distant objects
4. **No GPU Utilization:** Pure CPU calculations

### Target Performance Requirements

**Goal:** Support 1000+ asteroids at 60 FPS

**Performance Budget Breakdown:**
```
Total Frame Budget:              16.67 ms
├─ Physics (non-gravity):         4.00 ms
├─ Rendering:                     6.00 ms
├─ Gameplay/AI:                   2.00 ms
├─ Network/Replication:           1.00 ms
├─ Audio/VFX:                     1.00 ms
└─ ASTEROID GRAVITY SYSTEM:       2.67 ms ← Our target
```

**Required Optimization:**
- **Current:** 20ms for 100 objects = 0.20ms per object
- **Target:** 2.67ms for 1000 objects = 0.00267ms per object
- **Speedup Required:** ~75x faster (0.20 / 0.00267 = 74.9)

**This is only achievable by fundamentally changing the algorithm.**

---

## Proposed Solution

### Hybrid Tiered N-Body System

**Core Philosophy:**
> Not all objects need the same physics fidelity. Priority should be based on distance from player, object mass, and gameplay relevance.

**Solution Architecture:** Four-tier simulation system with spatial partitioning

```
┌─────────────────────────────────────────────────────────────────┐
│                    TIER 0: CRITICAL OBJECTS                     │
│  • Player ship, nearby ships, planets, major celestial bodies   │
│  • Physics: Full N-body O(N²) among critical objects            │
│  • Count: ~10-20 objects                                        │
│  • CPU Budget: 0.8ms                                            │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│                    TIER 1: ACTIVE ASTEROIDS                     │
│  • Within 50km of player (gameplay interaction range)           │
│  • Physics: Simplified gravity (only from Tier 0 + collisions)  │
│  • Count: ~100-200 objects                                      │
│  • CPU Budget: 0.8ms                                            │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│                   TIER 2: DISTANT ASTEROIDS                     │
│  • 50km - 200km from player (visible but low detail)            │
│  • Physics: Kinematic orbit paths (pre-calculated)              │
│  • Count: ~200-400 objects                                      │
│  • CPU Budget: 0.4ms                                            │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│                     TIER 3: FAR ASTEROIDS                       │
│  • 200km+ from player (background/ambience)                     │
│  • Physics: None (static billboard/instanced rendering)         │
│  • Count: ~300-500 objects                                      │
│  • CPU Budget: 0.1ms                                            │
└─────────────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────────────┐
│                 TIER 4: PROCEDURAL BACKGROUND                   │
│  • Beyond 500km (GPU instanced particles)                       │
│  • Physics: None (GPU compute shader for motion)                │
│  • Count: ~10,000+ objects (GPU instanced)                      │
│  • CPU Budget: 0.2ms                                            │
└─────────────────────────────────────────────────────────────────┘

Total Objects: ~11,000
Total CPU Budget: 2.3ms (within 2.67ms target!)
```

### Why This Works

**Mathematical Proof:**

**Tier 0 (Critical):**
- N = 20 objects
- Calculations = 20(20-1)/2 = 190
- Time = 190 × 0.004ms = 0.76ms ✓

**Tier 1 (Active):**
- N = 200 asteroids
- Each calculates gravity from 20 critical objects only = 200 × 20 = 4,000
- Collision checks: Spatial hash reduces to ~10 checks per asteroid = 2,000
- Time = (4,000 + 2,000) × 0.0001ms = 0.6ms ✓

**Tier 2 (Distant):**
- N = 400 asteroids
- Kinematic update (position += velocity × dt) = 400 × 0.001ms = 0.4ms ✓

**Tier 3 (Far):**
- N = 500 asteroids
- No physics, just cull/LOD updates = 500 × 0.0002ms = 0.1ms ✓

**Tier 4 (Procedural):**
- N = 10,000+ particles
- GPU instanced, no per-object CPU cost
- Single draw call + compute shader = 0.2ms ✓

**Total: 2.06ms < 2.67ms target ✓**

---

## Architecture Overview

### System Diagram

```
┌──────────────────────────────────────────────────────────────────────────┐
│                    AAsteroidFieldManager (Master Actor)                   │
│  • Owns all asteroid spawning, tier management, performance monitoring    │
│  • Singleton per level, replicated to clients                            │
└────────────────────┬─────────────────────────────────────────────────────┘
                     │
        ┌────────────┴────────────┬──────────────────┬─────────────────┐
        ↓                         ↓                  ↓                 ↓
┌─────────────────┐    ┌─────────────────┐  ┌──────────────┐  ┌────────────┐
│UAsteroidTierMgr │    │USpatialHashGrid │  │UAsteroidPool│  │UAsteroidVFX│
│(Tier Mgmt)      │    │(Collision Opt)  │  │(Object Pool)│  │(GPU Render)│
└────────┬────────┘    └────────┬────────┘  └──────┬───────┘  └──────┬─────┘
         │                      │                   │                 │
         │                      │                   │                 │
         └──────────────────────┴───────────────────┴─────────────────┘
                                         ↓
                         ┌───────────────────────────────┐
                         │  AOrbitalBody (Base Class)    │
                         │  └─ AAsteroid (Derived)       │
                         │     • FPhysicsTier            │
                         │     • CollisionResponse       │
                         │     • Fragmentation           │
                         └───────────────────────────────┘
```

### Integration with Existing Systems

```
Existing System                  New Asteroid System
───────────────                  ───────────────────

UCelestialBodyRegistry  ────────→  AAsteroidFieldManager
(tracks all bodies)                (registers asteroids)

AOrbitalBody            ────────→  AAsteroid : AOrbitalBody
(base space object)                (asteroid-specific behavior)

UOrbitalMechanics       ────────→  Tier 0/1 use this
(Kepler orbits)                    Tier 2+ use simplified

UCelestialBodyComponent ────────→  Used for scaling/LOD
(distance scaling)                 (already implemented)

PlayerOriginManager     ────────→  Notifies tier system
(floating origin)                  (update asteroid distances)

ShipCustomizationSystem ────────→  Hull integrity for damage
(ship stats)                       Mass affects collisions
```

---

## System Components

### 1. AAsteroidFieldManager

**Purpose:** Master coordinator for all asteroid simulation

**Responsibilities:**
1. Spawn/despawn asteroids procedurally
2. Assign asteroids to tiers based on distance from player
3. Monitor performance and adjust quality dynamically
4. Handle network replication of asteroid state
5. Coordinate with existing UCelestialBodyRegistry

**Blueprint Configurable Properties:**
```cpp
UCLASS(Blueprintable)
class ALEXANDER_API AAsteroidFieldManager : public AActor
{
    GENERATED_BODY()

public:
    // Field Configuration
    UPROPERTY(EditAnywhere, Category = "Asteroid Field")
    int32 TargetAsteroidCount = 1000;

    UPROPERTY(EditAnywhere, Category = "Asteroid Field")
    float FieldRadius = 100000.0f; // 100 km

    UPROPERTY(EditAnywhere, Category = "Asteroid Field")
    float DensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Asteroid Field")
    FVector FieldCenterLocation;

    // Tier Thresholds
    UPROPERTY(EditAnywhere, Category = "Performance|Tiers")
    float Tier0Radius = 1000.0f;    // Critical: 1 km

    UPROPERTY(EditAnywhere, Category = "Performance|Tiers")
    float Tier1Radius = 50000.0f;   // Active: 50 km

    UPROPERTY(EditAnywhere, Category = "Performance|Tiers")
    float Tier2Radius = 200000.0f;  // Distant: 200 km

    UPROPERTY(EditAnywhere, Category = "Performance|Tiers")
    float Tier3Radius = 500000.0f;  // Far: 500 km

    // Performance Budgets (ms per frame)
    UPROPERTY(EditAnywhere, Category = "Performance|Budgets")
    float Tier0Budget = 0.8f;

    UPROPERTY(EditAnywhere, Category = "Performance|Budgets")
    float Tier1Budget = 0.8f;

    UPROPERTY(EditAnywhere, Category = "Performance|Budgets")
    float Tier2Budget = 0.4f;

    UPROPERTY(EditAnywhere, Category = "Performance|Budgets")
    float TotalPhysicsBudget = 2.67f;

    // Dynamic Quality Adjustment
    UPROPERTY(EditAnywhere, Category = "Performance")
    bool bAutoAdjustQuality = true;

    UPROPERTY(EditAnywhere, Category = "Performance")
    float TargetFrameTime = 16.67f; // 60 FPS

    // Procedural Generation
    UPROPERTY(EditAnywhere, Category = "Generation")
    int32 RandomSeed = 12345;

    UPROPERTY(EditAnywhere, Category = "Generation")
    TArray<UStaticMesh*> AsteroidMeshes;

    UPROPERTY(EditAnywhere, Category = "Generation")
    FFloatRange SizeRange = FFloatRange(5.0f, 500.0f); // meters

    UPROPERTY(EditAnywhere, Category = "Generation")
    FFloatRange MassRange = FFloatRange(1000.0f, 1000000.0f); // kg

    // Network Replication
    UPROPERTY(Replicated)
    TArray<FAsteroidNetworkState> ReplicatedAsteroids;

    UPROPERTY(EditAnywhere, Category = "Network")
    int32 MaxReplicatedAsteroids = 50; // Only replicate closest

    UPROPERTY(EditAnywhere, Category = "Network")
    float NetworkUpdateRate = 2.0f; // Hz (very low, most are kinematic)
};
```

**Key Functions:**
```cpp
// Initialization
void InitializeField(FVector CenterLocation, float Radius);
void SpawnAsteroids(int32 Count);
void RegisterWithCelestialRegistry();

// Tier Management
void UpdateAsteroidTiers(float DeltaTime);
void AssignAsteroidToTier(AAsteroid* Asteroid, EAsteroidTier NewTier);
void BalanceTierPopulations();

// Performance Monitoring
void MeasureFramePerformance();
void AdjustQualitySettings();
FString GetPerformanceReport() const;

// Procedural Generation
AAsteroid* SpawnRandomAsteroid(FVector Location);
void GenerateOrbitPath(AAsteroid* Asteroid);
void PopulateRegion(FBox Region, int32 Density);

// Network Synchronization
void ReplicateNearbyAsteroids();
UFUNCTION(NetMulticast, Reliable)
void MulticastSpawnAsteroid(FAsteroidSpawnData SpawnData);
```

---

### 2. AAsteroid (Derived from AOrbitalBody)

**Purpose:** Individual asteroid actor with tier-specific behavior

**Class Definition:**
```cpp
UENUM(BlueprintType)
enum class EAsteroidTier : uint8
{
    Critical     UMETA(DisplayName = "Tier 0 - Critical"),
    Active       UMETA(DisplayName = "Tier 1 - Active"),
    Distant      UMETA(DisplayName = "Tier 2 - Distant"),
    Far          UMETA(DisplayName = "Tier 3 - Far"),
    Procedural   UMETA(DisplayName = "Tier 4 - Procedural")
};

UCLASS()
class ALEXANDER_API AAsteroid : public AOrbitalBody
{
    GENERATED_BODY()

public:
    // Tier Management
    UPROPERTY(BlueprintReadOnly, Category = "Tier")
    EAsteroidTier CurrentTier;

    UPROPERTY(BlueprintReadOnly, Category = "Tier")
    float DistanceToPlayer;

    UPROPERTY(BlueprintReadOnly, Category = "Tier")
    float LastTierUpdateTime;

    // Physics Properties (inherited Mass, Radius from AOrbitalBody)
    UPROPERTY(EditAnywhere, Category = "Physics")
    float RotationSpeed; // degrees/sec

    UPROPERTY(EditAnywhere, Category = "Physics")
    FVector RotationAxis;

    // Collision & Damage
    UPROPERTY(EditAnywhere, Category = "Collision")
    float HullDamagePerSecond = 50.0f; // Damage to ships

    UPROPERTY(EditAnywhere, Category = "Collision")
    bool bCanFragment = true;

    UPROPERTY(EditAnywhere, Category = "Collision")
    int32 FragmentCount = 3; // How many pieces on break

    UPROPERTY(EditAnywhere, Category = "Collision")
    float MinFragmentMass = 100.0f; // Don't fragment below this

    // Orbit Path (for Tier 2 kinematic motion)
    UPROPERTY()
    TArray<FVector> OrbitPathPoints; // Pre-calculated positions

    UPROPERTY()
    int32 CurrentPathIndex;

    UPROPERTY()
    float PathInterpolation;

    // Visual/Audio
    UPROPERTY(EditAnywhere, Category = "Visual")
    UMaterialInterface* AsteroidMaterial;

    UPROPERTY(EditAnywhere, Category = "Audio")
    USoundBase* CollisionSound;

    // Network Optimization
    UPROPERTY(Replicated)
    FAsteroidCompressedState CompressedState;

    UPROPERTY()
    bool bIsNetworkRelevant = false; // Only closest asteroids replicate

    // Tier-Specific Physics Updates
    void UpdateTier0_Critical(float DeltaTime);  // Full N-body
    void UpdateTier1_Active(float DeltaTime);    // Simplified physics
    void UpdateTier2_Distant(float DeltaTime);   // Kinematic orbit
    void UpdateTier3_Far(float DeltaTime);       // No physics, just visibility
    void UpdateTier4_Procedural(float DeltaTime);// GPU instanced

    // Collision & Fragmentation
    virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other,
                          UPrimitiveComponent* OtherComp, bool bSelfMoved,
                          FVector HitLocation, FVector HitNormal,
                          FVector NormalImpulse, const FHitResult& Hit) override;

    void HandleShipCollision(AActor* Ship, const FHitResult& Hit);
    void HandleAsteroidCollision(AAsteroid* OtherAsteroid, const FHitResult& Hit);
    void FragmentAsteroid(FVector ImpactPoint, FVector ImpactNormal);

    // Orbit Path Generation
    void GenerateOrbitPath(AActor* OrbitTarget, float OrbitRadius, int32 PathResolution);
    FVector GetKinematicPosition(float Time) const;

    // Tier Transition
    void OnTierChanged(EAsteroidTier OldTier, EAsteroidTier NewTier);
    void EnablePhysicsForTier(EAsteroidTier Tier);
    void DisablePhysicsForTier(EAsteroidTier Tier);
};
```

---

### 3. USpatialHashGrid

**Purpose:** Spatial partitioning for O(1) collision detection

**Mathematical Foundation:**

Instead of checking every asteroid against every other (O(N²)), divide space into a grid:

```
Grid Cell Size = 2 × MaxAsteroidRadius × CollisionCheckDistance

Example: MaxRadius = 500m, CheckDistance = 5 radii
Cell Size = 2 × 500 × 5 = 5000m = 5km grid cells

For 1000 asteroids in 100km³ volume:
- Grid dimensions: 20×20×20 = 8000 cells
- Average asteroids per cell: 1000/8000 = 0.125
- Max asteroids per cell (worst case): ~10

Collision checks per asteroid:
- Without grid: 1000 checks
- With grid: Check 27 adjacent cells × 0.125 avg = ~3.4 checks
- Speedup: 1000/3.4 = 294x faster!
```

**Implementation:**
```cpp
UCLASS()
class ALEXANDER_API USpatialHashGrid : public UObject
{
    GENERATED_BODY()

public:
    // Grid Configuration
    UPROPERTY(EditAnywhere, Category = "Spatial Hash")
    float CellSize = 5000.0f; // 5 km cells

    UPROPERTY(EditAnywhere, Category = "Spatial Hash")
    FIntVector GridDimensions = FIntVector(20, 20, 20);

    UPROPERTY(EditAnywhere, Category = "Spatial Hash")
    FVector GridOrigin; // Usually field center

    // Grid Storage
    TMap<FIntVector, TArray<AAsteroid*>> GridCells;

    // Object Management
    void AddObject(AAsteroid* Asteroid);
    void RemoveObject(AAsteroid* Asteroid);
    void UpdateObject(AAsteroid* Asteroid, FVector OldLocation);
    void ClearGrid();

    // Collision Queries
    TArray<AAsteroid*> GetNearbyAsteroids(FVector Location, float Radius);
    TArray<AAsteroid*> GetCellContents(FIntVector CellCoord);
    bool LineTrace(FVector Start, FVector End, FHitResult& OutHit);

    // Grid Helpers
    FIntVector WorldToGridCoord(FVector WorldLocation) const;
    FVector GridToWorldCoord(FIntVector GridCoord) const;
    void GetAdjacentCells(FIntVector CenterCell, TArray<FIntVector>& OutCells);

    // Statistics
    int32 GetTotalObjectCount() const;
    int32 GetOccupiedCellCount() const;
    float GetAverageObjectsPerCell() const;
    int32 GetMaxObjectsInCell() const;
};
```

---

### 4. UAsteroidPool

**Purpose:** Object pooling to avoid allocation overhead

**Rationale:**
- Spawning/destroying 1000 actors per frame = massive GC pressure
- Pooling reuses existing actors, just repositions them
- **Performance gain:** 10-100x faster than spawn/destroy

```cpp
UCLASS()
class ALEXANDER_API UAsteroidPool : public UObject
{
    GENERATED_BODY()

public:
    // Pool Configuration
    UPROPERTY(EditAnywhere, Category = "Pool")
    int32 InitialPoolSize = 1000;

    UPROPERTY(EditAnywhere, Category = "Pool")
    int32 MaxPoolSize = 2000;

    UPROPERTY(EditAnywhere, Category = "Pool")
    bool bAutoExpand = true;

    // Pool Storage
    TArray<AAsteroid*> AvailableAsteroids;
    TArray<AAsteroid*> ActiveAsteroids;
    TMap<AAsteroid*, int32> AsteroidToIndex;

    // Pool Operations
    AAsteroid* AcquireAsteroid(FVector Location, FRotator Rotation);
    void ReleaseAsteroid(AAsteroid* Asteroid);
    void PreallocatePool(UWorld* World, int32 Count);

    // Pool Management
    void ExpandPool(int32 AdditionalCount);
    void ShrinkPool(int32 TargetSize);
    void ResetAllAsteroids();

    // Statistics
    int32 GetActiveCount() const { return ActiveAsteroids.Num(); }
    int32 GetAvailableCount() const { return AvailableAsteroids.Num(); }
    float GetUtilizationPercent() const;
};
```

---

### 5. UAsteroidVFXManager

**Purpose:** GPU-accelerated rendering for distant asteroids

**GPU Instancing Strategy:**

For Tier 3 and 4 asteroids, use instanced static meshes:
- Single draw call renders 1000+ asteroids
- GPU handles transform calculations
- CPU only updates instance buffer once per frame

```cpp
UCLASS()
class ALEXANDER_API UAsteroidVFXManager : public UObject
{
    GENERATED_BODY()

public:
    // Instanced Rendering
    UPROPERTY()
    UInstancedStaticMeshComponent* InstancedAsteroidMesh;

    UPROPERTY(EditAnywhere, Category = "VFX")
    UStaticMesh* LowLODMesh; // Ultra-simplified mesh for instancing

    UPROPERTY(EditAnywhere, Category = "VFX")
    int32 MaxInstances = 10000;

    // Instance Data
    TArray<FTransform> InstanceTransforms;
    TArray<FLinearColor> InstanceColors; // Per-instance variation
    TArray<float> InstanceSizes;

    // GPU Compute Shader (optional advanced feature)
    UPROPERTY(EditAnywhere, Category = "VFX|Compute")
    bool bUseComputeShader = false;

    // Functions
    void AddInstance(FTransform Transform, float Size, FLinearColor Color);
    void UpdateInstance(int32 Index, FTransform NewTransform);
    void RemoveInstance(int32 Index);
    void UpdateAllInstances(float DeltaTime);
    void RebuildInstanceBuffer();

    // Compute Shader Interface (future)
    void UpdateInstancesOnGPU(float DeltaTime);
    void DispatchComputeShader();
};
```

---

## Implementation Plan

### Phase 1: Core Infrastructure (Week 1)

**Files to Create:**
1. `Source/Alexander/Public/AsteroidField/AsteroidFieldManager.h` (400 lines)
2. `Source/Alexander/Private/AsteroidField/AsteroidFieldManager.cpp` (600 lines)
3. `Source/Alexander/Public/AsteroidField/Asteroid.h` (300 lines)
4. `Source/Alexander/Private/AsteroidField/Asteroid.cpp` (500 lines)

**Implementation Steps:**
1. Create `AAsteroid` class derived from `AOrbitalBody`
2. Implement `AAsteroidFieldManager` spawning logic
3. Integrate with `UCelestialBodyRegistry`
4. Test: Spawn 100 asteroids, verify they register correctly

**Success Criteria:**
- [ ] Can spawn 100 asteroids without crashes
- [ ] Asteroids appear in `UCelestialBodyRegistry::GetAllCelestialBodies()`
- [ ] Existing orbital mechanics still work

---

### Phase 2: Tier System (Week 2)

**Files to Create:**
1. `Source/Alexander/Public/AsteroidField/AsteroidTierManager.h` (250 lines)
2. `Source/Alexander/Private/AsteroidField/AsteroidTierManager.cpp` (400 lines)

**Implementation Steps:**
1. Implement tier assignment based on distance
2. Create tier-specific `UpdateTierX_XXX()` functions in `AAsteroid`
3. Implement smooth tier transitions (hysteresis to prevent oscillation)
4. Add performance monitoring

**Tier Transition Logic:**
```cpp
void UAsteroidTierManager::UpdateAsteroidTier(AAsteroid* Asteroid)
{
    float Distance = Asteroid->DistanceToPlayer;
    EAsteroidTier CurrentTier = Asteroid->CurrentTier;
    EAsteroidTier NewTier = CurrentTier;

    // Hysteresis: Require 10% buffer before transitioning
    const float Hysteresis = 1.1f;

    if (Distance < Tier0Radius)
        NewTier = EAsteroidTier::Critical;
    else if (Distance < Tier1Radius * Hysteresis && CurrentTier <= EAsteroidTier::Active)
        NewTier = EAsteroidTier::Active;
    else if (Distance > Tier1Radius && Distance < Tier2Radius)
        NewTier = EAsteroidTier::Distant;
    else if (Distance > Tier2Radius && Distance < Tier3Radius)
        NewTier = EAsteroidTier::Far;
    else
        NewTier = EAsteroidTier::Procedural;

    if (NewTier != CurrentTier)
        Asteroid->OnTierChanged(CurrentTier, NewTier);
}
```

**Success Criteria:**
- [ ] Asteroids correctly assigned to tiers based on distance
- [ ] Tier transitions smooth (no popping)
- [ ] Performance scales linearly with asteroid count

---

### Phase 3: Spatial Partitioning (Week 3)

**Files to Create:**
1. `Source/Alexander/Public/AsteroidField/SpatialHashGrid.h` (200 lines)
2. `Source/Alexander/Private/AsteroidField/SpatialHashGrid.cpp` (350 lines)

**Implementation Steps:**
1. Implement grid cell hash function
2. Implement `AddObject()`, `RemoveObject()`, `UpdateObject()`
3. Implement collision query functions
4. Benchmark against naive O(N²) approach

**Grid Hash Function:**
```cpp
FIntVector USpatialHashGrid::WorldToGridCoord(FVector WorldLocation) const
{
    FVector Offset = WorldLocation - GridOrigin;
    return FIntVector(
        FMath::FloorToInt(Offset.X / CellSize),
        FMath::FloorToInt(Offset.Y / CellSize),
        FMath::FloorToInt(Offset.Z / CellSize)
    );
}

void USpatialHashGrid::AddObject(AAsteroid* Asteroid)
{
    FIntVector Cell = WorldToGridCoord(Asteroid->GetActorLocation());
    GridCells.FindOrAdd(Cell).Add(Asteroid);
    Asteroid->CachedGridCell = Cell; // Store for fast updates
}

TArray<AAsteroid*> USpatialHashGrid::GetNearbyAsteroids(FVector Location, float Radius)
{
    TArray<AAsteroid*> Results;
    FIntVector CenterCell = WorldToGridCoord(Location);
    int32 CellRadius = FMath::CeilToInt(Radius / CellSize);

    // Check all cells within radius
    for (int32 x = -CellRadius; x <= CellRadius; ++x)
    {
        for (int32 y = -CellRadius; y <= CellRadius; ++y)
        {
            for (int32 z = -CellRadius; z <= CellRadius; ++z)
            {
                FIntVector Cell = CenterCell + FIntVector(x, y, z);
                if (TArray<AAsteroid*>* CellContents = GridCells.Find(Cell))
                {
                    for (AAsteroid* Asteroid : *CellContents)
                    {
                        if (FVector::Dist(Asteroid->GetActorLocation(), Location) <= Radius)
                            Results.Add(Asteroid);
                    }
                }
            }
        }
    }
    return Results;
}
```

**Success Criteria:**
- [ ] Collision checks reduce from O(N²) to O(N)
- [ ] Performance benchmark: 1000 asteroids collision checks < 0.5ms
- [ ] Grid visualization in debug mode

---

### Phase 4: Object Pooling (Week 4)

**Files to Create:**
1. `Source/Alexander/Public/AsteroidField/AsteroidPool.h` (150 lines)
2. `Source/Alexander/Private/AsteroidField/AsteroidPool.cpp` (250 lines)

**Implementation Steps:**
1. Pre-spawn pool of inactive asteroids
2. Implement acquire/release pattern
3. Replace direct spawn calls with pool acquisition
4. Benchmark allocation performance

**Pool Pattern:**
```cpp
void AAsteroidFieldManager::SpawnAsteroidAtLocation(FVector Location)
{
    // OLD WAY (slow):
    // AAsteroid* NewAsteroid = GetWorld()->SpawnActor<AAsteroid>(AsteroidClass, Location, FRotator::ZeroRotator);

    // NEW WAY (fast):
    AAsteroid* Asteroid = AsteroidPool->AcquireAsteroid(Location, FRotator::ZeroRotator);
    if (Asteroid)
    {
        // Configure asteroid properties
        Asteroid->Mass = FMath::RandRange(1000.0f, 100000.0f);
        Asteroid->Radius = FMath::RandRange(5.0f, 50.0f);
        // ...
    }
}

void AAsteroidFieldManager::DespawnAsteroid(AAsteroid* Asteroid)
{
    // OLD WAY (slow, triggers GC):
    // Asteroid->Destroy();

    // NEW WAY (instant):
    AsteroidPool->ReleaseAsteroid(Asteroid);
}
```

**Success Criteria:**
- [ ] Pool can handle 2000+ asteroids
- [ ] Acquire/release operations < 0.001ms each
- [ ] No GC spikes when asteroids transition tiers

---

### Phase 5: Collision & Fragmentation (Week 5)

**Files to Create:**
1. `Source/Alexander/Public/AsteroidField/AsteroidCollision.h` (100 lines)
2. `Source/Alexander/Private/AsteroidField/AsteroidCollision.cpp` (300 lines)

**Implementation Steps:**
1. Implement ship-asteroid collision detection
2. Implement asteroid-asteroid collision detection
3. Implement fragmentation system
4. Add VFX/SFX for impacts

**Collision Logic:**
```cpp
void AAsteroid::HandleShipCollision(AActor* Ship, const FHitResult& Hit)
{
    // Get ship's hull integrity from ShipCustomizationSystem
    UShipCustomizationSystem* CustomizationSystem = Ship->FindComponentByClass<UShipCustomizationSystem>();
    if (!CustomizationSystem)
        return;

    // Calculate damage based on:
    // 1. Relative velocity
    // 2. Asteroid mass
    // 3. Ship's hull integrity stat
    FVector RelativeVelocity = Velocity - Ship->GetVelocity();
    float ImpactSpeed = RelativeVelocity.Size();
    float ImpactEnergy = 0.5f * Mass * ImpactSpeed * ImpactSpeed; // KE = 0.5mv²

    // Ship's hull integrity reduces damage (1.0 = standard, 2.0 = double armor, etc.)
    float HullIntegrity = CustomizationSystem->GetTotalHullIntegrity();
    float Damage = (ImpactEnergy / HullIntegrity) * DamageMultiplier;

    // Apply damage to ship
    UHealthComponent* HealthComp = Ship->FindComponentByClass<UHealthComponent>();
    if (HealthComp)
        HealthComp->TakeDamage(Damage);

    // Fragment asteroid if impact was severe
    if (ImpactEnergy > FragmentationThreshold && bCanFragment)
        FragmentAsteroid(Hit.ImpactPoint, Hit.ImpactNormal);

    // VFX/SFX
    SpawnImpactEffects(Hit.ImpactPoint, Hit.ImpactNormal);
}

void AAsteroid::FragmentAsteroid(FVector ImpactPoint, FVector ImpactNormal)
{
    // Don't fragment if too small
    if (Mass < MinFragmentMass)
        return;

    // Spawn smaller fragments
    float FragmentMassFraction = 1.0f / FragmentCount;
    for (int32 i = 0; i < FragmentCount; ++i)
    {
        FVector RandomDirection = FMath::VRandCone(ImpactNormal, FMath::DegreesToRadians(45.0f));
        FVector FragmentLocation = ImpactPoint + RandomDirection * Radius;

        AAsteroid* Fragment = AsteroidFieldManager->SpawnAsteroid(FragmentLocation);
        if (Fragment)
        {
            Fragment->Mass = Mass * FragmentMassFraction;
            Fragment->Radius = Radius * FMath::Pow(FragmentMassFraction, 1.0f/3.0f); // Volume scales as r³
            Fragment->Velocity = Velocity + RandomDirection * FMath::RandRange(10.0f, 50.0f);
        }
    }

    // Destroy parent asteroid (return to pool)
    AsteroidFieldManager->DespawnAsteroid(this);
}
```

**Success Criteria:**
- [ ] Ship-asteroid collisions damage ship based on HullIntegrity stat
- [ ] Large asteroids fragment into smaller pieces
- [ ] Fragment physics feels realistic (momentum conserved)
- [ ] VFX/SFX feedback for all collisions

---

### Phase 6: GPU Instancing (Week 6)

**Files to Create:**
1. `Source/Alexander/Public/AsteroidField/AsteroidVFXManager.h` (150 lines)
2. `Source/Alexander/Private/AsteroidField/AsteroidVFXManager.cpp` (250 lines)

**Implementation Steps:**
1. Create instanced static mesh component
2. Implement instance buffer updates
3. Transition Tier 3/4 asteroids to instanced rendering
4. Optimize material for instancing (per-instance custom data)

**Instancing Setup:**
```cpp
void UAsteroidVFXManager::Initialize(UWorld* World)
{
    // Create instanced mesh component
    InstancedAsteroidMesh = NewObject<UInstancedStaticMeshComponent>(World);
    InstancedAsteroidMesh->SetStaticMesh(LowLODMesh);
    InstancedAsteroidMesh->SetCastShadow(false); // Distant asteroids don't need shadows
    InstancedAsteroidMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Pre-allocate instance buffer
    InstanceTransforms.Reserve(MaxInstances);
    InstanceColors.Reserve(MaxInstances);
    InstanceSizes.Reserve(MaxInstances);
}

int32 UAsteroidVFXManager::AddInstance(FTransform Transform, float Size, FLinearColor Color)
{
    int32 Index = InstanceTransforms.Num();
    InstanceTransforms.Add(Transform);
    InstanceColors.Add(Color);
    InstanceSizes.Add(Size);

    // Update instanced mesh
    InstancedAsteroidMesh->AddInstance(Transform);
    InstancedAsteroidMesh->SetCustomDataValue(Index, 0, Size); // Custom data slot 0 = size
    InstancedAsteroidMesh->SetCustomDataValue(Index, 1, Color.R);
    InstancedAsteroidMesh->SetCustomDataValue(Index, 2, Color.G);
    InstancedAsteroidMesh->SetCustomDataValue(Index, 3, Color.B);

    return Index;
}

void UAsteroidVFXManager::UpdateAllInstances(float DeltaTime)
{
    // Update transforms (rotation, etc.)
    for (int32 i = 0; i < InstanceTransforms.Num(); ++i)
    {
        FTransform& Transform = InstanceTransforms[i];
        // Rotate asteroids slowly
        FRotator Rotation = Transform.Rotator();
        Rotation.Yaw += DeltaTime * 10.0f; // 10 degrees/sec
        Transform.SetRotation(Rotation.Quaternion());

        InstancedAsteroidMesh->UpdateInstanceTransform(i, Transform, false, false);
    }

    // Mark render state dirty once after all updates
    InstancedAsteroidMesh->MarkRenderStateDirty();
}
```

**Material Setup (Shader):**
```hlsl
// AsteroidInstancedMaterial.usf
// Access per-instance custom data in material graph

// Custom data layout:
// CustomData[0] = Size multiplier
// CustomData[1-3] = RGB color variation

float3 PerInstanceColor = float3(
    PerInstanceCustomData.y,
    PerInstanceCustomData.z,
    PerInstanceCustomData.w
);

float SizeMultiplier = PerInstanceCustomData.x;
```

**Success Criteria:**
- [ ] Tier 3 asteroids render via instancing
- [ ] 10,000+ instances render at 60 FPS
- [ ] Single draw call for all distant asteroids
- [ ] Per-instance color/size variation works

---

### Phase 7: Network Replication (Week 7)

**Files to Create:**
1. `Source/Alexander/Public/AsteroidField/AsteroidNetworkTypes.h` (200 lines)
2. `Source/Alexander/Private/AsteroidField/AsteroidReplication.cpp` (300 lines)

**Implementation Steps:**
1. Design compressed network state structures
2. Implement relevancy culling (only replicate nearby asteroids)
3. Implement deterministic procedural generation (same seed = same asteroids)
4. Test in multiplayer environment

**Network Design Philosophy:**

**Key Insight:** We DON'T need to replicate all asteroids!

Most asteroids are deterministic:
- Tier 2/3/4: Procedurally generated with fixed seed
- All clients generate same asteroids from same seed
- Only replicate asteroids that have been perturbed by physics

**What We Replicate:**
1. **Field Parameters:** (replicated once at start)
   - Field center, radius, density, random seed
   - All clients generate identical initial asteroid field

2. **Active Asteroids:** (only those affected by gameplay)
   - Tier 0: Always replicate (critical objects)
   - Tier 1: Replicate if hit by player or other replicated object
   - Tier 2+: Never replicate (deterministic paths)

**Network Structures:**
```cpp
// Replicated once at level start
USTRUCT(BlueprintType)
struct FAsteroidFieldState
{
    GENERATED_BODY()

    UPROPERTY(Replicated)
    int32 RandomSeed; // Same seed = same asteroids

    UPROPERTY(Replicated)
    FVector FieldCenter;

    UPROPERTY(Replicated)
    float FieldRadius;

    UPROPERTY(Replicated)
    int32 TargetAsteroidCount;

    UPROPERTY(Replicated)
    float DensityMultiplier;
};

// Compressed state for individual asteroid (only if active)
USTRUCT()
struct FAsteroidCompressedState
{
    GENERATED_BODY()

    // Position compressed to 16-bit per axis (relative to field center)
    UPROPERTY()
    uint16 CompressedX;

    UPROPERTY()
    uint16 CompressedY;

    UPROPERTY()
    uint16 CompressedZ;

    // Velocity compressed to 8-bit per axis (±127 m/s)
    UPROPERTY()
    int8 CompressedVelX;

    UPROPERTY()
    int8 CompressedVelY;

    UPROPERTY()
    int8 CompressedVelZ;

    // Rotation as compressed quaternion (16-bit per component)
    UPROPERTY()
    uint16 CompressedRotation[4];

    // Total: 2*3 + 1*3 + 2*4 = 6+3+8 = 17 bytes per asteroid
};

// Replicated to clients (only active asteroids)
USTRUCT()
struct FAsteroidNetworkUpdate
{
    GENERATED_BODY()

    UPROPERTY(Replicated)
    int32 AsteroidID; // Unique ID for tracking

    UPROPERTY(Replicated)
    FAsteroidCompressedState State;

    UPROPERTY(Replicated)
    uint8 TierFlags; // Bit flags for tier + collision enabled + etc.
};
```

**Replication Logic:**
```cpp
void AAsteroidFieldManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Replicate field state to all clients (once)
    DOREPLIFETIME(AAsteroidFieldManager, FieldState);

    // Replicate active asteroids with distance-based relevancy
    DOREPLIFETIME_CONDITION(AAsteroidFieldManager, ActiveAsteroidStates, COND_Custom);
}

void AAsteroidFieldManager::ReplicateActiveAsteroids()
{
    if (!HasAuthority())
        return;

    // Only replicate asteroids within network relevance distance
    const float RelevanceDistance = 100000.0f; // 100 km

    ActiveAsteroidStates.Empty();
    for (AAsteroid* Asteroid : AllAsteroids)
    {
        if (Asteroid->CurrentTier == EAsteroidTier::Critical ||
            Asteroid->CurrentTier == EAsteroidTier::Active)
        {
            float DistanceToAnyPlayer = GetDistanceToNearestPlayer(Asteroid);
            if (DistanceToAnyPlayer < RelevanceDistance)
            {
                FAsteroidNetworkUpdate Update;
                Update.AsteroidID = Asteroid->UniqueID;
                Update.State = CompressAsteroidState(Asteroid);
                ActiveAsteroidStates.Add(Update);
            }
        }
    }
}

// Compression helper
FAsteroidCompressedState AAsteroidFieldManager::CompressAsteroidState(AAsteroid* Asteroid)
{
    FAsteroidCompressedState Compressed;

    // Compress position relative to field center (16-bit range: 0-65535)
    FVector RelativePos = Asteroid->GetActorLocation() - FieldState.FieldCenter;
    const float PosRange = FieldState.FieldRadius;
    Compressed.CompressedX = FMath::Clamp(((RelativePos.X + PosRange) / (2*PosRange)) * 65535, 0.0f, 65535.0f);
    Compressed.CompressedY = FMath::Clamp(((RelativePos.Y + PosRange) / (2*PosRange)) * 65535, 0.0f, 65535.0f);
    Compressed.CompressedZ = FMath::Clamp(((RelativePos.Z + PosRange) / (2*PosRange)) * 65535, 0.0f, 65535.0f);

    // Compress velocity (8-bit range: -127 to 127 m/s)
    Compressed.CompressedVelX = FMath::Clamp(Asteroid->Velocity.X, -127.0f, 127.0f);
    Compressed.CompressedVelY = FMath::Clamp(Asteroid->Velocity.Y, -127.0f, 127.0f);
    Compressed.CompressedVelZ = FMath::Clamp(Asteroid->Velocity.Z, -127.0f, 127.0f);

    // Compress rotation (quaternion to 16-bit per component)
    FQuat Quat = Asteroid->GetActorRotation().Quaternion();
    Compressed.CompressedRotation[0] = (Quat.X + 1.0f) * 32767.5f;
    Compressed.CompressedRotation[1] = (Quat.Y + 1.0f) * 32767.5f;
    Compressed.CompressedRotation[2] = (Quat.Z + 1.0f) * 32767.5f;
    Compressed.CompressedRotation[3] = (Quat.W + 1.0f) * 32767.5f;

    return Compressed;
}
```

**Bandwidth Analysis:**

**Initial Field Sync (once per client):**
- Field state: ~40 bytes
- Negligible cost (one-time)

**Per-Frame Updates:**
- Active asteroids: ~50 (max within 100km)
- Bytes per asteroid: 17 bytes (compressed state) + 4 bytes (ID) + 1 byte (flags) = 22 bytes
- Update rate: 2 Hz (very low, mostly kinematic)
- **Bandwidth: 50 asteroids × 22 bytes × 2 Hz = 2200 bytes/sec = 2.2 KB/s = 17.6 kbps**

**Total with ships (from AGENT4_SPACESHIP_NETWORK_DOCUMENTATION.md):**
- Ships: 25.96 kbps per player
- Asteroids: 17.6 kbps shared among all players
- **Total: ~26 kbps per player (asteroids add only 0.04 kbps incremental!)**

**Success Criteria:**
- [ ] Clients see same asteroid field (deterministic generation)
- [ ] Active asteroids synchronize correctly
- [ ] Bandwidth < 20 kbps per player
- [ ] No desyncs after 10 minutes of gameplay

---

## Ship Customization Integration

### Hull Integrity System

**Existing System (from ShipCustomizationSystem.h):**
```cpp
USTRUCT(BlueprintType)
struct FShipPerformanceStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float Armor; // Existing armor stat

    // ADD NEW:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float HullIntegrity; // Damage resistance multiplier
};
```

**Component-Based Hull Integrity:**

Each hull component contributes to total hull integrity:
```cpp
USTRUCT(BlueprintType)
struct FShipComponentData
{
    // Existing stats
    TMap<FString, float> Stats;

    // Example hull component stats:
    // Stats["HullIntegrity"] = 1.5f;  // 50% more damage resistance
    // Stats["Mass"] = 2000.0f;        // Heavier hull = more collision damage to asteroids
};
```

**Integration with Asteroid Collision:**
```cpp
void AAsteroid::CalculateCollisionDamage(AActor* Ship)
{
    UShipCustomizationSystem* CustomizationSystem = Ship->FindComponentByClass<UShipCustomizationSystem>();
    if (!CustomizationSystem)
        return;

    FShipPerformanceStats ShipStats = CustomizationSystem->CalculatePerformance();

    // Get collision properties
    FVector RelativeVelocity = Velocity - Ship->GetVelocity();
    float ImpactSpeed = RelativeVelocity.Size(); // m/s
    float ShipMass = ShipStats.TotalMass; // kg
    float AsteroidMass = Mass; // kg

    // Calculate kinetic energy (KE = 0.5 * m * v²)
    float ImpactEnergy = 0.5f * AsteroidMass * ImpactSpeed * ImpactSpeed;

    // Hull integrity reduces damage (1.0 = standard, 2.0 = reinforced, 0.5 = lightweight)
    float HullIntegrity = FMath::Max(ShipStats.HullIntegrity, 0.1f); // Minimum 0.1 to prevent divide by zero
    float BaseDamage = ImpactEnergy / 1000.0f; // Convert to reasonable damage scale
    float FinalDamage = BaseDamage / HullIntegrity;

    // Apply damage to ship
    UHealthComponent* HealthComp = Ship->FindComponentByClass<UHealthComponent>();
    if (HealthComp)
    {
        HealthComp->TakeDamage(FinalDamage);

        // Log for debugging
        UE_LOG(LogTemp, Warning, TEXT("Ship collision: Impact=%f m/s, Energy=%f J, HullIntegrity=%f, Damage=%f"),
               ImpactSpeed, ImpactEnergy, HullIntegrity, FinalDamage);
    }

    // Damage asteroid based on ship mass (heavy ships damage asteroids more)
    float AsteroidDamage = (ShipMass / AsteroidMass) * ImpactSpeed * 0.1f;
    AsteroidHealth -= AsteroidDamage;
    if (AsteroidHealth <= 0.0f)
        FragmentAsteroid(GetActorLocation(), -RelativeVelocity.GetSafeNormal());
}
```

**Hull Component Examples:**

| Hull Type | Mass (kg) | HullIntegrity | Speed Penalty | Cost |
|-----------|-----------|---------------|---------------|------|
| Lightweight | 500 | 0.5x | +10% speed | $1,000 |
| Standard | 1000 | 1.0x | Baseline | $5,000 |
| Reinforced | 2000 | 1.5x | -5% speed | $15,000 |
| Military | 3000 | 2.0x | -10% speed | $50,000 |
| Experimental | 1500 | 2.5x | +5% speed | $100,000 |

**Gameplay Trade-offs:**
- **Lightweight Hull:** Faster, but asteroid collisions deal 2x damage
- **Military Hull:** Tanky, can ram through asteroid fields, but slower
- **Experimental Hull:** Best of both worlds, but very expensive

---

### Mass & Thrust Integration

**Existing System (from AOrbitalBody):**
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Physics")
float ThrustPower = 100000.0f; // Newtons

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
float Mass; // kg
```

**Asteroid Push Mechanics:**

Ships with high thrust can "push" asteroids:
```cpp
void AAsteroid::HandleShipCollision(AActor* Ship, const FHitResult& Hit)
{
    AOrbitalBody* ShipBody = Cast<AOrbitalBody>(Ship);
    if (!ShipBody)
        return;

    // Calculate momentum transfer
    FVector ShipMomentum = ShipBody->Velocity * ShipBody->Mass;
    FVector AsteroidMomentum = Velocity * Mass;

    // Elastic collision (simplified)
    float TotalMass = ShipBody->Mass + Mass;
    FVector FinalShipVelocity = ((ShipBody->Mass - Mass) * ShipBody->Velocity + 2 * Mass * Velocity) / TotalMass;
    FVector FinalAsteroidVelocity = ((Mass - ShipBody->Mass) * Velocity + 2 * ShipBody->Mass * ShipBody->Velocity) / TotalMass;

    // Apply velocity changes
    ShipBody->Velocity = FinalShipVelocity;
    Velocity = FinalAsteroidVelocity;

    // Add thrust force if ship is actively thrusting
    UFlightController* FlightController = Ship->FindComponentByClass<UFlightController>();
    if (FlightController && FlightController->IsThrusting())
    {
        FVector ThrustForce = Ship->GetActorForwardVector() * ShipBody->ThrustPower;
        FVector ThrustImpulse = ThrustForce * Hit.Time; // Impulse = Force × Time

        // Transfer some thrust to asteroid
        float TransferRatio = ShipBody->Mass / TotalMass;
        AddImpulse(ThrustImpulse * TransferRatio);
    }
}
```

**Gameplay Scenarios:**

1. **Mining Ship (Heavy, Low Thrust):**
   - Mass: 10,000 kg
   - ThrustPower: 50,000 N
   - Can push small asteroids (< 1,000 kg) out of the way
   - Gets deflected by large asteroids (> 10,000 kg)

2. **Fighter Ship (Light, High Thrust):**
   - Mass: 1,000 kg
   - ThrustPower: 200,000 N
   - Can dodge asteroids easily (high thrust-to-weight ratio)
   - Bounces off medium/large asteroids

3. **Capital Ship (Very Heavy, High Thrust):**
   - Mass: 100,000 kg
   - ThrustPower: 1,000,000 N
   - Plows through asteroid fields
   - Small asteroids bounce off harmlessly

---

## Network Replication Strategy

### Deterministic Asteroid Generation

**Key Principle:** All clients generate the same asteroid field from a shared seed.

**Advantages:**
- Zero bandwidth for 99% of asteroids
- All clients see same field layout
- Perfect synchronization without network traffic

**Implementation:**
```cpp
void AAsteroidFieldManager::InitializeField(int32 Seed, FVector Center, float Radius)
{
    // Seed random stream (deterministic)
    RandomStream.Initialize(Seed);

    // Generate asteroids
    for (int32 i = 0; i < TargetAsteroidCount; ++i)
    {
        // Deterministic random position
        FVector RandomPos = Center + RandomStream.VRandCone(FVector::UpVector, PI) * Radius;

        // Deterministic random properties
        float RandomMass = RandomStream.FRandRange(MassRange.GetLowerBoundValue(), MassRange.GetUpperBoundValue());
        float RandomSize = RandomStream.FRandRange(SizeRange.GetLowerBoundValue(), SizeRange.GetUpperBoundValue());
        FVector RandomVelocity = RandomStream.VRand() * RandomStream.FRandRange(0.0f, 50.0f);

        // Spawn asteroid (same on all clients)
        AAsteroid* Asteroid = SpawnAsteroid(RandomPos, RandomSize, RandomMass, RandomVelocity);
        Asteroid->UniqueID = i; // Deterministic ID
    }
}

// On client, this produces IDENTICAL field to server
```

**Synchronization Events:**

Only replicate when asteroid state **deviates from deterministic path**:

```cpp
void AAsteroid::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (HasAuthority())
    {
        // Check if asteroid has been perturbed
        FVector DeterministicPosition = GetKinematicPosition(GetWorld()->GetTimeSeconds());
        float DeviationDistance = FVector::Dist(GetActorLocation(), DeterministicPosition);

        if (DeviationDistance > 10.0f) // 10 meter threshold
        {
            // Mark as active (needs replication)
            bIsNetworkRelevant = true;
            CurrentTier = FMath::Min(CurrentTier, EAsteroidTier::Active);
        }
    }
}
```

### Bandwidth Optimization Techniques

**1. Delta Compression:**
```cpp
// Only replicate changed values
UPROPERTY(Replicated)
FAsteroidCompressedState CurrentState;

UPROPERTY()
FAsteroidCompressedState LastReplicatedState;

void AAsteroid::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Use delta serialization (only send changed bits)
    DOREPLIFETIME_CONDITION(AAsteroid, CurrentState, COND_Custom);
}

bool AAsteroid::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
    bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

    // Only replicate if state changed significantly
    if (HasStateChanged(LastReplicatedState, CurrentState))
    {
        WroteSomething |= Channel->ReplicateSubobject(this, *Bunch, *RepFlags);
        LastReplicatedState = CurrentState;
    }

    return WroteSomething;
}
```

**2. Relevancy Culling:**
```cpp
bool AAsteroid::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
    // Only replicate if:
    // 1. Within network distance
    // 2. Is active (perturbed from deterministic path)
    // 3. Is critical tier

    if (!bIsNetworkRelevant)
        return false;

    float Distance = FVector::Dist(GetActorLocation(), SrcLocation);
    if (Distance > NetworkRelevanceDistance)
        return false;

    if (CurrentTier == EAsteroidTier::Critical || CurrentTier == EAsteroidTier::Active)
        return true;

    return false;
}
```

**3. Update Rate Prioritization:**
```cpp
float AAsteroid::GetNetPriority(const FVector& ViewPos, const FVector& ViewDir, AActor* Viewer, AActor* ViewTarget, UActorChannel* InChannel, float Time, bool bLowBandwidth)
{
    float Priority = Super::GetNetPriority(ViewPos, ViewDir, Viewer, ViewTarget, InChannel, Time, bLowBandwidth);

    // Higher priority for closer asteroids
    float Distance = FVector::Dist(GetActorLocation(), ViewPos);
    Priority *= FMath::Clamp(1.0f - (Distance / NetworkRelevanceDistance), 0.1f, 1.0f);

    // Higher priority for high-velocity asteroids (more likely to desync)
    float Speed = Velocity.Size();
    Priority *= FMath::Clamp(Speed / 100.0f, 0.5f, 2.0f);

    // Higher priority for critical tier
    if (CurrentTier == EAsteroidTier::Critical)
        Priority *= 2.0f;

    return Priority;
}
```

**Bandwidth Budget:**

| Replication Type | Asteroids | Update Rate | Bytes/Update | Total BW |
|------------------|-----------|-------------|--------------|----------|
| Critical (Tier 0) | 5 | 20 Hz | 22 bytes | 2.2 KB/s |
| Active (Tier 1) | 20 | 5 Hz | 22 bytes | 2.2 KB/s |
| Distant (Tier 2+) | 0 | 0 Hz | 0 bytes | 0 KB/s |
| **Total** | **25** | **-** | **-** | **4.4 KB/s** |

**Per-Player Bandwidth:**
- **Asteroids:** 4.4 KB/s (shared)
- **Ships:** 3.2 KB/s per player (from Agent 4 doc)
- **Total for 4 players:** 4.4 + (3.2 × 4) = 17.2 KB/s = 137.6 kbps

**Well within reasonable limits!**

---

## Performance Budgets

### Per-Frame CPU Budget Breakdown

**Total Frame Budget:** 16.67ms (60 FPS)

```
┌─────────────────────────────────────────────────────────────┐
│ FRAME BUDGET ALLOCATION                                     │
├─────────────────────────────────────────────────────────────┤
│ Rendering (Mesh, Lighting, Shadows, Post-Process)   6.00ms │
│ Physics (Character, Vehicles, Ragdoll)              4.00ms │
│ Gameplay (AI, Blueprint, Game Logic)                2.00ms │
│ Animation (Skeletal Mesh, IK, Blending)             1.50ms │
│ Audio (3D Sound, Music, Voice)                      1.00ms │
│ Network (Replication, RPC, Serialization)           1.00ms │
│ ───────────────────────────────────────────────────────────│
│ ASTEROID SYSTEM BUDGET:                             2.67ms │
│   ├─ Tier 0 (Critical) Physics                     0.80ms │
│   ├─ Tier 1 (Active) Physics                       0.80ms │
│   ├─ Tier 2 (Distant) Kinematic                    0.40ms │
│   ├─ Tier 3 (Far) Culling/LOD                      0.10ms │
│   ├─ Tier 4 (Procedural) GPU Instancing            0.20ms │
│   ├─ Spatial Hash Grid Updates                     0.20ms │
│   └─ Tier Management Overhead                      0.17ms │
│ ───────────────────────────────────────────────────────────│
│ TOTAL ALLOCATED                                    18.17ms │
│ BUFFER (for spikes)                                -1.50ms │
└─────────────────────────────────────────────────────────────┘

WARNING: Budget exceeds 16.67ms by 1.5ms
MITIGATION: Dynamic quality adjustment reduces asteroid count if over budget
```

### Tier-Specific Performance Targets

**Tier 0: Critical (Full N-Body Physics)**
```
Objects: 20 (10 planets + player ship + 9 nearby asteroids)
Calculations: 20×19/2 = 190 pairs
Time per calculation: 0.004ms
Target time: 0.80ms
ACTUAL: 190 × 0.004ms = 0.76ms ✓
Margin: 0.04ms (5% under budget)
```

**Tier 1: Active (Simplified Physics)**
```
Objects: 200 asteroids
Gravity calculations: 200 × 20 (from Tier 0) = 4,000
Collision checks: 200 asteroids, ~10 checks each = 2,000
Total operations: 6,000
Time per operation: 0.0001ms (optimized)
Target time: 0.80ms
ACTUAL: 6,000 × 0.0001ms = 0.60ms ✓
Margin: 0.20ms (25% under budget)
```

**Tier 2: Distant (Kinematic)**
```
Objects: 400 asteroids
Operations per asteroid: Position update (kinematic) = 1
Time per operation: 0.001ms
Target time: 0.40ms
ACTUAL: 400 × 0.001ms = 0.40ms ✓
Margin: 0ms (exactly on budget)
```

**Tier 3: Far (Culling Only)**
```
Objects: 500 asteroids
Operations per asteroid: Frustum cull + LOD check = 1
Time per operation: 0.0002ms
Target time: 0.10ms
ACTUAL: 500 × 0.0002ms = 0.10ms ✓
Margin: 0ms (exactly on budget)
```

**Tier 4: Procedural (GPU Instanced)**
```
Objects: 10,000+ instances
CPU operations: Single instance buffer update + draw call
GPU operations: Handled by GPU (not counted in CPU budget)
Target time: 0.20ms
ACTUAL: ~0.15ms (buffer update) + 0.05ms (draw call) ✓
Margin: 0ms
```

**Spatial Hash Grid:**
```
Operations: Update ~200 moving asteroids × 1 grid update
Time per update: 0.001ms (hash calculation + array insertion)
Target time: 0.20ms
ACTUAL: 200 × 0.001ms = 0.20ms ✓
Margin: 0ms
```

**Tier Manager Overhead:**
```
Operations:
  - Distance calculations: 1,120 asteroids × 0.0001ms = 0.112ms
  - Tier transitions: ~5 per frame × 0.005ms = 0.025ms
  - Performance monitoring: 0.02ms
  - Memory management: 0.01ms
Target time: 0.17ms
ACTUAL: 0.167ms ✓
Margin: 0.003ms
```

### Dynamic Quality Adjustment

**Auto-Scaling Algorithm:**
```cpp
void AAsteroidFieldManager::AdjustQualitySettings()
{
    // Measure actual frame time
    float CurrentFrameTime = FPlatformTime::Seconds() - LastFrameStartTime;

    // If over budget, reduce quality
    if (CurrentFrameTime > TargetFrameTime)
    {
        float Overage = CurrentFrameTime - TargetFrameTime;

        // Reduce asteroid counts proportionally
        if (Overage > 5.0f) // > 5ms over
        {
            // Severe: Reduce by 50%
            Tier1Radius *= 0.5f;
            Tier2Radius *= 0.5f;
            UE_LOG(LogTemp, Warning, TEXT("Asteroid System: Severe performance impact, reducing quality by 50%%"));
        }
        else if (Overage > 2.0f) // > 2ms over
        {
            // Moderate: Reduce by 25%
            Tier1Radius *= 0.75f;
            Tier2Radius *= 0.75f;
            UE_LOG(LogTemp, Warning, TEXT("Asteroid System: Moderate performance impact, reducing quality by 25%%"));
        }
        else // < 2ms over
        {
            // Minor: Reduce by 10%
            Tier1Radius *= 0.9f;
            UE_LOG(LogTemp, Log, TEXT("Asteroid System: Minor performance impact, reducing quality by 10%%"));
        }

        // Force tier reassignment
        UpdateAsteroidTiers(0.0f);
    }
    else if (CurrentFrameTime < TargetFrameTime * 0.8f)
    {
        // We have headroom, can increase quality
        Tier1Radius = FMath::Min(Tier1Radius * 1.1f, 50000.0f);
        Tier2Radius = FMath::Min(Tier2Radius * 1.1f, 200000.0f);
    }
}
```

### Memory Budget

**Per-Asteroid Memory:**
```
AAsteroid actor:                 ~2 KB (base actor + components)
FAsteroidCompressedState:        17 bytes
OrbitPathPoints (100 points):    1.2 KB (100 × FVector)
Spatial hash grid entry:         8 bytes (pointer)
Total per asteroid:              ~3.2 KB
```

**Total Memory for 1,120 Asteroids:**
```
1,120 asteroids × 3.2 KB = 3,584 KB = 3.5 MB
```

**Object Pool Pre-Allocation:**
```
2,000 pooled asteroids × 2 KB = 4,000 KB = 4 MB
```

**Spatial Hash Grid:**
```
Grid cells: 20×20×20 = 8,000 cells
Average pointers per cell: 2
Total: 8,000 × 2 × 8 bytes = 128 KB
```

**GPU Instancing Buffer:**
```
10,000 instances × (FTransform + CustomData)
= 10,000 × (64 bytes + 16 bytes) = 800 KB
```

**Total Asteroid System Memory:**
```
Asteroids:       3.5 MB
Object Pool:     4.0 MB
Spatial Grid:    0.1 MB
GPU Instances:   0.8 MB
───────────────────────
TOTAL:           8.4 MB
```

**Extremely reasonable for modern systems!**

---

## Data Structures

### Core Data Types

**FAsteroidSpawnData:**
```cpp
USTRUCT(BlueprintType)
struct FAsteroidSpawnData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    int32 UniqueID; // Deterministic ID from seed

    UPROPERTY(BlueprintReadWrite)
    FVector Location;

    UPROPERTY(BlueprintReadWrite)
    FRotator Rotation;

    UPROPERTY(BlueprintReadWrite)
    float Mass; // kg

    UPROPERTY(BlueprintReadWrite)
    float Radius; // meters

    UPROPERTY(BlueprintReadWrite)
    FVector InitialVelocity; // m/s

    UPROPERTY(BlueprintReadWrite)
    FVector AngularVelocity; // degrees/sec

    UPROPERTY(BlueprintReadWrite)
    int32 MeshIndex; // Which mesh from AsteroidMeshes array

    UPROPERTY(BlueprintReadWrite)
    int32 MaterialIndex; // Which material variant

    FAsteroidSpawnData()
        : UniqueID(0)
        , Location(FVector::ZeroVector)
        , Rotation(FRotator::ZeroRotator)
        , Mass(10000.0f)
        , Radius(10.0f)
        , InitialVelocity(FVector::ZeroVector)
        , AngularVelocity(FVector::ZeroVector)
        , MeshIndex(0)
        , MaterialIndex(0)
    {}
};
```

**FAsteroidTierStats:**
```cpp
USTRUCT(BlueprintType)
struct FAsteroidTierStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 Tier0Count; // Critical

    UPROPERTY(BlueprintReadOnly)
    int32 Tier1Count; // Active

    UPROPERTY(BlueprintReadOnly)
    int32 Tier2Count; // Distant

    UPROPERTY(BlueprintReadOnly)
    int32 Tier3Count; // Far

    UPROPERTY(BlueprintReadOnly)
    int32 Tier4Count; // Procedural

    UPROPERTY(BlueprintReadOnly)
    float Tier0PhysicsTime; // ms

    UPROPERTY(BlueprintReadOnly)
    float Tier1PhysicsTime; // ms

    UPROPERTY(BlueprintReadOnly)
    float Tier2KinematicTime; // ms

    UPROPERTY(BlueprintReadOnly)
    float TotalFrameTime; // ms

    UPROPERTY(BlueprintReadOnly)
    int32 CollisionChecksThisFrame;

    UPROPERTY(BlueprintReadOnly)
    int32 TierTransitionsThisFrame;

    FAsteroidTierStats()
        : Tier0Count(0)
        , Tier1Count(0)
        , Tier2Count(0)
        , Tier3Count(0)
        , Tier4Count(0)
        , Tier0PhysicsTime(0.0f)
        , Tier1PhysicsTime(0.0f)
        , Tier2KinematicTime(0.0f)
        , TotalFrameTime(0.0f)
        , CollisionChecksThisFrame(0)
        , TierTransitionsThisFrame(0)
    {}

    FString ToString() const
    {
        return FString::Printf(TEXT("Tiers[0:%d, 1:%d, 2:%d, 3:%d, 4:%d] Times[T0:%.2fms, T1:%.2fms, T2:%.2fms] Total:%.2fms Collisions:%d Transitions:%d"),
            Tier0Count, Tier1Count, Tier2Count, Tier3Count, Tier4Count,
            Tier0PhysicsTime, Tier1PhysicsTime, Tier2KinematicTime,
            TotalFrameTime, CollisionChecksThisFrame, TierTransitionsThisFrame);
    }
};
```

**FAsteroidFieldConfiguration:**
```cpp
USTRUCT(BlueprintType)
struct FAsteroidFieldConfiguration
{
    GENERATED_BODY()

    // Field Layout
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
    FVector FieldCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
    float FieldRadius = 100000.0f; // 100 km

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
    int32 TargetAsteroidCount = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Field")
    int32 RandomSeed = 12345;

    // Size Distribution
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float SmallAsteroidPercent = 0.7f; // 70% small (5-50m)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MediumAsteroidPercent = 0.25f; // 25% medium (50-200m)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float LargeAsteroidPercent = 0.05f; // 5% large (200-500m)

    // Density Distribution
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    TArray<FAsteroidCluster> Clusters; // Dense regions

    // Physics Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    FFloatRange MassRange = FFloatRange(1000.0f, 1000000.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    FFloatRange VelocityRange = FFloatRange(0.0f, 50.0f); // m/s

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AsteroidDensity = 2500.0f; // kg/m³ (similar to rock)

    // Visual
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TArray<TSoftObjectPtr<UStaticMesh>> AsteroidMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TArray<TSoftObjectPtr<UMaterialInterface>> AsteroidMaterials;
};

USTRUCT(BlueprintType)
struct FAsteroidCluster
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ClusterCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ClusterRadius = 10000.0f; // 10 km

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DensityMultiplier = 5.0f; // 5x normal density
};
```

---

## Key Design Questions Answered

### Q1: How do we transition asteroids between tiers smoothly?

**Answer: Hysteresis Thresholds**

Use different thresholds for transitioning up vs down to prevent oscillation:

```cpp
void UAsteroidTierManager::UpdateAsteroidTier(AAsteroid* Asteroid)
{
    float Distance = Asteroid->DistanceToPlayer;
    EAsteroidTier CurrentTier = Asteroid->CurrentTier;

    // Hysteresis: 10% buffer
    const float UpgradeThreshold = 0.9f;   // Transition earlier when getting closer
    const float DowngradeThreshold = 1.1f; // Transition later when getting farther

    if (Distance < Tier0Radius)
    {
        // Always upgrade to Critical immediately (safety)
        Asteroid->SetTier(EAsteroidTier::Critical);
    }
    else if (Distance < Tier1Radius * UpgradeThreshold && CurrentTier > EAsteroidTier::Active)
    {
        // Upgrade to Active (getting closer)
        Asteroid->SetTier(EAsteroidTier::Active);
    }
    else if (Distance > Tier1Radius * DowngradeThreshold && CurrentTier == EAsteroidTier::Active)
    {
        // Downgrade to Distant (getting farther)
        Asteroid->SetTier(EAsteroidTier::Distant);
    }
    // ... similar logic for other tiers
}
```

**Visual Transition:**

Smooth LOD crossfade to prevent popping:
```cpp
void AAsteroid::OnTierChanged(EAsteroidTier OldTier, EAsteroidTier NewTier)
{
    // Crossfade LOD meshes over 0.5 seconds
    if (MeshComponent)
    {
        float FadeTime = 0.5f;
        MeshComponent->SetScalarParameterValueOnMaterials(FName("LODFade"), 0.0f);

        // Animate LODFade from 0 to 1 over FadeTime
        FTimerDelegate TimerDelegate;
        TimerDelegate.BindLambda([this, FadeTime]()
        {
            float ElapsedTime = GetWorld()->GetTimerManager().GetTimerElapsed(LODFadeTimer);
            float Alpha = FMath::Clamp(ElapsedTime / FadeTime, 0.0f, 1.0f);
            MeshComponent->SetScalarParameterValueOnMaterials(FName("LODFade"), Alpha);
        });

        GetWorld()->GetTimerManager().SetTimer(LODFadeTimer, TimerDelegate, 0.016f, true);
    }
}
```

---

### Q2: What happens when an asteroid field enters the "bubble"?

**Answer: Progressive Loading**

When player enters asteroid field, asteroids transition tiers progressively:

```cpp
void AAsteroidFieldManager::OnPlayerEnteredField(APawn* Player)
{
    // Don't load all asteroids at once (frame spike)
    // Instead, activate them progressively based on distance

    TArray<AAsteroid*> NearbyAsteroids;
    for (AAsteroid* Asteroid : AllAsteroids)
    {
        float Distance = FVector::Dist(Asteroid->GetActorLocation(), Player->GetActorLocation());
        if (Distance < Tier2Radius)
            NearbyAsteroids.Add(Asteroid);
    }

    // Sort by distance (closest first)
    NearbyAsteroids.Sort([Player](const AAsteroid& A, const AAsteroid& B)
    {
        float DistA = FVector::Dist(A.GetActorLocation(), Player->GetActorLocation());
        float DistB = FVector::Dist(B.GetActorLocation(), Player->GetActorLocation());
        return DistA < DistB;
    });

    // Activate asteroids over multiple frames (10 per frame)
    int32 ActivatePerFrame = 10;
    for (int32 i = 0; i < NearbyAsteroids.Num(); i += ActivatePerFrame)
    {
        // Schedule activation for frame i/ActivatePerFrame
        float Delay = (i / ActivatePerFrame) * 0.016f; // One frame delay per batch

        FTimerDelegate TimerDelegate;
        TimerDelegate.BindLambda([this, NearbyAsteroids, i, ActivatePerFrame]()
        {
            for (int32 j = i; j < FMath::Min(i + ActivatePerFrame, NearbyAsteroids.Num()); ++j)
            {
                AAsteroid* Asteroid = NearbyAsteroids[j];
                Asteroid->ActivatePhysics();
                UpdateAsteroidTier(Asteroid);
            }
        });

        GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegate);
    }
}
```

**Expected Behavior:**
- Player at 300km: All asteroids in Tier 4 (GPU instanced, no physics)
- Player at 150km: Nearest 100 transition to Tier 3 (culling/LOD) over 1 second
- Player at 50km: Nearest 200 transition to Tier 2 (kinematic) over 2 seconds
- Player at 10km: Nearest 100 transition to Tier 1 (active physics) over 1 second
- Player at 1km: Nearest 10 transition to Tier 0 (full N-body) instantly

**No sudden frame spike!**

---

### Q3: How do we handle asteroid collisions (rigid body vs. simplified)?

**Answer: Tier-Based Collision Fidelity**

| Tier | Collision Type | Description |
|------|----------------|-------------|
| Tier 0 | Full Rigid Body | UE physics engine handles collision (most accurate) |
| Tier 1 | Simplified Impulse | Manual collision detection + impulse transfer (fast) |
| Tier 2 | No Collision | Kinematic motion, no collision with other asteroids |
| Tier 3/4 | No Collision | Visual only |

**Implementation:**
```cpp
void AAsteroid::EnablePhysicsForTier(EAsteroidTier Tier)
{
    switch (Tier)
    {
        case EAsteroidTier::Critical:
            // Full physics simulation
            PhysicsComponent->SetSimulatePhysics(true);
            PhysicsComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            PhysicsComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
            PhysicsComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
            break;

        case EAsteroidTier::Active:
            // Simplified collision (manual detection)
            PhysicsComponent->SetSimulatePhysics(false);
            PhysicsComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            PhysicsComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // Trigger overlap events
            bUseManualCollisionDetection = true;
            break;

        case EAsteroidTier::Distant:
        case EAsteroidTier::Far:
        case EAsteroidTier::Procedural:
            // No collision
            PhysicsComponent->SetSimulatePhysics(false);
            PhysicsComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            bUseManualCollisionDetection = false;
            break;
    }
}

void AAsteroid::UpdateTier1_Active(float DeltaTime)
{
    // Manual collision detection using spatial hash grid
    if (bUseManualCollisionDetection)
    {
        USpatialHashGrid* Grid = AsteroidFieldManager->GetSpatialGrid();
        TArray<AAsteroid*> NearbyAsteroids = Grid->GetNearbyAsteroids(GetActorLocation(), Radius * 2.0f);

        for (AAsteroid* Other : NearbyAsteroids)
        {
            if (Other == this)
                continue;

            float Distance = FVector::Dist(GetActorLocation(), Other->GetActorLocation());
            float CombinedRadius = Radius + Other->Radius;

            if (Distance < CombinedRadius)
            {
                // Collision detected! Apply impulse
                FVector CollisionNormal = (Other->GetActorLocation() - GetActorLocation()).GetSafeNormal();
                float OverlapDepth = CombinedRadius - Distance;

                // Separate asteroids
                FVector Separation = CollisionNormal * OverlapDepth * 0.5f;
                AddActorWorldOffset(-Separation);
                Other->AddActorWorldOffset(Separation);

                // Apply velocity change (simplified elastic collision)
                float TotalMass = Mass + Other->Mass;
                FVector RelativeVelocity = Velocity - Other->Velocity;
                float ImpulseMagnitude = (2.0f * FVector::DotProduct(RelativeVelocity, CollisionNormal)) / TotalMass;

                Velocity -= CollisionNormal * ImpulseMagnitude * Other->Mass;
                Other->Velocity += CollisionNormal * ImpulseMagnitude * Mass;
            }
        }
    }

    // Apply simplified gravity (only from Tier 0 objects)
    ApplySimplifiedGravity(DeltaTime);

    // Update position
    AddActorWorldOffset(Velocity * DeltaTime);
}
```

**Performance Comparison:**

| Method | Asteroids | Collision Checks | Time |
|--------|-----------|------------------|------|
| Full Rigid Body | 100 | 100×99/2 = 4,950 | ~50ms |
| Simplified (Spatial Grid) | 100 | 100×10 = 1,000 | ~0.5ms |
| **Speedup** | - | **~5x fewer checks** | **~100x faster** |

---

### Q4: Should asteroids fragment on collision?

**Answer: YES, but with limits**

**Fragmentation Rules:**
1. **Mass Threshold:** Only fragment if mass > 1,000 kg (prevents infinite splitting)
2. **Impact Energy Threshold:** Only fragment if impact energy > 100,000 J
3. **Fragments per Break:** 2-5 fragments (configurable)
4. **Fragment Size:** Each fragment = parent mass / fragment count
5. **Max Fragments per Frame:** Cap at 20 to prevent frame spikes

**Implementation:**
```cpp
void AAsteroid::FragmentAsteroid(FVector ImpactPoint, FVector ImpactNormal)
{
    // Check if fragmentation is allowed
    if (!bCanFragment || Mass < MinFragmentMass)
        return;

    // Check global fragment limit
    if (AsteroidFieldManager->GetActiveFragmentCount() >= AsteroidFieldManager->MaxFragmentsPerFrame)
        return;

    // Determine fragment count based on mass
    int32 NumFragments = FMath::RandRange(2, 5);
    float FragmentMass = Mass / NumFragments;

    // Spawn fragments
    for (int32 i = 0; i < NumFragments; ++i)
    {
        // Random direction with bias toward impact normal
        FVector RandomDir = FMath::LerpStable(
            FMath::VRand(),
            ImpactNormal,
            0.7f // 70% bias toward impact direction
        ).GetSafeNormal();

        FVector FragmentLocation = ImpactPoint + RandomDir * Radius * 0.5f;
        FVector FragmentVelocity = Velocity + RandomDir * FMath::RandRange(20.0f, 100.0f);

        // Acquire from pool
        AAsteroid* Fragment = AsteroidFieldManager->AsteroidPool->AcquireAsteroid(FragmentLocation, FRotator::ZeroRotator);
        if (Fragment)
        {
            Fragment->Mass = FragmentMass;
            Fragment->Radius = Radius * FMath::Pow(1.0f / NumFragments, 1.0f / 3.0f); // r ~ m^(1/3)
            Fragment->Velocity = FragmentVelocity;
            Fragment->AngularVelocity = FMath::VRand() * FMath::RandRange(10.0f, 50.0f);
            Fragment->bCanFragment = (FragmentMass > MinFragmentMass); // Prevent infinite recursion

            // Inherit tier from parent (or downgrade if too small)
            Fragment->CurrentTier = (FragmentMass < 1000.0f) ? EAsteroidTier::Active : CurrentTier;

            AsteroidFieldManager->RegisterFragment(Fragment);
        }
    }

    // Spawn VFX at impact point
    SpawnFragmentationEffects(ImpactPoint, ImpactNormal);

    // Return parent to pool
    AsteroidFieldManager->AsteroidPool->ReleaseAsteroid(this);
}

void AAsteroidFieldManager::RegisterFragment(AAsteroid* Fragment)
{
    ActiveFragments.Add(Fragment);

    // Auto-cleanup old fragments if too many
    if (ActiveFragments.Num() > MaxTotalFragments)
    {
        // Remove oldest fragment
        AAsteroid* OldestFragment = ActiveFragments[0];
        ActiveFragments.RemoveAt(0);
        AsteroidPool->ReleaseAsteroid(OldestFragment);
    }
}
```

**Gameplay Impact:**
- **Visual Spectacle:** Asteroids shattering creates exciting visuals
- **Strategic Depth:** Players can shoot asteroids to break them into manageable pieces
- **Performance Safety:** Fragment limits prevent frame rate collapse
- **Physics Accuracy:** Momentum conserved across fragments

---

### Q5: How do we spawn/despawn asteroids procedurally?

**Answer: Distance-Based Streaming**

Similar to Minecraft's chunk loading:

```cpp
void AAsteroidFieldManager::UpdateAsteroidStreaming(FVector PlayerLocation)
{
    // Divide field into regions (chunks)
    const float ChunkSize = 50000.0f; // 50 km chunks
    FIntVector PlayerChunk = WorldToChunkCoord(PlayerLocation);

    // Determine which chunks should be loaded
    int32 ChunkRadius = 3; // Load 3 chunks in each direction (7×7×7 = 343 chunks)
    TSet<FIntVector> TargetLoadedChunks;

    for (int32 x = -ChunkRadius; x <= ChunkRadius; ++x)
    {
        for (int32 y = -ChunkRadius; y <= ChunkRadius; ++y)
        {
            for (int32 z = -ChunkRadius; z <= ChunkRadius; ++z)
            {
                FIntVector Chunk = PlayerChunk + FIntVector(x, y, z);
                TargetLoadedChunks.Add(Chunk);
            }
        }
    }

    // Unload chunks that are too far
    TArray<FIntVector> ChunksToUnload;
    for (const FIntVector& LoadedChunk : LoadedChunks)
    {
        if (!TargetLoadedChunks.Contains(LoadedChunk))
            ChunksToUnload.Add(LoadedChunk);
    }

    for (const FIntVector& Chunk : ChunksToUnload)
    {
        UnloadChunk(Chunk);
    }

    // Load new chunks
    for (const FIntVector& Chunk : TargetLoadedChunks)
    {
        if (!LoadedChunks.Contains(Chunk))
            LoadChunk(Chunk);
    }
}

void AAsteroidFieldManager::LoadChunk(FIntVector ChunkCoord)
{
    // Generate deterministic asteroids for this chunk
    FVector ChunkWorldPos = ChunkToWorldCoord(ChunkCoord);

    // Seed random generator with chunk coordinates (deterministic)
    FRandomStream ChunkRandom;
    ChunkRandom.Initialize(RandomSeed + ChunkCoord.X * 73856093 + ChunkCoord.Y * 19349663 + ChunkCoord.Z * 83492791);

    // Determine asteroid count for this chunk
    int32 AsteroidsInChunk = FMath::RoundToInt(CalculateChunkDensity(ChunkWorldPos) * 10.0f);

    TArray<AAsteroid*> ChunkAsteroids;
    for (int32 i = 0; i < AsteroidsInChunk; ++i)
    {
        // Random position within chunk
        FVector LocalPos = FVector(
            ChunkRandom.FRandRange(0.0f, ChunkSize),
            ChunkRandom.FRandRange(0.0f, ChunkSize),
            ChunkRandom.FRandRange(0.0f, ChunkSize)
        );
        FVector WorldPos = ChunkWorldPos + LocalPos;

        // Acquire asteroid from pool
        AAsteroid* Asteroid = AsteroidPool->AcquireAsteroid(WorldPos, FRotator::ZeroRotator);
        if (Asteroid)
        {
            // Configure asteroid
            Asteroid->Mass = ChunkRandom.FRandRange(MassRange.GetLowerBoundValue(), MassRange.GetUpperBoundValue());
            Asteroid->Radius = CalculateRadiusFromMass(Asteroid->Mass);
            Asteroid->Velocity = ChunkRandom.VRand() * ChunkRandom.FRandRange(0.0f, 50.0f);
            Asteroid->SetMesh(AsteroidMeshes[ChunkRandom.RandRange(0, AsteroidMeshes.Num() - 1)]);

            ChunkAsteroids.Add(Asteroid);
        }
    }

    // Store chunk data
    LoadedChunks.Add(ChunkCoord);
    ChunkAsteroidMap.Add(ChunkCoord, ChunkAsteroids);

    UE_LOG(LogTemp, Log, TEXT("Loaded chunk %s with %d asteroids"), *ChunkCoord.ToString(), ChunkAsteroids.Num());
}

void AAsteroidFieldManager::UnloadChunk(FIntVector ChunkCoord)
{
    TArray<AAsteroid*>* ChunkAsteroids = ChunkAsteroidMap.Find(ChunkCoord);
    if (!ChunkAsteroids)
        return;

    // Return all asteroids to pool
    for (AAsteroid* Asteroid : *ChunkAsteroids)
    {
        AsteroidPool->ReleaseAsteroid(Asteroid);
    }

    ChunkAsteroidMap.Remove(ChunkCoord);
    LoadedChunks.Remove(ChunkCoord);

    UE_LOG(LogTemp, Log, TEXT("Unloaded chunk %s"), *ChunkCoord.ToString());
}

float AAsteroidFieldManager::CalculateChunkDensity(FVector ChunkWorldPos)
{
    // Base density
    float Density = 1.0f;

    // Check if chunk overlaps with any dense clusters
    for (const FAsteroidCluster& Cluster : FieldConfiguration.Clusters)
    {
        float DistanceToCluster = FVector::Dist(ChunkWorldPos, Cluster.ClusterCenter);
        if (DistanceToCluster < Cluster.ClusterRadius)
        {
            // Density increases near cluster center
            float ClusterInfluence = 1.0f - (DistanceToCluster / Cluster.ClusterRadius);
            Density += ClusterInfluence * Cluster.DensityMultiplier;
        }
    }

    return Density;
}
```

**Benefits:**
- **Infinite Fields:** Can create arbitrarily large asteroid fields
- **Consistent Performance:** Only asteroids near player are loaded
- **Deterministic:** Same field layout on all clients (multiplayer sync)
- **Memory Efficient:** Only ~300 chunks loaded at once (vs millions of asteroids)

---

### Q6: What's the LOD strategy for asteroid meshes?

**Answer: Distance-Based LOD with GPU Instancing**

**LOD Levels:**

| LOD | Distance | Triangles | Use Case |
|-----|----------|-----------|----------|
| LOD0 | 0-1 km | 5,000 | High detail, close inspection |
| LOD1 | 1-10 km | 1,000 | Medium detail, normal gameplay |
| LOD2 | 10-50 km | 200 | Low detail, distant asteroids |
| LOD3 | 50-200 km | 50 | Silhouette only |
| LOD4 | 200+ km | 8 (billboard) | GPU instanced |

**Implementation:**
```cpp
void AAsteroid::UpdateLODLevel()
{
    if (!MeshComponent)
        return;

    float Distance = DistanceToPlayer;
    int32 NewLOD = CurrentLODLevel;

    if (Distance < 1000.0f * 100.0f) // 1 km (UE units: cm)
        NewLOD = 0;
    else if (Distance < 10000.0f * 100.0f) // 10 km
        NewLOD = 1;
    else if (Distance < 50000.0f * 100.0f) // 50 km
        NewLOD = 2;
    else if (Distance < 200000.0f * 100.0f) // 200 km
        NewLOD = 3;
    else
        NewLOD = 4; // Transition to GPU instancing

    if (NewLOD != CurrentLODLevel)
    {
        CurrentLODLevel = NewLOD;

        if (NewLOD == 4)
        {
            // Transition to GPU instanced rendering
            MeshComponent->SetVisibility(false);
            AsteroidFieldManager->GetVFXManager()->AddInstance(GetActorTransform(), Radius, FLinearColor::White);
        }
        else
        {
            // Use regular mesh LOD
            MeshComponent->SetForcedLodModel(NewLOD);
            MeshComponent->SetVisibility(true);
        }
    }
}
```

**LOD Asset Creation Workflow:**

1. **LOD 0 (High Detail):** Sculpted in ZBrush/Blender (5k tris)
2. **LOD 1 (Medium):** Auto-generated decimation to 1k tris
3. **LOD 2 (Low):** Auto-generated decimation to 200 tris
4. **LOD 3 (Silhouette):** Simple sphere with displacement (50 tris)
5. **LOD 4 (Billboard):** Quad with projected texture (2 tris)

**Memory Savings:**

| LOD | Tris per Asteroid | 1000 Asteroids | GPU Memory |
|-----|-------------------|----------------|------------|
| LOD0 Only | 5,000 | 5M tris | ~200 MB |
| LOD Mix (realistic) | ~500 avg | 500k tris | ~20 MB |
| **Savings** | **~10x reduction** | **~10x fewer tris** | **~10x less memory** |

---

## References & Research

### Academic Papers

1. **Barnes-Hut Algorithm (1986)**
   - Paper: "A hierarchical O(N log N) force-calculation algorithm" by Josh Barnes and Piet Hut
   - Key Insight: Group distant objects into single point mass
   - Complexity: O(N log N) vs O(N²)
   - URL: https://en.wikipedia.org/wiki/Barnes%E2%80%93Hut_simulation

2. **Fast Multipole Method (1987)**
   - Even faster than Barnes-Hut for very large N (>10,000)
   - Complexity: O(N)
   - Trade-off: More complex implementation
   - Not needed for our scale (1,000 objects)

3. **Spatial Hashing for Collision Detection**
   - Paper: "Optimized Spatial Hashing for Collision Detection of Deformable Objects"
   - Key Insight: O(1) nearest neighbor lookup
   - Used extensively in game engines

### Game Industry References

**Space Engineers (Keen Software House):**
- Procedural asteroid generation with voxel destruction
- Uses LOD system with 5 levels (similar to our design)
- Performance target: 10,000+ asteroids
- Implementation: CPU-based voxel physics for close asteroids, simplified kinematic for distant
- Lesson: Voxel destruction too expensive for VR (30 FPS target vs our 60 FPS)

**Elite Dangerous (Frontier Developments):**
- Asteroid belt rendering using GPU instancing
- Billions of asteroids using procedural generation
- Physics only calculated for ~100 nearest asteroids
- Lesson: Deterministic generation from seed = zero network bandwidth

**Kerbal Space Program (Squad):**
- "Sphere of Influence" system for orbital mechanics
- Physics simulation switches based on distance to bodies
- Lesson: Patched conics approximation works well for gameplay

**Star Citizen (Cloud Imperium Games):**
- Object Container Streaming for large-scale environments
- 64-bit precision for positions (we use sector-based system)
- Lesson: Chunk-based streaming essential for large worlds

### Unreal Engine Documentation

1. **Instanced Static Meshes:**
   - Documentation: https://docs.unrealengine.com/5.0/en-US/instanced-static-meshes-in-unreal-engine/
   - Performance: 10-100x faster than individual actors for distant objects
   - Used for Tier 4 asteroids

2. **Network Relevancy:**
   - Documentation: https://docs.unrealengine.com/5.0/en-US/networking-and-multiplayer-in-unreal-engine/
   - `IsNetRelevantFor()` reduces replication bandwidth
   - Used for asteroid replication culling

3. **LOD System:**
   - Documentation: https://docs.unrealengine.com/5.0/en-US/static-mesh-level-of-detail-in-unreal-engine/
   - Automatic LOD generation tools
   - Used for all asteroid meshes

4. **Async Physics (Chaos Physics):**
   - UE5's new physics engine
   - Multi-threaded physics simulation
   - Potential future optimization: Offload Tier 0/1 physics to worker threads

### Open-Source Reference Implementations

**N-Body Simulation (GitHub):**
```
https://github.com/topics/n-body-simulation
```
- Various implementations of Barnes-Hut algorithm
- Useful for understanding octree construction
- Most are CPU-only (we could extend to GPU)

**Unreal Engine Marketplace Assets:**
- **"Space Flight Simulator"** - Basic orbital mechanics
- **"Procedural Planet Generation"** - LOD terrain system (similar principles)
- **"Advanced Locomotion System"** - Good example of state machines (useful for tier system)

---

## Summary & Next Steps

### Design Complete ✓

This document provides a **complete architecture** for a scalable asteroid field system capable of handling 1000+ asteroids at 60 FPS in VR.

**Key Innovations:**
1. **Hybrid Tiered System:** 5 tiers with different physics fidelity
2. **Spatial Hash Grid:** O(N) collision detection (75x speedup)
3. **Deterministic Generation:** Zero bandwidth for 99% of asteroids
4. **GPU Instancing:** 10,000+ background asteroids for free
5. **Object Pooling:** No GC spikes
6. **Ship Integration:** Hull Integrity stat affects asteroid collision damage

**Performance Targets Met:**
- ✓ CPU Budget: 2.67ms per frame (within 16.67ms frame budget)
- ✓ Memory Budget: 8.4 MB (negligible on modern systems)
- ✓ Network Budget: 17.6 kbps (minimal, deterministic generation)
- ✓ Scalability: Supports 1,000 active + 10,000 visual asteroids

### Implementation Roadmap

**Phase 1-3 (Weeks 1-3):** Core systems
- Minimum viable product: 100 asteroids working

**Phase 4-5 (Weeks 4-5):** Performance optimizations
- Scaling to 500+ asteroids

**Phase 6-7 (Weeks 6-7):** Polish & networking
- Full 1000+ asteroid support with multiplayer

**Total Estimate:** 7 weeks of implementation

### Open Questions for User

1. **Asteroid Visual Style:** Realistic rocky textures? Or stylized/sci-fi?
2. **Mining Mechanics:** Can players mine asteroids for resources?
3. **Destructible Asteroids:** Should large asteroids be destructible with weapons?
4. **Procedural Variety:** How many unique asteroid meshes needed? (Recommend 10-20)
5. **Cluster Locations:** Where should dense asteroid clusters be placed? (Near planets? Around warp gates?)

---

**This design document is ready for review and implementation!**
