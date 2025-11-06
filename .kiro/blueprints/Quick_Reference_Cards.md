# Quick Reference Cards - Blueprint Implementation

## Purpose
One-page reference cards for each Blueprint. Print or keep on second monitor during implementation.

---

## Card 1: BP_CelestialBodyBase

### Basic Info
- **Location:** `Content/Blueprints/CelestialBodies/BP_CelestialBodyBase`
- **Parent Class:** Actor
- **Purpose:** Parent class for all celestial bodies

### Components Hierarchy
```
DefaultSceneRoot
├── CelestialBodyComponent (C++ component)
├── VisualMesh (StaticMeshComponent)
│   └── Material: Set in child Blueprints
└── GravityZone (SphereCollisionComponent)
    └── Radius: 100000.0 cm (1 km default)
```

### Class Default Variables
```
From CelestialBodyComponent (if exposed):
  BodyID: "Body" (override in children)
  Mass: 1.0e24 kg (override in children)
  Radius: 6371.0 km (override in children)
  bEnableDynamicScaling: true
  bEnableGravity: true
  MinScaleFactor: 0.01
  MaxScaleFactor: 100.0
```

### Construction Script Pseudo-Code
```
Event Construction Script
→ Get CelestialBodyComponent
→ If valid:
  → Get CurrentScaleFactor from component
  → Set VisualMesh Scale: (ScaleFactor, ScaleFactor, ScaleFactor)
  → Get InfluenceRadius from component
  → Set GravityZone Sphere Radius: InfluenceRadius
```

### Event BeginPlay Pseudo-Code
```
Event BeginPlay
→ Call Parent: Super::BeginPlay()
→ Get CelestialBodyComponent
→ Log: "Celestial body initialized: " + BodyID
```

---

## Card 2: BP_Sun

### Basic Info
- **Location:** `Content/Blueprints/CelestialBodies/BP_Sun`
- **Parent Class:** BP_CelestialBodyBase
- **Purpose:** Central star, primary light source

### Class Default Values
```
BodyID: "Sun"
Mass: 1.98892e30
Radius: 696000.0
MinScaleFactor: 0.001
MaxScaleFactor: 1000.0
```

### VisualMesh Configuration
```
Static Mesh: SM_Sphere (Engine Content/BasicShapes)
Material: M_Sun_Emissive (create this)
```

### Additional Components
```
SunLight (PointLightComponent)
  Parent: VisualMesh
  Intensity: 1000000.0
  Attenuation Radius: 100000000.0 (1000 km)
  Light Color: RGB(255, 247, 230)
  Source Radius: 696000000.0 (Sun radius in cm)
  Cast Shadows: true
  Indirect Lighting Intensity: 1.0
```

### Material: M_Sun_Emissive
```
Material Type: Unlit or Emissive Lit
Base Color: RGB(1.0, 0.9, 0.6) - Warm yellow
Emissive Color: RGB(1.0, 0.9, 0.6)
Emissive Strength: 100.0
Shading Model: Unlit (or Default Lit with high emissive)
```

### Level Placement
```
Location: (0, 0, 0)
Rotation: (0, 0, 0)
Scale: (1, 1, 1)
```

---

## Card 3: BP_Earth

### Basic Info
- **Location:** `Content/Blueprints/CelestialBodies/BP_Earth`
- **Parent Class:** BP_CelestialBodyBase
- **Purpose:** Earth planet

### Class Default Values
```
BodyID: "Earth"
Mass: 5.97237e24
Radius: 6371.0
MinScaleFactor: 0.01
MaxScaleFactor: 100.0
```

### VisualMesh Configuration
```
Static Mesh: SM_Sphere
Material: M_Earth_Simple (create this)
```

### Material: M_Earth_Simple
```
Material Type: Lit
Base Color: RGB(0.1, 0.3, 0.8) - Blue (or use NASA texture)
Roughness: 0.4
Metallic: 0.0
Specular: 0.5
```

### Optional: SkyAtmosphere Component
```
AtmosphereComponent (SkyAtmosphereComponent)
  Parent: VisualMesh
  Atmosphere Height: 100 km
  Ground Radius: 6371 km
  Multi Scattering Factor: 1.0
```

### Level Placement
```
Location: (1000000000, 0, 0) cm
  = 10,000,000 meters
  = 10,000 km from Sun
Rotation: (0, 0, 0)
Scale: (1, 1, 1)
```

---

## Card 4: BP_Moon

### Basic Info
- **Location:** `Content/Blueprints/CelestialBodies/BP_Moon`
- **Parent Class:** BP_CelestialBodyBase
- **Purpose:** Earth's moon

### Class Default Values
```
BodyID: "Moon"
Mass: 7.342e22
Radius: 1737.4
MinScaleFactor: 0.01
MaxScaleFactor: 100.0
```

### VisualMesh Configuration
```
Static Mesh: SM_Sphere
Material: M_Moon_Simple (create this)
```

### Material: M_Moon_Simple
```
Material Type: Lit
Base Color: RGB(0.4, 0.4, 0.4) - Gray
Roughness: 0.9 (very rough, dusty)
Metallic: 0.0
Normal Map: (Optional) Crater texture
```

### Level Placement
```
Location: (1050000000, 0, 0) cm
  = 10,500,000 meters
  = 10,500 km from Sun
  = 500 km from Earth
Rotation: (0, 0, 0)
Scale: (1, 1, 1)
```

---

## Card 5: BP_SolarSystemGameMode

### Basic Info
- **Location:** `Content/Blueprints/GameModes/BP_SolarSystemGameMode`
- **Parent Class:** GameModeBase
- **Purpose:** Initialize all celestial scaling subsystems

### Class Settings
```
Default Pawn Class: BP_VRSpaceshipPlayer
HUD Class: None
Player Controller Class: PlayerController
Game State Class: GameStateBase
```

### Class Default Variables
```
Celestial Scaling:
  bEnableCelestialScaling: true
  bEnableOriginRecentering: true
  UniverseRecenterThreshold: 1000000.0 (10 km)

Gravity:
  GravityMode: MultiBody (enum)
  bEnableGravity: true
  MaxGForce: 50.0
  GravityUpdateFrequency: 60.0

Relativity:
  bEnableRelativisticEffects: true
  bEnableTimeDebt: true
  TimeWarpWarningThreshold: 10.0
  RelativityScaleFactor: 1.0

Debug:
  bShowDebugOverlay: true
  bEnableDebugVisualization: false
  bLogSubsystemInitialization: true
```

### Event BeginPlay (Simplified)
```
1. Call Parent BeginPlay
2. Log: "=== Solar System Game Mode Initializing ==="
3. Get Game Instance
4. Get Subsystems:
   - CelestialBodyRegistry (WorldSubsystem)
   - GravitySimulator (GameInstanceSubsystem)
   - RelativitySystem (GameInstanceSubsystem)
   - ScalingCalculator (GameInstanceSubsystem)
   - SafetySystem (GameInstanceSubsystem)
5. Configure GravitySimulator:
   - SetSimulationMode(GravityMode)
   - SetMaxGForce(MaxGForce)
   - SetGravityEnabled(bEnableGravity)
6. Configure RelativitySystem:
   - SetRelativisticEffectsEnabled(true)
   - SetTimeDebtEnabled(true)
7. Delay 0.5 seconds
8. Get Player Controller (index 0)
9. Register player with RelativitySystem
10. Create WBP_DebugOverlay (if bShowDebugOverlay)
11. Create WBP_Chronometer
12. Add both widgets to viewport
13. Log: "=== Solar System Game Mode Ready ==="
```

---

## Card 6: BP_VRSpaceshipPlayer

### Basic Info
- **Location:** `Content/Blueprints/Pawns/BP_VRSpaceshipPlayer`
- **Parent Class:** VRSpaceshipPawn (C++ class) ⚠️ IMPORTANT
- **Purpose:** VR player pawn with origin management

### ⚠️ CRITICAL: Reparenting Required
```
1. Open existing BP_VRSpaceshipPlayer
2. Class Settings → Parent Class
3. Change from current parent to: VRSpaceshipPawn
4. Confirm reparenting warning
5. Fix any broken connections
```

### Components (Inherited from C++)
```
VROrigin (SceneComponent) - Root, always at (0,0,0)
├── VRCamera (CameraComponent)
├── LeftHand (MotionControllerComponent)
├── RightHand (MotionControllerComponent)
├── CockpitMesh (StaticMeshComponent)
├── OriginManager (UPlayerOriginManager)
└── TimeComponent (UPlayerTimeComponent)
```

### Class Default Values
```
VR Settings:
  bEnableVR: true
  HandTrackingPrecision: 0.001

Spaceship:
  bAutoPossessSpaceship: true
  SpaceshipClass: BP_Spaceship (if exists)
  SpaceshipSearchRadius: 100000.0 (1 km)

Origin Management:
  PrecisionThreshold: 1000000.0 (10 km)
  bAutoRecenterUniverse: true
  bShowOriginDebugInfo: false
```

### CockpitMesh Configuration
```
Static Mesh: SM_Cube (placeholder)
Scale: (2.0, 2.0, 1.5)
Location: (50, 0, 0) - In front of player
Material: Semi-transparent glass
Collision: NoCollision
```

---

## Card 7: WBP_Chronometer

### Basic Info
- **Location:** `Content/UI/Widgets/WBP_Chronometer`
- **Parent Class:** UserWidget
- **Purpose:** Display three time clocks and time debt

### Widget Hierarchy
```
Canvas Panel (Root)
└── VerticalBox
    ├── TextBlock: "⏰ CHRONOMETER" (title)
    ├── HorizontalBox
    │   ├── TextBlock: "Real Time:"
    │   └── TextBlock (Name: RealTimeText) → BIND
    ├── HorizontalBox
    │   ├── TextBlock: "Universal Time:"
    │   └── TextBlock (Name: UniversalTimeText) → BIND
    ├── HorizontalBox
    │   ├── TextBlock: "Biological Age:"
    │   └── TextBlock (Name: BiologicalAgeText) → BIND
    ├── HorizontalBox
    │   ├── TextBlock: "Time Debt:"
    │   └── TextBlock (Name: TimeDebtText) → BIND
    └── ProgressBar (Name: TimeDilationBar) → BIND
```

### Variables
```
RelativitySystem (URelativitySystem*) - Reference
LocalPlayerController (APlayerController*) - Reference
```

### Event Construct
```
Event Construct
→ Get Game Instance
→ Get Subsystem: URelativitySystem
→ Store in RelativitySystem variable
→ Get Player Controller (index 0)
→ Store in LocalPlayerController
```

### Text Bindings (return FText)
```
RealTimeText:
  Get current real time → Format as HH:MM:SS → Return as Text

UniversalTimeText:
  Get RelativitySystem → GetUniversalTime() → Format → Return

BiologicalAgeText:
  Get RelativitySystem → GetPlayerAge(LocalPlayerController)
  → Format as "XX.XXXX years" → Return

TimeDebtText:
  Get RelativitySystem → GetPlayerTimeDebt(LocalPlayerController)
  → Format as "X.XXXX years" → Return

TimeDilationBar (return float 0.0-1.0):
  Get current time dilation factor
  → Clamp to 0-2 range
  → Return as percent (0.5-1.0)
```

---

## Card 8: WBP_DebugOverlay

### Basic Info
- **Location:** `Content/UI/Widgets/WBP_DebugOverlay`
- **Parent Class:** UserWidget
- **Purpose:** Display debug info for all subsystems

### Widget Hierarchy
```
Canvas Panel (Root)
└── Border (Background: semi-transparent black)
    └── VerticalBox
        ├── TextBlock: "CELESTIAL SCALING DEBUG" (title)
        ├── Spacer
        ├── TextBlock (Name: VirtualPositionText) → BIND
        ├── TextBlock (Name: RegisteredBodiesText) → BIND
        ├── TextBlock (Name: GravityInfoText) → BIND
        └── TextBlock (Name: PerformanceText) → BIND
```

### Variables
```
CelestialBodyRegistry (UCelestialBodyRegistry*)
GravitySimulator (UGravitySimulator*)
LocalPlayerPawn (AVRSpaceshipPawn*)
```

### Event Construct
```
Get World Subsystem: CelestialBodyRegistry
Get Game Instance Subsystem: GravitySimulator
Get Player Pawn (cast to AVRSpaceshipPawn)
```

### Event Tick (or text bindings)
```
Update all debug text every frame:

VirtualPositionText:
  Get PlayerPawn → OriginManager → GetVirtualPosition()
  Format: "Sector: (X,Y,Z)\nLocal: (X,Y,Z) km"

RegisteredBodiesText:
  Get CelestialBodyRegistry → GetRegisteredBodyCount()
  For each body: Name, Distance, Scale

GravityInfoText:
  Get GravitySimulator → Current mode, sources, total G-force

PerformanceText:
  Get current FPS, frame time
```

---

## Card 9: WBP_TimeWarpWarning

### Basic Info
- **Location:** `Content/UI/Widgets/WBP_TimeWarpWarning`
- **Parent Class:** UserWidget
- **Purpose:** Modal dialog for time warp confirmation

### Widget Hierarchy
```
Overlay (Root, full screen)
├── ColoredOverlay (dim background, 50% opacity black)
└── Canvas Panel
    └── Border (dialog box, centered)
        └── VerticalBox
            ├── TextBlock: "⚠ TIME WARP WARNING ⚠"
            ├── TextBlock (Name: WarpFactorText)
            ├── Spacer
            ├── TextBlock (Name: CurrentStatusText)
            ├── TextBlock (Name: PredictedStatusText)
            ├── TextBlock (Name: ConsequencesText)
            ├── Spacer
            └── HorizontalBox (buttons)
                ├── Button (Name: CancelButton)
                │   └── TextBlock: "Cancel"
                └── Button (Name: ActivateButton)
                    └── TextBlock: "Activate Time Warp"
```

### Variables
```
RequestedWarpFactor (float)
RelativitySystem (URelativitySystem*)
bWarningConfirmed (bool)
```

### Custom Event: InitializeWarning
```
Parameters: WarpFactor (float)

Store WarpFactor
Calculate predictions
Update all text displays
Show warning severity
```

### Button Events
```
CancelButton → OnClicked:
  Set bWarningConfirmed = false
  Remove from parent (close dialog)

ActivateButton → OnClicked:
  Set bWarningConfirmed = true
  Call RelativitySystem → ApplyTimeWarp(WarpFactor)
  Remove from parent
```

---

## Card 10: SolarSystem.umap Level Setup

### World Settings
```
Window → World Settings

Game Mode:
  Game Mode Override: BP_SolarSystemGameMode
  Default Pawn Class: BP_VRSpaceshipPlayer (auto-set)

World:
  Kill Z: -10000000.0
  World Bounds Extent: 10000000.0
```

### Actor Placement (EXACT COORDINATES)

```
BP_Sun:
  Location: (0, 0, 0)
  Rotation: (0, 0, 0)
  Scale: (1, 1, 1)

BP_Earth:
  Location: (1000000000, 0, 0)
  Rotation: (0, 0, 0)
  Scale: (1, 1, 1)

BP_Moon:
  Location: (1050000000, 0, 0)
  Rotation: (0, 0, 0)
  Scale: (1, 1, 1)

PlayerStart:
  Location: (1051000000, 0, 0)
  Rotation: (0, 180, 0)
  Scale: (1, 1, 1)

DirectionalLight (Sun_DirectionalLight):
  Location: (0, 0, 100000)
  Rotation: Point toward origin
  Intensity: 10.0
  Light Color: (255, 247, 230)
  Temperature: 5778
  Cast Shadows: Dynamic

PostProcessVolume:
  Location: (0, 0, 0)
  Infinite Extent: ✓ Checked
  Exposure: Manual
  Bloom Intensity: 1.5
  Motion Blur: Disabled
  Depth of Field: Disabled
```

### Outliner Organization
```
Folders:
  CelestialBodies/
    - BP_Sun
    - BP_Earth
    - BP_Moon
  Lighting/
    - Sun_DirectionalLight
  Environment/
    - PostProcessVolume
    - BP_SkySphere (if added)
  PlayerSpawns/
    - PlayerStart
```

---

## Material Quick Reference

### M_Sun_Emissive
```
Material Domain: Surface
Blend Mode: Opaque
Shading Model: Unlit

Nodes:
  Constant3Vector (1.0, 0.9, 0.6) → Base Color
  Constant3Vector (1.0, 0.9, 0.6) × Constant (100.0) → Emissive Color
```

### M_Earth_Simple
```
Material Domain: Surface
Blend Mode: Opaque
Shading Model: Default Lit

Nodes:
  Constant3Vector (0.1, 0.3, 0.8) → Base Color
  Constant (0.4) → Roughness
  Constant (0.0) → Metallic
  Constant (0.5) → Specular
```

### M_Moon_Simple
```
Material Domain: Surface
Blend Mode: Opaque
Shading Model: Default Lit

Nodes:
  Constant3Vector (0.4, 0.4, 0.4) → Base Color
  Constant (0.9) → Roughness
  Constant (0.0) → Metallic
```

---

## Verification Checklist

### Before Testing
- [ ] All Blueprints compile without errors
- [ ] All materials created and assigned
- [ ] All actors placed at EXACT coordinates
- [ ] World Settings configured
- [ ] Outliner organized

### During Play Test
- [ ] Output Log shows subsystem initialization
- [ ] "Registered Bodies: 3" appears
- [ ] Moon visible and large (close)
- [ ] Earth visible in distance
- [ ] Sun visible (bright star)
- [ ] Chronometer displays and updates
- [ ] Debug overlay shows (if enabled)
- [ ] Performance >60 FPS

---

**Created:** 2025-01-05
**Version:** 1.0
**Use:** Keep open during Blueprint implementation in Unreal Editor
