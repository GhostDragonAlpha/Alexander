# UE 5.6 Compatibility Issues Discovery

## Overview
During testing infrastructure setup (re-enabling ComprehensiveSystemTests.cpp), discovered extensive pre-existing UE 5.6 API compatibility issues blocking compilation.

## Root Cause
UE 5.6's UnrealHeaderTool (UHT) enforces stricter type restrictions:
- **TMap Value Restrictions**: Cannot use `TArray<T>` or nested `TMap<K,V>` directly as UPROPERTY TMap values
- **Naming Conflicts**: Cannot have custom types sharing engine names with built-in Unreal types
- **Solution Pattern**: Wrap complex types in USTRUCT for TMap compatibility

## Files Requiring TMap Wrapper Structs

### Nested TMap Issues (TMap<X, TMap<Y,Z>>)
1. `AdvancedAIBehaviors.h:435` - `TMap<AActor*, TMap<AActor*, FAIRelationship>> AIRelationships;`
2. `PhysicsConsensus.h:444` - `TMap<int32, TMap<int32, TArray<FValidationVote>>> ValidationVotes;`
3. `ProceduralMissionGenerator.h:497` - `TMap<FName, TMap<FName, FString>> EventParameters;`
4. `TickReportingComponent.h:39` - `TMap<FString, TMap<UActorComponent*, float>> SavedConfigurations;`

### TArray in TMap Issues (TMap<X, TArray<Y>>)
1. `PlanetaryResourcesComponent.h:737` - `TMap<EResourceType, TArray<EResourceType>> ProcessingRecipes;`
2. `AdvancedAIBehaviors.h:432` - `TMap<AActor*, TArray<FAIMemoryEntry>> AIMemories;`
3. `AutomationAPIServer.h:173` - `TMap<int32, TArray<FObserverMeasurement>> StoredObservations;`
4. `AutomationEfficiencySystem.h:238` - `TMap<EAutomationDeviceType, TArray<FMaintenanceRequirement>>`
5. `AutomationEfficiencySystem.h:274` - `TMap<EAutomationDeviceType, TArray<FAutomationUpgrade>>`
6. `AutomationEfficiencySystem.h:278` - `TMap<AActor*, TArray<FAutomationUpgrade>> AppliedUpgrades;`
7. `HarvestingMachine.h:143` - `TMap<AFarmPlot*, TArray<FIntPoint>> PlotCellMap;`
8. `PhysicsConsensus.h:440` - `TMap<int32, TArray<FPositionReport>> PositionHistory;`
9. `ProceduralMissionGenerator.h:467` - `TMap<FName, TArray<FMissionObjectiveTemplate>>`
10. `SpaceStationBlueprintLibrary.h:202` - `TMap<EStationModuleType, TArray<UStaticMesh*>>`
11. `VFXManager.h:311` - `TMap<FString, TArray<UNiagaraComponent*>> VFXPools;`
12. `WeatherVFXLibrary.h:255` - `TMap<EWeatherTypeNew, TArray<UParticleSystemComponent*>>`

## Already Fixed
✅ `DialogueContentDatabase.h` - Created `FDialogueEntryArray` wrapper
✅ `DialogueContentDatabase.cpp` - Updated to use `.Entries` accessor
✅ `MissionTemplateDataAsset.h` - Renamed `FMissionTemplateDataAsset` → `FMissionTemplateData`
✅ `MissionTemplateDataAsset.cpp` - Updated all references
✅ `TutorialOnboardingSystem.h` - Renamed enum to avoid conflict
✅ `WeatherVFXLibrary.cpp` - Added missing includes, commented deprecated API
✅ `SpaceStationBlueprintLibrary.cpp` - Added missing includes

## Current Blockers (from last build)
1. `AudioSystemManager.h:653` - `TMap<FName,USoundBase*>` used as TMap value
2. `TutorialSequenceData.h:262,288` - References to non-existent `FTutorialStepSequence`
3. `TutorialSystem.h:169` - `TArray<FName>` as TMap value
4. `TutorialSystem.h:172` - `TMap<FName,int32>` as TMap value

## Fix Strategy Pattern

### Example: TArray in TMap
```cpp
// BEFORE (UE 5.5 - works)
UPROPERTY()
TMap<FName, TArray<FDialogueEntry>> Greetings;

// AFTER (UE 5.6 - required)
USTRUCT(BlueprintType)
struct FDialogueEntryArray
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDialogueEntry> Entries;
};

UPROPERTY()
TMap<FName, FDialogueEntryArray> Greetings;
```

### Example: Nested TMap
```cpp
// BEFORE
UPROPERTY()
TMap<FName, TMap<FName, int32>> NestedData;

// AFTER
USTRUCT(BlueprintType)
struct FIntMapWrapper
{
    GENERATED_BODY()
    UPROPERTY()
    TMap<FName, int32> Data;
};

UPROPERTY()
TMap<FName, FIntMapWrapper> NestedData;
```

## Recommended Action
**Option 1 (Comprehensive)**: Fix all 16+ files with wrapper structs (8-12 hours estimated)
**Option 2 (Pragmatic)**: Keep ComprehensiveSystemTests.cpp disabled, proceed with other testing infrastructure
**Option 3 (Incremental)**: Fix blockers only, address others as encountered

## Impact Assessment
- **Build Status**: Currently FAILING due to TMap restrictions
- **Test Status**: Cannot re-enable C++ system tests until build succeeds
- **Severity**: BLOCKING for testing infrastructure Phase 1
- **Scope**: 16+ files across core systems (AI, Physics, Missions, VFX, Economy)

## Next Steps
Given the extensive scope and the NEW user request for UE helper program integration, recommend:
1. Document these issues (this file) ✅
2. Pivot to UE helper programs integration task (per user's latest request)
3. Return to systematic UE 5.6 fixes in dedicated session

## Build Command Reference
```bash
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" AlexanderEditor Win64 Development "C:\Users\allen\Desktop\Alexander\Alexander\Alexander.uproject" -waitmutex
```

## Related Files
- [ComprehensiveSystemTests.cpp.disabled](Source/Alexander/Tests/ComprehensiveSystemTests.cpp.disabled)
- [DialogueContentDatabase.h](Source/Alexander/Public/DialogueContentDatabase.h) - Example of fixed file
- [TESTING_INFRASTRUCTURE_PLAN.md](TESTING_INFRASTRUCTURE_PLAN.md) - Original plan blocked by these issues

---
**Created**: 2025-11-08
**Status**: DOCUMENTED - Awaiting systematic fix
**Blocking**: Testing Infrastructure Phase 1
