// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "ShipCustomizationSystem.generated.h"

// Forward declarations
class UStaticMeshComponent;
class UMaterialInterface;

/**
 * Ship component types
 */
UENUM(BlueprintType)
enum class EShipComponentType : uint8
{
    Hull            UMETA(DisplayName = "Hull"),
    Engine          UMETA(DisplayName = "Engine"),
    Shield          UMETA(DisplayName = "Shield"),
    Weapon          UMETA(DisplayName = "Weapon"),
    PowerPlant      UMETA(DisplayName = "Power Plant"),
    Reactor         UMETA(DisplayName = "Reactor"),
    Thruster        UMETA(DisplayName = "Thruster"),
    Sensor          UMETA(DisplayName = "Sensor"),
    Computer        UMETA(DisplayName = "Computer"),
    LifeSupport     UMETA(DisplayName = "Life Support"),
    CargoHold       UMETA(DisplayName = "Cargo Hold"),
    FuelTank        UMETA(DisplayName = "Fuel Tank")
};

/**
 * Component rarity/quality
 */
UENUM(BlueprintType)
enum class EComponentRarity : uint8
{
    Standard        UMETA(DisplayName = "Standard"),
    Enhanced        UMETA(DisplayName = "Enhanced"),
    Advanced        UMETA(DisplayName = "Advanced"),
    Military        UMETA(DisplayName = "Military"),
    Prototype       UMETA(DisplayName = "Prototype"),
    Legendary       UMETA(DisplayName = "Legendary")
};

/**
 * Ship size classes
 */
UENUM(BlueprintType)
enum class EShipSizeClass : uint8
{
    Fighter         UMETA(DisplayName = "Fighter"),
    Interceptor     UMETA(DisplayName = "Interceptor"),
    Corvette        UMETA(DisplayName = "Corvette"),
    Frigate         UMETA(DisplayName = "Frigate"),
    Destroyer       UMETA(DisplayName = "Destroyer"),
    Cruiser         UMETA(DisplayName = "Cruiser"),
    Battleship      UMETA(DisplayName = "Battleship"),
    Carrier         UMETA(DisplayName = "Carrier"),
    Freighter       UMETA(DisplayName = "Freighter"),
    Capital         UMETA(DisplayName = "Capital")
};

/**
 * Paint scheme types
 */
UENUM(BlueprintType)
enum class EPaintScheme : uint8
{
    Solid           UMETA(DisplayName = "Solid Color"),
    TwoTone         UMETA(DisplayName = "Two Tone"),
    Camouflage      UMETA(DisplayName = "Camouflage"),
    Racing          UMETA(DisplayName = "Racing Stripes"),
    Military        UMETA(DisplayName = "Military"),
    Corporate       UMETA(DisplayName = "Corporate"),
    Pirate          UMETA(DisplayName = "Pirate"),
    Custom          UMETA(DisplayName = "Custom")
};

/**
 * Ship component data
 */
USTRUCT(BlueprintType)
struct FShipComponentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    FString ComponentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    FString ComponentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    EShipComponentType ComponentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    EComponentRarity Rarity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    FString Manufacturer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    int32 ComponentLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    float PowerRequirement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    float Mass; // kg

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    int32 Cost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    TMap<FString, float> Stats; // Generic stats (speed, armor, etc.)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    bool bIsInstalled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Component")
    FString SlotID; // Which slot it's installed in

    FShipComponentData()
        : ComponentType(EShipComponentType::Hull)
        , Rarity(EComponentRarity::Standard)
        , ComponentLevel(1)
        , PowerRequirement(10.0f)
        , Mass(100.0f)
        , Cost(1000)
        , bIsInstalled(false)
    {}
};

/**
 * Ship loadout configuration
 */
USTRUCT(BlueprintType)
struct FShipLoadout
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
    FString LoadoutID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
    FString LoadoutName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
    TMap<FString, FShipComponentData> Components; // SlotID -> Component

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
    FLinearColor PrimaryColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
    FLinearColor SecondaryColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
    EPaintScheme PaintScheme;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
    FString DecalID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
    bool bIsFavorite;

    FShipLoadout()
        : PrimaryColor(FLinearColor::White)
        , SecondaryColor(FLinearColor::Gray)
        , PaintScheme(EPaintScheme::Solid)
        , bIsFavorite(false)
    {}
};

/**
 * Ship template/blueprint
 */
USTRUCT(BlueprintType)
struct FShipTemplate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    FString TemplateID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    FString ShipName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    EShipSizeClass SizeClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    FString Manufacturer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    TMap<FString, EShipComponentType> ComponentSlots; // SlotID -> Required Type

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    int32 MaxHardpoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    float MaxPowerOutput;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    float MaxMass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    int32 BaseCost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Template")
    TSoftObjectPtr<UStaticMesh> ShipMesh;

    FShipTemplate()
        : SizeClass(EShipSizeClass::Fighter)
        , MaxHardpoints(4)
        , MaxPowerOutput(1000.0f)
        , MaxMass(10000.0f)
        , BaseCost(50000)
    {}
};

/**
 * Component slot data
 */
USTRUCT(BlueprintType)
struct FComponentSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    FString SlotID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    FString SlotName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    EShipComponentType AcceptedType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    bool bIsRequired;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    bool bIsOccupied;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    FString InstalledComponentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
    FVector SlotLocation; // Visual position on ship

    FComponentSlot()
        : AcceptedType(EShipComponentType::Hull)
        , bIsRequired(false)
        , bIsOccupied(false)
        , SlotLocation(FVector::ZeroVector)
    {}
};

/**
 * Ship performance statistics
 */
USTRUCT(BlueprintType)
struct FShipPerformanceStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float Acceleration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxShield;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ShieldRegenRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float Armor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PowerOutput;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PowerConsumption;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CargoCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FuelCapacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float SensorRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TotalMass;

    FShipPerformanceStats()
        : MaxSpeed(1000.0f)
        , Acceleration(100.0f)
        , MaxShield(1000.0f)
        , ShieldRegenRate(50.0f)
        , Armor(500.0f)
        , PowerOutput(1000.0f)
        , PowerConsumption(500.0f)
        , CargoCapacity(100.0f)
        , FuelCapacity(1000.0f)
        , SensorRange(5000.0f)
        , TotalMass(10000.0f)
    {}
};

/**
 * Customization preset
 */
USTRUCT(BlueprintType)
struct FCustomizationPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    FString PresetID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    FString PresetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    FShipLoadout Loadout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset")
    bool bIsDefault;

    FCustomizationPreset()
        : bIsDefault(false)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComponentInstalled, const FString&, ComponentID, const FString&, SlotID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComponentRemoved, const FString&, ComponentID, const FString&, SlotID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadoutChanged, const FString&, LoadoutID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPaintSchemeChanged, EPaintScheme, NewScheme);

/**
 * Ship Customization System Component
 * Manages ship components, loadouts, paint schemes, and visual customization
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ALEXANDER_API UShipCustomizationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UShipCustomizationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Component Management
    UFUNCTION(BlueprintCallable, Category = "Customization")
    bool InstallComponent(const FString& ComponentID, const FString& SlotID);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    bool RemoveComponent(const FString& SlotID);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    bool SwapComponents(const FString& SlotID1, const FString& SlotID2);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    FShipComponentData GetInstalledComponent(const FString& SlotID, bool& bSuccess) const;

    UFUNCTION(BlueprintCallable, Category = "Customization")
    TArray<FShipComponentData> GetAllInstalledComponents() const;

    UFUNCTION(BlueprintCallable, Category = "Customization")
    bool CanInstallComponent(const FString& ComponentID, const FString& SlotID) const;

    UFUNCTION(BlueprintCallable, Category = "Customization")
    FString FindCompatibleSlot(const FString& ComponentID) const;

    // Inventory Management
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void AddComponentToInventory(const FShipComponentData& Component);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    bool RemoveComponentFromInventory(const FString& ComponentID);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    TArray<FShipComponentData> GetInventoryComponents() const;

    UFUNCTION(BlueprintCallable, Category = "Customization")
    TArray<FShipComponentData> GetInventoryComponentsByType(EShipComponentType Type) const;

    UFUNCTION(BlueprintCallable, Category = "Customization")
    int32 GetInventoryCount() const;

    // Loadout Management
    UFUNCTION(BlueprintCallable, Category = "Customization")
    FString SaveLoadout(const FString& LoadoutName);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    bool LoadLoadout(const FString& LoadoutID);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    bool DeleteLoadout(const FString& LoadoutID);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    TArray<FShipLoadout> GetSavedLoadouts() const;

    UFUNCTION(BlueprintCallable, Category = "Customization")
    FShipLoadout GetCurrentLoadout() const;

    // Paint & Appearance
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetPrimaryColor(FLinearColor Color);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetSecondaryColor(FLinearColor Color);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetPaintScheme(EPaintScheme Scheme);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void ApplyDecal(const FString& DecalID);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void ApplyMaterialPreset(const FString& PresetID);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    FLinearColor GetPrimaryColor() const;

    UFUNCTION(BlueprintCallable, Category = "Customization")
    FLinearColor GetSecondaryColor() const;

    UFUNCTION(BlueprintCallable, Category = "Customization")
    EPaintScheme GetPaintScheme() const;

    // Performance Calculation
    UFUNCTION(BlueprintCallable, Category = "Customization")
    FShipPerformanceStats CalculatePerformance() const;

    UFUNCTION(BlueprintCallable, Category = "Customization")
    float GetTotalPowerConsumption() const;

    UFUNCTION(BlueprintCallable, Category = "Customization")
    float GetTotalMass() const;

    UFUNCTION(BlueprintCallable, Category = "Customization")
    bool IsLoadoutValid() const;

    UFUNCTION(BlueprintCallable, Category = "Customization")
    TArray<FString> GetLoadoutWarnings() const;

    // Ship Template
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetShipTemplate(const FShipTemplate& Template);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    FShipTemplate GetShipTemplate() const;

    UFUNCTION(BlueprintCallable, Category = "Customization")
    TArray<FComponentSlot> GetComponentSlots() const;

    // Visual Updates
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void UpdateShipVisuals();

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void ApplyColorToMesh(UStaticMeshComponent* Mesh);

    // Presets
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SavePreset(const FString& PresetName, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    bool LoadPreset(const FString& PresetID);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    TArray<FCustomizationPreset> GetAvailablePresets() const;

    // Comparison & Stats
    UFUNCTION(BlueprintCallable, Category = "Customization")
    FShipPerformanceStats CompareLoadouts(const FString& LoadoutID1, const FString& LoadoutID2) const;

    UFUNCTION(BlueprintCallable, Category = "Customization")
    float GetComponentStatValue(const FString& ComponentID, const FString& StatName) const;

    UFUNCTION(BlueprintCallable, Category = "Customization")
    int32 GetLoadoutCost() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Customization")
    FOnComponentInstalled OnComponentInstalled;

    UPROPERTY(BlueprintAssignable, Category = "Customization")
    FOnComponentRemoved OnComponentRemoved;

    UPROPERTY(BlueprintAssignable, Category = "Customization")
    FOnLoadoutChanged OnLoadoutChanged;

    UPROPERTY(BlueprintAssignable, Category = "Customization")
    FOnPaintSchemeChanged OnPaintSchemeChanged;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    FShipTemplate CurrentShipTemplate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    int32 MaxInventorySize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    bool bAutoCalculatePerformance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    bool bEnforceRequirements;

private:
    UPROPERTY()
    TMap<FString, FComponentSlot> ComponentSlots;

    UPROPERTY()
    TArray<FShipComponentData> ComponentInventory;

    UPROPERTY()
    TMap<FString, FShipLoadout> SavedLoadouts;

    UPROPERTY()
    TArray<FCustomizationPreset> Presets;

    FShipLoadout CurrentLoadout;
    FShipPerformanceStats CachedPerformance;
    bool bPerformanceDirty;

    // Helper functions
    void InitializeSlots();
    void RecalculatePerformance();
    FShipComponentData* FindComponentInInventory(const FString& ComponentID);
    bool ValidateComponentRequirements(const FShipComponentData& Component) const;
    float CalculateStatBonus(const FString& StatName) const;
    void ApplyComponentVisuals(const FShipComponentData& Component, const FString& SlotID);
    void RemoveComponentVisuals(const FString& SlotID);
    FString GenerateLoadoutID() const;
};
