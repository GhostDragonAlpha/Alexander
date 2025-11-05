#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Planet.h"
#include "PlanetaryLandingZone.generated.h"

UENUM(BlueprintType)
enum class ELandingZoneType : uint8
{
    Primary,
    Secondary,
    Emergency,
    Military,
    Commercial,
    Industrial,
    Research,
    Mining,
    Agricultural,
    Tourist
};

UENUM(BlueprintType)
enum class ETerrainType : uint8
{
    Flat,
    Hilly,
    Mountainous,
    Desert,
    Arctic,
    Jungle,
    Ocean,
    Swamp,
    Crater,
    Canyon
};

UENUM(BlueprintType)
enum class EAtmosphereType : uint8
{
    None,
    Thin,
    Normal,
    Dense,
    Toxic,
    Corrosive,
    Radioactive
};

UENUM(BlueprintType)
enum class ELandingZoneStatus : uint8
{
    Active,
    UnderConstruction,
    Abandoned,
    Damaged,
    Quarantined,
    Restricted,
    Offline
};

USTRUCT(BlueprintType)
struct FPlanetaryLandingPad
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PadID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxShipSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOccupied;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* OccupyingShip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresPermission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> AllowedFactions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, FString> PadProperties;
};

USTRUCT(BlueprintType)
struct FLandingZoneServices
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRefuelingAvailable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRepairAvailable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCargoTransfer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCustomization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bMedicalServices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bTradingPost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bMissionBoard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAccommodation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RefuelCost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RepairCost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> ServiceCosts;
};

USTRUCT(BlueprintType)
struct FLandingZoneEnvironment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETerrainType TerrainType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAtmosphereType AtmosphereType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Gravity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AirPressure;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WindSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Visibility;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasOxygen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasRadiation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RadiationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> EnvironmentalHazards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInterface* TerrainMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UParticleSystem* WeatherEffect;
};

USTRUCT(BlueprintType)
struct FLandingZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ELandingZoneType ZoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ELandingZoneStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class APlanet* ParentPlanet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FPlanetaryLandingPad> LandingPads;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLandingZoneServices Services;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLandingZoneEnvironment Environment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxPopulation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentPopulation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, float> FactionPresence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresLandingClearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> RequiredPermits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FName, FString> ZoneProperties;
};

UCLASS()
class ALEXANDER_API APlanetaryLandingZone : public AActor
{
    GENERATED_BODY()

public:
    APlanetaryLandingZone();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Landing Zone Management
    UFUNCTION(BlueprintCallable, Category = "Landing Zone")
    void InitializeLandingZone(const FLandingZone& ZoneData);

    UFUNCTION(BlueprintCallable, Category = "Landing Zone")
    void ShutdownLandingZone();

    UFUNCTION(BlueprintCallable, Category = "Landing Zone")
    bool IsLandingZoneActive() const { return CurrentStatus == ELandingZoneStatus::Active; }

    FLandingZone* GetLandingZoneData() { return &LandingZoneData; }

    // Landing Pad Management
    FPlanetaryLandingPad* GetAvailableLandingPad(float ShipSize, const FName& Faction = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Landing Pads")
    bool ReserveLandingPad(const FName& PadID, AActor* Ship);

    UFUNCTION(BlueprintCallable, Category = "Landing Pads")
    void ReleaseLandingPad(const FName& PadID);

    UFUNCTION(BlueprintCallable, Category = "Landing Pads")
    bool IsPadAvailable(const FName& PadID);

    UFUNCTION(BlueprintCallable, Category = "Landing Pads")
    TArray<FPlanetaryLandingPad> GetAllLandingPads();

    UFUNCTION(BlueprintCallable, Category = "Landing Pads")
    TArray<FPlanetaryLandingPad> GetOccupiedPads();

    // Landing Operations
    UFUNCTION(BlueprintCallable, Category = "Landing Operations")
    bool CanShipLand(AActor* Ship, const FName& Faction = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Landing Operations")
    bool RequestLandingClearance(AActor* Ship, const FName& Faction = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Landing Operations")
    void ProcessLanding(AActor* Ship, const FName& PadID);

    UFUNCTION(BlueprintCallable, Category = "Landing Operations")
    void ProcessTakeoff(AActor* Ship, const FName& PadID);

    UFUNCTION(BlueprintCallable, Category = "Landing Operations")
    void AbortLanding(AActor* Ship);

    // Services Management
    UFUNCTION(BlueprintCallable, Category = "Services")
    bool IsServiceAvailable(const FName& ServiceType);

    UFUNCTION(BlueprintCallable, Category = "Services")
    float GetServiceCost(const FName& ServiceType);

    UFUNCTION(BlueprintCallable, Category = "Services")
    bool UseService(APlayerState* Player, const FName& ServiceType, AActor* Ship = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Services")
    void UpdateServices();

    // Environmental Management
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateEnvironmentalConditions(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    FLandingZoneEnvironment GetCurrentEnvironment();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    bool IsEnvironmentSafe();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    TArray<FName> GetActiveHazards();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void TriggerEnvironmentalEvent(const FName& EventType);

    // Population Management
    UFUNCTION(BlueprintCallable, Category = "Population")
    void UpdatePopulation();

    UFUNCTION(BlueprintCallable, Category = "Population")
    bool CanAcceptNewResidents();

    UFUNCTION(BlueprintCallable, Category = "Population")
    void AddResidents(int32 Count, const FName& Faction = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "Population")
    void RemoveResidents(int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Population")
    TMap<FName, float> GetFactionPresence();

    // Faction Management
    UFUNCTION(BlueprintCallable, Category = "Factions")
    float GetFactionInfluence(const FName& Faction);

    UFUNCTION(BlueprintCallable, Category = "Factions")
    void UpdateFactionInfluence(const FName& Faction, float Change);

    UFUNCTION(BlueprintCallable, Category = "Factions")
    bool IsFactionWelcome(const FName& Faction);

    UFUNCTION(BlueprintCallable, Category = "Factions")
    TArray<FName> GetControllingFactions();

    // Zone Status Management
    UFUNCTION(BlueprintCallable, Category = "Zone Status")
    void SetZoneStatus(ELandingZoneStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "Zone Status")
    ELandingZoneStatus GetZoneStatus() const { return CurrentStatus; }

    UFUNCTION(BlueprintCallable, Category = "Zone Status")
    void HandleZoneDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Zone Status")
    void RepairZone(float RepairAmount);

    // Trading and Economy
    UFUNCTION(BlueprintCallable, Category = "Economy")
    void UpdateLocalEconomy();

    UFUNCTION(BlueprintCallable, Category = "Economy")
    TMap<FName, float> GetLocalPrices();

    UFUNCTION(BlueprintCallable, Category = "Economy")
    void ProcessTradeActivity(const FName& ItemID, int32 Quantity, bool bBuy);

    // Mission Integration
    UFUNCTION(BlueprintCallable, Category = "Missions")
    void GenerateLocalMissions();

    UFUNCTION(BlueprintCallable, Category = "Missions")
    TArray<FName> GetAvailableMissionTypes();

    UFUNCTION(BlueprintCallable, Category = "Missions")
    bool SupportsMissionType(EMissionType MissionType);

    // Utilities
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    float GetDistanceToZone(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    bool IsLocationInZone(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    FVector GetNearestSafeLocation(const FVector& DangerousLocation);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    TArray<AActor*> GetShipsInZone();

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    TArray<APlayerState*> GetPlayersInZone();

protected:
    // Landing Zone Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landing Zone")
    FLandingZone LandingZoneData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landing Zone")
    ELandingZoneStatus CurrentStatus;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* ZoneBounds;

    // Landing Pad Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Landing Pads")
    TMap<FName, FPlanetaryLandingPad> LandingPadMap;

    // Ship Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ships")
    TSet<AActor*> ShipsInZone;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ships")
    TMap<AActor*, FName> ShipPadAssignments;

    // Environmental Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment")
    FLandingZoneEnvironment CurrentEnvironment;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environment")
    TArray<FName> ActiveHazards;

    // Economic Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Economy")
    TMap<FName, float> LocalPrices;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Economy")
    TMap<FName, int32> LocalDemand;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ServiceUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float EnvironmentUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float EconomyUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableEnvironmentalHazards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableDynamicPricing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableFactionInfluence;

private:
    // Internal Functions
    void InitializeLandingPads();
    void SetupZoneBounds();
    void UpdateZoneServices(float DeltaTime);
    void ProcessEnvironmentalEffects(float DeltaTime);
    void UpdateLocalMarket(float DeltaTime);
    void HandleFactions(float DeltaTime);

    // Landing Pad Internal
    FName GeneratePadID();
    void CreateLandingPad(const FVector& Location, const FRotator& Rotation, float MaxSize);
    void UpdatePadStatus(const FName& PadID);

    // Environmental Internal
    void ApplyWeatherEffects();
    void ApplyRadiationDamage();
    void ProcessAtmosphericConditions();
    void UpdateTerrainConditions();

    // Service Internal
    void ProcessRefuelingService();
    void ProcessRepairService();
    void ProcessCargoTransfer();
    void UpdateServiceCosts();

    // Population Internal
    void CalculatePopulationGrowth();
    void UpdateFactionDemographics();
    void ProcessMigration();

    // Economic Internal
    void CalculateSupplyDemand();
    void UpdatePriceModifiers();
    void ProcessTradeFlow();

    // Hazards Internal
    void SpawnEnvironmentalHazard(const FName& HazardType);
    void RemoveEnvironmentalHazard(const FName& HazardType);
    bool IsHazardActive(const FName& HazardType);

    // Timers
    UPROPERTY()
    FTimerHandle ServiceUpdateTimer;

    UPROPERTY()
    FTimerHandle EnvironmentUpdateTimer;

    UPROPERTY()
    FTimerHandle EconomyUpdateTimer;

    UPROPERTY()
    FTimerHandle PopulationUpdateTimer;

    // Constants
    static constexpr float DEFAULT_SERVICE_UPDATE_INTERVAL = 5.0f;
    static constexpr float DEFAULT_ENVIRONMENT_UPDATE_INTERVAL = 1.0f;
    static constexpr float DEFAULT_ECONOMY_UPDATE_INTERVAL = 60.0f;
    static constexpr float DEFAULT_POPULATION_UPDATE_INTERVAL = 300.0f; // 5 minutes
    static constexpr int32 MAX_LANDING_PADS = 50;
    static constexpr float DEFAULT_ZONE_RADIUS = 5000.0f;

    // Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShipLanded, AActor*, Ship, const FName&, PadID);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShipTakeoff, AActor*, Ship, const FName&, PadID);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZoneStatusChanged, ELandingZoneStatus, OldStatus, ELandingZoneStatus, NewStatus);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnvironmentalEvent, const FName&, EventType);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFactionInfluenceChanged, const FName&, Faction, float, NewInfluence);

public:
    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Landing Zone Events")
    FOnShipLanded OnShipLanded;

    UPROPERTY(BlueprintAssignable, Category = "Landing Zone Events")
    FOnShipTakeoff OnShipTakeoff;

    UPROPERTY(BlueprintAssignable, Category = "Landing Zone Events")
    FOnZoneStatusChanged OnZoneStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Landing Zone Events")
    FOnEnvironmentalEvent OnEnvironmentalEvent;

    UPROPERTY(BlueprintAssignable, Category = "Landing Zone Events")
    FOnFactionInfluenceChanged OnFactionInfluenceChanged;
};