# Alexander Autonomous Baseline - Knowledge Base

Comprehensive conceptual guide, best practices, architecture overview, and tools reference for the Alexander autonomous baseline system.

---

## Table of Contents

1. [Core Concepts](#core-concepts)
2. [Best Practices](#best-practices)
3. [System Architecture](#system-architecture)
4. [Tools Reference](#tools-reference)
5. [Development Workflows](#development-workflows)
6. [Performance Guidelines](#performance-guidelines)

---

## Core Concepts

### What is CSG Primitive Modeling?

**Definition**: Constructive Solid Geometry (CSG) is a technique for creating complex shapes by combining simple primitive shapes using boolean operations.

**Primitive shapes**:
- **Box/Cube**: Rectangular solid with width, height, depth
- **Sphere**: Perfect ball with radius
- **Cylinder**: Circular column with radius and height
- **Cone**: Tapered cylinder
- **Plane**: Flat 2D surface

**Boolean operations**:
- **Union**: Combine shapes into one
- **Subtract**: Cut one shape from another
- **Intersect**: Keep only overlapping portions

**In Alexander**:
```cpp
// Creating a simple station module using CSG
UCsgBuilder* Builder = NewObject<UCsgBuilder>();

// Main hull (box)
Builder->AddBox(FVector(1000, 1000, 500));

// Subtract docking port (cylinder)
Builder->SubtractCylinder(
    FVector(1000, 0, 0),    // Position
    100.f,                   // Radius
    200.f,                   // Height
    ESide::Horizontal
);

// Result: Box with cylindrical hole for docking
```

**Advantages**:
- Fast to create (no 3D modeling software needed)
- Easy to modify (change parameters)
- Lightweight geometry (simple shapes)
- Perfect for prototyping

**Limitations**:
- Limited visual appeal (geometric look)
- No organic shapes
- Can't have texture detail baked in
- Not suitable for final production art

**Use cases in baseline**:
- Station exteriors and interiors
- Docking ports and airlocks
- Simple props and furniture
- Collision volumes
- Blocking out level layouts

**Replacement strategy**:
1. Create CSG prototype
2. Test gameplay with primitives
3. Lock down dimensions and positions
4. Commission final 3D art to match CSG bounds
5. Swap CSG with detailed mesh
6. Keep CSG collision volumes

---

### What is Niagara vs Cascade?

**Cascade** (Legacy particle system):
- Older Unreal particle system (pre-UE4.20)
- CPU-based simulation
- Limited to particle emitters
- Being phased out

**Niagara** (Modern particle system):
- Introduced in UE4.20, matured in UE5
- GPU-accelerated simulation
- Supports particles, ribbons, meshes, lights
- Modular and extensible
- Visual scripting with modules

**Key differences**:

| Feature | Cascade | Niagara |
|---------|---------|---------|
| Performance | CPU-only | GPU or CPU |
| Max particles | ~10,000 | 100,000+ |
| Workflow | Fixed modules | Custom modules |
| Data flow | Linear | Graph-based |
| Events | Limited | Full event system |
| Debugging | Minimal | Comprehensive |
| Status | Deprecated | Current standard |

**In Alexander baseline**:
```cpp
// All VFX use Niagara
UNiagaraSystem* EngineTrail = LoadObject<UNiagaraSystem>(
    nullptr,
    TEXT("/Game/VFX/NS_EngineTrail")
);

// Spawn and configure
UNiagaraComponent* Trail = UNiagaraFunctionLibrary::SpawnSystemAttached(
    EngineTrail,
    ShipMesh,
    TEXT("EngineSocket"),
    FVector::ZeroVector,
    FRotator::ZeroRotator,
    EAttachLocation::KeepRelativeOffset,
    true  // Auto-destroy
);

// Set parameters
Trail->SetFloatParameter("EmissionRate", 500.f);
Trail->SetVectorParameter("TrailColor", FVector(0.2f, 0.5f, 1.0f));
```

**Niagara structure**:
```
UNiagaraSystem (Asset)
├── Emitter 1 (Particle Spawner)
│   ├── Emitter Spawn (initialization)
│   ├── Emitter Update (per-frame logic)
│   ├── Particle Spawn (new particle setup)
│   └── Particle Update (existing particle logic)
├── Emitter 2 (Ribbon Trail)
└── System Scripts (coordination)
```

**Common Niagara modules**:
- **Spawn Rate**: How many particles per second
- **Lifetime**: How long particles exist
- **Color**: Particle color over lifetime
- **Size**: Particle scale over lifetime
- **Velocity**: Initial/ongoing movement
- **Forces**: Gravity, wind, attraction
- **Collision**: Bounce off world geometry

**Best practices**:
- Use GPU emitters for large particle counts
- Use CPU emitters when you need precise collision
- Pool Niagara components (don't spawn/destroy constantly)
- Set MaxDistance to cull distant VFX
- Use LOD to reduce complexity at distance
- Leverage Niagara events for interactive VFX

**Migration from Cascade**:
- Alexander doesn't use Cascade
- If importing old effects, use Niagara Emitter Wizard
- Manually recreate complex Cascade effects in Niagara

---

### What is UMG?

**UMG** = Unreal Motion Graphics

**Definition**: Visual UI design and scripting system in Unreal Engine.

**Components**:

**1. Widget Designer** (Visual editor):
- Drag-and-drop UI elements
- WYSIWYG (What You See Is What You Get)
- Real-time preview
- Anchor and alignment tools

**2. Widget Blueprint** (Logic):
- Event-driven scripting
- Data binding
- Animations
- Interaction handling

**3. Widget Classes**:
- **User Widget**: Base class for custom UI
- **HUD**: On-screen display (health, ammo, etc.)
- **Widget Component**: 3D widget in game world

**UI element types**:

**Primitives**:
- Text Block: Display text
- Image: Display texture/material
- Border: Container with background
- Button: Clickable interaction
- Progress Bar: Visual fill indicator

**Layout**:
- Canvas Panel: Absolute positioning
- Vertical/Horizontal Box: Auto-layout
- Grid Panel: Table layout
- Overlay: Layered elements
- Scroll Box: Scrollable content

**Interactive**:
- Button: Click to trigger action
- Checkbox: Toggle on/off
- Slider: Drag to select value
- Text Box: User text input
- Combo Box: Dropdown selection

**In Alexander baseline**:
```cpp
// HUD widget structure
WBP_MainHUD (WidgetBlueprint)
├── Canvas Panel (root)
│   ├── Top Bar (Horizontal Box)
│   │   ├── Mission Objective (Text)
│   │   └── Timer (Text - Binding)
│   ├── Center Crosshair (Image)
│   ├── Bottom Left Status (Vertical Box)
│   │   ├── Shield Bar (Progress Bar - Binding)
│   │   ├── Hull Bar (Progress Bar - Binding)
│   │   └── Speed Text (Text - Binding)
│   └── Mission Log (Scroll Box)
```

**Data binding**:
```cpp
// Widget Blueprint - Graph view
UFUNCTION(BlueprintPure)
float GetShieldPercent() const {
    if (ASpaceship* Ship = GetOwningPlayerPawn<ASpaceship>()) {
        return Ship->GetShieldPercentage();
    }
    return 0.f;
}

// Designer view:
// Progress Bar → Percent → Bind → GetShieldPercent
```

**Widget lifecycle**:
```cpp
void UMyWidget::NativeConstruct() {
    Super::NativeConstruct();
    // Called when widget added to viewport
    // Initialize, bind events
}

void UMyWidget::NativeDestruct() {
    Super::NativeDestruct();
    // Called when widget removed
    // Cleanup, unbind events
}

void UMyWidget::NativeTick(const FGeometry& Geometry, float DeltaTime) {
    Super::NativeTick(Geometry, DeltaTime);
    // Called every frame if ticking enabled
    // Update time-based elements
}
```

**VR considerations**:
```cpp
// 2D overlay vs 3D world widget
// 2D overlay: Traditional screen-space UI (OK for menus)
UWidgetComponent* VR_Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("VRWidget"));
VR_Widget->SetWidgetSpace(EWidgetSpace::World);  // 3D world widget
VR_Widget->SetDrawSize(FVector2D(1920, 1080));
VR_Widget->SetPivot(FVector2D(0.5f, 0.5f));

// Attach to cockpit for readable VR UI
VR_Widget->AttachToComponent(CockpitMesh, FAttachmentTransformRules::KeepRelativeTransform, TEXT("UISocket"));
```

**Performance tips**:
- Disable ticking when not needed
- Use data binding sparingly (can be expensive)
- Cache widget references (don't search every frame)
- Use Invalidation Boxes to reduce redraw
- Limit widget complexity (fewer elements = faster)

---

### What is Profiling?

**Definition**: Measuring performance characteristics of your game to identify bottlenecks and optimize.

**What profiling measures**:
- **Frame time**: How long each frame takes (milliseconds)
- **FPS**: Frames per second (inverse of frame time)
- **CPU time**: Game logic, rendering submission
- **GPU time**: Actual rendering work
- **Memory**: RAM and VRAM usage
- **System-specific**: Individual system performance

**Why profile**:
- Identify slow systems before they become critical
- Establish performance baselines
- Measure optimization impact
- Ensure VR frame rate targets met
- Find unexpected performance costs

**Profiling tools in Unreal**:

**1. Console commands**:
```cpp
stat unit         // Frame time breakdown (game, draw, GPU)
stat fps          // Show FPS counter
stat game         // Game thread details
stat scenerendering  // Rendering breakdown
stat gpu          // GPU timing
stat memory       // Memory usage
stat particles    // Particle system performance
```

**2. Built-in profiler**:
```cpp
// Capture profile session
stat startfile
// ... play game for 30-60 seconds ...
stat stopfile

// Analyze in Unreal Frontend
// Shows detailed breakdown of every function call
```

**3. External profilers**:
- **Unreal Insights**: Advanced profiling (UE 4.23+)
- **Visual Studio Profiler**: C++ code profiling
- **RenderDoc**: GPU debugging
- **NVIDIA Nsight**: GPU-specific profiling

**In Alexander**:
```python
# Automated profiling script
python autonomous_profiling_test.py

# Generates reports:
# - END_TO_END_TEST_REPORT.json (machine-readable)
# - END_TO_END_TEST_REPORT.md (human-readable)
```

**Performance subsystem**:
```cpp
// UPerformanceProfilerSubsystem
// Tracks system-specific performance

void UPerformanceProfilerSubsystem::BeginFrame() {
    FrameStartTime = FPlatformTime::Seconds();
}

void UPerformanceProfilerSubsystem::TrackSystem(FName SystemName) {
    double SystemStartTime = FPlatformTime::Seconds();

    // ... system executes ...

    double SystemEndTime = FPlatformTime::Seconds();
    double SystemDuration = (SystemEndTime - SystemStartTime) * 1000.0;

    SystemTimings.Add(SystemName, SystemDuration);
}

void UPerformanceProfilerSubsystem::EndFrame() {
    TotalFrameTime = (FPlatformTime::Seconds() - FrameStartTime) * 1000.0;

    // Log to JSON for analysis
    SaveToJSON();
}
```

**Interpreting results**:
```json
{
  "frame_time_ms": 11.2,  // 11.2ms = ~89 FPS (good for VR)
  "systems": {
    "MissionSystem": {
      "avg_ms": 0.8,  // Excellent - well under budget
      "max_ms": 2.1   // Occasional spike, but acceptable
    },
    "VFXManager": {
      "avg_ms": 4.2,  // WARNING: High for VFX budget (target ~2ms)
      "max_ms": 8.7   // BAD: Major spikes, needs optimization
    }
  }
}
```

**Profiling workflow**:
1. **Baseline**: Profile clean build before changes
2. **Develop**: Make changes to system
3. **Profile**: Run profiler again
4. **Compare**: Delta from baseline
5. **Optimize**: If slower, investigate why
6. **Document**: Commit with performance notes

**Best practices**:
- Profile in Development or Shipping builds (not Debug)
- Test on minimum-spec hardware
- Profile realistic gameplay scenarios
- Run multiple sessions for statistical average
- Profile both CPU and GPU
- Check memory leaks with extended sessions

---

### What is LOD?

**LOD** = Level of Detail

**Definition**: Technique for reducing rendering cost by using simpler versions of assets at distance.

**How it works**:
- Asset has multiple versions (LOD0, LOD1, LOD2, etc.)
- LOD0: Highest detail (close to camera)
- LOD1: Medium detail
- LOD2+: Progressively simpler
- Engine automatically switches based on distance

**LOD for meshes**:
```
Station Mesh LODs:
LOD0: 100,000 triangles (0-1000 units)
LOD1: 25,000 triangles  (1000-2500 units)
LOD2: 5,000 triangles   (2500-5000 units)
LOD3: 1,000 triangles   (5000+ units)
```

**Performance impact**:
```
Without LOD:
100 stations × 100,000 triangles = 10,000,000 triangles
Frame rate: 30 FPS

With LOD:
10 stations × 100,000 (LOD0) = 1,000,000
30 stations × 25,000 (LOD1)  =   750,000
60 stations × 5,000 (LOD2)   =   300,000
Total: 2,050,000 triangles
Frame rate: 90 FPS
```

**Types of LOD**:

**1. Mesh LOD** (geometry reduction):
```cpp
// Unreal auto-generates LODs
// Mesh Editor → LOD Settings → Auto Generate
// Or manually import LOD levels from 3D software
```

**2. Texture LOD** (mipmap levels):
```
LOD0: 2048×2048 (close)
LOD1: 1024×1024
LOD2: 512×512
LOD3: 256×256 (far)
```

**3. Material LOD**:
```cpp
// Simpler shader at distance
// Material Editor → Use Material LOD
// LOD0: Full PBR with normal maps
// LOD1: Simpler lighting
// LOD2: Unlit
```

**4. VFX LOD**:
```cpp
// Niagara LOD distances
UNiagaraComponent* VFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("VFX"));

// Near: 500 particles
// Medium: 200 particles
// Far: 50 particles
// Very far: Disabled

VFX->SetLODDistance(0, 0.f);      // LOD0: 0-1000 units
VFX->SetLODDistance(1, 1000.f);   // LOD1: 1000-2500 units
VFX->SetLODDistance(2, 2500.f);   // LOD2: 2500+ units
```

**In Alexander baseline**:
```cpp
// CSG primitives don't need LOD (already simple)
// But replacement meshes will need LOD

// VFX uses LOD heavily
UNiagaraSystem* EngineTrail = CreateEngineTrail();
EngineTrail->SetMaxDistance(5000.f);  // Culls beyond 5000 units
EngineTrail->SetLODDistanceScale(1.0f);

// LOD0 (close): 500 particles, full resolution
// LOD1 (medium): 200 particles, lower resolution
// LOD2 (far): 50 particles, minimal detail
// Beyond max distance: Disabled
```

**Automatic LOD generation**:
```cpp
// Unreal can auto-generate LODs
// Static Mesh Editor → LOD Settings
// Number of LODs: 4
// Reduction method: Triangles
// LOD1: 50% reduction
// LOD2: 75% reduction
// LOD3: 90% reduction
```

**Custom LOD control**:
```cpp
// Force specific LOD (for testing)
MeshComponent->SetForcedLodModel(2);  // Force LOD2

// Check current LOD
int32 CurrentLOD = MeshComponent->GetPredictedLODLevel();
```

**LOD optimization strategy**:
1. Identify high-poly assets
2. Generate or create LOD levels
3. Test LOD transitions (should be seamless)
4. Adjust distance thresholds
5. Profile performance improvement
6. Document LOD strategy for artists

---

### What is VR Frame Budget?

**Definition**: The maximum time available to render one frame in VR to maintain target refresh rate.

**VR refresh rates**:
- Meta Quest 2/3: 90 Hz (11.1ms per frame)
- Valve Index: 120 Hz (8.3ms per frame)
- PSVR2: 90 Hz / 120 Hz (11.1ms / 8.3ms)

**Why VR is more demanding**:
- **Dual rendering**: Must render scene twice (left and right eye)
- **Higher resolution**: Combined resolution higher than 4K
- **No tolerance**: Missing frame budget causes judder/nausea
- **Lower latency**: Head tracking requires fast response

**Frame budget breakdown** (90 Hz VR):
```
Total available: 11.1ms
Reserve (safety): 2.0ms
Usable budget: 9.1ms

Allocation:
├── Game thread: 3.0ms
│   ├── Tick functions: 1.5ms
│   ├── Physics: 0.8ms
│   └── Game logic: 0.7ms
├── Render thread: 3.0ms
│   ├── Culling: 0.8ms
│   ├── Draw call prep: 1.2ms
│   └── Command submission: 1.0ms
└── GPU: 3.1ms
    ├── Geometry: 1.0ms
    ├── Lighting: 0.8ms
    ├── VFX: 0.8ms
    └── Post-process: 0.5ms
```

**VR-specific optimizations**:

**1. Instanced stereo rendering**:
```cpp
// Render both eyes in one pass
r.InstancedStereo 1  // Enable (default in VR)

// Performance: ~30% faster than two separate passes
```

**2. Dynamic resolution**:
```cpp
// Automatically lower resolution if frame budget missed
r.DynamicRes.OperationMode 2  // Enable dynamic resolution
r.DynamicRes.TargetFPS 90     // Target 90 FPS

// Example: Rendering at 100% → Frame time 12ms (too slow)
// Engine drops to 90% → Frame time 10.5ms (on budget)
```

**3. Fixed foveated rendering**:
```cpp
// Render edges at lower resolution (peripheral vision)
// Supported on Quest devices
vr.MobileMultiView.FFR 1  // Enable
vr.MobileMultiView.FFRLevel 2  // Moderate foveation

// Performance: 15-25% GPU savings
```

**4. Aggressive culling**:
```cpp
// Cull more aggressively in VR
r.ViewDistanceScale 0.8  // Reduce view distance 20%
r.Shadow.MaxResolution 1024  // Lower shadow quality
r.VolumetricFog 0  // Disable expensive features
```

**Budget monitoring**:
```cpp
// Unreal VR console commands
stat unit  // Shows if over budget
stat vrperf  // VR-specific performance stats

// If "GPU" line in stat unit > 9ms → Over budget!
```

**In Alexander**:
```cpp
// PerformanceProfilerSubsystem tracks VR budget
void UPerformanceProfilerSubsystem::CheckVRBudget() {
    if (!IsVREnabled()) return;

    const float VR_BUDGET_MS = 11.1f;  // 90 Hz
    const float SAFETY_MARGIN_MS = 2.0f;

    if (CurrentFrameTime > (VR_BUDGET_MS - SAFETY_MARGIN_MS)) {
        UE_LOG(LogPerformance, Warning,
            TEXT("VR frame budget exceeded: %.2fms (target: %.2fms)"),
            CurrentFrameTime,
            VR_BUDGET_MS - SAFETY_MARGIN_MS
        );

        // Trigger dynamic quality reduction
        ReduceQualitySettings();
    }
}
```

**Common VR performance killers**:
- Too many particles (VFX)
- Complex post-processing
- High polygon counts
- Too many dynamic lights
- Expensive materials
- Physics simulation overhead

**VR profiling workflow**:
1. Test in VR headset (not desktop preview)
2. Enable stat unit display in headset
3. Play realistic gameplay scenario
4. Watch for frame time spikes
5. Profile to identify bottleneck
6. Optimize or reduce quality
7. Retest until consistently under budget

---

## Best Practices

### Mission Design Principles

**Clear objectives**:
```cpp
// GOOD: Specific, measurable objective
Objective: "Deliver 5 cargo containers to Station Alpha within 5 minutes"

// BAD: Vague objective
Objective: "Do some deliveries"
```

**Progressive difficulty**:
```
Tutorial missions: Single objective, forgiving time limits
Early missions: 2-3 objectives, moderate difficulty
Mid-tier missions: Multiple objectives, tighter constraints
Advanced missions: Complex chains, strict requirements, penalties
```

**Balanced rewards**:
```cpp
// Reward proportional to effort
Mission mission_easy = {
    .difficulty = Easy,
    .time_estimate_minutes = 5,
    .credits = 500,
    .xp = 100
};

Mission mission_hard = {
    .difficulty = Hard,
    .time_estimate_minutes = 20,
    .credits = 3000,  // 6x credits for 4x time = 1.5x efficiency
    .xp = 1000        // 10x XP for challenge
};
```

**Fail states and recovery**:
```cpp
// Missions should have clear fail conditions
if (TimerExpired && !ObjectiveComplete) {
    FailMission("Ran out of time");
    OfferRetry();  // Let player try again
}

// Avoid hard failures when possible
if (CargoDestroyed) {
    // Don't auto-fail - reduce reward instead
    MissionReward *= 0.5f;
    UpdateObjective("Deliver remaining cargo");
}
```

**Narrative integration**:
```
Mission should tell a story, not just be a task list

Example:
"Station Alpha is running low on medical supplies due to a recent outbreak.
Deliver 3 medical containers from Station Beta before the situation worsens.
Time is critical - you have 10 minutes."

Instead of:
"Deliver 3 containers to Station Alpha"
```

**Player choice**:
```cpp
// Offer mission variants where possible
Mission variants[] = {
    {
        .name = "Fast Delivery",
        .time_limit = 300,  // 5 minutes
        .reward = 1000,
        .difficulty = Hard
    },
    {
        .name = "Careful Delivery",
        .time_limit = 900,  // 15 minutes
        .reward = 500,
        .difficulty = Easy,
        .bonus_for_no_damage = 300
    }
};
```

**Testing and balance**:
1. Playtest each mission at least 3 times
2. Time how long mission takes skilled player
3. Add 50% time buffer for average player
4. Ensure mission is completable with baseline ship stats
5. Check mission isn't trivially completable
6. Verify rewards feel appropriate for effort

**Mission variety**:
```
Transport: Delivery, passenger transport, resource hauling
Combat: Defense, patrol, bounty hunting, escort
Exploration: Scanning, discovery, reconnaissance
Hybrid: Rescue (transport + combat), salvage (exploration + combat)
Time-based: Races, speed deliveries, timed challenges
Puzzle: Navigation challenges, docking precision
```

**Accessibility**:
- Provide difficulty options
- Allow mission restart without penalty
- Clear objective markers
- Helpful hints if player stuck
- Optional objectives for advanced players

---

### Station Layout Guidelines

**Functional zones**:
```
Every station should have clearly defined areas:

Docking Zone:
├── External docking ports (3-8 ports)
├── Airlock chambers
└── Landing clearance volume

Commercial Zone:
├── Mission board
├── Shop/trading terminal
├── Repair/refuel services
└── Crew quarters

Industrial Zone:
├── Cargo storage
├── Manufacturing
└── Resource processing

Command Zone:
├── Station control
├── Communications
└── Navigation systems
```

**Spatial hierarchy**:
```
Large → Medium → Small scale

Station exterior (1000+ units):
└── Docking bay (500 units)
    └── Airlock (200 units)
        └── Interior corridor (100 units)
            └── Room (50 units)
```

**Navigation flow**:
```cpp
// Design for easy player navigation
// Main path should be obvious
// Side paths for exploration

Entrance → Main corridor → Hub area → Specialized zones
                ↓
          Side rooms and services

// Avoid:
// - Dead ends without purpose
// - Confusing maze-like layouts
// - Backtracking without shortcuts
```

**Docking port placement**:
```cpp
// Evenly distributed around station
// 360-degree access when possible
// Clear approach lanes

Station with 6 ports (hexagonal layout):
Port 0: 0° (front)
Port 1: 60°
Port 2: 120°
Port 3: 180° (rear)
Port 4: 240°
Port 5: 300°

// Approach lanes: 30° cone clear of obstacles
```

**Scale considerations**:
```
Small station (frigate): 500-1000 units, 2-3 docking ports
Medium station (cruiser): 1000-2500 units, 4-6 ports
Large station (capital): 2500-5000 units, 8-12 ports
Megastructure: 5000+ units, 12+ ports
```

**Performance considerations**:
```cpp
// Station complexity budget
Total triangles (all LODs): 100,000 max
Lights: 10-15 static lights
VFX: 3-5 ambient effects (running lights, vents)
Collision: Simple hulls, no per-polygon collision
```

**CSG primitive strategy**:
```
Main hull: Large box or cylinder
Modules: Smaller attached boxes
Docking ports: Subtracted cylinders
Windows: Subtracted boxes with glass planes
Details: Additional small primitives
```

**Interior design**:
```
Ceiling height: 200-400 units (comfortable for VR)
Corridor width: 200-300 units (not claustrophobic)
Doorways: 150 units wide, 250 units tall
Rooms: 500-1000 units (varied sizes)
```

**Landmarks and orientation**:
```
Players should always know:
- Where they are (signage, color coding)
- Where they came from (breadcrumbs)
- Where they're going (objective markers)

Color coding example:
Red zone: Restricted/dangerous
Yellow zone: Industrial/work areas
Green zone: Safe/commercial areas
Blue zone: Docking/departure
```

**Modularity**:
```
Design stations from reusable modules:
- Standard corridor section
- Standard room template
- Standard docking port
- Standard junction

Benefits:
- Faster station creation
- Consistent player experience
- Easier maintenance
- Performance optimization
```

---

### VFX Performance Rules

**Particle count limits**:
```
Individual system:
Minimal effect: 50-100 particles
Standard effect: 200-500 particles
Heavy effect: 1000-2000 particles
Epic effect: 5000+ particles (rare, special occasions)

Total scene:
Desktop: 50,000 total particles max
VR: 20,000 total particles max
Mobile: 5,000 total particles max
```

**Emission strategies**:
```cpp
// Burst emission for explosions
EmissionMode = Burst
ParticlesPerBurst = 500
BurstInterval = 0.1s

// Continuous emission for trails
EmissionMode = Rate
ParticlesPerSecond = 100

// Event-driven emission
SpawnOnCollision = true
SpawnCount = 20
```

**Pooling and reuse**:
```cpp
// WRONG: Spawn and destroy constantly
void FireWeapon() {
    UNiagaraComponent* VFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(...);
    // Memory allocation every shot!
}

// RIGHT: Pool and reuse
class UVFXManager {
    TArray<UNiagaraComponent*> VFXPool;

    UNiagaraComponent* GetPooledVFX() {
        // Reuse inactive VFX from pool
        for (UNiagaraComponent* VFX : VFXPool) {
            if (!VFX->IsActive()) {
                VFX->Activate();
                return VFX;
            }
        }
        // Create new if pool exhausted
        return CreateAndPoolVFX();
    }
};
```

**Culling and LOD**:
```cpp
// Aggressive culling for VFX
VFXComponent->SetMaxDistance(5000.f);  // Disable beyond 5000 units

// LOD distances
VFX->SetLODDistance(0, 0.f);      // LOD0: Full detail 0-1000
VFX->SetLODDistance(1, 1000.f);   // LOD1: Medium 1000-2500
VFX->SetLODDistance(2, 2500.f);   // LOD2: Low 2500-5000

// Significance culling
VFX->SetSignificance(0.5f);  // Reduce quality if not important
```

**GPU vs CPU simulation**:
```cpp
// GPU: For large particle counts
EmitterProperties->SimTarget = ENiagaraSimTarget::GPUComputeSim;
// Best for: Explosions, large trails, ambient effects

// CPU: For collision, events, complex logic
EmitterProperties->SimTarget = ENiagaraSimTarget::CPUSim;
// Best for: Interactive particles, precise collision
```

**Texture optimization**:
```
Particle textures:
- Use texture atlases (multiple particles in one texture)
- Max resolution: 512×512 per particle type
- Format: DXT5 compressed
- Mipmaps: Enabled for distance rendering

Example:
Instead of 10 separate 256×256 textures (2.5 MB)
Use 1 texture atlas 1024×1024 with 10 particles (0.7 MB)
```

**Material complexity**:
```cpp
// SIMPLE particle material (good)
BaseColor: Texture sample
Emissive: BaseColor * Brightness
Opacity: Texture alpha
Blend Mode: Additive or Translucent
Shading Model: Unlit

// COMPLEX particle material (avoid)
Multiple texture samples
Normal mapping
Parallax occlusion
Dynamic lighting
Custom lighting model
← All of these are overkill for particles
```

**VFX budget allocation**:
```
Total VFX budget: 2ms per frame (VR)

Ambient effects: 0.5ms (background station lights, etc.)
Engine trails: 0.5ms (ship trails)
Weapon effects: 0.5ms (combat)
Environmental: 0.3ms (nebula, asteroids)
UI effects: 0.2ms (menu transitions, highlights)
```

**Testing and profiling**:
```cpp
// Test VFX performance
stat particles  // Show particle counts and timings

// Per-system breakdown
fx.Niagara.Debug.Enabled 1
fx.Niagara.Debug.Verbosity 2

// Stress test: Spawn maximum expected VFX
// 10 ships with engine trails
// 5 active weapon effects
// 3 explosions
// Ambient station effects
// → If frame time under budget, you're good
```

**When to optimize VFX**:
- Particle count > 20,000 in VR
- VFX frame time > 2ms
- Visible hitching when VFX spawn
- Frame rate drops during combat
- GPU time spikes correlate with VFX

---

### UI Readability in VR

**Text size and distance**:
```
Minimum readable text in VR:
At 50cm (cockpit UI): 24pt font
At 1m (arm's reach): 36pt font
At 2m (room scale): 72pt font

Formula: TextSizePt = 24 * (DistanceInMeters / 0.5)
```

**Widget sizing for VR**:
```cpp
// 3D widget component sizing
UWidgetComponent* VRWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("VRWidget"));

// Size for readability at 1m distance
VRWidget->SetDrawSize(FVector2D(800, 600));  // Virtual pixels
VRWidget->SetWorldScale3D(FVector(0.1f));     // Physical scale

// Position in cockpit
VRWidget->SetRelativeLocation(FVector(100, 0, 0));  // 1m in front of player
```

**Color contrast**:
```
Good contrast ratios for VR:
Text on background: 7:1 minimum (WCAG AAA)
UI elements: 4.5:1 minimum (WCAG AA)

Example good combinations:
White text on dark blue: ✓
Yellow text on black: ✓
Cyan text on dark gray: ✓

Example bad combinations:
Light gray text on white: ✗
Blue text on purple: ✗
Red text on green: ✗ (also colorblind issue)
```

**Depth and layering**:
```cpp
// Avoid flat UI in VR - use depth
// Layer elements in 3D space

HUD elements:
├── Critical info (closest): 80cm
├── Primary info: 100cm
├── Secondary info: 120cm
└── Ambient info (farthest): 150cm

// In UMG, use Transform→Translation Z to layer widgets
```

**Minimize head movement**:
```
UI within comfortable FOV:
Horizontal: ±30° from center (60° total)
Vertical: ±20° from center (40° total)

Avoid:
- UI elements at extreme edges
- Information requiring head turning
- Vertical stacking beyond comfortable range
```

**Reduce visual clutter**:
```
Desktop UI: Can show 20+ elements
VR UI: Limit to 5-8 primary elements

Desktop HUD:
├── Health
├── Shields
├── Energy
├── Speed
├── Altitude
├── Heading
├── Cargo
├── Mission objectives
├── Mini-map
├── Radar
├── Weapon status
└── ... 15 other things

VR HUD (simplified):
├── Shield/Health (combined)
├── Speed
├── Current objective
├── Weapon ready state
└── Critical warnings only
```

**Readable fonts**:
```
Good VR fonts:
- Roboto (clean, sans-serif)
- Open Sans (readable, modern)
- Arial (safe default)
- Courier (monospace for data)

Avoid:
- Decorative fonts
- Thin/light weight fonts
- Cursive or script fonts
- Fonts with small details
```

**Animation and movement**:
```cpp
// Slow, deliberate animations
// Fast UI movement causes VR discomfort

GOOD:
Widget->SetRenderOpacity(Opacity);  // Fade in over 0.5s
Interpolate smoothly

BAD:
Widget->SetVisibility(ESlateVisibility::Visible);  // Instant pop
Jarring in VR

// Lerp example
float TargetOpacity = 1.0f;
float CurrentOpacity = 0.0f;
float DeltaTime = 0.016f;  // 60 FPS

CurrentOpacity = FMath::FInterpTo(CurrentOpacity, TargetOpacity, DeltaTime, 5.0f);
Widget->SetRenderOpacity(CurrentOpacity);
```

**Gaze-based interaction**:
```cpp
// Natural VR interaction: Look at UI to interact
// Avoid requiring precise pointing

UWidgetComponent* Widget = ...;

// Detect when player looking at widget
FVector PlayerForward = PlayerCamera->GetForwardVector();
FVector ToWidget = (Widget->GetComponentLocation() - PlayerCamera->GetComponentLocation()).GetSafeNormal();

float DotProduct = FVector::DotProduct(PlayerForward, ToWidget);
if (DotProduct > 0.95f) {  // Looking at widget (within ~18° cone)
    // Highlight widget
    Widget->SetTintColorAndOpacity(FLinearColor::Yellow);

    // Auto-select after dwell time
    GazeDwellTime += DeltaTime;
    if (GazeDwellTime > 1.5f) {
        ActivateWidget();
    }
} else {
    GazeDwellTime = 0.f;
}
```

**Testing in VR**:
```
Checklist for VR UI testing:
✓ Text readable from expected distance?
✓ No eye strain after 10 minutes?
✓ All UI reachable without excessive head movement?
✓ Contrast sufficient in bright and dark scenes?
✓ Animations smooth, not jarring?
✓ Can interact without removing headset?
✓ Tested by someone with different IPD (interpupillary distance)?
```

---

### Git Workflow for Game Assets

**Branch strategy**:
```
master (main branch)
├── develop (integration branch)
├── feature/station-layouts
├── feature/mission-combat
├── feature/vfx-explosions
└── hotfix/docking-collision-bug
```

**Commit frequency**:
```bash
# Commit often, push periodically
git commit -am "Add Station Alpha exterior"
git commit -am "Add docking ports to Station Alpha"
git commit -am "Test Station Alpha docking - working"
git commit -am "Add Station Alpha interior zones"
# ... end of day ...
git push origin feature/station-layouts
```

**Git LFS for large files**:
```bash
# Setup Git LFS (one-time)
git lfs install

# Track large file types
git lfs track "*.uasset"
git lfs track "*.umap"
git lfs track "*.png"
git lfs track "*.jpg"
git lfs track "*.wav"
git lfs track "*.fbx"

# Commit .gitattributes
git add .gitattributes
git commit -m "Configure Git LFS for Unreal assets"
```

**.gitignore for Unreal**:
```bash
# .gitignore
# Build results
Binaries/
Build/
Intermediate/
DerivedDataCache/

# User-specific files
Saved/
.vs/
.vscode/
*.VC.db
*.opensdf
*.opendb
*.sdf
*.sln
*.suo
*.xcodeproj
*.xcworkspace

# Don't ignore:
# - Content/ (game assets)
# - Config/ (project settings)
# - Source/ (code)
# - *.uproject (project file)
```

**Merge conflicts with binary files**:
```bash
# Binary files (uasset, umap) can't be merged
# Strategy: Avoid working on same assets

# If conflict occurs:
git checkout --ours path/to/file.uasset    # Use your version
# OR
git checkout --theirs path/to/file.uasset  # Use their version

# Then manually reconcile in Unreal Editor
```

**Commit message conventions**:
```bash
# Format: [Category] Short description

# Good examples:
git commit -m "[Asset] Add Station Beta CSG model"
git commit -m "[Mission] Implement cargo delivery missions"
git commit -m "[VFX] Optimize engine trail particle pooling"
git commit -m "[Fix] Resolve docking collision detection bug"
git commit -m "[Perf] Reduce mission update frequency from 60Hz to 10Hz"

# Include performance data when relevant:
git commit -m "[VFX] Reduce explosion particles - improved frame time from 12ms to 8ms"
```

**Feature branch workflow**:
```bash
# Start new feature
git checkout develop
git pull origin develop
git checkout -b feature/my-feature

# Work on feature
# ... make changes ...
git add .
git commit -m "[Feature] Add initial implementation"

# Keep up-to-date with develop
git checkout develop
git pull origin develop
git checkout feature/my-feature
git merge develop  # Resolve conflicts if any

# Finish feature
git checkout develop
git merge feature/my-feature
git push origin develop

# Clean up
git branch -d feature/my-feature
```

**Release tagging**:
```bash
# Tag releases for version tracking
git tag -a v1.0-baseline -m "Initial autonomous baseline generation"
git push origin v1.0-baseline

# Later releases:
git tag -a v1.1-stations-complete -m "All 3 baseline stations complete"
git tag -a v1.2-missions-balanced -m "Mission rewards balanced, 15 missions"
git tag -a v2.0-alpha -m "First playable alpha with replacement assets"
```

**Collaboration workflow**:
```bash
# Developer A: Works on stations
git checkout -b feature/stations
# ... create Station Alpha ...
git commit -am "[Asset] Station Alpha complete"
git push origin feature/stations

# Developer B: Works on missions (needs stations)
git checkout -b feature/missions
git merge origin/feature/stations  # Get A's stations
# ... create missions using stations ...
git commit -am "[Mission] Add missions using Station Alpha"
git push origin feature/missions

# Both merge to develop when ready
```

---

### Testing Methodology

**Test pyramid**:
```
        /\
       /  \  E2E Tests (few, slow, comprehensive)
      /____\
     /      \  Integration Tests (moderate, medium speed)
    /________\
   /          \ Unit Tests (many, fast, focused)
  /__  ________\
```

**Unit tests**:
```cpp
// Test individual functions/classes in isolation
UTEST(MissionSystem, CalculatesRewardCorrectly) {
    UMissionSystem* Mission = NewObject<UMissionSystem>();

    Mission->BasePay = 1000;
    Mission->DifficultyMultiplier = 1.5f;
    Mission->BonusObjectivesCompleted = 2;
    Mission->BonusPerObjective = 200;

    int32 ExpectedReward = 1000 * 1.5 + (2 * 200);  // 1900
    int32 ActualReward = Mission->CalculateTotalReward();

    ASSERT_EQ(ExpectedReward, ActualReward);
}
```

**Integration tests**:
```cpp
// Test systems working together
UTEST(StationMissionIntegration, MissionsBoardShowsAvailableMissions) {
    // Setup
    UWorld* World = CreateTestWorld();
    AStationAlpha* Station = SpawnStation(World);
    UMissionBoardComponent* Board = Station->GetMissionBoard();

    // Load test missions
    Board->LoadMissionsFromPath("/Game/Data/Missions/Test/");

    // Execute
    TArray<UMissionData*> Missions = Board->GetAvailableMissions();

    // Verify
    ASSERT_GT(Missions.Num(), 0);
    ASSERT_TRUE(Missions[0]->IsValid());
}
```

**End-to-end tests**:
```python
# Test complete user workflows
def test_complete_mission_workflow():
    # Launch game
    game = launch_unreal_game()

    # Player accepts mission
    game.open_mission_board()
    game.select_mission("Cargo Delivery")
    game.accept_mission()

    # Player completes objectives
    game.navigate_to("Station Beta")
    game.dock_at_station()
    game.load_cargo()
    game.undock()
    game.navigate_to("Station Alpha")
    game.dock_at_station()
    game.unload_cargo()

    # Verify mission complete
    assert game.mission_status() == "COMPLETE"
    assert game.player_credits() == initial_credits + 1000
```

**Performance tests**:
```python
# Automated performance validation
def test_vfx_performance_budget():
    profiler = PerformanceProfiler()

    # Spawn maximum expected VFX
    spawn_effects(engine_trails=10, explosions=5, ambient=20)

    # Measure
    profiler.start()
    run_for_seconds(30)
    profiler.stop()

    # Validate
    avg_frame_time = profiler.get_average_frame_time()
    vfx_time = profiler.get_system_time("VFXManager")

    assert avg_frame_time < 11.0, f"Frame time {avg_frame_time}ms exceeds VR budget"
    assert vfx_time < 3.0, f"VFX time {vfx_time}ms exceeds VFX budget"
```

**Regression tests**:
```python
# Ensure optimizations don't break functionality
def test_mission_still_works_after_optimization():
    # Baseline
    baseline_data = load_json("baseline_mission_test.json")

    # Current
    result = run_mission_test("Cargo Delivery")

    # Compare
    assert result.completed == baseline_data.completed
    assert result.objectives_met == baseline_data.objectives_met
    assert result.reward == baseline_data.reward

    # Performance should improve or stay same
    assert result.avg_frame_time <= baseline_data.avg_frame_time
```

**Test automation**:
```bash
# Run all tests
python build_and_test.py

# Run specific test suites
python build_and_test.py --suite unit
python build_and_test.py --suite integration
python build_and_test.py --suite e2e

# Run with profiling
python build_and_test.py --profile

# Generate coverage report
python build_and_test.py --coverage
```

**Testing checklist**:
```
Before committing:
✓ All unit tests pass
✓ No compiler warnings
✓ Code builds in Development and Shipping
✓ Manual smoke test (launch game, basic functionality works)

Before merging to develop:
✓ All integration tests pass
✓ Performance tests pass
✓ No regressions from previous builds
✓ Code review completed

Before releasing:
✓ All E2E tests pass
✓ Manual playtesting session
✓ Performance profiling on minimum spec hardware
✓ VR testing (if VR supported)
✓ Compatibility testing (different OS/hardware)
```

**Bug report template**:
```markdown
## Bug Report

**Title**: [Short, descriptive title]

**Severity**: Critical / High / Medium / Low

**Steps to Reproduce**:
1. Launch game
2. Open mission board
3. Select "Cargo Delivery" mission
4. Click Accept

**Expected behavior**:
Mission should be added to player's active missions list

**Actual behavior**:
Game crashes with error: "Null pointer exception in MissionBoardComponent"

**Environment**:
- Unreal version: 5.6.0
- Platform: Windows 11
- Hardware: RTX 3080, 32GB RAM
- Branch: feature/mission-refactor
- Commit: abc123def456

**Logs**:
[Attach log file or paste relevant error messages]

**Screenshots/Video**:
[If applicable]
```

---

## System Architecture

### How Mission System Works

**Core components**:
```
MissionSystem Architecture:
├── UMissionTemplateDataAsset (Mission definitions)
├── UMissionBoardComponent (Mission availability)
├── UMissionTrackerComponent (Active mission tracking)
├── AMissionObjective (Specific objectives)
└── UMissionRewardSystem (Completion rewards)
```

**Data flow**:
```
1. Mission Creation (Design-time):
   Designer creates UMissionTemplateDataAsset
   ├── Define mission type
   ├── Set objectives
   ├── Configure rewards
   └── Save as asset

2. Mission Loading (Runtime):
   UMissionBoardComponent::BeginPlay()
   ├── Loads mission templates from Data/Missions/
   ├── Filters by player level/reputation
   ├── Populates mission board UI
   └── Waits for player interaction

3. Mission Acceptance (Player action):
   Player clicks "Accept" on mission board
   ├── UMissionBoardComponent→AcceptMission()
   ├── Creates UMissionTrackerComponent instance
   ├── Initializes objectives
   ├── Adds to HUD
   └── Triggers mission start events

4. Mission Execution (Gameplay):
   Player performs actions
   ├── Action triggers objective check
   ├── UMissionTrackerComponent→CheckObjective()
   ├── Update UI progress
   ├── If all objectives complete → CompleteMission()
   └── If fail condition → FailMission()

5. Mission Completion (Reward):
   UMissionRewardSystem→GrantRewards()
   ├── Calculate total reward
   ├── Apply bonuses/penalties
   ├── Grant credits to player
   ├── Grant XP
   ├── Update reputation
   └── Trigger completion UI/notifications
```

**Class relationships**:
```cpp
// Mission template (data asset)
UCLASS()
class UMissionTemplateDataAsset : public UDataAsset {
    UPROPERTY(EditAnywhere)
    FText MissionName;

    UPROPERTY(EditAnywhere)
    EMissionType MissionType;

    UPROPERTY(EditAnywhere)
    TArray<FMissionObjective> Objectives;

    UPROPERTY(EditAnywhere)
    FMissionRewards Rewards;

    UPROPERTY(EditAnywhere)
    int32 RequiredPlayerLevel;
};

// Mission board (provides available missions)
UCLASS()
class UMissionBoardComponent : public UActorComponent {
public:
    UFUNCTION(BlueprintCallable)
    TArray<UMissionTemplateDataAsset*> GetAvailableMissions();

    UFUNCTION(BlueprintCallable)
    void AcceptMission(UMissionTemplateDataAsset* MissionTemplate);

private:
    TArray<UMissionTemplateDataAsset*> LoadedMissions;
};

// Mission tracker (active mission instance)
UCLASS()
class UMissionTrackerComponent : public UActorComponent {
public:
    void Initialize(UMissionTemplateDataAsset* Template);

    UFUNCTION()
    void OnObjectiveComplete(int32 ObjectiveIndex);

    UFUNCTION()
    void CheckMissionCompletion();

private:
    UMissionTemplateDataAsset* MissionTemplate;
    TArray<bool> ObjectiveCompletionStatus;
    float TimeRemaining;
};

// Objective (individual task)
USTRUCT()
struct FMissionObjective {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    EMissionObjectiveType Type;  // Navigate, Dock, Destroy, Collect, etc.

    UPROPERTY(EditAnywhere)
    FVector TargetLocation;

    UPROPERTY(EditAnywhere)
    AActor* TargetActor;

    UPROPERTY(EditAnywhere)
    int32 TargetCount;  // For collection objectives

    UPROPERTY(EditAnywhere)
    float TimeLimit;
};
```

**Objective checking**:
```cpp
void UMissionTrackerComponent::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    // Update time-based objectives
    TimeRemaining -= DeltaTime;
    if (TimeRemaining <= 0.f && HasTimeLimit()) {
        FailMission("Time expired");
        return;
    }

    // Check objectives
    for (int32 i = 0; i < MissionTemplate->Objectives.Num(); ++i) {
        if (ObjectiveCompletionStatus[i]) {
            continue;  // Already complete
        }

        FMissionObjective& Objective = MissionTemplate->Objectives[i];

        bool bObjectiveComplete = false;
        switch (Objective.Type) {
            case EMissionObjectiveType::Navigate:
                bObjectiveComplete = CheckNavigateObjective(Objective);
                break;
            case EMissionObjectiveType::Dock:
                bObjectiveComplete = CheckDockObjective(Objective);
                break;
            case EMissionObjectiveType::Destroy:
                bObjectiveComplete = CheckDestroyObjective(Objective);
                break;
            case EMissionObjectiveType::Collect:
                bObjectiveComplete = CheckCollectObjective(Objective);
                break;
        }

        if (bObjectiveComplete) {
            OnObjectiveComplete(i);
        }
    }

    // Check if all objectives complete
    if (AreAllObjectivesComplete()) {
        CompleteMission();
    }
}
```

**Extension points**:
```cpp
// Custom objective types
UENUM()
enum class EMissionObjectiveType : uint8 {
    Navigate,      // Reach location
    Dock,          // Dock at station
    Destroy,       // Destroy target
    Collect,       // Collect resources
    Escort,        // Protect target
    Scan,          // Scan anomaly
    Deliver,       // Transport cargo
    Custom         // User-defined objective
};

// Implement custom objective checking
bool UMissionTrackerComponent::CheckCustomObjective(const FMissionObjective& Objective) {
    // Your custom logic here
    return false;
}
```

---

### How Station Docking Works

**Docking system components**:
```
Station Docking:
├── ADockingPort (Station side)
├── UDockingComponent (Ship side)
├── DockingAlignment (Positioning logic)
└── DockingSequence (Animation/state machine)
```

**Docking states**:
```cpp
UENUM()
enum class EDockingState : uint8 {
    Idle,              // Not docking
    Approaching,       // Flying toward port
    Aligning,          // Rotating to match port orientation
    RequestingClearance, // Asking station for permission
    ClearanceGranted,  // Permission received
    FinalApproach,     // Moving into port
    Locked,            // Docked successfully
    Undocking          // Leaving port
};
```

**Docking port**:
```cpp
UCLASS()
class ADockingPort : public AActor {
    GENERATED_BODY()

public:
    // Visual components
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* PortMesh;

    UPROPERTY(VisibleAnywhere)
    UBoxComponent* DockingVolume;  // Trigger for docking

    UPROPERTY(VisibleAnywhere)
    UArrowComponent* AlignmentArrow;  // Shows correct approach angle

    // Docking state
    UPROPERTY(BlueprintReadOnly)
    bool bIsOccupied;

    UPROPERTY(BlueprintReadOnly)
    ASpaceship* DockedShip;

    // Docking permissions
    UFUNCTION()
    bool RequestDocking(ASpaceship* RequestingShip);

    UFUNCTION()
    void CompleteDocking(ASpaceship* Ship);

    UFUNCTION()
    void Undock();

private:
    void LockShipInPlace(ASpaceship* Ship);
    void ReleaseShip(ASpaceship* Ship);
};
```

**Ship docking component**:
```cpp
UCLASS()
class UDockingComponent : public UActorComponent {
    GENERATED_BODY()

public:
    void BeginPlay() override;
    void TickComponent(float DeltaTime, ...) override;

    UFUNCTION(BlueprintCallable)
    void InitiateDocking(ADockingPort* TargetPort);

    UFUNCTION(BlueprintCallable)
    void CancelDocking();

    UFUNCTION(BlueprintCallable)
    void Undock();

private:
    UPROPERTY()
    EDockingState CurrentState;

    UPROPERTY()
    ADockingPort* TargetDockingPort;

    void UpdateApproaching(float DeltaTime);
    void UpdateAligning(float DeltaTime);
    void UpdateFinalApproach(float DeltaTime);

    bool IsAlignedWithPort() const;
    bool IsInDockingRange() const;
};
```

**Docking sequence**:
```cpp
void UDockingComponent::TickComponent(float DeltaTime, ...) {
    Super::TickComponent(DeltaTime, ...);

    if (!TargetDockingPort) return;

    switch (CurrentState) {
        case EDockingState::Approaching:
            UpdateApproaching(DeltaTime);
            break;

        case EDockingState::Aligning:
            UpdateAligning(DeltaTime);
            break;

        case EDockingState::RequestingClearance:
            if (TargetDockingPort->RequestDocking(GetOwner<ASpaceship>())) {
                CurrentState = EDockingState::ClearanceGranted;
            } else {
                // Port occupied or clearance denied
                CancelDocking();
            }
            break;

        case EDockingState::FinalApproach:
            UpdateFinalApproach(DeltaTime);
            break;

        case EDockingState::Locked:
            // Docked, no updates needed
            break;
    }
}

void UDockingComponent::UpdateApproaching(float DeltaTime) {
    ASpaceship* Ship = GetOwner<ASpaceship>();
    FVector TargetLocation = TargetDockingPort->GetActorLocation();

    // Move toward port
    FVector Direction = (TargetLocation - Ship->GetActorLocation()).GetSafeNormal();
    Ship->AddMovementInput(Direction);

    // Check if in alignment range
    float DistanceToPort = FVector::Dist(Ship->GetActorLocation(), TargetLocation);
    if (DistanceToPort < 500.f) {
        CurrentState = EDockingState::Aligning;
    }
}

void UDockingComponent::UpdateAligning(float DeltaTime) {
    ASpaceship* Ship = GetOwner<ASpaceship>();
    FRotator TargetRotation = TargetDockingPort->GetAlignmentRotation();

    // Rotate to match port orientation
    FRotator CurrentRotation = Ship->GetActorRotation();
    FRotator NewRotation = FMath::RInterpTo(
        CurrentRotation,
        TargetRotation,
        DeltaTime,
        2.0f  // Rotation speed
    );
    Ship->SetActorRotation(NewRotation);

    // Check if aligned
    if (IsAlignedWithPort()) {
        CurrentState = EDockingState::RequestingClearance;
    }
}

void UDockingComponent::UpdateFinalApproach(float DeltaTime) {
    ASpaceship* Ship = GetOwner<ASpaceship>();

    // Slow, precise approach
    FVector TargetLocation = TargetDockingPort->GetDockingPosition();
    FVector CurrentLocation = Ship->GetActorLocation();

    FVector NewLocation = FMath::VInterpConstantTo(
        CurrentLocation,
        TargetLocation,
        DeltaTime,
        50.0f  // Slow approach speed
    );
    Ship->SetActorLocation(NewLocation);

    // Check if fully docked
    float DistanceToFinalPosition = FVector::Dist(NewLocation, TargetLocation);
    if (DistanceToFinalPosition < 10.f) {
        // Lock ship in place
        TargetDockingPort->CompleteDocking(Ship);
        CurrentState = EDockingState::Locked;

        // Disable ship flight controls
        Ship->SetDocked(true);

        // Trigger docking complete events
        OnDockingComplete.Broadcast();
    }
}
```

**Collision and safety**:
```cpp
// Docking volume (trigger)
void ADockingPort::BeginPlay() {
    Super::BeginPlay();

    DockingVolume->OnComponentBeginOverlap.AddDynamic(
        this,
        &ADockingPort::OnShipEnterDockingZone
    );
}

void ADockingPort::OnShipEnterDockingZone(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
) {
    ASpaceship* Ship = Cast<ASpaceship>(OtherActor);
    if (!Ship) return;

    // Check if ship is attempting to dock
    UDockingComponent* DockingComp = Ship->FindComponentByClass<UDockingComponent>();
    if (DockingComp && DockingComp->GetTargetPort() == this) {
        // Ship is trying to dock with this port
        // Allow docking sequence to continue
    } else {
        // Ship entered docking zone without permission
        // Reject or warn
        if (Ship->GetVelocity().Size() > 100.f) {
            // Fast collision - damage both
            Ship->TakeDamage(50.f);
            TriggerDamageEffects();
        }
    }
}
```

**Visual feedback**:
```cpp
// Docking guidance VFX
void ADockingPort::UpdateDockingGuidance(ASpaceship* Ship) {
    if (!DockingGuidanceVFX) return;

    bool bShipAligned = IsShipAligned(Ship);
    bool bShipInRange = IsShipInRange(Ship);

    FLinearColor GuidanceColor;
    if (bShipAligned && bShipInRange) {
        GuidanceColor = FLinearColor::Green;  // Good to dock
    } else if (bShipInRange) {
        GuidanceColor = FLinearColor::Yellow;  // In range, need alignment
    } else {
        GuidanceColor = FLinearColor::Red;  // Too far
    }

    DockingGuidanceVFX->SetColorParameter("GuidanceColor", GuidanceColor);
}
```

---

### How VFX Pooling Works

**Object pooling concept**:
```
Instead of: Create → Use → Destroy → Create → Use → Destroy
Use pooling: Create pool → Reuse → Reuse → Reuse → Reuse...

Benefits:
- No allocation/deallocation overhead
- No garbage collection spikes
- Consistent performance
- Reduced memory fragmentation
```

**VFX pool manager**:
```cpp
UCLASS()
class UVFXPoolManager : public UObject {
    GENERATED_BODY()

public:
    // Initialize pool
    void InitializePool(UNiagaraSystem* SystemTemplate, int32 PoolSize);

    // Get VFX from pool
    UFUNCTION(BlueprintCallable)
    UNiagaraComponent* AcquireVFX(UNiagaraSystem* SystemTemplate);

    // Return VFX to pool
    UFUNCTION(BlueprintCallable)
    void ReleaseVFX(UNiagaraComponent* VFXComponent);

    // Cleanup
    void DestroyPool();

private:
    // Pool storage
    UPROPERTY()
    TMap<UNiagaraSystem*, FVFXPool> Pools;
};

USTRUCT()
struct FVFXPool {
    GENERATED_BODY()

    // Pre-created VFX components
    UPROPERTY()
    TArray<UNiagaraComponent*> AvailableVFX;

    // Currently in-use VFX components
    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveVFX;
};
```

**Pool initialization**:
```cpp
void UVFXPoolManager::InitializePool(UNiagaraSystem* SystemTemplate, int32 PoolSize) {
    if (!SystemTemplate) return;

    FVFXPool& Pool = Pools.FindOrAdd(SystemTemplate);

    // Pre-create VFX components
    for (int32 i = 0; i < PoolSize; ++i) {
        UNiagaraComponent* VFX = NewObject<UNiagaraComponent>(this);
        VFX->SetAsset(SystemTemplate);
        VFX->SetAutoActivate(false);
        VFX->SetAutoDestroy(false);  // IMPORTANT: Don't auto-destroy pooled VFX

        // Register but keep inactive
        VFX->RegisterComponent();
        VFX->Deactivate();

        Pool.AvailableVFX.Add(VFX);
    }

    UE_LOG(LogVFX, Log, TEXT("Initialized VFX pool for %s with %d instances"),
        *SystemTemplate->GetName(), PoolSize);
}
```

**Acquiring VFX from pool**:
```cpp
UNiagaraComponent* UVFXPoolManager::AcquireVFX(UNiagaraSystem* SystemTemplate) {
    FVFXPool* Pool = Pools.Find(SystemTemplate);
    if (!Pool) {
        // Pool doesn't exist - create on demand
        InitializePool(SystemTemplate, 10);  // Default size
        Pool = Pools.Find(SystemTemplate);
    }

    UNiagaraComponent* VFX = nullptr;

    if (Pool->AvailableVFX.Num() > 0) {
        // Reuse from pool
        VFX = Pool->AvailableVFX.Pop();
    } else {
        // Pool exhausted - create new (or expand pool)
        VFX = NewObject<UNiagaraComponent>(this);
        VFX->SetAsset(SystemTemplate);
        VFX->SetAutoActivate(false);
        VFX->SetAutoDestroy(false);
        VFX->RegisterComponent();

        UE_LOG(LogVFX, Warning, TEXT("VFX pool exhausted for %s, created new instance"),
            *SystemTemplate->GetName());
    }

    // Move to active list
    Pool->ActiveVFX.Add(VFX);

    // Reset and activate
    VFX->ResetSystem();
    VFX->Activate();

    return VFX;
}
```

**Releasing VFX back to pool**:
```cpp
void UVFXPoolManager::ReleaseVFX(UNiagaraComponent* VFXComponent) {
    if (!VFXComponent) return;

    UNiagaraSystem* SystemTemplate = VFXComponent->GetAsset();
    FVFXPool* Pool = Pools.Find(SystemTemplate);
    if (!Pool) return;

    // Deactivate
    VFXComponent->Deactivate();
    VFXComponent->SetVisibility(false);

    // Move from active to available
    Pool->ActiveVFX.Remove(VFXComponent);
    Pool->AvailableVFX.Add(VFXComponent);
}
```

**Automatic release**:
```cpp
// VFX that auto-releases when complete
void UVFXPoolManager::SpawnPooledVFX(
    UNiagaraSystem* SystemTemplate,
    FVector Location,
    FRotator Rotation,
    float Duration = -1.f  // -1 = use VFX natural duration
) {
    UNiagaraComponent* VFX = AcquireVFX(SystemTemplate);
    if (!VFX) return;

    VFX->SetWorldLocationAndRotation(Location, Rotation);

    // Auto-release when complete
    if (Duration < 0.f) {
        // Use system's natural duration
        Duration = VFX->GetSystemInstance()->GetMaxLifetime();
    }

    // Schedule release
    FTimerHandle ReleaseTimer;
    GetWorld()->GetTimerManager().SetTimer(
        ReleaseTimer,
        [this, VFX]() {
            ReleaseVFX(VFX);
        },
        Duration,
        false  // Don't loop
    );
}
```

**Usage example**:
```cpp
// In game code (weapon fire)
void AWeapon::Fire() {
    // WRONG: Creates and destroys VFX every shot
    // UNiagaraFunctionLibrary::SpawnSystemAtLocation(...);  // Don't do this

    // RIGHT: Use pooled VFX
    UVFXPoolManager* VFXManager = GetWorld()->GetSubsystem<UVFXPoolManager>();
    VFXManager->SpawnPooledVFX(
        MuzzleFlashVFX,
        MuzzleLocation,
        MuzzleRotation,
        0.5f  // Muzzle flash lasts 0.5s
    );
}
```

**Pool statistics**:
```cpp
void UVFXPoolManager::LogPoolStatistics() {
    for (auto& PoolPair : Pools) {
        UNiagaraSystem* SystemTemplate = PoolPair.Key;
        FVFXPool& Pool = PoolPair.Value;

        int32 TotalSize = Pool.AvailableVFX.Num() + Pool.ActiveVFX.Num();
        int32 Available = Pool.AvailableVFX.Num();
        int32 Active = Pool.ActiveVFX.Num();

        float UtilizationPercent = (float)Active / (float)TotalSize * 100.f;

        UE_LOG(LogVFX, Log,
            TEXT("%s: Total=%d, Available=%d, Active=%d (%.1f%% utilized)"),
            *SystemTemplate->GetName(),
            TotalSize,
            Available,
            Active,
            UtilizationPercent
        );
    }
}
```

---

### How UI Bindings Work

**Binding concept**:
```
UI Binding connects widget properties to data sources.

Example:
Widget property: "ProgressBar.Percent"
Data source: "PlayerShip.GetShieldPercentage()"

Engine automatically:
- Calls GetShieldPercentage() every frame (or on demand)
- Updates ProgressBar.Percent with returned value
- Widget displays current shield percentage
```

**Binding types**:

**1. Function binding** (most common):
```cpp
// Widget class
UFUNCTION(BlueprintPure)
float GetShieldPercent() const {
    ASpaceship* Ship = GetOwningPlayerPawn<ASpaceship>();
    return Ship ? Ship->GetShieldPercentage() : 0.f;
}

// In Widget Designer:
// ProgressBar → Percent → Bind → GetShieldPercent
```

**2. Property binding**:
```cpp
// Widget class
UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
float ShieldPercentage;

// Update manually
void UMyWidget::NativeTick(const FGeometry& Geometry, float DeltaTime) {
    Super::NativeTick(Geometry, DeltaTime);

    ASpaceship* Ship = GetOwningPlayerPawn<ASpaceship>();
    ShieldPercentage = Ship ? Ship->GetShieldPercentage() : 0.f;
}

// In Widget Designer:
// ProgressBar → Percent → Bind → ShieldPercentage (property)
```

**3. Event-driven binding**:
```cpp
// Data source broadcasts changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShieldChanged, float, NewPercent);

UCLASS()
class ASpaceship : public APawn {
    UPROPERTY(BlueprintAssignable)
    FOnShieldChanged OnShieldChanged;

    void TakeDamage(float Damage) {
        CurrentShield -= Damage;
        OnShieldChanged.Broadcast(GetShieldPercentage());  // Notify listeners
    }
};

// Widget subscribes
void UMyWidget::NativeConstruct() {
    Super::NativeConstruct();

    ASpaceship* Ship = GetOwningPlayerPawn<ASpaceship>();
    Ship->OnShieldChanged.AddDynamic(this, &UMyWidget::HandleShieldChanged);
}

void UMyWidget::HandleShieldChanged(float NewPercent) {
    // Update widget
    ShieldProgressBar->SetPercent(NewPercent);
}
```

**Binding performance**:
```cpp
// Function bindings are called EVERY FRAME (expensive!)

// EXPENSIVE (called 60 times per second):
UFUNCTION(BlueprintPure)
FText GetComplexCalculation() const {
    // Heavy computation here
    return FText::FromString(...);
}

// OPTIMIZED (cached, updated on change):
UPROPERTY()
FText CachedText;

void UpdateTextWhenNeeded() {
    CachedText = CalculateComplexText();
}

UFUNCTION(BlueprintPure)
FText GetCachedText() const {
    return CachedText;  // Fast, just returns cached value
}
```

**Conditional binding updates**:
```cpp
// Only update when data changes
UFUNCTION(BlueprintPure)
FText GetShieldText() const {
    static float LastShieldPercent = -1.f;
    static FText CachedText;

    float CurrentPercent = GetShieldPercent();

    if (!FMath::IsNearlyEqual(CurrentPercent, LastShieldPercent, 0.01f)) {
        // Shield changed, update cached text
        LastShieldPercent = CurrentPercent;
        CachedText = FText::Format(
            LOCTEXT("Shield", "Shields: {0}%"),
            FText::AsPercent(CurrentPercent)
        );
    }

    return CachedText;
}
```

**Widget reflector debugging**:
```
Unreal Editor → Window → Developer Tools → Widget Reflector
OR
Console command: WidgetReflector
OR
Keyboard shortcut: Ctrl+Shift+W

Shows:
- All active widgets
- Widget hierarchy
- Which widgets are updating
- Performance cost of bindings
- "Expensive bindings" highlighted in red
```

**Best practices**:
- Use event-driven updates when possible (most efficient)
- Cache expensive calculations
- Disable widget ticking when not visible
- Use `meta = (BindWidget)` for compile-time widget references
- Avoid complex logic in binding functions
- Profile binding performance with Widget Reflector

---

### How Profiling Subsystem Works

**Subsystem architecture**:
```cpp
UCLASS()
class UPerformanceProfilerSubsystem : public UGameInstanceSubsystem {
    GENERATED_BODY()

public:
    // Subsystem lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Profiling API
    UFUNCTION(BlueprintCallable)
    void BeginFrame();

    UFUNCTION(BlueprintCallable)
    void EndFrame();

    UFUNCTION(BlueprintCallable)
    void TrackSystemBegin(FName SystemName);

    UFUNCTION(BlueprintCallable)
    void TrackSystemEnd(FName SystemName);

    UFUNCTION(BlueprintCallable)
    FProfileData GetProfileData() const;

    UFUNCTION(BlueprintCallable)
    void SaveProfileToJSON(const FString& Filename);

private:
    // Frame timing
    double FrameStartTime;
    double TotalFrameTime;

    // System timing
    UPROPERTY()
    TMap<FName, FSystemTimingData> SystemTimings;

    // Historical data
    UPROPERTY()
    TArray<FFrameProfileData> FrameHistory;

    // Current system being timed
    FName CurrentSystem;
    double SystemStartTime;
};

USTRUCT()
struct FSystemTimingData {
    GENERATED_BODY()

    double TotalTime;
    double MinTime;
    double MaxTime;
    double AvgTime;
    int32 CallCount;
};
```

**Instrumentation**:
```cpp
// Automatic instrumentation using RAII
class FScopedSystemProfiler {
public:
    FScopedSystemProfiler(FName SystemName)
        : SystemName(SystemName)
    {
        UPerformanceProfilerSubsystem* Profiler = GetProfiler();
        if (Profiler) {
            Profiler->TrackSystemBegin(SystemName);
        }
    }

    ~FScopedSystemProfiler() {
        UPerformanceProfilerSubsystem* Profiler = GetProfiler();
        if (Profiler) {
            Profiler->TrackSystemEnd(SystemName);
        }
    }

private:
    FName SystemName;
};

// Usage in game code:
void UMissionBoardComponent::Tick(float DeltaTime) {
    SCOPED_SYSTEM_PROFILER("MissionBoard");  // Macro wraps FScopedSystemProfiler

    Super::Tick(DeltaTime);

    // Mission board logic here...
    UpdateAvailableMissions();
    CheckPlayerProximity();
}

// Profiler automatically tracks:
// - When MissionBoard system started
// - When it ended
// - Time spent in system
```

**Frame profiling**:
```cpp
void UPerformanceProfilerSubsystem::BeginFrame() {
    FrameStartTime = FPlatformTime::Seconds();
}

void UPerformanceProfilerSubsystem::EndFrame() {
    double FrameEndTime = FPlatformTime::Seconds();
    TotalFrameTime = (FrameEndTime - FrameStartTime) * 1000.0;  // Convert to ms

    // Store frame data
    FFrameProfileData FrameData;
    FrameData.FrameNumber = GFrameNumber;
    FrameData.TotalTime = TotalFrameTime;
    FrameData.SystemTimings = SystemTimings;

    FrameHistory.Add(FrameData);

    // Limit history size
    if (FrameHistory.Num() > 300) {  // Keep last 5 seconds at 60 FPS
        FrameHistory.RemoveAt(0);
    }

    // Reset for next frame
    SystemTimings.Empty();
}
```

**System tracking**:
```cpp
void UPerformanceProfilerSubsystem::TrackSystemBegin(FName SystemName) {
    CurrentSystem = SystemName;
    SystemStartTime = FPlatformTime::Seconds();
}

void UPerformanceProfilerSubsystem::TrackSystemEnd(FName SystemName) {
    if (SystemName != CurrentSystem) {
        UE_LOG(LogPerformance, Warning,
            TEXT("Mismatched profiler end: expected %s, got %s"),
            *CurrentSystem.ToString(),
            *SystemName.ToString()
        );
        return;
    }

    double SystemEndTime = FPlatformTime::Seconds();
    double SystemDuration = (SystemEndTime - SystemStartTime) * 1000.0;  // ms

    // Update system timing data
    FSystemTimingData& TimingData = SystemTimings.FindOrAdd(SystemName);
    TimingData.TotalTime += SystemDuration;
    TimingData.CallCount++;

    if (TimingData.CallCount == 1) {
        TimingData.MinTime = SystemDuration;
        TimingData.MaxTime = SystemDuration;
    } else {
        TimingData.MinTime = FMath::Min(TimingData.MinTime, SystemDuration);
        TimingData.MaxTime = FMath::Max(TimingData.MaxTime, SystemDuration);
    }

    TimingData.AvgTime = TimingData.TotalTime / TimingData.CallCount;

    CurrentSystem = NAME_None;
}
```

**JSON export**:
```cpp
void UPerformanceProfilerSubsystem::SaveProfileToJSON(const FString& Filename) {
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);

    // Overall stats
    RootObject->SetNumberField("total_frames", FrameHistory.Num());

    // Frame time statistics
    double AvgFrameTime = 0.0;
    double MinFrameTime = DBL_MAX;
    double MaxFrameTime = 0.0;

    for (const FFrameProfileData& Frame : FrameHistory) {
        AvgFrameTime += Frame.TotalTime;
        MinFrameTime = FMath::Min(MinFrameTime, Frame.TotalTime);
        MaxFrameTime = FMath::Max(MaxFrameTime, Frame.TotalTime);
    }
    AvgFrameTime /= FrameHistory.Num();

    RootObject->SetNumberField("avg_frame_time_ms", AvgFrameTime);
    RootObject->SetNumberField("min_frame_time_ms", MinFrameTime);
    RootObject->SetNumberField("max_frame_time_ms", MaxFrameTime);
    RootObject->SetNumberField("avg_fps", 1000.0 / AvgFrameTime);

    // System-specific stats
    TSharedPtr<FJsonObject> SystemsObject = MakeShareable(new FJsonObject);

    for (auto& Pair : SystemTimings) {
        FName SystemName = Pair.Key;
        FSystemTimingData& TimingData = Pair.Value;

        TSharedPtr<FJsonObject> SystemObject = MakeShareable(new FJsonObject);
        SystemObject->SetNumberField("avg_ms", TimingData.AvgTime);
        SystemObject->SetNumberField("min_ms", TimingData.MinTime);
        SystemObject->SetNumberField("max_ms", TimingData.MaxTime);
        SystemObject->SetNumberField("total_ms", TimingData.TotalTime);
        SystemObject->SetNumberField("call_count", TimingData.CallCount);

        SystemsObject->SetObjectField(SystemName.ToString(), SystemObject);
    }

    RootObject->SetObjectField("systems", SystemsObject);

    // Write to file
    FString JsonString;
    TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), JsonWriter);

    FFileHelper::SaveStringToFile(JsonString, *Filename);
}
```

**Automated profiling script**:
```python
# autonomous_profiling_test.py
import unreal
import time
import json

def run_profiling_session(duration_seconds=60):
    # Get profiler subsystem
    profiler = unreal.get_engine_subsystem(unreal.PerformanceProfilerSubsystem)

    # Start profiling
    profiler.begin_profiling()

    # Run game for specified duration
    start_time = time.time()
    while (time.time() - start_time) < duration_seconds:
        unreal.tick_engine()
        time.sleep(0.016)  # ~60 FPS tick rate

    # Stop profiling
    profiler.end_profiling()

    # Export results
    profiler.save_profile_to_json("END_TO_END_TEST_REPORT.json")

    # Generate readable report
    generate_markdown_report()

if __name__ == "__main__":
    run_profiling_session(60)
```

**Integration with CI/CD**:
```yaml
# .github/workflows/performance-regression.yml
name: Performance Regression Check

on: [push, pull_request]

jobs:
  profile:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v2

      - name: Run profiling
        run: python autonomous_profiling_test.py

      - name: Load baseline
        run: |
          $baseline = Get-Content baseline_performance.json | ConvertFrom-Json
          $current = Get-Content END_TO_END_TEST_REPORT.json | ConvertFrom-Json

      - name: Check for regressions
        run: |
          if ($current.avg_frame_time_ms > $baseline.avg_frame_time_ms * 1.1) {
            Write-Error "Performance regression detected!"
            exit 1
          }

      - name: Upload report
        uses: actions/upload-artifact@v2
        with:
          name: profiling-report
          path: END_TO_END_TEST_REPORT.*
```

---

## Tools Reference

### Unreal Editor Shortcuts

**Navigation**:
```
W, A, S, D     - Move camera (when right-click held)
Q, E           - Move camera down/up
Right-click    - Free-look camera
Middle-click   - Pan camera
Scroll wheel   - Zoom in/out
Alt + Left     - Orbit around selection
F              - Focus on selected object
Ctrl + Tab     - Cycle viewports
```

**Selection and transform**:
```
W              - Translate (move) mode
E              - Rotate mode
R              - Scale mode
Spacebar       - Cycle through modes
Ctrl + D       - Duplicate selected
Ctrl + W       - Duplicate and move
End            - Snap to floor
Ctrl + End     - Align to ground normal
Alt + drag     - Duplicate while dragging
```

**Editing**:
```
Ctrl + Z       - Undo
Ctrl + Y       - Redo
Del            - Delete selected
Ctrl + C/V     - Copy/paste
Ctrl + X       - Cut
Ctrl + A       - Select all
Ctrl + Shift + A - Deselect all
H              - Hide selected
Ctrl + H       - Unhide all
G              - Toggle game view (hide editor icons)
```

**Play and simulation**:
```
Alt + P        - Play in editor (PIE)
Alt + S        - Simulate
Esc            - Stop playing
F8             - Eject from player (free camera while playing)
Pause          - Pause game
.              - Step one frame (while paused)
Shift + F1     - Mouse cursor in PIE
```

**Content Browser**:
```
Ctrl + B       - Find selected in Content Browser
Ctrl + Shift + F - Find in Content Browser
Ctrl + S       - Save selected asset
Ctrl + Shift + S - Save all
Ctrl + P       - Open asset picker
```

**Useful windows**:
```
Ctrl + Shift + W   - Widget Reflector
Ctrl + Shift + ,   - Project Settings
Alt + Shift + ,    - Editor Preferences
Ctrl + Shift + O   - Output Log
Ctrl + Shift + D   - Developer Tools menu
```

**Blueprint shortcuts**:
```
Ctrl + F       - Find in Blueprint
Ctrl + Shift + F - Find in all Blueprints
F9             - Toggle breakpoint
F11            - Step into (debugging)
F10            - Step over (debugging)
C              - Add Comment
```

**Viewport show flags**:
```
F1             - Wireframe
F2             - Unlit
F3             - Lit (default)
F4             - Detail lighting
F5             - Shader complexity
```

---

### Python Console Tips

**Launching Python console**:
```
Unreal Editor → Window → Developer Tools → Python Console

Or:
Cmd key (`) → type python
```

**Basic commands**:
```python
# List all actors in level
actors = unreal.EditorLevelLibrary.get_all_level_actors()
for actor in actors:
    print(actor.get_name())

# Load asset
asset = unreal.EditorAssetLibrary.load_asset("/Game/Maps/StationAlpha")
print(asset)

# Spawn actor
location = unreal.Vector(0, 0, 100)
rotation = unreal.Rotator(0, 0, 0)
actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.Actor.static_class(),
    location,
    rotation
)

# Save level
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
```

**Asset manipulation**:
```python
# Find all assets of type
import unreal

# Find all Static Meshes
asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
assets = asset_registry.get_assets_by_class("StaticMesh")

for asset_data in assets:
    print(asset_data.asset_name)

# Batch rename
for asset_data in assets:
    old_name = asset_data.asset_name
    new_name = "SM_" + old_name
    unreal.EditorAssetLibrary.rename_asset(
        asset_data.object_path,
        "/Game/Meshes/" + new_name
    )
```

**Performance profiling**:
```python
# Run performance test
import unreal
import time

start = time.time()

# Your code here
for i in range(1000):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(...)

end = time.time()
print(f"Took {end - start:.2f} seconds")
```

**Automation**:
```python
# Run external Python script from console
exec(open("C:/path/to/script.py").read())

# Or import as module
import sys
sys.path.append("C:/path/to/scripts")
import my_script
my_script.run()
```

---

### Git LFS Commands

**Setup**:
```bash
# Install Git LFS (one-time, per machine)
git lfs install

# Track file types
git lfs track "*.uasset"
git lfs track "*.umap"
git lfs track "*.png"

# Commit .gitattributes
git add .gitattributes
git commit -m "Configure Git LFS"
```

**Checking status**:
```bash
# List tracked files
git lfs ls-files

# Check what's being tracked
git lfs status

# See LFS configuration
cat .gitattributes
```

**Fetching/pulling**:
```bash
# Fetch LFS files
git lfs fetch

# Pull with LFS
git lfs pull

# Pull specific file
git lfs pull --include="Content/Maps/StationAlpha.umap"
```

**Troubleshooting**:
```bash
# If LFS files are pointers (not actual files):
git lfs pull --force

# Migrate existing files to LFS
git lfs migrate import --include="*.uasset"

# Prune old LFS files
git lfs prune
```

---

### Profiling Commands

**Unreal console commands**:
```cpp
// Frame time breakdown
stat unit

// FPS display
stat fps

// Game thread details
stat game

// Rendering stats
stat scenerendering

// GPU breakdown
stat gpu

// Particle performance
stat particles

// Memory usage
stat memory

// Draw call stats
stat rhi

// Slow game thread functions
stat slow
```

**Capture profiling session**:
```cpp
// Start capture
stat startfile

// ... play for 30-60 seconds ...

// Stop capture
stat stopfile

// Opens .ue4stats file in Saved/Profiling/
// Analyze in Unreal Frontend
```

**Console variables**:
```cpp
// Show all stats on screen
stat fps 1
stat unit 1
stat gpu 1

// Frame rate limits (for testing)
t.MaxFPS 60
t.UnsmoothedFPS 1  // Show actual FPS, not smoothed

// Resolution scale (performance testing)
r.ScreenPercentage 50  // Render at 50% resolution
```

---

### Debug Console Commands

**Gameplay debugging**:
```cpp
// Player info
GetAll PlayerController
GetAll Pawn

// Spawn actor
Summon Actor

// Teleport
Teleport

// God mode
God

// Toggle collision
CollisionVisualization

// Show collision
show collision
```

**Performance**:
```cpp
// Slow motion
slomo 0.5  // 50% speed

// Frame step
pause
step  // Advance one frame

// Freeze rendering
freezerendering

// Disable occlusion
r.AllowOcclusionQueries 0
```

**Rendering**:
```cpp
// Wireframe
viewmode wireframe

// Unlit
viewmode unlit

// Shader complexity
viewmode shadercomplexity

// Toggle post-processing
show postprocessing

// Toggle VFX
fx.Niagara.Debug.Enabled 1
```

---

## Summary

This knowledge base provides comprehensive conceptual understanding, best practices, architectural details, and tool references for the Alexander autonomous baseline system. Use in conjunction with:

- **BASELINE_FAQ.md**: Quick answers to common questions
- **GLOSSARY.md**: Term definitions and examples
- **BUILD_INSTRUCTIONS_PHASE9.md**: Step-by-step implementation guide
- **PHASE9_STATUS_REPORT.md**: Current project status

For support, consult project documentation or community resources.
