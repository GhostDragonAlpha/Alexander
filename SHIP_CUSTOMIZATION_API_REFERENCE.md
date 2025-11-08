# Ship Customization System - API Reference

**Project:** Alexander VR Space Flight Game
**Version:** 1.0
**Last Updated:** November 7, 2025

---

## Table of Contents

1. [UShipCustomizationComponent](#ushipcustomizationcomponent)
2. [Data Structures](#data-structures)
3. [Enumerations](#enumerations)
4. [Delegates](#delegates)
5. [UShipPhysicsConfig](#ushipphysicsconfig)
6. [Usage Examples](#usage-examples)

---

## UShipCustomizationComponent

Main component class for ship customization. Attach to any ship actor to enable customization.

**Header**: `Source/Alexander/Public/ShipCustomizationComponent.h`

**Inheritance**: `UActorComponent` → `UShipCustomizationComponent`

---

### Constructor

```cpp
UShipCustomizationComponent::UShipCustomizationComponent()
```

**Description**: Initializes component with default values

**Default State**:
- `PrimaryComponentTick.bCanEverTick = false`
- `SetIsReplicated(true)`
- `CurrentLoadout.LoadoutName = "Default Loadout"`
- `ProgressionData.PlayerLevel = 1`
- `ProgressionData.Credits = 1000`

---

### Properties

#### Data References

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
UDataTable* ShipPartsDataTable;
```
**Description**: Reference to ship parts data table (FShipPartData rows)
**Required**: Yes (system won't function without this)
**Set In**: Blueprint editor or constructor

---

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
UDataTable* ShipSkinsDataTable;
```
**Description**: Reference to ship skins data table (FShipSkinData rows)
**Required**: Optional (skins can be skipped)

---

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
UShipPhysicsConfig* PhysicsConfig;
```
**Description**: Optional physics preset to apply on startup
**Required**: Optional (will use default physics if null)

---

#### Current State

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Customization")
FShipLoadout CurrentLoadout;
```
**Description**: Active ship loadout (equipped parts, skin, stats)
**Replication**: Replicated to all clients
**Access**: Read-only in Blueprint (use EquipPart/EquipSkin to modify)

---

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Progression")
FPlayerProgressionData ProgressionData;
```
**Description**: Player progression (level, XP, credits, unlocks)
**Replication**: NOT replicated (client-only, SaveGame-based)
**Access**: Read-only in Blueprint (use AddXP/AddCredits to modify)

---

### Loadout Management

#### EquipPart

```cpp
UFUNCTION(BlueprintCallable, Category = "Customization")
bool EquipPart(EShipPartCategory Category, FName PartID);
```

**Description**: Equips a part in the specified category

**Parameters**:
- `Category`: Part category (Engine, Weapon, Shield, etc.)
- `PartID`: Unique part identifier (row name from data table)

**Returns**: `true` if part was equipped successfully, `false` otherwise

**Validation Checks**:
- Part exists in data table
- Part is unlocked
- Player meets level requirement
- Category matches part's category

**Side Effects**:
- Updates `CurrentLoadout.EquippedParts`
- Recalculates `CurrentLoadout.TotalStats`
- Calls `ApplyStatsToFlightController()`
- Calls `UpdateShipVisuals()`
- Broadcasts `OnPartEquipped` and `OnLoadoutChanged`

**Example**:
```cpp
bool Success = CustomizationComponent->EquipPart(
    EShipPartCategory::Engine,
    FName("Engine_Advanced")
);

if (Success)
{
    UE_LOG(LogTemp, Log, TEXT("Engine equipped!"));
}
```

---

#### UnequipPart

```cpp
UFUNCTION(BlueprintCallable, Category = "Customization")
bool UnequipPart(EShipPartCategory Category);
```

**Description**: Removes equipped part from category

**Parameters**:
- `Category`: Category to unequip

**Returns**: `true` if part was unequipped, `false` if no part was equipped

**Side Effects**:
- Removes entry from `CurrentLoadout.EquippedParts`
- Recalculates stats
- Updates physics and visuals
- Broadcasts `OnLoadoutChanged`

**Example**:
```cpp
CustomizationComponent->UnequipPart(EShipPartCategory::Weapon);
```

---

#### EquipSkin

```cpp
UFUNCTION(BlueprintCallable, Category = "Customization")
bool EquipSkin(FName SkinID);
```

**Description**: Equips a visual skin

**Parameters**:
- `SkinID`: Unique skin identifier (row name from skins data table)

**Returns**: `true` if skin was equipped successfully

**Validation Checks**:
- Skin exists in data table
- Skin is unlocked

**Side Effects**:
- Updates `CurrentLoadout.EquippedSkin`
- Calls `ApplySkinMaterial()`
- Broadcasts `OnSkinEquipped` and `OnLoadoutChanged`

**Example**:
```cpp
CustomizationComponent->EquipSkin(FName("Skin_Racing_Red"));
```

---

#### GetEquippedPart

```cpp
UFUNCTION(BlueprintCallable, Category = "Customization")
FShipPartData GetEquippedPart(EShipPartCategory Category, bool& bFound);
```

**Description**: Gets data for currently equipped part in category

**Parameters**:
- `Category`: Category to query
- `bFound`: (output) Set to `true` if part found, `false` otherwise

**Returns**: Part data structure (check `bFound` before using)

**Example**:
```cpp
bool bFound = false;
FShipPartData EngineData = CustomizationComponent->GetEquippedPart(
    EShipPartCategory::Engine,
    bFound
);

if (bFound)
{
    UE_LOG(LogTemp, Log, TEXT("Engine: %s"), *EngineData.DisplayName.ToString());
}
```

---

#### GetEquippedSkin

```cpp
UFUNCTION(BlueprintCallable, Category = "Customization")
FShipSkinData GetEquippedSkin(bool& bFound);
```

**Description**: Gets data for currently equipped skin

**Parameters**:
- `bFound`: (output) Set to `true` if skin equipped and found

**Returns**: Skin data structure

---

### Stats Calculation

#### CalculateTotalStats

```cpp
UFUNCTION(BlueprintCallable, Category = "Customization")
FShipStats CalculateTotalStats();
```

**Description**: Calculates total ship stats from all equipped parts

**Algorithm**:
1. Initialize base stats (thrust=1.0, velocity=1.0, mass=1000, etc.)
2. For each equipped part, add part's `StatModifiers` to total
3. Return combined stats

**Returns**: Combined ship statistics

**Note**: This is called automatically when parts change. Manually calling is usually unnecessary.

**Example**:
```cpp
FShipStats Stats = CustomizationComponent->CalculateTotalStats();
UE_LOG(LogTemp, Log, TEXT("Total Thrust: %.2fx"), Stats.ThrustPower);
UE_LOG(LogTemp, Log, TEXT("Total Mass: %.2f kg"), Stats.Mass);
```

---

#### ApplyStatsToFlightController

```cpp
UFUNCTION(BlueprintCallable, Category = "Customization")
void ApplyStatsToFlightController();
```

**Description**: Applies calculated stats to physics systems

**Integration Points**:
- **AOrbitalBody**:
  - `Mass` → affects gravitational forces
  - `ThrustPower` → affects acceleration
  - `StandardGravitationalParameter` → recalculated (μ = G * Mass)
- **ASpaceship**:
  - `MaxThrust` → compatibility property
- **UFlightController**:
  - `MaxSafeVelocity` → speed limit

**Network Behavior**:
- **Server**: Applies directly to physics
- **Client**: Sends RPC to server (`ServerApplyCustomization`)

**Note**: Called automatically after stats recalculation. Manual calls rarely needed.

---

#### GetTotalStats

```cpp
UFUNCTION(BlueprintCallable, Category = "Customization")
FShipStats GetTotalStats() const;
```

**Description**: Returns cached total stats (read-only)

**Returns**: Current total stats (from `CurrentLoadout.TotalStats`)

**Example**:
```cpp
FShipStats Stats = CustomizationComponent->GetTotalStats();
float ThrustMultiplier = Stats.ThrustPower;  // e.g., 1.5 = 150% thrust
```

---

### Progression

#### UnlockPart

```cpp
UFUNCTION(BlueprintCallable, Category = "Progression")
bool UnlockPart(FName PartID);
```

**Description**: Unlocks a part for use (spends credits)

**Parameters**:
- `PartID`: Part to unlock

**Returns**: `true` if unlocked successfully

**Validation**:
- Part exists
- Not already unlocked
- Player has enough credits

**Side Effects**:
- Deducts credits (`ProgressionData.Credits -= UnlockCost`)
- Adds to `ProgressionData.UnlockedParts`
- Broadcasts `OnPartUnlocked`
- Saves to disk

**Example**:
```cpp
if (CustomizationComponent->UnlockPart(FName("Engine_Legendary")))
{
    UE_LOG(LogTemp, Log, TEXT("Legendary engine unlocked!"));
}
else
{
    UE_LOG(LogTemp, Warning, TEXT("Not enough credits or already unlocked"));
}
```

---

#### UnlockSkin

```cpp
UFUNCTION(BlueprintCallable, Category = "Progression")
bool UnlockSkin(FName SkinID);
```

**Description**: Unlocks a skin (spends credits)

**Similar to**: `UnlockPart` but for skins

---

#### IsPartUnlocked

```cpp
UFUNCTION(BlueprintCallable, Category = "Progression")
bool IsPartUnlocked(FName PartID) const;
```

**Description**: Checks if part is unlocked

**Returns**: `true` if part is unlocked (either by default or purchased)

**Example**:
```cpp
if (CustomizationComponent->IsPartUnlocked(FName("Weapon_Laser")))
{
    // Show "Equip" button
}
else
{
    // Show "Unlock for 500 credits" button
}
```

---

#### IsSkinUnlocked

```cpp
UFUNCTION(BlueprintCallable, Category = "Progression")
bool IsSkinUnlocked(FName SkinID) const;
```

**Description**: Checks if skin is unlocked

**Similar to**: `IsPartUnlocked` but for skins

---

#### AddXP

```cpp
UFUNCTION(BlueprintCallable, Category = "Progression")
void AddXP(int32 Amount);
```

**Description**: Adds experience points, checks for level up

**Parameters**:
- `Amount`: XP to add

**Side Effects**:
- Increases `ProgressionData.TotalXP`
- Checks for level up (XP curve: Level² * 100)
- Awards credits on level up (PlayerLevel * 100)
- Broadcasts `OnPlayerLevelUp` if leveled up
- Saves to disk

**Example**:
```cpp
// Award XP for completing mission
CustomizationComponent->AddXP(500);
```

---

#### AddCredits

```cpp
UFUNCTION(BlueprintCallable, Category = "Progression")
void AddCredits(int32 Amount);
```

**Description**: Adds currency

**Parameters**:
- `Amount`: Credits to add (can be negative to deduct)

**Side Effects**:
- Updates `ProgressionData.Credits`
- Saves to disk

**Example**:
```cpp
// Reward for destroying enemy
CustomizationComponent->AddCredits(250);
```

---

#### GetPlayerLevel

```cpp
UFUNCTION(BlueprintCallable, Category = "Progression")
int32 GetPlayerLevel() const;
```

**Description**: Gets current player level

**Returns**: Player level (1-20+)

---

#### GetCredits

```cpp
UFUNCTION(BlueprintCallable, Category = "Progression")
int32 GetCredits() const;
```

**Description**: Gets current credits balance

**Returns**: Available credits

---

### Loadout Presets

#### SaveLoadout

```cpp
UFUNCTION(BlueprintCallable, Category = "Loadout")
void SaveLoadout(const FText& LoadoutName);
```

**Description**: Saves current loadout as a preset

**Parameters**:
- `LoadoutName`: Display name for loadout (e.g., "Racing Build")

**Side Effects**:
- Adds to `ProgressionData.SavedLoadouts`
- Saves to disk

**Example**:
```cpp
CustomizationComponent->SaveLoadout(FText::FromString("My Tank Build"));
```

---

#### LoadLoadout

```cpp
UFUNCTION(BlueprintCallable, Category = "Loadout")
bool LoadLoadout(int32 LoadoutIndex);
```

**Description**: Loads a saved loadout preset

**Parameters**:
- `LoadoutIndex`: Index in saved loadouts array

**Returns**: `true` if loaded successfully

**Side Effects**:
- Replaces `CurrentLoadout` with saved loadout
- Recalculates stats
- Updates physics and visuals
- Broadcasts `OnLoadoutChanged`

**Example**:
```cpp
// Load first saved loadout
CustomizationComponent->LoadLoadout(0);
```

---

#### DeleteLoadout

```cpp
UFUNCTION(BlueprintCallable, Category = "Loadout")
bool DeleteLoadout(int32 LoadoutIndex);
```

**Description**: Deletes a saved loadout

**Parameters**:
- `LoadoutIndex`: Index to delete

**Returns**: `true` if deleted successfully

---

#### GetSavedLoadouts

```cpp
UFUNCTION(BlueprintCallable, Category = "Loadout")
TArray<FShipLoadout> GetSavedLoadouts() const;
```

**Description**: Gets all saved loadouts

**Returns**: Array of saved loadouts

**Example**:
```cpp
TArray<FShipLoadout> Loadouts = CustomizationComponent->GetSavedLoadouts();
for (const FShipLoadout& Loadout : Loadouts)
{
    UE_LOG(LogTemp, Log, TEXT("Loadout: %s"), *Loadout.LoadoutName.ToString());
}
```

---

### Visual Updates

#### UpdateShipVisuals

```cpp
UFUNCTION(BlueprintCallable, Category = "Visual")
void UpdateShipVisuals();
```

**Description**: Updates ship visual appearance (skin and part meshes)

**Currently**: Only applies skin material (part mesh swapping is TODO)

**Note**: Called automatically when parts/skins change

---

#### ApplySkinMaterial

```cpp
UFUNCTION(BlueprintCallable, Category = "Visual")
void ApplySkinMaterial();
```

**Description**: Applies equipped skin material to ship mesh

**Algorithm**:
1. Get ship mesh component
2. Get equipped skin data
3. Load skin material (synchronous)
4. Apply to all mesh material slots

**Note**: Manual calls rarely needed (called by `UpdateShipVisuals`)

---

### Data Queries

#### GetPartsInCategory

```cpp
UFUNCTION(BlueprintCallable, Category = "Data")
TArray<FShipPartData> GetPartsInCategory(EShipPartCategory Category);
```

**Description**: Gets all parts in a category (locked and unlocked)

**Parameters**:
- `Category`: Category to query

**Returns**: Array of part data

**Example**:
```cpp
TArray<FShipPartData> Engines = CustomizationComponent->GetPartsInCategory(
    EShipPartCategory::Engine
);

for (const FShipPartData& Engine : Engines)
{
    UE_LOG(LogTemp, Log, TEXT("Engine: %s"), *Engine.DisplayName.ToString());
}
```

---

#### GetUnlockedPartsInCategory

```cpp
UFUNCTION(BlueprintCallable, Category = "Data")
TArray<FShipPartData> GetUnlockedPartsInCategory(EShipPartCategory Category);
```

**Description**: Gets only unlocked parts in category

**Parameters**:
- `Category`: Category to query

**Returns**: Array of unlocked part data

**Use Case**: Populate UI dropdown with available parts

---

#### GetAllSkins

```cpp
UFUNCTION(BlueprintCallable, Category = "Data")
TArray<FShipSkinData> GetAllSkins();
```

**Description**: Gets all skins (locked and unlocked)

**Returns**: Array of skin data

---

#### GetUnlockedSkins

```cpp
UFUNCTION(BlueprintCallable, Category = "Data")
TArray<FShipSkinData> GetUnlockedSkins();
```

**Description**: Gets only unlocked skins

**Returns**: Array of unlocked skin data

---

#### GetPartData

```cpp
UFUNCTION(BlueprintCallable, Category = "Data")
FShipPartData GetPartData(FName PartID, bool& bFound);
```

**Description**: Looks up part data by ID

**Parameters**:
- `PartID`: Part identifier
- `bFound`: (output) Set to `true` if found

**Returns**: Part data (check `bFound` before using)

---

#### GetSkinData

```cpp
UFUNCTION(BlueprintCallable, Category = "Data")
FShipSkinData GetSkinData(FName SkinID, bool& bFound);
```

**Description**: Looks up skin data by ID

**Parameters**:
- `SkinID`: Skin identifier
- `bFound`: (output) Set to `true` if found

**Returns**: Skin data

---

### Save/Load

#### SaveCustomizationData

```cpp
UFUNCTION(BlueprintCallable, Category = "SaveLoad")
bool SaveCustomizationData();
```

**Description**: Saves progression and loadout to disk

**SaveGame Slot**: "ShipCustomization" (index 0)

**Saves**:
- `ProgressionData` (level, XP, credits, unlocks, saved loadouts)
- `CurrentLoadout` (equipped parts, skin)

**Returns**: `true` if save succeeded

**Note**: Called automatically on important events (unlock, equip, etc.)

---

#### LoadCustomizationData

```cpp
UFUNCTION(BlueprintCallable, Category = "SaveLoad")
bool LoadCustomizationData();
```

**Description**: Loads progression and loadout from disk

**Returns**: `true` if load succeeded, `false` if no save found

**Note**: Called automatically in `BeginPlay()`

---

### Physics Configuration

#### ApplyPhysicsConfig

```cpp
UFUNCTION(BlueprintCallable, Category = "Configuration")
void ApplyPhysicsConfig(UShipPhysicsConfig* Config);
```

**Description**: Applies a physics preset (overrides base stats)

**Parameters**:
- `Config`: Physics configuration asset (e.g., LightFighter, HeavyFreighter)

**Side Effects**:
- Overrides base stats with config values
- Recalculates total stats (base + parts)
- Updates physics systems

**Example**:
```cpp
UShipPhysicsConfig* LightFighterConfig = LoadObject<UShipPhysicsConfig>(...);
CustomizationComponent->ApplyPhysicsConfig(LightFighterConfig);
```

---

## Data Structures

### FShipStats

```cpp
USTRUCT(BlueprintType)
struct FShipStats
{
    GENERATED_BODY()

    // Thrust power multiplier (1.0 = 100%)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float ThrustPower = 1.0f;

    // Max velocity multiplier (1.0 = 100%)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxVelocity = 1.0f;

    // Rotation speed multiplier (1.0 = 100%)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float RotationSpeed = 1.0f;

    // Acceleration multiplier (1.0 = 100%)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Acceleration = 1.0f;

    // Hull integrity (health points)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float HullIntegrity = 100.0f;

    // Shield strength (shield points)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float ShieldStrength = 0.0f;

    // Weapon damage multiplier (1.0 = 100%)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float WeaponDamage = 1.0f;

    // Energy capacity (energy units)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float EnergyCapacity = 100.0f;

    // Energy regen rate (units per second)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float EnergyRegenRate = 10.0f;

    // Mass (kilograms)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Mass = 1000.0f;

    // Operator overload: Combine stats
    FShipStats operator+(const FShipStats& Other) const;
};
```

**Usage**:
- **Multipliers** (ThrustPower, MaxVelocity, RotationSpeed, Acceleration, WeaponDamage):
  - Values: 0.5 to 2.0 (50% to 200%)
  - Combined by addition: `1.0 + 0.5 + 0.3 = 1.8x`

- **Additive** (HullIntegrity, ShieldStrength, EnergyCapacity, EnergyRegenRate, Mass):
  - Values: Absolute units (HP, kg, etc.)
  - Combined by addition: `1000 kg + 500 kg = 1500 kg`

---

### FShipPartData

```cpp
USTRUCT(BlueprintType)
struct FShipPartData : public FTableRowBase
{
    GENERATED_BODY()

    // Unique identifier (row name)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
    FName PartID;

    // Display name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
    FText DisplayName;

    // Description text
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
    FText Description;

    // Part category
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
    EShipPartCategory Category;

    // Rarity tier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
    EShipPartRarity Rarity;

    // Minimum level to equip
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
    int32 LevelRequirement = 1;

    // Cost to unlock (credits)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
    int32 UnlockCost = 0;

    // Unlocked by default?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
    bool bUnlockedByDefault = false;

    // Stat modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    FShipStats StatModifiers;

    // Visual mesh (optional)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TSoftObjectPtr<UStaticMesh> PartMesh;

    // Icon texture
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TSoftObjectPtr<UTexture2D> Icon;

    // Parent part for upgrade tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    FName ParentPartID;

    // Can be upgraded?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    bool bCanBeUpgraded = true;

    // Upgrade level (0 = base)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    int32 UpgradeLevel = 0;
};
```

**Usage**: Defines a single part in a data table

**Example Data**:
```
PartID: Engine_Advanced
DisplayName: "Advanced Ion Engine"
Category: Engine
Rarity: Rare
LevelRequirement: 5
UnlockCost: 500
StatModifiers:
  - ThrustPower: 0.5  (adds 50% thrust)
  - Mass: 200.0       (adds 200 kg)
```

---

### FShipSkinData

```cpp
USTRUCT(BlueprintType)
struct FShipSkinData : public FTableRowBase
{
    GENERATED_BODY()

    // Unique identifier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin")
    FName SkinID;

    // Display name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin")
    FText DisplayName;

    // Description
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin")
    FText Description;

    // Rarity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin")
    EShipPartRarity Rarity;

    // Material instance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TSoftObjectPtr<UMaterialInstance> SkinMaterial;

    // Icon texture
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TSoftObjectPtr<UTexture2D> Icon;

    // Unlock cost
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    int32 UnlockCost = 0;

    // Unlocked by default?
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    bool bUnlockedByDefault = true;
};
```

**Usage**: Defines a visual skin in a data table

---

### FShipLoadout

```cpp
USTRUCT(BlueprintType)
struct FShipLoadout
{
    GENERATED_BODY()

    // Loadout name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
    FText LoadoutName;

    // Equipped parts (category → part ID)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
    TMap<EShipPartCategory, FName> EquippedParts;

    // Equipped skin
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
    FName EquippedSkin;

    // Total stats (cached)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loadout")
    FShipStats TotalStats;
};
```

**Usage**: Represents a complete ship configuration

---

### FPlayerProgressionData

```cpp
USTRUCT(BlueprintType)
struct FPlayerProgressionData
{
    GENERATED_BODY()

    // Player level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    int32 PlayerLevel = 1;

    // Total XP
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    int32 TotalXP = 0;

    // Currency
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    int32 Credits = 0;

    // Unlocked parts
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    TArray<FName> UnlockedParts;

    // Unlocked skins
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    TArray<FName> UnlockedSkins;

    // Saved loadouts
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    TArray<FShipLoadout> SavedLoadouts;

    // Active loadout index
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progression")
    int32 ActiveLoadoutIndex = 0;
};
```

**Usage**: Tracks player progression (NOT replicated, saved to disk)

---

## Enumerations

### EShipPartCategory

```cpp
UENUM(BlueprintType)
enum class EShipPartCategory : uint8
{
    Engine      UMETA(DisplayName = "Engine"),
    Thrusters   UMETA(DisplayName = "Thrusters"),
    Hull        UMETA(DisplayName = "Hull"),
    Wings       UMETA(DisplayName = "Wings"),
    Cockpit     UMETA(DisplayName = "Cockpit"),
    Weapon      UMETA(DisplayName = "Weapon"),
    Shield      UMETA(DisplayName = "Shield"),
    Utility     UMETA(DisplayName = "Utility")
};
```

**Description**: Part categories (mutually exclusive slots)

**Usage**: Each category can have ONE equipped part at a time

---

### EShipPartRarity

```cpp
UENUM(BlueprintType)
enum class EShipPartRarity : uint8
{
    Common      UMETA(DisplayName = "Common"),
    Uncommon    UMETA(DisplayName = "Uncommon"),
    Rare        UMETA(DisplayName = "Rare"),
    Epic        UMETA(DisplayName = "Epic"),
    Legendary   UMETA(DisplayName = "Legendary")
};
```

**Description**: Rarity tiers for parts and skins

**Recommended Power Scaling**:
- Common: 0.9x - 1.1x
- Uncommon: 1.0x - 1.3x
- Rare: 1.2x - 1.6x
- Epic: 1.5x - 1.9x
- Legendary: 1.8x - 2.0x

---

## Delegates

### FOnLoadoutChanged

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadoutChanged);
```

**Description**: Fired when loadout changes (part equipped, skin changed, loadout loaded)

**Blueprint Usage**:
```
Event OnLoadoutChanged
  → Update HUD
  → Refresh stats display
  → Save game
```

---

### FOnPartEquipped

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnPartEquipped,
    EShipPartCategory, Category,
    FName, PartID
);
```

**Description**: Fired when a part is equipped

**Parameters**:
- `Category`: Which category was equipped
- `PartID`: Which part was equipped

**Blueprint Usage**:
```
Event OnPartEquipped (Category, PartID)
  → Play equip sound
  → Show notification "Engine upgraded!"
```

---

### FOnSkinEquipped

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnSkinEquipped,
    FName, SkinID
);
```

**Description**: Fired when a skin is equipped

---

### FOnPartUnlocked

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnPartUnlocked,
    FName, PartID
);
```

**Description**: Fired when a part is unlocked

**Blueprint Usage**:
```
Event OnPartUnlocked (PartID)
  → Show "NEW!" badge on part
  → Play unlock animation
```

---

### FOnPlayerLevelUp

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnPlayerLevelUp,
    int32, OldLevel,
    int32, NewLevel
);
```

**Description**: Fired when player levels up

**Blueprint Usage**:
```
Event OnPlayerLevelUp (OldLevel, NewLevel)
  → Show level up screen
  → Play fanfare sound
  → Award bonus credits
```

---

## UShipPhysicsConfig

**Header**: `Source/Alexander/Public/ShipPhysicsConfig.h`

**Inheritance**: `UDataAsset` → `UShipPhysicsConfig`

**Description**: Preset physics configuration for different ship types

---

### Properties

```cpp
UCLASS(BlueprintType)
class UShipPhysicsConfig : public UDataAsset
{
    // Configuration name
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    FText ConfigName;

    // Description
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    FText Description;

    // Mass (kg)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Mass = 1000.0f;

    // Thrust power (Newtons)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float ThrustPower = 100000.0f;

    // Max velocity (m/s)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MaxVelocity = 1000.0f;

    // Angular thrust power (Newtons)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularThrustPower = 50000.0f;

    // Fuel capacity (future)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float FuelCapacity = 1000.0f;

    // Fuel consumption rate (future)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float FuelConsumptionRate = 1.0f;

    // Acceleration multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
    float AccelerationMultiplier = 1.0f;

    // Rotation speed multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
    float RotationSpeedMultiplier = 1.0f;

    // Stability assist strength (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flight")
    float StabilityAssistStrength = 0.5f;
};
```

**Example Presets**:

```
Light Fighter:
  Mass: 800 kg
  ThrustPower: 120000 N
  MaxVelocity: 1200 m/s
  AccelerationMultiplier: 1.3
  RotationSpeedMultiplier: 1.5

Heavy Freighter:
  Mass: 3000 kg
  ThrustPower: 150000 N
  MaxVelocity: 600 m/s
  AccelerationMultiplier: 0.7
  RotationSpeedMultiplier: 0.6
```

---

## Usage Examples

### Example 1: Basic Part Equip

```cpp
// C++
UShipCustomizationComponent* CustomComp = Ship->FindComponentByClass<UShipCustomizationComponent>();

if (CustomComp)
{
    bool Success = CustomComp->EquipPart(EShipPartCategory::Engine, FName("Engine_Advanced"));
    if (Success)
    {
        UE_LOG(LogTemp, Log, TEXT("Engine equipped! New stats:"));
        FShipStats Stats = CustomComp->GetTotalStats();
        UE_LOG(LogTemp, Log, TEXT("  Thrust: %.2fx"), Stats.ThrustPower);
        UE_LOG(LogTemp, Log, TEXT("  Mass: %.2f kg"), Stats.Mass);
    }
}
```

---

### Example 2: Unlock and Equip Flow

```cpp
// Blueprint equivalent in C++
void AGameMode::UnlockAndEquipPart(ASpaceship* Ship, FName PartID)
{
    UShipCustomizationComponent* CustomComp = Ship->FindComponentByClass<UShipCustomizationComponent>();
    if (!CustomComp) return;

    // Check if part is locked
    if (!CustomComp->IsPartUnlocked(PartID))
    {
        // Try to unlock
        if (CustomComp->UnlockPart(PartID))
        {
            UE_LOG(LogTemp, Log, TEXT("Part unlocked: %s"), *PartID.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to unlock part (not enough credits)"));
            return;
        }
    }

    // Get part data to determine category
    bool bFound = false;
    FShipPartData PartData = CustomComp->GetPartData(PartID, bFound);
    if (!bFound) return;

    // Equip the part
    CustomComp->EquipPart(PartData.Category, PartID);
}
```

---

### Example 3: Blueprint Event Binding

```cpp
// C++ setup for Blueprint events
void AMyShipActor::BeginPlay()
{
    Super::BeginPlay();

    UShipCustomizationComponent* CustomComp = FindComponentByClass<UShipCustomizationComponent>();
    if (CustomComp)
    {
        // Bind to loadout changed event
        CustomComp->OnLoadoutChanged.AddDynamic(this, &AMyShipActor::HandleLoadoutChanged);

        // Bind to part equipped event
        CustomComp->OnPartEquipped.AddDynamic(this, &AMyShipActor::HandlePartEquipped);

        // Bind to level up event
        CustomComp->OnPlayerLevelUp.AddDynamic(this, &AMyShipActor::HandleLevelUp);
    }
}

void AMyShipActor::HandleLoadoutChanged()
{
    UE_LOG(LogTemp, Log, TEXT("Loadout changed! Refreshing UI..."));
    RefreshStatsDisplay();
}

void AMyShipActor::HandlePartEquipped(EShipPartCategory Category, FName PartID)
{
    UE_LOG(LogTemp, Log, TEXT("Part equipped: %s in category %d"),
        *PartID.ToString(), static_cast<int>(Category));
    PlayEquipSound();
}

void AMyShipActor::HandleLevelUp(int32 OldLevel, int32 NewLevel)
{
    UE_LOG(LogTemp, Log, TEXT("Level up! %d -> %d"), OldLevel, NewLevel);
    ShowLevelUpScreen(NewLevel);
}
```

---

### Example 4: Creating Data Tables in Code

```cpp
// Create a ship part programmatically
FShipPartData CreateEngineData()
{
    FShipPartData Part;
    Part.PartID = FName("Engine_Test");
    Part.DisplayName = FText::FromString("Test Engine");
    Part.Description = FText::FromString("A test engine for debugging");
    Part.Category = EShipPartCategory::Engine;
    Part.Rarity = EShipPartRarity::Common;
    Part.LevelRequirement = 1;
    Part.UnlockCost = 0;
    Part.bUnlockedByDefault = true;

    // Set stats
    Part.StatModifiers.ThrustPower = 0.3f;  // +30% thrust
    Part.StatModifiers.Mass = 150.0f;       // +150 kg

    return Part;
}
```

---

### Example 5: Query All Unlocked Parts

```cpp
// Get all unlocked engines for UI display
void UCustomizationUI::PopulateEngineList()
{
    UShipCustomizationComponent* CustomComp = GetShipCustomizationComponent();
    if (!CustomComp) return;

    TArray<FShipPartData> UnlockedEngines = CustomComp->GetUnlockedPartsInCategory(
        EShipPartCategory::Engine
    );

    // Clear UI list
    EngineListWidget->ClearChildren();

    // Add each engine to UI
    for (const FShipPartData& Engine : UnlockedEngines)
    {
        UEngineListItem* Item = CreateWidget<UEngineListItem>(this, EngineListItemClass);
        Item->SetPartData(Engine);
        Item->OnEquipClicked.AddDynamic(this, &UCustomizationUI::HandleEngineEquipped);
        EngineListWidget->AddChild(Item);
    }
}
```

---

**Document Status**: COMPLETE
**Review Status**: READY FOR TECHNICAL REVIEW
**Recommended For**: Programmers, Blueprint Scripters, Technical Designers
