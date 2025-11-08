# Performance Budget - Ship Customization System

## Overview
This document defines the performance budget for all custom systems in the VR game. The game must maintain **90 FPS minimum** (11.1ms per frame budget) to ensure VR comfort and prevent motion sickness.

## Frame Budget Allocation (11.1ms total)

### Custom Systems Budget: 2.8ms (25.2%)
The following systems have dedicated performance budgets:

#### 1. Ship Customization Component: 0.5ms per frame
The ship customization component handles stat calculations, network replication, and visual updates.

- **Stat Calculation**: 0.1ms
  - Iterating through equipped parts
  - Summing stat modifiers
  - Network replication queries

- **Network Replication**: 0.2ms
  - Property replication (loadout)
  - Stat synchronization to clients
  - Event broadcasting

- **Visual Updates**: 0.2ms
  - Material updates
  - Mesh component modifications
  - VFX triggers

**Justification**: 8 equipment slots × 0.05ms average calculation = 0.4ms typical case

#### 2. N-Body Physics (per active ship): 0.3ms per frame
Gravity and orbital mechanics calculations per ship.

- **Gravity Force Calculation**: 0.1ms
  - Distance to other bodies
  - Mass-based force computation
  - Acceleration from gravity

- **Integration**: 0.1ms
  - Velocity integration
  - Position updates
  - Constraint application

- **Collision Detection**: 0.1ms
  - AABB queries
  - Sphere cast operations
  - Impact resolution

**Justification**: With up to 16 ships in multiplayer, total N-body overhead = 4.8ms (within 8.3ms engine budget)

#### 3. Asteroid Field System: 2.0ms per frame (1000 asteroids)
Manages dynamic asteroid spawning, culling, and physics updates.

- **Tier Management**: 0.5ms
  - Distance-based tier assignment
  - Spawn/despawn decisions
  - Memory pool management

- **Physics (Tier 0-2)**: 1.2ms
  - Tier 0: 100 asteroids fully simulated (0.8ms)
  - Tier 1: 300 asteroids simplified physics (0.3ms)
  - Tier 2: 600 asteroids disabled (0.1ms garbage collection)

- **Rendering**: 0.3ms
  - Batched draw calls
  - LOD culling
  - Instancing overhead

**Justification**: Tiered LOD system ensures scalability. On lower-end hardware, asteroid count can be reduced.

### Unreal Engine Systems: 8.3ms (74.8%)
Reserved for:
- Engine rendering (materials, lighting, post-processing)
- Input processing
- Physics engine (Chaos/PhysX)
- Animation systems
- Audio processing
- Garbage collection
- Platform-specific overhead

## Performance Thresholds

### Critical Performance Levels

| Metric | Green | Yellow | Red |
|--------|-------|--------|-----|
| Frame Time | < 11.0ms | 11.0-11.1ms | > 11.1ms |
| Average FPS | > 90 FPS | 89-90 FPS | < 89 FPS |
| 95th Percentile FPS | > 89 FPS | 88-89 FPS | < 88 FPS |
| Stat Calc Time | < 0.05ms | 0.05-0.1ms | > 0.1ms |
| Equip Part Time | < 0.3ms | 0.3-0.5ms | > 0.5ms |
| Ship Spawn Time | < 2.0ms | 2.0-3.0ms | > 3.0ms |

### Memory Budgets

| Component | Per Instance | Max Instances | Total Budget |
|-----------|--------------|---------------|--------------|
| Ship Customization | 40 KB | 16 | 640 KB |
| Asteroid (Tier 0-2) | 150 bytes | 1000 | 150 KB |
| Flight Controller | 30 KB | 16 | 480 KB |
| **Total Custom Systems** | | | **1.3 MB** |

## Key Functions and Their Budgets

### High-Priority Functions (< 0.1ms each)
```
CalculateTotalStats()        - 0.05ms (part iteration + addition)
GetEquippedPart()            - 0.02ms (map lookup)
GetPartData()                - 0.02ms (data table query)
```

### Medium-Priority Functions (0.1-0.3ms each)
```
EquipPart()                  - 0.15ms (validation + calculation)
ApplyStatsToFlightController() - 0.2ms (flight controller update)
UpdateShipVisuals()          - 0.25ms (mesh + material updates)
```

### Lower-Priority Functions (can exceed budget if called infrequently)
```
LoadLoadout()                - 1.0ms (allowed, called rarely)
SaveCustomizationData()      - 1.0ms (allowed, called on exit)
```

## Multi-Instance Scenarios

### 16 Multiplayer Ships (Worst Case)
- Stat calculation: 16 ships × 0.05ms = 0.8ms
- N-body physics: 16 ships × 0.3ms = 4.8ms
- Replication overhead: ~0.4ms
- **Total: 6.0ms** (within 8.3ms engine budget, leaves 2.3ms headroom)

### 1000 Asteroids with 16 Ships
- Asteroid field: 2.0ms
- All ships: 6.0ms
- **Total: 8.0ms** (just within 8.3ms budget)
- Note: This is worst-case. Typical scenario would be fewer asteroids or fewer ships.

## Profiling Strategy

### Continuous Monitoring
1. Enable profiling at development start
2. Run `stat ShipCustomization` in console
3. Monitor key metrics during development
4. Alert when any metric exceeds budget

### Testing Checkpoints
- **After each feature addition**: Run performance test
- **Weekly**: Generate performance regression report
- **Before release**: Full performance profiling with all systems active

### Tools Used
- Unreal Insights (UnrealInsights.exe) for detailed frame analysis
- `stat ShipCustomization` console command for real-time metrics
- Custom performance monitoring daemon
- Automated regression testing suite

## Optimization Opportunities (Future)

### Easy Wins (< 1 hour each)
1. Cache flight controller reference (already done)
2. Cache data table lookups
3. Use TArray instead of TMap for categories

### Medium Effort (2-4 hours each)
1. Implement stat calculation caching (recalculate on equip, not every frame)
2. Batch network replication for multiple ships
3. Use object pooling for asteroid creation

### Hard Optimization (8+ hours each)
1. SIMD vectorization of stat calculations
2. Parallel N-body computation
3. GPU-driven asteroid culling

## Performance Testing Schedule

- **Iteration after each major feature**: Run profiling tests
- **Weekly**: Generate regression reports
- **Before Phase transition**: Full profiling with 100% feature parity

## Success Criteria

✓ All functions stay within budget 95% of the time
✓ 90 FPS minimum maintained with 16 ships + 1000 asteroids
✓ Memory per ship < 50 KB
✓ No memory leaks (verified monthly)
✓ Network bandwidth < 100 kbps per client
✓ Frame time variance < 3ms (smooth frame pacing)

---

**Last Updated**: November 7, 2025
**Status**: ACTIVE
**Next Review**: After ship customization implementation complete
