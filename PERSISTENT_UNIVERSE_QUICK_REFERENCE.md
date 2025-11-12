# PersistentUniverseManager - Quick Reference Guide

## 🚀 **Bubble of Influence System**

The **Bubble of Influence** is your player's immediate play area where objects appear at full scale and detail. As objects move away from the player, they smoothly scale down while maintaining realistic apparent sizes.

### **Core Concept**
```
Player (Bubble Center)
    │
    ├── Inner Radius (10km) ← Full scale, full detail
    │   │
    │   └── Immediate Play Area
    │
    ├── Scaling Zone (10km → 1000km)
    │   │
    │   └── Objects smoothly scale down
    │
    └── Outer Radius (1000km) ← Minimum scale (10%)
        │
        └── Objects fade out/are culled
```

## 📦 **Component Setup**

### **1. Add to Game Mode**

```cpp
// In YourGameMode.h
UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
UPersistentUniverseManager* PersistenceManager;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
USphericalSpacePhysicsManager* PhysicsManager;

// In YourGameMode.cpp
void AYourGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Create managers
    PersistenceManager = NewObject<UPersistentUniverseManager>(this);
    PersistenceManager->RegisterComponent();
    
    PhysicsManager = NewObject<USphericalSpacePhysicsManager>(this);
    PhysicsManager->RegisterComponent();
    
    // Configure bubble
    FBubbleInfluenceConfig BubbleConfig;
    BubbleConfig.InnerRadius = 10000.0f;   // 100m
    BubbleConfig.OuterRadius = 1000000.0f; // 10km
    BubbleConfig.MinScaleFactor = 0.1f;    // Scale to 10%
    BubbleConfig.ScalingExponent = 2.0f;   // Quadratic falloff
    
    PhysicsManager->SetBubbleConfig(BubbleConfig);
    PhysicsManager->bEnableDistanceScaling = true;
    
    // Setup auto-save
    PersistenceManager->bEnableAutoSave = true;
    PersistenceManager->AutoSaveInterval = 300.0f; // 5 minutes
    PersistenceManager->EnableAutoSave(true);
}
```

### **2. Register Player**

```cpp
void AYourGameMode::OnPlayerSpawned(APlayerController* PlayerController)
{
    if (APawn* PlayerPawn = PlayerController->GetPawn())
    {
        // Set as bubble center
        PhysicsManager->SetPlayerBubbleCenter(PlayerPawn);
        
        // Register for physics tracking
        FCelestialScalingData PlayerData; // Player doesn't need celestial scaling
        PhysicsManager->RegisterActor(PlayerPawn, PlayerData);
    }
}
```

### **3. Register Celestial Bodies**

```cpp
void AYourGameMode::RegisterPlanet(AActor* PlanetActor)
{
    FCelestialScalingData PlanetData;
    PlanetData.TrueRadiusKm = 6371.0f;      // Earth's radius
    PlanetData.ApparentRadius = 1000.0f;    // 10m at reference distance
    PlanetData.ReferenceDistanceKm = 10000.0f; // 10,000km
    
    PhysicsManager->RegisterActor(PlanetActor, PlanetData);
    
    // Optional: Set orbital parameters
    AActor* StarActor = GetStarActor();
    PhysicsManager->SetOrbitalParameters(
        PlanetActor, 
        StarActor, 
        150000.0f,  // 150,000km orbital radius
        8760.0f     // 1 year orbital period
    );
}
```

## 💾 **Save/Load Operations**

### **Save Player Data**

```cpp
void AYourGameMode::SavePlayerProgress()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;
    
    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;
    
    // Prepare player data
    FPlayerPersistentData PlayerData;
    PlayerData.PlayerID = "Player001";
    PlayerData.PlayerName = "Space Explorer";
    PlayerData.Level = CurrentLevel;
    PlayerData.Experience = TotalExperience;
    PlayerData.Credits = PlayerCredits;
    PlayerData.LastLocation = PhysicsManager->GetActorCartesianPosition(PlayerPawn);
    PlayerData.CurrentSystemID = CurrentStarSystem;
    PlayerData.PlayTime = GetGameTime();
    
    // Save with priority
    FSaveOperationResult Result = PersistenceManager->SavePlayerData(PlayerData.PlayerID, PlayerData);
    
    if (Result.bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Player progress saved in %.2f seconds"), Result.SaveDuration);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Save failed: %s"), *Result.ErrorMessage);
    }
}
```

### **Load Player Data**

```cpp
void AYourGameMode::LoadPlayerProgress()
{
    FString PlayerID = "Player001";
    bool bLoadSuccess = false;
    
    FPlayerPersistentData LoadedData = PersistenceManager->LoadPlayerData(PlayerID, bLoadSuccess);
    
    if (bLoadSuccess)
    {
        // Restore player state
        CurrentLevel = LoadedData.Level;
        TotalExperience = LoadedData.Experience;
        PlayerCredits = LoadedData.Credits;
        CurrentStarSystem = LoadedData.CurrentSystemID;
        
        // Restore player position
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC && PC->GetPawn())
        {
            PhysicsManager->SetActorCartesianPosition(PC->GetPawn(), LoadedData.LastLocation);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Player progress loaded: Level %d, %d credits"), 
            LoadedData.Level, LoadedData.Credits);
    }
    else
    {
        // Create new player
        CreateNewPlayer();
    }
}
```

### **Save World State**

```cpp
void AYourGameMode::SaveWorldState()
{
    FWorldStateData WorldData;
    WorldData.WorldID = CurrentStarSystem;
    WorldData.WorldTime = FDateTime::UtcNow();
    
    // Save dynamic actor positions
    for (AActor* DynamicActor : DynamicActors)
    {
        FVector ActorPos = PhysicsManager->GetActorCartesianPosition(DynamicActor);
        WorldData.DynamicActorLocations.Add(DynamicActor->GetName(), ActorPos);
    }
    
    // Save world flags
    WorldData.WorldFlags.Add(TEXT("MainQuestComplete"), bMainQuestComplete);
    WorldData.WorldFlags.Add(TEXT("BossDefeated"), bBossDefeated);
    
    // Save completed events
    for (const FString& EventName : CompletedEvents)
    {
        WorldData.CompletedEvents.Add(EventName);
    }
    
    PersistenceManager->SaveWorldState(WorldData.WorldID, WorldData);
}
```

## 🎮 **Gameplay Integration**

### **Handle Important Events**

```cpp
void AYourGameMode::OnImportantEventOccurred()
{
    // Auto-save on important events
    PersistenceManager->SaveAllData();
    
    // Create backup for major milestones
    if (bMajorMilestone)
    {
        FString BackupName = FString::Printf(TEXT("Milestone_%s"), *FDateTime::Now().ToString());
        PersistenceManager->CreateBackup(BackupName);
    }
}
```

### **Bubble Transition Events**

```cpp
// In YourGameMode.cpp, bind to events
void AYourGameMode::SetupBubbleEvents()
{
    PhysicsManager->OnActorEnteredBubble.AddDynamic(this, &AYourGameMode::HandleActorEnteredBubble);
    PhysicsManager->OnActorLeftBubble.AddDynamic(this, &AYourGameMode::HandleActorLeftBubble);
    PhysicsManager->OnInfluenceLevelChanged.AddDynamic(this, &AYourGameMode::HandleInfluenceChanged);
}

void AYourGameMode::HandleActorEnteredBubble(AActor* Actor, float Distance)
{
    UE_LOG(LogTemp, Log, TEXT("%s entered bubble at distance %.1f"), *Actor->GetName(), Distance);
    
    // Could trigger AI activation, detail level increase, etc.
    if (AEnemyShip* Enemy = Cast<AEnemyShip>(Actor))
    {
        Enemy->ActivateAI();
    }
}

void AYourGameMode::HandleActorLeftBubble(AActor* Actor)
{
    UE_LOG(LogTemp, Log, TEXT("%s left bubble"), *Actor->GetName());
    
    // Could trigger AI deactivation, LOD reduction, etc.
    if (AEnemyShip* Enemy = Cast<AEnemyShip>(Actor))
    {
        Enemy->DeactivateAI();
    }
}

void AYourGameMode::HandleInfluenceChanged(AActor* Actor, float NewInfluenceLevel)
{
    // Adjust detail levels based on influence
    if (NewInfluenceLevel > 0.5f)
    {
        // High detail when close
        SetActorDetailLevel(Actor, EDetailLevel::High);
    }
    else if (NewInfluenceLevel > 0.1f)
    {
        // Medium detail at medium distance
        SetActorDetailLevel(Actor, EDetailLevel::Medium);
    }
    else
    {
        // Low detail when far
        SetActorDetailLevel(Actor, EDetailLevel::Low);
    }
}
```

## 📊 **Statistics and Debugging**

### **Monitor Save/Load Performance**

```cpp
void AYourGameMode::PrintPersistenceStats()
{
    FPersistenceStatistics Stats = PersistenceManager->GetStatistics();
    
    UE_LOG(LogTemp, Log, TEXT("=== Persistence Statistics ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Saves: %d"), Stats.TotalSaves);
    UE_LOG(LogTemp, Log, TEXT("Total Loads: %d"), Stats.TotalLoads);
    UE_LOG(LogTemp, Log, TEXT("Failed Operations: %d"), Stats.FailedOperations);
    UE_LOG(LogTemp, Log, TEXT("Average Save Time: %.2f seconds"), Stats.AverageSaveTime);
    UE_LOG(LogTemp, Log, TEXT("Average Load Time: %.2f seconds"), Stats.AverageLoadTime);
    UE_LOG(LogTemp, Log, TEXT("Total Data Written: %d bytes"), Stats.TotalBytesWritten);
    UE_LOG(LogTemp, Log, TEXT("Total Data Read: %d bytes"), Stats.TotalBytesRead);
}
```

### **Debug Visualization**

```cpp
void AYourGameMode::DrawBubbleDebug()
{
    if (!PlayerBubbleCenter) return;
    
    FVector PlayerPos = PlayerBubbleCenter->GetActorLocation();
    
    // Draw inner radius (full scale zone)
    DrawDebugSphere(
        GetWorld(),
        PlayerPos,
        PhysicsManager->GetBubbleConfig().InnerRadius,
        32,
        FColor::Green,
        false,
        0.0f,
        0,
        2.0f
    );
    
    // Draw outer radius (bubble boundary)
    DrawDebugSphere(
        GetWorld(),
        PlayerPos,
        PhysicsManager->GetBubbleConfig().OuterRadius,
        32,
        FColor::Yellow,
        false,
        0.0f,
        0,
        2.0f
    );
}
```

## 🔧 **Configuration Reference**

### **Bubble Influence Settings**

| Parameter | Default | Description |
|-----------|---------|-------------|
| `InnerRadius` | 10,000.0 | Full scale zone (Unreal units) |
| `OuterRadius` | 1,000,000.0 | Bubble boundary (Unreal units) |
| `MinScaleFactor` | 0.1 | Minimum scale at boundary (0.0-1.0) |
| `ScalingExponent` | 2.0 | Scaling curve exponent (0.0-5.0) |

### **Celestial Scaling Settings**

| Parameter | Default | Description |
|-----------|---------|-------------|
| `TrueRadiusKm` | 6,371.0 | Real-world radius in kilometers |
| `ApparentRadius` | 1,000.0 | Base apparent size in Unreal units |
| `ReferenceDistanceKm` | 10,000.0 | Reference distance for apparent size |

### **Auto-Save Settings**

| Parameter | Default | Description |
|-----------|---------|-------------|
| `bEnableAutoSave` | true | Enable automatic saving |
| `AutoSaveInterval` | 300.0 | Interval in seconds (5 minutes) |
| `MaxBackups` | 5 | Number of backups to keep |

## 🎯 **Best Practices**

### **Performance**
1. **Register only necessary actors** - Don't register every actor, only those that need scaling
2. **Use appropriate bubble sizes** - Larger bubbles = more calculations
3. **Batch operations** - Save multiple changes at once instead of individual saves
4. **Async saves for events** - Use async operations during gameplay to avoid hitches

### **Gameplay**
1. **Save before major events** - Always save before boss fights, zone transitions, etc.
2. **Create backups for milestones** - Major achievements deserve backup points
3. **Test bubble boundaries** - Ensure gameplay works at all distance scales
4. **Use events for transitions** - Leverage bubble events for AI and detail management

### **Data Management**
1. **Keep save data minimal** - Only save what's necessary
2. **Validate on load** - Always check save data integrity
3. **Version your saves** - Handle save format changes gracefully
4. **Monitor statistics** - Track save/load performance for optimization

## 🐛 **Troubleshooting**

### **Common Issues**

**Objects not scaling:**
- Check if actor is registered with physics manager
- Verify bubble center is set
- Ensure `bEnableDistanceScaling` is true

**Saves not working:**
- Check save directory permissions
- Verify sufficient disk space
- Look for error messages in logs

**Performance issues:**
- Reduce bubble size or actor count
- Disable smooth transitions (`bEnableSmoothTransitions = false`)
- Increase auto-save interval

**Load failures:**
- Check save file integrity
- Verify encryption key (if encryption enabled)
- Look for version mismatches

## 📚 **API Reference**

See full API documentation in:
- [`PersistentUniverseManager.h`](Source/Alexander/Public/PersistentUniverseManager.h)
- [`SphericalSpacePhysicsManager.h`](Source/Alexander/Public/SphericalSpacePhysicsManager.h)
- [`OriginCenteredPhysicsManager.h`](Source/Alexander/Public/OriginCenteredPhysicsManager.h)

## 🎉 **Quick Start Checklist**

- [ ] Add managers to GameMode
- [ ] Configure bubble settings
- [ ] Register player as bubble center
- [ ] Register celestial bodies with scaling data
- [ ] Set up auto-save intervals
- [ ] Bind to important events
- [ ] Test bubble transitions
- [ ] Verify save/load functionality
- [ ] Monitor performance statistics

**System Status**: ✅ **READY FOR PRODUCTION**