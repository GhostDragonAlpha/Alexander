# Alexander Autonomous Baseline - Glossary

Comprehensive glossary of technical terms used throughout the Alexander project documentation, organized by category.

---

## Table of Contents

1. [Unreal Engine Terminology](#unreal-engine-terminology)
2. [VR Terminology](#vr-terminology)
3. [Performance Terminology](#performance-terminology)
4. [Git Terminology](#git-terminology)
5. [General Game Development](#general-game-development)
6. [Alexander-Specific Terms](#alexander-specific-terms)

---

## Unreal Engine Terminology

### Actor
**Definition**: The base class for all objects that can be placed or spawned in an Unreal level.

**Example usage**: "Spawn an actor at the player's location to represent the explosion effect."

**Related terms**: Component, Pawn, Character, Level

**Technical details**:
```cpp
AActor* MyActor = GetWorld()->SpawnActor<AActor>(
    ActorClass,
    Location,
    Rotation
);
```

---

### Blueprint
**Definition**: Unreal's visual scripting system that allows creating gameplay logic without writing C++ code.

**Example usage**: "Create a Blueprint to handle the mission acceptance workflow."

**Related terms**: C++, Visual Scripting, Event Graph, Construction Script

**Technical details**: Blueprints compile to bytecode and can inherit from C++ classes or other Blueprints. While convenient, they're generally slower than equivalent C++ code.

---

### BSP (Binary Space Partitioning)
**Definition**: Unreal's legacy geometry system, now largely replaced by Static Meshes. Used for quick level blocking.

**Example usage**: "Block out the level layout using BSP brushes before creating final meshes."

**Related terms**: CSG, Geometry Mode, Additive/Subtractive Brushes

**Technical details**: BSP uses boolean operations (additive/subtractive brushes) to create geometry. Deprecated for production use but still useful for prototyping.

---

### Component
**Definition**: Modular functionality that can be added to Actors. Examples: MeshComponent, CameraComponent, LightComponent.

**Example usage**: "Add a UNiagaraComponent to the spaceship actor for engine trails."

**Related terms**: Actor, Actor Component, Scene Component

**Technical details**:
```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
UStaticMeshComponent* MeshComponent;

MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
```

---

### CSG (Constructive Solid Geometry)
**Definition**: Technique for creating complex shapes by combining primitive shapes (cubes, spheres, cylinders) using boolean operations.

**Example usage**: "Use CSG to create station interiors from primitive boxes and cylinders."

**Related terms**: BSP, Primitive, Boolean Operations, Geometry Mode

**Technical details**: In Unreal, CSG operations include Add, Subtract, and Intersect. The Alexander baseline uses CSG extensively for rapid prototyping.

---

### Data Asset
**Definition**: A UObject-derived asset that stores structured data, created and edited in the Unreal Editor.

**Example usage**: "Create a MissionTemplateDataAsset to define mission parameters."

**Related terms**: Data Table, UObject, Asset

**Technical details**:
```cpp
UCLASS(BlueprintType)
class UMissionTemplateDataAsset : public UDataAsset {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Difficulty;
};
```

---

### Data Table
**Definition**: Spreadsheet-like asset for storing rows of structured data, often used for configuration.

**Example usage**: "Store weapon stats in a Data Table for easy balancing."

**Related terms**: Data Asset, CSV, Struct

**Technical details**: Data Tables are backed by USTRUCTs and can be imported from CSV files. Each row has a unique name and contains all fields from the struct.

---

### GENERATED_BODY
**Definition**: Unreal macro required in all UCLASS, USTRUCT, and UENUM declarations for reflection system.

**Example usage**:
```cpp
UCLASS()
class AMyActor : public AActor {
    GENERATED_BODY()  // Required!
};
```

**Related terms**: UCLASS, USTRUCT, Reflection, UHT (Unreal Header Tool)

**Technical details**: This macro is replaced by Unreal Header Tool during build with generated code for reflection, serialization, and garbage collection.

---

### HUD (Heads-Up Display)
**Definition**: On-screen UI elements that display game information (health, ammo, etc.) during gameplay.

**Example usage**: "The HUD displays shield status and current mission objectives."

**Related terms**: UMG, Widget, UI, Canvas

**Technical details**:
```cpp
UCLASS()
class AMyHUD : public AHUD {
    GENERATED_BODY()

    virtual void DrawHUD() override;  // Legacy Canvas drawing
};
```

Modern approach uses UMG widgets instead of Canvas drawing.

---

### Level
**Definition**: A contained game environment, typically one map or scene. Also called "Map".

**Example usage**: "Load the StationAlpha level when the player accepts a docking mission."

**Related terms**: Map, World, Sub-Level, Streaming Level

**Technical details**: Levels are stored as .umap files. Multiple levels can be loaded simultaneously using level streaming.

---

### LOD (Level of Detail)
**Definition**: Technique where objects have multiple versions of decreasing complexity, swapped based on distance from camera.

**Example usage**: "Create LOD0 (high detail), LOD1 (medium), and LOD2 (low) for the station mesh."

**Related terms**: Mesh LOD, Material LOD, HLOD, Culling

**Technical details**:
```cpp
// LOD distances
LOD0: 0-1000 units (highest detail)
LOD1: 1000-2500 units
LOD2: 2500-5000 units
LOD3: 5000+ units (lowest detail)
```

---

### Material
**Definition**: Defines the visual surface properties of a mesh (color, roughness, metallic, etc.).

**Example usage**: "Create a metal material with high metallic value and low roughness for the spaceship hull."

**Related terms**: Shader, Texture, Material Instance, PBR (Physically Based Rendering)

**Technical details**: Materials are node-based shaders. Material Instances allow parameter overrides without creating new materials.

---

### Niagara
**Definition**: Unreal's modern VFX system for creating particle effects, ribbons, and sprite effects.

**Example usage**: "Use Niagara to create engine trails that spawn 500 particles per second."

**Related terms**: Cascade (legacy), VFX, Particle System, Emitter

**Technical details**: Niagara supports GPU and CPU simulation, modular emitters, events, and complex data-driven effects.

---

### Pawn
**Definition**: An Actor that can be "possessed" by a player or AI controller to receive input.

**Example usage**: "The Spaceship class inherits from Pawn to receive player input."

**Related terms**: Actor, Character, Controller, PlayerController, AIController

**Technical details**:
```cpp
UCLASS()
class ASpaceship : public APawn {
    GENERATED_BODY()

    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
};
```

---

### Static Mesh
**Definition**: A 3D model asset that doesn't deform or animate (unlike Skeletal Mesh).

**Example usage**: "Import the station model as a Static Mesh."

**Related terms**: Skeletal Mesh, Mesh, Geometry, LOD

**Technical details**: Static Meshes can have collision, LODs, sockets, and material slots. They're optimized for static or simple animated objects (like rotating doors).

---

### Subsystem
**Definition**: Singleton-like system that manages global functionality, lifetime tied to Engine, World, or GameInstance.

**Example usage**: "Create a PerformanceProfilerSubsystem to track system performance globally."

**Related terms**: Singleton, System, Manager

**Technical details**:
```cpp
UCLASS()
class UPerformanceProfilerSubsystem : public UGameInstanceSubsystem {
    GENERATED_BODY()

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
};
```

---

### Tick
**Definition**: Function called every frame by the engine to update an Actor or Component.

**Example usage**: "Disable ticking on UI widgets that don't need per-frame updates."

**Related terms**: Delta Time, Frame, Update Loop

**Technical details**:
```cpp
virtual void Tick(float DeltaTime) override {
    Super::Tick(DeltaTime);

    // Called every frame
    // DeltaTime = time since last frame (seconds)
}
```

Performance consideration: Each ticking actor/component has overhead. Disable ticking when possible.

---

### UCLASS
**Definition**: Macro marking a C++ class for Unreal's reflection system, enabling Blueprint exposure, serialization, and garbage collection.

**Example usage**:
```cpp
UCLASS(BlueprintType)
class ALEXANDER_API ASpaceship : public APawn {
    GENERATED_BODY()
};
```

**Related terms**: GENERATED_BODY, Reflection, UPROPERTY, UFUNCTION

**Technical details**: UCLASS() can include specifiers like BlueprintType, Blueprintable, Abstract, NotBlueprintable, etc.

---

### UMG (Unreal Motion Graphics)
**Definition**: Unreal's UI framework for creating HUDs, menus, and interactive interfaces.

**Example usage**: "Design the mission board UI in UMG with drag-and-drop widgets."

**Related terms**: Widget, Slate, Canvas, HUD

**Technical details**: UMG is built on Slate (lower-level UI framework). It provides visual designer and Blueprint/C++ integration.

---

### UPROPERTY
**Definition**: Macro marking a C++ variable for reflection, enabling editor editing, Blueprint access, and serialization.

**Example usage**:
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
float MaxShield = 100.f;
```

**Related terms**: UCLASS, UFUNCTION, Reflection, Specifiers

**Common specifiers**:
- EditAnywhere: Editable in editor and archetype
- BlueprintReadWrite: Blueprint can read and write
- Category: Organizes in editor
- VisibleAnywhere: Visible but not editable

---

### USTRUCT
**Definition**: Macro marking a C++ struct for Unreal's reflection system.

**Example usage**:
```cpp
USTRUCT(BlueprintType)
struct FMissionReward {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    int32 Credits;

    UPROPERTY(EditAnywhere)
    int32 Experience;
};
```

**Related terms**: UCLASS, Struct, Data Table

**Technical details**: USTRUCTs don't have garbage collection (use UCLASS for that) but get reflection, serialization, and Blueprint exposure.

---

### World
**Definition**: The highest-level container for all game objects in a running game instance.

**Example usage**: "Spawn the explosion VFX in the World at the projectile's impact location."

**Related terms**: Level, Map, UWorld

**Technical details**:
```cpp
UWorld* World = GetWorld();
World->SpawnActor<AActor>(...);
```

---

## VR Terminology

### 6DOF (Six Degrees of Freedom)
**Definition**: Full movement freedom in VR - three translational axes (X, Y, Z) and three rotational axes (pitch, yaw, roll).

**Example usage**: "The VR flight controller supports 6DOF for realistic spaceship control."

**Related terms**: 3DOF, Room-Scale, Positional Tracking

**Technical details**:
- Translational: Forward/back, left/right, up/down
- Rotational: Pitch (look up/down), yaw (look left/right), roll (tilt head)

Contrasts with 3DOF which only tracks rotation.

---

### Dynamic Resolution
**Definition**: Technique that automatically adjusts rendering resolution to maintain target frame rate.

**Example usage**: "Enable dynamic resolution to keep VR at 90 FPS even during heavy scenes."

**Related terms**: Adaptive Quality, Frame Budget, Resolution Scaling

**Technical details**:
```cpp
r.DynamicRes.OperationMode 2  // Enable
r.DynamicRes.TargetFPS 90     // Target
r.DynamicRes.MinScreenPercentage 50   // Lowest res allowed
r.DynamicRes.MaxScreenPercentage 100  // Highest res allowed
```

System drops resolution when frame time exceeds budget, raises it when headroom available.

---

### Foveated Rendering
**Definition**: Rendering technique that renders the center of view (fovea) at high quality and periphery at lower quality, matching human vision.

**Example usage**: "Use fixed foveated rendering on Quest to save 20% GPU time."

**Related terms**: Eye Tracking, Peripheral Vision, Variable Rate Shading

**Technical details**:
- Fixed foveation: Assumes center of screen is fovea (no eye tracking)
- Dynamic foveation: Uses eye tracking to follow gaze
- Performance gain: 15-30% depending on aggressiveness

---

### Frame Budget
**Definition**: Maximum time available to render one frame to maintain target refresh rate.

**Example usage**: "VR frame budget at 90 Hz is 11.1ms per frame."

**Related terms**: Frame Time, Refresh Rate, Performance Budget

**Technical details**:
```
90 Hz VR: 11.1ms budget
120 Hz VR: 8.3ms budget
60 Hz desktop: 16.6ms budget

Missing budget = dropped frame = judder/nausea in VR
```

---

### HMD (Head-Mounted Display)
**Definition**: VR headset worn on the head containing displays and tracking sensors.

**Example usage**: "Test the cockpit UI in the HMD to ensure readability at expected viewing distance."

**Related terms**: VR Headset, Display, Optics, Lens

**Common HMDs**:
- Meta Quest 2/3: Standalone VR
- Valve Index: High-end PC VR
- PlayStation VR2: Console VR

---

### IPD (Interpupillary Distance)
**Definition**: Distance between the centers of the pupils of the two eyes. Critical for proper VR display alignment.

**Example usage**: "Test UI with different IPD settings to ensure readability for all users."

**Related terms**: Eye Distance, Stereo Separation

**Technical details**:
- Average IPD: 63mm
- Range: 54-74mm
- Incorrect IPD causes eye strain, headaches, scale perception issues

---

### Judder
**Definition**: Visual stuttering/hitching in VR caused by inconsistent frame timing or dropped frames.

**Example usage**: "VFX particle burst caused judder - reduce particle count to maintain frame budget."

**Related terms**: Stutter, Frame Drop, Reprojection

**Technical details**: In VR, judder is more noticeable and can cause nausea. Consistent 90 FPS is critical; occasional 89 FPS is better than alternating 90/60.

---

### Motion Sickness
**Definition**: Nausea and discomfort in VR caused by sensory mismatch between visual movement and physical stillness.

**Example usage**: "Avoid sudden camera rotations to reduce motion sickness in VR flight."

**Related terms**: VR Sickness, Simulator Sickness, Comfort

**Causes**:
- Inconsistent frame rate
- Artificial locomotion
- Camera acceleration
- Mismatch between visual and vestibular input

**Mitigation**:
- Maintain 90+ FPS
- Use physical movement when possible
- Provide static reference frame (cockpit)
- Smooth, predictable motion

---

### Positional Tracking
**Definition**: VR system's ability to track the HMD's position in 3D space (not just rotation).

**Example usage**: "Positional tracking allows players to lean around corners in VR."

**Related terms**: 6DOF, Inside-Out Tracking, Outside-In Tracking

**Technical details**:
- Inside-out: Cameras on HMD track environment
- Outside-in: External sensors track HMD
- Tracking volume: Physical space where tracking works

---

### Refresh Rate
**Definition**: How many times per second the VR display updates. Higher = smoother.

**Example usage**: "Index runs at 120 Hz refresh rate, requiring 8.3ms frame budget."

**Related terms**: Frame Rate, Hz, Display

**Common rates**:
- 90 Hz: Standard VR (Quest 2, Vive)
- 120 Hz: High-end VR (Index, Quest 3, PSVR2)
- 144 Hz: Enthusiast VR (Index max mode)

---

### Reprojection
**Definition**: Technique that uses head tracking to warp previous frame when new frame isn't ready, hiding dropped frames.

**Example usage**: "Reprojection saved us when frame time spiked to 15ms, but we should still optimize."

**Related terms**: ASW (Asynchronous Spacewarp), ATW (Asynchronous Timewarp), Motion Smoothing

**Technical details**: Reprojection can hide occasional frame drops but adds artifacts (especially with fast motion) and increases latency. Not a substitute for hitting frame budget.

---

### Room-Scale VR
**Definition**: VR experience where player can physically walk around a tracked space.

**Example usage**: "Station interior supports room-scale VR for walking around the docking bay."

**Related terms**: Positional Tracking, Play Space, Guardian

**Technical details**: Typical room-scale space is 2m × 2m minimum, 3m × 3m recommended. Requires guardian/chaperone boundaries.

---

### Stereo Rendering
**Definition**: Rendering the scene twice from slightly different viewpoints (one per eye) to create 3D depth perception.

**Example usage**: "Instanced stereo rendering reduces VR rendering cost by 30%."

**Related terms**: Stereoscopy, 3D Vision, Parallax

**Technical details**:
- Naive: Render left eye, render right eye (2× cost)
- Instanced stereo: Render both eyes in one pass (1.3× cost)
- Single-pass stereo: Further optimization

---

### Widget Component
**Definition**: Component that displays a UMG widget in 3D world space instead of screen space.

**Example usage**: "Attach a Widget Component to the cockpit to display mission info in VR."

**Related terms**: 3D UI, World Widget, UMG

**Technical details**:
```cpp
UWidgetComponent* VRWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("VRWidget"));
VRWidget->SetWidgetSpace(EWidgetSpace::World);  // 3D space
VRWidget->SetDrawSize(FVector2D(1920, 1080));   // Virtual resolution
```

---

## Performance Terminology

### Bottleneck
**Definition**: The slowest part of the rendering/game loop that limits overall performance.

**Example usage**: "GPU is the bottleneck - reduce particle counts to improve frame rate."

**Related terms**: Performance Limit, Constraint, Critical Path

**Identification**:
```cpp
stat unit  // Shows frame, game, draw, GPU times
// Highest value is your bottleneck
```

---

### CPU-Bound
**Definition**: Performance limited by CPU, not GPU. Adding more geometry doesn't hurt; optimizing shaders doesn't help.

**Example usage**: "Game is CPU-bound due to complex Blueprint tick functions."

**Related terms**: Game Thread, Logic Bottleneck

**Solutions**:
- Reduce tick frequency
- Optimize algorithms
- Move work to GPU
- Use multithreading
- Reduce actor count

---

### Culling
**Definition**: Technique that skips rendering/updating objects not visible or relevant to player.

**Example usage**: "Frustum culling skips rendering objects outside camera view."

**Related terms**: Occlusion Culling, Distance Culling, Frustum Culling

**Types**:
- **Frustum culling**: Objects outside camera view
- **Occlusion culling**: Objects behind other objects
- **Distance culling**: Objects too far away
- **Significance culling**: Objects deemed not important

---

### Delta Time
**Definition**: Time elapsed since previous frame, used to make movement frame-rate independent.

**Example usage**: "Multiply velocity by DeltaTime to ensure consistent speed regardless of frame rate."

**Related terms**: Frame Time, dt, Time Step

**Technical details**:
```cpp
void Tick(float DeltaTime) {
    // DeltaTime at 60 FPS: ~0.0166 seconds
    // DeltaTime at 30 FPS: ~0.0333 seconds

    // Frame-rate independent movement:
    Position += Velocity * DeltaTime;

    // WRONG (frame-rate dependent):
    Position += Velocity;
}
```

---

### Draw Call
**Definition**: Command from CPU to GPU to render a specific mesh with a specific material.

**Example usage**: "Scene has 5000 draw calls - reduce by batching similar objects."

**Related terms**: Batch, Rendering, RHI (Rendering Hardware Interface)

**Technical details**: Each draw call has CPU overhead. Reducing draw calls improves performance:
- Merge static meshes
- Use instancing
- Use atlased textures
- Reduce unique materials

---

### FPS (Frames Per Second)
**Definition**: Number of frames rendered per second. Higher = smoother gameplay.

**Example usage**: "Target 90 FPS for VR, 60 FPS for desktop."

**Related terms**: Frame Rate, Refresh Rate, Frame Time

**Conversion**:
```
60 FPS = 16.6ms per frame
90 FPS = 11.1ms per frame
120 FPS = 8.3ms per frame
```

---

### Frame Time
**Definition**: How long it takes to render one frame (milliseconds). Inverse of FPS.

**Example usage**: "Frame time spiked to 25ms during explosion - exceeds VR budget."

**Related terms**: FPS, Performance, Frame Budget

**Measurement**:
```cpp
stat unit  // Shows frame time breakdown

Frame: 11.2ms  // Total frame time
Game: 4.5ms    // CPU game logic
Draw: 3.2ms    // CPU render submission
GPU: 3.5ms     // GPU rendering
```

---

### Game Thread
**Definition**: Primary CPU thread that runs game logic (tick functions, Blueprint, physics).

**Example usage**: "Game thread at 8ms - too high for VR. Reduce tick frequency."

**Related terms**: CPU, Thread, Game Logic

**Optimization**:
- Disable unnecessary ticking
- Move work to async threads
- Optimize Blueprint graphs
- Reduce physics complexity
- Cache expensive calculations

---

### GPU-Bound
**Definition**: Performance limited by GPU, not CPU. Reducing geometry/shaders helps; optimizing logic doesn't.

**Example usage**: "Game is GPU-bound - reduce particle effects and post-processing."

**Related terms**: Graphics Bottleneck, Rendering

**Solutions**:
- Reduce particle counts
- Lower resolution/screen percentage
- Simplify materials/shaders
- Reduce post-processing
- Use LODs aggressively
- Reduce shadow quality

---

### Instancing
**Definition**: Technique to render many copies of the same mesh in a single draw call.

**Example usage**: "Use instanced static mesh component for asteroid field - renders 1000 asteroids in one draw call."

**Related terms**: Draw Call Batching, Mesh Merging

**Technical details**:
```cpp
UInstancedStaticMeshComponent* ISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Asteroids"));

// Add 1000 instances
for (int32 i = 0; i < 1000; ++i) {
    FTransform Transform(...);
    ISM->AddInstance(Transform);
}

// Renders in 1 draw call instead of 1000
```

---

### Memory Leak
**Definition**: Bug where memory is allocated but never freed, causing memory usage to grow over time.

**Example usage**: "VFX spawning has memory leak - particles aren't being destroyed."

**Related terms**: Garbage Collection, Memory Management

**Detection**:
```cpp
stat memory  // Monitor memory usage over time

// Should be stable during gameplay
// Growing memory = potential leak
```

---

### Mipmap
**Definition**: Pre-calculated lower-resolution versions of a texture, used when texture appears small on screen.

**Example usage**: "Mipmaps reduce texture sampling cost for distant objects."

**Related terms**: LOD, Texture, Streaming

**Technical details**:
```
Mip 0: 2048×2048 (original)
Mip 1: 1024×1024 (50% size)
Mip 2: 512×512
Mip 3: 256×256
...
Mip N: 1×1
```

GPU automatically selects appropriate mip level based on screen size.

---

### Object Pooling
**Definition**: Reusing pre-allocated objects instead of creating/destroying them repeatedly.

**Example usage**: "Pool VFX components to avoid allocation overhead when spawning particles."

**Related terms**: Memory Management, Performance, Garbage Collection

**Technical details**:
```cpp
// WRONG: Create and destroy
for (int i = 0; i < 100; ++i) {
    UNiagaraComponent* VFX = SpawnVFX();  // Allocation
    // ... use VFX ...
    VFX->Destroy();  // Deallocation
}

// RIGHT: Pool and reuse
TArray<UNiagaraComponent*> Pool;
for (int i = 0; i < 100; ++i) {
    UNiagaraComponent* VFX = AcquireFromPool();  // Reuse
    // ... use VFX ...
    ReturnToPool(VFX);  // Return for reuse
}
```

---

### Profiling
**Definition**: Measuring performance to identify bottlenecks and optimization opportunities.

**Example usage**: "Profile the mission system to find why it's consuming 5ms per frame."

**Related terms**: Benchmarking, Performance Analysis, Optimization

**Tools**:
- Unreal Insights: Advanced profiling
- stat commands: Quick profiling
- Visual Studio Profiler: C++ code profiling
- Unreal Frontend: Session analysis

---

### Render Thread
**Definition**: CPU thread that prepares rendering commands for GPU.

**Example usage**: "Render thread at 6ms - too many draw calls. Use instancing."

**Related terms**: Draw Thread, CPU, Rendering

**Optimization**:
- Reduce draw calls
- Use instancing
- Merge static meshes
- Reduce material complexity
- Use LODs

---

### Shader
**Definition**: GPU program that defines how pixels/vertices are rendered.

**Example usage**: "Complex shader with many texture samples is causing GPU bottleneck."

**Related terms**: Material, HLSL, Rendering

**Types**:
- Vertex shader: Transforms vertices
- Pixel shader: Determines pixel color
- Compute shader: General-purpose GPU computation

---

### Texture Streaming
**Definition**: Dynamically loading/unloading texture mipmaps based on what's visible and at what distance.

**Example usage**: "Texture streaming reduces VRAM usage by only loading high-res textures when needed."

**Related terms**: Mipmap, LOD, Memory Management

**Technical details**: Engine loads low-res mipmaps first, streams in higher resolution as object gets closer. Reduces memory and load times.

---

## Git Terminology

### Branch
**Definition**: Independent line of development in Git, allowing parallel work on features.

**Example usage**: "Create a feature/station-layouts branch for station work."

**Related terms**: Master, Develop, Feature Branch, Merge

**Common branches**:
- master/main: Stable release code
- develop: Integration branch
- feature/*: Individual features
- hotfix/*: Emergency fixes

---

### Commit
**Definition**: Snapshot of project files at a specific point in time, with a descriptive message.

**Example usage**: "Commit your changes with message 'Add Station Alpha docking ports'."

**Related terms**: Hash, SHA, Change Set

**Good commit**:
```bash
git commit -m "[Asset] Add Station Alpha CSG model - 5 docking ports, interior zones"
```

**Bad commit**:
```bash
git commit -m "stuff"
```

---

### Diff
**Definition**: Shows differences between two versions of files.

**Example usage**: "Check the diff to see what changed since last commit."

**Related terms**: Patch, Change, Delta

**Usage**:
```bash
git diff  # Unstaged changes
git diff --staged  # Staged changes
git diff HEAD~1  # Changes from previous commit
```

---

### Fork
**Definition**: Personal copy of someone else's repository, allowing independent modifications.

**Example usage**: "Fork the Alexander repository to work on custom features."

**Related terms**: Clone, Pull Request, Upstream

**Workflow**:
1. Fork repository on GitHub
2. Clone your fork locally
3. Make changes
4. Push to your fork
5. Create Pull Request to original repository

---

### Git LFS (Large File Storage)
**Definition**: Git extension for versioning large files (like game assets) efficiently.

**Example usage**: "Use Git LFS for .uasset and .umap files to avoid repository bloat."

**Related terms**: Binary Files, Large Files, Storage

**Setup**:
```bash
git lfs install
git lfs track "*.uasset"
git lfs track "*.umap"
```

Without LFS: Full file stored in every commit (bloat)
With LFS: Only pointer stored in git, file stored separately

---

### Merge
**Definition**: Combining changes from one branch into another.

**Example usage**: "Merge feature/stations into develop when station work is complete."

**Related terms**: Merge Conflict, Rebase, Pull Request

**Process**:
```bash
git checkout develop
git merge feature/stations

# If conflicts, resolve them:
# Edit conflicting files
git add .
git commit
```

---

### Merge Conflict
**Definition**: Occurs when Git can't automatically merge changes because same lines were modified differently.

**Example usage**: "Resolve the merge conflict in StationAlpha.umap by choosing the correct version."

**Related terms**: Merge, Conflict Resolution

**Resolution**:
```bash
# Git marks conflicts in files:
<<<<<<< HEAD
Current code
=======
Incoming code
>>>>>>> feature/stations

# Choose one, or combine manually
# Then:
git add resolved_file
git commit
```

For binary files (.uasset, .umap): Choose one version entirely.

---

### Pull
**Definition**: Fetch changes from remote repository and merge into current branch.

**Example usage**: "Pull latest changes from origin/develop before starting work."

**Related terms**: Fetch, Merge, Remote

**Usage**:
```bash
git pull origin develop

# Equivalent to:
git fetch origin develop
git merge origin/develop
```

---

### Pull Request (PR)
**Definition**: Proposal to merge changes from one branch into another, typically with code review.

**Example usage**: "Create a Pull Request to merge feature/missions into develop."

**Related terms**: Merge Request, Code Review, Branch

**Workflow**:
1. Push branch to remote
2. Create PR on GitHub/GitLab
3. Team reviews code
4. Address feedback
5. PR approved and merged

---

### Push
**Definition**: Upload local commits to remote repository.

**Example usage**: "Push your commits to origin/feature/stations."

**Related terms**: Remote, Commit, Branch

**Usage**:
```bash
git push origin feature/stations

# First push of new branch:
git push -u origin feature/stations  # -u sets upstream
```

---

### Rebase
**Definition**: Move or combine commits by changing their base, rewriting history.

**Example usage**: "Rebase feature branch onto latest develop to incorporate recent changes."

**Related terms**: Merge, Commit, History

**Usage**:
```bash
git checkout feature/stations
git rebase develop

# Replays your commits on top of develop
```

**Warning**: Don't rebase commits already pushed to shared branches!

---

### Remote
**Definition**: Version of repository hosted on a server (GitHub, GitLab, etc.).

**Example usage**: "Add the team repository as a remote named 'origin'."

**Related terms**: Origin, Upstream, Repository

**Usage**:
```bash
git remote add origin https://github.com/team/alexander.git
git remote -v  # List remotes
git push origin master
git pull origin develop
```

---

### Repository (Repo)
**Definition**: Collection of files and their complete version history managed by Git.

**Example usage**: "Clone the Alexander repository to start working on the project."

**Related terms**: Project, Codebase, Remote

**Types**:
- Local repository: On your computer
- Remote repository: On GitHub/GitLab/etc.
- Bare repository: Server-side repo without working directory

---

### Staging Area (Index)
**Definition**: Intermediate area where changes are prepared before committing.

**Example usage**: "Add modified files to staging area before committing."

**Related terms**: Stage, Index, Commit

**Usage**:
```bash
git status  # See what's modified
git add file.cpp  # Stage specific file
git add .  # Stage all changes
git reset file.cpp  # Unstage specific file
git commit  # Commit staged changes
```

---

### Tag
**Definition**: Named reference to specific commit, typically used for releases.

**Example usage**: "Tag v1.0-baseline for the initial autonomous baseline generation."

**Related terms**: Release, Version, Commit

**Usage**:
```bash
git tag v1.0-baseline
git tag -a v1.0 -m "First release"  # Annotated tag with message
git push origin v1.0  # Push tag to remote
```

---

## General Game Development

### Archetype
**Definition**: Template actor or object used to create instances with predefined properties.

**Example usage**: "Create enemy ship archetype with default health, speed, and weapon loadout."

**Related terms**: Prefab, Template, Blueprint Class

**Technical details**: Changes to archetype propagate to all instances, unless instance has override.

---

### Asset
**Definition**: Any reusable content file - mesh, texture, audio, material, Blueprint, etc.

**Example usage**: "Import station mesh asset and apply metal material asset."

**Related terms**: Content, Resource, File

**Asset types**:
- Static Mesh (.uasset)
- Texture (.uasset)
- Material (.uasset)
- Blueprint (.uasset)
- Sound (.uasset)
- Map (.umap)

---

### Collision
**Definition**: System for detecting when objects physically intersect or touch.

**Example usage**: "Add collision to docking port so ships trigger docking sequence."

**Related terms**: Physics, Overlap, Hit

**Types**:
- Block: Stops movement
- Overlap: Triggers event but allows pass-through
- Ignore: No collision

**Channels**:
- Pawn: Player/NPC characters
- PhysicsBody: Physics-simulated objects
- Vehicle: Ships, cars, etc.
- Custom channels: Project-specific

---

### Game Mode
**Definition**: Defines rules and flow of the game - how to win, spawn points, etc.

**Example usage**: "FirstMissionGameMode sets up initial mission and player spaceship."

**Related terms**: Game State, Player Controller, Pawn

**Technical details**:
```cpp
UCLASS()
class AFirstMissionGameMode : public AGameModeBase {
    GENERATED_BODY()

    virtual void BeginPlay() override;
    virtual APawn* SpawnDefaultPawnFor(AController* NewPlayer, ...) override;
};
```

---

### Hitbox
**Definition**: Simplified collision shape that approximates complex geometry for performance.

**Example usage**: "Use box hitbox for station docking ports instead of precise mesh collision."

**Related terms**: Collision, Bounding Box, Trigger Volume

**Common shapes**:
- Box: Simple rectangular bounds
- Sphere: Single radius, cheapest collision
- Capsule: Character collision
- Convex hull: Wraps mesh, more expensive

---

### Placeholder
**Definition**: Temporary asset used during development, to be replaced later with final art.

**Example usage**: "CSG primitive stations are placeholders for detailed 3D models."

**Related terms**: Prototype, Temporary, Greybox

**Alexander approach**: Entire baseline uses placeholders intentionally for rapid prototyping.

---

### Prototype
**Definition**: Early, simplified version of game or feature to test concepts before full development.

**Example usage**: "Baseline is a prototype to validate gameplay before art production."

**Related terms**: Proof of Concept, Playable Mockup, Greybox

---

### Serialization
**Definition**: Converting object data into storable format (saving) and back (loading).

**Example usage**: "Mission progress is serialized to JSON for save game functionality."

**Related terms**: Save/Load, Persistence, JSON, Binary

**Unreal serialization**:
```cpp
UPROPERTY(SaveGame)
int32 PlayerCredits;  // Automatically serialized in save game

// Custom serialization:
virtual void Serialize(FArchive& Ar) override;
```

---

### Singleton
**Definition**: Design pattern ensuring only one instance of a class exists globally.

**Example usage**: "VFXPoolManager is a singleton - only one pool manager per game."

**Related terms**: Global, Manager, Subsystem

**Unreal approach**: Use Subsystems instead of manual singletons.

---

### Spawn
**Definition**: Create and place an actor or object in the game world.

**Example usage**: "Spawn explosion VFX at projectile impact location."

**Related terms**: Instantiate, Create, Destroy

**Usage**:
```cpp
AActor* NewActor = GetWorld()->SpawnActor<AActor>(
    ActorClass,
    Location,
    Rotation
);
```

---

## Alexander-Specific Terms

### Autonomous Baseline
**Definition**: Automated system that generates a fully functional game prototype using primitives and procedural generation.

**Example usage**: "Run the autonomous baseline script to generate stations, missions, VFX, and UI in 30 minutes."

**Related terms**: Prototype, Baseline Generation, CSG Primitives

**Components**:
- Station generation (CSG primitives)
- Mission templates (procedural data)
- VFX systems (Niagara pooling)
- UI/HUD (UMG with VR support)

---

### Baseline Performance
**Definition**: Performance metrics established with primitive assets, serving as target for final assets.

**Example usage**: "Baseline performance is 90 FPS in VR - final assets must maintain this."

**Related terms**: Performance Budget, Target FPS, Optimization

**Purpose**: Ensures final game performs acceptably by establishing headroom with simple assets.

---

### Docking Sequence
**Definition**: Multi-stage process of aligning and connecting spaceship to station docking port.

**Example usage**: "Docking sequence: approach → align → request clearance → final approach → lock."

**Related terms**: Docking Port, Station, Alignment

**States**:
1. Idle: Not docking
2. Approaching: Flying toward port
3. Aligning: Rotating to match port
4. Requesting clearance: Asking permission
5. Final approach: Precision movement
6. Locked: Docked successfully

---

### Mission Board
**Definition**: In-game interface displaying available missions for player selection.

**Example usage**: "Player opens mission board at station to see delivery and combat missions."

**Related terms**: Mission System, Quest Board, UI

**Implementation**: UMissionBoardComponent + UMG widget.

---

### Mission Template
**Definition**: Data asset defining mission parameters - objectives, rewards, requirements, etc.

**Example usage**: "Create cargo delivery mission template with transport objectives and credit rewards."

**Related terms**: Data Asset, Mission System, Quest

**Structure**:
```cpp
UCLASS()
class UMissionTemplateDataAsset : public UDataAsset {
    FText MissionName;
    TArray<FMissionObjective> Objectives;
    FMissionRewards Rewards;
    int32 RequiredPlayerLevel;
};
```

---

### Phase 9
**Definition**: Current development phase focused on performance optimization and autonomous baseline refinement.

**Example usage**: "Phase 9 introduces PerformanceProfilerSubsystem and automated profiling."

**Related terms**: Development Phase, Milestone

**Phase 9 goals**:
- Performance profiling integration
- Baseline optimization
- VR frame rate targets
- Documentation completion

---

### Profiling Subsystem
**Definition**: Custom UGameInstanceSubsystem that tracks performance of individual game systems.

**Example usage**: "PerformanceProfilerSubsystem tracks mission system frame time at 0.8ms average."

**Related terms**: Performance, Profiling, Subsystem

**Usage**:
```cpp
SCOPED_SYSTEM_PROFILER("MissionBoard");
// Automatically tracks time spent in this scope
```

---

### Station Zone
**Definition**: Defined area within station with specific purpose - docking, commercial, industrial, etc.

**Example usage**: "Station Alpha has 3 interior zones: docking bay, commercial area, command center."

**Related terms**: Station Layout, Level Design, Interior

---

### VFX Pooling
**Definition**: Alexander's system for reusing Niagara VFX components instead of spawning/destroying.

**Example usage**: "VFX pooling reduces allocation overhead from 2ms to 0.2ms per explosion."

**Related terms**: Object Pooling, Performance, Niagara

**Implementation**: UVFXPoolManager with pre-allocated component pools.

---

## Index by First Letter

**A**: Actor, Archetype, Asset, Autonomous Baseline

**B**: Baseline Performance, Blueprint, Bottleneck, Branch, BSP

**C**: Collision, Commit, Component, CPU-Bound, CSG, Culling

**D**: Data Asset, Data Table, Delta Time, Diff, Docking Sequence, Draw Call, Dynamic Resolution

**F**: Foveated Rendering, FPS, Frame Budget, Frame Time, Fork

**G**: Game Mode, Game Thread, GENERATED_BODY, Git LFS, GPU-Bound

**H**: Hitbox, HMD, HUD

**I**: Instancing, IPD

**J**: Judder

**L**: Level, LOD

**M**: Material, Memory Leak, Merge, Merge Conflict, Mipmap, Mission Board, Mission Template, Motion Sickness

**N**: Niagara

**O**: Object Pooling

**P**: Pawn, Phase 9, Placeholder, Positional Tracking, Profiling, Profiling Subsystem, Prototype, Pull, Pull Request, Push

**R**: Rebase, Refresh Rate, Remote, Render Thread, Reprojection, Repository, Room-Scale VR

**S**: Serialization, Shader, Singleton, Spawn, Staging Area, Static Mesh, Station Zone, Stereo Rendering, Subsystem

**T**: Tag, Texture Streaming, Tick

**U**: UCLASS, UMG, UPROPERTY, USTRUCT

**V**: VFX Pooling

**W**: Widget Component, World

**6**: 6DOF

---

## Summary

This glossary provides definitions, examples, and technical details for all major terms used in Alexander project documentation. For deeper conceptual understanding, see KNOWLEDGE_BASE.md. For quick answers to specific questions, see BASELINE_FAQ.md.

**Using this glossary**:
1. Find term alphabetically in relevant section
2. Read definition and example usage
3. Check related terms for connected concepts
4. Review technical details for implementation specifics

**Contributing**:
If you encounter undefined terms in documentation:
1. Note the term and context
2. Research definition
3. Add to appropriate section
4. Include example and related terms

Keep glossary updated as project evolves!
