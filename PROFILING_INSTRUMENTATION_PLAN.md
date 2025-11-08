# Ship Customization Profiling Instrumentation Plan

## Overview
This document outlines where and how to add profiling instrumentation to the ShipCustomizationComponent and related systems.

## File to Instrument
**Primary Target**: `Source/Alexander/Private/ShipCustomizationComponent.cpp`
**Header**: `Source/Alexander/Public/ShipCustomizationComponent.h`
**Profiling Macros**: `Source/Alexander/Public/ShipCustomizationProfiling.h` (already created)

## Instrumentation Strategy

All functions below should be instrumented with `PROFILE_SHIP_CUSTOMIZATION_SCOPE()` macro to track execution time automatically.

The macro is defined as:
```cpp
#define PROFILE_SHIP_CUSTOMIZATION_SCOPE(StatName) \
    SCOPE_CYCLE_COUNTER(STAT_ShipCustomization_##StatName)
```

This creates a scoped counter that:
- Starts timing at function entry
- Stops timing at function exit
- Automatically reports to the stat system
- Viewable via console: `stat ShipCustomization`

## Critical Path Functions (High Priority - Budget: < 0.1ms each)

### 1. UShipCustomizationComponent::CalculateTotalStats()
**Current Budget**: 0.05ms
**Frequency**: Called on part equip, loadout load
**Impact**: Core stat calculation, called frequently

**Location**: ShipCustomizationComponent.cpp, line ~150

**Instrumentation**:
```cpp
FShipStats UShipCustomizationComponent::CalculateTotalStats()
{
    PROFILE_SHIP_CUSTOMIZATION_SCOPE(CalculateStats);

    // Existing code...
    FShipStats TotalStats;

    // ... iteration and addition logic ...

    return TotalStats;
}
```

**Expected Baseline**: 0.05ms (iterates 8 categories, adds stats)

---

### 2. UShipCustomizationComponent::GetEquippedPart()
**Current Budget**: 0.02ms
**Frequency**: Multiple times per CalculateTotalStats
**Impact**: Helper function called in hot path

**Location**: ShipCustomizationComponent.cpp

**Instrumentation**:
```cpp
FShipPartData UShipCustomizationComponent::GetEquippedPart(EShipPartCategory Category, bool& bFound)
{
    PROFILE_SHIP_CUSTOMIZATION_SCOPE(GetEquippedPart);

    FShipPartData PartData;
    bFound = false;

    // ... map lookup logic ...

    return PartData;
}
```

**Expected Baseline**: 0.02ms (simple map lookup)

---

### 3. UShipCustomizationComponent::GetPartData()
**Current Budget**: 0.02ms
**Frequency**: Per part lookup
**Impact**: Data table query in hot path

**Location**: ShipCustomizationComponent.cpp

**Instrumentation**:
```cpp
FShipPartData UShipCustomizationComponent::GetPartData(FName PartID, bool& bFound)
{
    PROFILE_SHIP_CUSTOMIZATION_SCOPE(GetPartData);

    bFound = false;

    if (!ShipPartsDataTable)
    {
        return FShipPartData();
    }

    // ... data table lookup ...

    return PartData;
}
```

**Expected Baseline**: 0.02ms (data table row lookup is optimized)

---

## Performance-Sensitive Functions (Medium Priority - Budget: 0.1-0.3ms each)

### 4. UShipCustomizationComponent::EquipPart()
**Current Budget**: 0.15ms
**Frequency**: Called on user input, typically 1-3 times per play session equip screen
**Impact**: High visibility to player, should be responsive

**Location**: ShipCustomizationComponent.cpp, line ~91

**Instrumentation**:
```cpp
bool UShipCustomizationComponent::EquipPart(EShipPartCategory Category, FName PartID)
{
    PROFILE_SHIP_CUSTOMIZATION_SCOPE(EquipPart);

    // Existing validation code...

    // Equip the part
    CurrentLoadout.EquippedParts[Category] = PartID;

    // Recalculate stats
    CurrentLoadout.TotalStats = CalculateTotalStats();

    // Apply to flight controller
    ApplyStatsToFlightController();

    // ... rest of function ...

    return true;
}
```

**Expected Breakdown**:
- Validation: 0.02ms
- Equip: 0.03ms
- CalculateTotalStats: 0.05ms
- ApplyStatsToFlightController: 0.05ms
- **Total**: 0.15ms

---

### 5. UShipCustomizationComponent::ApplyStatsToFlightController()
**Current Budget**: 0.2ms
**Frequency**: Called on part equip, loadout load
**Impact**: Updates flight controller state

**Location**: ShipCustomizationComponent.cpp

**Instrumentation**:
```cpp
void UShipCustomizationComponent::ApplyStatsToFlightController()
{
    PROFILE_SHIP_CUSTOMIZATION_SCOPE(ApplyStats);

    UFlightController* FlightController = GetFlightController();
    if (!FlightController)
    {
        return;
    }

    FShipStats Stats = CurrentLoadout.TotalStats;

    // Apply each stat...
    FlightController->SetThrustPower(Stats.ThrustPower);
    FlightController->SetMaxVelocity(Stats.MaxVelocity);
    // ... etc ...

    UE_LOG(LogTemp, Verbose, TEXT("Applied stats to FlightController"));
}
```

**Expected Breakdown**:
- FlightController lookup: 0.01ms
- Stat application (10 properties): 0.15ms
- Event broadcast: 0.04ms
- **Total**: 0.2ms

---

### 6. UShipCustomizationComponent::UpdateShipVisuals()
**Current Budget**: 0.25ms
**Frequency**: Called on part/skin equip
**Impact**: Visual feedback to player

**Location**: ShipCustomizationComponent.cpp

**Instrumentation**:
```cpp
void UShipCustomizationComponent::UpdateShipVisuals()
{
    PROFILE_SHIP_CUSTOMIZATION_SCOPE(UpdateVisuals);

    // Update mesh components...
    // Update materials...
    // Trigger VFX...

    UE_LOG(LogTemp, Verbose, TEXT("Updated ship visuals"));
}
```

**Expected Baseline**: 0.25ms (mesh changes + material application)

---

### 7. UShipCustomizationComponent::ApplySkinMaterial()
**Current Budget**: 0.15ms
**Frequency**: Called on skin equip
**Impact**: Visual appearance

**Location**: ShipCustomizationComponent.cpp

**Instrumentation**:
```cpp
void UShipCustomizationComponent::ApplySkinMaterial()
{
    PROFILE_SHIP_CUSTOMIZATION_SCOPE(ApplySkin);

    UStaticMeshComponent* ShipMesh = GetShipMesh();
    if (!ShipMesh)
    {
        return;
    }

    // Get skin material
    // Apply to mesh...
}
```

**Expected Baseline**: 0.15ms (material assignment)

---

## Data Query Functions (Lower Priority - Budget: < 1.0ms, called infrequently)

### 8. UShipCustomizationComponent::GetPartsInCategory()
**Current Budget**: 0.5ms
**Frequency**: Called from UI
**Impact**: UI population

**Instrumentation**:
```cpp
TArray<FShipPartData> UShipCustomizationComponent::GetPartsInCategory(EShipPartCategory Category)
{
    PROFILE_SHIP_CUSTOMIZATION_SCOPE(GetPartsInCategory);

    TArray<FShipPartData> Result;

    if (!ShipPartsDataTable)
    {
        return Result;
    }

    // Iterate and filter...

    return Result;
}
```

**Expected Baseline**: 0.3-0.5ms (depends on part count, ~50 parts total)

---

### 9. UShipCustomizationComponent::GetUnlockedPartsInCategory()
**Current Budget**: 0.5ms
**Frequency**: Called from UI
**Impact**: UI population

**Instrumentation**:
```cpp
TArray<FShipPartData> UShipCustomizationComponent::GetUnlockedPartsInCategory(EShipPartCategory Category)
{
    PROFILE_SHIP_CUSTOMIZATION_SCOPE(GetUnlockedParts);

    TArray<FShipPartData> Result;

    // Filter by unlocked status...

    return Result;
}
```

**Expected Baseline**: 0.4-0.5ms

---

### 10. UShipCustomizationComponent::GetAllSkins()
**Current Budget**: 0.3ms
**Frequency**: Called from UI
**Impact**: UI population

**Instrumentation**:
```cpp
TArray<FShipSkinData> UShipCustomizationComponent::GetAllSkins()
{
    PROFILE_SHIP_CUSTOMIZATION_SCOPE(GetAllSkins);

    TArray<FShipSkinData> Result;

    if (!ShipSkinsDataTable)
    {
        return Result;
    }

    // Collect all skins...

    return Result;
}
```

**Expected Baseline**: 0.2-0.3ms (fewer skins than parts)

---

### 11. UShipCustomizationComponent::GetUnlockedSkins()
**Current Budget**: 0.3ms
**Frequency**: Called from UI
**Impact**: UI population

**Instrumentation**:
```cpp
TArray<FShipSkinData> UShipCustomizationComponent::GetUnlockedSkins()
{
    PROFILE_SHIP_CUSTOMIZATION_SCOPE(GetUnlockedSkins);

    TArray<FShipSkinData> Result;

    // Filter by unlocked status...

    return Result;
}
```

**Expected Baseline**: 0.2-0.3ms

---

## Progression Functions (Lower Priority - Budget: < 1.0ms, called infrequently)

### 12. UShipCustomizationComponent::UnlockPart()
**Current Budget**: 0.2ms
**Frequency**: Rare, on progression milestone
**Impact**: Progression system

**Instrumentation**:
```cpp
bool UShipCustomizationComponent::UnlockPart(FName PartID)
{
    PROFILE_SHIP_CUSTOMIZATION_SCOPE(UnlockPart);

    if (IsPartUnlocked(PartID))
    {
        return false;
    }

    ProgressionData.UnlockedParts.Add(PartID);

    OnPartUnlocked.Broadcast(PartID);

    return true;
}
```

**Expected Baseline**: 0.1-0.2ms

---

### 13. UShipCustomizationComponent::UnlockSkin()
**Current Budget**: 0.2ms
**Frequency**: Rare, on progression milestone
**Impact**: Progression system

**Instrumentation**:
```cpp
bool UShipCustomizationComponent::UnlockSkin(FName SkinID)
{
    PROFILE_SHIP_CUSTOMIZATION_SCOPE(UnlockSkin);

    if (IsSkinUnlocked(SkinID))
    {
        return false;
    }

    ProgressionData.UnlockedSkins.Add(SkinID);

    OnSkinEquipped.Broadcast(SkinID);

    return true;
}
```

**Expected Baseline**: 0.1-0.2ms

---

### 14. UShipCustomizationComponent::AddXP()
**Current Budget**: 0.1ms
**Frequency**: Called frequently during play
**Impact**: Progression system

**Instrumentation**:
```cpp
void UShipCustomizationComponent::AddXP(int32 Amount)
{
    PROFILE_SHIP_CUSTOMIZATION_SCOPE(AddXP);

    ProgressionData.TotalXP += Amount;

    CheckLevelUp();
}
```

**Expected Baseline**: 0.05-0.1ms

---

### 15. UShipCustomizationComponent::CheckLevelUp()
**Current Budget**: 0.5ms (called infrequently)
**Frequency**: On every XP gain, but only does work on level up
**Impact**: Progression system

**Instrumentation**:
```cpp
void UShipCustomizationComponent::CheckLevelUp()
{
    PROFILE_SHIP_CUSTOMIZATION_SCOPE(CheckLevelUp);

    int32 RequiredXP = GetXPForLevel(ProgressionData.PlayerLevel + 1);

    if (ProgressionData.TotalXP >= RequiredXP)
    {
        int32 OldLevel = ProgressionData.PlayerLevel;
        ProgressionData.PlayerLevel++;

        OnPlayerLevelUp.Broadcast(OldLevel, ProgressionData.PlayerLevel);
    }
}
```

**Expected Baseline**: 0.05ms (fast path when no level up), 0.3ms (on level up)

---

## Initialization Functions

### 16. UShipCustomizationComponent::BeginPlay()
**Current Budget**: 2.0ms (called once at level start)
**Frequency**: Once per level
**Impact**: Level initialization

**Instrumentation**:
```cpp
void UShipCustomizationComponent::BeginPlay()
{
    PROFILE_SHIP_CUSTOMIZATION_SCOPE(BeginPlay);

    Super::BeginPlay();

    // Load customization data...
    // Initialize default loadout...
    // Calculate stats...
    // Update visuals...
}
```

**Expected Baseline**: 1.5-2.0ms (includes file I/O for save game)

---

### 17. UShipCustomizationComponent::InitializeDefaultLoadout()
**Current Budget**: 1.0ms
**Frequency**: Once per new character
**Impact**: Character initialization

**Instrumentation**:
```cpp
void UShipCustomizationComponent::InitializeDefaultLoadout()
{
    PROFILE_SHIP_CUSTOMIZATION_SCOPE(InitLoadout);

    // Set default parts for each category...
    // Calculate initial stats...
}
```

**Expected Baseline**: 0.8-1.0ms

---

## Save/Load Functions

### 18. UShipCustomizationComponent::SaveCustomizationData()
**Current Budget**: 1.0ms
**Frequency**: On exit/save point
**Impact**: Data persistence

**Instrumentation**:
```cpp
bool UShipCustomizationComponent::SaveCustomizationData()
{
    PROFILE_SHIP_CUSTOMIZATION_SCOPE(SaveData);

    // Serialize to save game...
    // Write to disk...

    return true;
}
```

**Expected Baseline**: 0.8-1.0ms (includes file I/O)

---

### 19. UShipCustomizationComponent::LoadCustomizationData()
**Current Budget**: 1.5ms
**Frequency**: On level load
**Impact**: Data loading

**Instrumentation**:
```cpp
bool UShipCustomizationComponent::LoadCustomizationData()
{
    PROFILE_SHIP_CUSTOMIZATION_SCOPE(LoadData);

    // Read from save game...
    // Deserialize...

    return true;
}
```

**Expected Baseline**: 1.0-1.5ms (includes file I/O)

---

## Helper Functions

### 20. UShipCustomizationComponent::GetFlightController()
**Current Budget**: 0.02ms
**Frequency**: Multiple times per frame (cached)
**Impact**: Cache hit/miss affects performance

**Instrumentation** (optional, lightweight):
```cpp
UFlightController* UShipCustomizationComponent::GetFlightController() const
{
    // Already cached, minimal overhead
    if (!CachedFlightController)
    {
        CachedFlightController = /* lookup */;
    }

    return CachedFlightController;
}
```

Note: Caching is already implemented, so minimal profiling needed.

---

## Summary Table

| Function | Location | Budget | Priority | Include |
|----------|----------|--------|----------|---------|
| CalculateTotalStats | Line ~150 | 0.05ms | HIGH | ✓ |
| GetEquippedPart | Line ~200 | 0.02ms | HIGH | ✓ |
| GetPartData | Line ~300 | 0.02ms | HIGH | ✓ |
| EquipPart | Line ~91 | 0.15ms | HIGH | ✓ |
| ApplyStatsToFlightController | Line ~400 | 0.2ms | HIGH | ✓ |
| UpdateShipVisuals | Line ~500 | 0.25ms | MEDIUM | ✓ |
| ApplySkinMaterial | Line ~550 | 0.15ms | MEDIUM | ✓ |
| GetPartsInCategory | Line ~600 | 0.5ms | LOW | ✓ |
| GetUnlockedPartsInCategory | Line ~650 | 0.5ms | LOW | ✓ |
| GetAllSkins | Line ~700 | 0.3ms | LOW | ✓ |
| GetUnlockedSkins | Line ~750 | 0.3ms | LOW | ✓ |
| UnlockPart | Line ~800 | 0.2ms | LOW | ✓ |
| UnlockSkin | Line ~850 | 0.2ms | LOW | ✓ |
| AddXP | Line ~900 | 0.1ms | LOW | ✓ |
| CheckLevelUp | Line ~950 | 0.5ms | LOW | ✓ |
| BeginPlay | Line ~58 | 2.0ms | INIT | ✓ |
| InitializeDefaultLoadout | Line ~1000 | 1.0ms | INIT | ✓ |
| SaveCustomizationData | Line ~1100 | 1.0ms | SAVE | ✓ |
| LoadCustomizationData | Line ~1150 | 1.5ms | LOAD | ✓ |

## Implementation Notes

1. **Include Header**: Add `#include "ShipCustomizationProfiling.h"` at top of ShipCustomizationComponent.cpp
2. **Profiling is automatic**: Once `PROFILE_SHIP_CUSTOMIZATION_SCOPE()` is added, timing is handled by Unreal's stat system
3. **To view stats**: Type `stat ShipCustomization` in the console
4. **In shipping builds**: Profiling is disabled automatically (zero overhead)
5. **Test immediately after**: Run `test_performance_profiling.py` after implementing to verify baselines

## Performance Testing Workflow

1. Add `PROFILE_SHIP_CUSTOMIZATION_SCOPE()` to target function
2. Compile and package
3. Run with `-unattended` flag and stat logging
4. Execute test scenario (spawn ships, equip parts)
5. Dump stat output and parse results
6. Compare against budget
7. Log any regressions

---

**Last Updated**: November 7, 2025
**Status**: READY FOR IMPLEMENTATION
