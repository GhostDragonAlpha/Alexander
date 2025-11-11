# Planet Gameplay - Walk, Enter Ship, Fly

## Quick Summary

Your project already has:
- ✅ `APlanet` class with gravity (inherits from `AOrbitalBody`)
- ✅ Planet mesh, atmosphere, weather systems
- ✅ Ship flight system working (4/4 tests passing)
- ⏳ Need: Planet spawning + player start + ship entry

## Complete Gameplay Loop

**Start on Planet → Walk Around → Enter Ship → Fly Off → Land on Another Planet**

### Step 1: Spawn a Planet with Gravity (2 minutes)

In Unreal Editor Python console:
```python
py Content/Python/spawn_planets.py single
```

This spawns Earth at origin with:
- PlanetRadius: 6371 km
- Gravity from OrbitalBody (already has Mass property)
- Mesh for collision

### Step 2: Add Player Start (1 minute)

1. In Unreal Editor, place a **Player Start** actor
2. Position it at: `(0, 0, 650000)` - 6.5km above planet surface
3. Or on surface: `(0, 0, 6371000)` - at planet radius

### Step 3: Place Your Ship on the Ground (1 minute)

Option A: Manual Placement
- Drag `BP_VRSpaceshipPlayer` into level
- Position at `(100000, 0, 6371000)` - 1km away on surface

Option B: Via API
```bash
curl -X POST http://localhost:8080/spawn_ship \
  -d '{"location": [100000, 0, 6371000], "rotation": [0, 0, 0]}'
```

### Step 4: Setup Ship Entry System

Your ship needs an entry trigger. Quick Blueprint setup in `BP_VRSpaceshipPlayer`:

```
Add Component: Box Collision (name: EntryTrigger)
  - Size: 200x200x300 cm
  - Position: Near cockpit door

Event BeginOverlap (EntryTrigger)
  → Cast to Character
  → If valid:
      → Possess This Ship
      → Attach Player Camera to Cockpit
```

### Step 5: Test the Full Loop

1. Press Play (PIE)
2. You spawn as character on/near planet
3. Walk to ship using WASD
4. Approach ship → automatically enter
5. Fly using ship controls
6. Fly to another planet!

## Current Status

### ✅ Working Now
- Planet class with OrbitalBody gravity
- Ship flight (ThrustPower, Velocity, Rotation)
- Ship customization API
- Multiple planets can spawn

### ⏳ Quick Additions Needed (10-15 minutes)
1. **Ship Entry Trigger** - Add BoxCollision + possess on overlap
2. **Player Spawn** - Add PlayerStart to level
3. **Ground Collision** - Ensure planet mesh has collision enabled

## Testing Commands

**Spawn Planet:**
```python
# In Unreal Editor console
py Content/Python/spawn_planets.py single
```

**Spawn Ship on Planet Surface:**
```bash
curl -X POST http://localhost:8080/spawn_ship \
  -d '{
    "location": [0, 0, 6371000],
    "rotation": [0, 0, 0],
    "ship_class": "/Game/SpaceShip/Blueprints/BP_VRSpaceshipPlayer.BP_VRSpaceshipPlayer_C"
  }'
```

**Test Gravity:**
- Ships/objects should fall toward planet center due to OrbitalBody gravity
- Default gravity equation: F = G * M1 * M2 / r²

## Gravity Already Implemented

Your `OrbitalBody` class (parent of Planet) has:
- `Mass` property
- `CalculateGravitationalForce()` function
- Automatic physics integration

**Planet Default Mass**: ~5.97 × 10²⁴ kg (Earth)

All actors near the planet will experience gravitational pull toward center.

## Simiple Setup (5 Steps, 5 Minutes)

1. **Spawn planet**: `py Content/Python/spawn_planets.py single`
2. **Enable planet collision**: Planet mesh → Collision Preset = BlockAll
3. **Add PlayerStart**: At (0, 0, 6400000) - slightly above surface
4. **Add ship on ground**: At (1000, 0, 6371000)
5. **Test**: Press Play → Walk → Should feel gravity → Enter ship → Fly!

## Next: Create Complete Test Level

Want me to create a Blueprint that automatically sets up:
- Planet with gravity
- Player spawn point
- Ship on landing pad
- Entry/exit system
- HUD showing "Press E to enter ship"

This would be a complete "Planet to Space" gameplay demo level.
