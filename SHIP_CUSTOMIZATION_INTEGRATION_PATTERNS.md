# Ship Customization System - Integration Patterns

**Project:** Alexander VR Space Flight Game
**Version:** 1.0
**Last Updated:** November 7, 2025

---

## Table of Contents

1. [Integration Overview](#integration-overview)
2. [How to Integrate with Your System](#how-to-integrate-with-your-system)
3. [Best Practices](#best-practices)
4. [Common Pitfalls and Solutions](#common-pitfalls-and-solutions)
5. [Performance Considerations](#performance-considerations)
6. [Thread Safety Notes](#thread-safety-notes)
7. [Advanced Integration Patterns](#advanced-integration-patterns)

---

## Integration Overview

The Ship Customization System is designed to be **loosely coupled** with other game systems. It communicates primarily through:

- **Direct property modification** (physics values)
- **Event delegates** (loadout changes, unlocks, level-ups)
- **Blueprint-callable functions** (queries and commands)
- **SaveGame system** (persistence)

### System Dependencies

```
Ship Customization Component
│
├── REQUIRES (Hard Dependencies)
│   ├── AOrbitalBody (or derived class like ASpaceship)
│   │   └── For physics integration (Mass, ThrustPower, StandardGravParam)
│   ├── UDataTable (Engine)
│   │   └── For part/skin data storage
│   └── USaveGame (Engine)
│       └── For progression persistence
│
├── OPTIONAL (Soft Dependencies)
│   ├── UFlightController
│   │   └── For flight safety limits (MaxSafeVelocity)
│   ├── UStaticMeshComponent
│   │   └── For visual customization (skin materials, part meshes)
│   └── UShipPhysicsConfig (Data Asset)
│       └── For physics presets
│
└── PROVIDES (What it offers to other systems)
    ├── FShipStats (calculated ship performance)
    ├── Events (OnLoadoutChanged, OnPartEquipped, etc.)
    └── Queries (GetUnlockedParts, GetTotalStats, etc.)
```

---

## How to Integrate with Your System

### 1. Adding to an Existing Ship

**Scenario**: You have an existing ship class (`AMySpaceship`) and want to add customization.

**Steps**:

1. **Add Component in C++ Header**:

```cpp
// MySpaceship.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ShipCustomizationComponent.h"  // INCLUDE THIS
#include "MySpaceship.generated.h"

UCLASS()
class MYGAME_API AMySpaceship : public APawn
{
    GENERATED_BODY()

public:
    AMySpaceship();

    // ADD THIS COMPONENT
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Customization")
    UShipCustomizationComponent* CustomizationComponent;

protected:
    virtual void BeginPlay() override;
};
```

2. **Create Component in C++ Constructor**:

```cpp
// MySpaceship.cpp
#include "MySpaceship.h"

AMySpaceship::AMySpaceship()
{
    // Create customization component
    CustomizationComponent = CreateDefaultSubobject<UShipCustomizationComponent>(
        TEXT("CustomizationComponent")
    );
}

void AMySpaceship::BeginPlay()
{
    Super::BeginPlay();

    // Component will auto-initialize (load save data, apply physics, etc.)
    // No manual initialization needed!
}
```

3. **Assign Data Tables in Blueprint**:

- Open your ship Blueprint
- Select `CustomizationComponent`
- Set `Ship Parts Data Table` → your parts data table
- Set `Ship Skins Data Table` → your skins data table
- (Optional) Set `Physics Config` → a physics preset

**Done!** The component is now functional.

---

### 2. Creating a Customization UI

**Scenario**: You want to create a menu where players can equip parts and skins.

**Pattern**: Query available parts, display in UI, call equip functions on button click.

**Example Blueprint Widget**:

```
Widget: CustomizationMenu
├── Parts List (Vertical Box)
│   ├── For each category:
│   │   ├── Get Unlocked Parts In Category
│   │   ├── Create UI item for each part
│   │   └── On Click → Equip Part
│   └── ...
├── Skins List (Horizontal Box)
│   ├── Get Unlocked Skins
│   ├── Create UI item for each skin
│   └── On Click → Equip Skin
└── Stats Display (Text Block)
    └── Get Total Stats → Display
```

**C++ Implementation**:

```cpp
// CustomizationMenuWidget.h
UCLASS()
class UCustomizationMenuWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

public:
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void RefreshPartsList();

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void HandlePartClicked(FName PartID, EShipPartCategory Category);

private:
    UPROPERTY()
    UShipCustomizationComponent* CustomizationComp;
};

// CustomizationMenuWidget.cpp
void UCustomizationMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Get customization component from player ship
    APlayerController* PC = GetOwningPlayer();
    if (PC && PC->GetPawn())
    {
        CustomizationComp = PC->GetPawn()->FindComponentByClass<UShipCustomizationComponent>();
    }

    RefreshPartsList();
}

void UCustomizationMenuWidget::RefreshPartsList()
{
    if (!CustomizationComp) return;

    // Example: Get all unlocked engines
    TArray<FShipPartData> Engines = CustomizationComp->GetUnlockedPartsInCategory(
        EShipPartCategory::Engine
    );

    // Populate UI (pseudo-code)
    for (const FShipPartData& Engine : Engines)
    {
        // Create UI item widget
        UPartItemWidget* Item = CreateWidget<UPartItemWidget>(this, PartItemWidgetClass);
        Item->SetPartData(Engine);
        Item->OnClicked.AddDynamic(this, &UCustomizationMenuWidget::HandlePartClicked);

        // Add to list
        PartsListContainer->AddChild(Item);
    }
}

void UCustomizationMenuWidget::HandlePartClicked(FName PartID, EShipPartCategory Category)
{
    if (!CustomizationComp) return;

    // Equip the part
    bool Success = CustomizationComp->EquipPart(Category, PartID);

    if (Success)
    {
        // Refresh stats display
        RefreshStatsDisplay();

        // Play sound
        PlayEquipSound();
    }
    else
    {
        // Show error message
        ShowErrorMessage(TEXT("Cannot equip part!"));
    }
}
```

---

### 3. Listening to Customization Events

**Scenario**: You want to react when the player changes their loadout.

**Pattern**: Bind to component events in BeginPlay.

**C++ Example**:

```cpp
// MyGameMode.h
UCLASS()
class AMyGameMode : public AGameModeBase
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void OnPlayerLoadoutChanged();

    UFUNCTION()
    void OnPlayerPartEquipped(EShipPartCategory Category, FName PartID);

    UFUNCTION()
    void OnPlayerLevelUp(int32 OldLevel, int32 NewLevel);
};

// MyGameMode.cpp
void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();

    // Get player ship
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC || !PC->GetPawn()) return;

    UShipCustomizationComponent* CustomComp = PC->GetPawn()->FindComponentByClass<UShipCustomizationComponent>();
    if (!CustomComp) return;

    // Bind to events
    CustomComp->OnLoadoutChanged.AddDynamic(this, &AMyGameMode::OnPlayerLoadoutChanged);
    CustomComp->OnPartEquipped.AddDynamic(this, &AMyGameMode::OnPlayerPartEquipped);
    CustomComp->OnPlayerLevelUp.AddDynamic(this, &AMyGameMode::OnPlayerLevelUp);
}

void AMyGameMode::OnPlayerLoadoutChanged()
{
    UE_LOG(LogTemp, Log, TEXT("Player changed loadout - updating leaderboards"));
    // Update server leaderboards with new ship stats
}

void AMyGameMode::OnPlayerPartEquipped(EShipPartCategory Category, FName PartID)
{
    UE_LOG(LogTemp, Log, TEXT("Player equipped %s in category %d"),
        *PartID.ToString(), static_cast<int>(Category));

    // Award achievement "First Upgrade"
    if (Category == EShipPartCategory::Engine)
    {
        AwardAchievement(TEXT("FirstEngineUpgrade"));
    }
}

void AMyGameMode::OnPlayerLevelUp(int32 OldLevel, int32 NewLevel)
{
    UE_LOG(LogTemp, Log, TEXT("Player leveled up: %d -> %d"), OldLevel, NewLevel);

    // Unlock new parts
    if (NewLevel == 5)
    {
        // Automatically unlock rare engines at level 5
        UShipCustomizationComponent* CustomComp = /* ... get component ... */;
        CustomComp->UnlockPart(FName("Engine_Rare_Tier1"));
    }
}
```

**Blueprint Example**:

```
Event BeginPlay
  → Get Ship Customization Component
  → Bind Event to OnLoadoutChanged
      → RefreshHUD
      → SaveGame
  → Bind Event to OnPartEquipped
      → Play Equip Sound
      → Show "Part Equipped!" notification
```

---

### 4. Integrating with Physics Systems

**Scenario**: You have a custom physics system and want ship stats to affect it.

**Pattern**: Override `ApplyStatsToFlightController()` or listen to `OnLoadoutChanged`.

**Option A: Override in Custom Component Class**:

```cpp
// MyCustomizationComponent.h
UCLASS()
class UMyCustomizationComponent : public UShipCustomizationComponent
{
    GENERATED_BODY()

protected:
    virtual void ApplyStatsToFlightController() override;
};

// MyCustomizationComponent.cpp
void UMyCustomizationComponent::ApplyStatsToFlightController()
{
    // Call base implementation (applies to OrbitalBody and FlightController)
    Super::ApplyStatsToFlightController();

    // CUSTOM: Apply to your physics system
    FShipStats Stats = GetTotalStats();

    AMyCustomShip* Ship = Cast<AMyCustomShip>(GetOwner());
    if (Ship)
    {
        Ship->MyCustomPhysicsComponent->SetThrustMultiplier(Stats.ThrustPower);
        Ship->MyCustomPhysicsComponent->SetMass(Stats.Mass);
        Ship->MyCustomPhysicsComponent->SetMaxSpeed(Stats.MaxVelocity * 1000.0f);

        UE_LOG(LogTemp, Log, TEXT("Applied custom physics stats"));
    }
}
```

**Option B: Listen to Event**:

```cpp
// MyCustomShip.cpp
void AMyCustomShip::BeginPlay()
{
    Super::BeginPlay();

    UShipCustomizationComponent* CustomComp = FindComponentByClass<UShipCustomizationComponent>();
    if (CustomComp)
    {
        CustomComp->OnLoadoutChanged.AddDynamic(this, &AMyCustomShip::ApplyCustomPhysics);
    }
}

void AMyCustomShip::ApplyCustomPhysics()
{
    UShipCustomizationComponent* CustomComp = FindComponentByClass<UShipCustomizationComponent>();
    if (!CustomComp) return;

    FShipStats Stats = CustomComp->GetTotalStats();

    // Apply to custom physics
    MyPhysicsComponent->SetThrustMultiplier(Stats.ThrustPower);
    MyPhysicsComponent->SetMass(Stats.Mass);
}
```

---

### 5. Creating Custom Progression Systems

**Scenario**: You want to award XP based on player actions (kills, race wins, etc.).

**Pattern**: Call `AddXP()` from your game logic.

**Example: Award XP for Race Completion**:

```cpp
// RaceGameMode.cpp
void ARaceGameMode::OnRaceCompleted(APlayerController* Player, float Time, int32 Position)
{
    // Calculate XP based on position
    int32 XPReward = 0;
    switch (Position)
    {
        case 1: XPReward = 1000; break;  // 1st place
        case 2: XPReward = 750;  break;  // 2nd place
        case 3: XPReward = 500;  break;  // 3rd place
        default: XPReward = 250; break;  // Participation
    }

    // Award XP
    if (Player && Player->GetPawn())
    {
        UShipCustomizationComponent* CustomComp = Player->GetPawn()->FindComponentByClass<UShipCustomizationComponent>();
        if (CustomComp)
        {
            CustomComp->AddXP(XPReward);

            // Also award credits
            CustomComp->AddCredits(Position == 1 ? 500 : 250);

            UE_LOG(LogTemp, Log, TEXT("Player earned %d XP for finishing in position %d"),
                XPReward, Position);
        }
    }
}
```

**Example: Award XP for Combat**:

```cpp
// CombatSystem.cpp
void UCombatSystem::OnEnemyDestroyed(AActor* Enemy)
{
    // Get player ship
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC || !PC->GetPawn()) return;

    UShipCustomizationComponent* CustomComp = PC->GetPawn()->FindComponentByClass<UShipCustomizationComponent>();
    if (CustomComp)
    {
        // Award XP based on enemy type
        int32 XPReward = 100;
        if (Enemy->IsA(ABossEnemy::StaticClass()))
        {
            XPReward = 500;  // Boss kill
        }

        CustomComp->AddXP(XPReward);
        CustomComp->AddCredits(50);  // Loot credits
    }
}
```

---

### 6. Integrating with Save System

**Scenario**: You have a custom save system and want to include ship customization.

**Pattern**: Component auto-saves to its own slot. To integrate with your save system:

**Option A: Let Component Handle Its Own Save** (Recommended):

```cpp
// Just let the component auto-save
// It saves to slot "ShipCustomization" automatically
// No integration needed!
```

**Option B: Include in Your Custom SaveGame**:

```cpp
// MyGameSaveData.h
UCLASS()
class UMyGameSaveData : public USaveGame
{
    GENERATED_BODY()

public:
    // Your existing save data
    UPROPERTY()
    FVector PlayerPosition;

    UPROPERTY()
    int32 CurrentLevel;

    // ADD THIS: Ship customization data
    UPROPERTY()
    FPlayerProgressionData ShipProgression;

    UPROPERTY()
    FShipLoadout ShipLoadout;
};

// MyGameMode.cpp
void AMyGameMode::SaveGame()
{
    UMyGameSaveData* SaveData = Cast<UMyGameSaveData>(
        UGameplayStatics::CreateSaveGameObject(UMyGameSaveData::StaticClass())
    );

    // Save your data
    SaveData->PlayerPosition = Player->GetActorLocation();
    SaveData->CurrentLevel = CurrentLevelIndex;

    // INTEGRATION: Get ship customization data
    UShipCustomizationComponent* CustomComp = Player->GetPawn()->FindComponentByClass<UShipCustomizationComponent>();
    if (CustomComp)
    {
        SaveData->ShipProgression = CustomComp->ProgressionData;
        SaveData->ShipLoadout = CustomComp->CurrentLoadout;
    }

    UGameplayStatics::SaveGameToSlot(SaveData, TEXT("MyGameSave"), 0);
}

void AMyGameMode::LoadGame()
{
    UMyGameSaveData* LoadedData = Cast<UMyGameSaveData>(
        UGameplayStatics::LoadGameFromSlot(TEXT("MyGameSave"), 0)
    );

    if (LoadedData)
    {
        // Load your data
        Player->SetActorLocation(LoadedData->PlayerPosition);

        // INTEGRATION: Restore ship customization
        UShipCustomizationComponent* CustomComp = Player->GetPawn()->FindComponentByClass<UShipCustomizationComponent>();
        if (CustomComp)
        {
            CustomComp->ProgressionData = LoadedData->ShipProgression;
            CustomComp->CurrentLoadout = LoadedData->ShipLoadout;

            // Recalculate and apply stats
            CustomComp->CurrentLoadout.TotalStats = CustomComp->CalculateTotalStats();
            CustomComp->ApplyStatsToFlightController();
            CustomComp->UpdateShipVisuals();
        }
    }
}
```

---

## Best Practices

### 1. Always Check for Component Existence

**WRONG**:
```cpp
// DANGEROUS: Assumes component exists
UShipCustomizationComponent* CustomComp = Ship->FindComponentByClass<UShipCustomizationComponent>();
CustomComp->EquipPart(EShipPartCategory::Engine, FName("Engine_Basic"));  // CRASH if null!
```

**RIGHT**:
```cpp
UShipCustomizationComponent* CustomComp = Ship->FindComponentByClass<UShipCustomizationComponent>();
if (CustomComp)  // CHECK FIRST
{
    CustomComp->EquipPart(EShipPartCategory::Engine, FName("Engine_Basic"));
}
else
{
    UE_LOG(LogTemp, Warning, TEXT("Ship does not have customization component!"));
}
```

---

### 2. Use Events, Not Polling

**WRONG** (inefficient):
```cpp
// BAD: Polling in Tick
void AMyHUD::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Check if stats changed every frame (WASTEFUL!)
    FShipStats NewStats = CustomizationComp->GetTotalStats();
    if (NewStats != CachedStats)
    {
        UpdateStatsDisplay(NewStats);
        CachedStats = NewStats;
    }
}
```

**RIGHT** (event-driven):
```cpp
// GOOD: Bind to event once
void AMyHUD::BeginPlay()
{
    Super::BeginPlay();

    CustomizationComp->OnLoadoutChanged.AddDynamic(this, &AMyHUD::RefreshStatsDisplay);
}

void AMyHUD::RefreshStatsDisplay()
{
    // Only called when loadout actually changes!
    FShipStats Stats = CustomizationComp->GetTotalStats();
    UpdateStatsDisplay(Stats);
}
```

---

### 3. Validate Part IDs Before Using

**WRONG**:
```cpp
// DANGEROUS: Assumes part exists
CustomizationComp->EquipPart(EShipPartCategory::Engine, FName("Enigne_Advanced"));  // Typo!
```

**RIGHT**:
```cpp
FName PartID = FName("Engine_Advanced");

// Validate part exists
bool bFound = false;
FShipPartData PartData = CustomizationComp->GetPartData(PartID, bFound);

if (bFound)
{
    // Part exists, now check if unlocked
    if (CustomizationComp->IsPartUnlocked(PartID))
    {
        // Safe to equip
        CustomizationComp->EquipPart(PartData.Category, PartID);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Part is locked!"));
    }
}
else
{
    UE_LOG(LogTemp, Error, TEXT("Part ID '%s' does not exist!"), *PartID.ToString());
}
```

---

### 4. Don't Modify Stats Directly

**WRONG**:
```cpp
// BAD: Modifying cached stats directly (will be overwritten!)
FShipStats Stats = CustomizationComp->GetTotalStats();
Stats.ThrustPower += 0.5f;  // This change is LOST!
```

**RIGHT**:
```cpp
// GOOD: Create a temporary buff part
FShipPartData BuffPart;
BuffPart.PartID = FName("Buff_SpeedBoost");
BuffPart.Category = EShipPartCategory::Utility;
BuffPart.StatModifiers.ThrustPower = 0.5f;  // +50% thrust

// Add to data table temporarily (advanced)
// OR: Equip a pre-defined boost part
CustomizationComp->EquipPart(EShipPartCategory::Utility, FName("PowerUp_SpeedBoost"));
```

---

### 5. Cache Component References

**WRONG** (inefficient):
```cpp
void AMyActor::Tick(float DeltaTime)
{
    // BAD: Finding component every frame (SLOW!)
    UShipCustomizationComponent* CustomComp = GetOwner()->FindComponentByClass<UShipCustomizationComponent>();
    FShipStats Stats = CustomComp->GetTotalStats();
}
```

**RIGHT**:
```cpp
// MyActor.h
UPROPERTY()
UShipCustomizationComponent* CachedCustomComp;

// MyActor.cpp
void AMyActor::BeginPlay()
{
    Super::BeginPlay();

    // Find once, cache forever
    CachedCustomComp = GetOwner()->FindComponentByClass<UShipCustomizationComponent>();
}

void AMyActor::Tick(float DeltaTime)
{
    if (!CachedCustomComp) return;

    // Fast access
    FShipStats Stats = CachedCustomComp->GetTotalStats();
}
```

---

## Common Pitfalls and Solutions

### Pitfall 1: Stats Don't Update After Equipping Part

**Symptom**: Called `EquipPart()`, but ship physics didn't change.

**Causes**:
1. Data table reference not set in component
2. Part category doesn't match
3. Physics integration not set up correctly

**Solution**:

```cpp
// Debug logging
bool Success = CustomizationComp->EquipPart(EShipPartCategory::Engine, FName("Engine_Advanced"));

if (!Success)
{
    UE_LOG(LogTemp, Error, TEXT("Failed to equip part!"));

    // Check data table
    if (!CustomizationComp->ShipPartsDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("ShipPartsDataTable is NULL! Assign it in Blueprint."));
    }

    // Check if part exists
    bool bFound = false;
    CustomizationComp->GetPartData(FName("Engine_Advanced"), bFound);
    if (!bFound)
    {
        UE_LOG(LogTemp, Error, TEXT("Part 'Engine_Advanced' not found in data table!"));
    }

    // Check if unlocked
    if (!CustomizationComp->IsPartUnlocked(FName("Engine_Advanced")))
    {
        UE_LOG(LogTemp, Error, TEXT("Part is locked! Unlock cost: %d credits"),
            /* get unlock cost from part data */);
    }
}
else
{
    UE_LOG(LogTemp, Log, TEXT("Part equipped successfully!"));

    // Verify stats changed
    FShipStats Stats = CustomizationComp->GetTotalStats();
    UE_LOG(LogTemp, Log, TEXT("New thrust power: %.2f"), Stats.ThrustPower);
}
```

---

### Pitfall 2: Parts Not Showing in UI

**Symptom**: `GetUnlockedPartsInCategory()` returns empty array.

**Causes**:
1. No parts unlocked by default
2. Data table not assigned
3. Category enum mismatch

**Solution**:

```cpp
// Check data table assignment
if (!CustomizationComp->ShipPartsDataTable)
{
    UE_LOG(LogTemp, Error, TEXT("Data table not assigned!"));
    return;
}

// Check if any parts exist at all
TArray<FShipPartData> AllParts = CustomizationComp->GetPartsInCategory(
    EShipPartCategory::Engine
);
UE_LOG(LogTemp, Log, TEXT("Total engines in data table: %d"), AllParts.Num());

// Check how many are unlocked
TArray<FShipPartData> UnlockedParts = CustomizationComp->GetUnlockedPartsInCategory(
    EShipPartCategory::Engine
);
UE_LOG(LogTemp, Log, TEXT("Unlocked engines: %d"), UnlockedParts.Num());

// If no parts are unlocked, unlock some defaults
if (UnlockedParts.Num() == 0)
{
    UE_LOG(LogTemp, Warning, TEXT("No engines unlocked! Adding defaults..."));

    // Check data table for parts with bUnlockedByDefault = true
    for (const FShipPartData& Part : AllParts)
    {
        if (Part.bUnlockedByDefault)
        {
            UE_LOG(LogTemp, Log, TEXT("Default part found: %s"), *Part.PartID.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Part %s is NOT unlocked by default. Set bUnlockedByDefault = true in data table."),
                *Part.PartID.ToString());
        }
    }
}
```

---

### Pitfall 3: Multiplayer Desync

**Symptom**: Client and server have different ship stats.

**Causes**:
1. Client trying to equip parts directly (needs server authority)
2. Progression data replicating (it shouldn't)
3. Race condition in network replication

**Solution**:

```cpp
// ALWAYS check authority before applying physics
void UMyCustomizationComponent::ApplyStatsToFlightController()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Only server applies physics (authoritative)
    if (Owner->HasAuthority())
    {
        // Apply to physics system
        AOrbitalBody* OrbitalBody = Cast<AOrbitalBody>(Owner);
        if (OrbitalBody)
        {
            OrbitalBody->Mass = Stats.Mass;
            OrbitalBody->ThrustPower = Stats.ThrustPower * 100000.0f;
            // ... etc
        }

        UE_LOG(LogTemp, Log, TEXT("SERVER: Applied physics stats"));
    }
    else
    {
        // Client: Send RPC to server
        ASpaceship* Spaceship = Cast<ASpaceship>(Owner);
        if (Spaceship)
        {
            Spaceship->ServerApplyCustomization(Stats.Mass, Stats.ThrustPower * 100000.0f, Stats.MaxVelocity * 1000.0f);
        }

        UE_LOG(LogTemp, Log, TEXT("CLIENT: Requested server to apply physics stats"));
    }
}
```

---

### Pitfall 4: Save Game Not Loading

**Symptom**: Progression resets every time game restarts.

**Causes**:
1. SaveGame not being called
2. Wrong save slot name
3. Save file corrupted

**Solution**:

```cpp
// Verify save is actually happening
void UShipCustomizationComponent::UnlockPart(FName PartID)
{
    // ... unlock logic ...

    // CRITICAL: Save after unlock
    bool SaveSuccess = SaveCustomizationData();
    if (!SaveSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("FAILED TO SAVE! Progress may be lost!"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Saved customization data to slot 'ShipCustomization'"));
    }
}

// Verify load is happening
void UShipCustomizationComponent::BeginPlay()
{
    Super::BeginPlay();

    bool LoadSuccess = LoadCustomizationData();
    if (LoadSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Loaded customization data:"));
        UE_LOG(LogTemp, Log, TEXT("  - Player Level: %d"), ProgressionData.PlayerLevel);
        UE_LOG(LogTemp, Log, TEXT("  - Credits: %d"), ProgressionData.Credits);
        UE_LOG(LogTemp, Log, TEXT("  - Unlocked Parts: %d"), ProgressionData.UnlockedParts.Num());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No save data found, using defaults"));
    }
}
```

---

## Performance Considerations

### 1. Avoid Frequent `CalculateTotalStats()` Calls

**Problem**: Calculating stats every frame is wasteful.

**Solution**: Stats are cached in `CurrentLoadout.TotalStats` and only recalculated when parts change.

```cpp
// GOOD: Stats are cached
FShipStats Stats = CustomizationComp->GetTotalStats();  // Fast! Returns cached value

// BAD: Manual recalculation (unnecessary)
FShipStats Stats = CustomizationComp->CalculateTotalStats();  // Slow! Iterates all parts
```

---

### 2. Use Async Loading for Materials

**Problem**: `LoadSynchronous()` blocks game thread when loading skin materials.

**Current Implementation**: Synchronous loading (acceptable for small materials).

**Future Optimization**:

```cpp
void UShipCustomizationComponent::ApplySkinMaterialAsync()
{
    // Use Streamable Manager for async loading
    FStreamableManager& Streamable = UAssetManager::GetStreamableManager();

    Streamable.RequestAsyncLoad(
        SkinData.SkinMaterial.ToSoftObjectPath(),
        FStreamableDelegate::CreateUObject(this, &UShipCustomizationComponent::OnSkinMaterialLoaded)
    );
}

void UShipCustomizationComponent::OnSkinMaterialLoaded()
{
    // Material loaded, apply to mesh
    ApplySkinMaterial();
}
```

---

### 3. Batch Save Operations

**Problem**: Saving to disk on every part equip causes hitching.

**Current Behavior**: Immediate save after every unlock/equip.

**Optimization**:

```cpp
// Add dirty flag and timer-based save
private:
    bool bNeedsSave = false;
    float TimeSinceLastSave = 0.0f;

public:
    void TickComponent(float DeltaTime, ...)
    {
        TimeSinceLastSave += DeltaTime;

        // Auto-save every 30 seconds if dirty
        if (bNeedsSave && TimeSinceLastSave >= 30.0f)
        {
            SaveCustomizationData();
            bNeedsSave = false;
            TimeSinceLastSave = 0.0f;
        }
    }

    void MarkDirtyForSave()
    {
        bNeedsSave = true;
    }
```

---

## Thread Safety Notes

### Component is NOT Thread-Safe

**IMPORTANT**: `UShipCustomizationComponent` is **NOT thread-safe**. All functions **MUST** be called from the **game thread**.

**Safe Usage**:

```cpp
// SAFE: Called from game thread (Tick, BeginPlay, event handlers)
void AMyActor::Tick(float DeltaTime)
{
    CustomizationComp->GetTotalStats();  // OK
}
```

**Unsafe Usage**:

```cpp
// DANGEROUS: Called from worker thread (CRASH!)
void AMyActor::BackgroundTask()
{
    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
    {
        // CRASH! Not on game thread!
        CustomizationComp->EquipPart(EShipPartCategory::Engine, FName("Engine"));
    });
}
```

**Solution for Background Tasks**:

```cpp
// Queue task to game thread
AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]()
{
    // Do expensive work here (data processing, etc.)
    int32 ProcessedData = ExpensiveCalculation();

    // Queue game thread task
    AsyncTask(ENamedThreads::GameThread, [this, ProcessedData]()
    {
        // Now safe to call component functions
        CustomizationComp->AddCredits(ProcessedData);
    });
});
```

---

## Advanced Integration Patterns

### Pattern 1: Dynamic Part Unlocking Based on Achievements

```cpp
// AchievementManager.cpp
void UAchievementManager::OnAchievementUnlocked(FName AchievementID)
{
    // Map achievements to part unlocks
    TMap<FName, FName> AchievementToPartMap;
    AchievementToPartMap.Add(FName("FirstKill"), FName("Weapon_Laser_Tier2"));
    AchievementToPartMap.Add(FName("RaceWin"), FName("Engine_Racing_Advanced"));
    AchievementToPartMap.Add(FName("Survivor"), FName("Hull_Reinforced"));

    if (AchievementToPartMap.Contains(AchievementID))
    {
        FName PartToUnlock = AchievementToPartMap[AchievementID];

        // Get player ship
        APlayerController* PC = GetPlayerController();
        UShipCustomizationComponent* CustomComp = PC->GetPawn()->FindComponentByClass<UShipCustomizationComponent>();

        if (CustomComp)
        {
            // Unlock part for free (achievement reward)
            if (!CustomComp->IsPartUnlocked(PartToUnlock))
            {
                CustomComp->ProgressionData.UnlockedParts.Add(PartToUnlock);
                CustomComp->OnPartUnlocked.Broadcast(PartToUnlock);
                CustomComp->SaveCustomizationData();

                UE_LOG(LogTemp, Log, TEXT("Achievement '%s' unlocked part '%s'!"),
                    *AchievementID.ToString(), *PartToUnlock.ToString());
            }
        }
    }
}
```

---

### Pattern 2: Synergy Bonuses for Part Sets

```cpp
// CustomizationComponent_Extended.cpp
FShipStats UCustomizationComponent_Extended::CalculateTotalStats()
{
    // Calculate base stats
    FShipStats TotalStats = Super::CalculateTotalStats();

    // Detect synergies
    TArray<FString> EquippedPrefixes;
    for (const TPair<EShipPartCategory, FName>& Pair : CurrentLoadout.EquippedParts)
    {
        FString PartName = Pair.Value.ToString();
        FString Prefix = PartName.Left(PartName.Find("_"));
        EquippedPrefixes.AddUnique(Prefix);
    }

    // Check for "Combat" set (3+ parts)
    int32 CombatCount = 0;
    for (const TPair<EShipPartCategory, FName>& Pair : CurrentLoadout.EquippedParts)
    {
        if (Pair.Value.ToString().StartsWith("Combat_"))
        {
            CombatCount++;
        }
    }

    if (CombatCount >= 3)
    {
        TotalStats.WeaponDamage *= 1.15f;  // +15% weapon damage
        TotalStats.ShieldStrength += 50.0f;  // +50 shield
        UE_LOG(LogTemp, Log, TEXT("SYNERGY: Combat Set (+15% weapon damage, +50 shield)"));
    }

    return TotalStats;
}
```

---

### Pattern 3: Temporary Buffs/Debuffs

```cpp
// PowerUpSystem.cpp
void APowerUpSystem::ApplySpeedBoost(float Duration)
{
    APlayerController* PC = GetPlayerController();
    UShipCustomizationComponent* CustomComp = PC->GetPawn()->FindComponentByClass<UShipCustomizationComponent>();

    if (CustomComp)
    {
        // Create temporary buff part
        FShipPartData BuffPart;
        BuffPart.PartID = FName("Buff_SpeedBoost_Temp");
        BuffPart.Category = EShipPartCategory::Utility;
        BuffPart.StatModifiers.ThrustPower = 0.5f;  // +50% thrust
        BuffPart.StatModifiers.MaxVelocity = 0.3f;  // +30% speed

        // Add to temporary buffs (custom array)
        TemporaryBuffs.Add(BuffPart);

        // Recalculate stats
        RecalculateStatsWithBuffs();

        // Set timer to remove buff
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            RemoveSpeedBoost();
        }, Duration, false);
    }
}

void APowerUpSystem::RecalculateStatsWithBuffs()
{
    FShipStats TotalStats = CustomComp->CalculateTotalStats();

    // Add buffs
    for (const FShipPartData& Buff : TemporaryBuffs)
    {
        TotalStats = TotalStats + Buff.StatModifiers;
    }

    // Apply to physics (custom implementation)
    ApplyStatsToPhysics(TotalStats);
}
```

---

**Document Status**: COMPLETE
**Review Status**: READY FOR TECHNICAL REVIEW
**Recommended For**: Programmers, Technical Designers, Integration Engineers
