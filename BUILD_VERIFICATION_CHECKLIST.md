# Build Verification Checklist

## Overview
This document provides a comprehensive checklist for verifying that the Alexander codebase is buildable and properly configured. Use this checklist before attempting to compile the project.

---

## 1. Include Statements Validation

### Critical Headers
- [ ] `#include "CoreMinimal.h"` present in all .h files
- [ ] `#include "Generated.h"` or `.generated.h` includes at end of headers
- [ ] All forward declarations before #include "Generated.h"
- [ ] No circular #include patterns

### Ship Customization Includes
- [ ] `ShipCustomizationComponent.h` includes:
  - [ ] `CoreMinimal.h`
  - [ ] `Components/ActorComponent.h`
  - [ ] `Engine/DataTable.h`
  - [ ] `ShipCustomizationComponent.generated.h`

- [ ] `ShipPhysicsConfig.h` includes:
  - [ ] `CoreMinimal.h`
  - [ ] `Engine/DataAsset.h`
  - [ ] `ShipPhysicsConfig.generated.h`

- [ ] `OrbitalBody.h` includes:
  - [ ] `CoreMinimal.h`
  - [ ] `GameFramework/Actor.h`
  - [ ] `OrbitalMechanics.h`
  - [ ] `OrbitalBody.generated.h`

- [ ] `FlightController.h` includes appropriate engine headers

### Implementation File Includes
- [ ] `ShipCustomizationComponent.cpp` includes:
  - [ ] `ShipCustomizationComponent.h`
  - [ ] `FlightController.h`
  - [ ] `OrbitalBody.h`
  - [ ] `Spaceship.h`
  - [ ] `ShipPhysicsConfig.h`
  - [ ] `Components/StaticMeshComponent.h`
  - [ ] `GameFramework/SaveGame.h`
  - [ ] `Kismet/GameplayStatics.h`
  - [ ] `Net/UnrealNetwork.h`

---

## 2. Forward Declarations

### Header Files Must Use Forward Declarations
- [ ] `class UFlightController;` before using pointers
- [ ] `class AOrbitalBody;` before using pointers
- [ ] `class UStaticMeshComponent;` before using pointers
- [ ] `class UShipPhysicsConfig;` before using pointers
- [ ] All forward declarations BEFORE generated.h include

### Forward Declaration Naming
- [ ] Classes use `class` keyword (not `struct` for classes)
- [ ] Structs with complex members use `struct` keyword
- [ ] Names match actual class/struct definitions exactly

---

## 3. UPROPERTY Macro Validation

### Required Parameters
- [ ] `EditAnywhere` or `VisibleAnywhere` present
- [ ] `BlueprintReadWrite` or `BlueprintReadOnly` present (if Blueprint accessible)
- [ ] `Category` parameter specified with valid string
- [ ] `Replicated` keyword present for networked properties

### Common UPROPERTY Patterns
```
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Category")
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Category")
UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Category")
UPROPERTY(BlueprintAssignable, Category = "Events")
```

### Ship Customization Checks
- [ ] `CurrentLoadout` property has `Replicated` keyword
- [ ] All `BlueprintReadWrite` properties have appropriate categories
- [ ] Data table properties correctly typed as `UDataTable*`
- [ ] Event delegates properly declared with `BlueprintAssignable`
- [ ] Physics config property typed as `UShipPhysicsConfig*`

---

## 4. UFUNCTION Macro Validation

### BlueprintCallable Functions
- [ ] Return type valid (no void issues)
- [ ] Parameters properly typed with UPROPERTY or native types
- [ ] `Category` specified in macro
- [ ] Server RPC functions have matching `_Validate` and `_Implementation` variants

### Function Signature Requirements
- [ ] `GetLifetimeReplicatedProps()` has `const` keyword
- [ ] Virtual overrides marked with `override`
- [ ] Const correctness for getters (marked `const`)

### Example Validation
```cpp
UFUNCTION(BlueprintCallable, Category = "Customization")
bool EquipPart(EShipPartCategory Category, FName PartID);

UFUNCTION(BlueprintCallable, Category = "Customization")
FShipStats GetTotalStats() const { return CurrentLoadout.TotalStats; }
```

---

## 5. UCLASS Macro Validation

### Class Declarations
- [ ] `UCLASS()` macro present with appropriate specifiers
- [ ] `Blueprintable` if class needs Blueprint variants
- [ ] `BlueprintType` if struct used as Blueprint type
- [ ] `ClassGroup` specified
- [ ] `GENERATED_BODY()` present in class body

### Component Classes
- [ ] Components use `UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))`
- [ ] Inherits from `UActorComponent`

### Actor Classes
- [ ] Actors use `UCLASS(Blueprintable, ClassGroup = (Space))`
- [ ] Inherits from `AActor`

### Data Asset Classes
- [ ] Data assets use `UCLASS(BlueprintType)`
- [ ] Inherits from `UDataAsset`

---

## 6. Module Dependencies (Alexander.Build.cs)

### Required Modules
- [ ] `Core` - Core engine functionality
- [ ] `CoreUObject` - Object system
- [ ] `Engine` - Engine systems
- [ ] `InputCore` - Input handling
- [ ] `UMG` - UI framework (for data assets)
- [ ] `Net` - Network replication (if using Replicated)
- [ ] `PhysicsCore` - Physics simulation

### Optional Modules (Verify Usage)
- [ ] `Niagara` - VFX system
- [ ] `HTTP` - HTTP requests
- [ ] `Json`/`JsonUtilities` - JSON parsing
- [ ] `RHI`/`RenderCore` - Rendering
- [ ] `Slate`/`SlateCore` - UI framework
- [ ] `ToolMenus` - Editor menus
- [ ] `ProceduralMeshComponent` - Procedural meshes
- [ ] `GameplayTags` - Gameplay tag system
- [ ] `GameplayTasks` - Task system
- [ ] `AIModule` - AI systems
- [ ] `NavigationSystem` - Pathfinding
- [ ] `AudioMixer` - Audio system
- [ ] `EnhancedInput` - Enhanced input system
- [ ] `HeadMountedDisplay` - VR support
- [ ] `CinematicCamera` - Camera control
- [ ] `XRBase` - XR framework base

### Include Paths
- [ ] Public include paths set for Public/Testing
- [ ] Planetary module path added
- [ ] CelestialScaling/Public path added

---

## 7. Replication Setup

### Network Replication Properties
- [ ] Properties marked `Replicated` have matching `DOREPLIFETIME` calls
- [ ] `GetLifetimeReplicatedProps()` override present in .h
- [ ] `GetLifetimeReplicatedProps()` implementation in .cpp
- [ ] `DOREPLIFETIME()` macro called for each replicated property
- [ ] Macro format: `DOREPLIFETIME(ClassName, PropertyName);`

### Example Validation
```cpp
// In header
virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

// In implementation
void UShipCustomizationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UShipCustomizationComponent, CurrentLoadout);
}
```

### Replication Checklist
- [ ] `CurrentLoadout` property has `Replicated` in UPROPERTY
- [ ] `DOREPLIFETIME(UShipCustomizationComponent, CurrentLoadout)` exists
- [ ] Component has `SetIsReplicatedComponent(true)` in constructor
- [ ] No other replicated properties without DOREPLIFETIME calls

### Server RPC Functions
- [ ] Server functions have `_Validate()` variant
- [ ] Server functions have `_Implementation()` variant
- [ ] `_Validate()` returns bool (true = valid)
- [ ] `_Implementation()` contains actual logic
- [ ] Parameter validation in `_Validate()`

---

## 8. Constructor Initialization

### Component Constructor Requirements
- [ ] `PrimaryComponentTick.bCanEverTick` set appropriately
- [ ] `SetIsReplicatedComponent(true)` if networked
- [ ] All UPROPERTY members initialized to sensible defaults
- [ ] Cache pointers initialized to nullptr

### Example from ShipCustomizationComponent
```cpp
UShipCustomizationComponent::UShipCustomizationComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedComponent(true);

    CurrentLoadout.LoadoutName = FText::FromString("Default Loadout");
    CurrentLoadout.EquippedSkin = NAME_None;

    ProgressionData.PlayerLevel = 1;
    ProgressionData.Credits = 1000;

    CachedFlightController = nullptr;
    CachedShipMesh = nullptr;
    CachedOrbitalBody = nullptr;
}
```

---

## 9. BlueprintCallable Functions Accessibility

### Function Visibility
- [ ] `BlueprintCallable` functions are `public`
- [ ] Functions have proper `UFUNCTION()` macro
- [ ] Return type is blueprint-compatible (native types, UObject*, structs with USTRUCT)
- [ ] Parameters are blueprint-compatible

### Blueprint-Compatible Types
- [ ] Primitives: `bool`, `int32`, `float`, `FString`, `FName`, `FText`
- [ ] Vectors: `FVector`, `FRotator`, `FTransform`
- [ ] Containers: `TArray<T>`, `TMap<K,V>` (if T, K, V are compatible)
- [ ] UObject derivatives: Any UCLASS() type
- [ ] Structs: USTRUCT(BlueprintType)
- [ ] Enums: UENUM(BlueprintType)

### Ship Customization Functions
- [ ] `EquipPart()` - returns bool (compatible)
- [ ] `CalculateTotalStats()` - returns FShipStats struct (compatible)
- [ ] `GetTotalStats()` - returns FShipStats (compatible)
- [ ] `GetEquippedPart()` - returns FShipPartData, out param bool (compatible)
- [ ] `GetEquippedSkin()` - returns FShipSkinData, out param bool (compatible)

---

## 10. Struct Definitions (USTRUCT)

### Required for Data Types
- [ ] `FShipStats` has USTRUCT(BlueprintType)
- [ ] `FShipPartData` extends FTableRowBase
- [ ] `FShipSkinData` extends FTableRowBase
- [ ] `FShipLoadout` has USTRUCT(BlueprintType)
- [ ] `FPlayerProgressionData` has USTRUCT(BlueprintType)
- [ ] All have `GENERATED_BODY()`

### Struct Member Validation
- [ ] All members have UPROPERTY with category
- [ ] EditAnywhere/VisibleAnywhere specified
- [ ] BlueprintReadWrite/ReadOnly specified
- [ ] Category strings are meaningful

---

## 11. Enum Definitions (UENUM)

### Required for Gameplay
- [ ] `EShipPartCategory` has `UENUM(BlueprintType)`
- [ ] `EShipPartRarity` has `UENUM(BlueprintType)`
- [ ] All enum values have UMETA(DisplayName = "...")
- [ ] Enum type specified: `uint8`, `uint16`, `uint32`, `int64`

---

## 12. Integration Points

### ShipCustomizationComponent Connections
- [ ] Forward declare `UFlightController`
- [ ] Forward declare `AOrbitalBody`
- [ ] Forward declare `UShipPhysicsConfig`
- [ ] Include actual headers in .cpp only
- [ ] `GetFlightController()` helper method exists
- [ ] `GetOrbitalBody()` helper method exists

### OrbitalBody Integration
- [ ] OrbitalBody.h forward declares required types
- [ ] OrbitalMechanics.h properly included
- [ ] All physics components properly initialized

### FlightController Integration
- [ ] References are cached for performance
- [ ] Cache invalidation handled properly
- [ ] Null checks before dereferencing

---

## 13. Generated Files

### Header Generated Includes
- [ ] `.generated.h` include at END of .h file
- [ ] After all forward declarations
- [ ] After all type definitions (enums, structs)
- [ ] Before closing `#endif`

### Order of Includes (Correct Pattern)
```cpp
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"

// Forward declarations
class UFlightController;
class AOrbitalBody;

// Type definitions (enums, structs)
UENUM(BlueprintType)
enum class EShipPartCategory : uint8 { /* ... */ };

// Class definition
UCLASS(Blueprintable, BlueprintType, meta = (BlueprintSpawnableComponent))
class ALEXANDER_API UShipCustomizationComponent : public UActorComponent { /* ... */ };

// Generated include (LAST)
#include "ShipCustomizationComponent.generated.h"
```

---

## 14. Compile Verification Steps

### Pre-Compilation
- [ ] Run `verify_includes.py` - checks all #includes
- [ ] Run `verify_build_cs.py` - validates Alexander.Build.cs
- [ ] Run `verify_replication.py` - checks network setup
- [ ] Review `BUILD_STATUS_REPORT.md` for issues

### Compilation
- [ ] Generate Visual Studio project files
- [ ] Open Alexander.sln in Visual Studio 2022
- [ ] Clean solution (Build > Clean Solution)
- [ ] Build solution (Build > Build Solution)
- [ ] Check Output window for errors

### Post-Compilation
- [ ] No C4251 warnings (DLL interface)
- [ ] No redefinition errors
- [ ] No unresolved external symbol errors
- [ ] Binaries generated successfully

---

## Common Compilation Issues

### Issue: Unresolved External Symbol
**Cause**: Missing module dependency or forward declare not implemented
**Fix**:
- [ ] Check Alexander.Build.cs for required module
- [ ] Verify .cpp #includes matching .h forward declares
- [ ] Check UPROPERTY class references have pointers

### Issue: C4251 Warning
**Cause**: Exporting template instances
**Fix**:
- [ ] Mark UPROPERTY/UFUNCTION as `ALEXANDER_API`
- [ ] Use TSoftObjectPtr for deferred loads
- [ ] Use pointers instead of embedded objects

### Issue: Circular Dependencies
**Cause**: Headers #include each other
**Fix**:
- [ ] Use forward declarations
- [ ] Move includes to .cpp files
- [ ] Verify include order

### Issue: Generated.h Not Found
**Cause**: Macro or class definition errors
**Fix**:
- [ ] Verify UCLASS/USTRUCT syntax
- [ ] Check for typos in macro names
- [ ] Ensure closing braces and semicolons

---

## Sign-Off

- [ ] All sections reviewed
- [ ] No outstanding issues
- [ ] Ready for compilation
- [ ] Ready for integration

**Reviewed By**: ___________________
**Date**: ___________________
**Status**: ___________________

---

## Related Documents

- `compile_instructions.md` - Step-by-step compilation guide
- `BUILD_STATUS_REPORT.md` - Auto-generated verification report
- `verify_includes.py` - Include validation script
- `verify_build_cs.py` - Build.cs validation script
- `verify_replication.py` - Replication setup validation script
