# BP_CelestialBodyBase - Parent Blueprint Specification

## Overview
Parent Blueprint for all celestial bodies (Sun, planets, moons). Provides common scaling and gravity functionality.

---

## Blueprint Setup

**Blueprint Location:** `Content/Blueprints/CelestialBodies/BP_CelestialBodyBase`

**Parent Class:** `Actor`

---

## Components

### 1. CelestialBodyComponent (Inherited from C++)
```
Component Name: CelestialBodyComponent
Type: UCelestialBodyComponent (C++ class from CelestialScaling module)
Location: Automatically added when Blueprint is based on Actor with component

Properties to Expose:
- BodyID (String): Unique identifier for this celestial body
- Mass (Float): Mass in kilograms
- Radius (Float): Radius in kilometers
- bEnableDynamicScaling (Boolean): Enable distance-based scaling
- bEnableGravity (Boolean): Enable gravitational calculations
- MinScaleFactor (Float): Minimum visual scale (default: 0.01)
- MaxScaleFactor (Float): Maximum visual scale (default: 100.0)
- ScaleTransitionSpeed (Float): Speed of scale transitions (default: 2.0)
```

### 2. VisualMesh (Static Mesh Component)
```
Component Name: VisualMesh
Type: StaticMeshComponent
Parent: RootComponent

Default Settings:
- Static Mesh: SM_Sphere (default Unreal sphere)
- Scale: (1.0, 1.0, 1.0) - Will be controlled by CelestialBodyComponent
- Collision: NoCollision (physics handled separately)
- Cast Shadow: Yes
- Receive Decals: No
```

### 3. GravityZone (Sphere Component) - Optional
```
Component Name: GravityZone
Type: SphereComponent
Parent: RootComponent

Settings:
- Sphere Radius: Set to body radius (will be updated dynamically)
- Collision: OverlapAllDynamic
- Generate Overlap Events: Yes
- Purpose: Trigger for entering sphere of influence
```

---

## Variables

### Exposed Instance Editable Variables

```cpp
// Basic Body Properties
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial Body")
FString BodyID = "UnnamedBody";

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial Body")
float Mass = 1.0e24; // Default: Earth-like mass

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Celestial Body")
float Radius = 6371.0; // Default: Earth radius in km

// Scaling Configuration
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
bool bEnableDynamicScaling = true;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
float MinScaleFactor = 0.01;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
float MaxScaleFactor = 100.0;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scaling")
float ScaleTransitionSpeed = 2.0;

// Gravity Configuration
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity")
bool bEnableGravity = true;

// Visual Properties
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
UMaterialInterface* BodyMaterial = nullptr;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
bool bCastDynamicShadow = true;

// Debug
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
bool bShowDebugInfo = false;
```

---

## Blueprint Graph Functions

### Construction Script
```
Purpose: Initialize mesh and material

Steps:
1. Get VisualMesh component reference
2. If BodyMaterial is set:
   - Set material on VisualMesh (index 0)
3. Calculate mesh scale based on Radius:
   - Unreal sphere default radius is 50 cm
   - Target radius in cm: Radius * 100000 (km to cm)
   - Scale = (Radius * 100000) / 50
   - Set VisualMesh->SetWorldScale3D((Scale, Scale, Scale))
4. If GravityZone exists:
   - Set sphere radius to Radius * 100000 (match visual)
```

### BeginPlay (Event)
```
Purpose: Initialize CelestialBodyComponent and register with subsystem

Steps:
1. Call parent BeginPlay
2. Get CelestialBodyComponent reference
3. Set component properties:
   - SetMass(Mass)
   - SetRadius(Radius)
   - SetBodyID(BodyID)
   - SetEnableDynamicScaling(bEnableDynamicScaling)
   - SetEnableGravity(bEnableGravity)
   - SetMinScaleFactor(MinScaleFactor)
   - SetMaxScaleFactor(MaxScaleFactor)
   - SetScaleTransitionSpeed(ScaleTransitionSpeed)
4. Set VisualMesh reference:
   - CelestialBodyComponent->SetVisualMesh(VisualMesh)
5. Component will auto-register with CelestialBodyRegistry
6. If bShowDebugInfo:
   - Print "Celestial Body Initialized: [BodyID]"
```

### Tick (Event) - Optional
```
Purpose: Update debug visualization

Steps:
1. If bShowDebugInfo:
   - Get current scale factor from CelestialBodyComponent
   - Draw debug sphere at body location with current radius
   - Draw debug text showing:
     - Body ID
     - Current scale: X.XXx
     - Distance to player: XXX km
```

### GetCurrentScaleFactor (Function - Blueprint Callable)
```
Return Type: Float
Purpose: Query current visual scale

Steps:
1. Get CelestialBodyComponent reference
2. Return: CelestialBodyComponent->GetCurrentScaleFactor()
```

### GetDistanceToPlayer (Function - Blueprint Callable)
```
Return Type: Float
Purpose: Get distance to player in kilometers

Steps:
1. Get CelestialBodyComponent reference
2. Return: CelestialBodyComponent->GetDistanceToPlayer()
```

---

## Material Slot Configuration

```
Slot 0: Body Surface Material
- Default: M_Planet_Base
- Override in child Blueprints for specific bodies
```

---

## Collision Configuration

```
VisualMesh Collision:
- Collision Presets: NoCollision
- Reason: Visual representation only, physics handled by components

GravityZone Collision (if used):
- Collision Presets: OverlapAllDynamic
- Object Type: WorldDynamic
- Response to Pawn: Overlap
```

---

## Network Replication

```
Actor Settings:
- Replicates: TRUE
- Replicate Movement: FALSE (movement handled by universe translation)

Component Replication:
- CelestialBodyComponent handles own replication (configured in C++)
- Properties replicated:
  - CurrentScaleFactor
  - TargetScaleFactor
  - DistanceToPlayer
  - Mass
  - Radius
```

---

## Usage Example

Once BP_CelestialBodyBase is created, child Blueprints inherit all functionality:

```
1. Create new Blueprint
2. Parent Class: BP_CelestialBodyBase
3. Set unique BodyID (e.g., "Earth")
4. Set Mass and Radius from AstronomicalConstants
5. Override BodyMaterial with planet-specific material
6. Done! Scaling and gravity work automatically
```

---

## Testing Checklist

After creating BP_CelestialBodyBase:

- [ ] Blueprint compiles without errors
- [ ] Can place in level
- [ ] CelestialBodyComponent is present and configured
- [ ] VisualMesh is visible in viewport
- [ ] Material applies correctly
- [ ] Can set BodyID, Mass, Radius in Details panel
- [ ] BeginPlay initializes component (check Output Log)
- [ ] bShowDebugInfo displays debug sphere when enabled

---

## Common Issues & Solutions

**Issue: CelestialBodyComponent not found**
- Solution: Ensure C++ code is compiled and Alexander.Build.cs includes CelestialScaling module

**Issue: Mesh appears wrong size**
- Solution: Check Construction Script scale calculation
- Verify Radius is in kilometers
- Ensure VisualMesh scale is set correctly

**Issue: Component not registering with CelestialBodyRegistry**
- Solution: Check BeginPlay is called
- Verify CelestialBodyRegistry subsystem is initialized
- Check game mode creates subsystems properly

**Issue: Scaling doesn't work**
- Solution: Verify bEnableDynamicScaling = true
- Check VisualMesh reference is set in BeginPlay
- Ensure player has PlayerOriginManager component
- Verify distance calculations are working

---

## Performance Considerations

- **Mesh Complexity:** Use LODs for distant bodies
- **Material Complexity:** Keep shader complexity reasonable for VR
- **Update Frequency:** CelestialBodyComponent updates at 10Hz by default
- **Collision:** Disabled on visual mesh to avoid overhead
- **Debug Visualization:** Only enable during development

---

## Next Steps

After creating BP_CelestialBodyBase:
1. Create BP_Sun (child of BP_CelestialBodyBase)
2. Create BP_Earth (child of BP_CelestialBodyBase)
3. Create BP_Moon (child of BP_CelestialBodyBase)
4. Test in SolarSystem.umap

---

**Created:** 2025-01-05
**Last Updated:** 2025-01-05
**Version:** 1.0
