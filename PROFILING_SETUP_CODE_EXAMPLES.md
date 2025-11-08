# Code Examples: Programmatic Profiling Test Setup

This document provides ready-to-use code examples for setting up the minimal profiling test without manual editor work.

---

## 1. Blueprint Level Script Setup

If you prefer to use Blueprints, create a Level Blueprint with this logic:

### In FlightTest Level Blueprint:
1. **Event BeginPlay**
2. **Construct Spaceship:**
   - Add `Construct Object from Class`
   - Set Class to: `ASpaceship` (or `BP_Spaceship` if you created one)
   - Store result in a variable
3. **Spawn Spaceship:**
   - Add `Spawn Actor from Class`
   - Object: result from previous construct
   - Set Transform > Location: (0, 0, 500)
   - Set Transform > Rotation: (0, 0, 0)
4. **Construct Static Cube:**
   - Add `Construct Object from Class`
   - Set Class to: `AActor` (to be replaced with mesh)
   - Add Static Mesh Component in its construction script
5. **Spawn Profiling Test Actor:**
   - `Spawn Actor from Class`
   - Class: `AProfilingTestActor`
   - Location: (0, 0, 1000)

---

## 2. C++ GameMode Implementation

Create a custom GameMode that spawns test actors automatically:

### Header File: `TestProfilingGameMode.h`
```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TestProfilingGameMode.generated.h"

class ASpaceship;
class AProfilingTestActor;

/**
 * Minimal Game Mode for profiling validation
 * Automatically spawns a spaceship and profiling test actor
 */
UCLASS()
class ALEXANDER_API ATestProfilingGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ATestProfilingGameMode();

    virtual void BeginPlay() override;

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling Test")
    TSubclassOf<ASpaceship> SpaceshipClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling Test")
    TSubclassOf<AProfilingTestActor> ProfilingActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling Test")
    FVector SpaceshipSpawnLocation = FVector(0, 0, 500);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling Test")
    FVector EnvironmentActorLocation = FVector(5000, 0, 100);

    // Spawned actors
    UPROPERTY()
    ASpaceship* TestSpaceship = nullptr;

    UPROPERTY()
    AProfilingTestActor* ProfilingTestActor = nullptr;

private:
    void SpawnTestActors();
    void CreateEnvironment();
};
```

### Implementation File: `TestProfilingGameMode.cpp`
```cpp
#include "TestProfilingGameMode.h"
#include "Spaceship.h"
#include "ProfilingTestActor.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"

ATestProfilingGameMode::ATestProfilingGameMode()
{
    // Load default spaceship class if not set
    static ConstructorHelpers::FClassFinder<ASpaceship> DefaultSpaceshipClass(
        TEXT("/Game/Blueprints/Characters/BP_Spaceship")
    );
    if (DefaultSpaceshipClass.Class)
    {
        SpaceshipClass = DefaultSpaceshipClass.Class;
    }

    // Load default profiling actor class
    static ConstructorHelpers::FClassFinder<AProfilingTestActor> DefaultProfilingClass(
        TEXT("/Game/Blueprints/Testing/BP_ProfilingTestActor")
    );
    if (DefaultProfilingClass.Class)
    {
        ProfilingActorClass = DefaultProfilingClass.Class;
    }
}

void ATestProfilingGameMode::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("=== Profiling Test GameMode Starting ==="));

    SpawnTestActors();
    CreateEnvironment();

    UE_LOG(LogTemp, Log, TEXT("=== Profiling Test Setup Complete ==="));
}

void ATestProfilingGameMode::SpawnTestActors()
{
    if (!SpaceshipClass)
    {
        UE_LOG(LogTemp, Error, TEXT("SpaceshipClass not set!"));
        return;
    }

    // Spawn spaceship
    FActorSpawnParameters SpaceshipSpawnParams;
    SpaceshipSpawnParams.Owner = this;
    SpaceshipSpawnParams.Instigator = nullptr;

    TestSpaceship = GetWorld()->SpawnActor<ASpaceship>(
        SpaceshipClass,
        SpaceshipSpawnLocation,
        FRotator::ZeroRotator,
        SpaceshipSpawnParams
    );

    if (TestSpaceship)
    {
        UE_LOG(LogTemp, Log, TEXT("Spaceship spawned at %s"), *SpaceshipSpawnLocation.ToString());

        // Set as player's pawn if default controller exists
        if (GetWorld()->GetFirstPlayerController())
        {
            GetWorld()->GetFirstPlayerController()->Possess(TestSpaceship);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn spaceship!"));
    }

    // Spawn profiling test actor
    if (ProfilingActorClass)
    {
        FActorSpawnParameters ProfilingSpawnParams;
        ProfilingSpawnParams.Owner = this;

        ProfilingTestActor = GetWorld()->SpawnActor<AProfilingTestActor>(
            ProfilingActorClass,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            ProfilingSpawnParams
        );

        if (ProfilingTestActor)
        {
            UE_LOG(LogTemp, Log, TEXT("Profiling test actor spawned"));
        }
    }
}

void ATestProfilingGameMode::CreateEnvironment()
{
    // Create a simple static cube as environment
    FActorSpawnParameters EnvironmentParams;
    EnvironmentParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AActor* EnvironmentActor = GetWorld()->SpawnActor<AActor>(
        AActor::StaticClass(),
        EnvironmentActorLocation,
        FRotator::ZeroRotator,
        EnvironmentParams
    );

    if (EnvironmentActor)
    {
        // Add a static mesh component
        UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(EnvironmentActor);

        // Load a default cube mesh
        static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
            TEXT("StaticMesh'/Engine/BasicShapes/Cube'")
        );

        if (CubeMesh.Object)
        {
            MeshComponent->SetStaticMesh(CubeMesh.Object);
            MeshComponent->SetRelativeScale3D(FVector(100, 100, 10)); // Large, flat environment
            MeshComponent->RegisterComponent();
            EnvironmentActor->SetRootComponent(MeshComponent);
            MeshComponent->SetMobility(EComponentMobility::Static);

            UE_LOG(LogTemp, Log, TEXT("Environment created at %s"), *EnvironmentActorLocation.ToString());
        }
    }
}
```

---

## 3. Python Script for Automated Setup

Create this script in `Content/Python/setup_profiling_test.py`:

```python
import unreal

# Get the current world and level
editor = unreal.get_default_object(unreal.UnrealEditorSubsystem)
world = editor.get_editor_world()

print("=== Profiling Test Setup (Python) ===")

# 1. Load Spaceship class
spaceship_class = unreal.load_object(None, "/Game/Blueprints/Characters/BP_Spaceship")
if not spaceship_class:
    spaceship_class = unreal.load_object(None, "/Script/Alexander.Spaceship")

if spaceship_class:
    print(f"Found spaceship class: {spaceship_class}")

    # Spawn spaceship
    spaceship_location = unreal.Vector(0, 0, 500)
    spaceship = unreal.Actor.static_spawn_actor(
        spaceship_class,
        location=spaceship_location,
        rotation=unreal.Rotator(0, 0, 0)
    )
    print(f"Spaceship spawned at {spaceship_location}")
else:
    print("ERROR: Could not find spaceship class!")

# 2. Load profiling test actor class
profiling_class = unreal.load_object(None, "/Game/Blueprints/Testing/BP_ProfilingTestActor")
if not profiling_class:
    profiling_class = unreal.load_object(None, "/Script/Alexander.ProfilingTestActor")

if profiling_class:
    print(f"Found profiling actor class: {profiling_class}")

    # Spawn profiling actor
    profiling_actor = unreal.Actor.static_spawn_actor(
        profiling_class,
        location=unreal.Vector(0, 0, 1000),
        rotation=unreal.Rotator(0, 0, 0)
    )
    print("Profiling test actor spawned")
else:
    print("WARNING: Could not find profiling test actor class (using AProfilingTestActor)")

# 3. Create basic cube as environment
cube_actor = unreal.Actor()
cube_class = unreal.load_object(None, "/Script/Engine.StaticMeshActor")

if cube_class:
    cube_mesh = unreal.load_object(None, "StaticMesh'/Engine/BasicShapes/Cube'")
    if cube_mesh:
        cube = unreal.Actor.static_spawn_actor(
            cube_class,
            location=unreal.Vector(5000, 0, 100),
            rotation=unreal.Rotator(0, 0, 0)
        )
        # Set mesh and scale via properties
        cube.get_property("static_mesh_component").set_static_mesh(cube_mesh)
        print("Environment cube created")

print("=== Setup Complete ===")
```

**To run this script:**
1. In Unreal Editor, open the Python console (Tools > Python > Python Console)
2. Copy-paste the script above, or
3. Run: `exec(open('Content/Python/setup_profiling_test.py').read())`

---

## 4. Minimal C++ Actor Template

If you want to create a simple custom test actor with profiling:

### Header: `MinimalProfilingTestActor.h`
```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MinimalProfilingTestActor.generated.h"

class UPerformanceProfilerSubsystem;

/**
 * Simple actor to drive profiling test
 */
UCLASS()
class ALEXANDER_API AMinimalProfilingTestActor : public AActor
{
    GENERATED_BODY()

public:
    AMinimalProfilingTestActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profiling")
    float TestDurationSeconds = 60.0f;

private:
    UPROPERTY()
    UPerformanceProfilerSubsystem* Profiler;

    float TestStartTime;
    bool bTestRunning;

    void StartTest();
    void EndTest();
};
```

### Implementation: `MinimalProfilingTestActor.cpp`
```cpp
#include "MinimalProfilingTestActor.h"
#include "PerformanceProfilerSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AMinimalProfilingTestActor::AMinimalProfilingTestActor()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AMinimalProfilingTestActor::BeginPlay()
{
    Super::BeginPlay();

    Profiler = GetWorld()->GetSubsystem<UPerformanceProfilerSubsystem>();
    if (!Profiler)
    {
        UE_LOG(LogTemp, Error, TEXT("Could not get PerformanceProfilerSubsystem"));
        return;
    }

    Profiler->SetProfilingEnabled(true);
    StartTest();
}

void AMinimalProfilingTestActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bTestRunning)
        return;

    float ElapsedTime = GetWorld()->GetTimeSeconds() - TestStartTime;

    if (ElapsedTime >= TestDurationSeconds)
    {
        EndTest();
    }
}

void AMinimalProfilingTestActor::StartTest()
{
    bTestRunning = true;
    TestStartTime = GetWorld()->GetTimeSeconds();
    UE_LOG(LogTemp, Log, TEXT("Profiling test started for %.0f seconds"), TestDurationSeconds);
}

void AMinimalProfilingTestActor::EndTest()
{
    bTestRunning = false;
    UE_LOG(LogTemp, Log, TEXT("Profiling test complete"));

    if (Profiler)
    {
        Profiler->LogTopBottlenecks(10);
        Profiler->ExportToJSON(FPaths::ProjectSavedDir() / TEXT("Profiling") / TEXT("TestResult.json"));
    }
}
```

---

## 5. Quick Checklist for Programmatic Setup

- [ ] Copy GameMode code to `Source/Alexander/Private/TestProfilingGameMode.cpp`
- [ ] Copy header to `Source/Alexander/Public/TestProfilingGameMode.h`
- [ ] Add `TestProfilingGameMode` to your project's `.Build.cs` file
- [ ] Compile the project
- [ ] In Project Settings > Maps & Modes > Game Mode, set default to `TestProfilingGameMode`
- [ ] Open FlightTest level
- [ ] Click Play
- [ ] Check Output Log for profiling messages
- [ ] Check `Saved/Profiling/` for JSON export

---

## 6. Hybrid Approach (Recommended)

1. **Use Blueprint manually** to place the spaceship (faster iteration)
2. **Use C++ GameMode** for automated profiling test actor spawning
3. **Use Python** for batch testing multiple scenarios

This gives you the flexibility of Blueprints with the automation of code.

---

## 7. Troubleshooting Code-Based Setup

### Compilation Errors
- Make sure headers are included correctly
- Check that class names match your project (ALEXANDER_API)
- Run `Compile` and `Refresh` in Visual Studio

### Actors Not Spawning
- Add `UE_LOG` statements to verify code path execution
- Check `ConstructorHelpers::FObjectFinder` is finding correct paths
- Use `UE_LOG(LogTemp, Warning, TEXT("Found: %s"), *ClassPath)` to debug

### Profiler Not Running
- Verify `PerformanceProfilerSubsystem` exists in your project
- Check that `bProfilingEnabled` is true
- Look for errors related to subsystem initialization

---

## Summary

Choose your approach based on your workflow:
- **Manual (Blueprint)** - Best for visual level design
- **GameMode (C++)** - Best for automated tests
- **Python** - Best for batch scripting
- **Hybrid** - Best for flexibility

All approaches will successfully create the minimal profiling test setup.
