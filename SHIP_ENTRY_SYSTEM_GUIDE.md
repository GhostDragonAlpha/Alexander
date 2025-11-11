# Ship Entry/Exit System - Quick Implementation Guide

## Overview

Add a simple ship entry system so players can walk up to the ship and get in.

## Option A: Simple Blueprint Implementation (5 minutes)

### In BP_VRSpaceshipPlayer:

**1. Add Entry Trigger Component**
- Add Component → Box Collision
- Name: `EntryTrigger`
- Transform:
  - Location: (0, 0, 0) - at ship center or cockpit door
  - Scale: (2, 2, 3) - 200x200x300 cm box

**2. Set Collision Settings**
- Collision Preset: `OverlapAllDynamic`
- Generate Overlap Events: ✓ Enabled

**3. Add Entry Logic (Event Graph)**

```
Event BeginPlay
  → Get Player Character
  → Store reference

Event BeginComponentOverlap (EntryTrigger)
  → Cast to Character
  → Branch (Is Valid?)
      True:
        → Print String "Press E to Enter Ship"
        → Enable Input (E key)
      False:
        → Do nothing

Event EndComponentOverlap (EntryTrigger)
  → Disable E key input
  → Print String "" (clear message)

Input Action E (Pressed)
  → Get Player Controller
  → Possess (this ship)
  → Disable Character Mesh
  → Set View Target (ship camera)
  → Print String "Entered Ship - Use ship controls to fly"
```

**4. Add Exit Logic**

```
Input Action F (Pressed) - Exit Ship
  → Spawn Character at (ship location + offset)
  → Get Player Controller
  → UnPossess
  → Possess (spawned character)
  → Print String "Exited Ship"
```

## Option B: C++ Implementation (More robust, 15 minutes)

### Add to Spaceship.h:

```cpp
// Ship entry trigger
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entry")
class UBoxComponent* EntryTrigger;

// Can player enter this ship?
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entry")
bool bAllowEntry = true;

// Player in range
UPROPERTY()
class ACharacter* PlayerInRange;

// Entry functions
UFUNCTION()
void OnEntryTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

UFUNCTION()
void OnEntryTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

UFUNCTION(BlueprintCallable, Category = "Entry")
void EnterShip(ACharacter* Character);

UFUNCTION(BlueprintCallable, Category = "Entry")
void ExitShip();
```

### Add to Spaceship.cpp Constructor:

```cpp
// Create entry trigger
EntryTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("EntryTrigger"));
EntryTrigger->SetupAttachment(RootComponent);
EntryTrigger->SetBoxExtent(FVector(200, 200, 300));
EntryTrigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

// Bind overlap events
EntryTrigger->OnComponentBeginOverlap.AddDynamic(this, &ASpaceship::OnEntryTriggerBeginOverlap);
EntryTrigger->OnComponentEndOverlap.AddDynamic(this, &ASpaceship::OnEntryTriggerEndOverlap);
```

### Implement Functions:

```cpp
void ASpaceship::OnEntryTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character && bAllowEntry)
    {
        PlayerInRange = Character;

        // Show prompt (could use HUD widget instead)
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green,
                TEXT("Press E to Enter Ship"));
        }
    }
}

void ASpaceship::OnEntryTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    ACharacter* Character = Cast<ACharacter>(OtherActor);
    if (Character == PlayerInRange)
    {
        PlayerInRange = nullptr;
    }
}

void ASpaceship::EnterShip(ACharacter* Character)
{
    if (!Character) return;

    APlayerController* PC = Cast<APlayerController>(Character->GetController());
    if (!PC) return;

    // Store character for exit
    // ... (save character reference)

    // Possess ship
    PC->Possess(this);

    // Hide character
    Character->SetActorHiddenInGame(true);
    Character->SetActorEnableCollision(false);

    UE_LOG(LogTemp, Log, TEXT("Player entered ship"));
}

void ASpaceship::ExitShip()
{
    APlayerController* PC = GetController<APlayerController>();
    if (!PC) return;

    // Spawn character near ship exit
    FVector ExitLocation = GetActorLocation() + GetActorForwardVector() * 500.0f;

    // Re-possess character or spawn new one
    // ... (restore player character)

    UE_LOG(LogTemp, Log, TEXT("Player exited ship"));
}
```

## Option C: Quick Test Without C++ (2 minutes)

If you just want to test flying without walking:

1. Open level
2. Run: `py Content/Python/setup_planet_test_level.py close`
3. Delete PlayerStart
4. Set GameMode Default Pawn Class to `BP_VRSpaceshipPlayer`
5. Press Play → You spawn directly in ship!

## Testing the System

### Setup Test Level:
```python
# In Unreal Editor console
py Content/Python/setup_planet_test_level.py close
```

### Manual Test:
1. Press Play (PIE)
2. Spawn as character on planet
3. Walk forward 100m toward ship
4. See "Press E to Enter Ship"
5. Press E → You're now in the ship
6. Use ship controls to fly

### What Should Happen:
- ✓ Player walks on planet surface (gravity pulls down)
- ✓ Player approaches ship
- ✓ Overlap trigger fires
- ✓ Press E → Camera switches to ship
- ✓ Ship responds to controls
- ✓ Fly away from planet

## Troubleshooting

**Ship falls through planet:**
- Enable collision on planet mesh: `Collision Preset = BlockAll`
- Or increase planet mesh collision complexity

**No gravity:**
- Check planet has `Mass` property set (~5.97e24 for Earth)
- Check `OrbitalBody` tick is enabled
- Verify character has physics enabled

**Can't enter ship:**
- Check EntryTrigger collision is `OverlapAllDynamic`
- Check overlap events are bound
- Check character is valid

**Ship doesn't respond:**
- Verify ship has `FlightController` component
- Check input bindings exist
- Test ship with API first to isolate issue

## Quick Start Checklist

- [ ] Run `py Content/Python/setup_planet_test_level.py close`
- [ ] Add EntryTrigger BoxCollision to ship BP
- [ ] Add overlap event handling
- [ ] Test: Play → Walk → Enter → Fly
- [ ] Add exit functionality (Press F)
- [ ] Add HUD prompt widgets (optional polish)

---

**Estimated Time**: 5-15 minutes depending on implementation choice
**Result**: Complete walk → enter ship → fly gameplay loop
