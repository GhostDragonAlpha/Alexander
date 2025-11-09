# Integration Quick Reference Card

> **Copy-paste snippets for common integration tasks**

---

## Mission System

### Get Available Missions
```cpp
UMissionBoardComponent* Board = GetWorld()->GetSubsystem<UMissionBoardComponent>();
TArray<FMissionData> Missions = Board->GetAvailableMissions(TEXT("MainStationBoard"));
```

### Accept Mission
```cpp
bool bSuccess = Board->AcceptMission(PlayerState, MissionID);
```

### Update Objective
```cpp
Board->UpdateMissionProgress(PlayerState, MissionID, ObjectiveID, 1);
```

### Complete Mission
```cpp
if (Board->IsMissionCompleted(MissionID))
{
    Board->CompleteMission(PlayerState, MissionID);
}
```

### Generate Random Missions
```cpp
Board->GenerateRandomMissions(TEXT("MainStationBoard"), 10);
```

---

## VFX System

### Spawn VFX
```cpp
UVFXManager* VFX = FindComponentByClass<UVFXManager>();
FString ID = VFX->SpawnVFX(System, Location, Rotation, EVFXCategory::Weapon, 5.0f);
```

### Spawn Attached VFX
```cpp
FString ID = VFX->SpawnVFXAttached(System, ShipMesh, TEXT("EngineSocket"), EVFXCategory::Ship, -1.0f);
```

### Control VFX Parameters
```cpp
VFX->SetVFXParameter(ID, TEXT("Intensity"), 0.75f);
VFX->SetVFXColorParameter(ID, TEXT("Color"), FLinearColor::Red);
VFX->SetVFXVectorParameter(ID, TEXT("Direction"), FVector::UpVector);
```

### Spawn Explosion
```cpp
FString ID = VFX->SpawnExplosionVFX(Location, 2.0f);
```

### Preload VFX Pool
```cpp
VFX->PreloadVFXPool(TEXT("MuzzleFlash"), 20);
```

---

## Station System

### Spawn Station
```cpp
FStationConfiguration Config;
Config.StationID = TEXT("Station_Alpha");
Config.StationName = FText::FromString(TEXT("Alpha Station"));
Config.Modules.Add(EStationModuleType::TradingPost);

ASpaceStationHub* Station = USpaceStationBlueprintLibrary::SpawnStationFromConfig(
    GetWorld(), Config, SpawnTransform
);
```

### Access Trading
```cpp
Station->RefreshTradingPost();
TArray<FMarketItem> Items = Station->GetAvailableItems();
```

### Access Missions
```cpp
Station->RefreshMissionBoard();
TArray<FMissionData> Missions = Station->GetAvailableMissions(PlayerState);
```

### Register Player
```cpp
Station->OnPlayerEnterHub(PlayerState);
```

---

## Performance Profiling

### Add Profiling
```cpp
UPerformanceProfilerSubsystem* Profiler = GetWorld()->GetSubsystem<UPerformanceProfilerSubsystem>();
PROFILE_SCOPE(Profiler, TEXT("MySystem_Update"));
// ... code to profile ...
```

### Manual Profiling
```cpp
Profiler->StartProfiling(TEXT("MyOperation"));
// ... do work ...
Profiler->StopProfiling(TEXT("MyOperation"));
```

### Query Performance
```cpp
FProfileData Data = Profiler->GetSystemProfile(TEXT("MySystem"));
UE_LOG(LogTemp, Log, TEXT("Avg: %.2f ms, Max: %.2f ms"), Data.AverageTimeMs, Data.MaxTimeMs);
```

### Get Bottlenecks
```cpp
TArray<FProfileData> Bottlenecks = Profiler->GetTopBottlenecks(10);
Profiler->LogTopBottlenecks(10);
```

### Export Data
```cpp
Profiler->ExportToJSON(TEXT("C:/Temp/profile.json"));
```

---

## UI Integration

### Bind Ship Data
```cpp
// Cache reference in NativeConstruct
CachedShip = GetOwningPlayer()->GetPawn<ASpaceship>();

// Update in NativeTick
if (CachedShip)
{
    SpeedText->SetText(FText::AsNumber(CachedShip->GetVelocity().Size()));
    HealthBar->SetPercent(CachedShip->HullIntegrity / 100.0f);
}
```

### Show/Hide Widget
```cpp
// Show
Widget = CreateWidget<UUserWidget>(this, WidgetClass);
Widget->AddToViewport();

// Hide
Widget->SetVisibility(ESlateVisibility::Hidden);

// Remove
Widget->RemoveFromParent();
```

### Button Click Handler
```cpp
// In NativeConstruct
Button->OnClicked.AddDynamic(this, &UMyWidget::OnButtonClicked);

// Handler
void UMyWidget::OnButtonClicked()
{
    // Handle click
}
```

---

## Weather/Environment VFX

### Spawn Weather
```cpp
UParticleSystemComponent* Weather = UWeatherVFXLibrary::SpawnWeatherEffect(
    GetWorld(),
    EWeatherTypeNew::IonStorm,
    Location,
    0.7f // Intensity
);
```

### Transition Weather
```cpp
UWeatherVFXLibrary::TransitionWeatherEffect(
    GetWorld(),
    CurrentWeather,
    EWeatherTypeNew::NebulaCloud,
    5.0f // Transition time
);
```

---

## Common Patterns

### Cache Component References
```cpp
// In BeginPlay
void BeginPlay()
{
    Super::BeginPlay();
    CachedMissionBoard = GetWorld()->GetSubsystem<UMissionBoardComponent>();
    CachedVFXManager = FindComponentByClass<UVFXManager>();
}
```

### Null Check Pattern
```cpp
if (!Component)
{
    UE_LOG(LogTemp, Error, TEXT("Component not available"));
    return;
}
```

### Timer Pattern
```cpp
GetWorld()->GetTimerManager().SetTimer(
    TimerHandle,
    this,
    &AMyClass::UpdateFunction,
    1.0f,   // Interval
    true    // Loop
);
```

### Event Binding Pattern
```cpp
MissionBoard->OnMissionAccepted.AddDynamic(this, &UMyWidget::OnMissionAccepted);

void UMyWidget::OnMissionAccepted(APlayerState* Player, const FMissionData& Mission, const FName& Board)
{
    RefreshUI();
}
```

---

## Debugging Snippets

### Log Mission Info
```cpp
FMissionData* Mission = Board->GetMission(MissionID);
UE_LOG(LogTemp, Log, TEXT("Mission: %s"), *Mission->Title);
UE_LOG(LogTemp, Log, TEXT("  Type: %s"), *Board->GetMissionTypeDisplayName(Mission->MissionType));
UE_LOG(LogTemp, Log, TEXT("  Difficulty: %s"), *Board->GetMissionDifficultyDisplayName(Mission->Difficulty));
UE_LOG(LogTemp, Log, TEXT("  Objectives: %d"), Mission->Objectives.Num());
```

### Log VFX Stats
```cpp
FVFXStats Stats = VFX->GetVFXStatistics();
UE_LOG(LogTemp, Log, TEXT("VFX: Active=%d, Pooled=%d, Memory=%.2f MB"),
    Stats.ActiveInstances, Stats.PooledInstances, Stats.MemoryUsageMB);
```

### Log Performance Stats
```cpp
UE_LOG(LogTemp, Log, TEXT("FPS: %.1f, Frame Time: %.2f ms"),
    Profiler->GetCurrentFPS(),
    Profiler->GetAverageFrameTimeMs()
);
```

---

## Blueprint Node Equivalents

### Get Mission Board
**Node:** Get Subsystem → Cast to Mission Board Component

### Accept Mission
**Node:** Accept Mission (Target: Mission Board, Player State, Mission ID)

### Spawn VFX
**Node:** Spawn VFX (Target: VFX Manager, System, Location, Rotation, Category, Lifetime)

### Update Progress
**Node:** Update Mission Progress (Target: Mission Board, Player, Mission ID, Objective ID, Progress)

### Profile Code
**Node:** Start Profiling → [Your nodes] → Stop Profiling

---

## File Locations

| System | Header File |
|--------|-------------|
| Mission Board | Source/Alexander/Public/MissionBoardComponent.h |
| VFX Manager | Source/Alexander/Public/VFXManager.h |
| Weather VFX | Source/Alexander/Public/WeatherVFXLibrary.h |
| Station Library | Source/Alexander/Public/SpaceStationBlueprintLibrary.h |
| Station Hub | Source/Alexander/Public/SpaceStationHub.h |
| Profiler | Source/Alexander/Public/PerformanceProfilerSubsystem.h |
| Spaceship | Source/Alexander/Public/Spaceship.h |

---

## Content Browser Paths

| Asset Type | Path |
|------------|------|
| Mission Templates | /Game/Data/Missions/ |
| Station Blueprints | /Game/Blueprints/Stations/ |
| VFX Systems | /Game/VFX/ |
| UI Widgets | /Game/UI/ |
| Weather Data | /Game/Data/Weather/ |

---

## Performance Targets

| Operation | Target Time |
|-----------|-------------|
| Mission Accept | < 1 ms |
| VFX Spawn (pooled) | < 0.1 ms |
| VFX Spawn (new) | < 0.5 ms |
| UI Update | < 2 ms |
| Profiling Overhead | < 0.05 ms |
| Mission Progress Update | < 0.5 ms |

---

## Common Mistakes

### DON'T
```cpp
// Repeated lookup every frame
void Tick(float DeltaTime)
{
    GetWorld()->GetSubsystem<UMissionBoardComponent>()->Update();
}

// No null check
MyComponent->DoSomething(); // CRASH if null!

// Hardcoded values
Mission.Reward.Credits = 5000;
```

### DO
```cpp
// Cache in BeginPlay
void BeginPlay()
{
    CachedBoard = GetWorld()->GetSubsystem<UMissionBoardComponent>();
}

void Tick(float DeltaTime)
{
    if (CachedBoard)
    {
        CachedBoard->Update();
    }
}

// Load from data
UDataAsset* Asset = LoadObject<UDataAsset>(...);
Mission.Reward = Asset->GetReward();
```

---

## Need More Info?

See complete examples in:
- **INTEGRATION_CODE_EXAMPLES.md** - Full C++ and Blueprint examples
- **INTEGRATION_PATTERNS_SUMMARY.md** - Design patterns and best practices
