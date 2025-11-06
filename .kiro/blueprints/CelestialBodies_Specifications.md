# Celestial Bodies Blueprint Specifications

Quick reference for creating Sun, Earth, Moon, and Mars Blueprints.

---

## BP_Sun

**Location:** `Content/Blueprints/CelestialBodies/BP_Sun`
**Parent:** BP_CelestialBodyBase

### Instance Properties
```
BodyID: "Sun"
Mass: 1.98892e30 (kg)
Radius: 696000.0 (km)
bEnableDynamicScaling: true
bEnableGravity: true
MinScaleFactor: 0.001
MaxScaleFactor: 1000.0
BodyMaterial: M_Sun_Animated (or M_Sun_Emissive)
```

### Additional Components

**SunLight (PointLightComponent)**
```
Parent: VisualMesh
Intensity: 1,000,000 (very bright)
Attenuation Radius: 100,000,000 cm (1000 km)
Light Color: (255, 247, 230) - Warm white, 5778K
Cast Shadows: Yes (dynamic)
Source Radius: 696,000,000 cm (matches Sun radius)
```

**SunCorona (NiagaraComponent)** - Optional
```
Parent: VisualMesh
Niagara System: NS_SunCorona
Auto Activate: true
Scale: Matches Sun radius
```

**SolarFlares (NiagaraComponent)** - Optional
```
Parent: VisualMesh
Niagara System: NS_SolarFlares
Auto Activate: true
Spawn Rate: Low (performance)
```

### Construction Script Additions
```
After parent Construction Script:
1. Update SunLight source radius to match current scale
2. Scale Niagara effects to match Sun radius
3. Set emissive intensity based on distance from player (optional)
```

### Level Placement
```
Position: (0, 0, 0) - World origin
Rotation: (0, 0, 0)
Purpose: Central star, provides light and primary gravity
```

---

## BP_Earth

**Location:** `Content/Blueprints/CelestialBodies/BP_Earth`
**Parent:** BP_CelestialBodyBase

### Instance Properties
```
BodyID: "Earth"
Mass: 5.97237e24 (kg)
Radius: 6371.0 (km)
bEnableDynamicScaling: true
bEnableGravity: true
MinScaleFactor: 0.01
MaxScaleFactor: 100.0
BodyMaterial: M_Planet_Base (with Earth texture)
```

### Additional Components

**Atmosphere (SkyAtmosphereComponent)** - Optional but recommended
```
Parent: VisualMesh
Atmosphere Height: 100 km (scaled to match Earth)
Ground Radius: 6371 km
Rayleigh Scattering Scale: 8.0 km
Mie Scattering Scale: 1.2 km
Multi Scattering Factor: 1.0
Colors: Earth-like (blue sky)
```

**CloudLayer (StaticMeshComponent)** - Optional
```
Parent: VisualMesh
Static Mesh: SM_Sphere (slightly larger than Earth)
Scale: 1.01 (1% larger than Earth)
Material: M_Clouds_Animated (transparent, scrolling)
```

### Material Configuration
```
Material: Create Material Instance from M_Planet_Base
Parameters:
- BaseColor Map: T_Earth_Diffuse (8K recommended)
- Normal Map: T_Earth_Normal
- Roughness Map: T_Earth_Roughness (oceans smooth, land rough)
- Emissive Map: T_Earth_Lights (city lights on night side)
```

### Level Placement (Compressed Distances)
```
Position: (1000000000, 0, 0) cm = 10,000 km from Sun
Rotation: (0, 0, 0)
Initial Rotation Rate: (0, 0, 360/86400) deg/sec for 24h day
```

### Level Placement (Semi-Realistic Distances)
```
Position: (1000000000000, 0, 0) cm = 10,000,000 km from Sun
```

---

## BP_Moon

**Location:** `Content/Blueprints/CelestialBodies/BP_Moon`
**Parent:** BP_CelestialBodyBase

### Instance Properties
```
BodyID: "Moon"
Mass: 7.342e22 (kg)
Radius: 1737.4 (km)
bEnableDynamicScaling: true
bEnableGravity: true
MinScaleFactor: 0.01
MaxScaleFactor: 100.0
BodyMaterial: M_Planet_Base (gray, cratered)
```

### Material Configuration
```
Material: Create Material Instance from M_Planet_Base
Parameters:
- BaseColor: Gray (RGB: 0.4, 0.4, 0.4)
- BaseColor Map: T_Moon_Diffuse (crater texture)
- Normal Map: T_Moon_Normal (heavy cratering)
- Roughness: 0.9 (very rough, dusty regolith)
- Metallic: 0.0 (non-metallic rock)
```

### Level Placement (Compressed Distances)
```
Position: (1050000000, 0, 0) cm = 500 km from Earth
Rotation: (0, 0, 0)
Orbital Parent: Earth (if using orbital mechanics)
```

### Level Placement (Semi-Realistic Distances)
```
Position: (1004000000000, 0, 0) cm = 40,000 km from Earth
```

### Recommended Spawn Location
```
Player Start should be near Moon:
Position: (1051000000, 0, 0) cm = 10 km from Moon surface
Reason: Close enough to see Moon clearly, far enough to test scaling
```

---

## BP_Mars

**Location:** `Content/Blueprints/CelestialBodies/BP_Mars`
**Parent:** BP_CelestialBodyBase

### Instance Properties
```
BodyID: "Mars"
Mass: 6.4171e23 (kg)
Radius: 3389.5 (km)
bEnableDynamicScaling: true
bEnableGravity: true
MinScaleFactor: 0.01
MaxScaleFactor: 100.0
BodyMaterial: M_Planet_Base (red/orange)
```

### Material Configuration
```
Material: Create Material Instance from M_Planet_Base
Parameters:
- BaseColor: Red-orange (RGB: 0.8, 0.4, 0.2)
- BaseColor Map: T_Mars_Diffuse (rust-colored)
- Normal Map: T_Mars_Normal (valleys, mountains)
- Roughness: 0.85 (dusty surface)
- Emissive: None (no atmosphere)
```

### Additional Components

**ThinAtmosphere (SkyAtmosphereComponent)** - Optional
```
Parent: VisualMesh
Atmosphere Height: 50 km
Ground Radius: 3389.5 km
Density: 0.01 (very thin, 1% of Earth)
Color: Slight orange tint
```

### Level Placement (Compressed Distances)
```
Position: (1500000000, 0, 0) cm = 5,000 km from Earth
Rotation: (0, 0, 0)
```

### Level Placement (Semi-Realistic Distances)
```
Position: (15000000000000, 0, 0) cm = 150,000,000 km from Sun
```

---

## BP_Jupiter (Optional - Future Phase)

**Location:** `Content/Blueprints/CelestialBodies/BP_Jupiter`
**Parent:** BP_CelestialBodyBase

### Instance Properties
```
BodyID: "Jupiter"
Mass: 1.8982e27 (kg)
Radius: 69911.0 (km)
bEnableDynamicScaling: true
bEnableGravity: true
MinScaleFactor: 0.001
MaxScaleFactor: 500.0
BodyMaterial: M_Planet_Base (gas giant)
```

### Material Configuration
```
Material: Create Material Instance from M_Planet_Base
Parameters:
- BaseColor Map: T_Jupiter_Diffuse (banded atmosphere)
- Normal Map: T_Jupiter_Normal (storm features)
- Animated UV: Yes (cloud rotation)
- Great Red Spot: Custom emissive detail
```

### Level Placement (Very Far)
```
Position: (3000000000, 0, 0) cm = 20,000 km from Earth (compressed)
Note: In reality 778 million km from Sun - too far for initial testing
```

---

## Quick Setup Guide

### For each celestial body:

1. **Create Blueprint**
   - Right-click in Content Browser
   - Blueprint Class → Select BP_CelestialBodyBase as parent
   - Name: BP_Sun / BP_Earth / BP_Moon / BP_Mars

2. **Set Properties**
   - Open Blueprint
   - In Details panel, set BodyID, Mass, Radius
   - Assign appropriate BodyMaterial

3. **Add Special Components** (if needed)
   - Sun: Add PointLight and Niagara effects
   - Earth: Add SkyAtmosphere
   - Mars: Add thin atmosphere (optional)

4. **Compile & Save**
   - Click Compile button
   - Save Blueprint
   - Check for errors in Compiler Results

5. **Place in Level**
   - Drag from Content Browser into SolarSystem.umap
   - Set exact position in Details panel (Transform → Location)
   - Verify placement in viewport

---

## Visual Scale Reference

At different distances, bodies will appear as:

### Sun (Radius: 696,000 km)
- At 10,000 km: Enormous, fills field of view
- At 100,000 km: Very large sphere
- At 1,000,000 km: Bright disk
- At 10,000,000 km: Bright star

### Earth (Radius: 6,371 km)
- At 100 km: Huge planet below
- At 1,000 km: Large sphere
- At 10,000 km: Medium sphere
- At 100,000 km: Small sphere
- At 1,000,000 km: Tiny dot

### Moon (Radius: 1,737 km)
- At 10 km: Fills field of view (landing approach)
- At 100 km: Large sphere
- At 1,000 km: Medium sphere
- At 10,000 km: Small sphere
- At 100,000 km: Tiny dot

---

## Material Creation Notes

If planet textures don't exist, you can:

1. **Use free NASA textures:**
   - Earth: https://visibleearth.nasa.gov/
   - Moon: https://svs.gsfc.nasa.gov/cgi-bin/details.cgi?aid=4720
   - Mars: https://www.nasa.gov/mars-texture-maps

2. **Create procedural materials:**
   - Use noise functions in material editor
   - Combine multiple noise layers
   - Earth: Blue base + green continents
   - Moon: Gray base + crater normal map
   - Mars: Orange-red base + dark patches

3. **Placeholder materials:**
   - Use solid colors initially
   - Earth: Blue (RGB: 0.1, 0.3, 0.8)
   - Moon: Gray (RGB: 0.4, 0.4, 0.4)
   - Mars: Red-orange (RGB: 0.8, 0.4, 0.2)
   - Sun: Emissive yellow (RGB: 1.0, 0.9, 0.6, Emissive: 100)

---

## Testing Each Body

After creating each Blueprint:

### BP_Sun
- [ ] Placed at origin (0, 0, 0)
- [ ] Visible as large emissive sphere
- [ ] PointLight illuminates scene
- [ ] Niagara corona effect active (if added)
- [ ] Scales correctly when moving away

### BP_Earth
- [ ] Placed at correct distance from Sun
- [ ] Blue appearance with continents (if textured)
- [ ] Atmosphere visible (if added)
- [ ] Scales smoothly when approaching
- [ ] Gravity pulls player toward it

### BP_Moon
- [ ] Placed near Earth
- [ ] Gray cratered appearance
- [ ] Scales correctly
- [ ] Good spawn location nearby
- [ ] Gravity weaker than Earth

### BP_Mars
- [ ] Placed beyond Earth
- [ ] Red-orange appearance
- [ ] Thin atmosphere (if added)
- [ ] Scales correctly
- [ ] Gravity between Moon and Earth

---

**Created:** 2025-01-05
**Version:** 1.0
