# Asteroid Field System - Quick Reference Guide

**For:** Rapid implementation reference
**See Also:** ASTEROID_FIELD_SYSTEM_DESIGN.md (full details)

---

## System at a Glance

**Problem:** O(N²) n-body physics doesn't scale beyond 100 objects
**Solution:** 5-tier hybrid system with spatial partitioning
**Result:** 1000+ asteroids at 60 FPS (2.67ms CPU budget)

---

## The 5 Tiers

```
Tier 0 (Critical)     →  Full N-body physics     →  0-1 km    →  ~20 objects  →  0.8ms
Tier 1 (Active)       →  Simplified physics      →  1-50 km   →  ~200 objects →  0.8ms
Tier 2 (Distant)      →  Kinematic orbits        →  50-200 km →  ~400 objects →  0.4ms
Tier 3 (Far)          →  No physics, LOD only    →  200-500km →  ~500 objects →  0.1ms
Tier 4 (Procedural)   →  GPU instanced           →  500km+    →  ~10k objects →  0.2ms
```

---

## Key Components to Implement

### 1. AAsteroidFieldManager
**File:** `Source/Alexander/Public/AsteroidField/AsteroidFieldManager.h`
**Purpose:** Master coordinator for all asteroid simulation

**Key Properties:**
```cpp
int32 TargetAsteroidCount = 1000;
float FieldRadius = 100000.0f; // 100 km
float Tier0Radius = 1000.0f;   // 1 km
float Tier1Radius = 50000.0f;  // 50 km
float Tier2Radius = 200000.0f; // 200 km
```

**Key Functions:**
```cpp
void InitializeField(FVector Center, float Radius);
void UpdateAsteroidTiers(float DeltaTime);
void SpawnAsteroids(int32 Count);
```

---

### 2. AAsteroid (derived from AOrbitalBody)
**File:** `Source/Alexander/Public/AsteroidField/Asteroid.h`
**Purpose:** Individual asteroid with tier-specific physics

**Key Properties:**
```cpp
EAsteroidTier CurrentTier;
float DistanceToPlayer;
float HullDamagePerSecond = 50.0f;
bool bCanFragment = true;
```

**Key Functions:**
```cpp
void UpdateTier0_Critical(float DeltaTime);  // Full N-body
void UpdateTier1_Active(float DeltaTime);    // Simplified
void UpdateTier2_Distant(float DeltaTime);   // Kinematic
void OnTierChanged(EAsteroidTier OldTier, EAsteroidTier NewTier);
void FragmentAsteroid(FVector ImpactPoint, FVector ImpactNormal);
```

---

### 3. USpatialHashGrid
**File:** `Source/Alexander/Public/AsteroidField/SpatialHashGrid.h`
**Purpose:** O(N) collision detection (75x speedup over O(N²))

**Configuration:**
```cpp
float CellSize = 5000.0f; // 5 km cells
FIntVector GridDimensions = FIntVector(20, 20, 20);
```

**Key Functions:**
```cpp
void AddObject(AAsteroid* Asteroid);
TArray<AAsteroid*> GetNearbyAsteroids(FVector Location, float Radius);
FIntVector WorldToGridCoord(FVector WorldLocation) const;
```

**How It Works:**
```
1000 asteroids without grid: 1000×999/2 = 499,500 collision checks
1000 asteroids with grid:    1000×10 = 10,000 collision checks
Speedup: 50x faster!
```

---

### 4. UAsteroidPool
**File:** `Source/Alexander/Public/AsteroidField/AsteroidPool.h`
**Purpose:** Object pooling (avoid GC spikes)

**Configuration:**
```cpp
int32 InitialPoolSize = 1000;
int32 MaxPoolSize = 2000;
bool bAutoExpand = true;
```

**Usage Pattern:**
```cpp
// GOOD (fast):
AAsteroid* Asteroid = AsteroidPool->AcquireAsteroid(Location, Rotation);
// ... use asteroid ...
AsteroidPool->ReleaseAsteroid(Asteroid);

// BAD (slow, triggers GC):
AAsteroid* Asteroid = GetWorld()->SpawnActor<AAsteroid>(Location, Rotation);
// ... use asteroid ...
Asteroid->Destroy();
```

---

### 5. UAsteroidVFXManager
**File:** `Source/Alexander/Public/AsteroidField/AsteroidVFXManager.h`
**Purpose:** GPU instancing for 10,000+ distant asteroids

**Key Features:**
- Single draw call for all Tier 4 asteroids
- Per-instance size/color variation
- No CPU cost per instance

**Usage:**
```cpp
int32 InstanceIndex = VFXManager->AddInstance(Transform, Size, Color);
VFXManager->UpdateInstance(InstanceIndex, NewTransform);
VFXManager->RemoveInstance(InstanceIndex);
```

---

## Performance Budgets

**CPU Budget (per frame @ 60 FPS):**
```
Total frame budget:     16.67 ms
Asteroid system:         2.67 ms (16% of frame)
  ├─ Tier 0:             0.80 ms
  ├─ Tier 1:             0.80 ms
  ├─ Tier 2:             0.40 ms
  ├─ Tier 3:             0.10 ms
  ├─ Tier 4:             0.20 ms
  ├─ Spatial Grid:       0.20 ms
  └─ Tier Management:    0.17 ms
```

**Memory Budget:**
```
Total system memory:     8.4 MB
  ├─ Asteroids:          3.5 MB
  ├─ Object Pool:        4.0 MB
  ├─ Spatial Grid:       0.1 MB
  └─ GPU Instances:      0.8 MB
```

**Network Budget (per player):**
```
Asteroids:              17.6 kbps (shared)
Ships:                  25.96 kbps (per player)
Total (4 players):      137.6 kbps
```

---

## Ship Integration

### Hull Integrity System

**Component Stats (in ShipCustomizationSystem):**
```cpp
FShipPerformanceStats {
    float HullIntegrity; // 1.0 = standard, 2.0 = reinforced
    float TotalMass;     // kg
}
```

**Collision Damage Formula:**
```cpp
float ImpactEnergy = 0.5f * AsteroidMass * ImpactSpeed * ImpactSpeed;
float FinalDamage = ImpactEnergy / HullIntegrity;
```

**Example Hull Types:**
| Hull | Mass | HullIntegrity | Speed | Cost |
|------|------|---------------|-------|------|
| Lightweight | 500 kg | 0.5x | +10% | $1k |
| Standard | 1000 kg | 1.0x | 0% | $5k |
| Reinforced | 2000 kg | 1.5x | -5% | $15k |
| Military | 3000 kg | 2.0x | -10% | $50k |

---

## Network Replication Strategy

### Deterministic Generation (Zero Bandwidth!)

**Key Principle:** All clients generate same asteroids from shared seed

**Server → Clients (once at start):**
```cpp
struct FAsteroidFieldState {
    int32 RandomSeed;
    FVector FieldCenter;
    float FieldRadius;
    int32 TargetAsteroidCount;
};
```

**All clients execute:**
```cpp
RandomStream.Initialize(RandomSeed);
for (int32 i = 0; i < TargetAsteroidCount; ++i) {
    FVector Pos = RandomStream.VRandCone(...);
    float Mass = RandomStream.FRandRange(...);
    SpawnAsteroid(Pos, Mass); // Identical on all clients!
}
```

**Only Replicate Active Asteroids:**
- Tier 0: Always replicate (critical)
- Tier 1: Replicate if perturbed from deterministic path
- Tier 2+: Never replicate (kinematic)

**Bandwidth Savings:**
- Without determinism: 1000 asteroids × 22 bytes × 2 Hz = 44 KB/s
- With determinism: ~50 active asteroids × 22 bytes × 2 Hz = 2.2 KB/s
- **Savings: 95% bandwidth reduction!**

---

## Collision & Fragmentation

### Collision Detection by Tier

| Tier | Collision Type | Performance |
|------|----------------|-------------|
| Tier 0 | Full Rigid Body | Accurate, expensive |
| Tier 1 | Spatial Hash + Manual Impulse | Fast, good enough |
| Tier 2+ | None | No cost |

### Fragmentation Rules

```cpp
void FragmentAsteroid(FVector Impact, FVector Normal) {
    // 1. Check minimum mass
    if (Mass < 1000.0f) return;

    // 2. Check impact energy
    float ImpactEnergy = 0.5f * Mass * Speed * Speed;
    if (ImpactEnergy < 100000.0f) return;

    // 3. Spawn 2-5 fragments
    int32 NumFragments = FMath::RandRange(2, 5);
    for (int32 i = 0; i < NumFragments; ++i) {
        AAsteroid* Fragment = SpawnFragment();
        Fragment->Mass = Mass / NumFragments;
        Fragment->Radius = Radius * pow(1.0f/NumFragments, 1.0f/3.0f);
        Fragment->Velocity = Velocity + RandomDir * FMath::RandRange(20, 100);
    }

    // 4. Return parent to pool
    AsteroidPool->ReleaseAsteroid(this);
}
```

---

## LOD Strategy

| LOD | Distance | Triangles | Rendering |
|-----|----------|-----------|-----------|
| LOD0 | 0-1 km | 5,000 | Individual mesh |
| LOD1 | 1-10 km | 1,000 | Individual mesh |
| LOD2 | 10-50 km | 200 | Individual mesh |
| LOD3 | 50-200 km | 50 | Individual mesh |
| LOD4 | 200+ km | 8 (billboard) | GPU instanced |

**Memory Savings:**
- All LOD0: 1000 asteroids × 5k tris = 5M tris (~200 MB)
- Mixed LODs: 1000 asteroids × 500 avg = 500k tris (~20 MB)
- **10x memory reduction!**

---

## Tier Transition Logic

**Hysteresis to Prevent Oscillation:**
```cpp
const float UpgradeThreshold = 0.9f;   // Upgrade at 90% of distance
const float DowngradeThreshold = 1.1f; // Downgrade at 110% of distance

if (Distance < Tier1Radius * UpgradeThreshold && CurrentTier > Active)
    SetTier(Active); // Getting closer, upgrade early

if (Distance > Tier1Radius * DowngradeThreshold && CurrentTier == Active)
    SetTier(Distant); // Getting farther, downgrade late
```

**Smooth Visual Transition:**
```cpp
void OnTierChanged(EAsteroidTier OldTier, EAsteroidTier NewTier) {
    // Crossfade LOD over 0.5 seconds
    MeshComponent->SetScalarParameterValueOnMaterials("LODFade", 0.0f);
    // Animate LODFade from 0 to 1 over 0.5s
}
```

---

## Procedural Streaming

**Chunk-Based Loading (Like Minecraft):**
```cpp
const float ChunkSize = 50000.0f; // 50 km chunks
FIntVector PlayerChunk = WorldToChunkCoord(PlayerLocation);

// Load 7×7×7 = 343 chunks around player
for (int x = -3; x <= 3; ++x)
    for (int y = -3; y <= 3; ++y)
        for (int z = -3; z <= 3; ++z)
            LoadChunk(PlayerChunk + FIntVector(x,y,z));
```

**Deterministic Chunk Generation:**
```cpp
void LoadChunk(FIntVector ChunkCoord) {
    // Seed with chunk coordinates (deterministic)
    FRandomStream Random;
    Random.Initialize(Seed + ChunkCoord.X*73856093 + ChunkCoord.Y*19349663 + ChunkCoord.Z*83492791);

    // Generate asteroids for chunk
    for (int i = 0; i < AsteroidsPerChunk; ++i) {
        FVector Pos = Random.VRand() * ChunkSize;
        SpawnAsteroid(Pos);
    }
}
```

**Benefits:**
- Infinite asteroid fields
- Deterministic (multiplayer sync)
- Memory efficient (only ~300 chunks loaded)

---

## Implementation Checklist

### Phase 1: Core (Week 1)
- [ ] Create AAsteroid class (derive from AOrbitalBody)
- [ ] Create AAsteroidFieldManager
- [ ] Implement basic spawning (100 asteroids)
- [ ] Test integration with UCelestialBodyRegistry

### Phase 2: Tiers (Week 2)
- [ ] Implement tier assignment logic
- [ ] Implement Tier 0 (full N-body)
- [ ] Implement Tier 1 (simplified physics)
- [ ] Implement Tier 2 (kinematic)
- [ ] Add hysteresis for smooth transitions

### Phase 3: Spatial Grid (Week 3)
- [ ] Implement USpatialHashGrid
- [ ] Add/Remove/Update object functions
- [ ] Implement GetNearbyAsteroids()
- [ ] Benchmark collision detection

### Phase 4: Object Pool (Week 4)
- [ ] Implement UAsteroidPool
- [ ] Replace SpawnActor with AcquireAsteroid
- [ ] Test GC performance (should be zero spikes)

### Phase 5: Collision (Week 5)
- [ ] Implement ship-asteroid collision
- [ ] Implement asteroid-asteroid collision
- [ ] Implement fragmentation system
- [ ] Add VFX/SFX for impacts

### Phase 6: GPU Instancing (Week 6)
- [ ] Implement UAsteroidVFXManager
- [ ] Create instanced mesh component
- [ ] Transition Tier 4 to instancing
- [ ] Test with 10,000+ instances

### Phase 7: Networking (Week 7)
- [ ] Implement deterministic generation
- [ ] Implement compressed replication
- [ ] Test in multiplayer
- [ ] Optimize bandwidth

---

## Common Pitfalls & Solutions

### Pitfall 1: Tier Oscillation
**Problem:** Asteroid rapidly switches between tiers
**Solution:** Add 10% hysteresis buffer (upgrade at 90%, downgrade at 110%)

### Pitfall 2: Frame Spikes on Tier Transition
**Problem:** 100 asteroids activate physics simultaneously
**Solution:** Stagger activation over 10 frames (10 per frame)

### Pitfall 3: Collision Misses
**Problem:** Fast-moving asteroids tunnel through ships
**Solution:** Use continuous collision detection (CCD) for Tier 0/1

### Pitfall 4: Network Desync
**Problem:** Clients see different asteroid positions
**Solution:** Use deterministic generation with shared seed

### Pitfall 5: Memory Leaks
**Problem:** Fragments never cleaned up
**Solution:** Cap max fragments (1000), auto-cleanup oldest

---

## Debug Visualization

**Enable in AAsteroidFieldManager:**
```cpp
bool bShowDebugInfo = true;
```

**Console Commands:**
```
AsteroidField.ShowTiers 1        // Show tier colors
AsteroidField.ShowGrid 1         // Show spatial grid
AsteroidField.ShowPerformance 1  // Show frame timing
AsteroidField.ShowCollisions 1   // Show collision shapes
```

**On-Screen Stats:**
```
Tier 0: 15 (0.7ms)
Tier 1: 185 (0.6ms)
Tier 2: 380 (0.4ms)
Tier 3: 420 (0.1ms)
Total: 2.1ms / 2.67ms budget
```

---

## Performance Tuning

**If Over Budget:**
```cpp
// Reduce tier radii
Tier1Radius *= 0.75f; // Reduce by 25%
Tier2Radius *= 0.75f;

// Or reduce asteroid count
TargetAsteroidCount *= 0.8f; // Reduce by 20%

// Or increase cell size (fewer collision checks)
SpatialGrid->CellSize *= 1.5f;
```

**If Under Budget:**
```cpp
// Increase quality
Tier1Radius *= 1.1f; // Increase by 10%
TargetAsteroidCount *= 1.2f;
```

---

## Mathematical Formulas

**Collision Damage:**
```
ImpactEnergy = 0.5 × Mass × Velocity²
FinalDamage = ImpactEnergy / HullIntegrity
```

**Fragment Size:**
```
FragmentMass = ParentMass / NumFragments
FragmentRadius = ParentRadius × (1/NumFragments)^(1/3)
```

**Gravity Force (Tier 0):**
```
F = G × (m1 × m2) / r²
where G = 6.674×10⁻¹¹ m³/(kg·s²)
```

**Simplified Gravity (Tier 1):**
```
F = Sum(Force from each Tier 0 object)
Ignore asteroid-asteroid gravity
```

**Kinematic Motion (Tier 2):**
```
Position = InitialPosition + Velocity × Time
No force calculations
```

---

## File Structure

```
Source/Alexander/
├── AsteroidField/
│   ├── Public/
│   │   ├── AsteroidFieldManager.h      (400 lines)
│   │   ├── Asteroid.h                  (300 lines)
│   │   ├── SpatialHashGrid.h           (200 lines)
│   │   ├── AsteroidPool.h              (150 lines)
│   │   ├── AsteroidVFXManager.h        (150 lines)
│   │   └── AsteroidNetworkTypes.h      (200 lines)
│   └── Private/
│       ├── AsteroidFieldManager.cpp    (600 lines)
│       ├── Asteroid.cpp                (500 lines)
│       ├── SpatialHashGrid.cpp         (350 lines)
│       ├── AsteroidPool.cpp            (250 lines)
│       ├── AsteroidVFXManager.cpp      (250 lines)
│       └── AsteroidReplication.cpp     (300 lines)
└── Public/
    └── OrbitalBody.h (existing)        // AAsteroid derives from this
```

---

## Questions for User

Before implementation, clarify:

1. **Visual Style:** Realistic or stylized asteroids?
2. **Mining:** Can players mine asteroids for resources?
3. **Destructibility:** Can weapons destroy asteroids?
4. **Mesh Variety:** How many unique asteroid meshes? (Recommend 10-20)
5. **Cluster Locations:** Where should dense fields be? (Near planets? Around gates?)

---

**Ready to implement!** See full design doc for detailed architecture.
